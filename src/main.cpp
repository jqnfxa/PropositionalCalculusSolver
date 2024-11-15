#include <iostream>
#include <vector>
#include "./math/ast.hpp"
#include "./math/rules.hpp"
#include "./solver/solver.hpp"
#include "./math/helper.hpp"


int main()
{
	// (isr) implication swap rule: (!a>!b)>(b>a)
	/*Expression ax1("a>(b>a)");
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

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}*/

	// ax5 (a*b>b)
	/*Expression ax1("a>(b>a)");
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
	}*/

	// ax11 (a>a)
	/*
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	std::vector<Expression> axioms = { ax1, ax2 };

	axioms.push_back(modus_ponens(axioms[0], axioms[1]));
	axioms.push_back(modus_ponens(axioms[0], axioms[2]));

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	// ax7 (a>(!a>b)))
	/*
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");
	Expression isr("(!a>!b)>(b>a)");
	std::vector<Expression> axioms = { ax1, ax2, ax3, isr };

	axioms.push_back(modus_ponens(axioms[0], axioms[0]));
	axioms.push_back(modus_ponens(axioms[3], axioms[0]));
	axioms.push_back(modus_ponens(axioms[1], axioms[1]));
	axioms.push_back(modus_ponens(axioms[2], axioms[1]));
	axioms.push_back(modus_ponens(axioms[5], axioms[1]));
	axioms.push_back(modus_ponens(axioms[0], axioms[8]));

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	// ax8 (b>(!a>b)))
	/*
	Expression ax1("a>(b>a)");
	std::vector<Expression> axioms = { ax1 };

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	// ax10 (!a>(a>b)))
	/*
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax_swap_rule("(!a>!b)>(b>a)");
	std::vector<Expression> axioms = { ax1, ax2, ax_swap_rule };

	axioms.push_back(modus_ponens(axioms[2], axioms[0]));
	axioms.push_back(modus_ponens(axioms[3], axioms[1]));
	axioms.push_back(modus_ponens(axioms[0], axioms[4]));

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << axioms[i] << '\n';
	}
	*/

	// ax4 (a*b>a)
	/*
	Expression ax10("!a>(a>b)");
	Expression isr("(!a>!b)>(b>a)");
	Expression guess("a*b>a");
	guess.make_permanent();

	mp(ax4, isr) = P>(!P>!Q) ax10
	*/

	// ax6 (a>(b>(a*b)))
	/*Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax_swap_rule("(a>!b)>(b>!a)");
	Expression ax11("a>a");
	std::vector<Expression> axioms = { ax1, ax2, ax_swap_rule, ax11 };

	axioms.push_back(modus_ponens(axioms[0], axioms[0]));
	axioms.push_back(modus_ponens(axioms[2], axioms[0]));
	axioms.push_back(modus_ponens(axioms[1], axioms[1]));
	axioms.push_back(modus_ponens(axioms[3], axioms[1]));
	axioms.push_back(modus_ponens(axioms[7], axioms[0]));
	axioms.push_back(modus_ponens(axioms[5], axioms[1]));
	axioms.push_back(modus_ponens(axioms[4], axioms[6]));
	axioms.push_back(modus_ponens(axioms[8], axioms[10]));
	axioms.push_back(modus_ponens(axioms[11], axioms[9]));

	std::vector<std::pair<int, int>> order = {
		{-1, -1},
		{-1, -1},
		{-1, -1},
		{-1, -1},
		{0, 0},
		{2, 0},
		{1, 1},
		{3, 1},
		{7, 0},
		{5, 1},
		{4, 6},
		{8, 10},
		{11, 9},
	};

	for (std::size_t i = 0; i < axioms.size(); ++i)
	{
		std::cout << i << ' ' << "mp(" << order[i].first << ',' << order[i].second << "): " << axioms[i] << '\n';
	}*/

	//std::cout << axioms.back() << '\n';
	//Expression a("a>a");
	//Expression b("((a>b)>a)>((a>sb)>a)");

	//std::cout << modus_ponens(a,b) << '\n';

	/*Expression a("!a>!a");
	Expression b("(a>b)>(a>b)");
	Expression c("!a>!a");

	auto got = modus_ponens(a, b);
	std::cerr << got << ' ' << c << '\n';*/

	/*std::string expression_str = "c";
	//std::cin >> expression_str;
	Expression target(expression_str);
	//target.normalize();
	target.make_permanent();

	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");
	Expression ax4("a*b>a");
	Expression ax5("a*b>b");
	Expression ax6("a>(b>(a*b))");
	Expression ax7("a>(!a>b)");
	Expression ax8("b>(!a>b)");
	Expression ax10("!a>(a>b)");
	Expression isr("(!a>!b)>(b>a)");
	Expression idk("(a>(b*c))>(a>b)");
	Expression ax11("a>a");

	Expression _1("a>b");
	Expression _2("b>c");
	Expression _3("!a>b");

	_1.make_permanent();
	_2.make_permanent();
	_3.make_permanent();

	std::vector<Expression> axioms = {
		ax1,
		ax2,
		ax3,
		ax4,
		ax5,
		ax6,
		ax7,
		ax8,
		ax10,
		isr,
		idk,
		ax11,
		_1,
		_2,
		_3
	};

	std::cerr << "you input: " << target << '\n';

	Solver solve(axioms, target, 120000);
	solve.solve();

	std::cout << solve.thought_chain() << '\n';*/

	std::string expression_str = "!a>(a>b)";
	//std::cin >> expression_str;
	Expression target(expression_str);
	//target.normalize();
	target.make_permanent();

	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");

	std::vector<Expression> axioms = {
		ax1,
		ax2,
		ax3
	};

	std::cerr << "you input: " << target << '\n';

	Solver solve(axioms, target, 120000);
	solve.solve();

	std::cout << solve.thought_chain() << '\n';
	return 0;
}

/*
P->(Q->P);                      # 1;
(P->(Q->R))->((P->Q)->(P->R));  # 2;
(!P->!Q)->((!P->Q)->P); # 3;
!(P->!Q)->P; # 4;
!(P->!Q)->Q; # 5;
!P->(P->!Q); # 4inv;
!Q->(P->!Q); # 5inv;
P->(Q->!(P->!Q)); # 6;
(!P->!Q)->(Q->P); # isr;
P->(!P->Q); # 7;
Q->(!P->Q); # 8;
!P->(P->Q); # 10;
P->P; # 11;
!P->!P; # 11 real;
*/
