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
std::vector<expression_t> deduction_theorem_decomposition(const expression_t &expression);


/**
 * @brief decompose hypotheses using known theorems
 * @note known theorems:
 * 1. ?,A*B ⊢ C <=> ?,A*B,A,B ⊢ C (conjunction splitting rule)
 */
void conjunction_splitting_rule(std::vector<expression_t> &hypotheses);

#endif // HELPER_HPP
