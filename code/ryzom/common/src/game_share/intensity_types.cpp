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

#include "nel/misc/debug.h"
#include "intensity_types.h"

using namespace std;

namespace INTENSITY_TYPE
{
	static map< string, TAttackIntensity > StringToAttackIntensity;
	static map< TAttackIntensity, string > AttackIntensityToString;

	static map< string, TImpactIntensity > StringToImpactIntensity;
	static map< TImpactIntensity, string > ImpactIntensityToString;

	static bool mapInitialized = false;
	const static string unknown_attack = "UNDEFINED_ATTACK";
	const static string unknown_impact = "IMPACT_UNDEFINED";


	//-----------------------------------------------
	// initMap :
	//-----------------------------------------------
	void initMap()
	{
		mapInitialized = true;

		StringToAttackIntensity.insert( make_pair( string("WEAK"), WEAK) );
		AttackIntensityToString.insert( make_pair( WEAK, string("WEAK") ) );

		StringToAttackIntensity.insert( make_pair( string("STANDARD"), STANDARD) );
		AttackIntensityToString.insert( make_pair( STANDARD, string("STANDARD") ) );

		StringToAttackIntensity.insert( make_pair( string("STRONG"), STRONG) );
		AttackIntensityToString.insert( make_pair( STRONG, string("STRONG") ) );

		StringToAttackIntensity.insert( make_pair( string("CRITICAL_HIT"), CRITICAL_HIT) );
		AttackIntensityToString.insert( make_pair( CRITICAL_HIT, string("CRITICAL_HIT") ) );

		StringToAttackIntensity.insert( make_pair( string("WEAK_COMBO"), WEAK_COMBO) );
		AttackIntensityToString.insert( make_pair( WEAK_COMBO, string("WEAK_COMBO") ) );

		StringToAttackIntensity.insert( make_pair( string("STRONG_COMBO"), STRONG_COMBO) );
		AttackIntensityToString.insert( make_pair( STRONG_COMBO, string("STRONG_COMBO") ) );
		////////////////////////////////////////////////////////////////////////////////////
		StringToImpactIntensity.insert( make_pair( string("IMPACT_INSIGNIFICANT"), IMPACT_INSIGNIFICANT) );
		ImpactIntensityToString.insert( make_pair( IMPACT_INSIGNIFICANT, string("IMPACT_INSIGNIFICANT") ) );

		StringToImpactIntensity.insert( make_pair( string("IMPACT_VERY_WEAK"), IMPACT_VERY_WEAK) );
		ImpactIntensityToString.insert( make_pair( IMPACT_VERY_WEAK, string("IMPACT_VERY_WEAK") ) );

		StringToImpactIntensity.insert( make_pair( string("IMPACT_WEAK"), IMPACT_WEAK) );
		ImpactIntensityToString.insert( make_pair( IMPACT_WEAK, string("IMPACT_WEAK") ) );

		StringToImpactIntensity.insert( make_pair( string("IMPACT_AVERAGE"), IMPACT_AVERAGE) );
		ImpactIntensityToString.insert( make_pair( IMPACT_AVERAGE, string("IMPACT_AVERAGE") ) );

		StringToImpactIntensity.insert( make_pair( string("IMPACT_STRONG"), IMPACT_STRONG) );
		ImpactIntensityToString.insert( make_pair( IMPACT_STRONG, string("IMPACT_STRONG") ) );

		/*StringToImpactIntensity.insert( make_pair( string("IMPACT_VERY_STRONG"), IMPACT_VERY_STRONG) );
		ImpactIntensityToString.insert( make_pair( IMPACT_VERY_STRONG, string("IMPACT_VERY_STRONG") ) );

		StringToImpactIntensity.insert( make_pair( string("IMPACT_HUGE"), IMPACT_HUGE) );
		ImpactIntensityToString.insert( make_pair( IMPACT_HUGE, string("IMPACT_HUGE") ) );


		StringToImpactIntensity.insert( make_pair( string("IMPACT_NONE"), IMPACT_NONE) );
		ImpactIntensityToString.insert( make_pair( IMPACT_NONE, string("IMPACT_NONE") ) );
		*/
	} // initMap //


	//-----------------------------------------------
	// stringToAttackIntensity :
	//-----------------------------------------------
	TAttackIntensity stringToAttackIntensity(const string &str)
	{
		if ( !mapInitialized)
			initMap();

		map< string, TAttackIntensity >::const_iterator it = StringToAttackIntensity.find( str );
		if (it != StringToAttackIntensity.end() )
		{
			return (*it).second;
		}
		else
		{
			return UNDEFINED_ATTACK;
		}
	} // stringToAttackIntensity //


	//-----------------------------------------------
	// attackIntensityToString :
	//-----------------------------------------------
	const string & attackIntensityToString(TAttackIntensity type)
	{
		if ( !mapInitialized)
			initMap();

		map< TAttackIntensity, string >::const_iterator it = AttackIntensityToString.find( type );
		if (it != AttackIntensityToString.end() )
		{
			return (*it).second;
		}
		else
		{
			return unknown_impact;
		}
	} // attackIntensityToString //

/*******/

	//-----------------------------------------------
	// stringToImpactIntensity :
	//-----------------------------------------------
	TImpactIntensity stringToImpactIntensity(const string &str)
	{
		if ( !mapInitialized)
			initMap();

		map< string, TImpactIntensity >::const_iterator it = StringToImpactIntensity.find( str );
		if (it != StringToImpactIntensity.end() )
		{
			return (*it).second;
		}
		else
		{
			return IMPACT_UNDEFINED;
		}
	} // stringToImpactIntensity //


	//-----------------------------------------------
	// impactIntensityToString :
	//-----------------------------------------------
	const string & impactIntensityToString(TImpactIntensity type)
	{
		if ( !mapInitialized)
			initMap();

		map< TImpactIntensity, string >::const_iterator it = ImpactIntensityToString.find( type );
		if (it != ImpactIntensityToString.end() )
		{
			return (*it).second;
		}
		else
		{
			return unknown_impact;
		}
	} // impactIntensityToString //

}; // INTENSITY_TYPE
