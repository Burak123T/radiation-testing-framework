#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "mem_monitor.skel.h"

static struct mem_monitor_bpf *mem_skel;
static struct ring_buffer *rb_mem;

int handle_mem_event(void *ctx, void *data, uint64_t data_sz) {
    struct mem_event_t *event = data;
    printf("[MEMORY ERROR] Page: %lu | Type: %u | PID: %u | Time: %lu ns\n",
           event->page, event->error_type, event->pid, event->timestamp);
    return 0;
}

int setup_mem_monitor() {
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

    rb_mem = ring_buffer__new(bpf_map__fd(mem_skel->maps.mem_events), handle_mem_event, NULL, NULL);
    if (!rb_mem) {
        fprintf(stderr, "Failed to create ring buffer for Memory events\n");
        return -1;
    }

    return 0;
}

int poll_mem_events() {
    return ring_buffer__poll(rb_mem, 100);
}

void cleanup_mem_monitor() {
    ring_buffer__free(rb_mem);
    mem_monitor_bpf__destroy(mem_skel);
}
