#pragma once
#include "StringRef.hpp"
#include "Twine.hpp"
#include <string>


namespace akj
{
	class cHWGraphicsContext;


	class cHWGraphicsObject
	{
	public:
		cHWGraphicsObject(cHWGraphicsContext* context, const Twine& object_name);
		virtual ~cHWGraphicsObject();
		uint32_t GetID() const { return mObjectID; };
		cStringRef GetName() const { return mObjectName; };

	protected:
		
		uint32_t mObjectID;
		std::string mObjectName;
		cHWGraphicsContext* mParentContext;
	
	
	private:
		//no copy
		cHWGraphicsObject(const cHWGraphicsObject& obj){};
		cHWGraphicsObject& operator=(const cHWGraphicsContext& rhs)
		{ return *this; };
	};

	class iDrawCommand
	{
	public:
		virtual ~iDrawCommand(){};
		virtual void Execute() = 0;
	};

}
