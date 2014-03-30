#include "PopUpWindow.hpp"
#include <string.h>
#include "SDL.h"
#include "akjSDLUtils.hpp"
#include "akjLog.hpp"
#include "FancyDrawMath.hpp"
#include "akjApplication.hpp"
#define FL_INTERNALS 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <FL/Fl.H>
#include "FL/Fl_Draw.H"
#include "FL/Fl_Double_Window.H"
#include "FL/x.H"

namespace akj
{
	namespace fltk
	{
		class cPopUp : public Fl_Double_Window
		{
		public:
			cPopUp(int x_pos, int y_pos, int width, int height, SDL_Window* parent)
			: Fl_Double_Window(x_pos-2, y_pos-2, width, height, "Awesome Popup!")
			, mWidth(width)
			, mHeight(height)
			, mSelectionHeight(20)
			, mCursorHeight(0)
			, mDone(false)
			, mParent((Window)SDL::GetWindowHandle(parent))
			{
				clear_border();
				set_modal();
			}

			virtual int handle(int event)
			{
				if (mDone)
				{
					return Fl_Double_Window::handle(event);
				}
				int parent_result = 0;
				switch (event)
				{
				case FL_FOCUS:
					Log::Debug("Processed focus event in popup");
					return 1;
				case FL_PUSH:
					Log::Debug("Processed button down from within the popup");
					mDone = true;
					return 0;
				case FL_RELEASE:
					Log::Debug("Processed right button up from within the popup");
					return 0;
				case FL_ENTER:
					Log::Debug("Processed popup enter");
					mCursorHeight = Fl::event_y();
					redraw();
					return 1;
				case FL_MOVE:
					mCursorHeight = Fl::event_y();
					redraw();
					return 0;
				default:
					parent_result = Fl_Double_Window::handle(event);
					Log::Debug("Processed event %d with result %d", event, parent_result);
					return parent_result;
				}
				return 0;
			}

			virtual void show()
			{
				Fl_Double_Window::show();
			}

			void SelectItem(cApplication* parent)
			{
				Fl::visual(FL_DOUBLE|FL_RGB);
				show();
				Fl_X* fx = Fl_X::i(this);
				//SetParent(fx->xid, mParent);
				while (!mDone)
				{
					if (parent) parent->HandleEvents();
					Fl::wait();
				}
			}
			virtual void draw()
			{
				DrawSelectionRect(mCursorHeight);
			}
			void DrawSelectionRect(int y)
			{
				//color(FL_MAGENTA, FL_DARK3);
				int half_height = mSelectionHeight / 2;
				y = Clamp(half_height, y, mHeight - mSelectionHeight/2);
				float fraction = y / static_cast<float>(mHeight);
				uint8_t red = static_cast<uint8_t>(fraction * 255);
				fl_draw_box(FL_BORDER_BOX, 0, 0, mWidth, mHeight, FL_DARK1);
				//color(FL_MAGENTA, FL_YELLOW);
				fl_draw_box(FL_BORDER_BOX ,0, y - half_height, mWidth, mSelectionHeight, fl_rgb_color( red, 34, 128));
			}
		private:
			bool mDone;
			int mWidth;
			int mHeight;
			int mSelectionHeight;
			int mCursorHeight;
			Window mParent;
		};
	}

	cPopUpWindow::cPopUpWindow(int x_pos, int y_pos, int width, int height, SDL_Window* parent)
		:mPopImpl(new fltk::cPopUp(x_pos, y_pos, width, height, parent))
	{
		
		//Log::Info("Selected menu item %s", m->label());
		
		
	}

	cPopUpWindow::~cPopUpWindow()
	{

	}

	void cPopUpWindow::SelectItem(cApplication* parent /*= 0*/)
	{
		mPopImpl->SelectItem(parent);
	}



} // namespace akj