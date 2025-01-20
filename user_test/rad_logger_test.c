#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <rad_logger.h>
// #include "../../include/rad_logger.h"

int main(int argc, char **argv)
{
	while (1) {
		printf("Got the following event id: %d\r\n",
		       log_start(1, "custom_event_name", "custom_event_type", "custom_event_data"));
		sleep(1);
	}

	return 0;
}