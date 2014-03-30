#pragma once
#include "akjHWGraphicsObject.hpp"
#include "akjFrameBufferObject.hpp"
#include "akjFixedSizeVector.hpp"
#include "akjArrayBuffer.hpp"
#include <initializer_list>


namespace akj
{
	enum eAttributeType
	{
		ATTR_FLOAT,
		ATTR_UINT
	};
	struct cVertexAttribute
	{
		cVertexAttribute(eAttributeType t, uint32_t vec_size, uint32_t stride,
											uint32_t which_buffer, uint32_t divisor)
			:mType(t), mNum(vec_size), mByteStride(stride)
			, mBufferIndex(which_buffer), mDivisor(divisor)  
		{}
		eAttributeType mType;
		uint32_t mNum;
		uint32_t mByteStride;
		uint32_t mBufferIndex;
		uint32_t mDivisor;
	};
	class cVertexArrayObject : public cHWGraphicsObject
	{
	public:
		cVertexArrayObject(cHWGraphicsContext* context, const Twine& name);
		
		cVertexArrayObject(
			cHWGraphicsContext* context, const Twine& name, 
			std::initializer_list<cGLBufferDesc> desc_list );

		cVertexArrayObject(
			cHWGraphicsContext* context, const Twine& name, 
			std::initializer_list<cGLBufferDesc> desc_list,
			std::initializer_list<cVertexAttribute> attrib_list);
		
		~cVertexArrayObject();
		
		void DrawInstancedPoints(uint32_t base_vertex, uint32_t num_points);

		void Bind();
		void UnBind();
		
		void AddBuffer(std::unique_ptr<cArrayBuffer>&& buf);

		cArrayBuffer& Buffer(uint32_t index){ return *mBuffers.at(index);}
		void SetAttributes(std::initializer_list<cVertexAttribute> list);
		
		//////////////////////////////////////////////////////////////////////////
		// Many Types of draw Command
		//////////////////////////////////////////////////////////////////////////

		void DrawAsTriangles(int32_t start_index, int32_t count);



	private:
		void InitBuffers(std::initializer_list<cGLBufferDesc> desc_list);
		



		// 8 is totally arbitrary (but I've never needed more than 3)
		enum {kMaxBuffers = 8};
		cArray<kMaxBuffers, std::unique_ptr<cArrayBuffer>> mBuffers;

	};
		class cBindVAO : public iDrawCommand
	{
	public:
		cBindVAO( cVertexArrayObject& vao)
			:mVAO(vao){}
		virtual void Execute()
		{
			mVAO.Bind();
		}
		cVertexArrayObject& mVAO;
	};

}
