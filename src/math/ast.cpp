#include <algorithm>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <stack>
#include <queue>
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



constexpr const std::int32_t op_priority[] = {0, 5, 3, 4, 1, 2, 2};
std::int32_t priority(Operation operation)
{
	return op_priority[static_cast<std::int32_t>(operation)];
}


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

	for (const auto &token : tokens)
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

	for (const auto &token : tokens)
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

	for (const auto &token : tokens)
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
	for (const auto &token : tokens)
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
	return in_range(index) ? tokens[index].left() : INVALID_INDEX;
}


std::size_t Expression::right(std::size_t index) const noexcept
{
	return in_range(index) ? tokens[index].right() : INVALID_INDEX;
}


std::size_t Expression::parent(std::size_t index) const noexcept
{
	return in_range(index) ? tokens[index].parent() : INVALID_INDEX;
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
	ast.reserve(tokens.size());

	// bfs traverse
	std::queue<std::pair<ASTNode, std::size_t>> q;
	q.push({left(index), INVALID_INDEX});
	std::size_t order_index = 0;

	while (!q.empty())
	{
		const auto &[node, parent_idx] = q.front();

		// skip invalid nodes
		if (node.id() == INVALID_INDEX)
		{
			continue;
		}

		// add current node to ast subtree
		ast.emplace_back(
			node.var,
			node.op,
			order_index,
			order_index + 1,
			order_index + 2,
			parent_idx
		);

		// push nodes
		q.emplace(left(node.id()), index);
		q.emplace(right(node.id()), index);

		q.pop();
	}

	return {ast};
}


void Expression::negation(std::size_t index)
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
		const auto [node_idx] = q.front();
		q.pop();

		// skip invalid nodes
		if (node_idx == INVALID_INDEX)
		{
			continue;
		}

		if (result[node_idx].is_leaf())
		{
			result[node_idx].var *= -1;
			continue;
		}

		// inverse operation
		result[node_idx].op = opposite(result[node_idx].op);

		// continue negation if required
		if (result[node_idx].op == Operation::Implication ||
			result[node_idx].op == Operation::Conjunction)
		{
			result[node_idx].op = opposite(result[node_idx].op);
			q.push(result[node_idx].right());
		}
		else if (result[node_idx].op == Operation::Disjuction)
		{
			q.push(result[node_idx].left());
			q.push(result[node_idx].right());
		}
	}
}


static Expression Expression::negation(const Expression &expression)
{
	Expression new_expression{expression};
	new_expression.negation(0);
	return new_expression;
}


std::ostream &operator<<(std::ostream &out, const Expression &expression)
{
	std::stack<ASTNode> st;
	auto current = expression.root();

	while (!st.empty() || current.id() != INVALID_INDEX)
	{
		// push left child onto stack and move to leftmost node
		while (current.id() != INVALID_INDEX)
		{
			st.push(current);
			current = expression.left(current.id());
		}

		// pop top node from stack and visit it
		current = st.top();
		st.pop();

		// Print the value of the popped node
		out << current.to_string();

		// move to right subtree
		current = expression.right(current.id());
	}

	return out;
}
