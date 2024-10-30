#include <iostream>
#include <memory>
#include "./math/ast.hpp"
#include "./parser/parser.hpp"
#include "./solver/solver.hpp"


int main()
{
	//ExpressionParser parser{"(a>(b>c))>((a>b)>(a>c))"};
	//ExpressionParser parser{is};
	//auto expression = parser.parse();

	// in: 	(a>(b>c))>((a>b)>(a>c))
	// out: (a>(b>c))>((a>b)>(a>c))
	// in: 	(a+b)|c*d
	// out: (a+b)|(c*d)
	// in:	a|b*c
	// out: a|(b*c)
	// in: 	a*b|c
	// out: (a*b)|c
	// in: a|b|c
	// out: (a|b)|c
	// in: !a|b>c+e*!f
	// out: (!a|b)>(c+(e*!f))
	// in: !a
	// out: !a
	// in: !!!a
	// out: !a
	// in: !!a
	// out: a
	// in: !!!!!!!!!!!!!!!!a|!!!!!!f
	// out: a|f

	std::string is;
	std::cin >> is;

	auto target_expression = ExpressionParser(is).parse();
	std::cout << "your expression: " << target_expression << "\n\n";

	Solver solver({}, target_expression);
	solver.solve();
	std::cout << solver.thought_chain();

	return 0;
}
