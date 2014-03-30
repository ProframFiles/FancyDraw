#pragma once
#include "akjAlignedBuffer.hpp"


namespace akj
{
	// you feed it a list of indices, and it will spit back a list 
	// of those indices in the order received
	class cLinkedIndexList
	{
	public:
		enum {kHeadNode = 0xFFFFFFFF};
		struct cNode
		{
			uint32_t next;
			uint32_t prev;
		};
		struct iterator
		{
			cNode* mNode;
			uint32_t mIndex;
			uint32_t operator*(){return mIndex;}
			iterator& operator++()
			{
				const uint32_t old_i = mIndex;
				mIndex = mNode->next;
				mNode += static_cast<intptr_t>(mIndex - old_i);
			}
			bool operator ==(const iterator& other) {return other.mIndex == mIndex;}
		};

		cLinkedIndexList()
			:mStorage(1024)
			,mCount(0)
		{
			mHeadNode.next = kHeadNode;
			mHeadNode.prev = kHeadNode;
		}

		iterator begin()
		{
			iterator i;
			i.mIndex = Head();
			i.mNode = i.mIndex == kHeadNode? nullptr : &mStorage.ptr<cNode>()[Head()];
		}

		iterator at(uint32_t index)
		{
			iterator i;
			i.mIndex = index;
			i.mNode = index == kHeadNode? nullptr : &mStorage.ptr<cNode>()[index];
		}

		iterator end() const
		{
			iterator i = {nullptr, kHeadNode};
			return i;
		}

		~cLinkedIndexList();
		void RemoveIndex(uint32_t i)
		{
			AKJ_ASSERT(mCount>0);
			cNode old_node = NodeAt(i);
			NodeAt(old_node.next).prev = old_node.prev;
			NodeAt(old_node.prev).next = old_node.next;
			--mCount;
		}

		void AddIndex(uint32_t i)
		{
			if(i*4 >= mStorage.size())
			{
				Grow(i);
			}
			cNode& new_node = mStorage.ptr<cNode>()[i];
			new_node.next = kHeadNode;
			new_node.prev = ChangeTail(i);
			++mCount;
		}

	private:
		cNode& NodeAt(uint32_t i)
		{
			if(i == kHeadNode) return mHeadNode;
			return mStorage.ptr<cNode>()[i];
		}
		uint32_t Head()
		{
			return mHeadNode.next;
		}
		uint32_t Tail()
		{
			return mHeadNode.prev;
		}
		uint32_t ChangeTail(uint32_t new_tail)
		{
			const uint32_t old_tail = mHeadNode.prev;
			mHeadNode.prev = new_tail;
			return old_tail;
		}
		void Grow(uint32_t index)
		{
			uint32_t next_size = static_cast<uint32_t>(mStorage.size()*2);
			while(index*sizeof(cNode) > next_size)
			{
				next_size*=2;
			}
			cAlignedBuffer new_buf(next_size);
			memcpy(new_buf.data(), mStorage.data(), mStorage.size());
			mStorage = std::move(new_buf);
		}

		cAlignedBuffer mStorage;
		cNode mHeadNode;
		uint32_t mCount;
	};
}
