#pragma once
#include <cmath>
#include "akjIVec.hpp"

namespace akj
{

	struct cCoord2
	{
		typedef float value_type;
		enum {kNumElements = 2};
		cCoord2() :x(0.0f), y(0.0f){};
		cCoord2(float in) :x(in), y(in){};
		cCoord2(float in_x, float in_y) :x(in_x), y(in_y){};
		cCoord2(uint32_t in_x, uint32_t in_y) 
			:x(static_cast<float>(in_x)), y(static_cast<float>(in_y)){};
		cCoord2(int32_t in_x, int32_t in_y) 
			:x(static_cast<float>(in_x)), y(static_cast<float>(in_y)){};
		explicit cCoord2(const ivec2& int_vec)
			:x(static_cast<float>(int_vec.x)), y(static_cast<float>(int_vec.y)){};
		
		inline cCoord2 yx() const
		{
			return cCoord2(y, x);
		}

		inline cCoord2& operator +=( const cCoord2& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}
		inline cCoord2& operator +=(float val)
		{
			x += val;
			y += val;
			return *this;
		}
		inline cCoord2& operator *=( const cCoord2& rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}
		inline cCoord2& operator *=(float val)
		{
			x *= val;
			y *= val;
			return *this;
		}
		inline cCoord2& operator -=(const cCoord2& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}
		inline cCoord2& operator -=(float val)
		{
			x -= val;
			y -= val;
			return *this;
		}
		inline float Length()
		{
			return std::sqrt(x*x+y*y);
		}
		inline cCoord2 ceil() const
		{
			return cCoord2(std::ceil(x), std::ceil(y));
		}
		inline cCoord2 floor() const
		{
			return cCoord2(std::floor(x), std::floor(y));
		}
		inline cCoord2 abs() const 
		{
			return cCoord2(std::abs(x), std::abs(y));
		}
		inline cCoord2 sign() const
		{
			return cCoord2(std::copysign(1.0f, x), std::copysign(1.0f, y));
		}
		inline ivec2 ivec() const
		{
			return ivec2(static_cast<int>(x), static_cast<int>(y));
		}

		float x;
		float y;
	};
	inline cCoord2 operator +(const cCoord2& lhs, const cCoord2& rhs)
	{
		return cCoord2(lhs.x + rhs.x, lhs.y + rhs.y);
	}
	inline cCoord2 operator -(const cCoord2& lhs, const cCoord2& rhs)
	{
		return cCoord2(lhs.x - rhs.x, lhs.y - rhs.y);
	}
	inline cCoord2 operator *(const cCoord2& lhs, const cCoord2& rhs)
	{
		return cCoord2(lhs.x * rhs.x, lhs.y * rhs.y);
	}
	inline cCoord2 operator *(const cCoord2& lhs, float val)
	{
		return cCoord2(lhs.x * val, lhs.y * val);
	}
	inline cCoord2 operator *(float val, const cCoord2& rhs)
	{
		return cCoord2(rhs.x * val, rhs.y * val);
	}
	inline cCoord2 Sign(const cCoord2& lhs)
	{

	}
}
