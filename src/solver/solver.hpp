#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include "../math/ast.hpp"

using expression_t = std::shared_ptr<ASTNode>;

class Solver
{
	std::vector<expression_t> axioms_;

	std::vector<expression_t> hypotheses_;
	expression_t target_;

	/**
	 * @brief stores output thought chain
	 */
	std::stringstream ss;
	/**
	 * @brief Check if from our hypotheses target is proved
	 */
	bool is_target_proved(std::ostream &out) const;
public:
	Solver(std::vector<std::shared_ptr<ASTNode>> axioms,
		std::shared_ptr<ASTNode> target);

	void solve();
	std::string thought_chain() const;
};

#endif // SOLVER_HPP
