#include <set>
#include <unordered_map>
#include <ostream>
#include <functional>
#include "helper.hpp"


bool is_same_expression(const Expression &lhs, const Expression &rhs)
{
	// has different operation?
	if (lhs.root().op != rhs.root().op)
	{
		// not oposite so give up
		if (lhs.root().op != opposite(rhs.root().op))
		{
			return false;
		}

		return is_same_expression(lhs, rhs.negation());
	}

	// may be they are completely equal ?
	if (lhs == rhs)
	{
		return true;
	}

	// for non-commutative operations expressions are not equal
	if (!is_commutative(lhs.root().op))
	{
		return false;
	}

	// compare left subtree of A with right subtree of B
	return is_same_expression(lhs.subtree(lhs.left(0)), rhs.subtree(rhs.right(0)));
}


bool is_follows(const Expression &lhs, const Expression &rhs)
{
	// rule 1
	if (is_same_expression(lhs, rhs))
	{
		return true;
	}

	const auto left = rhs.subtree(rhs.left(0));
	const auto right = rhs.subtree(rhs.right(0));

	// rule 2 and rule 3
	if (rhs.root().op == Operation::Disjunction)
	{
                return is_same_expression(lhs, left) ||
			is_same_expression(lhs, right);
	}

	// rule 4 and rule 5
	if (rhs.root().op == Operation::Implication)
	{
		return is_same_expression(lhs, right) ||
			is_same_expression(lhs, left.negation());
	}

	// give up
	return false;
}


std::ostream &deduction_theorem_decomposition(
	std::ostream &out,
	std::vector<Expression> &left_side,
	Expression &target
)
{
	while (target.root().op == Operation::Implication)
	{
		out << "(deduction theorem): " << target << '\n';

		// add left subexpression
		left_side.push_back(target.subtree(target.left(0)));

		// traverse to right subexpression
		target = target.subtree(target.right(0));
	}

	return out;
}


std::ostream &conjunction_splitting_rule(
	std::ostream &out,
	std::vector<Expression> &hypotheses
)
{
	std::vector<Expression> new_hypotheses;

	for (const auto &hypothesis : hypotheses)
	{
		// decompose A*B into A,B
		if (hypothesis.root().op == Operation::Conjunction)
		{
			out << "(conjunction splitting rule): " << hypothesis << '\n';

			new_hypotheses.emplace_back(hypothesis.subtree(hypothesis.left(0)));
			new_hypotheses.emplace_back(hypothesis.subtree(hypothesis.right(0)));
		}
	}

	for (const auto &new_hypothesis : new_hypotheses)
	{
		hypotheses.emplace_back(new_hypothesis);
	}

	return out;
}


std::unordered_map<std::int32_t, Expression> unification(
	const Expression &lhs,
	const Expression &rhs
)
{
	// idea is to traverse both trees and store which nodes from `rhs` should be replaced
	std::unordered_map<std::int32_t, Expression> replacements;
	bool unifiable = true;

	std::function<void(std::size_t, std::size_t, const Expression &, const Expression &)> traverse =
	[&] (std::size_t i, std::size_t j, const Expression &a, const Expression &b)
	{
		if (!unifiable)
		{
			return;
		}

		if (i == INVALID_INDEX || j == INVALID_INDEX)
		{
			return;
		}

		if (b.root().is_leaf())
		{
			replacements[b.root().var] = a;
			return;
		}

		// operation must be the same
		if (a.root().op != b.root().op)
		{
			unifiable = false;
			return;
		}

		traverse(a.left(i), b.left(j), a, b);
		traverse(a.right(i), b.right(j), a, b);
	};

	traverse(0, 0, lhs, rhs);

	if (!unifiable)
	{
		replacements.clear();
	}

	return replacements;
}


void unify(Expression &expression, const std::unordered_map<std::int32_t, Expression> &rules)
{
	for (const auto &[var, replacement] : rules)
	{
		expression.replace(var, replacement);
	}
}
