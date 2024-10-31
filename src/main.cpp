#include <iostream>
#include <memory>
#include "./math/ast.hpp"
#include "./parser/parser.hpp"
#include "./solver/solver.hpp"


int main()
{
	auto axiom1{std::make_shared<ASTNode>(
		0,
		Operation::Implication,
		std::make_shared<ASTNode>(1),
		std::make_shared<ASTNode>(
			0,
			Operation::Implication,
			std::make_shared<ASTNode>(2),
			std::make_shared<ASTNode>(1)
		)
	)};

	auto axiom2{std::make_shared<ASTNode>(
		0,
		Operation::Implication,
		std::make_shared<ASTNode>(
			0,
			Operation::Implication,
			std::make_shared<ASTNode>(1),
			std::make_shared<ASTNode>(
				0,
				Operation::Implication,
				std::make_shared<ASTNode>(2),
				std::make_shared<ASTNode>(3)
			)
		),
		std::make_shared<ASTNode>(
			0,
			Operation::Implication,
			std::make_shared<ASTNode>(
				0,
				Operation::Implication,
				std::make_shared<ASTNode>(1),
				std::make_shared<ASTNode>(2)
			),
			std::make_shared<ASTNode>(
				0,
				Operation::Implication,
				std::make_shared<ASTNode>(1),
				std::make_shared<ASTNode>(3)
			)
		)
	)};

	auto axiom3{std::make_shared<ASTNode>(
		0,
		Operation::Implication,
		std::make_shared<ASTNode>(
			0,
			Operation::Implication,
			std::make_shared<ASTNode>(-2),
			std::make_shared<ASTNode>(-1)
		),
		std::make_shared<ASTNode>(
			0,
			Operation::Implication,
			std::make_shared<ASTNode>(
				0,
				Operation::Implication,
				std::make_shared<ASTNode>(-2),
				std::make_shared<ASTNode>(1)
			),
			std::make_shared<ASTNode>(2)
		)
	)};

	std::string is;
	std::cin >> is;

	auto target_expression = ExpressionParser(is).parse();
	std::cout << "your expression: " << target_expression << "\n\n";

	Solver solver({axiom1, axiom2, axiom3}, target_expression);
	solver.solve();
	std::cout << solver.thought_chain();

	return 0;
}
