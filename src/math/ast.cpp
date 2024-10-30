#include "ast.hpp"


/**
 * @note: https://studopedia.ru/13_131857_prioritet-logicheskih-operatsiy.html
 */
std::int32_t priority(Operation operation)
{
	switch (operation)
	{
		case Operation::Negation:
			return 5;
		case Operation::Disjunction:
			return 3;
		case Operation::Conjunction:
			return 4;
		case Operation::Implication:
			return 1;
		case Operation::Xor:
			return 2;
		case Operation::Equivalent:
			return 2;
		default:
		break;
	}

	return 0;
}


ASTNode::ASTNode(
	std::int32_t var,
	Operation op,
	std::shared_ptr<ASTNode> left,
	std::shared_ptr<ASTNode> right,
	ASTNode *parent
)	: var(var)
	, op(op)
	, left(std::move(left))
	, right(std::move(right))
	, parent(parent)
{
	// reset parent pointers
	if (this->left != nullptr)
	{
		this->left->parent = this;
	}
	if (this->right != nullptr)
	{
		this->right->parent = this;
	}
}


ASTNode::ASTNode(ASTNode &&other)
	: var(other.var)
	, op(other.op)
	, left(std::move(other.left))
	, right(std::move(other.right))
	, parent(std::move(other.parent))
{
	// reset parent pointers
	if (this->left != nullptr)
	{
		this->left->parent = this;
	}
	if (this->right != nullptr)
	{
		this->right->parent = this;
	}

	other.var = 0;
	other.op = Operation::Nop;
	other.left = nullptr;
	other.right = nullptr;
	other.parent = nullptr;
}


ASTNode &ASTNode::operator=(ASTNode &&other)
{
	if (&other != this)
	{
		var = other.var;
		op = other.op;
		left = std::move(other.left);
		right = std::move(other.right);
		parent = std::move(other.parent);

		// reset parent pointers
		if (left != nullptr)
		{
			left->parent = this;
		}
		if (right != nullptr)
		{
			right->parent = this;
		}

		other.var = 0;
		other.op = Operation::Nop;
		other.left = nullptr;
		other.right = nullptr;
		other.parent = nullptr;
	}

	return *this;
}


std::shared_ptr<ASTNode> ASTNode::deepcopy() const
{
	std::shared_ptr<ASTNode> lhs;
	std::shared_ptr<ASTNode> rhs;

	if (left != nullptr)
	{
		lhs = left->deepcopy();
	}
	if (right != nullptr)
	{
		rhs = right->deepcopy();
	}

	return std::make_shared<ASTNode>(var, op, lhs, rhs);
}


std::ostream &operator<<(std::ostream &out, const std::shared_ptr<ASTNode> &node)
{
	if (node == nullptr)
	{
		return out;
	}

	const bool should_use_brackets = node->parent != nullptr && !node->is_leaf();
	if (should_use_brackets)
	{
		out << "(";
	}

	out << node->left;
	out << node->stringify();
	out << node->right;

	if (should_use_brackets)
	{
		out << ")";
	}

	return out;
}
