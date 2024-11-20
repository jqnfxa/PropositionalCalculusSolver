#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <fstream>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "../math/ast.hpp"


class TopoSort
{
	std::unordered_map<std::string, std::vector<std::string>> graph_;
	std::string target_;
	std::vector<std::string> sorted_expressions_;

	void perform_sort(const std::string &target);
	void dfs(const std::string& expr, std::unordered_set<std::string>& visited);

public:
	TopoSort(
		std::unordered_map<std::string, std::vector<std::string>> &&graph,
		std::string target
	);

	std::string to_string() const;
};


class Solver
{
	std::unordered_set<std::string> known_axioms_;

	// map hash value of expression to hash_values of dependent expressions
	std::vector<Expression> axioms_;
	std::queue<Expression> produced_;

	std::vector<Expression> targets_;
	std::uint64_t time_limit_;

	// stream to store thought chain
	std::stringstream ss;
	std::ofstream dump_;

	// Γ ⊢ A → B <=> Γ U {A} ⊢ B
	bool deduction_theorem_decomposition(Expression expression);

	// tries to add expression to axioms
	bool add_expression(
		Expression expression,
		std::size_t max_len
	);

	// tries to add expression to produced_
	bool add_produced(Expression expression, std::size_t max_len);

	// iteration function
	void produce(std::size_t max_len);

	// is any target if follows from expression?
	bool is_target_proved_by(const Expression &expression) const;

public:
	Solver(std::vector<Expression> axioms,
		Expression target,
		std::uint64_t time_limit_ms = 1000
	);

	void solve();
	std::string thought_chain() const;
};

#endif // SOLVER_HPP
