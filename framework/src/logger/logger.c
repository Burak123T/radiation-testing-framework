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
int logger_init(loggers_config_t *config)
{
	// Initialize all output modules
	if (config->verbose) {
		output_modules[num_modules++] = &stdout_logger;
	}
	if (config->log_file) {
		output_modules[num_modules++] = &file_logger;
	}
	if (config->log_can) {
		output_modules[num_modules++] = &can_logger;
	}
	if (config->log_uart) {
		output_modules[num_modules++] = &uart_logger;
	}

	// Initialize all enabled modules
	for (int i = 0; i < num_modules; i++) {
		if (output_modules[i]->init(config) != 0) {
			fprintf(stderr, "Failed to initialize logger module\n");
			return -1;
		}
	}

	return 0;
}

// Log a string message
int logger_log(const char *message)
{
	for (int i = 0; i < num_modules; i++) {
		if (output_modules[i]->log(message) != 0) {
			fprintf(stderr, "Failed to log message\n");
			return -1;
		}
	}
	return 0;
}

// Flush logs (for buffered outputs)
int logger_flush()
{
	for (int i = 0; i < num_modules; i++) {
		if (output_modules[i]->flush() != 0) {
			fprintf(stderr, "Failed to flush logs\n");
			return -1;
		}
	}
	return 0;
}

// Cleanup resources
int logger_close(loggers_config_t *config)
{
	for (int i = 0; i < num_modules; i++) {
		if (output_modules[i]->close) {
			output_modules[i]->close();
		}
	}

	// Free memory allocated for config
	if (config->log_file) {
		free(config->log_file);
	}
	if (config->log_can) {
		free(config->log_can);
	}
	if (config->log_uart) {
		free(config->log_uart);
	}

	return 0;
}
