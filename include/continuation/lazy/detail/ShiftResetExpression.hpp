#ifndef CONTINUATION_LAZY_DETAIL_SHIFTRESETEXPRESSION_HPP
#define CONTINUATION_LAZY_DETAIL_SHIFTRESETEXPRESSION_HPP

#include "ShiftResetTraits.hpp"
#include "domain/ShiftReset.hpp"
#include "grammar/CaptureExtractor.hpp"

#include <boost/proto/extends.hpp>

#include <type_traits>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
//----------------------------------------------------------------------------//

template<typename Expr>
class ShiftResetExpression : boost::proto::extends<Expr,
        ShiftResetExpression<Expr>, domain::ShiftReset> {
private:
    using BaseType = boost::proto::extends<Expr, ShiftResetExpression<Expr>,
            domain::ShiftReset>;

public:
    explicit ShiftResetExpression(const Expr& expr = Expr{}) : BaseType(expr) {
    }

    template<typename ContainedExpr>
    auto operator ()(ContainedExpr&& containedExpr) const {

        using Result = ShiftResetTraits<ContainedExpr>::ContinuationResult;

        return Continuation<Result, ContainedExpr>(
                    std::move(containedExpr));

            // using Arg = typename K::argument_type;

            // using Placeholder = PlaceholderExpr<
            //         typename std::result_of<PlaceholderGrammar(LazyExpr)>::type,
            //                 Result, Arg>;

            // return Placeholder(Placeholder(lazyExpr))(Lambda());
    }
};

//----------------------------------------------------------------------------//
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_SHIFTRESETEXPRESSION_HPP
