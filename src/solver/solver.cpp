#include <queue>
#include <ranges>
#include "solver.hpp"
#include "../math/helper.hpp"


Solver::Solver(std::vector<std::shared_ptr<ASTNode>> axioms,
	std::shared_ptr<ASTNode> target)
	: axioms_(std::move(axioms))
	, hypotheses_()
	, target_(std::move(target))
{}


bool Solver::is_target_proved(std::ostream &out) const
{
	for (const auto &hypothesis : hypotheses_)
	{
		if (is_follows(hypothesis, target_))
		{
			out << hypothesis << " ⊢ " << target_ << '\n';
			return true;
		}
	}

	return false;
}


// ~O(m*n^2 + n^3)
void Solver::produce_expressions(std::ostream &out)
{
	// TODO: use all rules
	std::vector<expression_t> new_hypotheses;
	for (const auto &axiom : axioms_)
	{
		for (const auto &hypothesis : hypotheses_)
		{
			auto expression = mp(hypothesis, axiom);
			if (expression != nullptr)
			{
				thought_chain_log[expression->to_string()] = {
					"(modes ponens)",
					{hypothesis->to_string(), axiom->to_string()}
				};

				new_hypotheses.push_back(expression);
			}
		}

		if (auto expression = mp(target_, axiom); expression != nullptr)
		{
			thought_chain_log[expression->to_string()] = {
				"(modes ponens)",
				{target_->to_string(), axiom->to_string()}
			};

			new_hypotheses.push_back(expression);
		}
	}

	for (const auto &hypothesis1 : hypotheses_)
	{
		for (const auto &hypothesis2 : hypotheses_)
		{
			// mp should not modify hypotheses
			if (auto expression = mp(hypothesis1, hypothesis2, false); expression != nullptr)
			{
				thought_chain_log[expression->to_string()] = {
					"(modes ponens)",
					{hypothesis1->to_string(), hypothesis2->to_string()}
				};

				new_hypotheses.push_back(expression);
			}
		}
	}


	// heaviest part
	std::unordered_map<std::int32_t, expression_t> remapping;
	for (const auto &hypothesis1 : hypotheses_)
	{
		for (const auto &hypothesis2 : hypotheses_)
		{
			remapping[1] = hypothesis1;
			remapping[2] = hypothesis2;

			auto expression1 = axioms_[0]->deepcopy();
			unify(expression1, remapping);

			new_hypotheses.push_back(expression1);

			auto expression2 = axioms_[2]->deepcopy();
			unify(expression2, remapping);

			new_hypotheses.push_back(expression2);
		}
	}

	for (const auto &new_hypothesis : new_hypotheses)
	{
		if (new_hypothesis->operations() > 7)
		{
			continue;
		}

		add_hypothesis(new_hypothesis);
	}
}


std::ostream &Solver::print_hypotheses(std::ostream &out)
{
	for (std::size_t i = 0; i < hypotheses_.size(); ++i)
	{
		out << hypotheses_[i];

		if (i + 1 < hypotheses_.size())
		{
			out << ", ";
		}
	}

	return out << '\n';
}


void Solver::add_hypothesis(expression_t expression)
{
	for (const auto &hypothesis : hypotheses_)
	{
		if (is_same_expression(hypothesis, expression))
		{
			return;
		}
	}

	hypotheses_.emplace_back(expression->deepcopy());
}


// TODO: add search time limit
void Solver::solve()
{
	std::stringstream tss;
	ss.clear();

	// step 1: standartize target
	standartize(target_);

	// step 2: normalize target
	normalize(target_);

	// step 3: decompose target using deduction rule
	deduction_theorem_decomposition(ss, hypotheses_, target_);

	// step 4: standartize target (again)
	standartize(target_);
	ss << "standartization of target: " << target_ << '\n';

	// step 5: apply conjunction splitting rule
	conjunction_splitting_rule(ss, hypotheses_);

	// step 6: standartize hypotheses
	for (auto &hypothesis : hypotheses_)
	{
		standartize(hypothesis);
	}

	ss << "initial hypotheses: ";
	print_hypotheses(ss);
	ss << "initial target: " << target_ << "\n\n";

	// step 7: search for solution
	// TODO: search until time limit is reached
	std::int32_t lim = 3;
	while (!is_target_proved(tss) && lim > 0)
	{
		produce_expressions(ss);
		print_hypotheses(ss);
		--lim;
	}

	produce_report();
	ss << tss.str();
}


void Solver::produce_report()
{
	// ss already contains beginning, so add stored chain
	std::vector<std::string> chain;
	std::queue<std::string> expressions;

	expressions.push(target_->to_string());
	while (!expressions.empty())
	{
		auto expression = expressions.front();
		expressions.pop();

		if (!thought_chain_log.contains(expression))
		{
			continue;
		}

		const auto details = thought_chain_log.at(expression);

		std::stringstream tss;
		tss << details.rule << ": ";

		for (std::size_t i = 0; i < details.assumptions.size(); ++i)
		{
			tss << details.assumptions[i];

			if (i + 1 < details.assumptions.size())
			{
				tss << ", ";
			}

			// add assumptions to queue
			expressions.push(details.assumptions[i]);
		}
		tss << " ⊢ " << expression << '\n';
		chain.push_back(tss.str());
	}

	std::reverse(chain.begin(), chain.end());
	for (const auto &step : chain)
	{
		ss << step;
	}
}


std::string Solver::thought_chain() const
{
	return ss.str();
}
