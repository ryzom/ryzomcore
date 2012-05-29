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
// Application
#include "pact_sheet.h"
// Georges
#include "nel/georges/u_form_elm.h"

using namespace NLGEORGES;

//=================================================================================
void CPactSheet::build(const NLGEORGES::UFormElm &root)
{
	Type = PACT;
	const UFormElm *arrayDeathImpact = NULL;
	if( root.getNodeByName( &arrayDeathImpact, "death_impact" ) )
	{
		if( arrayDeathImpact )
		{
			uint size;
			nlverify( arrayDeathImpact->getArraySize(size) );
			PactLose.resize( size );

			const UFormElm *node;

			// variable is used for calculate pact effect in differential between pacts type
			sint16 LoseHitPoints = 0;
			sint16 LoseStamina = 0;
			sint16 LoseSap = 0;
			sint16 LoseSkills = 0;

			sint16 value;

			for( uint i = 0; i < size; ++i )
			{
				node = NULL;
				arrayDeathImpact->getArrayNode( &node, i );

				if( node )
				{
					node->getValueByName( value, "HitPoints" );
					value = 0 - value - LoseHitPoints;
					LoseHitPoints += value;
					PactLose[ i ].LoseHitPointsLevel = value;

					node->getValueByName( value, "Stamina" );
					value = 0 - value - LoseStamina;
					LoseStamina += value;
					PactLose[ i ].LoseStaminaLevel = value;

					node->getValueByName( value, "Sap" );
					value = 0 - value - LoseSap;
					LoseSap += value;
					PactLose[ i ].LoseSapLevel = value;

					node->getValueByName( value, "Skills" );
					value = 0 - value - LoseSkills;
					LoseSkills += value;
					PactLose[ i ].LoseSkillsLevel = value;

					node->getValueByName( PactLose[ i ].Duration, "Duration" );
					node->getValueByName( PactLose[ i ].Name, "Name" );
				}
			}
		}
		// get bitmap for icone display
		root.getValueByName(Icon, "icon");
		root.getValueByName(IconBackground, "icon_background");
	}
}


//=================================================================================
void CPactSheet::serial(class NLMISC::IStream &f)	throw(NLMISC::EStream)
{
	f.serialCont(PactLose);
	f.serial(Icon);
	f.serial(IconBackground);
}
