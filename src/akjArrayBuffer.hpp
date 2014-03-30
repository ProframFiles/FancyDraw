#pragma once
#include "akjHWGraphicsObject.hpp"
#include "akjDataPointer.hpp"
#include <vector>

namespace akj
{
enum eBufferUsage
	{
		DYNAMIC_VBO,
		DYNAMIC_IBO,
		DYNAMIC_PBO,
		STATIC_VBO,
		STATIC_IBO,
		STATIC_PBO,
		UNIFORM_BUFFER
	};

class cArrayBuffer;

struct cBufferRange
{
	cBufferRange()
		: mParent(nullptr)
		, start(0)
		, size(0)
		{}
	cBufferRange(cArrayBuffer& parent, uint32_t start_in, uint32_t size_in)
		: mParent(&parent)
		, start(start_in)
		, size(size_in)
		{}
	cArrayBuffer* mParent;
	uint32_t start;
	uint32_t size;
	uint32_t End() const {return size+start;}
	void BindRangeToIndex(uint32_t index) const;
};

template <class tRange>
class cScopedMapping
{
	public:
	~cScopedMapping()
	{
		if(mPtr)
		{
			mMappedRange.mParent->UnMapBuffer();
		}
	}
	cScopedMapping(void* ptr, tRange range)
		: mMappedRange(range)
		, mPtr(ptr)
		, mWritten(0)
	{}

	cScopedMapping(cScopedMapping&& other)
		:mPtr(other.mPtr)
		,mMappedRange(other.mMappedRange)
		,mWritten(other.mWritten)
	{
		other.mPtr = nullptr;
	}
	const tRange& Range() const { return mMappedRange; }
	
	template <typename T>
	void WriteToBuffer(const T* ptr, size_t count)
	{
		const size_t bytes = sizeof(T)*count;
		AKJ_ASSERT(bytes+mWritten <= mMappedRange.End());
		memcpy(mPtr+mWritten, ptr, bytes);
		mWritten += static_cast<uint32_t>(bytes);
	}

	template <typename tVector>
	void WriteToBuffer(const tVector& vec)
	{
		const size_t bytes = sizeof(tVector::value_type)*vec.size();
		AKJ_ASSERT(bytes+mWritten <= mMappedRange.End());
		memcpy(mPtr+mWritten, vec.data(), bytes);
		mWritten += static_cast<uint32_t>(bytes);
	}


	private:
	cScopedMapping& operator=(const cScopedMapping& other){return *this;}
	tRange mMappedRange;
	cDataPtr mPtr;
	uint32_t mWritten;

};

struct cGLBufferDesc
{
	cGLBufferDesc(eBufferUsage usage, uint32_t size)
		:mUsage(usage), mBytes(size) {}
	eBufferUsage mUsage;
	uint32_t mBytes;
};
class cArrayBuffer : public cHWGraphicsObject
{
public:
	enum eLimits
	{
		kMaxUniformBindings = 36,
		kMaxUniformBlockSize = 16*1024,
	};
	cArrayBuffer(cHWGraphicsContext* context, const Twine& object_name, 
									uint32_t bytes, eBufferUsage usage);
	cArrayBuffer(cHWGraphicsContext* context, const Twine& object_name, 
									const cGLBufferDesc& desc);
	~cArrayBuffer();
	uint32_t Size() const;
	void InitBuffer(uint32_t bytes, const void* data);
	void SetData(uint32_t bytes, const void* data, uint32_t start_byte);
	void ResetData(uint32_t data);
	void ResetData();
	void* MapBuffer();
	void UnMapBuffer();
	void DrawAsTriangles(int32_t start_index, int32_t end_index);
	void DrawAsTriangleStrip(int32_t start_index, int32_t end_index);
	void DrawIndexed(const std::vector<uint32_t>& indices);
	void DrawIndexedInstanced(const std::vector<uint32_t>& indices, 
														uint32_t repeats);
	void DrawIndexedInstanced(uint32_t verts_per_elem,
		uint32_t offset, uint32_t repeats);
	void DrawIndexedInstanced(uint32_t verts_per_elem,
		uint32_t offset, uint32_t repeats, uint32_t base_vertex);
	void Bind();
	void BindIndexed(uint32_t index);
	void BindIndexedRange(uint32_t index, uint32_t start_byte, uint32_t size);
	void UnBind();
	// a non-blocking or syncing mapping (make sure the range is clear!)
	void Grow();
	virtual cScopedMapping<cBufferRange> MapBytes(size_t size);
	cScopedMapping<cBufferRange> MapBufferRange(size_t start, size_t size);
	void DrawIndexedInstancedLines(uint32_t verts_per_elem, 
														uint32_t offset, uint32_t repeats);
	void DrawInstancedPoints(uint32_t base_vertex, uint32_t num_points);
protected:
	void SetUsage(eBufferUsage usage);

	uint32_t mBufferSize;
	uint32_t mUsage;
	uint32_t mTarget;
	bool mIsMapped;
};

class cDrawPointsCommand : public iDrawCommand
{
public:
	cDrawPointsCommand(const cBufferRange& range): mRange(range){}
	virtual void Execute()
	{
		mRange.mParent->DrawInstancedPoints(mRange.start,mRange.size);
	}
	cBufferRange mRange;
};

class cDrawTriStripCommand : public iDrawCommand
{
public:
	cDrawTriStripCommand(const cBufferRange& range): mRange(range){}
	virtual void Execute()
	{
		mRange.mParent->DrawAsTriangleStrip(mRange.start,mRange.size);
	}
	cBufferRange mRange;
};



inline void cBufferRange::BindRangeToIndex(uint32_t index) const
{
	mParent->BindIndexedRange(index, start, size);
}

}
