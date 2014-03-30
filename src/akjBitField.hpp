#pragma once

namespace akj
{
	template <typename tEnum>
	class cBitField
	{
	public:
		cBitField(): mValue(static_cast<tEnum>(0)){}
		cBitField(tEnum flag): mValue(static_cast<tEnum>(0)){ Set(flag); }
		~cBitField(){};

		cBitField operator |=( const cBitField& other)
		{
			Set(other.mValue);
			return *this;
		}

		cBitField Set(tEnum flag)
		{
			mValue = static_cast<tEnum> (
				static_cast<uint32_t>(flag) | static_cast<uint32_t>(mValue));
			return *this;
		}
		
		cBitField ClearAll()
		{
			mValue = static_cast<tEnum>(0);
			return *this;
		}

		cBitField Clear(tEnum flag = k)
		{
			mValue = static_cast<tEnum> (
				static_cast<uint32_t>(mValue) & (~static_cast<uint32_t>(flag)));
			return *this;
		}

		bool IsSet(tEnum flag) const
		{
			return (flag & mValue)>0;
		}

	private:
		enum {kAllFields = 0xFFFFFFFF};
		tEnum mValue;
	};
}
