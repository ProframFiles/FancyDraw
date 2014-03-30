#pragma once
#include "akjApplicationObject.hpp"
#include "akjTask.hpp"
#include "akjScreenTextFactory.hpp"
#include "akj2DPrimitiveFactory.hpp"
#include "akjAppObjectFactory.hpp"

namespace akj
{

	class cProgressBar : public cApplicationObject
	{
	public:
		cProgressBar(cWrappingCounter id, tTaskHandle handle, cAppObjectFactory& factory,
									cRoundedRectangle&& bar_rect, cRoundedRectangle&& main_rect,
									cScreenText&& title_text, cScreenText&& status_text)
			: cApplicationObject(id)
			, mTaskHandle(handle)
			, mObjectFactory(factory)
			, mMainRectangle(std::move(main_rect))
			, mBarBackGround(std::move(bar_rect))
			, mIsTaskFinished(false)
		{
			Log::Debug("ProgressBar Created!");
			mScreenText.emplace_back(std::move(title_text));
			mScreenText.emplace_back(std::move(status_text));
			iRect bb = TitleText().BoundingBox();
			cCoord2 br_main = mMainRectangle.BottomRight();
			if(br_main.x < bb.Right() + 10 )
			{
				br_main.x = static_cast<float>(bb.Right()+10);
				mMainRectangle.BottomRight(br_main);
				mBarBackGround.BottomRight(br_main - cCoord2(10.0f, 10.0f));
			}
			mMainRectangle.FillAlpha(200);
			mBarBackGround.FillAlpha(200);
			mBarMoving = mBarBackGround.Duplicate()
																	.FillColor(cWebColor::YELLOW)
																	.FillAlpha(170);
			cCoord2 top_left = mBarMoving.TopLeft();
			cCoord2 bottom_right = mBarMoving.BottomRight();
			const cCoord2 size = bottom_right - top_left;
			const cCoord2 padding = size*0.0f;
			top_left += padding;
			bottom_right -= padding;
			mXOrigin = top_left.x;
			mFullBarLength = bottom_right.x - mXOrigin;
			bottom_right.x = top_left.x;
			mBarMoving.TopLeft(top_left).BottomRight(bottom_right);

			mBarMoving.AddText(cMutableArrayRef<cScreenText>(mScreenText.data(), 2));
			mBarBackGround.Show();
			mBarMoving.Show();
			mMainRectangle.Show();
		}
		~cProgressBar(){
			Log::Debug("ProgressBar Destroyed!");
		};

		virtual cBitField<eListenToBits> ListensTo() const 
		{ return kTaskListenBit; } 

		virtual bool HasLimitedLifetime() const {return true;}
		virtual bool IsDead() const { return mIsTaskFinished; }

		virtual void
			OnTaskUpdate(tTaskHandle handle, cStringRef status, float progress)
		{
				
			if(mTaskHandle != handle)
			{
				Log::Debug("Not My task! %d", handle.id.operator uint32_t());
				return;
			}
			Log::Debug("Is My task! %d", handle.id.operator uint32_t());
			cCoord2 bottom_right = mBarMoving.BottomRight();
			bottom_right.x = mFullBarLength*progress + mXOrigin;
			//TODO: set text
			mBarMoving.BottomRight(bottom_right);
			StatusText().ChangeText(status);
		}

		virtual void OnTaskDestroyed(tTaskHandle handle)
		{
			if(handle == mTaskHandle)
			{
				mBarMoving.Hide();
				mBarBackGround.FillColor(cWebColor::GREEN);
				mObjectFactory.CreateAppObject<cAnimatedFuncObject>(3.0, 
				[this](double frac){
					const float ffrac = static_cast<float>(frac);
					mBarBackGround.FillAlpha(static_cast<uint8_t>(255*(1.0f-ffrac)));
					mMainRectangle.FillAlpha(static_cast<uint8_t>(170*(1.0f-ffrac)));
					mBarBackGround.StrokeAlpha(static_cast<uint8_t>(255*(1.0f-ffrac)));
					mMainRectangle.StrokeAlpha(static_cast<uint8_t>(255*(1.0f-ffrac)));
					mScreenText[0].FillAlpha(1.0f-ffrac);
					mScreenText[1].FillAlpha(1.0f-ffrac);
					if(frac==1.0){
						mIsTaskFinished = true; 
					}
				});
			}
		};

	private:
		cScreenText& TitleText()
		{
			return mScreenText[0];
		} 
		cScreenText& StatusText()
		{
			return mScreenText[1];
		}
		cProgressBar(const cProgressBar& other);
		tTaskHandle mTaskHandle;
		std::string mTaskName;

		cRoundedRectangle mMainRectangle;
		cRoundedRectangle mBarBackGround;
		cRoundedRectangle mBarMoving;
		cArray<2, cScreenText> mScreenText;
		cAppObjectFactory& mObjectFactory;
		float mXOrigin;
		float mFullBarLength;

		bool mIsTaskFinished;
	};

class cProgressBarCreator : public cApplicationObject
{
public:
	cProgressBarCreator(cWrappingCounter id, 
											cAppObjectFactory& object_factory,
											cScreenTextFactory& text_factory, 
											c2DPrimitiveFactory& primitive_factory)
		: cApplicationObject(id)
		, mObjectFactory(object_factory)
		, mTextFactory(text_factory)
		, mRectFactory(primitive_factory)
		, mProtoTypeRect (primitive_factory.CreateRoundedRect(
											10.0f, 300.0f, 10.0f, 90.0f, 1.5f, 4.0f,
											cWebColor::FIREBRICK, cWebColor::FLORALWHITE	).Hide())
		, mProtoTypeBar (primitive_factory.CreateRoundedRect(
											20.0f, 290.0f, 50.0f, 80.0f, 1.5f, 4.0f,
											cWebColor::FIREBRICK, cWebColor::CORAL	).Hide())
		, mNumOutstanding(0)
		, mYOffset(95.0f)
	{}
	~cProgressBarCreator(){}

	virtual cBitField<eListenToBits> ListensTo() const 
	{ return kTaskListenBit; } 

	virtual void OnTaskCreated(tTaskHandle handle,
															cStringRef task_name, 
															cStringRef status)
	{
		float offset = mNumOutstanding*mYOffset;
		mProtoTypeBar.Center({155.0f, 65.0f+offset});
		mProtoTypeRect.Center({155.0f,50.0f+ offset});
		cProgressBar* bar = mObjectFactory.CreateAppObject<cProgressBar>
				(handle, mObjectFactory,
					mRectFactory.CreateRoundedRect(mProtoTypeBar),
					mRectFactory.CreateRoundedRect(mProtoTypeRect),
					mTextFactory.CreateText(task_name, {20.0f, 35.0f+offset}, 18),
					mTextFactory.CreateText(status, {30.0f, 70.0f+offset}, 14));
		 ++mNumOutstanding;
	};

	virtual void OnTaskDestroyed(tTaskHandle handle)
	{
		--mNumOutstanding;

	}
	


private:
	cAppObjectFactory& mObjectFactory;
	cScreenTextFactory& mTextFactory;
	c2DPrimitiveFactory& mRectFactory;
	cRoundedRectangle mProtoTypeRect;
	cRoundedRectangle mProtoTypeBar;
	uint32_t mNumOutstanding;
	float mYOffset;

};
}
