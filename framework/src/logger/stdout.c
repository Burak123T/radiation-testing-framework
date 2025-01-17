#include <stdio.h>
#include "../../include/logger.h"

static int stdout_init(loggers_config_t *config)
{
	return 0; // No initialization needed
}

static int stdout_log(const char *log_event)
{
	printf("%s", log_event);
	return 0;
}

static int stdout_flush()
{
	fflush(stdout);
	return 0;
}

static int stdout_close()
{
	return 0;
}

// Register stdout logger
logger_output_t stdout_logger = {
	.init = stdout_init,
	.log = stdout_log,
	.flush = stdout_flush,
	.close = stdout_close,
};
