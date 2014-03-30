#pragma once
#include "FancyDrawMath.hpp"
#include "akjColor.hpp"
#include "Twine.hpp"
#include "akjPixelTree.hpp"
#include "akjWrappingCounter.hpp"

#include <memory>

struct SDL_Window;
namespace cl
{
	class Context;
}


namespace akj{

	class cHWGraphicsContext;
	class cTextureObject;
	class cBackgroundSurface;
	class cWorkerPool;
	class cFrameBufferObject;

	struct cPushedBGColor
	{
		cPushedBGColor(cHWGraphicsContext* context, RGBAf old_color);
		cPushedBGColor(cPushedBGColor& other);
		~cPushedBGColor();
		RGBAf mOldColor;
		cHWGraphicsContext* mContext;
	private: 
		
	};

	class cHWGraphicsContext
	{
	public:
		enum {kPrimitiveRestartIndex = 0xFFFFFFFF};
		enum eBlendMode
		{
			BLEND_ORDINARY,
			BLEND_SATURATION,
			BLEND_ADD,
			BLEND_MAX,
		};
		cHWGraphicsContext(SDL_Window* window);
		~cHWGraphicsContext();
		void Clear(const RGBAf& color);
		void ClearColor(const RGBAf& color);
		RGBAf ClearColor();

		cPushedBGColor PushBackGroundColor(const RGBAf& color);

		void Clear();
		void ClearDepth(float val);

		void EnableDepthWrites();
		void DisableDepth();

		void SetBlendMode(eBlendMode mode);
		void ResetFrameBuffer();

		void BlockUntilFinished();
		void SaveBackBuffer(cWorkerPool& task_pool,  const Twine& filename);
		cBitmap<4> GetBackBuffer(cAlignedBuffer& storage);
		void DrawBackground();

		void SubmitFBO(const std::string& name, cFrameBufferObject* FBO);
		cFrameBufferObject*  GetFBO(const std::string& name);

		void SetViewPort(int x, int y, int width, int height);
		void ViewPort(int* width, int* height);
		ivec2 ViewPort() const;
		void Swap();
		int CheckErrors(const Twine& where_and_why);
		void AssertErrorFree(const Twine& reason);
		cBitmap<4> SaveAlphaBuffer(cAlignedBuffer& buffer);

		cSparseCoverageMap& Coverage();
		void ResetDepthRange()
		{
			mMinDepth = 2e10f;
			mMaxDepth = 0.0f;
			mMinLayer = mDrawCounter;
			mMaxLayer = mMinLayer - cWrappingCounter::kMaxCount;
		}

		void AddMinDepth(cWrappingCounter dmin)
		{
			if( dmin < mMinLayer )
			{
				mMinLayer = dmin;
				const float range = static_cast<float>(mMaxLayer - mMinLayer) + 2.0f;
				mInvDepthRange = 1.0f/range;
			} 
			
		}

		cWrappingCounter MinDepth() const {return mMinLayer;}
		cWrappingCounter MaxDepth() const {return mMaxLayer;}

		void AddMaxDepth(cWrappingCounter dmax)
		{
			if( dmax > mMaxLayer )
			{
				mMaxLayer = dmax;
				const float range = static_cast<float>(mMaxLayer - mMinLayer) + 2.0f;
				mInvDepthRange = 1.0f/range;
			} 
		}
		
		inline float NormalizedDepth(cWrappingCounter depth) const
		{	
			return (1.0f+static_cast<float>(depth))*mInvDepthRange;
		}

		cWrappingCounter DrawCount()
		{
			return mDrawCounter++;
		}

		uint32_t UniformAlign() const {return mUniformAlign;}
		uint32_t UniformAlign(uint32_t bytes) const 
		{
			return ((bytes + mUniformAlign-1)/mUniformAlign)*mUniformAlign;
		}

		//////////////////////////////////////////////////////////////////////////
		// Static Functions
		//////////////////////////////////////////////////////////////////////////

		static std::vector<float> OrthoMatrix(float left, float right);
		
		static void MyOrtho2D(float* mat, 
			float left, float right, float bottom, float top);

		static const float* IdentityMatrix();
		static void GenerateTriStripIndices(uint32_t starting_index,
																				uint32_t num_triangles,
																				std::vector<uint32_t>& vec);
		





		private:
		void SaveBackBufferImpl();

		int mUniformAlign;
		SDL_Window* mGLWindow;
		std::unique_ptr<cl::Context> mComputeContext;

		cSparseCoverageMap mCoverage;
		std::unordered_map<std::string, cFrameBufferObject*> mFrameBuffers;
		std::unique_ptr<cBackgroundSurface> mBackGroundSurface;
		cWrappingCounter mDrawCounter;

		float mMinDepth;
		float mMaxDepth;
		cWrappingCounter mMaxLayer;
		cWrappingCounter mMinLayer;
		float mInvDepthRange;

		//transient state for saving backbuffers just before draw
		std::string mBackBufferSave;
		cWorkerPool* mWorkerPool;
	};
}
