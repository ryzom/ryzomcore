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

#ifndef GUILD_INTERFACE_H
#define GUILD_INTERFACE_H

#include "nel/misc/entity_id.h"

class CGuild;
namespace EGSPD
{
	class CGuildPD;
}

class IGuild
{
public:

	static IGuild *getGuildInterface(CGuild *guild);
	static IGuild *getGuildInterface(EGSPD::CGuildPD *guildPd);

	uint32			getIdWrap();
	void			setNameWrap(const ucstring &name);
	const ucstring	&getNameWrap();

	bool			isProxyWrap();
	virtual void	removeMember(const NLMISC::CEntityId &id) =0;


	void			updateMembersStringIds();
};

#endif //GUILD_INTERFACE_H

