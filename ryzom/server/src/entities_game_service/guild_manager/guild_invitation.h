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

#ifndef RY_GUILD_INVITATION_H
#define RY_GUILD_INVITATION_H

#include "../../gameplay_module_lib/gameplay_module_lib.h"

class CGuild;

/**
 * a player invited another one to join its guild
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildInvitation : public IModuleCore
{
	NL_INSTANCE_COUNTER_DECL(CGuildInvitation);
public:
	/// ctor
	inline CGuildInvitation(CGuild* guild, const TDataSetRow & invitor);
	/// get the guild concerned
	inline CGuild * getGuild()const;
	/// get the invitor
	inline const TDataSetRow & getInvitor()const;
private:
	/// guild concerned
	NLMISC::CRefPtr<CGuild>			_Guild;
	/// row of the player who invited
	TDataSetRow						_Invitor;
	
};

//----------------------------------------------------------------------------
inline CGuildInvitation::CGuildInvitation(CGuild* guild, const TDataSetRow & invitor)
:IModuleCore(),_Guild(guild),_Invitor(invitor)
{
	nlassert(guild);
}

//----------------------------------------------------------------------------
inline CGuild * CGuildInvitation::getGuild()const
{
	return _Guild;
}

//----------------------------------------------------------------------------
inline const TDataSetRow & CGuildInvitation::getInvitor()const
{
	return _Invitor;
}
#endif // RY_GUILD_INVITATION_H

/* End of guild_invitation.h */
