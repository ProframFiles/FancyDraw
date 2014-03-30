#pragma once
#include "akjLayeredDrawable.hpp"
#include "Allocator.hpp"

namespace akj
{
	struct uRange 
	{
		uRange(uint32_t start_in, uint32_t size_in)
			:start(start_in), size(size_in) {}
		uRange()
			:start(0), size(0) {}
		uint32_t start;
		uint32_t size;
		uRange Next() const { return {start+size, 0}; }
	};


	class cDrawCommander
	{
	public:
		cDrawCommander(){
			mSections.emplace_back(0, 0);
		}
		~cDrawCommander(){
			Deallocate();
		}

		void Clear()
		{
			Deallocate();
			mSections.clear();
			mSections.emplace_back(0, 0);
		}

	
		void PlayBackAll()
		{
			PlayBack(0, u32(mCommands.size()));
		}

		void PlayBackSection(uint32_t section_index)
		{
			uRange section = mSections.at(section_index);
			PlayBack(section.start, section.size);
		}

		void PlayBack(uint32_t start, uint32_t num = 1)
		{
			AKJ_ASSERT(num+start <= u32(mCommands.size()));
			for (uint32_t i = start; i < u32(start+num) ; ++i)
			{
				mCommands.at(i)->Execute();
			}
		}

		void NewSection()
		{
			mSections.emplace_back(mSections.back().Next());
		}

		template <typename tCommand>
		tCommand* Record()
		{
			void* mem = mAllocator.Allocate<tCommand>();
			tCommand* ptr = new(mem) tCommand;
			mCommands.push_back(ptr);
			++mSections.back().size;
			return ptr;
		}

		template <typename tCommand, typename A>
		tCommand* Record(A&& a)
		{
			void* mem = mAllocator.Allocate<tCommand>();
			tCommand* ptr = new(mem) tCommand(std::forward<A>(a));
			mCommands.push_back(ptr);
			++mSections.back().size;
			return ptr;
		}

		template <typename tCommand, typename A, typename B>
		tCommand* Record(A&& a, B&& b)
		{
			void* mem = mAllocator.Allocate<tCommand>();
			tCommand* ptr = new(mem) tCommand(
				std::forward<A>(a), std::forward<B>(b));
			mCommands.push_back(ptr);
			++mSections.back().size;
			return ptr;
		}

		template <typename tCommand, typename A, typename B, typename C>
		tCommand* Record(A&& a, B&& b, C&& c)
		{
			void* mem = mAllocator.Allocate<tCommand>();
			tCommand* ptr = new(mem) tCommand(
				std::forward<A>(a), std::forward<B>(b), std::forward<C>(c));
			mCommands.push_back(ptr);
			++mSections.back().size;
			return ptr;
		}

		template <typename tCommand, typename A, typename B, typename C, typename D>
		tCommand* Record(A&& a, B&& b, C&& c, D&& d)
		{
			void* mem = mAllocator.Allocate<tCommand>();
			tCommand* ptr = new(mem) tCommand(
				std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d));
			mCommands.push_back(ptr);
			++mSections.back().size;
			return ptr;
		}


	private:
		void Deallocate()
		{
			// the allocator doesn't call the destructor, so we should probably do so
			// (even if it doesn't do anything right now)
			while (!mCommands.empty())
			{
				iDrawCommand* ptr = mCommands.back();
				mCommands.pop_back();
				ptr->~iDrawCommand();
			}
			mAllocator.Reset();
		}


		std::vector<iDrawCommand*> mCommands;
		std::vector<uRange> mSections;
		BumpPtrAllocator mAllocator;
	};
}
