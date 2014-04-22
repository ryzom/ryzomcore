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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"

#include "game_share/r2_ligo_config.h"
#include "nel/misc/sstring.h"
namespace R2
{
	uint32 CR2LigoConfig::getFileStaticAliasMapping(const std::string &fileName) const
	{
		uint32 filesize = 8;// strlen("r2.0000.");
		if ( fileName.size() > filesize &&
			fileName.substr(0, 3) == "r2.")
		{
			NLMISC::CSString instanceNumber(fileName.substr(3,4));
			uint32 instance = instanceNumber.atoui();
			TScenarioType type;

			if (fileName[8] == 'a') { type = Act;}
			else type = Base;
			return getStaticAliasMapping(instance, type);
		}
		return this->CLigoConfig::getFileStaticAliasMapping(fileName);
	}


	uint32 CR2LigoConfig::getStaticAliasMapping(uint32 aiInstance, TScenarioType type) const
	{
		uint32 instance = aiInstance * 2;
		if (type == Act) { instance +=1; }
		return (1 << (getStaticAliasSize() - 1) ) + instance;
	}
}

