#include "akjShaderObject.hpp"
#include "akjOGL.hpp"
#include "FatalError.hpp"

namespace akj
{

uint32_t GLShaderType(cShaderObject::eShaderType t)
{
	switch (t)
	{
	case akj::cShaderObject::VERTEX_SHADER:
		return GL_VERTEX_SHADER;
	case akj::cShaderObject::GEOMETRY_SHADER:
		return GL_GEOMETRY_SHADER;
	case akj::cShaderObject::FRAGMENT_SHADER:
		return GL_FRAGMENT_SHADER;
	default:
		break;
	}
	AKJ_THROW("unknown shader type");
	return 0;
}

cShaderObject::cShaderObject(cHWGraphicsContext* context,
															const Twine& object_name)
	:cHWGraphicsObject(context, object_name)
	,mShaderProgram(0)
	,mGeometryShader(0)
	,mFragmentShader(0)
	,mVertexShader(0)
	,mProjectionUniform(-1)
	,mNextUniformBinding(0)
	,mErrorCount(0)
{
	cStringRef version = glGetString(GL_SHADING_LANGUAGE_VERSION);
	AKJ_ASSERT(version.size() >= 3);
	mVersionString.reserve(15);
	mVersionString = "#version ";
	size_t space_pos = version.find_first_of(" ");
	mVersionString.push_back(version.data()[0]);
	mVersionString.push_back(version.data()[2]);
	if(space_pos > 3)
	{
		mVersionString.push_back(version.data()[3]);
	}
	mVersionString.push_back('\n');
	mShaderProgram = glCreateProgram();
	if(glCheckAllErrors(__FILE__, __LINE__))
	{
		Log::Error("there were errors constructing the shader object.");
	}
}

cShaderObject::~cShaderObject()
{
	glDeleteProgram(mShaderProgram);
	glDeleteShader(mFragmentShader);
	glDeleteShader(mGeometryShader);
	glDeleteShader(mVertexShader);
}

void cShaderObject::CreateShaderProgram(cStringRef shader_source)
{
	// chop off leading newlines if possible: will make so that the source
	// file line numbers will match what the compiler sees
	cStringRef ss = shader_source;
	SetFragmentShader(ss.data());
	if(shader_source.find("{akj:use geometry}") != cStringRef::npos)
	{
		SetGeometryShader(ss.data());
	}
	SetVertexShader(ss.data());
	LinkShaderProgram();
	int is_linked;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &is_linked);
	if(!is_linked)
	{
		AKJ_THROW("Shader "+mObjectName + " Linking Failed.");
	}
}

bool cShaderObject::SetFragmentShader( const char* string_ptr)
{
	if(mFragmentShader == 0)
	{
		mFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	}
	if(GL_FRAGMENT_SHADER != GetShaderType(mFragmentShader))
	{
		Log::Error("mFragmentShader is not a valid shader object");
		return false;
	}
	if(GL_NO_ERROR != LoadShaderImpl(FRAGMENT_SHADER, string_ptr))
	{
		return false;
	}
	return true;
}

bool cShaderObject::SetGeometryShader(const char* string_ptr)
{
	if (mGeometryShader == 0)
	{
		mGeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
	}
	if (GL_GEOMETRY_SHADER != GetShaderType(mGeometryShader))
	{
		Log::Error("mGeometryShader is not a valid shader object");
		return false;
	}
	if (GL_NO_ERROR != LoadShaderImpl(GEOMETRY_SHADER, string_ptr))
	{
		FatalError::Die("broken geometry shader");
	}
	return true;
}


bool cShaderObject::SetVertexShader(const char* string_ptr)
{
	if (mVertexShader == 0)
	{
		mVertexShader = glCreateShader(GL_VERTEX_SHADER);
	}
	if(GL_VERTEX_SHADER != GetShaderType(mVertexShader))
	{
		Log::Error("mVertexShader is not a valid shader object");
		return false;
	}
	if(GL_NO_ERROR != LoadShaderImpl(VERTEX_SHADER, string_ptr))
	{
		return false;
	}
	return true;
}

int cShaderObject::LoadShaderImpl(eShaderType type, const char* shader_source)
{
	const char* shader_name = mObjectName.c_str();
	const char* shader_stage;
	uint32_t shader_id;
	const char* type_define;
	switch (type)
	{
	case VERTEX_SHADER:
		shader_id = mVertexShader;
		type_define = "#define VERTEX_SHADER 1\n";
		shader_stage = "vertex";
		break;
	case GEOMETRY_SHADER:
		shader_stage = "geometry";
		shader_id = mGeometryShader;
		type_define = "#define GEOMETRY_SHADER 1\n";
		break;
	case FRAGMENT_SHADER:
		shader_stage = "fragment";
		shader_id = mFragmentShader;
		type_define = "#define FRAGMENT_SHADER 1\n";
		break;
	default:
		FatalError::Die("unknown shader type");
		break;
	}

	cArray<16, const char*> string_pointers;
	string_pointers.push_back(mVersionString.c_str());
	string_pointers.push_back(type_define);
	string_pointers.push_back("#line 2\n");
	string_pointers.push_back(shader_source);

	glShaderSource(shader_id, string_pointers.size(), &string_pointers[0], NULL);
	if(glCheckAllErrors(__FILE__, __LINE__))
	{
		Log::Error("shader source error for shader '%s' (ID 0x%x).",
			shader_name, shader_id );
		return 1;
	}
	if(GL_FALSE == glIsShader(shader_id))
	{
		Log::Error("somehow this is not a shader object (id = %x).", shader_id );
		return 1;
	}

	glCompileShader(shader_id);
	
	if(glCheckAllErrors(__FILE__, __LINE__))
	{
		AKJ_THROW("Errors compiling shader "+Twine(shader_id)+ 
							": \""+shader_name+"\".");
	}

	const char* compile_status_string = "Failed";
	GLint compiled_ok = GL_FALSE;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);
	if(compiled_ok == GL_TRUE)
	{
		compile_status_string = "Succeeded";
	}
	Log::Info("%s: Compilation of shader \"%s\" (%s).",
		compile_status_string, shader_name, shader_stage);
	GLint log_length = 0; 
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
	std::string info_log;
	if(log_length > 1 && compiled_ok != GL_TRUE)
	{
		GLsizei actual_length = 0;
		info_log.resize(log_length, ' ');
		glGetShaderInfoLog(shader_id, log_length, &actual_length, &info_log[0]);
		info_log.resize(actual_length);
		AKJ_THROW(Twine("Shader ") + shader_name 
							+ " Compile failed: \n" + info_log.c_str());
	}
	
	if(glCheckAllErrors(__FILE__, __LINE__) || GL_FALSE == compiled_ok)
	{
		AKJ_THROW("Errors compiling shader "+Twine(shader_id)+ 
							": \""+shader_name+"\".");
	}

	if(!IsShaderAttached(shader_id))
	{
		glAttachShader(mShaderProgram, shader_id);
	}

	if(glCheckAllErrors(__FILE__, __LINE__))
	{
		Log::Warn("there were errors attaching shader to program for shader %x"
							", name: %s.", shader_id, shader_name );
		return 1;
	}
	
	//everything went better than expected :)
	return GL_NO_ERROR;
}

bool cShaderObject::IsShaderAttached( GLuint shader_id )
{
	const GLsizei max_shaders = 4;
	GLsizei actual_shaders = 0;
	GLuint shaders[max_shaders];
	for (int i = 0 ; i < max_shaders; ++i)
	{
		shaders[i] = 0;
	}
	glGetAttachedShaders(mShaderProgram, max_shaders, &actual_shaders, shaders);
	for (int i = 0; i < actual_shaders; ++i)
	{
		if(shader_id == shaders[i])
		{
			return true;
		}
	}
	return false;
}

GLint cShaderObject::GetShaderType( GLuint shader_id )
{
	GLint shader_type = 0;
	glGetShaderiv(shader_id, GL_SHADER_TYPE, &shader_type);
	if(glCheckAllErrors(__FILE__, __LINE__))
	{
		Log::Warn("errors checking the shader type for shader %x", shader_id);
	}
	return shader_type;
}

bool cShaderObject::LinkShaderProgram()
{
	glLinkProgram(mShaderProgram);
	int num_errors = glCheckAllErrors(__FILE__, __LINE__);
	if(num_errors >0)
	{
		Log::Warn("errors linking shader %x (\"%s\")", mShaderProgram, mObjectName);
	}
	const char* link_status_string = "Failed";
	GLint linked_ok = GL_FALSE;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &linked_ok);
	if(linked_ok == GL_TRUE)
	{
		link_status_string = "Succeeded";
	}
	Log::Info("%s: Linking of shader.", link_status_string);

	GLint log_length = 0; 
	glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &log_length);
	std::string info;
	if(log_length > 1)
	{
		GLsizei actual_length = 0;
		info.resize(log_length, ' ');
		glGetProgramInfoLog(mShaderProgram, log_length, &actual_length, &info[0]);
		info.resize(actual_length);
		Log::Info("log:\n%s", info.c_str());
	}

	mProjectionUniform 
		= glGetUniformLocation(mShaderProgram, "uProjectionMatrix");
	
	AKJ_ASSERT(num_errors==0);

	return linked_ok == GL_TRUE;
}

void cShaderObject::Bind()
{
	GLint is_linked = GL_FALSE;
	glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &is_linked);
	AKJ_ASSERT(is_linked);

	glUseProgram(mShaderProgram);
	if(glCheckAllErrors(__FILE__, __LINE__))
	{
		Log::Warn("there were errors when trying to use the shader program\n");
	}
}

void cShaderObject::UnBind()
{
	glUseProgram(0);
}


GLint cShaderObject::GetUniformLocation( const char * uniform_name ) const
{
	return glGetUniformLocation(mShaderProgram, uniform_name);
}

void cShaderObject::BindUniformToInt( const char* uniform_name, int value )
{
	Bind();
	GLint uniform_location = GetUniformLocation(uniform_name);
	glUniform1i(uniform_location, value);
	glCheckAllErrors(__FILE__,__LINE__);
}

void cShaderObject::BindUniformToFloat( const char* uniform_name, float value )
{
	Bind();
	GLint uniform_location = GetUniformLocation(uniform_name);
	glUniform1f(uniform_location, value);
	glCheckAllErrors(__FILE__,__LINE__);
}

void cShaderObject::
BindUniformToVec4Array( const char* uniform_name,
												const void* first_element, 
												int num_elements )
{
	Bind();
	GLint uniform_location = GetUniformLocation(uniform_name);
	//AKJ_ASSERT(uniform_location >=0 );
	glUniform4fv(uniform_location, num_elements, (const GLfloat*)first_element);
	glCheckAllErrors(__FILE__,__LINE__);
}

void cShaderObject::BindUniformToVec2( const char* uniform_name, float v1, float v2 )
{
	Bind();
	GLint uniform_location = GetUniformLocation(uniform_name);
	glUniform2f(uniform_location, v1, v2);
	glCheckAllErrors(__FILE__,__LINE__);
}
void cShaderObject::BindUniformToVec4( const char* uniform_name,const cCoord4& val  )
{
	Bind();
	GLint uniform_location = GetUniformLocation(uniform_name);
	glUniform4f(uniform_location, val.x, val.y, val.z, val.w);
	glCheckAllErrors(__FILE__,__LINE__);
}

void cShaderObject::BindUniformToVec4( const char* uniform_name, float v1, float v2, float v3, float v4  )
{
	Bind();
	GLint uniform_location = GetUniformLocation(uniform_name);
	glUniform4f(uniform_location, v1, v2, v3, v4);
	
	glCheckAllErrors(__FILE__,__LINE__);
}

void cShaderObject::BindUniformToVec4Array( const char* uniform_name,const std::vector<cCoord4>& vec )
{
	Bind();
	GLint uniform_location = GetUniformLocation(uniform_name);
	glUniform4fv(uniform_location, static_cast<GLsizei>(vec.size()),
		reinterpret_cast<const GLfloat*>(&vec.at(0)));
}

void cShaderObject::BindUniformToVec3( const char* uniform_name, const cCoord3& val )
{
	Bind();
	GLint uniform_location = GetUniformLocation(uniform_name);
	glUniform3f(uniform_location, val.x, val.y, val.z);
	glCheckAllErrors(__FILE__,__LINE__);
}

void cShaderObject::BindProjectionMatrix(const float* mat)
{
	Bind();
	if(mProjectionUniform < 0){
		return;
	}
	glUniformMatrix4fv(mProjectionUniform, 1, GL_FALSE, mat );
	glCheckAllErrors(__FILE__,__LINE__);
}


bool cShaderObject::IsBound() const
{
	GLint shader_program = -1;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader_program);
	return shader_program >=0 && shader_program == mShaderProgram;
}

uint32_t cShaderObject::GetUniformBlockSize(uint32_t index)
{
	int ret;
	glGetActiveUniformBlockiv(mShaderProgram,index,
														 GL_UNIFORM_BLOCK_DATA_SIZE, &ret);
	return ret;
}

int cShaderObject
::BindUniformBlock(cStringRef block_name, cBufferRange& range, uint32_t i)
{
	Bind();
	// so there's an index, and also a separate binding? ok...
	GLuint id = glGetUniformBlockIndex(mShaderProgram, block_name.data());
	if( id < 0 )
	{
		AKJ_THROW("Unable to bind uniform \"" + block_name + "\": name not found");
	}

	glUniformBlockBinding(mShaderProgram, id, i);
	range.BindRangeToIndex(i);
	

	int ret;
	glGetActiveUniformBlockiv(mShaderProgram, id, GL_UNIFORM_BLOCK_BINDING, &ret);
	if(ret != i)
	{
		AKJ_THROW("Unable to bind uniform \"" + block_name + "\" to index " + 
							Twine(i));
	}
	return ret;
}

uint32_t cShaderObject::GetSubroutineIndex(eShaderType type, cStringRef name)
{
	uint32_t ret =  glGetSubroutineUniformLocation(
			mShaderProgram, GLShaderType(type), name.data());
	AKJ_ASSERT_AND_THROW(ret != GL_INVALID_INDEX);
	return ret;

}

void cShaderObject::BindFragmentSubRoutinesImpl(
	const uint32_t* routine_ids, uint32_t num_ids)
{
	glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, num_ids, routine_ids);
}

uint32_t cShaderObject::GetSubroutineInstance(eShaderType type, cStringRef name)
{
	uint32_t ret 
		= glGetSubroutineIndex(mShaderProgram, GLShaderType(type), name.data());
	AKJ_ASSERT_AND_THROW(ret != GL_INVALID_INDEX);
	return ret;
}




} // namespace akj