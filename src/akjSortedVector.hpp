#pragma once
#include <vector>
#include <array>
#include <algorithm>

namespace akj
{
	template <typename T>
	class cSortedVectorSet
	{
	public:
		cSortedVectorSet(size_t initial_capacity)
			: mFirstUnsorted(0)
			, mLastUnsorted(0)
		{}
		~cSortedVectorSet();

		inline std::vector<T>::const_iterator begin() const
		{
			Sort();
			return mVector.begin();
		}

		inline std::vector<T>::const_iterator end() const
		{
			return mVector.end();
		}

		bool empty() const
		{
			return mTempItems.empty() && mVector.empty();
		}

		size_t size() const
		{
			return mTempItems.size() + mVector.size();
		}

		bool RemoveSingle(const T& val)
		{
			if ( !RemoveFromTemp(val) )
			{
				if(RemoveFromMainVector())
				{
					Sort();
					return true; //found in the main vector
				}
				return false; // found in neither
			}
			return true; // found in the temp vector
		}

		template <class tIterable>
		bool RemoveAll(const tIterable& container)
		{
			bool has_sorted_removal = false;
			for(const T& val : container)
			{
				if(!RemoveFromTemp(val))
				{
					has_sorted_removal |= RemoveFromMainVector(val);
				}
			}

			if(has_sorted_removal)
			{
				Sort();
			}
		}

		bool Insert(const T& val)
		{
			T* found = FindImpl(val);
			if(found)
			{
				return false;
			}
			// ok, lets insert it!
			if(TempStoreIsFull())
			{
				mVector.emplace_back(val);
				Sort();
			}
			else
			{
				mTempItems.emplace_back(val);
			}
		}


	private:

		bool RemoveFromTemp(const T& val)
		{
			T* found = FindInTempItems();
			if(found)
			{
				size_t index = found - mTempItems.data();
				mTempItems.at(index) = mTempItems.back();
				mTempItems.pop_back();
				return true;
			}
			return false;
		}



		T* FindInTempItems(const T& val)
		{
			auto found = std::find(mTempItems.begin(), mTempItems.end(), val);
			if (found != mTempItems.end())
			{
				return &(*found);
			}
			return
		}

		bool RemoveFromMainVector(const T& val)
		{
			T* found = FindInMainVector();
			if (found)
			{
				size_t index = found - mVector.data();
				mVector.at(index) = mVector.back();
				mVector.pop_back();
				return true;
			}
			return false;
		}

		T* FindInMainVector(const T& val)
		{
			auto found = std::binary_search(mVector.begin(), mVector.end(), val);
			if (found != mVector.end())
			{
				return &(*found);
			}
			return NULL;
		}

		T* FindImpl(const T& val)
		{
			T* found = FindInTempItems();
			if(!found)
			{
				found = FindInMainVector();
			}
			return found;
		}

		void MergeTempArray()
		{
			if(mVector.capacity()-mVector.size() < mTempItems.size())
			{
				mVector.reserve(mVector.capacity() * 2);
			}

			for (size_t i = 0; i < mTempItems.size() ; ++i)
			{
				mVector.emplace_back(mTempItems.back());
				mTempItems.pop_back();
			}
		}

		bool TempStoreIsFull() const
		{
			return mTempItems.size() < mTempItems.capacity();
		}

		bool IsSorted const 
		{
			return mTempItems.empty();
		}

		void Sort()
		{
			if( !IsSorted() )
			{
				MergeTempArray();
				std::vector<T>::iterator first = mVector.begin() + mFirstUnsorted;
				std::vector<T>::iterator last = mVector.begin() + mLastUnsorted;
				std::sort(first, last);
				mFirstUnsorted = mLastUnsorted = mVector.size();
			}
		}

		std::vector<T> mVector;
		std::array<T, 8> mTempItems;
		size_t mFirstUnsorted;
		size_t mLastUnsorted;
	};
}
