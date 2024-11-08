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
	for (const auto &[variable, sub] : substitution)
	{
		result.replace(variable, sub);
	}

	// prepare answer
	result = result.subtree_copy(result.subtree(0).right());
	result.normalize();

        return result;
}
