#include <continuation/lazy/Expression.hpp>
#include <continuation/lazy/ShiftReset.hpp>

#include <boost/optional/optional_io.hpp>

#include <iostream>

//============================================================================//

// struct ShiftTag {};
// struct ResetTag {};
// const boost::proto::terminal<ShiftTag>::type shift2;
// const boost::proto::terminal<ShiftTag>::type reset2;

int main() {
    using continuation::lazy::reset;
    using continuation::lazy::shift;

    // Capture expression
    auto expr = reset(3 + 2 * shift(
                    [](std::function<int(const int&)> k) -> int {
                        return k(1);
                    }) - 1);
    // That is convertible to a nullary std::function
    std::function<int()> exprFun = expr;
    // And has the real type:
    // NOTE: the second template parameter is only for debugging purposes and
    //       can be removed later...
    continuation::lazy::Expression<int, decltype(expr)::ContainedExpr>
            showExprType = expr;

    // THe result of the evaluation:
    std::cout << "Result of the expression: " << expr() << std::endl;
}

//============================================================================//
