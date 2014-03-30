#include "akjFrameBufferObject.hpp"
#include "akjOGL.hpp"
#include "akjHWGraphicsContext.hpp"


namespace akj
{
cFrameBufferObject::
	cFrameBufferObject(cHWGraphicsContext* context, 
											const Twine& name, 
											ivec2 size, pix::eFormat pix_format)
	: cHWGraphicsObject(context, name)
	, mTexture(context, name + " texture")
	, mSize(size)
{
		glGenFramebuffers(1, &mObjectID);
		mTexture.CreateEmptyTexture2D(size.x, size.y, pix_format);
		
		//trial bind, so that we can check for completion now
		BindForWrite();
		IsComplete();
		// ..and back to how it was
		mParentContext->ResetFrameBuffer();
}

cFrameBufferObject::~cFrameBufferObject()
{
	glDeleteFramebuffers(1, &mObjectID);
}

void cFrameBufferObject::BindForRead()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mObjectID);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, mTexture.GetID(), 0);
	if(mDepthBuffer)
	{
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT ,
		GL_TEXTURE_2D, mDepthBuffer->GetID(), 0);
	}
	IsComplete();
}

void cFrameBufferObject::UnBindForRead()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mObjectID);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, 0, 0);
	if(mDepthBuffer)
	{
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT ,
		GL_TEXTURE_2D, 0, 0);
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void cFrameBufferObject::BindForWrite()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mObjectID);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, mTexture.GetID(), 0);
	if(mDepthBuffer)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT ,
		GL_TEXTURE_2D, mDepthBuffer->GetID(), 0);
	}
	IsComplete();
}



bool cFrameBufferObject::IsComplete()
{
	uint32_t target = 0;
	GLint bound_object;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &bound_object);
	target = GL_DRAW_FRAMEBUFFER;
	if(bound_object != mObjectID)
	{
		//we're not bound to DRAW, let's try read
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &bound_object);
		target = GL_READ_FRAMEBUFFER;
		if(bound_object != mObjectID)
		{
			AKJ_THROW("Called IsComplete() on an unbound framebuffer");
		}
	}
	
	mParentContext->CheckErrors(AKJ_LOG_CSTR("cFrameBufferObject::IsComplete"));

	uint32_t status = glCheckFramebufferStatus(target);
	cStringRef str = "";
	bool ret = false;
	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		str = "FBO is OK!";
		ret = true;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		str = "Incomplete Attachment";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		str = "Missing Attachment";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		str = "Incomplete Draw Buffer";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		str = "Incomplete Read Buffer";
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		str = "Unsupported framebuffer formats";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		str = "Incomplete Multisample";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		str = "Incomplete layer targets";
		break;
	case 0:
		str = "Error occurred";
		break;
	default:
		FatalError::Die("Unexpected FBO check case");
	}

	if(!ret)
	{
		Log::Error("Something wrong with FBO %s: \"%s\"", mObjectName, str);
	}
	return ret;
}

void cFrameBufferObject::CreateDepthBuffer()
{
	mDepthBuffer.reset(new cTextureObject(mParentContext, mObjectName 
											+ "depth texture"));
	mDepthBuffer->CreateDepthTexture(mSize.x, mSize.y);
}


cBitmap<4> cFrameBufferObject::SaveDepthBuffer(cAlignedBuffer& buffer)
{
	if(!mDepthBuffer) return cBitmap<4>();
	BindForRead();
	ivec2 vp = mParentContext->ViewPort();
	cBitmap<4> bitmap(vp.x, vp.y, 1, BIT_DEPTH_32);
	AlignedBuffer<16> bitmap_data;
	bitmap.UseAsStorage(bitmap_data);

	glReadPixels(0, 0, bitmap.Width(), bitmap.Height(),
		GL_DEPTH_COMPONENT, GL_FLOAT, bitmap.Data());
	mParentContext
		->CheckErrors(AKJ_LOG_CSTR("cFrameBufferObject::SaveDepthBuffer"));
	cBitmap<4> converted(buffer, vp.x, vp.y, 1, BIT_DEPTH_8);
	const float inv_range = 1/(1.0f-0.0f);

	bitmap.ForEachPixelFlipVert([]
		(uint8_t* src, uint8_t* dst)
	{
		const float r = *reinterpret_cast<float*>(src);
		const float val = (r);
		*dst = static_cast<uint8_t>((val)*255.9f);
	}, converted);
	UnBindForRead();

	return converted;
}

} // namespace akj