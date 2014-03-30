#include "akjDoublePBO.hpp"
#include "akjOGL.hpp"

namespace akj
{

cDoublePBO::cDoublePBO( cHWGraphicsContext* context, cStringRef name,
												uint32_t width, uint32_t height, pix::eFormat fmt)
	:cTextureObject(context, name)
	,mArrayBufferA(context, (name.str() +" bufferA"), width*height*4, DYNAMIC_PBO)
	,mArrayBufferB(context, (name.str() +" bufferB"), width*height*4, DYNAMIC_PBO)
	,mClientBuffer(NULL)
	,mServerBuffer(NULL)
	,mWidth(width)
	,mHeight(height)
	,mPixFormat(fmt)
	,mIsSwapNeeded(false)
{
	mClientBuffer= &mArrayBufferA;
	mServerBuffer= &mArrayBufferB;
	SetSize(width, height);
}

void cDoublePBO::SetSize( int x, int y )
{
	mWidth = x;
	mHeight = y;
	CreateEmptyTexture2D( x, y, mPixFormat);
	uint32_t buffer_size = x*y*4;
	mArrayBufferA.InitBuffer(buffer_size, NULL);
	mArrayBufferB.InitBuffer(buffer_size, NULL);
	mArrayBufferA.UnBind();
}

cDoublePBO::~cDoublePBO()
{

}

void cDoublePBO::UpdateRect(cBitmap<4> bitmap, int x, int y )
{
	uint8_t* buffer = static_cast<uint8_t*>(mClientBuffer->MapBuffer());
	const size_t dest_stride = mWidth*4;
	buffer += y*dest_stride + x*4;
	for (uint32_t row = 0; row < bitmap.Height() ; ++row)
	{
		const uint8_t* src = bitmap.RowData(row);
		memcpy(buffer, src, bitmap.BytesPerLine());
		buffer += dest_stride;
	}
	mClientBuffer->UnMapBuffer();
	mClientBuffer->UnBind();
	mIsSwapNeeded = true;
}

void cDoublePBO::Swap()
{
	if(!mIsSwapNeeded) return;
	else
	{
		// "client" PBO has data now, so it's actually the "server" buffer 

		cArrayBuffer* temp = mClientBuffer;
		mClientBuffer = mServerBuffer;
		mServerBuffer = temp;

		// now we bind the data to the texture 
		// (and in so doing unbind the other PBO)
		mServerBuffer->Bind();
		cTextureObject::Bind();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight,
										GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// leave the PBO unbound now that it's not needed		
		mClientBuffer->ResetData(mHeight*mWidth*4);
		mClientBuffer->UnBind();
		mIsSwapNeeded = false;
	}

}

} // namespace akj