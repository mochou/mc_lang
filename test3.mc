function int print(var string info);
function string itos(var int i);
function string btos(var bool b);
function string dtos(var double d);
function double itod(var int i);
function int dtoi(var double d);
function int irand();

record list
{
	var int value;
	var list next;
};

record student
{
	var int id;
	var string name, addr;
	var bool local;
};

record class
{
	var student[] students;
};

function void print_student(var student s)
{
	print("id: ");
	print(itos(s.id));
	print(", name:");
	print(s.name);
	print(", addr: ");
	print(s.addr);
	print(", local: ");
	print(btos(s.local));
}
function void record_test()
{
	var int number = 5;
	var student [] student_array = new student[number];
	var int i = 0;
	for (i = 0; i < number; i++)
	{
		student_array[i] = new student;
		student_array[i].id = 1000 + i;
		student_array[i].name = itos(2000 + i);
		student_array[i].addr = itos(3000 + i);
		student_array[i].local = true;
	}
	for (i = 0; i < number; i++)
	{
		print_student(student_array[i]);
		print("\n");
	}
}
function void main()
{
	record_test();
}