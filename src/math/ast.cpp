#include <algorithm>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <stack>
#include <queue>
#include <functional>
#include "ast.hpp"


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


std::string Term::to_string() const
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
			char suitable_constant = std::abs(var) - 1 + 'a';
			representation << suitable_constant;
		}
		else
		{
			// TODO: think what if var value > 9 ?
			representation << std::abs(var);
		}
	}

	return representation.str();
}


Expression::Expression() = default;


Expression::Expression(std::string_view expression)
{
	// call of parser
}


Expression::Expression(Term term)
{
	nodes_.emplace_back(
		term,
		Relation(0)
	);
}


Expression(const Expression &other) : nodes_(other.nodes_)
{}


Expression(Expression &&other) : nodes_(std::move(other.nodes_))
{}


Expression &Expression::operator=(const Expression &other)
{
	if (*this == other)
	{
		return *this;
	}

	nodes_ = other.nodes_;
	return *this;
}


Expression &Expression::operator=(Expression &&other)
{
	if (*this == other)
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


std::ostream &operator<<(std::ostream &out) const
{
	if (empty())
	{
		return out << "empty";
	}

	std::stack<Relation> stack;
        Relation current = subtree(0);

	while (current.self() != INVALID_INDEX || !stack.empty())
	{
		// go as far left as possible
		while (current)
		{
			stack.push(current);
			current = subtree(current.left());
		}

		// backtrack and print
		current = stack.top();
		stack.pop();

		// print brackets
		if (has_left(current.self()) && has_right(current.self()))
		{
			out << "(";
		}

		out << nodes_[current.self()].term.to_string();

		// print brackets
		if (has_left(current.self()) && has_right(current.self()))
		{
			out << ")";
		}

		// move to right subtree
		current = subtree(current.right());
        }

	return out;
}


std::string Expression::to_string() const noexcept
{
	std::ostringstream representation;
	representation << *this;
        return representation.str();
}


const Relation &Expression::subtree(std::size_t idx) const noexcept
{
	return in_range(idx) ? nodes_[idx].rel : {};
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

		if (nodes_[node_idx].type != term_t::Function)
		{
			nodes_[node_idx].var *= -1;
			continue;
		}

		// inverse operation_t
		nodes_[node_idx].op = opposite(nodes_[node_idx].op);

		// continue negation if required
		if (nodes_[node_idx].op == operation_t::Implication ||
			nodes_[node_idx].op == operation_t::Conjunction)
		{
			q.push(nodes_.subtree(node_idx).right());
		}
		else if (nodes_[node_idx].op == operation_t::Disjunction)
		{
			q.push(nodes_.subtree(node_idx).left());
			q.push(nodes_.subtree(node_idx).right());
		}
	}
}


Expression &Expression::replace(std::size_t idx, const Expression &expression)
{
	if (idx == 0)
	{
		nodes_ = expression.nodes();
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
	nodes_[idx].term.type = expression.nodes_[0].type;
	nodes_[idx].term.op = expression.nodes_[0].op;
	nodes_[idx].term.value = expression.nodes_[0].value;
	nodes_[idx].rel.left = update_index(expression.nodes_[0].rel.left, offset);
	nodes_[idx].rel.right = update_index(expression.nodes_[0].rel.right, offset);

	for (std::size_t i = 1; i < expression.size(); ++i)
	{
		nodes_.push_back(node);
		for (auto &ref : nodes_.back().refs)
		{
			ref += offset;
		}
	}

	return *this;
}


static Expression Expression::negation(const Expression &expression)
{
	Expression negated_expression(expression);
	negated_expression.negation(0);
	return negated_expression;
}


static Expression Expression::construct(
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

	for (const auto &node : lhs)
	{
		expression.nodes_.push_back(node);

		for (auto &ref : expression.nodes_.back().rel.refs)
		{
			ref += offset;
		}
	}

	offset += lhs.size();
	for (const auto &node : rhs)
	{
		expression.nodes_.push_back(node);

		for (auto &ref : expression.nodes_.back().rel.refs)
		{
			ref += offset;
		}
	}
}
