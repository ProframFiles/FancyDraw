#pragma once
#include <cmath>
#include "akjIVec.hpp"
#include "akjCoord2.hpp"

namespace akj
{
		struct fRect
	{
		float left;
		float top;
		float right;
		float bottom;

		float Width() const { return std::abs(right-left); }
		float Height() const { return std::abs(top-bottom); }
	};

struct iRect
{

	const ivec2& pos() const
	{
		return *reinterpret_cast<const ivec2*>(this);
	}

	const ivec2& size() const
	{
		return *reinterpret_cast<const ivec2*>(&width);
	}

	bool Contains(const iRect& other) const
	{
		return width > 0 && height > 0 && other.left >= left
			&& other.Right() <= (width + left)
			&& other.top >= top
			&& other.Bottom() <= (top + height);
	}
		
	bool IntersectsWithVertical(int x) const
	{
		return x >= left && x <= Right();
	}

	bool IntersectsWithHorizontal(int y) const 
	{
		return y >= top && y <= Bottom();
	}

	iRect Intersected(iRect other) const
	{
		return other.Intersect(*this);
	}

	iRect& Intersect(const iRect& other)
	{
		int largest =  std::min(other.Right(), Right());
		left = std::max(other.left, left);
		width = std::max(largest - left, 0);
		largest = std::min(other.Bottom(), Bottom());
		top = std::max(other.top, top);
		height = std::max(largest - top, 0);
		return *this;
	}

	bool HasOverlap(const iRect& other) const
	{
		return width >0 && height >0 && other.width> 0 && other.height >0 
			&& !((other.Right()) <= left
			|| other.left >= Right()
			|| other.Bottom() <= top
			|| other.top >= Bottom());
	}

	iRect& operator +=(ivec2 shift)
	{
		left += shift.x;
		top += shift.y;
		return *this;
	}

	iRect operator +(ivec2 shift) const
	{
		return {left+shift.x, top+shift.y, width, height};
	}

	bool operator ==(const iRect& other) const
	{
		return other.width == width
			&& other.height == height
			&& other.left == left
			&& other.top == top;
	}


	int Right() const
	{
		return left + width;
	}
		
	int Bottom() const
	{
		return top + height;
	}

	iRect& Expand(uint32_t i)
	{
		top -= i;
		left -= i;
		width += 2*i;
		height += 2*i;
		return *this;
	}

	iRect& ExpandToFit(const cCoord2& top_left, const cCoord2& size)
	{
		const cCoord2 size2 = ((top_left+size).ceil()-top_left).ceil();

		return ExpandToFit({	static_cast<int>(top_left.x),
												static_cast<int>(top_left.y),
								static_cast<int>(size2.x),
								static_cast<int>(size2.y)});
		
	}
	iRect& ExpandToFit(const cCoord2& point)
	{
		return ExpandToFit({	static_cast<int>(point.x),
												static_cast<int>(point.y),
								static_cast<int>(std::ceil(point.x)-std::floor(point.x)),
								static_cast<int>(std::ceil(point.y)-std::floor(point.y))});
		
	}
	iRect& ExpandToFit(const iRect& other)
	{
		width += std::max(other.Right() - Right(), 0);
		height += std::max(other.Bottom() - Bottom(), 0);
		int extra = std::max(left-other.left, 0);
		left -= extra;
		width += extra; 
		extra = std::max(top-other.top, 0);
		top -= extra;
		height += extra;
		return *this;
	}

	static iRect ContainingRect(const iRect& a, const iRect& b)
	{
		int left;
		int top;
		return iRect
		{
			left = std::min(a.left, b.left),
			top = std::min(a.top, b.top),
			std::max(a.left + a.width, b.left + b.width) - left,
			std::max(a.Bottom(), b.Bottom()) - top,
		};
	}

	int left;
	int top;
	int width;
	int height;
};
	template <typename tStream>
	inline tStream& operator<<(tStream& out,const iRect& rect)
	{
		out << "Rectangle[{" << rect.left << ", " << rect.top << " }, {";
		out << rect.Right() << ", " << rect.Bottom() << " }]";
		return out;
	}
}
