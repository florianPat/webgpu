#pragma once

#include "HeapArray.h"

template <typename T>
class Vector : public HeapArray<T>
{
	void expandOrShrink(uint32_t newCapacity);

	void checkAndShrink(uint32_t newSize);
	void checkAndExpand(uint32_t minNewSize);
	Vector(Vector&& other, uint32_t newCapacity);
public:
	Vector();
	Vector(uint32_t count);
	Vector(uint32_t count, const T& value);
	Vector(std::initializer_list<T> initList);
	Vector(const Vector& other);
	Vector& operator=(const Vector& rhs);
	Vector(Vector&& other) noexcept;
	Vector& operator=(Vector&& rhs) noexcept;

	void reserve(uint32_t size);

	Iterator<T> insert(uint32_t pos, const T& value);
	Iterator<T> insert(const Iterator<T>& pos, const T& value);
	Iterator<T> insert(uint32_t pos, T&& value);
	Iterator<T> insert(const Iterator<T>& pos, T&& value);
	Iterator<T> insert(uint32_t pos, uint32_t count, const T& value);
	Iterator<T> insert(const Iterator<T>& pos, uint32_t count, const T& value);
	//NOTE: Do theses functions even make sense??
	Iterator<T> insertPush_back(uint32_t pos, const T& value);
	Iterator<T> insertPush_back(const Iterator<T>& pos, const T& value);
	Iterator<T> insertPush_back(uint32_t pos, T&& value);
	Iterator<T> insertPush_back(const Iterator<T>& pos, T&& value);
	Iterator<T> erase(uint32_t pos);
	Iterator<T> erase(const Iterator<T>& pos);
	Iterator<T> erase(uint32_t first, uint32_t last);
	Iterator<T> erase(const Iterator<T>& first, const Iterator<T>& last);
	Iterator<T> erasePop_back(uint32_t pos);
	Iterator<T> erasePop_back(const Iterator<T>& pos);
	void push_back(const T& value);
	void push_back(T&& value);
	template<typename ...Args>
	void emplace_back(Args&&... args);
	void pop_back();
	void resize(uint32_t count);
	void resize(uint32_t count, const T& value);
	void swap(Vector& other);
	void swap(uint32_t first, uint32_t second);
	void swap(const Iterator<T>& first, const Iterator<T>& second);

	bool operator==(const Vector& rhs) const;
	bool operator!=(const Vector& rhs) const;
	//NOTE: If you need other compares, make them!
};

template<typename T>
inline void Vector<T>::expandOrShrink(uint32_t newCapacity)
{
	Vector<T> newVector(std::move(*this), newCapacity);
	*this = std::move(newVector);

	/*T* newData = (T*) malloc(sizeof(T) * vectorCapacity);
	for (uint32_t i = 0; i < vectorSize; ++i)
	{
		new (&newData[i]) T(std::move(vectorData[i]));
	}
	free(vectorData);
	vectorData = newData;*/
}

template<typename T>
inline void Vector<T>::checkAndShrink(uint32_t newSize)
{
	uint32_t vectorCapacity = capacity();

	if ((vectorCapacity / 2) >= (newSize + 2))
	{
		do
		{
			vectorCapacity /= 2;
		} while ((vectorCapacity / 2) >= (newSize + 2));

		expandOrShrink(vectorCapacity);
	}
}

template<typename T>
inline void Vector<T>::checkAndExpand(uint32_t minNewSize)
{
	uint32_t vectorCapacity = capacity();

	if (vectorCapacity < minNewSize)
	{
		do
		{
			vectorCapacity *= 2;
		} while (vectorCapacity < minNewSize);

		expandOrShrink(vectorCapacity);
	}
}

template<typename T>
inline Vector<T>::Vector(Vector&& other, uint32_t newCapacity) : HeapArray<T>(std::move(other), newCapacity)
{
}

template<typename T>
inline Vector<T>::Vector() : HeapArray<T>(2)
{
}

template<typename T>
inline Vector<T>::Vector(uint32_t count) : HeapArray<T>(count)
{
}

template<typename T>
inline Vector<T>::Vector(uint32_t count, const T & value) : HeapArray<T>(count, 4, value)
{
}

template<typename T>
inline Vector<T>::Vector(std::initializer_list<T> initList) : HeapArray<T>((uint32_t)initList.size() + 4)
{
	for (auto it = initList.begin(); it != initList.end(); ++it)
	{
		HeapArray<T>::push_back(*it);
	}
}

template<typename T>
inline Vector<T>::Vector(const Vector& other) : HeapArray<T>(other)
{
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
	this->~Vector();

	new (this) Vector(rhs);

	return *this;
}

template<typename T>
inline Vector<T>::Vector(Vector && other) noexcept : HeapArray<T>(std::move(other))
{
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(Vector && rhs) noexcept
{
	this->~Vector();

	new (this) Vector(std::move(rhs));

	return *this;
}


template<typename T>
inline void Vector<T>::reserve(uint32_t newSize)
{
	//NOTE: Include an upper bound?
	assert(newSize >= 0);

	checkAndExpand(newSize);
}

template<typename T>
inline Iterator<T> Vector<T>::insert(uint32_t pos, const T & value)
{
	checkAndExpand(size() + 1);

	return HeapArray<T>::insert(pos, value);
}

template<typename T>
inline Iterator<T> Vector<T>::insert(const Iterator<T> & pos, const T & value)
{
	return insert(pos - begin(), value);
}

template<typename T>
inline Iterator<T> Vector<T>::insert(uint32_t pos, T && value)
{
	checkAndExpand(size() + 1);

	return HeapArray<T>::insert(pos, std::move(value));
}

template<typename T>
inline Iterator<T> Vector<T>::insert(const Iterator<T> & pos, T && value)
{
	return insert(pos - begin(), std::move(value));
}

template<typename T>
inline Iterator<T> Vector<T>::insert(uint32_t pos, uint32_t count, const T & value)
{
	checkAndExpand(size() + count);

	return HeapArray<T>::insert(pos, count, value);
}

template<typename T>
inline Iterator<T> Vector<T>::insert(const Iterator<T> & pos, uint32_t count, const T & value)
{
	return HeapArray<T>::insert(pos - begin(), count, value);
}

template<typename T>
inline Iterator<T> Vector<T>::insertPush_back(uint32_t pos, const T & value)
{
	checkAndExpand(size() + 1);

	return HeapArray<T>::insertPush_back(pos, value);
}

template<typename T>
inline Iterator<T> Vector<T>::insertPush_back(const Iterator<T> & pos, const T & value)
{
	return HeapArray<T>::insertPush_back(pos - begin(), value);
}

template<typename T>
inline Iterator<T> Vector<T>::insertPush_back(uint32_t pos, T && value)
{
	checkAndExpand(size() + 1);

	return HeapArray<T>::insertPush_back(pos, std::move(value));
}

template<typename T>
inline Iterator<T> Vector<T>::insertPush_back(const Iterator<T> & pos, T && value)
{
	return insertPush_back(pos - begin(), std::move(value));
}

template<typename T>
inline Iterator<T> Vector<T>::erase(uint32_t pos)
{
	HeapArray<T>::erase(pos);
	checkAndShrink(size());
	DEFINE_ARRAY_DATA;
	return Iterator<T>{ arrayData + pos };
}

template<typename T>
inline Iterator<T> Vector<T>::erase(const Iterator<T> & pos)
{
	return erase(pos - begin());
}

template<typename T>
inline Iterator<T> Vector<T>::erase(uint32_t first, uint32_t last)
{
	HeapArray<T>::erase(first, last);
	checkAndShrink(size());
	DEFINE_ARRAY_DATA;
	return Iterator<T>{ arrayData + first };
}

template<typename T>
inline Iterator<T> Vector<T>::erase(const Iterator<T> & first, const Iterator<T> & last)
{
	return erase(first - begin(), last - begin());
}

template<typename T>
inline Iterator<T> Vector<T>::erasePop_back(uint32_t pos)
{
	HeapArray<T>::erasePop_back(pos);
	checkAndShrink(size());
	DEFINE_ARRAY_DATA;
	return Iterator<T>{ arrayData + pos };
}

template<typename T>
inline Iterator<T> Vector<T>::erasePop_back(const Iterator<T> & pos)
{
	return erasePop_back(pos - begin());
}

template<typename T>
inline void Vector<T>::push_back(const T & value)
{
	checkAndExpand(size() + 1);

	HeapArray<T>::push_back(value);
}

template<typename T>
inline void Vector<T>::push_back(T && value)
{
	checkAndExpand(size() + 1);

	HeapArray<T>::push_back(std::move(value));
}

template<typename T>
template<typename ...Args>
inline void Vector<T>::emplace_back(Args&&... args)
{
	checkAndExpand(size() + 1);

	HeapArray<T>::emplace_back(std::forward<Args>(args)...);
}

template<typename T>
inline void Vector<T>::pop_back()
{
	HeapArray<T>::pop_back();

	checkAndShrink(size());
}

template<typename T>
inline void Vector<T>::resize(uint32_t count)
{
	//NOTE: Include an upper bound?
	assert(count >= 0);

	uint32_t vectorSize = size();

	if (count < vectorSize)
	{
		HeapArray<T>::erase(count, vectorSize);

		checkAndShrink(count);
	}
	else
	{
		checkAndExpand(count);

		HeapArray<T>::insert(vectorSize, (count - vectorSize));
	}
}

template<typename T>
inline void Vector<T>::resize(uint32_t count, const T & value)
{
	//NOTE: Include an upper bound?
	assert(count >= 0);

	uint32_t vectorSize = size();

	if (count < vectorSize)
	{
		HeapArray<T>::erase(count, vectorSize);

		checkAndShrink(count);
	}
	else
	{
		checkAndExpand(count);

		HeapArray<T>::insert(vectorSize, count, value);
	}
}

template<typename T>
inline void Vector<T>::swap(Vector & other)
{
	Vector<T> temp(std::move(other));
	other = std::move(*this);
	*this = std::move(temp);
}

template<typename T>
inline void Vector<T>::swap(uint32_t first, uint32_t second)
{
	assert(first < vectorArray.size() && second < vectorArray.size());

	T temp = std::move(operator[](second));
	operator[](second) = std::move(operator[](first));
	operator[](first) = std::move(temp);
}

template<typename T>
inline void Vector<T>::swap(const Iterator<T>& first, const Iterator<T>& second)
{
	return swap(first - begin(), second - begin());
}

template<typename T>
inline bool Vector<T>::operator==(const Vector& rhs) const
{
	return HeapArray<T>::operator==(rhs);
}

template<typename T>
inline bool Vector<T>::operator!=(const Vector& rhs) const
{
	return HeapArray<T>::operator!=(rhs);
}

namespace VectorTestSuit
{
	void runVectorUnitTest();
	void runStdVectorTest();
}