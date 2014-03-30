#pragma once
#include <stdint.h>

namespace akj
{
class cWrappingCounter
{
public:
	enum {kMaxCount = 0x7FFFFFFF};
	cWrappingCounter() :mNumber(0){}
	cWrappingCounter(uint32_t num) :mNumber(num){}
	cWrappingCounter(const cWrappingCounter& num) 
		:mNumber(num.mNumber){}
	~cWrappingCounter(){};
	operator uint32_t() const { return mNumber;}
	explicit operator float() const {
	 return static_cast<float>(mNumber);
	}
	cWrappingCounter operator ++()
	{ 
		mNumber = (mNumber+1);
		return *this;
	}
	cWrappingCounter operator +=(uint32_t num)
	{ 
		mNumber +=num;
		return *this;
	}
	cWrappingCounter operator ++(int)
	{ 
		cWrappingCounter temp(mNumber);
		mNumber = (mNumber+1);
		return temp;
	}
	bool operator <(const cWrappingCounter& other) const
	{
		return (other.mNumber != mNumber) 
				&& (other.mNumber - mNumber) < (kMaxCount);
	}
	bool operator >(const cWrappingCounter& other) const
	{
		return (other.mNumber != mNumber) 
				&& (mNumber - other.mNumber) < (kMaxCount);
	}
private:
	uint32_t mNumber;
};
}
