#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include "../math/ast.hpp"

using expression_t = std::shared_ptr<ASTNode>;

class Solver
{
	std::vector<expression_t> axioms_;

	std::vector<expression_t> hypotheses_;
	expression_t target_;

	/**
	 * @brief Check if from our hypotheses target is proved
	 */
	bool is_target_proved() const;
public:
	Solver(std::vector<std::shared_ptr<ASTNode>> axioms,
		std::shared_ptr<ASTNode> target);

	void solve();
	std::vector<std::string> thought_chain() const;
};

#endif // SOLVER_HPP
