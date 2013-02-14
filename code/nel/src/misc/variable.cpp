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

#include "stdmisc.h"

#include "nel/misc/variable.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {


void cbVarChanged (CConfigFile::CVar &cvar)
{
	CCommandRegistry &cr = CCommandRegistry::getInstance();
	for (CCommandRegistry::TCommand::iterator comm = cr._Commands.begin(); comm != cr._Commands.end(); comm++)
	{
		if (comm->second->Type == ICommand::Variable && comm->second->getName() == cvar.Name)
		{
			IVariable *var = static_cast<IVariable*>(comm->second);
			string val = cvar.asString();
			nlinfo ("VAR: Setting variable '%s' with value '%s' from config file", cvar.Name.c_str(), val.c_str());
			var->fromString(val, true);
		}
	}
}


void IVariable::init (NLMISC::CConfigFile &configFile)
{
	CCommandRegistry::getInstance().initVariables(configFile);
}

void CCommandRegistry::initVariables(NLMISC::CConfigFile &configFile)
{
	for (TCommand::iterator comm = _Commands.begin(); comm != _Commands.end(); comm++)
	{
		if (comm->second->Type == ICommand::Variable)
		{
			IVariable *var = static_cast<IVariable *>(comm->second);
			if (var->_UseConfigFile)
			{
				configFile.setCallback(var->_CommandName, cbVarChanged);
				CConfigFile::CVar *cvar = configFile.getVarPtr(var->_CommandName);
				if (cvar != 0)
				{
					string val = cvar->asString();
					//nldebug("VAR: Setting variable '%s' with value '%s' from config file '%s'", var->_CommandName.c_str(), val.c_str(), configFile.getFilename().c_str());
					var->fromString(val, true);
				}
				else
				{
					//nldebug("VAR: No variable '%s' in config file '%s'", var->_CommandName.c_str(), configFile.getFilename().c_str());
				}
			}
		}
	}
}

} // NLMISC
