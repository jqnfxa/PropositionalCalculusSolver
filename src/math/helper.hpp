#ifndef HELPER_HPP
#define HELPER_HPP

#include <vector>
#include "ast.hpp"


/**
 * @brief Determine whether `A` is same as `B`
 */
bool is_same_expression(const expression_t &A, const expression_t &B);


/**
 * @brief Determine whether A ⊢ B
 * @note defined rules:
 * 1. A ⊢ A
 * 2. A ⊢ A | B
 * 3. A ⊢ B | A
 * 4. A ⊢ B > A
 * 5. A ⊢ !A > B
 */
bool is_follows(const expression_t &A, const expression_t &B);


/**
 * @brief decompose expression using known theorems
 * @note known theorems:
 * 1. ? ⊢ A > B <=> ?, A ⊢ B (deduction theorem)
 *
 * @return decomposed expression stored in array, new expression (right side) stored last
 */
std::ostream &deduction_theorem_decomposition(
	std::ostream &out,
	std::vector<expression_t> &left_side,
	expression_t &target
);


/**
 * @brief decompose hypotheses using known theorems
 * @note known theorems:
 * 1. ?,A*B ⊢ C <=> ?,A*B,A,B ⊢ C (conjunction splitting rule)
 */
std::ostream &conjunction_splitting_rule(
	std::ostream &out,
	std::vector<expression_t> &hypotheses
);

/**
 * @brief standartization of expression
 * @note rules:
 * 1. A | B <=> !A > B
 */
void standartize(expression_t &expression);


/**
 * @brief normalization of expression
 * @note converting expression to use minimum positive nums (if possible)
 */
void normalize(expression_t &expression);

#endif // HELPER_HPP
