#ifndef __GENERATE_CODE_H
#define __GENERATE_CODE_H

#include "ABSYNT.h"
#include "mvm.h"

#include "environment.h"
#include "generate_code.h"
#include "tool.h"
#include "memory.h"
#include "symbol.h"
#include "LOG.h"
#include "mvm.h"
#include "semantic_analysis.h"
#include "literal.h"


/* tool function*/
int local_variable_max_offset(Block block);
//

MVM_Executable mvm_executable_generate(int argc, char **argv, Translation_Unit tu);
void variable_definition_generate_code(MVM_Executable mvm_executable, Literal literal, Translation_Unit tu);
void record_definition_generate_code(MVM_Executable mvm_executable, Literal literal, Translation_Unit tu);
void function_definition_generate_code(MVM_Executable mvm_executable, Literal literal, Translation_Unit tu);
void block_generate_code(MVM_Executable mvm_executable, Literal literal, Block block);
// generate statement code
void if_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm);
void do_while_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm);
void while_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm);
void for_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm);
void break_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm);
void continue_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm);
void return_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm);
void declaration_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block,
	Declaration_Statement declaration_statement);

// generate expression code
void expression_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Expression expression);
// assist functions for generating expression
int mvm_find_record_type_index(MVM_Executable mvm_executable, Literal literal, char *record_name);
int mvm_find_record_field_index(MVM_Executable mvm_executable, Literal literal, MVM_Record_Info record_info, char *field_name);
int mvm_find_function_index(MVM_Executable mvm_executable, Literal literal, char *function_name);
void identifier_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal,
	Block block, Expression expression);
void function_call_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal,
	Block block, Expression expression);
void field_access_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal,
	Block block, Expression expression);
void array_index_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal,
	Block block, Expression expression);
void new_expression_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Expression expression);


// fix code
void code_fix(MVM_Executable mvm_executable);
void generate_code(MVM_Executable mvm_executable, char opcode, int line_number, char *format, ...);

#endif