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


static Expression negation(const Expression &expression)
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

		// inverse operation_t
		tokens_[node_idx].op = opposite(tokens_[node_idx].op);

		// continue negation if required
		if (tokens_[node_idx].op == operation_t::Implication ||
			tokens_[node_idx].op == operation_t::Conjunction)
		{
			q.push(tokens_[node_idx].right());
		}
		else if (tokens_[node_idx].op == operation_t::Disjunction)
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
	operation_t op,
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
	if (expression.n_tokens() == 0)
	{
		return out << "Nan";
	}

	std::function<std::ostream &(
		std::ostream &,
		const Expression &,
		std::size_t
	)> f = [&] (std::ostream &out, const Expression &expression, std::size_t idx) -> std::ostream &
	{
		if (idx == INVALID_INDEX || idx > expression.n_tokens())
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
