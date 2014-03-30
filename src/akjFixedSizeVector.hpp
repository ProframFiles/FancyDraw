#pragma once
#include "FatalError.hpp"
#include "akjLog.hpp"
#include "akjExceptional.hpp"
#include "akjTypeTraits.hpp"

namespace akj
{
//ensure our stack storage has at least 8 byte alignment
union cStackStorage
{
	char chars[8];
	uint64_t longlong;
};
template <uint32_t kCapacity, typename tElement>
class cArray
{


public:
	typedef tElement value_type;
	typedef tElement& reference;
	typedef const tElement& const_reference;
	typedef uint32_t size_type;
	typedef tElement* iterator;
	typedef const tElement* const_iterator;

	cArray()
		:mSize(0)
	{
	}
	cArray(uint32_t initial_capacity)
		:mSize(0)
	{
		for(uint32_t i = 0; i < initial_capacity; ++i)
		{
			emplace_back();
		}
	}
	cArray(std::initializer_list<tElement> list)
		:mSize(0)
	{
		//CompileErrorIfNotTrue<tLessTest<list., kCapacity+1>>();
		AKJ_ASSERT(list.size() <= kCapacity);
		for(const tElement& item: list)
		{
			emplace_back(item);
		}
	}
	~cArray()
	{
		clear();
	}

	iterator end() 
	{
		return Array()+mSize;
	}

	const_iterator end() const 
	{
		return Array()+mSize;
	}

	const_iterator begin() const 
	{
		return Array();
	}

	iterator begin() 
	{
		return Array();
	}

	uint32_t capacity() const
	{
		return kCapacity;
	}

	bool empty() const
	{
		return mSize ==0;
	}

	uint32_t size() const { return mSize; }
	const tElement& at(uint32_t index) const 
	{
		AKJ_ASSERT(index < mSize);
		return Array()[index];
	}

	tElement& at(uint32_t index)
	{
		AKJ_ASSERT(index < mSize);
		return Array()[index];
	}

	tElement& operator[](uint32_t index)
	{
		return Array()[index];
	}
	
	void SwapAndPop(uint32_t index)
	{
		at(index) = back();
		pop_back();
	}

	const tElement& operator[](uint32_t index) const
	{
		return Array()[index];
	}

	void clear()
	{
		tElement* last = Array() + mSize;
		while (last-- != Array())
		{
			last->~tElement();
			--mSize;
		}
	}



	tElement* data()
	{
		return Array();
	}

	tElement& back()
	{
		AKJ_ASSERT(mSize > 0);
		return Array()[mSize-1];
	}

	const tElement* data() const
	{
		return Array();
	}

	void pop_back()
	{
		AKJ_ASSERT(mSize > 0);
		tElement* last = Array() + mSize;
		last->~tElement();
		--mSize;
	}

	cArray& push_back(std::initializer_list<tElement> list)
	{
		for(const tElement& item: list)
		{
			emplace_back(item);
		}
		return *this;
	}

	cArray& push_back(const tElement& thing)
	{
		AKJ_ASSERT(mSize < kCapacity);
		return emplace_back(thing);
	}

	cArray& emplace_back()
	{
		AKJ_ASSERT(mSize < kCapacity);
		new (Array() + mSize) tElement;
		mSize++;
		return *this;
	}

	template <typename tA>
	cArray& emplace_back(tA&& a)
	{
		AKJ_ASSERT(mSize < kCapacity);
		new (Array() + mSize) tElement(std::forward<tA>(a));
		mSize++;
		return *this;
	}

	template <typename tA, typename tB>
	cArray& emplace_back(tA&& a, tB&& b)
	{
		AKJ_ASSERT(mSize < kCapacity);
		new (Array() + mSize) tElement(std::forward<tA>(a), std::forward<tB>(b));
		mSize++;
		return *this;
	}

	template <typename tA, typename tB, typename tC>
	cArray& emplace_back(tA&& a, tB&& b, tC&& c)
	{
		AKJ_ASSERT(mSize < kCapacity);
		new (Array() + mSize) tElement(std::forward<tA>(a), 
																	std::forward<tB>(b), 
																	std::forward<tC>(c));
		mSize++;
		return *this;
	}

	template <typename tA, typename tB, typename tC, typename tD>
	cArray& emplace_back(tA&& a, tB&& b, tC&& c, tD&& d)
	{
		AKJ_ASSERT(mSize < kCapacity);
		new (Array() + mSize) tElement(std::forward<tA>(a), 
																	std::forward<tB>(b), 
																	std::forward<tC>(c)
																	std::forward<tD>(d));
		mSize++;
		return *this;
	}

	cArray(const cArray& other)
		:mSize(0)
	{
		CopyImpl(other, std::is_trivial<tElement>());
	}

	cArray& operator=(const cArray& other)
	{
		clear();
		CopyImpl(other, std::is_trivial<tElement>());
		return *this;
	}
private:

	template <typename tIsTrivialType>
	void CopyImpl(const cArray& other, tIsTrivialType)
	{
		for(const auto& item: other)
		{
			emplace_back(item);
		}
	}

	
	inline tElement* Array() 
	{
		return reinterpret_cast<tElement*>(mArray);
	}

	inline const tElement* Array() const
	{
		return reinterpret_cast<const tElement*>(mArray);
	}

	enum {
		kArraySize = 
		((kCapacity*sizeof(tElement)+sizeof(cStackStorage)-1)/sizeof(cStackStorage))
	};
	cStackStorage mArray[kArraySize];

	uint32_t mSize;
};

template<uint32_t tNum, class tElement>
// tElement is trivial
void CopyImpl(const cArray<tNum, tElement>& other, tTrue)
{
	mSize = other.mSize;
	memcpy(mArray, other.mArray, sizeof(tElement)*mSize);
}

}
