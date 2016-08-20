#include "environment.h"
#include "ABSYNT.h"
#include "absynt_create.h"
#include "LOG.h"
#include "LEX.h"
#include "memory.h"
#include "tool.h"


static Token expect_token(int token_kind)
{
	Token token = LEX_get_token();
	if (token->token_kind != token_kind)
	{
		Token before = LEX_get_token_before_token(token);
		if (before == NULL)
		{
			LOG_ERROR("[line %d] expect token %s", token->line_number, 
				LEX_get_token_kind_string(token_kind));
			LOG_EXIT();
		}
		else
		{
			LOG_ERROR("[line %d] token %s should be followed by token %s", before->line_number, 
				LEX_get_token_kind_string(before->token_kind), 
				LEX_get_token_kind_string(token_kind));
			LOG_EXIT();
		}
	}
	return token;
}

static void invalid_token(Token token)
{
	Token before = LEX_get_token_before_token(token);
	if (before == NULL)
	{
		LOG_ERROR("[line %d] invalid token %s", token->line_number,
			LEX_get_token_kind_string(token->token_kind));
		LOG_EXIT();
	}
	else
	{
		LOG_ERROR("[line %d] token %s should not be followed by token %s", before->line_number,
			LEX_get_token_kind_string(before->token_kind),
			LEX_get_token_kind_string(token->token_kind));
		LOG_EXIT();
	}
}

Translation_Unit translation_unit_parse()
{
	Translation_Unit translation_unit = MEM_malloc(sizeof(*translation_unit));
	translation_unit->import_list = import_list_parse();
	translation_unit->definition_list = definition_list_parse();
	return translation_unit;
}

Import_List import_list_parse()
{
	Token token = LEX_get_token();
	if (token->token_kind != IMPORT)
	{
		LEX_unget_token(token);
		return NULL;
	}
	Import_List import_list = MEM_malloc(sizeof(*import_list));
	import_list->line_number = token->line_number;
	token = expect_token(IDENTIFIER);
	import_list->package_name = string_create(token->buff);
	token = expect_token(SEMICOLON);
	import_list->next = import_list_parse();
	return import_list;
}


Definition_List definition_list_parse()
{
	Definition_List definition_list = NULL;
	Token token = LEX_get_token();
	if (token->token_kind == END_OF_FILE)
	{
		return NULL;
	}
	definition_list = MEM_malloc(sizeof(*definition_list));
	if (token->token_kind == RECORD)
	{
		LEX_unget_token(token);
		definition_list->kind = RECORD_DEFINITION;
		definition_list->u.record_defintion = record_definition_parse();
	}
	else if (token->token_kind == FUNCTION)
	{
		LEX_unget_token(token);
		definition_list->kind = FUNCTION_DEFINITION;
		definition_list->u.function_definition = function_definition_parse();
	}
	else if (token->token_kind == VARIABLE)
	{
		LEX_unget_token(token);
		definition_list->kind = VARIABLE_DEFINITION;
		definition_list->u.variable_definition = statement_parse();
	}
	else
	{
		invalid_token(token);
	}
	definition_list->next = definition_list_parse();
	return definition_list;
}

int bracket_list_parse()
{
	Token token = LEX_get_token();
	int dim = 0;
	while (token->token_kind == LB)
	{
		token = expect_token(RB);
		dim++;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return dim;
}

Type type_parse()
{
	Type type = MEM_malloc(sizeof(*type));
	Basic_Type basic_type = NULL;
	Record_Type record_type = NULL;
	Token token = LEX_get_token();
	if (token->token_kind == VOID || token->token_kind == BOOL
		|| token->token_kind == INT || token->token_kind == DOUBLE
		|| token->token_kind == STRING)
	{
		basic_type = MEM_malloc(sizeof(*basic_type));
		switch (token->token_kind)
		{
			case VOID:
				basic_type->kind = VOID_TYPE;
				break;
			case BOOL:
				basic_type->kind = BOOL_TYPE;
				break;
			case INT:
				basic_type->kind = INT_TYPE;
				break;
			case DOUBLE:
				basic_type->kind = DOUBLE_TYPE;
				break;
			case STRING:
				basic_type->kind = STRING_TYPE;
				break;
		}

	}
	else if (token->token_kind == IDENTIFIER)
	{
		record_type = MEM_malloc(sizeof(*record_type));
		record_type->record_name = string_create(token->buff);
	}
	else
	{
		//LOG_ERROR("%d: expect basic type or record name", token->line_number);
		invalid_token(token);
	}
	assert((basic_type != NULL && record_type == NULL || basic_type == NULL && record_type != NULL));
	int dim = 0;
	dim = bracket_list_parse();
	if (dim == 0)
	{
		if (basic_type != NULL)
		{
			type->kind = BASIC_TYPE;
			type->u.basic_type = basic_type;
		}
		else
		{
			type->kind = RECORD_TYPE;
			type->u.record_type = record_type;
		}
	}
	else
	{
		type->kind = ARRAY_TYPE;
		Type *iterator = &(type->u.array_type);
		while (dim-- > 0)
		{
			Type array_type = MEM_malloc(sizeof(*array_type));
			if (dim > 0)
			{
				array_type->kind = ARRAY_TYPE;
				array_type->u.array_type = NULL;
				*iterator = array_type;
				iterator = &(array_type->u.array_type);
			}
			else
			{
				if (basic_type != NULL)
				{
					array_type->kind = BASIC_TYPE;
					array_type->u.basic_type = basic_type;
				}
				else
				{
					array_type->kind = RECORD_TYPE;
					array_type->u.record_type = record_type;
				}
				*iterator = array_type;
				break;
			}
		}
	}
	return type;
}

Record_Definition record_definition_parse()
{
	Token token = expect_token(RECORD);
	Record_Definition record_definition = MEM_malloc(sizeof(*record_definition));
	record_definition->line_number = token->line_number;
	token = expect_token(IDENTIFIER);
	record_definition->record_name = string_create(token->buff);
	record_definition->field_block = block_parse();
	token = expect_token(SEMICOLON);
	return record_definition;
}

Function_Definition function_definition_parse()
{
	Token token = expect_token(FUNCTION);
	Function_Definition function_definition = MEM_malloc(sizeof(*function_definition));
	function_definition->line_number = token->line_number;
	token = LEX_get_token();
	if (token->token_kind == STATIC)
	{
		function_definition->is_static = TRUE;
	}
	else
	{
		LEX_unget_token(token);
		function_definition->is_static = FALSE;
	}
	function_definition->type = type_parse();
	token = expect_token(IDENTIFIER);
	function_definition->function_name = string_create(token->buff);
	expect_token(LP);
	token = LEX_get_token();
	if (token->token_kind != RP)
	{
		LEX_unget_token(token);
		function_definition->parameter_list = parameter_list_parse();
		expect_token(RP);
	}
	else
	{
		function_definition->parameter_list = NULL;
	}
	token = LEX_get_token();
	if (token->token_kind == SEMICOLON)
	{
		function_definition->function_kind = NAIVE_FUNCTION;
		//function_definition->u.naive_function = NULL;
	}
	else
	{
		LEX_unget_token(token);
		function_definition->function_kind = USER_DEFINIED_FUNCTION;
		function_definition->u.user_defined_function = block_parse();
	}
	return function_definition;
}

Parameter_List parameter_list_parse()
{
	Token token = expect_token(VARIABLE);
	Parameter_List parameter_list = MEM_malloc(sizeof(*parameter_list));
	parameter_list->line_number = token->line_number;
	parameter_list->next = NULL;
	parameter_list->type = type_parse();
	token = expect_token(IDENTIFIER);
	parameter_list->parameter_name = string_create(token->buff);
	token = LEX_get_token();
	if (token->token_kind == COMMA)
	{
		parameter_list->next = parameter_list_parse();
	}
	else
	{
		LEX_unget_token(token);
	}	
	return parameter_list;
}

Expression expression_create(Expression_Kind exp_kind)
{
	Expression exp = MEM_malloc(sizeof(*exp));
	exp->kind = exp_kind;
	exp->type = NULL;
	switch (exp->kind)
	{
		case COMMA_EXPRESSION:
		{
			exp->u.comma_expression = MEM_malloc(sizeof(*(exp->u.comma_expression)));
			exp->u.comma_expression->left = exp->u.comma_expression->right = NULL;
			break;
		}
		case ASSIGN_EXPRESSION:
		{
			exp->u.assign_expression = MEM_malloc(sizeof(*(exp->u.assign_expression)));
			exp->u.assign_expression->left = exp->u.assign_expression->right = NULL;
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
			exp->u.binary_expression = MEM_malloc(sizeof(*(exp->u.binary_expression)));
			exp->u.binary_expression->left = exp->u.binary_expression->right = NULL;
			break;
		}
		case MINUS_EXPRESSION:
		{
			exp->u.minus_expression = NULL;
			break;
		}
		case NOT_EXPRESSION:
		{
			exp->u.not_expression = NULL;
			break;
		}
		case INC_EXPRESSION:
		case DEC_EXPRESSION:
		{
			exp->u.inc_or_dec_expression = NULL;
			break;
		}
		case NEW_EXPRESSION:
		{
			exp->u.new_expression = MEM_malloc(sizeof(*(exp->u.new_expression)));
			exp->u.new_expression->u.basic_type = NULL;
			exp->u.new_expression->u.record_type = NULL;
			exp->u.new_expression->dimension_list = NULL;
			break;
		}
		case INT_EXPRESSION:
		{
			exp->u.int_value = 0;
			break;
		}
		case DOUBLE_EXPRESSION:
		{
			exp->u.double_value = 0.0;
			break;
		}
		case STRING_EXPRESSION:
		{
			exp->u.string_value = NULL;
			break;
		}
		case BOOL_EXPRESSION:
		{
			exp->u.bool_value = FALSE;
			break;
		}
		case NIL_EXPRESSION:
		{
			// do nothing
			break;
		}
		case IDENTIFIER_EXPRESSION:
		{
			exp->u.identifier_expression = MEM_malloc(sizeof(*(exp->u.identifier_expression)));
			exp->u.identifier_expression->name = NULL;
			break;
		}
		case FUNCTION_CALL_EXPRESSION:
		{
			exp->u.function_call_expression = MEM_malloc(sizeof(*(exp->u.function_call_expression)));
			exp->u.function_call_expression->function_name = NULL;
			exp->u.function_call_expression->argument_list = NULL;
			break;
		}
		case FIELD_ACCESS_EXPRESSION:
		{
			exp->u.field_access_expression = MEM_malloc(sizeof(*(exp->u.field_access_expression)));
			exp->u.field_access_expression->record_name = NULL;
			exp->u.field_access_expression->field_name = NULL;
			break;
		}
		case ARRAY_INDEX_EXPRESSION:
		{
			exp->u.array_index_expression = MEM_malloc(sizeof(*(exp->u.array_index_expression)));
			exp->u.array_index_expression->array_name = NULL;
			exp->u.array_index_expression->index = NULL;
			break;
		}
		case PARENTHESIS_EXPRESSION:
		{
			exp->u.parenthesis_expression = NULL;
			break;
		}
		default:
		{			
			LOG_ERROR("unknown expression kind");
			LOG_EXIT();
		}
	}
	return exp;
}


Expression expression_parse()
{
	Expression exp = assignment_expression_parse();
	Token token = LEX_get_token();
	while (token->token_kind == COMMA)
	{
		Expression next_exp = assignment_expression_parse();
		Expression temp_exp = expression_create(COMMA_EXPRESSION);
		temp_exp->u.comma_expression->left = exp;
		temp_exp->u.comma_expression->right = next_exp;
		temp_exp->line_number = exp->line_number;
		exp = temp_exp;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return exp;
}

Expression assignment_expression_parse()
{
	// this function is different from other expression parse functions
	Expression exp = logical_or_expression_parse();
	Token token = LEX_get_token();
	if (token->token_kind == ASSIGN)
	{
		Expression assign_exp = expression_create(ASSIGN_EXPRESSION);
		assign_exp->u.binary_expression->left = exp;
		assign_exp->u.binary_expression->right = assignment_expression_parse();
		assign_exp->line_number = exp->line_number;
		exp = assign_exp;
	}
	else
	{
		LEX_unget_token(token);
	}
	return exp;
	//
	/*
	Expression exp = logical_or_expression_parse();
	Token token = LEX_get_token();
	while (token->token_kind == ASSIGN)
	{
		Expression next_exp = logical_or_expression_parse();
		Expression temp_exp = expression_create(ASSIGN_EXPRESSION);
		temp_exp->u.assign_expression->left = exp;
		temp_exp->u.assign_expression->right = next_exp;
		temp_exp->line_number = exp->line_number;
		exp = temp_exp;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return exp;
	*/
}

Expression logical_or_expression_parse()
{
	Expression exp = logical_and_expression_parse();
	Token token = LEX_get_token();
	while (token->token_kind == OR)
	{
		Expression next_exp = logical_and_expression_parse();
		Expression temp_exp = expression_create(OR_EXPRESSION);
		temp_exp->u.binary_expression->left = exp;
		temp_exp->u.binary_expression->right = next_exp;
		temp_exp->line_number = exp->line_number;
		exp = temp_exp;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return exp;

}

Expression logical_and_expression_parse()
{
	Expression exp = equality_expression_parse();
	Token token = LEX_get_token();
	while (token->token_kind == AND)
	{
		Expression next_exp = equality_expression_parse();
		Expression temp_exp = expression_create(AND_EXPRESSION);
		temp_exp->u.binary_expression->left = exp;
		temp_exp->u.binary_expression->right = next_exp;
		temp_exp->line_number = exp->line_number;
		exp = temp_exp;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return exp;
}

Expression equality_expression_parse()
{
	Expression exp = relational_expression_parse();
	Token token = LEX_get_token();
	while (token->token_kind == EQ || token->token_kind == NE)
	{
		Expression next_exp = relational_expression_parse();
		Expression temp_exp = expression_create(token->token_kind == EQ ? EQ_EXPRESSION : NE_EXPRESSION);
		temp_exp->u.binary_expression->left = exp;
		temp_exp->u.binary_expression->right = next_exp;
		temp_exp->line_number = exp->line_number;
		exp = temp_exp;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return exp;
}

Expression relational_expression_parse()
{
	Expression exp = additive_expression_parse();
	Token token = LEX_get_token();
	while (token->token_kind == GT || token->token_kind == GE
		|| token->token_kind == LT || token->token_kind == LE)
	{
		Expression next_exp = additive_expression_parse();
		Expression temp_exp = NULL;
		switch (token->token_kind)
		{
			case GT:
				temp_exp = expression_create(GT_EXPRESSION);
				break;
			case GE:
				temp_exp = expression_create(GE_EXPRESSION);
				break;
			case LT:
				temp_exp = expression_create(LT_EXPRESSION);
				break;
			case LE:
				temp_exp = expression_create(LE_EXPRESSION);
				break;
		}
		temp_exp->u.binary_expression->left = exp;
		temp_exp->u.binary_expression->right = next_exp;
		temp_exp->line_number = exp->line_number;
		exp = temp_exp;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return exp;
}

Expression additive_expression_parse()
{
	Expression exp = multiplicative_expression_parse();
	Token token = LEX_get_token();
	while (token->token_kind == ADD || token->token_kind == SUB)
	{
		Expression next_exp = multiplicative_expression_parse();
		Expression temp_exp = expression_create(token->token_kind == ADD ? ADD_EXPRESSION : SUB_EXPRESSION);
		temp_exp->u.binary_expression->left = exp;
		temp_exp->u.binary_expression->right = next_exp;
		temp_exp->line_number = exp->line_number;
		exp = temp_exp;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return exp;
}

Expression multiplicative_expression_parse()
{
	Expression exp = unary_expression_parse();
	Token token = LEX_get_token();
	while (token->token_kind == MUL || token->token_kind == DIV || token->token_kind == MOD)
	{
		Expression next_exp = unary_expression_parse();
		Expression temp_exp = NULL;
		switch (token->token_kind)
		{
			case MUL:
				temp_exp = expression_create(MUL_EXPRESSION);
				break;
			case DIV:
				temp_exp = expression_create(DIV_EXPRESSION);
				break;
			case MOD:
				temp_exp = expression_create(MOD_EXPRESSION);
				break;
		}
		temp_exp->u.binary_expression->left = exp;
		temp_exp->u.binary_expression->right = next_exp;
		temp_exp->line_number = exp->line_number;
		exp = temp_exp;
		token = LEX_get_token();
	}
	LEX_unget_token(token);
	return exp;
}

Expression unary_expression_parse()
{
	Expression exp = NULL;
	Token token = LEX_get_token();
	if (token->token_kind == SUB)
	{
		exp = expression_create(MINUS_EXPRESSION);
		exp->u.minus_expression = postfix_expression_parse();
		exp->line_number = exp->u.minus_expression->line_number;
	}
	else if (token->token_kind == EXCLAMATION)
	{
		exp = expression_create(NOT_EXPRESSION);
		exp->u.not_expression = postfix_expression_parse();
		exp->line_number = exp->u.not_expression->line_number;
	}
	else
	{
		LEX_unget_token(token);
		exp = postfix_expression_parse();
	}
	return exp;
}

Expression postfix_expression_parse()
{
	Expression exp = primary_expression_parse();
	Token token = LEX_get_token();
	if (token->token_kind == INCREMENT || token->token_kind == DECREMENT)
	{
		Expression temp_exp = NULL;
		switch (token->token_kind)
		{
			case INCREMENT:
			{
				temp_exp = expression_create(INC_EXPRESSION);
				break;
			}
			case DECREMENT:
			{
				temp_exp = expression_create(DEC_EXPRESSION);
				break;
			}
		}
		temp_exp->u.inc_or_dec_expression = exp;
		temp_exp->line_number = exp->line_number;
		return temp_exp;
	}
	else
	{
		LEX_unget_token(token);
		return exp;
	}
}
Expression primary_expression_parse()
{
	Expression exp = NULL;
	Token token = LEX_get_token();
	if (token->token_kind == NEW)
	{
		Token second_token = LEX_get_token();
		if (second_token->token_kind == IDENTIFIER)
		{
			Token third_token = LEX_get_token();
			if (third_token->token_kind == LB)
			{
				LEX_unget_token(third_token);
				LEX_unget_token(second_token);
				LEX_unget_token(token);
				exp = primary_array_creation();
				return exp;
			}
			else
			{
				LEX_unget_token(third_token);
				LEX_unget_token(second_token);
				LEX_unget_token(token);
				exp = primary_no_new_array();
				return exp;
			}
		}
		else if (second_token->token_kind == BOOL || second_token->token_kind == INT
			|| second_token->token_kind == DOUBLE || second_token->token_kind == STRING)
		{
			LEX_unget_token(second_token);
			LEX_unget_token(token);
			exp = primary_array_creation();
			return exp;
		}
	}
	else
	{
		LEX_unget_token(token);
		exp = primary_no_new_array();
	}
	return exp;
}

Expression primary_no_new_array()
{
	Expression exp = NULL;
	Token token = LEX_get_token();
	switch (token->token_kind)
	{
		case NIL:
		{
			exp = expression_create(NIL_EXPRESSION);	
			exp->line_number = token->line_number;
			break;
		}
		case BOOL_V:
		{
			exp = expression_create(BOOL_EXPRESSION);
			exp->u.bool_value = (strcmp(token->buff, "true") == 0 ? TRUE : FALSE);
			exp->line_number = token->line_number;
			break;
		}
		case INT_V:
		{
			exp = expression_create(INT_EXPRESSION);
			exp->u.int_value = atoi(token->buff);
			exp->line_number = token->line_number;
			break;
		}
		case DOUBLE_V:
		{
			exp = expression_create(DOUBLE_EXPRESSION);
			exp->u.double_value = atof(token->buff);
			exp->line_number = token->line_number;
			break;
		}
		case STRING_V:
		{
			exp = expression_create(STRING_EXPRESSION);
			exp->u.string_value = string_create(token->buff);
			exp->line_number = token->line_number;
			break;
		}		
		case IDENTIFIER:
		{
			Token second_token = LEX_get_token();
			if (second_token->token_kind == LP)
			{
				// function call
				exp = expression_create(FUNCTION_CALL_EXPRESSION);
				exp->u.function_call_expression->function_name = string_create(token->buff);
				exp->line_number = token->line_number;
				token = LEX_get_token();
				if (token->token_kind != RP)
				{
					LEX_unget_token(token);
					exp->u.function_call_expression->argument_list = argument_list_parse();
					token = expect_token(RP);
				}
				else
				{
					exp->u.function_call_expression->argument_list = NULL;
				}
			}
			else if (second_token->token_kind == LB || second_token->token_kind == DOT)
			{
				// array index or field access
				exp = NULL;
				Expression left = expression_create(IDENTIFIER_EXPRESSION);
				left->u.identifier_expression->name = string_create(token->buff);
				left->line_number = token->line_number;
				Expression right = NULL;
				token = second_token;
				while (token->token_kind == LB || token->token_kind == DOT)
				{
					if (token->token_kind == LB)
					{
						right = expression_parse();
						token = expect_token(RB);
						exp = expression_create(ARRAY_INDEX_EXPRESSION);
						exp->u.array_index_expression->array_name = left;
						exp->u.array_index_expression->index = right;
						exp->line_number = left->line_number;
						left = exp;
					}
					else
					{
						token = expect_token(IDENTIFIER);
						right = expression_create(IDENTIFIER_EXPRESSION);
						right->u.identifier_expression->name = string_create(token->buff);
						right->line_number = token->line_number;
						exp = expression_create(FIELD_ACCESS_EXPRESSION);
						exp->u.field_access_expression->record_name = left;
						exp->u.field_access_expression->field_name = right;
						exp->line_number = left->line_number;
						left = exp;
					}
					token = LEX_get_token();
				}
				LEX_unget_token(token);
			}
			else
			{
				LEX_unget_token(second_token);
				exp = expression_create(IDENTIFIER_EXPRESSION);
				exp->u.identifier_expression->name = string_create(token->buff);
				exp->line_number = token->line_number;
			}
			break;
		}
		case LP:
		{
			exp = expression_create(PARENTHESIS_EXPRESSION);
			exp->line_number = token->line_number;
			exp->u.parenthesis_expression = expression_parse();
			token = expect_token(RP);
			break;
		}
		case LC:
		{
			// no implementions;
			LOG_ERROR("no implement for {}");
			LOG_EXIT();
			break;
		}
		case NEW:
		{
			exp = expression_create(NEW_EXPRESSION);
			exp->line_number = token->line_number;
			Token token = expect_token(IDENTIFIER);
			exp->u.new_expression->type_kind = RECORD_TYPE;
			exp->u.new_expression->u.record_type = MEM_malloc(sizeof(*(exp->u.new_expression->u.record_type)));
			exp->u.new_expression->u.record_type->record_name = string_create(token->buff);
			exp->u.new_expression->dimension_list = NULL;
			break;
		}
		default:
		{
			//LOG_ERROR("[line %d]invalid token %s", token->line_number, LEX_get_token_kind_string(token->token_kind));
			invalid_token(token);
		}
	}
	return exp;
}

Expression primary_array_creation()
{
	Expression exp = expression_create(NEW_EXPRESSION);
	Token token = expect_token(NEW);
	exp->line_number = token->line_number;
	token = LEX_get_token();
	if (token->token_kind == IDENTIFIER)
	{
		exp->u.new_expression->type_kind = RECORD_TYPE;
		exp->u.new_expression->u.record_type = MEM_malloc(sizeof(*(exp->u.new_expression->u.record_type)));
		exp->u.new_expression->u.record_type->record_name = string_create(token->buff);
		exp->u.new_expression->dimension_list = dimension_list_parse();
	}
	else if (token->token_kind == BOOL || token->token_kind == INT
		|| token->token_kind == DOUBLE || token->token_kind == STRING)
	{
		exp->u.new_expression->type_kind = BASIC_TYPE;
		exp->u.new_expression->u.basic_type = MEM_malloc(sizeof(*(exp->u.new_expression->u.basic_type)));
		switch (token->token_kind)
		{
			case BOOL:
				exp->u.new_expression->u.basic_type->kind = BOOL_TYPE;
				break;
			case INT:
				exp->u.new_expression->u.basic_type->kind = INT_TYPE;
				break;
			case DOUBLE:
				exp->u.new_expression->u.basic_type->kind = DOUBLE_TYPE;
				break;
			case STRING:
				exp->u.new_expression->u.basic_type->kind = STRING_TYPE;
				break;
		}
		exp->u.new_expression->dimension_list = dimension_list_parse();
	}
	return exp;
}

Dimension_List dimension_list_parse()
{
	Dimension_List dimension_list = MEM_malloc(sizeof(*dimension_list));
	Token token = expect_token(LB);
	token = LEX_get_token();
	if (token->token_kind != RB)
	{
		LEX_unget_token(token);
		dimension_list->dimension_expression = expression_parse();
		token = expect_token(RB);
	}
	else
	{
		dimension_list->dimension_expression = NULL;
	}
	token = LEX_get_token();
	if (token->token_kind == LB)
	{
		LEX_unget_token(token);
		dimension_list->next_dimension = dimension_list_parse();
	}
	else
	{
		LEX_unget_token(token);
		dimension_list->next_dimension = NULL;
	}
	return dimension_list;
}

/*
argument_list_parse old implementions
Argument_List argument_list_parse()
{
	Argument_List argument_list = NULL;
	Expression exp = expression_parse();
	while (exp != NULL)
	{
		Argument_List temp = MEM_malloc(sizeof(*temp));
		if (exp->kind == COMMA_EXPRESSION)
		{
			temp->expression = exp->u.comma_expression->right;
			temp->next = argument_list;
			argument_list = temp;
			Expression left = exp->u.comma_expression->left;
			MEM_free(exp->u.comma_expression);
			MEM_free(exp);
			exp = left;
		}
		else
		{
			temp->expression = exp;
			temp->next = argument_list;
			argument_list = temp;
			exp = NULL;
		}
	}
	return argument_list;
}
*/
Argument_List argument_list_parse()
{
	Argument_List argument_list = MEM_malloc(sizeof(*argument_list));
	Expression exp = assignment_expression_parse();
	argument_list->expression = exp;
	Token token = LEX_get_token();
	if (token->token_kind == COMMA)
	{
		argument_list->next = argument_list_parse();
	}
	else
	{
		LEX_unget_token(token);
		argument_list->next = NULL;
	}
	return argument_list;
}

Expression_List expression_list_parse()
{
	Expression_List expression_list = NULL;
	Expression exp = expression_parse();
	while (exp != NULL)
	{
		Expression_List temp = MEM_malloc(sizeof(*temp));
		if (exp->kind == COMMA_EXPRESSION)
		{
			temp->expression = exp->u.comma_expression->right;
			temp->next = expression_list;
			expression_list = temp;
			Expression left = exp->u.comma_expression->left;
			//expression_delete_no_recursive(exp);
			MEM_free(exp->u.comma_expression);
			MEM_free(exp);
			exp = left;
		}
		else
		{
			temp->expression = exp;
			temp->next = expression_list;
			expression_list = temp;
			exp = NULL;
		}
	}
	return expression_list;
}

Statement statement_create(Statement_Kind statement_kind)
{
	Statement stm = MEM_malloc(sizeof(*stm));
	stm->line_number = 0;
	stm->kind = statement_kind;
	switch (statement_kind)
	{
		case EXPRESSION_STATEMENT:
		{
			stm->u.expression_statement = NULL;
			break;
		}
		case IF_STATEMENT:
		{
			stm->u.if_statement = MEM_malloc(sizeof(*(stm->u.if_statement)));
			stm->u.if_statement->condition = NULL;
			stm->u.if_statement->then_block = NULL;
			break;
		}
		case DO_WHILE_STATEMENT:
		{
			stm->u.do_while_statement = MEM_malloc(sizeof(*(stm->u.do_while_statement)));
			stm->u.do_while_statement->block = NULL;
			stm->u.do_while_statement->condition = NULL;
			break;
		}
		case WHILE_STATEMENT:
		{
			stm->u.while_statement = MEM_malloc(sizeof(*(stm->u.while_statement)));
			stm->u.while_statement->condition = NULL;
			stm->u.while_statement->block = NULL;
			break;
		}
		case FOR_STATEMENT:
		{
			stm->u.for_statement = MEM_malloc(sizeof(*(stm->u.for_statement)));
			stm->u.for_statement->init = NULL;
			stm->u.for_statement->condition = NULL;
			stm->u.for_statement->post = NULL;
			stm->u.for_statement->block = NULL;
			break;
		}
		case RETURN_STATEMENT:
		{
			stm->u.return_statement = MEM_malloc(sizeof(*(stm->u.return_statement)));
			stm->u.return_statement->retval = NULL;
			break;
		}
		case BREAK_STATEMENT:
		{
			stm->u.break_statement = MEM_malloc(sizeof(*(stm->u.break_statement)));
			break;
		}
		case CONTINUE_STATEMENT:
		{
			stm->u.continue_statement = MEM_malloc(sizeof(*(stm->u.continue_statement)));
			break;
		}
		case DECLARATION_STATEMENT:
		{
			stm->u.declaration_statement = MEM_malloc(sizeof(*(stm->u.declaration_statement)));
			stm->u.declaration_statement->is_static = FALSE;
			stm->u.declaration_statement->type = NULL;
			stm->u.declaration_statement->variable_list = NULL;
			break;
		}
		case BLOCK:
		{
			stm->u.block = NULL;
			break;
		}
		default:
		{
			LOG_ERROR("unknown statement kind");
			LOG_EXIT();
		}
	}
	return stm;
}
Statement statement_parse()
{
	Statement stm = NULL;
	Token token = LEX_get_token();
	switch (token->token_kind)
	{
		case LP:
		case NIL:
		case BOOL_V:
		case INT_V:
		case DOUBLE_V:
		case STRING_V:
		//case LC:
		case NEW:
		case SUB:
		case EXCLAMATION:
		case IDENTIFIER:
		{
			LEX_unget_token(token);
			stm = expression_statement_parse();
			break;
		}
		case VARIABLE:
		{
			LEX_unget_token(token);
			stm = declaration_statement_parse();
			break;
		}
		case IF:
		{
			LEX_unget_token(token);
			stm = if_statement_parse();
			break;
		}
		case WHILE:
		{
			LEX_unget_token(token);
			stm = while_statement_parse();
			break;
		}
		case FOR:
		{
			LEX_unget_token(token);
			stm = for_statement_parse();
			break;
		}
		case DO:
		{
			LEX_unget_token(token);
			stm = do_while_statement_parse();
			break;
		}
		case RETURN:
		{
			LEX_unget_token(token);
			stm = return_statement_parse();
			break;
		}
		case BREAK:
		{
			LEX_unget_token(token);
			stm = break_statement_parse();
			break;
		}
		case CONTINUE:
		{
			LEX_unget_token(token);
			stm = continue_statement_parse();
			break;
		}
		case LC:
		{
			LEX_unget_token(token);
			stm = statement_create(BLOCK);
			stm->u.block = block_parse();
			stm->line_number = stm->u.block->line_number;
			break;
		}
		default:
		{
			//LOG_ERROR("unknown token kind");
			invalid_token(token);
		}
	}
	return stm;
}

Statement expression_statement_parse()
{
	Statement stm = statement_create(EXPRESSION_STATEMENT);
	stm->u.expression_statement = expression_parse();
	Token token = expect_token(SEMICOLON);
	stm->line_number = stm->u.expression_statement->line_number;
	return stm;
}

Statement if_statement_parse()
{
	Statement stm = statement_create(IF_STATEMENT);
	Token token = expect_token(IF);
	stm->line_number = token->line_number;
	stm->u.if_statement->line_number = token->line_number;
	token = expect_token(LP);
	stm->u.if_statement->condition = expression_parse();
	token = expect_token(RP);
	stm->u.if_statement->then_block = block_parse();
	token = LEX_get_token();
	if (token->token_kind == ELSE)
	{
		Token second_token = LEX_get_token();
		if (second_token->token_kind == IF)
		{
			LEX_unget_token(second_token);
			stm->u.if_statement->kind = IF_WITH_ELSE_IF;
			stm->u.if_statement->u.else_if_statement = if_statement_parse();
			return stm;
		}
		else
		{
			LEX_unget_token(second_token);
			stm->u.if_statement->kind = IF_WITH_ELSE;
			stm->u.if_statement->u.else_block = block_parse();
			return stm;
		}
	}
	else
	{
		LEX_unget_token(token);
		stm->u.if_statement->kind = IF_WITHOUT_ELSE;
		return stm;
	}
}

Statement while_statement_parse()
{
	Statement stm = statement_create(WHILE_STATEMENT);
	Token token = expect_token(WHILE);
	stm->line_number = token->line_number;
	stm->u.while_statement->line_number = stm->line_number;
	token = expect_token(LP);
	stm->u.while_statement->condition = expression_parse();
	token = expect_token(RP);
	stm->u.while_statement->block = block_parse();
	return stm;
}

Statement do_while_statement_parse()
{
	Statement stm = statement_create(DO_WHILE_STATEMENT);
	Token token = expect_token(DO);
	stm->line_number = token->line_number;
	stm->u.do_while_statement->line_number = stm->line_number;
	stm->u.do_while_statement->block = block_parse();
	token = expect_token(WHILE);
	token = expect_token(LP);
	stm->u.do_while_statement->condition = expression_parse();
	token = expect_token(RP);
	token = expect_token(SEMICOLON);
	return stm;
}

Statement for_statement_parse()
{
	Statement stm = statement_create(FOR_STATEMENT);
	Token token = expect_token(FOR);
	stm->line_number = stm->line_number;
	stm->u.for_statement->line_number = stm->line_number;
	token = expect_token(LP);
	token = LEX_get_token();
	if (token->token_kind != SEMICOLON)
	{
		LEX_unget_token(token);
		stm->u.for_statement->init = expression_parse();
		token = expect_token(SEMICOLON);
	}
	else
	{
		stm->u.for_statement->init = NULL;
	}
	token = LEX_get_token();
	if (token->token_kind != SEMICOLON)
	{
		LEX_unget_token(token);
		stm->u.for_statement->condition = expression_parse();
		token = expect_token(SEMICOLON);
	}
	else
	{
		stm->u.for_statement->condition = NULL;
	}
	token = LEX_get_token();
	if (token->token_kind != RP)
	{
		LEX_unget_token(token);
		stm->u.for_statement->post = expression_parse();
		token = expect_token(RP);
	}
	else
	{
		stm->u.for_statement->post = NULL;
	}
	stm->u.for_statement->block = block_parse();
	return stm;
}

Statement return_statement_parse()
{
	Statement stm = statement_create(RETURN_STATEMENT);
	Token token = expect_token(RETURN);
	stm->line_number = token->line_number;
	stm->u.return_statement->line_number = stm->line_number;
	token = LEX_get_token();
	if (token->token_kind != SEMICOLON)
	{
		LEX_unget_token(token);
		stm->u.return_statement->retval = expression_parse();
		token = expect_token(SEMICOLON);
	}
	return stm;
}
Statement break_statement_parse()
{
	Statement stm = statement_create(BREAK_STATEMENT);
	Token token = expect_token(BREAK);
	stm->line_number = token->line_number;
	stm->u.break_statement->line_number = stm->line_number;
	token = expect_token(SEMICOLON);
	return stm;
}

Statement continue_statement_parse()
{
	Statement stm = statement_create(CONTINUE_STATEMENT);
	Token token = expect_token(CONTINUE);
	stm->line_number = token->line_number;
	stm->u.continue_statement->line_number = stm->line_number;
	token = expect_token(SEMICOLON);
	return stm;
}

Statement declaration_statement_parse()
{
	Statement stm = statement_create(DECLARATION_STATEMENT);
	Token token = expect_token(VARIABLE);
	stm->line_number = token->line_number;
	stm->u.declaration_statement->line_number = stm->line_number;
	token = LEX_get_token();
	if (token->token_kind == STATIC)
	{
		stm->u.declaration_statement->is_static = TRUE;
	}
	else
	{
		LEX_unget_token(token);
		stm->u.declaration_statement->is_static = FALSE;
	}
	stm->u.declaration_statement->type = type_parse();
	Declaration_Variable_List *iterator = &(stm->u.declaration_statement->variable_list);
	while (TRUE)
	{
		token = expect_token(IDENTIFIER);
		Declaration_Variable_List variable_list = MEM_malloc(sizeof(*variable_list));
		variable_list->next = NULL;
		variable_list->variable_name = string_create(token->buff);
		token = LEX_get_token();
		if (token->token_kind == ASSIGN)
		{
			variable_list->initialization = logical_or_expression_parse();
		}
		else
		{
			LEX_unget_token(token);
			variable_list->initialization = NULL;
		}
		*iterator = variable_list;
		iterator = &(variable_list->next);
		token = LEX_get_token();
		if (token->token_kind == COMMA)
		{
			continue;
		}
		else if (token->token_kind == SEMICOLON)
		{
			break;
		}
		else
		{
			//
			invalid_token(token);
		}
	}
	
	return stm;
}
Statement_List statement_list_parse()
{
	Statement_List statement_list = MEM_malloc(sizeof(*statement_list));
	statement_list->statement = statement_parse();
	Token token = LEX_get_token();
	if (token->token_kind != RC)
	{
		LEX_unget_token(token);
		statement_list->next = statement_list_parse();
	}
	else
	{
		LEX_unget_token(token);
		statement_list->next = NULL;
	}
	return statement_list;
}

Block block_parse()
{
	Block block = MEM_malloc(sizeof(*block));
	Token token = expect_token(LC);
	block->line_number = token->line_number;
	token = LEX_get_token();
	if (token->token_kind != RC)
	{
		LEX_unget_token(token);
		block->statement_list = statement_list_parse();
		token = expect_token(RC);
	}
	else
	{
		block->statement_list = NULL;
	}	
	return block;
}