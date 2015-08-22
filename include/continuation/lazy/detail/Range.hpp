#ifndef CONTINUATION_LAZY_DETAIL_RANGE_HPP
#define CONTINUATION_LAZY_DETAIL_RANGE_HPP

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator_range.hpp>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
//----------------------------------------------------------------------------//

template<typename Iterator>
class RangeWrapper {
public:
    template<typename Container>
    RangeWrapper(Container& container)
            : iterator(boost::begin(container)),
              end(boost::end(container)) {
    }

    RangeWrapper(Iterator begin, Iterator end) : iterator(begin), end(end) {
    }

private:
	Iterator iterator;
	Iterator end;
};

template<typename Container>
using Range = RangeWrapper<typename boost::range_iterator<Container>::type>;

//----------------------------------------------------------------------------//
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_RANGE_HPP
