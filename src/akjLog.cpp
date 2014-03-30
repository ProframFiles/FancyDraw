#include "akjLog.hpp"
#include <stdexcept>
namespace akj{

Log* Log::sInstance = NULL;

Log::~Log(void)
{
	if(mFileHandle){
		mFileStream.reset();
		::close(mFileHandle);
	}
}

Log::Log( void )
	:mLogLevelThreshold(LOG_LEVEL_DEBUG)
	,mConsoleLevelThreshold(LOG_LEVEL_DEBUG)
	,mCurrentLogLevel(LOG_LEVEL_OFF)
	,mFileHandle(NULL)
{

	if (!sys::fs::openFileForWrite("log.txt", mFileHandle, sys::fs::F_Binary)) 
	{
		mFileStream.reset(new raw_fd_ostream(mFileHandle,false));
	}
	mLogBuffer.reserve(4096);
	mStream.str(mLogBuffer);
}

const char* Log::getLabel( eLogLevel level)
{
	static const char* labels[LOG_LEVEL_NUMLEVELS] = 
		{"OFF:      "
		,"CRITICAL: "
		,"ERROR: ## "
		,"WARNING:  "
		,"INFO:     "
		,"DEBUG:    "
		,"TMI:      "
	};
	return labels[level];
}

}
