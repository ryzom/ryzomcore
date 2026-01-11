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


#ifndef SS_STATE_MANAGER_H
#define SS_STATE_MANAGER_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/net/unified_network.h"


//-------------------------------------------------------------------------------------------------
// class CStateManager
//-------------------------------------------------------------------------------------------------

class CStateManager
{
private:
	// this is a singleton so make ctor private
	CStateManager();

public:
	// singleton accessor
	static CStateManager* getInstance();

public:
	// clear out the list of valid states
	void clearValidStateList();

	// add a new entry to the list of valid states
	void addValidState(const NLMISC::CSString& stateName);

	// clear all active state flags but don't execute end of state scripts
	void resetAll();

	// run the 'begin' script for a named state and mark the state as active
	// note: if the state was already active then does nothing
	bool beginState(const NLMISC::CSString& stateName);

	// run the 'end' script for a named script and mark the steta as inactive
	// note: if the state was not already active then does nothing
	bool endState(const NLMISC::CSString& stateName);

	// run the 'serviceUp' scripts for a named service for all active states
	void serviceUp(const NLMISC::CSString& serviceName,NLNET::TServiceId serviceId);

	// run the 'serviceDown' scripts for a named service for all active states
	void serviceDown(const NLMISC::CSString& serviceName,NLNET::TServiceId serviceId);

	// display the list of active states
	void display() const;

private:
	// private data
	typedef NLMISC::CVectorSString TStates;
	TStates _States;

	typedef std::set<std::string> TValidStates;
	TValidStates _ValidStates;
};


//-------------------------------------------------------------------------------------------------
#endif
