#pragma once

#include "Types.h"

template <typename T>
class VariableVector
{
    uint32_t size;
    uint8_t* vector;
    uint32_t offsetToEnd = 0;
public:
    VariableVector(uint32_t size);
    VariableVector(const VariableVector& other) = delete;
    VariableVector& operator=(const VariableVector& rhs) = delete;
    VariableVector(VariableVector&& other) noexcept;
    VariableVector& operator=(VariableVector&& rhs) noexcept;
    ~VariableVector();

    inline uint8_t* begin() { return vector; }
    inline uint8_t* end() { return begin() + offsetToEnd; }
    inline const uint8_t* begin() const { return vector; }
    inline const uint8_t* end() const { return begin() + offsetToEnd; }

    void clear();

    inline uint32_t getOffsetToEnd() const { return offsetToEnd; }

    template <typename Class, typename... Args>
    void push_back(Args&&... args);
};

template <typename T>
template <typename Class, typename... Args>
inline void VariableVector<T>::push_back(Args&&... args)
{
    uint32_t newOffsetToEnd = offsetToEnd + sizeof(uint32_t) + sizeof(Class);
    assert(newOffsetToEnd <= size);

    uint32_t* pSize = (uint32_t*) &vector[offsetToEnd];
    *pSize = sizeof(Class);
    Class* pPtr = (Class*) &vector[offsetToEnd + sizeof(uint32_t)];
    new (pPtr) Class(std::forward<Args>(args)...);
    assert(dynamic_cast<T*>(pPtr) != nullptr);

    _WriteBarrier();
    _mm_sfence();

    offsetToEnd = newOffsetToEnd;
}

template <typename T>
inline VariableVector<T>::~VariableVector()
{
    if (vector != nullptr)
    {
        for (auto it = begin(); it != end();)
        {
            uint32_t size = *((uint32_t*)it);
            it += sizeof(uint32_t);

            ((T*)(it))->~T();

            it += size;
        }

        free(vector);
        vector = nullptr;
    }
}

template<typename T>
inline void VariableVector<T>::clear()
{
    for (auto it = begin(); it != end();)
    {
        uint32_t size = *((uint32_t*)it);
        it += sizeof(uint32_t);

        ((T*)(it))->~T();

        it += size;
    }

    offsetToEnd = 0;
}

template <typename T>
inline VariableVector<T>::VariableVector(VariableVector&& other) noexcept
        :   size(std::exchange(other.size, 0)), vector(std::exchange(other.vector, nullptr)), offsetToEnd(std::exchange(other.offsetToEnd, 0))
{
}

template <typename T>
inline VariableVector<T>::VariableVector(uint32_t size) : size(size), vector((uint8_t*)malloc(size))
{
}

template <typename T>
inline VariableVector<T>& VariableVector<T>::operator=(VariableVector&& rhs) noexcept
{
    this->~VariableVector();

    size = std::exchange(rhs.size, 0);
    vector = std::exchange(rhs.vector, nullptr);
    offsetToEnd = std::exchange(rhs.offsetToEnd, 0);

    return *this;
}
