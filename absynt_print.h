#ifndef __ABSYNT_PRINT_H
#define __ABSYNT_PRINT_H

#include "ABSYNT.h"
// print function
void translation_unit_print(Translation_Unit tu);
void import_list_print(int white_space_count, Import_List il);
void definition_list_print(int white_space_count, Definition_List dl);
void basic_type_print(Basic_Type bt);
void record_type_print(Record_Type rt);
void type_print(Type type);
void parameter_list_print(Parameter_List pl);
void statement_list_print(int white_space_count, Statement_List sl);
void block_print(int white_space_count, Block block);
void function_definition_print(int white_space_count, Function_Definition df);
void record_definition_print(int white_space_count, Record_Definition rd);
void variable_definition_print(int white_space_count, Statement stm);
void binary_expression_print(Expression exp);
void argument_list_print(Argument_List al);
void identifier_expression_print(Identifier_Expression ie);
void function_call_expression_print(Function_Call_Expression fc);
void field_access_expression_print(Field_Access_Expression fa);
void array_index_expression_print(Array_Index_Expression ai);
void dimension_list_print(Dimension_List dl);
void new_expression_print(New_Expression ne);
void expression_print(Expression exp);
void if_statement_print(int white_space_count, If_Statement stm);
void for_statement_print(int white_space_count, For_Statement stm);
void while_statement_print(int white_space_count, While_Statement stm);
void do_while_statement_print(int white_space_count, Do_While_Statement stm);
void return_statement_print(int white_space_count, Return_Statement stm);
void break_statement_print(int white_space_count, Break_Statement stm);
void continue_statement_print(int white_space_count, Continue_Statement stm);
void declaration_variable_list_print(Declaration_Variable_List dvl);
void declaration_statement_print(int white_space_count, Declaration_Statement stm);
void statement_print(int white_space_count, Statement stm);


#endif