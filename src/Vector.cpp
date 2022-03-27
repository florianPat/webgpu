#include "Vector.h"
#include "UnitTester.h"

void VectorTestSuit::runVectorUnitTest()
{
	UnitTester::test([]()
	{
		Vector<int32_t> vec;
		unitAssert(vec.capacity() == 2);
		unitAssert(vec.empty());
		unitAssert(vec.size() == 0);
		unitAssert(vec.begin() == vec.end());
		Vector<int32_t> otherVec(4, 3);
		unitAssert(otherVec.capacity() == 8);
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
		vec.push_back(4);
		vec.push_back(7);
		vec.push_back(2);
		vec.push_back(9);
		vec.push_back(5);
		unitAssert(vec.size() == 9);
		unitAssert(vec.capacity() == 16);
		unitAssert((*(--vec.end())) == 5);
		vec.pop_back();
		unitAssert(vec.back() == 9);
		unitAssert(vec.erase(0, 4).operator*() == 4);
		unitAssert(vec.capacity() == 8);
		unitAssert(vec.size() == 4);
		unitAssert(vec.erase(0).operator*() == 7);
		vec.resize(1, 4);
		vec.resize(5);
		unitAssert(vec.back() == 0);
		Vector<int32_t> oOtherVec = std::move(vec);
		unitAssert(vec.size() == 0);
		oOtherVec.insert(3, 3, 1);
		unitAssert(oOtherVec.size() == 8);

		unitAssert(oOtherVec != vec);
		Vector<int32_t> thisVector(1, 7);
		thisVector.insert(thisVector.size() - 1, 2, 0);
		thisVector.erase(0, 2);
		thisVector.insert(thisVector.size(), 2, 0);
		thisVector.push_back(1);
		thisVector.insert(thisVector.size(), 2, 1);
		thisVector.insert(thisVector.size(), 2, 0);
		unitAssert(oOtherVec == thisVector);
		thisVector.reserve(5);
		thisVector.reserve(20);
		thisVector.clear();
		unitAssert(thisVector.capacity() == 20);

		int32_t sum = 0;
		for (int32_t e : thisVector)
		{
			sum += e;
		}

		return true;
	});
}

void VectorTestSuit::runStdVectorTest()
{
#if 0
#include <vector>

	UnitTester::test([]()
	{
		std::vector<int32_t> vec;
		unitAssert(vec.capacity() == 0);
		unitAssert(vec.empty());
		unitAssert(vec.size() == 0);
		unitAssert(vec.begin() == vec.end());
		std::vector<int32_t> otherVec(4, 3);
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
		vec.push_back(4);
		vec.push_back(7);
		vec.push_back(2);
		vec.push_back(9);
		vec.push_back(5);
		unitAssert(vec.size() == 9);
		unitAssert(vec.capacity() == 16);
		unitAssert((*(--vec.end())) == 5);
		vec.pop_back();
		unitAssert(vec.back() == 9);
		unitAssert(vec.erase(vec.begin(), vec.begin() + 4).operator*() == 4);
		unitAssert(vec.capacity() == 16);
		unitAssert(vec.size() == 4);
		unitAssert(vec.erase(vec.begin()).operator*() == 7);
		vec.resize(1, 4);
		vec.resize(5);
		unitAssert(vec.back() == 0);
		std::vector<int32_t> oOtherVec = std::move(vec);
		unitAssert(vec.size() == 0);
		oOtherVec.insert(oOtherVec.begin() + 3, 3, 1);
		unitAssert(oOtherVec.size() == 8);

		unitAssert(oOtherVec != vec);
		std::vector<int32_t> thisVector(1, 7);
		thisVector.insert(thisVector.end() - 1, 2, 0);
		thisVector.erase(thisVector.begin(), thisVector.begin() + 2);
		thisVector.insert(thisVector.end(), 2, 0);
		thisVector.push_back(1);
		thisVector.insert(thisVector.end(), 2, 1);
		thisVector.insert(thisVector.end(), 2, 0);
		unitAssert(oOtherVec == thisVector);
		thisVector.reserve(5);
		thisVector.reserve(20);
		thisVector.clear();
		unitAssert(thisVector.capacity() == 20);

		int32_t sum = 0;
		for (int32_t e : thisVector)
		{
			sum += e;
		}

		return true;
	});
#endif
}
