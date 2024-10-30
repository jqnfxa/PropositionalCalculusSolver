#include "./math/ast.hpp"
#include "./math/helper.hpp"
#include <iostream>
#include <memory>


int main()
{
	std::shared_ptr<ASTNode> node;

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

	const auto rules = unification(axiom1, axiom2->left);
	unify(axiom2, rules);
	std::cout << axiom2 << '\n';
	}

	return 0;
}
