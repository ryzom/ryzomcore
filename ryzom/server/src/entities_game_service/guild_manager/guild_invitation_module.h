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

#ifndef RY_GUILD_INVITATION_MODULE_H
#define RY_GUILD_INVITATION_MODULE_H

#include "../../gameplay_module_lib/gameplay_module_lib.h"
#include "guild_invitation.h"
#include "guild_manager/guild_char_proxy.h"

/**
 * module implementing guild invitation
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildInvitationModule : public IModule
{
	NL_INSTANCE_COUNTER_DECL(CGuildInvitationModule);
public:

	/// ctor
	CGuildInvitationModule( CGuildCharProxy& proxy, CGuildInvitation* invitation )
		:IModule(&(proxy.getModuleParent()),invitation),_Invitation(invitation)
	{
		nlassert( invitation );
	}
	/// user refuses an invitation
	void refuse();
	/// user accepts an invitation
	void accept();
private:
	/// handler called on parent ( player destruction )
	virtual void onParentDestructionHandler();
	/// "real" invitation object
	CGuildInvitation * _Invitation;
};


#endif // RY_GUILD_INVITATION_MODULE_H

/* End of guild_invitation_module.h */
