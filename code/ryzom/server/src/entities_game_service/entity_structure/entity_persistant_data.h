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



#ifndef RY_ENTITY_PERSISTANT_DATA_H
#define RY_ENTITY_PERSISTANT_DATA_H

#include "nel/misc/ucstring.h"

#include "game_share/mirror_prop_value.h"
#include "game_share/people.h"
#include "game_share/gender.h"
#include "game_share/characteristics.h"
#include "game_share/scores.h"
#include "game_share/skills.h"
#include "game_share/slot_equipment.h"
#include "game_share/ryzom_mirror_properties.h"
#include "server_share/entity_state.h"

#include "statistic.h"
#include "special_modifier.h"

/**
 * CEntityBasePersistantData
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2001
 */
class CEntityBasePersistantData
{
public:
	CEntityBasePersistantData( bool noSkills )
		:_Skills( noSkills ){}

	// Version History
	// 2 : added _DodgeAsDefense and _ProtectedSlot
	// 3 : race is now serialized as a string not as enum
	//
	/**
	 * \return the current version of the class. Useful for managing old versions of saved players
	 * WARNING : the version number should be incremented when the serial method is modified
	 */
	static inline uint16 getCurrentVersion(){ return 6; }

	// serial: reading off-mirror, writing from mirror
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	void serialXml(NLMISC::IStream &f) throw(NLMISC::EStream);
	

	// Entity state ( X, Y, Z,T )
	CEntityState							_EntityState;
	/// Fiche ID
	CMirrorPropValueAlice< TYPE_SHEET, CPropLocationPacked<2> >	_SheetId;
	/// name
	ucstring								_Name;
	/// name (family)	
//	ucstring								_Surname;
	/// Race of entity 
	EGSPD::CPeople::TPeople					_Race;
	/// Gender
	uint8									_Gender;
	/// Size
	uint8									_Size;
	/// physical characteristics
	CPhysicalCharacteristics				_PhysCharacs;
	/// physical scores
	CPhysicalScores							_PhysScores;
	/// nb bags
	uint8									_NbBag;
	// special modifiers
	CSpecialModifiers						_SpecialModifiers;

	/// location protected by the entity
	SLOT_EQUIPMENT::TSlotEquipment			_ProtectedSlot;
	/// use dogde instead of parry for defense
	bool									_DodgeAsDefense;


protected:
	/// skills
	CSkills					_Skills;
	
};

#endif // RY_ENTITY_PERSISTANT_DATA_H
/* entity_persistant_data.h */
