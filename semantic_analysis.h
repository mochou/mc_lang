#ifndef __SEMANTIC_ANALYSIS_H
#define __SEMANTIC_ANALYSIS_H

#include "ABSYNT.h"
#include "symbol.h"
void translation_unit_semantic_analysis(Translation_Unit tu);
void record_definition_semantic_analysis(Translation_Unit tu);
void variable_definition_semantic_analysis(Translation_Unit tu);
void function_definition_semantic_analysis(Translation_Unit tu);
void block_semantic_analysis(Block block, Symbol_Definition symbol_definition);
void if_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	If_Statement statement);
void while_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	While_Statement statement);
void do_while_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Do_While_Statement statement);
void for_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	For_Statement statement);
void return_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Return_Statement statement);
void break_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Break_Statement statement);
void continue_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Continue_Statement statement);
void expression_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Expression expression);
void declaration_statement_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Declaration_Statement statement);

int basic_type_equal(Basic_Type bt1, Basic_Type bt2);
int record_type_equal(Record_Type rt1, Record_Type rt2);
int type_equal(Type t1, Type t2);
Basic_Type basic_type_copy(Basic_Type bt);
Record_Type record_type_copy(Record_Type rt);
Type type_copy(Type type);
Type basic_type_create(Basic_Type_Kind kind);
Type record_type_create(char *record_name);
int is_basic_type(Type type, Basic_Type_Kind kind);
int is_record_type(Type type);
int type_check(Type type, Map record_definition);
void expression_type_error(Expression exp);
void binary_expression_semantic_analysis(Block cur_block, Symbol_Definition symbol_definition,
	Expression exp);

void semantic_print(Translation_Unit tu);
void block_semantic_print(Block block);
#endif