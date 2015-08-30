#ifndef CONTINUATION_LAZY_CONTINUATION_HPP
#define CONTINUATION_LAZY_CONTINUATION_HPP

#include "detail/grammar/DelimitedExpression.hpp"
#include "detail/grammar/CaptureExtractor.hpp"
#include "detail/DelimitedExpression.hpp"
#include "detail/ShiftResetTraits.hpp"

#include <boost/optional.hpp>

#include <functional>
#include <utility>

//============================================================================//
namespace continuation {
namespace lazy {
//----------------------------------------------------------------------------//

// This kind of object is returned by the operator() on the
// ShiftResetExpression. The contained expression is further processed
// and transformed here.
template<typename R, typename Expr>
class Expression : public std::function<R()> { // TODO: find a better name
public:
    using ContainedExpr = Expr;

    Expression(Expr&& expr) : expr(std::move(expr)) {
        using Arg = typename detail::ShiftResetTraits<Expr>::KArg;
        using Result = typename detail::ShiftResetTraits<Expr>::KResult;

        std::function<R(std::function<Result(Arg)>)> lambda =
                std::move(detail::grammar::extractCaptureExpression(expr));
        std::function<Result(Arg)> k =
                detail::DelimitedExpression<
                typename std::result_of<detail::grammar::DelimitedExpression(
                        Expr)>::type,
                R, Arg>(detail::grammar::DelimitedExpression()(expr));
        callable = std::bind(lambda, k);
    }

    Expression& operator()() {
        result = callable();
        return *this;
    }

    bool isTerminated() const {
        return true;
    }

    const boost::optional<R>& getResult() const {
        return result;
    }

    // The original expression is kept for debugging purposes.
    const Expr& getExpr() {
        return expr;
    }

private:
    const Expr expr;
    // TODO: Use bind expression type instead of function
    //       since it is internal.
    std::function<R()> callable;
    boost::optional<R> result;
};

template<typename Char, typename Traits, typename R, typename Expr>
std::basic_ostream<Char, Traits>& operator<<(
        std::basic_ostream<Char, Traits>& stream,
        const Expression<R, Expr>& continuation) {
    return stream << continuation.getResult();
}


//----------------------------------------------------------------------------//
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_CONTINUATION_HPP
