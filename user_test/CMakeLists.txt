cmake_minimum_required(VERSION 3.10)
project(UserLoggingTest C)

find_library(RAD_LOGGER librad_logger.so REQUIRED)
add_executable(rad_logger_test ${CMAKE_CURRENT_SOURCE_DIR}/rad_logger_test.c)

message(STATUS "RAD_LOGGER: ${RAD_LOGGER}")

target_link_libraries(rad_logger_test ${RAD_LOGGER})