#include "akjMovingVertex2D.hpp"
#include "akjOGL.hpp"
#include "akjVertexArrayObject.hpp"
#include "akjArrayBuffer.hpp"
namespace akj{
cMovingVertex2D::~cMovingVertex2D(void)
{
}

void cMovingVertex2D::BindToVAO( cVertexArrayObject& vao, cArrayBuffer& array_buffer )
{
	const int byte_stride = ByteStride();
	vao.Bind();
	array_buffer.Bind();

	//texCoord
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(0) );
	
	//normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(2) );

	//velocity
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(4) );

	//acceleration
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(6) );
	
	// start time
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(8) );
	
	// other stuff
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, byte_stride, AKJ_FLOAT_OFFSET(9) );

	vao.UnBind();
	array_buffer.UnBind();
}

int cMovingVertex2D::ByteStride()
{
	return 12*sizeof(GLfloat);
}
}