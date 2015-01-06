#include "threadedRGB.h"


threadedRGB::threadedRGB(void)
{
}


threadedRGB::~threadedRGB(void)
{
}

/// Start the thread.
void threadedRGB::start()
{
	// Mutex blocking is set to true by default
	// It is rare that one would want to use startThread(false).
	startThread();
}

/// Signal the thread to stop.  After calling this method,
/// isThreadRunning() will return false and the while loop will stop
/// next time it has the chance to.
void threadedRGB::stop()
{
	stopThread();
}

/// Our implementation of threadedFunction.
void threadedRGB::threadedFunction()
{
	while(isThreadRunning())
	{
		// Attempt to lock the mutex.  If blocking is turned on,
		if(lock())
		{



			// Unlock the mutex.  This is only
			// called if lock() returned true above.
			unlock();

			// Sleep for 1 second.
			sleep(1000);
		}
		else
		{
			// If we reach this else statement, it means that we could not
			// lock our mutex, and so we do not need to call unlock().
			// Calling unlock without locking will lead to problems.
			//ofLogWarning("threadedFunction()") << "Unable to lock mutex.";

		}
	}
}



// Use ofScopedLock to protect a copy of count while getting a copy.
ofPoint threadedRGB::getSmartWatchPos()
{
	ofScopedLock lock(mutex);
	return ofPoint(watch_x, watch_y);
}
