#ifndef CONTINUATION_LAZY_DETAIL_DELIMITEDEXPRESSION_HPP
#define CONTINUATION_LAZY_DETAIL_DELIMITEDEXPRESSION_HPP

#include "context/Placeholder.hpp"

#include <boost/proto/eval.hpp>
#include <utility>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
//----------------------------------------------------------------------------//

// This wraps an expression containing a placeholder and provides a callable
// interface to be converted into an std::function hiding Expr as a kind of
// type erasure.
template<typename Expr, typename R, typename Arg>
struct DelimitedExpression {

    // TODO: check if Expr is a valid shift-reset expression

    explicit DelimitedExpression(Expr&& expr) : expr(std::move(expr)) {
    }

    R operator()(Arg&& arg) const {
        context::Placeholder<R, Arg> context(std::move(arg));
        return boost::proto::eval(expr, context);
    }

private:
    const Expr expr;
};

//----------------------------------------------------------------------------//
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_DELIMITEDEXPRESSION_HPP
