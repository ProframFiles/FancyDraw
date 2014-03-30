#include "akjScreenTextFactory.hpp"
#include "akjDistanceFieldFont.hpp"
#include "resources/akjStaticResources.hpp"
#include "akjHWGraphicsContext.hpp"
#include "akjRandom.hpp"
#include "akjTexAtlasFont.hpp"
#include "akjOGL.hpp"
#include "akjGraphicsStateChange.hpp"

namespace akj
{


cScreenTextFactory::
	cScreenTextFactory(cHWGraphicsContext& context, cFontLoader& font_loader)
	: mContext(context)
	, mFontLoader(font_loader)
	, mVertexBuffers(&context, "Screen text factory VAO",
		{cGLBufferDesc(DYNAMIC_VBO, kInitialBufferSize)})

		// will actually create a buffer 4x the size (cUniformRingBuffer implicitly)
		// doubles the requested size
	, mUniformBuffer(context, "Screen Text uniform buffer", 
										context.UniformAlign(kUniformBlockBytes)*2)
	, mAlphaShader(&context, "Screen Text Alpha Shader")
	, mNeedDepthSort(true)
	, mHasChanges(false)
	, mEdgeShift(0.07f)
	, mEdgeWidth(1.10f)
	, mCurrentTime(0.0)
	, mTopFuzz(70.0f)
	, mBottomFuzz(0.0f)
{
	mAlphaShader.CreateShaderProgram(cStaticResources::get_ScreenTextAlpha_glsl());
	for(uint32_t i= 0; i < NUM_BUILTIN_FONTS; ++i)
	{
		eBuiltinFonts f = static_cast<eBuiltinFonts>(i);
		tDFFHandle distance_handle = mFontLoader.LoadDFFFromMemory(BuiltinFont(f));
		tFaceHandle face = mFontLoader.FaceHandleFrom(distance_handle);
		mBuiltinFonts.push_back(face);
		mDistanceTextures.emplace_back(&context, ToString(f) +" Distance Texture");
		mAtlasTextures.emplace_back(&context, ToString(f) +" Atlas Texture");
		LoadFontTextures(f);
	}

	mCurrentFrame.Reset();
	mCurrentFrame.mShader = &mAlphaShader;


	SetVertexAttributes();

	MakeSampleLocations({0.0f, 0.0f});

	mDebugText = CreateText( "y"+ Twine(EdgeWidth()) + ", " +
														Twine(EdgeShift()), {500.0f, 15.0f}, 20);
	SetShaderBindings();
}

cScreenTextFactory::~cScreenTextFactory()
{

}

void cScreenTextFactory::SetShaderBindings()
{
	//vao bindings
	const ivec2 vpi = mContext.ViewPort();
	const cCoord2 vp(vpi.x, vpi.y);
	mAlphaShader.Bind();
	float mat4[16] = {};
	mContext.MyOrtho2D(mat4, 0.0f, vp.x, vp.y,  0.0f);
	mAlphaShader.BindProjectionMatrix(mat4);
	mAlphaShader.BindUniformToVec2("uFudgeFactors", mEdgeShift, mEdgeWidth);

	mAlphaShader.UnBind();

}

void cScreenTextFactory::LoadFontTextures(eBuiltinFonts font_kind)
{
	tFaceHandle face = mBuiltinFonts.at(font_kind);
	if(!face.IsValid()) return;

	tDFFHandle distance_handle = mFontLoader.DistanceHandleFrom(face);
	if(distance_handle.IsValid())
	{
		cDistanceFieldFont& dff =  mFontLoader.DistanceFont(distance_handle);
		cTextureObject& d_texture = mDistanceTextures[font_kind];
			d_texture.CreateTexture2D(dff.TextureBitmap(), false, 0);
			d_texture.SetInterpMode(cTextureObject::NEAREST,
																cTextureObject::NEAREST);
	}
	tTAFHandle atlas_handle = mFontLoader.TexAtlasHandleFrom(face);
	if(atlas_handle.IsValid())
	{
		cTexAtlasFont& atlas_font = mFontLoader.TexAtlasFont(atlas_handle);
		cTextureObject& a_texture = mAtlasTextures[font_kind];
		a_texture.CreateTexture2D(atlas_font.GetTexture(), false, 0);
		a_texture.SetInterpMode(cTextureObject::NEAREST,
																cTextureObject::NEAREST);
	}
	
	SetShaderBindings();
}

void cScreenTextFactory::SetVertexAttributes()
{
	mVertexBuffers.SetAttributes({

		//per-vertex attributes
		cVertexAttribute(ATTR_FLOAT, 2, 48, 0, 0), // vec2 aCenterPos;
		cVertexAttribute(ATTR_FLOAT, 2, 48, 0, 0), // vec2 aVelocity;
		cVertexAttribute(ATTR_FLOAT, 2, 48, 0, 0), // vec2 aAccel;
		cVertexAttribute(ATTR_FLOAT, 2, 48, 0, 0), // vec2 aTexCenter;
		cVertexAttribute(ATTR_FLOAT, 2, 48, 0, 0), // vec2 aTexSize;
		cVertexAttribute(ATTR_FLOAT , 1, 48, 0, 0),  // float aClockwiseRadians;
		cVertexAttribute(ATTR_UINT, 1, 48, 0, 0),  // float aIndex;
	});
}

void cScreenTextFactory::AddReference(uint32_t id)
{
	if(id < kMaxNumShapes)
	{
		AKJ_ASSERT(mTextRecords.at(id).mReferences >= 0);
		if(mTextRecords.at(id).mReferences == 0 && mTextRecords[id].IsVisible())
		{
			MarkAsChanged(id);
		}
		++mTextRecords[id].mReferences;
	}
}

void cScreenTextFactory::RemoveReference(uint32_t id)
{
	if(id < kMaxNumShapes)
	{
		AKJ_ASSERT(mTextRecords.at(id).mReferences > 0);
		--mTextRecords.at(id).mReferences;
		if(mTextRecords.at(id).mReferences == 0)
		{
			mHasChanges++;
			FreeResources(id);
		}
	}
}

akj::cScreenText cScreenTextFactory
::CreateText(const Twine& text_in, cCoord2 location, uint32_t pixel_size,
							eBuiltinFonts style/* = UI_FONT*/)
{
	cSmallVector<char, 256> temp;
	cStringRef text = text_in.toNullTerminatedStringRef(temp);
	//Log::Debug("Emplacing Text: %s", text);
	
	cTextInfo& info = AllocateTextInfo();
	info.mPixelSize = pixel_size;
	info.mText = text;
	info.mFontType = style;

	if(!ShapeText(info, location))
	{
		info.SetInvisible();
		info.mRequestedFlags.Set(kForceHidden);
	}
	
	return cScreenText(this, info.mID);
}

void cScreenTextFactory::ChangeText(uint32_t id, const Twine& text)
{
	cTextInfo& info = mTextRecords.at(id);
	std::string new_text = text.str();
	if(info.mText != new_text)
	{
		info.mText = text.str();
		if(!ShapeText(info, UniformData(info).mPosition))
		{
			// something went wrong: no font for you
			MarkAsChanged(info.mID);
			info.SetInvisible();
		}
	}
}


bool cScreenTextFactory::ShapeText(cTextInfo& info,const cCoord2& location)
{
	if(!mBuiltinFonts[info.mFontType].IsValid())
	{
		return false;
	}
	const tFaceHandle face_handle = mBuiltinFonts[info.mFontType];

	if(info.mRequestedFlags.IsSet(kTexAtlasFont))
	{
		tTAFHandle atlas_handle = mFontLoader.TexAtlasHandleFrom(face_handle);
		
		if(atlas_handle.IsValid())
		{
			cTexAtlasFont& font = mFontLoader.TexAtlasFont(atlas_handle);
			if(font.SetPixelSize(info.mPixelSize))
			{
				info.mTextureUnit = mAtlasTextures[info.mFontType].GetBoundTextureUnit();
				ShapeTextTAF(font, info, location);
				info.UseTexAtlasStyle();
				return true;
			}
			else{

			}
		}
	}
	// didn't get an atlas font, for whatever reason
	tDFFHandle distance_handle = mFontLoader.DistanceHandleFrom(face_handle);
	if(distance_handle.IsValid())
	{
		info.mTextureUnit = mDistanceTextures[info.mFontType].GetBoundTextureUnit();
		cDistanceFieldFont& dff =  mFontLoader.DistanceFont(distance_handle);
		ShapeTextDFF(dff, info, location);
		info.UseDistanceFieldStyle();
		return true;
	}
	// didn't use any font :(
	return false;
}

void cScreenTextFactory::ShapeTextTAF(cTexAtlasFont& taf, cTextInfo& info, 
																		const cCoord2& location)
{
	AKJ_ASSERT_AND_THROW(taf.SetPixelSize(info.mPixelSize));
	auto positions = taf.ShapeText(info.mText);
	info.mGlyphs.clear();
	info.mGlyphs.reserve(positions.size());
	info.mBoundingBox = {0, 0, 0, 0};

	for(const auto& g: positions)
	{
		// this applies to newline...
		if(g.index == 0) continue; 
		const auto& atlas_glyph = taf.GlyphByIndex(g.index);
		// skip spaces
		if(atlas_glyph.mCharCode == 32) continue;

		info.mGlyphs.emplace_back(
			(g.offset-cCoord2(atlas_glyph.mOffset)+0.5).floor(), 
			info.mVelocity,
			info.mAccel,
			cCoord2(atlas_glyph.mTexRect.pos()+ivec2(0,atlas_glyph.mTexRect.height)),
			cCoord2(atlas_glyph.mTexRect.size()),
			-0.2f, 0);
		info.mBoundingBox.ExpandToFit(atlas_glyph.mTexRect);
	}
/*
	cCoord2 mGlyphPos;
	cCoord2 mVelocity;
	cCoord2 mAcceleration;
	cCoord2 mTexPos;
	cCoord2 mTexSize;
	float mClockwiseRadians;
	uint32_t mIndex;
*/
	ivec2 snapped_pos = (location+0.5f).ivec();
	info.mBoundingBox += snapped_pos;
	auto& u_data = UniformData(info);
	u_data.mMessageSize = cCoord2(1.0f);
	u_data.mPosition = cCoord2(snapped_pos);
	u_data.mOuterColor = 0xFFFFFFFF;
	u_data.mInnerColor = 0xFFFFFFFF;
	u_data.mSolidColor = cWebColor::BLACK;
	u_data.mFuzzFactor = 1.0f;
	u_data.mZDepth = -0.0f;
	u_data.mOuterRange = 1.0f;

}
void cScreenTextFactory::ShapeTextDFF(cDistanceFieldFont& dff, cTextInfo& info, 
																		const cCoord2& location)
{
	const float scale = dff.ScaleForPixelSize((float)info.mPixelSize);

	const float tex_size = dff.TextureSize();
	const float inv_tex_size = 1.0f/tex_size;

	auto positions = dff.ShapeText(info.mText);
	info.mGlyphs.clear();
	info.mGlyphs.reserve(positions.size());
	info.mBoundingBox = {0, 0, 0, 0};

	for(const auto& g: positions)
	{
		// this applies to newline...
		if(g.index == 0) continue; 
		const auto& dff_glyph = dff.GlyphByIndex(g.index);
		// skip spaces
		if(dff_glyph.mCharCode == 32) continue;
		const cCoord2 sz = dff_glyph.mSize;
		const cCoord2 off = dff_glyph.mOffset - cCoord2(0.0f, (sz.y)*tex_size);
		info.mGlyphs.emplace_back(
			g.offset+cCoord2(-1.0f, 1.0f)*dff_glyph.mOffset, //mCenterPos;
			info.mVelocity,
			info.mAccel,
			dff_glyph.mLocation*tex_size+off*cCoord2(1.0f,-1.0f),
			sz*tex_size, //cCoord2 mTexSize;
			-0.2f, 0);
	}
	
	info.mBoundingBox = {0, 0, 0, 0};
	info.mBoundingBox += location.ivec();
	
	for(const auto & vert: info.mGlyphs)
	{
		const cCoord2 quad_size = vert.mTexSize*(scale);
		const cCoord2 pos = scale*vert.mGlyphPos+location-cCoord2(0.0f,1.0f)*quad_size;
		info.mBoundingBox.ExpandToFit(pos, quad_size);
	}

	auto& u_data = UniformData(info);
	u_data.mMessageSize = cCoord2(scale, scale);
	u_data.mPosition = (cCoord2(location.x, location.y)+0.5).floor();
	u_data.mOuterColor = 0xFFFFFFFF;
	u_data.mInnerColor = 0xFFFFFFFF;
	u_data.mSolidColor = cWebColor::BLACK;
	u_data.mFuzzFactor = 1.0f+std::min(std::max(
			info.mPixelSize-mBottomFuzz, 0.0f)/(mTopFuzz-mBottomFuzz) , 1.0f);
	u_data.mZDepth = -0.0f;
	u_data.mOuterRange = dff.OuterPixelRange();
}


cTextInfo& cScreenTextFactory::AllocateTextInfo()
{
	uint32_t new_index;
	if(!mTombstones.empty())
	{
		new_index = mTombstones.top();
		mTombstones.pop();
		AKJ_ASSERT(mTextRecords.at(new_index).mReferences == -1);
	}
	else
	{
		new_index = static_cast<uint32_t>(mTextRecords.size());
		mTextRecords.emplace_back();
	}
	cTextInfo& info = mTextRecords.at(new_index);


	ZeroMem(info.mUniformData);

	info.mID = new_index;
	info.mDrawOrder = cWrappingCounter(0);
	info.mRequestedFlags.ClearAll();
	info.mActualFlags.ClearAll();
	info.SetVisible();
	info.RequestDistanceFieldStyle();
	info.mGlyphs.clear();
	info.mReferences = 0;
	info.mVelocity = {0.0f, 0.0f};
	info.mAccel = {0.0f, 0.0f};
	info.mTextureUnit = 0;
	info.mFontType = UI_FONT;
	UniformData(new_index).mStartTime = static_cast<float>(mCurrentTime);

	return info;
}

void cScreenTextFactory::FreeResources(uint32_t id)
{
	mTombstones.push(id);
	const uint32_t tombstone_top = mTombstones.top(); 
	cTextInfo& info = mTextRecords.at(id);
	info.mRequestedFlags.ClearAll();
	info.mActualFlags.ClearAll();
	info.mGlyphs.clear();
	info.mReferences = -1;

	while(!mTombstones.empty() && mTombstones.top() == mTextRecords.size()-1)
	{
		mTombstones.pop();
		mTextRecords.pop_back();
	}
}

void cScreenTextFactory::PrepareLayers(std::vector<cSubmittedLayer>& layer_vec)
{
	AKJ_ASSERT(!mNeedDepthSort);
	// if there were no changes, then the depth sort would not have changed, so don't check,
	// and just return the cached command list
	if(!mHasChanges)
	{
		for(const auto& layer: mCurrentFrame.mLayers)
		{
			layer_vec.push_back(layer);
		}
		return;
	}
	mHasChanges = false;
	mCurrentFrame.Reset();

	if(mZSortedText.empty())
	{
		return;
	}

	uint32_t req_ub = u32(mZSortedText.size()+kUniformBlockSize-1)/kUniformBlockSize;

	while(req_ub*kUniformBlockBytes*2 > mUniformBuffer.Size())
	{
		mUniformBuffer.Grow();
	}

	cArrayBuffer* back_buf = &mVertexBuffers.Back().Buffer(0);
	cSubmittedLayer current_layer;
	current_layer.mDrawable = this;
	current_layer.mDepth = mZSortedText.front()->mDrawOrder;
	current_layer.mLayer =0;

	cTextInfo** text_ptr = &mZSortedText.front();
	cTextInfo** text_end_ptr = text_ptr+mZSortedText.size();

	uRange vert_range = {0, 0};
	uint32_t uniform_index = 0;

	uint32_t texture_unit = 0;

	uint32_t text_count = 0;
	// first count the number of layers
	while(text_ptr < text_end_ptr)
	{	
		auto buf = mUniformBuffer.MapBytes(kUniformBlockBytes);
		mCurrentFrame.AddUniformSwitch(buf.Range(), uniform_index++);
		if(uniform_index>1)
		{
			bool crap = true;
		}
		uint32_t current_slot = 0;
		while(text_ptr < text_end_ptr && current_slot < kUniformBlockSize)
		{
			cTextInfo& text = *(*text_ptr++);
			// start a new draw layer
			if(text.mDrawOrder != current_layer.mDepth){
				vert_range = mCurrentFrame.AddDrawCommand(*back_buf, vert_range);
				mCurrentFrame.NextLayer();
				mCurrentFrame.mLayers.emplace_back(current_layer);
				current_layer.mDepth = text.mDrawOrder;
				++current_layer.mLayer;
			}
			//switch textures
			if(texture_unit != text.mTextureUnit){
				vert_range = mCurrentFrame.AddDrawCommand(*back_buf, vert_range);
				texture_unit = text.mTextureUnit;
				mCurrentFrame.AddUniformInt("uFontTexture", texture_unit);
				if(text.mActualFlags.IsSet(kTexAtlasFont)){
					mCurrentFrame.AddUniformFloat("uDirectProportion", 1.0f);
				}
				else{
					mCurrentFrame.AddUniformFloat("uDirectProportion", 0.0f);
				}
			}

			vert_range.size += static_cast<int>(text.mGlyphs.size());
			for(auto& glyph : text.mGlyphs ){
				glyph.mIndex = current_slot;
			}
			buf.WriteToBuffer(&text.mUniformData, 1);
			++current_slot;
		}

		if(vert_range.size > 0){
			vert_range = mCurrentFrame.AddDrawCommand(*back_buf, vert_range);
		}
	}

	mCurrentFrame.mLayers.emplace_back(current_layer);

	const uint32_t vert_bytes = vert_range.start*sizeof(cScreenText::cVertexData);

	while(vert_bytes > mVertexBuffers.Back().Buffer(0).Size())
	{
		mVertexBuffers.Back().Buffer(0).Grow();	
	}

	//now map the vertices
	{
		auto buf = back_buf->MapBufferRange(0, vert_bytes);
		for(auto text: mZSortedText)
		{
			UniformData(*text).mZDepth = mContext.NormalizedDepth(text->mDrawOrder);
			buf.WriteToBuffer(text->mGlyphs.data(), text->mGlyphs.size());
		}
	}
	std::copy(mCurrentFrame.mLayers.begin(),mCurrentFrame.mLayers.end(),
							std::back_inserter(layer_vec)); 
	mVertexBuffers.Swap();
}

void cScreenTextFactory::DrawSingleLayer(uint32_t layer)
{
	AKJ_ASSERT(!mHasChanges);

	mAlphaShader.Bind();
	mVertexBuffers.Front().Bind();
	mContext.SetBlendMode(cHWGraphicsContext::BLEND_ORDINARY);

	// draw the ranges for the current layer
	mCurrentFrame.RunCommands(layer);

}

void cScreenTextFactory::DrawBatches(cShaderObject& shader)
{
	
	
}

void cScreenTextFactory::PostDrawCleanup()
{
	mNeedDepthSort = true;
	mHasChanges = false;
}


void cScreenTextFactory::MakeSampleLocations(cCoord2 center)
{
	cRandom rng;
	mSampleLocations.reserve(16);
	mSampleLocations.clear();

	const float spread = 0.05f;

	const uint32_t elems = 1;


	if(elems < 2)
	{
		mSampleLocations.emplace_back(0.0f, 0.0f);
		return;
	}
	cArray<16, uint32_t> columns;
	for (uint32_t i = 0; i < elems; ++i)
	{
		columns.push_back(i);
	}
	const float incr = spread / static_cast<float>(columns.size());
	const cCoord2 corner(0.5f*(1.f - spread + incr), 0.5f + 0.5f*(spread - incr));
	
	for (uint32_t i = 0; i < elems; ++i)
	{
		uint32_t index  = rng.UInt(elems - i - 1);
		uint32_t column = columns[index];
		columns.SwapAndPop(index);
		mSampleLocations.emplace_back(corner + cCoord2(column*incr, i*incr));
	}
	for (uint32_t i = 0; i < elems; ++i)
	{
		mSampleLocations.at(i) -= cCoord2(0.5f, 0.5f)+center;
	}
}

void cScreenTextFactory::ChangeEdgeWidth(float incr)
{
	mEdgeWidth += incr;
	mDebugText.ChangeText(Twine(EdgeWidth()) + ", " + Twine(EdgeShift()));
	SetShaderBindings();
}

void cScreenTextFactory::ChangeEdgeShift(float incr)
{
	mEdgeShift += incr;
	mDebugText.ChangeText(Twine(EdgeWidth()) + ", " + Twine(EdgeShift()));
	SetShaderBindings();
}

void cScreenTextFactory::SetFont(tFaceHandle font, eBuiltinFonts font_kind)
{
	auto old_font = mBuiltinFonts.at(font_kind);
	if (font.IsValid())
	{
		try{
			tDFFHandle distance_handle = mFontLoader.DistanceHandleFrom(font);
			tTAFHandle atlas_handle = mFontLoader.TexAtlasHandleFrom(font);
			if(!distance_handle.IsValid() && !atlas_handle.IsValid())
			{
				AKJ_THROW("cannot load new "+ToString(font_kind)+ ", reverting to previous.");
			}
			mBuiltinFonts[font_kind] = font;
			LoadFontTextures(font_kind);			
			// reshape all the text
			for(auto& info: mTextRecords)
			{
				ShapeText(info, UniformData(info).mPosition);
			}
			mHasChanges = true;
		}
		catch(const Exception& e)
		{
			Log::Error("Error while trying to set a font in screenTextFactory: %s"
									, e.what());
			mBuiltinFonts.at(font_kind) = old_font;
		}

	}
}


void cScreenTextFactory::DepthSort()
{
	mNeedDepthSort = false;
	if(mHasChanges)
	{
		mZSortedText.clear();
		if(mTextRecords.empty()) return;

		for (size_t i = 0; i < mTextRecords.size() ; ++i)
		{
			cTextInfo* info = &mTextRecords[i];
			if(info->IsVisible())
			{
				mZSortedText.push_back(info);
			}
		}

		if(mZSortedText.empty()) return;

		std::sort(mZSortedText.begin(), mZSortedText.end(), PtrLess<cTextInfo>);
	}
	if(mZSortedText.empty())
	{
		return;
	}
	cWrappingCounter top_counter = mZSortedText.back()->mDrawOrder;
	cWrappingCounter bottom_counter = mZSortedText.front()->mDrawOrder;

	// make sure that the bottom text layer is the absolute bottom
	if(bottom_counter == mContext.MinDepth())
	{
		cWrappingCounter old_bottom = bottom_counter;
		bottom_counter = bottom_counter -1;
		for(auto& info: mTextRecords)
		{
			if(info.mDrawOrder == old_bottom)
			{
				old_bottom = bottom_counter;
			}
		}
	}
	mContext.AddMaxDepth(top_counter);
	mContext.AddMinDepth(bottom_counter);
	
}

void cScreenTextFactory::OnResize()
{
	SetShaderBindings();
}

void cScreenTextFactory::SetFrameTime(double abs_time)
{
	mCurrentTime = abs_time;
	mAlphaShader.BindUniformToFloat("uCurrentTime", static_cast<float>(abs_time));
}



void cScreenTextFactory::ToggleRenderMethod()
{
	mHasChanges = true;
}





} // namespace akj