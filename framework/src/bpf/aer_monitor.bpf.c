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

/**
 * Helper function to decode the "__data_loc_dev_name" field from the AER tracepoint.
 * Help found thanks to Github issue: https://github.com/bpftrace/bpftrace/issues/385
 */
int decode_aer_data_loc(aer_event_t *event){
    unsigned short offset = event->__data_loc_dev_name & 0xFFFF;
    unsigned short length = event->__data_loc_dev_name >> 16;
    return bpf_probe_read_kernel(event->dev_name, length, (char*)event + offset);
}

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

    int isDecodedDataLoc = decode_aer_data_loc(event);

    bpf_ringbuf_submit(event, 0);
    
    return 0;
}


char LICENSE[] SEC("license") = "GPL";