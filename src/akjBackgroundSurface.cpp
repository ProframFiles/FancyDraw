#include "akjBackgroundSurface.hpp"
#include "resources/akjStaticResources.hpp"
#include "akjIOUtil.hpp"
#include "akjHWGraphicsContext.hpp"

namespace akj
{

	cBackgroundSurface
	::cBackgroundSurface(cHWGraphicsContext* context, const Twine& name)
		:cHWGraphicsObject(context, name)
		,mVAO(context, "Background Surface VAO", 
										{cGLBufferDesc(STATIC_VBO, 3*sizeof(cCoord2))})
	{
		mVertexStore.emplace_back(-3.0f, -1.0f);
		mVertexStore.emplace_back(1.0f, 3.0f);
		mVertexStore.emplace_back(1.0f, -1.0f);

		mVAO.Buffer(0).ResetData(sizeof(cCoord2)*mVertexStore.size());
		mVAO.Buffer(0).SetData(sizeof(cCoord2)*mVertexStore.size(),
									mVertexStore.data(), 0);
		mVAO.SetAttributes({cVertexAttribute(ATTR_FLOAT, 2, 0, 0, 0)});
	}

	void cBackgroundSurface::Bind()
	{
		mVAO.Bind();
	}

	void cBackgroundSurface::UnBind()
	{
		mVAO.UnBind();
	}

	cBackgroundSurface::~cBackgroundSurface()
	{

	}

	void cBackgroundSurface::Draw()
	{
		mVAO.DrawAsTriangles(0, 3);
		mVAO.UnBind();
	}

} // namespace akj