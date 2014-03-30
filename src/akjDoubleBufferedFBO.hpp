#pragma once
#include "akjHWGraphicsObject.hpp"
#include "akjHWGraphicsContext.hpp"
#include "Twine.hpp"
#include "akjFrameBufferObject.hpp"


namespace akj
{
	class cHWGraphicsContext;
	class cDoubleBufferedFBO
	{
	public:
		cDoubleBufferedFBO(cHWGraphicsContext* context,
												const Twine& name,
												ivec2 size)
			: mFBO_A(context, name + " FBO A", size)
			, mFBO_B(context, name + " FBO B", size)
			, mContext(*context)
			, mIsAInFront(false)
		{}
		~cDoubleBufferedFBO(){};

		void Bind()
		{
			FrontBuffer().BindForRead();
			BackBuffer().BindForWrite();
			//AKJ_ASSERT(BackBuffer().IsComplete());
		}

		void UnBind()
		{

		}

		void Swap()
		{
			mIsAInFront = !mIsAInFront;
			//mContext.BlockUntilFinished();
		}

	private:
		cFrameBufferObject& FrontBuffer()
		{
			if (mIsAInFront){ return mFBO_A; }
			return mFBO_B;
		}

		cFrameBufferObject& BackBuffer()
		{
			if (mIsAInFront){ return mFBO_B; }
			return mFBO_A;
		}
		cFrameBufferObject mFBO_A;
		cFrameBufferObject mFBO_B;
		cHWGraphicsContext& mContext;
		bool mIsAInFront;

	};
}
