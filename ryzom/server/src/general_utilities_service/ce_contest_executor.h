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

#ifndef CE_CONTEST_EXECUTOR_H
#define CE_CONTEST_EXECUTOR_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/sstring.h"
#include "gus_client_manager.h"


//-----------------------------------------------------------------------------
// class CContestExecutor
//-----------------------------------------------------------------------------

class CContestExecutor
{
public:
	// get hold of the singleton instance
	static CContestExecutor* getInstance();

public:
	// interface called at contest launch time 
	virtual void beginContest(uint32 ctrlModuleId, const NLMISC::CSString& name)=0;

	// give the contest a localised title
	virtual void setTitle(const NLMISC::CSString& txt)=0;

	// display a chat text
	virtual void addText(const NLMISC::CSString& speakerName,const NLMISC::CSString& txt)=0;

	// add a valid answer
	virtual void addAnswer(const NLMISC::CSString& answer)=0;

	// a player submits an answer
	virtual void submitAnswer(GUS::TClientId clientId, const NLMISC::CSString& characterName, const ucstring& answer)=0;

	// display the list of lucky winners
	virtual void acknowledgeWinners(const std::vector<NLMISC::CSString>& winners)=0;

	// end the current contest
	virtual void endContest()=0;

	// display the state of the contest
	virtual void display()=0;

	// load a set of system messages in a language appropriate for this shard
	virtual bool readChatTextFile(const NLMISC::CSString& fileName)=0;

};


//-----------------------------------------------------------------------------
#endif
