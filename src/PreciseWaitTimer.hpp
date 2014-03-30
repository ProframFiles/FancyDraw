// Originally written by Eugene Manko:
// http://www.codeguru.com/cpp/w-p/system/timers/article.php/c5759/Creating-a-HighPrecision-HighResolution-and-Highly-Reliable-Timer-Utilising-Minimal-CPU-Resources.htm



//----------------------------------------------------------------
class PreciseTimer
{
public:
	PreciseTimer() : mRes(0), toLeave(false), stopCounter(-1)
	{
		InitializeCriticalSection(&crit);
		mRes = timeSetEvent(1, 0, &TimerProc, (DWORD)this,
												TIME_PERIODIC);
	}
	virtual ~PreciseTimer()
	{
		mRes = timeKillEvent(mRes);
		DeleteCriticalSection(&crit);
	}

	///////////////////////////////////////////////////////////////
	// Function name   : Wait
	// Description     : Waits for the required duration of msecs.
	//                 : Timer resolution is precisely 1 msec
	// Return type     : void  :
	// Argument        : int timeout : timeout in msecs
	///////////////////////////////////////////////////////////////
	void Wait(int timeout)
	{
		if(timeout)
		{
			stopCounter = timeout;
			toLeave = true;
			// this will do the actual delay - timer callback shares
			// same crit section
			EnterCriticalSection(&crit);
			LeaveCriticalSection(&crit);
		}
	}
	///////////////////////////////////////////////////////////////
	// Function name   : TimerProc
	// Description     : Timer callback procedure that is called
	//                 : every 1msec
	//                 : by high resolution media timers
	// Return type     : void CALLBACK  :
	// Argument        : UINT uiID :
	// Argument        : UINT uiMsg :
	// Argument        : DWORD dwUser :
	// Argument        : DWORD dw1 :
	// Argument        : DWORD dw2 :
	///////////////////////////////////////////////////////////////
	static void CALLBACK TimerProc(UINT uiID, UINT uiMsg, DWORD
																 dwUser, DWORD dw1, DWORD dw2)
	{
		static volatile bool entered = false;

		PreciseTimer* pThis = (PreciseTimer*)dwUser;
		if(pThis)
		{
			if(!entered && !pThis->toLeave)      // block section as
				// soon as we can
			{
				entered = true;
				EnterCriticalSection(&pThis->crit);
			}
			else if(pThis->toLeave && pThis->stopCounter == 0)
				// leave section
				// when counter
				// has expired
			{
				pThis->toLeave = false;
				entered = false;
				LeaveCriticalSection(&pThis->crit);
			}
			else if(pThis->stopCounter > 0)      // if counter is set
				// to anything, then
				// continue to drop
				// it...
			{
				--pThis->stopCounter;
			}
		}
	}

private:
	MMRESULT         mRes;
	CRITICAL_SECTION crit;
	volatile bool    toLeave;
	volatile int     stopCounter;
};
