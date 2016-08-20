function int print(var string info);
function string itos(var int i);
function string btos(var bool b);
function string dtos(var double d);
function double itod(var int i);
function int dtoi(var double d);
function int irand();

var int g_i = 10;
var double g_d = 10.10;
function void main(var int argc, var string[] argv)
{
	print(itos(g_i));
	print(", ");
	print(dtos(g_d));
	print("\n");
	var int i = 0; 
	for (i = 0; i < argc; i++)
	{
		print(argv[i]);
		print(", ");
	}
}