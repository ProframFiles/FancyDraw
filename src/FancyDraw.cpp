//needed to get vs2013 to properly encode this file as utf8
//otherwise it will mangle string literals (even with \u escapes)
#ifdef _WIN32
#pragma execution_character_set("utf-8")
#endif

#include "FancyDraw.hpp"
//#include "akjSDLUtils.hpp"
#include "SDL.h"
#include "akjSDLUtils.hpp"
#include "akjLog.hpp"
#include "Bitmap.hpp"
#include "akjAlignedBuffer.hpp"
#include "FatalError.hpp"
#include "akjStopWatch.hpp"
#include "akjAnimation.hpp"
#include "akjIOUtil.hpp"
#include "resources/akjStaticResources.hpp"

#include "FileChooserDialog.hpp"

#include "akjHWGraphicsContext.hpp"

#include "akjObjreader.hpp"

#include "akjBackgroundSurface.hpp"

#include "akj2DPrimitiveFactory.hpp"
#include "akjGraphicsTester.hpp"

#include "MemoryBuffer.hpp"
#include "FileOutputBuffer.hpp"
#include "OwningPtr.hpp"
#include "Compression.hpp"
#include "Format.hpp"
#include "RawOstream.hpp"
#include "akjPixelTree.hpp"

//#include "akjHilbertTransform.hpp"
#include "akjDistanceTransform.hpp"

#include "stb_image_write.h"
#include "stb_image.h"
#include "akjIVec.hpp"
#include "akjFontLoader.hpp"
#include "akjFreeTypeFace.hpp"
#include "BinPacker.hpp"
#include "akjDistanceFieldFont.hpp"
#include "akjScreenTextFactory.hpp"
#include "akjAppAction.hpp"
#include "akjBitmapOperations.hpp"
#include "akjWorkerPool.hpp"
#include "akjApplicationObject.hpp"
#include "akjAppObjectFactory.hpp"
#include "akjInput.hpp"
#include "akjProgressBar.hpp"
#include "akjSaveImageTask.hpp"
#include "akjPhantomPointsSpline.hpp"
#include "akjTexturedQuadFactory.hpp"

#include "akjBinarySearchSequence.hpp"

namespace akj
{
	cFancyDrawApp::cFancyDrawApp(int width, int height)
		:cSDLApplication("☆FancyDraw☆", width, height)
		,mFrameTimer()
		,mCurrentTime(0.0)
		,mLastTimedTask(0.0)
		,mWidth(0)
		,mHeight(0)
		,mTaskState(DOING_NOTHING)
		,mShouldGoSlow(false)
		,mMissedSwaps(0)
		,mFrameAllowance(1.0)
		,mSampleCenter(0.0f, 0.0f)
		,mHasVisibleChange(true)
	{
		AKJ_ASSERT_AND_THROW(mContext);
		mRectangles.reserve(512);
		ivec2 size = WindowSize();
		mContext->SetViewPort(0, 0, size.x, size.y);
		mWidth = size.width;
		mHeight = size.height;

		//////////////////////////////////////////////////////////////////////////
		// Allocate things...
		//////////////////////////////////////////////////////////////////////////
		try{
		mObjectFactory.reset(new cAppObjectFactory);
		AKJ_ASSERT_AND_THROW(mObjectFactory);
		mWorkerPool.reset(new cWorkerPool);
		AKJ_ASSERT_AND_THROW(mWorkerPool);

		mPrimitiveFactory.reset(new c2DPrimitiveFactory(mContext.get()));
		AKJ_ASSERT_AND_THROW(mPrimitiveFactory);
		mGraphicsTester.reset(
			new cGraphicsTester(mContext.get(), mPrimitiveFactory.get()));
		AKJ_ASSERT_AND_THROW(mGraphicsTester);
		mFontLoader.reset(new cFontLoader(*mWorkerPool));
		AKJ_ASSERT_AND_THROW(mFontLoader);
		mScreenTextFactory.reset(new cScreenTextFactory(*mContext, *mFontLoader));
		AKJ_ASSERT_AND_THROW(mScreenTextFactory);
		mTestText.reset(new std::vector<cScreenText>);
		AKJ_ASSERT_AND_THROW(mTestText);
		mQuadFactory.reset(new cTexturedQuadFactory(*mContext));
		AKJ_ASSERT_AND_THROW(mQuadFactory);
		mMousePointer.reset(new cMousePointer(mContext.get()));
		AKJ_ASSERT_AND_THROW(mMousePointer);

		OwningPtr<MemoryBuffer> mapped_file;
		MemoryBuffer::getFile("angry_computer_guy.png", mapped_file).assertOK();
		cAlignedBuffer read_file;
		auto in_bm = cBitmapReader::ReadPNGFromMem(mapped_file->getBuffer(), read_file);

		mQuadFactory->LoadTexture(in_bm, "Angry Computer Guy");
		//CreateRandomRect();

		for(int c = 0; c <1; ++c)
		{
			cCoord2 start((c/29.0f) * 100.0f);
			float sum = 10.0f;
			for (uint32_t i = 9; i <34 ; ++i)
			{
				eBuiltinFonts f = static_cast<eBuiltinFonts>(i%NUM_BUILTIN_FONTS);
				mTestText->emplace_back(mScreenTextFactory->CreateText(
					"Conversely, the more magically resistant something is,"
					" the more fragile it is. - "
					//"grumpy "
					//"wizards make Toxic brew for the evil Queen and Jack - " 
					+ Twine(i), start+cCoord2(.5f, std::ceil(sum)+0.5f), i, f));
				mTestText->back().ChangeTime(std::sin((i-6.0)/(32.0-6.0)));
				sum += i*1.4f;
			}
		}
		mTestText->emplace_back(
			mScreenTextFactory->CreateText("ATg", {370.0f, 440.0f}, 450));


		// now set the icon	
		cStringRef icon = cStaticResources::get_icon_png();
		
		cAlignedBuffer buf;
		cAlignedBitmap icon_bm = cBitmapReader::ReadPNGFromMem(icon,buf);
		if (icon_bm.IsValid())
		{
			SetWindowIcon(icon_bm);
		}
		else
		{
			Log::Error("Problems setting the window icon");
		}

		RGBAu8 selection_fill = cWebColor::DODGERBLUE;
		selection_fill.a = 74;
		RGBAu8 selection_edge = cWebColor::LIGHTSEAGREEN;
		selection_edge.a = 190;

		
		DisableVSync();
		
		mContext->ClearColor(cWebColor::WHITE);

		cProgressBarCreator* bar_creator = 
			mObjectFactory->CreateAppObject<cProgressBarCreator>(
			*mObjectFactory, *mScreenTextFactory, *mPrimitiveFactory);

		// Set all the task handlers
		// the object factory is the base listener, and it dispatches the events
		// to all the subscribing objects
		mWorkerPool->SetCreationHandler(
		[this](tTaskHandle handle, cStringRef name, cStringRef status)
		{
			if(mObjectFactory)
			{
				mObjectFactory->OnTaskCreated(handle, name, status);
			}
		});

		mWorkerPool->SetProgressHandler(
		[this](tTaskHandle handle, cStringRef name, float progress)
		{
			if(mObjectFactory)
			{
				mObjectFactory->OnTaskUpdate(handle, name, progress);
			}
		});

		mWorkerPool->SetDestructionHandler(
		[this](tTaskHandle handle)
		{
			if(mObjectFactory)
			{
				mObjectFactory->OnTaskDestroyed(handle);
			}
		});

		//CreateRandomRect();
		mMyRectangle = std::move(mPrimitiveFactory
			->CreateRoundedRect(0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 2.0f,
			selection_edge, selection_fill).Hide());

		PhantomPointsSpline<cCoord2> spline({{1.0f, 2.0f},{120.0f, 135.0f}});
		cArray<16, float> t = 
			{0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f};
		cArray<16, cCoord2> CP;
		cCoord2 last = {1.0f, 2.0f};
		spline.eval(0, t, CP);
		for(const auto& cp: CP)
		{
			Log::Debug("CP: (%f, %f, %f)", cp.x, cp.y, (cp-last).Length());
			last = cp;
		}	

		}
		catch(const Exception& e)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
																"Application Failed to Start",
																e.what(), mWindow);
			Log::Error("Application Failed to start: \n\t%s", e.what());
			AKJ_THROW("rethrown init failure.");
		}
		mFrameTimer.TargetFrameRate(120);
		InitAppActions();
		AKJ_ASSERT(cMinimizingSearchSeq::Test(3));
		AKJ_ASSERT(cTexturedQuadFactory::Test(*mQuadFactory));
	}

	cFancyDrawApp::~cFancyDrawApp()
	{
		//ensure that the factory outlives the shapes
		akj::Log::Info("FancyDraw App destructor. Bye!");

		// this one has to be shut down ahead of time...
		mWorkerPool.reset();

		// App Objects can hold ref-counted resources: make sure they die before
		// any of their parents do
		mObjectFactory.reset();

		mRectangles.clear();
		mOtherRects.clear();
		mRandomRects.clear();
	}


	void cFancyDrawApp::Run()
	{
		try{
			mElapsedTimer.Start();
			while (mDesiredState == cSDLApplication::STATUS_RUN)
			{
				DoFrame();
			}
			double dt = mElapsedTimer.Read();
			Log::Info("Finished application event loop, exiting Run()");
			Log::Info("Rendered %d frames in %f seconds (%f FPS, %fms frame time)",
				mFrameTimer.FrameCount(), dt, mFrameTimer.FrameCount() / dt,
				1000.0*(dt / mFrameTimer.FrameCount()));
		}
		catch(const Exception& e)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
																"Application Died (via akj::Exception) :(",
																e.what(), mWindow);
			Log::Error("Application Died (via akj::Exception): \n\t%s", e.what());
		}
	}

	void cFancyDrawApp::OnWindowResize(int x, int y)
	{
		mContext->SetViewPort(0, 0, x, y);
	}

	void cFancyDrawApp::DrawAll()
	{
		
		SDL_GetWindowSize(mWindow, &mWidth, &mHeight);

		mContext->ResetDepthRange();
		mContext->Clear();

		// keep primitives ahead of text for the sort!
		std::initializer_list<iLayeredDrawable*> list = {
			mPrimitiveFactory.get(),
			mScreenTextFactory.get()
		};
		LayeredDraw(list, mFrameLayers);

		mFrameTimer.MarkDraw();
		//mMousePointer->Draw(mMouse.pos, mCurrentTime);
		
		mHasVisibleChange = false;
	}

	void cFancyDrawApp::ShowFrame()
	{
		mFrameTimer.MarkSwap();
		mContext->Swap();
		mFrameTimer.MarkFrameStart();
	}


	void cFancyDrawApp::FontTester(cStringRef str)
	{
		auto face_handle = mFontLoader->LoadFont(str.data());
		FreeTypeFace& face = mFontLoader->TypeFace(face_handle);

		mFontLoader->CreateDistanceFieldFontAsync(
		face, 1024, std::function<void(tDFFHandle)>(
			[this](tDFFHandle dff_handle){
				cDistanceFieldFont& dff =  mFontLoader->DistanceFont(dff_handle);
				mFontLoader->ExportDFFToFile(dff_handle);
				auto face = mFontLoader->FaceHandleFrom(dff_handle);
				mScreenTextFactory->SetFont(face, UI_FONT);
			}));

		Log::Info("Tested serialization-deserialization loop: All OK");
	}


void cFancyDrawApp::DoDistanceTransform( cStringRef filename ) const
{

		int x = 0;
		int y = 0;
		int comp = 0;

		OwningPtr<MemoryBuffer> in_buffer;
		auto error = MemoryBuffer::getFile(filename, in_buffer);
		if (error) FatalError::Die(error.message());
		cStringRef buf = in_buffer->getBuffer();

		uint8_t* image = 
			stbi_load_from_memory(reinterpret_cast<const uint8_t*>(buf.data()),
										static_cast<uint32_t>(buf.size()), &x, &y, &comp, 0);

		cAlignedBitmap bitmap(image, x, y, comp, BIT_DEPTH_8);
		cAlignedBitmap bitmap_1ch(x, y, 1, BIT_DEPTH_8);

		AlignedBuffer<16> buffer_1ch(bitmap_1ch.Size());
		
		bitmap_1ch.SetData(buffer_1ch.data());
				
		for (int i = 0; i < comp ; ++i)
		{
			cStopWatch sw;
			// first extract a channel
			if(i < 3)
			{
				bitmap_1ch.ForEachPixel<uchar>([i](uchar* here, uchar* there){
					*here = there[i];
				}, bitmap);
			}
			else
			{
				bitmap_1ch.ForEachPixel<uchar>([i](uchar* here, uchar* there){
					*here = (255-there[i]);
				}, bitmap);
			}

			EuclideanDistanceTransform edt(bitmap_1ch);
			const float scale = 43.0f;

			cAlignedBitmap edt_result = edt.RunTransform(scale);
			

			Log::Debug("did distance transform in %f seconds", sw.Read());


			//convert edt result from float to uchar in converted_edt

			if(i < 3)
			{
				edt_result.ForEachPixel<uchar>([i](uchar* src, uchar* dest){
					const float source = *reinterpret_cast<float*>(src);
					dest[i] = static_cast<unsigned char>((source*255.5f));
				}, bitmap);
			}

			else
			{
				edt_result.ForEachPixel<uchar>([i](uchar* src, uchar* dest){
					const float source = *reinterpret_cast<float*>(src);
					dest[i] = 255 - static_cast<unsigned char>((source*255.5f));
				}, bitmap);
			}
			
			sw.Stop();
		}

		size_t ext_index = filename.find_last_of('.');
		std::string out_name = filename.substr(0, ext_index);
		raw_string_ostream(out_name) << "_edt.png";
		stbi_write_png(out_name.c_str(), x, y, comp, bitmap.Data(), 
			static_cast<uint32_t>(bitmap.Stride()));
		stbi_image_free(image);
	}

	void cFancyDrawApp::TestPopupMenu(int x, int y)
	{
		//SDL_EventState(SDL_WINDOWEVENT, SDL_IGNORE);
		int window_x, window_y;
		SDL_GetWindowPosition(mWindow, &window_x, &window_y);
		//SDL_Window* window;
		Log::Info("Popup window is go!");
		{
			//window = SDL_CreateWindow("crap", window_x + x, window_y + y, 120, 300, SDL_WINDOW_BORDERLESS|SDL_WINDOW_HIDDEN);
			//cPopUpWindow popup(x+window_x, y+window_y, 120, 300, mWindow);

			// Select the color for drawing. It is set to red here.
		//	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

			// Clear the entire screen to our selected color.
			//SDL_RenderClear(renderer);
			//SDL_RenderPresent(renderer);
		//	SDL_ShowWindow(window);
			//popup.SelectItem(this);
		}
		//SDL_EventState(SDL_WINDOWEVENT, SDL_ENABLE);
	}

	void TestFileChooser()
	{
		cFileChooserDialog chooser("Choose a File!", "E:\\Videos\\");
		auto files = chooser.ShowAndBlock();
		Log::Info("Selected %d paths in FIle Chooser", files->mPathIndices.size());
		for (size_t path = 0; path < files->mPathIndices.size() ; ++path)
		{
			const size_t index = files->mPathIndices.at(path);
			Log::Info("path %d: %s", path, &files->mRawPaths.at(index));
		}
	}

	std::string CopyAndFreeString(char* str)
	{
		if (!str) return std::string();
		std::string copied = str;
		SDL_free(reinterpret_cast<void*>(str));
		return copied;
	}


	void cFancyDrawApp::HandleFileDrop(cStringRef str)
	{
		std::string upper_str = str.upper();

		cStringRef filename = sys::path::filename(str);
		cStringRef stem = sys::path::stem(filename);
		cStringRef ext = sys::path::extension(filename);
		std::string lower_ext = ext.lower();
		ext = lower_ext;

		mContext->Clear();
		mContext->Swap();
		if (str.empty()){
			Log::Info("Drag and drop yielded a NULL string");
			mContext->Clear(cWebColor::RED);
			return;
		}
		if(ext == ".ttf" || ext == ".otf" || ext == ".woff")
		{
			FontTester(str);
		}
		else if(ext == ".png"
			|| ext == ".jpg" 
			|| ext == ".tga")
		{
			Log::Info("Got dropped image file: %s", str.data());
			DoDistanceTransform(str);
		}
		else if(ext == ".bmp")
		{
			OwningPtr<MemoryBuffer> mapped_file;
			auto result = MemoryBuffer::getFile(str, mapped_file);
			if(result || NULL == mapped_file.get() )
			{
				AKJ_THROW("Unable to open file: \"" + str +"\".\n" + result.message());
			}
			cAlignedBuffer bmp_storage;
			auto bmp = cBitmapReader::ReadBMPFromMem(mapped_file->getBuffer(), bmp_storage);
			if(bmp.Comp()==4 )
			{
				bool opaque = true;
				ForEachPixel([&opaque](pix::RGBA8 px){
					if(px.a()<255)
					{
						opaque = false;
					}
				},
					bmp.Pixels<pix::RGBA8>());
				if(opaque) bmp.ExportJPEG(str+".jpg");
				else bmp.ExportPNG(str+".png");
			}
			else bmp.ExportJPEG(str+".jpg");
			
		}
		else if(ext == ".dff")
		{
			OwningPtr<MemoryBuffer> mapped_file;
			auto result = MemoryBuffer::getFile(str, mapped_file);
			if(result || NULL == mapped_file.get() )
			{
				AKJ_THROW("Unable to open file: \"" + str +"\".\n" + result.message());
			}
			auto handle = mFontLoader->LoadDFFFromMemory(mapped_file->getBuffer());
			if(!handle.IsValid())
			{
				AKJ_THROW("Bad font file: invalid handle from \"" + str + "\".");
			}
			auto face = mFontLoader->FaceHandleFrom(handle);
			mScreenTextFactory->SetFont(face, UI_FONT);
		}
		else
		{
			mContext->Clear(cWebColor::RED);
				return;
		}
	}

	void TestRNGFloat()
	{
		cRandom rng;
		for (size_t i = 0; i < 134217728; ++i)
		{
			const float r = rng.Float();
			AKJ_ASSERT(r > 0.0f && r < 1.0f);
		}
	}

	void cFancyDrawApp::HandleMouseButton(SDL_Event& event)
	{
		mMouse.pos.x = static_cast<float>(event.button.x);
		mMouse.pos.y = static_cast<float>(event.button.y);
		if (event.button.type == SDL_MOUSEBUTTONDOWN)
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				mMouse.left = event.button.state;
				mMyRectangle.TopLeft(mMouse.pos).BottomRight(mMouse.pos).Show();
				break;
			case SDL_BUTTON_MIDDLE:
				mMouse.middle = event.button.state;
				break;
			case SDL_BUTTON_RIGHT:
				mMouse.right = event.button.state;
				break;
			case SDL_BUTTON_X1:
				mMouse.extra_a = event.button.state;
				break;
			case SDL_BUTTON_X2:
				mMouse.extra_b = event.button.state;
			default:
				break;
			}
		}
		else //button up
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				mMouse.left = event.button.state;
				mMyRectangle.Hide();
				break;
			case SDL_BUTTON_MIDDLE:
				mMouse.middle = event.button.state;
				break;
			case SDL_BUTTON_RIGHT:
				TestPopupMenu(static_cast<int32_t>(mMouse.pos.x), 
					static_cast<int32_t>(mMouse.pos.y));
				mMouse.right = event.button.state;
				break;
			case SDL_BUTTON_X1:
				mMouse.extra_a = event.button.state;
				break;
			case SDL_BUTTON_X2:
				mMouse.extra_b = event.button.state;
			default:
				break;
			}
		}
	}

	void ExportAlphaBuf(cHWGraphicsContext& context)
	{
		cAlignedBuffer buf;
		auto out = context.SaveAlphaBuffer(buf);
		cAlignedBuffer expand_buf;
		cBitmapExpander<pix::A8> expander(expand_buf);
		uint32_t height = 221;
		expander.Expanded( out.SubRect<1>(0, out.H()-height, 400, height),5)
						.ExportPNG("mag_alpha.png");
		out.SubRect<1>(20, out.H()-height, 700, height)
						.ExportPNG("normal_size.png");
	}

	void cFancyDrawApp::InitAppActions()
	{
		cAppAction* quit_action 
			= mObjectFactory->CreateAppObject<cAppAction>("Quit", 
			[](){
				SDL_Event quit_event;
				quit_event.type = SDL_QUIT;
				// this had better make a copy...
				SDL_PushEvent(&quit_event);
			});
		mInputActions.emplace(FromSDLK(SDLK_ESCAPE), quit_action);

		cAppAction* slowness 
			= mObjectFactory->CreateAppObject<cAppAction>("Toggle Slowness", 
			[this](){ ToggleSlowness(); });
		mInputActions.emplace(FromSDLK(SDLK_ESCAPE), slowness);

		mInputActions.emplace(FromSDLK(SDLK_i), 
			mObjectFactory->CreateAppObject<cAppAction>("Text edge out", 
			[this](){ mScreenTextFactory->ChangeEdgeShift(0.01f); }));

		mInputActions.emplace(FromSDLK(SDLK_k), 
			mObjectFactory->CreateAppObject<cAppAction>("Text edge in", 
			[this](){ mScreenTextFactory->ChangeEdgeShift(-0.01f); }));

		mInputActions.emplace(FromSDLK(SDLK_j), 
			mObjectFactory->CreateAppObject<cAppAction>("Text edge widen", 
			[this](){ mScreenTextFactory->ChangeEdgeWidth(0.01f); }));

		mInputActions.emplace(FromSDLK(SDLK_l), 
			mObjectFactory->CreateAppObject<cAppAction>("Text edge thin", 
			[this](){ mScreenTextFactory->ChangeEdgeWidth(-0.01f); }));

		mInputActions.emplace(FromSDLK( SDLK_b ), 
			mObjectFactory->CreateAppObject<cAppAction>("Quick benchmark", 
			[this](){ BenchMark(); }));

		mInputActions.emplace(FromSDLK( SDLK_a ), 
			mObjectFactory->CreateAppObject<cAppAction>("Export back buffer", 
			[this](){
				mContext->SaveBackBuffer(*mWorkerPool,"BackBuffer.png");
			}));

		mInputActions.emplace(FromSDLK( SDLK_t ), 
			mObjectFactory->CreateAppObject<cAppAction>("Tile Rect test", 
			[this](){ 			
				mRectangles.clear();
				mRandomRects.clear();
				ToggleTask(TILE_RECTS); 
			}));

		mInputActions.emplace(FromSDLK( SDLK_f ), 
			mObjectFactory->CreateAppObject<cAppAction>("Go Fullscreen", 
			[this](){ 
				SDL_SetWindowFullscreen(mWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
				SDL_SetWindowFullscreen(mWindow, SDL_WINDOW_FULLSCREEN); 
			}));

		mInputActions.emplace(FromSDLK( SDLK_r ), 
			mObjectFactory->CreateAppObject<cAppAction>("Change text Render method", 
			[this](){ 
				mScreenTextFactory->ToggleRenderMethod();
			}));

		mInputActions.emplace(FromSDLK( SDLK_w ), 
			mObjectFactory->CreateAppObject<cAppAction>("Toggle wireframe mode", 
			[this](){ mPrimitiveFactory->ToggleWireframeMode(); }));		
	}


	void cFancyDrawApp::HandleKeyDown(SDL_Event& event)
	{
		cInputID input = FromSDLK(event.key.keysym.sym);

		auto found = mInputActions.find(input);
		if(found == mInputActions.end())
		{
		 return;
		}
		found->second->Execute();
	}

	void cFancyDrawApp::DoFrame()
	{
		mCurrentTime = mElapsedTimer.Read();
		double time_remaining;
		DoTimedTasks();
		DrawAll();
		do
		{
			HandleEvents();
			time_remaining = mFrameTimer.FrameTimeRemaining();
			if(time_remaining > 10.0)
			{
				SDL_Delay(10);
				mCurrentTime = mElapsedTimer.Read();
				HandleEvents();
				time_remaining = mFrameTimer.FrameTimeRemaining();
			}
			// quit when we have less than 1ms left, or when we have less than
			// 4 ms left and still need to draw

		}while( (!mHasVisibleChange && time_remaining > 1.0) 
						|| time_remaining > 4.0 );
		
		if(mHasVisibleChange)
		{
			mPrimitiveFactory->SetFrameTime(mCurrentTime);
			mScreenTextFactory->SetFrameTime(mCurrentTime);
			DrawAll();
		}
		// if we are throttled, wait a bit
		if (mShouldGoSlow)
		{
			//SDL_Delay(15);
		}
		ShowFrame();
	}

	void cFancyDrawApp::DoTimedTasks()
	{
		double elapsed = mCurrentTime -mLastTimedTask;
		mLastTimedTask = mCurrentTime;
		mObjectFactory->OnTimeElapsed(elapsed, mCurrentTime);
		mPrimitiveFactory->SetFrameTime(mCurrentTime);
		mScreenTextFactory->SetFrameTime(mCurrentTime);
		mObjectFactory->PruneDeadObjects();
		mWorkerPool->UpdateTasks();
		if (mTaskState & TILE_RECTS){ TileRects(); }
		if (mTaskState & PIXEL_TESTS){
			mGraphicsTester
				->PixelBorderTest(static_cast<int32_t>(mMouse.pos.x),
				static_cast<int32_t>(mMouse.pos.y), mCurrentTime);
		}
		if (mTaskState & RANDOM_TESTING){ RandomTesting(); }
	}


	void cFancyDrawApp::HandleEvents()
	{
		SDL_Event event = { 0 };
		while (mDesiredState == cSDLApplication::STATUS_RUN 
					&& SDL_PollEvent(&event))
		{
			mCurrentTime = mElapsedTimer.Read();
			//Log::Debug("event timestamp! %d", event.common.timestamp);

			switch (event.type)
			{
			case SDL_MOUSEMOTION:
				mMouse.pos.x = static_cast<float>(event.motion.x);
				mMouse.pos.y = static_cast<float>(event.motion.y);
				if(mMyRectangle.IsVisible())
				{
					mMyRectangle.BottomRight(mMouse.pos);
				}
				mHasVisibleChange = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				// as opposed to a popup menu or file dialog
				if (event.button.windowID == mMainWindowID)
				{
					Log::Debug("Got mouse event that is mine");
					HandleMouseButton(event);
				}
				else
				{
					Log::Debug("Got mouse event that isn't mine");
				}
				mHasVisibleChange = true;
				break;
			case SDL_DROPFILE:
				HandleFileDrop(CopyAndFreeString(event.drop.file));
				mHasVisibleChange = true;
				break;
			case SDL_QUIT:
				Log::Debug("received \"Quit\" event");
				mDesiredState = cSDLApplication::STATUS_QUIT;
				break;
			case SDL_KEYDOWN:
				HandleKeyDown(event);
				mHasVisibleChange = true;
				break;
			case SDL_TEXTINPUT:
				Log::Info("text input: %s", &event.text.text);
				break;
			case SDL_WINDOWEVENT:
				if (event.window.windowID == mMainWindowID)  {
					//Log::Debug("My window event: %s", SDL::WindowEventTypeToString(event.window.event));
					switch (event.window.event)  {
					case SDL_WINDOWEVENT_ENTER:
					//	SDL_ShowCursor(SDL_DISABLE);
						mMousePointer->Show();
						mShouldGoSlow = false;
						mFrameTimer.TargetFrameRate(60);
						break;
					case SDL_WINDOWEVENT_LEAVE:
					//	SDL_ShowCursor(SDL_ENABLE);
						mMousePointer->Hide();
						//mFrameTimer.TargetFrameRate(15);
						//mShouldGoSlow = true;
						break;
					case SDL_WINDOWEVENT_CLOSE:
						Log::Debug("got window close event, forwarding \"Quit\" request");
						event.type = SDL_QUIT;
						SDL_PushEvent(&event);
						break;
					case SDL_WINDOWEVENT_RESIZED:
						OnWindowResize(event.window.data1, event.window.data2);
						break;

					case SDL_WINDOWEVENT_EXPOSED:
						//DrawFrame();
						break;
					default:
						
						break;
					}
				}
				else
				{
					Log::Debug("Other window event: %s", SDL::WindowEventTypeToString(event.window.event));
				}
				break;

			default:
				Log::Debug("Event: %s, timestamp: %d", SDL::EventTypeToString(event.type), event.common.timestamp);
				break;
			}
		}
		
		
	}

	void cFancyDrawApp::BenchMark()
	{
		for (size_t i = 0; i < 20 ; ++i)
		{
			RandomTesting();
		}
		DisableVSync();
		cStopWatch timer;
		double last_checkin = mCurrentTime;
		uint32_t max_frames = 5000;
		for (uint32_t i = 0; i < max_frames ; ++i)
		{
			const float fraction = (static_cast<float>(i)/max_frames);
			mCurrentTime = mElapsedTimer.Read();
			DoTimedTasks();
			auto rect = mPrimitiveFactory->
				CreateRoundedRect(20.0f, fraction*(mWidth-20.0f), 20.0f, 45.0f,
				2.0f, 2.0f, cWebColor::DARKCYAN, cWebColor::DARKORANGE);
			auto text = mScreenTextFactory->CreateText(
				Twine(static_cast<uint32_t>(fraction*100))+"%", {25.0f, 40.0f},18, MONOSPACE_FONT);
			rect.AddText(cMutableArrayRef<cScreenText>(&text, 1));
			DrawAll();
			ShowFrame();
			if(mCurrentTime - last_checkin > 0.5)
			{
				HandleEvents();
				last_checkin = mCurrentTime;
			}
		}
		timer.Stop();
		mRandomRects.clear();
		Log::Info("Completed 5000 frames in %fs (%ffps, %fms frame time)",
							timer.Read(), 5000/timer.Read(), 1000.0*timer.Read()/5000.0);
		cScreenText done_text = mScreenTextFactory->CreateText(
			"Benchmark: " + Twine(5000/timer.Read())+ "fps",
			{15.0f, static_cast<float>(mHeight)},35, MONOSPACE_FONT);
		iRect bb = done_text.BoundingBox();
		bb.Expand(2);
		int text_top = bb.top;
		auto baseline = mPrimitiveFactory->CreateRoundedRect(
			bb, 1.0f, 0.0f, cWebColor::BLACK, cWebColor::AZURE);
		auto obj = mObjectFactory->CreateAppObject<cAnimatedFuncObject>(1.0, 
		[baseline, text_top, done_text, this](double frac){
			baseline.AddText(cArrayRef<cScreenText>(&done_text, 1));
			const cCoord2& old_pos = done_text.Pos();	
			cCoord2 pos(35.0f, mHeight+200.0f-0.9f*(mHeight+200.0f)*static_cast<float>(frac));
			cCoord2 diff = pos - old_pos;
			baseline.Move(diff);
			done_text.Move(diff); 
		});
		obj->SetEasing(kCubicOut);
		obj->SetFinalPause(4.0);
	}


	void cFancyDrawApp::DrawStrings()
	{
		
	}

	void cFancyDrawApp::RunTest()
	{
		TestDraw();
		mContext->Swap();
		SDL_Delay(2000);
	}

	void cFancyDrawApp::RandomTesting()
	{
		if(mMyRectangle.IsVisible())
		{
			return;
		}
		if(!mRandomRects.empty())
		{
			//mRandomRects.pop_back();
			/*mRandomRects.emplace_back(mPrimitiveFactory->
				CreateRoundedRect(300.0f, 600.0f, 200.0f, 500.0f,
				10, 150.0f, cWebColor::DARKORANGE, cWebColor::LEMONCHIFFON));

				mRandomRects.emplace_back(mPrimitiveFactory->
				CreateRoundedRect(350.0f, 650.0f, 250.0f, 550.0f,
				10, 150.0f, cWebColor::DARKORANGE, cWebColor::LEMONCHIFFON));*/
		}
		if (mOtherRects.empty() && mRandomRects.size() < 150)
		{
			CreateRandomRect();

		}
		else if (!mOtherRects.empty())
		{
			int removed = 0;
			size_t index = mOtherRects.size() - 1;
			while (removed == 0)
			{
				if (mRNG.Float() > 0.2f)
				{
					//Log::Debug("removing rect %d", mOtherRects.at(index).ID() );
					mOtherRects.at(index) = std::move(mOtherRects.back());
					mOtherRects.pop_back();
					removed++;
				}
				if (index-- == 0)
				{
						//Log::Debug("Index reset");
					index = mOtherRects.size() - 1;
				}
			}
		}
		else if(false)
		{
			//Log::Debug("Clearing random rects");
			mOtherRects = mRandomRects;
			mRandomRects.clear();
		}
		mMyRectangle = 
			std::move(mPrimitiveFactory->CreateRoundedRect(mMyRectangle).Hide());
	//	mRandomRects.emplace_back(mPrimitiveFactory->
	//		CreateRoundedRect(100, mWidth - 100.0f, 100.0f, mHeight - 100.0f,
		//	150, 0.0f, cWebColor::DARKGRAY, cWebColor::DODGERBLUE));
		
		//SDL_Delay(300);
	}



	void GenerateRandomRects(std::vector<iRect>& rects, int num, int w, int h)
	{
		cRandom rng;
		rects.clear();
		iRect temp;
		int max_width = (w * 3) >> 1;
		int max_height = (h * 3) >> 1;
		for (int i = 0; i < num ; ++i)
		{
			temp.width = rng.UInt(max_width);
			temp.height = rng.UInt(max_height);
			temp.left = rng.UInt(w - temp.width);
			temp.top = rng.UInt(h - temp.height);
			rects.push_back(temp);
		}
	}

	void cFancyDrawApp::RectangleBenchMark()
	{
		cStopWatch sw;
		cSparseCoverageMap cover(ivec2(mWidth, mHeight));
		Log::Info("starting rectangle coverage benchmark");
		std::vector<iRect> rects;
		rects.reserve(3000);
		bool old_go_slow_setting = mShouldGoSlow;
		mShouldGoSlow = false;
		double time = 0.0f;
		size_t trials = 5000;
		for (size_t i = 0; i < trials ; ++i)
		{
			GenerateRandomRects(rects, 5000, mWidth, mHeight);
			sw.Start();
			for (size_t r = 0; r < rects.size() ; ++r)
			{
				const iRect& rect = rects.at(r);
				if (!cover.IsFullyOccluded(rect))
				{
					// not totally occluded
					cover.AddRect(rect);
				}
			}
			time += sw.Read();
			cover.Clear();
		}
		Log::Info("Rect benchmark completed in %fs (%fms per frame)",
			time, 1000*time/trials);
		mShouldGoSlow = old_go_slow_setting;
	}

	void cFancyDrawApp::TileRects()
	{
		mRectangles.clear();
		if (mRectangles.size() != 20)
		{
			mContext->Clear(RGBAu8(228, 228, 228, 255));
			RGBAu8 mac_border(0x94, 0x94, 0x94, 255);
			RGBAu8 mac_button(251, 251, 251, 255);
			float xrs = (80.0f);
			float yrs = (20.0f);
			float xrc = (200.0f);
			float yrc = (50.0f);
			for (size_t s = 0; s < 10 ; ++s)
			{
				for (size_t r = 0; r < 10 ; ++r)
				{
					float thick = 0.0f + s*0.2f;
					float adj = 0.5f;
					float xorig = xrc + adj;
					float yorig = yrc + adj;
					mRectangles.emplace_back(mPrimitiveFactory->
						CreateRoundedRect(xorig, xorig + xrs, yorig, yorig + yrs,
						thick, static_cast<float>(r), mac_border, mac_button));
						yrc += 30.0f;
				}
				xrc += 100.0f;
				yrc = 50.0f;
			}	
			mRectangles.emplace_back(mPrimitiveFactory->
				CreateRoundedRect(1, mWidth - 1.f, 1.f, 1.f, 0.f, 0.f,
				cWebColor::BLACK, cWebColor::BLACK));
			mRectangles.emplace_back(mPrimitiveFactory->
				CreateRoundedRect(3.f, 4.f, 10.f, mHeight-1.f, 0.f, 0.f,
				cWebColor::BLACK, cWebColor::BLACK));
			mRectangles.emplace_back(mPrimitiveFactory->
				CreateRoundedRect(8.f, 9.f, 10.f, mHeight - 1.f, 0.f, 0.f,
				cWebColor::BLACK, cWebColor::BLACK));
			mRectangles.emplace_back(mPrimitiveFactory->
				CreateRoundedRect(0.f, 1.f*mWidth , 8.f, 8.f, 0.f, 0.f,
				cWebColor::BLACK, cWebColor::BLACK));
		}
		
		
	}

	void cFancyDrawApp::ToggleTask(eTaskState task)
	{
		if(mTaskState & task){
			mTaskState = static_cast<eTaskState>(mTaskState & (~task));
		}
		else
		{
			mTaskState = static_cast<eTaskState>(mTaskState | task);;
		}
	}

	void cFancyDrawApp::DoTask(eTaskState task)
	{
		mTaskState = static_cast<eTaskState>(mTaskState | (task));
	}

	void cFancyDrawApp::ClearTask(eTaskState task)
	{
		mTaskState = static_cast<eTaskState>(mTaskState & (~task));
	}	

	void cFancyDrawApp::ToggleSlowness()
	{
		if (mShouldGoSlow)
		{
			//DisableVSync();
			mShouldGoSlow = false;
			mFrameTimer.TargetFrameRate(5000);
		}
		else
		{
			EnableVSync();
			mShouldGoSlow = true;
			mFrameTimer.TargetFrameRate(15);
		}
	}

	void cFancyDrawApp::CreateRandomRect()
	{
		float xrs = mRNG.Float()*(0.5f*mWidth - 20.0f);
		float yrs = mRNG.Float()*(0.5f*mHeight - 20.0f);
		float xrc = mRNG.Float()*(mWidth - 20 - xrs) + 10.0f;
		float yrc = mRNG.Float()*(mHeight - 20 - yrs) + 10.0f;
		float min_size = std::min(xrs, yrs);
		mRandomRects.emplace_back(mPrimitiveFactory->
			CreateRoundedRect(xrc, xrc + xrs, yrc, yrc + yrs,
			mRNG.Float()*10.0f, mRNG.Float()*min_size*0.5f,
			cWebColor::Random(), cWebColor::Random()));
	}




} // namespace akj