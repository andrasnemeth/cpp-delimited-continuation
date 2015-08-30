#ifndef CONTINUATION_LAZY_DETAIL_DOMAIN_SHIFTRESET_HPP
#define CONTINUATION_LAZY_DETAIL_DOMAIN_SHIFTRESET_HPP

#include <boost/proto/domain.hpp>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
//----------------------------------------------------------------------------//

template<typename Expr>
class ShiftResetExpression;

//============================================================================//
namespace domain {
//----------------------------------------------------------------------------//

struct ShiftReset : boost::proto::domain<
        boost::proto::generator<detail::ShiftResetExpression>> {
};

//----------------------------------------------------------------------------//
} // namespace domain
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_DOMAIN_SHIFTRESET_HPP
