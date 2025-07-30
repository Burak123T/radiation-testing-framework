#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} mc_events SEC(".maps");

/**
 * Helper function to decode the various "__data_loc" fields for the Memory Controller tracepoint.
 * Help found thanks to Github issue: https://github.com/bpftrace/bpftrace/issues/385
 */
/*
void decode_mc_data_loc(mc_event_t *event){
    unsigned short offset = event->__data_loc_msg & 0xFFFF;
    unsigned short length = event->__data_loc_msg >> 16;
    bpf_probe_read_kernel_str(event->msg, length, (char*)event + offset);
    
    offset = event->__data_loc_label & 0xFFFF;
    length = event->__data_loc_label >> 16;
    bpf_probe_read_kernel_str(event->label, length, (char*)event + offset);
    
    offset = event->__data_loc_driver_detail & 0xFFFF;
    length = event->__data_loc_driver_detail >> 16;
    bpf_probe_read_kernel_str(event->driver_detail, length, (char*)event + offset);
}
*/

SEC("tracepoint/ras/mc_event")
int trace_mc_event(void *ctx) {
    bpf_printk("mc event triggered");
    
    return 0;
}

char LICENSE[] SEC("license") = "GPL";