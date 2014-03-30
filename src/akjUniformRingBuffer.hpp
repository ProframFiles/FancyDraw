#pragma once
#include "akjArrayBuffer.hpp"
#include "akjDataPointer.hpp"
#include "akjHWGraphicsContext.hpp"

namespace akj
{

	class cGPURingBuffer : public cArrayBuffer
	{
	public:
		typedef cScopedMapping<cBufferRange> tScopedMapping;
		cGPURingBuffer(cHWGraphicsContext& context, 
										const Twine& name,
										uint32_t desired_capacity,
										eBufferUsage usage = UNIFORM_BUFFER)

			: cArrayBuffer(&context, name,context.UniformAlign(desired_capacity)*2,
										 usage)
			, mCursor(0)
		{
			Log::Debug("Uniform block alignment: %d bytes", 
									mParentContext->UniformAlign());
		}
		~cGPURingBuffer(){};

		void Resize(uint32_t num_bytes)
		{
			num_bytes = mParentContext->UniformAlign(num_bytes);
			InitBuffer(num_bytes*2, NULL);
		}

		tScopedMapping MapBytes(uint32_t num_bytes)
		{
			num_bytes = mParentContext->UniformAlign(num_bytes);
			AKJ_ASSERT_AND_THROW(num_bytes*2 <= (Size()));
			if(num_bytes+mCursor > Size())
			{
				mCursor = 0;
			}
			mCursor += num_bytes;
			return 
				std::move(MapBufferRange(mCursor-num_bytes, num_bytes));
		}

		cArrayBuffer& ArrayBuffer() {return *this;}
	private:
		uint32_t mCursor;
	};

}
