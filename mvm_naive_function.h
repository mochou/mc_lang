#ifndef __MVM_NAIVE_FUNCTION_H
#define __MVM_NAIVE_FUNCTION_H

#include "mvm.h"

MVM_Value mvm_naive_function_print(MVM_Executable mvm_executable, int argc, MVM_Value *argv);
MVM_Value mvm_naive_function_int_to_string(MVM_Executable mvm_executable, int argc, MVM_Value *argv);
MVM_Value mvm_naive_function_double_to_string(MVM_Executable mvm_executable, int argc, MVM_Value *argv);
MVM_Value mvm_naive_function_bool_to_string(MVM_Executable mvm_executable, int argc, MVM_Value *argv);
MVM_Value mvm_naive_function_int_to_double(MVM_Executable mvm_executable, int argc, MVM_Value *argv);
MVM_Value mvm_naive_function_double_to_int(MVM_Executable mvm_executable, int argc, MVM_Value *argv);
MVM_Value mvm_naive_function_random(MVM_Executable mvm_executable, int argc, MVM_Value *argv);
#endif