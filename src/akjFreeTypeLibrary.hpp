////////////////////////////////////////////////////////////////////////////
//
// file akjFreeTypeLibrary.hpp
//
////////////////////////////////////////////////////////////////////////////

#include "SystemError.hpp"
#include "StringRef.hpp"
#include <memory>

#pragma once
struct FT_LibraryRec_;
namespace akj {
	class FreeTypeErrors;
	class FreeTypeLibrary {
	public:
		FreeTypeLibrary();
		~FreeTypeLibrary();
		inline FT_LibraryRec_*  ref(){ return mLibrary; };
		static cStringRef ErrorString(int error_code);
	private:
	FT_LibraryRec_* mLibrary;
	static std::unique_ptr<FreeTypeErrors> mErrors;

	};



}//end namespace akj

