
//! component="System"

#ifndef header_thread
#define header_thread

class CL_Runnable
//: Thread callback interface.
// When a thread is created, it will call run() in its attached CL_Runnable interface.
{
public:
	virtual void run()=0;
	// Called when a thread is run.
};

class CL_Thread
{
public:
	static CL_Thread *create(CL_Runnable *runnable);
	// Create a thread that uses the CL_Runnable callback interface.
	//!param: runnable - Class to call when thread is started.

	static CL_Thread *create(int (*func)(void*), void* value);
	// Create a thread that calls the function specified, with the value specified.
	//!param: func - Function that gets called at thread start.
	//!param: value - Value passed to the function at thread start.
	
	virtual ~CL_Thread () {;}
	
	virtual void start()=0;
	// Starts the thread.

	virtual void terminate()=0;
	// Terminate the thread. (use with caution under win98)

	virtual void wait()=0;
	// Wait until the thread finishes its execution.
};

#endif
