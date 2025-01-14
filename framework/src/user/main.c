#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <bpf/libbpf.h>
#include "../../include/event_handler.h"

volatile sig_atomic_t stop = 0;

void handle_signal(int sig) {
    stop = 1;
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (
        setup_cpu_monitor() != 0 || 
        // setup_mem_monitor() != 0 || 
        setup_storage_monitor() != 0
    ) {
        fprintf(stderr, "Failed to initialize event monitoring\n");
        cleanup_cpu_monitor();
        // cleanup_mem_monitor();
        cleanup_storage_monitor();
        return EXIT_FAILURE;
    }

    printf("Monitoring system faults... Press Ctrl+C to stop.\n");

    while (!stop) {
        if (
            poll_cpu_events() < 0 || 
            // poll_mem_events() < 0 || 
            poll_storage_events() < 0
        ) {
            fprintf(stderr, "Error polling event buffers\n");
            break;
        }
    }

    cleanup_cpu_monitor();
    // cleanup_mem_monitor();
    cleanup_storage_monitor();

    return EXIT_SUCCESS;
}
