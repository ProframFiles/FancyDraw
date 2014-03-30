#include "akj2DVertex.hpp"
#include "akjOGL.hpp"
#include "akjVertexArrayObject.hpp"
#include "akjArrayBuffer.hpp"
#include "FatalError.hpp"
#include "akj2DGraphicsPrimitives.hpp"

namespace akj{

	void c2DVertex::BindToVAO(cVertexArrayObject& vao,
														cArrayBuffer& array_buffer_per_vertex,
														cArrayBuffer& array_buffer_per_instance,
														cArrayBuffer& index_buffer)
	{
		CheckRuntimeMemLayout();
		int byte_stride = PerVertexByteStride();
		vao.Bind();
		array_buffer_per_vertex.Bind();
		// relative position (texcoord)
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, 0, byte_stride, AKJ_FLOAT_OFFSET(0) );
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, 0, byte_stride, AKJ_FLOAT_OFFSET(2));
		
		byte_stride = PerInstanceByteStride();
		int divisor = 1;

		array_buffer_per_instance.Bind();
		uint32_t offset = 0;
		uint32_t size;

		//global position
		size = 2;
		glEnableVertexAttribArray(2);
		glVertexAttribDivisor(2, divisor);
		glVertexAttribPointer(2, size, GL_FLOAT, 0, byte_stride, 
													AKJ_FLOAT_OFFSET((offset+=size, offset-size)) );
		

		//shape position
		size = 2;
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, divisor);
		glVertexAttribPointer(3, size, GL_FLOAT, 0, byte_stride,
													 AKJ_FLOAT_OFFSET((offset+=size, offset-size)) );


		//corner radius
		size = 1;
		glEnableVertexAttribArray(4);
		glVertexAttribDivisor(4, divisor);
		glVertexAttribPointer(4, size, GL_FLOAT, 0, byte_stride,
													 AKJ_FLOAT_OFFSET((offset+=size, offset-size))  );


		// stroke width
		size = 1;
		glEnableVertexAttribArray(5);
		glVertexAttribDivisor(5, divisor);
		glVertexAttribPointer(5, size, GL_FLOAT, 0, byte_stride, 
													AKJ_FLOAT_OFFSET((offset+=size, offset-size)) );


		// stroke color
		size = 1;
		glEnableVertexAttribArray(6);
		glVertexAttribDivisor(6, divisor);
		glVertexAttribIPointer(6, size, GL_UNSIGNED_INT, byte_stride,
														 AKJ_FLOAT_OFFSET((offset+=size, offset-size))  );


		// fill color
		size = 1;
		glEnableVertexAttribArray(7);
		glVertexAttribDivisor(7, divisor);
		glVertexAttribIPointer(7, size, GL_UNSIGNED_INT, byte_stride,
														 AKJ_FLOAT_OFFSET((offset+=size, offset-size))  );


		//fill texture coord
		size = 2;
		glEnableVertexAttribArray(8);
		glVertexAttribDivisor(8, divisor);
		glVertexAttribPointer(8, size, GL_FLOAT, 0, byte_stride,
														 AKJ_FLOAT_OFFSET(offset+=size) );
		// depth
		size = 1;
		glEnableVertexAttribArray(9);
		glVertexAttribDivisor(9, divisor);
		glVertexAttribPointer(9, size, GL_FLOAT, 0, byte_stride,
														AKJ_FLOAT_OFFSET((offset+=size, offset-size)) );

		//fill solid color percentage
		size = 1;
		glEnableVertexAttribArray(10);
		glVertexAttribDivisor(10, divisor);
		glVertexAttribPointer(10, size, GL_FLOAT, 0, byte_stride,
														 AKJ_FLOAT_OFFSET(offset+=size) );


		index_buffer.Bind();
		vao.UnBind();
		array_buffer_per_instance.UnBind();
		index_buffer.UnBind();
	}
	
	/*static*/ void c2DVertex::UnbindAttributes()
	{
		for (int i = 0; i < 9; ++i)
		{
			glDisableVertexAttribArray(i);
		}
	}

	/*static*/ int c2DVertex::PerVertexByteStride()
	{
		return sizeof(c2DVertex);
	}

	int c2DVertex::PerInstanceByteStride()
	{
		return sizeof(c2DVertex::Instanced);
	}

	void c2DVertex::CheckRuntimeMemLayout()
	{
		c2DVertex::Instanced test_vert;
		float* float_ptr = reinterpret_cast<float*>(&test_vert);
		uint32_t* uint_ptr = reinterpret_cast<uint32_t*>(&test_vert);
		bool layout_ok = true;
		layout_ok &= (&float_ptr[0] == &test_vert.mPosition.x);
		layout_ok &= (&float_ptr[1] == &test_vert.mPosition.y);
		layout_ok &= (&float_ptr[2] == &test_vert.mHalfSize.x);
		layout_ok &= (&float_ptr[3] == &test_vert.mHalfSize.y);
		layout_ok &= (&float_ptr[4] == &test_vert.mCornerRadius);
		layout_ok &= (&float_ptr[5] == &test_vert.mStrokeWidth);
		layout_ok &= (&uint_ptr[6] 
							== reinterpret_cast<uint*>(&test_vert.mStrokeColor));
		layout_ok &= (&uint_ptr[7] 
							== reinterpret_cast<uint*>(&test_vert.mFillColor));
		layout_ok &= (&float_ptr[8] == &test_vert.mFillCoord.x);
		layout_ok &= (&float_ptr[9] == &test_vert.mFillCoord.y);
		layout_ok &= (&float_ptr[10] == &test_vert.mDepth);
		layout_ok &= (&float_ptr[11] == &test_vert.mExtraFillAlpha);
		if (!layout_ok)
		{
			AKJ_THROW("Layout for primitive shape does not work out correctly");
		}
	}



	

}
