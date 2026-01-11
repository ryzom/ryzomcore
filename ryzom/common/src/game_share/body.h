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



#ifndef RY_BODY_H
#define RY_BODY_H

#include "nel/misc/types_nl.h"
//
#include "slot_equipment.h"

namespace BODY
{
	/** Side of body
      */
	enum TSide
	{
		Right = 0,
		Left
	};

	/**
	 * describe the different body types, according to the considered entity
	 */
	enum TBodyType
	{
		Homin = 0,
		Quadruped,
		LandKitin,
		FlyingKitin,
		Fish,
		Bird,
		Plant,

		NbBodyTypes,
		UnknownBodyType = NbBodyTypes,
	};

	/**
	 * Following enum describe the body parts according to considered entity (body) type
	 * MUST be always in this order (regarding slot)
	 *
	 * HEAD
	 * CHEST
	 * ARMS
	 * HANDS
	 * LEGS
	 * FEET
	 * NbBodySlotParts = 6
	 */
	enum TBodyPart
	{
		BeginHomin = 0,
			HHead = BeginHomin,
			HChest,
			HArms,
			HHands,
			HLegs,
			HFeet,
		EndHomin = HFeet,

		BeginQuadruped,
			QHead = BeginQuadruped,
			QBody,
			QFrontPaws,
			QFrontHooves,
			QRearPaws,
			QRearHooves,
		EndHomins = QRearHooves,

		BeginLandKitin,
			LKHead = BeginLandKitin,
			LKBody,
			LKFrontPaws1,
			LKFrontPaws2,
			LKRearPaws1,
			LKRearPaws2,
		EndLandKitin = LKRearPaws2,

		BeginFlyingKitin,
			FKHead = BeginFlyingKitin,
			FKBody,
			FKPaws1,
			FKPaws2,
			FKWings1,
			FKWings2,
		EndFlyingKitin = FKWings2,

		BeginFish,
			FHead = BeginFish,
			FBody,
			FFrontFins1,
			FFrontFins2,
			FRearFins1,
			FRearFins2,
		EndFish = FRearFins2,

		BeginBird,
			BHead = BeginBird,
			BBody,
			BFeet1,
			BFeet2,
			BWings1,
			BWings2,
		EndBird = BWings2,

		BeginPlant,
			PUpperTrunk = BeginPlant,
			PTrunk,
			PLeaves1,
			PLeaves2,
			PLowerTrunk,
			PVisibleRoots,
		EndPlant = PVisibleRoots,

		NbBodyParts,
		UnknownBodyPart = NbBodyParts,
	};

	/**
	 * get the localisation type from the input string
	 * \param str the input string
	 * \return the TBodyType associated to this string
	 */
	TBodyType toBodyType(const std::string &str);

	/// convert a body type to a string
	const std::string & toString(TBodyType type);

	/**
	 * get the body part from the input string
	 * \param str the input string
	 * \return the TBodyPart associated to this string
	 */
	TBodyPart toBodyPart(const std::string &str);

	/// convert a localisation to a string
	const std::string & toString(TBodyPart part);

	/// convert a body part to a slot type
	SLOT_EQUIPMENT::TSlotEquipment toSlotEquipement(TBodyPart part);

	/// get a body part from a body type and a slot type
	TBodyPart getBodyPart( TBodyType type, SLOT_EQUIPMENT::TSlotEquipment slot);

	// from any body part, gives the matching body part for homins, or UnknownBodyPart
	TBodyPart getMatchingHominBodyPart(TBodyPart src);



}; // BODY

#endif // RY_BODY_H
/* End of body.h */
