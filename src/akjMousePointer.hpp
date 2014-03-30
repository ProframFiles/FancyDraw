#pragma once
#include "akjArrayBuffer.hpp"
#include "akjArrayBuffer.hpp"
#include "akjVertexArrayObject.hpp"
#include "akjShaderObject.hpp"
#include "akjTextureObject.hpp"
#include "akjIVec.hpp"
#include "Bitmap.hpp"

namespace akj
{
	class cMousePointer
	{
	public:
		cMousePointer(cHWGraphicsContext* context);
		~cMousePointer(){};
		void Draw(cCoord2 mousepos, double time);
		void SetCursor(cAlignedBitmap bitmap);

		void Hide(){ mIsHidden = true; }
		void Show(){ mIsHidden = false; }

	private:
		cArrayBuffer mStaticVerts;
		cVertexArrayObject mVAO;
		cTextureObject mTexture;
		cShaderObject mShader;
		ivec2 mHotPixel;
		ivec2 mImageSize;
		cHWGraphicsContext& mContext;
		bool mIsHidden;
	};
}
