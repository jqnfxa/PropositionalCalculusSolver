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

	std::function<void(std::ostream &, Relation, const Expression &)> f =
	[&] (std::ostream &out, Relation root, const Expression &expression)
	{
		if (root.self() == INVALID_INDEX)
		{
			return;
		}

		const bool brackets = root.parent() != INVALID_INDEX &&
			expression[root.self()].type == term_t::Function;
		if (brackets)
		{
			out << "(";
		}

		f(out, subtree(root.left()), expression);
		out << expression[root.self()].to_string();
		f(out, subtree(root.right()), expression);

		if (brackets)
		{
			out << ")";
		}

		return;
	};

	f(ss, subtree(0), *this);
	return ss.str();
}


Relation Expression::subtree(std::size_t idx) const noexcept
{
	return in_range(idx) ? nodes_[idx].rel : Relation{};
}


bool Expression::has_left(std::size_t idx) const noexcept
{
	return !in_range(idx) ? false : in_range(nodes_[idx].rel.left());
}


bool Expression::has_right(std::size_t idx) const noexcept
{
	return !in_range(idx) ? false : in_range(nodes_[idx].rel.right());
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


Expression &Expression::replace(value_t value, const Expression &expression)
{
	std::vector<std::size_t> indices;
	Expression new_expr = expression;

	value_t appropriate_value = 0;

	// find all places
	for (const auto &node : nodes_)
	{
		if (node.term.type == term_t::Variable && node.term.value == value)
		{
			indices.push_back(node.rel.self());
			appropriate_value = std::max(appropriate_value, node.term.value);
		}
	}

	// nothing to replace
	if (indices.empty())
	{
		return *this;
	}

	const auto update_index = [&] (std::size_t index, std::size_t offset)
	{
		return index == INVALID_INDEX ? INVALID_INDEX : index + offset;
	};

	// don't forget that other variables are different and should not intersect with ours
	auto offset = size();
	appropriate_value = appropriate_value + 1;
	for (const auto &entry : indices)
	{
		// replace entry with new root node
		nodes_[entry].term.type = new_expr.nodes_[0].term.type;
		nodes_[entry].term.op = new_expr.nodes_[0].term.op;
		nodes_[entry].term.value = new_expr.nodes_[0].term.value;
		nodes_[entry].rel.refs[1] =
			update_index(new_expr.subtree(0).left(), offset - 1);
		nodes_[entry].rel.refs[2] =
			update_index(new_expr.subtree(0).right(), offset - 1);

		for (std::size_t i = 1; i < new_expr.nodes_.size(); ++i)
		{
			nodes_.push_back(new_expr.nodes_[i]);
			//nodes_.back().term.value += appropriate_value;

			for (auto &ref : nodes_.back().rel.refs)
			{
				ref = update_index(ref, offset - 1);
			}
		}

		// update relations for subroot nodes
		nodes_[subtree(entry).left()].rel.refs[3] = entry;
		nodes_[subtree(entry).right()].rel.refs[3] = entry;

		// mode offset
		offset = nodes_.size();
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
		Relation(0, 1, 1 + lhs.size())
	);

	const auto update_index = [] (std::size_t index, std::size_t off)
	{
		return index == INVALID_INDEX ? INVALID_INDEX : index + off;
	};

	for (const auto &node : lhs.nodes_)
	{
		expression.nodes_.push_back(node);

		for (auto &ref : expression.nodes_.back().rel.refs)
		{
			ref = update_index(ref, offset);
		}

		if (expression.nodes_.back().rel.refs[3] == INVALID_INDEX)
		{
			expression.nodes_.back().rel.refs[3] = 0;
		}
	}

	offset += lhs.size();
	for (const auto &node : rhs.nodes_)
	{
		expression.nodes_.push_back(node);

		for (auto &ref : expression.nodes_.back().rel.refs)
		{
			ref = update_index(ref, offset);
		}

		if (expression.nodes_.back().rel.refs[3] == INVALID_INDEX)
		{
			expression.nodes_.back().rel.refs[3] = 0;
		}
	}

	return expression;
}


/*std::ostream &operator<<(std::ostream &out, const Expression &expression)
{
	return out << expression.to_string();
}*/
