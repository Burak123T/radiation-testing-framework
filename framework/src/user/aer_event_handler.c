#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "aer_monitor.skel.h"

static struct aer_monitor_bpf *aer_skel;
static struct ring_buffer *rb_aer;
static aer_handler_config_t *aer_config;

/**
 * FIXME: turn data_loc into actual string
 */
int handle_aer_event(void *ctx, void *data, uint64_t data_sz)
{
	aer_event_t *event = data;
	char *log_string = (char *)malloc(128);

	sprintf(log_string, 
	"[RAS aer_event] CPU: %u | Severity: %u | Device Name: %s", event->cpu, event->severity, event->dev_name);
	logger_log(log_string);
	free(log_string);
	return 0;
}

int setup_aer_monitor()
{
	struct bpf_object_open_opts opts = {
		.sz = sizeof(struct bpf_object_open_opts),
		.object_name = "aer_monitor",
	};

	aer_skel = aer_monitor_bpf__open_opts(&opts);
	if (!aer_skel) {
		fprintf(stderr, "Failed to open aer monitor BPF program\n");
		return -1;
	}

	if (aer_monitor_bpf__load(aer_skel) || aer_monitor_bpf__attach(aer_skel)) {
		fprintf(stderr, "Failed to load or attach aer monitor BPF program\n");
		return -1;
	}

	rb_aer = ring_buffer__new(bpf_map__fd(aer_skel->maps.aer_events), handle_aer_event, NULL,
				  NULL);
	if (!rb_aer) {
		fprintf(stderr, "Failed to create ring buffer for aer events\n");
		return -1;
	}

	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework");

	// pin all of the maps
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/map");
	bpf_map__pin(aer_skel->maps.aer_events,
		     "/sys/fs/bpf/radiation_testing_framework/map/aer_events");

	// pin all of the programs
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/prog");
	bpf_program__pin(aer_skel->progs.trace_aer_event,
			 "/sys/fs/bpf/radiation_testing_framework/prog/trace_aer_event");

	return 0;
}

int poll_aer_events()
{
	return ring_buffer__poll(rb_aer, 100);
}

void cleanup_aer_monitor()
{
	// unpin the maps
	bpf_map__unpin(aer_skel->maps.aer_events,
		       "/sys/fs/bpf/radiation_testing_framework/map/aer_events");
	// unping the programs
	bpf_program__unpin(aer_skel->progs.trace_aer_event,
			   "/sys/fs/bpf/radiation_testing_framework/prog/trace_aer_event");
	ring_buffer__free(rb_aer);
	aer_monitor_bpf__destroy(aer_skel);
	if (aer_config->param2) {
		free(aer_config->param2);
	}
	free(aer_config);
}

int add_aer_config_key(const char *name, const char *value)
{
	if (!aer_config) {
		aer_config = (aer_handler_config_t *)malloc(sizeof(aer_handler_config_t));
		if (!aer_config) {
			fprintf(stderr, "Failed to allocate memory for aer handler config\n");
			return 0; // error
		}
	}

	if (strcmp(name, "param1") == 0) {
		aer_config->param1 = atoi(value);
	} else if (strcmp(name, "param2") == 0) {
		aer_config->param2 = (char *)malloc(strlen(value) + 1);
		if (!aer_config->param2) {
			fprintf(stderr,
				"Failed to allocate memory for aer handler config param2\n");
			return 0; // error
		}
		strcpy(aer_config->param2, value);
	} else {
		fprintf(stderr, "Unknown configuration key for aer handler: %s\n", name);
		return 0; // not found
	}

	return 1; // success
}

handler_t aer_handler = {
	.name = "aer_handler",
	.setup = setup_aer_monitor,
	.poll = poll_aer_events,
	.cleanup = cleanup_aer_monitor,
	.add_config_key = add_aer_config_key,
};