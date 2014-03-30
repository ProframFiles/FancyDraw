#include "akjHWGraphicsContext.hpp"
#include "akjHWGraphicsObject.hpp"
#include "SDL.h"
#include "akjOGL.hpp"
#include "CL1_1/cl.hpp"
#include "FancyDrawMath.hpp"
#include "FatalError.hpp"
#include "akjSaveImageTask.hpp"
#include "akjWorkerPool.hpp"
#include "akjBitmapOperations.hpp"


namespace akj{
	template <class tVec>	
	void PlatformSpecificSharingProps(tVec& props)
	{
	    // Define OS-specific context properties and create the OpenCL context
    #if defined (__APPLE__)
        CGLContextObj kCGLContext = CGLGetCurrentContext();
        CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
        props.push_back(
            {
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 
        });
    #else
        #ifdef UNIX
            props.push_back({
                CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(), 
                CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(), 
            });
        #else // Win32
            props.push_back(
            {
                CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), 
                CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), 
            });
        #endif
    #endif
	}



	cHWGraphicsContext::cHWGraphicsContext(SDL_Window* window)
		: mGLWindow(window)
		, mCoverage(ivec2(512, 512))
		, mDrawCounter(1)
	{
		ResetDepthRange();
		AKJ_ASSERT(mGLWindow != NULL);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);

		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);

		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);

		//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,0);
	
#ifdef _DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
		SDL_GLContext context = SDL_GL_CreateContext(mGLWindow);
		
		if (!context)
		{
			std::string error_string = SDL_GetError();
			Log::Error("could not create an OGL context at the SDL level: %s", error_string.c_str());
			FatalError::Die(error_string.c_str());
		}
		else if(!glCheckAllErrors(__FILE__,__LINE__)){
			Log::Info("successfully created GL context:\n%s", glGetString(GL_VERSION));
		}
		int height, width;
		SDL_GetWindowSize(mGLWindow, &width, &height);
		glInit();
		SetViewPort(0, 0, width, height);
		
		//glStencilMask(0);
		
		glDepthFunc(GL_ALWAYS);
		DisableDepth();

		glEnable(GL_BLEND);
		SetBlendMode(BLEND_ORDINARY);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glPrimitiveRestartIndex(kPrimitiveRestartIndex);
		glCheckAllErrors(__FILE__,__LINE__);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,&mUniformAlign);
		
		raw_ostream& outs = Log::Debug();
		outs << "OpenCL Init\n";
		std::vector< cl::Platform > platformList;
		std::vector< cl::Device > deviceList;
		outs.flush();
		cl::Platform::get(&platformList);
		if(platformList.empty()) 
		{
			AKJ_THROW("No OpenCL 1.1 capable platforms found");
		}

		uint32_t ocl_platform_metric = 0;
		uint32_t best_metric = 0;
		cl::Platform* best_platform = nullptr;
		std::vector< cl::Device > best_device;

		outs << "\tNumber of OpenCL Platforms: " << platformList.size() << "\n";
		uint32_t index_metric = u32(platformList.size());
		outs.flush();
		for(auto & platform: platformList){
			deviceList.clear();
			uint32_t metric = index_metric--;
			std::string platformVendor;
			platform.getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &platformVendor);
			outs << "\tPlatform is by: " << platformVendor << "\n";
			std::string platform_extensions;
			platform.getInfo((cl_platform_info)CL_PLATFORM_EXTENSIONS, &platform_extensions);
			//outs << "\t\tPlatform extensions: " << platform_extensions << "\n";
			const bool plat_share = std::string::npos != platform_extensions.find(CLGL_SHARING_EXTENSION);
			platform.getDevices(CL_DEVICE_TYPE_DEFAULT, &deviceList);
			uint32_t dev_index = 0;
			for(auto& device : deviceList){
				std::string dev_extensions;
				std::string dev_name;
				device.getInfo((cl_platform_info)CL_DEVICE_NAME, &dev_name);
				device.getInfo((cl_platform_info)CL_DEVICE_EXTENSIONS, &dev_extensions);
				bool dev_share = false;
				outs << "\t\tDevice: " << dev_name << "\n";
				if(!dev_extensions.empty())
				{
					outs << "\t\t\tDevice extensions: " << dev_extensions << "\n";
					dev_share = std::string::npos != dev_extensions.find(CLGL_SHARING_EXTENSION);
				}
				
				if(!dev_share && !plat_share)
				{
					metric = 0;
					outs << "\t\tNo sharing of this device\n";
				}

				if(metric > best_metric){
					best_platform = &platform;
					best_metric = metric;
					if(best_device.empty()) best_device.push_back(device);
					else best_device.at(0) = device; 
				}
				dev_index++;
			}
		}
		outs.flush();
		AKJ_ASSERT_AND_THROW(best_platform);
		std::string platform_string;
		std::string device_string;
		best_device.back().getInfo((cl_device_info)CL_DEVICE_VENDOR, &platform_string);
		best_device.back().getInfo((cl_device_info)CL_DEVICE_NAME, &device_string);
		outs << "Choose " << platform_string << ": "<< device_string <<"\n";


		cArray<16, cl_context_properties> cprops;
		cprops.emplace_back(CL_CONTEXT_PLATFORM);
		cprops.emplace_back((cl_context_properties) (*best_platform)());
		PlatformSpecificSharingProps(cprops);
		cprops.emplace_back(0);
	
		int err = 0;
		mComputeContext.reset(new cl::Context( best_device, cprops.data(), NULL, NULL, &err));

		if(!mComputeContext || err != 0)
		{
			AKJ_THROW("Unable to create OpenCL context");
		}

		outs.flush();

		mBackGroundSurface.reset( new cBackgroundSurface(this, "Default background surface"));
		
	}

	cHWGraphicsContext::~cHWGraphicsContext()
	{
		mComputeContext.reset();
		SDL_GLContext context = SDL_GL_GetCurrentContext();
		SDL_GL_DeleteContext(context);
	}

	void cHWGraphicsContext::Clear(const RGBAf& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void cHWGraphicsContext::SetBlendMode(eBlendMode mode)
	{
		switch (mode)
		{
		case akj::cHWGraphicsContext::BLEND_ORDINARY:
			glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case akj::cHWGraphicsContext::BLEND_SATURATION:
			glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_DST_ALPHA);
			glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
			break;
		case akj::cHWGraphicsContext::BLEND_ADD:
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
	case akj::cHWGraphicsContext::BLEND_MAX:
			glBlendEquation(GL_MAX);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		default:
			break;
		}
	}

	void cHWGraphicsContext::ClearColor(const akj::RGBAf& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	akj::RGBAf cHWGraphicsContext::ClearColor()
	{
		float c[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, c);
		return RGBAf(c[0], c[1], c[2], c[3]);
	}


	void cHWGraphicsContext::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void cHWGraphicsContext::ClearDepth(float val)
	{
		glClearDepth(val);
		mCoverage.Clear();
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void cHWGraphicsContext::SetViewPort(int x, int y, int width, int height)
	{
		glViewport(x, y, width, height);
		int new_x, new_y;
		ViewPort(&new_x, &new_y);
		AKJ_ASSERT(new_x == width && new_y == height);
		mCoverage.Reset(new_x, new_y);
	}

	void cHWGraphicsContext::Swap()
	{
		if(!mBackBufferSave.empty())
		{
			glFinish();
			SaveBackBufferImpl();
		}

		SDL_GL_SwapWindow(mGLWindow);
		glFinish();
		
	}

	static const float kIdentityMat4f[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	std::vector<float> cHWGraphicsContext::OrthoMatrix(float x_size,float y_size)
	{
		//this is custom: will map 0 to -0.5, and max to max+0.5
		// this is apparently what is needed to map pixels properly
		// under opengl (my testing has shown that this is how it works)

		const float bottom = y_size;
		const float right = x_size;
		const float zFar = 1.0;
		const float zNear = -10.0;
		const float inv_z = 1.0f / (zFar - zNear);
		const float inv_y = 1.0f / (bottom);
		const float inv_x = 1.0f / (right);
		std::vector<float> ret;
		ret.reserve(16);

		//first column
		ret.push_back(2.0f*(1.0f+right)*inv_x*inv_x);
		ret.push_back(0.0f);
		ret.push_back(0.0f);
		ret.push_back(0.0f);

		//second
		ret.push_back(0.0f);
		ret.push_back(-2.0f*(1.0f+bottom)*inv_y*inv_y);
		ret.push_back(0.0f);
		ret.push_back(0.0f);

		//third
		ret.push_back(0.0f);
		ret.push_back(0.0f);
		ret.push_back(-2.0f*inv_z);
		ret.push_back(0.0f);

		//fourth
		ret.push_back(-(right+1)*inv_x);
		ret.push_back((bottom+1)*inv_y);
		ret.push_back(-(zFar + zNear)*inv_z);
		ret.push_back(1.0f);

		return ret;
	}

	void cHWGraphicsContext::MyOrtho2D(float* mat, float left, float right, float bottom, float top)
	{
		// this is basically from
		// http://en.wikipedia.org/wiki/Orthographic_projection_(geometry)
		const float zNear = -10.0f;
		const float zFar = 1.0f;
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

	void cHWGraphicsContext::GenerateTriStripIndices(	uint32_t starting_index, 
																										uint32_t num_triangles,
																										std::vector<uint32_t>& vec)
	{
		for (uint32_t i = 0; i < num_triangles; ++i)
		{
			if (i & 1)
			{
				vec.push_back(starting_index + i+1 + 0);
				vec.push_back(starting_index + i+1 - 1);
				vec.push_back(starting_index + i+1 + 1);
			}
			else
			{
				vec.push_back(starting_index + (i) + 0);
				vec.push_back(starting_index + (i) + 1);
				vec.push_back(starting_index + (i) + 2);
			}
		}
	}

	const float* cHWGraphicsContext::IdentityMatrix()
	{
		return kIdentityMat4f;
	}

	void cHWGraphicsContext::ViewPort(int* width, int* height)
	{
		ivec2 ret = ViewPort();
		*width = ret.x;
		*height = ret.y;
	}

	ivec2 cHWGraphicsContext::ViewPort() const
	{
		ivec2 ret;
		int vals[4];
		glGetIntegerv(GL_VIEWPORT, vals);
		AKJ_ASSERT(vals[0]==0 && vals[1]==0);
		ret.x = vals[2];
		ret.y = vals[3];
		return ret;
	}

	int cHWGraphicsContext::CheckErrors(const Twine& where_and_why)
	{
		int count = glCheckAllErrors(__FILE__, __LINE__);
		if(count > 0)
		{
			Log::Error("Errors are from: %s", where_and_why.str());
		}
		return count;
	}

	void cHWGraphicsContext::AssertErrorFree(const Twine& reason)
	{
		int num_errors = 0;
		if(num_errors = glCheckAllErrors(__FILE__, __LINE__))
		{
			AKJ_ASSERT(num_errors == 0);
			FatalError::Die("Failed graphics context error-free assertion:"
											+ reason + " Failed.");
		}
	}

	cPushedBGColor cHWGraphicsContext::PushBackGroundColor(const RGBAf& color)
	{
		RGBAf old_color = ClearColor();
		ClearColor(color);
		return cPushedBGColor(this, old_color);
	}

	cSparseCoverageMap& cHWGraphicsContext::Coverage()
	{
		return mCoverage;
	}

	void cHWGraphicsContext::ResetFrameBuffer()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void cHWGraphicsContext::BlockUntilFinished()
	{
		glFinish();
	}

	cBitmap<4> cHWGraphicsContext::SaveAlphaBuffer(cAlignedBuffer& buffer)
	{
		//mAlphaBuffer.BindForRead();
		ivec2 vp = ViewPort();
		cBitmap<4> bitmap(vp.x, vp.y, 2, BIT_DEPTH_32);
		AlignedBuffer<16> bitmap_data;
		bitmap.UseAsStorage(bitmap_data);

		glReadPixels(0, 0, bitmap.Width(), bitmap.Height(),
			GL_RG, GL_FLOAT, bitmap.Data());

		cBitmap<4> converted(buffer, vp.x, vp.y, 1, BIT_DEPTH_8);

		bitmap.ForEachPixelFlipVert([]
			(uint8_t* src, uint8_t* dst)
		{
			const float r = *reinterpret_cast<float*>(src);
			const float g = *reinterpret_cast<float*>(src+4);
			*dst = static_cast<uint8_t>
						((1.0f-std::min((r), 1.0f))*255.9f);
		}, converted);
		return converted;
	}	

	cBitmap<4> cHWGraphicsContext::GetBackBuffer(cAlignedBuffer& storage)
	{
		CheckErrors(AKJ_LOG_CSTR("cHWGraphicsContext::SaveBackBufferImpl"));
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		ivec2 vp = ViewPort();
		
		cBitmap<4> bitmap(storage, vp.x, vp.y, 4, BIT_DEPTH_8);
		
		glReadPixels(0, 0, bitmap.Width(), bitmap.Height(),
			GL_RGBA, GL_UNSIGNED_BYTE, bitmap.Data());
		CheckErrors(AKJ_LOG_CSTR("cHWGraphicsContext::SaveBackBufferImpl"));
		
		FlipVertically(bitmap);
		return bitmap;
	}
	
	void cHWGraphicsContext::SaveBackBufferImpl()
	{
		if(mWorkerPool == nullptr)
		{
			Log::Error("Unexpected Null pointer at" 
									AKJ_LOG_CSTR("SaveBackBufferImpl"));
			return;
		}
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		ivec2 vp = ViewPort();
		
		cBitmap<4> bitmap( vp.x, vp.y, 4, BIT_DEPTH_8);
		std::unique_ptr<cSaveImageTask> task(
			new cSaveImageTask(bitmap, mBackBufferSave)
		);
		
		glReadPixels(0, 0, bitmap.Width(), bitmap.Height(),
			GL_RGBA, GL_UNSIGNED_BYTE, task->Bitmap().Data());
		CheckErrors(AKJ_LOG_CSTR("cHWGraphicsContext::SaveBackBufferImpl"));
		
		FlipVertically(task->Bitmap());
		mWorkerPool->AddTask(std::move(task));
		mWorkerPool = nullptr;
		mBackBufferSave.clear();
	}


	void cHWGraphicsContext::DrawBackground()
	{
		mBackGroundSurface->Draw();

	}

	void cHWGraphicsContext
		::SubmitFBO(const std::string& name, cFrameBufferObject* FBO)
	{
		mFrameBuffers[name] = FBO;
	}

	cFrameBufferObject* cHWGraphicsContext::GetFBO(const std::string& name)
	{
		auto found = mFrameBuffers.find(name);
		if(found == mFrameBuffers.end())
		{
			return nullptr;
		}
		return found->second;
	}

	void cHWGraphicsContext::EnableDepthWrites()
	{
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
	}

	void cHWGraphicsContext::DisableDepth()
	{
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		//glDepthFunc(GL_ALWAYS);
	}

	void cHWGraphicsContext::SaveBackBuffer(cWorkerPool& task_pool, 
																					const Twine& filename)
	{
		mBackBufferSave = filename.str();
		mWorkerPool = & task_pool;
	}

	cPushedBGColor::cPushedBGColor(cHWGraphicsContext* context, RGBAf old_color)
		:mOldColor(old_color)
		,mContext(context)
	{}

	cPushedBGColor::cPushedBGColor(cPushedBGColor& other)
		:mContext(other.mContext)
	{
		other.mContext = NULL;
	}

	cPushedBGColor::~cPushedBGColor()
	{
		if(mContext)
		{
			mContext->ClearColor(mOldColor);
		}
	}


} // namespace akj