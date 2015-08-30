#ifndef CONTINUATION_LAZY_RESET_HPP
#define CONTINUATION_LAZY_RESET_HPP

#include "detail/domain/ShiftReset.hpp"
#include "detail/tag/Reset.hpp"
#include "detail/ShiftResetTraits.hpp"
#include "Expression.hpp"

#include <boost/proto/extends.hpp>
#include <boost/proto/traits.hpp>

#include <type_traits>

//============================================================================//
namespace continuation {
namespace lazy {
//----------------------------------------------------------------------------//

//============================================================================//
namespace detail {
//----------------------------------------------------------------------------//

template<typename Expr>
class ShiftResetExpression : boost::proto::extends<Expr,
        ShiftResetExpression<Expr>, detail::domain::ShiftReset> {
private:
    using BaseType = boost::proto::extends<Expr, ShiftResetExpression<Expr>,
            detail::domain::ShiftReset>;

public:
    explicit ShiftResetExpression(const Expr& expr = Expr{}) : BaseType(expr) {
    }

    template<typename ContainedExpr>
    auto operator()(ContainedExpr&& containedExpr) const {

        using Result =
                typename ShiftResetTraits<ContainedExpr>::ContinuationResult;

        return Expression<Result, ContainedExpr>(
                std::move(containedExpr));
    }
};

//----------------------------------------------------------------------------//
}  // namespace detail
//============================================================================//

const detail::ShiftResetExpression<
        boost::proto::terminal<detail::tag::Reset>::type> reset{};

//const boost::proto::terminal<detail::tag::Reset>::type reset;

//----------------------------------------------------------------------------//
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_RESET_HPP
