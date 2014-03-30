#pragma once
#include <string>
#include <sstream>
#include "StringRef.hpp"
#include <unistd.h>
#include "RawOstream.hpp"
#include "akj_typedefs.hpp"
#include "Twine.hpp"
#include "akjmisc.hpp"



#define AKJ_LOG_CSTR(mytext) __FILE__ AKJ_STRINGIFY(__LINE__) ": " mytext

namespace akj{
	inline std::ostream& operator <<(std::ostream& sout, 	cStringRef str)
	{
		sout.write(str.data(), str.size());
		return sout;
	}



class Log
{
public:
	enum eLogLevel
	{
		LOG_LEVEL_OFF = 0,
		LOG_LEVEL_CRITICAL = 1,
		LOG_LEVEL_ERROR = 2,
		LOG_LEVEL_WARN = 3,
		LOG_LEVEL_INFO = 4,
		LOG_LEVEL_DEBUG = 5,
		LOG_LEVEL_TMI = 6,
		LOG_LEVEL_NUMLEVELS
	};

	~Log(void);

	template< typename... Args>
	static void TMI( Args... args){
		GetInstance()->NewMessage(LOG_LEVEL_TMI).Write(args...);
	}

	static raw_ostream& Debug()
	{
		return GetInstance()->OutStream(LOG_LEVEL_DEBUG);
	}

	template< typename... Args>
	static void Debug( Args... args){
		GetInstance()->NewMessage(LOG_LEVEL_DEBUG).Write(args...);
	}

	template< typename... Args>
	static void Critical(Args... args){
		GetInstance()->NewMessage(LOG_LEVEL_CRITICAL).Write(args...);
	}

	template<typename... Args>
	static void Warn( Args... args){
		GetInstance()->NewMessage(LOG_LEVEL_WARN).Write(args...);
	}

	template< typename... Args>
	static void Info( Args... args){
		GetInstance()->NewMessage(LOG_LEVEL_INFO).Write(args...);
	}

	template<typename... Args>
	static void Error(Args... args) {
		GetInstance()->NewMessage(LOG_LEVEL_ERROR).Write(args...);
	}

	static raw_ostream& Outs(){
		return GetInstance()->OutStream();
	}

	Log& SetLogLevel(eLogLevel level){
		mLogLevelThreshold = level;
		return *this;
	}

	Log& SetConsoleLogLevel(eLogLevel level){
		mConsoleLevelThreshold = level;
		return *this;
	}
	
	// adapted from http://www.stroustrup.com/C++11FAQ.html#variadic-templates
	template<typename T, typename... Args>
	void Write(const char* format_string, T first_arg, Args... other_args) {
		if(mCurrentLogLevel <= mLogLevelThreshold){
			while (format_string && *format_string) {
				if (*format_string=='%' && *++format_string!='%') {	
					mStream.str(std::string());
					mStream << first_arg;
					mLogBuffer.append(mStream.str());
					Write(++format_string, other_args...);
					return;
				}
				mLogBuffer.push_back(*(format_string++));
			}
			Write("\n\t ... Error in format specifier: too many arguments");
		}
	}
	template<typename T>
	void Write(T arg){
		if(mCurrentLogLevel <= mLogLevelThreshold){
			mStream.str(std::string());
			mStream << arg << std::endl;
			mLogBuffer.append(mStream.str());
			WriteString(mLogBuffer.c_str());
			if(mCurrentLogLevel <= mConsoleLevelThreshold){
				printf("%s",mLogBuffer.c_str());
			}
		}
	}

	void Write(const char* arg){
		if(mCurrentLogLevel <= mLogLevelThreshold){
			mLogBuffer.append(arg);
			mLogBuffer.push_back('\n');
			WriteString(mLogBuffer.c_str());
			if(mCurrentLogLevel <= mConsoleLevelThreshold){
				printf("%s",mLogBuffer.c_str());
			}
			
		}
	}


	void WriteString(cStringRef str)
	{
		if (mFileHandle == 0) return;
		int written = -1;
		uint32_t length = static_cast<uint32_t>(str.size());
		const char* string = str.data();
		while (length > 0 && written != 0)
		{
			written = write(mFileHandle, string, length);
			string += written;
			length -= written;
		}
	}


	raw_ostream& OutStream(){
		if(!mFileStream)
		{
			return nulls();
		}
		return *mFileStream;
	}

	raw_ostream& OutStream(eLogLevel level){
		if (!mFileStream)
		{
			return nulls();
		}
		*mFileStream << getLabel(level);
		return *mFileStream;
	}

	const char* getLabel(eLogLevel);

	Log& NewMessage(eLogLevel level){
		mCurrentLogLevel = level;
		if(mCurrentLogLevel <= mLogLevelThreshold){
			mLogBuffer.clear();
			mLogBuffer.append(getLabel(mCurrentLogLevel));
		}
		return *this;
	}
	
	static Log* GetInstance(){
		if(sInstance == NULL){
			sInstance = new Log();
		}
		return sInstance;
	}
private:
	
	Log(void);
	int mFileHandle;
	std::unique_ptr<raw_ostream> mFileStream;
	std::ostringstream mStream;
	std::string mLogBuffer;
	eLogLevel mLogLevelThreshold;
	eLogLevel mConsoleLevelThreshold;
	eLogLevel mCurrentLogLevel;
	static Log* sInstance;

};
}

