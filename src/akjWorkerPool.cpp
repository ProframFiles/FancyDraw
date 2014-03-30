#include "akjWorkerPool.hpp"
#include "akjStopWatch.hpp"

namespace akj
{
	void cWorker::Run()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		while(mShouldRun.load())
		{
			while(mCurrentTask = mAtomicTaskPointer.load())
			{
				mCurrentTask->DoWork(mThreadID);
				if(mCurrentTask->IsDone())
				{
					mAtomicTaskPointer.compare_exchange_strong(mCurrentTask, nullptr);
				}
			}
			
			// no task to run, wait for instruction from the main thread
			
			mRunCondition.wait(lock);

		}
	}

	cWorker::cWorker(uint32_t index)
		: mCurrentTask(NULL)
		, mAtomicTaskPointer(NULL)
		, mThreadID(index)
		, mShouldRun(true)
		, mHasError(false)
		, mThisThread(new std::thread(RunWorker, this))
	{
		
	}

	void cWorker::Reset()
	{
		mCurrentTask = nullptr;
		mAtomicTaskPointer.store(nullptr);
		mHasError.store(false);
		mErrorString.clear();
		if(mThisThread && mThisThread->joinable())
		{
			WaitForShutDown();
		}
		mShouldRun.store(true);
		mThisThread.reset(new std::thread(RunWorker, this));
	}


	void cWorker::SetTask(cTask* task)
	{
		mAtomicTaskPointer.store(task);
		mRunCondition.notify_one();
	}

	void cWorker::StartShutDown()
	{
		mShouldRun.store(false);
		SetTask(nullptr);
	}

	cWorker::~cWorker()
	{
		cStopWatch sw;
		if(mThisThread && mThisThread->joinable())
		{
			WaitForShutDown();
		}
		Log::Debug("Shut down worker thread in %f seconds", sw.Stop());
		if(mHasError.load())
		{
			Log::Error("Worker thread had an error: %s", mErrorString);
		}
	}

	void cWorker::WaitForShutDown()
	{
		StartShutDown();
		// when we get this lock, that means the thread is finished its task chunk
		// and has just started waiting at the condition variable 
		// (or it's done and just waiting to join)
		std::unique_lock<std::mutex> lock(mMutex);
		lock.unlock();
		//	 notify implies that the notified thread will try to grab the lock again
		// so we need to unlock it now
		mRunCondition.notify_one();
		mThisThread->join();
	}



	cWorkerPool::~cWorkerPool()
	{
		for (uint32_t w = 0; w < mWorkers.size() ; ++w)
		{
			mWorkers.at(w).StartShutDown();
		}
		mWorkers.clear();
	}

	akj::tTaskHandle cWorkerPool::AddTask(std::unique_ptr<cTask>&& task)
	{
		UpdateTasks();
		AKJ_ASSERT_AND_THROW(task);
		cTask* task_ptr = task.get();
		tTaskHandle handle(mTaskCounter++);
		
		if(cWorker* avail = AvailableWorker())
		{
			avail->SetTask(task_ptr);
			mRunningTasks[handle] = std::move(task);
		}
		else
		{
			mWaitingTasks[handle] = std::move(task);
		}
		mHandleLookup[task_ptr] = handle;
		if(mOnCreateFunc)
		{
			mOnCreateFunc(handle, task_ptr->Name(), task_ptr->StatusString());
		}
		return handle;
	}

	cWorker* cWorkerPool::AvailableWorker()
	{
		if(!mFreeWorkers.empty())
		{
			uint32_t avail = mFreeWorkers.back();
			mFreeWorkers.pop_back();
			return mWorkers.data() + avail;
		}
		else if(mWorkers.size() < mWorkers.capacity() 
						&& mWorkers.size() < std::thread::hardware_concurrency())
		{
			uint32_t avail = mWorkers.size();
			mWorkers.emplace_back(avail);
			mBusyWorkers.push_back(avail);
			return mWorkers.data() + avail;
		}
		// otherwise it'll just have to wait
		return nullptr;
	}

	void cWorkerPool::UpdateTasks()
	{

		for(uint32_t i=0; i < mBusyWorkers.size(); ++i)
		{
			uint32_t worker_index = mBusyWorkers[i];
			if(mWorkers[worker_index].HasError())
			{
				Log::Error("Thread %d died with: %s. Restarting.", worker_index,
										mWorkers[worker_index].GetError());
				cTask* err_task = mWorkers[worker_index].GetErrorTask();
				mWorkers[worker_index].Reset();
				mBusyWorkers.SwapAndPop(i);
				mFreeWorkers.push_back(worker_index);
				AbortTask(mHandleLookup.at(err_task));
			}
			else if(!mWorkers[worker_index].HasWork())
			{
				mBusyWorkers.SwapAndPop(i);
				mFreeWorkers.push_back(worker_index);
			}
			else
			{
				//pump the condition variable to make sure no one is waiting
				mWorkers[worker_index].WakeUp();
			}
		}
		//iterate over not-done tasks, move them to finished if they are
		auto running = mRunningTasks.begin();
		while(running != mRunningTasks.end())
		{
			const tTaskHandle handle = running->first;
			if(running->second->IsDone())
			{
				
				running->second->WhenFinished(handle);
				if(mOnCompleteFunc)
				{
					mOnCompleteFunc(handle);
				}
				auto hand_it = mHandleLookup.find(running->second.get());
				mRunningTasks.erase(running++);
				mHandleLookup.erase(hand_it);
			}
			else
			{
				float progress = 0.0f;
				cTask& task = *(running->second);
				if(mOnProgressFunc && task.ProgressIfAny(progress))
				{
					Log::Debug("Progress Happened! %f", progress);
					mOnProgressFunc(handle, task.StatusString(), progress);
				}
				++running;
			}
		}
		
		// assign a worker (if there are any) to the waiting tasks
		auto waiting = mWaitingTasks.begin();
		cWorker* avail = nullptr;
		while(waiting != mWaitingTasks.end() && (avail = AvailableWorker()))
		{
			avail->SetTask(waiting->second.get());
			mRunningTasks[waiting->first] = std::move(waiting->second);
			mWaitingTasks.erase(waiting++);
		}
	}

	void cWorkerPool::AbortTask(tTaskHandle handle)
	{
		auto found = mWaitingTasks.find(handle);
		if(found == mWaitingTasks.end())
		{
			//we're releasing a running task
			auto found_again = mRunningTasks.find(handle);

			// it had better be there...
			AKJ_ASSERT_AND_THROW(found_again != mRunningTasks.end());
			auto hand_it = mHandleLookup.find(found_again->second.get());
			AKJ_ASSERT_AND_THROW(hand_it != mHandleLookup.end());
			ClearWorkersWithTask(found_again->second.get());
			mRunningTasks.erase(found_again);
			mHandleLookup.erase(hand_it);
		}
		else
		{
			auto hand_it = mHandleLookup.find(found->second.get());
			AKJ_ASSERT_AND_THROW(hand_it != mHandleLookup.end());
			mWaitingTasks.erase(found);
			mHandleLookup.erase(hand_it);
		}
		if(mOnCompleteFunc)
		{
			mOnCompleteFunc(handle);
		}
	}

	void cWorkerPool::ClearWorkersWithTask(cTask* task)
	{
		for(uint32_t i=0; i < mBusyWorkers.size(); ++i)
		{
			uint32_t worker_index = mBusyWorkers[i];
			if(task == mWorkers[worker_index].AssignedTask())
			{
				mWorkers[worker_index].SetTask(nullptr);
				mBusyWorkers.SwapAndPop(i);
				mFreeWorkers.push_back(worker_index);
			}
		}
	}

} // namespace akj