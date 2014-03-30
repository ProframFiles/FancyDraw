#pragma once
#include <atomic>
#include <thread>
#include <unordered_map>
#include <map>
#include <mutex>
#include <condition_variable>
#include "akjFixedSizeVector.hpp"
#include "akjTask.hpp"
#include "akjUintHandle.hpp"

namespace akj
{
  class cTask;

	typedef std::function<void(tTaskHandle, cStringRef, float)> 
		tTaskProgressFunc;
	typedef std::function<void(tTaskHandle, cStringRef ,cStringRef)> 
		tTaskCreatedFunc;
	typedef std::function<void(tTaskHandle)> tTaskDestroyedFunc;

	class cWorker
	{
	public:
		cWorker(uint32_t thread_index);
		~cWorker();
		void Run();
		void StartShutDown();
		void WaitForShutDown();

		void SetTask(cTask* task);
		const cTask* AssignedTask() const
		{
			return mAtomicTaskPointer.load();
		}

		void SetError(const Twine& text)
		{
			mErrorString = text.str();
			mHasError.store(true);
		}

		bool HasError() const
		{
			return mHasError.load();
		}

		cStringRef GetError() const
		{
			return mErrorString;
		}

		cTask* GetErrorTask() const
		{
			return mCurrentTask;
		}

		void Reset();

		static void RunWorker(cWorker* worker)
		{			
			try{
				worker->Run();
			}
			catch(const Exception& e)
			{
				worker->SetError(Twine("died with an akj exception:") + e.what());
			}
			catch(const std::exception& e)
			{
				worker->SetError(Twine("died with an std exception:") + e.what());
			}
		}

		bool HasWork() const
		{
			return (nullptr != mAtomicTaskPointer.load());
		}

		void WakeUp()
		{
			mRunCondition.notify_one();
		}

	private:
		cTask* mCurrentTask;
		std::string mErrorString;
		std::atomic<bool> mHasError;

		// shared state
		
		std::atomic<cTask*> mAtomicTaskPointer;
		std::mutex mMutex;
		std::condition_variable mRunCondition;
		std::atomic<bool> mShouldRun;
		tTaskHandle mErrorHandle;
		
		const uint32_t mThreadID;
		std::unique_ptr<std::thread> mThisThread;
	};



	class cWorkerPool
	{
	enum {kMaxThreads = 16};
	public:
		cWorkerPool(){};
		~cWorkerPool();
		tTaskHandle AddTask(std::unique_ptr<cTask>&& task);

		void AbortTask(tTaskHandle handle);
		
		void UpdateTasks();

		template <typename tFunctor>
		void SetCreationHandler(tFunctor&& func)
		{
			mOnCreateFunc = std::forward<tFunctor>(func);
		}

		template <typename tFunctor>
		void SetDestructionHandler(tFunctor&& func)
		{
			mOnCompleteFunc = std::forward<tFunctor>(func);
		}

		template <typename tFunctor>
		void SetProgressHandler(tFunctor&& func)
		{
			mOnProgressFunc = std::forward<tFunctor>(func);
		}

	private:

		void ClearWorkersWithTask(cTask* task);
		cWorker* AvailableWorker();
		cWrappingCounter mTaskCounter;
		cArray<kMaxThreads, cWorker> mWorkers;	
		cArray<kMaxThreads, uint32_t> mBusyWorkers;	
		cArray<kMaxThreads, uint32_t> mFreeWorkers;
		std::unordered_map<tTaskHandle, std::unique_ptr<cTask>> mRunningTasks;
		std::unordered_map<const cTask*, tTaskHandle> mHandleLookup;
		std::map<tTaskHandle, std::unique_ptr<cTask>> mWaitingTasks;

		tTaskCreatedFunc mOnCreateFunc;
		tTaskProgressFunc mOnProgressFunc;
		tTaskCompleteFunc mOnCompleteFunc;


	};

}
