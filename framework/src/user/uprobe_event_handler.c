#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "uprobe_monitor.skel.h"

static struct uprobe_monitor_bpf *uprobe_skel;
static uprobe_handler_config_t *uprobe_config;

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}

int handle_uprobe_event(void *ctx, void *data, uint64_t data_sz)
{
	return 0;
}

int setup_uprobe_monitor()
{
	struct bpf_object_open_opts opts = {
		.sz = sizeof(struct bpf_object_open_opts),
		.object_name = "uprobe_monitor",
	};

	libbpf_set_print(libbpf_print_fn);

	uprobe_skel = uprobe_monitor_bpf__open_opts(&opts);
	if (!uprobe_skel) {
		fprintf(stderr, "Failed to open uprobe monitor BPF program\n");
		return -1;
	}

	if (uprobe_monitor_bpf__load(uprobe_skel) || uprobe_monitor_bpf__attach(uprobe_skel)) {
		fprintf(stderr, "Failed to load uprobe monitor BPF program\n");
		return -1;
	}

	printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
	       "to see output of the BPF programs.\n");

	return 0;
}

int poll_uprobe_events()
{
	return 0;
}

void cleanup_uprobe_monitor()
{
	uprobe_monitor_bpf__destroy(uprobe_skel);
	if (uprobe_config->param2) {
		free(uprobe_config->param2);
	}
	free(uprobe_config);
}

int add_uprobe_config_key(const char *name, const char *value)
{
	if (!uprobe_config) {
		uprobe_config = (uprobe_handler_config_t *)malloc(sizeof(uprobe_handler_config_t));
		if (!uprobe_config) {
			fprintf(stderr, "Failed to allocate memory for uprobe handler config\n");
			return 0; // error
		}
	}

	if (strcmp(name, "param1") == 0) {
		uprobe_config->param1 = atoi(value);
	} else if (strcmp(name, "param2") == 0) {
		uprobe_config->param2 = (char *)malloc(strlen(value) + 1);
		if (!uprobe_config->param2) {
			fprintf(stderr,
				"Failed to allocate memory for uprobe handler config param2\n");
			return 0; // error
		}
		strcpy(uprobe_config->param2, value);
	} else {
		fprintf(stderr, "Unknown configuration key for uprobe handler: %s\n", name);
		return 0; // not found
	}

	return 1; // success
}

handler_t uprobe_handler = {
	.name = "uprobe_handler",
	.setup = setup_uprobe_monitor,
	.poll = poll_uprobe_events,
	.cleanup = cleanup_uprobe_monitor,
	.add_config_key = add_uprobe_config_key,
};
