/*
	Singleton test

	project: RYZOM / TEST

*/

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "game_share/singleton_registry.h"

class CSingletonTest: public IServiceSingleton
{
public:
	void init() 
	{
		nlinfo("CSingletonTest::init()");
		serviceCounter=0;
		tickCounter=0;
	}

	void serviceUpdate()
	{
		++serviceCounter;
		if (serviceCounter>=50)
		{
			nlinfo("CSingletonTest::serviceUpdate() @50");
			serviceCounter=0;
		}	
	}

	void tickUpdate()
	{
		++tickCounter;
		if (tickCounter>=25)
		{
			nlinfo("CSingletonTest::tickUpdate() @25");
			tickCounter=0;
		}	
	}

	void release()
	{
		nlinfo("CSingletonTest::release()");
	}

private:
	uint32 tickCounter;
	uint32 serviceCounter;
};

static CSingletonTest SingletonTest;
