#include <iostream>
#include <vector>
#include "./math/ast.hpp"
#include "./math/rules.hpp"
#include "./solver/solver.hpp"
#include "./math/helper.hpp"


int main()
{
	/* implication swap rule: (a>!b)>(b>!a)
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");
	std::vector<Expression> axioms = { ax1, ax2, ax3 };

	axioms.push_back(modus_ponens(axioms[0], axioms[0]));
	axioms.push_back(modus_ponens(axioms[1], axioms[0]));
	axioms.push_back(modus_ponens(axioms[3], axioms[1]));
	axioms.push_back(modus_ponens(axioms[4], axioms[1]));
	axioms.push_back(modus_ponens(axioms[2], axioms[5]));
	axioms.push_back(modus_ponens(axioms[6], axioms[6]));
	axioms.push_back(modus_ponens(axioms[7], axioms[8]));
	axioms.push_back(modus_ponens(axioms[3], axioms[9]));

	for (const auto &axiom : axioms)
	{
		std::cout << axiom << '\n';
	}
	*/

	/*
	ax4 (a*b>a) ~ !a>!(a*b) ~ !a>(a>!b) (ax10)
	*/

	/* ax5 (a*b>b)
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");
	std::vector<Expression> axioms = { ax1, ax2, ax3 };

	axioms.push_back(modus_ponens(axioms[0], axioms[0]));
	axioms.push_back(modus_ponens(axioms[1], axioms[1]));
	axioms.push_back(modus_ponens(axioms[2], axioms[1]));
	axioms.push_back(modus_ponens(axioms[3], axioms[4]));
	axioms.push_back(modus_ponens(axioms[3], axioms[5]));
	axioms.push_back(modus_ponens(axioms[7], axioms[0]));
	axioms.push_back(modus_ponens(axioms[8], axioms[6]));

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	/* ax11 (a>a)
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");
	std::vector<Expression> axioms = { ax1, ax2, ax3 };

	axioms.push_back(modus_ponens(axioms[0], axioms[1]));
	axioms.push_back(modus_ponens(axioms[0], axioms[2]));

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	/* ax7 (a>(!a>b)))
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");
	Expression ax_swap_rule("(!a>!b)>(b>a)");
	Expression ax5("(!a>!b)>(b>a)");
	std::vector<Expression> axioms = { ax1, ax2, ax3, ax_swap_rule, ax5 };

	axioms.push_back(modus_ponens(axioms[2], axioms[0]));
	axioms.push_back(modus_ponens(axioms[3], axioms[2]));
	axioms.push_back(modus_ponens(axioms[4], axioms[1]));
	axioms.push_back(modus_ponens(axioms[5], axioms[6]));

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	/* ax8 (b>(!a>b)))
	Expression ax1("a>(b>a)");
	std::vector<Expression> axioms = { ax1 };

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	/* ax10 (!a>(a>b)))
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax_swap_rule("(!a>!b)>(b>a)");
	std::vector<Expression> axioms = { ax1, ax2, ax_swap_rule };

	axioms.push_back(modus_ponens(axioms[2], axioms[0]));
	axioms.push_back(modus_ponens(axioms[3], axioms[2]));
	axioms.push_back(modus_ponens(axioms[0], axioms[4]));

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	/* ax6 (a>(b>!(a->!b)))
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax_swap_rule("(a>!b)>(b>!a)");
	Expression ax11("a>a");
	std::vector<Expression> axioms = { ax1, ax2, ax3, ax_swap_rule, ax11 };

	axioms.push_back(modus_ponens(axioms[0], axioms[0]));
	axioms.push_back(modus_ponens(axioms[2], axioms[0]));
	axioms.push_back(modus_ponens(axioms[1], axioms[1]));
	axioms.push_back(modus_ponens(axioms[3], axioms[1]));
	axioms.push_back(modus_ponens(axioms[7], axioms[0]));
	axioms.push_back(modus_ponens(axioms[5], axioms[1]));
	axioms.push_back(modus_ponens(axioms[4], axioms[6]));
	axioms.push_back(modus_ponens(axioms[8], axioms[10]));
	axioms.push_back(modus_ponens(axioms[11], axioms[9]));

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/


	//std::cout << axioms.back() << '\n';
	//Expression a("a>a");
	//Expression b("((a>b)>a)>((a>b)>a)");

	//std::cout << modus_ponens(a,b) << '\n';

	/*std::string expression_str = "!a>!a";
	//std::cin >> expression_str;
	Expression target(expression_str);
	//target.make_permanent();

	std::cout << "Your input: " << target << '\n';

	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");
	//Expression ax11("a>a");
	//Expression ax112("!a>!a");
	std::vector<Expression> axioms = { ax1, ax2, ax3 };

	Solver solve(axioms, target);
	solve.solve();

	std::cout << solve.thought_chain() << '\n';*/
	return 0;
}
