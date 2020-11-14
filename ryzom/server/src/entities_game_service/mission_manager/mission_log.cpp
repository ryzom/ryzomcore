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



#include "stdpch.h"
#include "mission_log.h"
#include "nel/misc/displayer.h"

using namespace std;
using namespace NLMISC;

CMissionLog MissionLog;
//bool VerboseMissions = false;

//-----------------------------------------------
// CMissionLog init
//-----------------------------------------------
void CMissionLog::init(const std::string & logFile)
{
	_LogFile = logFile;
	if ( logFile.empty() )
		Log = NLMISC::WarningLog;
	else
	{
		Log = new NLMISC::CLog;
		_Fd.setParam ( logFile, true);
		Log->addDisplayer (&_Fd);
	}
}// CMissionLog init

//-----------------------------------------------
// CMissionLog release
//-----------------------------------------------
void CMissionLog::release()
{
	if ( Log != NLMISC::WarningLog )
		delete Log;
}// CMissionLog release

//-----------------------------------------------
//-----------------------------------------------
void CMissionLog::display(const char *format, ...)
{
	if (Log == NULL) return;

	char *str;
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);
	
	string toDisp = str;
	toDisp = string(IDisplayer::dateToHumanString()) + string(" ") + toDisp;

	Log->displayRawNL(toDisp.c_str());
}
