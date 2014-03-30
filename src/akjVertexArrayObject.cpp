#include "akjVertexArrayObject.hpp"
#include "akjOGL.hpp"

namespace akj
{
cVertexArrayObject
	::cVertexArrayObject(cHWGraphicsContext* context, const Twine& name)
:cHWGraphicsObject(context, name)
{
	glGenVertexArrays(1, &mObjectID);
}

cVertexArrayObject
	::cVertexArrayObject(cHWGraphicsContext* context, const Twine& name,	
												std::initializer_list<cGLBufferDesc> desc_list,
												std::initializer_list<cVertexAttribute> attrib_list)
:cHWGraphicsObject(context, name)
{
	glGenVertexArrays(1, &mObjectID);
	InitBuffers(desc_list);
	SetAttributes(attrib_list);
}

cVertexArrayObject
	::cVertexArrayObject(cHWGraphicsContext* context, const Twine& name,	
												std::initializer_list<cGLBufferDesc> desc_list)
:cHWGraphicsObject(context, name)
{
	glGenVertexArrays(1, &mObjectID);
	InitBuffers(desc_list);
}

void cVertexArrayObject::Bind()
{
	glBindVertexArray(mObjectID);
}

void cVertexArrayObject::UnBind()
{
	glBindVertexArray(0);
}

cVertexArrayObject::~cVertexArrayObject()
{
	glDeleteVertexArrays(1, &mObjectID);
}

void cVertexArrayObject
	::SetAttributes(std::initializer_list<cVertexAttribute> list)
{
	Bind();
	int max_attributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attributes);

	int attrib_num = 0;
	uint32_t offsets[kMaxBuffers] = {};
	int index = -1;
	for(const auto& attr: list)
	{
		AKJ_ASSERT(attrib_num < max_attributes);
		if(attrib_num >= max_attributes) continue;
		if(attr.mBufferIndex != index)
		{
			AKJ_ASSERT(index < static_cast<int>(mBuffers.size()));
			index = attr.mBufferIndex;
			mBuffers.at(index)->Bind();
		}
			
		glEnableVertexAttribArray(attrib_num);
		switch (attr.mType)
		{
		case ATTR_FLOAT:
			glVertexAttribPointer(attrib_num, attr.mNum, GL_FLOAT,
														false, attr.mByteStride,
														AKJ_FLOAT_OFFSET(offsets[index]));
			break;
		case ATTR_UINT:
			glVertexAttribIPointer(attrib_num, attr.mNum, GL_UNSIGNED_INT,
														attr.mByteStride,
														AKJ_FLOAT_OFFSET(offsets[index]));
			break;
		default:
			AKJ_ASSERT(!"unknown attribute type");
			break;
		}
			
		glVertexAttribDivisor(attrib_num, attr.mDivisor);
		offsets[index] += attr.mNum;
		++attrib_num;
	}
	mBuffers.at(index)->UnBind();
	UnBind();
}

void cVertexArrayObject
::InitBuffers(std::initializer_list<cGLBufferDesc> desc_list)
{
	int label = 1;
	int num_buffers = static_cast<int>(desc_list.size());
	mBuffers.clear();
	for(const auto& desc: desc_list)
	{
		mBuffers.emplace_back(new cArrayBuffer(
			mParentContext, mObjectName + " (buffer " 
			+ Twine(label) + "/" + Twine(num_buffers) +")", desc));
		++label;
	}
}

void cVertexArrayObject
::DrawInstancedPoints(uint32_t base_vertex, uint32_t num_points)
{
	Bind();
	glDrawArraysInstanced(GL_POINTS, base_vertex, num_points, 1);
	//glDrawArrays(GL_LINE_STRIP, start_index, end_index);
}

void cVertexArrayObject
::DrawAsTriangles(int32_t start_index, int32_t count)
{
	Bind();
	glDrawArrays(GL_TRIANGLES, start_index, count);
}

void cVertexArrayObject::AddBuffer(std::unique_ptr<cArrayBuffer>&& buf)
{
	mBuffers.emplace_back(std::move(buf));
}


} // namespace akj