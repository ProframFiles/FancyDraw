#pragma once
#include "akjHWGraphicsObject.hpp"
#include "akjVertexArrayObject.hpp"
#include "akjTextureObject.hpp"
#include "akjArrayBuffer.hpp"
#include "akjShaderObject.hpp"
#include "akjDoublePBO.hpp"
#include "akjMovingVertex2D.hpp"
#include "akjFixedSizeVector.hpp"


namespace akj
{
	class cBackgroundSurface : public cHWGraphicsObject
	{
	public:
		cBackgroundSurface(cHWGraphicsContext* context, const Twine& name);
		~cBackgroundSurface();

		virtual void Bind();
		void Draw();
		void UnBind();
	
	private:
		cArray<3, cCoord2> mVertexStore;
		cVertexArrayObject mVAO;
	};
}
