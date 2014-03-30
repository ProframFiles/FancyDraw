#include "akjFreeTypeLibrary.hpp"
#include "akjFreeTypeErrors.hpp"
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_MODULE_H 

namespace akj
{
	std::unique_ptr<FreeTypeErrors> FreeTypeLibrary::mErrors;


	FreeTypeLibrary::FreeTypeLibrary() {
		int ret = 0;

		ret = FT_Init_FreeType(&mLibrary);
		int hinting_engine;
		FT_Property_Get( mLibrary, "cff", "hinting-engine", &hinting_engine );

		if (ret) {
			AKJ_FREETYPE_ERROR("Freetype Library Inititialization"
				" Failed: ", ret);
		}
	}

	FreeTypeLibrary::~FreeTypeLibrary() {
		FT_Done_FreeType(mLibrary);
	}

	cStringRef FreeTypeLibrary::ErrorString(int error_code)
	{
		if(mErrors)
		{
			return mErrors->getErrorString(error_code);
		}
		return cStringRef();
	}

} // namespace akj