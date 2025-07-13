#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "inferencetime_monitor.skel.h"

static struct inferencetime_monitor_bpf *inferencetime_skel;
static inferencetime_handler_config_t *inferencetime_config;

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}

int handle_write_event(void *ctx, void *data, uint64_t data_sz)
{
	return 0;
}

int setup_inferencetime_monitor()
{
	struct bpf_object_open_opts opts = {
		.sz = sizeof(struct bpf_object_open_opts),
		.object_name = "inferencetime_monitor",
	};

	libbpf_set_print(libbpf_print_fn);

	inferencetime_skel = inferencetime_monitor_bpf__open_opts(&opts);
	if (!inferencetime_skel) {
		fprintf(stderr, "Failed to open inferencetime monitor BPF program\n");
		return -1;
	}

	if (inferencetime_monitor_bpf__load(inferencetime_skel) || inferencetime_monitor_bpf__attach(inferencetime_skel)) {
		fprintf(stderr, "Failed to load inferencetime monitor BPF program\n");
		return -1;
	}

	printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
	       "to see output of the BPF programs.\n");

	return 0;
}

int poll_inferencetime_events()
{
	return 0;
}

void cleanup_inferencetime_monitor()
{
	inferencetime_monitor_bpf__destroy(inferencetime_skel);
	if (inferencetime_config->param2) {
		free(inferencetime_config->param2);
	}
	free(inferencetime_config);
}

int add_inferencetime_config_key(const char *name, const char *value)
{
	if (!inferencetime_config) {
		inferencetime_config = (inferencetime_handler_config_t *)malloc(sizeof(inferencetime_handler_config_t));
		if (!inferencetime_config) {
			fprintf(stderr, "Failed to allocate memory for inferencetime handler config\n");
			return 0; // error
		}
	}

	if (strcmp(name, "param1") == 0) {
		inferencetime_config->param1 = atoi(value);
	} else if (strcmp(name, "param2") == 0) {
		inferencetime_config->param2 = (char *)malloc(strlen(value) + 1);
		if (!inferencetime_config->param2) {
			fprintf(stderr,
				"Failed to allocate memory for inferencetime handler config param2\n");
			return 0; // error
		}
		strcpy(inferencetime_config->param2, value);
	} else {
		fprintf(stderr, "Unknown configuration key for inferencetime handler: %s\n", name);
		return 0; // not found
	}

	return 1; // success
}

handler_t inferencetime_handler = {
	.name = "inferencetime_handler",
	.setup = setup_inferencetime_monitor,
	.poll = poll_inferencetime_events,
	.cleanup = cleanup_inferencetime_monitor,
	.add_config_key = add_inferencetime_config_key,
};
