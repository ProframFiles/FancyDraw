#pragma once
#include "FancyDrawMath.hpp"
#include "akjHWGraphicsObject.hpp"


namespace akj
{
	class cArrayBuffer;
	class cShaderObject : public cHWGraphicsObject
	{
	public:
		enum eShaderType{
			VERTEX_SHADER,
			GEOMETRY_SHADER,
			FRAGMENT_SHADER
		};
		cShaderObject(cHWGraphicsContext* context, const Twine& object_name);
		~cShaderObject();
		void CreateShaderProgram(cStringRef shader_source);

		void Bind();
		void UnBind();
		
		template <class tVector>
		void BindFragmentSubRoutines(const tVector& vec)
		{
			BindFragmentSubRoutinesImpl(vec.data(), u32(vec.size()));
		}
		
		uint32_t GetSubroutineInstance(eShaderType type, cStringRef name);
		uint32_t GetSubroutineIndex(eShaderType type, cStringRef name);
		void BindProjectionMatrix(const float* mat);
		// returns the binding number
		int BindUniformBlock(cStringRef block_name, 
													cBufferRange& buffer_range, uint32_t bind_point);
		void BindUniformToVec4Array( const char* uniform_name,
																const std::vector<cCoord4>& vec );
		void BindUniformToInt(const char* uniform_name, int value);
		void BindUniformToFloat(const char* uniform_name, float value);
		void BindUniformToVec2(const char* uniform_name, float v1, float v2);
		void BindUniformToVec3(const char* uniform_name, const cCoord3& val);
		void BindUniformToVec4(const char* uniform_name, float v1, float v2, float v3, float v4 );
		void BindUniformToVec4(const char* uniform_name, const cCoord4& val );
		void BindUniformToQuat(const char* uniform_name,const cUnitQuat& q)
		{
			BindUniformToVec4(uniform_name, q.s.x, q.s.y, q.s.z, q.w);
		}
		void BindUniformToVec4Array(const char* uniform_name, const void* first_element, int num_elements);
		int GetUniformLocation(const char * uniform_name) const;
		bool IsBound() const;
		uint32_t GetUniformBlockSize(uint32_t index);
	private:
		void BindFragmentSubRoutinesImpl(
			const uint32_t* routine_ids, uint32_t num_ids);
		bool SetFragmentShader(const char* string_ptr);
		bool SetVertexShader(const char* string_ptr);
		bool SetGeometryShader(const char* string_ptr);
		bool LinkShaderProgram();
		bool IsShaderAttached(uint32_t shader_id);
		int GetShaderType(uint32_t shader_id);
		int LoadShaderImpl(eShaderType type, const char* string_ptr);
		


		uint32_t mShaderProgram;
		uint32_t mFragmentShader;
		uint32_t mGeometryShader;
		uint32_t mVertexShader;
		uint32_t mProjectionUniform;
		uint32_t mNextUniformBinding;
		std::string mVersionString;
		uint32_t mErrorCount;
		
	};

	class cBindShader : public iDrawCommand
	{
	public:
		cBindShader( cShaderObject& shader)
			:mShader(shader){}
		virtual void Execute()
		{
			mShader.Bind();
		}
		cShaderObject& mShader;
	};

	class cBindUniformInt : public iDrawCommand
	{
	public:
		cBindUniformInt(cStringRef u_name, cShaderObject& shader, uint32_t val)
			: mName(u_name), mShader(shader), mValue(val){}
		virtual void Execute()
		{
			mShader.BindUniformToInt(mName.data(), mValue);
		}
		cStringRef mName;
		cShaderObject& mShader;
		uint32_t mValue;
	};



	class cBindUniformFloat : public iDrawCommand
	{
	public:
		cBindUniformFloat(cStringRef name, cShaderObject& shader, float val)
			:mName(name), mShader(shader), mValue(val){}
		virtual void Execute()
		{
			mShader.BindUniformToFloat(mName.data(), mValue);
		}
		cStringRef mName;
		cShaderObject& mShader;
		float mValue;
	};

	class cBindUniformBlock : public iDrawCommand
	{
	public:
		cBindUniformBlock(cStringRef name, const cBufferRange& range, 
												cShaderObject& shader, uint32_t index)
			:mName(name), mBindRange(range), mShader(shader), mIndex(index){}
		virtual void Execute()
		{
			mShader.BindUniformBlock(mName, mBindRange ,mIndex);
		}
		cStringRef mName;
		cBufferRange mBindRange;
		cShaderObject& mShader;
		uint32_t mIndex;
	};

}
