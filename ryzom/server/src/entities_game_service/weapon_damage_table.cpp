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



/////////////
// INCLUDE 
/////////////
#include "stdpch.h"
//
#include "nel/misc/variable.h"
//
#include "weapon_damage_table.h"
#include "egs_mirror.h"
#include "egs_variables.h"

/////////////
// USING
/////////////
using namespace std;
using namespace NLMISC;


/////////////
// GLOBALS 
/////////////


//--------------------------------------------------------------
//				init()  
//--------------------------------------------------------------
void CWeaponDamageTable::init()
{
//	nlinfo("Weapon damage table config :\nMinDamage = %.2f\t\tDamageStep = %.2f\t\tExponentialPower = %.2f\t\tSmoothingFactor = %.2f", MinDamage, DamageStep, ExponentialPower, SmoothingFactor);
	for (uint reference = 0 ; reference <= MaxRefenceSkillValue ; ++reference )
	{
		if (reference  == 50)
		{
//			nlinfo("\n\nNew table, recommended skill value = %u\n", reference);
//			nlinfo("Skill\t\tDamage");
		}

		const float dmgLimit = MinDamage + DamageStep*reference;

		for (uint skill = 0 ; skill <= MaxSkillValue ; ++skill )
		{
			// compute ref damage value, linear progression
			const float ref = MinDamage + DamageStep*skill;			
			
			/// % of reference reached by skill (max 100%)
			const float pos = (skill>=reference) ? 1.0f : float(skill)/reference;

			float value;
			if (pos < 1.0f)
			{
				value = (float) ((MinDamage + (dmgLimit-MinDamage)*pow(pos, ExponentialPower) + ref) / 2.0f);
			}
			else
			{
				if (skill <= 1)
				{
					value = ref;
				}
				else
				{
					value = _DamageTable[reference][skill-1] + (_DamageTable[reference][skill-1] - _DamageTable[reference][skill-2]) * SmoothingFactor;
				}				
			}

			_DamageTable[reference][skill] = value;

			if (reference  == 50)
			{				
//				nlinfo("%u\t\t%.2f", skill, value);
			}
		}
	}
//	nlinfo("\n\n");
	
} // init //


