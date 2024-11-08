#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <fstream>
#include <queue>
#include "../math/ast.hpp"


class Solver
{
	std::vector<Expression> axioms_;
	std::queue<Expression> produced_;
	Expression target_;
	std::uint64_t time_limit_;

	// stream to store thought chain
	std::stringstream ss;
	std::ofstream dump_;

	// dependencies
	std::vector<std::vector<std::size_t>> dep_;

	std::vector<Expression> insert_in_axiom(
		std::size_t index,
		std::vector<Expression> &replacements
	);

	bool add_expression(Expression expression, std::size_t max_len);
	void produce_basic_axioms();
	void produce(std::size_t max_len);
	bool is_target_proved() const;

public:
	Solver(std::vector<Expression> axioms,
		Expression target,
		std::uint64_t time_limit_ms = 20000
	);

	void solve();
	std::string thought_chain() const;
};

#endif // SOLVER_HPP
