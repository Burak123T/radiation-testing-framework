// #include <bpf/libbpf.h>
#include "../../include/event_handler.h"

// Stubs for event handler setup/cleanup
extern int setup_cpu_monitor();
extern int setup_mem_monitor();
extern int setup_storage_monitor();

extern int poll_cpu_events();
extern int poll_mem_events();
extern int poll_storage_events();

extern void cleanup_cpu_monitor();
extern void cleanup_mem_monitor();
extern void cleanup_storage_monitor();
