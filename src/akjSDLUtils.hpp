#pragma once
#include "akj_typedefs.hpp"

struct SDL_Window;

namespace akj
{
	namespace SDL{
		const char* EventTypeToString(uint32_t event_type);
		const char* WindowEventTypeToString(uint32_t event_type);
		void* GetWindowHandle(SDL_Window* window);
	}
}
