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

#ifndef IOS_INTERFACE_H
#define	IOS_INTERFACE_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/net/message.h"
#include "nel/net/unified_network.h"

//-------------------------------------------------------------------------------------------------
// struct CBackupMsgReceiveFile
//-------------------------------------------------------------------------------------------------

struct CIOSMsgSetPhrase
{
	std::string PhraseName;
	ucstring Txt;

	CIOSMsgSetPhrase(const std::string& phraseName, const ucstring& txt)
	{
		PhraseName= phraseName;
		Txt= txt;
	}

	CIOSMsgSetPhrase(const std::string& phraseName, const std::string& txt)
	{
		PhraseName= phraseName;
		Txt.fromUtf8(txt);
	}

	void send() const
	{
		NLNET::CMessage msg("SET_PHRASE");
		msg.serial(const_cast<CIOSMsgSetPhrase*>(this)->PhraseName);
		ucstring ucTxt= ucstring(PhraseName+"(){[")+Txt+ucstring("]}");
		msg.serial(ucTxt);
		NLNET::CUnifiedNetwork::getInstance()->send("IOS",msg);
	}
};


//-------------------------------------------------------------------------------------------------
#endif
