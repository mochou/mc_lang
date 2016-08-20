var bool b1 = true;
var bool b2 = false;
var int a = 100;
var double d = 10.10;
var string b = "liuzhijaing";
var int[][][] arr = nil;

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

var student[][][] student_list = nil;

function void print(var string info)
{
	return;
}
function void while_statement_test()
{
	var int i = 0;
	while (i < 10)
	{
		if (i > 5)
		{
			break;
		}
		if (i < 6)
		{
			continue;
		}
		i++;
	}
}

function void do_while_statement_test()
{
	var double i = 0.0;
	do
	{
		if (i < 5.0)
		{
			continue;
		}
		if (i > 6.0)
		{
			break;
		}
		i = i + 1.0;
	}while (i < 10.0);
}

function int for_statement_test()
{
	var int i = 0;
	for (i = 0; i < 10; i++)
	{
		if (i < 5)
		{
			continue;
		}
		else if (i == 6)
		{
			break;
		}
		else
		{
			return i;
		}
		var int j = i - 33;
	}
}

function void if_statement_test()
{
	var string i = "liu";
	if (i == "1")
	{
		i = "equal";
	}
	else if (i > "2")
	{
		i = "lt";
	}
	else if (i < "3")
	{
		i = "gt";
	}
	else
	{
		i = "unknown";
	}
}

function void new_expression_test()
{
	var student s = new student;
	var class c = new class;
	var int[] int_array = new int[10];
	var student[] student_array = new student[20];
	var double[][] i2 = new double[2][3];
	var int[][][] i3 = new int[4][5][6];
	var string[][][][] i4 = new string[7][8][9][10];
}
function void operator_test()
{
	var bool b1 = true;
	var bool b2 = false;
	var int i1 = 1;
	var int i2 = 2;
	var student s1, s2, s3, s4;
	var int[] array;
	b1 && b2;
	b1 || b2;	
	i1 == i2;
	i1 != i2;
	i1 < i2;
	i1 <= i2;
	i1 > i2; 
	i1 >= i2;	
	i1 = i2 = 13;
	s1.name = s2.name = s3.name = s4.name = "liu";
	i1++;
	i2--;
	s1.id++;
	s2.id--;
	i1 = array[0] = i2 = array[1] = array[2] = 3;
	array[0]++;
	array[0]--;
}
function void class_test(var class[] classes)
{
	operator_test();
	var int a = 1 + 2;
	3 + 4;
	5 * 6 + 7 / 8 - 9 % 10;
	11 * (12 + 13) / 14 * (15 - 16);
	var student s = classes[0].students[0];
	var string name = classes[0].students[0].name;
	s = classes[0].students[0];
	//1 + 2 = 2;
	name = classes[0].students[0].name;
}

/*

function void array_test(var student[][][] student_list)
{
	var student s = student_list[0][0][0];
	var student[][] s2 = student_list[0];
	s = student_list[0][0][0];
	var string name = student_list[0][0][0].name;
	name = student_list[0][0][0].name;
}
function void record_test(var list l)
{
	var list n;
	n = l.next;
}
function void print(var int i);
function int add(var int a, var int b)
{
	var int c;
	var int cd;
	var int s;
	c = a + b;
	print(s);
	return c;
}

function int sign(var int a)
{
	if (a < 0)
	{
		var int a1;
		var int a2;
		return -1;
	}
	else if (a == 0)
	{
		var int b1;
		var int b2;
		return 0;
	}
	else if (a > 0)
	{
		var int a1;
		var int a2;
		var int b1;
		var int b2;
		{
			var int c1;
			var int c2;
			{
				var int d1;
				var int d2;
			}
		}
		return 1;
	}
	return 0;
}

function int sum(var int a)
{
	var int result = 0;
	var int idx = 1;
	for (idx = 1; idx <= a; idx++)
	{
		result = result + idx;
	}
	return result;
}

function int sum2(var int a)
{
	var int result = 0;
	while (result > 0)
	{
		result = result + a;
		a--;
	}
	return result;
}

function int sum3(var int a)
{
	var int result = 0;
	do 
	{
		result = result + a;
		a--;
	}while (result > 0);
	return result;
}

function int fac(var int a)
{
	if (a > 1)
	{
		return a * fac(a - 1);
	}
	else
	{
		return 1;
	}
}

function bool student_equal(var student s1, var student s2)
{
	if (s1.id != s2.id)
	{
		return false;
	}
	if (s1.name != s2.name)
	{
		return false;
	}
	if (s1.addr != s2.addr)
	{
		return false;
	}
	if (s1.local != s2.local)
	{
		return false;
	}
	return true;
}

*/