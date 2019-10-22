#include <iostream>
#include<vector>
#include <string>
#include "lexer.h"
using namespace std;
using namespace lexertk;

void example01()
{
	string expression = "(sin(x/pi)cos(2y) + 1) == (sin(x / pi) * cos(2 * y) + 1)";

	generator generator;

	if (!generator.process(expression))
	{
		cout << "Example01 - Failed to lex: " << expression << endl;
		return;
	}

	cout << "***** Example01 *****" << endl;
	helper::dump(generator);
	cout << "*********************" << endl;

}

void example02()
{
	string expression = "(sin(x/pi)123.456cos(2y) + 1) == (sin(x / pi) * 123.456 * cos(2 * y) + 1)";

	generator generator;

	if (!generator.process(expression))
	{
		cout << "Example02 - Failed to lex: " << expression << endl;
		return;
	}

	helper::commutative_inserter ci;
	ci.process(generator);

	std::cout << "***** Example02 *****" << std::endl;
	helper::dump(generator);
	std::cout << "*********************" << std::endl;
}

void example03()
{
	string expression = "((1.1 > = 2.2) ! = (3.3 < = 4.4)) < > [x_x : = y_y]";

	generator generator;

	if (!generator.process(expression))
	{
		std::cout << "Example03 - Failed to lex: " << expression << std::endl;
		return;
	}

	helper::operator_joiner oj;
	oj.process(generator);

	cout << "***** Example03 *****" << endl;
	helper::dump(generator);
	cout << "*********************" << endl;
}

void example04()
{
	string expression = "{a+(b-[c*(e/{f+g}-h)*i]%[j+(k-{l*m}/n)+o]-p)*q}";

	generator generator;

	if (!generator.process(expression))
	{
		cout << "Example04 - Failed to lex: " << expression << endl;
		return;
	}

	helper::bracket_checker bc;
	bc.process(generator);

	if (!bc.result())
	{
		cout << "Example04 - Failed Bracket Checker!" << endl;
		return;
	}

	cout << "***** Example04 *****" << endl;
	lexertk::helper::dump(generator);
	cout << "*********************" << endl;
}

void example05()
{
	string expression = "(sin(x/pi) * cos(2y) + 1)";

	generator generator;

	if (!generator.process(expression))
	{
		std::cout << "Example05 - Failed to lex: " << expression << std::endl;
		return;
	}

	helper::symbol_replacer sr;

	sr.add_replace("sin", "Deg2RadSine");
	sr.add_replace("cos", "Deg2RadCosine");

	size_t change_count = sr.process(generator);

	cout << "Example05 - Number of changes: " << change_count << std::endl;

	cout << "***** Example05 *****" << std::endl;
	helper::dump(generator);
	std::cout << "*********************" << std::endl;
}

struct function_definition
{
	string name;
	string body;
	vector<std::string> var_list;

	void clear()
	{
		name.clear();
		body.clear();
		var_list.clear();
	}
};

struct parse_function_definition_impl : public lexertk::parser_helper
{
	/*
	   Structure: function <name> (v0,v1,...,vn) { expression }
	*/
	bool process(std::string& func_def, function_definition& fd)
	{
		if (!init(func_def))
			return false;

		if (!token_is(token_t::e_symbol, "function"))
			return false;

		if (!token_is_then_assign(token_t::e_symbol, fd.name))
			return false;

		if (!token_is(token_t::e_lbracket))
			return false;

		if (!token_is(token_t::e_rbracket))
		{
			vector<string> var_list;

			for (; ; )
			{
				// (x,y,z,....w)
				if (!token_is_then_assign(token_t::e_symbol, var_list))
					return false;

				if (token_is(token_t::e_rbracket))
					break;

				if (!token_is(token_t::e_comma))
					return false;
			}

			var_list.swap(fd.var_list);
		}

		size_t body_begin = current_token().position;
		size_t body_end = current_token().position;

		int bracket_stack = 0;

		if (!token_is(token_t::e_lcrlbracket, e_hold))
			return false;

		for (; ; )
		{
			body_end = current_token().position;

			if (token_is(token_t::e_lcrlbracket))
				bracket_stack++;
			else if (token_is(token_t::e_rcrlbracket))
			{
				if (0 == --bracket_stack)
					break;
			}
			else
			{
				if (lexer().finished())
					return false;

				next_token();
			}
		}

		std::size_t size = body_end - body_begin + 1;

		fd.body = func_def.substr(body_begin, size);

		const std::size_t index = body_begin + size;

		if (index < func_def.size())
			func_def = func_def.substr(index, func_def.size() - index);
		else
			func_def = "";

		return true;
	}
};

bool parse_function_definition(std::string& func_def, function_definition& fd)
{
	parse_function_definition_impl parser;
	return parser.process(func_def, fd);
}

void example06()
{
	const string f =
		"function foo0( ) { if (x < '}}}') { x+y; x+=1;} else {x;} }          "
		"function foo1(x) { if (x < '}}}') { x+y; x+=1;} else {x;} }          "
		"function foo2(x,y) { if (x < '}}}') { x+y; x+=1;} else {x;} }        "
		"function foo3(x,y,z) { if (x < '}}}') { x+y; x+=1;} else {x;} }      "
		"function foo4(x,y,z,w) { if (x < '}}}') { x+y; w+=1;} else {x; w;} } "
		"function foo5( ) { if (x < '}}}') { x+y; x+=1;} else {x;} }          "
		"function foo6(x) { if (x < '}}}') { x+y; x+=1;} else {x;} }          "
		"function foo7(x,y) { if (x < '}}}') { x+y; x+=1;} else {x;} }        "
		"function foo8(x,y,z) { if (x < '}}}') { x+y; x+=1;} else {x;} }      "
		"function foo9(x,y,z,w) { if (x < '}}}') { x+y; w+=1;} else {x; w;} } "
		"function foo10(x,y,z,w) { if (x < '\\'}\\'}\\'}\\'') { x+y; w+=1;} else {x; w;} } "
		"function foox( )       {  } "
		"function fooy(x)       {  } "
		"function fooz(x,y)     {  } "
		"function foow(x,y,z)   {  } "
		"function foou(x,y,z,w) {  } "
		"{xxx + yyy + zzz {k / l} }  ";

	function_definition fd;

	string residual = f;

	cout << "***** Example06 *****" << std::endl;

	int function_count = 0;

	do
	{
		if (parse_function_definition(residual, fd))
		{
			string vars;

			for (size_t i = 0; i < fd.var_list.size(); ++i)
			{
				vars += fd.var_list[i] + ((i < fd.var_list.size() - 1) ? "," : "");
			}

			printf("Function[%02d]\n", function_count++);
			printf("Name: %s\n", fd.name.c_str());
			printf("Vars: (%s)\n", vars.c_str());
			printf("Body: \n%s\n", fd.body.c_str());
			printf("-----------------------------\n\n");

			fd.clear();
		}
		else
			break;
	} while (!residual.empty());

	if (!residual.empty())
	{
		printf("Residual: %s\n", residual.c_str());
	}

	std::cout << "*********************" << std::endl;
}

int main()
{
	//example01();
	//example02();
	//example03();
	//example04();
	//example05();
	//example06();

	system("pause");
	return 0;
}
