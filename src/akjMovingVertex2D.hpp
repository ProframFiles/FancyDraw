#pragma once
#include <memory>
namespace akj{
	class cVertexArrayObject;
	class cArrayBuffer;

class cMovingVertex2D
{
public:
	cMovingVertex2D()
	{
		memset(this, 0, sizeof(this));
		mStartTime = -1.0f;
	}
	cMovingVertex2D(float x, float y, float u, float v )
	{
		memset(this, 0, sizeof(this));
		mTexCoord[0]=u;
		mTexCoord[1]=v;
		mPosition[0]=x;
		mPosition[1]=y;
		mStartTime = -1.0f;
	}
	~cMovingVertex2D();
	static void BindToVAO(cVertexArrayObject& vao, cArrayBuffer& array_buffer);
	static int ByteStride();

	float mTexCoord[2];
	float mPosition[2];
	float mVelocity [2];
	float mAcceleration[2];
	float mStartTime;
	float mColor[3];
};
}

