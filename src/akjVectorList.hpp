#pragma once
#include <vector>
#include "akj_typedefs.hpp"

namespace akj
{
	struct cListNode
	{
		uint32_t mIndex;
		uint32_t mNext;
		uint32_t mPrev;
	};

	class cVectorList;

	struct cVectorListIterator
	{
	public:
		cVectorListIterator& operator++();
		cVectorListIterator& operator--();
		bool operator==(const cVectorListIterator& other)
		{
			return mCurrentIndex == other.mCurrentIndex;
		}
		bool operator!=(const cVectorListIterator& other)
		{
			return !(mCurrentIndex == other.mCurrentIndex);
		}
		uint32_t operator*();
		cVectorListIterator& erase();

	private:
		enum { kInvalidIndex = 0xFFFFFFFF };
		friend class cVectorList;
		cVectorListIterator(cVectorList& list, uint32_t starting_index)
			:mParentVector(list)
			,mCurrentIndex(starting_index)
		{}

		cVectorList& mParentVector;
		uint32_t mCurrentIndex;
		
	};

	class cVectorList
	{
	public:
		enum { kInvalidIndex = cVectorListIterator::kInvalidIndex };
		cVectorList()
			:mTailIndex(kInvalidIndex)
			,mHeadIndex(kInvalidIndex)
		{};
		~cVectorList(){};
		size_t size() const { return mNodeVector.size(); }
		bool empty() const { return mNodeVector.empty(); }

		cVectorList& push_back(uint32_t index)
		{
			mNodeVector.emplace_back();
			cListNode& new_node = mNodeVector.back();
			new_node.mIndex = index;
			if(size() == 1)
			{
				mHeadIndex = 0;
				mTailIndex = 0;
				new_node.mNext = kInvalidIndex;
				new_node.mPrev = kInvalidIndex;
			}
			else
			{

				mNodeVector.at(mTailIndex).mNext = static_cast<uint32_t>(size()) - 1;
				new_node.mPrev = mTailIndex;
				mTailIndex = static_cast<uint32_t>(size()) - 1;
			}
			return *this;
		}

		uint32_t pop_back()
		{
			uint32_t ret = mNodeVector.at(mTailIndex).mIndex;
			RemoveAt(mTailIndex);
			return ret;
		}
		uint32_t pop_front()
		{
			uint32_t ret = mNodeVector.at(mHeadIndex).mIndex;
			RemoveAt(mHeadIndex);
			return ret;
		}

		cVectorListIterator begin()
		{
			return cVectorListIterator(*this, mHeadIndex);
		}
		cVectorListIterator last()
		{
			return cVectorListIterator(*this, mTailIndex);
		}
		cVectorListIterator end()
		{
			return cVectorListIterator(*this, kInvalidIndex);
		}

	private:
		uint32_t RemoveAt(uint32_t i)
		{
			uint32_t next_index = kInvalidIndex;
			cListNode dead_node = mNodeVector.at(i);
			if(i == mHeadIndex)
			{
				// new head
				mHeadIndex = dead_node.mNext;
				next_index = mHeadIndex;
				if(mHeadIndex != kInvalidIndex)
				{
					// new head has no prev.
					mNodeVector.at(mHeadIndex).mPrev = kInvalidIndex;
				}
			}
			else if(i == mTailIndex)
			{
				//new tail
				mTailIndex = dead_node.mPrev;
				next_index = mTailIndex;
				if(mTailIndex != kInvalidIndex)
				{
					mNodeVector.at(mTailIndex).mNext = kInvalidIndex;
				}
			}
			else if(size() > 1)
			{
				cListNode& prev = mNodeVector.at(dead_node.mPrev);
				cListNode& next = mNodeVector.at(dead_node.mNext);
				prev.mNext = dead_node.mNext;
				next.mPrev = dead_node.mPrev;
				next_index = dead_node.mNext;
			}
			MoveBackInto(i);
			
			return next_index;
		}
		void MoveBackInto(uint32_t i)
		{
			if (size() == 1)
			{
				mNodeVector.pop_back();
				mHeadIndex = kInvalidIndex;
				mTailIndex = kInvalidIndex;
				return;
			}

			// ok, we have items
			if(mHeadIndex == size()-1)
			{
				// the back node is the head
				mHeadIndex = i;
			}
			else if(mTailIndex == size()-1)
			{
				// the back node is the tail
				mTailIndex = i;
			}
			if(size() > 1)
			{
				mNodeVector.at(i) = mNodeVector.back();
				const uint32_t prev = mNodeVector.at(i).mPrev;
				const uint32_t next = mNodeVector.at(i).mNext;
				if (prev != kInvalidIndex)
				{
					mNodeVector.at(prev).mNext = i;
				}
				if (next != kInvalidIndex)
				{
					mNodeVector.at(next).mPrev = i;
				}
			}
			mNodeVector.pop_back();
		}

		void SetHeadIndex(uint32_t index)
		{
			mHeadIndex = index;
		}
		void SetTailIndex(uint32_t index)
		{
			mTailIndex = index;
		}
		cListNode& at(uint32_t index){ return mNodeVector.at(index); }
		cListNode& operator[](uint32_t index){ return mNodeVector[index]; }
		friend struct cVectorListIterator;
		std::vector<cListNode> mNodeVector;
		uint32_t mTailIndex;
		uint32_t mHeadIndex;
	};

	inline cVectorListIterator& cVectorListIterator::operator++()
	{
		mCurrentIndex = mParentVector.at(mCurrentIndex).mNext;
		return *this;
	}

	inline cVectorListIterator& cVectorListIterator::operator--()
	{
		if(mCurrentIndex == cVectorList::kInvalidIndex)
		{
			mCurrentIndex = mParentVector.mTailIndex;
		}
		mCurrentIndex = mParentVector.at(mCurrentIndex).mPrev;
		return *this;
	}

	inline uint32_t cVectorListIterator::operator*()
	{
		return mParentVector.at(mCurrentIndex).mIndex;
	}

	inline cVectorListIterator& cVectorListIterator::erase()
	{
		if (mCurrentIndex != kInvalidIndex)
		{
			mCurrentIndex = mParentVector.RemoveAt(mCurrentIndex);
		}
		return *this;
	}

}
