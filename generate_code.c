#include "environment.h"
#include "generate_code.h"
#include "tool.h"
#include "memory.h"
#include "symbol.h"
#include "LOG.h"
#include "mvm.h"
#include "semantic_analysis.h"
#include "red_black_tree.h"

static void mvm_check_code_buff(MVM_Executable mvm_executable, int needed_size)
{
	union
	{
		MVM_Int i;
		MVM_Double d;
		MVM_Object o;
	}ALIGNMENT;
	if ((int)(mvm_executable->pc + needed_size + sizeof(ALIGNMENT)) >= mvm_executable->code_size)
	{
		int new_increament_size = (CODE_SIZE > needed_size ? CODE_SIZE : (needed_size / CODE_SIZE + 1) * CODE_SIZE);
		int old_code_size = mvm_executable->code_size;
		char *old_code = mvm_executable->code;
		int *old_line_numbers = mvm_executable->line_numbers;
		int new_code_size = mvm_executable->code_size + new_increament_size;
		char *new_code = MEM_malloc(sizeof(*(mvm_executable->code)) * new_code_size);
		int *new_line_numbers = MEM_malloc(sizeof(*(mvm_executable->line_numbers)) * new_code_size);
		memset(new_code, 0, sizeof(*(mvm_executable->code)) * new_code_size);
		memset(new_line_numbers, 0, sizeof(*(mvm_executable->line_numbers)) * new_code_size);
		memcpy(new_code, old_code, sizeof(*(mvm_executable->code)) * old_code_size);
		memcpy(new_line_numbers, old_line_numbers, sizeof(*(mvm_executable->line_numbers)) * old_code_size);
		MEM_free(old_code);
		MEM_free(old_line_numbers);
		mvm_executable->code = new_code;
		mvm_executable->line_numbers = new_line_numbers;
		mvm_executable->code_size = new_code_size;
	}
}

static void generate_code(MVM_Executable mvm_executable, char opcode, int line_number, char *format, ...)
{
	mvm_check_code_buff(mvm_executable, sizeof(char));
	mvm_executable->code[mvm_executable->pc] = opcode;
	if (line_number == 0)
	{
		int default_line_number = 0;
		if (mvm_executable->pc == 0)
		{
			default_line_number = line_number;
		}
		else
		{
			int i = mvm_executable->pc - 1;
			for (; i >= 0; i--)
			{
				if (mvm_executable->line_numbers[i] != 0)
				{
					default_line_number = mvm_executable->line_numbers[i];
					break;
				}
			}
		}
		mvm_executable->line_numbers[mvm_executable->pc] = default_line_number;
	}
	else
	{
		mvm_executable->line_numbers[mvm_executable->pc] = line_number;
	}
	mvm_executable->pc += sizeof(char);
	if (format == NULL || strcmp(format, "") == 0)
	{
		return;
	}
	va_list args;
	va_start(args, format);
	int format_len = strlen(format);
	for (int i = 0; i < format_len; i++)
	{
		switch (format[i])
		{
			case 'i':
			{
				mvm_check_code_buff(mvm_executable, sizeof(int));
				int arg = va_arg(args, int);
				*(int*)(&mvm_executable->code[mvm_executable->pc]) = arg;
				mvm_executable->pc += sizeof(int);
				break;
			}
			default:
			{
				LOG_ERROR("format error");
				LOG_EXIT();
				break;
			}
		}
	}
}

void mvm_constant_pool_create(MVM_Executable mvm_executable, Literal literal)
{
	mvm_executable->constants = MEM_malloc(sizeof(*mvm_executable->constants));
	mvm_executable->constants->double_size = mvm_executable->constants->string_size = 0;
	mvm_executable->constants->double_literal = NULL;
	mvm_executable->constants->string_literal = NULL;
	int double_number = map_get_number(literal->double_literal);
	int string_number = map_get_number(literal->string_literal);
	mvm_executable->constants->double_size = double_number;
	mvm_executable->constants->string_size = string_number;

	// double
	mvm_executable->constants->double_literal = MEM_malloc(sizeof(double) * double_number);
	Map_Iterator double_iterator = map_iterator_create(literal->double_literal);
	Map_Node_Pair double_pair = NULL;
	while ((double_pair = map_iterator_get_next(double_iterator)) != NULL)
	{
		mvm_executable->constants->double_literal[*(int*)map_node_pair_get_value(double_pair)] =
			*(double*)map_node_pair_get_key(double_pair);
	}
	map_iterator_free(double_iterator);

	// string
	mvm_executable->constants->string_literal = MEM_malloc(sizeof(char*) * string_number);
	Map_Iterator string_iterator = map_iterator_create(literal->string_literal);
	Map_Node_Pair string_pair = NULL;
	while ((string_pair = map_iterator_get_next(string_iterator)) != NULL)
	{
		mvm_executable->constants->string_literal[*(int*)map_node_pair_get_value(string_pair)] =
			string_create(map_node_pair_get_key(string_pair));
	}
	map_iterator_free(string_iterator);
}

static void load_user_input_argument(int argc, char **argv, MVM_Executable mvm_executable, Literal literal)
{
	generate_code(mvm_executable, I_IPUSH, 0, "i", argc - 1);
	generate_code(mvm_executable, I_IPUSH, 0, "i", argc - 1);
	generate_code(mvm_executable, I_NEWARRAY, 0, "i", MVM_OBJECT_TYPE + MVM_STRING_TYPE);
	for (int i = 1; i < argc; i++)
	{
		generate_code(mvm_executable, I_DUP, 0, NULL);
		generate_code(mvm_executable, I_IPUSH, 0, "i", i - 1);
		generate_code(mvm_executable, I_SPUSH, 0, "i",
			literal_get_offset(literal, argv[i], STRING_LITERAL));
		generate_code(mvm_executable, I_ASTORE, 0, NULL);
	}
}
MVM_Executable mvm_executable_generate(int argc, char **argv, Translation_Unit tu)
{
	MVM_Executable mvm_executable = mvm_executable_create();
	Literal literal = literal_create();
	// init
	mvm_executable->init->is_user_defined = TRUE;
	mvm_executable->init->function_index = -1;
	mvm_executable->init->function_name_index = -1;
	mvm_executable->init->u.user_defined.opcode_begin = mvm_executable->pc;
	generate_code(mvm_executable, I_NOP, 0, NULL);
	variable_definition_generate_code(mvm_executable, literal, tu);
	generate_code(mvm_executable, I_NPUSH, 0, NULL);
	load_user_input_argument(argc, argv, mvm_executable, literal);
	Symbol_Name main_function_name = symbol_name_create("main");
	Function_Prototype function_prototype = map_search(tu->symbol_definition->function_prototype, main_function_name);
	symbol_name_free(main_function_name);
	int main_function_index = function_prototype->function_index;
	generate_code(mvm_executable, I_INVOKE, 0, "i", main_function_index);
	generate_code(mvm_executable, I_POP, 0, NULL); // pop argv
	generate_code(mvm_executable, I_POP, 0, NULL); // pop argc
	generate_code(mvm_executable, I_POP, 0, NULL); // pop nil
	generate_code(mvm_executable, I_STOP, 0, NULL);
	mvm_executable->init->u.user_defined.opcode_end = mvm_executable->pc;
	// 
	record_definition_generate_code(mvm_executable, literal, tu);
	function_definition_generate_code(mvm_executable, literal, tu);
	//
	mvm_constant_pool_create(mvm_executable, literal);
	code_fix(mvm_executable);
	return mvm_executable;
}

void code_fix(MVM_Executable mvm_executable)
{
	int idx = 0;
	while (idx < mvm_executable->pc)
	{
		char opcode = mvm_executable->code[idx++];
		switch (opcode)
		{
			case I_NOP:
			{
				break;
			}
			case I_RPUSH:
			{
				idx += sizeof(int);
				break;
			}
			case I_NPUSH:
			{
				break;
			}
			case I_BPUSH:
			{
				idx += sizeof(int);
				break;
			}
			case I_IPUSH:
			{
				idx += sizeof(int);
				break;
			}
			case I_DPUSH:
			{
				idx += sizeof(int);
				break;
			}
			case I_SPUSH:
			{
				idx += sizeof(int);
				break;
			}
			case I_LOAD:
			{
				break;
			}
			case I_ALOAD:
			{
				break;
			}
			case I_STORE:
			{
				break;
			}
			case I_ASTORE:
			{
				break;
			}
			case I_POP:
			{
				break;
			}
			case I_DUP:
			{
				break;
			}
			case I_DUP_X1:
			{
				break;
			}
			case I_DUP2:
			{
				break;
			}
			case I_SWAP:
			{
				break;
			}
			case I_IADD:
			{
				break;
			}
			case I_DADD:
			{
				break;
			}
			case I_SADD:
			{
				break;
			}
			case I_ISUB:
			{
				break;
			}
			case I_DSUB:
			{
				break;
			}
			case I_IMUL:
			{
				break;
			}
			case I_DMUL:
			{
				break;
			}
			case I_IDIV:
			{
				break;
			}
			case I_DDIV:
			{
				break;
			}
			case I_IREM:
			{
				break;
			}
			case I_DREM:
			{
				break;
			}
			case I_INEG:
			{
				break;
			}
			case I_DNEG:
			{
				break;
			}
			case I_ICMP:
			{
				break;
			}
			case I_DCMP:
			{
				break;
			}
			case I_SCMP:
			{
				break;
			}
			case I_IFEQ:
			{
				int original_label_index = *(int*)&(mvm_executable->code[idx]);
				*(int*)&(mvm_executable->code[idx]) = mvm_executable->label[original_label_index];
				idx += sizeof(int);
				break;
			}
			case I_IFNE:
			{
				int original_label_index = *(int*)&(mvm_executable->code[idx]);
				*(int*)&(mvm_executable->code[idx]) = mvm_executable->label[original_label_index];
				idx += sizeof(int);
				break;
			}
			case I_IFLT:
			{
				int original_label_index = *(int*)&(mvm_executable->code[idx]);
				*(int*)&(mvm_executable->code[idx]) = mvm_executable->label[original_label_index];
				idx += sizeof(int);
				break;
			}
			case I_IFLE:
			{
				int original_label_index = *(int*)&(mvm_executable->code[idx]);
				*(int*)&(mvm_executable->code[idx]) = mvm_executable->label[original_label_index];
				idx += sizeof(int);
				break;
			}
			case I_IFGT:
			{
				int original_label_index = *(int*)&(mvm_executable->code[idx]);
				*(int*)&(mvm_executable->code[idx]) = mvm_executable->label[original_label_index];
				idx += sizeof(int);
				break;
			}
			case I_IFGE:
			{
				int original_label_index = *(int*)&(mvm_executable->code[idx]);
				*(int*)&(mvm_executable->code[idx]) = mvm_executable->label[original_label_index];
				idx += sizeof(int);
				break;
			}
			case I_GOTO:
			{
				int original_label_index = *(int*)&(mvm_executable->code[idx]);
				*(int*)&(mvm_executable->code[idx]) = mvm_executable->label[original_label_index];
				idx += sizeof(int);
				break;
			}
			case I_RETURN:
			{
				break;
			}
			case I_GETFIELD:
			{
				break;
			}
			case I_PUTFIELD:
			{
				break;
			}
			case I_INVOKE:
			{
				break;
			}
			case I_NEW:
			{
				break;
			}
			case I_NEWARRAY:
			{
				break;
			}
			case I_ARRAYLENGTH:
			{
				break;
			}
			case I_STOP:
			{
				break;
			}
		}
	}
}

static MVM_Value mvm_global_variable_initialization(MVM_Executable mvm_executable, Literal literal, 
	Expression exp)
{
	MVM_Value retval;
	assert(exp->type->kind == BASIC_TYPE);
	switch (exp->type->u.basic_type->kind)
	{
		case NIL_TYPE:
		{
			retval.value_type = MVM_NIL_TYPE;
			break;
		}
		case VOID_TYPE:
		{
			LOG_ERROR("the type void cannot be used as basic type");
			LOG_EXIT();
			break;
		}
		case BOOL_TYPE:
		{
			retval.value_type = MVM_BOOL_TYPE;
			retval.u.bool_value = exp->u.bool_value;
			break;
		}
		case INT_TYPE:
		{
			retval.value_type = MVM_INT_TYPE;
			retval.u.int_value = exp->u.int_value;
			break;
		}
		case DOUBLE_TYPE:
		{
			retval.value_type = MVM_DOUBLE_TYPE;
			retval.u.double_value = exp->u.double_value;
			literal_get_offset(literal, &exp->u.double_value, DOUBLE_LITERAL);
			break;
		}
		case STRING_TYPE:
		{
			//retval.value_type = MVM_OBJECT_TYPE;
			//retval.u.object_value = mvm_string_create(mvm_executable, exp->u.string_value);
			literal_get_offset(literal, exp->u.string_value, STRING_LITERAL);
			break;
		}
	}
	return retval;
}

static void global_variable_default_init_code(MVM_Executable mvm_executable, Literal literal, 
	Type type)
{
	//MVM_Value retval;
	switch (type->kind)
	{
		case BASIC_TYPE:
		{
			switch (type->u.basic_type->kind)
			{
				case NIL_TYPE:
				{	
					LOG_ERROR("the type nil cannot be used as basic type");
					LOG_EXIT();
					break;
				}
				case VOID_TYPE:
				{
					LOG_ERROR("the type void cannot be used as basic type");
					LOG_EXIT();
					break;
				}
				case BOOL_TYPE:
				{
					generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
					break;
				}
				case INT_TYPE:
				{
					generate_code(mvm_executable, I_IPUSH, 0, "i", 0);
					break;
				}
				case DOUBLE_TYPE:
				{
					double zero = 0.0;
					int offset = literal_get_offset(literal, &zero, DOUBLE_LITERAL);
					generate_code(mvm_executable, I_DPUSH, 0, "i", offset);
					break;
				}
				case STRING_TYPE:
				{
					char *empty = "";
					int offset = literal_get_offset(literal, empty, STRING_LITERAL);
					generate_code(mvm_executable, I_SPUSH, 0, "i", offset);
					break;
				}
			}
			break;
		}
		case RECORD_TYPE:
		{
			generate_code(mvm_executable, I_NPUSH, 0, NULL);
			break;
		}
		case ARRAY_TYPE:
		{
			generate_code(mvm_executable, I_NPUSH, 0, NULL);
			break;
		}
	}
}

static void global_variable_init_code(MVM_Executable mvm_executable, Literal literal, Expression expression)
{
	switch (expression->kind)
	{
		case NIL_EXPRESSION:
		{
			generate_code(mvm_executable, I_NPUSH, expression->line_number, NULL);
			break;
		}
		case BOOL_EXPRESSION:
		{
			generate_code(mvm_executable, I_BPUSH, expression->line_number,
				"i", expression->u.bool_value);
			break;
		}
		case INT_EXPRESSION:
		{
			generate_code(mvm_executable, I_IPUSH, expression->line_number,
				"i", expression->u.int_value);
			break;
		}
		case DOUBLE_EXPRESSION:
		{
			int offset = literal_get_offset(literal, &expression->u.double_value, DOUBLE_LITERAL);
			generate_code(mvm_executable, I_DPUSH, expression->line_number,
				"i", offset);
			break;
		}
		case STRING_EXPRESSION:
		{
			int offset = literal_get_offset(literal, expression->u.string_value, STRING_LITERAL);
			generate_code(mvm_executable, I_SPUSH, expression->line_number, "i", offset);
			break;
		}
	}
}

void variable_definition_generate_code(MVM_Executable mvm_executable, Literal literal, Translation_Unit tu)
{
	Definition_List definition_list = NULL;
	//mvm_executable->static_data_offset = 0;
	//mvm_executable->static_data_size = tu->symbol_definition->global_variable->symbol_number;
	//mvm_executable->stack_offset = mvm_executable->static_data_size;
	//mvm_executable->stack_size = 0;
	definition_list = tu->definition_list;
	int index = 0;
	
	while (definition_list != NULL)
	{
		if (definition_list->kind == VARIABLE_DEFINITION)
		{
			Declaration_Statement declaration_statement = definition_list->u.variable_definition->u.declaration_statement;
			Declaration_Variable_List variable_list = declaration_statement->variable_list;
			while (variable_list != NULL)
			{
				Symbol_Name variable_name = symbol_name_create(variable_list->variable_name);
				Symbol_Information variable_information =
					map_search(tu->symbol_definition->global_variable->table, variable_name);
				symbol_name_free(variable_name);
				int variable_offset = variable_information->offset;
				if (variable_list->initialization != NULL)
				{
					global_variable_init_code(mvm_executable, literal, variable_list->initialization);
				}
				else
				{
					global_variable_default_init_code(mvm_executable, literal,
						declaration_statement->type);
				}
				variable_list = variable_list->next;
			}
		}
		definition_list = definition_list->next;
	}
	//
	
	/*
	mvm_executable->bp += mvm_executable->stack_offset;
	mvm_executable->sp = mvm_executable->bp;
	*/
}


static int _mvm_compute_type_string_length(Type type)
{
	switch (type->kind)
	{
		case BASIC_TYPE:
		{
			return 1;
		}
		case RECORD_TYPE:
		{
			return 1 + strlen(type->u.record_type->record_name);
		}
		case ARRAY_TYPE:
		{
			return 1 + _mvm_compute_type_string_length(type->u.array_type);
		}
	}
	return 0;
}
static void _mvm_type_string_create(Type type, char *buf)
{
	int len = strlen(buf);
	switch (type->kind)
	{
		case BASIC_TYPE:
		{
			switch (type->u.basic_type->kind)
			{
				case NIL_TYPE:
				{
					buf[len] = 'N';
					break;
				}
				case VOID_TYPE:
				{
					buf[len] = 'V';
					break;
				}
				case BOOL_TYPE:
				{
					buf[len] = 'B';
					break;
				}
				case INT_TYPE:
				{
					buf[len] = 'I';
					break;
				}
				case DOUBLE_TYPE:
				{
					buf[len] = 'D';
					break;
				}
				case STRING_TYPE:
				{
					buf[len] = 'S';
					break;
				}
			}
			break;
		}
		case RECORD_TYPE:
		{
			buf[len] = 'L';
			strcat(buf + len + 1, type->u.record_type->record_name);
			break;
		}
		case ARRAY_TYPE:
		{
			buf[len] = '[';
			_mvm_type_string_create(type->u.array_type, buf);
			break;
		}
	}
}
static char *mvm_type_string_create(Type type)
{
	int size = _mvm_compute_type_string_length(type);
	char *type_string = MEM_malloc(sizeof(char) * (size + 1));
	memset(type_string, 0, size + 1);
	_mvm_type_string_create(type, type_string);
	return type_string;
}

static void mvm_type_string_free(char *str)
{
	MEM_free(str);
}

void record_definition_generate_code(MVM_Executable mvm_executable, Literal literal, Translation_Unit tu)
{
	int record_size = map_get_number(tu->symbol_definition->record_definition);
	mvm_executable->record_size = record_size;
	mvm_executable->records = MEM_malloc(sizeof(*mvm_executable->records) * record_size);
	Definition_List definition_list = NULL;
	definition_list = tu->definition_list;
	int record_index = 0;
	MVM_Record_Info record_info = NULL;
	while (definition_list != NULL)
	{
		if (definition_list->kind == RECORD_DEFINITION)
		{
			record_info = &(mvm_executable->records[record_index]);
			Record_Definition record_definition = definition_list->u.record_defintion;
			record_info->record_name_index = literal_get_offset(literal, 
				record_definition->record_name, STRING_LITERAL);
			record_info->field_number = map_get_number(record_definition->table->table);
			record_info->fields = MEM_malloc(sizeof(*(record_info->fields)) * record_info->field_number);
			Symbol_Name record_name = symbol_name_create(record_definition->record_name);
			Symbol_Table record_table = map_search(tu->symbol_definition->record_definition, record_name);
			symbol_name_free(record_name);
			Map_Iterator iterator = map_iterator_create(record_table->table);
			Map_Node_Pair pair = NULL;
			MVM_Field_Info field_info = NULL;
			while ((pair = map_iterator_get_next(iterator)) != NULL)
			{
				Symbol_Name field_name = map_node_pair_get_key(pair);
				Symbol_Information info = map_node_pair_get_value(pair);
				field_info = &record_info->fields[info->offset];
				char *field_type = mvm_type_string_create(info->type);
				field_info->field_name_index = literal_get_offset(literal, field_name, STRING_LITERAL);
				field_info->field_type_index = literal_get_offset(literal, field_type, STRING_LITERAL);
				mvm_type_string_free(field_type);
			}
			map_iterator_free(iterator);
			record_index++;
		}
		definition_list = definition_list->next;
	}
}

void function_definition_generate_code(MVM_Executable mvm_executable, Literal literal, Translation_Unit tu)
{
	unsigned long function_number = map_get_number(tu->symbol_definition->function_prototype);
	mvm_executable->frame_size = function_number;
	mvm_executable->frames = MEM_malloc(sizeof(*(mvm_executable->frames)) * function_number);
	Definition_List definition_list = NULL;
	definition_list = tu->definition_list;
	int function_index = 0;
	MVM_Function_Frame frame = NULL;
	// first step is to process function frame
	while (definition_list != NULL)
	{
		if (definition_list->kind == FUNCTION_DEFINITION
			&& definition_list->u.function_definition->function_kind == USER_DEFINIED_FUNCTION)
		{
			Function_Definition function_definition = definition_list->u.function_definition;
			Symbol_Name function_name = symbol_name_create(function_definition->function_name);
			Function_Prototype function_prototype = map_search(tu->symbol_definition->function_prototype, function_name);
			symbol_name_free(function_name);
			function_index = function_prototype->function_index;
			frame = &mvm_executable->frames[function_index];
			//frame->function_name = string_create(function_definition->function_name);
			frame->function_name_index = literal_get_offset(literal,
				function_definition->function_name,	STRING_LITERAL);
			frame->function_index = function_index;
			frame->is_user_defined = TRUE;
			frame->u.user_defined.opcode_begin = -1;
			frame->u.user_defined.opcode_end = -1;
		}
		else if (definition_list->kind == FUNCTION_DEFINITION
			&& definition_list->u.function_definition->function_kind == NAIVE_FUNCTION)
		{
			Function_Definition function_definition = definition_list->u.function_definition;
			Symbol_Name function_name = symbol_name_create(function_definition->function_name);
			Function_Prototype function_prototype = map_search(tu->symbol_definition->function_prototype, function_name);
			symbol_name_free(function_name);
			function_index = function_prototype->function_index;
			frame = &mvm_executable->frames[function_index];
			//frame->function_name = string_create(function_definition->function_name);
			frame->function_name_index = literal_get_offset(literal,
				function_definition->function_name, STRING_LITERAL);
			frame->function_index = function_index;
			frame->is_user_defined = FALSE;
			frame->u.naive.parameter_number = function_prototype->parameter_number;
			frame->u.naive.naive_function = NULL;
		}
		definition_list = definition_list->next;
	}

	// second step is to generate code for function
	definition_list = tu->definition_list;
	while (definition_list != NULL)
	{
		if (definition_list->kind == FUNCTION_DEFINITION
			&& definition_list->u.function_definition->function_kind == USER_DEFINIED_FUNCTION)
		{
			Function_Definition function_definition = definition_list->u.function_definition;
			Symbol_Name function_name = symbol_name_create(function_definition->function_name);
			Function_Prototype function_prototype = map_search(tu->symbol_definition->function_prototype, function_name);
			symbol_name_free(function_name);
			function_index = function_prototype->function_index;
			frame = &mvm_executable->frames[function_index];
			
			frame->u.user_defined.opcode_begin = mvm_executable->pc;
			int max_offset = local_variable_max_offset(function_definition->u.user_defined_function);
			//LOG_INFO("function name: %s, max_offset: %d\n", function_definition->function_name, max_offset);
			if (max_offset >= 0)
			{
				for (int idx = 0; idx <= max_offset; idx++)
				{
					generate_code(mvm_executable, I_NPUSH, function_definition->u.user_defined_function->line_number, NULL);
				}
			}
			block_generate_code(mvm_executable, literal, function_definition->u.user_defined_function);
			if (function_definition->u.user_defined_function->statement_list != NULL)
			{
				generate_code(mvm_executable, I_RETURN, 0, NULL);
			}
			else
			{
				generate_code(mvm_executable, I_RETURN, function_definition->line_number, NULL);;
			}
			frame->u.user_defined.opcode_end = mvm_executable->pc;
		}
		else if (definition_list->kind == FUNCTION_DEFINITION
			&& definition_list->u.function_definition->function_kind == NAIVE_FUNCTION)
		{
			
		}
		definition_list = definition_list->next;
	}
}

int local_variable_max_offset(Block block)
{
	int max_offset = -1;
	int temp = 0;
	Statement_List statement_list = block->statement_list;
	while (statement_list != NULL)
	{
		Statement stm = statement_list->statement;
		switch (stm->kind)
		{
			case EXPRESSION_STATEMENT:
			{
				break;
			}
			case IF_STATEMENT:
			{
				If_Statement if_statement = stm->u.if_statement;
				while (TRUE)
				{
					temp = local_variable_max_offset(if_statement->then_block);
					if (max_offset < temp)
					{
						max_offset = temp;
					}
					if (if_statement->kind == IF_WITHOUT_ELSE)
					{
						break;
					}
					else if (if_statement->kind == IF_WITH_ELSE)
					{
						temp = local_variable_max_offset(if_statement->u.else_block);
						if (max_offset < temp)
						{
							max_offset = temp;
						}
						break;
					}
					else if (if_statement->kind == IF_WITH_ELSE_IF)
					{
						if_statement = if_statement->u.else_if_statement->u.if_statement;
					}
				}
				break;
			}
			case DO_WHILE_STATEMENT:
			{
				temp = local_variable_max_offset(stm->u.do_while_statement->block);
				if (max_offset < temp)
				{
					max_offset = temp;
				}
				break;
			}
			case WHILE_STATEMENT:
			{
				temp = local_variable_max_offset(stm->u.while_statement->block);
				if (max_offset < temp)
				{
					max_offset = temp;
				}
				break;
			}
			case FOR_STATEMENT:
			{
				temp = local_variable_max_offset(stm->u.for_statement->block);
				if (max_offset < temp)
				{
					max_offset = temp;
				}
				break;
			}
			case RETURN_STATEMENT:
			{
				break;
			}
			case BREAK_STATEMENT:
			{
				break;
			}
			case CONTINUE_STATEMENT:
			{
				break;
			}
			case DECLARATION_STATEMENT:
			{
				Declaration_Variable_List variable_list = stm->u.declaration_statement->variable_list;
				while (variable_list != NULL)
				{
					Symbol_Name variable_name = symbol_name_create(variable_list->variable_name);
					Symbol_Information variable_information =
						variable_information = map_search(block->block_table->table, variable_name);
					symbol_name_free(variable_name);
					if (max_offset < variable_information->offset)
					{
						max_offset = variable_information->offset;
					}
					variable_list = variable_list->next;
				}
				break;
			}
			case BLOCK:
			{
				temp = local_variable_max_offset(stm->u.block);
				if (max_offset < temp)
				{
					max_offset = temp;
				}
				break;
			}
		}
		statement_list = statement_list->next;
	}
	return max_offset;
}

void identifier_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal, 
	Block block, Expression expression)
{
	Symbol_Name variable_name = symbol_name_create(expression->u.identifier_expression->name);
	Symbol_Table table = block->block_table;
	Symbol_Information variable_info = NULL;
	while (TRUE)
	{
		variable_info = map_search(table->table, variable_name);
		if (variable_info != NULL)
		{
			symbol_name_free(variable_name);
			break;
		}
		else
		{
			table = table->parent;
		}
	}
	assert(variable_info != NULL);
	if (table->parent != NULL)
	{
		// not global variable
		generate_code(mvm_executable, I_RPUSH, expression->line_number, "i", MVM_BP);
		generate_code(mvm_executable, I_IPUSH, 0, "i", variable_info->offset);
		generate_code(mvm_executable, I_IADD, 0, NULL);
	}
	else
	{
		// global variable
		generate_code(mvm_executable, I_IPUSH, expression->line_number, "i", variable_info->offset);
	}
}

int mvm_find_function_index(MVM_Executable mvm_executable, Literal literal, char *function_name)
{
	int offset = literal_get_offset(literal, function_name, STRING_LITERAL);
	for (int i = 0; i < mvm_executable->frame_size; i++)
	{
		if (mvm_executable->frames[i].function_name_index == offset)
		{
			return i;
		}
	}
	return -1;
}
void function_call_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal, Block block, Expression expression)
{
	generate_code(mvm_executable, I_NPUSH, expression->line_number, 0, NULL);
	int argument_number = 0;
	Argument_List argument_list = expression->u.function_call_expression->argument_list;
	while (argument_list != NULL)
	{
		argument_number++;
		expression_generate_code(mvm_executable, literal, block, argument_list->expression);
		argument_list = argument_list->next;
	}
	int function_index = mvm_find_function_index(mvm_executable, literal,
		expression->u.function_call_expression->function_name);
	generate_code(mvm_executable, I_INVOKE, 0, "i", function_index);
	for (int i = 0; i < argument_number; i++)
	{
		generate_code(mvm_executable, I_POP, 0, NULL);
	}
	/*
	if (is_basic_type(expression->type, VOID_TYPE) == TRUE)
	{
		mvm_executable->code[mvm_postfix_increase_pc(mvm_executable)] = I_POP;
	}
	*/
}

int mvm_find_record_type_index(MVM_Executable mvm_executable, Literal literal, char *record_name)
{
	int offset = literal_get_offset(literal, record_name, STRING_LITERAL);
	for (int i = 0; i < mvm_executable->record_size; i++)
	{
		if (mvm_executable->records[i].record_name_index == offset)
		{
			return i;
		}
	}
	return -1;
}

int mvm_find_record_field_index(MVM_Executable mvm_executable, Literal literal, MVM_Record_Info record_info, char *field_name)
{
	int offset = literal_get_offset(literal, field_name, STRING_LITERAL);
	for (int i = 0; i < record_info->field_number; i++)
	{
		if (record_info->fields[i].field_name_index == offset)
		{
			return i;
		}
	}
	return -1;
}

void field_access_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal, Block block, Expression expression)
{
	expression_generate_code(mvm_executable, literal, block, expression->u.field_access_expression->record_name);
	Type type = expression->u.field_access_expression->record_name->type;
	int record_type_index = mvm_find_record_type_index(mvm_executable, literal,
		type->u.record_type->record_name);
	MVM_Record_Info record_info = &(mvm_executable->records[record_type_index]);
	int record_field_index = mvm_find_record_field_index(mvm_executable, literal, record_info,
		expression->u.field_access_expression->field_name->u.identifier_expression->name);
	generate_code(mvm_executable, I_IPUSH,
		expression->u.field_access_expression->field_name->line_number, 
		"i", record_field_index);
}

void array_index_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal, Block block, Expression expression)
{
	expression_generate_code(mvm_executable, literal, block, expression->u.array_index_expression->array_name);
	expression_generate_code(mvm_executable, literal, block, expression->u.array_index_expression->index);
}

void assign_expression_generate_code_assist(MVM_Executable mvm_executable, Literal literal, Block block, Expression expression)
{
	Expression iterator_exp = expression;
	Expression lvalue_exp = NULL;
	if (iterator_exp->kind == ASSIGN_EXPRESSION)
	{
		assign_expression_generate_code_assist(mvm_executable, literal, block,
			iterator_exp->u.assign_expression->right);
		lvalue_exp = iterator_exp->u.assign_expression->left;
		switch (lvalue_exp->kind)
		{
			case IDENTIFIER_EXPRESSION:
			{
				generate_code(mvm_executable, I_DUP_X1, lvalue_exp->line_number, NULL);
				generate_code(mvm_executable, I_STORE, 0, NULL);
				break;
			}
			case FIELD_ACCESS_EXPRESSION:
			{
				generate_code(mvm_executable, I_DUP_X2, 0, NULL);
				generate_code(mvm_executable, I_PUTFIELD, 0, NULL);
				break;
			}
			case ARRAY_INDEX_EXPRESSION:
			{
				generate_code(mvm_executable, I_DUP_X2, lvalue_exp->line_number, NULL);
				generate_code(mvm_executable, I_ASTORE, 0, NULL);
				break;
			}
		}
	}
	else
	{
		expression_generate_code(mvm_executable, literal, block, iterator_exp);
	}
}
void assign_expression_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Expression expression)
{
	Expression iterator_exp = expression;
	Expression lvalue_exp = NULL;
	while (iterator_exp->kind == ASSIGN_EXPRESSION)
	{
		lvalue_exp = iterator_exp->u.binary_expression->left;
		switch (lvalue_exp->kind)
		{
			case IDENTIFIER_EXPRESSION:
			{
				identifier_expression_generate_code_assist(mvm_executable, literal, block, lvalue_exp);
				break;
			}
			case FIELD_ACCESS_EXPRESSION:
			{
				field_access_expression_generate_code_assist(mvm_executable, literal, block, lvalue_exp);
				break;
			}
			case ARRAY_INDEX_EXPRESSION:
			{
				array_index_expression_generate_code_assist(mvm_executable, literal, block, lvalue_exp);
				break;
			}
		}
		iterator_exp = iterator_exp->u.binary_expression->right;
	}
	// generate store code
	assign_expression_generate_code_assist(mvm_executable, literal, block, expression);
}
void expression_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Expression expression)
{
	switch (expression->kind)
	{
		case COMMA_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, expression->u.comma_expression->left);
			generate_code(mvm_executable, I_POP, 0, NULL);
			expression_generate_code(mvm_executable, literal, block, expression->u.comma_expression->right);
			break;
		}
		case ASSIGN_EXPRESSION:
		{
			assign_expression_generate_code(mvm_executable, literal, block, expression);
			break;
		}
		case OR_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->left);
			int ne_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_IFNE, 0, "i", ne_label_index);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			generate_code(mvm_executable, I_IFNE, 0, "i", ne_label_index);
			generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
			int end_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_GOTO, 0, "i", end_label_index);
			mvm_executable->label[ne_label_index] = mvm_executable->pc;
			generate_code(mvm_executable, I_BPUSH, 0, "i", TRUE);
			mvm_executable->label[end_label_index] = mvm_executable->pc;
			break;
		}
		case AND_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->left);
			int eq_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_IFEQ, 0, "i", eq_label_index);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			generate_code(mvm_executable, I_IFEQ, 0, "i", eq_label_index);
			generate_code(mvm_executable, I_BPUSH, 0, "i", TRUE);
			int end_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_GOTO, 0, "i", end_label_index);
			mvm_executable->label[eq_label_index] = mvm_executable->pc;
			generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
			mvm_executable->label[end_label_index] = mvm_executable->pc;
			break;
		}
		case EQ_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, STRING_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_SCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE)
				|| is_basic_type(expression->u.binary_expression->left->type, BOOL_TYPE))
			{
				generate_code(mvm_executable, I_ICMP, 0, NULL);
			}
			int ne_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_IFNE, 0, "i", ne_label_index);
			generate_code(mvm_executable, I_BPUSH, 0, "i", TRUE);
			int end_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_GOTO, 0, "i", end_label_index);
			mvm_executable->label[ne_label_index] = mvm_executable->pc;
			generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
			mvm_executable->label[end_label_index] = mvm_executable->pc;
			break;
		}
		case NE_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, STRING_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_SCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE)
				|| is_basic_type(expression->u.binary_expression->left->type, BOOL_TYPE))
			{
				generate_code(mvm_executable, I_ICMP, 0, NULL);
			}
			int eq_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_IFEQ, 0, "i", eq_label_index);
			generate_code(mvm_executable, I_BPUSH, 0, "i", TRUE);
			int end_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_GOTO, 0, "i", end_label_index);
			mvm_executable->label[eq_label_index] = mvm_executable->pc;
			generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
			mvm_executable->label[end_label_index] = mvm_executable->pc;
			break;
		}
		case GT_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, STRING_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_SCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE)
				|| is_basic_type(expression->u.binary_expression->left->type, BOOL_TYPE))
			{
				generate_code(mvm_executable, I_ICMP, 0, NULL);
			}
			int le_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_IFLE, 0, "i", le_label_index);
			generate_code(mvm_executable, I_BPUSH, 0, "i", TRUE);
			int end_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_GOTO, 0, "i", end_label_index);
			mvm_executable->label[le_label_index] = mvm_executable->pc;
			generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
			mvm_executable->label[end_label_index] = mvm_executable->pc;
			break;
		}
		case GE_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, STRING_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_SCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE)
				|| is_basic_type(expression->u.binary_expression->left->type, BOOL_TYPE))
			{
				generate_code(mvm_executable, I_ICMP, 0, NULL);
			}
			int lt_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_IFLT, 0, "i", lt_label_index);
			generate_code(mvm_executable, I_BPUSH, 0, "i", TRUE);
			int end_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_GOTO, 0, "i", end_label_index);
			mvm_executable->label[lt_label_index] = mvm_executable->pc;
			generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
			mvm_executable->label[end_label_index] = mvm_executable->pc;
			break;
		}
		case LT_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, STRING_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_SCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE)
				|| is_basic_type(expression->u.binary_expression->left->type, BOOL_TYPE))
			{
				generate_code(mvm_executable, I_ICMP, 0, NULL);
			}
			int ge_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_IFGE, 0, "i", ge_label_index);
			generate_code(mvm_executable, I_BPUSH, 0, "i", TRUE);
			int end_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_GOTO, 0, "i", end_label_index);
			mvm_executable->label[ge_label_index] = mvm_executable->pc;
			generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
			mvm_executable->label[end_label_index] = mvm_executable->pc;
			break;
		}
		case LE_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, STRING_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_SCMP, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE)
				|| is_basic_type(expression->u.binary_expression->left->type, BOOL_TYPE))
			{
				generate_code(mvm_executable, I_ICMP, 0, NULL);
			}
			int gt_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_IFGT, 0, "i", gt_label_index);
			generate_code(mvm_executable, I_BPUSH, 0, "i", TRUE);
			int end_label_index = mvm_next_label_index(mvm_executable);
			generate_code(mvm_executable, I_GOTO, 0, "i", end_label_index);
			mvm_executable->label[gt_label_index] = mvm_executable->pc;
			generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
			mvm_executable->label[end_label_index] = mvm_executable->pc;
			break;
		}
		case ADD_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DADD, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, STRING_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_SADD, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_IADD, 0, NULL);
			}
			break;
		}
		case SUB_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DSUB, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_ISUB, 0, NULL);
			}
			break;
		}
		case MUL_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DMUL, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_IMUL, 0, NULL);
			}
			break;
		}
		case DIV_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DDIV, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_IDIV, 0, NULL);
			}
			break;
		}
		case MOD_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block,
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DREM, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_IREM, 0, NULL);
			}
			break;
		}
		case MINUS_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->left);
			expression_generate_code(mvm_executable, literal, block, 
				expression->u.binary_expression->right);
			if (is_basic_type(expression->u.binary_expression->left->type, DOUBLE_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_DNEG, 0, NULL);
			}
			else if (is_basic_type(expression->u.binary_expression->left->type, INT_TYPE) == TRUE)
			{
				generate_code(mvm_executable, I_INEG, 0, NULL);
			}
			break;
		}
		case NOT_EXPRESSION:
		{
			break;
		}
		case INC_EXPRESSION:
		{
			switch (expression->u.inc_or_dec_expression->kind)
			{
				case IDENTIFIER_EXPRESSION:
				{
					identifier_expression_generate_code_assist(mvm_executable, literal, block,
						expression->u.inc_or_dec_expression);
					generate_code(mvm_executable, I_DUP, 0, NULL);
					generate_code(mvm_executable, I_LOAD, 0, NULL);
					generate_code(mvm_executable, I_IPUSH, 0, "i", 1);
					generate_code(mvm_executable, I_IADD, 0, NULL);
					generate_code(mvm_executable, I_DUP_X1, 0, NULL);
					generate_code(mvm_executable, I_STORE, 0, NULL);
					break;
				}
				case FIELD_ACCESS_EXPRESSION:
				{
					field_access_expression_generate_code_assist(mvm_executable, literal, block,
						expression->u.inc_or_dec_expression);
					generate_code(mvm_executable, I_DUP2, 0, NULL);
					generate_code(mvm_executable, I_GETFIELD, 0, NULL);
					generate_code(mvm_executable, I_IPUSH, 0, "i", 1);
					generate_code(mvm_executable, I_IADD, 0, NULL);
					generate_code(mvm_executable, I_DUP_X2, 0, NULL);
					generate_code(mvm_executable, I_PUTFIELD, 0, NULL);
					break;
				}
				case ARRAY_INDEX_EXPRESSION:
				{
					array_index_expression_generate_code_assist(mvm_executable, literal, block,
						expression->u.inc_or_dec_expression);
					generate_code(mvm_executable, I_DUP2, 0, NULL);
					generate_code(mvm_executable, I_ALOAD, 0, NULL);
					generate_code(mvm_executable, I_IPUSH, 0, "i", 1);
					generate_code(mvm_executable, I_IADD, 0, NULL);
					generate_code(mvm_executable, I_DUP_X2, 0, NULL);
					generate_code(mvm_executable, I_ASTORE, 0, NULL);
					break;
				}
			}
			break;
		}
		case DEC_EXPRESSION:
		{
			switch (expression->u.inc_or_dec_expression->kind)
			{
				case IDENTIFIER_EXPRESSION:
				{
					identifier_expression_generate_code_assist(mvm_executable, literal, block,
						expression->u.inc_or_dec_expression);
					generate_code(mvm_executable, I_DUP, 0, NULL);
					generate_code(mvm_executable, I_LOAD, 0, NULL);
					generate_code(mvm_executable, I_IPUSH, 0, "i", 1);
					generate_code(mvm_executable, I_ISUB, 0, NULL);
					generate_code(mvm_executable, I_DUP_X1, 0, NULL);
					generate_code(mvm_executable, I_STORE, 0, NULL);
					break;
				}
				case FIELD_ACCESS_EXPRESSION:
				{
					field_access_expression_generate_code_assist(mvm_executable, literal, block,
						expression->u.inc_or_dec_expression);
					generate_code(mvm_executable, I_DUP2, 0, NULL);
					generate_code(mvm_executable, I_GETFIELD, 0, NULL);
					generate_code(mvm_executable, I_IPUSH, 0, "i", 1);
					generate_code(mvm_executable, I_ISUB, 0, NULL);
					generate_code(mvm_executable, I_DUP_X2, 0, NULL);
					generate_code(mvm_executable, I_PUTFIELD, 0, NULL);
					break;
				}
				case ARRAY_INDEX_EXPRESSION:
				{
					array_index_expression_generate_code_assist(mvm_executable, literal, block,
						expression->u.inc_or_dec_expression);
					generate_code(mvm_executable, I_DUP2, 0, NULL);
					generate_code(mvm_executable, I_ALOAD, 0, NULL);
					generate_code(mvm_executable, I_IPUSH, 0, "i", 1);
					generate_code(mvm_executable, I_ISUB, 0, NULL);
					generate_code(mvm_executable, I_DUP_X2, 0, NULL);
					generate_code(mvm_executable, I_ASTORE, 0, NULL);
					break;
				}
			}
			break;
		}
		case IDENTIFIER_EXPRESSION:
		{
			identifier_expression_generate_code_assist(mvm_executable, literal, block, expression);
			generate_code(mvm_executable, I_LOAD, 0, NULL);
			break;
		}
		case FUNCTION_CALL_EXPRESSION:
		{
			function_call_expression_generate_code_assist(mvm_executable, literal, block, expression);
			break;
		}
		case FIELD_ACCESS_EXPRESSION:
		{
			field_access_expression_generate_code_assist(mvm_executable, literal, block, expression);
			generate_code(mvm_executable, I_GETFIELD, 0, NULL);
			break;
		}
		case ARRAY_INDEX_EXPRESSION:
		{
			array_index_expression_generate_code_assist(mvm_executable, literal, block, expression);
			generate_code(mvm_executable, I_ALOAD, 0, NULL);
			break;
		}
		case ARRAY_LITERAL_EXPRESSION:
		{
			/////////////////////////////////////////////////////////////////////////
			break;
		}
		case NEW_EXPRESSION:
		{
			new_expression_generate_code(mvm_executable, literal, block, expression);
			break;
		}
		case NIL_EXPRESSION:
		{
			generate_code(mvm_executable, I_NPUSH, expression->line_number, NULL);
			break;
		}
		case BOOL_EXPRESSION:
		{
			generate_code(mvm_executable, I_BPUSH, expression->line_number,
				"i", expression->u.bool_value);
			break;
		}
		case INT_EXPRESSION:
		{
			generate_code(mvm_executable, I_IPUSH, expression->line_number,
				"i", expression->u.int_value);
			break;
		}
		case DOUBLE_EXPRESSION:
		{
			int offset = literal_get_offset(literal, &expression->u.double_value, DOUBLE_LITERAL);
			generate_code(mvm_executable, I_DPUSH, expression->line_number, "i", offset);
			break;
		}
		case STRING_EXPRESSION:
		{
			int offset = literal_get_offset(literal, expression->u.string_value, STRING_LITERAL);
			generate_code(mvm_executable, I_SPUSH, expression->line_number, "i", offset);
			break;
		}
		case PARENTHESIS_EXPRESSION:
		{
			expression_generate_code(mvm_executable, literal, block, expression->u.parenthesis_expression);
			break;
		}
	}
}

void new_expression_generate_code(MVM_Executable mvm_executable, Literal literal, 
	Block block, Expression expression)
{
	Dimension_List dimension_list = expression->u.new_expression->dimension_list;
	int dimensions = 0;
	while (dimension_list != NULL)
	{
		dimensions++;
		dimension_list = dimension_list->next_dimension;
	}
	dimension_list = expression->u.new_expression->dimension_list;
	if (dimensions == 0)
	{
		// new record_index
		generate_code(mvm_executable, I_NEW, expression->line_number, "i",
			mvm_find_record_type_index(mvm_executable, literal,
			expression->u.new_expression->u.record_type->record_name));
	}
	else if (dimensions == 1)
	{
		expression_generate_code(mvm_executable, literal, block, dimension_list->dimension_expression);
		// newarray value_type(int value)
		if (expression->u.new_expression->type_kind == BASIC_TYPE)
		{
			switch (expression->u.new_expression->u.basic_type->kind)
			{
				case NIL_TYPE:
				{
					LOG_ERROR("cannt new NIL type\n");
					LOG_EXIT();
					break;
				}
				case VOID_TYPE:
				{
					LOG_ERROR("cannot new VOID type\n");
					LOG_EXIT();
					break;
				}
				case BOOL_TYPE:
				{
					generate_code(mvm_executable, I_NEWARRAY, expression->line_number,
						"i", MVM_BOOL_TYPE);
					break;
				}
				case INT_TYPE:
				{
					generate_code(mvm_executable, I_NEWARRAY, expression->line_number,
						"i", MVM_INT_TYPE);
					break;
				}
				case DOUBLE_TYPE:
				{
					generate_code(mvm_executable, I_NEWARRAY, expression->line_number,
						"i", MVM_DOUBLE_TYPE);
					break;
				}
				case STRING_TYPE:
				{
					generate_code(mvm_executable, I_NEWARRAY, expression->line_number,
						"i", MVM_OBJECT_TYPE + MVM_STRING_TYPE);
					break;
				}
			}
		}
		else if (expression->u.new_expression->type_kind == RECORD_TYPE)
		{
			generate_code(mvm_executable, I_NEWARRAY, expression->line_number,
				"i", MVM_OBJECT_TYPE + MVM_RECORD_TYPE);
		}		
	}
	else
	{
		// multinewarray value_type(int value) dimensions(int value)
		while (dimension_list != NULL)
		{
			expression_generate_code(mvm_executable, literal, block, dimension_list->dimension_expression);
			dimension_list = dimension_list->next_dimension;
		}
		if (expression->u.new_expression->type_kind == BASIC_TYPE)
		{
			switch (expression->u.new_expression->u.basic_type->kind)
			{
				case NIL_TYPE:
				{
					LOG_ERROR("cannot new NIL type\n");
					LOG_EXIT();
					break;
				}
				case VOID_TYPE:
				{
					LOG_ERROR("cannot new VOID type\n");
					LOG_EXIT();
					break;
				}
				case BOOL_TYPE:
				{
					generate_code(mvm_executable, I_MULTINEWARRAY, expression->line_number, 
						"ii", MVM_BOOL_TYPE, dimensions);
					break;
				}
				case INT_TYPE:
				{
					generate_code(mvm_executable, I_MULTINEWARRAY, expression->line_number,
						"ii", MVM_INT_TYPE, dimensions);
					break;
				}
				case DOUBLE_TYPE:
				{
					generate_code(mvm_executable, I_MULTINEWARRAY, expression->line_number,
						"ii", MVM_DOUBLE_TYPE, dimensions);
					break;
				}
				case STRING_TYPE:
				{
					generate_code(mvm_executable, I_MULTINEWARRAY, expression->line_number,
						"ii", MVM_OBJECT_TYPE + MVM_STRING_TYPE, dimensions);
					break;
				}
			}
		}
		else if (expression->u.new_expression->type_kind == RECORD_TYPE)
		{
			generate_code(mvm_executable, I_MULTINEWARRAY, expression->line_number,
				"ii", MVM_OBJECT_TYPE + MVM_RECORD_TYPE, dimensions);
		}
	}
}
void declaration_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block,
	Declaration_Statement declaration_statement)
{
	Declaration_Variable_List variable_list = declaration_statement->variable_list;
	while (variable_list != NULL)
	{
		Symbol_Name variable_name = symbol_name_create(variable_list->variable_name);
		Symbol_Information variable_information = map_search(block->block_table->table,
			variable_name);
		symbol_name_free(variable_name);
		int variable_offset = variable_information->offset;
		generate_code(mvm_executable, I_RPUSH, declaration_statement->line_number,
			"i", MVM_BP);
		generate_code(mvm_executable, I_IPUSH, 0, "i", variable_offset);
		generate_code(mvm_executable, I_IADD, 0, NULL);
		if (variable_list->initialization != NULL)
		{
			expression_generate_code(mvm_executable, literal, block, variable_list->initialization);
		}
		else
		{			
			//mvm_executable->code[mvm_postfix_increase_pc(mvm_executable)] = 
			switch (declaration_statement->type->kind)
			{
				case BASIC_TYPE:
				{
					switch (declaration_statement->type->u.basic_type->kind)
					{
						case NIL_TYPE:
						{
							generate_code(mvm_executable, I_NPUSH, 0, NULL);
							break;
						}
						case VOID_TYPE:
						{
							LOG_ERROR("the type void cannot be used as basic type");
							LOG_EXIT();
							break;
						}
						case BOOL_TYPE:
						{
							generate_code(mvm_executable, I_BPUSH, 0, "i", FALSE);
							break;
						}
						case INT_TYPE:
						{
							generate_code(mvm_executable, I_IPUSH, 0, "i", 0);
							break;
						}
						case DOUBLE_TYPE:
						{
							double zero_double_key = 0.0;
							generate_code(mvm_executable, I_DPUSH, 0, "i", literal_get_offset(literal,
								&zero_double_key, DOUBLE_LITERAL));
							break;
						}
						case STRING_TYPE:
						{
							char *empty_string_key = "";
							generate_code(mvm_executable, I_SPUSH, 0, "i", literal_get_offset(literal,
								&empty_string_key, STRING_LITERAL));
							break;
						}
					}
					break;
				}
				case RECORD_TYPE:
				{
					generate_code(mvm_executable, I_NPUSH, 0, NULL);
					break;
				}
				case ARRAY_TYPE:
				{
					generate_code(mvm_executable, I_NPUSH, 0, NULL);
					break;
				}
			}
		}
		generate_code(mvm_executable, I_STORE, 0, NULL);
		variable_list = variable_list->next;
	}
}

void if_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm)
{
	If_Statement if_statement = stm->u.if_statement;
	if_statement->else_label_index = mvm_next_label_index(mvm_executable);
	if_statement->end_label_index = mvm_next_label_index(mvm_executable);
	expression_generate_code(mvm_executable, literal, block, if_statement->condition);
	generate_code(mvm_executable, I_IFEQ, 0, "i", if_statement->else_label_index);
	block_generate_code(mvm_executable, literal, if_statement->then_block);
	generate_code(mvm_executable, I_GOTO, 0, "i", if_statement->end_label_index);
	mvm_executable->label[if_statement->else_label_index] = mvm_executable->pc;

	if (if_statement->kind == IF_WITHOUT_ELSE)
	{
		mvm_executable->label[if_statement->end_label_index] = mvm_executable->pc;
		return;
	}
	else if (if_statement->kind == IF_WITH_ELSE)
	{
		block_generate_code(mvm_executable, literal, if_statement->u.else_block);
		generate_code(mvm_executable, I_GOTO, 0, "i", if_statement->end_label_index);
		mvm_executable->label[if_statement->end_label_index] = mvm_executable->pc;
		return;
	}
	else if (if_statement->kind == IF_WITH_ELSE_IF)
	{
		if_statement_generate_code(mvm_executable, literal, block, if_statement->u.else_if_statement);
		mvm_executable->label[if_statement->end_label_index] = 
			mvm_executable->label[if_statement->u.else_if_statement->u.if_statement->end_label_index];
	}
}
	void do_while_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm)
{
	Do_While_Statement do_while_statement = stm->u.do_while_statement;
	do_while_statement->begin_label_index = mvm_next_label_index(mvm_executable);
	do_while_statement->end_label_index = mvm_next_label_index(mvm_executable);
	mvm_executable->label[do_while_statement->begin_label_index] = mvm_executable->pc;
	block_generate_code(mvm_executable, literal, do_while_statement->block);
	expression_generate_code(mvm_executable, literal, block, do_while_statement->condition);
	generate_code(mvm_executable, I_IFNE, 0, "i", do_while_statement->begin_label_index);
	mvm_executable->label[do_while_statement->end_label_index] = mvm_executable->pc;
}

void while_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm)
{
	While_Statement while_statement = stm->u.while_statement;
	while_statement->begin_label_index = mvm_next_label_index(mvm_executable);
	while_statement->end_label_index = mvm_next_label_index(mvm_executable);
	mvm_executable->label[while_statement->begin_label_index] = mvm_executable->pc;
	expression_generate_code(mvm_executable, literal, block, while_statement->condition);
	generate_code(mvm_executable, I_IFEQ, 0, "i", while_statement->end_label_index);
	block_generate_code(mvm_executable, literal, while_statement->block);
	generate_code(mvm_executable, I_GOTO, 0, "i", while_statement->begin_label_index);
	mvm_executable->label[while_statement->end_label_index] = mvm_executable->pc;
}

void for_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm)
{
	For_Statement for_statement = stm->u.for_statement;
	for_statement->condition_label_index = mvm_next_label_index(mvm_executable);
	for_statement->post_label_index = mvm_next_label_index(mvm_executable);
	for_statement->end_label_index = mvm_next_label_index(mvm_executable);
	expression_generate_code(mvm_executable, literal, block, for_statement->init);
	generate_code(mvm_executable, I_POP, 0, NULL);
	mvm_executable->label[for_statement->condition_label_index] = mvm_executable->pc;
	expression_generate_code(mvm_executable, literal, block, for_statement->condition);
	generate_code(mvm_executable, I_IFEQ, 0, "i", for_statement->end_label_index);
	block_generate_code(mvm_executable, literal, for_statement->block);
	mvm_executable->label[for_statement->post_label_index] = mvm_executable->pc;
	expression_generate_code(mvm_executable, literal, block, for_statement->post);
	generate_code(mvm_executable, I_POP, 0, NULL);
	generate_code(mvm_executable, I_GOTO, 0, "i", for_statement->condition_label_index);
	mvm_executable->label[for_statement->end_label_index] = mvm_executable->pc;
}

void break_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm)
{
	Break_Statement break_statement = stm->u.break_statement;
	Block iterator = block;
	while (iterator != NULL)
	{
		if (iterator->block_type == FOR_BLOCK)
		{
			generate_code(mvm_executable, I_GOTO, stm->line_number, 
				"i", iterator->u.for_statement->end_label_index);
			break;
		}
		else if (iterator->block_type == DO_WHILE_BLOCK)
		{
			generate_code(mvm_executable, I_GOTO, stm->line_number,
				"i", iterator->u.do_while_statement->end_label_index);
			return;
		}
		else if (iterator->block_type == WHILE_BLOCK)
		{
			generate_code(mvm_executable, I_GOTO, stm->line_number,
				"i", iterator->u.while_statement->end_label_index);
			return;
		}
		else
		{
			iterator = iterator->outer;
		}
	}
}
void continue_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm)
{
	Continue_Statement continue_statement = stm->u.continue_statement;
	Block iterator = block;
	while (iterator != NULL)
	{
		if (iterator->block_type == FOR_BLOCK)
		{
			generate_code(mvm_executable, I_GOTO, stm->line_number, 
				"i", iterator->u.for_statement->post_label_index);
			break;
		}
		else if (iterator->block_type == DO_WHILE_BLOCK)
		{
			generate_code(mvm_executable, I_GOTO, stm->line_number,
				"i", iterator->u.do_while_statement->begin_label_index);
			break;
		}
		else if (iterator->block_type == WHILE_BLOCK)
		{
			generate_code(mvm_executable, I_GOTO, stm->line_number,
				"i", iterator->u.while_statement->begin_label_index);
			break;
		}
		else
		{
			iterator = iterator->outer;
		}

	}
}
void return_statement_generate_code(MVM_Executable mvm_executable, Literal literal, Block block, Statement stm)
{
	Return_Statement return_statement = stm->u.return_statement;
	if (return_statement->retval != NULL)
	{
		Block iterator = block;
		while (iterator->block_type != FUNCTION_BLOCK)
		{
			iterator = iterator->outer;
		}
		int parameters = 0;
		Parameter_List parameter_list = iterator->u.function_definition->parameter_list;
		while (parameter_list != NULL)
		{
			parameters++;
			parameter_list = parameter_list->next;
		}
		generate_code(mvm_executable, I_RPUSH, return_statement->retval->line_number,
			"i", MVM_BP);
		generate_code(mvm_executable, I_IPUSH, 0, "i", (-3 - parameters));
		generate_code(mvm_executable, I_IADD, 0, NULL);
		expression_generate_code(mvm_executable, literal, block, return_statement->retval);
		generate_code(mvm_executable, I_STORE, 0, NULL);
	}	
	generate_code(mvm_executable, I_RETURN, stm->line_number, NULL);
}

void block_generate_code(MVM_Executable mvm_executable, Literal literal, Block block)
{
	Statement_List statement_list = block->statement_list;
	while (statement_list != NULL)
	{
		Statement stm = statement_list->statement;
		switch (stm->kind)
		{
			case EXPRESSION_STATEMENT:
			{
				expression_generate_code(mvm_executable, literal, block, stm->u.expression_statement);
				generate_code(mvm_executable, I_POP, 0, NULL);
				break;
			}
			case IF_STATEMENT:
			{
				if_statement_generate_code(mvm_executable, literal, block, stm);
				break;
			}
			case DO_WHILE_STATEMENT:
			{
				do_while_statement_generate_code(mvm_executable, literal, block, stm);
				break;
			}
			case WHILE_STATEMENT:
			{
				while_statement_generate_code(mvm_executable, literal, block, stm);
				break;
			}
			case FOR_STATEMENT:
			{
				for_statement_generate_code(mvm_executable, literal, block, stm);
				break;
			}
			case RETURN_STATEMENT:
			{
				return_statement_generate_code(mvm_executable, literal, block, stm);
				break;
			}
			case BREAK_STATEMENT:
			{
				break_statement_generate_code(mvm_executable, literal, block, stm);
				break;
			}
			case CONTINUE_STATEMENT:
			{
				continue_statement_generate_code(mvm_executable, literal, block, stm);
				break;
			}
			case DECLARATION_STATEMENT:
			{
				declaration_statement_generate_code(mvm_executable, literal, block, stm->u.declaration_statement);
				break;
			}
			case BLOCK:
			{
				block_generate_code(mvm_executable, literal, stm->u.block);
				break;
			}
		}
		statement_list = statement_list->next;
	}
}