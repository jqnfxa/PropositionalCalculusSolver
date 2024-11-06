#include <algorithm>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <stack>
#include <queue>
#include <functional>
#include <string>
#include "ast.hpp"
#include "../parser/parser.hpp"


const auto operation_dict = [] () -> std::unordered_map<operation_t, std::string>
{
	return std::unordered_map<operation_t, std::string>{
		{operation_t::Nop, "Nop"},
		{operation_t::Negation, "!"},
		{operation_t::Disjunction, "|"},
		{operation_t::Conjunction, "*"},
		{operation_t::Implication, ">"},
		{operation_t::Xor, "+"},
		{operation_t::Equivalent, "="}
	};
}();


const auto opposite_operation = [] () -> std::unordered_map<operation_t, operation_t>
{
	return std::unordered_map<operation_t, operation_t>{
		{operation_t::Nop, operation_t::Nop},
		{operation_t::Negation, operation_t::Negation},
		{operation_t::Disjunction, operation_t::Conjunction},
		{operation_t::Conjunction, operation_t::Implication},
		{operation_t::Implication, operation_t::Conjunction},
		{operation_t::Xor, operation_t::Equivalent},
		{operation_t::Equivalent, operation_t::Xor}
	};
}();


operation_t opposite(operation_t operation_t)
{
	return opposite_operation.at(operation_t);
}


Term::Term(term_t type, operation_t op, value_t value) noexcept
	: type(type)
	, op(op)
	, value(value)
{}


std::string Term::to_string() const noexcept
{
	std::stringstream representation;

	if (type == term_t::None)
	{
		representation << "None";
		return representation.str();
	}

	if (type == term_t::Function)
	{
		representation << operation_dict.at(op);
	}
	else
	{
		if (op == operation_t::Negation)
		{
			representation << '!';
		}

		if (type == term_t::Constant)
		{
			char suitable_constant = std::abs(value) - 1 + 'a';
			representation << suitable_constant;
		}
		else
		{
			// TODO: think what if var value > 9 ?
			representation << std::abs(value);
		}
	}

	return representation.str();
}


Expression::Expression() = default;


Expression::Expression(std::string_view expression)
{
	nodes_ = std::move(ExpressionParser(expression).parse().nodes_);
}


Expression::Expression(Term term)
{
	nodes_.emplace_back(
		term,
		Relation(0)
	);
}


Expression::Expression(const Expression &other) : nodes_(other.nodes_)
{}


Expression::Expression(Expression &&other) : nodes_(std::move(other.nodes_))
{}


Expression &Expression::operator=(const Expression &other)
{
	if (this == &other)
	{
		return *this;
	}

	nodes_ = other.nodes_;
	return *this;
}


Expression &Expression::operator=(Expression &&other)
{
	if (this == &other)
	{
		return *this;
	}

	nodes_ = std::move(other.nodes_);
	return *this;
}


std::size_t Expression::size() const noexcept
{
	return nodes_.size();
}


bool Expression::empty() const noexcept
{
	return nodes_.empty();
}


std::string Expression::to_string() const noexcept
{
	std::stringstream ss;
	if (empty())
	{
		ss << "empty";
		return ss.str();
	}

	//TODO: fix wrong indices
	std::stack<Relation> stack;
	Relation current = nodes_[0].rel;

	while (current.self() != INVALID_INDEX || !stack.empty())
	{
		// go as far left as possible
		while (current.self() != INVALID_INDEX)
		{
			stack.push(current);
			current = current.left();
		}

		// backtrack and print
		current = stack.top();
		stack.pop();

		// print brackets
		if (has_left(current.self()) && has_right(current.self()))
		{
			ss << "(";
		}

		ss << nodes_[current.self()].term.to_string();

		// print brackets
		if (has_left(current.self()) && has_right(current.self()))
		{
			ss << ")";
		}

		// move to right subtree
		current = current.right();
	}

	return ss.str();
}


Relation Expression::subtree(std::size_t idx) const noexcept
{
	return in_range(idx) ? nodes_[idx].rel : Relation{};
}


bool Expression::has_left(std::size_t idx) const noexcept
{
	return in_range(idx) ? false : in_range(nodes_[idx].rel.left());
}


bool Expression::has_right(std::size_t idx) const noexcept
{
	return in_range(idx) ? false : in_range(nodes_[idx].rel.right());
}


void Expression::negation(std::size_t idx)
{
	if (!in_range(idx))
	{
		return;
	}

	// bfs traverse
	std::queue<std::size_t> q;
	q.push(idx);

	while (!q.empty())
	{
		const auto node_idx = q.front();
		q.pop();

		// skip invalid nodes
		if (node_idx == INVALID_INDEX)
		{
			continue;
		}

		if (nodes_[node_idx].term.type != term_t::Function)
		{
			nodes_[node_idx].term.value *= -1;
			continue;
		}

		// inverse operation_t
		nodes_[node_idx].term.op = opposite(nodes_[node_idx].term.op);

		// continue negation if required
		if (nodes_[node_idx].term.op == operation_t::Implication ||
			nodes_[node_idx].term.op == operation_t::Conjunction)
		{
			q.push(subtree(node_idx).right());
		}
		else if (nodes_[node_idx].term.op == operation_t::Disjunction)
		{
			q.push(subtree(node_idx).left());
			q.push(subtree(node_idx).right());
		}
	}
}


Expression &Expression::replace(std::size_t idx, const Expression &expression)
{
	if (idx == 0)
	{
		nodes_ = expression.nodes_;
		return *this;
	}

	if (!in_range(idx))
	{
		return *this;
	}

	const auto update_index = [&] (std::size_t index, std::size_t offset)
	{
		return index == INVALID_INDEX ? INVALID_INDEX : index + offset;
	};

	auto offset = size();
	nodes_[idx].term.type = expression.nodes_[0].term.type;
	nodes_[idx].term.op = expression.nodes_[0].term.op;
	nodes_[idx].term.value = expression.nodes_[0].term.value;
	nodes_[idx].rel.refs[1] = update_index(expression.nodes_[0].rel.refs[1], offset);
	nodes_[idx].rel.refs[2] = update_index(expression.nodes_[0].rel.refs[2], offset);

	for (std::size_t i = 1; i < expression.size(); ++i)
	{
		nodes_.push_back(expression.nodes_[i]);
		for (auto &ref : nodes_.back().rel.refs)
		{
			ref += offset;
		}
	}

	return *this;
}


Expression Expression::construct(
	const Expression &lhs,
	operation_t op,
	const Expression &rhs
)
{
	Expression expression;
	std::size_t offset = 1;

	expression.nodes_.emplace_back(
		Term(term_t::Function, op),
		Relation(0, 1, offset + lhs.size())
	);

	for (const auto &node : lhs.nodes_)
	{
		expression.nodes_.push_back(node);

		for (auto &ref : expression.nodes_.back().rel.refs)
		{
			ref += offset;
		}
	}

	offset += lhs.size();
	for (const auto &node : rhs.nodes_)
	{
		expression.nodes_.push_back(node);

		for (auto &ref : expression.nodes_.back().rel.refs)
		{
			ref += offset;
		}
	}

	return expression;
}


/*std::ostream &operator<<(std::ostream &out, const Expression &expression)
{
	return out << expression.to_string();
}*/
