#include "environment.h"
#include "LEX.h"

static void test()
{
	char *file_name = "test.mc";
	LEX_initialize(file_name);
	Token token = NULL;
	do
	{
 		token = LEX_get_token();
		LEX_print_token(token);
	} while (token->token_kind != END_OF_FILE);
}
int _main(int argc, char **argv)
{
	test();
	getchar();
	return 0;
}