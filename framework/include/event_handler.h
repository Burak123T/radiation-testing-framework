#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

// Generic handler configuration interface
typedef struct {
    const char* name;
    int (*setup)();
    int (*poll)();
    void (*cleanup)();
    int (*add_config_key)(const char* name, const char* value);
} handler_t;

typedef struct {
    int param1;
    char* param2;
} cpu_handler_config_t;

typedef struct {
    int param1;
    char* param2;
} mem_handler_config_t;

typedef struct {
    int param1;
    char* param2;
} storage_handler_config_t;

// Declare the handlers
extern handler_t cpu_handler;
extern handler_t mem_handler;
extern handler_t storage_handler;

#endif // EVENT_HANDLER_H
