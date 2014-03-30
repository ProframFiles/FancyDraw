#pragma once
#include <stdint.h>
#include "akjColor.hpp"
#include "FancyDrawMath.hpp"
#include "akj2DVertex.hpp"
#include "akjArrayBuffer.hpp"
#include "akjVertexArrayObject.hpp"
#include "akjShaderObject.hpp"
#include <vector>
#include <memory>
#include "akj2DGraphicsPrimitives.hpp"
#include "akjVectorList.hpp"
#include "akjIVec.hpp"
#include "akjPixelTree.hpp"
#include "akjWrappingCounter.hpp"
#include "akjScreenTextFactory.hpp"
#include "ArrayRef.hpp"
#include "akjLayeredDrawable.hpp"

namespace akj
{
	class cHWGraphicsContext;
	class c2DPrimitiveFactory;
	class cArrayBuffer;
	class cShaderObject;
	class cVertexArrayObject;
	


	class c2DPrimitiveFactory : public iLayeredDrawable
	{
		enum { 
			kInitialBufferSize = 1 * 1024 * 1024,
			// arbitrary limit, just to catch infinite allocation errors
			kMaxBufferSize = 64 * 1024 * 1024,
			//if there are more than 16 million shapes, we've got issues
			kMaxNumShapes = (16 * 1024 * 1024) -1,
		};
		enum eShapeKind 
		{
			ROUNDED_RECT,
		};
		struct cCurrentDraw
		{
			void Reset()
			{
				mDrawRanges.clear();
				mLayers.clear();
			}

			cBufferRange& RangeForLayer(uint32_t layer)
			{
				return mDrawRanges.at(layer);
			} 
			
			std::vector<cBufferRange> mDrawRanges;
			std::vector<cSubmittedLayer> mLayers;
		};
	public:
		c2DPrimitiveFactory(cHWGraphicsContext* context);
		~c2DPrimitiveFactory();

		cRoundedRectangle CreateRoundedRect(float left, float right, 
																				float top, float bottom,
																				float stroke_width, float radius,
																				RGBAu8 stroke_color, RGBAu8 fill_color);
		cRoundedRectangle CreateRoundedRect(iRect rect, float stroke_width, float radius,
																				RGBAu8 stroke_color, RGBAu8 fill_color)
		{
			return CreateRoundedRect(static_cast<float>(rect.left), static_cast<float>(rect.Right()),
															static_cast<float>(rect.top), static_cast<float>(rect.Bottom()),
															stroke_width, radius, stroke_color, fill_color);
		}

		cRoundedRectangle CreateRoundedRect(const cRoundedRectangle& other_rect);
		void DepthSort();
		void PrepareLayers(std::vector<cSubmittedLayer>& layers);
		virtual void DrawSingleLayer(uint32_t layer);
		virtual uint32_t LayerPriority() const {return kPrimitivePriority;}
		virtual cStringRef LayerName() const {return "shapes";};
		void PostDrawCleanup();
		
		void DrawWireFrame(uint32_t layer);

		void SetFrameTime(double abs_time);


		void WireframeMode(bool do_wireframe){ mIsWireframeMode = do_wireframe; }
		void ToggleWireframeMode() { WireframeMode(!mIsWireframeMode); }


		void SaveDepthBuffer();

	private:
		void SetShaderBindings();
		void DrawWithShader(cShaderObject& other_shader, uint32_t layer);
		friend class cRoundedRectangle;

		struct cShapeInfo
		{
			cShapeInfo() :mZOrder(0) {}
			uint32_t mID;
			cWrappingCounter mZOrder;
			c2DVertex::Instanced mInstanceData;
			iRect mOcclusionRect;
			iRect mCoverageRect;
			int mReferences;
			bool mIsHidden;
			cArrayRef<cScreenText> mText;
			bool operator <(const cShapeInfo& other) const
			{
				return  mZOrder < other.mZOrder;
			}
			bool operator >(const cShapeInfo& other) const
			{
				return mZOrder > other.mZOrder;;
			}
			void SetDepth(float dmin, float range)
			{
				mInstanceData.mDepth = (static_cast<float>(mZOrder + 1) - dmin)/range; 
			}
			void AdjustOcclusion();
		private:
			cShapeInfo& operator =(const cShapeInfo& other){ return *this; }
			
		};

		struct cSpansByIndexLess;
		struct cSpansByIndexGreater;

		void SetChildText(uint32_t id, cArrayRef<cScreenText> text);
		cShapeInfo& AllocateShape(eShapeKind num_vertices);
		void InitializeRoundedRect(uint32_t id, const c2DVertex& model);
		void ShapeDestroyed(uint32_t id);
		void ShapeCreated(uint32_t id);
		uint32_t UpdateBuffers();
		void ProcessTombstones();
		uint32_t GetNextShapeID();
		void InitStaticVertexData();
		void GrowBuffers(uint32_t max_bytes);
		void MarkChanged(uint32_t id);
		void SetFrameBufferTarget();
		void AddTombStone(uint32_t id);


		//////////////////////////////////////////////////////////////////////////
		// Members
		//////////////////////////////////////////////////////////////////////////

		std::vector<uint32_t> mTombstones;

		// list of all the shapes we have record of, sorted by ID only
		std::vector<cShapeInfo> mShapeRecords;

		// pointers to the shapes that we'll actually be drawing, 
		// sorted by draw order
		std::vector<cShapeInfo*> mZSortedShapes;

		// Filled and emptied every frame, 
		// might as well just keep the memory around
		std::vector<cShapeInfo*> mShapesToDrawTemp;


		cArrayBuffer mStaticVertexBuffer;
		cArrayBuffer mIndexBuffer;
		cArrayBuffer mInstancedBufferA;
		cArrayBuffer mInstancedBufferB;
		cFrameBufferObject mRenderBuffer;
		
		cShaderObject mShader;
		cShaderObject mDebugShader;
		
		cCurrentDraw mCurrentDraw;
		cSparseCoverageMap mTextCover;


		cVertexArrayObject mVAOA;
		cVertexArrayObject mVAOB;

		double mCurrentTime;

		cArrayBuffer* mInstanceFrontBuffer;
		cArrayBuffer* mInstanceBackBuffer;
		cVertexArrayObject* mFrontVBO;
		cVertexArrayObject* mBackVBO;
		
		cHWGraphicsContext* mContext;

		uint32_t mNewDraws;
		uint32_t mNumPrimitives;
		uint32_t mBufferCursor;
		uint32_t mIndexCursor;
		uint32_t mBufferSize;
		bool mIsDepthSorted;
		bool mNeedBufferUpdate;
		bool mNeedTombstoneUpdate;
		bool mIsWireframeMode;
	};
}
