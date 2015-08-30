#ifndef CONTINUATION_LAZY_DETAIL_TRANSFORM_PLACEHOLDER_HPP
#define CONTINUATION_LAZY_DETAIL_TRANSFORM_PLACEHOLDER_HPP

#include "../tag/Placeholder.hpp"

#include <boost/proto/expr.hpp>
#include <boost/proto/transform/impl.hpp>
#include <boost/proto/traits.hpp>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
namespace transform {
//----------------------------------------------------------------------------//

// A callable transformation that is called by proto when the shift-part of an
// expression is reached. With this, the shift-part is replaced with a single
// placeholder to indicate where to substitute the given parameter when 'k' is
// called.
struct Placeholder : boost::proto::transform<Placeholder> {

    template<typename Expr, typename Unused1, typename Unused2>
    struct impl : boost::proto::transform_impl<Expr, Unused1, Unused2> {

        typedef typename boost::proto::result_of::as_expr<
                tag::Placeholder>::type result_type;

        result_type operator ()(typename impl::expr_param, typename impl::state_param,
                typename impl::data_param) const {
            return boost::proto::as_expr(tag::Placeholder());
        }
    };
};

//----------------------------------------------------------------------------//
} // namespace transform
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_TRANSFORM_PLACEHOLDER_HPP
