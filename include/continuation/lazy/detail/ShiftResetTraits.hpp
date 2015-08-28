#ifndef CONTINUATION_LAZY_DETAIL_SHIFTRESETTRAITS_HPP
#define CONTINUATION_LAZY_DETAIL_SHIFTRESETTRAITS_HPP

#include "grammar/CaptureExtractor.hpp"

#include <functional/LambdaTraits.hpp>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
//----------------------------------------------------------------------------//

template<typename Expr>
struct ShiftResetTraits {
private:
    using Lambda = functional::LambdaTraits<
        typename std::result_of<grammar::CaptureExpression<Expr>(Expr)>::type>;

public:
    using ContinuationResult = typename Lambda::result_type;
    using K = typename Lambda::arg1_type;
    using KResult = typename K::result_type;
};

//----------------------------------------------------------------------------//
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_SHIFTRESETTRAITS_HPP
