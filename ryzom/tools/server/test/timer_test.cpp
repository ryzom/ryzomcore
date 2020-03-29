/* 
	Timer test

	project: RYZOM / TEST
*/

#include "game_share/timer.h"

class CTimerTest: public IServiceSingleton
{
public:
	class CTimerTestTimerEvent:public CTimerEvent
	{
	public:
		void timerCallback(CTimer* owner)
		{
			nlinfo("tick!");
			// repeat every 20 ticks
			owner->setRemaining(20,this);
		}
	};

	void init()
	{
		_Timer.setRemaining(20,new CTimerTestTimerEvent);
	}

private:
	CTimer _Timer;
};

static CTimerTest Test;
