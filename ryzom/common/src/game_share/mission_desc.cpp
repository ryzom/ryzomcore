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
#include "mission_desc.h"
#include "nel/misc/string_conversion.h"


namespace MISSION_DESC
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TStepType)
		NL_STRING_CONVERSION_TABLE_ENTRY( KillCreature )
		NL_STRING_CONVERSION_TABLE_ENTRY( Talk )
		NL_STRING_CONVERSION_TABLE_ENTRY( Target )
		NL_STRING_CONVERSION_TABLE_ENTRY( SkillProgress )
		NL_STRING_CONVERSION_TABLE_ENTRY( Craft )
		NL_STRING_CONVERSION_TABLE_ENTRY( Harvest )
		NL_STRING_CONVERSION_TABLE_ENTRY( BuyItem )
		NL_STRING_CONVERSION_TABLE_ENTRY( SellItem )
		NL_STRING_CONVERSION_TABLE_ENTRY( KillNpc )
		NL_STRING_CONVERSION_TABLE_ENTRY( GiveItem )
		NL_STRING_CONVERSION_TABLE_ENTRY( NbStepTypes )
	NL_END_STRING_CONVERSION_TABLE(TStepType, StepTypeConversion, NbStepTypes)

	NL_BEGIN_STRING_CONVERSION_TABLE (TRewardType)
		NL_STRING_CONVERSION_TABLE_ENTRY( Seeds )
		NL_STRING_CONVERSION_TABLE_ENTRY( Sp )
		NL_STRING_CONVERSION_TABLE_ENTRY( ZCBuilding )
		NL_STRING_CONVERSION_TABLE_ENTRY( NbReward )
	NL_END_STRING_CONVERSION_TABLE(TRewardType, RewardTypeConversion, NbReward)

	const std::string & toString(TStepType type)
	{
		return StepTypeConversion.toString( type );
	}
	TStepType toStepType( const std::string & str )
	{
		return StepTypeConversion.fromString( str );
	}

	const std::string & toString(TRewardType type)
	{
		return RewardTypeConversion.toString( type );
	}
	TRewardType toRewardType( const std::string & str )
	{
		return RewardTypeConversion.fromString( str );
	}


	TClientMissionType	getClientMissionType(TIconId iconId)
	{
		nlctassert(sizeof(IconToClientMissionType)/sizeof(IconToClientMissionType[0]) == NumIcons);
		if(iconId<0 || iconId>=NumIcons)
			return Mission;
		else
			return IconToClientMissionType[iconId];
	}

}

