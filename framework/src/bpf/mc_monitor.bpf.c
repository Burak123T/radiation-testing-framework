#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} mc_events SEC(".maps");

SEC("tracepoint/ras/mc_event")
int trace_mc_event(struct trace_event_raw_mc_event *ctx) {
    mc_event_t *event;
    
    event = bpf_ringbuf_reserve(&mc_events, sizeof(mc_event_t), 0);
    if (!event) return 0;

    event->time = bpf_ktime_get_ns();
    event->cpu = bpf_get_smp_processor_id();

    event->error_type = ctx->error_type;
    event->__data_loc_msg = ctx->__data_loc_msg;
    event->__data_loc_label = ctx->__data_loc_label;
    event->error_count = ctx->error_count;
    event->mc_index = ctx->mc_index;
    event->address = ctx->address;
    event->grain_bits = ctx->grain_bits;
    event->syndrome = ctx->syndrome;
    event->__data_loc_driver_detail = ctx->__data_loc_driver_detail;

    bpf_ringbuf_submit(event, 0);
    return 0;
}

char LICENSE[] SEC("license") = "GPL";