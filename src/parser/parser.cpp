#include <stack>
#include <cstdint>
#include "parser.hpp"


ExpressionParser::ExpressionParser(std::string_view expression)
	: neg(false)
	, brackets(0)
	, expression(expression)
	, operands{}
	, operations{}
{}


void ExpressionParser::construct_node()
{
	// extract nodes
	auto rhs = operands.top();
	operands.pop();
	auto op = operations.top();
	operations.pop();
	auto lhs = operands.top();
	operands.pop();

	// add produced node
	operands.push(std::make_shared<ASTNode>(0, op, lhs, rhs));
}


bool ExpressionParser::is_operation(char token)
{
	return token == '!' || token == '|' || token == '*' ||
		token == '>' || token == '+' || token == '=';
}


Operation ExpressionParser::determine_operation(char token)
{
	switch (token)
	{
		case '!':
			return Operation::Negation;
		case '|':
			return Operation::Disjunction;
		case '*':
			return Operation::Conjunction;
		case '>':
			return Operation::Implication;
		case '+':
			return Operation::Xor;
		case '=':
			return Operation::Equivalent;
		default:
		break;
	}

	return Operation::Nop;
}


std::shared_ptr<ASTNode> ExpressionParser::determine_operand(char token)
{
	if (!('a' <= token && token <= 'z'))
	{
		throw std::runtime_error("invalid variable name");
	}

        auto value = token - 'a' + 1;
	if (neg)
	{
		value *= -1;
		neg = !neg;
	}

	return std::make_shared<ASTNode>(value, Operation::Nop);
}


std::shared_ptr<ASTNode> ExpressionParser::parse()
{
	bool last_token_is_operation = false;
	for (const auto &token : expression)
	{
		if (token == ' ')
		{
			continue;
		}

		if (token == '(')
		{
			++brackets;
			last_token_is_operation = false;
			continue;
		}

		if (token == ')')
		{
			--brackets;
			if (brackets < 0)
			{
				throw std::runtime_error("incorrect parentheses");
			}

			construct_node();
			last_token_is_operation = false;
			continue;
		}

		if (is_operation(token))
		{
			const auto op = determine_operation(token);
			if (op == Operation::Negation)
			{
				neg = !neg;
				continue;
			}

			if (last_token_is_operation)
			{
				throw std::runtime_error("incorrect input:"
				" multiple operations defined one by one");
			}
			last_token_is_operation = true;

			if (!operations.empty() &&
				priority(operations.top()) >= priority(op))
			{
				construct_node();
			}

			operations.push(op);
		}
		else
		{
			last_token_is_operation = false;
			operands.push(std::move(determine_operand(token)));
		}
	}

        if (brackets > 0)
	{
		throw std::runtime_error("incorrect parentheses");
	}

	while (!operations.empty())
	{
		construct_node();
	}

	return operands.top();
}
