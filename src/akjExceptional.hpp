#pragma once
#include "akjLog.hpp"
#include "StringRef.hpp"
#include <exception>
#include "FatalError.hpp"
#include "SmallString.hpp"
#include "akjWindowsHeader.hpp"

#ifdef _MSC_VER
#include "akjWindowsHeader.hpp"
#define AKJ_DEBUG_BREAK  DebugBreak();
#endif


#define AKJ_ASSERT_ALWAYS(cond_) do{if(!(cond_)){akj::Log::Error(\
	"(" __FILE__ ":" AKJ_STRINGIFY(__LINE__) ") Asertion failed: \"" #cond_ "\"" \
); AKJ_DEBUG_BREAK; assert(false); } } while(false)

#ifndef NDEBUG
// in debug-compatible mode
#define AKJ_ASSERT(cond_) AKJ_ASSERT_ALWAYS(cond_)
#define AKJ_THROW(reason_) do{ \
	throw ::akj::Exception(reason_, __FILE__, __LINE__); }while(false)
#else
// release mode, swallow it
#define AKJ_ASSERT(cond_)

#define AKJ_THROW(reason_) throw akj::Exception(reason_, __FILE__, __LINE__)
#endif 

#define AKJ_WARN_IF(cond_) do{if(!(cond_)){akj::Log::Warn(\
	"(" __FILE__ ":" AKJ_STRINGIFY(__LINE__) ") " #cond_ \
	); } }while(false) 
#define AKJ_TERMINATE(reason) do{\
akj::FatalError::Die( \
	"(" __FILE__ ":" AKJ_STRINGIFY(__LINE__) ") " reason \
);}while(false) 








#define AKJ_ASSERT_AND_THROW(cond_) do{ \
if(!(cond_)){\
	AKJ_ASSERT(!#cond_); \
 AKJ_THROW("Assertion failed " #cond_); \
}}while(false)

namespace akj
{
	class Exception: public std::exception
	{
	public:
		Exception(const Twine& wat, cStringRef file, int line)
			:mWhatString()
			,mFile(file)
			,mLine(line)
		{
			SmallString<1024> temp_string;
			wat.toVector(temp_string);
			(Twine("(")+ cStringRef(mFile) + ":" + Twine(mLine) + ") \n\t")
				.toVector(mWhatString);
			
			// re-indent the error message
			for (uint32_t i = 0; i < temp_string.size() ; ++i)
			{
				mWhatString.push_back(temp_string[i]);
				if(temp_string[i] == '\n')
				{
					mWhatString.push_back('\t');
				}
			}
			//null terminate!
			mWhatString.push_back('\0');
		}
		virtual const char* what() const
		{
			return mWhatString.data();
		}
	private:
		SmallString<256> mFile;
		int mLine;
		SmallString<1024> mWhatString;
	};
}
