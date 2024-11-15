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
) 	: known_axioms_()
	, conclusions_()
	, axioms_(std::move(axioms))
	, produced_()
	, targets_()
	, time_limit_(time_limit_ms)
	, ss{}
	, dump_("log.txt")
	, dep_{}
{
	if (axioms_.size() < 3)
	{
		throw std::invalid_argument("[-] error: at least 3 axioms are required");
	}

	targets_.emplace_back(std::move(target));
	axioms_.reserve(10000);
	dep_.reserve(10000);
	depends_.reserve(10000);
	known_axioms_.reserve(10000);
	conclusions_.reserve(10000);

	// produce hack: implication swap rule (a->b) ~ (!b->!a)
	Expression ax1("a>(b>a)");
	Expression ax2("(a>(b>c))>((a>b)>(a>c))");
	Expression ax3("(!a>!b)>((!a>b)>a)");
	std::vector<Expression> ax = { ax1, ax2, ax3 };

	ax.push_back(modus_ponens(ax[0], ax[0]));
	ax.push_back(modus_ponens(ax[1], ax[0]));
	ax.push_back(modus_ponens(ax[3], ax[1]));
	ax.push_back(modus_ponens(ax[4], ax[1]));
	ax.push_back(modus_ponens(ax[2], ax[5]));
	ax.push_back(modus_ponens(ax[6], ax[6]));
	ax.push_back(modus_ponens(ax[7], ax[8]));
	ax.push_back(modus_ponens(ax[3], ax[9]));

	add_conclusion(ax1, {});
	add_conclusion(ax2, {});
	add_conclusion(ax3, {});
	add_conclusion(ax[3], {ax[0], ax[0]});
	add_conclusion(ax[4], {ax[1], ax[0]});
	add_conclusion(ax[5], {ax[3], ax[1]});
	add_conclusion(ax[6], {ax[4], ax[1]});
	add_conclusion(ax[7], {ax[2], ax[5]});
	add_conclusion(ax[8], {ax[6], ax[6]});
	add_conclusion(ax[9], {ax[7], ax[8]});
	add_conclusion(ax[10], {ax[3], ax[9]});

	axioms_.push_back(ax.back());
}


bool Solver::is_target_proved_by(const Expression &expression) const
{
	if (expression.empty())
	{
		return false;
	}

	for (const auto &target : targets_)
	{
		if (is_equal(target, expression))
		{
			return true;
		}
	}

	return false;
}


bool Solver::deduction_theorem_decomposition(Expression expression)
{
	if (expression.empty())
	{
		return false;
	}

	if (expression[0].op != operation_t::Implication)
	{
		return false;
	}

	auto left = expression.subtree_copy(expression.subtree(0).left());
	auto right = expression.subtree_copy(expression.subtree(0).right());

	// Γ ⊢ A → B <=> Γ U {A} ⊢ B
	axioms_.push_back(left);
	targets_.push_back(right);
	return true;
}


bool Solver::add_expression(Expression expression, std::size_t max_len)
{
	// expression is too long
	if (2 * max_len < expression.size())
	{
		return false;
	}

	expression.normalize();
	if (known_axioms_.contains(expression.to_string()))
	{
		return false;
	}

	dump_ << axioms_.size() << ". " << expression << std::endl;
	axioms_.emplace_back(std::move(expression));
	known_axioms_.insert(axioms_.back().to_string());
	return true;
}


bool Solver::add_produced(Expression expression, std::size_t max_len)
{
	if (expression.empty())
	{
		return false;
	}

	if (2 * max_len < expression.size())
	{
		return false;
	}

	if (known_axioms_.contains(expression.to_string()))
	{
		return false;
	}

	produced_.emplace(std::move(expression));
	return true;
}


void Solver::add_conclusion(
	Expression source,
	const std::initializer_list<Expression> &expressions
)
{
	if (expressions.size() == 0)
	{
		return;
	}

	if (conclusions_.contains(source.to_string()))
	{
		conclusions_[source.to_string()].clear();
	}

	for (const auto &expression : expressions)
	{
		conclusions_[source.to_string()].push_back(expression.to_string());
	}
}


void Solver::produce(std::size_t max_len)
{
	if (produced_.empty())
	{
		return;
	}

	std::size_t iteration_size = produced_.size();
	std::cerr << "iter: " << iteration_size << '\n';

	Expression expr;
	for (std::size_t i = 0; i < iteration_size; ++i)
	{
		if (ms_since_epoch() > time_limit_)
		{
			break;
		}

		auto expression = std::move(produced_.front());
		produced_.pop();

		if (max_len * 2 < expression.size())
		{
			continue;
		}

		// early checks
		if (is_target_proved_by(expression))
		{
			add_expression(expression, max_len);
			break;
		}

		for (const auto &axiom : axioms_)
		{
			expr = std::move(modus_ponens(axiom, expression));
			if (is_target_proved_by(expr))
			{
				add_conclusion(expr, {axiom, expression});
				add_expression(expr, max_len);
				return;
			}

			if (add_produced(expr, max_len))
			{
				add_conclusion(expr, {axiom, expression});
			}

			// inverse order
			expr = std::move(modus_ponens(expression, axiom));
			if (is_target_proved_by(expr))
			{
				add_conclusion(expr, {expression, axiom});
				add_expression(expr, max_len);
				return;
			}

			if (add_produced(expr, max_len))
			{
				add_conclusion(expr, {expression, axiom});
			}
		}

		add_expression(expression, max_len);
		expr = std::move(modus_ponens(axioms_.back(), axioms_.back()));
		if (add_produced(expr, max_len))
		{
			add_conclusion(expr, {axioms_.back(), axioms_.back()});
		}
	}

	if (ms_since_epoch() > time_limit_)
	{
		return;
	}

	std::cerr << "newly produced: " << produced_.size() << std::endl;
}


void Solver::solve()
{
	ss.clear();

	// simplify target if it's possible
	while (deduction_theorem_decomposition(targets_.back()))
	{
		const auto &prev = targets_[targets_.size() - 2];
		const auto &curr = targets_.back();
		const auto &axiom = axioms_.back();

		ss << "deduction theorem: " << "Γ ⊢ " << prev << " <=> "
		<< "Γ U {" << axiom << "} ⊢ " << curr << '\n';
	}

	// write all axioms to produced array
	for (std::size_t i = 0; i < axioms_.size(); ++i)
	{
		axioms_[i].normalize();
		produced_.push(axioms_[i]);
	}
	axioms_.clear();

	// calculating the stopping criterion
	const auto time = ms_since_epoch();
	time_limit_ =
		time > std::numeric_limits<std::uint64_t>::max() - time_limit_ ?
		std::numeric_limits<std::uint64_t>::max() :
		time + time_limit_;

	// we will produce at most 7 operations in one expression
	// if more operations are required, increase it
	std::size_t len = 7;

	while (ms_since_epoch() < time_limit_)
	{
		produce(len);

		if (is_target_proved_by(axioms_.back()))
		{
			break;
		}
	}

	if (std::ranges::none_of(axioms_, [&] (const auto &expression) {
		return is_target_proved_by(expression);
	}))
	{
		ss << "No proof was found in the time allotted\n";
		return;
	}

	// find which target was proved
	Expression proof;
	for (const auto &axiom : axioms_)
	{
		if (!proof.empty())
		{
			break;
		}

		for (const auto &target : targets_)
		{
			if (is_equal(target, axiom))
			{
				proof = axiom;
				break;
			}
		}
	}

	// build proof chain
	std::queue<std::string> q;
	std::set<std::string> chain;
	q.push(proof.to_string());

	std::vector<std::vector<std::string>> levels;

	while (!q.empty())
	{
		auto node = q.front();
		chain.insert(node);
		q.pop();

		if (!conclusions_.contains(node))
		{
			continue;
		}

		for (const auto &new_node : conclusions_.at(node))
		{
			q.push(new_node);
		}
	}

	// print in pretty format:
	// 1) sort expressions by their order in thought chain (begin from axioms)
	// 2) assing indices from 1 to m
	// 3) print mp(i, j) or axiom for each row
	// 4) add variable changes after last row to get target

	for (const auto &step : chain)
	{
		std::cerr << step;
		if (!conclusions_.contains(step))
		{
			std::cerr << " axiom\n";
			continue;
		}

		std::cerr << " deps: ";
		for (const auto &dep : conclusions_.at(step))
		{
			std::cerr << dep << ' ';
		}
		std::cerr << '\n';
	}

/*
	for (std::size_t i = 1; i < sequence_index; ++i)
	{
		if (!conclusions_.contains(index_to_node.at(i)))
		{
			ss << i << ". axiom: " << index_to_node.at(i) << '\n';
			continue;
		}

		auto lhs = node_to_index.at(conclusions_.at(index_to_node.at(i))[0]);
		auto rhs = node_to_index.at(conclusions_.at(index_to_node.at(i))[1]);

		ss << i << ". mp(" << lhs << ',' << rhs << "): " << index_to_node.at(i) << '\n';
	}
*/
}


std::string Solver::thought_chain() const
{
	return ss.str();
}
