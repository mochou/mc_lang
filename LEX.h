#ifndef __LEX_H
#define __LEX_H

enum Token_Kind
{
	TOKEN_BEGIN,// MARK
	COMMA,
	COLON,
	SEMICOLON,
	LP,
	RP,
	LB,
	RB,
	LC,
	RC,
	DOT,
	EXCLAMATION,
	INCREMENT, 
	DECREMENT, 
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	EQ,
	NE,
	LT,
	LE,
	GT,
	GE,
	AND,
	OR,
	ASSIGN,
	//
	NEW,
	FUNCTION,
	VARIABLE,
	IMPORT, 
	STATIC,
	IDENTIFIER,
	VOID, 
	BOOL,
	INT,
	DOUBLE,
	STRING,
	NIL,
	BOOL_V,
	INT_V,
	DOUBLE_V,
	STRING_V,
	IF,
	ELSE,
	SWITCH,
	CASE,
	DO,
	WHILE,
	FOR,
	CONTINUE,
	BREAK,
	RETURN,
	RECORD,	
	COMMENT,
	END_OF_FILE,
	INVALID,
	TOKEN_END, // MARK
};

typedef struct Token_tag *Token;
struct Token_tag
{
	enum Token_Kind token_kind;
	char *buff;
	int buff_size;
	int char_count;
	int line_number;
};

void LEX_initialize(char *file_name);
Token LEX_get_token();
Token LEX_get_token_before_token(Token token);
void LEX_unget_token(Token token);
void LEX_print_token(Token token);
void LEX_uninitialize();
char *LEX_get_token_kind_string(int token_kind);

#endif