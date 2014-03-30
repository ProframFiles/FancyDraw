#pragma once
#include "akjWorkerPool.hpp"
#include "akjInput.hpp"
#include "akjBitField.hpp"

namespace akj
{
class cApplicationObject
{
public:
	enum eListenerType
	{
		kMouseListener,
		kTaskListener,
		kTimeListener,
		kLastListener
	};
	enum eListenToBits
	{
		kMouseListenBit = 1 << kMouseListener,
		kTaskListenBit = 1 << kTaskListener,
		kTimeListenBit = 1 << kTimeListener,
		kLastListenBit = 1 << kLastListener
	};



	cApplicationObject(const cWrappingCounter& object_count)
		:mUniqueID(object_count)
	{}
	virtual ~cApplicationObject(){};

	cWrappingCounter UniqueID() const
	{
		return mUniqueID;
	}

	bool operator ==(const cApplicationObject& other) const 
	{
		return mUniqueID == other.mUniqueID;
	}

	bool operator <(const cApplicationObject& other) const
	{
		return mUniqueID < other.mUniqueID;
	}

	//////////////////////////////////////////////////////////////////////////
	// Event requests
	//////////////////////////////////////////////////////////////////////////

	virtual cBitField<eListenToBits> ListensTo() const { return cBitField<eListenToBits>(); } 

	//////////////////////////////////////////////////////////////////////////
	// Object Lifetime
	//////////////////////////////////////////////////////////////////////////

	virtual bool HasLimitedLifetime() const { return false; }
	virtual bool IsDead() const { return false; }

	//////////////////////////////////////////////////////////////////////////
	// Task management
	//////////////////////////////////////////////////////////////////////////
	virtual void 
		OnTaskCreated(tTaskHandle handle, cStringRef task_name, cStringRef status)
		{};
	virtual void 
		OnTaskUpdate(tTaskHandle handle, cStringRef status, float progress){};
	virtual void 
		OnTaskDestroyed(tTaskHandle handle){};

	//////////////////////////////////////////////////////////////////////////
	// Timing
	//////////////////////////////////////////////////////////////////////////

	virtual void
		OnTimeElapsed(double elapsed, double absolute){};

	//////////////////////////////////////////////////////////////////////////
	// Mouse Movement
	//////////////////////////////////////////////////////////////////////////

	virtual void
		OnMouseMovement(const cMouseState& mouse){};

private:
// no copy or assignment
	cApplicationObject(const cApplicationObject& other){}
	cApplicationObject& operator=(const cApplicationObject& other){return *this;}
	cWrappingCounter mUniqueID;
};


}

//hash table support
namespace std
{
    template<>
    struct hash<akj::cApplicationObject>
    {
        typedef akj::cApplicationObject argument_type;
        typedef uint32_t value_type;
 
        value_type operator()( const argument_type& obj) const
        {
           return obj.UniqueID().operator uint32_t();
        }
    };
}

