#ifndef __LOG_H
#define __LOG_H

void LOG_info(char *file_name, int line_number, char *format, ...);
#define LOG_INFO(format, ...) LOG_info(__FILE__, __LINE__, format, __VA_ARGS__)

void LOG_error(char *file_name, int line_number, char *format, ...);
#define LOG_ERROR(format, ...) LOG_error(__FILE__, __LINE__, format, __VA_ARGS__)

void LOG_print(char *file_name, int line_number, char *format, ...);
#define LOG_PRINT(format, ...) LOG_print(__FILE__, __LINE__, format, __VA_ARGS__)

void LOG_exit();
#define LOG_EXIT() LOG_exit()

#endif