#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "arm_monitor.skel.h"

static struct arm_monitor_bpf *arm_skel;
static struct ring_buffer *rb_mc;
static arm_handler_config_t *arm_config;

int handle_arm_event(void *ctx, void *data, uint64_t data_sz)
{
	arm_event_t *event = data;
	char *log_string = (char *)malloc(128);

	sprintf(log_string, 
	"[RAS arm_event] affinity level: %d; MPIDR: %016lx; MIDR: %016lx; running state: %d; PSCI state: %d", 
	event->affinity, event->mpidr, event->midr, event->running_state, event->psci_state);
	logger_log(log_string);
	free(log_string);
	return 0;
}

int setup_arm_monitor()
{
	struct bpf_object_open_opts opts = {
		.sz = sizeof(struct bpf_object_open_opts),
		.object_name = "arm_monitor",
	};

	arm_skel = arm_monitor_bpf__open_opts(&opts);
	if (!arm_skel) {
		fprintf(stderr, "Failed to open ARM monitor BPF program\n");
		return -1;
	}

	if (arm_monitor_bpf__load(arm_skel) || arm_monitor_bpf__attach(arm_skel)) {
		fprintf(stderr, "Failed to load or attach ARM monitor BPF program\n");
		return -1;
	}

	rb_mc = ring_buffer__new(bpf_map__fd(arm_skel->maps.arm_events), handle_arm_event, NULL,
				  NULL);
	if (!rb_mc) {
		fprintf(stderr, "Failed to create ring buffer for ARM events\n");
		return -1;
	}

	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework");

	// pin all of the maps
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/map");
	bpf_map__pin(arm_skel->maps.arm_events,
		     "/sys/fs/bpf/radiation_testing_framework/map/arm_events");

	// pin all of the programs
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/prog");
	bpf_program__pin(arm_skel->progs.trace_arm_event,
			 "/sys/fs/bpf/radiation_testing_framework/prog/trace_arm_event");

	return 0;
}

int poll_arm_events()
{
	return ring_buffer__poll(rb_mc, 100);
}

void cleanup_arm_monitor()
{
	// unpin the maps
	bpf_map__unpin(arm_skel->maps.arm_events,
		       "/sys/fs/bpf/radiation_testing_framework/map/arm_events");
	// unping the programs
	bpf_program__unpin(arm_skel->progs.trace_arm_event,
			   "/sys/fs/bpf/radiation_testing_framework/prog/trace_arm_event");
	ring_buffer__free(rb_mc);
	arm_monitor_bpf__destroy(arm_skel);
	if (arm_config->param2) {
		free(arm_config->param2);
	}
	free(arm_config);
}

int add_arm_config_key(const char *name, const char *value)
{
	if (!arm_config) {
		arm_config = (arm_handler_config_t *)malloc(sizeof(arm_handler_config_t));
		if (!arm_config) {
			fprintf(stderr, "Failed to allocate memory for ARM handler config\n");
			return 0; // error
		}
	}

	if (strcmp(name, "param1") == 0) {
		arm_config->param1 = atoi(value);
	} else if (strcmp(name, "param2") == 0) {
		arm_config->param2 = (char *)malloc(strlen(value) + 1);
		if (!arm_config->param2) {
			fprintf(stderr,
				"Failed to allocate memory for ARM handler config param2\n");
			return 0; // error
		}
		strcpy(arm_config->param2, value);
	} else {
		fprintf(stderr, "Unknown configuration key for ARM handler: %s\n", name);
		return 0; // not found
	}

	return 1; // success
}

handler_t arm_handler = {
	.name = "arm_handler",
	.setup = setup_arm_monitor,
	.poll = poll_arm_events,
	.cleanup = cleanup_arm_monitor,
	.add_config_key = add_arm_config_key,
};