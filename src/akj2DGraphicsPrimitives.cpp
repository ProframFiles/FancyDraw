#include "akj2DGraphicsPrimitives.hpp"
#include "akjFixedSizeVector.hpp"
#include "akj2DPrimitiveFactory.hpp"
#include "akjHWGraphicsContext.hpp"
#include "akjScreenTextFactory.hpp"
#include "akjTexturedQuadFactory.hpp"


namespace akj
{
	void cRoundedRectangle::GenerateVertices(std::vector<c2DVertex>& vec)
	{
			// TODO calculate how this works with round edges
			cCoord2 outer_offset(1.0f, 1.0f);
			cCoord2 inner_offset(-1.0f, -1.0f);
			size_t vert_index = vec.size();
			vec.resize(vec.size() + kNumVertices);
			
			// start at top left inside, traverse the rectangle clockwise
			// create this direction array to ease looping over the vertices
			cArray<4, cCoord2> directions;
			directions.emplace_back(-1.0f, -1.0f);
			directions.emplace_back(1.0f, -1.0f);
			directions.emplace_back(1.0f, 1.0f);
			directions.emplace_back(-1.0f, 1.0f);
			for (uint32_t i = 0; i < directions.size() ; ++i)
			{
				const cCoord2 corner_direction = directions.at(i);
				const cCoord2 corner_pos = corner_direction;

				c2DVertex* next_vert;

				next_vert = &vec.at(4*i + vert_index + 0);
				next_vert->mTexCoord = corner_pos;
				next_vert->mShiftDirection = inner_offset*corner_direction;

				next_vert = &vec.at(4*i + vert_index + 1);
				next_vert->mTexCoord = corner_pos;
				next_vert->mShiftDirection = outer_offset*corner_direction;

				next_vert = &vec.at(4*i + vert_index + 2);
				next_vert->mTexCoord = corner_pos;
				next_vert->mShiftDirection = inner_offset*corner_direction;

				next_vert = &vec.at(4*i + vert_index + 3);
				next_vert->mTexCoord = corner_pos;
				next_vert->mShiftDirection =  outer_offset*corner_direction;
			}
	}
	cRoundedRectangle::cRoundedRectangle()
		: mFactory(NULL)
		, mID(-1)
	{}

	cRoundedRectangle::
		cRoundedRectangle(c2DPrimitiveFactory* factory, uint32_t id)
		: mFactory(factory)
		, mID(id)
		{
			mFactory->ShapeCreated(mID);
		}

	cRoundedRectangle::cRoundedRectangle(cRoundedRectangle&& other)
		:mFactory(other.mFactory), mID(other.mID)
	{
		other.Release();
	}

	cRoundedRectangle::cRoundedRectangle(const cRoundedRectangle& other)
		: mFactory(other.mFactory), mID(other.mID)
	{
		mFactory->ShapeCreated(mID);
	}


	cRoundedRectangle::~cRoundedRectangle()
	{
		if (mFactory)
		{
			mFactory->ShapeDestroyed(mID);
		}
	}

	cRoundedRectangle& cRoundedRectangle::
		operator =(cRoundedRectangle&& other)
	{
		if(other.mID != this->mID)
		{
			if (mFactory)
			{
				mFactory->ShapeDestroyed(mID);
			}
			this->mID = other.mID;
			this->mFactory = other.mFactory;
			other.Release();
		}
		return *this;
	}

	cRoundedRectangle& cRoundedRectangle::
		operator =(const cRoundedRectangle& other)
	{
			if (other.mID != this->mID)
			{
				if (mFactory)
				{
					mFactory->ShapeDestroyed(mID);
				}
				this->mID = other.mID;
				this->mFactory = other.mFactory;
				mFactory->ShapeCreated(mID);
			}
			return *this;
	}

	void cRoundedRectangle::
		GenerateIndices(uint32_t start, std::vector<uint32_t>& vec)
	{
			cHWGraphicsContext::GenerateTriStripIndices(start, 15, vec);
			vec[vec.size() - 1] = start;

			vec.push_back(start + 0);
			vec.push_back(start + 15);
			vec.push_back(start + 1);

			vec.push_back(start + 14);
			vec.push_back(start + 0);
			vec.push_back(start + 2);

			vec.push_back(start + 12);
			vec.push_back(start + 14);
			vec.push_back(start + 2);

			vec.push_back(start + 12);
			vec.push_back(start + 2);
			vec.push_back(start + 4);

			vec.push_back(start + 10);
			vec.push_back(start + 12);
			vec.push_back(start + 4);

			vec.push_back(start + 10);
			vec.push_back(start + 4);
			vec.push_back(start + 6);

			vec.push_back(start + 8);
			vec.push_back(start + 10);
			vec.push_back(start + 6);
		}

	cRoundedRectangle& cRoundedRectangle::StrokeWidth(float width)
	{
		mFactory->mShapeRecords[mID].mInstanceData.mStrokeWidth = 0.5f*width;
		mFactory->MarkChanged(mID);
		return *this;
	}

	cRoundedRectangle& cRoundedRectangle::CornerRadius(float radius)
	{
		mFactory->mShapeRecords[mID].mInstanceData.mCornerRadius = radius;
		mFactory->MarkChanged(mID);
		return *this;
	}

	cRoundedRectangle& cRoundedRectangle::Center(cCoord2 xy)
	{
		mFactory->mShapeRecords[mID].mInstanceData.mPosition = xy;
		mFactory->MarkChanged(mID);
		return *this;
	}

	cRoundedRectangle& cRoundedRectangle::Hide()
	{
		mFactory->MarkChanged(mID);
		mFactory->mShapeRecords.at(mID).mIsHidden = true;
		return *this;
	}

	cRoundedRectangle& cRoundedRectangle::Show()
	{
		mFactory->mShapeRecords.at(mID).mIsHidden = false;
		mFactory->MarkChanged(mID);
		return *this;
	}

	cRoundedRectangle& cRoundedRectangle::StrokeColor(RGBAu8 color)
	{
		mFactory->mShapeRecords[mID].mInstanceData.mStrokeColor = color;
		mFactory->MarkChanged(mID);
		return *this;
	}

	cRoundedRectangle& cRoundedRectangle::StrokeAlpha(uint8_t alpha)
	{
		auto& instance_data = mFactory->mShapeRecords[mID].mInstanceData;
		RGBAu8 color(instance_data.mStrokeColor);
		instance_data.mStrokeColor = color.Alpha(alpha);
		mFactory->MarkChanged(mID);
		if(color.a == 255 && alpha != 255)
		{
			mFactory->mShapeRecords[mID].AdjustOcclusion();
		}
		return *this;
	}


	cRoundedRectangle& cRoundedRectangle::FillColor(RGBAu8 color)
	{
		mFactory->mShapeRecords[mID].mInstanceData.mFillColor = color;
		mFactory->MarkChanged(mID);
		return *this;
	}

	cRoundedRectangle& cRoundedRectangle::FillAlpha(uint8_t alpha)
	{
		auto& instance_data = mFactory->mShapeRecords[mID].mInstanceData;
		RGBAu8 color(instance_data.mFillColor);
		instance_data.mFillColor = color.Alpha(alpha);
		mFactory->MarkChanged(mID);
		if(color.a == 255 && alpha != 255)
		{
			mFactory->mShapeRecords[mID].AdjustOcclusion();
		}
		return *this;
	}


	const iRect& cRoundedRectangle::OccludedRect() const
	{
		return mFactory->mShapeRecords.at(mID).mOcclusionRect;
	}

	bool cRoundedRectangle::IsVisible() const
	{
		return !mFactory->mShapeRecords.at(mID).mIsHidden;
	}

	cRoundedRectangle& cRoundedRectangle::TopLeft(cCoord2 pos)
	{
		c2DVertex::Instanced& data = mFactory->mShapeRecords[mID].mInstanceData;
		const cCoord2 half_delta = (pos - (data.mPosition - data.mHalfSize))*0.5f;
		data.mPosition += half_delta;
		data.mHalfSize -= half_delta;
		mFactory->mShapeRecords[mID].AdjustOcclusion();
		mFactory->MarkChanged(mID);
		return *this;
	}

	cCoord2 cRoundedRectangle::TopLeft() const
	{
		const c2DVertex::Instanced& data 
			= mFactory->mShapeRecords[mID].mInstanceData;
		return (data.mPosition - data.mHalfSize);
	}

	cCoord2 cRoundedRectangle::BottomRight() const
	{
		const c2DVertex::Instanced& data 
			= mFactory->mShapeRecords[mID].mInstanceData;
		return (data.mPosition + data.mHalfSize);
	}

	cRoundedRectangle& cRoundedRectangle::BottomRight(cCoord2 pos)
	{
		c2DVertex::Instanced& data = mFactory->mShapeRecords[mID].mInstanceData;
		const cCoord2 half_delta = (pos - (data.mPosition + data.mHalfSize))*0.5f;
		data.mPosition += half_delta;
		data.mHalfSize += half_delta;
		mFactory->mShapeRecords[mID].AdjustOcclusion();
		mFactory->MarkChanged(mID);
		return *this;
	}

	const cRoundedRectangle& cRoundedRectangle::Move(const cCoord2& amount) const
	{
		auto& data = InstanceData();
		data.mPosition += amount;
		mFactory->mShapeRecords[mID].AdjustOcclusion();
		mFactory->MarkChanged(mID);
		return *this;
	}



	akj::cRoundedRectangle cRoundedRectangle::Duplicate()
	{
		return mFactory->CreateRoundedRect(*this);
	}

const cRoundedRectangle& cRoundedRectangle::AddText(cArrayRef<cScreenText> text) const
{
	mFactory->SetChildText(mID, text);
	const cWrappingCounter depth = Depth();
	for(auto& val: text)
	{
		val.Depth(depth);
	}
	return *this;
}

	akj::cWrappingCounter cRoundedRectangle::Depth() const
	{
		return mFactory->mShapeRecords.at(mID).mZOrder;
	}

	c2DVertex::Instanced& cRoundedRectangle::InstanceData() const
	{
		return mFactory->mShapeRecords[mID].mInstanceData;
	}



		cScreenText::cScreenText()
	: mFactory(NULL)
	, mID(-1)
{}

cScreenText::
	cScreenText(cScreenTextFactory* factory, uint32_t id)
	: mFactory(factory)
	, mID(id)
	{
		mFactory->AddReference(mID);
	}

cScreenText::cScreenText(cScreenText&& other)
	:mFactory(other.mFactory), mID(other.mID)
{
	other.Release();
}

cScreenText::cScreenText(const cScreenText& other)
	: mFactory(other.mFactory), mID(other.mID)
{
	mFactory->AddReference(mID);
}


cScreenText::~cScreenText()
{
	if (mFactory)
	{
		mFactory->RemoveReference(mID);
	}
}

cScreenText& cScreenText::
	operator =(cScreenText&& other)
{
	if(other.mID != this->mID || mFactory != other.mFactory)
	{
		if (mFactory)
		{
			mFactory->RemoveReference(mID);
		}
	
		this->mID = other.mID;
		this->mFactory = other.mFactory;
		other.Release();
	}
	return *this;
}

cScreenText& cScreenText::
	operator =(const cScreenText& other)
{
	if (other.mID != this->mID || mFactory != other.mFactory)
	{
		if (mFactory)
		{
			mFactory->RemoveReference(mID);
		}
		this->mID = other.mID;
		this->mFactory = other.mFactory;
		mFactory->AddReference(mID);
	}
	return *this;
}

cTextInfo& cScreenText::Info() const
{
	return mFactory->mTextRecords.at(mID);
}

const cScreenText& cScreenText::Move(const cCoord2& shift) const
{
	mFactory->UniformData(mID).mPosition += shift;
	mFactory->MarkAsChanged(mID);
	return *this;
}

const cScreenText& cScreenText::MoveTo(const cCoord2& pos) const
{
	mFactory->UniformData(mID).mPosition = pos;
	mFactory->MarkAsChanged(mID);
	return *this;
}

const cScreenText& cScreenText::ChangeText(const Twine& text) const 
{
	mFactory->ChangeText(mID, text);
	mFactory->MarkAsChanged(mID);
	return *this;
}

akj::iRect cScreenText::BoundingBox() const
{
	return mFactory->mTextRecords.at(mID).mBoundingBox;
}

const cScreenText& cScreenText::Depth(cWrappingCounter depth) const
{
	mFactory->mTextRecords.at(mID).mDrawOrder = depth;
	return *this;
}

const cScreenText& cScreenText::FillAlpha(float val) const
{
	UniformData().mSolidColor.a = static_cast<uint8_t>(val*255.99f);
	mFactory->MarkAsChanged(mID);
	return *this;
}

const cScreenText& cScreenText::Fill(RGBAu8 color) const
{
	UniformData().mSolidColor = color;
	mFactory->MarkAsChanged(mID);
	return *this;
}

const cScreenText& cScreenText::Hide() const 
{
	mFactory->MarkAsChanged(mID);
	Info().SetInvisible();
	return *this;
}

const cScreenText& cScreenText::Show() const 
{
	Info().SetVisible();
	mFactory->MarkAsChanged(mID);
	return *this;
}

cScreenText& cScreenText::ChangeTimef(float time_change)
{
	mFactory->UniformData(mID).mStartTime -= time_change;
	mFactory->MarkAsChanged(mID);
	return *this;
}

const cCoord2& cScreenText::Pos() const 
{
	return UniformData().mPosition;
}

cScreenText::cInstanceData& cScreenText::UniformData() const
{
	return mFactory->UniformData(Info());
}




cTexturedQuad::cTexturedQuad() 
	: mParent(nullptr)
	, mID(-1)
{

}

cTexturedQuad::cTexturedQuad(cTexturedQuad&& other) 
	: mParent(other.mParent)
	, mID(other.mID)
{
	other.Release();
}

cTexturedQuad::cTexturedQuad(cTexturedQuadFactory& factory, uint32_t id) 
	: mParent(&factory)
	, mID(id)
{
	mParent->AddReference(Info());
}

cTexturedQuad::cTexturedQuad(const cTexturedQuad& other) 
	: mParent(other.mParent), mID(other.mID)
{
	mParent->AddReference(Info());
}

cTexturedQuadFactory* cTexturedQuad::Release()
{
	cTexturedQuadFactory* temp = mParent;
	mParent = nullptr;
	return temp;
}

cTexturedQuad& cTexturedQuad::operator=(const cTexturedQuad& other)
{
	if (other.mID != this->mID || mParent != other.mParent)
	{
		if (mParent)
		{
			mParent->RemoveReference(Info());
		}
		this->mID = other.mID;
		this->mParent = other.mParent;
		mParent->AddReference(Info());
	}
	return *this;
}

cTexturedQuad& cTexturedQuad::operator=(cTexturedQuad&& other)
{
	if (other.mID != this->mID || mParent != other.mParent)
	{
		if (mParent)
		{
			mParent->RemoveReference(Info());
		}
		this->mID = other.mID;
		this->mParent = other.Release();
	}
	return *this;
}

cTexturedQuad::~cTexturedQuad()
{
	if (mParent)
	{
		mParent->RemoveReference(Info());
	}
}

cQuadInfo& cTexturedQuad::Info()
{
	return mParent->Info(mID);
}


} // namespace akj