#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <bpf/libbpf.h>
#include "../../include/event_handler.h"
#include "../../include/logger.h"
#include "../../lib/inih/ini.h"

#define MAX_HANDLERS 10

// Global registry of available handlers
handler_t *g_available_handlers[] = {
	&mem_handler, &storage_handler, &uprobe_handler, &arm_handler,
	NULL // Sentinel
};

handler_t *active_handlers[MAX_HANDLERS];
int active_handlers_count = 0;

volatile sig_atomic_t stop = 0;

void handle_signal(int sig)
{
	stop = 1;
}

// Environment variables for the program
static struct env {
	char *config_file;
} env;

loggers_config_t loggers = {
	.verbose = false,
	.log_file = NULL,
	.log_can = NULL,
	.log_uart = NULL,
	.uart_baudrate = 0,
};

const char *argp_program_version = "rad-testing-framework 1.0";
const char *argp_program_bug_address = "<roba@itu.dk>";
const char argp_program_doc[] =
	"COTS Radiation Testing Framework\n"
	"\n"
	"It monitors and logs CPU, memory, and storage faults in a radiation environment\n"
	"\n"
	"USAGE: sudo ./radiation_testing_framework [-c <config_file>] [-v] [-o <file_path>] "
	"[-a <CAN_interface>] [-u <UART_interface>] [-b <UART_baudrate>] \n";

static const struct argp_option opts[] = {
	{ "config_file", 'c', "config_file", 0, "Configuration file (.ini)" },
	{ "verbose", 'v', NULL, 0, "Log to stdout" },
	{ "log_file", 'o', "file_path", 0, "File to log to" },
	{ "log_can", 'a', "CAN_interface", 0, "UART interface to use for logging" },
	{ "log_uart", 'u', "UART_interface", 0, "UART interface to use for logging" },
	{ "uart_baudrate", 'b', "UART_baudrate", 0, "UART baudrate" },
	{},
};

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'c':
		env.config_file = malloc(strlen(arg) + 1);
		if (!env.config_file) {
			fprintf(stderr, "Failed to allocate memory for config file\n");
			return -1;
		}
		strcpy(env.config_file, arg);
		break;
	case 'v':
		loggers.verbose = true;
		break;
	case 'o':
		loggers.log_file = malloc(strlen(arg) + 1);
		if (!loggers.log_file) {
			fprintf(stderr, "Failed to allocate memory for log file\n");
			return -1;
		}
		strcpy(loggers.log_file, arg);
		break;
	case 'a':
		loggers.log_can = malloc(strlen(arg) + 1);
		if (!loggers.log_can) {
			fprintf(stderr, "Failed to allocate memory for CAN interface\n");
			return -1;
		}
		strcpy(loggers.log_can, arg);
		break;
	case 'u':
		loggers.log_uart = malloc(strlen(arg) + 1);
		if (!loggers.log_uart) {
			fprintf(stderr, "Failed to allocate memory for UART interface\n");
			return -1;
		}
		strcpy(loggers.log_uart, arg);
		break;
	case 'b':
		loggers.uart_baudrate = atoi(arg);
		break;
	case ARGP_KEY_ARG:
		argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static const struct argp argp = {
	.options = opts,
	.parser = parse_arg,
	.doc = argp_program_doc,
};

static int handler(void *user, const char *section, const char *name, const char *value)
{
	printf("section: %s, name: %s, value: %s\n", section, name, value);

	// loop through the array and add key if handler active
	for (int i = 0; i < active_handlers_count; i++) {
		if (strcmp(section, active_handlers[i]->name) == 0) {
			return active_handlers[i]->add_config_key(name, value);
		}
	}

	// if handler not active, loop through the available handlers and activate the one that matches the section
	handler_t **handler_ptr = g_available_handlers;
	while (*handler_ptr) {
		if (strcmp(section, (*handler_ptr)->name) == 0) {
			active_handlers[active_handlers_count] = *handler_ptr;
			active_handlers_count++;
			return (*handler_ptr)->add_config_key(name, value);
		}
		handler_ptr++;
	}
	return 0; // Unknown section/name, error
}

int main(int argc, char **argv)
{
	int err;

	printf("Starting radiation testing framework...\r\n");

	err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (err)
		return err;

	printf("Config file: %s\n", env.config_file);

	if (ini_parse(env.config_file, handler, NULL) < 0) {
		printf("Can't load '%s'\n", env.config_file);
		free(env.config_file);
		return 1;
	}

	// print active handlers
	for (int i = 0; i < active_handlers_count; i++) {
		printf("Active handler: %s\n", active_handlers[i]->name);
	}

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	int status = 0;

	for (int i = 0; i < active_handlers_count; i++) {
		if (active_handlers[i]->setup() != 0) {
			fprintf(stderr, "Failed to setup handler: %s\n", active_handlers[i]->name);
			status = -1;
			break;
		}
	}

	if (status != 0) {
		for (int i = 0; i < active_handlers_count; i++) {
			active_handlers[i]->cleanup();
		}
		free(env.config_file);
		return EXIT_FAILURE;
	}

	printf("Monitoring system faults... Press Ctrl+C to stop.\n");

	logger_init(&loggers);

	while (!stop) {
		for (int i = 0; i < active_handlers_count; i++) {
			if (stop) {
				break;
			}
			if (active_handlers[i]->poll() < 0) {
				fprintf(stderr, "Error polling events for handler: %s\n",
					active_handlers[i]->name);
				break;
			}
		}
	}

	printf("Stopping radiation testing framework...\r\n");

	for (int i = 0; i < active_handlers_count; i++) {
		printf("Cleaning up handler: %s\n", active_handlers[i]->name);
		active_handlers[i]->cleanup();
	}

	logger_flush();
	logger_close(&loggers);

	free(env.config_file);

	return EXIT_SUCCESS;
}
