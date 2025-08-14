#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} arm_events SEC(".maps");


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