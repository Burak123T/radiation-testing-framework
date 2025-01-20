import rad_logger
from time import sleep

while True:
    rad_logger.log_start(1, "Event name", "Event type", "Event data")
    sleep(1)

