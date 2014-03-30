#pragma once
#include <functional>
#include "akjUintHandle.hpp"

namespace akj
{
	class cTask;

	typedef cOrderedHandle<cTask> tTaskHandle;
	
	typedef std::function<void(tTaskHandle)> tTaskCompleteFunc;

	class cTask
	{
	public:
		cTask(const Twine& task_name)
			: mTaskName(task_name.str())
			, mLastReportedProgress(0.0f)
		{};
		virtual ~cTask(){};

		virtual uint32_t MaxConcurrency(){return 1;}

		cStringRef Name() const
		{
			return mTaskName;
		}

		virtual void DoWork(uint32_t thread_number = 0){};

		virtual float Progress() const {return 0.0f;};

		virtual cStringRef StatusString() const {return cStringRef();};

		virtual bool IsDone() const {return Progress() >= 1.0f;}
		
		bool ProgressIfAny(float& progress)
		{
			float new_progress = Progress();
			if(mLastReportedProgress == new_progress)
			{
				return false;
			}
			mLastReportedProgress = new_progress;
			progress = new_progress;
			return true;
		}

		void WhenFinished(tTaskHandle handle)
		{
			if(mCompletionFunction)
			{
				mCompletionFunction(handle);
			}
		}

		template <class tFunctor>
		void SetCallback(tFunctor func)
		{
			mCompletionFunction = func;
		}

		private:
		std::string mTaskName;
		tTaskCompleteFunc mCompletionFunction;
		float mLastReportedProgress;
	};
}
