#ifndef CONTINUATION_LAZY_SHIFT_HPP
#define CONTINUATION_LAZY_SHIFT_HPP

#include "detail/tag/Shift.hpp"

#include <boost/proto/traits.hpp>

//============================================================================//
namespace continuation {
namespace lazy {
//----------------------------------------------------------------------------//

const boost::proto::terminal<detail::tag::Shift>::type shift{};

//----------------------------------------------------------------------------//
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_SHIFT_HPP
