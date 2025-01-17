#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "mem_monitor.skel.h"

static struct mem_monitor_bpf *mem_skel;
static struct ring_buffer *rb_mem;
static mem_handler_config_t *mem_config;

int handle_mem_event(void *ctx, void *data, uint64_t data_sz)
{
	mem_event_t *event = data;
	char *log_string = (char *)malloc(100);
	sprintf(log_string, "[MEMORY ERROR] Page: %lu | Type: %u | PID: %u | Time: %lu ns\n",
		event->page, event->error_type, event->pid, event->timestamp);
	logger_log(log_string);
	free(log_string);
	return 0;
}

int setup_mem_monitor()
{
	struct bpf_object_open_opts opts = {
		.sz = sizeof(struct bpf_object_open_opts),
		.object_name = "mem_monitor",
	};

	mem_skel = mem_monitor_bpf__open_opts(&opts);
	if (!mem_skel) {
		fprintf(stderr, "Failed to open Memory monitor BPF program\n");
		return -1;
	}

	if (mem_monitor_bpf__load(mem_skel) || mem_monitor_bpf__attach(mem_skel)) {
		fprintf(stderr, "Failed to load or attach Memory monitor BPF program\n");
		return -1;
	}

	rb_mem = ring_buffer__new(bpf_map__fd(mem_skel->maps.mem_events), handle_mem_event, NULL,
				  NULL);
	if (!rb_mem) {
		fprintf(stderr, "Failed to create ring buffer for Memory events\n");
		return -1;
	}

	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework");

	// pin all of the maps
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/map");
	bpf_map__pin(mem_skel->maps.mem_events,
		     "/sys/fs/bpf/radiation_testing_framework/map/mem_events");

	// pin all of the programs
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/prog");
	bpf_program__pin(mem_skel->progs.trace_memory_failure,
			 "/sys/fs/bpf/radiation_testing_framework/prog/trace_memory_failure");

	return 0;
}

int poll_mem_events()
{
	return ring_buffer__poll(rb_mem, 100);
}

void cleanup_mem_monitor()
{
	// unpin the maps
	bpf_map__unpin(mem_skel->maps.mem_events,
		       "/sys/fs/bpf/radiation_testing_framework/map/mem_events");
	// unping the programs
	bpf_program__unpin(mem_skel->progs.trace_memory_failure,
			   "/sys/fs/bpf/radiation_testing_framework/prog/trace_memory_failure");
	ring_buffer__free(rb_mem);
	mem_monitor_bpf__destroy(mem_skel);
	if (mem_config->param2) {
		free(mem_config->param2);
	}
	free(mem_config);
}

int add_mem_config_key(const char *name, const char *value)
{
	if (!mem_config) {
		mem_config = (mem_handler_config_t *)malloc(sizeof(mem_handler_config_t));
		if (!mem_config) {
			fprintf(stderr, "Failed to allocate memory for Memory handler config\n");
			return 0; //  error
		}
	}

	if (strcmp(name, "param1") == 0) {
		mem_config->param1 = atoi(value);
	} else if (strcmp(name, "param2") == 0) {
		mem_config->param2 = malloc(strlen(value) + 1);
		if (!mem_config->param2) {
			fprintf(stderr, "Failed to allocate memory for Memory handler config\n");
			return 0; // error
		}
		strcpy(mem_config->param2, value);
	} else {
		fprintf(stderr, "Unknown configuration key: %s\n", name);
		return 0; // not found
	}

	return 1; // success
}

handler_t mem_handler = {
	.name = "mem_handler",
	.setup = setup_mem_monitor,
	.poll = poll_mem_events,
	.cleanup = cleanup_mem_monitor,
	.add_config_key = add_mem_config_key,
};