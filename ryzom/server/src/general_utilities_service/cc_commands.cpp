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

//nel
#include "nel/misc/command.h"

// game share
#include "game_share/utils.h"

// local
#include "cc_contest_ctrl_script.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// CContestCtrlScript Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(ContestCtrl,ccAdvance,"adavnce the clock to the end of the current delay","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CContestCtrlScript::getInstance()->setDelay(0);

	return true;
}

NLMISC_CATEGORISED_COMMAND(ContestCtrl,ccDisplayState,"display the state of the contest control module","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CContestCtrlScript::getInstance()->display();

	return true;
}

NLMISC_CATEGORISED_COMMAND(ContestCtrl,ccLoad,"load a contest control script","<filename>")
{
	CNLSmartLogOverride logOverride(&log);

	CSString ccsFileName;
	switch (args.size())
	{
	case 0:
		ccsFileName.readFromFile("gus_cc_last_ccs.txt");
		break;

	case 1:
		ccsFileName= args[0];
		ccsFileName.writeToFile("gus_cc_last_ccs.txt");
		break;

	default:
		return false;
	}

	CContestCtrlScript::getInstance()->load(ccsFileName);

	return true;
}

NLMISC_CATEGORISED_COMMAND(ContestCtrl,ccQuit,"stop running the current contest control script","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CContestCtrlScript::getInstance()->stop();

	return true;
}

NLMISC_CATEGORISED_COMMAND(ContestCtrl,ccRun,"start running the current contest control script","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CContestCtrlScript::getInstance()->start();

	return true;
}

NLMISC_CATEGORISED_COMMAND(ContestCtrl,ccShow,"show the current contest control script","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CContestCtrlScript::getInstance()->list();

	return true;
}

NLMISC_CATEGORISED_COMMAND(ContestCtrl,ccWait,"modify duration of current pause","<time to wait in seconds>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	sint32 time= atoi(args[0].c_str());
	if (time<=0)
		return false;

	CContestCtrlScript::getInstance()->setDelay(time*1000);

	return true;
}


//-----------------------------------------------------------------------------
