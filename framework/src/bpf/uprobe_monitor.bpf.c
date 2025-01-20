// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

SEC("uprobe//usr/local/lib/librad_logger.so:log_start")
int BPF_KPROBE(uprobe_start, int event_id, const char *event_name, const char *event_type,
	       const char *event_data)
{
	// copy the event_name string from userscape
	char event_name_copy[100];
	bpf_probe_read_str(event_name_copy, sizeof(event_name_copy), event_name);

	// copy the event_name string from userscape
	char event_type_copy[100];
	bpf_probe_read_str(event_type_copy, sizeof(event_type_copy), event_type);

	// copy the event_name string from userscape
	char event_data_copy[100];
	bpf_probe_read_str(event_data_copy, sizeof(event_data_copy), event_data);

	bpf_printk(
		"uprobed_start ENTRY: event_id = %d, event_name = %s, event_type = %s, event_data = %s",
		event_id, event_name_copy, event_type_copy, event_data_copy);
	return 0;
}

SEC("uretprobe//usr/local/lib/librad_logger.so:log_start")
int BPF_KRETPROBE(uretprobe_start, int ret)
{
	bpf_printk("uprobed_start EXIT: return = %d", ret);
	return 0;
}