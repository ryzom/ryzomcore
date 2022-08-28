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

#ifndef CC_CONTEST_CTRL_SCRIPT_H
#define CC_CONTEST_CTRL_SCRIPT_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/sstring.h"
#include "nel/misc/entity_id.h"


//-----------------------------------------------------------------------------
// class CContestCtrlScript
//-----------------------------------------------------------------------------

class CContestCtrlScript
{
public:
	// get hold of the singleton instance
	static CContestCtrlScript* getInstance();

public:
	// load the script file
	virtual void load(const NLMISC::CSString& fileName)=0;

	// start running the loaded script
	virtual void start()=0;

	// set the delay value for the current 'wait' or 'wait for answer' script line
	virtual void setDelay(uint32 duration)=0;

	// stop running the loaded script
	virtual void stop()=0;

	// list the currently loaded script
	virtual void list()=0;

	// display the current state of the ctrl script singleton
	virtual void display()=0;
};


//-----------------------------------------------------------------------------
#endif
