#include <iostream>
#include "./math/ast.hpp"
#include "./math/rules.hpp"


int main()
{
	Expression axiom1("(a>(b>a))");
	Expression axiom2("(a>(b>c))>((a>b)>(a>c))");

	Expression e1 = modus_ponens(axiom1, axiom2);
	Expression e2 = modus_ponens(axiom1, e1);

	std::cout << e1.to_string() << '\n';
	std::cout << e2.to_string() << '\n';
	return 0;
}
