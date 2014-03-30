#include "akjStaticResources.hpp"

// a string literal containing the contents of ..\src\shaders\text_shader.vert
// made with akjToStaticString.exe

namespace akj {
	static const char* text_shader_vert = 
		"#ifdef VERTEX_SHADER\n"
		"layout(location = 0) in vec2 aTexCoord;\n"
		"layout(location = 1) in vec2 aPos;\n"
		"layout(location = 2) in vec2 aVel;\n"
		"layout(location = 3) in vec2 aAccel;\n"
		"layout(location = 4) in float aStartTime;\n"
		"layout(location = 5) in vec3 aColor;\n"
		"\n"
		"uniform mat4 uProjectionMatrix;\n"
		"uniform float uCurrentTime;\n"
		"\n"
		"out vec2 vTexCoord;\n"
		"out vec3 vColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	vColor = aColor;\n"
		"\n"
		"	vTexCoord = aTexCoord;\n"
		"	//world_vertex += 0.001*noise_fac;\n"
		"	gl_Position = uProjectionMatrix*(vec4(aPos, -0.5, 1.0));\n"
		"}\n"
		"#endif\n";
}
