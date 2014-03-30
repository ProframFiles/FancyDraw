#pragma once
#include <time.h>
#include "FancyDrawMath.hpp"
#include <limits>

namespace akj
{
	class cRandom
	{
	public:
		cRandom()
		{
			time_t seed = time(0);
			mX = static_cast<uint32_t>(seed);
			mW = static_cast<uint32_t>(seed << 1);
			mZ = static_cast<uint32_t>(seed << 2);
			mY = static_cast<uint32_t>(seed << 3);
			UInt();
			UInt();
			UInt();
			UInt();
		}
		uint32_t UInt()
		{
			uint t = (mX ^ (mX << 11));
			mX = mY;
			mY = mZ;
			mZ = mW;
			return (mW = (mW ^ (mW >> 19)) ^ (t ^ (t >> 8)));
		}

		uint32_t UInt(uint32_t max)
		{
			return static_cast<uint32_t>(Double()*(max + 1));
		}

		uint32_t UIntOther(uint32_t max)
		{
			return static_cast<uint32_t>(Double()*(max + 1));
		}

		//returns a float between 0 and 1, but never exactly 0 or 1
		double Double()
		{
			//  2.32830643545449e-10 == 1/(2^32+2)
			return (static_cast<double>(UInt())+1)*2.32830643545449e-10;
		}

		//returns a float between 0 and 1, but never exactly 0 or 1
		float Float()
		{

			return NormalizeToFloat(UInt());
		}

		float FloatOther()
		{

			float num =  ((UInt())*2.3283064370808e-10f);
			num *= 1.0f - 2*std::numeric_limits<float>::epsilon();
			return num + std::numeric_limits<float>::epsilon();
		}

		static uint32_t MaxUInt()
		{
			return 0xFFFFFFFF;
		}

		static float NormalizeToFloat(uint32_t num)
		{
			// here we take all the significant bits of UInt that are significant
			// 7FFFFe == (2^23)-2 == 11111111111111111111110
			// 1.192093-07f ~= 1/(2^23+2)
			// with an exponent of 150 (127 corresponds to zero, so 150 is 23)
			// a floating point number is 2^23 + 23 lower bits as an int
			// we set the last bit so that it's always above 0 by a bit
			// 
			const float conv = unsafe::UintBitsToFloat((((num & 0x7FFFFF)) | 0x4b000001));
			return (conv - 8388608.0f) * 1.19209289550781e-7f;
		}
	private:
		
		uint32_t mX;
		uint32_t mY;
		uint32_t mZ;
		uint32_t mW;
	};
}
