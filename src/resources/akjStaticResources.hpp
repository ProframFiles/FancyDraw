#pragma once
#include "StringRef.hpp"

namespace akj
{
	class cStaticResources
	{
	public:
		static cStringRef get_icon_png();
		static cStringRef get_svord_png();
		static cStringRef get_UI_font();
		static cStringRef get_Text_font();
		static cStringRef get_Mono_font();
		static cStringRef get_PrimitiveShape_glsl();
		static cStringRef get_TexturedQuad_glsl();
		static cStringRef get_ScreenTextAlpha_glsl();
		static cStringRef get_PrimitiveShapeTester_glsl();
		static cStringRef get_WireframeShape_glsl();
		static cStringRef get_MousePointer_glsl();
	};
	
}
