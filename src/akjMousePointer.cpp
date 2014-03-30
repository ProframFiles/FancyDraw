#include "akjMousePointer.hpp"
#include "akjIOUtil.hpp"
#include "akjOGL.hpp"
#include "resources/akjStaticResources.hpp"
#include "akjHWGraphicsContext.hpp"

namespace akj
{
	cMousePointer::cMousePointer(cHWGraphicsContext* context)
		:mStaticVerts(context, "Mouse pointer verts",
									24*sizeof(float), STATIC_VBO)
		,mVAO(context, "Mouse Pointer VAO")
		,mTexture(context, "Mouse Pointer texture")
		,mShader(context, "Mouse pointer shader")
		,mContext(*context)
		,mIsHidden(false)
	{
		mShader.CreateShaderProgram(cStaticResources::get_MousePointer_glsl());

		cAlignedBuffer image_buf;
		cAlignedBitmap image = 
			LoadImageFromMemory(cStaticResources::get_svord_png(), image_buf);
		SetCursor(image);
		

		mVAO.Bind();
		mStaticVerts.Bind();
		uint32_t byte_stride = 16;
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, 0, byte_stride, AKJ_FLOAT_OFFSET(0));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, 0, byte_stride, AKJ_FLOAT_OFFSET(2));
		mVAO.UnBind();
		mStaticVerts.UnBind();
	}

	void cMousePointer::SetCursor(cAlignedBitmap image)
	{
		mImageSize.x = image.W();
		mImageSize.y = image.H();
		float x = static_cast<float>(mImageSize.x);
		float y = static_cast<float>(mImageSize.y);
		mTexture.CreateTexture2D(image);
		float verts[24] =
		{
			  0.0f,  y  , 0.0f, 1.0f // bottom left
			, 0.0f, 0.0f, 0.0f, 0.0f // top left
			,  x  , 0.0f, 1.0f, 0.0f // top right
			, 0.0f,  y  , 0.0f, 1.0f // bottom left
			,  x  , 0.0f, 1.0f, 0.0f // top right
			,  x  ,  y  , 1.0f, 1.0f // bottom right
		};
		mStaticVerts.SetData(sizeof(verts), verts, 0);
	}

	void cMousePointer::Draw(cCoord2 mousepos, double time)
	{
		if(mIsHidden)
		{
			return;
		}
		ivec2 vp = mContext.ViewPort();
		float proj[16];
		cHWGraphicsContext::MyOrtho2D(proj,
			0.0f, static_cast<float>(vp.x),
			static_cast<float>(vp.y), 0.0f);

		mShader.Bind();
		mShader.BindUniformToInt("uTexture0", mTexture.GetBoundTextureUnit());
		mShader.BindProjectionMatrix(&proj[0]);
		mShader.BindUniformToFloat("uCurrentTime", static_cast<float>(time));
		mShader.BindUniformToVec2("uMousePosition",mousepos.x, mousepos.y);
		mVAO.Bind();
		mStaticVerts.DrawAsTriangles(0, 6);
		mVAO.UnBind();
	}

} // namespace akj