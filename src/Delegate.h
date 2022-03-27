#pragma once
#include <type_traits>

template <typename T>
class Delegate;

// TODO: Think about to add lambda and other callable (operator() objects) types into this:
// - One needs a new template arguemnt but it should not be part of the class template list because then every delegate has a different type and
//   no storing can be done
// - One would also has to destruct the callable object thing (whatever it is a lambda or not). Currently, the problem does not exist because
//   a function and even method ptr do not contain state or take care of saving back the "this" pointer or whatever variable is used. So in a sense,
//   it is faster to just use member functions, even if it requires more code for small functions... BUT: Epic for example also does not care
//   and just passes a user defined thing into its callbacks (but it also is a c api), so maybe I should also not care even if std::function supports all callables
template <typename R, typename ... Args>
class Delegate<R(Args...)>
{
    typedef R(*MemFn)(void*, Args...);

    void* thiz = nullptr;
    MemFn memFn = nullptr;

    Delegate() = default;
    Delegate(void* thiz, MemFn memFn) : thiz(thiz), memFn(memFn) {}

    template <R(*functionPtr)(Args...)>
    static R functionStub(void*, Args... args)
    {
        return functionPtr(std::forward<Args>(args)...);
    }

    template <typename C, R(C::*methodPtr)(Args...)>
    static R methodStub(void* thiz, Args... args)
    {
        return (((C*)thiz)->*methodPtr)(std::forward<Args>(args)...);
    }
public:
    template <class C, R(C::*methodPtr)(Args...)>
    static Delegate from(C* thiz)
    {
        return Delegate<R(Args...)>(thiz, &methodStub<C, methodPtr>);
    }

    template <R(*functionPtr)(Args...)>
    static Delegate from()
    {
        return Delegate(nullptr, &functionStub<functionPtr>);
    }

    static Delegate empty()
    {
        return Delegate();
    }

    R operator()(Args... args) const
    {
        return memFn(thiz, std::forward<Args>(args)...);
    }

    explicit operator bool() const
    {
        return memFn != nullptr;
    }
};