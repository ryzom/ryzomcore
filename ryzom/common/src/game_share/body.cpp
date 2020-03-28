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
//
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"
//
#include "body.h"

using namespace std;
using namespace NLMISC;

const uint8 NbBodySlotParts = 6;

namespace BODY
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TBodyType)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Homin)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Quadruped)
	  NL_STRING_CONVERSION_TABLE_ENTRY(LandKitin)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FlyingKitin)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Fish)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Bird)
	  NL_STRING_CONVERSION_TABLE_ENTRY(Plant)
	NL_END_STRING_CONVERSION_TABLE(TBodyType, ConversionType, UnknownBodyType)

	///
	TBodyType toBodyType( const std::string &str )
	{
		return ConversionType.fromString(str);
	}

	///
	const std::string& toString( TBodyType type )
	{
		return ConversionType.toString(type);
	}


	NL_BEGIN_STRING_CONVERSION_TABLE (TBodyPart)
	  NL_STRING_CONVERSION_TABLE_ENTRY(HHead)
	  NL_STRING_CONVERSION_TABLE_ENTRY(HChest)
	  NL_STRING_CONVERSION_TABLE_ENTRY(HArms)
	  NL_STRING_CONVERSION_TABLE_ENTRY(HHands)
	  NL_STRING_CONVERSION_TABLE_ENTRY(HLegs)
	  NL_STRING_CONVERSION_TABLE_ENTRY(HFeet)

	  NL_STRING_CONVERSION_TABLE_ENTRY(QHead)
	  NL_STRING_CONVERSION_TABLE_ENTRY(QBody)
	  NL_STRING_CONVERSION_TABLE_ENTRY(QFrontPaws)
	  NL_STRING_CONVERSION_TABLE_ENTRY(QFrontHooves)
	  NL_STRING_CONVERSION_TABLE_ENTRY(QRearPaws)
	  NL_STRING_CONVERSION_TABLE_ENTRY(QRearHooves)

	  NL_STRING_CONVERSION_TABLE_ENTRY(LKHead)
	  NL_STRING_CONVERSION_TABLE_ENTRY(LKBody)
	  NL_STRING_CONVERSION_TABLE_ENTRY(LKFrontPaws1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(LKFrontPaws2)
	  NL_STRING_CONVERSION_TABLE_ENTRY(LKRearPaws1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(LKRearPaws2)

	  NL_STRING_CONVERSION_TABLE_ENTRY(FKHead)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FKBody)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FKPaws1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FKPaws2)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FKWings1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FKWings2)

	  NL_STRING_CONVERSION_TABLE_ENTRY(FHead)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FBody)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FFrontFins1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FFrontFins2)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FRearFins1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(FRearFins2)

	  NL_STRING_CONVERSION_TABLE_ENTRY(BHead)
	  NL_STRING_CONVERSION_TABLE_ENTRY(BBody)
	  NL_STRING_CONVERSION_TABLE_ENTRY(BFeet1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(BFeet2)
	  NL_STRING_CONVERSION_TABLE_ENTRY(BWings1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(BWings2)

	  NL_STRING_CONVERSION_TABLE_ENTRY(PUpperTrunk)
	  NL_STRING_CONVERSION_TABLE_ENTRY(PTrunk)
	  NL_STRING_CONVERSION_TABLE_ENTRY(PLeaves1)
	  NL_STRING_CONVERSION_TABLE_ENTRY(PLeaves2)
	  NL_STRING_CONVERSION_TABLE_ENTRY(PLowerTrunk)
	  NL_STRING_CONVERSION_TABLE_ENTRY(PVisibleRoots)
	NL_END_STRING_CONVERSION_TABLE(TBodyPart, ConversionLoc, UnknownBodyPart)


	///
	TBodyPart toBodyPart( const std::string &str )
	{
		return ConversionLoc.fromString(str);
	}

	///
	const std::string& toString( TBodyPart loc )
	{
		return ConversionLoc.toString(loc);
	}

	/// convert a body part to a slot equipement
	SLOT_EQUIPMENT::TSlotEquipment toSlotEquipement(TBodyPart loc)
	{
		if (loc >= 0 && loc < NbBodySlotParts)
		{
			switch(loc%NbBodySlotParts)
			{
			case 0:
				return (SLOT_EQUIPMENT::HEAD);
			case 1:
				return (SLOT_EQUIPMENT::CHEST);
			case 2:
				return (SLOT_EQUIPMENT::ARMS);
			case 3:
				return (SLOT_EQUIPMENT::HANDS);
			case 4:
				return (SLOT_EQUIPMENT::LEGS);
			case 5:
				return (SLOT_EQUIPMENT::FEET);
			default:
				return SLOT_EQUIPMENT::UNDEFINED;
			};
		}
		else
			return SLOT_EQUIPMENT::UNDEFINED;
	}

	/// get a body part from a body type and a slot type
	TBodyPart getBodyPart( TBodyType type, SLOT_EQUIPMENT::TSlotEquipment slot)
	{
		uint8 index;
		switch(slot)
		{
		case SLOT_EQUIPMENT::HEAD:
			index = 0;
			break;
		case SLOT_EQUIPMENT::CHEST:
			index = 1;
			break;
		case SLOT_EQUIPMENT::ARMS:
			index = 2;
			break;
		case SLOT_EQUIPMENT::HANDS:
			index = 3;
			break;
		case SLOT_EQUIPMENT::LEGS:
			index = 4;
			break;
		case SLOT_EQUIPMENT::FEET:
			index = 5;
			break;
		default:
			return UnknownBodyPart;
		};

		switch(type)
		{
		case Homin:
			return (TBodyPart)(BeginHomin+index);
		case Quadruped:
			return (TBodyPart)(BeginQuadruped+index);
		case LandKitin:
			return (TBodyPart)(BeginLandKitin+index);
		case FlyingKitin:
			return (TBodyPart)(BeginFlyingKitin+index);
		case Fish:
			return (TBodyPart)(BeginFish+index);
		case Bird:
			return (TBodyPart)(BeginBird+index);
		case Plant:
			return (TBodyPart)(BeginPlant+index);
		default:
			return UnknownBodyPart;
		};
	}

	TBodyPart getMatchingHominBodyPart(TBodyPart src)
	{
		if ((uint) src >= NbBodyParts)   return UnknownBodyPart;
		return (TBodyPart) (src % 6);
	}


}; // BODY
