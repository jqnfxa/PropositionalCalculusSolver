#include <iostream>
#include <queue>
#include <unordered_map>
#include <string>
#include "rules.hpp"
#include "helper.hpp"
#include "ast.hpp"


Expression modus_ponens(const Expression &lhs, const Expression &rhs)
{
	if (lhs.empty() || rhs.empty())
	{
		return {};
	}

	if (rhs[0].op != operation_t::Implication)
	{
		return {};
	}

	// idea is to unify lhs to rhs->left
	std::unordered_map<value_t, Expression> left_substitution;
	std::unordered_map<value_t, Expression> right_substitution;

	// create copies
	auto rhs_copy = rhs;
	auto left = lhs;

	// change variables to avoid intersections
	rhs_copy.change_variables(left.max_value() + 1);

	// create left subtree of right expression
	auto right = rhs_copy.subtree_copy(rhs.subtree(0).left());


	// algorithm
	// step 1: find the set of mismatches
	// step 2: apply appropriate changes (if possible)
	// goto 1
	// mismatches can only be in `current`, `left` or `right` subtrees
	// therefore we will use preorder tree traverse

	std::queue<std::size_t> left_expression;
	std::queue<std::size_t> right_expression;

	left_expression.push(left.subtree(0).self());
	right_expression.push(right.subtree(0).self());

	while (!left_expression.empty() && !right_expression.empty())
	{
		auto left_idx = left_expression.front();
		left_expression.pop();
		auto right_idx = right_expression.front();
		right_expression.pop();

		auto &left_term = left[left_idx];
		auto &right_term = right[right_idx];

		// case 0: both terms are functions
		if (left_term.type == term_t::Function && right_term.type == term_t::Function)
		{
			// it's impossible to unify different operations
			if (left_term.op != right_term.op)
			{
				return {};
			}

			// add nodes to process
			left_expression.push(left.subtree(left_idx).left());
			left_expression.push(left.subtree(left_idx).right());
			right_expression.push(right.subtree(right_idx).left());
			right_expression.push(right.subtree(right_idx).right());
			continue;
		}

		// case 1: both terms are constants
		if (left_term.type == term_t::Constant && right_term.type == term_t::Constant)
		{
			// can't unify two constants
			if (left_term.value != right_term.value)
			{
				return {};
			}

			// well, everything seems to be fine
			continue;
		}

		// case 2: left term is constant and right is variable
		if (left_term.type == term_t::Constant && right_term.type == term_t::Variable)
		{
			// variable of right term already have substitution?
			if (right_substitution.contains(right_term.value))
			{
				return {};
			}

			right_substitution[right_term.value] = Expression{left_term};
			continue;
		}

		// case 3: left term is variable and right is constant
		if (left_term.type == term_t::Variable && right_term.type == term_t::Constant)
		{
			// variable of right term already have substitution?
			if (left_substitution.contains(left_term.value))
			{
				return {};
			}

			left_substitution[left_term.value] = Expression{right_term};
			continue;
		}

		// case 4: both terms are variables
		if (left_term.type == term_t::Variable && right_term.type == term_t::Variable)
		{
			// are variables equal?
			if (left_term.value == right_term.value)
			{
				continue;
			}

			// can we change variables?
			if (left_substitution.contains(left_term.value) ||
				right_substitution.contains(right_term.value))
			{
				// only one variable can be changed
				if (!left_substitution.contains(left_term.value))
				{
					left_substitution[left_term.value] = Expression{right_term};
					continue;
				}
				if (!right_substitution.contains(right_term.value))
				{
					right_substitution[right_term.value] = Expression{left_term};
					continue;
				}

				// only one variable can't be changed, are substitution equal?
				if (!left_substitution.at(left_term.value).equal_to(
					right_substitution.at(right_term.value)))
				{
					// give up
					return {};
				}

				continue;
			}

			// since we can change variables to any variables let's pick left
			left_substitution[left_term.value] = Expression{left_term};
			right_substitution[right_term.value] = Expression{left_term};
			continue;
		}

		// case 5: left term is function
		if (left_term.type == term_t::Function)
		{
			if (right_term.type != term_t::Variable)
			{
				return {};
			}

			// can we change variable?
			if (right_substitution.contains(right_term.value))
			{
				return {};
			}

			// function can't depend on variable
			if (left.contains(left_idx, right_term))
			{
				return {};
			}

			// apply changes
			right_substitution[right_term.value] = left.subtree_copy(left_idx);
			continue;
		}

		// case 6: right term is function
		if (right_term.type == term_t::Function)
		{
			if (left_term.type != term_t::Variable)
			{
				return {};
			}

			// can we change variable?
			if (left_substitution.contains(left_term.value))
			{
				return {};
			}

			// function can't depend on variable
			if (right.contains(right_idx, left_term))
			{
				return {};
			}

			// apply changes
			left_substitution[left_term.value] = right.subtree_copy(right_idx);
			continue;
		}

		// something went wrong during unification?
		// this point is basically unreachable
		return {};
	}

	// unification succeeded, let's apply changes
	for (const auto &[variable, substitution] : right_substitution)
	{
		rhs_copy.replace(variable, substitution);
	}

	// prepare answer
	auto result = rhs_copy.subtree_copy(rhs_copy.subtree(0).right());
	result.normalize();

        return result;
}
