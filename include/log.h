#ifndef LOG_H
#define LOG_H

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
