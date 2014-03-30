#pragma once
#include "akjLog.hpp"

namespace akj
{
	class cMaximizingSearchSeq
	{
		enum {kBadValue = 0xFFFFFFFF};
		enum eSearchState
		{
			kFirstTry,
			kInitialClimb,
			kDescending,
			kAscending,
			kAlmostDone,
			kDone
		};

	public:
		cMaximizingSearchSeq(uint32_t start_val, uint32_t increment)
			: mLastGoodVal(kBadValue)
			, mCurrentVal(start_val)
			, mIncrement(increment)
			, mState(kInitialClimb)
		{}
		
		~cMaximizingSearchSeq(){};
		
		operator uint32_t() const {return mCurrentVal;}

		cMaximizingSearchSeq& operator ++() 
		{
			mLastGoodVal = mCurrentVal;
			switch (mState)
			{
			case kFirstTry:
			case kInitialClimb:
				mCurrentVal += mIncrement;
				mIncrement <<= 1;
				break;
			case kDescending:
				mState = kAscending;
			case kAscending:
				mCurrentVal += mIncrement;
				mIncrement >>= 1;
				if(mIncrement == 0) mState = kAlmostDone;
				break;
			case kAlmostDone:
				mState = kDone;
			case kDone:
			default:
				break;
			}
			return *this;
		}

		uint32_t CurrentIncrement() const {return mIncrement;}

		void IncreaseIfOK( bool result)
		{ 
			if(result)
			{
				operator++();
			}
			else
			{
				operator--();
			}
		}


		uint32_t Result() 
		{
			AKJ_ASSERT_AND_THROW(mLastGoodVal != kBadValue);
			return mLastGoodVal;
		}

		bool IsDone() const {return mState == kDone;}

		cMaximizingSearchSeq& operator --() 
		{
			switch (mState)
			{
			case kFirstTry:
				AKJ_THROW("initial value must be known to be in the range!");
				break;
			case kInitialClimb:
				mIncrement >>= 2;
			case kAscending:
				mState = kDescending;
			case kDescending:
				mCurrentVal -= mIncrement;
				mIncrement >>= 1;
				if(mIncrement == 0) mState = kAlmostDone;
				break;
			case kAlmostDone:
				mCurrentVal = mLastGoodVal;
				mState = kDone;
			case kDone:
			default:
				break;
			}
			return *this;
		}

	private:
		uint32_t mIncrement;
		uint32_t mCurrentVal;
		uint32_t mLastGoodVal;
		eSearchState mState;
	};

	class cMinimizingSearchSeq
	{
	public:
		cMinimizingSearchSeq(int32_t bottom, uint32_t top)
			: mTopBound(top)
			, mBottomBound(bottom)
		{
			--mBottomBound;
		}

		operator int32_t() const 
		{
			return mBottomBound + (CurrentIncrement());
		}
		uint32_t CurrentIncrement() const 
		{
			return static_cast<uint32_t>(mTopBound-mBottomBound)>>1;
		}
		cMinimizingSearchSeq& operator --()
		{
			const uint32_t ci = mTopBound-mBottomBound;
			if(ci <= 3)
			{	
				mTopBound = mBottomBound+(ci>>1);
				mBottomBound = mTopBound;
			}
			else {
				mTopBound = mBottomBound + (CurrentIncrement());
			}
			return *this;
		}
		cMinimizingSearchSeq& operator ++()
		{
			const int ci = mTopBound-mBottomBound;
			if(ci <= 2){
				mBottomBound = mTopBound;
				return *this;
			}
			mBottomBound = mBottomBound + ((CurrentIncrement()));
			return *this;
		}

		bool IsDone() const {return mTopBound == mBottomBound;}

		int mTopBound;
		int mBottomBound;

		template <class tFunctor>
		static uint32_t FindUpperEdge(uint32_t bottom, uint32_t top, tFunctor func)
		{
			cMinimizingSearchSeq seq(bottom, top);
			const cMinimizingSearchSeq& cseq = seq;
			uint32_t searches = 0;
			do 
			{
				uint32_t cv = cseq;
				
				if(func(cseq))
				{
					++seq;
				}
				else
				{
					--seq;
				}
				AKJ_ASSERT(++searches < top-bottom);
			} while (!seq.IsDone());
			//Log::Debug("found %d", cseq);
			return seq;
		}
		template <typename tSomething>
		static bool Test(tSomething not_used)
		{
			uint32_t val = 23;
			uint32_t counts[84] = {};
			bool failed = false;

			auto myfunc = 	[&val, &counts, &failed](uint32_t seq_val)
			{
				++counts[seq_val];
				failed |= (counts[seq_val] > 1);
				return seq_val < val;
			};
			
			uint32_t ret = FindUpperEdge(0, 64, myfunc);
			AKJ_ASSERT(!failed && ret == val);
			for(uint32_t i = 1; i < 64 ; ++i)
			{
				val = i;
				memset(counts, 0, sizeof(counts));
				ret = FindUpperEdge(0, 64, myfunc);
				AKJ_ASSERT(!failed && ret == val);
			}
			for(uint32_t i = 1; i < 64 ; ++i)
			{
				val = i;
				memset(counts, 0, sizeof(counts));
				ret = FindUpperEdge(1, 64, myfunc);
				AKJ_ASSERT(!failed && ret == val);
			}
			for(uint32_t i = 23; i < 83 ; ++i)
			{
				val = i;
				memset(counts, 0, sizeof(counts));
				ret = FindUpperEdge(23, 83, myfunc);
				AKJ_ASSERT(!failed && ret == val);
			}
			return true;
		}

	};


}
