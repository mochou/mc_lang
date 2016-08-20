#include "environment.h"
#include "LEX.h"
#include "absynt.h"
#include "absynt_create.h"
#include "absynt_print.h"
#include "semantic_analysis.h"
#include "generate_code.h"
void mvm_test(int argc, char **argv)
{
	char *file_name = argv[1];
	LEX_initialize(file_name);
	Translation_Unit tu = translation_unit_parse();
	translation_unit_semantic_analysis(tu);
	MVM_Executable mvm_executable = mvm_executable_generate(argc, argv, tu);
	mvm_code_show(mvm_executable);
	mvm_run(mvm_executable);
}

int main(int argc, char **argv)
{
	mvm_test(argc, argv);
	getchar();
	return 0;
}