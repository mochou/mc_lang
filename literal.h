#ifndef __LITERAL_H
#define __LITERAL_H

#include "red_black_tree.h"

typedef struct Literal_tag *Literal;

typedef enum
{
	//INT_LITERAL,
	DOUBLE_LITERAL,
	STRING_LITERAL,
}Literal_Type;

struct Literal_tag
{
	//Map int_literal;
	Map double_literal;
	Map string_literal;
};


//int *int_literal_create(int k);
double *double_literal_create(double d);
char *string_literal_create(char *s);
int *offset_value_create(int v);

Literal literal_create();
void literal_free(Literal literal);
int literal_get_offset(Literal literal, void *key, Literal_Type type);

#endif