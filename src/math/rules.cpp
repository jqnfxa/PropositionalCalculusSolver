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

	// try to apply unification
	std::unordered_map<value_t, Expression> substitution;
	if (!unification(
		lhs,
		rhs.subtree_copy(rhs.subtree(0).left()),
		substitution))
	{
		return {};
	}

	// unification succeeded, let's apply changes
	auto result = rhs;
	result.change_variables(lhs.max_value() + 1);
	auto vars = result.variables();
	int depth_lim = 5;

	for (const auto &var : vars)
	{
		if (!substitution.contains(var))
		{
			continue;
		}

		depth_lim = 5;
		auto change = substitution.at(var);
		while (change[0].type == term_t::Variable &&
			substitution.contains(change[0].value) &&
			depth_lim > 0)
		{
			--depth_lim;
			change = substitution.at(change[0].value);
		}

		if (depth_lim == 0 ||
			change.contains(0, Term(term_t::Variable, operation_t::Nop, var)))
		{
			return {};
		}

		result.replace(var, change);
	}

	// prepare answer
	result = result.subtree_copy(result.subtree(0).right());
	result.normalize();

        return result;
}
