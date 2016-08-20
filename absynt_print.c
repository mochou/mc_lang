#include "environment.h"
#include "LEX.h"
#include "LOG.h"
#include "ABSYNT.h"
#include "absynt_print.h"

#define TAB_WIDTH 4

static void indent_code(int white_space_count)
{
	for (int i = 0; i < white_space_count; i++)
	{
		LOG_INFO(" ");
	}
}
void translation_unit_print(Translation_Unit tu)
{
	import_list_print(0, tu->import_list);
	definition_list_print(0, tu->definition_list);
}

void import_list_print(int white_space_count, Import_List il)
{
	if (il == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("import %s;\n", il->package_name);
	import_list_print(white_space_count, il->next);
}

void definition_list_print(int white_space_count, Definition_List dl)
{
	if (dl == NULL)
	{
		return;
	}
	switch (dl->kind)
	{
		case RECORD_DEFINITION:
		{
			record_definition_print(white_space_count, dl->u.record_defintion);
			break;
		}
		case FUNCTION_DEFINITION:
		{
			function_definition_print(white_space_count, dl->u.function_definition);
			break;
		}
		case VARIABLE_DEFINITION:
		{
			variable_definition_print(white_space_count, dl->u.variable_definition);
			break;
		}
		default:
		{
			LOG_ERROR("unknown definition kind");
			LOG_EXIT();
		}
	}
	definition_list_print(white_space_count, dl->next);
}
void basic_type_print(Basic_Type bt)
{
	if (bt == NULL)
	{
		return;
	}
	switch (bt->kind)
	{
		case VOID_TYPE:
		{
			LOG_INFO("void");
			break;
		}
		case BOOL_TYPE:
		{
			LOG_INFO("bool");
			break;
		}
		case INT_TYPE:
		{
			LOG_INFO("int");
			break;
		}
		case DOUBLE_TYPE:
		{
			LOG_INFO("double");
			break;
		}
		case STRING_TYPE:
		{
			LOG_INFO("string");
			break;
		}
	}
}
void record_type_print(Record_Type rt)
{
	if (rt == NULL)
	{
		return;
	}
	LOG_INFO("%s", rt->record_name);
}
void type_print(Type type)
{
	if (type == NULL)
	{
		return;
	}
	switch (type->kind)
	{
		case BASIC_TYPE:
		{
			basic_type_print(type->u.basic_type);
			break;
		}
		case RECORD_TYPE:
		{
			record_type_print(type->u.record_type);
			break;
		}
		case ARRAY_TYPE:
		{
			type_print(type->u.array_type);
			LOG_INFO("[]");
			break;
		}
	}
}
void parameter_list_print(Parameter_List pl)
{
	if (pl == NULL)
	{
		return;
	}
	LOG_INFO("var ");
	type_print(pl->type);
	LOG_INFO(" ");
	LOG_INFO("%s", pl->parameter_name);
	if (pl->next != NULL)
	{
		LOG_INFO(", ");
		parameter_list_print(pl->next);
	}
}
void statement_list_print(int white_space_count, Statement_List sl)
{
	if (sl == NULL)
	{
		return;
	}
	statement_print(white_space_count, sl->statement);
	statement_list_print(white_space_count, sl->next);
}
void block_print(int white_space_count, Block block)
{
	if (block == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("{\n");
	statement_list_print(white_space_count + TAB_WIDTH, block->statement_list);
	indent_code(white_space_count);
	LOG_INFO("}\n");
}
void function_definition_print(int white_space_count, Function_Definition df)
{
	if (df == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("function ");
	if (df->is_static == TRUE)
	{
		LOG_INFO("static ");
	}
	type_print(df->type);
	LOG_INFO(" ");
	LOG_INFO("%s", df->function_name);
	LOG_INFO("(");
	parameter_list_print(df->parameter_list);
	LOG_INFO(")\n");
	if (df->function_kind == USER_DEFINIED_FUNCTION)
	{
		block_print(white_space_count, df->u.user_defined_function);
	}
	else if (df->function_kind = NAIVE_FUNCTION)
	{
		LOG_INFO(";\n");
	}
}

void record_definition_print(int white_space_count, Record_Definition rd)
{
	if (rd == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("record ");
	LOG_INFO("%s\n", rd->record_name);
	indent_code(white_space_count);
	//LOG_INFO("{\n");
	//statement_list_print(white_space_count + TAB_WIDTH, rd->field_list);
	//indent_code(white_space_count);
	//LOG_INFO("}\n");
	block_print(white_space_count, rd->field_block);
}

void variable_definition_print(int white_space_count, Statement stm)
{
	statement_print(white_space_count, stm);
}

void binary_expression_print(Expression exp)
{
	expression_print(exp->u.binary_expression->left);
	switch (exp->kind)
	{
		case OR_EXPRESSION:
		{
			LOG_INFO(" || ");
			break;
		}
		case AND_EXPRESSION:
		{
			LOG_INFO(" && ");
			break;
		}
		case EQ_EXPRESSION:
		{
			LOG_INFO(" == ");
			break;
		}
		case NE_EXPRESSION:
		{
			LOG_INFO(" != ");
			break;
		}
		case GT_EXPRESSION:
		{
			LOG_INFO(" > ");
			break;
		}
		case GE_EXPRESSION:
		{
			LOG_INFO(" >= ");
			break;
		}
		case LT_EXPRESSION:
		{
			LOG_INFO(" < ");
			break;
		}
		case LE_EXPRESSION:
		{
			LOG_INFO(" <= ");
			break;
		}
		case ADD_EXPRESSION:
		{
			LOG_INFO(" + ");
			break;
		}
		case SUB_EXPRESSION:
		{
			LOG_INFO(" - ");
			break;
		}
		case MUL_EXPRESSION:
		{
			LOG_INFO(" * ");
			break;
		}
		case DIV_EXPRESSION:
		{
			LOG_INFO(" / ");
			break;
		}
		case MOD_EXPRESSION:
		{
			LOG_INFO(" % ");
			break;
		}
	}
	expression_print(exp->u.binary_expression->right);
}
void argument_list_print(Argument_List al)
{
	if (al == NULL)
	{
		return;
	}
	expression_print(al->expression);
	if (al->next != NULL)
	{
		LOG_INFO(", ");
		argument_list_print(al->next);
	}
}
void identifier_expression_print(Identifier_Expression ie)
{
	if (ie == NULL)
	{
		return;
	}
	LOG_INFO("%s", ie->name);
}
void function_call_expression_print(Function_Call_Expression fc)
{
	if (fc == NULL)
	{
		return;
	}
	LOG_INFO("%s(", fc->function_name);
	argument_list_print(fc->argument_list);
	LOG_INFO(")");
}
void field_access_expression_print(Field_Access_Expression fa)
{
	if (fa == NULL)
	{
		return;
	}
	expression_print(fa->record_name);
	LOG_INFO(".");
	expression_print(fa->field_name);
}
void array_index_expression_print(Array_Index_Expression ai)
{
	if (ai == NULL)
	{
		return;
	}
	expression_print(ai->array_name);
	LOG_INFO("[");
	expression_print(ai->index);
	LOG_INFO("]");
}
void dimension_list_print(Dimension_List dl)
{
	if (dl == NULL)
	{
		return;
	}
	LOG_INFO("[");
	if (dl->dimension_expression != NULL)
	{
		expression_print(dl->dimension_expression);
	}
	LOG_INFO("]");
	dimension_list_print(dl->next_dimension);
}
void new_expression_print(New_Expression ne)
{
	if (ne == NULL)
	{
		return;
	}
	LOG_INFO("new ");
	switch (ne->type_kind)
	{
		case BASIC_TYPE:
		{
			basic_type_print(ne->u.basic_type);
			break;
		}
		case RECORD_TYPE:
		{
			record_type_print(ne->u.record_type);
			break;
		}
		default:
		{
			LOG_ERROR("error type");
			LOG_EXIT();
		}
	}
	dimension_list_print(ne->dimension_list);
}
void expression_print(Expression exp)
{
	if (exp == NULL)
	{
		return;
	}
	switch (exp->kind)
	{
		case COMMA_EXPRESSION:
		{
			expression_print(exp->u.comma_expression->left);
			LOG_INFO(", ");
			expression_print(exp->u.comma_expression->right);
			break;
		}
		case ASSIGN_EXPRESSION:
		{
			expression_print(exp->u.assign_expression->left);
			LOG_INFO(" = ");
			expression_print(exp->u.assign_expression->right);
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
			binary_expression_print(exp);
			break;
		}
		case MINUS_EXPRESSION:
		{
			LOG_INFO("-");
			expression_print(exp->u.minus_expression);
			break;
		}
		case NOT_EXPRESSION:
		{
			LOG_INFO("!");
			expression_print(exp->u.minus_expression);
			break;
		}
		case INC_EXPRESSION:
		{
			expression_print(exp->u.inc_or_dec_expression);
			LOG_INFO("++");
			break;
		}
		case DEC_EXPRESSION:
		{
			expression_print(exp->u.inc_or_dec_expression);
			LOG_INFO("--");
			break;
		}
		case IDENTIFIER_EXPRESSION:
		{
			identifier_expression_print(exp->u.identifier_expression);
			break;
		}
		case FUNCTION_CALL_EXPRESSION:
		{
			function_call_expression_print(exp->u.function_call_expression);
			break;
		}
		case FIELD_ACCESS_EXPRESSION:
		{
			field_access_expression_print(exp->u.field_access_expression);
			break;
		}
		case ARRAY_INDEX_EXPRESSION:
		{
			array_index_expression_print(exp->u.array_index_expression);
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
			new_expression_print(exp->u.new_expression);
			break;
		}
		case NIL_EXPRESSION:
		{
			LOG_INFO("nil");
			break;
		}
		case BOOL_EXPRESSION:
		{
			LOG_INFO("%s", exp->u.bool_value == TRUE ? "true" : "false");
			break;
		}
		case INT_EXPRESSION:
		{
			LOG_INFO("%d", exp->u.int_value);
			break;
		}
		case DOUBLE_EXPRESSION:
		{
			LOG_INFO("%.6f", exp->u.double_value);
			break;
		}
		case STRING_EXPRESSION:
		{
			LOG_INFO("\"");
			LOG_INFO("%s", exp->u.string_value);
			LOG_INFO("\"");
			break;
		}
		case PARENTHESIS_EXPRESSION:
		{
			LOG_INFO("(");
			expression_print(exp->u.parenthesis_expression);
			LOG_INFO(")");
			break;
		}
	}
}

void if_statement_print(int white_space_count, If_Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	while (TRUE)
	{
		LOG_INFO("if (");
		expression_print(stm->condition);
		LOG_INFO(")\n");
		block_print(white_space_count, stm->then_block);
		if (stm->kind == IF_WITHOUT_ELSE)
		{
			break;
		}
		else if (stm->kind == IF_WITH_ELSE)
		{
			indent_code(white_space_count);
			LOG_INFO("else\n");
			block_print(white_space_count, stm->u.else_block);
			break;
		}
		else if (stm->kind == IF_WITH_ELSE_IF)
		{
			indent_code(white_space_count);
			LOG_INFO("else ");
			stm = stm->u.else_if_statement->u.if_statement;
		}
	}
}
void for_statement_print(int white_space_count, For_Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("for (");
	expression_print(stm->init);
	LOG_INFO("; ");
	expression_print(stm->condition);
	LOG_INFO("; ");
	expression_print(stm->post);
	LOG_INFO(")\n");
	block_print(white_space_count, stm->block);
}
void while_statement_print(int white_space_count, While_Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("while (");
	expression_print(stm->condition);
	LOG_INFO(")\n");
	block_print(white_space_count, stm->block);
}
void do_while_statement_print(int white_space_count, Do_While_Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("do\n");
	block_print(white_space_count, stm->block);
	indent_code(white_space_count);
	LOG_INFO("while (");
	expression_print(stm->condition);
	LOG_INFO(");\n");
}
void return_statement_print(int white_space_count, Return_Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("return");
	if (stm->retval != NULL)
	{
		LOG_INFO(" ");
		expression_print(stm->retval);
	}
	LOG_INFO(";\n");
}
void break_statement_print(int white_space_count, Break_Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("break;\n");
}
void continue_statement_print(int white_space_count, Continue_Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("continue;\n");
}
void declaration_variable_list_print(Declaration_Variable_List dvl)
{
	if (dvl == NULL)
	{
		return;
	}
	LOG_INFO("%s", dvl->variable_name);
	if (dvl->initialization != NULL)
	{
		LOG_INFO(" = ");
		expression_print(dvl->initialization);
	}
	if (dvl->next != NULL)
	{
		LOG_INFO(", ");
		declaration_variable_list_print(dvl->next);
	}
}
void declaration_statement_print(int white_space_count, Declaration_Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	indent_code(white_space_count);
	LOG_INFO("var ");
	if (stm->is_static == TRUE)
	{
		LOG_INFO("static ");
	}
	type_print(stm->type);
	LOG_INFO(" ");
	declaration_variable_list_print(stm->variable_list);
	LOG_INFO(";");
}
void statement_print(int white_space_count, Statement stm)
{
	if (stm == NULL)
	{
		return;
	}
	switch (stm->kind)
	{
		case EXPRESSION_STATEMENT:
		{
			indent_code(white_space_count);
			expression_print(stm->u.expression_statement);
			LOG_INFO(";");
			LOG_INFO("\n");
			break;
		}
		case IF_STATEMENT:
		{
			if_statement_print(white_space_count, stm->u.if_statement);
			break;
		}
		case DO_WHILE_STATEMENT:
		{
			do_while_statement_print(white_space_count, stm->u.do_while_statement);
			break;
		}
		case WHILE_STATEMENT:
		{
			while_statement_print(white_space_count, stm->u.while_statement);
			break;
		}
		case FOR_STATEMENT:
		{
			for_statement_print(white_space_count, stm->u.for_statement);
			break;
		}
		case RETURN_STATEMENT:
		{
			return_statement_print(white_space_count, stm->u.return_statement);
			break;
		}
		case BREAK_STATEMENT:
		{
			break_statement_print(white_space_count, stm->u.break_statement);
			break;
		}
		case CONTINUE_STATEMENT:
		{
			continue_statement_print(white_space_count, stm->u.continue_statement);
			break;
		}
		case DECLARATION_STATEMENT:
		{
			declaration_statement_print(white_space_count, stm->u.declaration_statement);
			LOG_INFO("\n");
			break;
		}
		case BLOCK:
		{
			block_print(white_space_count, stm->u.block);
			break;
		}
	}
}
