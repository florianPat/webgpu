#pragma once

#include <utility>

template<class T>
class UniquePtr
{
    T* pointer = nullptr;
public:
    UniquePtr() = default;
    UniquePtr(nullptr_t) {}
    UniquePtr(T* pointer);
    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr& rhs) = delete;
    UniquePtr(UniquePtr&& other) noexcept;
    template<class U, typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    UniquePtr(UniquePtr<U>&& other) noexcept;
    UniquePtr& operator=(UniquePtr&& rhs) noexcept;
    template<class U>
    UniquePtr& operator=(UniquePtr<U>&& rhs) noexcept;
    ~UniquePtr();
    void reset(T* pointer = nullptr);
    T* release();
    T* get() const;
    explicit operator bool() const;
    T& operator*() const;
    T* operator->() const;
};

template<class T>
class UniquePtr<T[]>
{
    T* pointer = nullptr;
public:
    UniquePtr() = default;
    UniquePtr(nullptr_t) {}
    UniquePtr(T* pointer);
    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr& rhs) = delete;
    UniquePtr(UniquePtr&& other) noexcept;
    UniquePtr& operator=(UniquePtr&& rhs) noexcept;
    ~UniquePtr();
    void reset(T* pointer = nullptr);
	T* release();
    T* get() const;
    explicit operator bool() const;
    T& operator[](size_t i) const;
};

template<class T, class... Args, typename = typename std::enable_if<std::rank<T>::value == 0>::type>
UniquePtr<T> makeUnique(Args&&... args)
{
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

template<class T, typename = typename std::enable_if<std::rank<T>::value == 1>::type>
UniquePtr<T> makeUnique(size_t size)
{
    return UniquePtr<T>(new typename std::remove_extent<T>::type[size]);
}

template<class T, class U>
bool operator==(const UniquePtr<T>& x, const UniquePtr<U>& y)
{
    return (x.get() == y.get());
}

template<class T, class U>
bool operator!=(const UniquePtr<T>& x, const UniquePtr<U>& y)
{
    return (x.get() != y.get());
}

template<class T>
bool operator==(const UniquePtr<T>& x, nullptr_t)
{
    return (!x);
}

template<class T>
bool operator==(nullptr_t, const UniquePtr<T>& x)
{
    return (!x);
}

template<class T>
bool operator!=(const UniquePtr<T>& x, nullptr_t)
{
    return ((bool)x);
}

template<class T>
bool operator!=(nullptr_t, const UniquePtr<T>& x)
{
    return ((bool)x);
}

template<typename T>
inline UniquePtr<T>::UniquePtr(T* pointer) : pointer(pointer)
{
}

template<typename T>
inline UniquePtr<T[]>::UniquePtr(T* pointer) : pointer(pointer)
{
}

template<typename T>
inline UniquePtr<T>::UniquePtr(UniquePtr&& other) noexcept : pointer(other.release())
{
}

template<typename T>
inline UniquePtr<T[]>::UniquePtr(UniquePtr&& other) noexcept : pointer(other.release())
{
}

template<class T>
template<class U, typename>
inline UniquePtr<T>::UniquePtr(UniquePtr<U>&& other) noexcept : pointer(other.release())
{
}

template<typename T>
inline UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr&& rhs) noexcept
{
    this->~UniquePtr();

    new (this) UniquePtr(std::move(rhs));

    return *this;
}

template<typename T>
inline UniquePtr<T[]>& UniquePtr<T[]>::operator=(UniquePtr&& rhs) noexcept
{
    this->~UniquePtr();

    new (this) UniquePtr(std::move(rhs));

    return *this;
}

template<class T>
template<class U>
inline UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr<U>&& rhs) noexcept
{
    this->~UniquePtr();

    new (this) UniquePtr(std::move(rhs));

    return *this;
}

template<typename T>
inline UniquePtr<T>::~UniquePtr()
{
    if(pointer != nullptr)
    {
        delete pointer;
        pointer = nullptr;
    }
}

template<typename T>
inline UniquePtr<T[]>::~UniquePtr()
{
    if(pointer != nullptr)
    {
        delete[] pointer;
        pointer = nullptr;
    }
}

template<typename T>
inline void UniquePtr<T>::reset(T* pointerIn)
{
    this->~UniquePtr();

    pointer = pointerIn;
}

template<typename T>
inline void UniquePtr<T[]>::reset(T* pointerIn)
{
    this->~UniquePtr();

    pointer = pointerIn;
}

template<typename T>
inline T* UniquePtr<T>::get() const
{
    return pointer;
}

template<typename T>
inline T* UniquePtr<T[]>::get() const
{
    return pointer;
}

template<typename T>
inline UniquePtr<T>::operator bool() const
{
    return (pointer != nullptr);
}

template<typename T>
inline UniquePtr<T[]>::operator bool() const
{
    return (pointer != nullptr);
}

template<typename T>
inline T& UniquePtr<T>::operator*() const
{
    return *get();
}

template<typename T>
inline T* UniquePtr<T>::operator->() const
{
    return get();
}

template<class T>
T* UniquePtr<T>::release()
{
    T* result = pointer;
    pointer = nullptr;
    return result;
}

template<class T>
T* UniquePtr<T[]>::release()
{
	T* result = pointer;
	pointer = nullptr;
	return result;
}

template<typename T>
T& UniquePtr<T[]>::operator[](size_t i) const
{
    return get()[i];
}
