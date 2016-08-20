#include "environment.h"
#include "LEX.h"
#include "absynt.h"
#include "absynt_create.h"
#include "absynt_print.h"

void absynt_test()
{
	char *file_name = "test.mc";
	LEX_initialize(file_name);
	Translation_Unit tu = translation_unit_parse();
	translation_unit_print(tu);
	//printf("hello");
}

int main_absynt_test(int argc, char **argv)
{
	absynt_test();
	getchar();
	return 0;
}