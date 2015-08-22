#ifndef CONTINUATION_LAZY_DETAIL_CONTEXT_PLACEHOLDER_HPP
#define CONTINUATION_LAZY_DETAIL_CONTEXT_PLACEHOLDER_HPP

#include "../tag/Placeholder.hpp"

#include <boost/proto/tags.hpp>
#include <boost/proto/context/callable.hpp>

#include <utility>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
namespace context {
//----------------------------------------------------------------------------//

// The context of the evaluation. The most part of the processing is left to
// proto built-in evaluation function, only the placeholder needs to be
// substituted to the argument of k.
template<typename R, typename Arg>
class Placeholder : public boost::proto::callable_context<
        const Placeholder<R, Arg>> {
public:
    using result_type = R;

    explicit Placeholder(Arg&& arg) : arg(arg) {
    }

    const Arg& operator()(boost::proto::tag::terminal, tag::Placeholder) const {
        return arg;
    }

private:
    const Arg arg;
};

//----------------------------------------------------------------------------//
} // namespace context
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_CONTEXT_PLACEHOLDER_HPP
