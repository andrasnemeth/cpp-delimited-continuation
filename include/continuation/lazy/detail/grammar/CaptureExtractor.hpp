#ifndef CONTINUATION_LAZY_DETAIL_GRAMMAR_CAPTUREEXTRACTOR_HPP
#define CONTINUATION_LAZY_DETAIL_GRAMMAR_CAPTUREEXTRACTOR_HPP

#include "../tag/Shift.hpp"

#include <boost/proto/matches.hpp>
#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/transform/when.hpp>

#include <type_traits>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
namespace grammar {
//----------------------------------------------------------------------------//

// This is not a real grammar. It searches for a shift-part of a proto
// expression tree and extracts the lamda function instance from there. The
// lambda is put into the state of the expression and popped out when the
// walking on the tree is finished.
struct CaptureExtractor : boost::proto::or_<
        boost::proto::when<
		boost::proto::function<boost::proto::terminal<tag::Shift>,
				boost::proto::terminal<boost::proto::_>>,
                CaptureExtractor(boost::proto::_left,
				boost::proto::_value(boost::proto::_right))>, // <-- here it is put into the state
        // boost::proto::when<
        //         boost::proto::function<boost::proto::terminal<ResetTag>,
        //                 boost::proto::_>,
        //         CaptureExtractor(boost::proto::_right)>,
        // boost::proto::when<
        //         boost::proto::binary_expr<boost::proto::_, CaptureExtractor,
        //                 CaptureExtractor>,
        //         CaptureExtractor(boost::proto::_left,
        //         CaptureExtractor(boost::proto::_right))>,
        // boost::proto::when<
        //         boost::proto::unary_expr<boost::proto::_, CaptureExtractor>,
        //         CaptureExtractor(boost::proto::_right)>,
        boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<CaptureExtractor>>,
        boost::proto::when<
                boost::proto::terminal<boost::proto::_>, boost::proto::_state>> { // <-- here it is returned
};

//----------------------------------------------------------------------------//

template<typename Expr>
auto extractCaptureExpression(const Expr& expr) {
    return CaptureExtractor()(expr);
}

//----------------------------------------------------------------------------//

template<typename Expr>
using CaptureExpression = typename std::result_of<CaptureExtractor(Expr)>::type;

//----------------------------------------------------------------------------//
} // namespace grammar
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_GRAMMAR_CAPTUREEXTRACTOR_HPP
