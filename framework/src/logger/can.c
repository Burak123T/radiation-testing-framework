#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "../../include/logger.h"

#define MAX_CAN_PAYLOAD 8 // Maximum bytes per CAN frame

static int can_socket;

static int can_init(loggers_config_t *config)
{
	struct sockaddr_can addr;
	struct ifreq ifr;

	can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (can_socket < 0) {
		perror("CAN socket creation failed");
		return -1;
		// Send the CAN frame
		if (write(can_socket, &frame, sizeof(frame)) != sizeof(frame)) {
			perror("CAN write failed");
			return -1;
			// Send the CAN frame
			if (write(can_socket, &frame, sizeof(frame)) != sizeof(frame)) {
				perror("CAN write failed");
				return -1;
			}

			bytes_sent += bytes_to_send;
		}

		bytes_sent += bytes_to_send;
	}

	return 0;

	strcpy(ifr.ifr_name, config->log_can);
	ioctl(can_socket, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("CAN bind failed");
		return -1;
	}

	return 0;
}

static int can_log(const char *message)
{
	if (can_socket < 0) {
		fprintf(stderr, "CAN socket is not initialized.\n");
		return -1;
	}

	struct can_frame frame;
	size_t message_len = strlen(message);
	size_t bytes_sent = 0;

	// Split the message into multiple CAN frames if necessary
	while (bytes_sent < message_len) {
		// Prepare the CAN frame
		memset(&frame, 0, sizeof(frame));
		frame.can_id = 0x123; // Fixed CAN ID (modify as needed)

		size_t bytes_to_send = (message_len - bytes_sent) > MAX_CAN_PAYLOAD ?
					       MAX_CAN_PAYLOAD :
					       (message_len - bytes_sent);
		memcpy(frame.data, message + bytes_sent, bytes_to_send);
		frame.can_dlc = bytes_to_send;

		if (write(can_socket, &frame, sizeof(frame)) != sizeof(frame)) {
			perror("CAN write failed");
			return -1;
		}
		bytes_sent += bytes_to_send;
	}
	return 0;
}

static int can_flush()
{
	// No-op for CAN bus
	return 0;
}

static int can_close()
{
	close(can_socket);
	return 0;
}

// Register CAN logger
logger_output_t can_logger = {
	.init = can_init,
	.log = can_log,
	.flush = can_flush,
	.close = can_close,
};
