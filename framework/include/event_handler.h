#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <bpf/libbpf.h>

int setup_cpu_monitor();
int setup_mem_monitor();
int setup_storage_monitor();

int poll_cpu_events();
int poll_mem_events();
int poll_storage_events();

void cleanup_cpu_monitor();
void cleanup_mem_monitor();
void cleanup_storage_monitor();

#endif // EVENT_HANDLER_H
