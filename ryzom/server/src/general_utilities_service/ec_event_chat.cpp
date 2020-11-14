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

// game share
#include "game_share/utils.h"

// local
#include "gus_module_manager.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;


//-----------------------------------------------------------------------------
// class CEventChat
//-----------------------------------------------------------------------------

class CEventChat: public GUS::IModule
{
public:
	// IModule specialisation implementation
	bool initialiseModule(const NLMISC::CSString& rawArgs);

	NLMISC::CSString getState() const;
	NLMISC::CSString getName() const;
	NLMISC::CSString getParameters() const;
	void displayModule() const;

public:
	// remaining public interface
	CEventChat();

private:
};


//-----------------------------------------------------------------------------
// methods CEventChat
//-----------------------------------------------------------------------------

CEventChat::CEventChat()
{
}

bool CEventChat::initialiseModule(const NLMISC::CSString& rawArgs)
{
	return true;
}

NLMISC::CSString CEventChat::getState() const
{
	return getName()+" "+getParameters();
}

NLMISC::CSString CEventChat::getName() const
{
	return "EC";
}

NLMISC::CSString CEventChat::getParameters() const
{
	return "";
}

void CEventChat::displayModule() const
{
}


//-----------------------------------------------------------------------------
// CEventChat registration
//-----------------------------------------------------------------------------

REGISTER_GUS_MODULE(CEventChat,"EC")

