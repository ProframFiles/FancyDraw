#pragma once
#include <stdint.h>
#include "akjAlignedMalloc.hpp"

// basically taken from 
//	"The Standard Librarian: What Are Allocators Good For?" @
// http://www.drdobbs.com/the-standard-librarian-what-are-allocato/184403759



namespace akj {


template <class T, uint32_t kAlignment = 16>
class AlignedAllocator
{
public:
	typedef T                 value_type;
	typedef value_type*       pointer;
	typedef const value_type* const_pointer;
	typedef value_type&       reference;
	typedef const value_type& const_reference;
	typedef std::size_t       size_type;
	typedef std::ptrdiff_t    difference_type;
	AlignedAllocator() {}
	AlignedAllocator(const AlignedAllocator&) {}
	~AlignedAllocator() {}
	pointer address(reference ref) const
	{
		return &ref;
	}

	const_pointer address(const_reference ref) const
	{
		return &ref;
	}

	pointer allocate(size_type n, const_pointer = 0)
	{
		void* p = aligned_malloc(n * sizeof(T), kAlignment);
		if(!p)
		{
			throw std::bad_alloc();
		}
		return static_cast<pointer>(p);
	}

	void deallocate(pointer p, size_type)
	{
		aligned_free(p);
	}

	void construct(pointer p, const value_type & x)
	{
		new(p) value_type(x);
	}
	
	void destroy(pointer p)
	{
		p->~value_type(); 
	}

	size_type max_size() const
	{
		return static_cast<size_type>(-1) / sizeof(value_type);
	}

	template <class U>
	AlignedAllocator(const AlignedAllocator<U, kAlignment>&) {}

	template <class U>
	struct rebind 
	{ 
		typedef AlignedAllocator<U, kAlignment> other; 
	};

private:
	void operator=(const AlignedAllocator&);

};

template<uint32_t kAlignment> class AlignedAllocator<void, kAlignment>
{
	typedef void        value_type;
	typedef void*       pointer;
	typedef const void* const_pointer;

	template <class U>
	struct rebind 
	{
		typedef AlignedAllocator<U, kAlignment> other;
	};
};

template <class T, uint32_t kAlignment>
inline bool operator==(const AlignedAllocator<T, kAlignment>&,
                       const AlignedAllocator<T, kAlignment>&)
{
	return true;
}

template <class T, uint32_t kAlignment>
inline bool operator!=(const AlignedAllocator<T, kAlignment>&,
                       const AlignedAllocator<T, kAlignment>&)
{
	return false;
}

}

