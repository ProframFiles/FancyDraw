#pragma once
#include "akjHWGraphicsObject.hpp"
#include "akjUintHandle.hpp"
#include "Bitmap.hpp"

namespace akj
{
	class cImageData;
	
	class cTextureObject : public cHWGraphicsObject
	{
	public:
		enum eInterpMode
		{
			NEAREST,
			LINEAR,
		};
		cTextureObject(cHWGraphicsContext* context, const Twine& object_name);
		~cTextureObject();
		void CreateCubeMap( const char* base_name, bool is_srgb = false );
		void CreateTexture2D(const cAlignedBitmap& image, bool is_srgb = false,
													int miplevels = -1 );
		void CreateEmptyTexture2D( int width, int height, pix::eFormat format);
		void SetWrapMode(uint32_t sval, uint32_t tval) const;
		int GetBoundTextureUnit() const {return mBoundTextureUnit;}
		virtual void Bind();
		virtual void UnBind();
		void SetInterpMode( eInterpMode shrink_mode, eInterpMode grow_mode ) const;
		void DisableInterpolation() const;
		void CreateDepthTexture(int width, int height);
	private:
		int mBoundTextureUnit;
	};
	typedef cHandle<cTextureObject> tTexHandle;
}
