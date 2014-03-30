
#include "SDL.h"
#include "FancyDraw.hpp"
#include "akjLog.hpp"
#include "FatalError.hpp"
#include "akjExceptional.hpp"



namespace akj{
	class cLoggingErrorHandler :public FatalErrorHandler
	{
	public:
		cLoggingErrorHandler();
		virtual void ReportAndDie(cStringRef error_msg);
	private:
		enum { kBufferSize = 1024 };
		unsigned char mStringBuffer[kBufferSize];
	};
}


// as long as we define _UNICODE on windows, SDL should automatically 
// change argv into utf8 encoded text (rather than UTF16 narrowed into 
// god-knows-what character encoding is the default)

int main(int argc, char *argv[])
{
	//install a logging error handler
	akj::FatalError::InstallErrorHandler<akj::cLoggingErrorHandler>();


	// load a config file + any startup arguments here
	akj::Log::Info("Application Arguments:");
	for (int i = 0; i < argc; ++i)
	{
		akj::Log::Info("arg #%d = \"%s\"", i, argv[i]);
	}
	akj::Log::Info("Initializing application");
	try 
	{
		akj::cStopWatch sw;
		akj::cFancyDrawApp my_application( 1220, 660);
		sw.Stop();
		akj::Log::Info("...Initialization took % seconds", sw.Read());

		akj::Log::Info("Running Application");
		//Will not return until the application is finished
		my_application.Run();
		//my_application.RectangleBenchMark();
		akj::Log::Info("Ending Application");
	}
	catch(const akj::Exception& e)
	{
		akj::Log::Error("terminating with uncaught akj::Exception:" 
										"\n\t\t%s", e.what() );
	}
	catch(const std::exception& e)
	{
		akj::Log::Error("terminating with uncaught std::exception:" 
										"\n\t\t%s", e.what() );
	}
	
	
	// at this point we're actually returning to the SDL version of main
	// where it will do whatever other system-dependent magic it needs to do
	return 0;
}


void akj::cLoggingErrorHandler::ReportAndDie(cStringRef error_msg)
{
	if (error_msg.size() >= kBufferSize)
	{
		error_msg = error_msg.drop_back(error_msg.size() - (kBufferSize));
	}
	memcpy(mStringBuffer, error_msg.data(), error_msg.size());
	Log::Error("Fatal Error, shutting down:\n\t%s", mStringBuffer);
	exit(1);
}

akj::cLoggingErrorHandler::cLoggingErrorHandler()
{
	memset(mStringBuffer, 0, kBufferSize);
}
