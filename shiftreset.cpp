#include <boost/mpl/if.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/proto/context.hpp>
#include <boost/proto/core.hpp>
#include <boost/proto/debug.hpp>
#include <boost/proto/extends.hpp>
#include <boost/proto/transform.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

//============================================================================//

struct ResetTag {
};

// The reset terminal tag of the new expression.
// TODO: examine if it could be a proto function, because in this form the
//       terminal could be used outside of the head of a function expression.
// const boost::proto::terminal<ResetTag>::type reset = {
// };

struct ShiftTag {
};

// The shift terminal tag of the new exppression
// TODO: same as for 'reset' above
const boost::proto::terminal<ShiftTag>::type shift{};

//============================================================================//

struct ForeachTag {
};

const boost::proto::terminal<ForeachTag>::type foreach{};


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
                                typename impl::data_param) const {
            return boost::proto::as_expr(Placeholder());
        }
    };
};

//============================================================================//

template<typename Range>
class RangeWrapper {
public:
    RangeWrapper(Range& range)
			: iterator(boost::begin(range)),
			  end(boost::end(range)) {
    }

private:
	using Iterator = typename boost::range_iterator<Range>::type;
	Iterator iterator;
	Iterator end;
};

//============================================================================//

// This is not a real grammar. It searches for a shift-part of a proto
// expression tree and extracts the lamda function instance from there. The
// lambda is put into the state of the expression and popped out when the
// walking on the tree is finished.
struct ShiftExtractGrammar : boost::proto::or_<
        boost::proto::when<
                boost::proto::function<boost::proto::terminal<ShiftTag>,
                        boost::proto::terminal<boost::proto::_>>,
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
                boost::proto::terminal<boost::proto::_>, boost::proto::_state> // <-- here it is returned
        > {
};

//============================================================================//

// The context of the evaluation. The most part of the processing is left to
// proto built-in evaluation function, only the placeholder needs to be
// substituted to the argument of k.
template<typename R, typename Arg>
class PlaceholderContext : public boost::proto::callable_context<
        const PlaceholderContext<R, Arg>> {
public:
    typedef R result_type;

    explicit PlaceholderContext(Arg&& arg) : arg(std::move(arg)) {
    }

    const Arg& operator ()(boost::proto::tag::terminal, Placeholder) const {
        return arg;
    }

private:
    const Arg arg;
};

//============================================================================//

// This wraps an expression containing a placeholder and provides a callable
// interface to be converted into an std::function hiding Expr as a kind of
// type erasure.
template<typename Expr, typename R, typename Arg>
struct PlaceholderExpr {

    // TODO: check if Expr is a valid shift-reset expression

    explicit PlaceholderExpr(Expr&& expr) : expr(std::move(expr)) {
    }

    R operator ()(Arg&& arg) const {
        PlaceholderContext<R, Arg> context(std::move(arg));
        return boost::proto::eval(expr, context);
    }

private:
    const Expr expr;
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
		boost::proto::when<
				boost::proto::subscript<
						boost::proto::terminal<ForeachTag>,
						boost::proto::_>,
				RangeWrapper<boost::proto::_left>(boost::proto::_value(left))>,
        boost::proto::nary_expr<boost::proto::_,
                boost::proto::vararg<PlaceholderGrammar>>> {
};

//============================================================================//

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

//============================================================================//

// This kind of object is returned by the operator() on the
// ShiftResetExpression. The contained expression is further processed
// and transformed here.
template<typename R, typename Expr>
class Continuation : std::function<R()> {
public:
    //    template<typename Expr>
    Continuation(Expr&& expr) : expr(std::move(expr)) {
        // TODO: eliminate duplicated typedefs
        using Lambda = typename std::result_of<
                ShiftExtractGrammar(Expr)>::type;

        using K = typename LambdaTraits<Lambda>::arg1_type;

        using Arg = typename K::argument_type;

        std::function<R(std::function<R(Arg)>)> lambda =
                std::move(ShiftExtractGrammar()(expr));
        std::function<R(Arg)> k =
                PlaceholderExpr<
                        typename std::result_of<
                                PlaceholderGrammar(Expr)>::type,
                        R, Arg>(PlaceholderGrammar()(expr));
        callable = std::bind(lambda, k);
    }

    Continuation operator()() {
        result = callable();
        return *this;
    }

    bool isTerminated() const {
        return true;
    }

    const boost::optional<R>& getResult() const {
        return result;
    }

    // The original expression is kept for debugging purposes.
    const Expr& getExpr() {
        return expr;
    }

private:
    const Expr expr;
    std::function<R()> callable;
    boost::optional<R> result;
};

template<typename Char, typename Traits, typename R, typename Expr>
std::basic_ostream<Char, Traits>& operator<<(
        std::basic_ostream<Char, Traits>& stream,
        const Continuation<R, Expr>& continuation) {
    return stream << continuation.getResult();
}

// It is possible to create the ShiftResetExpression type. It would provide more
// control on how an expression is assembled and also validate expressions.
// Using this, all expression might have an operator() that can do the
// evaluation.
template<typename Expr>
class ShiftResetExpression;

struct ShiftResetDomain : boost::proto::domain<
        boost::proto::generator<ShiftResetExpression>> {
};

template<typename Expr>
class ShiftResetExpression : boost::proto::extends<Expr,
        ShiftResetExpression<Expr>, ShiftResetDomain> {
private:
    using BaseType = boost::proto::extends<Expr, ShiftResetExpression<Expr>,
            ShiftResetDomain>;

public:
    explicit ShiftResetExpression(const Expr& expr = Expr{}) : BaseType(expr) {
    }

    template<typename ContainedExpr>
    auto operator ()(ContainedExpr&& containedExpr) const {
            using Lambda = typename std::result_of<
                    ShiftExtractGrammar(ContainedExpr)>::type;

            using K = typename LambdaTraits<Lambda>::arg1_type;

            using Result = typename K::result_type;

            return Continuation<Result, ContainedExpr>(
                    std::move(containedExpr));

            // using Arg = typename K::argument_type;

            // using Placeholder = PlaceholderExpr<
            //         typename std::result_of<PlaceholderGrammar(LazyExpr)>::type,
            //                 Result, Arg>;

            // return Placeholder(Placeholder(lazyExpr))(Lambda());
    }
};

// The reset terminal is now enclosed in an expression wrapper. It allows
// full control over the contained expression that is put into the reset()
// body. With this, no auxiliary function call / object creation is needed
// to get a callable and transformed shift-reset expression.
const ShiftResetExpression<boost::proto::terminal<ResetTag>::type> reset;

//============================================================================//

int main() {

    // An example shift-reset expression.
    auto expr = reset(2 + 8 * shift(
                          [](std::function<int(const int&)> k) -> int {
                              return k(6);
                          }) - 1);

    // The created expressions can be statically checked if those conform the
    // grammar.
    std::cout << "Test whether 'expr' matches the grammar: " <<
            (boost::proto::matches<decltype(expr.getExpr()),
            ShiftExtractGrammar>::value) <<  std::endl;
    BOOST_MPL_ASSERT((boost::proto::matches<decltype(expr.getExpr()),
            ShiftExtractGrammar>));

    // The below grammar is invalid and does not compile.
    // TODO: Display a readable error here.
    //auto badExpr = reset(2 + 3 * 5);
    //BOOST_MPL_ASSERT((boost::proto::matches<decltype(badExpr.getExpr()),
    //        ShiftExtractGrammar>));

    // Here we can see the full exrpression tree of 'expr' without any
    // transformations. The lambda couldn't be displayed, just a terminal is
    // there containing a "1", believe me, it is still part of
    // the expression, just proto cannot display it.
    std::cout << "The original expression tree:" << std::endl;
    boost::proto::display_expr(expr.getExpr());

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
    std::function<int(std::function<int(const int&)> k)> lambda = extract(
            expr.getExpr());
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
    auto g = grammar(expr.getExpr());
    // The reset wrapper part is removed and the shift-enclosed part is replaced
    // with an instance of Placeholder
    boost::proto::display_expr(g);
    // TODO: Do this transformation compile time instead of calling the
    //       grammars directly runtime.

    // Now, the whole transformation is done inside the operator() of the
    // ShiftResetExpression that wraps reset.
    // All types are deduced there and in the end we get a simple callable
    // continuation that evaluates the expression.
    auto continuation = reset(1 + shift(
                    [](std::function<int(int)> k) {
                        return k(5);
                    }));

    boost::proto::display_expr(continuation.getExpr());

    // Let's see if it works. If the operator() is called on the delimited
    // continuation, then the expression is evaluated.
    std::cout << "The newer method: " << continuation() << std::endl;

    struct ExampleTag {};
    const boost::proto::terminal<ExampleTag>::type example{};

    boost::proto::display_expr(example[1](2));

    struct ForeachTag {};
    // // const ShiftResetExpression<boost::proto::terminal<ForeachTag>::type> foreach;
    const boost::proto::terminal<ForeachTag>::type foreach{};

    std::vector<int> data = {1,2,3,4};
    boost::proto::display_expr(foreach[data](1));

    auto vexpr = reset(foreach[data](shift(
                        [](std::function<int(int)> k) {
                            return k(0);
                        })));

    // boost::proto::display_expr(vexpr.getExpr());


    // TODO: contrive how to do the selection- and iteration-based use cases.
    //       something similar needs to be created here that is
    //       boost.phoenix's if_ and for_.
}
