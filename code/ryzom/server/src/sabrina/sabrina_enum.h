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



#ifndef RY_SABRINA_ENUM_H
#define RY_SABRINA_ENUM_H

namespace SABRINA
{
	// type of healing to apply
	enum THealType
	{
		HealUnknown,		HealHP,				HealSap,			HealSta,			HealFocus,
	};

	// types of damage to apply
	enum TDmgType
	{
		// basic damage types *** See DMGTYPE in game_share for definitive enum ***
		DmgSlashing,		DmgPiercing,		DmgBlunt,
		DmgRot,				DmgAcid,			DmgCold,
		DmgFire,			DmgPoison,			DmgElectric,		DmgShock,

		// energy damage types
		DmgSap,				DmgStamina,

		// damage to armor or equipment *** see SLOT_EQUIPMENT::TSlotEquipment *** for correct order ***
		DmgRightHandItem,	DmgLeftHandItem,			
		DmgArmorHead,		DmgArmorBody,		DmgArmorLegs,		DmgArmorFeet,		DmgArmorArms,		DmgArmorHands,
	};

	enum TEventCode						// this enum is used for both cancelation /failure and success
	{
		Unknown=-1,

		SuccessPartial,
		SuccessNormal,
		SuccessCritical,

		FailInsufficientHP,			// not enough HP to pay couterpart
		FailInsufficientSap,		// not enough Sap to pay couterpart	
		FailInsufficientStamina,	// not enough Sta to pay couterpart
		FailTgtDead,				// the target died
		FailTgtInaccessible,		// the target teleported or became inaccessible
		FailIMoved,					// the actor moved and invalidated their action
		FailInterrupt,				// the actor was interrupted by an external action
		FailActionFailed,			// just unlucky - the action failed
		FailTgtResist,				// the target resisted the action
		FailTgtDodge,				// the target dodged the action

		PlayerLoggedOff,			// player logged off and player record deleted
		PlayerTeleported,			// player teleported
	};

	inline bool isSuccess(TEventCode code)
	{
		return (code==SuccessPartial || code==SuccessNormal || code==SuccessCritical);
	}

	enum TEffectType
	{
		EffUnknown,
		EffStun,
		EffMezz,
		EffRoot,
		EffSkillDebuffMagic,
		EffSkillDebuffRange,
		EffSkillDebuffMelee,
	};
}

//------------------------------------------------------------------------------------------------------
#endif
