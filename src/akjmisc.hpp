#pragma once

#define AKJ_NO_COPY(T) \
  T (const T &o); \
  T &operator = (const T &o)

#ifndef AKJ_STRINGIFY
	#define AKJ_STRINGIFY(thing) AKJ_DO_STRINGIFY(thing)
	#define AKJ_DO_STRINGIFY(thing) #thing
#endif

namespace akj
{
	// I never want to type another static cast to uint as long as I live.
	inline uint32_t u32(size_t in){ return static_cast<uint32_t>(in);}

	template <typename T>
	inline void ZeroMem(T& object){
		memset(&object, 0, sizeof(T));
	}

	// min and max functions, tired of fighting with un-namespaced or macro'd min/max
	template <typename T>
	inline T GreaterOf(const T& x, const T&  y)
	{
		return x < y ? y : x;
	}
	template <typename T>
	inline T LesserOf(const T& x, const T&  y)
	{
		return y < x ? y : x;
	}

	template <typename T>
	inline T Clamp(const T& lower_bound, const T& val, const T& upper_bound)
	{
		return GreaterOf(LesserOf(upper_bound, val), lower_bound);
	}


}
