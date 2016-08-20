#ifndef __SYMBOL_H
#define __SYMBOL_H
#include "ABSYNT.h"
#include "red_black_tree.h"
#include "stack.h"
#include "literal.h"

typedef struct Symbol_Table_tag *Symbol_Table;
typedef char *Symbol_Name;
typedef struct Symbol_Information_tag *Symbol_Information;
typedef struct Function_Prototype_tag *Function_Prototype;
typedef struct Symbol_Definition_tag *Symbol_Definition;

struct Symbol_Information_tag
{
	Type type;
	int offset;
};

struct Function_Prototype_tag
{
	Function_Definition function_definition;
	char *function_name;
	Type retval_type;
	int function_index;
	int parameter_number;
	Type *parameter_list;
};

struct Symbol_Table_tag
{
	Map table;
	struct Symbol_Table_tag *parent;
	char *table_name;
	int start_offset;
	int symbol_number;
};

struct Symbol_Definition_tag
{
	Symbol_Table global_variable;
	Map record_definition;
	Map function_prototype;
	Map function_definition;
};

// function
int symbol_name_compare(Symbol_Name symbol_name1, Symbol_Name symbol_name2);
Symbol_Name symbol_name_create(char *symbol_name);
Symbol_Information symbol_information_create(Type type, int offset);
Function_Prototype function_prototype_create(Function_Definition function_definiton, Map record_definition);
void symbol_name_free(Symbol_Name symbol_name);
void symbol_information_free(Symbol_Information si);
void function_prototype_free(Function_Prototype function_prototype);
void symbol_name_print(Symbol_Name symbol_name);
void symbol_information_print(Symbol_Information si);
void function_prototype_print(Function_Prototype function_prototype);
Symbol_Table symbol_table_create();
void symbol_table_free(Symbol_Table st);
void symbol_table_print(Symbol_Table st);
Symbol_Definition symbol_definition_create();
void symbol_definition_free(Symbol_Definition sd);
#endif