#pragma once
#include "akjApplicationObject.hpp"

namespace akj
{
	class cAnimationEasing
	{
	public:
		virtual ~cAnimationEasing(){};
		virtual double Eased(double in) {return in;}
	};
	enum eAnimationEasingType
	{
		kLinear,
		kCubicIn,
		kCubicOut,
		kCubic,
		kCustom
	};
	class cAnimationObject : public cApplicationObject
	{
	public:


	cAnimationObject(const cWrappingCounter& id, double duration)
		: cApplicationObject(id)
		, mStartTime(-1.0)
		, mDuration(duration)
		, mFraction(-1.0)
		, mEaseType(kLinear)
		, mFinalPause(0.0)
	{}
	~cAnimationObject(){};

	virtual cBitField<eListenToBits> ListensTo() const 
		{ return kTimeListenBit; }
	virtual bool HasLimitedLifetime() const { return true; }
	virtual bool IsDead() const { return mFraction >= 1.0 && mFinalPause <=0.0; }

	virtual void
	OnTimeElapsed(double elapsed, double absolute){
		if(mStartTime < 0.0) mStartTime = absolute;
		mCurrentTime = absolute;
		if(mFraction < 1.0)
		{
			mFraction = -1.0;
		}
		else
		{
			mFinalPause -= elapsed;
		}
	};

	void SetEasing(eAnimationEasingType t, double v1 = 0.0, double v2 = 0.0){
		mEaseType = t;
	}

	void SetFinalPause(double pause)
	{
		mFinalPause = pause;
	}

	double FractionComplete()
	{
		if(mFraction >= 0.0) return mFraction;
		double raw_fraction = Clamp(0.0, (mCurrentTime - mStartTime)/mDuration, 1.0);
		switch (mEaseType)
		{
		case akj::kLinear:
			mFraction = raw_fraction;
			break;
		case akj::kCubic:
			mFraction = EaseCubic(raw_fraction);
			break;
		case akj::kCubicOut:
			mFraction = EaseCubicOut(raw_fraction);
			break;
		case akj::kCubicIn:
			mFraction = EaseCubicIn(raw_fraction);
			break;
		case akj::kCustom:
			AKJ_ASSERT(mEaser);
			mFraction = mEaser->Eased(raw_fraction);
			break;
		default:
			break;
		}
		return mFraction;
	}

	private:
		double EaseCubic(double frac)
		{
			const double edge = std::floor(0.5+frac);
			double fhalf = 2.0*(frac-edge);
			const double f3 = fhalf*fhalf*fhalf;
			double ret = edge+0.5*(f3);
			return ret;
		}
		double EaseCubicOut(double frac)
		{
			double fhalf = (frac-1.0);
			const double f3 = fhalf*fhalf*fhalf;
			double ret = 1.0+(f3);
			return ret;
		}
		double EaseCubicIn(double frac)
		{
			double fhalf = (frac);
			const double f3 = fhalf*fhalf*fhalf;
			double ret = (f3);
			return ret;
		}
		double mStartTime;
		double mCurrentTime;
		double mDuration;
		double mFraction;
		double mFinalPause;
		eAnimationEasingType mEaseType;
		std::unique_ptr<cAnimationEasing> mEaser;
	};

	class cAnimatedFuncObject: public cAnimationObject
	{
	public:
		cAnimatedFuncObject(const cWrappingCounter& id, double duration,
			std::function<void(double)>&& func )
			:cAnimationObject(id, duration)
			,mFunc(std::move(func))
		{}

	virtual void OnTimeElapsed(double elapsed, double absolute){
		cAnimationObject::OnTimeElapsed(elapsed, absolute);
		if(!IsDead())
		{
			mFunc(FractionComplete());
		}
	}

	private:
		std::function<void(double)> mFunc;
	};

}
