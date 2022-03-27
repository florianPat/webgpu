#include "Array.h"
#include "HeapArray.h"
#include "UnitTester.h"

void ArrayTestSuit::runTestSuit()
{
	UnitTester::test([]()
	{
		HeapArray<int32_t> vec(2);
		unitAssert(vec.capacity() == 2);
		unitAssert(vec.empty());
		unitAssert(vec.size() == 0);
		unitAssert(vec.begin() == vec.end());
		HeapArray<int32_t> otherVec(4, 3);
		unitAssert(otherVec.capacity() == 4);
		unitAssert(otherVec.size() == 4);
		unitAssert((*(otherVec.begin())) == 3);
		unitAssert(otherVec.begin() != otherVec.end());
		vec = otherVec;
		for (auto it = vec.begin(); it != vec.end(); ++it)
		{
			unitAssert((*it) == 3);
		}
		unitAssert(vec.size() == 4);
		unitAssert(vec.at(2) == 3);
		unitAssert(vec[3] == 3);
		vec.pop_back();
		unitAssert(vec.back() == 3);
		unitAssert(vec.erase(0, 2).operator*() == 3);
		unitAssert(vec.capacity() == 4);
		unitAssert(vec.size() == 1);
		HeapArray<int32_t> oOtherVec = std::move(vec);
		unitAssert(vec.size() == 0);
		oOtherVec.insert(1, 3, 1);
		unitAssert(oOtherVec.size() == 4);

		HeapArray<int32_t> thisVector(7);
		thisVector.insert(thisVector.size(), 2, 0);
		thisVector.erase(0, 2);
		thisVector.insert(thisVector.size(), 2, 0);
		thisVector.push_back(1);
		thisVector.insert(thisVector.size(), 2, 1);
		thisVector.insert(thisVector.size(), 2, 0);

		int32_t sum = 0;
		for (int32_t e : thisVector)
		{
			sum += e;
		}

		return true;
	});
}
