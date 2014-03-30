#pragma once
#include "akjShaderObject.hpp"
#include "akj2DPrimitiveFactory.hpp"
#include "akjTextureObject.hpp"
#include "Twine.hpp"

namespace akj
{
	class cGraphicsTester
	{
	public:
		cGraphicsTester(cHWGraphicsContext* context, c2DPrimitiveFactory* shape_factory);
		~cGraphicsTester();
		void PixelBorderTest(int mouse_x, int mouse_y, double time_now);
		void RoundedRectTest(float width, float height, float stroke, float radius);


		void WriteToMatFile(const std::vector<float>& out_data);

		void SaveNextFrame(const Twine& filename);
		enum { kTestSize = 120 };
	private:
		std::vector<cRoundedRectangle> mRectangles;
		cHWGraphicsContext* mContext;
		c2DPrimitiveFactory* mPrimitiveFactory;
		cShaderObject mTestShader;
		cTextureObject mBGTexture;
		uint32_t mID;
		std::string mOutFileName;
	};
}
