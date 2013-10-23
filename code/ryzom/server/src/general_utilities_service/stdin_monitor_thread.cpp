// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/common.h"
#include "nel/misc/thread.h"
#include "nel/misc/command.h"
#include "game_share/singleton_registry.h"
#include <string>


//-----------------------------------------------------------------------------
// class CStdinMonitorThread
//-----------------------------------------------------------------------------

class CStdinMonitorThread : public NLMISC::IRunnable
{
public:
	// ctor
	CStdinMonitorThread();

	// main routine executed when the thread is started
	void run();

	// interface for adding commands, retrieving commands and verifying whether there are commands waiting
	void pushCommand(std::string nextCommand);
	std::string popCommand();
	bool commandWaiting() const;

private:
	std::string _NextCommand;
	volatile bool _CommandWaiting;

};

//-----------------------------------------------------------------------------
// methods CStdinMonitorThread
//-----------------------------------------------------------------------------

CStdinMonitorThread::CStdinMonitorThread()
{
	_CommandWaiting= false;
}

void CStdinMonitorThread::run ()
{
	while(!feof(stdin))
	{
		// wait for the main thread to deal with the previous command
		while (commandWaiting())
		{
			NLMISC::nlSleep(1);
		}

		// get the next command from the command line
		std::string theCommand;
		theCommand.resize(1024,0);
		fgets((char*)theCommand.c_str(),theCommand.size()-1,stdin);
		theCommand.resize(strlen(theCommand.c_str()));

		// push the command to allow reader thread to deal with it
		pushCommand(theCommand);
	}
}

void CStdinMonitorThread::pushCommand(std::string nextCommand)
{
	// wait for the previous command to be treated
	while (_CommandWaiting)
	{
		NLMISC::nlSleep(1);
	}

	// copy the next command into the appropriate buffer
	_NextCommand= nextCommand;

	// set the _CommandWaiting flag, to allow reader thread to treat the new command
	_CommandWaiting= true;
}

std::string CStdinMonitorThread::popCommand()
{
	// wait for a command to be ligned up (waiting)
	while (!_CommandWaiting)
	{
	}

	// copy out the next command
	std::string result= _NextCommand;

	// clear out the next command variable ready for its next use
	_NextCommand.clear();

	// clear the _CommandWaiting flag, to allow writer thread to add a new command
	_CommandWaiting= false;

	// all done so return the command
	return result;
}

bool CStdinMonitorThread::commandWaiting() const
{
	return _CommandWaiting;
}


//-----------------------------------------------------------------------------
// class CStdinMonitorSingleton
//-----------------------------------------------------------------------------

class CStdinMonitorSingleton: public IServiceSingleton
{
public:
	void init();
	void serviceUpdate();
	void release();

private:
	CStdinMonitorThread* _StdinMonitorThreadInstance;
	NLMISC::IThread* _StdinMonitorThreadHandle;
};

CStdinMonitorSingleton StdinMonitorSingleton;


//-----------------------------------------------------------------------------
// methods CStdinMonitorSingleton
//-----------------------------------------------------------------------------

void CStdinMonitorSingleton::init()
{
	_StdinMonitorThreadInstance= new CStdinMonitorThread;
	_StdinMonitorThreadHandle = NLMISC::IThread::create (_StdinMonitorThreadInstance);
	_StdinMonitorThreadHandle->start();
}

void CStdinMonitorSingleton::serviceUpdate()
{
	if (_StdinMonitorThreadInstance->commandWaiting())
	{
		std::string nextCommand= _StdinMonitorThreadInstance->popCommand();
		NLMISC::ICommand::execute(nextCommand,*NLMISC::InfoLog);
	}
}

void CStdinMonitorSingleton::release()
{
	_StdinMonitorThreadHandle->terminate();
}

