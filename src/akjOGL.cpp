#include "akjOGL.hpp"
#include "akjIOUtil.hpp"
#include "FancyDrawMath.hpp"
#include <stdio.h>
#include <assert.h>
#include "akjVertexArray.hpp"
#include "FatalError.hpp"

#ifndef STRINGIFY
#define STRINGIFY(var) #var
#endif


#ifdef _WIN32
	#define AKJ_GET_GL_FUNCTION_ADDRESS(name,type) {name = (type)wglGetProcAddress(#name); AKJ_ASSERT(name != NULL);}
#else   //linux or mac
	#include "GL/glx.h"
	#define AKJ_GET_GL_FUNCTION_ADDRESS(name,type)	{name = (type)glXGetProcAddress((const GLubyte*)#name); AKJ_ASSERT(name != NULL);}
#endif 

#define PRINT_GL_INT_DEF(enum) {GLint i = 0; glGetIntegerv(enum, &i); Log::Debug("%d = %s", i, #enum);}

static bool gIsInit = false;

#ifdef WIN32
	PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
	PFNGLBLENDEQUATIONPROC glBlendEquation = NULL;
#endif
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders = NULL;
PFNGLISSHADERPROC glIsShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLVALIDATEPROGRAMPROC glValidateProgram = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORM2FPROC glUniform2f = NULL;
PFNGLUNIFORM3FPROC glUniform3f = NULL;
PFNGLUNIFORM4FPROC glUniform4f = NULL;
PFNGLUNIFORM4FVPROC glUniform4fv = NULL;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = NULL;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = NULL;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = NULL;
PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = NULL;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = NULL;
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLMAPBUFFERPROC glMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = NULL;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLPRIMITIVERESTARTINDEXPROC glPrimitiveRestartIndex = NULL;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor = NULL;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced = NULL;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC 
	glDrawElementsInstancedBaseVertex = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding = NULL;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv = NULL;
PFNGLBINDBUFFERRANGEPROC glBindBufferRange = NULL;

PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation = NULL;
PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex = NULL;
PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv = NULL;






namespace akj{

void glInit()
{
	if (!gIsInit)
	{
		gIsInit = true;
		AKJ_GET_GL_FUNCTION_ADDRESS(glCreateProgram, PFNGLCREATEPROGRAMPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glCreateShader, PFNGLCREATESHADERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glCompileShader, PFNGLCOMPILESHADERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glDeleteShader, PFNGLDELETESHADERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glDeleteProgram, PFNGLDELETEPROGRAMPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glShaderSource, PFNGLSHADERSOURCEPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGetShaderiv, PFNGLGETSHADERIVPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glAttachShader, PFNGLATTACHSHADERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGetAttachedShaders, PFNGLGETATTACHEDSHADERSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glIsShader, PFNGLISSHADERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glLinkProgram, PFNGLLINKPROGRAMPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGetProgramiv, PFNGLGETPROGRAMIVPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glValidateProgram, PFNGLVALIDATEPROGRAMPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUseProgram, PFNGLUSEPROGRAMPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glCreateProgram, PFNGLCREATEPROGRAMPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUniform1i, PFNGLUNIFORM1IPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUniform2f, PFNGLUNIFORM2FPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUniform3f, PFNGLUNIFORM3FPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUniform4f, PFNGLUNIFORM4FPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUniform4fv, PFNGLUNIFORM4FVPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUniform1f, PFNGLUNIFORM1FPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBlendEquationSeparate, PFNGLBLENDEQUATIONSEPARATEPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGenBuffers, PFNGLGENBUFFERSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBindBuffer, PFNGLBINDBUFFERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBufferData, PFNGLBUFFERDATAPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glDeleteBuffers, PFNGLDELETEBUFFERSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBufferSubData, PFNGLBUFFERSUBDATAPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glVertexAttribIPointer, PFNGLVERTEXATTRIBIPOINTERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glMapBuffer, PFNGLMAPBUFFERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glUnmapBuffer, PFNGLUNMAPBUFFERPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glBindVertexArray, PFNGLBINDVERTEXARRAYPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glPrimitiveRestartIndex, PFNGLPRIMITIVERESTARTINDEXPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glVertexAttribDivisor, PFNGLVERTEXATTRIBDIVISORPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glDrawElementsInstanced, PFNGLDRAWELEMENTSINSTANCEDPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glMapBufferRange, PFNGLMAPBUFFERRANGEPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glDrawElementsInstancedBaseVertex,
			PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glDrawArraysInstanced, PFNGLDRAWARRAYSINSTANCEDPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glGetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glUniformBlockBinding, PFNGLUNIFORMBLOCKBINDINGPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(
			glGetActiveUniformBlockiv, PFNGLGETACTIVEUNIFORMBLOCKIVPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBindBufferRange, PFNGLBINDBUFFERRANGEPROC);
		//AKJ_GET_GL_FUNCTION_ADDRESS(
		//	glGetSubroutineUniformLocation,
		//	PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC);

		//AKJ_GET_GL_FUNCTION_ADDRESS(
		//	glGetSubroutineIndex, PFNGLGETSUBROUTINEINDEXPROC);
		//AKJ_GET_GL_FUNCTION_ADDRESS(
		//	glUniformSubroutinesuiv, PFNGLUNIFORMSUBROUTINESUIVPROC);

#ifdef WIN32
		AKJ_GET_GL_FUNCTION_ADDRESS(glActiveTexture, PFNGLACTIVETEXTUREPROC);
		AKJ_GET_GL_FUNCTION_ADDRESS(glBlendEquation, PFNGLBLENDEQUATIONPROC);
#endif // WIN32

		//PRINT_GL_INT_DEF(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		//PRINT_GL_INT_DEF(GL_MAX_VERTEX_ATTRIBS);
	//	PRINT_GL_INT_DEF(GL_MAX_TEXTURE_IMAGE_UNITS);
	//	PRINT_GL_INT_DEF(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
		//PRINT_GL_INT_DEF(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
	//	PRINT_GL_INT_DEF(GL_MAX_VARYING_FLOATS);
	//	PRINT_GL_INT_DEF(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
	//	PRINT_GL_INT_DEF(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
	}
	
}

const char* glErrorStringFromCode( int code )
{
	const char* ret = "";
	switch (code)
	{
	case GL_NO_ERROR:
		ret = "GL_NO_ERROR";
		break;
	case GL_INVALID_ENUM:
		ret = "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		ret = "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		ret = "GL_INVALID_OPERATION";
		break;
	case GL_STACK_OVERFLOW:
		ret = "GL_STACK_OVERFLOW";
		break;
	case GL_STACK_UNDERFLOW:
		ret = "GL_STACK_UNDERFLOW";
		break;
	case GL_OUT_OF_MEMORY:
		ret = "GL_OUT_OF_MEMORY";
		break;
	case GL_TABLE_TOO_LARGE:
		ret = "TABLE_TOO_LARGE";
		break;
	default:
		ret = "Unknown error";
	}
	return ret;
}

int glCheckAllErrors( const char* file, int line )
{
	int error_count = 0;
	for (GLint error_code = glGetError(); 
			error_code != GL_NO_ERROR && error_count < 64; 
			error_code = glGetError())
	{
		if(0 == error_count)
		{
			Log::Error("found glError(s) at %s: %d", file, line);
		}
		++error_count;

		Log::Error("## %d >> glError %x : %s", error_count, error_code,
			glErrorStringFromCode(error_code));
	}
	if(error_count > 0 && error_count < 64)
	{
		Log::Error("## end errors");
	}
	else if(error_count == 64)
	{
		FatalError::Die("glGetError returned non-0 64 times in a row: "
			"Our context is probably broken (and this driver is buggy)");
	}
	return error_count;
}
int glCheckAllErrors()
{
	int error_count = 0;
	for (GLint error_code = glGetError(); 
			error_code != GL_NO_ERROR && error_count < 64; 
			error_code = glGetError(), error_count++)
	{
		Log::Error( "## glError %x : %s", error_code, 
			glErrorStringFromCode(error_code));
	}
	return error_count;
}


const GLubyte* glTestTexture()
{
	static const GLubyte kTestImage[64] = 
	{
		0,   128,   124, 0,
		 255,   0,   0, 255,
		   0, 255,   0, 255,
		   0,   0, 255, 0,
		 255, 255,   0, 0,
		   0, 255, 255, 255,
		 255,   0, 255, 0,
		 255, 255,   0, 255,
		 255,   0, 255, 255,
		   0, 255,   0, 0,
		   0,   0, 255, 255,
		   0, 128,   0, 255,
		   0, 128,   0, 255,
		   0,   0, 128, 255,
		   0,   0, 128, 255,
		   0,   0, 128, 255,
	};
	return kTestImage;
}






const GLfloat kCubeVerts[108] =
{
	 0.5f, -0.5f, -0.5f, //0
	 0.5f, -0.5f,  0.5f, //1
	 0.5f,  0.5f, -0.5f, //2
	 0.5f,  0.5f, -0.5f, //2
	 0.5f, -0.5f,  0.5f, //1
	 0.5f,  0.5f,  0.5f, //3

	 0.5f,  0.5f, -0.5f, //2
	 0.5f,  0.5f,  0.5f, //3
	-0.5f,  0.5f, -0.5f, //6
	-0.5f,  0.5f, -0.5f, //6
	 0.5f,  0.5f,  0.5f, //3
	-0.5f,  0.5f,  0.5f, //7

	 0.5f, -0.5f,  0.5f, //1
	-0.5f, -0.5f,  0.5f, //5
	 0.5f,  0.5f,  0.5f, //3
	 0.5f,  0.5f,  0.5f, //3
	-0.5f, -0.5f,  0.5f, //5
	-0.5f,  0.5f,  0.5f, //7

	 0.5f, -0.5f, -0.5f, //0
	-0.5f, -0.5f, -0.5f, //4
	 0.5f, -0.5f,  0.5f, //1
	 0.5f, -0.5f,  0.5f, //1
	-0.5f, -0.5f, -0.5f, //4
	-0.5f, -0.5f,  0.5f, //5

	 0.5f,  0.5f, -0.5f, //2
	-0.5f,  0.5f, -0.5f, //6
	 0.5f, -0.5f, -0.5f, //0
	 0.5f, -0.5f, -0.5f, //0
	-0.5f,  0.5f, -0.5f, //6
	-0.5f, -0.5f, -0.5f, //4
	
	-0.5f, -0.5f, -0.5f, //4
	-0.5f, -0.5f,  0.5f, //5
	-0.5f,  0.5f, -0.5f, //6
	-0.5f,  0.5f, -0.5f, //6
	-0.5f, -0.5f,  0.5f, //5
	-0.5f,  0.5f,  0.5f  //7
};

const GLfloat kCubeNormals[108] = 
{
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,

	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,

	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,

	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,

	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,

	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f
};

const GLfloat kCubeTexCoords[108] = 
{
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,

	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,

	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,

	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,

	0.0f,  -1.0f,  0.0f,
	0.0f,  -1.0f,  0.0f,
	0.0f,  -1.0f,  0.0f,
	0.0f,  -1.0f,  0.0f,
	0.0f,  -1.0f,  0.0f,
	0.0f,  -1.0f,  0.0f,

	0.0f,  0.0f,  -1.0f,
	0.0f,  0.0f,  -1.0f,
	0.0f,  0.0f,  -1.0f,
	0.0f,  0.0f,  -1.0f,
	0.0f,  0.0f,  -1.0f,
	0.0f,  0.0f,  -1.0f
};

const GLfloat kFullScreenTriVerts[6] =
{
	-2.0f, 0.0f,
	0.0f, 2.0f,
	0.0f, 0.0f
};

const GLfloat kFullScreenTriTex[6] =
{
	 -2.0f, 0.0f,
	 0.0f, 2.0f,
		0.0f, 0.0f
};

const GLfloat kFullScreenTriNormals[9] =
{
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f
};

const GLfloat kBillBoardXVerts[54] =
{
	0.0f, -0.5f, -0.5f, //0
	0.0f, -0.5f,  0.5f, //1
	0.0f,  0.5f, -0.5f, //2
	0.0f,  0.5f, -0.5f, //2
	0.0f, -0.5f,  0.5f, //1
	0.0f,  0.5f,  0.5f, //3

	0.5f,  0.0f, -0.5f, //2
	0.5f,  0.0f,  0.5f, //3
	-0.5f,  0.0f, -0.5f, //6
	-0.5f,  0.0f, -0.5f, //6
	0.5f,  0.0f,  0.5f, //3
	-0.5f,  0.0f,  0.5f, //7

	0.5f, -0.5f,  0.0f, //1
	-0.5f, -0.5f,  0.0f, //5
	0.5f,  0.5f,  0.0f, //3
	0.5f,  0.5f,  0.0f, //3
	-0.5f, -0.5f,  0.0f, //5
	-0.5f,  0.5f,  0.0f, //7
};
const GLfloat kBillBoardXTex[54] = 
{
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,

	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,

	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f
};
const GLfloat kBillBoardXNormals[54] = 
{
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,

	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,

	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f,
	0.0f,  0.0f,  1.0f
};




}//end namespace akj
