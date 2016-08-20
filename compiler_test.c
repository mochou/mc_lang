#include "environment.h"
#include "LEX.h"
#include "absynt.h"
#include "absynt_create.h"
#include "absynt_print.h"
#include "semantic_analysis.h"
#include "generate_code.h"
void compiler_test()
{
	char *file_name = "test.mc";
	LEX_initialize(file_name);
	Translation_Unit tu = translation_unit_parse();
	//translation_unit_print(tu);
	//printf("hello");
	translation_unit_semantic_analysis(tu);
	generate_code(tu);
}

int compiler_main(int argc, char **argv)
{
	compiler_test();
	getchar();
	return 0;
}