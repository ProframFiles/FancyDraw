#include "akjStaticResources.hpp"
#include "PrimitiveShape.glsl.inc"
#include "WireframeShape.glsl.inc"
#include "MousePointer.glsl.inc"
#include "ScreenTextAlpha.glsl.inc"
#include "TexturedQuad.glsl.inc"
#include "PrimitiveShapeTester.glsl.inc"
#include "icon.png.inc"
#include "svord.png.inc"
#include "Aller.dff.inc"
#include "Consolas.dff.inc"
#include "SegoeUI_SemiBold.dff.inc"



namespace akj{
	cStringRef cStaticResources::get_icon_png()
	{
		return cStringRef(reinterpret_cast<const char*>(icon_png)
											,sizeof(icon_png));
	}
	cStringRef cStaticResources::get_svord_png()
	{
		return cStringRef(reinterpret_cast<const char*>(svord_png)
											,sizeof(svord_png));
	}
	cStringRef cStaticResources::get_UI_font()
	{
		return cStringRef((const char*)(SegoeUI_SemiBold_dff)
							,sizeof(SegoeUI_SemiBold_dff));
	}
	cStringRef cStaticResources::get_Text_font()
	{
		return cStringRef((const char*)(Aller_dff)
							,sizeof(Aller_dff));
	}
	cStringRef cStaticResources::get_Mono_font()
	{
		return cStringRef((const char*)(Consolas_dff)
							,sizeof(Consolas_dff));
	}
	cStringRef cStaticResources::get_ScreenTextAlpha_glsl()
	{
		return cStringRef(ScreenTextAlpha_glsl);
	}
	cStringRef cStaticResources::get_TexturedQuad_glsl()
	{
		return cStringRef(TexturedQuad_glsl);
	}
	cStringRef cStaticResources::get_PrimitiveShape_glsl()
	{
		return cStringRef(PrimitiveShape_glsl);
	}
	cStringRef cStaticResources::get_WireframeShape_glsl()
	{
		return cStringRef(WireframeShape_glsl);
	}
	cStringRef cStaticResources::get_PrimitiveShapeTester_glsl()
	{
		return cStringRef(PrimitiveShapeTester_glsl);
	}
	cStringRef cStaticResources::get_MousePointer_glsl()
	{
		return cStringRef(MousePointer_glsl);
	}
}
