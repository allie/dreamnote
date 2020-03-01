#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static char* log_colors[] = {
	COLOR_RESET,
	COLOR_RED_B,
	COLOR_RED,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_GREEN
};

static FILE* fp;
static int current_level;
static int current_color;
static int mirror;

static void write_line(const char* prefix, const char* fmt, va_list args) {
	// Get the current timestamp
	// [YYYY-MM-DD HH:MM:SS]
	time_t timer;
	char timestamp[20];
	struct tm* tm_info;
	timer = time(NULL);
	tm_info = localtime(&timer);
	strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	// Format the message if necessary
	if (args != NULL) {
		char message[4096];
		vsnprintf(message, 4096, fmt, args);

		// Write the formatted message with timestamp and prefix to the file
		fprintf(fp, "[%s] [%s] %s\n", timestamp, prefix, message);

		// Do the same thing in stdout if necessary
		if (mirror) {
			printf("[%s] %s[%s] %s%s\n", timestamp, log_colors[current_color], prefix, message, log_colors[0]);
		}
	} else {
		// Write the message with timestamp and prefix to the file
		fprintf(fp, "[%s] [%s] %s\n", timestamp, prefix, fmt);

		// Do the same thing in stdout if necessary
		if (mirror) {
			printf("[%s] %s[%s] %s%s\n", timestamp, log_colors[current_color], prefix, fmt, log_colors[0]);
		}
	}
}

int Log_start(const char* file, int level, int mirror_to_console) {
	current_level = level;
	mirror = mirror_to_console;

	fp = fopen(file, "w");

	if (fp == NULL) {
		return 0;
	}

	write_line("LOG", "Beginning logging session", NULL);

	return 1;
}

void Log_set_level(int level) {
	current_level = level;
}

void Log_set_mirror_to_console(int mirror_to_console) {
	mirror = mirror_to_console;
}

void Log_debug(const char* fmt, ...) {
	if (current_level < LOG_DEBUG) {
		return;
	}
	current_color = LOG_DEBUG;

	va_list args;
	va_start(args, fmt);
	write_line("DEBUG", fmt, args);
	va_end(args);
}

void Log_info(const char* fmt, ...) {
	if (current_level < LOG_INFO) {
		return;
	}
	current_color = LOG_INFO;

	va_list args;
	va_start(args, fmt);
	write_line("INFO", fmt, args);
	va_end(args);
}

void Log_warn(const char* fmt, ...) {
	if (current_level < LOG_WARN) {
		return;
	}
	current_color = LOG_WARN;

	va_list args;
	va_start(args, fmt);
	write_line("WARN", fmt, args);
	va_end(args);
}

void Log_error(const char* fmt, ...) {
	if (current_level < LOG_ERROR) {
		return;
	}
	current_color = LOG_ERROR;

	va_list args;
	va_start(args, fmt);
	write_line("ERROR", fmt, args);
	va_end(args);
}

void Log_fatal(const char* fmt, ...) {
	if (current_level < LOG_FATAL) {
		return;
	}
	current_color = LOG_FATAL;

	va_list args;
	va_start(args, fmt);
	write_line("FATAL", fmt, args);
	va_end(args);
}

void Log_destroy() {
	write_line("LOG", "Ending logging session", NULL);
	fprintf(fp, "\n");
	fclose(fp);
}
