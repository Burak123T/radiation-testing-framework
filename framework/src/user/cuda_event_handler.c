#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "cuda_monitor.skel.h"

static struct cuda_monitor_bpf *cuda_skel;
static cuda_handler_config_t *cuda_config;

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}

int handle_cuda_event(void *ctx, void *data, uint64_t data_sz)
{
	cuda_event_t *event = data;
	char *log_string = (char *)malloc(100);

	sprintf(log_string, 
		"[CUDA] CPU: %u | PID: %u | Time: %lu ns", event->cpu, event->pid, event->time);
		logger_log(log_string);
		free(log_string);

	return 0;
}

int setup_cuda_monitor()
{
	struct bpf_object_open_opts opts = {
		.sz = sizeof(struct bpf_object_open_opts),
		.object_name = "cuda_monitor",
	};

	libbpf_set_print(libbpf_print_fn);

	cuda_skel = cuda_monitor_bpf__open_opts(&opts);
	if (!cuda_skel) {
		fprintf(stderr, "Failed to open cuda monitor BPF program\n");
		return -1;
	}

	if (cuda_monitor_bpf__load(cuda_skel) || cuda_monitor_bpf__attach(cuda_skel)) {
		fprintf(stderr, "Failed to load cuda monitor BPF program\n");
		return -1;
	}

	printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
	       "to see output of the BPF programs.\n");

	return 0;
}

int poll_cuda_events()
{
	return 0;
}

void cleanup_cuda_monitor()
{
	cuda_monitor_bpf__destroy(cuda_skel);
	if (cuda_config->param2) {
		free(cuda_config->param2);
	}
	free(cuda_config);
}

int add_cuda_config_key(const char *name, const char *value)
{
	if (!cuda_config) {
		cuda_config = (cuda_handler_config_t *)malloc(sizeof(cuda_handler_config_t));
		if (!cuda_config) {
			fprintf(stderr, "Failed to allocate memory for cuda handler config\n");
			return 0; // error
		}
	}

	if (strcmp(name, "param1") == 0) {
		cuda_config->param1 = atoi(value);
	} else if (strcmp(name, "param2") == 0) {
		cuda_config->param2 = (char *)malloc(strlen(value) + 1);
		if (!cuda_config->param2) {
			fprintf(stderr,
				"Failed to allocate memory for cuda handler config param2\n");
			return 0; // error
		}
		strcpy(cuda_config->param2, value);
	} else {
		fprintf(stderr, "Unknown configuration key for cuda handler: %s\n", name);
		return 0; // not found
	}

	return 1; // success
}

handler_t cuda_handler = {
	.name = "cuda_handler",
	.setup = setup_cuda_monitor,
	.poll = poll_cuda_events,
	.cleanup = cleanup_cuda_monitor,
	.add_config_key = add_cuda_config_key,
};
