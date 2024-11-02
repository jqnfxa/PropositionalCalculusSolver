#include <algorithm>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <stack>
#include <queue>
#include <functional>
#include "ast.hpp"


const auto operation_dict = [] () -> std::unordered_map<Operation, std::string>
{
	return std::unordered_map<Operation, std::string>{
		{Operation::Nop, "Nop"},
		{Operation::Negation, "!"},
		{Operation::Disjunction, "|"},
		{Operation::Conjunction, "*"},
		{Operation::Implication, ">"},
		{Operation::Xor, "+"},
		{Operation::Equivalent, "="}
	};
}();


const auto opposite_operation = [] () -> std::unordered_map<Operation, Operation>
{
	return std::unordered_map<Operation, Operation>{
		{Operation::Nop, Operation::Nop},
		{Operation::Negation, Operation::Negation},
		{Operation::Disjunction, Operation::Conjunction},
		{Operation::Conjunction, Operation::Implication},
		{Operation::Implication, Operation::Conjunction},
		{Operation::Xor, Operation::Equivalent},
		{Operation::Equivalent, Operation::Xor}
	};
}();


bool is_commutative(Operation operation)
{
	// operation: ?, !, >, |, *, +, =
	// commut.  : 0, 0, 0, 1, 1, 1, 1
	// index    : 0, 1, 2, 3, 4, 5, 6
	return static_cast<std::int32_t>(operation) > 2;
}


Operation opposite(Operation operation)
{
	return opposite_operation.at(operation);
}


std::string ASTNode::to_string() const
{
	if (is_leaf())
	{
		if (std::abs(var) == 0 || std::abs(var) > 26)
		{
			return "Nan";
		}

		// vars should not exeed 26 by abs value
		std::string result = var < 0 ? "!" : "";
		result.push_back(std::clamp<char>('a' + std::abs(var) - 1, 'a', 'z'));
		return result;
	}

	return operation_dict.at(op);
}


// TODO: use memorization variables to reduce getting to amortized O(1)
std::size_t Expression::n_ops() const noexcept
{
	std::size_t c = 0;

	for (const auto &token : tokens_)
	{
		if (!token.is_leaf())
		{
			++c;
		}
	}

	return c;
}


// TODO: use memorization variables to reduce getting to amortized O(1)
std::size_t Expression::n_vars() const noexcept
{
	std::size_t c = 0;

	for (const auto &token : tokens_)
	{
		if (token.is_leaf())
		{
			++c;
		}
	}

	return c;
}


std::vector<std::int32_t> Expression::vars() const noexcept
{
	std::vector<std::int32_t> result;
	result.reserve(n_vars());

	for (const auto &token : tokens_)
	{
		if (token.is_leaf())
		{
			result.push_back(token.var);
		}
	}

	return result;
}


bool Expression::contains(std::int32_t needle) const noexcept
{
	for (const auto &token : tokens_)
	{
		if (token == needle)
		{
			return true;
		}
	}

	return false;
}


std::size_t Expression::left(std::size_t index) const noexcept
{
	return in_range(index) ? tokens_[index].left() : INVALID_INDEX;
}


std::size_t Expression::right(std::size_t index) const noexcept
{
	return in_range(index) ? tokens_[index].right() : INVALID_INDEX;
}


std::size_t Expression::parent(std::size_t index) const noexcept
{
	return in_range(index) ? tokens_[index].parent() : INVALID_INDEX;
}


void Expression::insert(std::size_t index, const Expression &expression, std::size_t side) noexcept
{
	if (tokens_.size() == 0)
	{
		tokens_ = expression.tokens_;
		return;
	}

	if (!in_range(index))
	{
		return;
	}

	// not left or right?
	if (side != 1 && side != 2)
	{
		return;
	}

	// function to update indices
	auto update_index = [] (std::size_t index, std::size_t offset)
	{
		if (index == INVALID_INDEX)
		{
			return index;
		}

		return index + offset;
	};

	// idea is to just add new array with updated indices
	std::size_t offset = n_tokens();
	tokens_[index].refs[side] = offset;
	tokens_.emplace_back(
		expression.tokens_[0].var,
		expression.tokens_[0].op,
		offset,
		update_index(expression.tokens_[0].refs[1], offset),
		update_index(expression.tokens_[0].refs[2], offset),
		index
	);

	for (std::size_t i = 1; i < expression.n_tokens(); ++i)
	{
		tokens_.emplace_back(
			expression.tokens_[i].var,
			expression.tokens_[i].op,
			update_index(expression.tokens_[i].refs[0], offset),
			update_index(expression.tokens_[i].refs[1], offset),
			update_index(expression.tokens_[i].refs[2], offset),
			update_index(expression.tokens_[i].refs[3], offset)
		);
	}
}


void Expression::insert(std::size_t index, const ASTNode &node, std::size_t side) noexcept
{
	if (tokens_.empty())
	{
		tokens_.push_back(node);
		return;
	}

	if (!in_range(index))
	{
		return;
	}

	// not left or right?
	if (side != 1 && side != 2)
	{
		return;
	}

	tokens_[index].refs[side] = n_tokens();
	tokens_.emplace_back(
		node.var,
		node.op,
		n_tokens(),
		INVALID_INDEX,
		INVALID_INDEX,
		index
	);
}


void Expression::replace(std::int32_t var, const Expression &expression) noexcept
{
	// find all occurrences
	std::vector<std::int32_t> places;
	for (const auto &token : tokens_)
	{
		if (token.var == var)
		{
			places.push_back(token.id());
		}
	}

	for (const auto &place : places)
	{
		// get parent
		const auto parent = tokens_[place].parent();

		// determine with which edge we connected (left or right)
		const auto side = tokens_[parent].refs[1] == place ? 1 : 2;
		insert(parent, expression, side);
	}
}


Expression Expression::subtree(std::size_t index) const noexcept
{
	std::vector<ASTNode> ast;

	// no subtree
	if (root().is_leaf())
	{
		return {ast};
	}

	// reserve memory
	ast.reserve(tokens_.size());

	// bfs traverse
	std::queue<std::pair<std::size_t, std::size_t>> q;
	std::unordered_map<std::size_t, std::size_t> remapping;
	q.push({index, INVALID_INDEX});
	std::size_t order_index = 0;

	while (!q.empty())
	{
		const auto &[node_idx, parent_index] = q.front();

		// skip invalid nodes
		if (node_idx == INVALID_INDEX)
		{
			q.pop();
			continue;
		}

		// add current node to ast subtree
		ast.emplace_back(
			tokens_[node_idx].var,
			tokens_[node_idx].op,
			order_index,
			INVALID_INDEX,
			INVALID_INDEX,
			parent_index
		);

		// store mapping index to restore relations
		remapping[order_index] = node_idx;

		// push nodes
		if (left(node_idx) != INVALID_INDEX)
		{
			q.emplace(left(node_idx), order_index);
		}
		if (right(node_idx)!= INVALID_INDEX)
		{
			q.emplace(right(node_idx), order_index);
		}

		++order_index;
		q.pop();
	}

	// restore relations
	for (auto &node : ast)
	{
		auto parent = node.parent();
		if (parent == INVALID_INDEX)
		{
			continue;
		}

                // determine with which edge we connected (left or right)
		auto old_node = remapping.at(node.id());
		auto side = tokens_[tokens_[old_node].parent()].refs[1] == old_node ? 1 : 2;
		ast[parent].refs[side] = node.id();
	}

	return {ast};
}


void Expression::negation_inplace(std::size_t index)
{
	if (!in_range(index))
	{
		return;
	}

	// bfs traverse
	std::queue<std::size_t> q;
	q.push(index);

	while (!q.empty())
	{
		const auto node_idx = q.front();
		q.pop();

		// skip invalid nodes
		if (node_idx == INVALID_INDEX)
		{
			continue;
		}

		if (tokens_[node_idx].is_leaf())
		{
			tokens_[node_idx].var *= -1;
			continue;
		}

		// inverse operation
		tokens_[node_idx].op = opposite(tokens_[node_idx].op);

		// continue negation if required
		if (tokens_[node_idx].op == Operation::Implication ||
			tokens_[node_idx].op == Operation::Conjunction)
		{
			q.push(tokens_[node_idx].right());
		}
		else if (tokens_[node_idx].op == Operation::Disjunction)
		{
			q.push(tokens_[node_idx].left());
			q.push(tokens_[node_idx].right());
		}
	}
}


Expression Expression::negation() const
{
	Expression new_expression{*this};
	new_expression.negation_inplace(0);
	return new_expression;
}


std::string Expression::to_string() const
{
	std::ostringstream oss;
	oss << *this;
	return oss.str();
}


Expression contruct_expression(
	const Expression &lhs,
	Operation op,
	const Expression &rhs
)
{
	// create storage
	std::vector<ASTNode> expression;
	expression.reserve(lhs.n_tokens() + rhs.n_tokens() + 1);
	expression.emplace_back(0, op, 0);

	Expression new_expression(std::move(expression));
	new_expression.insert(0, lhs, 1);
	new_expression.insert(0, rhs, 2);
	return new_expression;
}


std::ostream &operator<<(std::ostream &out, const Expression &expression)
{
	std::function<std::ostream &(
		std::ostream &,
		const Expression &,
		std::size_t
	)> f = [&] (std::ostream &out, const Expression &expression, std::size_t idx) -> std::ostream &
	{
		if (idx == INVALID_INDEX)
		{
			return out;
		}

		const bool should_use_brackets =
			expression[idx].parent() != INVALID_INDEX &&
			!expression[idx].is_leaf();
		if (should_use_brackets)
		{
			out << "(";
		}

		f(out, expression, expression.left(idx));
		out << expression[idx].to_string();
		f(out, expression, expression.right(idx));

		if (should_use_brackets)
		{
			out << ")";
		}

		return out;
	};

	f(out, expression, 0);
	return out;
}
