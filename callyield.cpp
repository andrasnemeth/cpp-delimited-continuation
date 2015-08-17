#include <boost/assert.hpp>
#include <boost/context/all.hpp>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>

// A simple stack allocator
// Function names kept lower case to keep compatibility with boost context
template< std::size_t Max, std::size_t Default, std::size_t Min >
class SimpleStackAllocator {
public:
    static std::size_t maximum_stacksize() {
        return Max;
    }

    static std::size_t default_stacksize() {
        return Default;
    }

    static std::size_t minimum_stacksize() {
        return Min;
    }

    void* allocate(std::size_t size) const {
        BOOST_ASSERT(minimum_stacksize() <= size);
        BOOST_ASSERT(maximum_stacksize() >= size);

        void* limit = std::calloc(size, sizeof(char));
        if (!limit) {
            throw std::bad_alloc();
//            std::memset(limit, 0xFF, size);
        }

        return static_cast<char*>(limit) + size;
    }

    void deallocate(void * vp, std::size_t size) const {
        BOOST_ASSERT(vp);
        BOOST_ASSERT(minimum_stacksize() <= size);
        BOOST_ASSERT(maximum_stacksize() >= size);

        void* limit = static_cast<char*>(vp) - size;
        std::free(limit);
    }
};

//============================================================================//
// these memory values coming from the examples provided by the library
// also this allocator is copied from examples
typedef SimpleStackAllocator<
        8 * 1024 * 1024, // 8mB
        64 * 1024, // 64 kB
        8 * 1024 // 8 kB
        > StackAllocator;

//============================================================================//

template<typename Result>
class Continuation;

//============================================================================//

template<typename Result>
void continueContinuation(intptr_t arg) {
//    std::cout << "e continueContinuation" << std::endl;
    Continuation<Result>* continuation = (Continuation<Result>*) arg;
    continuation->contextFunction();
}

//============================================================================//
// Base class declaration
template<typename Result>
class Continuation {
public:
    Continuation() :
            //fun(boost::bind(&Continuation<Result>::contextFunction, this, _1)),
            stackSize(StackAllocator::minimum_stacksize()),
            stackPointerDelimited(stackAllocator.allocate(
                    stackSize)),
            delimitedContext(
                    static_cast<boost::context::fcontext_t*>(
                            boost::context::make_fcontext(stackPointerDelimited,
                            stackSize, &continueContinuation<Result>))),
            finished(false) {
    }

    virtual ~Continuation() {
        stackAllocator.deallocate(stackPointerDelimited, stackSize);
    }

    Continuation& operator ()() {
        return reduce();
    }

    Continuation& reduce() {
        std::cout << "e reduce" << std::endl;
        boost::context::jump_fcontext(&mainContext, delimitedContext,
                reinterpret_cast<intptr_t>(this));
        return *this;
    }

    std::size_t getStackSize() {
        char* start = reinterpret_cast<char*>(stackPointerDelimited);
        char* end = start - stackSize;
        std::size_t freeSize = 0;
        while (start != end) {
            if (*start == static_cast<char>(0xFF)) {
                ++freeSize;
            } else {
                freeSize = 0;
            }
            --start;
//            if (start == end) break;
        }
        return stackSize - freeSize;
    }

//    std::size_t getStackSize() {
//        char* start = (char*) stackPointerDelimited;
//        char* end = start - stackSize;
//        std::size_t size = 0;
//        for (; start != end; --start) {
//            if (*start != (char)0x00) {
//                ++size;
//            }
//        }
//        return size;
//    }

    bool isFinished() {
        return finished;
    }

    Result getResult() {
        return result;
    }

protected:
    void yield() {
//        std::cout << "yield size: ";
//        std::cout << delimitedContext->fc_stack.size
//                << " p: " << delimitedContext->fc_stack.sp << std::endl;
        std::cout << "current stack size is: ";
        int a;
        std::cout << ((int*)stackPointerDelimited - &a) << std::endl;
        boost::context::jump_fcontext(delimitedContext, &mainContext, 0);
    }

    virtual void contextFunction() = 0;

    void returnResult(const Result& resultValue) {
        result = resultValue;
        finished = true;
        yield();
    }

private:
    Result result;
    std::function<Result(intptr_t)> fun;
    StackAllocator stackAllocator;
    std::size_t stackSize;
    void* stackPointerDelimited;
    boost::context::fcontext_t mainContext;
    boost::context::fcontext_t* delimitedContext;
    bool finished;

    friend void continueContinuation<Result>(intptr_t);
//    std::function<Result(intptr_t)>
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class ContinuationExample : public Continuation<int> {
public:
    ContinuationExample() : number(1) {
    }

    // for testing purposes
    int getNumber() {
        return number;
    }

protected:
    virtual void contextFunction() {
        yield();
        for (int i = 0; i < 10; ++i) {
            std::cout << "contextFunction" << std::endl;
            number *= 2;
            yield();
        }
        returnResult(number);
    }
private:
    int number;
};

int main() {
    int counter = 0;
    ContinuationExample ce;
    while (!ce.isFinished() && counter < 20) {
        ce.reduce();
        std::cout << "the continuation is reduced, a partial result is "
            "yielded: " << ce.getNumber() << std::endl;
        std::cout << "current stack size: " << ce.getStackSize() << std::endl;
        ++counter;
    }
    std::cout << "the contination is finished, result is: "
              << ce.getResult() << std::endl;
}
