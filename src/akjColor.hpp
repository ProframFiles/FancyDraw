#pragma once

#include "akj_typedefs.hpp"
#include "FancyDrawMath.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include "akjRandom.hpp"
#include <cmath>

namespace akj{

	inline float LinearToSRGB(float input)
	{
		if(input >= 0.0031308f)
		{
			return 1.055f*std::pow(input, 0.416666f)-0.055f;
		}
		return 12.92f*input;
	}

	class cWebColor
	{
	public:
		enum eWebColor {
			ALICEBLUE = 0XF0F8FF,
			ANTIQUEWHITE = 0XFAEBD7,
			AQUA = 0X00FFFF,
			AQUAMARINE = 0X7FFFD4,
			AZURE = 0XF0FFFF,
			BEIGE = 0XF5F5DC,
			BISQUE = 0XFFE4C4,
			BLACK = 0X000000,
			BLANCHEDALMOND = 0XFFEBCD,
			BLUE = 0X0000FF,
			BLUEVIOLET = 0X8A2BE2,
			BROWN = 0XA52A2A,
			BURLYWOOD = 0XDEB887,
			CADETBLUE = 0X5F9EA0,
			CHARTREUSE = 0X7FFF00,
			CHOCOLATE = 0XD2691E,
			CORAL = 0XFF7F50,
			CORNFLOWERBLUE = 0X6495ED,
			CORNSILK = 0XFFF8DC,
			CRIMSON = 0XDC143C,
			CYAN = 0X00FFFF,
			DARKBLUE = 0X00008B,
			DARKCYAN = 0X008B8B,
			DARKGOLDENROD = 0XB8860B,
			DARKGRAY = 0XA9A9A9,
			DARKGREY = 0XA9A9A9,
			DARKGREEN = 0X006400,
			DARKKHAKI = 0XBDB76B,
			DARKMAGENTA = 0X8B008B,
			DARKOLIVEGREEN = 0X556B2F,
			DARKORANGE = 0XFF8C00,
			DARKORCHID = 0X9932CC,
			DARKRED = 0X8B0000,
			DARKSALMON = 0XE9967A,
			DARKSEAGREEN = 0X8FBC8F,
			DARKSLATEBLUE = 0X483D8B,
			DARKSLATEGRAY = 0X2F4F4F,
			DARKSLATEGREY = 0X2F4F4F,
			DARKTURQUOISE = 0X00CED1,
			DARKVIOLET = 0X9400D3,
			DEEPPINK = 0XFF1493,
			DEEPSKYBLUE = 0X00BFFF,
			DIMGRAY = 0X696969,
			DIMGREY = 0X696969,
			DODGERBLUE = 0X1E90FF,
			FIREBRICK = 0XB22222,
			FLORALWHITE = 0XFFFAF0,
			FORESTGREEN = 0X228B22,
			FUCHSIA = 0XFF00FF,
			GAINSBORO = 0XDCDCDC,
			GHOSTWHITE = 0XF8F8FF,
			GOLD = 0XFFD700,
			GOLDENROD = 0XDAA520,
			GRAY = 0X808080,
			GREY = 0X808080,
			GREEN = 0X00FF00,
			GREENYELLOW = 0XADFF2F,
			HONEYDEW = 0XF0FFF0,
			HOTPINK = 0XFF69B4,
			INDIANRED = 0XCD5C5C,
			INDIGO = 0X4B0082,
			IVORY = 0XFFFFF0,
			KHAKI = 0XF0E68C,
			LAVENDER = 0XE6E6FA,
			LAVENDERBLUSH = 0XFFF0F5,
			LAWNGREEN = 0X7CFC00,
			LEAFGREEN = 0x008000,
			LEMONCHIFFON = 0XFFFACD,
			LIGHTBLUE = 0XADD8E6,
			LIGHTCORAL = 0XF08080,
			LIGHTCYAN = 0XE0FFFF,
			LIGHTGOLDENRODYELLOW = 0XFAFAD2,
			LIGHTGRAY = 0XD3D3D3,
			LIGHTGREY = 0XD3D3D3,
			LIGHTGREEN = 0X90EE90,
			LIGHTPINK = 0XFFB6C1,
			LIGHTSALMON = 0XFFA07A,
			LIGHTSEAGREEN = 0X20B2AA,
			LIGHTSKYBLUE = 0X87CEFA,
			LIGHTSLATEGRAY = 0X778899,
			LIGHTSLATEGREY = 0X778899,
			LIGHTSTEELBLUE = 0XB0C4DE,
			LIGHTYELLOW = 0XFFFFE0,
			LIME = 0X00FF00,
			LIMEGREEN = 0X32CD32,
			LINEN = 0XFAF0E6,
			MAGENTA = 0XFF00FF,
			MAROON = 0X800000,
			MEDIUMAQUAMARINE = 0X66CDAA,
			MEDIUMBLUE = 0X0000CD,
			MEDIUMORCHID = 0XBA55D3,
			MEDIUMPURPLE = 0X9370D8,
			MEDIUMSEAGREEN = 0X3CB371,
			MEDIUMSLATEBLUE = 0X7B68EE,
			MEDIUMSPRINGGREEN = 0X00FA9A,
			MEDIUMTURQUOISE = 0X48D1CC,
			MEDIUMVIOLETRED = 0XC71585,
			MIDNIGHTBLUE = 0X191970,
			MINTCREAM = 0XF5FFFA,
			MISTYROSE = 0XFFE4E1,
			MOCCASIN = 0XFFE4B5,
			NAVAJOWHITE = 0XFFDEAD,
			NAVY = 0X000080,
			OLDLACE = 0XFDF5E6,
			OLIVE = 0X808000,
			OLIVEDRAB = 0X6B8E23,
			ORANGE = 0XFFA500,
			ORANGERED = 0XFF4500,
			ORCHID = 0XDA70D6,
			PALEGOLDENROD = 0XEEE8AA,
			PALEGREEN = 0X98FB98,
			PALETURQUOISE = 0XAFEEEE,
			PALEVIOLETRED = 0XD87093,
			PAPAYAWHIP = 0XFFEFD5,
			PEACHPUFF = 0XFFDAB9,
			PERU = 0XCD853F,
			PINK = 0XFFC0CB,
			PLUM = 0XDDA0DD,
			POWDERBLUE = 0XB0E0E6,
			PURPLE = 0X800080,
			RED = 0XFF0000,
			ROSYBROWN = 0XBC8F8F,
			ROYALBLUE = 0X4169E1,
			SADDLEBROWN = 0X8B4513,
			SALMON = 0XFA8072,
			SANDYBROWN = 0XF4A460,
			SEAGREEN = 0X2E8B57,
			SEASHELL = 0XFFF5EE,
			SIENNA = 0XA0522D,
			SILVER = 0XC0C0C0,
			SKYBLUE = 0X87CEEB,
			SLATEBLUE = 0X6A5ACD,
			SLATEGRAY = 0X708090,
			SLATEGREY = 0X708090,
			SNOW = 0XFFFAFA,
			SPRINGGREEN = 0X00FF7F,
			STEELBLUE = 0X4682B4,
			TAN = 0XD2B48C,
			TEAL = 0X008080,
			THISTLE = 0XD8BFD8,
			TOMATO = 0XFF6347,
			TURQUOISE = 0X40E0D0,
			VIOLET = 0XEE82EE,
			WHEAT = 0XF5DEB3,
			WHITE = 0XFFFFFF,
			WHITESMOKE = 0XF5F5F5,
			YELLOW = 0XFFFF00,
			YELLOWGREEN = 0X9ACD32
		};
		static cWebColor::eWebColor Random();
		static std::string ToString(cWebColor::eWebColor enum_in);
	private:

		static void ToLower(std::string& s);
		static std::pair<std::string, cWebColor::eWebColor>
			colorPair(const char* s_in, cWebColor::eWebColor code);
		class cColorTable
		{
		public:
			cColorTable();
			~cColorTable(){};
			std::unordered_map<std::string, cWebColor::eWebColor > colors;
			std::unordered_map<uint32_t, std::string > names;
			std::unordered_map<cWebColor::eWebColor, uint32_t > colorsFromEnum;
			std::vector<cWebColor::eWebColor> flatVec;
			std::vector<uint32_t> redVec;
			std::vector<uint32_t> greenVec;
			std::vector<uint32_t> blueVec;
			uint32_t size;
		};
		static cColorTable sColorTable;
		static cRandom mRNG;
	};

	struct RGBAu8;
	struct RGBAf
	{
		RGBAf(float in_r, float in_g, float in_b, float in_a)
		: r(in_r), g(in_g), b(in_b), a(in_a)
		{}
		RGBAf()
			: r(1.0f), g(0.0f), b(1.0f), a(1.0f)
		{}
		RGBAf(const RGBAu8& color);
		RGBAf(cWebColor::eWebColor color);

		float r, g, b, a;
	};

	struct RGBAu8
	{
		RGBAu8()
			: r(255), g(0), b(255), a(255)
		{}
		RGBAu8(uint8_t in_r, uint8_t in_g, uint8_t in_b, uint8_t in_a)
			: r(in_r), g(in_g), b(in_b), a(in_a)
		{}
		RGBAu8(uint8_t in_r, uint8_t in_g, uint8_t in_b)
			: r(in_r), g(in_g), b(in_b), a(255)
		{}
		RGBAu8(uint32_t hex_in)
			: r(static_cast<uint8_t>((0xFF000000 & hex_in) >> 24))
			, g(static_cast<uint8_t>((0x00FF0000 & hex_in) >> 16))
			, b(static_cast<uint8_t>((0x0000FF00 & hex_in) >> 8))
			, a(static_cast<uint8_t>((0xFF & hex_in)))
		{}
		explicit RGBAu8(const RGBAf& color)
			: r(static_cast<uint8_t>(255.999f*Clamp(0.0f, color.r, 1.0f)))
			, g(static_cast<uint8_t>(255.999f*Clamp(0.0f, color.g, 1.0f)))
			, b(static_cast<uint8_t>(255.999f*Clamp(0.0f, color.b, 1.0f)))
			, a(static_cast<uint8_t>(255.999f*Clamp(0.0f, color.a, 1.0f)))
		{}
		RGBAu8(cWebColor::eWebColor color)
			: a(255)
			, b(static_cast<uint8_t>(0x0000FF & color))
			, g(static_cast<uint8_t>((0x00FF00 & color) >> 8))
			, r(static_cast<uint8_t>((0xFF0000 & color) >> 16))
		{}
		RGBAu8 Alpha(uint8_t alpha)
		{
			RGBAu8 ret = *this;
			ret.a = alpha;
			return ret;
		}
		uint32_t AsUInt() const
		{
			return *reinterpret_cast<const uint32_t*>(this);
		}
		uint8_t r, g, b, a;
	};
	//compiler error on wrong size
	typedef  char cRGBASIZE[(sizeof(RGBAu8) == 4)];

	inline RGBAf::RGBAf(const RGBAu8& color)
		: r(color.r*0.003922f)
		, g(color.g*0.003922f)
		, b(color.b*0.003922f)
		, a(color.a*0.003922f)
	{}
	inline RGBAf::RGBAf(cWebColor::eWebColor color)
	{
		*this = RGBAf(RGBAu8(color));
	}


} //namespace akj
