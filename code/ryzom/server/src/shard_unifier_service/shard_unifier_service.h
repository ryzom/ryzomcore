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


#ifndef SHARD_UNIFIER_SERVICE_H
#define SHARD_UNIFIER_SERVICE_H

#include "nel/misc/sstring.h"
#include "nel/net/service.h"

extern NLMISC::CVariable<uint32>	TotalConcurentUser;

class CShardUnifier : public NLNET::IService
{

	void init();

	bool update();

	void release();

	std::string		getServiceStatusString() const
	{
		// add the TotalConcurentPlayer counter to the status string
		NLMISC::CSString ret;
		ret << "TotalConcurentUser="<<TotalConcurentUser.get();

		return ret;
	}

};


#endif //SHARD_UNIFIER_SERVICE_H

