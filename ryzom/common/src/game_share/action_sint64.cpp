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

#include "action_sint64.h"
#include "pvp_mode.h"
#include "pvp_clan.h"


namespace CLFECOMMON {



/*
 * Init
 */
void CActionSint64::init()
{
	for ( uint i=0; i!=MAX_PROPERTIES_PER_ENTITY; ++i )
	{
		_PropertyToNbBit[i] = 0; // default value
	}
}


/* Register a property to set the number of bits
 * that must be transmitted.
 */
void CActionSint64::registerNumericProperty( TPropIndex propIndex, uint nbbits )
{
	nlassert( (nbbits > 0) && (nbbits <= 64) );
	_PropertyToNbBit[propIndex] = nbbits;
}


/*
 * TEMP
 */
void CActionSint64::registerNumericPropertiesRyzom()
{
	CActionSint64::registerNumericProperty( PROPERTY_ORIENTATION, 32 ); // overridden in fillOutBox
	CActionSint64::registerNumericProperty( PROPERTY_SHEET, 52 ); // 32(sheet) + 20(row)
	CActionSint64::registerNumericProperty( PROPERTY_BEHAVIOUR, 48 );
	CActionSint64::registerNumericProperty( PROPERTY_NAME_STRING_ID, 32 ); // please do not lower it (or tell Olivier, used for forage sources)
	CActionSint64::registerNumericProperty( PROPERTY_TARGET_ID, 8 ); // slot
	CActionSint64::registerNumericProperty( PROPERTY_MODE, 44 );
	CActionSint64::registerNumericProperty( PROPERTY_VPA, 64 );
	CActionSint64::registerNumericProperty( PROPERTY_VPB, 47 );
	CActionSint64::registerNumericProperty( PROPERTY_VPC, 58 );
	CActionSint64::registerNumericProperty( PROPERTY_ENTITY_MOUNTED_ID, 8 ); // slot
	CActionSint64::registerNumericProperty( PROPERTY_RIDER_ENTITY_ID, 8 ); // slot
	CActionSint64::registerNumericProperty( PROPERTY_CONTEXTUAL, 16 );
	CActionSint64::registerNumericProperty( PROPERTY_BARS, 32 ); // please do not lower it (or tell Olivier, used for forage sources)
	CActionSint64::registerNumericProperty( PROPERTY_TARGET_LIST, USER_DEFINED_PROPERTY_NB_BITS );
	CActionSint64::registerNumericProperty( PROPERTY_VISUAL_FX, 11 ); // please do not lower it (or tell Olivier, used for forage sources)
	CActionSint64::registerNumericProperty( PROPERTY_GUILD_SYMBOL, 60 );
	CActionSint64::registerNumericProperty( PROPERTY_GUILD_NAME_ID, 32 );
	CActionSint64::registerNumericProperty( PROPERTY_EVENT_FACTION_ID, 32 );
	CActionSint64::registerNumericProperty( PROPERTY_PVP_MODE, PVP_MODE::NbBits );
	CActionSint64::registerNumericProperty( PROPERTY_PVP_CLAN, 32 );
	CActionSint64::registerNumericProperty( PROPERTY_OWNER_PEOPLE, 3 ); // 4 races and unknow
	CActionSint64::registerNumericProperty( PROPERTY_OUTPOST_INFOS, 16 ); // 15+1
}


/* This function creates initializes its fields using the buffer.
 * \param buffer pointer to the buffer where the data are
 * \size size of the buffer
 */
void	CActionSint64::unpack (NLMISC::CBitMemStream &message)
{
	message.serialAndLog2( _Value, _NbBits );
}


/* This function transform the internal field and transform them into a buffer for the UDP connection.
 * \param buffer pointer to the buffer where the data will be written
 * \size size of the buffer
 */
void	CActionSint64::pack (NLMISC::CBitMemStream &message)
{
	message.serialAndLog2( _Value, _NbBits );
}


/*
 * This functions is used when you want to transform an action into an IStream.
 */
void	CActionSint64::serial (NLMISC::IStream &f)
{
	f.serial( _Value );
}


uint		CActionSint64::_PropertyToNbBit [MAX_PROPERTIES_PER_ENTITY];
}


