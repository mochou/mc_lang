#include "environment.h"
#include "semantic_analysis.h"
#include "symbol.h"
#include "LOG.h"
#include "tool.h"
#include "memory.h"
#include "absynt_print.h"

char *operator_information[] =
{
	",",
	"=",
	"||",
	"&&",
	"==",
	"!=",
	">",
	">=",
	"<",
	"<=",
	"+",
	"-",
	"*",
	"/",
	"%",
	"-",
	"!",
	"++",
	"--",
	"IDENTIFIER",
	"FUNCTION_CALL",
	"FIELD_ACCESS",
	"ARRAY_INDEX",
	"ARRAY_LITERAL",
	"NEW",
	"NIL",
	"BOOL",
	"INT",
	"DOUBLE",
	"STRING",
	"PARENTHESIS",
};
void translation_unit_semantic_analysis(Translation_Unit tu)
{
	if (tu == NULL)
	{
		return;
	}
	tu->symbol_definition = symbol_definition_create();
	record_definition_semantic_analysis(tu);
	variable_definition_semantic_analysis(tu);
	function_definition_semantic_analysis(tu);
	//semantic_print(tu);
	//symbol_definition_free(symbol_definition);
}
void variable_definition_semantic_analysis(Translation_Unit tu)
{
	Definition_List definition_list = tu->definition_list;
	while (definition_list != NULL)
	{
		if (definition_list->kind == VARIABLE_DEFINITION)
		{
			Statement stm = definition_list->u.variable_definition;
			declaration_statement_semantic_analysis(NULL, tu->symbol_definition, 
				stm->u.declaration_statement);			
		}
		definition_list = definition_list->next;
	}
	//symbol_table_print(tu->symbol_definition->global_variable);
}

void record_definition_semantic_analysis(Translation_Unit tu)
{
	Definition_List definition_list = tu->definition_list;
	while (definition_list != NULL)
	{
		if (definition_list->kind == RECORD_DEFINITION)
		{
			Symbol_Name record_name = symbol_name_create(definition_list->u.record_defintion->record_name);
			Symbol_Table record_table = map_search(tu->symbol_definition->record_definition, record_name);
			if (record_table != NULL)
			{
				LOG_ERROR("record %s redefined", record_name);
				LOG_EXIT();
			}
			record_table = symbol_table_create();
			record_table->table_name = string_create(definition_list->u.record_defintion->record_name);
			record_table->parent = tu->symbol_definition->global_variable; 
			record_table->start_offset = 0;
			record_table->symbol_number = 0;			
			map_insert(tu->symbol_definition->record_definition, record_name, record_table);
			definition_list->u.record_defintion->table = record_table;
		}
		definition_list = definition_list->next;
	}
	definition_list = tu->definition_list;
	while (definition_list != NULL)
	{
		if (definition_list->kind == RECORD_DEFINITION)
		{
			Symbol_Name record_name = symbol_name_create(definition_list->u.record_defintion->record_name);
			Symbol_Table record_table = map_search(tu->symbol_definition->record_definition, record_name);
			Statement_List statement_list = definition_list->u.record_defintion->field_block->statement_list;
			if (statement_list == NULL)
			{
				LOG_ERROR("record %s has no field", record_name);
				LOG_EXIT();
			}
			while (statement_list != NULL)
			{
				Statement stm = statement_list->statement;
				if (stm->kind != DECLARATION_STATEMENT)
				{
					LOG_ERROR("record %s need variable declaration", record_name);
					LOG_EXIT();
				}
				//declaration_statement_semantic_analysis(NULL, tu->symbol_definition, stm->u.declaration_statement);
				statement_list = statement_list->next;
			}
			definition_list->u.record_defintion->field_block->block_table = record_table;
			definition_list->u.record_defintion->field_block->block_type = RECORD_BLOCK;
			definition_list->u.record_defintion->field_block->outer = NULL;
			block_semantic_analysis(definition_list->u.record_defintion->field_block, tu->symbol_definition);
			symbol_name_free(record_name);
		}
		definition_list = definition_list->next;
	}
	//map_print(tu->symbol_definition->record_definition);
}

void function_definition_semantic_analysis(Translation_Unit tu)
{
	Definition_List definition_list = tu->definition_list;
	int function_index = 0;
	while (definition_list != NULL)
	{
		if (definition_list->kind == FUNCTION_DEFINITION)
		{
			Function_Definition function_definition = definition_list->u.function_definition;
			// first process function name and type
			Symbol_Name function_name = symbol_name_create(function_definition->function_name);
			Function_Prototype function_prototype = map_search(tu->symbol_definition->function_prototype, 
				function_name);
			if (function_prototype != NULL)
			{
				LOG_ERROR("function %s redefined", function_name);
				LOG_EXIT();
			}
			function_prototype = function_prototype_create(function_definition, 
				tu->symbol_definition->record_definition);			
			function_prototype->function_index = function_index++;
			map_insert(tu->symbol_definition->function_prototype, function_name, function_prototype);
		}
		definition_list = definition_list->next;
	}
	definition_list = tu->definition_list;
	while (definition_list != NULL)
	{
		if (definition_list->kind == FUNCTION_DEFINITION)
		{
			// second process function parameter
			Function_Definition function_definition = definition_list->u.function_definition;
			Symbol_Name function_name = symbol_name_create(function_definition->function_name);
			Symbol_Table function_table = symbol_table_create();
			function_table->table_name = string_create(function_definition->function_name);
			function_table->parent = tu->symbol_definition->global_variable;
			function_table->start_offset = -2;
			function_table->symbol_number = 2;
			if (function_definition->function_kind == USER_DEFINIED_FUNCTION)
			{
				function_definition->u.user_defined_function->block_table = function_table;
			}
			map_insert(tu->symbol_definition->function_definition, function_name, function_table);
			Parameter_List pl = function_definition->parameter_list;
			int parameter_number = 0;
			while (pl != NULL)
			{
				parameter_number++;
				pl = pl->next;
			}
			function_table->start_offset -= parameter_number;
			function_table->symbol_number += parameter_number;;
			pl = function_definition->parameter_list;
			while (pl != NULL)
			{
				Symbol_Name parameter_name = symbol_name_create(pl->parameter_name);
				Symbol_Information parameter_information = NULL;
				parameter_information = map_search(function_table->table, parameter_name);
				if (parameter_information != NULL)
				{
					LOG_ERROR("parameter %s redefined", parameter_name);
					LOG_EXIT();
				}
				parameter_information = symbol_information_create(pl->type,	-(parameter_number--) - 2);
				map_insert(function_table->table, parameter_name, parameter_information);
				pl = pl->next;
			}
			// third process function body
			if (function_definition->function_kind == USER_DEFINIED_FUNCTION)
			{
				function_definition->u.user_defined_function->block_type = FUNCTION_BLOCK;
				function_definition->u.user_defined_function->outer = NULL;
				function_definition->u.user_defined_function->u.function_definition = function_definition;
				block_semantic_analysis(function_definition->u.user_defined_function, tu->symbol_definition);
			}
			function_table->symbol_number -= 2;
			//symbol_table_print(function_table);
		}
		definition_list = definition_list->next;
	}
	//map_print(tu->symbol_definition->function_prototype);
	//map_print(tu->symbol_definition->function_definition);
}

void block_semantic_analysis(Block block, Symbol_Definition symbol_definition)
{
	//LOG_INFO("line: %d\n", block->line_number);
	Statement_List statement_list = block->statement_list;
	while (statement_list != NULL)
	{
		Statement stm = statement_list->statement;
		switch (stm->kind)
		{
			case EXPRESSION_STATEMENT:
			{
				expression_semantic_analysis(block, symbol_definition,
					stm->u.expression_statement);
				break;
			}
			case IF_STATEMENT:
			{				
				if_statement_semantic_analysis(block, symbol_definition,
					stm->u.if_statement);
				break;
			}
			case DO_WHILE_STATEMENT:
			{
				do_while_statement_semantic_analysis(block, symbol_definition,
					stm->u.do_while_statement);
				break;
			}
			case WHILE_STATEMENT:
			{
				while_statement_semantic_analysis(block, symbol_definition,
					stm->u.while_statement);
				break;
			}
			case FOR_STATEMENT:
			{
				for_statement_semantic_analysis(block, symbol_definition,
					stm->u.for_statement);
				break;
			}
			case RETURN_STATEMENT:
			{
				return_statement_semantic_analysis(block, symbol_definition,
					stm->u.return_statement);
				break;
			}
			case BREAK_STATEMENT:
			{
				break_statement_semantic_analysis(block, symbol_definition,
					stm->u.break_statement);
				break;
			}
			case CONTINUE_STATEMENT:
			{
				continue_statement_semantic_analysis(block, symbol_definition,
					stm->u.continue_statement);
				break;
			}
			case DECLARATION_STATEMENT:
			{
				declaration_statement_semantic_analysis(block, symbol_definition,
					stm->u.declaration_statement);
				break;
			}
			case BLOCK:
			{
				Symbol_Table local_block_table = symbol_table_create();
				local_block_table->parent = block->block_table;
				local_block_table->table_name = string_add(block->block_table->table_name, "->[local block]");
				local_block_table->start_offset = block->block_table->start_offset 
					+ block->block_table->symbol_number;
				local_block_table->symbol_number = 0;
				stm->u.block->block_type = LOCAL_BLOCK;
				stm->u.block->outer = block;
				stm->u.block->block_table = local_block_table;
				block_semantic_analysis(stm->u.block, symbol_definition);
				//symbol_table_free(local_block_table);
				break;
			}
		}
		statement_list = statement_list->next;
	}
	//symbol_table_print(block->table);
}

void if_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	If_Statement statement)
{
	while (TRUE)
	{
		Symbol_Table child_symbol_table = symbol_table_create();
		child_symbol_table->parent = cur_block->block_table;
		child_symbol_table->table_name = string_add(cur_block->block_table->table_name, "->[if]");
		child_symbol_table->start_offset = cur_block->block_table->start_offset
			+ cur_block->block_table->symbol_number;
		child_symbol_table->symbol_number = 0;
		statement->then_block->block_type = IF_ELSE_BLOCK;
		statement->then_block->u.if_statement = statement;
		statement->then_block->outer = cur_block;
		statement->then_block->block_table = child_symbol_table;
		expression_semantic_analysis(cur_block, symbol_definition, statement->condition);
		block_semantic_analysis(statement->then_block, symbol_definition);
		//symbol_table_free(child_symbol_table);
		if (statement->kind == IF_WITHOUT_ELSE)
		{
			break;
		}
		else if (statement->kind == IF_WITH_ELSE)
		{
			child_symbol_table = symbol_table_create();
			child_symbol_table->parent = cur_block->block_table;
			child_symbol_table->table_name = string_add(cur_block->block_table->table_name, "->[else]");
			child_symbol_table->start_offset = cur_block->block_table->start_offset
				+ cur_block->block_table->symbol_number;
			child_symbol_table->symbol_number = 0;
			statement->u.else_block->block_type = IF_ELSE_BLOCK;
			statement->u.else_block->u.if_statement = statement;
			statement->u.else_block->outer = cur_block;
			statement->u.else_block->block_table = child_symbol_table;
			block_semantic_analysis(statement->u.else_block, symbol_definition);
			//symbol_table_free(child_symbol_table);
			break;
		}
		else if (statement->kind == IF_WITH_ELSE_IF)
		{
			statement = statement->u.else_if_statement->u.if_statement;
		}
	}
}
void while_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	While_Statement statement)
{
	Symbol_Table child_symbol_table = symbol_table_create();
	child_symbol_table->parent = cur_block->block_table;
	child_symbol_table->table_name = string_add(cur_block->block_table->table_name, "->[while]");
	child_symbol_table->start_offset = cur_block->block_table->start_offset
		+ cur_block->block_table->symbol_number;
	child_symbol_table->symbol_number = 0;
	statement->block->block_type = WHILE_BLOCK;
	statement->block->u.while_statement = statement;
	statement->block->outer = cur_block;
	statement->block->block_table = child_symbol_table;
	expression_semantic_analysis(cur_block, symbol_definition, statement->condition);
	block_semantic_analysis(statement->block, symbol_definition);
	//symbol_table_free(child_symbol_table);
}
void do_while_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Do_While_Statement statement)
{
	Symbol_Table child_symbol_table = symbol_table_create();
	child_symbol_table->parent = cur_block->block_table;
	child_symbol_table->table_name = string_add(cur_block->block_table->table_name, "->[do while]");
	child_symbol_table->start_offset = cur_block->block_table->start_offset
		+ cur_block->block_table->symbol_number;
	child_symbol_table->symbol_number = 0;
	statement->block->block_type = DO_WHILE_BLOCK;
	statement->block->u.do_while_statement = statement;
	statement->block->outer = cur_block;
	statement->block->block_table = child_symbol_table;
	expression_semantic_analysis(cur_block, symbol_definition, statement->condition);
	block_semantic_analysis(statement->block, symbol_definition);
	//symbol_table_free(child_symbol_table);
}
void for_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	For_Statement statement)
{
	Symbol_Table child_symbol_table = symbol_table_create();
	child_symbol_table->parent = cur_block->block_table;
	child_symbol_table->table_name = string_add(cur_block->block_table->table_name, "->[for]");
	child_symbol_table->start_offset = cur_block->block_table->start_offset
		+ cur_block->block_table->symbol_number;
	child_symbol_table->symbol_number = 0;
	statement->block->block_type = FOR_BLOCK;
	statement->block->u.for_statement = statement;
	statement->block->outer = cur_block;
	statement->block->block_table = child_symbol_table;
	expression_semantic_analysis(cur_block, symbol_definition, statement->init);
	expression_semantic_analysis(cur_block, symbol_definition, statement->condition);
	expression_semantic_analysis(cur_block, symbol_definition, statement->post);
	block_semantic_analysis(statement->block, symbol_definition);
	//symbol_table_free(child_symbol_table);
}
void return_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Return_Statement statement)
{
	Block function_block = cur_block;
	while (function_block != NULL)
	{
		if (function_block->block_type == FUNCTION_BLOCK)
		{
			break;
		}
		else
		{
			function_block = function_block->outer;
		}
	}
	if (function_block == NULL)
	{
		LOG_ERROR("return statement may only be used within function definition");
		LOG_EXIT();
	}
	if (statement->retval != NULL)
	{
		expression_semantic_analysis(cur_block, symbol_definition, statement->retval);

		if (type_equal(statement->retval->type, function_block->u.function_definition->type) == FALSE)
		{
			LOG_ERROR("the type of function %s is ", function_block->u.function_definition->function_name);
			type_print(function_block->u.function_definition->type);
			LOG_ERROR(", but the type of returned value ");
			expression_print(statement->retval);
			LOG_ERROR(" is ");
			type_print(statement->retval->type);
			LOG_ERROR("");
			LOG_EXIT();
		}
	}
}
void break_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Break_Statement statement)
{
	Block block = cur_block;
	while (block != NULL)
	{
		if (block->block_type == FOR_BLOCK || block->block_type == WHILE_BLOCK
			|| block->block_type == DO_WHILE_BLOCK)
		{
			// find the loop block
			break;
		}
		else
		{
			block = block->outer;
		}
	}
	if (block == NULL)
	{
		LOG_ERROR("a break statement may only be used within a loop");
		LOG_EXIT();
	}
}
void continue_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Continue_Statement statement)
{
	Block block = cur_block;
	while (block != NULL)
	{
		if (block->block_type == FOR_BLOCK || block->block_type == WHILE_BLOCK
			|| block->block_type == DO_WHILE_BLOCK)
		{
			// find the loop block
			break;
		}
		else
		{
			block = block->outer;
		}
	}
	if (block == NULL)
	{
		LOG_ERROR("a continue statement may only be used within a loop");
		LOG_EXIT();
	}
}

int basic_type_equal(Basic_Type bt1, Basic_Type bt2)
{
	assert(bt1 != NULL && bt2 != NULL);
	return (bt1->kind == bt2->kind ? TRUE : FALSE);
}
int record_type_equal(Record_Type rt1, Record_Type rt2)
{
	assert(rt1 != NULL && rt2 != NULL);
	return (strcmp(rt1->record_name, rt2->record_name) == 0 ? TRUE : FALSE);
}

int type_equal(Type t1, Type t2)
{
	assert(t1 != NULL && t2 != NULL);
	if (t1->kind != t2->kind)
	{
		return FALSE;
	}
	if (t1->kind == BASIC_TYPE)
	{
		return basic_type_equal(t1->u.basic_type, t2->u.basic_type);
	}
	else if (t1->kind == RECORD_TYPE)
	{
		return record_type_equal(t1->u.record_type, t2->u.record_type);
	}
	else if (t1->kind == ARRAY_TYPE)
	{
		return type_equal(t1->u.array_type, t2->u.array_type);
	}
	return FALSE;
}

Basic_Type basic_type_copy(Basic_Type bt)
{
	assert(bt != NULL);
	Basic_Type ret = MEM_malloc(sizeof(*ret));
	ret->kind = bt->kind;
	return ret;
}

Record_Type record_type_copy(Record_Type rt)
{
	assert(rt != NULL);
	Record_Type ret = MEM_malloc(sizeof(*ret));
	ret->record_name = string_create(rt->record_name);
	return ret;
}

Type type_copy(Type type)
{
	assert(type != NULL);
	Type ret = MEM_malloc(sizeof(*ret));
	ret->kind = type->kind;
	switch (type->kind)
	{
		case BASIC_TYPE:
		{
			ret->u.basic_type = basic_type_copy(type->u.basic_type);
			break;
		}
		case RECORD_TYPE:
		{
			ret->u.record_type = record_type_copy(type->u.record_type);
			break;
		}
		case ARRAY_TYPE:
		{
			ret->u.array_type = type_copy(type->u.array_type);
			break;
		}
	}
	return ret;
}
Type basic_type_create(Basic_Type_Kind kind)
{
	Type type = MEM_malloc(sizeof(*type));
	type->kind = BASIC_TYPE;
	type->u.basic_type = MEM_malloc(sizeof(*(type->u.basic_type)));
	type->u.basic_type->kind = kind;
	return type;
}
Type record_type_create(char *record_name)
{
	assert(record_name != NULL);
	Type type = MEM_malloc(sizeof(*type));
	type->kind = RECORD_TYPE;
	type->u.record_type = MEM_malloc(sizeof(*(type->u.record_type)));
	type->u.record_type->record_name = string_create(record_name);
	return type;
}
int is_basic_type(Type type, Basic_Type_Kind kind)
{
	assert(type != NULL);
	if (type->kind != BASIC_TYPE)
	{
		return FALSE;
	}
	if (type->u.basic_type->kind == kind)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
int is_record_type(Type type)
{
	assert(type != NULL);
	if (type->kind != RECORD_TYPE)
	{
		return FALSE;
	}
	return TRUE;
}

int is_array_type(Type type)
{
	assert(type != NULL);
	if (type->kind != ARRAY_TYPE)
	{
		return FALSE;
	}
	return TRUE;
}
void expression_type_error(Expression exp)
{
	char *op = operator_information[exp->kind];
	switch (exp->kind)
	{
		case COMMA_EXPRESSION:
		{
			break;
		}
		case ASSIGN_EXPRESSION:
		{
			LOG_ERROR("[line: %d] the type of expression ", exp->line_number);
			expression_print(exp->u.assign_expression->left);
			LOG_ERROR(" before operator %s is ", op);
			type_print(exp->u.assign_expression->left->type);
			LOG_ERROR(", but the type of expression ");
			expression_print(exp->u.assign_expression->right);
			LOG_ERROR(" after operator %s is ", op);
			type_print(exp->u.assign_expression->right->type);
			LOG_ERROR(". They are not the same type.\n");
			break;
		}
		case OR_EXPRESSION:
		case AND_EXPRESSION:
		case EQ_EXPRESSION:
		case NE_EXPRESSION:
		case GT_EXPRESSION:
		case GE_EXPRESSION:
		case LT_EXPRESSION:
		case LE_EXPRESSION:
		case ADD_EXPRESSION:
		case SUB_EXPRESSION:
		case MUL_EXPRESSION:
		case DIV_EXPRESSION:
		case MOD_EXPRESSION:
		{
			LOG_ERROR("[line: %d] the type of expression ", exp->line_number);
			expression_print(exp->u.binary_expression->left);
			LOG_ERROR(" before operator %s is ", op);
			type_print(exp->u.binary_expression->left->type);
			LOG_ERROR(" but the type of expression");
			expression_print(exp->u.binary_expression->right);
			LOG_ERROR(" after operator %s is ", op);
			type_print(exp->u.binary_expression->right->type);
			LOG_ERROR(" , and they do not match the operator %s.\n", op);
			break;
		}
		case MINUS_EXPRESSION:
		{
			LOG_ERROR("the expression type for operator %s is ", op);
			type_print(exp->u.minus_expression->type);
			break;
		}
		case NOT_EXPRESSION:
		{
			LOG_ERROR("the expression type for operator %s is ", op);
			type_print(exp->u.not_expression->type);
			break;
		}
		case INC_EXPRESSION:
		case DEC_EXPRESSION:
		{
			LOG_ERROR("the expression type for operator %s is ", op);
			type_print(exp->u.inc_or_dec_expression->type);
			break;
		}
		case IDENTIFIER_EXPRESSION:
		{
			break;
		}
		case FUNCTION_CALL_EXPRESSION:
		{
			break;
		}
		case FIELD_ACCESS_EXPRESSION:
		{
			break;
		}
		case ARRAY_INDEX_EXPRESSION:
		{
			break;
		}
		case ARRAY_LITERAL_EXPRESSION:
		{
			break;
		}
		case NEW_EXPRESSION:
		{
			break;
		}
		case NIL_EXPRESSION:
		{
			break;
		}
		case BOOL_EXPRESSION:
		{
			break;
		}
		case INT_EXPRESSION:
		{
			break;
		}
		case DOUBLE_EXPRESSION:
		{
			break;
		}
		case STRING_EXPRESSION:
		{
			break;
		}
		case PARENTHESIS_EXPRESSION:
		{
			break;
		}
	}
	LOG_ERROR("expression type error");
	LOG_EXIT();
}
void binary_expression_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Expression exp)
{
	expression_semantic_analysis(cur_block, symbol_definition, exp->u.binary_expression->left);
	expression_semantic_analysis(cur_block, symbol_definition, exp->u.binary_expression->right);
	switch (exp->kind)
	{
		case OR_EXPRESSION:
		case AND_EXPRESSION:
		{
			if (type_equal(exp->u.binary_expression->left->type, exp->u.binary_expression->right->type)
				== TRUE
				&& is_basic_type(exp->u.binary_expression->left->type, BOOL_TYPE) == TRUE)
			{
				exp->type = type_copy(exp->u.binary_expression->left->type);
			}
			else
			{
				expression_type_error(exp);
			}
			break;
		}
		case EQ_EXPRESSION:
		case NE_EXPRESSION:
		{
			if (type_equal(exp->u.binary_expression->left->type, exp->u.binary_expression->right->type)
				== TRUE
				&& (
				/* 
				is_basic_type(exp->u.binary_expression->left->type, NIL_TYPE) == TRUE
				|| 
				*/
				is_basic_type(exp->u.binary_expression->left->type, BOOL_TYPE) == TRUE
				|| is_basic_type(exp->u.binary_expression->left->type, INT_TYPE) == TRUE
				|| is_basic_type(exp->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE
				|| is_basic_type(exp->u.binary_expression->left->type, STRING_TYPE) == TRUE)
				)
			{
				exp->type = basic_type_create(BOOL_TYPE);
			}
			else
			{
				expression_type_error(exp);
			}
			break;
		}
		case GT_EXPRESSION:
		case GE_EXPRESSION:
		case LT_EXPRESSION:
		case LE_EXPRESSION:
		{
			if (type_equal(exp->u.binary_expression->left->type, exp->u.binary_expression->right->type)
				== TRUE
				&& (is_basic_type(exp->u.binary_expression->left->type, INT_TYPE) == TRUE
				|| is_basic_type(exp->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE
				|| is_basic_type(exp->u.binary_expression->left->type, STRING_TYPE) == TRUE))
			{
				exp->type = basic_type_create(BOOL_TYPE);
			}
			else
			{
				expression_type_error(exp);
			}
			break;
		}
		case ADD_EXPRESSION:
		{
			if (type_equal(exp->u.binary_expression->left->type, exp->u.binary_expression->right->type)
				== TRUE
				&& (is_basic_type(exp->u.binary_expression->left->type, INT_TYPE) == TRUE
				|| is_basic_type(exp->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE
				|| is_basic_type(exp->u.binary_expression->left->type, STRING_TYPE) == TRUE))
			{
				exp->type = exp->u.binary_expression->left->type;
			}
			else
			{
				expression_type_error(exp);
			}
			break;
		}
		case SUB_EXPRESSION:
		case MUL_EXPRESSION:
		case DIV_EXPRESSION:
		{
			if (type_equal(exp->u.binary_expression->left->type, exp->u.binary_expression->right->type)
				== TRUE
				&& (is_basic_type(exp->u.binary_expression->left->type, INT_TYPE) == TRUE
				|| is_basic_type(exp->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE))
			{
				exp->type = exp->u.binary_expression->left->type;
			}
			else
			{
				expression_type_error(exp);
			}
			break;
		}
		case MOD_EXPRESSION:
		{
			if (type_equal(exp->u.binary_expression->left->type, exp->u.binary_expression->right->type)
				== TRUE
				&& is_basic_type(exp->u.binary_expression->left->type, INT_TYPE) == TRUE)
			{
				exp->type = exp->u.binary_expression->left->type;
			}
			else
			{
				expression_type_error(exp);
			}
			break;
		}
	}
}
void invalid_symbol(int line_number, char *symbol_name)
{
	LOG_ERROR("[line: %d]cannot find the definition of symbol name \"%s\"\n", line_number, symbol_name);
	LOG_EXIT();
}
void expression_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,	Expression exp)
{
	if (exp == NULL)
	{
		LOG_ERROR("the expression is NULL");
		LOG_EXIT();
	}
	//LOG_INFO("line: %d\n", exp->line_number);
	if (cur_block != NULL)
	{
		// function block or local block
		switch (exp->kind)
		{
			case COMMA_EXPRESSION:
			{
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.comma_expression->left);
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.comma_expression->right);
				exp->type = type_copy(exp->u.comma_expression->right->type);
				break;
			}
			case ASSIGN_EXPRESSION:
			{
				if (!(exp->u.assign_expression->left->kind == IDENTIFIER_EXPRESSION
					|| exp->u.assign_expression->left->kind == FIELD_ACCESS_EXPRESSION
					|| exp->u.assign_expression->left->kind == ARRAY_INDEX_EXPRESSION))
				{
					LOG_ERROR("[line %d] ", exp->u.assign_expression->left->line_number);
					expression_print(exp->u.assign_expression->left);
					LOG_ERROR(": left operand must be l-value");
					LOG_EXIT();
				}
				expression_semantic_analysis(cur_block, symbol_definition, 
					exp->u.assign_expression->left);
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.assign_expression->right);
				
				if (type_equal(exp->u.assign_expression->left->type, exp->u.assign_expression->right->type)
					== TRUE)
				{
					exp->type = type_copy(exp->u.assign_expression->right->type);
				}
				else
				{
					expression_type_error(exp);
				}
				break;
			}
			case OR_EXPRESSION:
			case AND_EXPRESSION:
			case EQ_EXPRESSION:
			case NE_EXPRESSION:
			case GT_EXPRESSION:
			case GE_EXPRESSION:
			case LT_EXPRESSION:
			case LE_EXPRESSION:
			case ADD_EXPRESSION:
			case SUB_EXPRESSION:
			case MUL_EXPRESSION:
			case DIV_EXPRESSION:
			case MOD_EXPRESSION:
			{
				binary_expression_semantic_analysis(cur_block, symbol_definition, exp);
				break;
			}
			case MINUS_EXPRESSION:
			{
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.minus_expression);
				if (is_basic_type(exp->u.minus_expression->type, INT_TYPE) == TRUE
					|| is_basic_type(exp->u.minus_expression->type, DOUBLE_TYPE) == TRUE)
				{
					exp->type = type_copy(exp->u.minus_expression->type);
				}
				else
				{
					expression_type_error(exp);
				}
				break;
			}
			case NOT_EXPRESSION:
			{
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.not_expression);
				if (is_basic_type(exp->u.not_expression->type, BOOL_TYPE) == TRUE)
				{
					exp->type = type_copy(exp->u.not_expression->type);
				}
				else
				{
					expression_type_error(exp);
				}
				break;
			}
			case INC_EXPRESSION:
			case DEC_EXPRESSION:
			{
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.inc_or_dec_expression);
				if (is_basic_type(exp->u.inc_or_dec_expression->type, INT_TYPE) == TRUE)
				{
					exp->type = type_copy(exp->u.inc_or_dec_expression->type);
				}
				else
				{
					expression_type_error(exp);
				}
				break;
			}
			case IDENTIFIER_EXPRESSION:
			{
				Symbol_Name identifier = symbol_name_create(exp->u.identifier_expression->name);
				Symbol_Information si = NULL;
				Symbol_Table st = cur_block->block_table;
				while (st != NULL)
				{
					si = map_search(st->table, identifier);
					if (si != NULL)
					{
						exp->type = type_copy(si->type);
						symbol_name_free(identifier);
						break;
					}
					st = st->parent;
				}
				if (st == NULL)
				{
					invalid_symbol(exp->line_number, exp->u.identifier_expression->name);
				}
				break;
			}
			case FUNCTION_CALL_EXPRESSION:
			{
				// first find the function prototype
				Symbol_Name function_name = symbol_name_create(exp->u.function_call_expression->function_name);
				Function_Prototype function_prototype = map_search(symbol_definition->function_prototype, function_name);
				symbol_name_free(function_name);
				if (function_prototype == NULL)
				{
					invalid_symbol(exp->line_number, exp->u.function_call_expression->function_name);
				}
				// second check the argument 
				int argument_number = 0;
				Argument_List argument_list = exp->u.function_call_expression->argument_list;
				while (argument_list != NULL)
				{
					argument_number++;
					argument_list = argument_list->next;
				}
				if (argument_number != function_prototype->parameter_number)
				{
					LOG_ERROR("%d:the number of argument(s) of function %s is %u, but the function needs %u argument(s)",
						exp->line_number, exp->u.function_call_expression->function_name, 
						argument_number, function_prototype->parameter_number);
					LOG_EXIT();
				}
				argument_list = exp->u.function_call_expression->argument_list;
				for (int idx = 0; idx < function_prototype->parameter_number; idx++)
				{
					expression_semantic_analysis(cur_block, symbol_definition, argument_list->expression);
					if (type_equal(argument_list->expression->type, function_prototype->parameter_list[idx])
						== FALSE)
					{
						LOG_ERROR("the type of %u-th argument is ", idx);
						type_print(argument_list->expression->type);
						LOG_ERROR(", but the function \"");
						function_prototype_print(function_prototype);
						LOG_ERROR("\" needs type ");
						type_print(function_prototype->parameter_list[idx]);
						LOG_ERROR("");
						LOG_EXIT();
					}
					else
					{
						argument_list = argument_list->next;
					}
				}
				exp->type = type_copy(function_prototype->retval_type);
				break;
			}
			case FIELD_ACCESS_EXPRESSION:
			{
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.field_access_expression->record_name);
				Type record_type = exp->u.field_access_expression->record_name->type;
				if (is_record_type(record_type) == FALSE)
				{
					LOG_ERROR("the type of ");
					expression_print(exp->u.field_access_expression->record_name);
					LOG_ERROR(" is ");
					type_print(record_type);
					LOG_ERROR(", but the record type is needed");
					LOG_EXIT();
				}
				Symbol_Name record_name = symbol_name_create(record_type->u.record_type->record_name);
				Symbol_Table record_definition = map_search(symbol_definition->record_definition, record_name);
				symbol_name_free(record_name);
				if (record_definition == NULL)
				{
					invalid_symbol(exp->u.field_access_expression->record_name->line_number,
						record_type->u.record_type->record_name);
				}
				Symbol_Name field_name = symbol_name_create(exp->u.field_access_expression->field_name->u.identifier_expression->name);
				Symbol_Information field_information = map_search(record_definition->table, field_name);
				symbol_name_free(field_name);
				if (field_information == NULL)
				{
					invalid_symbol(exp->u.field_access_expression->field_name->line_number,
						exp->u.field_access_expression->field_name->u.identifier_expression->name);
				}
				exp->u.field_access_expression->field_name->type = type_copy(field_information->type);
				exp->type = type_copy(exp->u.field_access_expression->field_name->type);
				break;
			}
			case ARRAY_INDEX_EXPRESSION:
			{
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.array_index_expression->array_name);
				expression_semantic_analysis(cur_block, symbol_definition,
					exp->u.array_index_expression->index);
				Type array_name_type = exp->u.array_index_expression->array_name->type;
				Type index_type = exp->u.array_index_expression->index->type;
				if (is_array_type(array_name_type) == FALSE)
				{
					LOG_ERROR("the type of ");
					expression_print(exp->u.array_index_expression->array_name);
					LOG_ERROR(" is ");
					type_print(array_name_type);
					LOG_ERROR(", but the array type is needed");
					LOG_EXIT();
				}
				if (is_basic_type(index_type, INT_TYPE) == FALSE)
				{
					LOG_ERROR("the type of ");
					expression_print(exp->u.array_index_expression->index);
					LOG_ERROR(" is ");
					type_print(index_type);
					LOG_ERROR(", but the int type is needed");
					LOG_EXIT();
				}
				exp->type = type_copy(array_name_type->u.array_type);
				break;
			}
			case ARRAY_LITERAL_EXPRESSION:
			{
				LOG_ERROR("no implementions");
				LOG_EXIT();
				break;
			}
			case NEW_EXPRESSION:
			{
				exp->u.new_expression;
				int dimension = 0;
				Dimension_List dimension_list = exp->u.new_expression->dimension_list;
				while (dimension_list != NULL)
				{
					expression_semantic_analysis(cur_block, symbol_definition,
						dimension_list->dimension_expression);
					Type dimension_expression_type = dimension_list->dimension_expression->type;
					if (is_basic_type(dimension_expression_type, INT_TYPE) == FALSE)
					{
						LOG_ERROR("the type of ");
						expression_print(dimension_list->dimension_expression);
						LOG_ERROR(" is ");
						type_print(dimension_expression_type);
						LOG_ERROR(", but the int type is needed");
						LOG_EXIT();
					}
					dimension++;
					dimension_list = dimension_list->next_dimension;
				}
				Type type = NULL;
				if (exp->u.new_expression->type_kind == BASIC_TYPE)
				{
					type = basic_type_create(exp->u.new_expression->u.basic_type->kind);
					if (dimension == 0)
					{
						LOG_ERROR("cannot new basic type");
						LOG_EXIT();
					}
				}
				else if (exp->u.new_expression->type_kind == RECORD_TYPE)
				{
					type = record_type_create(exp->u.new_expression->u.record_type->record_name);
				}
				while (dimension-- > 0)
				{
					Type temp = MEM_malloc(sizeof(*temp));
					temp->kind = ARRAY_TYPE;
					temp->u.array_type = type;
					type = temp;
				}
				exp->type = type;
				break;
			}
			case NIL_EXPRESSION:
			{
				exp->type = basic_type_create(NIL_TYPE);
				break;
			}
			case BOOL_EXPRESSION:
			{
				exp->type = basic_type_create(BOOL_TYPE);
				break;
			}
			case INT_EXPRESSION:
			{
				exp->type = basic_type_create(INT_TYPE);
				break;
			}
			case DOUBLE_EXPRESSION:
			{
				exp->type = basic_type_create(DOUBLE_TYPE);
				break;
			}
			case STRING_EXPRESSION:
			{
				exp->type = basic_type_create(STRING_TYPE);
				break;
			}
			case PARENTHESIS_EXPRESSION:
			{
				expression_semantic_analysis(cur_block, symbol_definition, exp->u.parenthesis_expression);
				exp->type = type_copy(exp->u.parenthesis_expression->type);
				break;
			}
		}
	}
	else
	{
		// global variable initialization
		switch (exp->kind)
		{
			case NIL_EXPRESSION:
			{
				exp->type = basic_type_create(NIL_TYPE);
				break;
			}
			case BOOL_EXPRESSION:
			{
				exp->type = basic_type_create(BOOL_TYPE);
				break;
			}
			case INT_EXPRESSION:
			{
				exp->type = basic_type_create(INT_TYPE);
				break;
			}
			case DOUBLE_EXPRESSION:
			{
				exp->type = basic_type_create(DOUBLE_TYPE);
				break;
			}
			case STRING_EXPRESSION:
			{
				exp->type = basic_type_create(STRING_TYPE);
				break;
			}
			default:
			{
				LOG_ERROR("[line: %d] ", exp->line_number);
				expression_print(exp);
				LOG_ERROR(", global variable initialization must be literal of basic type");
				LOG_EXIT();
			}
		}
	}
}

int type_check(Type type, Map record_definition)
{
	if (type->kind == BASIC_TYPE)
	{
		return TRUE;
	}
	else if (type->kind == RECORD_TYPE)
	{
		Symbol_Name record_name = symbol_name_create(type->u.record_type->record_name);
		Symbol_Information si = map_search(record_definition, record_name);
		symbol_name_free(record_name);
		if (si == NULL)
		{
			return FALSE;
		}
		return TRUE;
	}
	else if (type->kind == ARRAY_TYPE)
	{
		return type_check(type->u.array_type, record_definition);
	}
	LOG_ERROR("unknown type");
	LOG_EXIT();
	return FALSE;
}
void declaration_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Declaration_Statement statement)
{
	if (type_check(statement->type, symbol_definition->record_definition) == FALSE)
	{
		LOG_ERROR("[line %u]the type ", statement->line_number);
		type_print(statement->type);
		LOG_ERROR(" in declaration statement ");
		declaration_statement_print(0, statement);
		LOG_ERROR(" is unknown");
		LOG_EXIT();
	}
	Declaration_Variable_List variable_list = statement->variable_list;
	while (variable_list != NULL)
	{
		Symbol_Name variable_name = symbol_name_create(variable_list->variable_name);
		if (variable_list->initialization != NULL)
		{
			expression_semantic_analysis(cur_block, symbol_definition, 
				variable_list->initialization);
			Type initialization_type = variable_list->initialization->type;
			if (!(type_equal(statement->type, initialization_type) == TRUE
				|| ((is_record_type(statement->type) || is_array_type(statement->type)) 
				&& is_basic_type(initialization_type, NIL_TYPE))))
			{
				LOG_ERROR("[line %d] the type of variable %s ", statement->line_number, 
					variable_list->variable_name);
				type_print(statement->type);
				LOG_ERROR(", but the type of initialization expression ");
				expression_print(variable_list->initialization);
				LOG_ERROR(" is ");
				type_print(variable_list->initialization->type);
				LOG_ERROR(". They are not the same type.\n");
				LOG_EXIT();
			}
		}
		Type variable_type = statement->type;
		Symbol_Information variable_information = NULL;
		if (cur_block == NULL)
		{
			// global variable
			variable_information = map_search(symbol_definition->global_variable->table, variable_name);
		}
		else
		{
			// function or record or local block
			variable_information = map_search(cur_block->block_table->table, variable_name);
		}
		if (variable_information != NULL)
		{
			LOG_ERROR("variable %s redefined", variable_name);
			LOG_EXIT();
		}
		if (cur_block == NULL)
		{
			variable_information = symbol_information_create(variable_type,
				symbol_definition->global_variable->start_offset + symbol_definition->global_variable->symbol_number);
			symbol_definition->global_variable->symbol_number++;
			map_insert(symbol_definition->global_variable->table, variable_name, variable_information);
		}
		else
		{
			variable_information = symbol_information_create(variable_type,
				cur_block->block_table->start_offset + cur_block->block_table->symbol_number);
			cur_block->block_table->symbol_number++;
			map_insert(cur_block->block_table->table, variable_name, variable_information);
		}		
		variable_list = variable_list->next;
	}
}


void semantic_print(Translation_Unit tu)
{
	if (tu == NULL)
	{
		return;
	}
	symbol_table_print(tu->symbol_definition->global_variable);
	Definition_List definition_list = tu->definition_list;
	while (definition_list != NULL)
	{
		if (definition_list->kind == RECORD_DEFINITION)
		{
			block_semantic_print(definition_list->u.record_defintion->field_block);
		}
		else if (definition_list->kind == FUNCTION_DEFINITION)
		{
			if (definition_list->u.function_definition->function_kind == USER_DEFINIED_FUNCTION)
			{
				block_semantic_print(definition_list->u.function_definition->u.user_defined_function);
			}
		}
		definition_list = definition_list->next;
	}
}
void block_semantic_print(Block block)
{
	Statement_List statement_list = block->statement_list;
	while (statement_list != NULL)
	{
		Statement stm = statement_list->statement;
		switch (stm->kind)
		{
			case EXPRESSION_STATEMENT:
			{
				break;
			}
			case IF_STATEMENT:
			{
				If_Statement if_statement = stm->u.if_statement;
				while (TRUE)
				{
					block_semantic_print(if_statement->then_block);
					if (if_statement->kind == IF_WITHOUT_ELSE)
					{
						break;
					}
					else if (if_statement->kind == IF_WITH_ELSE)
					{
						block_semantic_print(if_statement->u.else_block);
						break;
					}
					else if (if_statement->kind == IF_WITH_ELSE_IF)
					{
						if_statement = if_statement->u.else_if_statement->u.if_statement;
					}
				}
				break;
			}
			case DO_WHILE_STATEMENT:
			{
				block_semantic_print(stm->u.do_while_statement->block);
				break;
			}
			case WHILE_STATEMENT:
			{
				block_semantic_print(stm->u.while_statement->block);
				break;
			}
			case FOR_STATEMENT:
			{
				block_semantic_print(stm->u.for_statement->block);
				break;
			}
			case RETURN_STATEMENT:
			{
				break;
			}
			case BREAK_STATEMENT:
			{
				break;
			}
			case CONTINUE_STATEMENT:
			{
				break;
			}
			case DECLARATION_STATEMENT:
			{
				break;
			}
			case BLOCK:
			{
				block_semantic_print(stm->u.block);
				break;
			}
		}
		statement_list = statement_list->next;
	}
	symbol_table_print(block->block_table);
}