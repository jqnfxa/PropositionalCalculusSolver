#include "rules.hpp"
#include "helpers.hpp"


Expression modus_ponens(const Expression &a, const Expression &b)
{
	const auto rules = unification(a, b.subtree(b.left(0)));

	if (rules.empty())
	{
		return Expression{};
	}

	unify(b, rules);
	return b.subtree(b.right(0));
}
