#include "akjColor.hpp" 
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <algorithm>
#include "akjRandom.hpp"
#include "akjColorHash.hpp"

namespace akj{
	cWebColor::cColorTable cWebColor::sColorTable;
	cRandom cWebColor::mRNG;

	cWebColor::eWebColor cWebColor::Random(){
		return sColorTable.flatVec[mRNG.UInt(static_cast<uint32_t>(sColorTable.size-1))];
	}

	std::string cWebColor::ToString(cWebColor::eWebColor enum_in){
		return sColorTable.names[enum_in];
	}

	void cWebColor::ToLower(std::string& s){
		for (size_t i = 0; i < s.size() ; ++i)
		{
			if(s[i] > 91 && s[i] > 64)
			{
				s[i] += 0x20;
			}
		}
	}

	std::pair<std::string, cWebColor::eWebColor> cWebColor::colorPair(const char* s_in, cWebColor::eWebColor code){
		std::string s=s_in;
		ToLower(s);
		return make_pair(s, code);
	}

	cWebColor::cColorTable::cColorTable(){
		auto iter =colors.begin();

		iter=colors.insert(iter, colorPair("Alice Blue", cWebColor::ALICEBLUE));
		iter=colors.insert(iter, colorPair("Antique White", cWebColor::ANTIQUEWHITE));
		iter=colors.insert(iter, colorPair("Aqua", cWebColor::AQUA));
		iter=colors.insert(iter, colorPair("Aquamarine", cWebColor::AQUAMARINE));
		iter=colors.insert(iter, colorPair("Azure", cWebColor::AZURE));
		iter=colors.insert(iter, colorPair("Beige", cWebColor::BEIGE));
		iter=colors.insert(iter, colorPair("Bisque", cWebColor::BISQUE));
		iter=colors.insert(iter, colorPair("Black", cWebColor::BLACK));
		iter=colors.insert(iter, colorPair("Blanched Almond", cWebColor::BLANCHEDALMOND));
		iter=colors.insert(iter, colorPair("Blue", cWebColor::BLUE));
		iter=colors.insert(iter, colorPair("Blue Violet", cWebColor::BLUEVIOLET));
		iter=colors.insert(iter, colorPair("Brown", cWebColor::BROWN));
		iter=colors.insert(iter, colorPair("Burly Wood", cWebColor::BURLYWOOD));
		iter=colors.insert(iter, colorPair("Cadet Blue", cWebColor::CADETBLUE));
		iter=colors.insert(iter, colorPair("Chartreuse", cWebColor::CHARTREUSE));
		iter=colors.insert(iter, colorPair("Chocolate", cWebColor::CHOCOLATE));
		iter=colors.insert(iter, colorPair("Coral", cWebColor::CORAL));
		iter=colors.insert(iter, colorPair("Cornflower Blue", cWebColor::CORNFLOWERBLUE));
		iter=colors.insert(iter, colorPair("Cornsilk", cWebColor::CORNSILK));
		iter=colors.insert(iter, colorPair("Crimson", cWebColor::CRIMSON));
		iter=colors.insert(iter, colorPair("Cyan", cWebColor::CYAN));
		iter=colors.insert(iter, colorPair("Dark Blue", cWebColor::DARKBLUE));
		iter=colors.insert(iter, colorPair("Dark Cyan", cWebColor::DARKCYAN));
		iter=colors.insert(iter, colorPair("Dark Golden Rod", cWebColor::DARKGOLDENROD));
		iter=colors.insert(iter, colorPair("Dark Gray", cWebColor::DARKGRAY));
		iter=colors.insert(iter, colorPair("Dark Grey", cWebColor::DARKGREY));
		iter=colors.insert(iter, colorPair("Dark Green", cWebColor::DARKGREEN));
		iter=colors.insert(iter, colorPair("Dark Khaki", cWebColor::DARKKHAKI));
		iter=colors.insert(iter, colorPair("Dark Magenta", cWebColor::DARKMAGENTA));
		iter=colors.insert(iter, colorPair("Dark Olive Green", cWebColor::DARKOLIVEGREEN));
		iter=colors.insert(iter, colorPair("Dark Orange", cWebColor::DARKORANGE));
		iter=colors.insert(iter, colorPair("Dark Orchid", cWebColor::DARKORCHID));
		iter=colors.insert(iter, colorPair("Dark Red", cWebColor::DARKRED));
		iter=colors.insert(iter, colorPair("Dark Salmon", cWebColor::DARKSALMON));
		iter=colors.insert(iter, colorPair("Dark Sea Green", cWebColor::DARKSEAGREEN));
		iter=colors.insert(iter, colorPair("Dark Slate Blue", cWebColor::DARKSLATEBLUE));
		iter=colors.insert(iter, colorPair("Dark Slate Gray", cWebColor::DARKSLATEGRAY));
		iter=colors.insert(iter, colorPair("Dark Slate Grey", cWebColor::DARKSLATEGREY));
		iter=colors.insert(iter, colorPair("Dark Turquoise", cWebColor::DARKTURQUOISE));
		iter=colors.insert(iter, colorPair("Dark Violet", cWebColor::DARKVIOLET));
		iter=colors.insert(iter, colorPair("Deep Pink", cWebColor::DEEPPINK));
		iter=colors.insert(iter, colorPair("Deep Sky Blue", cWebColor::DEEPSKYBLUE));
		iter=colors.insert(iter, colorPair("Dim Gray", cWebColor::DIMGRAY));
		iter=colors.insert(iter, colorPair("Dim Grey", cWebColor::DIMGREY));
		iter=colors.insert(iter, colorPair("Dodger Blue", cWebColor::DODGERBLUE));
		iter=colors.insert(iter, colorPair("Fire Brick", cWebColor::FIREBRICK));
		iter=colors.insert(iter, colorPair("Floral White", cWebColor::FLORALWHITE));
		iter=colors.insert(iter, colorPair("Forest Green", cWebColor::FORESTGREEN));
		iter=colors.insert(iter, colorPair("Fuchsia", cWebColor::FUCHSIA));
		iter=colors.insert(iter, colorPair("Gainsboro", cWebColor::GAINSBORO));
		iter=colors.insert(iter, colorPair("Ghost White", cWebColor::GHOSTWHITE));
		iter=colors.insert(iter, colorPair("Gold", cWebColor::GOLD));
		iter=colors.insert(iter, colorPair("Golden Rod", cWebColor::GOLDENROD));
		iter=colors.insert(iter, colorPair("Gray", cWebColor::GRAY));
		iter=colors.insert(iter, colorPair("Grey", cWebColor::GREY));
		iter=colors.insert(iter, colorPair("Green", cWebColor::GREEN));
		iter=colors.insert(iter, colorPair("Green Yellow", cWebColor::GREENYELLOW));
		iter=colors.insert(iter, colorPair("Honey Dew", cWebColor::HONEYDEW));
		iter=colors.insert(iter, colorPair("Hot Pink", cWebColor::HOTPINK));
		iter=colors.insert(iter, colorPair("Indian Red", cWebColor::INDIANRED));
		iter=colors.insert(iter, colorPair("Indigo", cWebColor::INDIGO));
		iter=colors.insert(iter, colorPair("Ivory", cWebColor::IVORY));
		iter=colors.insert(iter, colorPair("Khaki", cWebColor::KHAKI));
		iter=colors.insert(iter, colorPair("Lavender", cWebColor::LAVENDER));
		iter=colors.insert(iter, colorPair("Lavender Blush", cWebColor::LAVENDERBLUSH));
		iter=colors.insert(iter, colorPair("Lawn Green", cWebColor::LAWNGREEN));
		iter=colors.insert(iter, colorPair("Leaf Green", cWebColor::LEAFGREEN));
		iter=colors.insert(iter, colorPair("Lemon Chiffon", cWebColor::LEMONCHIFFON));
		iter=colors.insert(iter, colorPair("Light Blue", cWebColor::LIGHTBLUE));
		iter=colors.insert(iter, colorPair("Light Coral", cWebColor::LIGHTCORAL));
		iter=colors.insert(iter, colorPair("Light Cyan", cWebColor::LIGHTCYAN));
		iter=colors.insert(iter, colorPair("Light Goldenrod Yellow", cWebColor::LIGHTGOLDENRODYELLOW));
		iter=colors.insert(iter, colorPair("Light Gray", cWebColor::LIGHTGRAY));
		iter=colors.insert(iter, colorPair("Light Grey", cWebColor::LIGHTGREY));
		iter=colors.insert(iter, colorPair("Light Green", cWebColor::LIGHTGREEN));
		iter=colors.insert(iter, colorPair("Light Pink", cWebColor::LIGHTPINK));
		iter=colors.insert(iter, colorPair("Light Salmon", cWebColor::LIGHTSALMON));
		iter=colors.insert(iter, colorPair("Light Sea Green", cWebColor::LIGHTSEAGREEN));
		iter=colors.insert(iter, colorPair("Light Sky Blue", cWebColor::LIGHTSKYBLUE));
		iter=colors.insert(iter, colorPair("Light Slate Gray", cWebColor::LIGHTSLATEGRAY));
		iter=colors.insert(iter, colorPair("Light Slate Grey", cWebColor::LIGHTSLATEGREY));
		iter=colors.insert(iter, colorPair("Light Steel Blue", cWebColor::LIGHTSTEELBLUE));
		iter=colors.insert(iter, colorPair("Light Yellow", cWebColor::LIGHTYELLOW));
		iter=colors.insert(iter, colorPair("Lime", cWebColor::LIME));
		iter=colors.insert(iter, colorPair("Lime Green", cWebColor::LIMEGREEN));
		iter=colors.insert(iter, colorPair("Linen", cWebColor::LINEN));
		iter=colors.insert(iter, colorPair("Magenta", cWebColor::MAGENTA));
		iter=colors.insert(iter, colorPair("Maroon", cWebColor::MAROON));
		iter=colors.insert(iter, colorPair("Medium Aqua Marine", cWebColor::MEDIUMAQUAMARINE));
		iter=colors.insert(iter, colorPair("Medium Blue", cWebColor::MEDIUMBLUE));
		iter=colors.insert(iter, colorPair("Medium Orchid", cWebColor::MEDIUMORCHID));
		iter=colors.insert(iter, colorPair("Medium Purple", cWebColor::MEDIUMPURPLE));
		iter=colors.insert(iter, colorPair("Medium Sea Green", cWebColor::MEDIUMSEAGREEN));
		iter=colors.insert(iter, colorPair("Medium Slate Blue", cWebColor::MEDIUMSLATEBLUE));
		iter=colors.insert(iter, colorPair("Medium Spring Green", cWebColor::MEDIUMSPRINGGREEN));
		iter=colors.insert(iter, colorPair("Medium Turquoise", cWebColor::MEDIUMTURQUOISE));
		iter=colors.insert(iter, colorPair("Medium Violet Red", cWebColor::MEDIUMVIOLETRED));
		iter=colors.insert(iter, colorPair("Midnight Blue", cWebColor::MIDNIGHTBLUE));
		iter=colors.insert(iter, colorPair("Mint Cream", cWebColor::MINTCREAM));
		iter=colors.insert(iter, colorPair("Misty Rose", cWebColor::MISTYROSE));
		iter=colors.insert(iter, colorPair("Moccasin", cWebColor::MOCCASIN));
		iter=colors.insert(iter, colorPair("Navajo White", cWebColor::NAVAJOWHITE));
		iter=colors.insert(iter, colorPair("Navy", cWebColor::NAVY));
		iter=colors.insert(iter, colorPair("Old Lace", cWebColor::OLDLACE));
		iter=colors.insert(iter, colorPair("Olive", cWebColor::OLIVE));
		iter=colors.insert(iter, colorPair("Olive Drab", cWebColor::OLIVEDRAB));
		iter=colors.insert(iter, colorPair("Orange", cWebColor::ORANGE));
		iter=colors.insert(iter, colorPair("Orange Red", cWebColor::ORANGERED));
		iter=colors.insert(iter, colorPair("Orchid", cWebColor::ORCHID));
		iter=colors.insert(iter, colorPair("Pale Golden Rod", cWebColor::PALEGOLDENROD));
		iter=colors.insert(iter, colorPair("Pale Green", cWebColor::PALEGREEN));
		iter=colors.insert(iter, colorPair("Pale Turquoise", cWebColor::PALETURQUOISE));
		iter=colors.insert(iter, colorPair("Pale Violet Red", cWebColor::PALEVIOLETRED));
		iter=colors.insert(iter, colorPair("Papaya Whip", cWebColor::PAPAYAWHIP));
		iter=colors.insert(iter, colorPair("Peach Puff", cWebColor::PEACHPUFF));
		iter=colors.insert(iter, colorPair("Peru", cWebColor::PERU));
		iter=colors.insert(iter, colorPair("Pink", cWebColor::PINK));
		iter=colors.insert(iter, colorPair("Plum", cWebColor::PLUM));
		iter=colors.insert(iter, colorPair("Powder Blue", cWebColor::POWDERBLUE));
		iter=colors.insert(iter, colorPair("Purple", cWebColor::PURPLE));
		iter=colors.insert(iter, colorPair("Red", cWebColor::RED));
		iter=colors.insert(iter, colorPair("Rosy Brown", cWebColor::ROSYBROWN));
		iter=colors.insert(iter, colorPair("Royal Blue", cWebColor::ROYALBLUE));
		iter=colors.insert(iter, colorPair("Saddle Brown", cWebColor::SADDLEBROWN));
		iter=colors.insert(iter, colorPair("Salmon", cWebColor::SALMON));
		iter=colors.insert(iter, colorPair("Sandy Brown", cWebColor::SANDYBROWN));
		iter=colors.insert(iter, colorPair("Sea Green", cWebColor::SEAGREEN));
		iter=colors.insert(iter, colorPair("Sea Shell", cWebColor::SEASHELL));
		iter=colors.insert(iter, colorPair("Sienna", cWebColor::SIENNA));
		iter=colors.insert(iter, colorPair("Silver", cWebColor::SILVER));
		iter=colors.insert(iter, colorPair("Sky Blue", cWebColor::SKYBLUE));
		iter=colors.insert(iter, colorPair("Slate Blue", cWebColor::SLATEBLUE));
		iter=colors.insert(iter, colorPair("Slate Gray", cWebColor::SLATEGRAY));
		iter=colors.insert(iter, colorPair("Slate Grey", cWebColor::SLATEGREY));
		iter=colors.insert(iter, colorPair("Snow", cWebColor::SNOW));
		iter=colors.insert(iter, colorPair("Spring Green", cWebColor::SPRINGGREEN));
		iter=colors.insert(iter, colorPair("Steel Blue", cWebColor::STEELBLUE));
		iter=colors.insert(iter, colorPair("Tan", cWebColor::TAN));
		iter=colors.insert(iter, colorPair("Teal", cWebColor::TEAL));
		iter=colors.insert(iter, colorPair("Thistle", cWebColor::THISTLE));
		iter=colors.insert(iter, colorPair("Tomato", cWebColor::TOMATO));
		iter=colors.insert(iter, colorPair("Turquoise", cWebColor::TURQUOISE));
		iter=colors.insert(iter, colorPair("Violet", cWebColor::VIOLET));
		iter=colors.insert(iter, colorPair("Wheat", cWebColor::WHEAT));
		iter=colors.insert(iter, colorPair("White", cWebColor::WHITE));
		iter=colors.insert(iter, colorPair("White Smoke", cWebColor::WHITESMOKE));
		iter=colors.insert(iter, colorPair("Yellow", cWebColor::YELLOW));
		iter=colors.insert(iter, colorPair("Yellow Green", cWebColor::YELLOWGREEN));

		int indx=0;
		flatVec.resize(colors.size());
		redVec.resize(colors.size());
		greenVec.resize(colors.size());
		blueVec.resize(colors.size());
		size = static_cast<uint32_t>(colors.size());
		std::for_each(colors.begin(),colors.end(),
			[&](std::pair<std::string, cWebColor::eWebColor> str_hex){
				flatVec[indx]=str_hex.second;
				redVec[indx]=str_hex.second&0xFF0000;
				greenVec[indx]=str_hex.second&0x00FF00;
				blueVec[indx]=str_hex.second&0x0000FF;
				indx++;
				if (str_hex.second >= 256) colorsFromEnum.insert(std::make_pair(str_hex.second, static_cast< cWebColor::eWebColor>(str_hex.second - 256)));
				names.insert(std::make_pair(str_hex.second, str_hex.first));
		});
	
	}

} //end namespace akj


