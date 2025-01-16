#include <linux/can.h>      
#include <linux/can/raw.h>  
#include <sys/socket.h>    
#include <sys/ioctl.h>    
#include <net/if.h>         
#include <unistd.h>         
#include <string.h>         
#include <stdio.h>          
#include "../../include/logger.h"

static int can_socket;

static int can_init(const char *config) {
    struct sockaddr_can addr;
    struct ifreq ifr;

    can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket < 0) {
        perror("CAN socket creation failed");
        return -1;
    }

    strcpy(ifr.ifr_name, "can0");  // Replace with desired CAN interface
    ioctl(can_socket, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("CAN bind failed");
        return -1;
    }

    return 0;
}

static int can_log(const char *message) {
    struct can_frame frame;
    frame.can_id = 0x123;  // Use a fixed CAN ID
    frame.can_dlc = strlen(message) > 8 ? 8 : strlen(message);
    memcpy(frame.data, message, frame.can_dlc);

    if (write(can_socket, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("CAN write failed");
        return -1;
    }

    return 0;
}

static int can_flush() {
    // No-op for CAN bus
    return 0;
}

static int can_close() {
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
