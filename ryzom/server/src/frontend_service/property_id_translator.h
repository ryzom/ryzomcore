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



#ifndef NL_PROPERTY_ID_TRANSLATOR_H
#define NL_PROPERTY_ID_TRANSLATOR_H

#include "nel/misc/types_nl.h"
#include "game_share/entity_types.h"
#include "game_share/ryzom_entity_id.h"
#include "vision_array.h"


/**
 * Property Translator.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CPropertyIdTranslator
{
public:

	/// Constructor
	CPropertyIdTranslator()
	{
		uint p;
		for ( p=0; p!=CLFECOMMON::MAX_PROPERTIES_PER_ENTITY; ++p )
		{
			_PropTable[p] = CLFECOMMON::INVALID_PROPERTY;
			_DistThresholdTable[p] = -1;
		}
		_NbTable = 0;
	}

	/// Return the number of properties corresponding to an entity index (<= MAX_PROPERTIES_PER_ENTITY)
	/*CLFECOMMON::TPropIndex	nbProperties()
						{ return _NbTable; }*/

	/// Return the property id corresponding to a (TEntityIndex, TPropIndex)
	/*CLFECOMMON::TProperty	getPropertyId( CLFECOMMON::TPropIndex propindex )
						{ return _PropTable[propindex]; }*/


	/* Return the "delta coefficient" of a discrete property (result is > 0).
	 *
	 * This coefficient was precalculated using the amount of screen difference
	 * that occurs when changing the value of the property ("number of pixels
	 * changed" compared to 2*the entity's surface for a position change)
	 */
	/*CLFECOMMON::TCoord	getDeltaCoefficient( CLFECOMMON::TProperty propertyid )
						{ nlassert( propertyid > CLFECOMMON::LAST_CONTINUOUS_PROPERTY );
						  //return 500; // TEMP
						  return 5000; // high delta because discrete prop is sent only once
						}*/

	/// Return true if the property is a continuous one, false for a discreet one
	bool				isContinuous( const TEntityIndex& /*entityindex*/, CLFECOMMON::TPropIndex propindex )
						{ //return (((1 << propindex) & CFrontEndPropertyReceiver::PropertiesContinuousMask[CFrontEndPropertyReceiver::getEntity(entityindex)->id.Type & 0x7F]) != 0 );
						 return propindex < CLFECOMMON::FIRST_DISCREET_PROPINDEX; }

	// Deprecated
	/*bool				isContinuous( CLFECOMMON::TProperty propertyid )
						{ return propertyid < CLFECOMMON::FIRST_DISCREET_PROPERTY; }*/


	// Does not perform the "&0x7F", do it before calling
	/*CLFECOMMON::TPropIndex		addProperty( CLFECOMMON::TProperty propertyid )
	{
		nlassert( propertyid < CLFECOMMON::MAX_PROPERTIES_PER_ENTITY );

		// Check that the propertyid is unique
		bool found = false;
		sint p;
		for ( p=0; p!=_NbTable; ++p )
		{
			if ( _PropTable[p] == propertyid )
				found = true;
		}
		if ( found )
			return CLFECOMMON::INVALID_PROP_INDEX;

		// Set the propertyid
		CLFECOMMON::TPropIndex propindex = _NbTable++;
		_PropTable[propindex] = propertyid;
		return propindex;
	}*/

	// Set distance threshold
	void						setDistThreshold( CLFECOMMON::TPropIndex propIndex, CLFECOMMON::TCoord dist )
	{
		if ( _DistThresholdTable[propIndex] == -1 ) 
		{
			_DistThresholdTable[propIndex] = dist;
		}
		else if ( _DistThresholdTable[propIndex] != dist )
		{
			nlwarning( "Found two different DistThreshold %d and %d for the same propIndex %hu", _DistThresholdTable[propIndex], dist, propIndex );
		}
	}

	// Get distance threshold
	CLFECOMMON::TCoord			getDistThreshold( CLFECOMMON::TPropIndex propIndex )
	{
		//nlassert( propertyid < CLFECOMMON::NB_PROPERTIES );
		return _DistThresholdTable[propIndex];
	}

private:

	// Property table
	CLFECOMMON::TProperty			_PropTable [CLFECOMMON::MAX_PROPERTIES_PER_ENTITY];

	/// Number of properties
	CLFECOMMON::TPropIndex			_NbTable;

	// Distance threshold
	CLFECOMMON::TCoord				_DistThresholdTable [CLFECOMMON::MAX_PROPERTIES_PER_ENTITY];
};



#endif // NL_PROPERTY_ID_TRANSLATOR_H

/* End of property_id_translator.h */
