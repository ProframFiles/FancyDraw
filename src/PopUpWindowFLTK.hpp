#pragma once
#include "akj_typedefs.hpp"
#include <memory>

struct SDL_Window;
namespace akj
{
	namespace fltk
	{
		class cPopUp;
	}
	class cApplication;
	
	class cPopUpWindow
	{
		public:
			cPopUpWindow(int x_pos, int y_pos, int width, int height, SDL_Window* parent);
			virtual ~cPopUpWindow();
			virtual void SelectItem(cApplication* parent = 0);
		private:
			std::unique_ptr<fltk::cPopUp> mPopImpl;
	};
	
}
