#ifndef _UTILS_
#define _UTILS_

typedef enum {
	CRITICAL,
	ERROR,
	WARNING,
	INFO,
	DEBUG,
	MAX_LEVEL
} message_level;

void set_log_level(message_level log_level);

void log_message(const message_level level, const char *char_ptr, ...);

void log_error(const char *char_ptr, ...);

#endif
