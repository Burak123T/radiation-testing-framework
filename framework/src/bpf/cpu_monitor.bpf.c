#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} mce_events SEC(".maps");

SEC("tracepoint/mce/mce_record")
int trace_mce_event(struct trace_event_raw_mce_record *ctx) {
    mce_event_t *event;
    
    event = bpf_ringbuf_reserve(&mce_events, sizeof(mce_event_t), 0);
    if (!event) return 0;

    event->timestamp = bpf_ktime_get_ns();
    event->cpu = bpf_get_smp_processor_id();
    event->status = ctx->status;
    event->addr = ctx->addr;
    event->misc = ctx->misc;

    bpf_ringbuf_submit(event, 0);
    return 0;
}

char LICENSE[] SEC("license") = "GPL";
