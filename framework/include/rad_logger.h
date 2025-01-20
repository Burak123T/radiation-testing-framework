#ifndef RAD_LOGGER_H
#define RAD_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

int log_start(int event_id, const char *event_name, const char *event_type, const char *event_data);

#ifdef __cplusplus
}
#endif

#endif // RAD_LOGGER_H
