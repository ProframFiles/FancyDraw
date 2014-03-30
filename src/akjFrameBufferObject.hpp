#pragma once

#include "akjHWGraphicsObject.hpp"
#include "akjTextureObject.hpp"
#include "akjIVec.hpp"
#include "akjPixelFormats.hpp"

namespace akj
{
	class cFrameBufferObject : public cHWGraphicsObject
	{
		enum eBindState
		{
			kNotBound,
			kBoundAsRead,
			kBoundAsWrite
		};

	public:
		cFrameBufferObject(cHWGraphicsContext* context, const Twine& name,
												ivec2 size, pix::eFormat pix_format);
		~cFrameBufferObject();
		void CreateDepthBuffer();
		void BindForRead();
		void BindForWrite();
		bool IsComplete();
		cTextureObject& Texture() {return mTexture;};
		cTextureObject* DepthTexture() {return mDepthBuffer.get();};
		cBitmap<4> SaveDepthBuffer(cAlignedBuffer& buffer);
		void UnBindForRead();
	private:

		cTextureObject mTexture;
		std::unique_ptr<cTextureObject> mDepthBuffer;
		ivec2 mSize;
	};
}
