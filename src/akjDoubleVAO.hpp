#pragma once
#include "akjVertexArrayObject.hpp"

namespace akj
{
	class cDoubleVAO
	{
	public:
		cDoubleVAO( cHWGraphicsContext* context, const Twine& name,	
								std::initializer_list<cGLBufferDesc> desc_list,
								std::initializer_list<cVertexAttribute> attrib_list)
			:mBufferAlpha(context, name +" Alpha", desc_list, attrib_list)
			,mBufferOmega(context, name +" Omega", desc_list, attrib_list)
		{}
		cDoubleVAO( cHWGraphicsContext* context, const Twine& name,	
								std::initializer_list<cGLBufferDesc> desc_list)
			:mBufferAlpha(context, name +" Alpha", desc_list)
			,mBufferOmega(context, name +" Omega", desc_list)
		{}

		~cDoubleVAO(){};

		cVertexArrayObject& Front()
		{
			if(mAlphaIsFront){ return mBufferAlpha; }
			return mBufferOmega;
		}
		cVertexArrayObject& Back()
		{
			if(!mAlphaIsFront){ return mBufferAlpha; }
			return mBufferOmega;
		}
		void Swap(){ mAlphaIsFront = !mAlphaIsFront; }

		void SetAttributes(std::initializer_list<cVertexAttribute> list)
		{
			mBufferAlpha.SetAttributes(list);
			mBufferOmega.SetAttributes(list);
		}

	private:
		cVertexArrayObject mBufferAlpha;
		cVertexArrayObject mBufferOmega;
		bool mAlphaIsFront;
	};
}
