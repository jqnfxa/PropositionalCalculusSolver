#include <iostream>
#include "./math/ast.hpp"
#include "./parser/parser.hpp"


int main()
{
	Expression axiom1("(!a>(b>c))>((b>c)>(!a>c))");
	Expression axiom2("(a>(b>a))");
	axiom1.replace(1, axiom2);
	std::cout << axiom1.to_string() << std::endl;
	return 0;
}
