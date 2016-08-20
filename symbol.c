#include "environment.h"
#include "symbol.h"
#include "tool.h"
#include "memory.h"
#include "LOG.h"
#include "absynt_print.h"
#include "semantic_analysis.h"

int symbol_name_compare(Symbol_Name symbol_name1, Symbol_Name symbol_name2)
{
	return strcmp(symbol_name1, symbol_name2);
}
Symbol_Name symbol_name_create(char *symbol_name)
{
	return string_create(symbol_name);
}
Symbol_Information symbol_information_create(Type type, int offset)
{
	Symbol_Information si = MEM_malloc(sizeof(*si));
	si->type = type;
	si->offset = offset;
	return si;
}
Function_Prototype function_prototype_create(Function_Definition function_definition, Map record_definition)
{
	Function_Prototype function_prototype = MEM_malloc(sizeof(*function_prototype));
	function_prototype->function_definition = function_definition;
	function_prototype->function_index = -1;
	function_prototype->function_name = string_create(function_definition->function_name);
	function_prototype->retval_type = type_copy(function_definition->type);
	function_prototype->parameter_number = 0;
	Parameter_List parameter_list = function_definition->parameter_list;
	while (parameter_list != NULL)
	{
		function_prototype->parameter_number++;
		parameter_list = parameter_list->next;
	}
	function_prototype->parameter_list = MEM_malloc(sizeof(*(function_prototype->parameter_list)) 
		* function_prototype->parameter_number);
	parameter_list = function_definition->parameter_list;
	for (int idx = 0; idx < function_prototype->parameter_number; idx++)
	{
		if (type_check(parameter_list->type, record_definition) == FALSE)
		{
			LOG_ERROR("[line %d]the type ", function_prototype->function_definition->line_number);
			type_print(parameter_list->type);
			LOG_ERROR(" in function %s definition", function_prototype->function_name);
			LOG_ERROR(" is unknown");
			LOG_EXIT();
		}
		function_prototype->parameter_list[idx] = type_copy(parameter_list->type);
		parameter_list = parameter_list->next;
	}
	return function_prototype;
}

void symbol_name_free(Symbol_Name symbol_name)
{
	string_free(symbol_name);
}

void symbol_information_free(Symbol_Information si)
{
	MEM_free(si);
}

void function_prototype_free(Function_Prototype function_prototype)
{
	MEM_free(function_prototype->parameter_list);
	string_free(function_prototype->function_name);
	MEM_free(function_prototype);
}
void symbol_name_print(Symbol_Name symbol_name)
{
	if (symbol_name == NULL)
	{
		return;
	}
	LOG_INFO(symbol_name);
}
void symbol_information_print(Symbol_Information si)
{
	if (si == NULL)
	{
		return;
	}
	LOG_INFO("[type: ");
	type_print(si->type);
	LOG_INFO(", offset:");
	LOG_INFO("%d]", si->offset);
}
void function_prototype_print(Function_Prototype function_prototype)
{
	type_print(function_prototype->retval_type);
	LOG_INFO(" %s(", function_prototype->function_name);
	for (int idx = 0; idx < function_prototype->parameter_number; idx++)
	{
		if (idx > 0)
		{
			LOG_INFO(", ");
		}
		type_print(function_prototype->parameter_list[idx]);
	}
	LOG_INFO(")");
}
Symbol_Table symbol_table_create()
{
	Symbol_Table st = MEM_malloc(sizeof(*st));
	st->symbol_number = 0;
	st->start_offset = 0;
	st->parent = NULL;
	st->table_name = NULL;
	st->table = map_create(symbol_name_compare, symbol_name_free, symbol_information_free,
		symbol_name_print, symbol_information_print);
	return st;
}

void symbol_table_free(Symbol_Table st)
{
	MEM_free(st->table_name);
	map_free(st->table);
	MEM_free(st);
}

void symbol_table_print(Symbol_Table st)
{
	LOG_INFO("table name: %s\nstart offset: %d, symbol number: %u\n",
		st->table_name, st->start_offset, st->symbol_number);
	LOG_INFO("table definition:\n");
	LOG_INFO("{\n");
	map_print(st->table);
	LOG_INFO("}\n");
}

Symbol_Definition symbol_definition_create()
{
	Symbol_Definition sd = MEM_malloc(sizeof(*sd));
	sd->global_variable = symbol_table_create();
	sd->global_variable->table_name = string_create("global variable");
	sd->function_prototype = map_create(symbol_name_compare, symbol_name_free, function_prototype_free,
		symbol_name_print, function_prototype_print);
	sd->record_definition = map_create(symbol_name_compare, symbol_name_free, symbol_table_free,
		symbol_name_print, symbol_table_print);
	sd->function_definition = map_create(symbol_name_compare, symbol_name_free, symbol_table_free,
		symbol_name_print, symbol_table_print);
	return sd;
}

void symbol_definition_free(Symbol_Definition sd)
{
	map_free(sd->function_prototype);
	map_free(sd->function_definition);
	map_free(sd->record_definition);
	symbol_table_free(sd->global_variable);
	MEM_free(sd);
}