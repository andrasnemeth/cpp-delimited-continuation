#ifndef FUNCTIONAL_LAMBDATRAITS_HPP
#define FUNCTIONAL_LAMBDATRAITS_HPP

#include <boost/type_traits/function_traits.hpp>

#include <type_traits>

//============================================================================//
namespace functional {
//----------------------------------------------------------------------------//

// The classes participating in the deduction of the lambda type traits
// given in the shift part of the expression
template<typename T>
struct MemberFunctionTraits;

template<typename Class, typename R, typename... Args>
struct MemberFunctionTraits<R(Class::*)(Args...) const>
        : boost::function_traits<R(Args...)> {
};

template<typename T>
struct RemoveConstRef {
    using type = typename std::remove_reference<
            typename std::remove_const<T>::type>::type;
};

template<typename Lambda>
struct LambdaTraits : MemberFunctionTraits<
        decltype(&RemoveConstRef<Lambda>::type::operator())> {
};

//----------------------------------------------------------------------------//
} // namespace functional
//============================================================================//

#endif // FUNCTIONAL_LAMBDATRAITS_HPP
