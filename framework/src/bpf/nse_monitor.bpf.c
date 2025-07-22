#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

/**
 * eBPF program for a Non-Standard Event. 
 */

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} ns_events SEC(".maps");

SEC("tracepoint/ras/ns_event")
int trace_ns_event(struct trace_event_raw_non_standard_event *ctx) {
    ns_event_t *event;
    
    event = bpf_ringbuf_reserve(&ns_events, sizeof(ns_event_t), 0);
    if (!event) return 0;

    event->time = bpf_ktime_get_ns();
    event->cpu = bpf_get_smp_processor_id();

    event->__data_loc_fru_text = ctx->__data_loc_fru_text;
    event->sev = ctx->sev;
    event->len = ctx->len;

    bpf_ringbuf_submit(event, 0);
    return 0;
}

char LICENSE[] SEC("license") = "GPL";

/*
struct trace_event_raw_non_standard_event {
	struct trace_entry ent;
	char sec_type[16];
	char fru_id[16];
	u32 __data_loc_fru_text;
	u8 sev;
	u32 len;
	u32 __data_loc_buf;
	char __data[0];
};
*/