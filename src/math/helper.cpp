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

		auto left_term = left[left_idx];
		auto right_term = right[right_idx];

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

			// different operations
			if (left_term.op != right_term.op)
			{
				return false;
			}

			// well, everything seems to be fine
			continue;
		}

		Expression lhs = left.subtree_copy(left_idx);
		Expression rhs = right.subtree_copy(right_idx);

		// adjust terms since it may have subs
		while (lhs[0].type == term_t::Variable &&
			sub.contains(lhs[0].value))
		{
			bool should_negate = lhs[0].op == operation_t::Negation;

			lhs = sub.at(lhs[0].value);
			if (should_negate)
			{
				lhs.negation();
			}
		}
		while (rhs[0].type == term_t::Variable &&
			sub.contains(rhs[0].value))
		{
			bool should_negate = rhs[0].op == operation_t::Negation;

			rhs = sub.at(rhs[0].value);
			if (should_negate)
			{
				rhs.negation();
			}
		}

		// case 2: left term is constant and right is variable
		if (lhs[0].type == term_t::Constant &&
			rhs[0].type == term_t::Variable)
		{
			if (!add_constraint(rhs[0], lhs, sub))
			{
				return false;
			}

			continue;
		}

		// case 3: left term is variable and right is constant
		if (lhs[0].type == term_t::Variable &&
			rhs[0].type == term_t::Constant)
		{
			if (!add_constraint(lhs[0], rhs, sub))
			{
				return false;
			}

			continue;
		}

		// case 4: both terms are variables
		if (lhs[0].type == term_t::Variable &&
			rhs[0].type == term_t::Variable)
		{
			// are variables equal?
			if (lhs[0] == rhs[0])
			{
				continue;
			}

			// add new variable
			Expression expr(Term(
				term_t::Variable,
				lhs[0].op == operation_t::Negation ||
				rhs[0].op == operation_t::Negation ?
				operation_t::Negation :
				operation_t::Nop,
				v++
			));

			if (lhs[0].op == operation_t::Negation)
			{
				lhs.negation();
				expr.negation();
			}

			add_constraint(lhs[0], expr, sub);

			if (lhs[0].op == operation_t::Negation)
			{
				expr.negation();
			}

			if (rhs[0].op == operation_t::Negation)
			{
				rhs.negation();
				expr.negation();
			}

			add_constraint(rhs[0], expr, sub);

			if (rhs[0].op == operation_t::Negation)
			{
				expr.negation();
			}

			continue;
		}

		// case 5: left term is function
		if (lhs[0].type == term_t::Function)
		{
			if (rhs[0].type != term_t::Variable)
			{
				return false;
			}

			if (rhs[0].op == operation_t::Negation)
			{
				lhs.negation();
			}

			if (!add_constraint(rhs[0], lhs, sub))
			{
				return false;
			}

			continue;
		}

		// case 6: right term is function
		if (rhs[0].type == term_t::Function)
		{
			if (lhs[0].type != term_t::Variable)
			{
				return false;
			}

			if (lhs[0].op == operation_t::Negation)
			{
				rhs.negation();
			}

			if (!add_constraint(lhs[0], rhs, sub))
			{
				return false;
			}

			continue;
		}

		// something went wrong during unification?
		// this point is basically unreachable
		return false;
	}

	// normalize substitutions
	for (auto &[v, expr] : sub)
	{
		if (expr[0].type != term_t::Function)
		{
			continue;
		}

		for (const auto &var : expr.variables())
		{
			if (!sub.contains(var))
			{
				continue;
			}

			auto replacement = sub.at(var);
			while (replacement[0].type == term_t::Variable &&
					sub.contains(replacement[0].value))
			{
				bool should_negate = replacement[0].op == operation_t::Negation;
				replacement = sub.at(replacement[0].value);
				if (should_negate)
				{
					replacement.negation();
				}
			}

			expr.replace(var, replacement);
		}
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
