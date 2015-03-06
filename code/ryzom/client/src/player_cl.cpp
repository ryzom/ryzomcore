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
// INCLUDE //
/////////////
#include "stdpch.h"
// Misc
#include "nel/misc/time_nl.h"
// Client.
#include "player_cl.h"
#include "ingame_database_manager.h"
#include "net_manager.h"
#include "time_client.h"
#include "entity_animation_manager.h"
#include "sheet_manager.h"
#include "color_slot_manager.h"
#include "debug_client.h"
#include "gabarit.h"
#include "interface_v3/interface_manager.h"
#include "misc.h"
#include "pacs_client.h"
#include "motion/user_controls.h"
#include "client_cfg.h"
#include "user_entity.h"
#include "faction_war_manager.h"
// Client Sheets
#include "client_sheets/player_sheet.h"
// 3D
#include "nel/3d/u_scene.h"
#include "nel/3d/u_instance_material.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_bone.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_point_light.h"
// game share
#include "game_share/player_visual_properties.h"
#include "game_share/gender.h"
#include "game_share/bot_chat_types.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;
using namespace std;


////////////
// EXTERN //
////////////
extern UScene					*Scene;
extern CEntityAnimationManager	*EAM;
extern UTextContext				*TextContext;
extern UCamera					MainCam;


//-----------------------------------------------
// CPlayerCL :
// Constructor.
//-----------------------------------------------
CPlayerCL::CPlayerCL()
: CCharacterCL()
{
	Type = Player;

	// Resize _Instances to the number of visual slots.
	_Instances.resize(SLOTTYPE::NB_SLOT);

	// No sheet pointed.
	_Sheet			= 0;
	_PlayerSheet	= 0;

	// Some default colors.
	_HairColor = 0;
	_EyesColor = 0;

	// Not enough information to display the player.
	_WaitForAppearance = true;

	_PlayerCLAsyncTextureLoading= false;

	// Light Off and not allocated
	_LightOn = false;
}// CPlayerCL //


//-----------------------------------------------
// ~CPlayerCL :
// Destructor.
//-----------------------------------------------
CPlayerCL::~CPlayerCL()
{
	// No more sheet pointed.
	_PlayerSheet	= NULL;

	// Remove the light
	if(!_Light.empty())
	{
		if(Scene)
			Scene->deletePointLight(_Light);
	}
}

//---------------------------------------------------
// getScale :
// Return the entity scale. (return 1.0 if there is any problem).
//---------------------------------------------------
float CPlayerCL::getScale() const	// virtual
{
	// Default Scale.
	return _CharacterScalePos;
}// getScale //



//-----------------------------------------------
// getGroundFX :
// retrieve ground fxs for the entity depending on the ground
//-----------------------------------------------
const std::vector<CGroundFXSheet> *CPlayerCL::getGroundFX() const
{
	switch (getGender())
	{
		case 0: return &(_PlayerSheet->GenderInfos[0].GroundFX);
		case 1: return &(_PlayerSheet->GenderInfos[1].GroundFX);
		default: break;
	}
	return NULL;
}


//-----------------------------------------------
// isNeutral :
// Return true if this entity is a neutral entity(pvp or not)
//-----------------------------------------------
bool CPlayerCL::isNeutral() const
{
	return (!isEnemy() && !isAlly());
}


//-----------------------------------------------
// isFriend :
// Return true if this entity is a user's friend.
//-----------------------------------------------
bool CPlayerCL::isFriend () const
{
	return isNeutral() || isAlly();
}


//-----------------------------------------------
// isEnemy :
// true if at least enemy in on pvp mode
//-----------------------------------------------
bool CPlayerCL::isEnemy () const
{
	// Challenge i.e. SOLO FULL PVP
	if( getPvpMode()&PVP_MODE::PvpChallenge  ||
		UserEntity->getPvpMode()&PVP_MODE::PvpChallenge )
	{
		return true;
	}


	// if one of 2 players is not in pvp they can't be enemies
	if( UserEntity->getPvpMode() == PVP_MODE::None ||
		getPvpMode() == PVP_MODE::None )
	{
		return false;
	}
	
	// if one of 2 players is safe they can't be enemies
	if( UserEntity->getPvpMode()&PVP_MODE::PvpSafe ||
		getPvpMode()&PVP_MODE::PvpSafe )
	{
		return false;
	}

	// if one of 2 players are in safe zone and not flagged they can't be enemies
	if ((UserEntity->getPvpMode()&PVP_MODE::PvpZoneSafe &&
		((UserEntity->getPvpMode()&PVP_MODE::PvpFactionFlagged) == 0))
		||
		(getPvpMode()&PVP_MODE::PvpZoneSafe &&
		((getPvpMode()&PVP_MODE::PvpFactionFlagged) == 0)))
	{
		return false;
	}

	// Duel
	if( getPvpMode()&PVP_MODE::PvpDuel &&
		UserEntity->getPvpMode()&PVP_MODE::PvpDuel )
	{
		return true; // TODO
	}

	// Outpost
	if ( isAnOutpostEnemy() )
	{
		return true;
	}

	// Zone Free
	if( getPvpMode()&PVP_MODE::PvpZoneFree &&
		UserEntity->getPvpMode()&PVP_MODE::PvpZoneFree )
	{
		// If not in same Team and not in same League => ennemy
		if( !isInTeam() && !isInSameLeague() )
			return true;
	}

	// Zone Guild
	if( getPvpMode()&PVP_MODE::PvpZoneGuild &&
		UserEntity->getPvpMode()&PVP_MODE::PvpZoneGuild )
	{
		// If in same Guild but different Leagues => ennemy
		if ( isInSameGuild() && oneInLeague() && !isInSameLeague() )
			return true;

		if( !isInTeam() && !isInSameLeague() )
			return true;
	}

	// Zone Faction
	if( getPvpMode()&PVP_MODE::PvpZoneFaction &&
		UserEntity->getPvpMode()&PVP_MODE::PvpZoneFaction )
	{
		if (getPvpClan() != UserEntity->getPvpClan())
			return true;
	}

	// Free PVP : Ennemis are not in team AND not in league
	if ((getPvpMode()&PVP_MODE::PvpFaction || getPvpMode()&PVP_MODE::PvpFactionFlagged) &&
		(UserEntity->getPvpMode()&PVP_MODE::PvpFaction || UserEntity->getPvpMode()&PVP_MODE::PvpFactionFlagged))
	{
		// If in same Guild but different Leagues => ennemy
		if ( isInSameGuild() && oneInLeague() && !isInSameLeague() )
			return true;

		if (!isInTeam() && !isInSameLeague())
			return true;
	}

	return false;

} // isEnemy //


//-----------------------------------------------
// isAlly :
// true if at least ally in one pvp mode
//-----------------------------------------------
bool CPlayerCL::isAlly() const
{

	// Challenge i.e. SOLO FULL PVP
	if( getPvpMode()&PVP_MODE::PvpChallenge  ||
		UserEntity->getPvpMode()&PVP_MODE::PvpChallenge )
	{
		return false;
	}

	// if one of 2 players is not in pvp they can't be allies
	if( UserEntity->getPvpMode() == PVP_MODE::None ||
		getPvpMode() == PVP_MODE::None )
	{
		return false;
	}

	// if one of 2 players is in safe zone and not other they can't be allies
	if ((UserEntity->getPvpMode()&PVP_MODE::PvpSafe) != (getPvpMode()&PVP_MODE::PvpSafe))
	{
		return false;
	}

	// Outpost
	if ( isAnOutpostAlly() )
	{
		return true;
	}

	// Zone Free
	if( getPvpMode()&PVP_MODE::PvpZoneFree &&
		UserEntity->getPvpMode()&PVP_MODE::PvpZoneFree )
	{
		if( isInTeam() || isInSameLeague() )
			return true;
	}

	// Zone Guild
	if( getPvpMode()&PVP_MODE::PvpZoneGuild &&
		UserEntity->getPvpMode()&PVP_MODE::PvpZoneGuild )
	{
		if( isInTeam() || isInSameLeague() )
			return true;

		if ( isInSameGuild() && !oneInLeague() )
			return true;

	}

	// Zone Faction
	if( getPvpMode()&PVP_MODE::PvpZoneFaction &&
		UserEntity->getPvpMode()&PVP_MODE::PvpZoneFaction )
	{
		if (getPvpClan() == UserEntity->getPvpClan())
			return true;
	}

	// Free PVP : Allies are in team OR in league
	if ((getPvpMode()&PVP_MODE::PvpFaction || getPvpMode()&PVP_MODE::PvpFactionFlagged) &&
		(UserEntity->getPvpMode()&PVP_MODE::PvpFaction || UserEntity->getPvpMode()&PVP_MODE::PvpFactionFlagged))
	{
		if (isInTeam() || isInSameLeague())
			return true;

		if ( isInSameGuild() && !oneInLeague() )
			return true;
	}

	return false;

} // isAlly //


//-----------------------------------------------
// isNeutralPVP :
//-----------------------------------------------
bool CPlayerCL::isNeutralPVP() const
{
	// if player is not in pvp they can't be neutral pvp
	if( getPvpMode() == PVP_MODE::None )
	{
		return false;
	}

	if( UserEntity->getPvpMode() == PVP_MODE::None )
	{
		return false;
	}

	return (!isEnemy() && !isAlly());
}


//-----------------------------------------------
// build :
// Build the entity from a sheet.
//-----------------------------------------------
bool CPlayerCL::build(const CEntitySheet *sheet)	// virtual
{
	// Cast the sheet in the right type.
	_PlayerSheet = dynamic_cast<const CRaceStatsSheet *>(sheet);
	if(_PlayerSheet==0)
	{
		pushDebugStr(NLMISC::toString("Player '%d' sheet is not a '.race_stats' -> BIG PROBLEM.", _Slot));
		return false;
	}
	else
		pushInfoStr(NLMISC::toString("Player '%d' sheet is valid.", _Slot));
	// Get the DB Entry
	if(IngameDbMngr.getNodePtr())
	{
		CCDBNodeBranch *nodeRoot = dynamic_cast<CCDBNodeBranch *>(IngameDbMngr.getNodePtr()->getNode(0));
		if(nodeRoot)
		{
			_DBEntry = dynamic_cast<CCDBNodeBranch *>(nodeRoot->getNode(_Slot));
			if(_DBEntry == 0)
				pushDebugStr("Cannot get a pointer on the DB entry.");
		}
	}

	// Compute the first automaton.
	_CurrentAutomaton = automatonType() + "_normal.automaton";

	// Initialize the player look.
	init3d();
	// Compute the primitive
	initPrimitive(0.5f, 2.0f, 0.0f, 0.0f, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, MaskColPlayer, MaskColNone);
	// Create the collision entity (used to snap the entity to the ground).
	computeCollisionEntity();

	// Initialize properties of the client.
	initProperties();
	// Entity Created.
	return true;
}// build //

//-----------------------------------------------
// automatonType :
// Return the automaton type of the entity (homin, creature, etc.)
//-----------------------------------------------
std::string CPlayerCL::automatonType() const	// virtual
{
	return _PlayerSheet->Automaton;
}// automatonType //


//-----------------------------------------------
// init3d :
// Initialize the graphic for the player.
//-----------------------------------------------
void CPlayerCL::init3d()
{
	createPlayList();
	// Initialize the internal time.
	_LastFrameTime = ((double)T1) * 0.001;
}// init3d //


//-----------------------------------------------
// initProperties :
// Initialize properties of the entity (according to the class).
//-----------------------------------------------
void CPlayerCL::initProperties()
{
	properties().selectable(true);
	properties().attackable(false);
	properties().givable(true);
	properties().invitable(true);
	properties().canExchangeItem(true);
}// initProperties //


//-----------------------------------------------
// equip :
// Set the equipmenent worn.
//-----------------------------------------------
void CPlayerCL::equip(SLOTTYPE::EVisualSlot slot, const std::string &shapeName, const CItemSheet *item)
{
	// Check slot.
	if(slot == SLOTTYPE::HIDDEN_SLOT || slot >= SLOTTYPE::NB_SLOT)
	{
		nlwarning("CCharacterCL::equip : slot %d is not valid.", (uint)slot);
		return;
	}

	uint s = (uint)slot;

	// If exactly the same than before -> return

	if (!_Instances[s].Loading.empty())
	{
		if ((_Instances[s].LoadingName == shapeName) && (_Instances[s].FXItemSheet == item))
			return;
	}
	else if (!_Instances[s].Current.empty())
	{
		if ((_Instances[s].CurrentName == shapeName) && (_Instances[s].FXItemSheet == item))
			return;
	}



	// Attach to the skeleton.
	string stickPoint;
	if(!_Skeleton.empty())
	{
		switch(slot)
		{
			case SLOTTYPE::RIGHT_HAND_SLOT:
				if( item && item->ItemType != ITEM_TYPE::MAGICIAN_STAFF )
					stickPoint = "box_arme";
			break;

			case SLOTTYPE::LEFT_HAND_SLOT:
				if(_Items[slot].Sheet && _Items[slot].Sheet->getAnimSet()=="s")
					stickPoint = "Box_bouclier";
				else
					stickPoint = "box_arme_gauche";
			break;

			default:
			break;
		}
	}

	/* If the object is sticked (ie not a skin), decide to delete the Current instance. Why? because the animation
		is changed according to the equipped item.

		Hence, For example, if a sword would be changed for a gun, then the new gun animation would take place,
		while Keeping the old sword shape. BAD.
	*/
	if(!stickPoint.empty())
		_Instances[s].createLoading(string(), stickPoint);

	// Create the instance.
	if(item)
		_Instances[s].createLoading(shapeName, stickPoint, item->MapVariant);
	else
		_Instances[s].createLoading(shapeName, stickPoint);

	// If shapeName is empty, only clear the slot
	if(shapeName.empty())
	{
		_Items[slot].release();
		return;
	}

	if(!_Instances[s].Loading.empty())
	{
		_Instances[s].FXItemSheet = item;

		_Items[slot].initFXs(slot, _Instances[s].Loading);
	}
	else
		nlwarning("PL::equip(1):%d: cannot create the instance '%s'.", _Slot, shapeName.c_str());

	if ((slot != SLOTTYPE::RIGHT_HAND_SLOT) && (slot != SLOTTYPE::LEFT_HAND_SLOT))
		applyColorSlot(_Instances[s], skin(), 0, _HairColor, _EyesColor);

}// equip //

//-----------------------------------------------
// equip :
// Compute the equipmenent worn.
//-----------------------------------------------
void CPlayerCL::equip(SLOTTYPE::EVisualSlot slot, uint index, uint color)
{
	// Get the sheet according to the visual slot
	_Items[slot].Sheet = SheetMngr.getItem(slot, index);
	if(_Items[slot].Sheet)
	{
		const CItemSheet *item = _Items[slot].Sheet;

		// If the gender is a female get the right shape.
		if(_Gender == GSGENDER::female)
			equip(slot, item->getShapeFemale(), item);
		// Else get the default shape.
		else
			equip(slot, item->getShape(), item);

		// Check there is a shape.
		UInstance pInst = _Instances[slot].createLoadingFromCurrent();
		if(!pInst.empty())
		{
			// Set the right texture variation (quality).
			pInst.selectTextureSet((uint)item->MapVariant);
			_Instances[slot].TextureSet = item->MapVariant;

			// If Hair, color is for the head slot.
			if(slot == SLOTTYPE::HEAD_SLOT && item->Family != ITEMFAMILY::ARMOR)
				applyColorSlot(_Instances[slot], skin(), 0, color, _EyesColor);
			else
			{
				// Set the User Color.
				if(item->Color == -1)
					applyColorSlot(_Instances[slot], skin(), color, _HairColor, _EyesColor);
				// Set the Item Color.
				else if(item->Color != -2)
					applyColorSlot(_Instances[slot], skin(), item->Color, _HairColor, _EyesColor);
				// Else let the default color.
				else
					applyColorSlot(_Instances[slot], skin(), 0, _HairColor, _EyesColor);
			}
		}
	}
	// Default equipment.
	else
	{
		nlwarning("PL:equip(2):%d: VS '%d' default equipement used.", _Slot, slot);
		sint idx = SheetMngr.getVSIndex(_PlayerSheet->GenderInfos[_Gender].Items[slot], slot);
		if(idx != -1)
		{
			if(SheetMngr.getItem(slot, (uint)idx))
			{
				// If the gender is a female get the right shape.
				if(_Gender == GSGENDER::female)
					equip(slot, SheetMngr.getItem(slot, (uint)idx)->getShapeFemale());
				// Else get the default shape.
				else
					equip(slot, SheetMngr.getItem(slot, (uint)idx)->getShape());
			}
		}
/*
		//
		switch(slot)
		{
		case SLOTTYPE::CHEST_SLOT:
			nlwarning("PL::equip(2):%d: default equipement used for the Chest.", _Slot);
			equip(slot, _DefaultChest);
			break;
		case SLOTTYPE::LEGS_SLOT:
			nlwarning("PL::equip(2):%d: default equipement used for the Legs.", _Slot);
			equip(slot, _DefaultLegs);
			break;
		case SLOTTYPE::ARMS_SLOT:
			nlwarning("PL::equip(2):%d: default equipement used for the Arms.", _Slot);
			equip(slot, _DefaultArms);
			break;
		case SLOTTYPE::HANDS_SLOT:
			nlwarning("PL::equip(2):%d: default equipement used for the Hands.", _Slot);
			equip(slot, _DefaultHands);
			break;
		case SLOTTYPE::FEET_SLOT:
			nlwarning("PL::equip(2):%d: default equipement used for the Feet.", _Slot);
			equip(slot, _DefaultFeet);
			break;
		case SLOTTYPE::HEAD_SLOT:
			nlwarning("PL::equip(2):%d: default equipement used for the Head.", _Slot);
			equip(slot, _DefaultHair);
			break;

		default:
			nlwarning("PL::equip(2):%d: default equipement used for an unknown slot.", _Slot);
			break;
		}
*/
	}
}// equip //

//-----------------------------------------------
// computeAnimSet :
// Compute the animation set to use according to weapons, mode and race.
//-----------------------------------------------
void CPlayerCL::computeAnimSet()
{
	// We need a valid Gender to compute the animset.
	if(_Gender >= 2)
	{
		nlwarning("PL:computeAnimSet:%d: not a male or a female (gender=%d).", _Slot, _Gender);
		return;
	}

	// Now computing the animset.
	// Do not count weapons if swimming.
	if(!::computeAnimSet(_CurrentAnimSet[MOVE], _Mode, _PlayerSheet->GenderInfos[_Gender].AnimSetBaseName, _Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet, _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet, !modeWithHiddenItems()))
		nlwarning("PL:computeAnimSet:%d: pb when computing the animset.", _Slot);

}// computeAnimSet //


//-----------------------------------------------
// updateVisualPropertyVpa :
// Update the Visual Property A.
// \todo GUIGUI : use gender enum
//-----------------------------------------------
void CPlayerCL::updateVisualPropertyVpa(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();

	// Player will now have enough information to display the character.
	_WaitForAppearance = false;

	// Get the property.
	SPropVisualA visualA = *(SPropVisualA *)(&prop);

	// GENDER
	_Gender = (GSGENDER::EGender)(visualA.PropertySubData.Sex);
	if(_Gender!=GSGENDER::male && _Gender!=GSGENDER::female)
	{
		nlwarning("PL::updateVPVpa:%d: neither a male nor a female -> male selected.", _Slot);
		_Gender = GSGENDER::male;
	}

	// update title when gender changed
	const ucstring replacement(STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(_TitleRaw, _Gender == GSGENDER::female));
	if (!replacement.empty() || !ClientCfg.DebugStringManager)
	{
		// Get extended name
		_NameEx = replacement;
		_Title = replacement;

		// display the new title in player interface
		if (_Slot == 0)
		{
			CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:player:header_opened:player_title"));
			if (pVT != NULL) pVT->setText(_Title);
		}

		// rebuild in scene interface
		buildInSceneInterface();
	}

	// Setup _CharacterScalePos
	_CharacterScalePos = _PlayerSheet->GenderInfos[_Gender].CharacterScalePos;

	// Check if skeleton has changed
	if (_CacheSkeletonShapeName != _PlayerSheet->GenderInfos[_Gender].Skelfilename)
	{
		_CacheSkeletonShapeName = _PlayerSheet->GenderInfos[_Gender].Skelfilename;

		// Clean the playlist.
		if(_PlayList)
			_PlayList->resetAllChannels();

		// We can now build the skeleton so do it now.
		skeleton(_CacheSkeletonShapeName);

		// Invalidate instances cache
		for (uint i = 0; i < _Instances.size(); ++i)
			_Instances[i].CurrentName = _Instances[i].LoadingName = "";
		_Face.CurrentName = _Face.LoadingName = "";
	}
	// Check the skeleton.
	if(skeleton() && !ClientCfg.Light)
	{
		// To re-link the skeleton to the mount if needed.
		parent(parent());
		// Set the skeleton scale.
		// \todo GUIGUI: mettre le scale aussi dans race_stats.
		// Setup Lod Character skeleton, if skeleton exist
		// Get Lod Character Id from the sheet.
		sint clodId= getLodCharacterId(*Scene, _PlayerSheet->GenderInfos[_Gender].LodCharacterName);
		if(clodId>=0)
		{
			// Setup Lod Character shape and distance
			skeleton()->setLodCharacterShape(clodId);
			skeleton()->setLodCharacterDistance(_PlayerSheet->GenderInfos[_Gender].LodCharacterDistance);
		}
		// Compute the
		computeSomeBoneId();

		// CHEST
		equip(SLOTTYPE::CHEST_SLOT, visualA.PropertySubData.JacketModel, visualA.PropertySubData.JacketColor);
		// LEGS
		equip(SLOTTYPE::LEGS_SLOT, visualA.PropertySubData.TrouserModel, visualA.PropertySubData.TrouserColor);
		// ARMS
		equip(SLOTTYPE::ARMS_SLOT, visualA.PropertySubData.ArmModel, visualA.PropertySubData.ArmColor);
		// HAT
		equip(SLOTTYPE::HEAD_SLOT, visualA.PropertySubData.HatModel, visualA.PropertySubData.HatColor);
		// OBJECT in the RIGHT HAND
		_Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet = SheetMngr.getItem(SLOTTYPE::RIGHT_HAND_SLOT, visualA.PropertySubData.WeaponRightHand);
		// Equip the weapon(object/tool).
		if(_Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet)
		{
			if(_Gender == GSGENDER::female)
				equip(SLOTTYPE::RIGHT_HAND_SLOT, _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet->getShapeFemale(), _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet);
			else
				equip(SLOTTYPE::RIGHT_HAND_SLOT, _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet->getShape(), _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet);

			NL3D::UInstance itemInstance = (!_Instances[SLOTTYPE::RIGHT_HAND_SLOT].Loading.empty()) ? _Instances[SLOTTYPE::RIGHT_HAND_SLOT].Loading : _Instances[SLOTTYPE::RIGHT_HAND_SLOT].Current;
			if (!itemInstance.empty())
			{
				// update fxs
				_Items[SLOTTYPE::RIGHT_HAND_SLOT].enableAdvantageFX(itemInstance);
				if ( _CurrentBehaviour.Behaviour != MBEHAV::EXTRACTING )
					_Items[SLOTTYPE::RIGHT_HAND_SLOT].setTrailSize(0);
					//_Items[SLOTTYPE::RIGHT_HAND_SLOT].setTrailSize(visualA.PropertySubData.RTrail);
			}
		}
		else
		{
			// No Valid item in the right hand.
			equip(SLOTTYPE::RIGHT_HAND_SLOT, "");
		}

		// OBJECT in the LEFT HAND
		_Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet = SheetMngr.getItem(SLOTTYPE::LEFT_HAND_SLOT, visualA.PropertySubData.WeaponLeftHand);
		// Equip the weapon(object/tool).
		if(_Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet)
		{
			equip(SLOTTYPE::LEFT_HAND_SLOT, _Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet->getShape(), _Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet);
			NL3D::UInstance itemInstance = (!_Instances[SLOTTYPE::LEFT_HAND_SLOT].Loading.empty()) ? _Instances[SLOTTYPE::LEFT_HAND_SLOT].Loading : _Instances[SLOTTYPE::LEFT_HAND_SLOT].Current;
			if (!itemInstance.empty())
			{
				// update fxs
				_Items[SLOTTYPE::LEFT_HAND_SLOT].enableAdvantageFX(itemInstance);
				_Items[SLOTTYPE::LEFT_HAND_SLOT].setTrailSize(0);
				//_Items[SLOTTYPE::LEFT_HAND_SLOT].setTrailSize(2 * (uint) visualA.PropertySubData.LTrail);
			}
		}
		else
		{
			// No Valid item in the left hand.
			equip(SLOTTYPE::LEFT_HAND_SLOT, "");
		}
		// Create face
		// Only create a face when there is no Helmet
		if(_Items[SLOTTYPE::HEAD_SLOT].Sheet == 0 || _Items[SLOTTYPE::HEAD_SLOT].Sheet->Family != ITEMFAMILY::ARMOR)
		{
			CItemSheet *faceItem = getItem(_PlayerSheet->GenderInfos[_Gender], SLOTTYPE::FACE_SLOT);
			if (faceItem)
			{
				string sFaceName;

				if(_Gender == GSGENDER::female)
					sFaceName = faceItem->getShapeFemale();
				else
					sFaceName = faceItem->getShape();

				if (((!_Face.Loading.empty()) && (_Face.LoadingName != sFaceName)) ||
					((!_Face.Current.empty()) && (_Face.CurrentName != sFaceName)) ||
					(_Face.Current.empty()))
				{
					if (!_Face.Loading.empty())
					{
						Scene->deleteInstance(_Face.Loading);
						_Face.Loading = NULL;
						_Face.LoadingName = sFaceName;
					}
					_Face.Loading = Scene->createInstance(sFaceName);
					if (!_Face.Loading.empty())
					{
						_Face.LoadingName = sFaceName;
						if(!skeleton()->bindSkin(_Face.Loading))
							nlwarning("PL::updateVPVpa:%d: Cannot bind the face.", _Slot);
						_Face.Loading.hide();
						// set it async for texture
						_Face.Loading.enableAsyncTextureMode(true);
					}
					else
						nlwarning("PL::updateVPVpa:%d: Cannot create the face.", _Slot);
				}
				_Face.TextureSet = faceItem->MapVariant;
				applyColorSlot(_Face, skin(), 0, visualA.PropertySubData.HatColor, 0); // Set a default ruflaket color.
			}
			else
				nlwarning("PL::updateVPVpa:%d: Face Item '%s' does not exist.", _Slot,
					_PlayerSheet->GenderInfos[_Gender].Items[SLOTTYPE::FACE_SLOT].c_str());
		}
		else
		{
			// There is a helmet !
			if (!_Face.Loading.empty())
				Scene->deleteInstance(_Face.Loading);
			_Face.Loading = NULL;
			_Face.LoadingName = "";
			if (!_Face.Current.empty())
				Scene->deleteInstance(_Face.Current);
			_Face.Current = NULL;
			_Face.CurrentName = "";
		}
		// Now we have a skeleton, we can update VpB and VpC.
		sint64 vB, vC;
		string propName;
		propName = toString("SERVER:Entities:E%d:P%d", _Slot, CLFECOMMON::PROPERTY_VPB);
		vB = NLGUI::CDBManager::getInstance()->getDbProp(propName)->getValue64();
		propName = toString("SERVER:Entities:E%d:P%d", _Slot, CLFECOMMON::PROPERTY_VPC);
		vC = NLGUI::CDBManager::getInstance()->getDbProp(propName)->getValue64();
		updateVisualPropertyVpb(0, vB);
		updateVisualPropertyVpc(0, vC);

		// Attach The Light if there is one.
		if(!_Light.empty() && _NameBoneId!=-1)
			_Skeleton.stickObject(_Light, _NameBoneId);

		// Compute the new animation set to use (due to weapons).
		computeAnimSet();

		// Set the animation to idle.
		setAnim(CAnimationStateSheet::Idle);
	}
	// No skeleton
	else
		nlwarning("PL::updateVPVpa:%d: Skeleton not allocated.", _Slot);
}// updateVisualPropertyVpa //

//-----------------------------------------------
// updateVisualPropertyVpb :
// Update the Visual Property B.
//-----------------------------------------------
void CPlayerCL::updateVisualPropertyVpb(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// Get the property.
	SPropVisualB visualB = *(SPropVisualB *)(&prop);

	if(_Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet)
	{
		NL3D::UInstance itemInstance = (!_Instances[SLOTTYPE::RIGHT_HAND_SLOT].Loading.empty()) ? _Instances[SLOTTYPE::RIGHT_HAND_SLOT].Loading : _Instances[SLOTTYPE::RIGHT_HAND_SLOT].Current;
		if (!itemInstance.empty())
		{
			// update fxs
			if ( _CurrentBehaviour.Behaviour != MBEHAV::EXTRACTING )
				_Items[SLOTTYPE::RIGHT_HAND_SLOT].setTrailSize(visualB.PropertySubData.RTrail);
		}
	}

	if(_Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet)
	{
		NL3D::UInstance itemInstance = (!_Instances[SLOTTYPE::LEFT_HAND_SLOT].Loading.empty()) ? _Instances[SLOTTYPE::LEFT_HAND_SLOT].Loading : _Instances[SLOTTYPE::LEFT_HAND_SLOT].Current;
		if (!itemInstance.empty())
		{
			// update fxs
			_Items[SLOTTYPE::LEFT_HAND_SLOT].setTrailSize(2 * (uint) visualB.PropertySubData.LTrail);
		}
	}

	if(skeleton())
	{
		// HANDS
		equip(SLOTTYPE::HANDS_SLOT, visualB.PropertySubData.HandsModel, visualB.PropertySubData.HandsColor);
		// FEET
		equip(SLOTTYPE::FEET_SLOT, visualB.PropertySubData.FeetModel, visualB.PropertySubData.FeetColor);
	}
	else
		nlinfo("PL::updateVPVpb:%d: Prop Vpb received before prop Vpa.", _Slot);
}// updateVisualPropertyVpb //

//-----------------------------------------------
// updateVisualPropertyVpc :
// Update the Visual Property C.
// \todo GUIGUI : factorize tatoos with character creation
//-----------------------------------------------
void CPlayerCL::updateVisualPropertyVpc(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	if(skeleton())
	{
		// Get the property.
		SPropVisualC visualC = *(SPropVisualC *)(&prop);

		// EYES
		_EyesColor = visualC.PropertySubData.EyesColor;
		UInstance inst;

		// must recreate the face asynchronously (because of color change / makeup change)
		inst= _Face.createLoadingFromCurrent();

		// if exist
		if (!inst.empty())
		{
			// change eyes color only
			applyColorSlot(_Face, _Face.ACSkin, _Face.ACUser, _Face.ACHair, visualC.PropertySubData.EyesColor);

			// Tattoo
			makeUp(inst, visualC.PropertySubData.Tattoo);

			// Morph
			static const char *baseName = "visage_00";
			float MTmin, MTmax;

			const CGenderInfo *pGI = &_PlayerSheet->GenderInfos[_Gender];
			if (pGI == NULL)
				return;

			MTmin = pGI->BlendShapeMin[0];
			MTmax = pGI->BlendShapeMax[0];
			if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
			inst.setBlendShapeFactor(baseName + toString(0), (float)(visualC.PropertySubData.MorphTarget1) / 7.f * (MTmax-MTmin) + MTmin, true);

			MTmin = pGI->BlendShapeMin[1];
			MTmax = pGI->BlendShapeMax[1];
			if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
			inst.setBlendShapeFactor(baseName + toString(1), (float)(visualC.PropertySubData.MorphTarget2) / 7.f * (MTmax-MTmin) + MTmin, true);

			MTmin = pGI->BlendShapeMin[2];
			MTmax = pGI->BlendShapeMax[2];
			if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
			inst.setBlendShapeFactor(baseName + toString(2), (float)(visualC.PropertySubData.MorphTarget3) / 7.f * (MTmax-MTmin) + MTmin, true);

			MTmin = pGI->BlendShapeMin[3];
			MTmax = pGI->BlendShapeMax[3];
			if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
			inst.setBlendShapeFactor(baseName + toString(3), (float)(visualC.PropertySubData.MorphTarget4) / 7.f * (MTmax-MTmin) + MTmin, true);

			MTmin = pGI->BlendShapeMin[4];
			MTmax = pGI->BlendShapeMax[4];
			if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
			inst.setBlendShapeFactor(baseName + toString(4), (float)(visualC.PropertySubData.MorphTarget5) / 7.f * (MTmax-MTmin) + MTmin, true);

			MTmin = pGI->BlendShapeMin[5];
			MTmax = pGI->BlendShapeMax[5];
			if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
			inst.setBlendShapeFactor(baseName + toString(5), (float)(visualC.PropertySubData.MorphTarget6) / 7.f * (MTmax-MTmin) + MTmin, true);

			MTmin = pGI->BlendShapeMin[6];
			MTmax = pGI->BlendShapeMax[6];
			if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
			inst.setBlendShapeFactor(baseName + toString(6), (float)(visualC.PropertySubData.MorphTarget7) / 7.f * (MTmax-MTmin) + MTmin, true);

			MTmin = pGI->BlendShapeMin[7];
			MTmax = pGI->BlendShapeMax[7];
			if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
			inst.setBlendShapeFactor(baseName + toString(7), (float)(visualC.PropertySubData.MorphTarget8) / 7.f * (MTmax-MTmin) + MTmin, true);
		}

		// Set the Gabarit
		float characterHeight	= (float)((sint8)(visualC.PropertySubData.CharacterHeight)-7)/7.f;
		float torsoWidth		= (float)((sint8)(visualC.PropertySubData.TorsoWidth)-7)/7.f;
		float armsWidth			= (float)((sint8)(visualC.PropertySubData.ArmsWidth)-7)/7.f;
		float legsWidth			= (float)((sint8)(visualC.PropertySubData.LegsWidth)-7)/7.f;
		float breastSize		= (float)((sint8)(visualC.PropertySubData.BreastSize)-7)/7.f;
		float heightScale, baseHeightScale;
		//	TODO : manage breast size
		GabaritSet.applyGabarit(*skeleton(), _Gender, people(), characterHeight, torsoWidth, armsWidth, legsWidth, breastSize, &heightScale);
		baseHeightScale = GabaritSet.getRefHeightScale(_Gender, people());

		if(baseHeightScale != 0.f)
			_CustomScalePos = heightScale/baseHeightScale;
		else
		{
			_CustomScalePos = 1.f;
			nlwarning("PL::updateVPVpc:'%d': baseHeight == 0.", _Slot);
		}
	}
	else
		nlinfo("PL:updateVPVpc:'%d': Prop Vpc received before prop Vpa.", _Slot);
}// updateVisualPropertyVpc //

//-----------------------------------------------
//-----------------------------------------------
void CPlayerCL::updateVisualPropertyPvpMode(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	CCharacterCL::updateVisualPropertyPvpMode(gameCycle, prop);
	// Additionaly, if i am the target, inform interface of the change
	if(isTarget())
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CCDBNodeLeaf *pDB= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:TRACK_TARGET_PVP_CHANGE_MODE");
		if(pDB)
		{
			sint32	val= pDB->getValue32();
			pDB->setValue32(val+1);
		}
	}
}


//-----------------------------------------------
// skin :
// Get The Entity Skin
//-----------------------------------------------
sint CPlayerCL::skin() const	// virtual
{
	return _PlayerSheet->Skin;
}// skin //

//-----------------------------------------------
// people :
// Return the People for the entity.
//-----------------------------------------------
EGSPD::CPeople::TPeople CPlayerCL::people()	const// virtual
{
	if(_PlayerSheet)
		return _PlayerSheet->People;
	else
		return EGSPD::CPeople::Unknown;
}// people //

//-----------------------------------------------
// people :
// Setup the People for the entity.
//-----------------------------------------------
void CPlayerCL::setPeople(EGSPD::CPeople::TPeople /* people */)
{
}// people //

//-----------------------------------------------
// drawName :
// Draw the name.
//-----------------------------------------------
void CPlayerCL::drawName(const NLMISC::CMatrix &mat)	// virtual
{
	// Draw the name.
	if(!getEntityName().empty())
		TextContext->render3D(mat, getEntityName());
}// drawName //


//-----------------------------------------------
// getFace :
// Update eyes blink. For the moment, called by updatePos.
//-----------------------------------------------
CEntityCL::SInstanceCL *CPlayerCL::getFace()
{
	// Implemented in CPlayerCL
	return &_Face;
}// getFace //

//-----------------------------------------------
// attackRadius :
// Method to return the attack radius of an entity (take the scale into account).
//-----------------------------------------------
double CPlayerCL::attackRadius() const	// virtual
{
	return 0.5;
}// attackRadius //

//-----------------------------------------------
// Return the position the attacker should have to combat according to the attack angle.
// \param ang : 0 = the front, >0 and <Pi = left side, <0 and >-Pi = right side.
//-----------------------------------------------
CVectorD CPlayerCL::getAttackerPos(double ang, double dist) const
{
	// Compute the local angle
	ang = computeShortestAngle(atan2(front().y, front().x), ang);
	ang += Pi;
	if(ang > Pi)
		ang -= 2*Pi;

	// Compute the local position.
	CVectorD p;
	p.x = 0.5 * sin(-ang) + dist*sin(-ang);	// or: pos.x = _Sheet->DistToSide*cos(ang) + dist*cos(ang); but 0 should be right side.
	p.y = 0.5 * cos(ang)  + dist*cos(ang);
	p.z = 0.0;

	// Compute the world position.
	// Create the target matrix.
	CVector vj = front();
	vj.z = 0;
	CVector vk(0,0,1);
	CVector vi = vj^vk;
	CMatrix bodyBase;
	bodyBase.setRot(vi,vj,vk,true);
	bodyBase.setPos(pos());

	// Get the destination in the world.
	return bodyBase * p;
}// getAttackerPos //


///////////////
// 3D SYSTEM //
///////////////
//-----------------------------------------------
// updateAsyncTexture
//-----------------------------------------------
float	CPlayerCL::updateAsyncTexture()
{
	// Call parent.
	float	distToCam= CCharacterCL::updateAsyncTexture();

	// Check all instance to know if they need to start async load their textures
	if(!_Face.Loading.empty())
	{
		// dirty?
		if(_Face.Loading.isAsyncTextureDirty())
		{
			// reset instance state.
			_Face.Loading.setAsyncTextureDirty(false);
			// must start loading for this isntance
			_Face.Loading.startAsyncTextureLoading();
			// the entity is now currently loading.
			_PlayerCLAsyncTextureLoading= true;
			// The LodTexture need to be recomputed
			_LodTextureDirty= true;
		}
	}


	// Update Async Texture loading of all instances.
	if(_PlayerCLAsyncTextureLoading)
	{
		bool	allLoaded= true;
		// update loading for all instances.
		if(!_Face.Loading.empty())
		{
			// update async texture loading
			allLoaded= allLoaded && _Face.Loading.isAsyncTextureReady();
		}

		// if all are loaded, then End! don't need to check all instances every frame.
		if(allLoaded)
		{
			_PlayerCLAsyncTextureLoading= false;
			_Face.updateCurrentFromLoading(_Skeleton);
		}
	}


	// For LOD texture, must update the "texture distance"
	if(!_Face.Current.empty())
	{
		// update async texture loading
		_Face.Current.setAsyncTextureDistance(distToCam);
	}

	return distToCam;
}

//-----------------------------------------------
// updateLodTexture
//-----------------------------------------------
void	CPlayerCL::updateLodTexture()
{
	// if need to recompute, and if Async loading ended
	if( _LodTextureDirty && !_PlayerCLAsyncTextureLoading )
		// check parent and upadte lod
		CCharacterCL::updateLodTexture();
}

//-----------------------------------------------
// getMaxSpeed :
// Return the basic max speed for the entity in meter per sec
//-----------------------------------------------
double CPlayerCL::getMaxSpeed()	const// virtual
{
	return 6.0f;
}// getMaxSpeed //



//---------------------------------------------------
// displayDebug :
// Display Debug Information.
//---------------------------------------------------
void CPlayerCL::displayDebug(float x, float &y, float lineStep)	// virtual
{
	CCharacterCL::displayDebug(x, y, lineStep);
}// displayDebug //






//---------------------------------------------------
// readWrite :
// Read/Write Variables from/to the stream.
//---------------------------------------------------
void CPlayerCL::readWrite(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CCharacterCL::readWrite(f);

	// PUBLIC

	// PROTECTED
//	const CPlayerSheet		*_Sheet;
//	const CRaceStatsSheet	*_PlayerSheet;
//	NL3D::UInstance			_Face;
	f.serial(_DefaultChest);
	f.serial(_DefaultLegs);
	f.serial(_DefaultArms);
	f.serial(_DefaultHands);
	f.serial(_DefaultFeet);
	f.serial(_DefaultHair);
	f.serial(_HairColor);
	f.serial(_EyesColor);
	f.serial(_WaitForAppearance);
	f.serial(_PlayerCLAsyncTextureLoading);
	f.serial(_LightOn);
//	NL3D::UPointLight		_Light;

	// PRIVATE
}// readWrite //

//---------------------------------------------------
// load :
// To call after a read from a stream to re-initialize the entity.
//---------------------------------------------------
void CPlayerCL::load()	// virtual
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();

	// If the entity should be in the world already
	if(_First_Pos == false)
	{
		// Insert the primitive into the world.
		if(_Primitive)
			_Primitive->insertInWorldImage(dynamicWI);
		// Insert the entity into PACS
		pacsPos(pos());
	}

	// update
	if(!_WaitForAppearance)
	{
		// Visual properties A
		sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", _Slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->getValue64();
		updateVisualPropertyVpa(0, prop);	// Vpa udapte vpb and vpc too.
	}
}// load //

// *********************************************************************************************
const char *CPlayerCL::getBoneNameFromBodyPart(BODY::TBodyPart part, BODY::TSide side) const
{
	if (!_PlayerSheet) return CCharacterCL::getBoneNameFromBodyPart(part, side);
	return _PlayerSheet->BodyToBone.getBoneName(part, side);
}

// *********************************************************************************************
const CItemSheet *CPlayerCL::getRightHandItemSheet() const
{
	return _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet;
}

// *********************************************************************************************
const CItemSheet *CPlayerCL::getLeftHandItemSheet() const
{
	return _Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet;
}

// *********************************************************************************************
const CAttack *CPlayerCL::getAttack(const CAttackIDSheet &id) const
{
	if (!_PlayerSheet) return NULL;
	return CCharacterCL::getAttack(id, _PlayerSheet->AttackLists);
}

// *********************************************************************************************
float CPlayerCL::getScaleRef() const
{
	float fyrosRefScale = GabaritSet.getRefHeightScale(0, EGSPD::CPeople::Fyros);
	if (fyrosRefScale == 0) return 1.f;
	return _CustomScalePos * (GabaritSet.getRefHeightScale(_Gender, people()) / fyrosRefScale);

}

// *********************************************************************************************
float CPlayerCL::getNamePosZ() const
{
	if (!_PlayerSheet)
		return 0.f;

	float namePosZ;
	switch (_ModeWanted)
	{
	case MBEHAV::DEATH:
	case MBEHAV::SIT:
		namePosZ = _PlayerSheet->GenderInfos[_Gender].NamePosZLow;
		break;

	case MBEHAV::MOUNT_NORMAL:
	case MBEHAV::MOUNT_SWIM:
		namePosZ = _PlayerSheet->GenderInfos[_Gender].NamePosZHigh;
		break;

	default:
		namePosZ = _PlayerSheet->GenderInfos[_Gender].NamePosZNormal;
		break;
	}

	return namePosZ * _CharacterScalePos * _CustomScalePos;
}

// ***************************************************************************
void CPlayerCL::doSetVisualSelectionBlink(bool bOnOff, NLMISC::CRGBA emitColor)
{
	// Do it on Face
	if(bOnOff)
		_Face.setEmissive(emitColor);
	else
		_Face.restoreEmissive();

	// and parent call
	CCharacterCL::doSetVisualSelectionBlink(bOnOff, emitColor);
}



//-----------------------------------------------
// computePrimitive :
// Create (or re-create) a primitive.
//-----------------------------------------------
void CPlayerCL::computePrimitive()
{
	// Initialize the primitive.
	initPrimitive(0.5f, 2.0f, 0.0f, 0.0f, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, MaskColPlayer, MaskColNone);
	if(_Primitive)
		_Primitive->insertInWorldImage(dynamicWI);
	// Set the position.
	pacsPos(pos());
}// computePrimitive //

