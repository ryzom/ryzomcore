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

#ifndef CL_LOGIN_PROGRESS_POST_THREAD_H
#define CL_LOGIN_PROGRESS_POST_THREAD_H

#include "nel/misc/thread.h"
#include "nel/misc/singleton.h"
#include "game_share/http_client.h"


namespace NLMISC
{
	class CConfigFile;
}

// A step in the login process, to be posted
class CLoginStep
{
public:
	uint		Step;
	std::string PostString;
	std::string*	AsyncRet;
	bool*		AsyncSent;
public:
	// if async is not null the return of the sendMsg will be in *async then *AsyncSent will be passed to true;
	// You add to check via pulling if data have been send (maximum time 1 second)
	CLoginStep(uint step = 0, const std::string &postString = "", std::string* asyncRet = 0, bool* asyncSent=0)
		: Step(step), PostString(postString), AsyncRet(asyncRet), AsyncSent(asyncSent) {}
};


/** Notify the http logon server from the various step that have been reached in the client.
  * Php scripts on server side use these information to collect statistics such as :
  * - how many player manage to do a successful video driver initialization ?
  * - how many player did pass the login screen ?
  * - what is the mean loading time from character selection until in game ?
  * - etc.
  * The sending is done in a separate thread to avoid freeze of the client (especially at launch
  * -> if the server is unavailable, the user will at least be able to reach the login screen
  * without having to wait 10 seconds or so)
  */
class CLoginProgressPostThread : public NLMISC::CSingleton<CLoginProgressPostThread>
{
public:
	CLoginProgressPostThread();
	~CLoginProgressPostThread();
	void init(const std::string &startupHost,
			  const std::string &startupPage);
	// Init from a config file (Using the InstallStatsUrl variable)
	void init(NLMISC::CConfigFile &configFile);
	//
	void release();
	// Mark a new step in the login. Only newer step are posted to the server
	void step(const CLoginStep &ls);
	// Send the msg (wait until the message is send) return the answer string
	std::string forceStep(const CLoginStep &ls);
private:
	NLMISC::IThread	  *_Thread;
	NLMISC::IRunnable *_Task;
};


// some values for the login step
enum
{
	LoginStep_Stop					  = -1,
	LoginStep_Unknown				  = 0,
	InstallStep_StartDownload		  = 100,
	InstallStep_UpdateDownload		  = 200,
	InstallStep_StopDownload		  = 300,
	InstallStep_StartInstall		  = 400,
	InstallStep_UpdateInstall		  = 500,
	InstallStep_StopInstall			  = 600,
	LoginStep_VideoModeSetup		  = 1000,
	LoginStep_VideoModeSetupHighColor = 2000,
	LoginStep_LoginScreen		      = 3000,
	LoginStep_PostLogin			      = 4000,
	// following are not really part of the login, but are useful information as well (TODO : find a better name for the class ...)
	LoginStep_CharacterSelection      = 5000,
	LoginStep_InGameEntry		      = 6000,
	LoginStep_GameExit				  = 7000
};




#endif
