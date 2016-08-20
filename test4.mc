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

function void array_test()
{
	var int dim = 2;	
	var int[] array = new int[dim];
	print("array:\n");
	var int i = 0;
	for (i = 0; i < dim; i++)
	{
		array[i] = irand() % (10);
	}
	print_int_array(array, dim);	
}

function void main(var int argc, var string[] argv)
{
	print(argv[0]);	
	print(", ");
	array_test();
}