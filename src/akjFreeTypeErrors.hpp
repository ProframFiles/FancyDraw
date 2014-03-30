////////////////////////////////////////////////////////////////////////////
//
// file akjFreeTypeErrors.hpp
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ft2build.h"
#include "StringRef.hpp"
#include FT_FREETYPE_H
#include <map>
#include <cstdint>
#include "akjExceptional.hpp"
namespace akj {
	
	class FreeTypeException: public Exception {
	public:
		explicit FreeTypeException(const char* msg, const char* file, 
																int line, int code)
		: Exception(msg, file, line), mFreeTypeError(code){};
		const int mFreeTypeError;
	};
#define AKJ_FREETYPE_ERROR(msg_,code_)\
	throw FreeTypeException(msg_, __FILE__, __LINE__, code_)

#define AKJ_FREETYPE_CHECKED_CALL(call_)\
	do{int err; if( (err = call_ )!=0 ){ \
	throw FreeTypeException(#call_ , __FILE__, __LINE__, err); \
	}}while(false)
	
	
	class FreeTypeErrors{
	public:
		FreeTypeErrors(){
		//This is a bunch of trickery used to get the freetype error codes into the map						                                          
      #undef __FTERRORS_H__                                          
      #define FT_ERRORDEF( e, v, s )  it=errors.insert( it,std::pair<FT_Error, std::string>(v, s)) ;
		#define FT_ERROR_START_LIST    auto it = errors.begin();                             
		#define FT_ERROR_END_LIST                                         
      #include FT_ERRORS_H                                         
    };
			
		~FreeTypeErrors(){};
		cStringRef getErrorString(FT_Error fte){
			auto found = errors.find(fte);
			if(found != errors.end())
			{
				return found->second;
			}
			return cStringRef();
		};
	private:
		std::map<FT_Error, std::string> errors;};

}//end namespace akj

