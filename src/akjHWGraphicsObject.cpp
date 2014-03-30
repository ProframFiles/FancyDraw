#include "akjHWGraphicsObject.hpp"
#include "akjOGL.hpp"

namespace akj
{

	cHWGraphicsObject::cHWGraphicsObject(cHWGraphicsContext* context, 
																				const Twine& object_name)
		: mObjectID(GL_INVALID_VALUE)
		, mObjectName(object_name.str())
		, mParentContext(context)
	{
		AKJ_ASSERT(context != NULL);
		//Log::Debug("Creating graphics object \"%s\"", mObjectName.c_str());
	}

	cHWGraphicsObject::~cHWGraphicsObject()
	{
		//Log::Debug("Destroying graphics object \"%s\"", mObjectName.c_str());
	}


}