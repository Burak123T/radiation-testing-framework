#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include "../../include/event_types.h"
#include "../../include/event_handler.h"
#include "storage_monitor.skel.h"

static struct storage_monitor_bpf *storage_skel;
static struct ring_buffer *rb_storage;

int handle_storage_event(void *ctx, void *data, uint64_t data_sz) {
    struct storage_event_t *event = data;
    printf("[STORAGE ERROR] Device: %u | Sector: %lu | Error Code: %u | Time: %lu ns\n",
           event->device, event->sector, event->error_code, event->timestamp);
    return 0;
}

int setup_storage_monitor() {
    struct bpf_object_open_opts opts = {
        .sz = sizeof(struct bpf_object_open_opts),
        .object_name = "storage_monitor",
    };

    storage_skel = storage_monitor_bpf__open_opts(&opts);
    if (!storage_skel) {
        fprintf(stderr, "Failed to open Storage monitor BPF program\n");
        return -1;
    }

    if (storage_monitor_bpf__load(storage_skel) || storage_monitor_bpf__attach(storage_skel)) {
        fprintf(stderr, "Failed to load or attach Storage monitor BPF program\n");
        return -1;
    }

    rb_storage = ring_buffer__new(bpf_map__fd(storage_skel->maps.storage_events), handle_storage_event, NULL, NULL);
    if (!rb_storage) {
        fprintf(stderr, "Failed to create ring buffer for Storage events\n");
        return -1;
    }

    return 0;
}

int poll_storage_events() {
    return ring_buffer__poll(rb_storage, 100);
}

void cleanup_storage_monitor() {
    ring_buffer__free(rb_storage);
    storage_monitor_bpf__destroy(storage_skel);
}

