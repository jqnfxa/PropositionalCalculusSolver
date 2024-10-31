#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include "../math/ast.hpp"

using expression_t = std::shared_ptr<ASTNode>;


struct Details
{
	std::string rule;
	std::vector<std::string> assumptions;
};


class Solver
{
	std::vector<expression_t> axioms_;

	std::vector<expression_t> hypotheses_;
	expression_t target_;

	std::stringstream ss;

	/**
	 * @brief store thought chain
	 */
	std::unordered_map<std::string, Details> thought_chain_log;


	/**
	 * @brief Check if from our hypotheses target is proved
	 */
	bool is_target_proved(std::ostream &out) const;


	/**
	 * @brief Produces new expressions from axioms
	 */
	void produce_expressions(std::ostream &out);


	/**
	 * @brief Produces new expressions from axioms
	 */
	std::ostream &print_hypotheses(std::ostream &out);

	/**
	 * @brief unique hypotheses will be stored
	 */
	void add_hypothesis(expression_t expression);

	void produce_report();
public:
	Solver(std::vector<std::shared_ptr<ASTNode>> axioms,
		std::shared_ptr<ASTNode> target);

	void solve();
	std::string thought_chain() const;
};

#endif // SOLVER_HPP
