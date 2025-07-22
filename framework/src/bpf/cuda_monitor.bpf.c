#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include "../../include/event_types.h"

const char CUDA_PATH[] = "/usr/local/cuda-12.2/targets/aarch64-linux/lib/libcudart.so";

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 4096);
} cuda_events SEC(".maps");

/**
 * Loading the TensorRT engine (ENTRY)
 */
SEC("uprobe//usr/local/cuda-12.2/targets/aarch64-linux/lib/libcudart.so:deserialize_cuda_engine")
int BPF_KPROBE(cuda__deserialize_cuda_engine, int event_id, const char *event_name, const char *event_type,
	       const char *event_data)
{
	cuda_event_t *event;
	event = bpf_ringbuf_reserve(&cuda_events, sizeof(cuda_event_t), 0);
    if (!event) return 0;

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
		"[CUDA deserialize_cuda_engine] uprobed_start ENTRY: event_id = %d, event_name = %s, event_type = %s, event_data = %s",
		event_id, event_name_copy, event_type_copy, event_data_copy);
	return 0;
}

/**
 * Loading the TensorRT engine (EXIT)
 */
SEC("uprobe//usr/local/cuda-12.2/targets/aarch64-linux/lib/libcudart.so:deserialize_cuda_engine")
int BPF_KRETPROBE(uretprobe_start, int ret)
{
	bpf_printk("[CUDA deserialize_cuda_engine] uprobed_start EXIT: return = %d", ret);
	return 0;
}