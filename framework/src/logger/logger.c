#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/logger.h"

// Max number of output modules
#define MAX_OUTPUT_MODULES 10

// List of output modules
static logger_output_t *output_modules[MAX_OUTPUT_MODULES];
static int num_modules = 0;

// Initialize the logger with a config file
int logger_init(const char *config_file) {
    // Parse configuration (stdout, CAN, UART, etc.)
    // For simplicity, assume it's a list of output types (e.g., "stdout,can,uart")
    FILE *file = fopen(config_file, "r");
    if (!file) {
        perror("Failed to open config file");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "stdout")) {
            extern logger_output_t stdout_logger;
            output_modules[num_modules++] = &stdout_logger;
        } else if (strstr(line, "can")) {
            extern logger_output_t can_logger;
            output_modules[num_modules++] = &can_logger;
        } else if (strstr(line, "uart")) {
            extern logger_output_t uart_logger;
            output_modules[num_modules++] = &uart_logger;
        }
    }
    fclose(file);

    // Initialize all enabled modules
    for (int i = 0; i < num_modules; i++) {
        output_modules[i]->init(NULL);  // Pass specific config if needed
    }

    return 0;
}

// Log a string message
int logger_log(const char *message) {
    for (int i = 0; i < num_modules; i++) {
        output_modules[i]->log(message);
    }
    return 0;
}

// Flush logs (for buffered outputs)
int logger_flush() {
    for (int i = 0; i < num_modules; i++) {
        if (output_modules[i]->flush) {
            output_modules[i]->flush();
        }
    }
    return 0;
}

// Cleanup resources
int logger_close() {
    for (int i = 0; i < num_modules; i++) {
        if (output_modules[i]->close) {
            output_modules[i]->close();
        }
    }
    return 0;
}
