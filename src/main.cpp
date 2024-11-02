#include <iostream>
#include "./math/ast.hpp"
#include "./parser/parser.hpp"


int main()
{
	std::string is;
	//std::cin >> is;
	Expression axiom1 = ExpressionParser("a>(b>a)").parse();
	Expression axiom2 = ExpressionParser("(a>(b>c))>((b>c)>(a>c))").parse();
	Expression a = ExpressionParser("a").parse();
	std::cout << axiom1 << '\n';
	axiom2.replace(3, a);
	std::cout << axiom2 << '\n';
	std::cout << axiom2.subtree(axiom2.right(0));
	return 0;
}
