#include "akjStopWatch.hpp"

#if (defined _WIN32 || defined WIN32)
	//windows.h, why must you be such a pain? 
	#ifndef NOMINMAX
		#define NOMINMAX
		#define STOPWATCH_DEFINED_NOMINMAX
	#endif
	#include <windows.h>
	#ifdef STOPWATCH_DEFINED_NOMINMAX
		#undef NOMINMAX
	#endif

#elif (defined __iOS__ || __OSX__ )
	//from http://www.macresearch.org/tutorial_performance_and_time
	#include <mach/mach_time.h>
#else
	#include <time.h>
	#include <sys/time.h>
#endif


namespace akj {



	uint64_t cStopWatch::QueryTimer() const
	{
#if (defined _WIN32 || defined WIN32)
		LARGE_INTEGER time_start;
		QueryPerformanceCounter(&time_start);
		return static_cast<uint64_t>(time_start.QuadPart);
#elif (defined __iOS__ || defined __OSX__ )
		return static_cast<uint64_t>(mach_absolute_time());
#else
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return static_cast<uint64_t>(ts.tv_sec*1000000000LL + ts.tv_nsec);
#endif
	}

	double cStopWatch::QueryPeriod() const
	{
#if (defined _WIN32 || defined WIN32)
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return 1.0/static_cast<double>(frequency.QuadPart);
#elif (defined __iOS__ || defined __OSX__ )
		mach_timebase_info_data_t info;
		kern_return_t err = mach_timebase_info( &info );
		//Convert the timebase into seconds
		assert(err == 0);
		return 1e-9 * static_cast<double>( info.numer) / static_cast<double>( info.denom);
#else
		return 1e-9;
#endif
	}

}