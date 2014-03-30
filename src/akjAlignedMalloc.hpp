#pragma once

#if defined(_MSC_VER)
	#include <malloc.h>
#elif defined(__linux__) || defined(__APPLE__)
	#include <stdlib.h>
#endif

namespace akj{
#if defined(_MSC_VER)
	inline void* aligned_malloc(size_t size_bytes, unsigned int align_bytes)
	{
		return _aligned_malloc(size_bytes, align_bytes);
	}
	inline void aligned_free(void* ptr)
	{
		_aligned_free(ptr);
	}
#elif defined(__linux__) || defined(__APPLE__)
	inline void* aligned_malloc(size_t size_bytes, unsigned int align_bytes)
	{
		void* ret_ptr = NULL;
		const int result =  posix_memalign(&ret_ptr, align_bytes, size_bytes);
		return ret_ptr;
	}

	inline void aligned_free(void* ptr)
	{
		free(ptr);
	}

#endif
} //end akj

