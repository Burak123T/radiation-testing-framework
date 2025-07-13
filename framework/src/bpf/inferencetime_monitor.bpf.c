/*
	Heavily inspired by this tutorial: https://eunomia.dev/en/tutorials/4-opensnoop/
*/

#include "vmlinux.h"  
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"



SEC("tracepoint/syscalls/sys_enter_openat")
int trace_file_opening(struct trace_event_raw_sys_enter* context){
    bpf_printk("File open detected somewhere!");
    return 0;
}

char LICENSE[] SEC("license") = "GPL";