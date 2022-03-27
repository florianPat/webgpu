#pragma once

//NOTE: For std::move
#include <utility>
#include "Utils.h"
#include "Types.h"
#include <stdlib.h>

//NOTE: Just returning the pointer does not work... Strong typedefs are not a real thing.
template <typename T>
class Iterator
{
	T* itData;
public:
	Iterator(T* dataIn) : itData(dataIn) {}

	Iterator& operator++()
	{
		++itData;
		return *this;
	}
	Iterator operator++(int32_t)
	{
		Iterator temp(*this);
		operator++();
		return temp;
	}
	Iterator& operator--()
	{
		--itData;
		return *this;
	}
	Iterator operator--(int32_t)
	{
		Iterator temp(*this);
		operator--();
		return temp;
	}
	Iterator& operator+=(uint32_t rhs)
	{
		itData += rhs;
		return *this;
	}
	Iterator& operator-=(uint32_t rhs)
	{
		itData -= rhs;
		return *this;
	}
	Iterator operator+(int32_t rhs) const
	{
		Iterator lhs(*this);
		lhs += rhs;
		return lhs;
	}
	Iterator operator-(int32_t rhs) const
	{
		Iterator lhs(*this);
		lhs -= rhs;
		return lhs;
	}
	uint32_t operator-(const Iterator& rhs) const
	{
		return (uint32_t)(itData - rhs.itData);
	}
	T& operator*()
	{
		return *itData;
	}
	T& operator[](uint32_t indexIn)
	{
		return *(this->operator+(indexIn));
	}
	T* operator->()
	{
		return itData;
	}
	friend bool operator==(const Iterator& lhs, const Iterator& rhs)
	{
		return (lhs.itData == rhs.itData);
	}
	friend bool operator!=(const Iterator& lhs, const Iterator& rhs)
	{
		return !(lhs == rhs);
	}
	//NOTE: If you need other compares, make them!
};

template <typename T>
class ConstIterator
{
	const T* itData;
public:
	ConstIterator(const T* dataIn) : itData(dataIn) {}

	ConstIterator& operator++()
	{
		++itData;
		return *this;
	}
	ConstIterator operator++(int32_t)
	{
		ConstIterator temp(*this);
		operator++();
		return temp;
	}
	ConstIterator& operator--()
	{
		--itData;
		return *this;
	}
	ConstIterator operator--(int32_t)
	{
		ConstIterator temp(*this);
		operator--();
		return temp;
	}
	ConstIterator& operator+=(uint32_t rhs)
	{
		itData += rhs;
		return *this;
	}
	ConstIterator& operator-=(uint32_t rhs)
	{
		itData -= rhs;
		return *this;
	}
	ConstIterator operator+(int32_t rhs) const
	{
		ConstIterator lhs(*this);
		lhs += rhs;
		return lhs;
	}
	ConstIterator operator-(int32_t rhs) const
	{
		ConstIterator lhs(*this);
		lhs -= rhs;
		return lhs;
	}
	uint32_t operator-(const ConstIterator& rhs) const
	{
		return (uint32_t)(itData - rhs.itData);
	}
	const T& operator*()
	{
		return *itData;
	}
	const T& operator[](uint32_t indexIn)
	{
		return *(this->operator+(indexIn));
	}
	const T* operator->()
	{
		return itData;
	}
	friend bool operator==(const ConstIterator& lhs, const ConstIterator& rhs)
	{
		return (lhs.itData == rhs.itData);
	}
	friend bool operator!=(const ConstIterator& lhs, const ConstIterator& rhs)
	{
		return !(lhs == rhs);
	}
	//NOTE: If you need other compares, make them!
};

#define DEFINE_ARRAY_DATA auto arrayData = getRightPointer(); assert(arrayData != nullptr)

template <typename T, uint32_t N>
struct Array
{
	union ArrayUnion
	{
		T arrayValue[N];
		struct
		{
			T* p = nullptr;
			uint32_t capacity = 0;
		} heapArray;

		ArrayUnion() { heapArray.p = nullptr; heapArray.capacity = 0; }
		~ArrayUnion();
	} arrayUnion;

	uint32_t arraySize = 0;
protected:
	constexpr T* getRightPointer();
	constexpr const T* getRightPointer() const;
public:
	~Array();

	inline T& at(uint32_t pos);
	inline const T& at(uint32_t pos) const;
	inline T& operator[](uint32_t pos);
	inline const T& operator[](uint32_t pos) const;

	inline T& front();
	inline const T& front() const;
	inline T& back();
	inline const T& back() const;
	inline T* data();
	inline const T* data() const;

	inline Iterator<T> begin();
	inline Iterator<T> end();

	inline ConstIterator<T> begin() const;
	inline ConstIterator<T> end() const;

	inline uint32_t capacity() const;
	inline uint32_t size() const;
	inline bool empty() const;

	void clear();
	Iterator<T> insert(uint32_t pos, const T& value);
	Iterator<T> insert(const Iterator<T>& pos, const T& value);
	Iterator<T> insert(uint32_t pos, uint32_t count, const T& value);
	Iterator<T> insert(uint32_t pos, uint32_t count);
	Iterator<T> insert(uint32_t pos, T&& value);
	Iterator<T> insert(const Iterator<T>& pos, uint32_t count, const T& value);
	Iterator<T> insert(const Iterator<T>& pos, T&& value);
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

	bool operator==(const Array& rhs) const;
	bool operator!=(const Array& rhs) const;
};

template<typename T, uint32_t N>
inline T& Array<T, N>::at(uint32_t pos)
{
	DEFINE_ARRAY_DATA;

	assert(pos < arraySize);
	return arrayData[pos];
}

template<typename T, uint32_t N>
inline const T& Array<T, N>::at(uint32_t pos) const
{
	DEFINE_ARRAY_DATA;

	assert(pos < arraySize);
	return arrayData[pos];
}

template<typename T, uint32_t N>
inline T& Array<T, N>::operator[](uint32_t pos)
{
	DEFINE_ARRAY_DATA;

	return arrayData[pos];
}

template<typename T, uint32_t N>
inline const T& Array<T, N>::operator[](uint32_t pos) const
{
	DEFINE_ARRAY_DATA;

	return arrayData[pos];
}

template<typename T, uint32_t N>
inline T& Array<T, N>::front()
{
	DEFINE_ARRAY_DATA;

	assert(arraySize >= 1);
	return arrayData[0];
}

template <typename T, uint32_t N>
inline const T& Array<T, N>::front() const
{
	DEFINE_ARRAY_DATA;

	assert(arraySize >= 1);
	return arrayData[0];
}

template <typename T, uint32_t N>
inline T& Array<T, N>::back()
{
	DEFINE_ARRAY_DATA;

	assert(arraySize >= 1);
	return arrayData[arraySize - 1];
}

template <typename T, uint32_t N>
inline const T& Array<T, N>::back() const
{
	DEFINE_ARRAY_DATA;

	assert(arraySize >= 1);
	return arrayData[arraySize - 1];
}

template <typename T, uint32_t N>
inline T* Array<T, N>::data()
{
	DEFINE_ARRAY_DATA;

	return arrayData;
}

template <typename T, uint32_t N>
inline const T* Array<T, N>::data() const
{
	DEFINE_ARRAY_DATA;

	return arrayData;
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::begin()
{
	DEFINE_ARRAY_DATA;

	return Iterator<T>{ arrayData };
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::end()
{
	DEFINE_ARRAY_DATA;

	return Iterator<T>{ arrayData + arraySize };
}

template <typename T, uint32_t N>
inline ConstIterator<T> Array<T, N>::begin() const
{
	DEFINE_ARRAY_DATA;

	return ConstIterator<T>{ arrayData };
}

template <typename T, uint32_t N>
inline ConstIterator<T> Array<T, N>::end() const
{
	DEFINE_ARRAY_DATA;

	return ConstIterator<T>{ arrayData + arraySize };
}

template <typename T, uint32_t N>
inline bool Array<T, N>::empty() const
{
	return (arraySize == 0);
}

template <typename T, uint32_t N>
inline uint32_t Array<T, N>::size() const
{
	return arraySize;
}

template <typename T, uint32_t N>
inline uint32_t Array<T, N>::capacity() const
{
	if constexpr (N == 0)
		return arrayUnion.heapArray.capacity;
	else
		return N;
}

template <typename T, uint32_t N>
inline void Array<T, N>::clear()
{
	DEFINE_ARRAY_DATA;

	for (uint32_t i = 0; i < arraySize; ++i)
	{
		arrayData[i].~T();
	}
	arraySize = 0;
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insert(uint32_t pos, const T& value)
{
	DEFINE_ARRAY_DATA;

	assert((pos <= arraySize) && ((arraySize + 1) <= capacity()));

	++arraySize;
	if (pos != (arraySize - 1))
	{
		new (&arrayData[arraySize - 1]) T(std::move(arrayData[arraySize - 2]));
		for (uint32_t i = (arraySize - 2); i > pos; --i)
		{
			arrayData[i] = std::move(arrayData[i - 1]);
		}
	}
	arrayData[pos] = value;

	return Iterator<T>{ arrayData + pos };
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insert(const Iterator<T>& pos, const T& value)
{
	return insert(pos - begin(), value);
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insert(uint32_t pos, T&& value)
{
	DEFINE_ARRAY_DATA;

	assert((pos <= arraySize) && ((arraySize + 1) <= capacity()));

	++arraySize;
	if (pos != (arraySize - 1))
	{
		new (&arrayData[arraySize - 1]) T(std::move(arrayData[arraySize - 2]));
		for (uint32_t i = (arraySize - 2); i > pos; --i)
		{
			arrayData[i] = std::move(arrayData[i - 1]);
		}
	}
	arrayData[pos] = std::move(value);

	return Iterator<T>{ arrayData + pos };
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insert(const Iterator<T>& pos, T&& value)
{
	return insert(pos - begin(), std::move(value));
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insert(uint32_t pos, uint32_t count, const T& value)
{
	DEFINE_ARRAY_DATA;

	assert((pos <= arraySize) && (count > 0) && ((arraySize + count) <= capacity()));

	if (pos != arraySize)
	{
		//TODO: Isn`t is better if you shift the buckets up by there amount, so that I do not get O(n�)?
		for (uint32_t j = 0; j < count; ++j, ++pos, ++arraySize)
		{
			new (&arrayData[arraySize]) T(std::move(arrayData[arraySize - 1]));
			for (uint32_t i = (arraySize - 1); i > pos; --i)
			{
				arrayData[i] = std::move(arrayData[i - 1]);
			}
		}
	}
	else
	{
		pos += count;
		arraySize += count;
	}

	for (uint32_t i = (pos - count); i < pos; ++i)
	{
		arrayData[i] = value;
	}

	return Iterator<T>{ arrayData + pos - count };
}

template<typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insert(uint32_t pos, uint32_t count)
{
	DEFINE_ARRAY_DATA;

	assert((pos <= arraySize) && (count > 0) && ((arraySize + count) <= capacity()));

	if (pos != arraySize)
	{
		//TODO: Isn`t is better if you shift the buckets up by there amount, so that I do not get O(n�)?
		for (uint32_t j = 0; j < count; ++j, ++pos, ++arraySize)
		{
			new (&arrayData[arraySize]) T(std::move(arrayData[arraySize - 1]));
			for (uint32_t i = (arraySize - 1); i > pos; --i)
			{
				arrayData[i] = std::move(arrayData[i - 1]);
			}
		}
	}
	else
	{
		pos += count;
		arraySize += count;
	}

	for (uint32_t i = (pos - count); i < pos; ++i)
	{
		arrayData[i] = std::move(T());
	}

	return Iterator<T>{ arrayData + pos - count };
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insert(const Iterator<T>& pos, uint32_t count, const T& value)
{
	return insert(pos - begin(), count, value);
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insertPush_back(uint32_t pos, const T& value)
{
	DEFINE_ARRAY_DATA;

	assert((pos <= arraySize) && ((arraySize + 1) <= capacity()));

	new (&arrayData[arraySize]) T(std::move(arrayData[pos]));
	arrayData[pos] = value;
	++arraySize;

	return Iterator<T>(arrayData + pos);
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insertPush_back(const Iterator<T>& pos, const T& value)
{
	return insertPush_back(pos - begin(), value);
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insertPush_back(uint32_t pos, T&& value)
{
	DEFINE_ARRAY_DATA;

	assert((pos <= arraySize) && ((arraySize + 1) <= capacity()));

	new (&arrayData[arraySize]) T(std::move(arrayData[pos]));
	arrayData[pos] = std::move(value);
	++arraySize;

	return Iterator<T>(arrayData + pos);
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::insertPush_back(const Iterator<T>& pos, T&& value)
{
	return insertPush_back(pos - begin(), std::move(value));
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::erase(uint32_t pos)
{
	DEFINE_ARRAY_DATA;

	assert((pos < arraySize));

	arrayData[pos].~T();
	for (uint32_t i = (pos + 1); i < arraySize; ++i)
	{
		arrayData[i - 1] = std::move(arrayData[i]);
	}

	--arraySize;

	return Iterator<T>{ arrayData + pos };
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::erase(const Iterator<T>& pos)
{
	return erase(pos - begin());
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::erase(uint32_t first, uint32_t last)
{
	DEFINE_ARRAY_DATA;

	assert((first < arraySize) && (last <= arraySize));

	for (uint32_t i = first; i < last; ++i)
	{
		arrayData[i].~T();
	}

	if (last != arraySize)
	{
		uint32_t newarraySize = arraySize;
		//TODO: Isn`t is better if you shift the buckets down by there amount, so that I do not get O(n�)?
		for (uint32_t j = last; j > first; --j, --newarraySize)
		{
			for (uint32_t i = j; i < newarraySize; ++i)
			{
				arrayData[i - 1] = std::move(arrayData[i]);
			}
		}

		arraySize = newarraySize;
	}
	else
		arraySize -= (last - first);

	return Iterator<T>{ arrayData + first };
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::erase(const Iterator<T>& first, const Iterator<T>& last)
{
	return erase(first - begin(), last - begin());
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::erasePop_back(uint32_t pos)
{
	DEFINE_ARRAY_DATA;

	assert((pos < arraySize));

	arrayData[pos].~T();
	new (&arrayData[pos]) T(std::move(arrayData[arraySize - 1]));

	--arraySize;

	return Iterator<T>{ arrayData + pos };
}

template <typename T, uint32_t N>
inline Iterator<T> Array<T, N>::erasePop_back(const Iterator<T>& pos)
{
	return erasePop_back(pos - begin());
}

template <typename T, uint32_t N>
inline void Array<T, N>::push_back(const T& value)
{
	DEFINE_ARRAY_DATA;

	assert((arraySize + 1) <= capacity());

	new (&arrayData[arraySize++]) T(value);
}

template <typename T, uint32_t N>
inline void Array<T, N>::push_back(T&& value)
{
	DEFINE_ARRAY_DATA;

	assert((arraySize + 1) <= capacity());

	new (&arrayData[arraySize++]) T(std::move(value));
}

template<typename T, uint32_t N>
template<typename ...Args>
inline void Array<T, N>::emplace_back(Args&&... args)
{
	DEFINE_ARRAY_DATA;

	assert((arraySize + 1) <= capacity());

	new (&arrayData[arraySize++]) T(std::forward<Args>(args)...);
}

template <typename T, uint32_t N>
inline void Array<T, N>::pop_back()
{
	DEFINE_ARRAY_DATA;

	assert(arraySize > 0);

	arrayData[--arraySize].~T();
}

template<typename T, uint32_t N>
inline constexpr T* Array<T, N>::getRightPointer()
{
	if constexpr (N == 0)
		return arrayUnion.heapArray.p;
	else
		return arrayUnion.arrayValue;
}

template<typename T, uint32_t N>
inline constexpr const T* Array<T, N>::getRightPointer() const
{
	if constexpr (N == 0)
		return arrayUnion.heapArray.p;
	else
		return arrayUnion.arrayValue;
}

template<typename T, uint32_t N>
inline Array<T, N>::~Array()
{
	if constexpr (N == 0)
	{
		if (arrayUnion.heapArray.p != nullptr)
		{
			for (uint32_t i = 0; i < arraySize; ++i)
				arrayUnion.heapArray.p[i].~T();

			free(arrayUnion.heapArray.p);
			arrayUnion.heapArray.p = nullptr;
		}
	}
	arraySize = 0;
}

template<typename T, uint32_t N>
inline bool  Array<T, N>::operator==(const Array& rhs) const
{
	if (this->arraySize != rhs.arraySize)
		return false;

	DEFINE_ARRAY_DATA;
	auto otherArrayData = rhs.getRightPointer();
	assert(otherArrayData != nullptr);

	for (uint32_t i = 0; i < this->arraySize; ++i)
	{
		if (arrayData[i] != otherArrayData[i])
			return false;
	}

	return true;
}

template<typename T, uint32_t N>
inline bool  Array<T, N>::operator!=(const Array& rhs) const
{
	return (!(this->operator==(rhs)));
}

namespace ArrayTestSuit
{
	void runTestSuit();
}

template<typename T, uint32_t N>
inline Array<T, N>::ArrayUnion::~ArrayUnion()
{
	//NOTE: Oh man... The union needs a destructor so that the destructor of Array is not implicitly deleted!
}
