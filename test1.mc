function int print(var string info);
function string itos(var int i);
function string btos(var bool b);
function string dtos(var double d);
function double itod(var int i);
function int dtoi(var double d);
function int irand();




function int isum(var int len, var int [] numbers)
{
	var int result = 0;
	var int idx = 0;
	for (idx = 0; idx < len; idx++)
	{
		result = result + numbers[idx];
	}
	return result;
}



function void init_array(var int len, var int[] numbers)
{
	var int idx = 0;
	for (idx = 0; idx < len; idx++)
	{
		numbers[idx] = idx;
	}
}


function void test1()
{
	var int len = 10;
	var int[] numbers = new int[len];
	init_array(len, numbers);
	var int result = isum(len, numbers);
	var string str = itos(result);
	var string prompt = "the sum is: ";
	var string endl = "\n";
	print(prompt);
	print(str);
	print(endl);
}


function void print_int_array(var int len, var int[] numbers)
{
	var int idx = 0;
	for (idx = 0; idx < len; idx++)
	{
		var string str = itos(numbers[idx]);
		print(str);
		var string space = " ";
		print(space);
	}	
	var string sp = "\n";
	print(sp);
}


function void bubblesort(var int len, var int[] numbers)
{
	if (len < 2)
	{
		return;
	}
	var int i = 0;
	var int count = 0;
	for (i = 0; i < len - 1; i++)
	{
		var int j = 0;
		for (j = i + 1; j < len; j++)
		{
			if (numbers[i] > numbers[j])
			{
				print("½»»»´ÎÊı: ");
				print(itos(count + 1));
				print("\n");
				print("before swap:\n");
				print_int_array(len, numbers);
				var int temp = numbers[i];
				numbers[i] = numbers[j];
				numbers[j] = temp;
				print("after swap:\n");
				print_int_array(len, numbers);
				count++;
			}
		}
	}
	var string times = itos(count);
	print("total times:");
	print(times);
	print("\n");
}


function void test2()
{
	var int len = 10;
	var int[] numbers = new int[len];
	numbers[0] = 38;
	numbers[1] = 26;
	numbers[2] = 55;
	numbers[3] = 61;
	numbers[4] = 14;
	numbers[5] = 35;
	numbers[6] = 26;
	numbers[7] = 32;
	numbers[8] = 12;
	numbers[9] = 62;
	print("initialial array:\n");
	print_int_array(len, numbers);
	bubblesort(len, numbers);
	print("the sorted array:\n");
	print_int_array(len, numbers);	
}

function void multiarray_test()
{
	var int row = 4, col = 4;
	var int[][] aa = new int[row][col];
	var int count = 0;
	var int i = 0, j = 0;
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			aa[i][j] = count++;
		}
	}
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			print(itos(aa[i][j]));
			print(", ");
		}
		print("\n");
	}
}
function void main()
{
	//test1();
	//test2();	
	multiarray_test();
}