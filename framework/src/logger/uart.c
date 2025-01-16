#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "../../include/logger.h"

static int uart_fd;

static int uart_init(const char *config) {
    // TODO: Parse configuration
    // interface, baudrate, etc.
    uart_fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_SYNC);
    if (uart_fd < 0) {
        perror("Failed to open UART");
        return -1;
    }

    struct termios tty;
    tcgetattr(uart_fd, &tty);

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;  // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                      // Disable break processing
    tty.c_lflag = 0;                             // No signaling chars, no echo
    tty.c_oflag = 0;                             // No remapping, no delays

    tcsetattr(uart_fd, TCSANOW, &tty);
    return 0;
}

static int uart_log(const char *message) {
    write(uart_fd, message, strlen(message));
    return 0;
}

static int uart_flush() {
    // No-op for UART
    return 0;
}

static int uart_close() {
    close(uart_fd);
    return 0;
}

// Register UART logger
logger_output_t uart_logger = {
    .init = uart_init,
    .log = uart_log,
    .flush = uart_flush,
    .close = uart_close,
};
