#ifndef AST_HPP
#define AST_HPP

#include <cstdint>
#include <ostream>
#include <algorithm>
#include <vector>
#include <array>

constexpr const std::size_t INVALID_INDEX = static_cast<std::size_t>(-1);

enum class Operation : std::int32_t
{
	Nop = 0,
	Negation,
	Implication,
	Disjunction,
	Conjunction,
	Xor,
	Equivalent
};


/**
 * @brief determine whether operation is commutative or not
 */
bool is_commutative(Operation operation);


/**
 * @brief returns !(operation)
 * @note list:
 * Nop - Nop
 * Negation - Negation
 * Implication - Conjuction
 * Disjunction - Conjuction
 * Conjunction - Implication
 * Xor - Equivalent
 * Equivalent - Xor
 */
Operation opposite(Operation operation);


struct ASTNode
{
	std::int32_t var;
	Operation op;

	// references to self,left,right,parent
	std::array<std::size_t, 4> refs;

	inline ASTNode(std::int32_t var = 0,
		Operation operation = Operation::Nop,
		std::size_t self = INVALID_INDEX,
		std::size_t left = INVALID_INDEX,
		std::size_t right = INVALID_INDEX,
		std::size_t parent = INVALID_INDEX) noexcept
		: var(var)
		, op(operation)
		, refs{self, left, right, parent}
	{}

	std::string to_string() const;
	inline bool is_leaf() const noexcept
	{
		return refs[1] == INVALID_INDEX && refs[2] == INVALID_INDEX;
	}
	inline bool is_valid() const noexcept
	{
		return var != 0 || op != Operation::Nop;
	}

	inline std::size_t id() const noexcept { return refs[0]; }
	inline std::size_t left() const noexcept { return refs[1]; }
	inline std::size_t right() const noexcept { return refs[2]; }
	inline std::size_t parent() const noexcept { return refs[3]; }

	// we don't compare refs
	inline bool operator==(const ASTNode& other) const noexcept
	{
		return var == other.var && op == other.op;
	}
};


class Expression
{
private:
	std::vector<ASTNode> tokens_;

	inline bool in_range(std::size_t index) const noexcept
	{
		return index < tokens_.size();
	}

	inline bool equals(const Expression &other) const noexcept
	{
		return std::ranges::equal(tokens_, other.tokens_);
	}
public:
        Expression() : tokens_{} {}
	Expression(std::vector<ASTNode> tokens_) : tokens_(std::move(tokens_)) {}
	Expression(const Expression &other) : tokens_(other.tokens_) {}
	Expression &operator=(const Expression &other) { tokens_ = other.tokens_; }

	// getters
	inline std::size_t n_tokens() const noexcept { return tokens_.size(); }
	std::size_t n_ops() const noexcept;
	std::size_t n_vars() const noexcept;
	std::vector<std::int32_t> vars() const noexcept;
	bool contains(std::int32_t needle) const noexcept;

	// tree getters
	inline const ASTNode &root() const { return tokens_[0]; }
	inline ASTNode &root() { return tokens_[0]; }
	inline const ASTNode &operator[](std::size_t idx) const { return tokens_[idx]; }
	std::size_t left(std::size_t index) const noexcept;
	std::size_t right(std::size_t index) const noexcept;
	std::size_t parent(std::size_t index) const noexcept;
	void insert(std::size_t index, const Expression &expression, std::size_t side = 1) noexcept;
	void insert(std::size_t index, const ASTNode &node, std::size_t side = 1) noexcept;
	void replace(std::int32_t var, const Expression &expression) noexcept;

	// get subtree down from index
	Expression subtree(std::size_t index) const noexcept;

	// comparisons
	inline bool operator==(const Expression &other) const noexcept
	{
		return equals(other);
	}

	inline bool operator!=(const Expression &other) const noexcept
	{
		return !equals(other);
	}

	// inplace negation of subtree
	void negation_inplace(std::size_t index = 0);
	Expression negation() const;
};


/**
 * Non-recursive operator
 */
std::ostream &operator<<(std::ostream &out, const Expression &expression);

/**
 * expression constructor
 */
Expression contruct_expression(
	const Expression &lhs,
	Operation op,
	const Expression &rhs
);

#endif // AST_HPP
