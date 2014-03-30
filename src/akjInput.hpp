#pragma once
#include <stdint.h>
#include "akjCoord2.hpp"

namespace akj
{
	struct cMouseState
	{
		cCoord2 pos;
		uint32_t left;
		uint32_t right;
		uint32_t middle;
		uint32_t extra_a;
		uint32_t extra_b;
		int wheel;
	};

	//ideally we would like to keep this as a 32 bit value 
	// (that can be used as a std::hash)
	// SDl sets the 30th bit for scancodes, but the highest scancode has
	// only 9 bits, so this leaves a gap. There are also 16 bits for modifiers
	// I'm going to fit the modifiers in the gap and use the 31st bit to indicate
	// mouse input
	// 
	// this gives:
	// bits 0-9: mouse buttons
	// bits 0-9: keycodes
	// bits 10-26: modifiers
	// bit 27 button press
	// bit 28 button release
	// bit 29 button double click
	// bit 30 scancode bit
	// bit 31 reserved
	
	// From SDL_keycode.h
	/*
		typedef enum
	{
			KMOD_NONE = 0x0000,
			KMOD_LSHIFT = 0x0001,
			KMOD_RSHIFT = 0x0002,
			KMOD_LCTRL = 0x0040,
			KMOD_RCTRL = 0x0080,
			KMOD_LALT = 0x0100,
			KMOD_RALT = 0x0200,
			KMOD_LGUI = 0x0400,
			KMOD_RGUI = 0x0800,
			KMOD_NUM = 0x1000,
			KMOD_CAPS = 0x2000,
			KMOD_MODE = 0x4000,
			KMOD_RESERVED = 0x8000
	} SDL_Keymod;

	#define KMOD_CTRL   (KMOD_LCTRL|KMOD_RCTRL)
	#define KMOD_SHIFT  (KMOD_LSHIFT|KMOD_RSHIFT)
	#define KMOD_ALT    (KMOD_LALT|KMOD_RALT)
	#define KMOD_GUI    (KMOD_LGUI|KMOD_RGUI)

*/
	struct cInputID
	{
	cInputID(): mValue(0)
	{}
		enum eSDLADjust
		{
			kFirstSDLButton = 1
		};
		enum eMouseButtons
		{
			kLeftButton = 1 << 0,
			kMiddleButton = 2 << 1,
			kRightButton = 3 << 1,
			kX1Button = 4 << 1,
			kX2Button = 5 << 1,
			kWheelUpButton = 6 << 1,
			kWheelDownButton = 7 << 1,
			kWheelLeftButton = 8 << 1,
			kWheelRightButton = 9 << 1
		};
		enum 
		{
			kModifierShift = 10,
			kValueMask = (1 << kModifierShift)-1, 
			kModifierMask = ((1 << 16)-1) << kModifierShift,
			kMouseButtonDown = 1 << 27,
			kMouseButtonUp = 1 << 28,
			kMouseButtonDouble = 1 << 29,
			kMouseButtonBits = kMouseButtonUp | kMouseButtonDown | kMouseButtonDouble,
			kScanCodeBit = 1 << 30,
			kKeyReleaseBit = 1 << 31
		};
		bool IsKeyboardInput() const 
		{
			return (mValue & kValueMask) > 0  && (kMouseButtonBits & mValue) == 0;
		}
		bool IsMouseInput() const 
		{
			return (mValue & kValueMask) > 0 && (kMouseButtonBits & mValue) == 1;
		}
		uint16_t Modifier() const
		{
			return static_cast<uint16_t>((kModifierMask&mValue) >> kModifierShift);
		}
		uint32_t mValue;
	};

	inline cInputID FromSDLK(uint32_t keycode, unsigned short modifier = 0)
	{
		cInputID ret;
		ret.mValue = keycode | (modifier << cInputID::kModifierShift );
		return ret;
	}

	inline cInputID FromSDLKRelease(uint32_t keycode, unsigned short modifier = 0)
	{
		cInputID ret;
		ret.mValue = keycode | (modifier << cInputID::kModifierShift )
								| cInputID::kKeyReleaseBit;
		return ret;
	}

	inline cInputID FromSDLMouseUp(uint32_t button, unsigned short modifier = 0)
	{
		cInputID ret;
		ret.mValue = cInputID::kMouseButtonUp
							| 1 << (button - cInputID::kFirstSDLButton)
							| (modifier << cInputID::kModifierShift );
		return ret;
	}

	inline cInputID FromSDLMouseDown(uint32_t button, unsigned short modifier = 0)
	{
		cInputID ret;
		ret.mValue = cInputID::kMouseButtonDown
							| 1 << (button - cInputID::kFirstSDLButton)
							| (modifier << cInputID::kModifierShift );
		return ret;
	}
}

namespace std
{
  template<>
  struct hash<akj::cInputID>
  {
    typedef akj::cInputID argument_type;
    typedef uint32_t value_type;
 
    value_type operator()( const argument_type& obj) const
    {
        return obj.mValue;
    }
  };
	template<>
  struct equal_to<akj::cInputID>
  {
    typedef akj::cInputID argument_type;
 
    bool operator()
			( const argument_type& lhs, const argument_type& rhs) const
    {
        return lhs.mValue == rhs.mValue;
    }
  };
}

