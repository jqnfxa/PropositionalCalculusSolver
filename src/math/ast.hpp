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
	Disjunction,
	Conjunction,
	Implication,
	Xor,
	Equivalent
};

/**
 * @note: higher priority - operation must be executed first
 * https://studopedia.ru/13_131857_prioritet-logicheskih-operatsiy.html
 */
std::int32_t priority(Operation operation);


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

	inline bool is_leaf() const noexcept { return var != 0; }
	inline std::size_t id() const noexcept { return refs[0]; }
	std::string to_string() const;

	// we don't compare refs
	inline bool operator==(const ASTNode& other) const noexcept
	{
		return var == other.var && op == other.op;
	}
};


class Expression
{
private:
	std::vector<ASTNode> tokens;

	inline bool in_range(std::size_t index) const noexcept
	{
		return index < tokens.size();
	}

	inline bool equals(const Expression &other) const noexcept
	{
		return std::ranges::equal(tokens, other.tokens);
	}
public:
        inline Expression()
		: tokens(1)
	{}
	inline Expression(std::vector<ASTNode> tokens)
		: tokens(std::move(tokens))
	{}

	// getters
	std::size_t n_ops() const noexcept;
	std::size_t n_vars() const noexcept;
	std::vector<std::int32_t> vars() const noexcept;
	bool contains(std::int32_t needle) const noexcept;

	// tree getters
	inline const ASTNode &root() const { return tokens[0]; }
	inline ASTNode &root() { return tokens[0]; }
	std::size_t left(std::size_t index) const noexcept;
	std::size_t right(std::size_t index) const noexcept;
	std::size_t parent(std::size_t index) const noexcept;

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
};


/**
 * Non-recursive operator
 */
std::ostream &operator<<(std::ostream &out, const Expression &expression);


#endif // AST_HPP
