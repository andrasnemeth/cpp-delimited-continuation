#ifndef CONTINUATION_LAZY_RESET_HPP
#define CONTINUATION_LAZY_RESET_HPP

#include "detail/ShiftResetExpression.hpp"
#include "detail/tag/Reset.hpp"

#include <boost/proto/traits.hpp>

//============================================================================//
namespace continuation {
namespace lazy {
//----------------------------------------------------------------------------//

const detail::ShiftResetExpression<
        boost::proto::terminal<detail::tag::Reset>::type> reset;

//----------------------------------------------------------------------------//
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_RESET_HPP
