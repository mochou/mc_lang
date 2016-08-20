#ifndef __MVM_H
#define __MVM_H

typedef int MVM_Int;
typedef MVM_Int MVM_Bool;
typedef double MVM_Double;
typedef struct MVM_Value_tag MVM_Value;
typedef struct MVM_Object_tag *MVM_Object;
typedef struct MVM_String_tag *MVM_String;
typedef struct MVM_Record_tag *MVM_Record;
typedef struct MVM_Array_tag *MVM_Array;
typedef struct MVM_Executable_Tag *MVM_Executable;
typedef struct MVM_Naive_Function_tag *MVM_Naive_Function;
typedef MVM_Value (*Naive_Function)(MVM_Executable mvm_executable, int argc, MVM_Value *argv);
typedef enum
{
	I_NOP,
	//
	I_RPUSH, // push register
	I_NPUSH, // push nil
	I_BPUSH, // push bool
	I_IPUSH, // push int
	I_DPUSH, // push double
	I_SPUSH, // push string
	//
	I_LOAD,
	I_ALOAD,
	//
	I_STORE,
	I_ASTORE,
	//
	I_POP,
	I_DUP,
	I_DUP_X1,
	I_DUP_X2,
	I_DUP2,
	I_DUP2_X1,
	I_DUP2_X2,
	I_SWAP,
	//
	I_IADD,
	I_DADD,
	I_SADD,
	I_ISUB,
	I_DSUB,
	I_IMUL,
	I_DMUL,
	I_IDIV,
	I_DDIV,
	I_IREM,
	I_DREM,
	I_INEG,
	I_DNEG,
	//
	I_ICMP,
	I_DCMP,
	I_SCMP,
	//
	I_IFEQ,
	I_IFNE,
	I_IFLT,
	I_IFLE,
	I_IFGT,
	I_IFGE,
	I_GOTO,
	//
	I_RETURN,
	I_GETFIELD,
	I_PUTFIELD,
	I_INVOKE,
	I_NEW,
	I_NEWARRAY,
	I_MULTINEWARRAY,
	I_ARRAYLENGTH,
	// 
	I_STOP,
}Opcode;

typedef enum
{
	MVM_NIL_TYPE = 0,
	MVM_BOOL_TYPE,
	MVM_INT_TYPE,
	MVM_DOUBLE_TYPE,
	MVM_OBJECT_TYPE,
}MVM_Value_Type;

typedef enum
{
	MVM_STRING_TYPE = 0,
	MVM_RECORD_TYPE,
	MVM_ARRAY_TYPE,
}MVM_Object_Type;

struct MVM_Value_tag
{
	MVM_Value_Type value_type;
	union
	{
		MVM_Bool bool_value;
		MVM_Int int_value;
		MVM_Double double_value;
		MVM_Object object_value;
	}u;
};

struct MVM_Object_tag
{
	MVM_Object_Type object_type;
	union
	{
		MVM_String string_object;
		MVM_Record record_object;
		MVM_Array array_object;
	}u;
	MVM_Object next;
	int marked;
};

struct MVM_String_tag
{
	char *string;
};

struct MVM_Record_tag
{
	int data_size;
	MVM_Value *data;
};

struct MVM_Array_tag
{
	int data_size;
	MVM_Value *data;
};


typedef struct MVM_Function_Frame_tag *MVM_Function_Frame;
struct MVM_Function_Frame_tag
{
	//char *function_name;
	int function_name_index;
	int is_user_defined;
	int function_index;
	union
	{
		struct
		{
			Naive_Function naive_function;
			int parameter_number;
		}naive;
		struct
		{
			int opcode_begin;
			int opcode_end;
		}user_defined;
	}u;
};

typedef struct MVM_Record_Info_tag *MVM_Record_Info;
typedef struct MVM_Field_Info_tag *MVM_Field_Info;
typedef struct MVM_Constant_tag *MVM_Constant;
struct MVM_Record_Info_tag
{
	//char *record_name;
	int record_name_index;
	int field_number;
	MVM_Field_Info fields;
};

struct MVM_Field_Info_tag
{
	//char *field_name;
	//char *field_type;
	int field_name_index;
	int field_type_index;
};
//#define CODE_SIZE (1024 * 1024)
#define CODE_SIZE (1024 * 1024)
#define STACK_SIZE (1024 * 1024)
#define LABEL_SIZE (1024 * 1024)

typedef enum
{
	MVM_BP,
	MVM_SP,
	MVM_PC,
}MVM_Register;

struct MVM_Naive_Function_tag
{
	char *function_name;
	Naive_Function function_address;
};

struct MVM_Constant_tag
{
	int double_size;
	double *double_literal;
	int string_size;
	char **string_literal;
};
struct MVM_Executable_Tag
{
	MVM_Function_Frame init;
	int naive_function_size;
	MVM_Naive_Function naive_functions;
	int frame_size;
	MVM_Function_Frame frames;
	int record_size;
	MVM_Record_Info records;
	// constant pool
	MVM_Constant constants;
	//int opcode_idx;
	int pc;
	int code_size;
	unsigned char *code;
	int *line_numbers;
	int bp;
	int sp;
	int stack_size;
	MVM_Value *stack;
	MVM_Object root;
	int next_label_index;
	int label_size;
	int *label;
};

MVM_Executable mvm_executable_create();
MVM_Object mvm_string_create(MVM_Executable mvm_executable, char *str);
MVM_Object mvm_record_create(MVM_Executable mvm_executable, int record_index);
MVM_Object mvm_array_create(MVM_Executable mvm_executable, MVM_Value_Type value_type, int len);
MVM_Object mvm_multiarray_create(MVM_Executable mvm_executable, MVM_Value_Type value_type,
	int dimensions,	int *lens);
int mvm_next_label_index(MVM_Executable mvm_executable);
char *mvm_get_opcode_string_info(int opcode);
char *mvm_get_type_string_info(int type);
char *mvm_get_register_string_info(int reg);
// show code
void mvm_initialize(MVM_Executable mvm_executable);
void mvm_run(MVM_Executable mvm_executable);
void mvm_load_naive_function(MVM_Executable mvm_executable);
Naive_Function mvm_find_naive_function(MVM_Executable mvm_executable, char *function_name);
void mvm_code_show(MVM_Executable mvm_executable);
void mvm_buff_show(MVM_Executable mvm_executable);
void mvm_gc(MVM_Executable mvm_executable);
void mvm_object_show(MVM_Executable mvm_executable);
#endif