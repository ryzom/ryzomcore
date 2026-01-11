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


#ifndef MODULE_SECURITY_H
#define MODULE_SECURITY_H

#include "nel/misc/entity_id.h"
#include "nel/net/module.h"

enum TRyzomModuleSecurityTag
{
	rmst_client_info,
};


struct TClientInfo : public NLNET::TSecurityData
{
	TClientInfo(const NLNET::TSecurityData::TCtorParam &param)
		: TSecurityData(param)
	{
	}

	uint32				UserId;
	NLMISC::CEntityId	ClientEid;
	std::string			UserPriv;
	std::string			ExtendedPriv;

	void serial(NLMISC::CMemStream &s)
	{
		s.serial(UserId);
		s.serial(ClientEid);
		s.serial(UserPriv);
		s.serial(ExtendedPriv);
	}

};

NLMISC_REGISTER_OBJECT(NLNET::TSecurityData, TClientInfo, uint8, rmst_client_info);

#endif // MODULE_SECURITY_H
