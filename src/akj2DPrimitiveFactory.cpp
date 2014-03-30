#include "akj2DPrimitiveFactory.hpp"
#include "akjHelpers.hpp"
#include "akjFixedSizeVector.hpp"

#include "akjHWGraphicsContext.hpp"
#include "resources/akjStaticResources.hpp"
#include <functional>
#include "akjPixelTree.hpp"
#define AKJ_INV_SQRT2 0.707106781186547f



namespace akj
{
namespace 
{
	template <typename T> 
	bool PointerCompare(const T * const & a, const T * const & b)
	{
		return *a < *b;
	}
}
c2DPrimitiveFactory::c2DPrimitiveFactory(cHWGraphicsContext* context)
	: mStaticVertexBuffer(context, "2D Primitive buffer (static vertices)",
	((sizeof(c2DVertex)*cRoundedRectangle::kNumVertices+4095)/4096)*4096,
	STATIC_VBO)
	, mIndexBuffer(context, "2D Primitive Index Buffer",
	((sizeof(uint32_t)*cRoundedRectangle::kNumIndices+4095)/4096)*4096,
	STATIC_IBO)
	, mInstancedBufferA(context, "2D Primitive Instance buffer A",
	kInitialBufferSize, DYNAMIC_VBO)
	, mInstancedBufferB(context, "2D Primitive Instance buffer B",
	kInitialBufferSize, DYNAMIC_VBO)
	, mShader(context, "2D Primitive shader")
	, mDebugShader(context, "2D primitive debug wireframe shader")
	, mVAOA(context, "2D rounded rect Array Object A")
	, mVAOB(context, "2D rounded rect Array Object B")
	, mRenderBuffer(context, "Shape FrameBuffer",
										context->ViewPort(), pix::kRGBA8)
	, mContext(context)
	, mNewDraws(0)
	, mNumPrimitives(0)
	, mIndexCursor(0)
	, mIsDepthSorted(false)
	, mNeedBufferUpdate(true)
	, mNeedTombstoneUpdate(false)
	, mBufferSize(kInitialBufferSize)
	, mIsWireframeMode(false)
	, mTextCover({1, 1})
{
	mRenderBuffer.CreateDepthBuffer();
	mRenderBuffer.Texture().DisableInterpolation();
	c2DVertex::CheckRuntimeMemLayout();
	mZSortedShapes.reserve(1024);
	mInstanceFrontBuffer = &mInstancedBufferA;
	mInstanceBackBuffer = &mInstancedBufferB;
	mBackVBO = &mVAOB;
	mFrontVBO = &mVAOA;


	mShader
		.CreateShaderProgram(cStaticResources::get_PrimitiveShape_glsl());
	mDebugShader
		.CreateShaderProgram(cStaticResources::get_WireframeShape_glsl());

	c2DVertex::
		BindToVAO(mVAOA, mStaticVertexBuffer, mInstancedBufferA, mIndexBuffer);
	c2DVertex::
		BindToVAO(mVAOB, mStaticVertexBuffer, mInstancedBufferB, mIndexBuffer);
	InitStaticVertexData();
	mContext->SubmitFBO(mRenderBuffer.GetName(), &mRenderBuffer);
	SetShaderBindings();
}

	
void c2DPrimitiveFactory::SetShaderBindings()
{
	ivec2 vp = mContext->ViewPort();



	const float half_height = static_cast<float>(vp.y)*0.5f;
	const float half_width = static_cast<float>(vp.x)*0.5f;
	float proj[16];
	cHWGraphicsContext::MyOrtho2D(proj, 0,
		2.0f*half_width,
		2.0f*half_height, 0);
	mShader.Bind();
	mShader.BindProjectionMatrix(&proj[0]);
	mShader.UnBind();
	mDebugShader.Bind();
	mDebugShader.BindProjectionMatrix(&proj[0]);
	mDebugShader.UnBind();
}


void c2DPrimitiveFactory::InitStaticVertexData()
{
	mStaticVertexBuffer.Bind();
	std::vector<c2DVertex> verts;
	verts.reserve(cRoundedRectangle::kNumVertices);
	cRoundedRectangle::GenerateVertices(verts);
	uint32_t num_bytes = cRoundedRectangle::kNumVertices*sizeof(c2DVertex);
	c2DVertex* vert_buffer = 
		reinterpret_cast<c2DVertex*>(mStaticVertexBuffer.MapBuffer());
	memcpy(vert_buffer, verts.data(), sizeof(c2DVertex)*verts.size());
	mStaticVertexBuffer.UnMapBuffer();

	std::vector<uint32_t> indices;
	indices.reserve(cRoundedRectangle::kNumIndices);
	cRoundedRectangle::GenerateIndices(0, indices);
	num_bytes = cRoundedRectangle::kNumIndices*sizeof(uint32_t);
	mIndexBuffer.SetData(num_bytes, indices.data(), 0);
	mIndexBuffer.UnBind();
}

cRoundedRectangle c2DPrimitiveFactory::
	CreateRoundedRect(const float left, const float right,
										const float top,const float bottom, 
										const float stroke_width, const float radius,
										RGBAu8 stroke_color, RGBAu8 fill_color)
{
		cShapeInfo& new_shape = AllocateShape(ROUNDED_RECT);
		c2DVertex::Instanced& instance_data = new_shape.mInstanceData;

		const cCoord2 half_size = 
			cCoord2((right - left)*0.5f, (bottom - top)*0.5f);
		const cCoord2 center = half_size + cCoord2(left, top);
			
		instance_data.mPosition = center;
		instance_data.mHalfSize = half_size;
		instance_data.mStrokeWidth = stroke_width*0.5f;
		instance_data.mStrokeColor = stroke_color;
		instance_data.mFillColor = fill_color;
		const float radius_adjust = radius <= 0.0f ? -0.5f*stroke_width : 0.0f;
		instance_data.mCornerRadius = radius+radius_adjust;
		instance_data.mFillCoord = cCoord2(0.0f, 0.0f);
		instance_data.mExtraFillAlpha = 0.0f;
		new_shape.AdjustOcclusion();

		return cRoundedRectangle(this, new_shape.mID);
}

void c2DPrimitiveFactory::cShapeInfo::AdjustOcclusion()
{
	const float stroke_width = mInstanceData.mStrokeWidth;
	const cCoord2 tl = mInstanceData.mPosition - mInstanceData.mHalfSize;
	const cCoord2 br = mInstanceData.mPosition + mInstanceData.mHalfSize;
	if (RGBAu8(mInstanceData.mFillColor).a < 255)
	{
		// transparent, no occlusion
		mOcclusionRect.width = 0;
		mOcclusionRect.height = 0;
		mOcclusionRect.top = -1;
		mOcclusionRect.left = -1;
	}
	else
	{
		// find the largest rectangle that can be fully inscribed on the
		// actual rounded rectangle.
		// goes on the idea that you can fit a 1/sqrt(2) sized square in
		// a circle of unit radius
		//TODO: verify for negative radii
		const RGBAu8 s_color = RGBAu8(mInstanceData.mStrokeColor);
		const float strk = s_color.a == 255 ? stroke_width : 0.0f;
		const float ocl_radius 
			= (1.0f - AKJ_INV_SQRT2)*(mInstanceData.mCornerRadius + strk);
		int ileft = static_cast<int>(std::ceil(tl.x - strk + ocl_radius));
		int iright = static_cast<int>(std::floor(br.x + strk - ocl_radius));
		int itop = static_cast<int>(std::ceil(tl.y - strk + ocl_radius));
		int ibottom = static_cast<int>(std::floor(br.y + strk - ocl_radius));
		mOcclusionRect.width = std::max(iright - ileft, 0);
		mOcclusionRect.height = std::max(ibottom - itop, 0);
		mOcclusionRect.left = ileft;
		mOcclusionRect.top = itop;
	}

	mCoverageRect.left =
		static_cast<int>(std::floor(tl.x - stroke_width - 2));
	mCoverageRect.width
		= static_cast<int>(std::ceil(br.x + stroke_width + 2))
		- mCoverageRect.left;

	mCoverageRect.top =
		static_cast<int>(std::floor(tl.y - stroke_width - 2));
	mCoverageRect.height
		= static_cast<int>(std::ceil(br.y + stroke_width + 2))
		- mCoverageRect.top;
}

cRoundedRectangle c2DPrimitiveFactory::
	CreateRoundedRect(const cRoundedRectangle& other_rect)
{
	cShapeInfo& new_shape = AllocateShape(ROUNDED_RECT);

	const cShapeInfo& old_shape = mShapeRecords.at(other_rect.mID);
	new_shape.mInstanceData = old_shape.mInstanceData;
	new_shape.mCoverageRect = old_shape.mCoverageRect;
	new_shape.mOcclusionRect = old_shape.mOcclusionRect;
	new_shape.mIsHidden = old_shape.mIsHidden;

	return cRoundedRectangle(this, new_shape.mID);
}


c2DPrimitiveFactory::cShapeInfo& c2DPrimitiveFactory::
	AllocateShape( eShapeKind kind )
{
	const uint32_t id = GetNextShapeID();
		
	cShapeInfo& new_shape = mShapeRecords.at(id);
	new_shape.mID = id;
	new_shape.mReferences = 0;
	new_shape.mIsHidden = false;
	new_shape.mZOrder = mContext->DrawCount();
	return new_shape;
}

uint32_t c2DPrimitiveFactory::GetNextShapeID()
{
	mNewDraws++;
	//first, try recycling ids
	if(!mTombstones.empty())
	{
		uint32_t index = mTombstones.back();
		mTombstones.pop_back();
		return index;
	}
	//otherwise, make a new one
	const uint32_t index = static_cast<uint32_t>(mShapeRecords.size());
	mShapeRecords.emplace_back();
	return index;
}

void c2DPrimitiveFactory::ShapeDestroyed(uint32_t id)
{
	AKJ_ASSERT(mShapeRecords.at(id).mReferences >= 1);
	mShapeRecords.at(id).mReferences--;
	if(mShapeRecords.at(id).mReferences == 0){
		mNewDraws++;
		AddTombStone(id);
		mShapeRecords.at(id).mText = cMutableArrayRef<cScreenText>();
		//Log::Debug("Destroyed shape %d, x = %f", id, 
		//	mShapeRecords.at(id).mInstanceData.mHalfSize.x);
	}
	//Log::Debug("Destroyed a reference to shape %d, now with %d references",
	//	id, mShapeRecords.at(id).mReferences );
		
}

void c2DPrimitiveFactory::ShapeCreated(uint32_t id)
{
	mShapeRecords.at(id).mReferences++;
	if(mShapeRecords.at(id).mReferences == 1)
	{
		//Log::Debug("Created shape %d, x = %f", id, 
		//	mShapeRecords.at(id).mInstanceData.mHalfSize.x );
	}
	//
	//	id, mShapeRecords.at(id).mReferences);
}

c2DPrimitiveFactory::~c2DPrimitiveFactory()
{

}

void c2DPrimitiveFactory::GrowBuffers(uint32_t max_bytes)
{
	while (mBufferSize < max_bytes)
	{
		mBufferSize *= 2;
	}
	if(mBufferSize > kMaxBufferSize)
	{
		FatalError::Die("Tried to allocate too much video memory. "
			"("AKJ_STRINGIFY(__LINE__)")" __FILE__ );
	}
	mInstancedBufferA.ResetData(mBufferSize);
	mInstancedBufferB.ResetData(mBufferSize);
	mContext->AssertErrorFree("Primitive Vector Growth");
}

void c2DPrimitiveFactory::DepthSort()
{
	mIsDepthSorted = true;
	if(mNewDraws == 0)
	{
		//early out if there is nothing to do
		if(!mZSortedShapes.empty())
		{
			cWrappingCounter bottom_counter = mZSortedShapes.back()->mZOrder;
			cWrappingCounter top_counter = mZSortedShapes.front()->mZOrder;
			mContext->AddMaxDepth(top_counter);
			mContext->AddMinDepth(bottom_counter);
		}
		return;
	}
	mNumPrimitives = 0;
	mZSortedShapes.clear();
	mShapesToDrawTemp.clear();
	for (cShapeInfo& shape : mShapeRecords)
	{
		if(shape.mReferences>0 && !shape.mIsHidden)
		{
			mShapesToDrawTemp.push_back(&shape);
		}
	}

	if (mShapesToDrawTemp.empty())
	{
		return;
	}

	std::sort(mShapesToDrawTemp.begin(), mShapesToDrawTemp.end(), 
		PointerCompare<cShapeInfo>);

	cWrappingCounter top_counter = mShapesToDrawTemp.back()->mZOrder;
	cWrappingCounter bottom_counter = mShapesToDrawTemp.front()->mZOrder;
	mContext->AddMaxDepth(top_counter);
	mContext->AddMinDepth(bottom_counter);

	if(mShapesToDrawTemp.size() > 100)
	{
		// only keep the non-occluded rectangles
		cSparseCoverageMap& coverage = mContext->Coverage();
		for (int i = static_cast<int>(mShapesToDrawTemp.size())-1;
			i >=0; --i)
		{
			cShapeInfo* shape = mShapesToDrawTemp.at(i);
			if (!coverage.IsFullyOccluded(shape->mCoverageRect))
			{
				// not totally occluded
				coverage.AddRect(shape->mOcclusionRect);
				mZSortedShapes.push_back(shape);
			}
		}
	}
	else
	{
		// don't bother with the fancy occlusion rect in this case
		for (int i = static_cast<int>(mShapesToDrawTemp.size())-1; i >=0; --i)
		{
			mZSortedShapes.push_back(mShapesToDrawTemp[i]);
		}
	}
		

	mShapesToDrawTemp.clear();
}

void c2DPrimitiveFactory::PrepareLayers(std::vector<cSubmittedLayer>& layers)
{
	AKJ_ASSERT(mIsDepthSorted);
	// nothing to draw!
	if(mZSortedShapes.empty())
	{
		mNewDraws = 0;
		return;
	}
	// nothing new to draw, so just do the same as before
	else if(mNewDraws == 0)
	{
		std::copy(	
			mCurrentDraw.mLayers.begin(), 
			mCurrentDraw.mLayers.end(),
			std::back_inserter(layers)
		);
		return;
	}
	// do the full buffer mapping

	mNewDraws = 0;
	mCurrentDraw.Reset();
	const uint32_t max_bytes = static_cast<uint32_t>(
		mZSortedShapes.size()*sizeof(c2DVertex::Instanced)
	);

	if (max_bytes > mBufferSize)
	{
		Log::Warn(
			"Exceeded our buffer capacity (% bytes) by trying to draw "
			"%d bytes (% primitives)", mBufferSize, max_bytes, mZSortedShapes.size()
		);
		GrowBuffers(max_bytes);
	}
	
	cShapeInfo** shape_ptr = &mZSortedShapes.back();
	cShapeInfo** shape_ptr_start = &mZSortedShapes.front() - 1;
	std::vector<cBufferRange>& ranges = mCurrentDraw.mDrawRanges;
	cArrayBuffer* back_buf = mInstanceBackBuffer;
	cSmallVector<const cScreenText*, 1024> found_text;
	cSubmittedLayer current_layer;
	current_layer.mDrawable = this;
	current_layer.mLayer = 0;
	int this_layer_count = 0;
	int layer_offset = 0;
	mTextCover.Reset(mContext->ViewPort());
	while(shape_ptr > shape_ptr_start)
	{
		cShapeInfo& shape = *(*shape_ptr--);
		// want to start a new layer whenever text is occluded by a rectangle
		// we also want to change the layer of all encountered text
		// so that it's on the same layer as the top non-occluding rectangle
		if(!found_text.empty() && mTextCover.HasIntersection(shape.mCoverageRect))
		{
			ranges.emplace_back(*back_buf, layer_offset, this_layer_count);
			current_layer.mDepth = shape.mZOrder - 1;
			mCurrentDraw.mLayers.push_back(current_layer);
			++current_layer.mLayer;
			layer_offset += this_layer_count;
			this_layer_count = 0;
			for (uint32_t i = 0; i < found_text.size(); i++)
			{
				found_text[i]->Depth(current_layer.mDepth);
			}
			found_text.clear();
			mTextCover.Clear();
		}
		if(!shape.mText.empty())
		{
			for(auto& text : shape.mText)
			{
				found_text.push_back(&text);
				mTextCover.AddRect(text.BoundingBox());
			} 
		}
		this_layer_count++;
	}
	//place the back layer
	ranges.emplace_back(*back_buf, layer_offset, this_layer_count);
	current_layer.mDepth = mZSortedShapes.back()->mZOrder;
	mCurrentDraw.mLayers.push_back(current_layer);
	for (uint32_t i = 0; i < found_text.size(); i++)
	{
		found_text[i]->Depth(current_layer.mDepth);
	}
	
	auto mapped_range = mInstanceBackBuffer->MapBufferRange(0, max_bytes);
					
	for (int i = static_cast<int>(mZSortedShapes.size()) - 1;
		i >= 0; --i)
	{
		cShapeInfo* shape = mZSortedShapes.at(i);
		// set the z depth (taking into account all the other z depths)
		shape->mInstanceData.mDepth = mContext->NormalizedDepth(shape->mZOrder);
		mNumPrimitives++;
		mapped_range.WriteToBuffer(&shape->mInstanceData, 1);
	}
	
	std::copy(	
			mCurrentDraw.mLayers.begin(), 
			mCurrentDraw.mLayers.end(),
			std::back_inserter(layers)
	);

	//swap the doubled buffers
	cArrayBuffer* temp = mInstanceBackBuffer;
	mInstanceBackBuffer = mInstanceFrontBuffer;
	mInstanceFrontBuffer = temp;
	cVertexArrayObject* temp2 = mBackVBO;
	mBackVBO = mFrontVBO;
	mFrontVBO = temp2;
	
}

void c2DPrimitiveFactory::DrawSingleLayer(uint32_t layer)
{
	DrawWithShader(mShader, layer);
	if(mIsWireframeMode)
	{
		DrawWithShader(mDebugShader, layer);
	}
}
	
void c2DPrimitiveFactory::MarkChanged(uint32_t id)
{
	if(!mShapeRecords.at(id).mIsHidden)
	{
		mNewDraws++;
	}
}


void c2DPrimitiveFactory::DrawWireFrame(uint32_t layer)
{
	DrawWithShader(mDebugShader, layer);
}

void c2DPrimitiveFactory::DrawWithShader(cShaderObject& shader, uint32_t layer)
{
	shader.Bind();
	mFrontVBO->Bind();
	cBufferRange& to_draw = mCurrentDraw.RangeForLayer(layer);

	mStaticVertexBuffer.DrawIndexedInstanced	(
		cRoundedRectangle::kNumIndices, to_draw.start, to_draw.size
	);

	mFrontVBO->UnBind();
	
}

void c2DPrimitiveFactory::PostDrawCleanup()
{
	mIsDepthSorted = false;
	mNewDraws = 0;

	if(mNeedTombstoneUpdate)
	{
		ProcessTombstones();
	}	
}

void c2DPrimitiveFactory::ProcessTombstones()
{
	mNeedTombstoneUpdate = false;
	std::sort(mTombstones.begin(), mTombstones.end());


	//get rid of the tombstones on the end (shrink the id list if possible)
	while(!mTombstones.empty() && mTombstones.back() == mShapeRecords.size()-1)
	{
		mTombstones.pop_back();
		mShapeRecords.pop_back();
	}

	// sort again so that used things can just be popped off the back
	std::sort(mTombstones.begin(), mTombstones.end(), std::greater<uint32_t>());

	//mark all the tombstones as found, so they don't get re-added
	for(uint32_t i: mTombstones)
	{
		//AKJ_ASSERT(mShapeRecords.at(i).mReferences == 0);
		mShapeRecords.at(i).mReferences = -1;
	}
}

void c2DPrimitiveFactory::SaveDepthBuffer()
{
	cAlignedBuffer buffer;
	auto bitmap = mRenderBuffer.SaveDepthBuffer(buffer);
	if(!bitmap.IsValid()) return;

	bitmap.ExportPNG("primitive_depth.png");
}
	
void c2DPrimitiveFactory::SetFrameBufferTarget()
{
	ivec2 vp = mContext->ViewPort();
	mRenderBuffer.BindForWrite();
	mContext->SetViewPort(0, 0, vp.x, vp.y);
	auto scoped_bg = mContext->PushBackGroundColor(RGBAu8(0, 0, 0, 0));
	mContext->Clear();
	mContext->ClearDepth(1.1f);
}

void c2DPrimitiveFactory::AddTombStone(uint32_t id)
{
	mTombstones.push_back(id);
	mShapeRecords.at(id).mReferences = -1;
	mNeedTombstoneUpdate = true;
}

void c2DPrimitiveFactory::SetChildText(uint32_t id, cArrayRef<cScreenText> text)
{
	cShapeInfo& info = mShapeRecords.at(id);
	info.mText = text;
	if(!info.mIsHidden)
	{
		++mNewDraws; 
	}
}

void c2DPrimitiveFactory::SetFrameTime(double abs_time)
{
	mCurrentTime = abs_time;
	mShader.BindUniformToFloat("uCurrentTime", static_cast<float>(abs_time));
}




} // namespace akj