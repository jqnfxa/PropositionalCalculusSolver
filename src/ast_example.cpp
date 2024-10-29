#include "./math/ast.hpp"
#include <iostream>
#include <memory>


int main()
{
	auto axiom1{std::make_shared<ASTNode>(
		0,
		Operation::Implication,
		new ASTNode(1),
		new ASTNode(
			0,
			Operation::Implication,
			new ASTNode(1),
			new ASTNode(2)
		)
	)};

	auto axiom2{std::make_shared<ASTNode>(
		0,
		Operation::Implication,
		new ASTNode(
			0,
			Operation::Implication,
			new ASTNode(1),
			new ASTNode(
				0,
				Operation::Implication,
				new ASTNode(2),
				new ASTNode(3)
			)
		),
		new ASTNode(
			0,
			Operation::Implication,
			new ASTNode(
				0,
				Operation::Implication,
				new ASTNode(1),
				new ASTNode(2)
			),
			new ASTNode(
				0,
				Operation::Implication,
				new ASTNode(1),
				new ASTNode(3)
			)
		)
	)};

	std::cout << axiom1 << std::endl;
	std::cout << axiom2 << std::endl;
	return 0;
}
