#ifndef __ABSYNT_CREATE_H
#define __ABSYNT_CREATE_H

#include "ABSYNT.h"
// function declaration
Translation_Unit translation_unit_parse();
Import_List import_list_parse();
Definition_List definition_list_parse();
int bracket_list_parse();
Type type_parse();
Record_Definition record_definition_parse();
Function_Definition function_definition_parse();
Parameter_List parameter_list_parse();
Expression expression_create(Expression_Kind exp_kind);
Expression expression_parse();
Expression assignment_expression_parse();
Expression logical_or_expression_parse();
Expression logical_and_expression_parse();
Expression equality_expression_parse();
Expression relational_expression_parse();
Expression additive_expression_parse();
Expression multiplicative_expression_parse();
Expression unary_expression_parse();
Expression postfix_expression_parse();
Expression primary_expression_parse();
Expression primary_no_new_array();
Expression primary_array_creation();
Dimension_List dimension_list_parse();
Argument_List argument_list_parse();
Expression_List expression_list_parse();
Statement statement_create(Statement_Kind statement_kind);
Statement statement_parse();
Statement expression_statement_parse();
Statement if_statement_parse();
Statement while_statement_parse();
Statement do_while_statement_parse();
Statement for_statement_parse();
Statement return_statement_parse();
Statement break_statement_parse();
Statement continue_statement_parse();
Statement declaration_statement_parse();
Statement_List statement_list_parse();
Block block_parse();

#endif