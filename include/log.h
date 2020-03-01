#ifndef LOG_H
#define LOG_H

#define COLOR_RED     "\x1b[31m"
#define COLOR_RED_B   "\x1b[31;1m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

enum {
	LOG_OFF = 0,
	LOG_FATAL,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG,
	LOG_ALL
};

int Log_start(const char* file, int level, int mirror_to_console);
void Log_set_level(int level);
void Log_set_mirror_to_console(int mirror_to_console);
void Log_debug(const char* fmt, ...);
void Log_info(const char* fmt, ...);
void Log_warn(const char* fmt, ...);
void Log_error(const char* fmt, ...);
void Log_fatal(const char* fmt, ...);
void Log_destroy();

#endif
