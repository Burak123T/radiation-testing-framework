#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <time.h>
#include <signal.h>
#include <bpf/libbpf.h>
#include "../../include/event_handler.h"
#include "../../lib/inih/ini.h"

#define MAX_HANDLERS 10

// Global registry of available handlers
handler_t *g_available_handlers[] = {
    &cpu_handler,
    &mem_handler,
    &storage_handler,
    NULL // Sentinel
};

handler_t *active_handlers[MAX_HANDLERS];
int active_handlers_count = 0;

volatile sig_atomic_t stop = 0;

void handle_signal(int sig) {
    stop = 1;
}

// Environment variables for the program
static struct env {
    char* config_file;
} env;

const char *argp_program_version = "rad-testing-framework 1.0";
const char *argp_program_bug_address = "<roba@itu.dk>";
const char argp_program_doc[] = "COTS Radiation Testing Framework\n"
				"\n"
				"It monitors and logs CPU, memory, and storage faults in a radiation environment\n"
				"\n"
				"USAGE: sudo ./radiation_testing_framework [-c <config_file>]\n";

static const struct argp_option opts[] = {
	{ "config_file", 'c', "CONFIG-FILE", 0, "Configuration file (.ini)" },
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

// static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
// {
// 	if (level == LIBBPF_DEBUG && !env.verbose)
// 		return 0;
// 	return vfprintf(stderr, format, args);
// }

static int handler(void* user, const char* section, const char* name, const char* value) {
    handler_t** handler_ptr = g_available_handlers;
    printf("section: %s, name: %s, value: %s\n", section, name, value);

    // loop through the array and add key if handler active
    for (int i = 0; i < active_handlers_count; i++) {
        if (strcmp(section, active_handlers[i]->name) == 0) {
            return active_handlers[i]->add_config_key(name, value);
        }
    }

    // if handler not active, loop through the available handlers and activate the one that matches the section
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

int main(int argc, char **argv) {
    int err; 

    printf("Starting radiation testing framework...\r\n");

    err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
    if (err)
		return err;

    printf("Config file: %s\n", env.config_file);

    if (ini_parse(env.config_file, handler, NULL) < 0) {
        printf("Can't load '%s'\n", env.config_file);
        return 1;
    }

    // print active handlers
    for (int i = 0; i < active_handlers_count; i++) {
        printf("Active handler: %s\n", active_handlers[i]->name);
    }

    /* Set up libbpf errors and debug info callback */
	// libbpf_set_print(libbpf_print_fn);

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
