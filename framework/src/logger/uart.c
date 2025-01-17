#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "../../include/logger.h"

static int uart_fd;

speed_t uart_get_baudrate(int baudrate)
{
	switch (baudrate) {
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	default:
		return 0;
	}
}

static int uart_init(loggers_config_t *config)
{
	// TODO: Parse configuration
	// interface, baudrate, etc.
	uart_fd = open(config->log_uart, O_RDWR | O_NOCTTY | O_SYNC);
	if (uart_fd < 0) {
		perror("Failed to open UART");
		return -1;
	}

	struct termios tty;
	tcgetattr(uart_fd, &tty);

	// Set baud rate
	speed_t baudrate = uart_get_baudrate(config->uart_baudrate);
	if (baudrate == 0) {
		fprintf(stderr, "Invalid baudrate: %d\n", config->uart_baudrate);
		return -1;
	}

	cfsetospeed(&tty, baudrate);
	cfsetispeed(&tty, baudrate);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
	tty.c_iflag &= ~IGNBRK; // Disable break processing
	tty.c_lflag = 0; // No signaling chars, no echo
	tty.c_oflag = 0; // No remapping, no delays

	tcsetattr(uart_fd, TCSANOW, &tty);
	return 0;
}

static int uart_log(const char *message)
{
	write(uart_fd, message, strlen(message));
	return 0;
}

static int uart_flush()
{
	// No-op for UART
	return 0;
}

static int uart_close()
{
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
