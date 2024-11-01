#include <iostream>
#include "./math/ast.hpp"
#include "./parser/parser.hpp"


int main()
{
	std::string is;
	std::cin >> is;

	auto target_expression = ExpressionParser(is).parse();
	std::cout << "your expression: " << target_expression << "\n";

	return 0;
}
