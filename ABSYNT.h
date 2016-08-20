/*
** abstract syntax tree
*/
#ifndef __ABSYNT_H
#define __ABSYNT_H
#include "constant.h"

typedef struct Translation_Unit_tag *Translation_Unit;
typedef struct Import_List_tag *Import_List;
typedef struct Definition_List_tag *Definition_List;
typedef struct Function_Definition_tag *Function_Definition;
typedef struct Record_Definition_tag *Record_Definition;
typedef struct Parameter_List_tag *Parameter_List;
typedef struct Argument_List_tag *Argument_List;
typedef struct Statement_List_tag *Statement_List;
typedef struct Basic_Type_tag *Basic_Type;
typedef struct Record_Type_tag *Record_Type;
typedef struct Type_tag *Type;
typedef struct Expression_List_tag *Expression_List;
typedef struct Comma_Expression_tag *Comma_Expression;
typedef struct Assign_Expression_tag *Assign_Expression;
typedef struct Function_Call_Expression_tag *Function_Call_Expression;
typedef struct Field_Access_Expression_tag *Field_Access_Expression;
typedef struct Array_Index_Expression_tag *Array_Index_Expression;
typedef struct New_Expression_tag *New_Expression;
typedef struct Dimension_List_tag *Dimension_List;
typedef struct Array_Literal_tag *Array_Litera;
typedef struct Identifier_Expression_tag *Identifier_Expression;
typedef struct Expression_tag *Expression;
typedef struct Binary_Expression_tag *Binary_Expression;
typedef struct Statement_tag *Statement;
typedef struct If_Statement_tag *If_Statement;
typedef struct While_Statement_tag *While_Statement;
typedef struct Do_While_Statement_tag *Do_While_Statement;
typedef struct For_Statement_tag *For_Statement;
typedef struct Switch_Statement_tag *Swithc_Statement;
typedef struct Case_List_tag *Case_List;
typedef struct Return_Statement_tag *Return_Statement;
typedef struct Continue_Statement_tag *Continue_Statement;
typedef struct Break_Statement_tag *Break_Statement;
typedef struct Declaration_Statement_tag *Declaration_Statement;
typedef struct Declaration_Variable_List_tag *Declaration_Variable_List;
typedef struct Block_tag *Block;

//
typedef struct Symbol_Table_tag *Symbol_Table;
typedef struct Symbol_Definition_tag *Symbol_Definition;

struct Translation_Unit_tag
{
	Import_List import_list;
	Definition_List definition_list;
	Symbol_Definition symbol_definition;
};

struct Import_List_tag
{
	char *package_name;
	Import_List next;
	int line_number;
};
typedef enum 
{
	FUNCTION_DEFINITION, RECORD_DEFINITION, VARIABLE_DEFINITION
} Definition_Kind;

struct Definition_List_tag
{
	Definition_Kind kind;
	union
	{
		Function_Definition function_definition;
		Record_Definition record_defintion;
		Statement variable_definition;
	}u;
	Definition_List next;
};

typedef enum 
{
	NIL_TYPE, VOID_TYPE, BOOL_TYPE, INT_TYPE, DOUBLE_TYPE, STRING_TYPE
} Basic_Type_Kind;

struct Basic_Type_tag
{
	Basic_Type_Kind kind;
};
struct Record_Type_tag
{
	char *record_name;
};

typedef enum Type_Kind_tag
{
	BASIC_TYPE,
	RECORD_TYPE,
	ARRAY_TYPE
} Type_Kind;

struct Type_tag
{
	Type_Kind kind;
	union
	{
		Basic_Type basic_type;
		Record_Type record_type;
		Type array_type;
	}u;
};

struct Record_Definition_tag
{
	char *record_name;
	Block field_block;
	//Statement_List field_list;
	int line_number;
	Symbol_Table table;
};

typedef enum
{
	USER_DEFINIED_FUNCTION,
	NAIVE_FUNCTION
}Function_Type;

struct Function_Definition_tag
{
	int is_static;
	Type type;
	char *function_name;
	Parameter_List parameter_list;
	Function_Type function_kind;
	union
	{
		Block user_defined_function;
	}u;
	//Block body;
	int line_number;
};

struct Parameter_List_tag
{
	Type type;
	char *parameter_name;
	Parameter_List next;
	int line_number;
};

struct Argument_List_tag
{
	Expression expression;
	Argument_List next;
};

struct Expression_List_tag
{
	Expression expression;
	Expression_List next;
};
typedef enum 
{
	COMMA_EXPRESSION,
	ASSIGN_EXPRESSION,
	OR_EXPRESSION,
	AND_EXPRESSION,
	EQ_EXPRESSION,
	NE_EXPRESSION,
	GT_EXPRESSION,
	GE_EXPRESSION,
	LT_EXPRESSION,
	LE_EXPRESSION,
	ADD_EXPRESSION,
	SUB_EXPRESSION,
	MUL_EXPRESSION,
	DIV_EXPRESSION,
	MOD_EXPRESSION,
	MINUS_EXPRESSION,
	NOT_EXPRESSION,
	INC_EXPRESSION,
	DEC_EXPRESSION,
	IDENTIFIER_EXPRESSION,
	FUNCTION_CALL_EXPRESSION,
	FIELD_ACCESS_EXPRESSION,
	ARRAY_INDEX_EXPRESSION,
	ARRAY_LITERAL_EXPRESSION,
	NEW_EXPRESSION,
	NIL_EXPRESSION,
	BOOL_EXPRESSION,
	INT_EXPRESSION,
	DOUBLE_EXPRESSION,
	STRING_EXPRESSION,
	PARENTHESIS_EXPRESSION,
}Expression_Kind;

struct Comma_Expression_tag
{
	Expression left;
	Expression right;
};

struct Assign_Expression_tag
{
	Expression left;
	Expression right;
};

struct Binary_Expression_tag
{
	Expression left;
	Expression right;
};

struct Function_Call_Expression_tag
{
	Function_Definition function_definition;
	char *function_name;
	Argument_List argument_list;
};

struct Field_Access_Expression_tag
{
	Expression record_name;
	Expression field_name;
};

struct Array_Index_Expression_tag
{
	Expression array_name;
	Expression index;
};
struct Array_Literal_tag
{
	Expression_List expression_list;
};

struct New_Expression_tag
{
	Type_Kind type_kind;
	union
	{
		Basic_Type basic_type;
		Record_Type record_type;
	}u;
	Dimension_List dimension_list;
};

struct Dimension_List_tag
{
	Expression dimension_expression;
	Dimension_List next_dimension;
};

struct Identifier_Expression_tag
{
	char *name;
};

struct Expression_tag
{
	Expression_Kind kind;
	Type type;
	int line_number;
	union
	{
		Comma_Expression comma_expression;
		Assign_Expression assign_expression;
		Binary_Expression binary_expression;
		Expression minus_expression;
		Expression not_expression;
		Expression inc_or_dec_expression;
		Function_Call_Expression function_call_expression;
		Field_Access_Expression field_access_expression;
		Array_Index_Expression array_index_expression;
		New_Expression new_expression;
		Identifier_Expression identifier_expression;
		int int_value;
		double double_value;
		int bool_value;
		char *string_value;
		Expression parenthesis_expression;
	}u;
};

typedef enum
{
	IF_WITHOUT_ELSE,
	IF_WITH_ELSE,
	IF_WITH_ELSE_IF,
}If_Else_Kind;

struct If_Statement_tag
{
	int else_label_index;
	int end_label_index;
	Expression condition;
	Block then_block;
	If_Else_Kind kind;
	union
	{
		Block else_block;
		Statement else_if_statement;
	}u;
	int line_number;
};

struct Do_While_Statement_tag
{
	int begin_label_index;
	int end_label_index;
	Block block;
	Expression condition;
	int line_number;
};

struct While_Statement_tag
{
	int begin_label_index;
	int end_label_index;
	Expression condition;
	Block block;
	int line_number;
};

struct For_Statement_tag
{
	int condition_label_index;
	int post_label_index;
	int end_label_index;
	Expression init;
	Expression condition;
	Expression post;
	Block block;
	int line_number;
};

struct Switch_Statement_tag
{
	Expression condition;
	Case_List case_list;
	Statement default_case;
};
struct Case_List_tag
{
	Expression condition;
	Block block;
	Case_List next_case;
};
struct Return_Statement_tag
{
	Expression retval;
	int line_number;
};

struct Break_Statement_tag
{
	int line_number;
};

struct Continue_Statement_tag
{
	int line_number;
};

struct Declaration_Variable_List_tag
{
	char *variable_name;
	Expression initialization;
	struct Declaration_Variable_List_tag *next;
};
struct Declaration_Statement_tag
{
	int is_static;
	Type type;
	Declaration_Variable_List variable_list;
	int line_number;
};

typedef enum 
{
	EXPRESSION_STATEMENT,
	IF_STATEMENT,
	DO_WHILE_STATEMENT,
	WHILE_STATEMENT,
	FOR_STATEMENT,
	RETURN_STATEMENT,
	BREAK_STATEMENT,
	CONTINUE_STATEMENT,
	DECLARATION_STATEMENT,
	BLOCK,
}Statement_Kind;
struct Statement_tag
{
	Statement_Kind kind;
	int line_number;
	union
	{
		Expression expression_statement;
		If_Statement if_statement;
		While_Statement while_statement;
		Do_While_Statement do_while_statement;
		For_Statement for_statement;
		Return_Statement return_statement;
		Break_Statement break_statement;
		Continue_Statement continue_statement;
		Declaration_Statement declaration_statement;
		Block block;
	}u;
};

struct Statement_List_tag
{
	Statement statement;
	struct Statement_List_tag *next;
};
typedef enum
{
	RECORD_BLOCK, // record definition
	FUNCTION_BLOCK, // function definition
	FOR_BLOCK, 
	DO_WHILE_BLOCK, 
	WHILE_BLOCK,
	IF_ELSE_BLOCK,
	LOCAL_BLOCK,
	UNKNOWN_BLOCK, // the type of block is not determined
}Block_Type;

struct Block_tag
{
	Block_Type block_type;
	union
	{
		For_Statement for_statement;
		Do_While_Statement do_while_statement;
		While_Statement while_statement;
		If_Statement if_statement;
		Function_Definition function_definition;
	}u;
	Statement_List statement_list;
	int line_number;
	Block outer;
	Symbol_Table block_table;
};


#endif