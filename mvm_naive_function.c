#include "environment.h"
#include "constant.h"
#include "LOG.h"
#include "mvm_naive_function.h"


MVM_Value mvm_naive_function_print(MVM_Executable mvm_executable, int argc, MVM_Value *argv)
{
	MVM_Value retval;
	if (argc != 1)
	{
		LOG_ERROR("the argc must be 1");
		LOG_EXIT();
		exit(-1);
	}
	if (argv[0].value_type != MVM_OBJECT_TYPE && argv[0].u.object_value->object_type != MVM_STRING_TYPE)
	{
		LOG_ERROR("the arg must be string");
		LOG_EXIT();
	}
	retval.value_type = MVM_INT_TYPE;
	retval.u.int_value = printf("%s", argv[0].u.object_value->u.string_object->string);
	LOG_PRINT("%s", argv[0].u.object_value->u.string_object->string);
	return retval;
}

MVM_Value mvm_naive_function_int_to_string(MVM_Executable mvm_executable, int argc, MVM_Value *argv)
{
	MVM_Value retval;
	if (argc != 1)
	{
		LOG_ERROR("the argc must be 1");
		LOG_EXIT();
		exit(-1);
	}
	if (argv[0].value_type != MVM_INT_TYPE)
	{
		LOG_ERROR("the arg must be int");
		LOG_EXIT();		
	}
	char str[64];
	sprintf(str, "%d", argv[0].u.int_value);
	retval.value_type = MVM_OBJECT_TYPE;
	retval.u.object_value = mvm_string_create(mvm_executable, str);
	return retval;
}

MVM_Value mvm_naive_function_double_to_string(MVM_Executable mvm_executable, int argc, MVM_Value *argv)
{
	MVM_Value retval;
	if (argc != 1)
	{
		LOG_ERROR("the argc must be 1");
		LOG_EXIT();
	}
	if (argv[0].value_type != MVM_DOUBLE_TYPE)
	{
		LOG_ERROR("the arg must be double");
		LOG_EXIT();
	}
	char str[64];
	sprintf(str, "%.6lf", argv[0].u.double_value);
	retval.value_type = MVM_OBJECT_TYPE;
	retval.u.object_value = mvm_string_create(mvm_executable, str);
	return retval;
}

MVM_Value mvm_naive_function_bool_to_string(MVM_Executable mvm_executable, int argc, MVM_Value *argv)
{
	MVM_Value retval;
	if (argc != 1)
	{
		LOG_ERROR("the argc must be 1");
		LOG_EXIT();
	}
	if (argv[0].value_type != MVM_BOOL_TYPE && argv[0].value_type != MVM_INT_TYPE)
	{
		LOG_ERROR("the arg must be bool");
		LOG_EXIT();
	}
	retval.value_type = MVM_OBJECT_TYPE;
	if (argv[0].u.bool_value == TRUE)
	{
		retval.u.object_value = mvm_string_create(mvm_executable, "true");
	}
	else
	{
		retval.u.object_value = mvm_string_create(mvm_executable, "false");
	}
	return retval;
}

MVM_Value mvm_naive_function_int_to_double(MVM_Executable mvm_executable, int argc, MVM_Value *argv)
{
	MVM_Value retval;
	if (argc != 1)
	{
		LOG_ERROR("the argc must be 1");
		LOG_EXIT();
		exit(-1);
	}
	if (argv[0].value_type != MVM_INT_TYPE)
	{
		LOG_ERROR("the arg must be int");
		LOG_EXIT();
	}
	retval.value_type = MVM_DOUBLE_TYPE;
	retval.u.double_value = argv[0].u.int_value;
	return retval;
}

MVM_Value mvm_naive_function_double_to_int(MVM_Executable mvm_executable, int argc, MVM_Value *argv)
{
	MVM_Value retval;
	if (argc != 1)
	{
		LOG_ERROR("the argc must be 1");
		LOG_EXIT();
		exit(-1);
	}
	if (argv[0].value_type != MVM_DOUBLE_TYPE)
	{
		LOG_ERROR("the arg must be double");
		LOG_EXIT();
	}
	retval.value_type = MVM_INT_TYPE;
	retval.u.int_value = (MVM_Int)argv[0].u.double_value;
	return retval;
}

MVM_Value mvm_naive_function_random(MVM_Executable mvm_executable, int argc, MVM_Value *argv)
{
	MVM_Value retval;
	retval.value_type = MVM_INT_TYPE;
	retval.u.int_value = rand();
	return retval;
}
