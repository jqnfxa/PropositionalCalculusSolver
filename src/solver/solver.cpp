#include "solver.hpp"
#include "../math/helper.hpp"


Solver::Solver(std::vector<std::shared_ptr<ASTNode>> axioms,
	std::shared_ptr<ASTNode> target)
	: axioms_(std::move(axioms))
	, hypotheses_()
	, target_(std::move(target))
{}


bool Solver::is_target_proved() const
{
	for (const auto &hypothesis : hypotheses_)
	{
		if (is_follows(hypothesis, target_))
		{
			return true;
		}
	}

	return false;
}


void Solver::solve()
{
	// step 1: decompose target using deduction rule
	hypotheses_ = deduction_theorem_decomposition(target_);
	target_ = hypotheses_.back();
	hypotheses_.pop_back();

	// step 2: apply conjunction splitting rule
	conjunction_splitting_rule(hypotheses_);

	for (const auto &hypothesis : hypotheses_)
	{
		std::cout << hypothesis << '\n';
	}
	std::cout << target_ << '\n';
}


std::vector<std::string> Solver::thought_chain() const
{
	return {};
}
