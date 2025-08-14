#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

/**
 * eBPF program for AER events. 
 */

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} aer_events SEC(".maps");

SEC("tracepoint/ras/aer_event")
int trace_aer_event(struct trace_event_raw_aer_event *ctx) {
    aer_event_t *event;
    
    event = bpf_ringbuf_reserve(&aer_events, sizeof(aer_event_t), 0);
    if (!event) return 0;

    event->time = bpf_ktime_get_ns();
    event->cpu = bpf_get_smp_processor_id();

    event->__data_loc_dev_name = ctx->__data_loc_dev_name;
    event->severity = ctx->severity;
    event->status = ctx->status;

    bpf_ringbuf_submit(event, 0);
    
    return 0;
}


char LICENSE[] SEC("license") = "GPL";