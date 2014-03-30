#pragma once
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>



namespace akj
{
	//////////////////////////////////////////////////////////////////////////
	// A poor mans memory pool for POD objects
	//////////////////////////////////////////////////////////////////////////

	template <typename T>
	class cIndexedStorage
	{
	public:
		cIndexedStorage(uint32_t init_capacity = 1024)
			:mFreeList()
		{
			mData.reserve(init_capacity);
			mData.reserve(init_capacity);
		}
		~cIndexedStorage(){}

		T& operator[](uint32_t index)
		{
			return mData[index];
		}

		const T& operator[](uint32_t index) const
		{
			return mData[index];
		}

		typename std::vector<T>::iterator begin()
		{
			return mData.begin();
		}

		typename std::vector<T>::iterator end()
		{
			return mData.end();
		}

		void erase(uint32_t id)
		{
			if(id == static_cast<uint32_t>(mData.size()))
			{
				mData.pop_back();
			}
			else
			{
				memset(mData.data()+id, 0, sizeof(T));
				mFreeList.push(id);
			}
		}

		uint32_t size()
		{
			return static_cast<uint32_t>(mData.size() - mFreeList.size());
		}

		T& New()
		{
			if(mFreeList.empty())
			{
				uint32_t new_index = static_cast<uint32_t>(mData.size());
				mData.emplace_back();
				ZeroMem(mData.at(new_index));
				mData.back().mID = new_index;
				return mData.back();
			}
			uint32_t new_index = mFreeList.top();
			mFreeList.pop();
			ZeroMem(mData.at(new_index));
			mData.at(new_index).mID = new_index;
			return mData[new_index];
		}

	private:
		std::vector<T> mData;
		std::priority_queue<
			uint32_t, 
			std::vector<uint32_t>, 
			std::greater<uint32_t> 
		> mFreeList;

	};


}
