#ifndef CONTINUATION_LAZY_DETAIL_GRAMMAR_DELIMITEDEXPRESSION_HPP
#define CONTINUATION_LAZY_DETAIL_GRAMMAR_DELIMITEDEXPRESSION_HPP

#include "../tag/Foreach.hpp"
#include "../tag/Shift.hpp"
#include "../tag/Reset.hpp"
#include "../transform/Placeholder.hpp"
#include "../transform/Range.hpp"

#include <boost/proto/proto_fwd.hpp>
#include <boost/proto/matches.hpp>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
namespace grammar {
//----------------------------------------------------------------------------//

// This is not a real grammar, it checks if the expression tree contains a
// shift-part and replaces the subtree with the result of PlaceholderTransform.
// The grammar is extended to make a range transformation to support iteration.
struct DelimitedExpression : boost::proto::or_<
		boost::proto::when<
				boost::proto::subscript<
				        boost::proto::terminal<tag::Foreach>,
			            boost::proto::_>,
				transform::Range(boost::proto::_value(DelimitedExpression(
						boost::proto::_right)))>,
        boost::proto::when<
                boost::proto::function<boost::proto::terminal<tag::Reset>,
                        boost::proto::_>,
                DelimitedExpression(boost::proto::_right)>,
        boost::proto::when<boost::proto::function<
                                   boost::proto::terminal<tag::Shift>,
                                   boost::proto::terminal<boost::proto::_>>,
                transform::Placeholder>,
        boost::proto::nary_expr<boost::proto::_,
                boost::proto::vararg<DelimitedExpression>>> {
};

//----------------------------------------------------------------------------//
} // namespace grammar
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_GRAMMAR_DELIMITEDEXPRESSION_HPP
