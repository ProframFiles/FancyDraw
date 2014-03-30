#pragma once
#include "akj_typedefs.hpp"
#include "akjApplication.hpp"
#include "StringRef.hpp"
#include "akjIVec.hpp"
#include "akjInput.hpp"
#include "Bitmap.hpp"
#include <string>
#include <memory>

struct SDL_Window;

namespace akj{

	class cHWGraphicsContext;


	class cSDLApplication : public cApplication
	{
	public:
		enum eSwapType
		{
			VSYNC_DISABLED = 0,
			VSYNC_ENABLED = 1,
			VSYNC_ADAPTIVE = -1
		};

		cSDLApplication(cStringRef window_title, int width, int height);
		virtual ~cSDLApplication();
		virtual void Run() = 0;

	protected:
		virtual void HandleEvents();
		void EnableVSync();
		void DisableVSync();
		void ToggleVSync();
		ivec2 WindowSize() const;
		void SetWindowIcon(cBitmap<4> bitmap);
		
		enum eDesiredState
		{
			STATUS_RUN,
			STATUS_QUIT,
		};
		
		eDesiredState mDesiredState;
		SDL_Window* mWindow;
		uint32_t mMainWindowID;
		std::unique_ptr<cHWGraphicsContext> mContext;

	private:
		void SetVsync( eSwapType swap_type );
		void CreateOpenGLContext();
		void MakeWindow(cStringRef title, int width, int height);
		
		uint32_t mInitFlags;
		eSwapType mVSyncState;
		void Init();
		

	};

}