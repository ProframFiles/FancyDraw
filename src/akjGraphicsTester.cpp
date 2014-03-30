#include "akjGraphicsTester.hpp"
#include "resources/akjStaticResources.hpp"
#include "akjHWGraphicsContext.hpp"
#include "akj2DPrimitiveFactory.hpp"
#include "akjOGL.hpp"
#include "FileOutputBuffer.hpp"
#include "RawOstream.hpp"
#include "Bitmap.hpp"
#include "akjAlignedBuffer.hpp"
#include "stb_image_write.h"

namespace akj
{




	cGraphicsTester::~cGraphicsTester()
	{
		mRectangles.clear();
		glDeleteFramebuffers(1, &mID);
	}

	cGraphicsTester::
		cGraphicsTester(cHWGraphicsContext* context, c2DPrimitiveFactory* factory)
		:mContext(context)
		,mPrimitiveFactory(factory)
		,mTestShader(context, "2D Pixel testing shader")
		,mBGTexture(context, "Pixel testing render texture")
	{
		mTestShader.
			CreateShaderProgram(cStaticResources::get_PrimitiveShapeTester_glsl());
		glGenFramebuffers(1, &mID);
	}

	void cGraphicsTester::PixelBorderTest(int mouse_x, int mouse_y, double time_now)
	{
		int mouse_uniform = mTestShader.GetUniformLocation("uMousePosition");
		int width, height;
		
		mContext->ViewPort(&width, &height);
		int orig_vp_x = width;
		int orig_vp_y = height;

		if(!mOutFileName.empty())
		{
			mContext->ViewPort(&width, &height);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mID);
			mContext->SetViewPort(0, 0, kTestSize, kTestSize);
			mContext->ViewPort(&width, &height);
			width = kTestSize;
			height = kTestSize;
		}

		mTestShader.
			BindUniformToFloat("uCurrentTime", static_cast<float>(time_now));
		mTestShader.BindUniformToVec2("uMousePosition", 
			static_cast<float>(mouse_x), static_cast<float>(mouse_y));


		
		mRectangles.clear();
		if (mRectangles.size() != 20)
		{
			mContext->Clear(RGBAu8(0, 0, 0, 255));
		
			// horizontal stripes
			static int first_time = 0;
			int vert = 30;
			float incr = 4.0f / (vert);
			for (int i = 0; i < vert ; ++i)
			{
				if(first_time==0)
				{
					Log::Debug("rect (%d, %d, %d, %d): w = %d", incr*i,width-incr*i,
										5*i, 5*i+4, width-2*i*incr);
				}
				mRectangles.emplace_back(mPrimitiveFactory->
					CreateRoundedRect(incr*i, width-incr*i, 4.0f*i, 4.0f*i+2, 0, 0,
					cWebColor::BLACK, cWebColor::BLACK));
			}
			first_time++;
		}

		//mPrimitiveFactory->DrawWithShader(mTestShader);

		if(!mOutFileName.empty())
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mID);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, mBGTexture.GetID(), 0);
			
			
			std::vector<float> out_data;
			out_data.resize(4 * kTestSize * kTestSize);

			glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, out_data.data());
			glCheckAllErrors(__FILE__, __LINE__);

			WriteToMatFile(out_data);

			mOutFileName.clear();
			
			mContext->SetViewPort(0, 0, orig_vp_x, orig_vp_y);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		}
		

	}

	void cGraphicsTester::SaveNextFrame(const Twine& filename)
	{
		mOutFileName = filename.str();
		mBGTexture.CreateEmptyTexture2D(kTestSize, kTestSize, pix::kRGBA32f);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mID);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, mBGTexture.GetID(), 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void cGraphicsTester::WriteToMatFile(const std::vector<float>& out_data)
	{
		std::string filename = mOutFileName;
		uint32_t width = kTestSize;
		uint32_t height = kTestSize;
		uint32_t numvars = 4;
		AKJ_ASSERT(numvars <= 4);
		char varnames[] = { 'r', 'g', 'b', 'a' };
		OwningPtr<raw_fd_ostream> fout;
		raw_fd_ostream::Create(filename+".m", fout, sys::fs::F_Binary).assertOK();

		*fout << "function [";
		for (size_t var = 0; var < numvars ; ++var)
		{
			*fout << varnames[var];
			if (var != numvars - 1){ *fout << ","; };
		}
		*fout << "] = " <<  filename << "()\n";
		
		for (size_t var = 0; var < numvars ; ++var)
		{
			*fout << varnames[var] << " = ...\n[";
			uint32_t stride = width * numvars;
			for (int row = height - 1; row >= 0; --row)
			{
				uint32_t row_start = stride*row;
				size_t col;
				for (col = 0; col < width - 1; ++col)
				{
					*fout << out_data[row_start + numvars * col+var] << ',';
				}
				if (col < width)
				{
					*fout << out_data[row_start + numvars * col+var];
				}
				if (row != 0)
				{
					*fout << ";...\n";
				}
				else
				{
					*fout << "];\n";
				}
			}
		//code
		}
		fout->close();
	}



} // namespace akj