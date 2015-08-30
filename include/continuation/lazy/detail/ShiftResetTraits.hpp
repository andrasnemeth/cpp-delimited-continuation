#ifndef CONTINUATION_LAZY_DETAIL_SHIFTRESETTRAITS_HPP
#define CONTINUATION_LAZY_DETAIL_SHIFTRESETTRAITS_HPP

#include "grammar/CaptureExtractor.hpp"

#include <functional/LambdaTraits.hpp>

#include <type_traits>

//============================================================================//
namespace continuation {
namespace lazy {
namespace detail {
//----------------------------------------------------------------------------//

template<typename Expr>
struct ShiftResetTraits {
private:
    using Lambda = functional::LambdaTraits<grammar::CaptureExpression<Expr>>;

public:
    using ContinuationResult = typename Lambda::result_type;
    using K = typename Lambda::arg1_type;
    using KArg = typename K::argument_type;
    using KResult = typename K::result_type;
};

//----------------------------------------------------------------------------//
} // namespace detail
} // namespace lazy
} // namespace continuation
//============================================================================//

#endif // CONTINUATION_LAZY_DETAIL_SHIFTRESETTRAITS_HPP
