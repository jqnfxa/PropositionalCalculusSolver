#include <iostream>
#include <vector>
#include "./math/ast.hpp"
#include "./math/rules.hpp"
#include "./solver/solver.hpp"
#include "./math/helper.hpp"


int main()
{
	std::string expression_str = "a>(b>(!a>b))";
	//std::cin >> expression_str;
	Expression target(expression_str);
	//target.make_permanent();

	std::cout << "Your input: " << target << '\n';

	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!b>!a)>((!b>a)>b)");
	//Expression ax11("a>a");
	//Expression ax112("!a>!a");
	std::vector<Expression> axioms = { ax1, ax2, ax3 };

	Solver solve(axioms, target);
	solve.solve();

	std::cout << solve.thought_chain() << '\n';
	return 0;
}
