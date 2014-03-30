#pragma once
#include "akjStopWatch.hpp"
#include <algorithm>

namespace akj
{
	class cFrameState
	{
	public:
		cFrameState()
			:mTimer()
			,mTargetFrameRate(60)
			,mCounter(0)
			,mDrawsWithoutSwap(0)
			,mSoothingFactor(0.9)
			,mSwapTime(0.01)
			,mSmoothedFrameTime(1.0/mTargetFrameRate)

		{}
		~cFrameState(){};

		void MarkFrameStart()
		{
			double frame_time = mTimer.Lap();
			if(mCounter > 0)
			{
				mSmoothedFrameTime = mSoothingFactor*mSmoothedFrameTime
					+ (1.0 - mSoothingFactor)*(frame_time);
			}
			mDrawsWithoutSwap = 0;
		}

		uint64_t FrameCount() const
		{
			return mCounter;
		}

		double FrameTimeRemaining()
		{
			return 1000.0*std::max((1.0/mTargetFrameRate) - mTimer.Read(), 0.0);
		}

		void TargetFrameRate(uint32_t rate)
		{
			mTargetFrameRate = rate;
		}

		double SmoothedFrameTime() const
		{
			return mSmoothedFrameTime*1000.0;
		}

		void MarkDraw()
		{
			++mDrawsWithoutSwap; 
		}
		void MarkSwap()
		{
			mSwapTime = mTimer.Read();
			if (mCounter > 0)
			{
				mSmoothedFrameTime = mSoothingFactor*mSmoothedFrameTime
					+ (1.0 - mSoothingFactor)*(mSwapTime);
			}
			mCounter++;
		}
		bool HasDraw() const {return mDrawsWithoutSwap >0 ;}
		uint32_t DrawCount() const {return mDrawsWithoutSwap  ;}
		
	private:
		cStopWatch mTimer;
		uint32_t mTargetFrameRate;
		uint64_t mCounter;
		uint32_t mDrawsWithoutSwap;
		

		// 1.0 = constant, 0.0 = no smoothing
		double mSoothingFactor;
		double mSwapTime;
		double mSmoothedFrameTime;
		double mSmoothedSwapTime;
		

	};
	
}
