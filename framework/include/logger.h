#ifndef LOGGER_H
#define LOGGER_H

#define MAX_LOGGERS 10

typedef struct {
	// Initialize the output module
	int (*init)(const char *config);

	// Log a message
	int (*log)(const char* message);

	// Flush buffered logs (if applicable)
	int (*flush)();

	// Cleanup resources
	int (*close)();
} logger_output_t;

extern logger_output_t stdout_logger;
// extern logger_output_t file_logger;
extern logger_output_t can_logger;
extern logger_output_t uart_logger;

// Encapsulating logger module
int logger_init(const char *config);
int logger_log(const char *message);
int logger_flush();
int logger_close();

#endif // LOGGER_H
