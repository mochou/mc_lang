#ifndef __LEXICAL_H
#define __LEXICAL_H

#include <stdio.h>
#include <stdio.h>
#include "LEX.h"

#define TOKEN_BUFF_SIZE (2)

struct Token_List
{
	Token token;
	union
	{
		struct Token_List *prev;
		struct Token_List *next;
	}u;
};

struct Lexical_Analyzer
{
	FILE *file;
	struct Token_List *prev;
	struct Token_List *next;
	int line_number;
};

#endif