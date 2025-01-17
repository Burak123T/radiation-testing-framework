#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/logger.h"

// Static variable to store the file pointer
static FILE *log_file = NULL;

// Initialize the file logger
static int file_logger_init(loggers_config_t *config)
{
	log_file = fopen(config->log_file, "a"); // Open file in append mode
	if (!log_file) {
		perror("Failed to open log file");
		return -1;
	}
	return 0;
}

// Log a message to the file
static int file_logger_log(const char *message)
{
	if (!log_file) {
		fprintf(stderr, "File logger is not initialized.\n");
		return -1;
	}
	fprintf(log_file, "%s", message); // Write the message to the file
	return 0;
}

// Flush the log file (force writing buffered content)
static int file_logger_flush()
{
	if (!log_file) {
		fprintf(stderr, "File logger is not initialized.\n");
		return -1;
	}
	fflush(log_file); // Ensure all buffered data is written to the file
	return 0;
}

// Close the log file
static int file_logger_close()
{
	if (log_file) {
		fclose(log_file); // Close the file
		log_file = NULL;
	}
	return 0;
}

// Register the file logger
logger_output_t file_logger = {
	.init = file_logger_init,
	.log = file_logger_log,
	.flush = file_logger_flush,
	.close = file_logger_close,
};
