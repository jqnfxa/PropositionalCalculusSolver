#include <unordered_map>
#include <queue>
#include <iostream>
#include "helper.hpp"


bool add_constraint(
	Term term,
	Expression substitution,
	std::unordered_map<value_t, Expression> &sub
)
{
	if (substitution.contains(0, term))
	{
		return false;
	}

	sub[term.value] = substitution;
	if (term.op == operation_t::Negation)
	{
		sub[term.value].negation();
	}

	return true;
}


bool unification(
	Expression left,
	Expression right,
	std::unordered_map<value_t, Expression> &substitution
)
{
	std::unordered_map<value_t, Expression> sub;

	// change variables to avoid intersections
	right.change_variables(left.max_value() + 1);
	value_t v = right.max_value() + 1;

	// algorithm
	// step 1: find the set of mismatches
	// step 2: apply appropriate changes (if possible)
	// goto 1
	// mismatches can only be in `current`, `left` or `right` subtrees
	// therefore we will use preorder tree traverse

	std::queue<std::pair<std::size_t, std::size_t>> expression;

	expression.emplace(left.subtree(0).self(), right.subtree(0).self());

	while (!expression.empty())
	{
		auto [left_idx, right_idx] = expression.front();
		expression.pop();

		auto &left_term = left[left_idx];
		auto &right_term = right[right_idx];

		// case 0: both terms are functions
		if (left_term.type == term_t::Function &&
			right_term.type == term_t::Function)
		{
			// it's impossible to unify different operations
			if (left_term.op != right_term.op)
			{
				return false;
			}

			// add nodes to process
			expression.emplace(
				left.subtree(left_idx).left(),
				right.subtree(right_idx).left()
			);
			expression.emplace(
				left.subtree(left_idx).right(),
				right.subtree(right_idx).right()
			);
			continue;
		}

		// case 1: both terms are constants
		if (left_term.type == term_t::Constant &&
			right_term.type == term_t::Constant)
		{
			// can't unify two constants
			if (left_term.value != right_term.value)
			{
				return false;
			}

			// well, everything seems to be fine
			continue;
		}

		// case 2: left term is constant and right is variable
		if (left_term.type == term_t::Constant &&
			right_term.type == term_t::Variable)
		{
			if (!add_constraint(
				right_term,
				Expression{left_term},
				sub
			))
			{
				return false;
			}

			continue;
		}

		// case 3: left term is variable and right is constant
		if (left_term.type == term_t::Variable &&
			right_term.type == term_t::Constant)
		{
			if (!add_constraint(
				left_term,
				Expression{right_term},
				sub
			))
			{
				return false;
			}

			continue;
		}

		// case 4: both terms are variables
		if (left_term.type == term_t::Variable &&
			right_term.type == term_t::Variable)
		{
			// are variables equal?
			if (left_term == right_term)
			{
				continue;
			}

			// since we can change variables to any variables let's do it
			add_constraint(
				left_term,
				right_term,
				sub
			);

			add_constraint(
				right_term,
				right_term,
				sub
			);
			continue;
		}

		// case 5: left term is function
		if (left_term.type == term_t::Function)
		{
			if (right_term.type != term_t::Variable)
			{
				return false;
			}

			// apply changes
			add_constraint(
				right_term,
				left.subtree_copy(left_idx),
				sub
			);
			continue;
		}

		// case 6: right term is function
		if (right_term.type == term_t::Function)
		{
			if (left_term.type != term_t::Variable)
			{
				return false;
			}

			// apply changes
			add_constraint(
				left_term,
				right.subtree_copy(right_idx),
				sub
			);
			continue;
		}

		// something went wrong during unification?
		// this point is basically unreachable
		return false;
	}

	substitution = std::move(sub);
	return true;
}


bool is_equal(Expression left, Expression right)
{
	// few O(1) checks
	if (left.size() != right.size())
	{
		return false;
	}

	if (left[0].op != right[0].op)
	{
		return false;
	}

	left.normalize();
	right.normalize();

	return left.to_string() == right.to_string();
}
