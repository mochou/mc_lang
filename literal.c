#include "environment.h"
#include "literal.h"
#include "memory.h"
#include "LOG.h"
#include "tool.h"

int *offset_value_create(int v)
{
	int *value = MEM_malloc(sizeof(*value));
	*value = v;
	return value;
}

void offset_value_free(int *v)
{
	MEM_free(v);
}

void offset_value_print(int *v)
{
	LOG_INFO("%d", *v);
}


double *double_literal_create(double d)
{
	double *key = MEM_malloc(sizeof(*key));
	*key = d;
	return key;
}
double *double_literal_copy(double *key)
{
	return double_literal_create(*key);
}

int double_literal_compare(double *k1, double *k2)
{
	return (*k1 < *k2 ? -1 : (*k1 == *k2 ? 0 : 1));
}

void double_literal_free(double *k)
{
	MEM_free(k);
}

void double_literal_print(double *k)
{
	LOG_INFO("%0.6lf", *k);
}

char *string_literal_create(char *s)
{
	char *key = string_create(s);
	return key;
}

char *string_literal_copy(char *key)
{
	return string_literal_create(key);
}

int string_literal_compare(char *k1, char *k2)
{
	return strcmp(k1, k2);
}

void string_literal_free(char *k)
{
	MEM_free(k);
}

void string_literal_print(char *k)
{
	LOG_INFO(k);
}


Literal literal_create()
{
	Literal literal = MEM_malloc(sizeof(*literal));
	literal->double_literal = map_create(double_literal_compare, double_literal_free,
		offset_value_free, double_literal_print, offset_value_print);
	literal->string_literal = map_create(string_literal_compare, string_literal_free,
		offset_value_free, string_literal_print, offset_value_print);
	return literal;
}
void literal_free(Literal literal)
{
	map_free(literal->double_literal);
	map_free(literal->string_literal);
	MEM_free(literal);
}

int literal_get_offset(Literal literal, void *key, Literal_Type type)
{
	int *offset = NULL;
	void *new_key = NULL;
	int number = 0;
	switch (type)
	{
		case DOUBLE_LITERAL:
		{
			offset = map_search(literal->double_literal, key);
			if (offset == NULL)
			{
				new_key = double_literal_copy(key);
				number = map_get_number(literal->double_literal);
				offset = offset_value_create(number);
				map_insert(literal->double_literal, new_key, offset);
			}
			break;
		}
		case STRING_LITERAL:
		{
			offset = map_search(literal->string_literal, key);
			if (offset == NULL)
			{
				new_key = string_literal_copy(key);
				number = map_get_number(literal->string_literal);
				offset = offset_value_create(number);
				map_insert(literal->string_literal, new_key, offset);
			}
			break;
		}
	}
	return *offset;
}