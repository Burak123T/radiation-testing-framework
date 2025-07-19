#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "mc_monitor.skel.h"

static struct mc_monitor_bpf *mc_skel;
static struct ring_buffer *rb_mc;
static mc_handler_config_t *mc_config;

/**
 * FIXME: turn data locations into strings
 */
int handle_mc_event(void *ctx, void *data, uint64_t data_sz)
{
	mc_event_t *event = data;
	char *log_string = (char *)malloc(256);
	sprintf(log_string,
		"[RAS mc_event] cpu: %d | time: %ld | error_count: %d | event_type: %d | mc_index: %d | address: %ld | syndrome: %lu | data_loc_msg: %u | data_loc_label: %u | data_loc_driver_details: %u",
		event->cpu, event->time, event->error_count, event->error_type, event->mc_index,
		event->address, event->syndrome, event->__data_loc_msg, event->__data_loc_label,
		event->__data_loc_driver_detail);
	logger_log(log_string);
	free(log_string);
	return 0;
}

int setup_mc_monitor()
{
	struct bpf_object_open_opts opts = {
		.sz = sizeof(struct bpf_object_open_opts),
		.object_name = "mc_monitor",
	};

	mc_skel = mc_monitor_bpf__open_opts(&opts);
	if (!mc_skel) {
		fprintf(stderr, "Failed to open MC monitor BPF program\n");
		return -1;
	}

	if (mc_monitor_bpf__load(mc_skel) || mc_monitor_bpf__attach(mc_skel)) {
		fprintf(stderr, "Failed to load or attach MC monitor BPF program\n");
		return -1;
	}

	rb_mc = ring_buffer__new(bpf_map__fd(mc_skel->maps.mc_events), handle_mc_event, NULL, NULL);
	if (!rb_mc) {
		fprintf(stderr, "Failed to create ring buffer for MC events\n");
		return -1;
	}

	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework");

	// pin all of the maps
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/map");
	bpf_map__pin(mc_skel->maps.mc_events,
		     "/sys/fs/bpf/radiation_testing_framework/map/mc_events");

	// pin all of the programs
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/prog");
	bpf_program__pin(mc_skel->progs.trace_mc_event,
			 "/sys/fs/bpf/radiation_testing_framework/prog/trace_mc_event");

	return 0;
}

int poll_mc_events()
{
	return ring_buffer__poll(rb_mc, 100);
}

void cleanup_mc_monitor()
{
	// unpin the maps
	bpf_map__unpin(mc_skel->maps.mc_events,
		       "/sys/fs/bpf/radiation_testing_framework/map/mc_events");
	// unping the programs
	bpf_program__unpin(mc_skel->progs.trace_mc_event,
			   "/sys/fs/bpf/radiation_testing_framework/prog/trace_mc_event");
	ring_buffer__free(rb_mc);
	mc_monitor_bpf__destroy(mc_skel);
	if (mc_config->param2) {
		free(mc_config->param2);
	}
	free(mc_config);
}

int add_mc_config_key(const char *name, const char *value)
{
	if (!mc_config) {
		mc_config = (mc_handler_config_t *)malloc(sizeof(mc_handler_config_t));
		if (!mc_config) {
			fprintf(stderr, "Failed to allocate memory for MC handler config\n");
			return 0; // error
		}
	}

	if (strcmp(name, "param1") == 0) {
		mc_config->param1 = atoi(value);
	} else if (strcmp(name, "param2") == 0) {
		mc_config->param2 = (char *)malloc(strlen(value) + 1);
		if (!mc_config->param2) {
			fprintf(stderr, "Failed to allocate memory for MC handler config param2\n");
			return 0; // error
		}
		strcpy(mc_config->param2, value);
	} else {
		fprintf(stderr, "Unknown configuration key for MC handler: %s\n", name);
		return 0; // not found
	}

	return 1; // success
}

handler_t mc_handler = {
	.name = "mc_handler",
	.setup = setup_mc_monitor,
	.poll = poll_mc_events,
	.cleanup = cleanup_mc_monitor,
	.add_config_key = add_mc_config_key,
};