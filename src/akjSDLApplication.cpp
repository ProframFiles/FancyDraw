#include "akjSDLApplication.hpp"
#include "SDL.h"
#include "akjLog.hpp"
#include "FL/Fl_Window.H"
#include "akjHWGraphicsContext.hpp"
#include "FileChooserDialog.hpp"

namespace akj{

	void* GetNativeHandle()
	{
		return NULL;
	}

	cSDLApplication::
		cSDLApplication(cStringRef window_title, int width, int height) 
		: mInitFlags(0)
		, mWindow(NULL)
		, mDesiredState(STATUS_RUN)
		, mVSyncState(VSYNC_DISABLED)
		, mMainWindowID(0)
	{
		Init();
		MakeWindow(window_title, width, height);
		CreateOpenGLContext();
		SetVsync(mVSyncState);
	}

	// The Event Handling, File I/O, and Threading subsystems are initialized 
	// by default, even if flags == 0
	// http://wiki.libsdl.org/CategoryInit
	void cSDLApplication::Init()
	{
		//by default the 
		SDL_Init(mInitFlags);
	}

	cSDLApplication::~cSDLApplication()
	{
		if (mContext){
			mContext.reset();
		}
		if (mWindow){
			SDL_DestroyWindow(mWindow);
		}
		SDL_Quit();
	}

	void cSDLApplication::MakeWindow(cStringRef window_title, int width, int height)
	{
		akj::Log::Info("Creating Window");
		// initialize the video subsystem if needed
		if ((mInitFlags&SDL_INIT_VIDEO) == 0)
		{
			mInitFlags |= SDL_INIT_VIDEO;
			SDL_Init(SDL_INIT_VIDEO);
		}
		SDL_Rect rect = {0};
		int ret = SDL_GetDisplayBounds(0, &rect);
		if (ret)
		{
			std::string error_string = SDL_GetError();
			Log::Error("Couldn't get the display bounds %s", error_string.c_str());
			throw std::runtime_error(error_string.c_str());
		}

		width = LesserOf(rect.w - 60, width);
		height = LesserOf(rect.h - 60, height);
		mWindow = SDL_CreateWindow(window_title.data(), 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		//

		if (!mWindow)
		{
			std::string error_string = SDL_GetError();
			Log::Error("window creation failed with error %s", error_string.c_str());
			throw std::runtime_error(error_string.c_str());
		}
		mMainWindowID = SDL_GetWindowID(mWindow);
	}

	void cSDLApplication::HandleEvents()
	{
		SDL_Event event = { 0 };
		while (SDL_PollEvent(&event))
		{
		}
	}

	void cSDLApplication::CreateOpenGLContext()
	{
		akj::Log::Info("Creating OpenGL Context");
		if (!mWindow)
		{
			Log::Error("Cannot create an OpenGL context without creating a window first");
			throw std::runtime_error("Cannot create an OpenGL context without creating a window first");
		}

		mContext.reset(new cHWGraphicsContext(mWindow));
	}

	void cSDLApplication::SetWindowIcon(cBitmap<4> bitmap)
	{
		if(bitmap.Height() == bitmap.Width())
		{
			SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(bitmap.Data(),
				bitmap.Width(), bitmap.Height(),
				8*bitmap.BytesPerComponent()*bitmap.NumComponents(), 
				static_cast<uint32_t>(bitmap.Stride()),
				0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
			SDL_SetWindowIcon(mWindow, surface);
			SDL_FreeSurface(surface);
		}
		else
		{
			Log::Error("Cannot set window icon: expected a square icon,"
				" but got %dx%d", bitmap.Width(), bitmap.Height());
		}
		
	}
	
	void cSDLApplication::SetVsync( eSwapType swap_type )
	{
		if(swap_type == VSYNC_ENABLED)
		{
			//swap_type = VSYNC_ADAPTIVE;
		}
		int result = SDL_GL_SetSwapInterval(swap_type);
		if (swap_type == VSYNC_ADAPTIVE && result == -1)
		{
			Log::Debug("No late swap tearing: asking for plain vsync");
			SDL_GL_SetSwapInterval(1);
		}
	}

	void cSDLApplication::EnableVSync()
	{
		if(mVSyncState != VSYNC_ENABLED)
		{
			SetVsync(VSYNC_ENABLED);
			mVSyncState = VSYNC_ENABLED;
		}
	}

	void cSDLApplication::DisableVSync()
	{
		if (mVSyncState != VSYNC_DISABLED)
		{
			SetVsync(VSYNC_DISABLED);
			mVSyncState = VSYNC_DISABLED;
		}
	}

	void cSDLApplication::ToggleVSync()
	{
		if (mVSyncState != VSYNC_DISABLED)
		{
			SetVsync(VSYNC_DISABLED);
			mVSyncState = VSYNC_DISABLED;
		}
		else
		{
			SetVsync(VSYNC_ENABLED);
			mVSyncState = VSYNC_ENABLED;
		}
	}

	ivec2 cSDLApplication::WindowSize() const
	{
		ivec2 ret;
		SDL_GetWindowSize(mWindow, &ret.width, &ret.height);
		return ret;
	}

}
