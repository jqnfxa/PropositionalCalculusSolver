#include <queue>
#include <ranges>
#include <chrono>
#include <iostream>
#include <set>
#include <queue>
#include "solver.hpp"
#include "../math/helper.hpp"
#include "../math/rules.hpp"


std::uint64_t ms_since_epoch()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}


Solver::Solver(std::vector<Expression> axioms,
		Expression target,
		std::uint64_t time_limit_ms
) 	: axioms_(std::move(axioms))
	, produced_()
	, target_(std::move(target))
	, time_limit_(time_limit_ms)
	, ss{}
	, dump_("log.txt")
	, dep_{}
{
	if (axioms_.size() < 3)
	{
		throw std::invalid_argument("[-] error: at least 3 axioms are required");
	}
}


bool Solver::is_target_proved() const
{
	for (const auto &axiom : axioms_)
	{
		if (is_equal(target_, axiom))
		{
			return true;
		}
	}

	return false;
}


bool Solver::add_expression(Expression expression, std::size_t max_len)
{
	// expression is too long
	if (2 * max_len < expression.size())
	{
		return false;
	}

	expression.normalize();
	if (known_axioms.contains(expression.to_string()))
	{
		return false;
	}

	dump_ << axioms_.size() << ". " << expression << std::endl;
	axioms_.emplace_back(expression);
	known_axioms.insert(axioms_.back().to_string());
	return true;
}


void Solver::produce(std::size_t max_len)
{
	if (produced_.empty())
	{
		return;
	}

	std::size_t iteration_size = produced_.size();
	std::vector<Expression> new_expressions;
	Expression expr;
	std::cerr << "iter: " << iteration_size << '\n';
	for (std::size_t i = 0; i < iteration_size; ++i)
	{
		if (ms_since_epoch() > time_limit_)
		{
			break;
		}

		auto expression = produced_.top();
		produced_.pop();

		if (max_len * 2 < expression.size())
		{
			continue;
		}

		if (is_equal(expression, target_))
		{
			add_expression(expression, max_len);
			break;
		}

		for (const auto &axiom : axioms_)
		{
			expr = std::move(modus_ponens(axiom, expression));
			if (!expr.empty() && max_len * 2 >= expression.size() &&
				!known_axioms.contains(expr.to_string()))
			{
				new_expressions.push_back(expr);
			}

			expr = std::move(modus_ponens(expression, axiom));
			if (!expr.empty() && max_len * 2 >= expression.size() &&
				!known_axioms.contains(expr.to_string()))
			{
				new_expressions.push_back(expr);
			}
		}

		add_expression(expression, max_len);
		expr = std::move(modus_ponens(axioms_.back(), axioms_.back()));

		if (!expr.empty() && max_len * 2 >= expression.size() &&
			!known_axioms.contains(expr.to_string()))
		{
			new_expressions.push_back(expr);
		}
	}

	if (ms_since_epoch() > time_limit_)
	{
		return;
	}

	std::cerr << "newly produced: " << new_expressions.size() << std::endl;
	for (const auto &expr : new_expressions)
	{
		produced_.emplace(expr);
	}
}


void Solver::solve()
{
	ss.clear();
	ss << '\n';

	for (std::size_t i = 0; i < axioms_.size(); ++i)
	{
		axioms_[i].normalize();
		produced_.push(axioms_[i]);
	}

	axioms_.clear();
	axioms_.reserve(10000);
	dep_.reserve(10000);

	// step 2: calculating the stopping criterion
	const auto time = ms_since_epoch();
	time_limit_ =
		time > std::numeric_limits<std::uint64_t>::max() - time_limit_ ?
		std::numeric_limits<std::uint64_t>::max() :
		time + time_limit_;

	std::size_t seq_size = axioms_.size();
	std::size_t len = 15;

	while (ms_since_epoch() < time_limit_)
	{
		produce(len);

		if (is_equal(axioms_.back(), target_))
		{
			break;
		}

		// nothing was found, so we need to expand max_len of expression
		if (seq_size == axioms_.size())
		{
			break;
		}

		seq_size = axioms_.size();
	}

	if (!is_target_proved())
	{
		ss << "No proof was found in the time allotted\n";
		return;
	}

	// find index of target_ in axioms_
	const auto target_index = std::ranges::find_if(
		axioms_,
                [&](const Expression &axiom) { return is_equal(target_, axiom); }
	) - axioms_.begin();

	// build proof chain
	std::queue<std::size_t> chain;
	std::set<std::size_t> sequence;
	chain.push(target_index);

	while (!chain.empty())
	{
		auto index = chain.front();
		chain.pop();

		sequence.insert(index);

		if (dep_[index].empty())
		{
			continue;
		}

		sequence.insert(dep_[index].begin(), dep_[index].end());
		chain.push(dep_[index][0]);
		chain.push(dep_[index][1]);
	}

	std::size_t order_index = 1;
	for (const auto &step : sequence)
	{
		ss << order_index << ". ";

		if (dep_[step].empty())
		{
			ss << "axiom_" << order_index << ": " << axioms_[step];
		}
		else
		{
			const auto i1 = std::distance(
				sequence.begin(),
				sequence.find(dep_[step][0])
			);
			const auto i2 = std::distance(
				sequence.begin(),
				sequence.find(dep_[step][1])
			);

			ss << "mp(" << i1 + 1 << "," << i2 + 1 << "): " << axioms_[step];
		}

		ss << '\n';
		++order_index;
	}
}


std::string Solver::thought_chain() const
{
	return ss.str();
}
