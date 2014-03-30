#pragma once
#include "akjApplicationObject.hpp"

namespace akj
{
class cAppObjectFactory : public cApplicationObject
{
public:


	cAppObjectFactory()
		: cApplicationObject(cWrappingCounter())
		, mCounter(1)
		, mListeners(kLastListener)
	{}
	~cAppObjectFactory(){};


	//////////////////////////////////////////////////////////////////////////
	// Overrides
	//////////////////////////////////////////////////////////////////////////
	
	// this listens to everything (but no one cares)
virtual cBitField<eListenToBits> ListensTo() const {
	return cBitField<eListenToBits>()
		.Set(kMouseListenBit)
		.Set(kTaskListenBit)
		.Set(kTimeListenBit); }

virtual void OnMouseMovement(const cMouseState& mouse)
	{
		std::vector<cApplicationObject*>& listeners 
			= mListeners.at(kMouseListener);
		for(size_t i = 0; i < listeners.size(); ++i)
		{
			listeners[i]->OnMouseMovement(mouse);
		}
	}

virtual void 
	OnTimeElapsed(double elapsed, double absolute)
	{
		std::vector<cApplicationObject*>& listeners 
			= mListeners.at(kTimeListener);
		for(size_t i = 0; i < listeners.size(); ++i)
		{
			listeners[i]->OnTimeElapsed(elapsed, absolute);
		}
	};



virtual void 
	OnTaskCreated(tTaskHandle handle, cStringRef task_name, cStringRef status)
	{
		std::vector<cApplicationObject*>& listeners 
			= mListeners.at(kTaskListener);
		for(size_t i = 0; i < listeners.size(); ++i)
		{
			listeners[i]->OnTaskCreated(handle, task_name, status);
		}
	};
virtual void 
	OnTaskUpdate(tTaskHandle handle, cStringRef task_name, float progress)
	{
		Log::Debug("Task update!");
		std::vector<cApplicationObject*>& listeners 
			= mListeners.at(kTaskListener);
		for(size_t i = 0; i < listeners.size(); ++i)
		{
			Log::Debug("Listener #%d!", i);
			listeners[i]->OnTaskUpdate(handle, task_name, progress);
		}
	};
virtual void 
	OnTaskDestroyed(tTaskHandle handle)
	{
		std::vector<cApplicationObject*>& listeners 
			= mListeners.at(kTaskListener);
		for(size_t i = 0; i < listeners.size(); ++i)
		{
			listeners[i]->OnTaskDestroyed(handle);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// Unique methods
	//////////////////////////////////////////////////////////////////////////

	template <class tAppObject>
	tAppObject* CreateAppObject()
	{
		tAppObject* obj = new tAppObject(mCounter++);
		InsertObject(obj);
		return obj;
	}

	template <class tAppObject, typename tArg1>
	tAppObject* CreateAppObject(tArg1&& arg1)
	{
		tAppObject* obj = 
			new tAppObject(mCounter++, std::forward<tArg1>(arg1));
		InsertObject(obj);
		return obj;
	}

	template <class tAppObject, typename tArg1, typename tArg2>
	tAppObject* CreateAppObject(tArg1&& arg1, tArg2&& arg2)
	{
		tAppObject* obj = 
			new tAppObject(mCounter++, std::forward<tArg1>(arg1),
																std::forward<tArg2>(arg2));
		InsertObject(obj);
		return obj;
	}

	template <class tAppObject, typename tArg1, typename tArg2,typename tArg3>
	tAppObject* CreateAppObject(tArg1&& arg1, tArg2&& arg2, tArg3&& arg3)
	{
		tAppObject* obj = 
			new tAppObject(mCounter++, std::forward<tArg1>(arg1),
																std::forward<tArg2>(arg2),
																std::forward<tArg3>(arg3));
		InsertObject(obj);
		return obj;
	}

	template <class tAppObject, typename tArg1, typename tArg2,typename tArg3,
															 typename tArg4, typename tArg5,typename tArg6>
	tAppObject* CreateAppObject(tArg1&& arg1, tArg2&& arg2, tArg3&& arg3, 
															tArg4&& arg4, tArg5&& arg5, tArg6&& arg6)
	{
		tAppObject* obj = 
			new tAppObject(mCounter++, std::forward<tArg1>(arg1),
																std::forward<tArg2>(arg2),
																std::forward<tArg3>(arg3),
																std::forward<tArg4>(arg4),
																std::forward<tArg5>(arg5), 
																std::forward<tArg6>(arg6));
		InsertObject(obj);
		return obj;
	}
		

	void EraseObject(const cApplicationObject* obj)
	{
		auto found = mObjects.find(obj->UniqueID());
		if(found != mObjects.end())
		{
			UnListenAll(obj);
			mObjects.erase(found);
		}
	}

	void PruneDeadObjects()
	{
		for (size_t i = 0; i < mLimitedLifetimeObjs.size() ; ++i)
		{
			const cApplicationObject* obj = mLimitedLifetimeObjs.at(i);
			if(!obj->HasLimitedLifetime())
			{
				mLimitedLifetimeObjs.at(i) = mLimitedLifetimeObjs.back();
				mLimitedLifetimeObjs.pop_back();
			}
			else if(obj->IsDead())
			{
				mLimitedLifetimeObjs.at(i) = mLimitedLifetimeObjs.back();
				mLimitedLifetimeObjs.pop_back();
				EraseObject(obj);
			}
		}
	}

private:
		
	void UnListenAll(const cApplicationObject* obj)
	{
		auto found = mListenerRegistry.find(obj);
		if(found != mListenerRegistry.end())
		{
			const uint32_t flags = found->second;
			uint32_t testing_flag = 1;
			uint32_t current_listener = 0;
			while(flags >0 && current_listener < kLastListener)
			{
				if(flags & testing_flag)
				{
					std::vector<cApplicationObject*>& vec
						= mListeners.at(current_listener);
					uint32_t found_index = 0;
					while(found_index < static_cast<uint32_t>(vec.size()))
					{
						if(vec.at(found_index) == obj)
						{
							vec.at(found_index) = vec.back();
							vec.pop_back();

							break;
						}
						found_index++;
					}
				}
				testing_flag <<= 1;
				++current_listener;
			}
			mListenerRegistry.erase(found);
		}
	}

	void RegisterListener(cApplicationObject* obj, eListenerType listen)
	{
		auto found = mListenerRegistry.find(obj);
		if((found == mListenerRegistry.end() 
			|| (found->second & (1 << listen)) == 0))
		{
			mListeners[listen].push_back(obj);
			if(found == mListenerRegistry.end())
			{
				//Log::Debug("Added as a %d listener", listen);
				mListenerRegistry.insert(std::make_pair(obj, (1 << listen)));
			}
			else
			{
				//Log::Debug("Added as a %d listener", listen);
				found ->second |= (1 << listen);
			}
		}
	}


	//takes ownership of the object
	void InsertObject(cApplicationObject* obj)
	{
		mObjects[obj->UniqueID()] 
		= std::move(std::unique_ptr<cApplicationObject>(obj));

		cBitField<eListenToBits> listens_to = obj->ListensTo();
		for (uint32_t t = 0; t < kLastListener ; ++t)
		{
			if(listens_to.IsSet (static_cast<eListenToBits>(1 << t)))
			{
				//Log::Debug("I'm a listener of %d", t);
				RegisterListener(obj, static_cast<eListenerType>(t));
			}
		}
		if(obj->HasLimitedLifetime())
		{
			//Log::Debug("I have limited lifetime");
			mLimitedLifetimeObjs.push_back(obj);
		}	
	}

	cArray<kLastListener, std::vector<cApplicationObject*>> mListeners;
	std::unordered_map<const cApplicationObject*, uint32_t> mListenerRegistry;

	std::unordered_map<uint32_t, std::unique_ptr<cApplicationObject>> mObjects;
	std::vector<cApplicationObject*> mLimitedLifetimeObjs;
	cWrappingCounter mCounter;
};

}
