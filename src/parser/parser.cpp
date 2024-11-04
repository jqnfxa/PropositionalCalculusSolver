#include <stack>
#include <cstdint>
#include <unordered_map>
#include <iostream>
#include "parser.hpp"


const auto char_to_op = [] () -> std::unordered_map<char, Operation>
{
	return std::unordered_map<char, Operation>{
		{'\0', Operation::Nop},
		{'!', Operation::Negation},
		{'|', Operation::Disjunction},
		{'*', Operation::Conjunction},
		{'>', Operation::Implication},
		{'+', Operation::Xor},
		{'=', Operation::Equivalent}
	};
}();


/**
 * @note: higher priority - operation must be executed first
 * https://studopedia.ru/13_131857_prioritet-logicheskih-operatsiy.html
 */
constexpr const std::int32_t pri[] = {0, 5, 1, 3, 4, 2, 2, 0, 0};
std::int32_t priority(Token operation)
{
	return pri[static_cast<std::int32_t>(operation)];
}


std::int32_t priority(Operation operation)
{
	return pri[static_cast<std::int32_t>(operation)];
}


ExpressionParser::ExpressionParser(std::string_view expression)
	: brackets(0)
	, expression(expression)
	, operands{}
	, operations{}
{}


void ExpressionParser::construct_node()
{
	if (operations.top() == Token::Negation)
	{
		// extract nodes
		auto operand = operands.top();
		operands.pop();
		operations.pop();

		operand.negation_inplace(0);
		operands.push(operand);
		return;
	}

	if (operands.size() < 2 || operations.size() < 1)
	{
		if (operations.top() == Token::OpenBracket ||
			operations.top() == Token::CloseBracket)
		{
			throw std::runtime_error("incorrect parentheses");
		}

		throw std::runtime_error("something went wrong in construct node");
		return;
	}

	// extract nodes
	const auto rhs = operands.top();
	operands.pop();
	const auto op = static_cast<Operation>(static_cast<std::int32_t>(operations.top()));
	operations.pop();
	const auto lhs = operands.top();
	operands.pop();

	// add produced node
	operands.emplace(contruct_expression(lhs, op, rhs));
}


bool ExpressionParser::is_operation(char token)
{
	return char_to_op.contains(token);
}


Operation ExpressionParser::determine_operation(char token)
{
	if (!is_operation(token))
	{
		return Operation::Nop;
	}

	return char_to_op.at(token);
}


ASTNode ExpressionParser::determine_operand(char token)
{
	if (!('a' <= token && token <= 'z'))
	{
		throw std::runtime_error("invalid variable name");
	}

	return {token - 'a' + 1, Operation::Nop, 0};
}


Expression ExpressionParser::parse()
{
	bool last_token_is_op = false;
	for (const auto &token : expression)
	{
		if (std::isspace(token))
		{
			continue;
		}

		if (token == '(')
		{
			operations.push(Token::OpenBracket);
			last_token_is_op = false;
			continue;
		}

		if (token == ')')
		{
			if (operations.empty())
			{
				throw std::runtime_error("incorrect parentheses");
			}

			while (!operations.empty() && operations.top() != Token::OpenBracket)
			{
				construct_node();
			}

			// pop open bracket
			operations.pop();
			last_token_is_op = false;
			continue;
		}

		if (is_operation(token))
		{
			const auto op = determine_operation(token);
			if (op == Operation::Negation)
			{
				// "!(!a)" -> "a"
				operations.push(static_cast<Token>(static_cast<std::int32_t>(op)));
				continue;
			}

			if (last_token_is_op)
			{
				throw std::runtime_error("incorrect input:"
				" multiple operations defined one by one");
			}

			last_token_is_op  = true;
			while (!operations.empty() &&
				priority(operations.top()) > priority(op))
			{
				construct_node();
			}

			operations.push(static_cast<Token>(static_cast<std::int32_t>(op)));
		}
		else
		{
			last_token_is_op = false;
			operands.emplace(std::vector<ASTNode>{determine_operand(token)});
		}
	}

	while (!operations.empty())
	{
		construct_node();
	}

	return operands.top();
}
