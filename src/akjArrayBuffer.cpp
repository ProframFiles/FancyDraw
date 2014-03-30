#include "akjArrayBuffer.hpp"
#include "akjOGL.hpp"


namespace akj
{
	cArrayBuffer::cArrayBuffer(cHWGraphicsContext* context, 
															const Twine& object_name,
															uint32_t bytes, 
															eBufferUsage usage)
	: cHWGraphicsObject(context, object_name)
	, mIsMapped(false)
{
	
	SetUsage(usage);
	glGenBuffers(1, &mObjectID);
	glCheckAllErrors(__FILE__, __LINE__);
	InitBuffer(bytes, NULL);
}

	cArrayBuffer::cArrayBuffer(cHWGraphicsContext* context, const Twine& object_name, const cGLBufferDesc& desc)
		: cHWGraphicsObject(context, object_name)
		, mIsMapped(false)
	{
		SetUsage(desc.mUsage);
		glGenBuffers(1, &mObjectID);
		glCheckAllErrors(__FILE__, __LINE__);
		InitBuffer(desc.mBytes, NULL);
	}

cArrayBuffer::~cArrayBuffer()
{
	glDeleteBuffers(1, &mObjectID);
}

void cArrayBuffer::InitBuffer( uint32_t bytes, const void* data)
{
	mBufferSize = bytes;
	if(bytes > 100*1024)
	{
		Log::Debug("%s: allocated %dk",mObjectName , bytes / 1024);
	}
	else
	{
		Log::Debug("%s: allocated %db",mObjectName , bytes);
	}
	Bind();
	glBufferData(mTarget, bytes, data, mUsage);
	glCheckAllErrors(__FILE__, __LINE__);
}

void cArrayBuffer::SetUsage(eBufferUsage usage)
{
	switch(usage)
	{
	case STATIC_VBO:
		mUsage = GL_STATIC_DRAW;
		mTarget = GL_ARRAY_BUFFER;
		break;
	case STATIC_IBO:
		mUsage = GL_STATIC_DRAW;
		mTarget = GL_ELEMENT_ARRAY_BUFFER;
		break;
	case STATIC_PBO:
		mUsage = GL_STREAM_DRAW;
		mTarget = GL_PIXEL_UNPACK_BUFFER;
		break;
	case DYNAMIC_VBO:
		mUsage = GL_STREAM_DRAW;
		mTarget = GL_ARRAY_BUFFER;
		break;
	case DYNAMIC_IBO:
		mUsage = GL_DYNAMIC_DRAW;
		mTarget = GL_ELEMENT_ARRAY_BUFFER;
		break;
	case DYNAMIC_PBO:
		mUsage = GL_STATIC_DRAW;
		mTarget = GL_PIXEL_UNPACK_BUFFER;
		break;
	case UNIFORM_BUFFER:
		mUsage = GL_DYNAMIC_DRAW;
		mTarget = GL_UNIFORM_BUFFER;
		break;
	}
}

void cArrayBuffer::Bind()
{
	glBindBuffer(mTarget, mObjectID);
}

void cArrayBuffer
	::SetData( uint32_t bytes, const void* data, uint32_t start_byte )
{
	mBufferSize = bytes;
	Bind();
	glBufferSubData(mTarget, start_byte, bytes, data);
	glCheckAllErrors(__FILE__, __LINE__);
}

void cArrayBuffer::ResetData(uint32_t size )
{
	mBufferSize = size;
	Bind();
	glBufferSubData(mTarget, 0, size, 0 );
}

void cArrayBuffer::ResetData()
{
	Bind();
	glBufferData(mTarget, mBufferSize, NULL, mUsage);
}

void* cArrayBuffer::MapBuffer()
{
	Bind();
	void* ptr = glMapBuffer(mTarget,GL_WRITE_ONLY);
	AKJ_ASSERT(ptr != NULL);
	mIsMapped = true;
	return ptr;
}

cScopedMapping<cBufferRange> cArrayBuffer
	::MapBufferRange(size_t start, size_t size)
{
	Bind();
	void* ptr = glMapBufferRange(mTarget, start, size, 
															GL_MAP_WRITE_BIT|
															GL_MAP_UNSYNCHRONIZED_BIT);
	if(!ptr)
	{
		mParentContext->CheckErrors(AKJ_LOG_CSTR("cArrayBuffer::MapBufferRange"));
		AKJ_THROW("Failed to map arraybuffer range " 
							+ Twine(start) +"-"+ Twine(start+size));
	}
	mIsMapped = true;
	return cScopedMapping<cBufferRange>(ptr,
				 cBufferRange(*this, static_cast<uint32_t>(start),
														 static_cast<uint32_t>(size)));
}

void cArrayBuffer::UnMapBuffer()
{
	glUnmapBuffer(mTarget);
	mIsMapped = false;
}

void cArrayBuffer::UnBind()
{
	glBindBuffer(mTarget, 0);
}

void cArrayBuffer::DrawAsTriangles(int32_t start_index, int32_t end_index)
{
	Bind();
	glDrawArrays(GL_TRIANGLES, start_index, end_index);
}

void cArrayBuffer::DrawAsTriangleStrip(int32_t start_index, int32_t end_index)
{
	Bind();
	glDrawArrays(GL_TRIANGLE_STRIP, start_index, end_index);
	//glDrawArrays(GL_LINE_STRIP, start_index, end_index);
}

void cArrayBuffer::DrawIndexed(const std::vector<uint32_t>& indices)
{
	Bind();
	glDrawElements(GL_TRIANGLES, static_cast<uint32_t>(indices.size()),
									GL_UNSIGNED_INT, indices.data());
	//glDrawArrays(GL_LINE_STRIP, start_index, end_index);
}

void cArrayBuffer
::DrawInstancedPoints(uint32_t base_vertex, uint32_t num_points)
{
	Bind();
	glDrawArraysInstanced(GL_POINTS, base_vertex,num_points, 1);
	//glDrawArrays(GL_LINE_STRIP, start_index, end_index);
}

void cArrayBuffer::
	DrawIndexedInstanced(const std::vector<uint32_t>& indices, uint32_t repeats)
{
	Bind();
	glDrawElementsInstanced(GL_TRIANGLES, static_cast<uint32_t>(indices.size()),
		GL_UNSIGNED_INT, indices.data(), repeats);
}

void cArrayBuffer::
DrawIndexedInstanced(uint32_t verts_per_elem, uint32_t offset, uint32_t repeats)
{
	Bind();
	glDrawElementsInstanced(GL_TRIANGLES, verts_per_elem,
		GL_UNSIGNED_INT, (void*)0, repeats);
}

void cArrayBuffer::
DrawIndexedInstancedLines(uint32_t verts_per_elem,
													uint32_t offset, uint32_t repeats)
{
	Bind();
	glDrawElementsInstanced(GL_LINE_STRIP, verts_per_elem,
		GL_UNSIGNED_INT, (void*)0, repeats);
}

void cArrayBuffer::
DrawIndexedInstanced(uint32_t verts_per_elem, 
										uint32_t offset, uint32_t repeats, uint32_t base_vertex)
{
	Bind();
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, verts_per_elem,
		GL_UNSIGNED_INT, (void*)0, repeats, base_vertex);
}

void cArrayBuffer::BindIndexed(uint32_t index)
{
	glBindBufferRange(mTarget, index, mObjectID, 0, mBufferSize);
}

void cArrayBuffer::BindIndexedRange(uint32_t index,
																		uint32_t start_byte, uint32_t size)
{
	
	glBindBufferRange(mTarget, index, mObjectID, start_byte, size);
	int error = mParentContext->CheckErrors(
		AKJ_LOG_CSTR("cArrayBuffer::BindIndexedRange"));
	if(error)
	{
		AKJ_THROW("Failed to bind array " + mObjectName +"("+Twine(mObjectID) +") "+
			Twine(index)+ " at byte " + Twine(start_byte) + ", length " +Twine(size));
	}
}

uint32_t cArrayBuffer::Size() const
{
	return mBufferSize;
}

cScopedMapping<cBufferRange> cArrayBuffer::MapBytes(size_t size)
{
	return std::move(MapBufferRange(0, size));
}

void cArrayBuffer::Grow()
{
	InitBuffer(mBufferSize*2, nullptr);
}

} // namespace akj