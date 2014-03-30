#pragma once
#include "akjIvec.hpp"
#include "akjIRect.hpp"
#include <array>
#include <vector>
#include <stdint.h>
#include "akjFixedSizeVector.hpp"


namespace akj
{
//////////////////////////////////////////////////////////////////////////
// Essentially a quadtree that can only hold binary coverage information
// for rectangles.
// This is for occlusion purposes, and leads to very noticable performance
// gains past a few occluded layers. It becomes worth the cost of doing
// CPU based occlusion vs. ignoring it and just drawing everything in shaders 
// I haven't figured out a good way to do z-buffer based occlusion while
// maintaining shader-based anti-aliased edges on the shapes, hence the CPU
// based solution
// 
// Ballpark numbers on a 3.5ghz i7: 0.5ms to add and filter the drawn subset 
// of 5000 ~1/4 screen sized rectangles (around 200 get drawn). 
// this is vs. 10ms to draw all of these on an NV 660
//
// The "sparse" refers to the fact that it lazily populates the leaf nodes
// and merges fully covered neighbor nodes to keep the tree shallow 
//////////////////////////////////////////////////////////////////////////
class cSparseCoverageMap
{
	enum
	{
		TOP_LEFT=0,
		TOP_RIGHT,
		BOTTOM_RIGHT,
		BOTTOM_LEFT
	};

	enum eNodeKind
	{
		kLeafKind,
		kBranchKind,
		kRectRefKind,
		kRectListKind,
	};

	struct cNode
	{
		union{
			iRect rect;
			uint32_t children[4];
		};
		eNodeKind kind;
		void MakeBranch()
		{
			kind = kBranchKind;
			children[0] = 0;
			children[1] = 0;
			children[2] = 0;
			children[3] = 0;
		}
	};

	enum
	{
		kNoCoverage = 0,
		kFullCoverage = 1,
		kHeadNode = 2
	};

public:
	cSparseCoverageMap(ivec2 bounds)
		:mNumRects(0)
		,mWidth(bounds.width)
		,mHeight(bounds.height)
		,mWidth2(NextPowerOfTwo(bounds.width))
		,mHeight2(NextPowerOfTwo(bounds.height))
	{
		mMainRect.left = 0;
		mMainRect.top = 0;
		mMainRect.width = bounds.width;
		mMainRect.height = bounds.height;
		mDirectionTable[0] = ivec2(1, 0);
		mDirectionTable[1] = ivec2(0, 1);
		mDirectionTable[2] = ivec2(-1, 0);
		mDirectionTable[3] = ivec2(0, -1);
		mNodes.reserve(1024);
		//mRectStore.reserve(1024);
		Clear();
	}

	int AddRect(iRect rect)
	{
		rect.Intersect(mMainRect);
		iRect node_rect;
		node_rect.left = 0;
		node_rect.top = 0;
		node_rect.width = mWidth2;
		node_rect.height = mHeight2;
		if (!node_rect.HasOverlap(rect))
		{
			return 0;
		}
		mNumRects++;
		int ret = AddRectImpl(rect,node_rect, kHeadNode);
		return ret;
	}

	bool IsFullyOccluded(iRect rect)
	{
		rect.Intersect(mMainRect);
		iRect node_rect;
		node_rect.left = 0;
		node_rect.top = 0;
		node_rect.width = mWidth2;
		node_rect.height = mHeight2;
		if (!node_rect.HasOverlap(rect))
		{
			return false;
		}
		bool ret = IsFullyOccludedImpl(rect, node_rect, kHeadNode);
		return ret;
	}

	bool HasIntersection(iRect rect)
	{
		rect.Intersect(mMainRect);
		iRect node_rect;
		node_rect.left = 0;
		node_rect.top = 0;
		node_rect.width = mWidth2;
		node_rect.height = mHeight2;
		if (!node_rect.HasOverlap(rect))
		{
			return false;
		}
		bool ret = HasIntersectionImpl(rect, node_rect, kHeadNode);
		return ret;
	}

	void Clear()
	{
	//	Log::Debug("Cleared %d nodes (for %d rects)",
	//		mNodes.size() - mFreeList.size()-2, mNumRects);
	//	mRectStore.clear();
		mNodes.resize(2);
		mFreeList.clear();
		NewNode();
		mNodes.at(kHeadNode).MakeBranch();
		mNumRects = 0;
	}

	void Reset(ivec2 bounds)
	{
		Reset(bounds.width, bounds.height);
	}

	void Reset(int w, int h)
	{
		mWidth = w;
		mHeight = h;
		mWidth2 = NextPowerOfTwo(w);
		mHeight2 = NextPowerOfTwo(h);
		mMainRect.left = 0;
		mMainRect.top = 0;
		mMainRect.width = w;
		mMainRect.height = h;
		Clear();
	}

private:
	uint32_t NextPowerOfTwo(uint32_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return v+1;
	}

	int AddRectImpl(const iRect& rect, iRect node_rect, uint32_t index)
	{
		if(rect.width == 0 || rect.height ==0)
		{
			return 0;
		}
		if(mNodes[index].kind == kLeafKind)
		{
			if(mNodes[index].rect.Contains(rect))
			{
				return 0;
			}
			else if( mNodes[index].rect.width == 0 
							|| mNodes[index].rect.height == 0
							|| rect.Contains(mNodes[index].rect))
			{
				mNodes[index].rect = rect;
				if(rect.height!=566)
				{
					bool breaker = true;
				}
				return 0;
			}
			else
			{
				// we have some kind of non-trivial union of two rectangles in this
				// node: we need to branch
				iRect old_rect = mNodes[index].rect;
				mNodes[index].MakeBranch();
				AddRectImpl(old_rect, node_rect, index);
				AddRectImpl(rect, node_rect, index);
				return 0;
			}
		}
			
		node_rect.width = node_rect.width == 1 ? 1 : node_rect.width >> 1;
		node_rect.height = node_rect.height == 1 ? 1 : node_rect.height >> 1;
		int result = 0;

		if (mNodes[index].children[TOP_LEFT] != 1)
		{
			ProcessBranch(rect.Intersected(node_rect),
										node_rect, index, TOP_LEFT);
		}
		node_rect.left += node_rect.width;

		if (mNodes[index].children[TOP_RIGHT] != 1)
		{
				ProcessBranch(rect.Intersected(node_rect),
				node_rect, index, TOP_RIGHT);
		}

		node_rect.top += node_rect.height;
			
		if (mNodes[index].children[BOTTOM_RIGHT] != 1)
		{
				ProcessBranch(rect.Intersected(node_rect),
				node_rect, index, BOTTOM_RIGHT);
		}

		node_rect.left -= node_rect.width;
			
		if (mNodes[index].children[BOTTOM_LEFT] != 1)
		{
			ProcessBranch(rect.Intersected(node_rect),
				node_rect, index, BOTTOM_LEFT);
		}

		// merge nodes if possible
		if (mNodes[index].children[TOP_LEFT] == 1
			&& mNodes[index].children[TOP_RIGHT] == 1
			&& mNodes[index].children[BOTTOM_RIGHT] == 1
			&& mNodes[index].children[BOTTOM_LEFT] == 1)
		{
			//Log::Debug("could have merged node %d", index);
			return 1;
		}
		return 0;
	}

	int ProcessBranch(const iRect &new_rect,const iRect& node_rect,
										const uint32_t index, const uint32_t i)
	{
		if (new_rect.width == 0 || new_rect.height == 0)
		{
			// no coverage in this quadrant
			return 0;
		}
		else if (new_rect == node_rect)
		{
			// full coverage, we can erase this node
			FreeNode(mNodes[index].children[i]);
			mNodes[index].children[i] = 1;
			return 0;
		}
		else
		{
			if (mNodes[index].children[i] == 0)
			{
				// no existing coverage
				const uint32_t ni = NewNode();
				mNodes[index].children[i] = ni;
				mNodes[ni].kind = kLeafKind;
				mNodes[ni].rect = new_rect;
			}
			else
			{
				// something more complicated
				if(AddRectImpl(new_rect, node_rect, mNodes[index].children[i]))
				{
					FreeNode(mNodes[index].children[i]);
					mNodes[index].children[i] = 1;
				}
			}
		}
		return 0;
	}

	bool HasIntersectionImpl(const iRect& rect, iRect node_rect, uint32_t index)
	{
		if(mNodes[index].kind == kLeafKind)
		{
			return mNodes[index].rect.HasOverlap(rect.Intersected(node_rect));
		}
		cNode& node = mNodes[index];
		node_rect.width = node_rect.width == 1 ? 1 : node_rect.width >> 1;
		node_rect.height = node_rect.height == 1 ? 1 : node_rect.height >> 1;
		for (uint32_t i = 0; i < 4; ++i)
		{	
			if(node.children[i] != 0 && node_rect.HasOverlap(rect))
			{
				if(node.children[i] == 1)
				{
					return true;
				}
				else if(HasIntersectionImpl(rect, node_rect, node.children[i]))
				{
				 return true;
				}
			}
			node_rect +=
				ivec2(node_rect.width, node_rect.height)*mDirectionTable[i];
		}
		return false;
	}
	bool IsFullyOccludedImpl(const iRect& rect, iRect node_rect, uint32_t index)
	{
		if(mNodes[index].kind == kLeafKind)
		{
			return mNodes[index].rect.Contains(rect.Intersected(node_rect));
		}
		cNode& node = mNodes[index];
		node_rect.width = node_rect.width == 1 ? 1 : node_rect.width >> 1;
		node_rect.height = node_rect.height == 1 ? 1 : node_rect.height >> 1;
		for (uint32_t i = 0; i < 4; ++i)
		{
			if (node.children[i] != 1 
				&& node_rect.HasOverlap(rect)
				&& ( node.children[i] == 0
						|| !IsFullyOccludedImpl(rect, node_rect, node.children[i])))
			{
				return false;
			}
			node_rect +=
				ivec2(node_rect.width, node_rect.height)*mDirectionTable[i];
		}
		return true;
	}

	int NewNode()
	{
		uint32_t new_index = 0;
		if(!mFreeList.empty())
		{
			new_index = mFreeList.back();
			mFreeList.pop_back();
		}
		else
		{
			mNodes.emplace_back();
			new_index = static_cast<uint32_t>(mNodes.size()-1);
		}
		cNode& new_node = mNodes.at(new_index);
		new_node.kind = kLeafKind;
		new_node.rect.top = 0;
		new_node.rect.left = 0;
		new_node.rect.width = 0;
		new_node.rect.height = 0;
		return new_index;
	}

	void FreeNode(uint32_t index)
	{
		if(index == kFullCoverage || index == kNoCoverage)
		{
			return;
		}
		mFreeList.push_back(index);
		if(mNodes[index].kind == kBranchKind)
		{
			FreeNode(mNodes[index].children[0]);
			FreeNode(mNodes[index].children[1]);
			FreeNode(mNodes[index].children[2]);
			FreeNode(mNodes[index].children[3]);
		}
	}
	std::vector<cNode> mNodes;
	//std::vector<iRect> mRectStore;
	std::vector<uint32_t> mFreeList;
	ivec2 mDirectionTable[4];
	iRect mMainRect;
	int mNumRects;
	int mWidth;
	int mHeight;
	int mWidth2;
	int mHeight2;
};

// a generic quadtree implementation
template <typename T>
class cPixelTree
{
public:
	// the pixel grid is a 0 based array with the origin at the top left
	struct cNode
	{
		T val;
		iRect rect;
	};
private:
	std::vector<cNode> mNodes;
		
};
}
