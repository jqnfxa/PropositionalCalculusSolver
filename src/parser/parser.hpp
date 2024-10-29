#ifndef PARSER_HPP
#define PARSER_HPP

#include <memory>
#include <string>
#include <cstdint>
#include <stack>
#include <string>
#include "../math/ast.hpp"


class ExpressionParser
{
	// used for `!` unary negation
	bool neg;

	// number of brackets `(` ~ +1; `)` ~ -1
	std::int32_t brackets;

	// input expression to be parsed
	std::string_view expression;

	// stacks for rpn
	std::stack<std::shared_ptr<ASTNode>> operands;
	std::stack<Operation> operations;

	// helper functions
	void construct_node();

	std::int32_t priority(Operation operation);
	bool is_operation(char token);
	Operation determine_operation(char token);
	std::shared_ptr<ASTNode> determine_operand(char token);

public:
	ExpressionParser(std::string_view expression);
	std::shared_ptr<ASTNode> parse();
};


#endif // PARSER_HPP
