#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "nse_monitor.skel.h"

static struct nse_monitor_bpf *ns_skel;
static struct ring_buffer *rb_mc;
static ns_handler_config_t *ns_config;

int handle_ns_event(void *ctx, void *data, uint64_t data_sz)
{
	ns_event_t *event = data;
	char *log_string = (char *)malloc(128);

	sprintf(log_string, 
	"[RAS ns_event] CPU: %u | Severity: %d | Section Type: %pU | FRU ID: %pU | Length: %d", event->cpu, event->sev, event->sec_type, event->fru_id, event->len);
	logger_log(log_string);
	free(log_string);
	return 0;
}

int setup_nse_monitor()
{
	struct bpf_object_open_opts opts = {
		.sz = sizeof(struct bpf_object_open_opts),
		.object_name = "nse_monitor",
	};

	ns_skel = nse_monitor_bpf__open_opts(&opts);
	if (!ns_skel) {
		fprintf(stderr, "Failed to open ns monitor BPF program\n");
		return -1;
	}

	if (nse_monitor_bpf__load(ns_skel) || nse_monitor_bpf__attach(ns_skel)) {
		fprintf(stderr, "Failed to load or attach ns monitor BPF program\n");
		return -1;
	}

	rb_mc = ring_buffer__new(bpf_map__fd(ns_skel->maps.ns_events), handle_ns_event, NULL,
				  NULL);
	if (!rb_mc) {
		fprintf(stderr, "Failed to create ring buffer for ns events\n");
		return -1;
	}

	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework");

	// pin all of the maps
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/map");
	bpf_map__pin(ns_skel->maps.ns_events,
		     "/sys/fs/bpf/radiation_testing_framework/map/ns_events");

	// pin all of the programs
	create_directory_if_not_exists("/sys/fs/bpf/radiation_testing_framework/prog");
	bpf_program__pin(ns_skel->progs.trace_ns_event,
			 "/sys/fs/bpf/radiation_testing_framework/prog/trace_ns_event");

	return 0;
}

int poll_ns_events()
{
	return ring_buffer__poll(rb_mc, 100);
}

void cleanup_nse_monitor()
{
	// unpin the maps
	bpf_map__unpin(ns_skel->maps.ns_events,
		       "/sys/fs/bpf/radiation_testing_framework/map/ns_events");
	// unping the programs
	bpf_program__unpin(ns_skel->progs.trace_ns_event,
			   "/sys/fs/bpf/radiation_testing_framework/prog/trace_ns_event");
	ring_buffer__free(rb_mc);
	nse_monitor_bpf__destroy(ns_skel);
	if (ns_config->param2) {
		free(ns_config->param2);
	}
	free(ns_config);
}

int add_ns_config_key(const char *name, const char *value)
{
	if (!ns_config) {
		ns_config = (ns_handler_config_t *)malloc(sizeof(ns_handler_config_t));
		if (!ns_config) {
			fprintf(stderr, "Failed to allocate memory for ns handler config\n");
			return 0; // error
		}
	}

	if (strcmp(name, "param1") == 0) {
		ns_config->param1 = atoi(value);
	} else if (strcmp(name, "param2") == 0) {
		ns_config->param2 = (char *)malloc(strlen(value) + 1);
		if (!ns_config->param2) {
			fprintf(stderr,
				"Failed to allocate memory for ns handler config param2\n");
			return 0; // error
		}
		strcpy(ns_config->param2, value);
	} else {
		fprintf(stderr, "Unknown configuration key for ns handler: %s\n", name);
		return 0; // not found
	}

	return 1; // success
}

handler_t ns_handler = {
	.name = "nse_handler",
	.setup = setup_nse_monitor,
	.poll = poll_ns_events,
	.cleanup = cleanup_nse_monitor,
	.add_config_key = add_ns_config_key,
};