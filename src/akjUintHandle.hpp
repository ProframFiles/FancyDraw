#pragma once
#include "akjWrappingCounter.hpp"


namespace akj
{
	// The template parameter does nothing:
	// it is only there to uniquify the class definition
	template <typename tHandleTo>
	struct cHandle
	{
		typedef tHandleTo tHandled;
		enum { kInvalidHandle = 0xFFFFFFFF };
		cHandle():id(kInvalidHandle){}
		cHandle(const cHandle& in_handle): id(in_handle.id){}
		explicit cHandle(uint32_t in_id):id(in_id){}
		bool operator == (uint32_t rhs) const {return rhs == id;};
		operator uint32_t() const {return id;}
		bool IsValid() const { return id != kInvalidHandle; }
		uint32_t id;
	};

	// The template parameter does nothing:
	// it is only there to uniquify the class definition
	template <typename tHandleTo>
	struct cOrderedHandle
	{
		typedef tHandleTo tHandled;
		enum { kInvalidHandle = 0xFFFFFFFF };
		cOrderedHandle():id(kInvalidHandle){}
		cOrderedHandle(const cOrderedHandle& in_handle):id(in_handle.id){}
		explicit cOrderedHandle(uint32_t in_id):id(in_id){}
		bool operator == (uint32_t rhs) const 
		{
			AKJ_ASSERT(IsValid());
			return id == rhs;
		};
		bool operator <(const cOrderedHandle& other) const
		{
			AKJ_ASSERT(IsValid());
			return  id < other.id;
		}
		bool operator >(const cOrderedHandle& other) const
		{
			AKJ_ASSERT(IsValid());
			return  id > other.id;
		}
		operator uint32_t() const 
		{
			AKJ_ASSERT(IsValid());
			return static_cast<uint32_t>(id);
		}
		bool IsValid() const { return id != kInvalidHandle; }
		cWrappingCounter id;
	};

}
namespace std 
{
	template<class tHandled> struct hash<akj::cOrderedHandle<tHandled>>
	{
		size_t operator()(akj::cOrderedHandle<tHandled> handle)
		{
			return handle;
		}
	};
	template<class tHandled> struct hash<akj::cHandle<tHandled>>
	{
		size_t operator()(akj::cHandle<tHandled> handle)
		{
			return handle;
		}
	};
}
