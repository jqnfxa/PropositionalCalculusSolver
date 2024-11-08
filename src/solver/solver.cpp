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
	for (const auto &axiom : axioms_)
	{
		if (is_equal(expression, axiom))
		{
			return false;
		}
	}

	// expression is too long
	if (expression.size() / 2  > max_len)
	{
		return false;
	}

	dump_ << axioms_.size() << ". " << expression << "\n";
	axioms_.emplace_back(expression);
	return true;
}


void Solver::produce(std::size_t max_len)
{
	if (produced_.empty())
	{
		return;
	}

	auto expression = produced_.front();
	produced_.pop();

	if (expression.size() / 2 > max_len)
	{
		std::cerr << "failed\n";
		return;
	}

	for (const auto &axiom : axioms_)
	{
		auto e1 = modus_ponens(axiom, expression);
		auto e2 = modus_ponens(expression, axiom);

		if (!e1.empty())
		{
			produced_.push(e1);
		}
		if (!e2.empty())
		{
			produced_.push(e2);
		}
	}

	add_expression(expression, max_len);
}


std::vector<Expression> Solver::insert_in_axiom(
	std::size_t index,
	std::vector<Expression> &replacements
)
{
	std::vector<Expression> result;

	if (index >= axioms_.size())
	{
		return result;
	}

	// insert values into axioms
	for (const auto &r1 : replacements)
	{
		for (const auto &r2 : replacements)
		{
			auto e = axioms_[index];
			e.replace(1, r1);
			e.replace(2, r2);
			result.emplace_back(e);
		}

		auto e1 = axioms_[index];
		e1.replace(1, r1);
		auto e2 = axioms_[index];
		e2.replace(2, r1);

		result.emplace_back(e1);
		result.emplace_back(e2);
	}

	return result;
}


void Solver::produce_basic_axioms()
{
	value_t max_value = 2;
	std::vector<Expression> initial_guess;

	// ugly way to do it
	for (std::int32_t neg1 = 0; neg1 <= 1; ++neg1)
	{
	for (std::int32_t neg2 = 0; neg2 <= 1; ++neg2)
	{
	for (value_t a = 1; a <= max_value; ++a)
	{
	initial_guess.emplace_back(
		Expression(
			Term(
				term_t::Variable,
				neg1 ? operation_t::Negation : operation_t::Nop,
				a
			)
		)
	);
	for (value_t b = 1; b <= max_value; ++b)
	{
	for (std::int32_t op = 2; op <= 2; ++op)
	{
		initial_guess.emplace_back(
			Expression::construct(
				Expression(Term(
					term_t::Variable,
					neg1 ? operation_t::Negation : operation_t::Nop,
					a)
				),
				static_cast<operation_t>(op),
				Expression(Term(
					term_t::Variable,
					neg2 ? operation_t::Negation : operation_t::Nop,
					b)
				)
			)
		);
	}
	}
	}
	}
	}

	// axiom1
	for (const auto &new_axiom : insert_in_axiom(0, initial_guess))
	{
		add_expression(new_axiom, 100);
	}
}


void Solver::solve()
{
	ss.clear();
	ss << '\n';

	for (std::size_t i = 0; i < axioms_.size(); ++i)
	{
		dump_ << i << ". " << axioms_[i] << '\n';
	}

	// step 1: decompose target expression to produce more initial axioms
	//produce_basic_axioms();
	dep_.resize(axioms_.size());
	axioms_.reserve(10000);
	dep_.reserve(10000);

	Expression e;
	for (const auto &axiom1 : axioms_)
	{
		for (const auto &axiom2 : axioms_)
		{
			e = std::move(modus_ponens(axiom1, axiom2));
			if (e.empty())
			{
				continue;
			}

			produced_.push(e);
		}
	}

	// step 2: calculating the stopping criterion
	const auto time = ms_since_epoch();
	time_limit_ =
		time > std::numeric_limits<std::uint64_t>::max() - time_limit_ ?
		std::numeric_limits<std::uint64_t>::max() :
		time + time_limit_;

	std::size_t len = 10;
	std::size_t seq_size = axioms_.size();

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
			++len;
			continue;
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
