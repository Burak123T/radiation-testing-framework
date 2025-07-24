#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} arm_events SEC(".maps");

/**
 * Helper function to decode the "__data_loc_fru_text" field from the Non-standard Event tracepoint.
 * Help found thanks to Github issue: https://github.com/bpftrace/bpftrace/issues/385
 */
int decode_arm_data_loc(ns_event_t *event){
    unsigned short offset = event->__data_loc_fru_text & 0xFFFF;
    unsigned short length = event->__data_loc_fru_text >> 16;
    return bpf_probe_read_kernel(event->fru_text, length, (char*)event + offset);
}

SEC("tracepoint/ras/arm_event")
int trace_arm_event(struct trace_event_raw_arm_event *ctx) {
    arm_event_t *event;
    
    event = bpf_ringbuf_reserve(&arm_events, sizeof(arm_event_t), 0);
    if (!event) return 0;

    event->time = bpf_ktime_get_ns();
    event->cpu = bpf_get_smp_processor_id();

    event->mpidr = ctx->mpidr;
    event->midr = ctx->midr;
    event->running_state = ctx->running_state;
    event->psci_state = ctx->psci_state;
    event->affinity = ctx->affinity;

    bpf_ringbuf_submit(event, 0);
    return 0;
}

char LICENSE[] SEC("license") = "GPL";