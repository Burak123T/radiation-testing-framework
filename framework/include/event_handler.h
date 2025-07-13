#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <sys/stat.h>

// Generic handler configuration interface
typedef struct {
	const char *name;
	int (*setup)();
	int (*poll)();
	void (*cleanup)();
	int (*add_config_key)(const char *name, const char *value);
} handler_t;

typedef struct {
	int param1;
	char *param2;
} cpu_handler_config_t;

typedef struct {
	int param1;
	char *param2;
} mem_handler_config_t;

typedef struct {
	int param1;
	char *param2;
} storage_handler_config_t;

typedef struct {
	int param1;
	char *param2;
} uprobe_handler_config_t;

typedef struct {
	int param1;
	char *param2;
} inferencetime_handler_config_t;

// Declare the handlers
extern handler_t cpu_handler;
extern handler_t mem_handler;
extern handler_t storage_handler;
extern handler_t uprobe_handler;
extern handler_t inferencetime_handler;

static int create_directory_if_not_exists(const char *path)
{
	struct stat st = { 0 };
	if (stat(path, &st) == -1) {
		if (mkdir(path, 0755) < 0) {
			perror("Failed to create directory");
			return -1;
		}
	}
	return 0;
}

#endif // EVENT_HANDLER_H
