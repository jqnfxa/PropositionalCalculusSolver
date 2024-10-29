#include "ast.hpp"


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
	if (this->left != nullptr)
	{
		this->left->parent = this;
	}
	if (this->right != nullptr)
	{
		this->right->parent = this;
	}
}


ASTNode &ASTNode::operator=(ASTNode &&other)
{
	if (&other != this)
	{
		op = other.op;
		var = other.var;
		left = std::move(other.left);
		right = std::move(other.right);
		parent = std::move(other.parent);

		if (left != nullptr)
		{
			left->parent = this;
		}
		if (right != nullptr)
		{
			right->parent = this;
		}
	}

	return *this;
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
