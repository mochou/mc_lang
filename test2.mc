function int print(var string info);
function string itos(var int i);
function string btos(var bool b);
function string dtos(var double d);
function double itod(var int i);
function int dtoi(var double d);
function int irand();

function void print_int_array(var int[] numbers, var int len)
{
	var int temp;
	var int temp2, temp3;
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

function int partition(var int[] numbers, var int start, var int end)
{
	var int i = start - 1, j = start;
	var int guard = numbers[end - 1];
	for (i = start - 1, j = start; j < end - 1; j++)
	{
		if (numbers[j] < guard)
		{
			i++;
			var int temp = numbers[i];
			numbers[i] = numbers[j];
			numbers[j] = temp;
		}
	}
	i++;
	var int temp = numbers[end - 1];
	numbers[end - 1] = numbers[i];
	numbers[i] = temp;
	return i;
}

function void qsort(var int[] numbers, var int start, var int end)
{
	if (end - start > 1)
	{
		var int middle = partition(numbers, start, end);
		qsort(numbers, start, middle);
		qsort(numbers, middle + 1, end);
	}
	return;
}


function void bubblesort(var int[] numbers, var int len)
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
				print("swap times: ");
				print(itos(count + 1));
				print("\n");
				print("before swap:\n");
				print_int_array(numbers, len);
				var int temp = numbers[i];
				numbers[i] = numbers[j];
				numbers[j] = temp;
				print("after swap:\n");
				print_int_array(numbers, len);
				count++;
			}
		}
	}
	var string times = itos(count);
	print("total times:");
	print(times);
	print("\n");
}

function void multiarray_test()
{
	var int row = 8, col = 8;	
	var int[][] aa = new int[row][col];
	var int count = 0;
	var int i = 0, j = 0;
	print("before sorted:\n");
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			aa[i][j] = irand() % (row * col);
		}
	}
	for (i = 0; i < row; i++)
	{
		print_int_array(aa[i], col);
	}
	for (i = 0; i < row; i++)
	{		
		print("the " + itos(i) + "-th array before sorted:\n");
		print_int_array(aa[i], col);
		bubblesort(aa[i], col);
		//qsort(aa[i], 0, col);
		print("the " + itos(i) + "-th array after sorted:\n");
		print_int_array(aa[i], col);
	}
	print("after sorted:\n");
	for (i = 0; i < row; i++)
	{
		print_int_array(aa[i], col);
	}
}

function void main(var int argc, var string[] argv)
{
	var int i = 0; 
	for (i = 0; i < argc; i++)
	{
		print(argv[i]);
		print(", ");
	}
	//test1();
	//test2();	
	multiarray_test();
}