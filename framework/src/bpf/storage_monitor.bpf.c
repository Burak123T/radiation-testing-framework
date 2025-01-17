#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include "../../include/event_types.h"

#define DEVICE_NAME_LEN 32

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 4096);
} storage_events SEC(".maps");

SEC("tracepoint/block/block_rq_complete")
int trace_block_rq_complete(struct trace_event_raw_block_rq_completion *ctx)
{
	if (ctx->error) { // Only capture errors
		storage_event_t *event;

		event = bpf_ringbuf_reserve(&storage_events, sizeof(storage_event_t), 0);
		if (!event)
			return 0;

		event->timestamp = bpf_ktime_get_ns();
		event->sector = ctx->sector;
		event->rw_flag = (ctx->rwbs[0] == 'R') ? 0 : 1;
		event->error_code = ctx->error;
		event->device = ctx->dev;

		bpf_ringbuf_submit(event, 0);
	}

	return 0;
}

char LICENSE[] SEC("license") = "GPL";