#pragma once
#include "akjTextureObject.hpp"
#include "akjArrayBuffer.hpp"
#include "Bitmap.hpp"
#include "akjPixelFormats.hpp"

namespace akj
{
		class cDoublePBO: public cTextureObject
	{
	public:
		cDoublePBO(cHWGraphicsContext* context, cStringRef name,
								uint32_t width, uint32_t height, pix::eFormat fmt);
		~cDoublePBO();

		void SetSize(int x, int y);
		void UpdateRect(cBitmap<4> bitmap, int x = 0, int y = 0);
		void Unbind();
		void Swap();

		private:

			cArrayBuffer mArrayBufferA;
			cArrayBuffer mArrayBufferB;
			cArrayBuffer* mClientBuffer;
			cArrayBuffer* mServerBuffer;
			uint32_t mWidth;
			uint32_t mHeight;
			pix::eFormat mPixFormat;
			bool mIsSwapNeeded;
	};

}
