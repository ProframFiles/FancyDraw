#pragma once
#include "akjLayeredDrawable.hpp"
#include "StringRef.hpp"
#include "akj2DGraphicsPrimitives.hpp"
#include "akjTextureObject.hpp"
#include "akjIndexedStorage.hpp"
#include "akjArrayBuffer.hpp"
#include "akjLinkedIndexList.hpp"
#include "akjHWGraphicsContext.hpp"
#include "akjDoubleVAO.hpp"
#include "akjDrawCommander.hpp"
#include "akjShaderObject.hpp"

namespace akj
{
	class cShaderObject;

	struct cPosTexCoord
	{
		cCoord2 pos;
		cCoord2 texCoord;
	};
	struct cQuadInfo
	{
		int mReferences;
		cWrappingCounter mDrawOrder;
		uint32_t mID;
		cTextureObject* mTexture;
		cCoord2 mPos;
		iRect mTexViewport;
		cArray<4,cPosTexCoord> mVerts;
		bool mIsVisible;
		bool operator <(const cQuadInfo& other) const
		{
			return mDrawOrder < other.mDrawOrder;
		}
	};


	class cTexturedQuadFactory : public iLayeredDrawable
	{
	public:
		enum{kInitBufferSize = sizeof(cPosTexCoord)*4*1024};

		cTexturedQuadFactory(cHWGraphicsContext& context);
		~cTexturedQuadFactory();
		//////////////////////////////////////////////////////////////////////////
		// Drawable interface functions
		//////////////////////////////////////////////////////////////////////////
		virtual void DrawSingleLayer(uint32_t layer);
		virtual void DepthSort();
		virtual void PrepareLayers(std::vector<cSubmittedLayer>& layers);
		virtual void PostDrawCleanup();
		virtual cStringRef LayerName() const {return "textured quad";};
		//higher priority draws last (on top)
		virtual uint32_t LayerPriority() const { return kTexturedQuadPriority; };

		//////////////////////////////////////////////////////////////////////////
		// Creation Functions
		//////////////////////////////////////////////////////////////////////////

		tTexHandle LoadTexture(cAlignedBitmap bitmap, const Twine& name);

		cTexturedQuad CreateTexturedQuad(const cTexturedQuad other);
		cTexturedQuad CreateTexturedQuad(iRect rect, tTexHandle texture);
		
		template <class tThis>
		static bool Test(tThis& factory);

	private:
		//////////////////////////////////////////////////////////////////////////
		// Graphics setup + state
		//////////////////////////////////////////////////////////////////////////
		void SetShaderBindings();
		void SetVertexAttributes();

		//////////////////////////////////////////////////////////////////////////
		// Handle relations
		//////////////////////////////////////////////////////////////////////////
		friend class cTexturedQuad;
		void AddReference(cQuadInfo& info);
		void RemoveReference(cQuadInfo& info);

		//////////////////////////////////////////////////////////////////////////
		// Data management
		//////////////////////////////////////////////////////////////////////////
		cQuadInfo& Info(uint32_t id)
		{
			return mQuadRecords[id];
		}
		cQuadInfo& AllocateInfo();
		void DestroyInfo(cQuadInfo& info);

		//////////////////////////////////////////////////////////////////////////
		// Draw State management
		//////////////////////////////////////////////////////////////////////////
		struct cVisibleState
		{
			enum {kQuadBytes = sizeof(cPosTexCoord)*4};
			void Reset(cVertexArrayObject& vao, cShaderObject* shader, iLayeredDrawable* drawable)
			{
				mShader = shader;
				mHasChanges = false;
				mNumCommandsPerLayer.clear();
				mVAO = &vao;
				mDrawable = drawable;
				mCommands.Clear();
				mLayers.clear();
				mOffset = 0;
			}

			void RunCommands(uint32_t layer)
			{
				auto r = mNumCommandsPerLayer.at(layer);
				mCommands.PlayBack(r.start, r.size);
			}

			void StartLayer(cWrappingCounter depth,
											uint32_t start_index = 0)
			{
				
				mLayers.emplace_back();
				mLayers.back().mDrawable = mDrawable;
				mLayers.back().mLayer = static_cast<uint32_t>(mLayers.size()-1);
				if(mNumCommandsPerLayer.empty()){
					mNumCommandsPerLayer.emplace_back(0, 0);
				}
				else{
					mNumCommandsPerLayer.emplace_back(mNumCommandsPerLayer.back().Next());
				}
				mCommands.Record<cBindShader>(*mShader);
				++mNumCommandsPerLayer.back().size;
				mCommands.Record<cBindVAO>(*mVAO);
				++mNumCommandsPerLayer.back().size;
			}

			void NewTexture(cTextureObject* tex, uint32_t num_quads)
			{
				mTexture = tex;
				if(num_quads > 0)
				{
					mCommands.Record<cDrawTriStripCommand>(
						cBufferRange(mVAO->Buffer(0), mOffset*kQuadBytes, num_quads*kQuadBytes));
					mOffset += num_quads;
					++mNumCommandsPerLayer.back().size;
				}
				mCommands.Record<cBindUniformInt>("uTexture0", *mShader, tex->GetBoundTextureUnit());
				++mNumCommandsPerLayer.back().size;
			}

			void NewLayer(cWrappingCounter layer, uint32_t num_quads)
			{
				mLayers.back().mDepth = layer;
				
				if(num_quads > 0){
					++mNumCommandsPerLayer.back().size;
						mCommands.Record<cDrawTriStripCommand>(
					cBufferRange(mVAO->Buffer(0), mOffset*kQuadBytes, num_quads*kQuadBytes));
					mOffset+=num_quads;
				}
				StartLayer(layer, mOffset);
			}

			uint32_t FinishLayers(uint32_t num_quads)
			{
				++mNumCommandsPerLayer.back().size;
				mCommands.Record<cDrawTriStripCommand>(
						cBufferRange(mVAO->Buffer(0), mOffset*kQuadBytes, num_quads*kQuadBytes));

				mOffset+=num_quads;
				return mOffset;
			}

			uint32_t mOffset;
			bool mHasChanges;
			cTextureObject* mTexture;
			std::vector<uRange> mNumCommandsPerLayer;
			cDrawCommander mCommands;
			iLayeredDrawable* mDrawable;
			cVertexArrayObject* mVAO;
			cShaderObject* mShader;
			std::vector<cSubmittedLayer> mLayers;
		};

		void MarkChanged(cQuadInfo& info)
		{
			if(info.mIsVisible)
			{
				mVisibleState.mHasChanges = true;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Members
		//////////////////////////////////////////////////////////////////////////

		cDoubleVAO mVertexBuffers;
		cShaderObject mShader;

		cIndexedStorage<cQuadInfo> mQuadRecords;
		std::vector<cQuadInfo*> mZSortedInfo;
		std::vector<std::unique_ptr<cTextureObject>> mTextures;

		// these are simply layout descriptions: they don't store any data
		std::vector<cStoredBitmap> mTexBitmaps;
		cVisibleState mVisibleState;
		cWrappingCounter mDepthCounter;
		cHWGraphicsContext& mContext;
	};

	template <class tThis>
	bool akj::cTexturedQuadFactory::Test(tThis& factory)
	{
		cTexturedQuadFactory& f = factory;
		
		//make a test texture
		cStoredBitmap bm(cAlignedBitmap(128, 128, 4, BIT_DEPTH_8));
		RGBAu8 colors[] = {cWebColor::BLUE, cWebColor::RED, cWebColor::CYAN};
		uint32_t c_index = 0;
		bm().ForEachPixel<pix::RGBA8>(
		[&colors, &c_index](pix::RGBA8& pixel){
			CopyColor(pixel, colors[c_index++]);
			if(c_index == sizeof(colors)/sizeof(RGBAu8)) { c_index = 0; }
		});
		bm().ExportPNG("testtexture.png");
		auto tex_handle = f.LoadTexture(bm(), "Test texture");
		const iRect rect = {25, 12, bm().W(), bm().H()};
		auto quad = f.CreateTexturedQuad(rect, tex_handle);
		std::vector<cSubmittedLayer> layers;
		std::initializer_list<iLayeredDrawable*> list = {&f};
		f.mContext.PushBackGroundColor(cWebColor::WHITE);
		f.mContext.Clear();
		LayeredDraw(list, layers);
		cAlignedBuffer bb_storage;
		auto backbuffer = f.mContext.GetBackBuffer(bb_storage);
		bool all_same = true;
		//take the subrectangle from the back buffer and make sure it's the same as the source texture
		ForEachPixel(
			[&all_same](const pix::RGBA8& src,const pix::RGBA8& dst){
			if(src.r() != dst.r()
				|| src.g() != dst.g()
				|| src.b() != dst.b()
				|| src.a() != dst.a())
				{
					all_same = false;
				}
			},
			backbuffer.SubRect<4>(rect.left, rect.top, rect.width, rect.height).Pixels<pix::RGBA8>(),
			bm().Pixels<pix::RGBA8>()
		);
		cAlignedBuffer serial_buffer;
		cSerialization::SerializeLZMA(bb_storage, serial_buffer);
		cAlignedBuffer serial_round_trip;
		cSerialization::ReaderLZMA(ToStringRef(serial_buffer), serial_round_trip);
		backbuffer.SetData(serial_round_trip.data());
		backbuffer.ExportPNG("roundtripped.png");
		return all_same;
	}

}
