#ifndef HELPER_HPP
#define HELPER_HPP

#include <vector>
#include "ast.hpp"


/**
 * @brief Determine whether `A` is same as `B`
 */
bool is_same_expression(const Expression &A, const Expression &B);


/**
 * @brief Determine whether A ⊢ B
 * @note defined rules:
 * 1. A ⊢ A
 * 2. A ⊢ A | B
 * 3. A ⊢ B | A
 * 4. A ⊢ B > A
 * 5. A ⊢ !A > B
 */
bool is_follows(const Expression &A, const Expression &B);


/**
 * @brief decompose expression using known theorems
 * @note known theorems:
 * 1. ? ⊢ A > B <=> ?, A ⊢ B (deduction theorem)
 *
 * @return decomposed expression stored in array, new expression (right side) stored last
 */
std::ostream &deduction_theorem_decomposition(
	std::ostream &out,
	std::vector<Expression> &left_side,
	Expression &target
);


/**
 * @brief decompose hypotheses using known theorems
 * @note known theorems:
 * 1. ?,A*B ⊢ C <=> ?,A*B,A,B ⊢ C (conjunction splitting rule)
 */
std::ostream &conjunction_splitting_rule(
	std::ostream &out,
	std::vector<Expression> &hypotheses
);


/**
 * @brief tries to produce rules to unify rhs to lhs with restrictions
 * @note restrictions:
 * let's assume `a` is variable
 * let's assume `b` is variable
 * let's assume `c` is any expression
 *
 * then:
 * 1. `a` can be replaced with `b`
 * 2. `a` can be replaced with `c` <=> `c` does not contain `a`
 *
 * @return unification rules, if map is empty, then no unification is possible
 */
std::unordered_map<std::int32_t, Expression> unification(
	const Expression &lhs,
	const Expression &rhs
);


/**
 * @brief unify expression with specific rules
 *
 * @return unificated B or nullptr if no unification is possible
 */
void unify(
	Expression &expression,
	const std::unordered_map<std::int32_t, Expression> &rules
);

#endif // HELPER_HPP
