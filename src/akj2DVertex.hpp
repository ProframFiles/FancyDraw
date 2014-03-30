#pragma once
#include <memory>
#include "FancyDrawMath.hpp"

namespace akj{
	class cVertexArrayObject;
	class cArrayBuffer;

struct c2DVertex
{
	static void BindToVAO(cVertexArrayObject& vao,
												cArrayBuffer& array_buffer_per_vertex,
												cArrayBuffer& array_buffer_per_instance,
												cArrayBuffer& index_buffer);
	static int PerVertexByteStride();
	static int PerInstanceByteStride();
	static void UnbindAttributes();
	static void CheckRuntimeMemLayout();
	const float* ptr() const {
		return reinterpret_cast<const float*>(this); 
	}
	// this should be a multiple of 32 bytes in size
	// (this is what the video card wants)
	cCoord2 mTexCoord;							// 8 bytes
	cCoord2 mShiftDirection;      // 16 bytes
	struct Instanced{
		cCoord2 mPosition;						// 24 bytes
		cCoord2 mHalfSize;					// 32 bytes
		float mCornerRadius;					// 36 bytes
		float mStrokeWidth;					// 40 bytes
		RGBAu8 mStrokeColor;				// 44 bytes
		RGBAu8 mFillColor;					// 48 bytes
		cCoord2 mFillCoord;						//56
		float mDepth;								//60
		float mExtraFillAlpha;				//64

	};
};

AKJ_SIZE_CHECK(c2DVertex, 16)
}