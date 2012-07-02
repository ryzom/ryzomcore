// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef NL_TIMEOUT_ASSERTION_THREAD_H
#define NL_TIMEOUT_ASSERTION_THREAD_H

#include "common.h"
#include "thread.h"

/*

  example:
	Encapsulation of message callbacks


  // in init
  CTimeoutAssertionThread *myTAT = new CTimeoutAssertionThread(1000);
  IThread::create(myTAT)->start();

  ...

  // in callback management
  myTAT->activate();
  msg->callback();
  myTAT->disactivate();

  // if the callback call is too slow, an assertion will happen

*/

class CTimeoutAssertionThread: public NLMISC::IRunnable
{
public:
	enum TControl { ACTIVE, INACTIVE, QUIT };

	CTimeoutAssertionThread(uint32 timeout = 0) : _Control(INACTIVE), _Counter(0), _Timeout(timeout)
	{
	}

	void run()
	{
		uint32 lastCounter;
		while(_Control != QUIT)
		{
			if(_Control != ACTIVE || _Timeout == 0)
			{
				//nldebug("not active, sleep");
				NLMISC::nlSleep(1000);
			}
			else
			{
				//nldebug("active, enter sleep");
				lastCounter = _Counter;

				uint32 cummuledSleep = 0;

				// do sleep until cumulated time reached timeout value
				while (cummuledSleep < _Timeout)
				{
					// sleep 1 s
					NLMISC::nlSleep(std::min((uint32)(1000), (uint32)(_Timeout-cummuledSleep)));

					cummuledSleep += 1000;

					// check if exit is required
					if (_Control == QUIT)
						return;
				}
				//nldebug("active, leave sleep, test assert");

				// If this assert occured, it means that a checked part of the code was
				// to slow and then I decided to assert to display the problem.
				nlassert(!(_Control==ACTIVE && _Counter==lastCounter));
			}
		}
	}

	void activate()
	{
		if(_Control == QUIT) return;
		nlassert(_Control == INACTIVE);
		_Counter++;
		_Control = ACTIVE;
		//nldebug("activate");
	}

	void deactivate()
	{
		if(_Control == QUIT) return;
		nlassert(_Control == ACTIVE);
		_Control = INACTIVE;
		//nldebug("deactivate");
	}

	void quit()
	{
		nlassert(_Control != QUIT);
		_Control = QUIT;
		//nldebug("quit");
	}

	void timeout(uint32 to)
	{
		_Timeout = to;
		//nldebug("change timeout to %d", to);
	}

private:
	volatile TControl	_Control;
	volatile uint32		_Counter;
	volatile uint32		_Timeout;	// in millisecond
};


#endif // NL_TIMEOUT_ASSERTION_THREAD_H

/* End of timeout_assertion_thread.h */
