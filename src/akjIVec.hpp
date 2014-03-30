#pragma once
#include "akjFixedSizeVector.hpp"
#include <algorithm>
#include "RawOstream.hpp"

namespace akj
{
	struct ivec2
	{
		ivec2(int in_x, int in_y)
			:x(in_x), y(in_y)
		{}
		ivec2()
		{}
		ivec2 operator *(ivec2 other) const
		{
			return ivec2(other.x*x, other.y*y);
		}
		ivec2 operator +(ivec2 other) const
		{
			return ivec2(other.x+x, other.y+y);
		}
		ivec2 operator +(int other) const
		{
			return ivec2(other+x, other+y);
		}
		ivec2 operator -(ivec2 other) const
		{
			return ivec2(x-other.x, y-other.y);
		}
		ivec2 operator >>(uint32_t val) const
		{
			return ivec2(x>>val, y>>val);
		}
		ivec2 operator <<(uint32_t val) const
		{
			return ivec2(x<<val, y<<val);
		}
		ivec2 operator -() const
		{
			return ivec2(-x, -y);
		}
		ivec2& operator *=(ivec2 other)
		{
			x *= other.x;
			y *= other.y;
			return *this;
		}
		ivec2& operator >>=(uint32_t val)
		{
			x >>= val;
			y >>= val;
			return *this;
		}
		ivec2& operator +=(ivec2 other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}
		ivec2 RoundedDownToMultiple(uint32_t num)
		{
			AKJ_ASSERT(num > 0);
			return ivec2((x/num)*num,(y/num)*num);
		}
		ivec2 RoundedUpToMultiple(uint32_t num)
		{
			AKJ_ASSERT(num > 0);
			return ivec2(((x+num-1)/num)*num,((y+num-1)/num)*num);
		}
		inline ivec2 abs() const 
		{
			return ivec2(std::abs(x), std::abs(y));
		}
		inline ivec2 sign() const
		{
			return ivec2( x > 0 ? 1 : (x == 0 ? 0 : -1 ) ,
										 y > 0 ? 1 : (y == 0 ? 0 : -1 ));
		}
		union{
			int x;
			int width;
		};
		union{
			int y;
			int height;
		};
	};

}
