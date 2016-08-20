#include "environment.h"
#include "mvm.h"
#include "memory.h"
#include "tool.h"
#include "constant.h"
#include "LOG.h"
#include "mvm_naive_function.h"

#define MVM_GC_FREQUENCY (2)

char* opcode_info[] = {
	"nop",
	"rpush",
	"npush",
	"bpush",
	"ipush",
	"dpush",
	"spush",
	"load",
	"aload",
	"store",
	"astore",
	"pop",
	"dup",
	"dup_x1",
	"dup_x2",
	"dup2",
	"dup2_x1",
	"dup2_x2",
	"swap",
	"iadd",
	"dadd",
	"sadd",
	"isub",
	"dsub",
	"imul",
	"dmul",
	"idiv",
	"ddiv",
	"irem",
	"drem",
	"ineg",
	"dneg",
	"icmp",
	"dcmp",
	"scmp",
	"ifeq",
	"ifne",
	"iflt",
	"ifle",
	"ifgt",
	"ifge",
	"goto",
	"return",
	"getfield",
	"putfield",
	"invoke",
	"new",
	"newarray",
	"multinewarray",
	"arraylenth",
	"stop",
};

char* mvm_register_info[] =
{
	"bp",
	"sp",
	"pc",
};
char *mvm_type_info[] =
{
	"nil",
	"bool",
	"int",
	"double",
	"string",
	"record",
	"array",
};

static struct MVM_Naive_Function_tag mvm_naive_functions[] =
{
	{ "print", mvm_naive_function_print },
	{ "itos", mvm_naive_function_int_to_string },
	{ "dtos", mvm_naive_function_double_to_string },
	{ "btos", mvm_naive_function_bool_to_string },
	{ "itod", mvm_naive_function_int_to_double },
	{ "dtoi", mvm_naive_function_double_to_int },
	{ "irand", mvm_naive_function_random },
};


MVM_Executable mvm_executable_create()
{
	MVM_Executable mvm_executable = MEM_malloc(sizeof(*mvm_executable));
	mvm_executable->init = MEM_malloc(sizeof(*mvm_executable->init));
	mvm_executable->naive_function_size = 0;
	mvm_executable->naive_functions = NULL;
	mvm_executable->frame_size = 0;
	mvm_executable->frames = NULL;
	mvm_executable->constants = NULL;
	mvm_executable->pc = 0;
	mvm_executable->code_size = CODE_SIZE;
	mvm_executable->code = MEM_malloc(sizeof(*(mvm_executable->code)) * mvm_executable->code_size);
	memset(mvm_executable->code, 0, sizeof(*(mvm_executable->code)) * mvm_executable->code_size);
	mvm_executable->line_numbers = MEM_malloc(sizeof(*(mvm_executable->line_numbers))
		* mvm_executable->code_size);
	memset(mvm_executable->line_numbers, 0,
		sizeof(*(mvm_executable->line_numbers)) * mvm_executable->code_size);
	mvm_executable->bp = 0;
	mvm_executable->sp = 0;
	mvm_executable->stack_size = STACK_SIZE;
	mvm_executable->stack = MEM_malloc(sizeof(*(mvm_executable->stack)) * mvm_executable->stack_size);
	memset(mvm_executable->stack, 0, sizeof(*(mvm_executable->stack)) * mvm_executable->stack_size);
	mvm_executable->root = NULL;
	mvm_executable->label_size = LABEL_SIZE;
	mvm_executable->label = MEM_malloc(sizeof(*(mvm_executable->label)) * mvm_executable->label_size);
	memset(mvm_executable->label, 0, sizeof(*(mvm_executable->label)) * mvm_executable->label_size);
	mvm_executable->next_label_index = 0;
	// load naive_functions
	mvm_load_naive_function(mvm_executable);
	return mvm_executable;
}

void mvm_add_object(MVM_Executable mvm_executable, MVM_Object object)
{
	object->next = mvm_executable->root;
	mvm_executable->root = object;
}

void mvm_object_show(MVM_Executable mvm_executable)
{
	int object_number = 0;
	MVM_Object iterator = mvm_executable->root;
	while (iterator != NULL)
	{
		object_number++;
		iterator = iterator->next;
	}
	LOG_INFO("object number: %d\n", object_number);
}

static void mvm_memory_check(MVM_Executable mvm_executable)
{
	static int times = 0;
	times++;
	if (times % MVM_GC_FREQUENCY == 0)
	{
		LOG_INFO("before gc:\n");
		mvm_object_show(mvm_executable);
		mvm_gc(mvm_executable);
		LOG_INFO("after gc:\n");
		mvm_object_show(mvm_executable);
	}
}
MVM_Object mvm_object_create(MVM_Executable mvm_executable)
{
	//mvm_memory_check(mvm_executable);
	MVM_Object mvm_object = MEM_malloc(sizeof(*mvm_object));
	mvm_add_object(mvm_executable, mvm_object);
	return mvm_object;
}

MVM_Object mvm_string_create(MVM_Executable mvm_executable, char *str)
{
	MVM_String mvm_string = MEM_malloc(sizeof(*mvm_string));
	mvm_string->string = string_create(str);
	MVM_Object mvm_object = mvm_object_create(mvm_executable);
	mvm_object->object_type = MVM_STRING_TYPE;
	mvm_object->u.string_object = mvm_string;
	return mvm_object;
}
MVM_Object mvm_record_create(MVM_Executable mvm_executable, int record_index)
{
	MVM_Record record = MEM_malloc(sizeof(*record));
	MVM_Record_Info record_info = &(mvm_executable->records[record_index]);
	record->data_size = record_info->field_number;
	record->data = MEM_malloc(sizeof(*(record->data)) * record->data_size);
	for (int field_index = 0; field_index < record_info->field_number; field_index++)
	{
		char *field_type =
			mvm_executable->constants->string_literal[record_info->fields[field_index].field_type_index];
		switch (field_type[0])
		{
		case 'N':
		{
			record->data[field_index].value_type = MVM_NIL_TYPE;
			break;
		}
		case 'V':
		{
			LOG_ERROR("type void cannot be assigned to a l-value");
			LOG_EXIT();
			break;
		}
		case 'B':
		{
			record->data[field_index].value_type = MVM_INT_TYPE;
			record->data[field_index].u.int_value = FALSE;
			break;
		}
		case 'I':
		{
			record->data[field_index].value_type = MVM_INT_TYPE;
			record->data[field_index].u.int_value = 0;
			break;
		}
		case 'D':
		{
			record->data[field_index].value_type = MVM_DOUBLE_TYPE;
			record->data[field_index].u.double_value = 0.0;
			break;
		}
		case 'S':
		{
			record->data[field_index].value_type = MVM_OBJECT_TYPE;
			record->data[field_index].u.object_value = mvm_string_create(mvm_executable, "");
			break;
		}
		case 'L':
		{
			record->data[field_index].value_type = MVM_NIL_TYPE;
			break;
		}
		case '[':
		{
			record->data[field_index].value_type = MVM_NIL_TYPE;
			break;
		}
		}
	}
	MVM_Object retval = mvm_object_create(mvm_executable);
	retval->object_type = MVM_RECORD_TYPE;
	retval->u.record_object = record;
	return retval;
}
MVM_Object mvm_array_create(MVM_Executable mvm_executable, int value_type, int len)
{
	MVM_Array a = MEM_malloc(sizeof(*a));
	a->data_size = len;
	a->data = MEM_malloc(sizeof(*(a->data)) * a->data_size);
	for (int i = 0; i < len; i++)
	{
		switch (value_type)
		{
		case MVM_NIL_TYPE:
		{
			LOG_ERROR("ERROR");
			LOG_EXIT();
			break;
		}
		case MVM_BOOL_TYPE:
		{
			a->data[i].value_type = MVM_INT_TYPE;
			a->data[i].u.int_value = FALSE;
			break;
		}
		case MVM_INT_TYPE:
		{
			a->data[i].value_type = MVM_INT_TYPE;
			a->data[i].u.int_value = 0;
			break;
		}
		case MVM_DOUBLE_TYPE:
		{
			a->data[i].value_type = MVM_DOUBLE_TYPE;
			a->data[i].u.double_value = 0.0;
			break;
		}
		case MVM_OBJECT_TYPE + MVM_STRING_TYPE:
		{
			a->data[i].value_type = MVM_OBJECT_TYPE;
			a->data[i].u.object_value = mvm_string_create(mvm_executable, "");
			break;
		}
		case MVM_OBJECT_TYPE + MVM_RECORD_TYPE:
		{
			a->data[i].value_type = MVM_NIL_TYPE;
			break;
		}
		case MVM_OBJECT_TYPE + MVM_ARRAY_TYPE:
		{
			a->data[i].value_type = MVM_NIL_TYPE;
			break;
		}
		}
	}
	MVM_Object retval = mvm_object_create(mvm_executable);
	retval->object_type = MVM_ARRAY_TYPE;
	retval->u.array_object = a;
	return retval;
}


MVM_Object mvm_multiarray_create(MVM_Executable mvm_executable, MVM_Value_Type value_type,
	int dimensions, int *lens)
{
	if (dimensions == 1)
	{
		return mvm_array_create(mvm_executable, value_type, lens[0]);
	}
	else
	{
		MVM_Object multiarray = mvm_object_create(mvm_executable);
		multiarray->object_type = MVM_ARRAY_TYPE;
		multiarray->u.array_object = MEM_malloc(sizeof(*multiarray->u.array_object));
		multiarray->u.array_object->data_size = lens[0];
		multiarray->u.array_object->data = MEM_malloc(sizeof(*(multiarray->u.array_object->data)) *lens[0]);
		for (int i = 0; i < lens[0]; i++)
		{
			multiarray->u.array_object->data[i].value_type = MVM_OBJECT_TYPE;
			multiarray->u.array_object->data[i].u.object_value = mvm_multiarray_create(mvm_executable,
				value_type, dimensions - 1, lens + 1);
		}
		return multiarray;
	}
}
int mvm_next_label_index(MVM_Executable mvm_executable)
{
	if (mvm_executable->next_label_index == mvm_executable->label_size)
	{
		mvm_executable->label_size += LABEL_SIZE;
		mvm_executable->label = MEM_realloc(mvm_executable->label, mvm_executable->label_size);
	}
	return mvm_executable->next_label_index++;
}
char *mvm_get_opcode_string_info(int opcode)
{
	return opcode_info[opcode];
}
char *mvm_get_type_string_info(int type)
{
	return mvm_type_info[type];
}

char *mvm_get_register_string_info(int reg)
{
	return mvm_register_info[reg];
}

static void mvm_show_function_frame(MVM_Executable mvm_executable, MVM_Function_Frame frame, int spaces)
{
	if (frame->is_user_defined == TRUE)
	{
		int opcode_begin = frame->u.user_defined.opcode_begin;
		int opcode_end = frame->u.user_defined.opcode_end;
		int idx = opcode_begin;
		while (idx < opcode_end)
		{
			LOG_INFO("[line: %d] ", mvm_executable->line_numbers[idx]);
			LOG_INFO("pc: %*d, ", spaces, idx);
			char opcode = mvm_executable->code[idx++];
			LOG_INFO("%s", mvm_get_opcode_string_info(opcode));
			switch (opcode)
			{
			case I_NOP:
			{
				LOG_INFO("\n");
				break;
			}
			case I_RPUSH:
			{
				LOG_INFO(" %d #[%s]\n", *(MVM_Int*)&mvm_executable->code[idx],
					mvm_get_register_string_info(*(MVM_Int*)&mvm_executable->code[idx]));
				idx += sizeof(int);
				break;
			}
			case I_NPUSH:
			{
				LOG_INFO("\n");
				break;
			}
			case I_BPUSH:
			{
				int value = *(int*)&mvm_executable->code[idx];
				LOG_INFO(" %d\n", value);
				idx += sizeof(int);
				break;
			}
			case I_IPUSH:
			{
				int value = *(int*)&mvm_executable->code[idx];
				LOG_INFO(" %d\n", value);
				idx += sizeof(int);
				break;
			}
			case I_DPUSH:
			{
				int offset = *(int*)&mvm_executable->code[idx];
				LOG_INFO(" %d #[%lf]\n", offset, mvm_executable->constants->double_literal[offset]);
				idx += sizeof(int);
				break;
			}
			case I_SPUSH:
			{
				int offset = *(int*)&mvm_executable->code[idx];
				LOG_INFO(" %d #[\"%s\"]\n", offset, mvm_executable->constants->string_literal[offset]);
				idx += sizeof(int);
				break;
			}
			case I_LOAD:
			{
				LOG_INFO("\n");
				break;
			}
			case I_ALOAD:
			{
				LOG_INFO("\n");
				break;
			}
			case I_STORE:
			{
				LOG_INFO("\n");
				break;
			}
			case I_ASTORE:
			{
				LOG_INFO("\n");
				break;
			}
			case I_POP:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DUP:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DUP_X1:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DUP_X2:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DUP2:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DUP2_X1:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DUP2_X2:
			{
				LOG_INFO("\n");
				break;
			}
			case I_SWAP:
			{
				LOG_INFO("\n");
				break;
			}
			case I_IADD:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DADD:
			{
				LOG_INFO("\n");
				break;
			}
			case I_SADD:
			{
				LOG_INFO("\n");
				break;
			}
			case I_ISUB:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DSUB:
			{
				LOG_INFO("\n");
				break;
			}
			case I_IMUL:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DMUL:
			{
				LOG_INFO("\n");
				break;
			}
			case I_IDIV:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DDIV:
			{
				LOG_INFO("\n");
				break;
			}
			case I_IREM:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DREM:
			{
				LOG_INFO("\n");
				break;
			}
			case I_INEG:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DNEG:
			{
				LOG_INFO("\n");
				break;
			}
			case I_ICMP:
			{
				LOG_INFO("\n");
				break;
			}
			case I_DCMP:
			{
				LOG_INFO("\n");
				break;
			}
			case I_SCMP:
			{
				LOG_INFO("\n");
				break;
			}
			case I_IFEQ:
			{
				LOG_INFO(" %d\n", *(int*)&mvm_executable->code[idx]);
				idx += sizeof(int);
				break;
			}
			case I_IFNE:
			{
				LOG_INFO(" %d\n", *(int*)&mvm_executable->code[idx]);
				idx += sizeof(int);
				break;
			}
			case I_IFLT:
			{
				LOG_INFO(" %d\n", *(int*)&mvm_executable->code[idx]);
				idx += sizeof(int);
				break;
			}
			case I_IFLE:
			{
				LOG_INFO(" %d\n", *(int*)&mvm_executable->code[idx]);
				idx += sizeof(int);
				break;
			}
			case I_IFGT:
			{
				LOG_INFO(" %d\n", *(int*)&mvm_executable->code[idx]);
				idx += sizeof(int);
				break;
			}
			case I_IFGE:
			{
				LOG_INFO(" %d\n", *(int*)&mvm_executable->code[idx]);
				idx += sizeof(int);
				break;
			}
			case I_GOTO:
			{
				LOG_INFO(" %d\n", *(int*)&mvm_executable->code[idx]);
				idx += sizeof(int);
				break;
			}
			case I_RETURN:
			{
				LOG_INFO("\n");
				break;
			}
			case I_GETFIELD:
			{
				LOG_INFO("\n");
				break;
			}
			case I_PUTFIELD:
			{
				LOG_INFO("\n");
				break;
			}
			case I_INVOKE:
			{
				LOG_INFO(" %d #[%s]\n", *(int*)&mvm_executable->code[idx],
					mvm_executable->constants->string_literal[mvm_executable->frames[*(int*)&mvm_executable->code[idx]].function_name_index]);
				idx += sizeof(int);
				break;
			}
			case I_NEW:
			{
				LOG_INFO(" %d #[%s]\n", *(int*)&(mvm_executable->code[idx]),
					mvm_executable->constants->string_literal
					[mvm_executable->records[*(int*)&(mvm_executable->code[idx])].record_name_index]);
				idx += sizeof(int);
				break;
			}
			case I_NEWARRAY:
			{
				LOG_INFO(" %d #%s\n", *(int*)&(mvm_executable->code[idx]),
					mvm_get_type_string_info(*(int*)&(mvm_executable->code[idx])));
				idx += sizeof(int);
				break;
			}
			case I_MULTINEWARRAY:
			{
				LOG_INFO(" %d %d #%s\n", *(int*)&(mvm_executable->code[idx]),
					*(int*)&(mvm_executable->code[idx + sizeof(MVM_Int)]),
					mvm_get_type_string_info(*(int*)&(mvm_executable->code[idx])));
				idx += sizeof(int) + sizeof(int);
				break;
			}
			case I_ARRAYLENGTH:
			{
				LOG_INFO("\n");
				break;
			}
			case I_STOP:
			{
				LOG_INFO("\n");
				break;
			}
			default:
			{
				LOG_ERROR("ERROR OPCODE: %d", opcode);
				LOG_EXIT();
				break;
			}
			}
		}
	}
	else
	{
		// naive function
		LOG_INFO("naive function");
	}
}
void mvm_code_show(MVM_Executable mvm_executable)
{
	LOG_INFO("MVM SHOW\n");
	LOG_INFO("Record Definition: \n");
	for (int i = 0; i < mvm_executable->record_size; i++)
	{
		LOG_INFO("%s\n{\n", mvm_executable->constants->string_literal[mvm_executable->records[i].record_name_index]);
		for (int j = 0; j < mvm_executable->records[i].field_number; j++)
		{
			//LOG_INFO(mvm_executable->records[i].fields[j].field_type);
			LOG_INFO("%s %s;\n",
				mvm_executable->constants->string_literal[mvm_executable->records[i].fields[j].field_type_index],
				mvm_executable->constants->string_literal[mvm_executable->records[i].fields[j].field_name_index]);
		}
		LOG_INFO("}\n");
	}

	int spaces = 0;
	int pc = mvm_executable->pc;
	while (pc > 0)
	{
		spaces += 1;
		pc = pc / 10;
	}
	LOG_INFO("Constant Pool\n");
	LOG_INFO("Double Constant\n");
	LOG_INFO("{\n");
	for (int i = 0; i < mvm_executable->constants->double_size; i++)
	{
		LOG_INFO("[%d: %lf], ", i, mvm_executable->constants->double_literal[i]);
	}
	LOG_INFO("\n}\n");
	LOG_INFO("String Constant\n");
	LOG_INFO("{\n");
	for (int i = 0; i < mvm_executable->constants->string_size; i++)
	{
		LOG_INFO("[%d: \"%s\"], ", i, mvm_executable->constants->string_literal[i]);
	}
	LOG_INFO("}\n");
	LOG_INFO("MVM initialization:\n");
	mvm_show_function_frame(mvm_executable, mvm_executable->init, spaces);
	LOG_INFO("\nFunction Definition:\n");
	for (int i = 0; i < mvm_executable->frame_size; i++)
	{
		LOG_INFO("%s:\n", mvm_executable->constants->string_literal[mvm_executable->frames[i].function_name_index]);
		mvm_show_function_frame(mvm_executable, &(mvm_executable->frames[i]), spaces);
		LOG_INFO("\n");
		//
	}
}

MVM_Function_Frame mvm_find_main_function_frame(MVM_Executable mvm_executable)
{
	MVM_Function_Frame main_frame = NULL;
	for (int i = 0; i < mvm_executable->frame_size; i++)
	{
		if (strcmp("main", mvm_executable->constants->string_literal[mvm_executable->frames[i].function_name_index]) == 0)
		{
			main_frame = &(mvm_executable->frames[i]);
			break;
		}
	}
	return main_frame;
}
Naive_Function mvm_find_naive_function(MVM_Executable mvm_executable, char *function_name)
{
	Naive_Function naive_function = NULL;
	for (int i = 0; i < mvm_executable->naive_function_size; i++)
	{
		if (strcmp(function_name, mvm_executable->naive_functions[i].function_name) == 0)
		{
			naive_function = mvm_executable->naive_functions[i].function_address;
			break;
		}
	}
	if (naive_function == NULL)
	{
		LOG_ERROR("cannot find naive function %s\n", function_name);
		LOG_EXIT();
	}
	return naive_function;
}

void mvm_load_naive_function(MVM_Executable mvm_executable)
{
	mvm_executable->naive_function_size = sizeof(mvm_naive_functions) / sizeof(mvm_naive_functions[0]);
	mvm_executable->naive_functions = MEM_malloc(sizeof(*(mvm_executable->naive_functions))
		* mvm_executable->naive_function_size);
	for (int i = 0; i < mvm_executable->naive_function_size; i++)
	{
		mvm_executable->naive_functions[i] = mvm_naive_functions[i];
	}
}


#define INC_SP(mvm_executable) \
do{\
	assert(mvm_executable->sp < mvm_executable->stack_size);\
	mvm_executable->sp++;\
	if (mvm_executable->sp >= mvm_executable->stack_size)\
	{\
		mvm_executable->stack_size += STACK_SIZE > 0 ? STACK_SIZE : 1024;\
		mvm_executable->stack = MEM_realloc(mvm_executable->stack, \
			sizeof(*(mvm_executable->stack)) * mvm_executable->stack_size);\
	}\
}while(0)

#define DEC_SP(mvm_executable) ((mvm_executable->sp--))
#define STACK_TOP(mvm_executable) (mvm_executable->stack[mvm_executable->sp])

void mvm_run(MVM_Executable mvm_executable)
{
	char opcode = 0;
	int opcode_index = 0;
	mvm_executable->pc = mvm_executable->init->u.user_defined.opcode_begin;
	while (TRUE)
	{
		// show mvm
		//mvm_buff_show(mvm_executable);
		opcode = mvm_executable->code[mvm_executable->pc];
		LOG_INFO("[line: %d] ", mvm_executable->line_numbers[mvm_executable->pc]);
		LOG_INFO("pc:%d, %s ", mvm_executable->pc, mvm_get_opcode_string_info(opcode));
		mvm_executable->pc++;
		switch (opcode)
		{
		case I_NOP:
		{
			LOG_INFO("\n");
			break;
		}
		case I_RPUSH:
		{
			int register_index = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d #[%s]\n", register_index, mvm_get_register_string_info(register_index));
			mvm_executable->pc += sizeof(int);
			STACK_TOP(mvm_executable).value_type = MVM_INT_TYPE;
			switch (register_index)
			{
			case MVM_BP:
			{
				STACK_TOP(mvm_executable).u.int_value = mvm_executable->bp;
				break;
			}
			case MVM_SP:
			{
				STACK_TOP(mvm_executable).u.int_value = mvm_executable->sp;
				break;
			}
			case MVM_PC:
			{
				STACK_TOP(mvm_executable).u.int_value = mvm_executable->pc;
				break;
			}
			}
			INC_SP(mvm_executable);
			break;
		}
		case I_NPUSH:
		{
			LOG_INFO("\n");
			STACK_TOP(mvm_executable).value_type = MVM_NIL_TYPE;
			INC_SP(mvm_executable);
			break;
		}
		case I_BPUSH:
		{
			int value = *(int*)&mvm_executable->code[mvm_executable->pc];
			STACK_TOP(mvm_executable).value_type = MVM_INT_TYPE;
			STACK_TOP(mvm_executable).u.int_value = value;
			LOG_INFO(" %d\n", value);
			mvm_executable->pc += sizeof(int);
			INC_SP(mvm_executable);
			break;
		}
		case I_IPUSH:
		{
			int value = *(int*)&mvm_executable->code[mvm_executable->pc];
			STACK_TOP(mvm_executable).value_type = MVM_INT_TYPE;
			STACK_TOP(mvm_executable).u.int_value = value;
			LOG_INFO(" %d\n", value);
			mvm_executable->pc += sizeof(int);
			INC_SP(mvm_executable);
			break;
		}
		case I_DPUSH:
		{
			int offset = *(int*)&mvm_executable->code[mvm_executable->pc];
			STACK_TOP(mvm_executable).value_type = MVM_DOUBLE_TYPE;
			STACK_TOP(mvm_executable).u.double_value =
				mvm_executable->constants->double_literal[offset];
			LOG_INFO(" %d #[%lf]\n", offset, mvm_executable->constants->double_literal[offset]);
			mvm_executable->pc += sizeof(int);
			INC_SP(mvm_executable);
			break;
		}
		case I_SPUSH:
		{
			int offset = *(int*)&mvm_executable->code[mvm_executable->pc];
			MVM_Object string_object = mvm_string_create(mvm_executable,
				mvm_executable->constants->string_literal[offset]);
			STACK_TOP(mvm_executable).value_type = MVM_OBJECT_TYPE;
			STACK_TOP(mvm_executable).u.object_value = string_object;
			LOG_INFO(" %d #[\"%s\"]\n", offset, string_object->u.string_object->string);
			mvm_executable->pc += sizeof(int);
			INC_SP(mvm_executable);
			break;
		}
		case I_LOAD:
		{
			DEC_SP(mvm_executable);
			int offset = STACK_TOP(mvm_executable).u.int_value;
			STACK_TOP(mvm_executable) =
				mvm_executable->stack[offset];
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_ALOAD:
		{
			DEC_SP(mvm_executable);
			int index = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			MVM_Object mvm_object = STACK_TOP(mvm_executable).u.object_value;
			MVM_Array mvm_array = mvm_object->u.array_object;
			if (index >= mvm_array->data_size)
			{
				LOG_ERROR("INVALID INDEX: %d\n", index);
				LOG_EXIT();
			}
			STACK_TOP(mvm_executable) = mvm_array->data[index];
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_STORE:
		{
			DEC_SP(mvm_executable);
			MVM_Value value = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			int offset = STACK_TOP(mvm_executable).u.int_value;
			mvm_executable->stack[offset] = value;
			LOG_INFO("\n");
			break;
		}
		case I_ASTORE:
		{
			DEC_SP(mvm_executable);
			MVM_Value value = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			int index = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			MVM_Object mvm_object = STACK_TOP(mvm_executable).u.object_value;
			MVM_Array mvm_array = mvm_object->u.array_object;
			if (index >= mvm_array->data_size)
			{
				LOG_ERROR("INVALID INDEX: %d\n", index);
				LOG_EXIT();
			}
			mvm_array->data[index] = value;
			LOG_INFO("\n");
			break;
		}
		case I_POP:
		{
			DEC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DUP:
		{
			DEC_SP(mvm_executable);
			MVM_Value value = STACK_TOP(mvm_executable);
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DUP_X1:
		{
			// value3, value2, valu1 -> value3, value1, value2, value1
			DEC_SP(mvm_executable);
			MVM_Value value1 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value2 = STACK_TOP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value2;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DUP_X2:
		{
			// value3, value2, valu1 -> value1, value3, value2, value1
			DEC_SP(mvm_executable);
			MVM_Value value1 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value2 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value3 = STACK_TOP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value3;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value2;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DUP2:
		{
			// value3, value2, value1 -> value3, value2, value1, value2, value1
			DEC_SP(mvm_executable);
			MVM_Value value1 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value2 = STACK_TOP(mvm_executable);
			INC_SP(mvm_executable);
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value2;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DUP2_X1:
		{
			// value3, value2, value1 -> value2, value1, value3, value2, value1
			DEC_SP(mvm_executable);
			MVM_Value value1 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value2 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value3 = STACK_TOP(mvm_executable);
			STACK_TOP(mvm_executable) = value2;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value3;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value2;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DUP2_X2:
		{
			// value4, value3, value2, value1 -> value2, value1, value4, value3, value2, value1
			DEC_SP(mvm_executable);
			MVM_Value value1 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value2 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value3 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value4 = STACK_TOP(mvm_executable);
			STACK_TOP(mvm_executable) = value2;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value4;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value3;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value2;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_SWAP:
		{
			// value2, value1 -> value1, value2
			DEC_SP(mvm_executable);
			MVM_Value value1 = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			MVM_Value value2 = STACK_TOP(mvm_executable);
			STACK_TOP(mvm_executable) = value1;
			INC_SP(mvm_executable);
			STACK_TOP(mvm_executable) = value2;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_IADD:
		{
			DEC_SP(mvm_executable);
			MVM_Int v1 = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			MVM_Int v2 = STACK_TOP(mvm_executable).u.int_value;
			STACK_TOP(mvm_executable).u.int_value = v2 + v1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DADD:
		{
			DEC_SP(mvm_executable);
			MVM_Double v1 = STACK_TOP(mvm_executable).u.double_value;
			DEC_SP(mvm_executable);
			MVM_Double v2 = STACK_TOP(mvm_executable).u.double_value;
			STACK_TOP(mvm_executable).u.double_value = v2 + v1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_SADD:
		{
			DEC_SP(mvm_executable);
			MVM_Object o1 = STACK_TOP(mvm_executable).u.object_value;
			DEC_SP(mvm_executable);
			MVM_Object o2 = STACK_TOP(mvm_executable).u.object_value;
			char *str = string_add(o2->u.string_object->string, o1->u.string_object->string);
			STACK_TOP(mvm_executable).u.object_value = mvm_string_create(mvm_executable, str);
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_ISUB:
		{
			DEC_SP(mvm_executable);
			MVM_Int v1 = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			MVM_Int v2 = STACK_TOP(mvm_executable).u.int_value;
			STACK_TOP(mvm_executable).u.int_value = v2 - v1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DSUB:
		{
			DEC_SP(mvm_executable);
			MVM_Double v1 = STACK_TOP(mvm_executable).u.double_value;
			DEC_SP(mvm_executable);
			MVM_Double v2 = STACK_TOP(mvm_executable).u.double_value;
			STACK_TOP(mvm_executable).u.double_value = v2 - v1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_IMUL:
		{
			DEC_SP(mvm_executable);
			MVM_Int v1 = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			MVM_Int v2 = STACK_TOP(mvm_executable).u.int_value;
			STACK_TOP(mvm_executable).u.int_value = v2 * v1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DMUL:
		{
			DEC_SP(mvm_executable);
			MVM_Double v1 = STACK_TOP(mvm_executable).u.double_value;
			DEC_SP(mvm_executable);
			MVM_Double v2 = STACK_TOP(mvm_executable).u.double_value;
			STACK_TOP(mvm_executable).u.double_value = v2 * v1;
			INC_SP(mvm_executable);
			break;
		}
		case I_IDIV:
		{
			DEC_SP(mvm_executable);
			MVM_Int v1 = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			MVM_Int v2 = STACK_TOP(mvm_executable).u.int_value;
			STACK_TOP(mvm_executable).u.int_value = v2 / v1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DDIV:
		{
			DEC_SP(mvm_executable);
			MVM_Double v1 = STACK_TOP(mvm_executable).u.double_value;
			DEC_SP(mvm_executable);
			MVM_Double v2 = STACK_TOP(mvm_executable).u.double_value;
			STACK_TOP(mvm_executable).u.double_value = v2 / v1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_IREM:
		{
			DEC_SP(mvm_executable);
			MVM_Int v1 = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			MVM_Int v2 = STACK_TOP(mvm_executable).u.int_value;
			STACK_TOP(mvm_executable).u.int_value = v2 % v1;
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DREM:
		{
			DEC_SP(mvm_executable);
			MVM_Double v1 = STACK_TOP(mvm_executable).u.double_value;
			DEC_SP(mvm_executable);
			MVM_Double v2 = STACK_TOP(mvm_executable).u.double_value;
			STACK_TOP(mvm_executable).u.double_value = ((MVM_Int)v2) % ((MVM_Int)v1);
			INC_SP(mvm_executable);
			LOG_INFO("\n");
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
			DEC_SP(mvm_executable);
			MVM_Int v1 = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			MVM_Int v2 = STACK_TOP(mvm_executable).u.int_value;
			if (v2 < v1)
			{
				STACK_TOP(mvm_executable).u.int_value = -1;
			}
			else if (v2 > v1)
			{
				STACK_TOP(mvm_executable).u.int_value = 1;
			}
			else if (v2 == v1)
			{
				STACK_TOP(mvm_executable).u.int_value = 0;
			}
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_DCMP:
		{
			DEC_SP(mvm_executable);
			MVM_Double v1 = STACK_TOP(mvm_executable).u.double_value;
			DEC_SP(mvm_executable);
			MVM_Double v2 = STACK_TOP(mvm_executable).u.double_value;
			STACK_TOP(mvm_executable).value_type = MVM_INT_TYPE;
			if (v2 < v1)
			{
				STACK_TOP(mvm_executable).u.double_value = -1;
			}
			else if (v2 > v1)
			{
				STACK_TOP(mvm_executable).u.double_value = 1;
			}
			else if (v2 == v1)
			{
				STACK_TOP(mvm_executable).u.double_value = 0;
			}
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_SCMP:
		{
			DEC_SP(mvm_executable);
			MVM_Object o1 = STACK_TOP(mvm_executable).u.object_value;
			DEC_SP(mvm_executable);
			MVM_Object o2 = STACK_TOP(mvm_executable).u.object_value;
			STACK_TOP(mvm_executable).value_type = MVM_INT_TYPE;
			int result = strcmp(o2->u.string_object->string, o1->u.string_object->string);
			if (result < 0)
			{
				STACK_TOP(mvm_executable).u.int_value = -1;
			}
			else if (result > 0)
			{
				STACK_TOP(mvm_executable).u.int_value = 1;
			}
			else if (result == 0)
			{
				STACK_TOP(mvm_executable).u.int_value = 0;
			}
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_IFEQ:
		{
			DEC_SP(mvm_executable);
			int v1 = STACK_TOP(mvm_executable).u.int_value;
			int new_pc = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d\n", new_pc);
			mvm_executable->pc += sizeof(int);
			if (v1 == 0)
			{
				mvm_executable->pc = new_pc;
			}
			break;
		}
		case I_IFNE:
		{
			DEC_SP(mvm_executable);
			int v1 = STACK_TOP(mvm_executable).u.int_value;
			int new_pc = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d\n", new_pc);
			mvm_executable->pc += sizeof(int);
			if (v1 != 0)
			{
				mvm_executable->pc = new_pc;
			}
			break;
		}
		case I_IFLT:
		{
			DEC_SP(mvm_executable);
			int v1 = STACK_TOP(mvm_executable).u.int_value;
			int new_pc = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d\n", new_pc);
			mvm_executable->pc += sizeof(int);
			if (v1 < 0)
			{
				mvm_executable->pc = new_pc;
			}
			break;
		}
		case I_IFLE:
		{
			DEC_SP(mvm_executable);
			int v1 = STACK_TOP(mvm_executable).u.int_value;
			int new_pc = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d\n", new_pc);
			mvm_executable->pc += sizeof(int);
			if (v1 <= 0)
			{
				mvm_executable->pc = new_pc;
			}
			break;
		}
		case I_IFGT:
		{
			DEC_SP(mvm_executable);
			int v1 = STACK_TOP(mvm_executable).u.int_value;
			int new_pc = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d\n", new_pc);
			mvm_executable->pc += sizeof(int);
			if (v1 > 0)
			{
				mvm_executable->pc = new_pc;
			}
			break;
		}
		case I_IFGE:
		{
			DEC_SP(mvm_executable);
			int v1 = STACK_TOP(mvm_executable).u.int_value;
			int new_pc = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d\n", new_pc);
			mvm_executable->pc += sizeof(int);
			if (v1 >= 0)
			{
				mvm_executable->pc = new_pc;
			}
			break;
		}
		case I_GOTO:
		{
			int new_pc = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d\n", new_pc);
			mvm_executable->pc = new_pc;
			break;
		}
		case I_RETURN:
		{
			//
			mvm_executable->sp = mvm_executable->bp;
			DEC_SP(mvm_executable);
			mvm_executable->bp = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			int return_address = STACK_TOP(mvm_executable).u.int_value;
			mvm_executable->pc = return_address;
			LOG_INFO("\n");
			break;
		}
		case I_GETFIELD:
		{
			DEC_SP(mvm_executable);
			int field_offset = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			if (STACK_TOP(mvm_executable).value_type != MVM_OBJECT_TYPE)
			{
				LOG_ERROR("the type is not record or the record is nil");
				LOG_EXIT();
			}
			MVM_Object mvm_object = STACK_TOP(mvm_executable).u.object_value;
			MVM_Record mvm_record = mvm_object->u.record_object;
			STACK_TOP(mvm_executable) = mvm_record->data[field_offset];
			INC_SP(mvm_executable);
			LOG_INFO("\n");
			break;
		}
		case I_PUTFIELD:
		{
			DEC_SP(mvm_executable);
			MVM_Value value = STACK_TOP(mvm_executable);
			DEC_SP(mvm_executable);
			int field_offset = STACK_TOP(mvm_executable).u.int_value;
			DEC_SP(mvm_executable);
			if (STACK_TOP(mvm_executable).value_type != MVM_OBJECT_TYPE)
			{
				LOG_ERROR("the type is not record or the record is nil");
				LOG_EXIT();
			}
			MVM_Object mvm_object = STACK_TOP(mvm_executable).u.object_value;
			MVM_Record mvm_record = mvm_object->u.record_object;
			mvm_record->data[field_offset] = value;
			LOG_INFO("\n");
			break;
		}
		case I_INVOKE:
		{
			int frame_index = *(int*)&mvm_executable->code[mvm_executable->pc];
			LOG_INFO(" %d #%s\n", *(int*)&mvm_executable->code[mvm_executable->pc],
				mvm_executable->constants->string_literal
				[mvm_executable->frames[*(int*)&mvm_executable->code[mvm_executable->pc]].function_name_index]);
			mvm_executable->pc += sizeof(int);
			MVM_Function_Frame frame = &(mvm_executable->frames[frame_index]);
			if (frame->is_user_defined == TRUE)
			{
				int return_address = mvm_executable->pc;
				mvm_executable->pc = frame->u.user_defined.opcode_begin;
				STACK_TOP(mvm_executable).value_type = MVM_INT_TYPE;
				STACK_TOP(mvm_executable).u.int_value = return_address;
				INC_SP(mvm_executable);
				STACK_TOP(mvm_executable).value_type = MVM_INT_TYPE;
				STACK_TOP(mvm_executable).u.int_value = mvm_executable->bp;
				INC_SP(mvm_executable);
				mvm_executable->bp = mvm_executable->sp;
			}
			else
			{
				Naive_Function naive_function = NULL;
				naive_function = mvm_find_naive_function(mvm_executable,
					mvm_executable->constants->string_literal[frame->function_name_index]);
				int argc = frame->u.naive.parameter_number;
				MVM_Value *argv = &(mvm_executable->stack[mvm_executable->sp - argc]);
				MVM_Value retval = naive_function(mvm_executable, argc, argv);
				mvm_executable->stack[mvm_executable->sp - argc - 1] = retval;
			}
			break;
		}
		case I_NEW:
		{
			int record_index = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d #%s\n", record_index,
				mvm_executable->constants->string_literal[mvm_executable->records[record_index].record_name_index]);
			mvm_executable->pc += sizeof(int);
			STACK_TOP(mvm_executable).value_type = MVM_OBJECT_TYPE;
			STACK_TOP(mvm_executable).u.object_value = mvm_record_create(mvm_executable, record_index);
			INC_SP(mvm_executable);
			break;
		}
		case I_NEWARRAY:
		{
			DEC_SP(mvm_executable);
			int len = STACK_TOP(mvm_executable).u.int_value;
			int value_type = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			LOG_INFO(" %d #%s\n", value_type, mvm_get_type_string_info(value_type));
			mvm_executable->pc += sizeof(int);
			STACK_TOP(mvm_executable).value_type = MVM_OBJECT_TYPE;
			STACK_TOP(mvm_executable).u.object_value = mvm_array_create(mvm_executable, value_type, len);
			INC_SP(mvm_executable);
			break;
		}
		case I_MULTINEWARRAY:
		{
			int value_type = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			mvm_executable->pc += sizeof(int);
			int dimensions = *(int*)&(mvm_executable->code[mvm_executable->pc]);
			mvm_executable->pc += sizeof(int);
			LOG_INFO(" %d %d #%s\n", value_type, dimensions, mvm_get_type_string_info(value_type));
			int *lens = MEM_malloc(sizeof(*lens) * dimensions);
			for (int i = dimensions - 1; i >= 0; i--)
			{
				DEC_SP(mvm_executable);
				lens[i] = STACK_TOP(mvm_executable).u.int_value;
			}
			STACK_TOP(mvm_executable).value_type = MVM_OBJECT_TYPE;
			STACK_TOP(mvm_executable).u.object_value = mvm_multiarray_create(mvm_executable, value_type,
				dimensions, lens);
			INC_SP(mvm_executable);
			break;
		}
		case I_ARRAYLENGTH:
		{
			break;
		}
		case I_STOP:
		{
			return;
			break;
		}
		default:
		{
			LOG_ERROR("ERROR OPCODE: %d", opcode);
			LOG_EXIT();
			break;
		}
		}
		// show status
		//mvm_show(mvm_executable);
	}
}

void mvm_value_show(MVM_Value value)
{
	switch (value.value_type)
	{
	case MVM_NIL_TYPE:
	{
		LOG_INFO("(nil)");
		break;
	}
	case MVM_BOOL_TYPE:
	{
		LOG_INFO("(bool, %d)", value.u.bool_value);
		break;
	}
	case MVM_INT_TYPE:
	{
		LOG_INFO("(int, %d)", value.u.int_value);
		break;
	}
	case MVM_DOUBLE_TYPE:
	{
		LOG_INFO("(double, %lf)", value.u.double_value);
		break;
	}
	case MVM_OBJECT_TYPE:
	{
		switch (value.u.object_value->object_type)
		{
		case MVM_STRING_TYPE:
		{
			LOG_INFO("(string, \"%s\")", value.u.object_value->u.string_object->string);
			break;
		}
		case MVM_RECORD_TYPE:
		{
			LOG_INFO("record");
			break;
		}
		case MVM_ARRAY_TYPE:
		{
			LOG_INFO("(array, ");
			LOG_INFO("[");
			for (int idx = 0; idx < value.u.object_value->u.array_object->data_size; idx++)
			{
				mvm_value_show(value.u.object_value->u.array_object->data[idx]);
				LOG_INFO(", ");
			}
			LOG_INFO("])");
			break;
		}
		}
		break;
	}
	default:
	{
		LOG_ERROR("unknown type: %d", value.value_type);
		LOG_EXIT();
	}
	}
}
void mvm_buff_show(MVM_Executable mvm_executable)
{
	/*
	int naive_function_size;
	MVM_Naive_Function naive_functions;
	int frame_size;
	MVM_Function_Frame frames;
	int record_size;
	MVM_Record_Info records;
	int pc;
	int code_size;
	unsigned char *code;
	int static_data_size;
	int static_data_offset;
	int bp;
	int sp;
	int stack_size;
	int stack_offset;
	int buff_size;
	MVM_Value *buff;
	MVM_Object root;
	int next_label_index;
	int label_size;
	int *label;
	*/
	LOG_INFO("bp: %d, sp: %d, stack size: %d\n", mvm_executable->bp, mvm_executable->sp, mvm_executable->stack_size);
	LOG_INFO("[");
	for (int i = 0; i < mvm_executable->sp; i++)
	{
		mvm_value_show(mvm_executable->stack[i]);
		LOG_INFO(",");
	}
	LOG_INFO("]\n");
}


static void mvm_object_set_unmarked(MVM_Object root)
{
	MVM_Object iterator = root;
	while (iterator != NULL)
	{
		iterator->marked = FALSE;
		iterator = iterator->next;
	}
}

static void mvm_object_traverse(MVM_Value *array, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (array[i].value_type == MVM_OBJECT_TYPE)
		{
			if (array[i].u.object_value->marked == FALSE)
			{
				array[i].u.object_value->marked = TRUE;
				switch (array[i].u.object_value->object_type)
				{
				case MVM_STRING_TYPE:
				{
					break;
				}
				case MVM_RECORD_TYPE:
				{
					mvm_object_traverse(array[i].u.object_value->u.record_object->data, 
						array[i].u.object_value->u.record_object->data_size);
					break;
				}
				case MVM_ARRAY_TYPE:
				{
					mvm_object_traverse(array[i].u.object_value->u.array_object->data,
						array[i].u.object_value->u.array_object->data_size);
					break;
				}
				default:
				{
					LOG_ERROR("error mvm object type");
					LOG_EXIT();
				}
				}
			}
		}
	}
}

static void mvm_object_free(MVM_Object object)
{
	switch (object->object_type)
	{
	case MVM_STRING_TYPE:
	{
		MEM_free(object->u.string_object->string);
		MEM_free(object->u.string_object);
		break;
	}
	case MVM_RECORD_TYPE:
	{
		MEM_free(object->u.record_object->data);
		MEM_free(object->u.record_object);
		break;
	}
	case MVM_ARRAY_TYPE:
	{		
		MEM_free(object->u.array_object->data);
		MEM_free(object->u.array_object);
		break;
	}
	default:
	{
		LOG_ERROR("error mvm object type");
		LOG_EXIT();
	}
	}
	MEM_free(object);
}
void mvm_gc(MVM_Executable mvm_executable)
{
	mvm_object_set_unmarked(mvm_executable->root);
	mvm_object_traverse(mvm_executable->stack, mvm_executable->sp);
	MVM_Object *iterator = &(mvm_executable->root);
	MVM_Object temp = NULL;
	while (*iterator != NULL)
	{
		if ((*iterator)->marked == FALSE)
		{
			temp = *iterator;
			*iterator = temp->next;
			mvm_object_free(temp);
		}
		else
		{
			iterator = &((*iterator)->next);
		}
	}
}

