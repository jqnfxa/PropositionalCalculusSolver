#include "helper.hpp"


/**
 * @brief return representation of !expression
 */
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


/**
 * @brief Determine whether `A` is same as `B`
 */
bool is_same_expression(const expression_t &A, const expression_t &B)
{
	const bool ea = A == nullptr;
	const bool eb = B == nullptr;

	// both expressions must be NULL at same time or valid at same time
	if (ea == eb)
	{
		return false;
	}

	// two empty expressions are equivalent
	if (!ea && !eb)
	{
		return true;
	}

	// compare operation
	if (A->op != B->op)
	{
		return false;
	}

	// otherwise check both subtrees
	return is_same_expression(A->left, B->left) &&
		is_same_expression(A->right, B->right);
}

/**
 * @brief Determine whether A ⊢ B
 * @note defined rules:
 * 1. A ⊢ A
 * 2. A ⊢ A | B
 * 3. A ⊢ B | A
 * 4. A ⊢ B > A
 * 5. A ⊢ !A > B
 */
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


/**
 * @brief decompose expression using known theorems
 * @note known theorems:
 * 1. ? ⊢ A > B <=> ?, A ⊢ B (deduction theorem)
 *
 * @return decomposed expression stored in array, new expression (right side) stored last
 */
std::vector<expression_t> deduction_theorem_decomposition(const expression_t &expression)
{
	std::vector<expression_t> left_side;
	auto new_expression = expression->deepcopy();

	while (new_expression != nullptr &&
		new_expression->op == Operation::Implication)
	{
		// remove relations
		new_expression->left->parent = nullptr;
		new_expression->right->parent = nullptr;

		// add left subexpression
		left_side.push_back(new_expression->left);

		// traverse to right subexpression
		new_expression = new_expression->right;
	}

	left_side.push_back(new_expression);
	return left_side;
}


/**
 * @brief decompose hypotheses using known theorems
 * @note known theorems:
 * 1. ?,A*B ⊢ C <=> ?,A*B,A,B ⊢ C (conjunction splitting rule)
 */
void conjunction_splitting_rule(std::vector<expression_t> &hypotheses)
{
	std::vector<expression_t> new_hypotheses;

	for (const auto &hypothesis : hypotheses)
	{
		// decompose A*B into A,B
		if (hypothesis->op == Operation::Conjunction)
		{
			new_hypotheses.push_back(hypothesis->left->deepcopy());
			new_hypotheses.push_back(hypothesis->right->deepcopy());
		}
	}

	for (const auto &new_hypothesis : new_hypotheses)
	{
		hypotheses.push_back(new_hypothesis);
	}
}
