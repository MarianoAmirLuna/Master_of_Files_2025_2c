#ifndef EXTS_LOG_H
#define EXTS_LOG_H

//TODO: Implement log wihtout save on log for different color
//TODO: Implement same above but with save on log
#ifndef LOG_H_
#include "commons/log.h"
#endif
#ifndef TXT_H_
#include "commons/txt.h"
#endif

#include "commons/temporal.h"
#include "commons/string.h"

#define BF_COLOR_SIZE 4

/**
 * Private Functions
 */
static void _log_write_in_level(t_log* logger, t_log_level level, const char* message_template, va_list arguments);
static bool _is_level_enabled(t_log* logger, t_log_level level);

static char *reset_color = "\x1b[0m";
/// @brief Background and Foreground Color
typedef enum{
    WHITE,
    GRAY,
    CYAN,
    VIOLET
}bf_color;

static void _log_with_fore_back_color(t_log* logger, int id_fore_color, int id_back_color,const char* message_template, va_list list_arguments)
{
    t_log_level lvl = LOG_LEVEL_DEBUG;
    if (!_is_level_enabled(logger, lvl)) {
		return;
	}
	char *message = string_from_vformat(message_template, list_arguments);
	char *time = temporal_get_string_time("%H:%M:%S:%MS");
	int thread_id = process_get_thread_id();

	char *buffer = string_from_format("[%s] %s %s/(%d:%d): %s\n",
		log_level_as_string(lvl),
		time,
		logger->program_name,
		logger->pid,
		thread_id,
		message);

	if (logger->file != NULL) {
		txt_write_in_file(logger->file, buffer);
	}

	if (logger->is_active_console) {
        char *console_buffer;
        if(id_back_color == -1){
            console_buffer = string_from_format("\x1b[38;5;%dm%s%s",
			id_fore_color,
			buffer,
			reset_color);
        }else{
            console_buffer= string_from_format("\x1b[38;5;%dm\x1b[48;5;%dm%s%s",
			id_fore_color,
            id_back_color,
			buffer,
			reset_color);
        }
	
		txt_write_in_stdout(console_buffer);
		free(console_buffer);
	}

	free(time);
	free(message);
	free(buffer);
}

void log_with_fore_color(t_log* logger, int id_color, const char* message_template, ...) {
	va_list arguments;
	va_start(arguments, message_template);
	_log_with_fore_back_color(logger, id_color, -1, message_template, arguments);
	va_end(arguments);
}

void log_with_fore_back_color(t_log* logger, int id_fore_color, int id_back_color, const char* message_template, ...) {
	va_list arguments;
	va_start(arguments, message_template);
	_log_with_fore_back_color(logger, id_fore_color, id_back_color, message_template, arguments);
	va_end(arguments);
}

void log_violet(t_log* logger, const char* message_template, ...){
    va_list arguments;
	va_start(arguments, message_template);
    _log_with_fore_back_color(logger, 56, -1, message_template, arguments);
    va_end(arguments);
}
void log_light_blue(t_log* logger, const char* message_template, ...){
    va_list arguments;
	va_start(arguments, message_template);
    _log_with_fore_back_color(logger, 75, -1, message_template, arguments);
    va_end(arguments);
}
void log_light_green(t_log* logger, const char* message_template, ...){
    va_list arguments;
	va_start(arguments, message_template);
    _log_with_fore_back_color(logger, 36, -1, message_template, arguments);
    va_end(arguments);
}
void log_pink(t_log* logger, const char* message_template, ...){
    va_list arguments;
	va_start(arguments, message_template);
    _log_with_fore_back_color(logger, 225, -1, message_template, arguments);
    va_end(arguments);
}
void log_orange(t_log* logger, const char* message_template, ...){
    va_list arguments;
	va_start(arguments, message_template);
    _log_with_fore_back_color(logger, 202, -1, message_template, arguments);
    va_end(arguments);
}
void log_white_background(t_log* logger, const char* message_template, ...){
    va_list arguments;
	va_start(arguments, message_template);
    _log_with_fore_back_color(logger, 16,255 , message_template, arguments);
    va_end(arguments);
}
static bool _is_level_enabled(t_log* logger, t_log_level level) {
	return level >= logger->detail;
}
#endif