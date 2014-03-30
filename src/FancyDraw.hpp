#pragma once
#include "akjSDLApplication.hpp"
#include <vector>
#include "StringRef.hpp"
#include "akjMovingVertex2D.hpp"
#include "akjStopWatch.hpp"
#include "akjFrameState.hpp"
#include "akjAlignedBuffer.hpp"
#include "akj2DGraphicsPrimitives.hpp"
#include "testbed.hpp"
#include "akjMousePointer.hpp"
#include "akjInput.hpp"
#include "akjLayeredDrawable.hpp"

union SDL_Event;


namespace akj
{
	class cBackgroundSurface;
	class c2DPrimitiveFactory;
	class cGraphicsTester;
	class cFontLoader;
	class cMousePointer;
	class cScreenTextFactory;
	class cWorkerPool;
	class cScreenText;
	class cAppObjectFactory;
	class cApplicationObject;
	class cAppAction;

	class cFancyDrawApp: public cSDLApplication
	{
	public:
		cFancyDrawApp(int width, int height);

		virtual ~cFancyDrawApp();
		virtual void Run();
		
		void RunTest();
		void RectangleBenchMark();
	protected:
		virtual void HandleEvents();
	private:
		enum eTaskState
		{
			DOING_NOTHING=0,
			RANDOM_TESTING=1,
			TILE_RECTS=RANDOM_TESTING << 1,
			PIXEL_TESTS=TILE_RECTS << 1,
		};

		void HandleKeyDown(SDL_Event& event);

		void ToggleSlowness();

		void HandleFileDrop(cStringRef str);
		void HandleMouseButton(SDL_Event& event);
		void TestPopupMenu(int x, int y);
		void DoDistanceTransform(cStringRef filename) const;
		void DrawStrings();

		void OnWindowResize(int x, int y);
		

		void ShowFrame();

		void RandomTesting();

		void CreateRandomRect();

		void DoTask(eTaskState task);
		void ClearTask(eTaskState task);
		void ToggleTask(eTaskState task);
		void TileRects();
		void FontTester(cStringRef filename);
		void DoFrame();
		void DrawAll();
		void DoTimedTasks();
		void BenchMark();
		void InitAppActions();
		//these should be abstracted at some point
		double mCurrentTime;
		double mLastTimedTask;
		cStopWatch mElapsedTimer;
		cFrameState mFrameTimer;

		// a pre-seeded Random generator
		cRandom mRNG;
		
		std::unique_ptr<cAppObjectFactory> mObjectFactory;
		std::unique_ptr<cWorkerPool> mWorkerPool;
	

		std::unique_ptr<c2DPrimitiveFactory> mPrimitiveFactory;
		std::unique_ptr<cGraphicsTester> mGraphicsTester;
		std::unique_ptr<cFontLoader> mFontLoader;
		std::unique_ptr<cScreenTextFactory> mScreenTextFactory;
		std::unique_ptr<cTexturedQuadFactory> mQuadFactory;
		std::unique_ptr<cMousePointer> mMousePointer;
		
		std::unique_ptr< std::vector<cScreenText> > mTestText;

		std::unordered_map<cInputID, cAppAction*> mInputActions;

		std::vector<cSubmittedLayer> mFrameLayers;

		std::vector<const cAppAction*> mLifetimeObjects;
		std::vector<cRoundedRectangle> mRectangles;
		std::vector<cRoundedRectangle> mRandomRects;
		std::vector<cRoundedRectangle> mOtherRects;
		cRoundedRectangle mMyRectangle;


		int mWidth;
		int mHeight;
		eTaskState mTaskState;
		cMouseState mMouse;

		// maybe the app doesn't have focus, or we're on a laptop
		bool mShouldGoSlow;
		bool mHasVisibleChange;
		uint32_t mMissedSwaps;
		double mFrameAllowance;
		cCoord2 mSampleCenter;
	};
	
}
