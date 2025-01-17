#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

#define MAX_LOGGERS 10

typedef struct {
	bool verbose;
	char *log_file;
	char *log_can;
	char *log_uart;
	int uart_baudrate;
} loggers_config_t;

// Encapsulating logger module
int logger_init(loggers_config_t *config);
int logger_log(const char *message);
int logger_flush();
int logger_close(loggers_config_t *config);

typedef struct {
	// Initialize the output module
	int (*init)(loggers_config_t *config);

	// Log a message
	int (*log)(const char *message);

	// Flush buffered logs (if applicable)
	int (*flush)();

	// Cleanup resources
	int (*close)();
} logger_output_t;

extern logger_output_t stdout_logger;
extern logger_output_t file_logger;
extern logger_output_t can_logger;
extern logger_output_t uart_logger;

#endif // LOGGER_H
