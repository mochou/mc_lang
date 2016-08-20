#include "environment.h"
#include <stdlib.h>
#include <string.h>
#include "tool.h"
#include "memory.h"
char *string_create(const char* src)
{
	int len = strlen(src);
	char *str = MEM_malloc(sizeof(char) * (len + 1));
	strncpy(str, src, len);
	str[len] = 0;
	return str;
}
char *string_add(const char*src1, const char *src2)
{
	int len1 = strlen(src1);
	int len2 = strlen(src2);
	char *str = MEM_malloc(sizeof(char) * (len1 + len2 + 1));
	strncpy(str, src1, len1);
	strncpy(str + len1, src2, len2);
	str[len1 + len2] = 0;
	return str;
}
void string_free(char *str)
{
	MEM_free(str);
}