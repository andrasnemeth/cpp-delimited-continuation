#ifndef CONTINUATION_LAZY_DETAIL_TRANSFORM_RANGE_HPP
#define CONTINUATION_LAZY_DETAIL_TRANSFORM_RANGE_HPP

#include "../Range.hpp"

#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/traits.hpp>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
namespace transform {
//----------------------------------------------------------------------------//

struct Range : boost::proto::callable {

    template<typename Signature>
    struct result;

    template<typename This, typename Container>
    struct result<This(Container&)> {
        using type = typename boost::proto::result_of::as_expr<
                detail::Range<Container>>::type;
    };

    template<typename Container>
    auto operator()(Container& container) {
        return boost::proto::as_expr(detail::Range<Container>(container));
    }
};

//----------------------------------------------------------------------------//
} // namespace transform
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_TRANSFORM_RANGE_HPP
