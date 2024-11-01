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
		{Operation::Disjunction, "*"},
		{Operation::Conjunction, "|"},
		{Operation::Implication, ">"},
		{Operation::Xor, "+"},
		{Operation::Equivalent, "="}
	};
}();


constexpr const std::int32_t op_priority[] = {0, 5, 3, 4, 1, 2, 2};
std::int32_t priority(Operation operation)
{
	return op_priority[static_cast<std::int32_t>(operation)];
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
	return in_range(index) ? tokens[index].refs[1] : INVALID_INDEX;
}


std::size_t Expression::right(std::size_t index) const noexcept
{
	return in_range(index) ? tokens[index].refs[2] : INVALID_INDEX;
}


std::size_t Expression::parent(std::size_t index) const noexcept
{
	return in_range(index) ? tokens[index].refs[3] : INVALID_INDEX;
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

	// preorder traverse
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
