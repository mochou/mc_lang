#include "environment.h"
#include "constant.h"
#include "lexical.h"
#include "LOG.h"
#include "memory.h"

static struct Lexical_Analyzer st_lexical_analyzer = { NULL, NULL, NULL, 0 };
char* st_token_kind_string[] =
{
	"TOKEN_BEGIN",
	"COMMA",
	"COLON",
	"SEMICOLON",
	"LP",
	"RP",
	"LB",
	"RB",
	"LC",
	"RC",
	"DOT",
	"EXCLAMATION",
	"INCREMENT",
	"DECREMENT",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"MOD",
	"EQ",
	"NE",
	"LT",
	"LE",
	"GT",
	"GE",
	"AND",
	"OR",
	"ASSIGN",
	//
	"NEW", 
	"FUNCTION",
	"VARIABLE",
	"IMPORT", 
	"STATIC",
	"IDENTIFIER",
	"VOID",
	"BOOL",
	"INT",
	"DOUBLE",
	"STRING",
	"NIL",
	"BOOL_V",
	"INT_V",
	"DOUBLE_V",
	"STRING_V",
	"IF",
	"ELSE",
	"SWITCH",
	"CASE",
	"DO",
	"WHILE",
	"FOR",
	"CONTINUE",
	"BREAK",
	"RETURN",
	"RECORD",
	"COMMENT",
	"END_OF_FILE",
	"INVALID",
	"TOKEN_END",
};

char *LEX_get_token_kind_string(int token_kind)
{
	return st_token_kind_string[token_kind];
}
static Token new_token()
{
	Token token = NULL;
	token = MEM_malloc(sizeof(*token));
	int token_buff_size = sizeof(char) * (TOKEN_BUFF_SIZE != 0 ? TOKEN_BUFF_SIZE : 1);
	char *buff = MEM_malloc(token_buff_size);
	memset(buff, 0, token_buff_size);
	token->buff = buff;
	token->buff_size = token_buff_size;
	token->char_count = 0;
	token->token_kind = INVALID;
	token->line_number = 0;
	return token;
}

static void delete_token(Token token)
{
	if (token == NULL)
	{
		return;
	}
	MEM_free(token->buff);
	MEM_free(token);
}

static void expand_token_buff(Token token)
{
	if (token == NULL)
	{
		return;
	}
	if (token->char_count >= token->buff_size - 1)
	{
		char *temp = MEM_realloc(token->buff, token->buff_size * 2);
		token->buff = temp;
		token->buff_size *= 2;
	}
}

static void put_char(Token token, char c)
{
	expand_token_buff(token);
	token->buff[token->char_count++] = c;
	token->buff[token->char_count] = 0;
}

static Token get_number() 
{
	Token token = new_token();
	char c = 0;
	int dot_count = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c >= '0' && c <= '9'));	// the first char must be 0-9 and must not be dot
	while (c >= '0' && c <= '9' || c == '.')
	{
		put_char(token, c);
		if (c == '.')
		{
			dot_count++;
			if (dot_count == 2)
			{
				token->char_count--;
				token->buff[token->char_count] = 0;
				dot_count = 1;
				break;
			}
		}
		c = getc(st_lexical_analyzer.file);
	}
	if (c == '.' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
	{
		LOG_ERROR("char %c after digit\n", c);
		LOG_EXIT();
	}
	ungetc(c, st_lexical_analyzer.file);
	if (dot_count == 0)
	{
		token->token_kind = INT_V;
	}
	else if (dot_count == 1)
	{
		token->token_kind = DOUBLE_V;
	}
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_keyword_or_identifier( )
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_'));	
	int first_char = TRUE;
	while ((first_char == TRUE && (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_'))
		|| (first_char == FALSE && (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_' || c >= '0' && c <= '9')))
	{
		if (first_char)
		{
			first_char = FALSE;
		}
		put_char(token, c);
		c = getc(st_lexical_analyzer.file);
	}
	ungetc(c, st_lexical_analyzer.file);

	if (strcmp(token->buff, "new") == 0)
	{
		token->token_kind = NEW;
	}
	else if (strcmp(token->buff, "function") == 0)
	{
		token->token_kind = FUNCTION;
	}
	else if (strcmp(token->buff, "variable") == 0 || strcmp(token->buff, "var") == 0)
	{
		token->token_kind = VARIABLE;
	}
	else if (strcmp(token->buff, "import") == 0)
	{
		token->token_kind = IMPORT;
	}
	else if (strcmp(token->buff, "static") == 0)
	{
		token->token_kind = STATIC;
	}
	else if (strcmp(token->buff, "bool") == 0)
	{
		token->token_kind = BOOL;
	}	
	else if (strcmp(token->buff, "void") == 0)
	{
		token->token_kind = VOID;
	}
	else if (strcmp(token->buff, "int") == 0)
	{
		token->token_kind = INT;
	}
	else if (strcmp(token->buff, "double") == 0)
	{
		token->token_kind = DOUBLE;
	}
	else if (strcmp(token->buff, "string") == 0)
	{
		token->token_kind = STRING;
	}
	else if (strcmp(token->buff, "true") == 0 || strcmp(token->buff, "false") == 0)
	{
		token->token_kind = BOOL_V;
	}
	else if (strcmp(token->buff, "if") == 0)
	{
		token->token_kind = IF;
	}
	else if (strcmp(token->buff, "else") == 0)
	{
		token->token_kind = ELSE;
	}
	else if (strcmp(token->buff, "switch") == 0)
	{
		token->token_kind = SWITCH;
	}
	else if (strcmp(token->buff, "case") == 0)
	{
		token->token_kind = CASE;
	}
	else if (strcmp(token->buff, "do") == 0)
	{
		token->token_kind = DO;
	}
	else if (strcmp(token->buff, "while") == 0)
	{
		token->token_kind = WHILE;
	}
	else if (strcmp(token->buff, "for") == 0)
	{
		token->token_kind = FOR;
	}
	else if (strcmp(token->buff, "continue") == 0)
	{
		token->token_kind = CONTINUE;
	}
	else if (strcmp(token->buff, "break") == 0)
	{
		token->token_kind = BREAK;
	}
	else if (strcmp(token->buff, "return") == 0)
	{
		token->token_kind = RETURN;
	}
	else if (strcmp(token->buff, "record") == 0)
	{
		token->token_kind = RECORD;
	}
	else if (strcmp(token->buff, "nil") == 0)
	{
		token->token_kind = NIL;
	}
	else
	{
		token->token_kind = IDENTIFIER;
	}

	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_string()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '"'));
	c = getc(st_lexical_analyzer.file);
	while (c != '"' && c != EOF)
	{
		if (c == '\n')
		{
			LOG_ERROR("new line in string literal");
			LOG_EXIT();
		}
		if (c == '\\')
		{
			// ESCAPE CHAR
			c = getc(st_lexical_analyzer.file);
			if (c == 'n')
			{
				c = '\n';
			}
			else if (c == '"')
			{
				c = '"';
			}
			else if (c == 't')
			{
				c = '\t';
			}
			else if (c == '\\')
			{
				c = '\\';
			}
			else if (c == EOF)
			{
				LOG_ERROR("EOF in string literal");
				LOG_EXIT();
			}
			else
			{
				c = c;
			}
		}
		put_char(token, c);
		c = getc(st_lexical_analyzer.file);
	}
	if (c == '"')
	{
		token->token_kind = STRING_V;
	}
	else if (c == EOF)
	{
		LOG_ERROR("EOF in string literal");
		LOG_EXIT();
	}
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_dot()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '.'));
	token->token_kind = DOT;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_add_or_increment()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '+'));
	put_char(token, c);
	c = getc(st_lexical_analyzer.file);
	if (c == '+')
	{
		token->token_kind = INCREMENT;
		put_char(token, c);
	}
	else
	{
		token->token_kind = ADD;
		ungetc(c, st_lexical_analyzer.file);
	}
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_sub_or_decrement()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '-'));
	put_char(token, c);
	c = getc(st_lexical_analyzer.file);
	if (c == '-')
	{
		token->token_kind = DECREMENT;
		put_char(token, c);
	}
	else
	{
		token->token_kind = SUB;
		ungetc(c, st_lexical_analyzer.file);
	}
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_mul()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '*'));
	token->token_kind = MUL;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_div_or_comment()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '/'));
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	c = getc(st_lexical_analyzer.file);	
	if (c != '*' && c != '/')
	{
		token->token_kind = DIV;
		ungetc(c, st_lexical_analyzer.file);	
	}
	else if (c == '*' || c == '/')
	{
		// comment
		/*
		/**/
		put_char(token, c);
		if (c == '*')
		{
			while (TRUE)
			{
				while (c = getc(st_lexical_analyzer.file))
				{
					put_char(token, c);
					if (c == '*')
					{
						break;
					}
					if (c == '\n')
					{
						st_lexical_analyzer.line_number++;
					}
				}
				c = getc(st_lexical_analyzer.file);
				put_char(token, c);
				if (c == '/')
				{
					break;
				}
			}
			token->token_kind = COMMENT;
		}
		else if (c == '/')
		{
			while (c = getc(st_lexical_analyzer.file))
			{
				put_char(token, c);
				if (c == '\n')
				{
					st_lexical_analyzer.line_number++;
					token->token_kind = COMMENT;
					break;
				}
			}
		}
	}
	return token;
}

static Token get_mod()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '%'));
	token->token_kind = MOD;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_assign_or_eq()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '='));
	put_char(token, c);
	c = getc(st_lexical_analyzer.file);
	if (c == '=')
	{
		token->token_kind = EQ;
		put_char(token, c);		
	}
	else
	{
		token->token_kind = ASSIGN;
		ungetc(c, st_lexical_analyzer.file);
	}
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_exclamation_or_ne()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '!'));
	put_char(token, c);
	c = getc(st_lexical_analyzer.file);
	if (c == '=')
	{
		token->token_kind = NE;
		put_char(token, c);		
	}
	else
	{
		token->token_kind = EXCLAMATION;
		ungetc(c, st_lexical_analyzer.file);
	}	
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_and()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '&'));
	put_char(token, c);
	c = getc(st_lexical_analyzer.file);
	if (c == '&')
	{
		put_char(token, c);
		token->token_kind = AND;
		token->line_number = st_lexical_analyzer.line_number;
	}
	else
	{		
		LOG_ERROR("wrong char: %c after &", c);
		LOG_EXIT();
	}
	return token;
}

static Token get_or()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '|'));
	put_char(token, c);
	c = getc(st_lexical_analyzer.file);
	if (c == '|')
	{
		put_char(token, c);
		token->token_kind = OR;
		token->line_number = st_lexical_analyzer.line_number;
	}
	else
	{
		LOG_ERROR("wrong char: %c after |", c);
		LOG_EXIT();
	}
	return token;
}

static Token get_lp()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '('));
	token->token_kind = LP;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}


static Token get_rp()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == ')'));
	token->token_kind = RP;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_lb()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '['));
	token->token_kind = LB;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}


static Token get_rb()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == ']'));
	token->token_kind = RB;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_lc()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '{'));
	token->token_kind = LC;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}


static Token get_rc()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '}'));
	token->token_kind = RC;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}


static Token get_lt_or_le()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '<'));
	put_char(token, c);
	c = getc(st_lexical_analyzer.file);
	if (c == '=')
	{
		put_char(token, c);
		token->token_kind = LE;
	}
	else
	{
		token->token_kind = LT;
		ungetc(c, st_lexical_analyzer.file);
	}
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}


static Token get_gt_or_ge()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == '>'));
	put_char(token, c);
	c = getc(st_lexical_analyzer.file);
	if (c == '=')
	{
		put_char(token, c);
		token->token_kind = GE;
	}
	else
	{
		token->token_kind = GT;
		ungetc(c, st_lexical_analyzer.file);
	}
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_comma()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == ','));
	token->token_kind = COMMA;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_semicolon()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == ';'));
	token->token_kind = SEMICOLON;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_colon()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == ':'));
	token->token_kind = COLON;
	put_char(token, c);
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_end_of_file()
{
	Token token = new_token();
	char c = 0;
	c = getc(st_lexical_analyzer.file);
	assert((c == EOF));
	token->token_kind = END_OF_FILE;
	char *eof = "EOF";
	int len = strlen(eof);
	for (int i = 0; i < len; i++)
	{
		put_char(token, eof[i]);
	}
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token get_invalid()
{
	Token token = new_token();
	token->token_kind = INVALID;
	token->line_number = st_lexical_analyzer.line_number;
	return token;
}

static Token next_token()
{
	Token token = NULL;
	// begin analyze
	char c;
	while (c = getc(st_lexical_analyzer.file))
	{
		if (c >= '0' && c <= '9')
		{
			//INT_NUMBER OR REAL_NUMBER
			ungetc(c, st_lexical_analyzer.file);
			token = get_number( );
			break;
		}
		else if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_')
		{
			// IDENTIFIER OR KEYWORD 
			ungetc(c, st_lexical_analyzer.file);
			token = get_keyword_or_identifier();
			break;
		}
		else if (c == '"')
		{
			// STRING
			ungetc(c, st_lexical_analyzer.file);
			token = get_string();
			break;
		}
		else if (c == ',')
		{
			// COMMA
			ungetc(c, st_lexical_analyzer.file);
			token = get_comma();
			break;
		}
		else if (c == ':')
		{
			// COLON or ASSIGN
			ungetc(c, st_lexical_analyzer.file);
			token = get_colon();
			break;
		}
		else if (c == ';')
		{
			// SEMICOLON
			ungetc(c, st_lexical_analyzer.file);
			token = get_semicolon();
			break;
		}
		else if (c == '.')
		{
			// DOT
			ungetc(c, st_lexical_analyzer.file);
			token = get_dot();
			break;
		}
		else if (c == '(')
		{
			// LPAREN
			ungetc(c, st_lexical_analyzer.file);
			token = get_lp();
			break;
		}
		else if (c == ')')
		{
			// RPAREN
			ungetc(c, st_lexical_analyzer.file);
			token = get_rp();
			break;
		}
		else if (c == '[')
		{
			// LBRACK
			ungetc(c, st_lexical_analyzer.file);
			token = get_lb();
			break;
		}
		else if (c == ']')
		{
			// RBRACK
			ungetc(c, st_lexical_analyzer.file);
			token = get_rb();
			break;
		}
		else if (c == '{')
		{
			// LBRACE
			ungetc(c, st_lexical_analyzer.file);
			token = get_lc();
			break;
		}
		else if (c == '}')
		{
			// RBRACE
			ungetc(c, st_lexical_analyzer.file);
			token = get_rc();
			break;
		}
		else if (c == '+')
		{
			// ADD
			ungetc(c, st_lexical_analyzer.file);
			token = get_add_or_increment();
			break;
		}
		else if (c == '-')
		{
			// SUB
			ungetc(c, st_lexical_analyzer.file);
			token = get_sub_or_decrement();;
			break;
		}
		else if (c == '*')
		{
			// MUL
			ungetc(c, st_lexical_analyzer.file);
			token = get_mul();
			break;
		}
		else if (c == '/')
		{
			// DIV
			ungetc(c, st_lexical_analyzer.file);
			token = get_div_or_comment();
			break;
		}	
		else if (c == '%')
		{
			// MOD
			ungetc(c, st_lexical_analyzer.file);
			token = get_mod();
			break;
		}
		else if (c == '=')
		{
			// ASSIGN or EQ
			ungetc(c, st_lexical_analyzer.file);
			token = get_assign_or_eq();
			break;
		}
		else if (c == '!')
		{
			// EXCLAMATION or NE
			ungetc(c, st_lexical_analyzer.file);
			token = get_exclamation_or_ne();
			break;
		}
		else if (c == '<')
		{
			// LT or LE
			ungetc(c, st_lexical_analyzer.file);
			token = get_lt_or_le();
			break;
		}
		else if (c == '>')
		{
			// > or >=
			ungetc(c, st_lexical_analyzer.file);
			token = get_gt_or_ge();
			break;
		}
		
		else if (c == '&')
		{
			ungetc(c, st_lexical_analyzer.file);
			token = get_and();
			break;
		}
		else if (c == '|')
		{
			ungetc(c, st_lexical_analyzer.file);
			token = get_or();
			break;
		}

		else if (c == EOF)
		{
			// END_OF_FILE
			ungetc(c, st_lexical_analyzer.file);
			token = get_end_of_file();
			break;
		}
		else if (c == ' ' || c == '\t' || c == '\n')
		{
			if (c == '\n')
			{
				st_lexical_analyzer.line_number++;
			}
			continue;
		}
		else
		{
			ungetc(c, st_lexical_analyzer.file);
			token = get_invalid();
			break;
		}
	}
	return token;
}

void LEX_initialize(char *file_name)
{
	FILE *file = fopen(file_name, "r");
	if (file == NULL)
	{
		LOG_ERROR("cannot open file %s\n", file_name);
		LOG_EXIT();
	}
	st_lexical_analyzer.file = file;
	st_lexical_analyzer.prev = NULL;
	st_lexical_analyzer.next = NULL;
	st_lexical_analyzer.line_number = 1;
	// initialize
	struct Token_List * token_list = NULL;
	Token token = NULL;
	struct Token_List **iterator = NULL;
	iterator = &(st_lexical_analyzer.next);
	do
	{
		token = next_token();
		token_list = MEM_malloc(sizeof(struct Token_List));
		token_list->token = token;
		token_list->u.next = NULL;
		*iterator = token_list;
		iterator = &(token_list->u.next);
	} while (token->token_kind != END_OF_FILE);
}

void LEX_uninitialize()
{
	struct Token_List *token_list = NULL;
	while (st_lexical_analyzer.prev != NULL)
	{
		token_list = st_lexical_analyzer.prev;
		st_lexical_analyzer.prev = token_list->u.prev;
		// free
		delete_token(token_list->token);
		free(token_list);
	}
	while (st_lexical_analyzer.next != NULL)
	{
		token_list = st_lexical_analyzer.next;
		st_lexical_analyzer.next = token_list->u.next;
		// free
		delete_token(token_list->token);
		free(token_list);
	}
	st_lexical_analyzer.file = NULL;
	st_lexical_analyzer.prev = NULL;
	st_lexical_analyzer.next = NULL;
	st_lexical_analyzer.line_number = 1;
}

Token LEX_get_token()
{
	if (st_lexical_analyzer.file == NULL)
	{
		LOG_ERROR("no source file");
		LOG_EXIT();
	}
	if (st_lexical_analyzer.next != NULL)
	{
		struct Token_List *temp = NULL;
		// next list
		temp = st_lexical_analyzer.next;
		st_lexical_analyzer.next = temp->u.next;
		// prev list
		temp->u.prev = st_lexical_analyzer.prev;
		st_lexical_analyzer.prev = temp;
		if (temp->token->token_kind != COMMENT)
		{
			return temp->token;
		}
		else
		{
			return LEX_get_token();
		}
	}
	return NULL;
}

Token LEX_get_token_before_token(Token token)
{
	if (st_lexical_analyzer.prev == NULL)
	{
		return NULL;
	}
	struct Token_List *it = st_lexical_analyzer.prev;
	while (it != NULL)
	{
		if (it->token == token)
		{
			if (it->u.prev == NULL)
			{
				return NULL;
			}
			else
			{
				return it->u.prev->token;
			}
		}
		else
		{
			it = it->u.prev;
		}
	}
	return NULL;
}
void LEX_unget_token(Token token)
{
	if (token == NULL)
	{
		return;
	}
	struct Token_List *temp = NULL;
	temp = st_lexical_analyzer.prev;
	if (temp->token != token)
	{
		LOG_ERROR("cannot unget token because the order is wrong");
		LOG_EXIT();
	}
	// prev list
	st_lexical_analyzer.prev = temp->u.prev;
	// next;
	temp->u.next = st_lexical_analyzer.next;
	st_lexical_analyzer.next = temp;
}


void LEX_print_token(Token token)
{
	if (token == NULL)
	{
		return;
	}
	char *str = st_token_kind_string[token->token_kind - TOKEN_BEGIN];
	LOG_INFO("%d [%s, %s, char count: %d, buff size: %d]\n", token->line_number, token->buff, str, token->char_count, token->buff_size);
	return;
}