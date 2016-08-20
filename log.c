#include "environment.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "LOG.h"

//#define __LOG_DEBUG__
//#define __SHOW_FILENAME_AND_LINENUMBER__
#ifdef __LOG_DEBUG__

void LOG_info(char *file_name, int line_number, char *format, ...)
{
	if (file_name == NULL || format == NULL)
	{
		return;
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(stdout, "[file: %s, line: %d]\n", file_name, line_number);
#endif
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

	// output to file
	static FILE *file = NULL;
	if (file == NULL)
	{
		file = fopen("info.log", "w");
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(file, "[file: %s, line: %d]\n", file_name, line_number);
#endif
	va_start(args, format);
	vfprintf(file, format, args);
	fflush(file);
	va_end(args);
}

void LOG_error(char *file_name, int line_number, char *format, ...)
{
	if (file_name == NULL || format == NULL)
	{
		return;
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(stderr, "[file: %s, line: %d]", file_name, line_number);
#endif
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	// output to file
	static FILE *file = NULL;
	if (file == NULL)
	{
		file = fopen("error.log", "w");
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(file, "[file: %s, line: %d]\n", file_name, line_number);
#endif
	va_start(args, format);
	vfprintf(file, format, args);
	fflush(file);
	va_end(args);
}

void LOG_print(char *file_name, int line_number, char *format, ...)
{
	if (file_name == NULL || format == NULL)
	{
		return;
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(stdout, "[file: %s, line: %d]", file_name, line_number);
#endif
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

	// output to file
	static FILE *file = NULL;
	if (file == NULL)
	{
		file = fopen("print.log", "w");
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(file, "[file: %s, line: %d]\n", file_name, line_number);
#endif
	va_start(args, format);
	vfprintf(file, format, args);
	fflush(file);
	va_end(args);
}
#else
void LOG_info(char *file_name, int line_number, char *format, ...)
{
	if (file_name == NULL || format == NULL)
	{
		return;
	}
	static FILE *file = NULL;
	if (file == NULL)
	{
		file = fopen("info.log", "w");
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(file, "[file: %s, line: %d]\n", file_name, line_number);
#endif
	va_list args;
	va_start(args, format);
	vfprintf(file, format, args);
	fflush(file);
	va_end(args);
}

void LOG_error(char *file_name, int line_number, char *format, ...)
{
	if (file_name == NULL || format == NULL)
	{
		return;
	}
	static FILE *file = NULL;
	if (file == NULL)
	{
		file = fopen("error.log", "w");
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(file, "[file: %s, line: %d]\n", file_name, line_number);
#endif
	va_list args;
	va_start(args, format);
	vfprintf(file, format, args);
	fflush(file);
	va_end(args);

	// output to stderr
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(stderr, "[file: %s, line: %d]", file_name, line_number);
#endif
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

void LOG_print(char *file_name, int line_number, char *format, ...)
{
	if (file_name == NULL || format == NULL)
	{
		return;
	}
	static FILE *file = NULL;
	if (file == NULL)
	{
		file = fopen("print.log", "w");
	}
#ifdef __SHOW_FILENAME_AND_LINENUMBER__
	fprintf(file, "[file: %s, line: %d]\n", file_name, line_number);
#endif
	va_list args;
	va_start(args, format);
	vfprintf(file, format, args);
	fflush(file);
	va_end(args);
}
#endif

void LOG_exit()
{
	getchar();
	exit(-1);
}