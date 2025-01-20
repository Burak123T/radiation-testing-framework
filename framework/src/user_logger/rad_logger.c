#include <stdio.h>
#include <stdlib.h>
#include "../../include/rad_logger.h"

__attribute__((noinline)) int log_start(int event_id, const char *event_name,
					const char *event_type, const char *event_data)
{
	asm volatile("");
	return event_id;
}