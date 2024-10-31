#ifndef AST_HPP
#define AST_HPP

#include <cstdint>
#include <memory>
#include <ostream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <functional>


enum class Operation : std::int32_t
{
	Nop = 0,
	Negation,
	Disjunction,
	Conjunction,
	Implication,
	Xor,
	Equivalent
};


/**
 * higher priority - operation must be executed first
 */
std::int32_t priority(Operation operation);


struct ASTNode
{
	std::int32_t var;
	Operation op;
	std::shared_ptr<ASTNode> left;
	std::shared_ptr<ASTNode> right;
	ASTNode *parent;

public:
	/**
	 * Construction
	 */
	ASTNode(std::int32_t var = 0,
		Operation op = Operation::Nop,
		std::shared_ptr<ASTNode> left = nullptr,
		std::shared_ptr<ASTNode> right = nullptr,
		ASTNode *parent = nullptr);
	~ASTNode() = default;

	/**
	 * Deepcopy contruction is not allowed, use `deepcopy` instead
	 */
	ASTNode(const ASTNode &other) = delete;
	ASTNode &operator=(const ASTNode &other) = delete;

	/**
	 * Move semantic is ok
	 */
	ASTNode(ASTNode &&other);
	ASTNode &operator=(ASTNode &&other);

	inline bool is_leaf() const noexcept
	{
		// should be just `var != 0`?
		return var != 0;
	}

	inline std::string stringify() const noexcept
	{
		if (is_leaf())
		{
			// vars should not exeed 26 by abs value
			std::string result = var < 0 ? "!" : "";
			result.push_back(std::clamp<char>('a' + std::abs(var) - 1, 'a', 'z'));
			return result;
		}

		switch (op)
		{
			case Operation::Nop:
				return "Nop";
			case Operation::Negation:
				return "!";
			case Operation::Disjunction:
				return "|";
			case Operation::Conjunction:
				return "*";
			case Operation::Implication:
				return ">";
			case Operation::Xor:
				return "+";
			case Operation::Equivalent:
				return "=";
			default:
			break;
		}

		return "unknown";
	}

	std::string to_string() const;
	std::shared_ptr<ASTNode> deepcopy() const;
	std::int32_t depth() const;
	std::int32_t operations() const;
	std::vector<std::int32_t> values() const;
	bool contains(const std::int32_t v) const;
};


/**
 * Recursive operator
 */
std::ostream &operator<<(std::ostream &out, const ASTNode *node);
std::ostream &operator<<(std::ostream &out, const std::shared_ptr<ASTNode> &node);

using expression_t = std::shared_ptr<ASTNode>;

struct Expression
{
	expression_t expression;
	std::int32_t depth;
};

#endif // AST_HPP
