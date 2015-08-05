#include <boost/mpl/if.hpp>
#include <boost/proto/context.hpp>
#include <boost/proto/core.hpp>
#include <boost/proto/debug.hpp>
#include <boost/proto/transform.hpp>

#include <functional>
#include <iostream>
#include <type_traits>

//============================================================================//

struct ResetTag {
};

// The reset terminal tag of the new expression.
// TODO: examine if it could be a proto function, because in this form the
//       terminal could be used outside of the head of a function expression.
const boost::proto::terminal<ResetTag>::type reset = {
};

struct ShiftTag {
};

// The shift terminal tag of the new exppression
// TODO: same as for 'reset' above
const boost::proto::terminal<ShiftTag>::type shift = {
};

// It is possible to create the ShiftResetExpression type. It would provide more
// control on how an expression is assembled and also validate expressions.
// Using this, all expression might have an operator() that can do the
// evaluation. For simplicity in the first step, an evaluation context is used.
#if 0
template<typename Expr>
struct ShiftResetExpression;

struct ShiftResetDomain : boost::proto::domain<
        boost::proto::generator<ShiftResetExpression>> {
};

template<typename Expr>
struct ShiftResetExpression : boost::proto::extends<Expr,
        ShiftResetExpression<Expr>, ShiftResetDomain> {

    typedef boost::proto::extends<Expr, ShiftResetExpression<Expr>,
            ShiftResetDomain> BaseType;

    explicit ShiftResetExpression(const Expr& expr = Expr) : BaseType(expr) {
    }
};
#endif

//============================================================================//

struct Placeholder {
};

// A callable transformation that is called by proto when the shift-part of an
// expression is reached. With this, the shift-part is replaced with a single
// placeholder to indicate where to substitute the given parameter when 'k' is
// called.
struct PlaceholderTransform : boost::proto::transform<PlaceholderTransform> {

    template<typename Expr, typename Unused1, typename Unused2>
    struct impl : boost::proto::transform_impl<Expr, Unused1, Unused2> {

        typedef typename boost::proto::result_of::as_expr<Placeholder>
                ::type result_type;

        result_type operator ()(typename impl::expr_param,
                                typename impl::state_param,
                                typename impl::data_param) {
            return boost::proto::as_expr(Placeholder());
        }
    };

};

//============================================================================//

// This is not a real grammar, it checks if the expression tree contains a
// shift-part and replaces the subtree with the result of PlaceholderTransform.
struct PlaceholderGrammar : boost::proto::or_<
        boost::proto::when<
                boost::proto::function<boost::proto::terminal<ResetTag>,
                        boost::proto::_>,
                PlaceholderGrammar(boost::proto::_right)>,
        boost::proto::when<boost::proto::function<
                boost::proto::terminal<ShiftTag>,
                boost::proto::terminal<boost::proto::_>>,
                       PlaceholderTransform>,
        boost::proto::nary_expr<boost::proto::_,
                boost::proto::vararg<PlaceholderGrammar>>> {
};

// This is not a real grammar. It searches for a shift-part of a proto
// expression tree and extracts the lamda function instance from there. The
// lambda is put into the state of the expression and popped out when the
// walking on the tree is finished.
struct ShiftExtractGrammar : boost::proto::or_<
        boost::proto::when<
                boost::proto::function<boost::proto::terminal<ShiftTag>,
                        boost::proto::terminal<boost::proto::_ >>,
                ShiftExtractGrammar(boost::proto::_left,
                        boost::proto::_value(boost::proto::_right))>, // <-- here it is put into the state
        boost::proto::when<
                boost::proto::function<boost::proto::terminal<ResetTag>,
                        boost::proto::_>,
                ShiftExtractGrammar(boost::proto::_right)>,
        boost::proto::when<
                boost::proto::binary_expr<boost::proto::_, ShiftExtractGrammar,
                        ShiftExtractGrammar>,
                ShiftExtractGrammar(boost::proto::_left,
                        ShiftExtractGrammar(boost::proto::_right))>,
        boost::proto::when<
                boost::proto::unary_expr<boost::proto::_, ShiftExtractGrammar>,
                        ShiftExtractGrammar(boost::proto::_right)>,
        boost::proto::when<
                boost::proto::terminal<boost::proto::_>, boost::proto::_state>
        > {
};

// The context of the evaluation. The most part of the processing is left to
// proto built-in evaluation function, only the placeholder needs to be
// substituted to the argument of k.
template<typename R, typename Arg>
class PlaceholderContext : public boost::proto::callable_context<
        const PlaceholderContext<R, Arg>/*,
        const boost::proto::null_context*/> {
public:
    typedef R result_type;

    explicit PlaceholderContext(const Arg& arg) : arg(arg) {
    }

    Arg operator ()(boost::proto::tag::terminal, Placeholder) const {
        return arg;
    }

private:
    Arg arg;
};

// This wraps an expression containing a placeholder and provides a callable
// interface to be converted into an std::function hiding Expr as a kind of
// type erasure.
template<typename Expr, typename R, typename Arg>
struct PlaceholderExpr {

    // TODO: check if Expr is a valid shift-reset expression

    explicit PlaceholderExpr(Expr expr) : expr(expr) {
    }

    R operator ()(Arg arg) {
        PlaceholderContext<R, Arg> context(arg);
        return boost::proto::eval(expr, context);
    }

private:
    Expr expr;
};

//============================================================================//

template<typename Sig>
struct DelimitedContinuation;

//----------------------------------------------------------------------------//

// The main class of a delimited continuation. An arbitrary shift-reset
// expression can be implicitly converted into a DelimitedContinuation.
// Currently there is no better way of doing so.
template<typename R, typename Arg>
class DelimitedContinuation<std::function<R(Arg)>>
    : std::function<R(Arg)> {
public:

    template<typename Expr>
    DelimitedContinuation(const Expr& expr)
            : k(PlaceholderExpr<
                    typename std::result_of<PlaceholderGrammar(Expr)>::type,
                            R, Arg>(PlaceholderGrammar()(expr))),
              lambda(ShiftExtractGrammar()(expr)) {
    }

    R operator ()() {
        return lambda(k);
    }

private:
    std::function<R(Arg)> k;
    std::function<R(std::function<R(Arg)>)> lambda;
 };

// The classes participating in the deduction of the lambda type traits
// given in the shift part of the expression
// TODO: DelimitedContinuation might use this deduction
template<typename T>
struct FunctionTraits2;

template<typename T, typename R, typename Arg>
struct FunctionTraits2<R(T::*)(Arg) const> {

    using Result = R;
    using Argument = Arg;

};

template<typename T>
struct FunctionTraits : FunctionTraits2<decltype(&
        std::remove_reference<
                typename std::remove_const<T>::type>::type::operator())> {
    // TODO: make another metafunction that extracts operator()
    //       and leave this to strip the const &
};

//============================================================================//

int main() {

    // An example shift-reset expression.
    auto expr = reset(2 + 8 * shift(
                          [](std::function<int(const int&)> k) -> int {
                              return k(6);
                          }) - 1);

    // Here is the deduction of the Lambda type
    using Lambda = std::result_of<ShiftExtractGrammar(decltype(expr))>::type;

    // Here is the extraction of the lambda argument type
    std::function<int(std::function<int(const int&)>)> extractedLambda =
            ShiftExtractGrammar()(expr);

    using LArgument = FunctionTraits<Lambda>::Argument;

    LArgument a = [](const int& value) {
        return value + 3;
    };

    std::cout << "The extracted lamba argument type called with 45: " << a(45)
              << std::endl;

    // The created expressions can be statically checked if those conform the
    // grammar.
    std::cout << "Test whether 'expr' matches the grammar" <<
            (boost::proto::matches<decltype(expr),
            ShiftExtractGrammar>::value) <<  std::endl;
    BOOST_MPL_ASSERT((boost::proto::matches<decltype(expr),
            ShiftExtractGrammar>));
    // TODO: create the full grammar without transformations and match every
    //       expression against it. The below example also matches despite
    //       it is not a valid shift-reset expression:
    auto badExpr = reset(2 + 3 * 5);
    BOOST_MPL_ASSERT((boost::proto::matches<decltype(badExpr),
            ShiftExtractGrammar>));

    // Here we can see the full exrpression tree of 'expr' without any
    // transformations. The lambda couldn't be displayed, just a terminal is
    // there containing a "1", believe me, it is still part of
    // the expression, just proto cannot display it.
    std::cout << "The original expression tree:" << std::endl;
    boost::proto::display_expr(expr);

    // Lambda expressions does not satisfy DefaultConstructibe, hence the lambda
    // cannot be extracted from the expression compile-time, just runtime.
    // This is the first problem with lambdas. The below code does not compile.
#if 0
    typedef std::result_of<ShiftExtractGrammar(decltype(expr))>::type Extracted;
    std::remove_reference<Extracted>::type extracted;
    boost::proto::display_expr(extracted);
#endif

    // The lambda in the shift part needs to be extracted. It is converted to
    // the proper function type that is the signature of the lambda expression
    // written into the shift part of "expr"
    ShiftExtractGrammar extract;
    std::function<int(std::function<int(const int&)> k)> lambda = extract(expr);
    // To demonstrate that the original lambda is extracted, see what happens
    // if we pass a function to the lambda that just displays the integer
    // value it is accepted as its argument:
    lambda([](const int& value) {
        std::cout << "Value accepted by lambda is: "
        << value << std::endl;
        return value;
    });

    PlaceholderGrammar grammar;
    // Calling the grammar will do the transformations on the expression g.
    auto g = grammar(expr);
    // The reset wrapper part is removed and the shift-enclosed part is replaced
    // with an instance of Placeholder
    boost::proto::display_expr(g);
    // TODO: Do this transformation compile time instead of calling the
    //       grammars directly runtime.

    // Currently this is the only way to capture a delimited continuation with
    // the correct result and argument types using a lambda expression inside,
    // since the exact type of lambda expressions are not defined by the
    // standard, only that they are convertible to std::function. The compiler
    // does not check convertible types hence the type deduction of the argument
    // type of the lambda is not feasible with these given constraints.
    // Using "auto" as the type of delcont2 will leave the type of the
    // reset(...) expression as a pure proto expression. This is one part that
    // could be developed further.
    DelimitedContinuation<std::function<int(int)>> delcont =
            reset(2 + shift([](std::function<int(int)> k) { return k(3); }));

    // If we call the operator() on the delimited continuation, it will
    // evaluate the expression and returns the result, 5 in this case.
    std::cout << "Evaluating the delimited continuation: "
            << delcont() << std::endl;

    // The original "expr" can be converted into a DelimitedContinuation
    DelimitedContinuation<std::function<int(const int&)>> exprCont = expr;
    std::cout << "Evaluating the original 'expr' as a delimited continuation: "
            << exprCont() << std::endl;

    // TODO: solve the type deduction problem that arises from the usage of
    //       lambda expressions.
    // TODO: contrive how to do the selection- and iteration-based usa cases.
    //       something similar needs to be created here that is
    //       boost.phoenix's if_ and for_.
}
