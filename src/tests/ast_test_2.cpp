#include <iostream>
#include <cassert>
#include "./math/ast.hpp"
#include "./parser/parser.hpp"

void test_expression_to_string(const Expression &expr, const std::string &expected) {
    assert(expr.to_string() == expected);
}

void test_assignment_operator() {
	std::string consecutive_string = "(a+!b)";
	std::string a_xor_not_b_string = "(a+!b)";
	std::string implication_string = ">";

	ASTNode node_a(1);
    ASTNode node_b(2);

	Expression expr_a({node_a});
    Expression expr_b({node_b});

    Expression a_xor_not_b = contruct_expression(expr_a, Operation::Xor, expr_b.negation());

	std::cout << "debug1" << std::endl;

	Expression consecutive_expression = a_xor_not_b;

    std::cout << "debug2" << std::endl;

	consecutive_expression = contruct_expression(consecutive_expression, Operation::Implication, a_xor_not_b); // тут вылетает

    std::cout << "debug3" << std::endl;

	// for (int i = 0; i < 20; ++i) {
	// 	consecutive_expression = contruct_expression(consecutive_expression, Operation::Implication, a_xor_not_b);
	// 	consecutive_string = consecutive_string + implication_string + a_xor_not_b_string;
	// }

	std::cout << consecutive_string << std::endl;
	std::cout << consecutive_expression << std::endl;

	test_expression_to_string(consecutive_expression, consecutive_string);

	std::cout << "Test assignment operator passed." << std::endl;

}

int main() {
	test_assignment_operator();

    std::cout << "All tests passed." << std::endl;
    return 0;
}