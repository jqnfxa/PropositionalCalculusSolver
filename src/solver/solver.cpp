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
	, axioms_(std::move(axioms))
	, produced_()
	, targets_()
	, time_limit_(time_limit_ms)
	, ss{}
	, dump_("conclusions.txt")
{
	if (axioms_.size() < 3)
	{
		throw std::invalid_argument("[-] error: at least 3 axioms are required");
	}

	targets_.emplace_back(std::move(target));
	axioms_.reserve(1000);
	known_axioms_.reserve(10000);

	// produce hack: implication swap rule (a->b) ~ (!b->!a)
	add_expression(modus_ponens(axioms_[0], axioms_[0]), 100);
	add_expression(modus_ponens(axioms_[1], axioms_[0]), 100);
	add_expression(modus_ponens(axioms_[3], axioms_[1]), 100);
	add_expression(modus_ponens(axioms_[4], axioms_[1]), 100);
	add_expression(modus_ponens(axioms_[2], axioms_[5]), 100);
	add_expression(modus_ponens(axioms_[6], axioms_[6]), 100);
	add_expression(modus_ponens(axioms_[7], axioms_[8]), 100);
	add_expression(modus_ponens(axioms_[3], axioms_[9]), 100);

	dump_ << axioms_[3] << ' ' << "mp" << ' ' << axioms_[0] << ' ' << axioms_[0] << '\n';
	dump_ << axioms_[4] << ' ' << "mp" << ' ' << axioms_[1] << ' ' << axioms_[0] << '\n';
	dump_ << axioms_[5] << ' ' << "mp" << ' ' << axioms_[3] << ' ' << axioms_[1] << '\n';
	dump_ << axioms_[6] << ' ' << "mp" << ' ' << axioms_[4] << ' ' << axioms_[1] << '\n';
	dump_ << axioms_[7] << ' ' << "mp" << ' ' << axioms_[2] << ' ' << axioms_[5] << '\n';
	dump_ << axioms_[8] << ' ' << "mp" << ' ' << axioms_[6] << ' ' << axioms_[5] << '\n';
	dump_ << axioms_[9] << ' ' << "mp" << ' ' << axioms_[7] << ' ' << axioms_[8] << '\n';
	dump_ << axioms_[10] << ' ' << "mp" << ' ' << axioms_[3] << ' ' << axioms_[9] << '\n';

	axioms_.erase(axioms_.begin() + 3, axioms_.end());
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

	// Γ ⊢ A → B <=> Γ U {A} ⊢ B
	axioms_.emplace_back(expression.subtree_copy(expression.subtree(0).left()));
	targets_.emplace_back(expression.subtree_copy(expression.subtree(0).right()));
	return true;
}


bool Solver::add_expression(
	Expression expression,
	std::size_t max_len
)
{
	// expression is too long
	if (2 * max_len < expression.size())
	{
		return false;
	}

	// store axiom
	axioms_.emplace_back(expression);
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

	produced_.emplace(expression);
	return true;
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
	std::string representation;
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

		expression.normalize();
		representation = expression.to_string();

		if (known_axioms_.contains(representation))
		{
			continue;
		}

		// add expression
		add_expression(expression, max_len);

		if (is_target_proved_by(expression))
		{
			return;
		}

		// produce new expressions
		for (std::size_t j = 0; j < axioms_.size(); ++j)
		{
			expr = std::move(modus_ponens(axioms_[j], axioms_.back()));
			if (add_produced(expr, max_len))
			{
				dump_ << expr << ' ' << "mp" << ' '
				<< axioms_[j] << ' ' << axioms_.back() << '\n';
			}

			if (is_target_proved_by(expr))
			{
				add_expression(expr, max_len);
				return;
			}

			if (j + 1 == axioms_.size())
			{
				break;
			}

			// inverse order
			expr = std::move(modus_ponens(axioms_.back(), axioms_[j]));
			if (add_produced(expr, max_len))
			{
				dump_ << expr << ' ' << "mp" << ' '
				<< axioms_.back() << ' ' << axioms_[j] << '\n';
			}
			if (is_target_proved_by(expr))
			{
				add_expression(expr, max_len);
				return;
			}
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

	// we will produce at most m operations in one expression
	// if more operations are required, increase it
	std::size_t len = targets_.back().size();

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
		dump_ << axioms_[i] << ' ' << " axiom\n";
	}
	// isr rule
	produced_.push(Expression("(!a>!b)>(b>a)"));
	axioms_.clear();
	known_axioms_.clear();

	// calculating the stopping criterion
	const auto time = ms_since_epoch();
	time_limit_ =
		time > std::numeric_limits<std::uint64_t>::max() - time_limit_ ?
		std::numeric_limits<std::uint64_t>::max() :
		time + time_limit_;

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
	Expression target_proved;

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
				target_proved = target;
				break;
			}
		}
	}

	// build proof chain
	dump_.flush();

	std::ifstream conclusions("conclusions.txt");
	std::unordered_map<std::string, std::vector<std::string>> conclusions_;
	conclusions_.reserve(10000);

	std::string line;
	std::string expression;
	std::string extra_info;
	std::stringstream tss;

	while (std::getline(conclusions, line))
	{
		tss.clear();
		tss << line;
		tss >> expression;

		conclusions_[expression] = {};
		while (tss >> extra_info)
		{
			conclusions_[expression].push_back(extra_info);
		}
	}

	TopoSort ts(std::move(conclusions_), proof.to_string());
	ss << ts.to_string();

	// change variables if required
	std::unordered_map<value_t, Expression> substitution;
	unification(target_proved, proof, substitution);

	if (substitution.empty())
	{
		return;
	}

	ss << "change variables: " << proof << "\n";
	for (const auto &[v, s] : substitution)
	{
		ss << (char)(v + 'A' - 1) << " -> " << s << '\n';
	}

	ss << "proved: " << target_proved << '\n';
}


std::string Solver::thought_chain() const
{
	return ss.str();
}


TopoSort::TopoSort(
	std::unordered_map<std::string, std::vector<std::string>> &&graph,
	std::string target
)	: graph_(std::move(graph))
	, target_(std::move(target))
{
	perform_sort(target_);
}


void TopoSort::dfs(const std::string &expr, std::unordered_set<std::string> &visited)
{
	visited.insert(expr);

	auto rule = graph_[expr][0];
	if (rule == "axiom")
	{
		sorted_expressions_.push_back(expr);
		return;
	}

	for (std::size_t i = 1; i < graph_[expr].size(); ++i)
	{
		if (const auto dependency = graph_[expr][i];
			!visited.contains(dependency))
		{
			dfs(dependency, visited);
		}
	}

	sorted_expressions_.push_back(expr);
}


void TopoSort::perform_sort(const std::string &target)
{
	std::unordered_set<std::string> visited;

        // start from the target expression
        dfs(target, visited);
}


std::string TopoSort::to_string() const
{
	std::stringstream ss;
	std::unordered_map<std::string, std::size_t> index_of;

	for (std::size_t i = 0; i < sorted_expressions_.size(); ++i)
	{
		index_of[sorted_expressions_[i]] = i + 1;
	}

	for (std::size_t i = 0; i < sorted_expressions_.size(); ++i)
	{
		const auto &expr = graph_.at(sorted_expressions_[i]);
		ss << index_of[sorted_expressions_[i]] << ". ";

		if (expr[0] == "mp")
		{
			ss << "mp(" << index_of[expr[1]] << ',' << index_of[expr[2]] << "): ";
		}
		else
		{
			ss << "axiom: ";
		}

		ss << sorted_expressions_[i] << '\n';
	}

	return ss.str();
}
