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
			out << "target deduced from: " << hypothesis << '\n';
			return true;
		}
	}

	return false;
}


// TODO: add search time limit
void Solver::solve()
{
	ss.clear();

	// step 1: standartize target
	standartize(target_);
	ss << "standartization: " << target_ << '\n';

	// step 2: normalize target
	normalize(target_);
	ss << "normalization: " << target_ << '\n';

	// step 3: decompose target using deduction rule
	deduction_theorem_decomposition(ss, hypotheses_, target_);

	// step 3: standartize target (again)
	standartize(target_);
	ss << "standartization: " << target_ << '\n';

	// step 4: apply conjunction splitting rule
	conjunction_splitting_rule(ss, hypotheses_);

	// step 5: standartize hypotheses
	for (auto &hypothesis : hypotheses_)
	{
		standartize(hypothesis);
	}

	ss << "hypotheses: ";
	for (std::size_t i = 0; i < hypotheses_.size(); ++i)
	{
		ss << hypotheses_[i];

		if (i + 1 < hypotheses_.size())
		{
			ss << ", ";
		}
	}
	ss << "\ntarget: " << target_ << '\n';

	// step 4: search for solution
	// TODO: search until time limit is reached
	while (!is_target_proved(ss))
	{
		break;
	}

	// use MP and given axioms
}


std::string Solver::thought_chain() const
{
	return ss.str();
}
