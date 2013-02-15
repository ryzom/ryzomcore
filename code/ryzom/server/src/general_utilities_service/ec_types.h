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

#ifndef EC_TYPES_H
#define EC_TYPES_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"


//-----------------------------------------------------------------------------
// NLMISC namespace
//-----------------------------------------------------------------------------

namespace NLMISC
{
	//-----------------------------------------------------------------------------
	// advanced class declarations
	//-----------------------------------------------------------------------------

	class CSString;
}


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// advanced class declarations
	//-----------------------------------------------------------------------------

	class CChatChannel;
	typedef NLMISC::CSmartPtr<CChatChannel> TChatChannelPtr;
}


//-----------------------------------------------------------------------------
// EC namespace
//-----------------------------------------------------------------------------

namespace EC
{
	//-----------------------------------------------------------------------------
	// advanced class declarations
	//-----------------------------------------------------------------------------

	class CEventChatModule;
	typedef NLMISC::CSmartPtr<CEventChatModule>	TEventChatModulePtr;

	class IChannel;
	class CCtrlChannel;
	class CFactionChannel;
	class CPartyChannel;
	typedef NLMISC::CSmartPtr<CCtrlChannel>		TCtrlChannelPtr;
	typedef NLMISC::CSmartPtr<CFactionChannel>	TFactionChannelPtr;
	typedef NLMISC::CSmartPtr<CPartyChannel>		TPartyChannelPtr;
}


//-----------------------------------------------------------------------------
#endif
