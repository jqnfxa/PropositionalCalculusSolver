#include <set>
#include <ostream>
#include "helper.hpp"


expression_t negation(const expression_t &expression)
{
	if (expression == nullptr)
	{
		return nullptr;
	}

	// simple case: just one letter
	// notice that parent is lost
	if (expression->is_leaf())
	{
		return std::make_shared<ASTNode>(-expression->var);
	}

	auto new_expression = expression->deepcopy();

	// `xor` changes to `equivalent` and wise versa
	if (expression->op == Operation::Xor)
	{
		new_expression->op = Operation::Equivalent;
		return new_expression;
	}
	if (expression->op == Operation::Equivalent)
	{
		new_expression->op = Operation::Xor;
		return new_expression;
	}

	if (expression->op == Operation::Disjunction)
	{
		// negation of disjunction is conjunction
		new_expression->op = Operation::Conjunction;
		new_expression->left = negation(new_expression->left);
		new_expression->right = negation(new_expression->right);
		new_expression->left->parent = new_expression.get();
		new_expression->right->parent = new_expression.get();

		return new_expression;
	}

	if (expression->op == Operation::Conjunction)
	{
		// negation of disjunction is conjunction
		new_expression->op = Operation::Disjunction;
		new_expression->left = negation(new_expression->left);
		new_expression->right = negation(new_expression->right);
		new_expression->left->parent = new_expression.get();
		new_expression->right->parent = new_expression.get();

		return new_expression;
	}

	// Implication
	// negation of implication is conjunction
	new_expression->op = Operation::Conjunction;
	new_expression->left = new_expression->left;
	new_expression->right = negation(new_expression->right);
	new_expression->left->parent = new_expression.get();
	new_expression->right->parent = new_expression.get();

	return new_expression;
}


bool is_same_expression(const expression_t &A, const expression_t &B)
{
	const bool ea = A == nullptr;
	const bool eb = B == nullptr;

	// both expressions must be NULL at same time or valid at same time
	if (ea ^ eb)
	{
		return false;
	}

	// two empty expressions are equivalent
	if (ea && eb)
	{
		return true;
	}

	// both expressions must be leaf or not leaf at same time
	if (A->is_leaf() ^ B->is_leaf())
	{
		return false;
	}

	// compare variables
	if (A->is_leaf() && B->is_leaf())
	{
		return A->var == B->var;
	}

	// compare operation
	if (A->op != B->op)
	{
		return false;
	}

	if (A->op == Operation::Implication)
	{
		// check both subtrees
		return is_same_expression(A->left, B->left) &&
			is_same_expression(A->right, B->right);
	}

	// all other operations are commutative
	return (is_same_expression(A->left, B->left) && is_same_expression(A->right, B->right)) ||
		(is_same_expression(A->right, B->left) && is_same_expression(A->left, B->right));
}


bool is_follows(const expression_t &A, const expression_t &B)
{
	// rule 1
	if (is_same_expression(A, B))
	{
		return true;
	}

	// rule 2 and rule 3
	if (B->op == Operation::Disjunction)
	{
                return is_same_expression(A, B->left) ||
			is_same_expression(A, B->right);
	}

	// rule 4 and rule 5
	if (B->op == Operation::Implication)
	{
		return is_same_expression(A, B->right) ||
			is_same_expression(A, negation(B->left));
	}

	// give up
	return false;
}


std::ostream &deduction_theorem_decomposition(
	std::ostream &out,
	std::vector<expression_t> &left_side,
	expression_t &target
)
{
	while (target != nullptr &&
		target->op == Operation::Implication)
	{
		out << "(deduction theorem): " << target << '\n';

		// remove relations
		target->left->parent = nullptr;
		target->right->parent = nullptr;

		// add left subexpression
		left_side.push_back(target->left);

		// traverse to right subexpression
		target = target->right;
	}

	return out;
}


std::ostream &conjunction_splitting_rule(
	std::ostream &out,
	std::vector<expression_t> &hypotheses
)
{
	std::vector<expression_t> new_hypotheses;

	for (const auto &hypothesis : hypotheses)
	{
		// decompose A*B into A,B
		if (hypothesis->op == Operation::Conjunction)
		{
			out << "(conjunction splitting rule): " << hypothesis << '\n';

			new_hypotheses.push_back(hypothesis->left->deepcopy());
			new_hypotheses.push_back(hypothesis->right->deepcopy());
		}
	}

	for (const auto &new_hypothesis : new_hypotheses)
	{
		hypotheses.push_back(new_hypothesis);
	}

	return out;
}


void standartize(expression_t &expression)
{
	if (expression->is_leaf())
	{
		return;
	}

	if (expression->op == Operation::Disjunction)
	{
		expression->op = Operation::Implication;
		expression->left = negation(expression->left);
		expression->left->parent = expression.get();
	}
}


void normalize(expression_t &expression)
{
	if (expression == nullptr)
	{
		return;
	}

	auto values = expression->values();
	std::unordered_map<std::int32_t, std::int32_t> remapping;
	std::int32_t new_variable = 1;

	for (auto &value : values)
	{
		value = std::abs(value);
	}

	for (const auto &value : values)
	{
		if (remapping.contains(value))
		{
			continue;
		}

		remapping[value] = new_variable;
		++new_variable;
	}

	// update expression tree
	std::function<void(expression_t &)> traverse = [&] (expression_t &e)
	{
		if (e == nullptr)
		{
			return;
		}

		if (e->is_leaf())
		{
			auto sign = e->var < 0 ? -1 : 1;
			e->var = sign * remapping.at(std::abs(e->var));
		}

		traverse(e->left);
		traverse(e->right);
	};

	traverse(expression);
}


std::unordered_map<std::int32_t, expression_t> unification(const expression_t &A, const expression_t &B)
{
	// idea is to traverse both trees and store which nodes from B should be replaced
	std::unordered_map<std::int32_t, expression_t> replacements;
	bool unifiable = true;

	std::function<void(const expression_t &, const expression_t &)> traverse =
	[&] (const expression_t &a, const expression_t &b)
	{
		if (!unifiable)
		{
			return;
		}

		const bool is_valid_a = a != nullptr;
		const bool is_valid_b = b != nullptr;

		if (!is_valid_a && !is_valid_b)
		{
			return;
		}

		if (is_valid_a ^ is_valid_b)
		{
			unifiable = false;
			return;
		}

		if (a->is_leaf() ^ b->is_leaf())
		{
			// we can't unify ASTNode to single variable
			if (a->is_leaf())
			{
				unifiable = false;
				return;
			}
			if (b->is_leaf())
			{
				// rule is broke!
				/*if (a->contains(b->var))
				{
					unifiable = false;
					return;
				}*/

				replacements[b->var] = a->deepcopy();
			}
		}
		if (a->is_leaf() && b->is_leaf())
		{
			if (a->var != b->var)
			{
				replacements[b->var] = a->deepcopy();
			}
		}

		// operation must be the same
		if (a->op != b->op)
		{
			unifiable = false;
			return;
		}

		traverse(a->left, b->left);
		traverse(a->right, b->right);
	};

	traverse(A, B);

	if (!unifiable)
	{
		replacements.clear();
	}

	return replacements;
}


void unify(expression_t &A, const std::unordered_map<std::int32_t, expression_t> &rules)
{
	std::function<void(expression_t &)> apply_modifications =
	[&] (expression_t &expression)
	{
		if (expression == nullptr)
		{
			return;
		}

		if (expression->is_leaf() && rules.contains(expression->var))
		{
			auto parent = expression->parent;
			expression = rules.at(expression->var)->deepcopy();
			expression->parent = parent;
			return;
		}

		apply_modifications(expression->left);
		apply_modifications(expression->right);
	};

	apply_modifications(A);
}


/**
 * @brief Modes Ponens rule
 *
 * @return A, A > B ⊢ B
 */
expression_t mp(const expression_t &A, const expression_t &B, bool mut_b)
{
	if (is_same_expression(A, B->left))
	{
		return B->right->deepcopy();
	}

	if (!mut_b)
	{
		return nullptr;
	}

	auto copy = B->deepcopy();
	const auto rules = unification(A, B->left);
	if (rules.empty())
	{
		return nullptr;
	}

	unify(copy, rules);
	return copy->right;
}
