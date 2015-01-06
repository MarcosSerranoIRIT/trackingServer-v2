#pragma once

#include "ofThread.h"

class threadedRGB : public ofThread
{
public:
	threadedRGB(void);
	~threadedRGB(void);

	 void start();
	 void stop();
	 void threadedFunction();
	 ofPoint getSmartWatchPos();

	 protected:
    // This is a simple variable that we aim to always access from both the
    // main thread AND this threaded object.  Therefore, we need to protect it
    // with the mutex.  In the case of simple numerical variables, some
    // garuntee thread safety for small integral types, but for the sake of
    // illustration, we use an int.  This int could represent ANY complex data
    // type that needs to be protected.
    int watch_x;
	int watch_y;
};

