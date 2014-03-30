#pragma once
#include "xxhash.h"

namespace akj
{
	class Hash32
	{
	public:
		explicit Hash32(cStringRef bytes)
			:mHashVal(XXH32(bytes.data(), (int)bytes.size(),0))
		{}
		explicit Hash32(cStringRef bytes, uint32_t seed)
			:mHashVal(XXH32(bytes.data(), (int)bytes.size(),0))
		{}
		Hash32& AddData(cStringRef bytes)
		{
			mHashVal = XXH32(bytes.data(), (int)bytes.size(), mHashVal);
			return *this;
		}
		operator uint32_t()
		{
			return mHashVal;
		}
	private:
		uint32_t mHashVal;
	};
}
