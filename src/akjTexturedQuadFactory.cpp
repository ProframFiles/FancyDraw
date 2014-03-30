#include "akjTexturedQuadFactory.hpp"
#include "resources/akjStaticResources.hpp"

namespace akj
{




	void cTexturedQuadFactory::DrawSingleLayer(uint32_t layer)
	{
		mVisibleState.RunCommands(layer);
	}

	void cTexturedQuadFactory::AddReference(cQuadInfo& info)
	{
		AKJ_ASSERT(info.mReferences >= 0);
		if(++info.mReferences == 1)
		{
			MarkChanged(info);
		}
	}

	void cTexturedQuadFactory::RemoveReference(cQuadInfo& info)
	{
		AKJ_ASSERT(info.mReferences >0);
		if(--info.mReferences == 0)
		{
			DestroyInfo(info);
		}
	}

	void cTexturedQuadFactory::PrepareLayers(std::vector<cSubmittedLayer>& layers)
	{
		if(!mVisibleState.mHasChanges)
		{
			std::copy(
				mVisibleState.mLayers.begin()
				,mVisibleState.mLayers.end()
				,std::back_inserter(layers)
			);
			return;
		}
		mVisibleState.Reset(mVertexBuffers.Back(), &mShader, this);
		auto last = mZSortedInfo.begin();
		mVisibleState.StartLayer((*last)->mDrawOrder);
		mVisibleState.NewTexture((*last)->mTexture, 0);
		uint32_t num_quads = 0;
		for(auto it = mZSortedInfo.begin(); it != mZSortedInfo.end(); ++it)
		{
			if((*it)->mTexture != (*last)->mTexture)
			{
				mVisibleState.NewTexture((*it)->mTexture, num_quads);
				num_quads = 0;
			}
			if((*it)->mDrawOrder != (*last)->mDrawOrder)
			{
				mVisibleState.NewLayer((*it)->mDrawOrder, num_quads);
				num_quads = 0;
			}
			last = it;
			++num_quads;
		}
		const uint32_t total_quads = mVisibleState.FinishLayers(num_quads);
		const uint32_t total_bytes = sizeof(cPosTexCoord)*4*total_quads;


		//map vertices into buffer
		{
			auto mapped = mVertexBuffers.Back().Buffer(0).MapBytes(total_bytes);
			for(auto it = mZSortedInfo.begin(); it != mZSortedInfo.end(); ++it)
			{
				mapped.WriteToBuffer((*it)->mVerts);
			}
		}

		std::copy(
				mVisibleState.mLayers.begin()
				,mVisibleState.mLayers.end()
				,std::back_inserter(layers)
			);
	}

	cQuadInfo& cTexturedQuadFactory::AllocateInfo()
	{
		cQuadInfo& info = mQuadRecords.New();
		info.mDrawOrder = 0;
		info.mIsVisible = true;
		return info;
	}

	void cTexturedQuadFactory::DestroyInfo(cQuadInfo& info)
	{
		mQuadRecords.erase(info.mID);
	}

		template <typename T> 
	bool TexLess(const T * const & a, const T * const & b)
	{
		return (*a).mTexture < (*b).mTexture;
	}


	void cTexturedQuadFactory::DepthSort()
	{
		if(!mVisibleState.mHasChanges)
		{
			return;
		}
		mZSortedInfo.clear();
		for(auto& quad : mQuadRecords)
		{
			if(quad.mIsVisible && quad.mReferences > 0)
			{
				mZSortedInfo.push_back(&quad);
			}
		}
		if(mZSortedInfo.empty()) return;
		std::sort(mZSortedInfo.begin(), mZSortedInfo.end(), PtrLess<cQuadInfo>);

		cWrappingCounter counter = mZSortedInfo.back()->mDrawOrder;
		auto start_it = mZSortedInfo.begin();

		//now sort the sub-ranges according to texture
		for(auto it = mZSortedInfo.begin(); it != mZSortedInfo.end(); ++it)
		{
			if(counter != (*it)->mDrawOrder)
			{
				std::sort(start_it, it, TexLess<cQuadInfo>);
				counter = (*it)->mDrawOrder;
				start_it = it;
			}
		}
		std::sort(start_it, mZSortedInfo.end(), TexLess<cQuadInfo>);
		//w00t, all done
		
		mContext.AddMinDepth(mZSortedInfo.front()->mDrawOrder);
		mContext.AddMinDepth(mZSortedInfo.back()->mDrawOrder);

	}

	cTexturedQuadFactory::~cTexturedQuadFactory()
	{

	}

	void cTexturedQuadFactory::PostDrawCleanup()
	{
		mVisibleState.mHasChanges = false;
	}

	cTexturedQuadFactory::cTexturedQuadFactory(cHWGraphicsContext& context)
			:	mContext(context)
			, mShader(&context, "Quad shader")
			, mVertexBuffers(&context, "Texture Quad Factory VAO",
				{cGLBufferDesc(DYNAMIC_VBO, kInitBufferSize)},
				{cVertexAttribute(ATTR_FLOAT, 2, 16, 0, 0), // vec2 pos;
				cVertexAttribute(ATTR_FLOAT, 2, 16, 0, 0)}) // vec2 texcoord;
			, mDepthCounter(0)
	{
		mShader.CreateShaderProgram(cStaticResources::get_TexturedQuad_glsl());
		SetShaderBindings();
	}

	akj::tTexHandle cTexturedQuadFactory::LoadTexture(cAlignedBitmap bitmap, const Twine& name)
	{
		tTexHandle handle(u32(mTextures.size()));
		mTextures.emplace_back(new cTextureObject(&mContext, name + " Texture"));
		mTextures.back()->CreateTexture2D(bitmap, false, 0);
		mTexBitmaps.emplace_back(cStoredBitmap::Copy(bitmap));
		return handle;
	}

	akj::cTexturedQuad cTexturedQuadFactory::CreateTexturedQuad(iRect rect, tTexHandle texture)
	{
		if(!texture.IsValid()){
			AKJ_THROW("Bad texture handle: " + Twine(texture.id));
		}
		uint32_t tex_unit = mTextures.at(texture)->GetBoundTextureUnit();
		cQuadInfo& info = AllocateInfo();
		info.mPos = cCoord2(static_cast<float>(rect.left), static_cast<float>(rect.top));
		info.mTexture = mTextures.at(texture).get();
		const float w = static_cast<float>(rect.width);
		const float h = static_cast<float>(rect.height);
		cPosTexCoord vert;
		vert.pos = info.mPos;
		vert.texCoord = {0.0f, 0.0f};
		info.mVerts.push_back(vert);
		vert.pos.x += w;
		vert.texCoord.x += w;
		info.mVerts.push_back(vert);
		vert.pos.y += h;
		vert.texCoord.y += h;
		vert.pos.x -= w;
		vert.texCoord.x -= w;
		info.mVerts.push_back(vert);
		vert.pos.x += w;
		vert.texCoord.x += w;
		info.mVerts.push_back(vert);
		return cTexturedQuad(*this,  info.mID);
	}


void cTexturedQuadFactory::SetShaderBindings()
{
	ivec2 vp = mContext.ViewPort();
	float proj[16];
	cHWGraphicsContext::MyOrtho2D(proj, 0,
		vp.width,
		vp.height, 0);
	mShader.Bind();
	mShader.BindProjectionMatrix(&proj[0]);
	mShader.UnBind();
}

} // namespace akj