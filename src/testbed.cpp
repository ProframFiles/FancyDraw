#include "testbed.hpp"
#include "SDL.h"
#include "akjOGL.hpp"
#include "akjHWGraphicsContext.hpp"

namespace akj
{

	void TestDraw(){
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		

		// create a vertex buffer (This is a buffer in video memory)
		GLuint my_vertex_buffer;
		glGenBuffers(1 /*ask for one buffer*/, &my_vertex_buffer);

		const float a_2d_triangle[] =
		{
			200.0f, 10.0f,
			10.0f, 200.0f,
			400.0f, 200.0f
		};

		// GL_ARRAY_BUFFER indicates we're using this for 
		// vertex data (as opposed to things like feedback, index, or texture data)
		// so this call says use my_vertex_data as the vertex data source
		// this will become relevant as we make draw calls later 
		glBindBuffer(GL_ARRAY_BUFFER, my_vertex_buffer);


		// allocate some space for our buffer

		glBufferData(GL_ARRAY_BUFFER, 4096, NULL, GL_DYNAMIC_DRAW);

		// we've been a bit optimistic, asking for 4k of space even 
		// though there is only one triangle.
		// the NULL source indicates that we don't have any data 
		// to fill the buffer quite yet.
		// GL_DYNAMIC_DRAW indicates that we intend to change the buffer
		// data from frame-to-frame.
		// the idea is that we can place more than 3(!) vertices in the
		// buffer later as part of normal drawing activity

		// now we actually put the vertices into the buffer.
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(a_2d_triangle), a_2d_triangle);


		// next we need to define how the data contained in 
		// my_vertex_array is structured.
		// this state is contained in a vertex array object (VAO)
		// in modern OpenGL there needs to be at least one of these
		GLuint my_vao;
		glGenVertexArrays(1, &my_vao);

		//lets use the VAO we created
		glBindVertexArray(my_vao);

		// now we need to tell the VAO how the vertices in my_vertex_buffer
		// are structured
		// our vertices are really simple: each one contains 2 floats of position data
		// they could have been more complicated (texture coordinates, color -- 
		// whatever you want)

		// enable the first attribute in our VAO
		glEnableVertexAttribArray(0);

		// describe what the data for this attribute is like
		glVertexAttribPointer(0, // the index we just enabled
			2, // the number of components (our two position floats) 
			GL_FLOAT, // the type of each component
			false, // should the GL normalize this for us?
			2 * sizeof(float), // number of bytes until the next component like this
			(void*)0); // the offset into our vertex buffer where this element starts

		// OK, we have our source data all set up, now we can set up the shader
		// which will transform it into pixels

		// first create some ids
		GLuint my_shader_program = glCreateProgram();
		GLuint my_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		GLuint my_vertex_shader = glCreateShader(GL_VERTEX_SHADER);

		// we'll need to compile the vertex shader and fragment shader
		// and then link them into a full "shader program"

		const char* my_fragment_source = FragmentSourceFromSomewhere();

		// load one string from &my_fragment_source
		// the NULL indicates that the string is null-terminated
		glShaderSource(my_fragment_shader, 1, &my_fragment_source, NULL);
		// now compile it:
		glCompileShader(my_fragment_shader);

		// then check the result
		GLint compiled_ok;
		glGetShaderiv(my_fragment_shader, GL_COMPILE_STATUS, &compiled_ok);

		if (!compiled_ok){ printf("Oh Noes, fragment shader didn't compile!\n"); }
		else{
			glAttachShader(my_shader_program, my_fragment_shader);
		}

		// and again for the vertex shader
		const char* my_vertex_source = VertexSourceFromSomewhere();
		glShaderSource(my_vertex_shader, 1, &my_vertex_source, NULL);
		glCompileShader(my_vertex_shader);
		glGetShaderiv(my_vertex_shader, GL_COMPILE_STATUS, &compiled_ok);
		if (!compiled_ok){ printf("Oh Noes, vertex shader didn't compile!\n"); }
		else{
			glAttachShader(my_shader_program, my_vertex_shader);
		}

		//finally, link the program, and set it active
		glLinkProgram(my_shader_program);
		glUseProgram(my_shader_program);

		//get the screen size
		float my_viewport[4];
		glGetFloatv(GL_VIEWPORT, my_viewport);

		//now create a projection matrix
		float my_proj_matrix[16];
		MyOrtho2D(my_proj_matrix, 0.0f, my_viewport[2], my_viewport[3], 0.0f);

		GLuint my_projection_ref = 
			glGetUniformLocation(my_shader_program, "uProjectionMatrix");

		// send our projection matrix to the shader
		glUniformMatrix4fv(my_projection_ref, 1, GL_FALSE, my_proj_matrix );


		//clear the background
		glClearColor(0.3f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

		// *now* after that tiny setup, we're ready to draw the best 24 bytes of
		// vertex data ever.

		// draw the 3 vertices starting at index 0, interpreting them as triangles
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// now just swap buffers however your window manager lets you
	}

	const char* VertexSourceFromSomewhere()
	{
		return
			"#version 330 \n"
			"layout(location = 0) in vec2 inCoord; \n"
			"uniform mat4 uProjectionMatrix; \n"
			"void main() \n"
			"{"
			"gl_Position = uProjectionMatrix*(vec4(inCoord, 0, 1.0)); "
			"}";
	}

	const char* FragmentSourceFromSomewhere()
	{
		return
			"#version 330 \n"
			"out vec4 outFragColor;"
			"vec4 DebugMagenta(){ return vec4(1.0, 0.0, 1.0, 1.0); }\n"
			"void main() \n"
			"{\n"
			"   outFragColor = DebugMagenta();\n"
			"}\n";
	}

	void MyOrtho2D(float* mat, float left, float right, float bottom, float top)
	{
		// this is basically from
		// http://en.wikipedia.org/wiki/Orthographic_projection_(geometry)
		const float zNear = - 1.0f;
		const float zFar =  1.0f;
		const float inv_z = 1.0f / (zFar - zNear);
		const float inv_y = 1.0f / (top - bottom);
		const float inv_x = 1.0f / (right - left);

		//first column
		*mat++ = (2.0f*inv_x);
		*mat++ = (0.0f);
		*mat++ = (0.0f);
		*mat++ = (0.0f);

		//second
		*mat++ = (0.0f);
		*mat++ = (2.0f*inv_y);
		*mat++ = (0.0f);
		*mat++ = (0.0f);

		//third
		*mat++ = (0.0f);
		*mat++ = (0.0f);
		*mat++ = (-2.0f*inv_z);
		*mat++ = (0.0f);

		//fourth
		*mat++ = (-(right + left)*inv_x);
		*mat++ = (-(top + bottom)*inv_y);
		*mat++ = (-(zFar + zNear)*inv_z);
		*mat++ = (1.0f);
	}

} // namespace akj