#pragma once

#include "Array.h"
#include <new>

template <typename T>
struct HeapArray : public Array<T, 0>
{
	HeapArray(uint32_t size);
	HeapArray(uint32_t count, const T& value);
	HeapArray(uint32_t count, uint32_t plusCount, const T& value);
	HeapArray(const HeapArray& other);
	HeapArray(HeapArray&& other) noexcept;
	HeapArray(const HeapArray& other, uint32_t sizeIn);
	HeapArray(HeapArray&& other, uint32_t sizeIn);
	HeapArray& operator=(const HeapArray& other);
	HeapArray& operator=(HeapArray&& other) noexcept;
};

template <typename T>
inline HeapArray<T>::HeapArray(uint32_t size) : Array<T, 0>()
{
	arrayUnion.heapArray.p = (T*)malloc(sizeof(T) * size);
	arrayUnion.heapArray.capacity = size;

    uint8_t* charPtr = (uint8_t*)this->arrayUnion.heapArray.p;
	for(uint32_t i = 0; i < (sizeof(T) * size); ++i)
    {
		charPtr[i] = 0;
    }
}

template <typename T>
inline HeapArray<T>::HeapArray(uint32_t count, const T & value) : HeapArray(count)
{
	for (uint32_t i = 0; i < count; ++i)
		new (&this->arrayUnion.heapArray.p[i]) T(value);

	this->arraySize = count;
}

template<typename T>
inline HeapArray<T>::HeapArray(uint32_t count, uint32_t plusCount, const T & value) : HeapArray(count + plusCount)
{
	for (uint32_t i = 0; i < count; ++i)
		new (&this->arrayUnion.heapArray.p[i]) T(value);

	this->arraySize = count;
}

template <typename T>
inline HeapArray<T>::HeapArray(const HeapArray & other) : HeapArray(other.arrayUnion.heapArray.capacity)
{
	for (uint32_t i = 0; i < other.arraySize; ++i)
		new (&this->arrayUnion.heapArray.p[i]) T(other.arrayUnion.heapArray.p[i]);

	this->arraySize = other.arraySize;
}

template <typename T>
inline HeapArray<T>::HeapArray(HeapArray&& other) noexcept
{
	arrayUnion.heapArray.p = std::exchange(other.arrayUnion.heapArray.p, nullptr);
	arrayUnion.heapArray.capacity = std::exchange(other.arrayUnion.heapArray.capacity, 0);
	arraySize = std::exchange(other.arraySize, 0);
}

template <typename T>
inline HeapArray<T>::HeapArray(const HeapArray & other, uint32_t capacityIn) : HeapArray(capacityIn)
{
	//NOTE: Why would you call this then?
	assert(other.arrayUnion.heapArray.capacity != capacityIn);

	uint32_t size;
	if (other.arraySize < capacityIn)
		size = other.arraySize;
	else
		size = capacityIn;

	for (uint32_t i = 0; i < size; ++i)
	{
		new (&this->arrayUnion.heapArray.p[i]) T(other.arrayUnion.heapArray.p[i]);
	}
	this->arraySize = size;
}

template <typename T>
inline HeapArray<T>::HeapArray(HeapArray && other, uint32_t capacityIn) : HeapArray(capacityIn)
{
	//NOTE: Why would you call this then?
	assert(other.arrayUnion.heapArray.capacity != capacityIn);

	uint32_t size = 0;
	if (other.arraySize <= capacityIn)
		size = other.arraySize;
	else if (other.arraySize > capacityIn)
	{
		for (uint32_t i = capacityIn; i < other.arraySize; ++i)
			other.arrayUnion.heapArray.p[i].~T();

		size = capacityIn;
	}
	else
		InvalidCodePath;

	for (uint32_t i = 0; i < size; ++i)
		new (&this->arrayUnion.heapArray.p[i]) T(std::move(other.arrayUnion.heapArray.p[i]));

	free(other.arrayUnion.heapArray.p);
	other.arrayUnion.heapArray.p = nullptr;
	other.arrayUnion.heapArray.capacity = 0;
	other.arraySize = 0;

	this->arraySize = size;
}

template <typename T>
inline HeapArray<T> & HeapArray<T>::operator=(const HeapArray<T> & other)
{
	this->~HeapArray<T>();

	new (this) HeapArray(other);

	return *this;
}

template <typename T>
inline HeapArray<T> & HeapArray<T>::operator=(HeapArray<T> && other) noexcept
{
	this->~HeapArray<T>();

	new (this) HeapArray(std::move(other));

	return *this;
}
