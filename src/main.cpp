#include <iostream>
#include <vector>
#include "./math/ast.hpp"
#include "./math/rules.hpp"
#include "./solver/solver.hpp"
#include "./math/helper.hpp"


int main()
{
	std::string expression_str = "(a>c)>((b>c)>((a|b)>c))";
	//std::cin >> expression_str;
	Expression target(expression_str);
	target.standardize();
	target.make_permanent();

	std::vector<Expression> axioms = {
		Expression("a>(b>a)"),
		Expression("(a>(b>c))>((a>b)>(a>c))"),
		Expression("(!a>!b)>((!a>b)>a)"),
		Expression("a>(!a>b)"),
		Expression("a*b>a"),
		Expression("a*b>b"),
		Expression("a>(b>(a*b))"),
		Expression("a>a")
	};

	std::cerr << "your input: " << target << '\n';

	Solver solve(axioms, target, 10000);
	solve.solve();

	std::cout << solve.thought_chain() << '\n';
	return 0;
}

// ax4 - ok a*b>a
// ax5 - ok a*b>b
// ax6 - ok a>(b>(a*b))
// ax7 - ok a>(a|b)
// ax8 - ok b>(a|b)
// ax9 - ? (a>c)>((b>c)>((a|b)>c))
// ax10 - ok !a>(a>b)
// ax11 - ok a|!a
