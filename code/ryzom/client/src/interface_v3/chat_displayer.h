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



#ifndef NL_CHAT_DISPLAYER_H
#define NL_CHAT_DISPLAYER_H

#include "nel/misc/displayer.h"
#include "nel/gui/group_list.h"
#include "interface_manager.h"

#include "nel/misc/mutex.h"

// to fix a conflict with syslog.h being included by libwww
#ifdef LOG_WARNING
#undef LOG_WARNING
#endif


/**
 * class used to display console text commands in the chat window
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class CChatDisplayer : public NLMISC::IDisplayer
{
public:

	struct SDispString
	{
		std::string Str;
		CInterfaceManager::TSystemInfoMode Mode;
	};

	// To make it thread safe
	NLMISC::CSynchronized< std::vector<SDispString> > StringToDisplay;

	/// Constructor
	CChatDisplayer() :
		IDisplayer( "ChatDisplayer" ),
		StringToDisplay("StringToDisplay")
	{
	}

	/// Display the string to the chat window
	virtual void doDisplay ( const NLMISC::CLog::TDisplayInfo& args, const char *message )
	{
		std::string temp = message;
		std::string str;
		CInterfaceManager::TSystemInfoMode mode;
		if (args.LogType == NLMISC::CLog::LOG_ERROR)
		{
			str = "ERR: ";
			mode = CInterfaceManager::ErrorMsg;
		}
		else if (args.LogType == NLMISC::CLog::LOG_WARNING)
		{
			str = "WRN: ";
			mode = CInterfaceManager::WarningMsg;
		}
		else
		{
			str = "";
			mode = CInterfaceManager::InfoMsg;
		}

		str += temp.substr(0, temp.size()-1);

		{ // create a new scope for the access
			// get an access to the value
			NLMISC::CSynchronized<std::vector<SDispString> >::CAccessor acces(&StringToDisplay);
			// now, you have a thread safe access until the end of the scope, so you can do whatever you want. for example, change the value
			SDispString toAdd;
			toAdd.Str = str;
			toAdd.Mode = mode;
			acces.value ().push_back(toAdd);
		} // end of the access
 	}


	// Called each frames
	void update ()
	{
		NLMISC::CSynchronized<std::vector<SDispString> >::CAccessor acces(&StringToDisplay);
		std::vector<SDispString> &rVal = acces.value ();
		for (uint i = 0; i < rVal.size(); ++i)
		{
			CInterfaceManager::getInstance()->displayDebugInfo(ucstring(rVal[i].Str), rVal[i].Mode);
		}
		rVal.clear();
	}
};


#endif // NL_CHAT_DISPLAYER_H

/* End of chat_displayer.h */
