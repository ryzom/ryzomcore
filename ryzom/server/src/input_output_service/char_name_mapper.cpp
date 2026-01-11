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

#include "stdpch.h"
#include "nel/net/module_builder_parts.h"

#include "server_share/char_name_mapper_itf.h"

#include "string_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CNM;

class CCharNameMapper : 
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CCharNameMapperSkel
{


public:

	CCharNameMapper()
	{
		CCharNameMapperSkel::init(this);
	}

	////////////////////////////////////////////////////////////////
	// Virtual overrides from char name mapper
	////////////////////////////////////////////////////////////////
	virtual void mapCharNames(NLNET::IModuleProxy *sender, const std::vector < TCharNameInfo > &charNameInfos)
	{
		// we receive a list of character name, map them in the IOS string table

		vector<TCharMappedInfo> result(charNameInfos.size());

		for (uint i=0; i<charNameInfos.size(); ++i)
		{
			const TCharNameInfo &cni = charNameInfos[i];
			uint32 stringId = SM->storeString(cni.getCharName());

			result[i].setCharEid(cni.getCharEid());
			result[i].setStringId(stringId);
		}

		// return the result to the client
		CCharNameMapperClientProxy cnmc(sender);
		cnmc.charNamesMapped(this, result);
	}


};


NLNET_REGISTER_MODULE_FACTORY(CCharNameMapper, "CharNameMapper");
