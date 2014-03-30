#pragma once

#include <stdint.h>
#include <cassert>
#include "akjExceptional.hpp"
#include "akjDataPointer.hpp"
#include <unordered_map>

namespace akj{


	template <unsigned int kByteAlignment>
	class AlignedBuffer
	{
	public:

		enum {kAlignment = kByteAlignment,
					kEndTagVal = 0x55575555};
		AlignedBuffer()
		: mAlignedArray(0)
		, mSize(0)
		, mAllocatedArray(0)
		{}

		explicit AlignedBuffer(size_t num_bytes)
		: mAlignedArray(0)
		, mSize(0)
		, mAllocatedArray(0)
		{
			Allocate(num_bytes);
		}

		AlignedBuffer(AlignedBuffer<kByteAlignment>&& other)
			:mAlignedArray(other.mAlignedArray)
			,mAllocatedArray(other.mAllocatedArray)
			,mSize(other.mSize)
		{
			other.mAlignedArray = 0;
			other.mAllocatedArray = 0;
			other.mSize = 0;
		}

		AlignedBuffer& operator=(AlignedBuffer<kByteAlignment>&& other)
		{
			Deallocate();
			mAlignedArray = other.mAlignedArray;
			mAllocatedArray = other.mAllocatedArray;
			mSize = other.mSize;
			other.mSize = 0;
			other.mAlignedArray = nullptr;
			other.mAllocatedArray = nullptr;
			return *this;
		}


		~AlignedBuffer()
		{
			Deallocate();
		}

		template<typename T>
		T* const ptr() const
		{
			assert(mSize == 0 || sizeof(T) <= mSize);
			return reinterpret_cast<T*>(mAlignedArray);
		}
		template<typename T>
		T* ptr()
		{
			assert(mSize == 0 || sizeof(T) <= mSize);
			return reinterpret_cast<T*>(mAlignedArray);
		}

		const cDataPtr data() const
		{
			return cDataPtr(ptr<uint8_t>());
		}

		cDataPtr data()
		{
			return cDataPtr(ptr<uint8_t>());
		}

		size_t size() const 
		{
			return mSize;
		}
		void reset(size_t num_bytes){
			//need more space
			if(num_bytes > size())
			{
				Allocate(num_bytes);
				return;
			}
			// we have a buffer already, and it has enough space
			else if(size() > 0)
			{
				//shrinking the buffer
				mSize = num_bytes;
				*reinterpret_cast<int*>(mAlignedArray+mSize) = kEndTagVal;
				return;
			}
			// if we don't need more space and don't need a buffer, then
			// we must have had 0 bytes requested
			AKJ_ASSERT(num_bytes == 0);
		}
		
		void Deallocate(){
			if(mAllocatedArray)
			{
				if(*reinterpret_cast<int*>(mAlignedArray+mSize) != kEndTagVal)
				{
					Log::Error("Wrote past the end of the buffer in an Aligned buffer!");
					AKJ_ASSERT(false);
				}
				delete[] mAllocatedArray;
				mAllocatedArray = NULL;
				mSize = 0;
			}
		}

		bool BoundsIntact()
		{
			return *reinterpret_cast<int*>(mAlignedArray+mSize) == kEndTagVal;
		}

		template <typename T>
		void fill_value(T val)
		{
			size_t prims = (size()+sizeof(T)-1)/sizeof(T);
			assert(kByteAlignment >= sizeof(T) && kByteAlignment%sizeof(T) == 0);
			T* array = ptr<T>();
			for (size_t i = 0; i < prims; i++)
			{
				array[i] = val;
			}
		}

		template <typename tWriter>
		void Serialize(tWriter& sout) const
		{
			sout.Write(*this);
		}

	private:

		void Allocate(size_t num_bytes)
		{
			Deallocate();
			if(num_bytes > 0){
				//allocate with some padding
				const size_t to_allocate =
					((num_bytes+4+kByteAlignment-1)/kByteAlignment+1)*kByteAlignment;
				mAllocatedArray = new uint8_t[to_allocate];
				//shift the pointer if required
				const uintptr_t unalignment = reinterpret_cast<uintptr_t>
					(mAllocatedArray) % kByteAlignment;
				if(unalignment != 0)
				{
					uintptr_t offset = kByteAlignment - unalignment;
					mAlignedArray = reinterpret_cast<uint8_t*>
						(reinterpret_cast<uintptr_t>(mAllocatedArray)+offset);
				}
				else
				{
					mAlignedArray = mAllocatedArray;
				}
				mSize = num_bytes;
				// end tag
				*reinterpret_cast<int*>(mAlignedArray+mSize) = kEndTagVal;
			}
		}
		//no copy
		AlignedBuffer(const AlignedBuffer<kByteAlignment>& ab){}
		AlignedBuffer& operator=(const AlignedBuffer<kByteAlignment>& ab)
		{return *this;}
		size_t mSize;
		uint8_t *mAlignedArray;
		uint8_t *mAllocatedArray;
	};

	typedef AlignedBuffer<16> cAlignedBuffer;

	}
	namespace std
	{
		template<uint32_t kAlign> struct hash<akj::AlignedBuffer<kAlign>>
		{
			size_t operator()(const akj::AlignedBuffer& buffer)
			{
				return static_cast<size_t>(buffer.data().AsUInt());
			}
		};
	}

	namespace akj {

	class cBufferPool
	{
	public:
		cBufferPool(){};
		~cBufferPool(){};

		cDataPtr Allocate(size_t num_bytes)
		{
			cAlignedBuffer buf(num_bytes);
			void* ptr = buf.data();
			auto result = mBuffers.insert(std::make_pair(ptr, std::move(buf)));
			return result.first->second.data();
		}

		cAlignedBuffer* Find(const void* ptr)
		{
			auto found = mBuffers.find(ptr);
			if(found != mBuffers.end())
			{
				return &(found->second);
			}
			return nullptr;
		}

		void DeAllocate(void* ptr)
		{
			if(ptr == nullptr) return;
			auto found = mBuffers.find(ptr);
			AKJ_ASSERT(found != mBuffers.end());
			mBuffers.erase(found);
		}

		static void* BufferPoolMalloc(cBufferPool* pool, size_t bytes)
		{
			return pool->Allocate(bytes);
		}

		static void BufferPoolFree(cBufferPool* pool, void* ptr)
		{
			pool->DeAllocate(ptr);
		}

	private:
		std::unordered_map<const void*, akj::cAlignedBuffer> mBuffers;
	};


}

