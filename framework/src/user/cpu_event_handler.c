#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "cpu_monitor.skel.h"

static struct cpu_monitor_bpf *cpu_skel;
static struct ring_buffer *rb_cpu;
static cpu_handler_config_t *cpu_config;

int handle_cpu_event(void *ctx, void *data, uint64_t data_sz) {
    mce_event_t *event = data;
    char* log_string = (char*)malloc(100);
    sprintf(log_string, "[CPU/MCE] CPU %d | Status: 0x%lx | Addr: 0x%lx | Misc: 0x%lx | Time: %lu ns\n",
           event->cpu, event->status, event->addr, event->misc, event->timestamp);
    logger_log(log_string);
    free(log_string);
    return 0;
}

int setup_cpu_monitor() {
    struct bpf_object_open_opts opts = {
        .sz = sizeof(struct bpf_object_open_opts),
        .object_name = "cpu_monitor",
    };

    cpu_skel = cpu_monitor_bpf__open_opts(&opts);
    if (!cpu_skel) {
        fprintf(stderr, "Failed to open CPU monitor BPF program\n");
        return -1;
    }

    if (cpu_monitor_bpf__load(cpu_skel) || cpu_monitor_bpf__attach(cpu_skel)) {
        fprintf(stderr, "Failed to load or attach CPU monitor BPF program\n");
        return -1;
    }

    rb_cpu = ring_buffer__new(bpf_map__fd(cpu_skel->maps.mce_events), handle_cpu_event, NULL, NULL);
    if (!rb_cpu) {
        fprintf(stderr, "Failed to create ring buffer for CPU events\n");
        return -1;
    }

    return 0;
}

int poll_cpu_events() {
    return ring_buffer__poll(rb_cpu, 100);
}

void cleanup_cpu_monitor() {
    ring_buffer__free(rb_cpu);
    cpu_monitor_bpf__destroy(cpu_skel);
    free(cpu_config->param2);
    free(cpu_config);
}

int add_cpu_config_key(const char* name, const char* value) {
    if (!cpu_config) {
        cpu_config = (cpu_handler_config_t *)malloc(sizeof(cpu_handler_config_t));
        if (!cpu_config) {
            fprintf(stderr, "Failed to allocate memory for CPU handler config\n");
            return 0; // error
        }
    }

    if (strcmp(name, "param1") == 0) {
        cpu_config->param1 = atoi(value);
    } else if (strcmp(name, "param2") == 0) {
        cpu_config->param2 = (char *)malloc(strlen(value) + 1);
        if (!cpu_config->param2) {
            fprintf(stderr, "Failed to allocate memory for CPU handler config param2\n");
            return 0; // error
        }
        strcpy(cpu_config->param2, value);
    } else {
        fprintf(stderr, "Unknown configuration key for CPU handler: %s\n", name);
        return 0; // not found
    }

    return 1; // success
}

handler_t cpu_handler = {
    .name = "cpu_handler",
    .setup = setup_cpu_monitor,
    .poll = poll_cpu_events,
    .cleanup = cleanup_cpu_monitor,
    .add_config_key = add_cpu_config_key,
};
