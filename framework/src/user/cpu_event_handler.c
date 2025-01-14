#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "cpu_monitor.skel.h"

static struct cpu_monitor_bpf *cpu_skel;
static struct ring_buffer *rb_cpu;

int handle_cpu_event(void *ctx, void *data, uint64_t data_sz) {
    struct mce_event_t *event = data;
    printf("[CPU/MCE] CPU %d | Status: 0x%lx | Addr: 0x%lx | Misc: 0x%lx | Time: %lu ns\n",
           event->cpu, event->status, event->addr, event->misc, event->timestamp);
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
}
