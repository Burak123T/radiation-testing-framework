#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} mem_events SEC(".maps");

SEC("tracepoint/memory/memory_failure")
int trace_memory_failure(struct trace_event_raw_memory_failure_event *ctx) {
    struct mem_event_t *event;
    
    event = bpf_ringbuf_reserve(&mem_events, sizeof(struct mem_event_t), 0);
    if (!event) return 0;

    event->timestamp = bpf_ktime_get_ns();
    event->page = ctx->pfn;
    event->error_type = ctx->type;
    event->pid = bpf_get_current_pid_tgid() >> 32;

    bpf_ringbuf_submit(event, 0);
    return 0;
}

char LICENSE[] SEC("license") = "GPL";