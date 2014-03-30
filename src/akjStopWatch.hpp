#pragma once
#include "akj_typedefs.hpp"
#include <cassert>
#include "FancyDrawMath.hpp"

namespace akj {
	class cStopWatch
	{
	public:
		cStopWatch()
		{
			mPeriod = QueryPeriod();
			Start();
		}
		~cStopWatch(){};

		void Start()
		{
			mStartTicks = QueryTimer();
			mNowTicks = mStartTicks;
		}

		double Lap()
		{
			mNowTicks=QueryTimer();
			double now = mPeriod*(mNowTicks - mStartTicks);
			mStartTicks = mNowTicks;
			return now;
		}

		double Stop()
		{
			mNowTicks=QueryTimer();
			return Read();
		}
		uint64_t GetTicks() const
		{
			return QueryTimer();
		}
		double Read() const
		{
			if(mNowTicks != mStartTicks)
			{
				return mPeriod*(mNowTicks - mStartTicks);
			}
			return mPeriod*(QueryTimer() - mStartTicks);
		}
	private:
		uint64_t QueryTimer() const;
		double QueryPeriod() const;

		uint64_t mStartTicks;
		uint64_t mNowTicks;
		double mPeriod;
	};

	template <typename T, uint32_t kNumElements>
	struct WrappedArray{
		T& at(uint32_t index)
		{
			return m_data[index];
		}
		const T& at(uint32_t index) const
		{
			return m_data[index];
		}
		T m_data[kNumElements];
	};


	template <typename tCategories>
		class TimingStats{
			struct SavedTime{
				SavedTime()
				:num_records(0)
				{}
		
				int num_records;
				float min_time;
				float max_time;
				float total_time;
			};

			typedef WrappedArray<SavedTime, tCategories::kMaxIndex> tTimeList;
			typedef WrappedArray<cStopWatch, tCategories::kMaxIndex> tTimersList;
			typedef typename tCategories::tIndex tIndex;
			public:
		
			TimingStats()
				:mTimeList()
				,mTimer()
			{}

			void StartTime(tIndex index)
			{
				mTimer.Start();
			}
			void StopTime(tIndex index)
			{
				double last_time = mTimer.Stop();
				AddTime(index, last_time);
			}

			uint32_t GetNumRecords(tIndex index) const
			{
				return mTimeList.at(index).num_records;
			}

			float GetTotalTime(tIndex index) const 
			{
				if(mTimeList.at(index).num_records == 0){
					return -0.0f;
				}
				return mTimeList.at(index).total_time;
			}

			float GetMaxTime(tIndex index) const 
			{
				if(mTimeList.at(index).num_records == 0){
					return -0.0f;
				}
				return mTimeList.at(index).max_time;
			}

			float GetMinTime(tIndex index) const 
			{
				if(mTimeList.at(index).num_records == 0){
					return -0.0f;
				}
				return mTimeList.at(index).min_time;
			}

			void MergeStats(const TimingStats<tCategories>& other)
			{
				for (uint32_t i = 0; i < tCategories::kMaxIndex; i++)
				{
					SavedTime& t = mTimeList.at(i);
					const SavedTime& other_t = other.GetStats(i);
					if(t.num_records == 0){
						t = other_t;
					}
					else{
						t.num_records += other_t.num_records;
						t.min_time = LesserOf(t.min_time, other_t.min_time);
						t.max_time = GreaterOf(t.max_time,other_t.max_time);
						t.total_time += other_t.total_time;
					}
				}	
			}

		private:
			const SavedTime& GetStats(uint32_t index) const
			{
				return mTimeList.at(index);
			}
			void AddTime(tIndex index, double double_time){
				SavedTime& t = mTimeList.at(index);
				const float new_time = static_cast<float>(double_time);
				if(t.num_records == 0){
					t.num_records = 1;
					t.min_time = new_time;
					t.max_time = new_time;
					t.total_time = 0.0f;
				}
				else{
					t.num_records ++;
					t.min_time = LesserOf(t.min_time, new_time);
					t.max_time = GreaterOf(t.max_time, new_time);
				}
				t.total_time += new_time;
			}
			tTimeList mTimeList;
			cStopWatch mTimer;
		};

} // akj

