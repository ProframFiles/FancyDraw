#pragma once

namespace akj
{
	template <uint32_t N, class tFunctor>
	void RunNTimes(tFunctor func)
	{
		for (size_t i = 0; i < N ; ++i)
		{
			func();
		}
	}
	template <class tFunctor>
	void RunNTimes(const size_t N, tFunctor func)
	{
		for (size_t i = 0; i < N; ++i)
		{
			func();
		}
	}
	template <class tFunctor>
	void ForN(const size_t N, tFunctor func)
	{
		for (size_t i = 0; i < N; ++i)
		{
			func(i);
		}
	}
}
