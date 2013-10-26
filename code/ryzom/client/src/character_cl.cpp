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



/*
 *	Update character position notes
 *	-------------------------------
 *
 *	The update of the character position is done as followed :
 *
 *	For each character
 *		updatePreCollision
 *			updatePos
 *				PACS::move
 *
 *	The characters know where they want to be in 2d (pos().x and pos().y valid).
 *
 *	PACS::evalCollision
 *
 *	For each character
 *		updatePostCollision
 *			pacsFinalizepos
 *				PACS::getGlobalPosition
 *				If real position too far from PACS position
 *					PACS::setGlobalPosition
 *			-- Here pos().z is estimated
 *			snapToGround
 *				NL3D::visualCollisionEntities
 *				-- Here pos().z is good
 *			updateDisplay
 *			updateCluster
 *			updateHeadDirection
 *			-- Here the character is setuped in the engine
 *
 */

/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Client.
#include "character_cl.h"
#include "pacs_client.h"
#include "net_manager.h"
#include "entity_animation_manager.h"
#include "time_client.h"
#include "ingame_database_manager.h"
#include "client_chat_manager.h"
#include "motion/user_controls.h"
#include "color_slot_manager.h"
#include "sheet_manager.h"
#include "debug_client.h"
#include "animation_misc.h"
#include "entities.h"						// \todo GUIGUI : try to remove this dependency.
#include "misc.h"
#include "string_manager_client.h"
#include "interface_v3/interface_manager.h"
#include "fx_manager.h"
#include "interface_v3/group_in_scene_user_info.h"
#include "interface_v3/group_in_scene_bubble.h"
#include "client_cfg.h"
#include "user_entity.h"
#include "projectile_manager.h"
#include "init_main_loop.h"
#include "nel/misc/cdb_branch.h"
#include "animation_fx_misc.h"
#include "attack_list.h"
#include "animation_fx_id_array.h"
#include "cursor_functions.h"
#include "interface_v3/bar_manager.h"
// sheets
#include "client_sheets/item_fx_sheet.h"
#include "client_sheets/attack_id_sheet.h"
// Misc
#include "nel/misc/geom_ext.h"
#include "nel/misc/random.h"
// 3D
#include "nel/3d/u_scene.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_bone.h"
#include "nel/3d/u_track.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_instance_material.h"
// PACS
#include "nel/pacs/u_global_position.h"
// SOUND
#include "nel/sound/sound_anim_manager.h"
// Std.
#include <vector>
// Game share
#include "game_share/intensity_types.h"
#include "game_share/multi_target.h"
#include "game_share/visual_fx.h"
#include "game_share/range_weapon_type.h"
//


////////////
// DEFINE //
////////////
#define INVALID_POS		CVectorD(-1.0,-1.0,-1.0)
#define INVALID_DIST	-1.0
#define INVALID_TIME	-1.0
//#define MAX_HEAD_H_ROTATION		Pi/2.5
#define MAX_HEAD_H_ROTATION		Pi/3.0
#define MAX_HEAD_V_ROTATION		Pi/4.0
#define DEFAULT_BLEND_LENGTH	0


static const double AURA_SHUTDOWN_TIME = 1.f; // number of seconds for auras and links to shutdown

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;
using namespace NLSOUND;
using namespace std;
using namespace MBEHAV;
using namespace CLFECOMMON;


////////////
// EXTERN //
////////////
extern UScene					*Scene;
extern UDriver					*Driver;
extern CEntityAnimationManager	*EAM;
extern CClientChatManager		ChatMngr;
extern UTextContext				*TextContext;
extern UMaterial				GenericMat;
extern UCamera					MainCam;


///////////
// MACRO //
///////////
#if !FINAL_VERSION
	std::string LastMethod;

	#define ADD_METHOD(header) \
	header                     \
	{                          \
		LastMethod = #header;

	#define CHECK(param)                                                            \
	if((param)==false)                                                              \
	{                                                                               \
		nlwarning("entity:%d: Test '%s'", _Slot, #param);                           \
		nlwarning("entity:%d: Last Method called '%s'", _Slot, LastMethod.c_str()); \
		nlstop;                                                                     \
	}
	#define METHOD_NAME(param) LastMethod = (param);

#else
	#define ADD_METHOD(header) \
	header                     \
	{
	#define CHECK(param)
	#define METHOD_NAME(param)
#endif



////////////
// STATIC //
////////////
const std::string CCharacterCL::_EmptyString = "";
const uint8	 CCharacterCL::_BadHairIndex = 0xFF;

H_AUTO_DECL ( RZ_Client_Character_CL_Update_Pos_Combat_Float )
H_AUTO_DECL ( RZ_Client_Entity_CL_Update_Pos_Pacs )
H_AUTO_DECL ( RZ_Client_Entity_CL_Update_Pos_Combat_Float )
H_AUTO_DECL ( RZ_Client_Entity_CL_Update_Pos_Compute_Motion )

/////////////
// METHODS //
/////////////


//---------------------------------------------------
// dirEndAnim :
// Set the direction that should have the character at the end of the animation.
// \param vect : vector used to set the direction at the end of the animation.
//---------------------------------------------------
void CCharacterCL::dirEndAnim(const CVector &vect)
{
	setVect(_DirEndAnim, vect, true, true);
}// dirEndAnim //


//-----------------------------------------------
// CCharacterCL :
// Constructor.
//-----------------------------------------------
CCharacterCL::CCharacterCL()
: CEntityCL()
{
	Type				= NPC;

	_FirstPos			= INVALID_POS;	// Initialize the first with a bad position.
	_FirstTime			= INVALID_TIME;	// Initialize the time for the first position with a bad one.
	dist2FirstPos(INVALID_DIST);		// Initialize the distance to the first position with a bad value.
	_RunStartTimeNoPop	= INVALID_TIME;

	_DestPos			= INVALID_POS;
	_DestTime			= INVALID_TIME;
	dist2Dest(INVALID_DIST);

	_OldPos				= INVALID_POS;
	_OldPosTime			= INVALID_TIME;


	// Initialize the time for the last loop with the current time when entity created.
	_LastFrameTime = 0.0;
	// The animation should be played from the begin to the end.
	_AnimReversed.resize(animTypeCount, false);
	// Id among all the animations for each slot
	_AnimId.resize(animTypeCount, NL3D::UPlayList::empty);
	// Index in the state of the current animation for each slot.
	_AnimIndex.resize(animTypeCount, CAnimation::UnknownAnim);
	// ID of the current sound animation for each slot.
	_SoundId.resize(animTypeCount, -1);
	// ID of the current animation state for each slot.
	_AnimState.resize(animTypeCount, CAnimationStateSheet::Idle);
	// Time offest in the current animation for each slot.
	_AnimOffset.resize(animTypeCount, 0.0);
	// Subsidiary Key for the Animation State (emote).
	_SubStateKey = CAnimationStateSheet::UnknownState;
	// The character does not accept special "job animation" by default
	_AnimJobSpecialisation= 0;

	// Reset Lod.
	_LodCharacterAnimEnabled= false;
	_LodCharacterMasterAnimSlot= MOVE;

	// default POS scale to 1.
	_CharacterScalePos= 1.f;

	// No sheet pointed.
	_Sheet = 0;

	// Unknown gender at the entity creation.
	_Gender = GSGENDER::unknown;

	// The bone for the name is not known for the time
	_NameBoneId = -1;
	// No UTransform for the name needed if there is no name so not allocated for the time.
	_NameTransform = 0;
	// default Clod apparition => force compute the bone
	_NameCLodDeltaZ = NameCLodDeltaZNotComputed;

	// There is no anim set for the time.
	_CurrentAnimSet.resize(animTypeCount, 0);

	// Same as the animation at the beginning.
	_RotationFactor = 1.f;

	_CurrentState = 0;


	_RightFXActivated	= false;
	_LeftFXActivated	= false;


	dirEndAnim(CVector(0.f, 1.f, 0.f));

	// No item associated at the beginning but there is a room for them.
	_Items.resize(SLOTTYPE::NB_SLOT);
	_HeadIdx = CEntityCL::BadIndex;
	_FaceIdx = CEntityCL::BadIndex;

	// No frame remaining forthe blend at the beginning.
	_BlendRemaining = 0;

	// Scale for the skeleton according to the gabarit. Default : 1
	_CustomScalePos = 1.f;

	// Start with "unknown mode wanted by the server"
	_ModeWanted = MBEHAV::UNKNOWN_MODE; //MBEHAV::NORMAL;

	_Mount = CLFECOMMON::INVALID_SLOT;
	_Rider = CLFECOMMON::INVALID_SLOT;
	_TheoreticalMount = CLFECOMMON::INVALID_SLOT;
	_TheoreticalRider = CLFECOMMON::INVALID_SLOT;
	_OwnerPeople = MOUNT_PEOPLE::Unknown;

	// Default is : entity has no bone for the head and Neck.
	_HeadBoneId = -1;

	_IsThereAMode = false;
	_ImportantStepTime= 0.0;
	_StartDecreaseLCTImpact= 0;

	// Entity has no look and so is not displayable for the time.
	_LookRdy = false;

	// Index of the instance in the right hand (0xFFFFFFFF = no index).
	_RHandInstIdx = CEntityCL::BadIndex;
	// Index of the instance in the left hand (0xFFFFFFFF = no index).
	_LHandInstIdx = CEntityCL::BadIndex;

	_HairColor = 0;
	_EyesColor = 0;
	// No Hair Index at the beginning.
	_HairIndex = _BadHairIndex;
	_ClothesSheet = 0;

	_NbLoopAnim = 0;
	_MaxLoop = false;

	setAlive();

	_InSceneUserInterface = NULL;
	_CurrentBubble = NULL;

	// Initialize the head offset with a Null Vector.
	_HeadOffset = CVector::Null;
	_HeadOffsetComputed = false;
	// Initialize the Run Factor
	runFactor(0.0);
	// Initialize the Speed
	speed(0.0);



	_CurrentAttack = NULL;
	_CurrentAttackID.Type = CAttackIDSheet::Unknown;

	//_PelvisBoneId = -1;

	_ChestBoneId = -1;


	_HideSkin = false;


	_GuildNameId = 0;
	_GuildSymbol = 0;

	_EventFactionId = 0;
	_PvpMode = PVP_MODE::None;

	_LeagueId = 0;
	_OutpostId = 0;
	_OutpostSide = OUTPOSTENUMS::UnknownPVPSide;

	_SelectableBySpace = true;

	_LastSelectBoxComputeTime= 0;



	_CustomScale = 1.f;
}// CCharacterCL //

//-----------------------------------------------
// ~CCharacterCL:
// Default Destructor
// \warning : Do not remove sheets before the entity.
//-----------------------------------------------
CCharacterCL::~CCharacterCL()
{
	// Delete the UTransform used to compute the Bone for the Name.
	if(!_NameTransform.empty())
	{
		if(Scene)
		{
			if (!_Skeleton.empty())
				_Skeleton.detachSkeletonSon(_NameTransform);
			Scene->deleteTransform(_NameTransform);
		}
		_NameTransform = 0;
	}

	// No more sheet pointed.
	_Sheet = NULL;

	// Release items (but not their mesh, because they are managed by _Instances)
	for(uint k = 0; k < _Items.size(); ++k)
	{
		_Items[k].release();
	}

	// Delete previous interface
	releaseInSceneInterfaces();

	if (_CurrentBubble)
	{
		_CurrentBubble->unlink();
	}

}

//-----------------------------------------------
// computePrimitive :
// Create (or re-create) a primitive.
//-----------------------------------------------
void CCharacterCL::computePrimitive()
{
	// Initialize the primitive.
	if (_Sheet)
	{
		initPrimitive(_Sheet->ColRadius*getScale(), _Sheet->ColHeight*getScale(), _Sheet->ColLength, _Sheet->ColWidth, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, MaskColNpc, MaskColNone, _Sheet->ClipRadius, _Sheet->ClipHeight);
	}
	else
	{
		initPrimitive(0.5f, 2.0f, 0.0f, 0.0f, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, MaskColNpc, MaskColNone);
	}
	if(_Primitive)
		_Primitive->insertInWorldImage(dynamicWI);
	// Set the position.
	pacsPos(pos());
}// computePrimitive //


//-----------------------------------------------
void CCharacterCL::removeAllAttachedFX()
{
	_AttachedFXListForCurrentAnim.clear();
	_AttachedFXListToRemove.clear();
	_StaticFX = NULL;
	for(uint k = 0; k < MaxNumAura; ++k)
	{
		_AuraFX[k] = NULL;
	}
	_LinkFX = NULL;
}


//-----------------------------------------------

void CCharacterCL::releaseInSceneInterfaces()
{
	if (_InSceneUserInterface)
	{
		CWidgetManager::getInstance()->unMakeWindow(_InSceneUserInterface);
		if (_InSceneUserInterface->getParent())
		{
			_InSceneUserInterface->getParent()->delGroup(_InSceneUserInterface);
		}
		else
		{
			delete _InSceneUserInterface;
		}

		_InSceneUserInterface = NULL;
	}
}

//-----------------------------------------------
// stopAttachedFXForCurrrentAnim
// Stop all attached fxs linked  to current animation
//-----------------------------------------------
void CCharacterCL::stopAttachedFXForCurrrentAnim(bool stopLoopingFX)
{
	// shutdown fxs for current anim
	std::list<CAttachedFX::TSmartPtr>::iterator itAttachedFx = _AttachedFXListForCurrentAnim.begin();
	while(itAttachedFx != _AttachedFXListForCurrentAnim.end())
	{
		std::list<CAttachedFX::TSmartPtr>::iterator tmpItAttached = itAttachedFx;
		++itAttachedFx;
		if (!(!stopLoopingFX && (*tmpItAttached)->AniFX && (*tmpItAttached)->AniFX->Sheet->RepeatMode == CAnimationFXSheet::Loop)) // dont remove looping fx if it is requested
		{
			// test if emitters should be shutdown at the end of the anim
			if ((*tmpItAttached)->AniFX &&
				(*tmpItAttached)->AniFX->Sheet->RepeatMode != CAnimationFXSheet::Respawn &&
				(*tmpItAttached)->StickMode != CFXStickMode::SpawnPermanent
			   )
			{
				if(!(*tmpItAttached)->FX.empty())
				{
					if (!(*tmpItAttached)->FX.removeByID("STOP") && !(*tmpItAttached)->FX.removeByID("main"))
					{
						(*tmpItAttached)->FX.activateEmitters(false);
					}
				}
			}
			_AttachedFXListToRemove.splice(_AttachedFXListToRemove.begin(), _AttachedFXListForCurrentAnim, tmpItAttached);
			_AttachedFXListToRemove.front()->TimeOutDate += TimeInSec; // compute absolute timeout date
		}
	}
	/** Removes fx that lives on more that a given count of animation
	  * Useful if framerate is low to avoid that fx overlap
	  */
	itAttachedFx = _AttachedFXListToRemove.begin();
	while(itAttachedFx != _AttachedFXListToRemove.end())
	{
		std::list<CAttachedFX::TSmartPtr>::iterator tmpItAttachedFX = itAttachedFx;
		++itAttachedFx;
		if ((*tmpItAttachedFX)->AniFX)
		{
			if ((*tmpItAttachedFX)->MaxAnimCount != 0) // if there a limit to the number of animation during which the fx can live ?
			{
				(*tmpItAttachedFX)->MaxAnimCount -= 1;
				if ((*tmpItAttachedFX)->MaxAnimCount == 0)
				{
					// remove the fx
					_AttachedFXListToRemove.erase(tmpItAttachedFX);
				}
			}
		}
	}
}


//-----------------------------------------------
// applyColorSlot :
//-----------------------------------------------
void CCharacterCL::applyColorSlot(SInstanceCL &instance, sint skin, sint userColor, sint hair, sint eyes)
{
	CColorSlotManager::TIntCouple array[4];

	// Skin
	array[0].first = (uint)0; array[0].second = (uint)skin;

	// User Color
	array[1].first = (uint)1; array[1].second = (uint)userColor;

	// Hair Color
	array[2].first = (uint)2; array[2].second = (uint)hair;

	// Eyes Color
	array[3].first = (uint)3; array[3].second = (uint)eyes;

	// Set Values.
	UInstance inst = instance.createLoadingFromCurrent();
	if (!inst.empty())
	{
		instance.setColors(skin, userColor, hair, eyes);
		ColorSlotManager.setInstanceSlot(inst, array, 4);
	}
}// applyColorSlot //


//-----------------------------------------------
// changeColors :
//-----------------------------------------------
void CCharacterCL::changeColors(sint userColor, sint hair, sint eyes, sint part)	// virtual
{
	// Color Just a part of the entity.
	if(part >= 0)
	{
		if((uint)part < _Instances.size())
			applyColorSlot(_Instances[part], skin(), userColor, hair, eyes);
	}
	// Color the whole entity.
	else
	{
		for(uint i=0; i<_Instances.size(); i++)
			applyColorSlot(_Instances[i], skin(), userColor, hair, eyes);
	}
}// changeColors //

//-----------------------------------------------
// addColoredInstance :
//-----------------------------------------------
uint32 CCharacterCL::addColoredInstance(const std::string &shapeName, const std::string &stickPoint, sint texture, uint32 instIdx, sint color)
{
	// Get the instance
	uint32 idx = addInstance(shapeName, stickPoint, texture, instIdx);
	SInstanceCL *instance = idx2Inst(idx);
	if(instance)
		applyColorSlot(*instance, skin(), color, _HairColor, _EyesColor);
	else
		nlwarning("CH::addColoredInstance: cannot create the instance for the shape '%s'.", shapeName.c_str());

	return idx;
}// addColoredInstance //


//-----------------------------------------------
// buildEquipment :
// \param slot: structure of the equipement.
// \param visualSlot: visual slot used by this item.
// \return uint32 : index of the instance or 0xFFFFFFFF.
// \todo GUIGUI : find a better choice to avoid all visualSlot checks
//-----------------------------------------------
uint32 CCharacterCL::buildEquipment(const CCharacterSheet::CEquipment &slot, SLOTTYPE::EVisualSlot visualSlot, sint color, uint32 instIdx)
{
	uint32 idx = CEntityCL::BadIndex;

	// Do something only if the slot is not empty.
	if(slot.getItem().empty())
		return idx;

	sint slotColor = slot.Color;

	// This is a reference on an item (the file is store with an UPPER case so check in UPPER CASE).
	string ext = CFile::getExtension(slot.getItem());
	if((ext == "item") || (ext == "sitem"))
	{
		// IS the item a valid one ?
		CSheetId itemId;
		if(itemId.buildSheetId(NLMISC::strlwr(slot.getItem())))
		{
			// Is it stored in the database ?
			CEntitySheet *entitySheet = SheetMngr.get(itemId);
			if(entitySheet)
			{
				_Items[visualSlot].Sheet = dynamic_cast<CItemSheet *>(entitySheet);
				if(_Items[visualSlot].Sheet)
				{
					const CItemSheet &itemSheet = *(_Items[visualSlot].Sheet);

					// Compute the bind point
					string bindBone;
					switch(visualSlot)
					{
						// Right Hand
					case SLOTTYPE::RIGHT_HAND_SLOT:
						if( itemSheet.ItemType != ITEM_TYPE::MAGICIAN_STAFF )
							bindBone = "box_arme";
						break;
						// Left Hand
					case SLOTTYPE::LEFT_HAND_SLOT:
						// Shields are not stick to the same point.
						if(itemSheet.getAnimSet() == "s")
							bindBone = "Box_bouclier";
						else
							bindBone = "box_arme_gauche";
						break;
					default:
						bindBone = slot.getBindPoint();
						break;
					}

					// Choose the right colour.
					if(color==-1)
					{
						// Color in the item
						if(slotColor < 0)
						{
							// Get the item color
							slotColor = itemSheet.Color;
							// Bad item color -> set to 0
							if(slotColor < 0)
								slotColor = 0;
						}
					}
					else
						slotColor = color;

					idx = createItemInstance(itemSheet, instIdx, visualSlot, bindBone, slot.Texture, slotColor);
				}
				else
					nlwarning("CH::buildEquipment: the sheet '%s' is not an item one.", slot.getItem().c_str());
			}
			else
				nlwarning("CH::buildEquipment: the sheet '%s' is not stored in the database.", slot.getItem().c_str());
		}
		else
			nlwarning("CH::buildEquipment: item '%s' is not in the Sheet Id list.", slot.getItem().c_str());
	}
	// This is a shape.
	else
	{
		if(color==-1)
		{
			if(slotColor < 0)
				slotColor = 0;
		}
		else
			slotColor = color;

		// Get the instance
		idx = addColoredInstance(slot.getItem(), slot.getBindPoint(), slot.Texture, instIdx, slotColor);
	}

	// Return the index.
	return idx;
}// buildEquipment //

//-----------------------------------------------
// computeSomeBoneId :
// Compute the bone for the name, head...
// \warning This method do not check the bone is valid, nor there is a Scene.
//-----------------------------------------------
void CCharacterCL::computeSomeBoneId()
{
	// **** Get the Bone for the name.
	_NameBoneId = _Skeleton.getBoneIdByName("name");
	// Dummy found -> return the name position.
	if(_NameBoneId != -1)
	{
		// Just to force the bone to be compute (else not computed if not used).
		_NameTransform = Scene->createTransform();
		if(!_NameTransform.empty())
			_Skeleton.stickObject(_NameTransform, _NameBoneId);
	}
	// No Bone for the Name.
	else
	{
#if !FINAL_VERSION
		pushDebugStr("The Bone for the name is missing.");
#endif // FINAL_VERSION
	}

	// **** Get the head bone.
	_HeadBoneId = _Skeleton.getBoneIdByName("Bip01 Head");
	// Bone found
	if(_HeadBoneId == -1)
	{
#if !FINAL_VERSION
		pushDebugStr("The Bone for the Head is missing.");
#endif // FINAL_VERSION
	}
	else
	{
		_TargetAnimCtrl.EyePos = CVector::Null;
		_Skeleton.setBoneAnimCtrl(_HeadBoneId, &_TargetAnimCtrl);
	}

	// **** Get the "chest" bone. take spine1.
	_ChestBoneId = _Skeleton.getBoneIdByName("Bip01 Spine1");
	if(_ChestBoneId == -1)
	{
#if !FINAL_VERSION
		pushDebugStr("The Bone for the Chest 'Bip01 Spine1' is missing.");
#endif // FINAL_VERSION
	}
}// computeSomeBoneId //


//-----------------------------------------------
// createPlayList :
// Create the play list for this entity.
//-----------------------------------------------
void CCharacterCL::createPlayList()
{
	// Release the old animation playlist.
	if(_PlayList)
	{
		EAM->deletePlayList(_PlayList);
		_PlayList = 0;
	}

	// Create the new animation playlist.
	_PlayList = EAM->createPlayList();
	if(!_PlayList)
	{
		pushDebugStr("Cannot create a playlist for the entity.");
		return;
	}

	// Initialize the new playlist.
	// MOVE Channel
	_PlayList->setSpeedFactor	(MOVE, 1.f);
	_PlayList->setWrapMode		(MOVE, UPlayList::Clamp);
	// ACTION Channel
	_PlayList->setSpeedFactor	(ACTION, 1.f);
	_PlayList->setWrapMode		(ACTION, UPlayList::Clamp);
}// createPlayList //



//-----------------------------------------------
// getGroundFX :
// retrieve ground fxs for that entity
//-----------------------------------------------
const std::vector<CGroundFXSheet> *CCharacterCL::getGroundFX() const
{
	return &(_Sheet->GroundFX);
}

//-----------------------------------------------
// build :
// Build the entity from a sheet.
//-----------------------------------------------
bool CCharacterCL::build(const CEntitySheet *sheet)	// virtual
{
	// Cast the sheet in the right type.
	_Sheet = dynamic_cast<const CCharacterSheet *>(sheet);
	if(!_Sheet)
	{
		pushDebugStr("This is not a character sheet -> entity not initialized.");
		return false;
	}

	// Type
	Type = (_Sheet->Race >= EGSPD::CPeople::Creature) ? Fauna : NPC;

	// Names
	if (Type == Fauna)
	{
		// Get the fauna name in the sheet
		const ucstring creatureName(STRING_MANAGER::CStringManagerClient::getCreatureLocalizedName(_Sheet->Id));
		if (creatureName.find(ucstring("<NotExist:")) != 0)
			_EntityName = creatureName;
	}
	else
	{
		// Name and title will be send by the server
	}

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

	if (!ClientCfg.Light && !_Sheet->getSkelFilename().empty())
	{
		// Create the Playlist for the entity.
		createPlayList();
	}
	// Compute the first automaton.
	_CurrentAutomaton = automatonType() + "_normal.automaton";

	// Get the Character gender.
	_Gender = (GSGENDER::EGender)_Sheet->Gender;

	// Initialize the internal time.
	_LastFrameTime = ((double)T1) * 0.001;

	// Set the skeleton.
	if(!ClientCfg.Light && !_Sheet->getSkelFilename().empty() && skeleton(_Sheet->getSkelFilename()))
	{
		// Set the skeleton scale.
		skeleton()->setScale(getScale(), getScale(), getScale());

		// Can create all characters except NPC.
		if(isNPC()== false)
		{
			// Eyes Color
			if(_Sheet->EyesColor >= SheetMngr.nbEyesColor())
			{
				if (SheetMngr.nbEyesColor() == 0)
					_EyesColor = 0;
				else
					_EyesColor = rand()%SheetMngr.nbEyesColor();
			}
			else
			{
				_EyesColor = _Sheet->EyesColor;
			}

			// Hair Color
			if(_Sheet->HairColor >= SheetMngr.nbHairColor())
			{
				if (SheetMngr.nbHairColor() == 0)
					_HairColor = 0;
				else
					_HairColor = rand()%SheetMngr.nbHairColor();
			}
			else
			{
				_HairColor = _Sheet->HairColor;
			}

			// -- Dress the character --


			// Top Items
			buildEquipment(_Sheet->Body,	SLOTTYPE::CHEST_SLOT);
			buildEquipment(_Sheet->Arms,	SLOTTYPE::ARMS_SLOT);
			buildEquipment(_Sheet->Hands,	SLOTTYPE::HANDS_SLOT);
			// Bottom Items
			buildEquipment(_Sheet->Legs,	SLOTTYPE::LEGS_SLOT);
			buildEquipment(_Sheet->Feet,	SLOTTYPE::FEET_SLOT);
			// Face
			_FaceIdx = buildEquipment(_Sheet->Face,	SLOTTYPE::FACE_SLOT);

			// -- Manage the Head --
			// Display the helm.
			if(!_Sheet->Head.getItem().empty())
			{
				// Create the Helm.
				_HeadIdx = buildEquipment(_Sheet->Head, SLOTTYPE::HEAD_SLOT, -1, _HeadIdx);
				// Hide the face
				SInstanceCL *pInstFace = getFace();
				if(pInstFace)
				{
					if(!pInstFace->Current.empty())
						pInstFace->Current.hide();
					else
						pInstFace->KeepHiddenWhenLoaded = true;
				}
			}
			// Display the Hair.
			else
			{
				// Create the Hair.
				if(_HairIndex != _BadHairIndex)
					_HeadIdx = buildEquipment(_Sheet->HairItemList[_HairIndex], SLOTTYPE::HEAD_SLOT, -1, _HeadIdx);
				// Display the face.
				SInstanceCL *pInstFace = getFace();
				if(pInstFace)
					if(!pInstFace->Current.empty())
						pInstFace->Current.show();
			}

			// Objects in Hands
			_RHandInstIdx = buildEquipment(_Sheet->ObjectInRightHand, SLOTTYPE::RIGHT_HAND_SLOT, -1, _RHandInstIdx);	// In The Right Hand
			_LHandInstIdx = buildEquipment(_Sheet->ObjectInLeftHand,  SLOTTYPE::LEFT_HAND_SLOT,  -1, _LHandInstIdx);	// In The Left Hand

			// Look is now ready.
			_LookRdy = true;
		}
		// Cannot build as long as the alternative look property not received, so hide the entity.
		else
			skeleton()->hide();

		// Compute the animation set (after weapons are set to choose the right animation set).
		computeAnimSet();
		// Check the animation set is correct.
		if(_CurrentAnimSet[MOVE] == 0)
			pushDebugStr("Bad animation set");

		// Set the animation to idle.
		setAnim(CAnimationStateSheet::Idle);

		// Compute the bone for the name.
		computeSomeBoneId();

		// Compute pelvis bone
		//_PelvisBoneId = _Skeleton.getBoneIdByName("Bip01 Pelvis");

		// Setup Lod Character skeleton and shapes colors, if skeleton exist
		// Get Lod Character Id from the sheet.
		sint clodId = getLodCharacterId(*Scene, _Sheet->getLodCharacterName());
		if(clodId >= 0)
		{
			// Setup Lod Character shapes, if enabled.
			skeleton()->setLodCharacterShape(clodId);
			skeleton()->setLodCharacterDistance(_Sheet->LodCharacterDistance);
		}
	}
	// Instances
	else
	{
		uint32	idx= buildEquipment(_Sheet->Body,	SLOTTYPE::CHEST_SLOT);
		// must set the scale for the BotObject too
		if(idx<_Instances.size())
		{
			float	s= getScale();
			_Instances[idx].setScale(CVector(s,s,s));
		}
	}

	// Setup _CharacterScalePos
	_CharacterScalePos = _Sheet->CharacterScalePos;

	// Adjust the custom scale position according to the entity scale.
	_CustomScalePos *= getScale();

	// Create PACS Primitive.
	initPrimitive(_Sheet->ColRadius*getScale(), _Sheet->ColHeight*getScale(), _Sheet->ColLength, _Sheet->ColWidth, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, MaskColNpc, MaskColNone, _Sheet->ClipRadius, _Sheet->ClipHeight);

	// Compute the element to be able to snap the entity to the ground.
	computeCollisionEntity();

	// Initialize properties of the entity (selectable/attackable/etc.).
	initProperties();

	// copy some properties (special bot objects). before buildInSceneInterface
	_DisplayInRadar= _Sheet->DisplayInRadar;
	_DisplayOSDName= _Sheet->DisplayOSDName;
	_DisplayOSDBars= _Sheet->DisplayOSDBars;
	_DisplayOSDForceOver= _Sheet->DisplayOSDForceOver;
	_Traversable= _Sheet->Traversable;
	_CanTurn = _Sheet->Turn;

	_SelectableBySpace = _Sheet->SelectableBySpace;

	// Rebuild interface
	buildInSceneInterface ();


	initStaticFX();



	// Entity created.
	return true;
}// build //

//-----------------------------------------------
// isKami()
//-----------------------------------------------
bool CCharacterCL::isKami() const
{
	if (!_Sheet)
		return false;
	return (_Sheet->Race == EGSPD::CPeople::Kami);
}

//-----------------------------------------------
// isUnknownRace()
//-----------------------------------------------
bool CCharacterCL::isUnknownRace() const
{
	if (!_Sheet)
		return false;
	return (_Sheet->Race == EGSPD::CPeople::Unknown);
}

//-----------------------------------------------
// getAttackHeight :
// Return the atk height.
// \todo GUIGUI : height relative to attacker instead of global height
//-----------------------------------------------
CCharacterCL::TAtkHeight CCharacterCL::getAttackHeight(CEntityCL *target, BODY::TBodyPart localisation, BODY::TSide side) const
{
	// Check there is a target.
	if(target == 0)
		return CCharacterCL::AtkMiddle;
	// Get the position for a bone.
	float height;
	if(target->getBoneHeight(localisation, side, height))
	{
		// Low
		if(height < 1.0f)
			return CCharacterCL::AtkLow;
		// High
		else if(height > 2.0f)
			return CCharacterCL::AtkHigh;
	}
	// Default is Middle atk.
	return CCharacterCL::AtkMiddle;
}// getAttackHeight //

//-----------------------------------------------
// getBoneHeight :
//-----------------------------------------------
bool CCharacterCL::getBoneHeight(BODY::TBodyPart localisation, BODY::TSide side, float &height) const	// virtual
{
	// If there is no skeleton return false
	if(_Skeleton.empty())
		return false;
	// Get the Bone Name
	const char *boneName = getBoneNameFromBodyPart(localisation, side);
	if(boneName == 0)
		return false;
	// Get the Bone Id
	sint boneId = _Skeleton.getBoneIdByName(std::string(boneName));
	if (boneId == -1)
		return false;
	if(_Skeleton.isBoneComputed(boneId) == false)
		return false;
	NL3D::UBone bone = _Skeleton.getBone(boneId);
	CMatrix BoneMat = bone.getLastWorldMatrixComputed();
	height = (float)(BoneMat.getPos().z-pos().z);
	if(height < 0.0f)
		height = 0.0f;
	else if(height > 10.0f)
		height = 10.0f;
	return true;
}// getBoneHeight //


//-----------------------------------------------
// lookAtItemsInHands :
// Look at items in hands to change the animation set.
// \return true if the mode is a mode where items in hands should be hidden
//-----------------------------------------------
bool CCharacterCL::modeWithHiddenItems() const
{
	return ((ClientCfg.PutBackItems && !isFighting()) || isSit() || _Mode==MBEHAV::SWIM || isRiding() || _Mode==MBEHAV::SWIM_DEATH || _Mode==MBEHAV::REST);
}// lookAtItemsInHands //


//-----------------------------------------------
// automatonType :
//-----------------------------------------------
string CCharacterCL::automatonType() const	// virtual
{
	return _Sheet->getAutomaton();
}// automatonType //

//-----------------------------------------------
// computeAutomaton :
// Compute the current automaton for the entity.
//-----------------------------------------------
void CCharacterCL::computeAutomaton()
{
	_CurrentAutomaton = automatonType() + "_" + NLMISC::strlwr(MBEHAV::modeToString(_Mode)) + ".automaton";
}// computeAutomaton //


//-----------------------------------------------
// computeAnimSet :
// Compute the animation set to use according to weapons, mode and race.
//-----------------------------------------------
void CCharacterCL::computeAnimSet()
{
	if(ClientCfg.Light)
		return;
	// Use the generic method to compute the animation set.
	if(!::computeAnimSet(_CurrentAnimSet[MOVE], _Mode, _Sheet->getAnimSetBaseName(), _Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet, _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet, !modeWithHiddenItems()))
	{
		//nlwarning("CH:computeAnimSet:%d: pb when trying to compute the animset. Sheet Id '%u(%s)'.", _Slot, _SheetId.asInt(), _SheetId.toString().c_str());
	}

}// computeAnimSet //

//-----------------------------------------------
// adjustPI :
// Adjust the Predicted Interval to fix some errors according to the distance.
//-----------------------------------------------
NLMISC::TGameCycle CCharacterCL::adjustPI(float x , float y, float /* z */, const NLMISC::TGameCycle &pI)
{
	NLMISC::TGameCycle adjustedPI = pI;
	if(ClientCfg.RestrainPI && adjustedPI > 0)
	{
		double dist = (x-UserEntity->pos().x)*(x-UserEntity->pos().x) + (y-UserEntity->pos().y)*(y-UserEntity->pos().y);
		// If under 50m check Predicted Interval
		if(dist < 50*50)
		{
			NLMISC::TGameCycle maxPi = (NLMISC::TGameCycle)(sqrt(dist)/5.0)+1;
			if(adjustedPI > maxPi)
			{
				adjustedPI = maxPi;
				if(VerboseAnimSelection && _Slot == UserEntity->selection())
					nlinfo("CH:updtVPPos:%d: dist'%f' PI'%d' newPI'%d'.", _Slot, sqrt(dist), pI, adjustedPI);
			}
		}
	}
	return adjustedPI;
}// adjustPI //

//-----------------------------------------------
// updateVisualPropertyPos :
// Received a new position for the entity.
// \warning Do not send position for the user
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyPos(const NLMISC::TGameCycle &gameCycle, const sint64 &prop, const NLMISC::TGameCycle &pI)
{
	// Check the DB entry (the warning is already done in the build method).
	if(_DBEntry == 0)
		return;
	// Get The property 'Y'.
	CCDBNodeLeaf *nodeY	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSY));
	if(nodeY == 0)
	{
		nlwarning("CH::updtVPPos:%d: Cannot find the property 'PROPERTY_POSY(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSY);
		return;
	}
	// Get The property 'Z'.
	CCDBNodeLeaf *nodeZ	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSZ));
	if(nodeZ == 0)
	{
		nlwarning("CH::updtVPPos:%d: Cannot find the property 'PROPERTY_POSZ(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSZ);
		return;
	}

	// Convert Database into a Position
	float x = (float)(prop)/1000.0f;
	float y = (float)(nodeY->getValue64())/1000.0f;
	float z = (float)(nodeZ->getValue64())/1000.0f;

#ifdef TMP_DEBUG_GUIGUI
	// Theoretical Position
	_TheoreticalPosition = CVectorD((double)x, (double)y, (double)z);
#endif // TMP_DEBUG_GUIGUI

	// First position Managed -> set the PACS Position
	if(_FirstPosManaged)
	{
		pacsPos(CVectorD(x, y, z));
		_FirstPosManaged = false;
		return;
	}

	// Wait for the entity to be spawned
	if(_First_Pos)
		return;

	// Stock the position (except if this is the user mount because it's the user that control him not the server)
	if( !isRiding() || _Rider != 0)
	{
		// Adjust the Predicted Interval to fix some "bug" into the Prediction Algo.
		NLMISC::TGameCycle adjustedPI = adjustPI(x, y, z, pI);
		// Add Stage.
		_Stages.addStage(gameCycle, PROPERTY_POSX, prop, adjustedPI);
		_Stages.addStage(gameCycle, PROPERTY_POSY, nodeY->getValue64());
		_Stages.addStage(gameCycle, PROPERTY_POSZ, nodeZ->getValue64());
	}
}// updateVisualPropertyPos //

//-----------------------------------------------
// updateVisualPropertyOrient :
// Received a new orientation.
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyOrient(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
#ifdef TMP_DEBUG_GUIGUI
	// Backup the last orientation received.
	_TheoreticalOrientation = *(float *)(&prop);
#endif	// TMP_DEBUG_GUIGUI

	// New Mode Received.
	if(verboseVP(this))
	{
		float ori = *(float *)(&prop);
		nlinfo("(%05d,%03d) CH::updateVPOri:%d: '%f' received.", sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot, ori);
	}

	// if no skeleton we set the orientation
	if(_Skeleton.empty())
	{
		// server forces the entity orientation even if it cannot turn
		front(CVector((float)cos(_TheoreticalOrientation), (float)sin(_TheoreticalOrientation), 0.f), true, true, true);
		dir(front(), false, false);
		if(_Primitive)
			_Primitive->setOrientation(_TheoreticalOrientation, dynamicWI);
	}
	else
	{
		if( !isRiding() || _Rider != 0 )
		{
			// Add in right stage.
			_Stages.addStage(gameCycle, PROPERTY_ORIENTATION, prop);
		}
	}
}// updateVisualPropertyOrient //

//-----------------------------------------------
// updateVisualPropertyMode :
// New mode received.
// \warning For the first mode, we must have received the position and orientation (but this should be the case).
// \warning Read the position or orientation from the database when reading the mode (no more updated in updateVisualPropertyPos and updateVisualPropertyOrient).
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyMode(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	if(verboseVP(this))
		nlinfo("(%05d,%03d) CH:updtVPMode:%d: '%s(%d)' received.", sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot, modeToString((MBEHAV::EMode)prop).c_str(), (MBEHAV::EMode)prop);
	// New Mode Received : Set the Theoretical Current Mode if different.
	if(_TheoreticalMode != (MBEHAV::EMode)(prop & 0xffff))
		_TheoreticalMode = (MBEHAV::EMode)(prop & 0xffff);
	else
	{
		nlwarning("CH:updtVPMode:%d: The mode '%s(%d)' sent is the same as the current one.", _Slot, modeToString(_TheoreticalMode).c_str(), _TheoreticalMode);
		return;
	}
	// If it is the first mode, set the mode.
	if(_Mode == MBEHAV::UNKNOWN_MODE)
	{
		// SET THE FIRST POSITION
		//-----------------------
		// Check the DB entry (the warning is already done in the build method).
		if(_DBEntry == 0)
			return;
		// Get The property 'PROPERTY_POSX'.
		CCDBNodeLeaf *nodeX	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSX));
		if(nodeX == 0)
		{
			nlwarning("CH::updtVPMode:%d: Cannot find the property 'PROPERTY_POSX(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSX);
			return;
		}
		// Get The property 'PROPERTY_POSY'.
		CCDBNodeLeaf *nodeY	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSY));
		if(nodeY == 0)
		{
			nlwarning("CH::updtVPMode:%d: Cannot find the property 'PROPERTY_POSY(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSY);
			return;
		}
		// Get The property 'PROPERTY_POSZ'.
		CCDBNodeLeaf *nodeZ	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSZ));
		if(nodeZ == 0)
		{
			nlwarning("CH::updtVPMode:%d: Cannot find the property 'PROPERTY_POSZ(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSZ);
			return;
		}
		// Next position will no longer be the first one.
		_First_Pos = false;
		// Insert the primitive into the world.
		if(_Primitive)
			_Primitive->insertInWorldImage(dynamicWI);
		// float makes a few cm error
		double x = (double)(nodeX->getValue64())/1000.0;
		double y = (double)(nodeY->getValue64())/1000.0;
		double z = (double)(nodeZ->getValue64())/1000.0;
		// Set the primitive position.
		pacsPos(CVectorD(x, y, z));
		// SET THE FIRST ORIENTATION
		//--------------------------
		// Get The property 'PROPERTY_ORIENTATION'.
		CCDBNodeLeaf *nodeOri = dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_ORIENTATION));
		if(nodeOri == 0)
		{
			nlwarning("CH::updtVPMode:%d: Cannot find the property 'PROPERTY_ORIENTATION(%d)'.", _Slot, CLFECOMMON::PROPERTY_ORIENTATION);
			return;
		}
		const sint64 &ori = nodeOri->getValue64();
		float angleZ = *(float *)(&ori);
		// server forces the entity orientation even if it cannot turn
		front(CVector((float)cos(angleZ), (float)sin(angleZ), 0.f), true, true, true);
		dir(front(), false, false);
		_TargetAngle = angleZ;
		if(_Primitive)
			_Primitive->setOrientation(angleZ, dynamicWI);
		// SET THE FIRST MODE
		//-------------------
		// Set the mode Now
		_Mode       = _TheoreticalMode;
		_ModeWanted = _TheoreticalMode;
		if((_Mode == MBEHAV::MOUNT_NORMAL) && (_Rider == CLFECOMMON::INVALID_SLOT))
		{
			_Mode = MBEHAV::NORMAL;
			_ModeWanted = MBEHAV::MOUNT_NORMAL;
			// See also updateVisualPropertyRiderEntity() for the case when _Rider is received after the mode
			computeAutomaton();
			computeAnimSet();
			setAnim(CAnimationStateSheet::Idle);
			// Add the mode to the stage.
			_Stages.addStage(gameCycle, PROPERTY_MODE, prop);
		}
		computeAutomaton();
		computeAnimSet();
		setAnim(CAnimationStateSheet::Idle);
	}
	// Not the first mode -> Add to a stage.
	else
	{
		// Add the mode to the stage.
		_Stages.addStage(gameCycle, PROPERTY_MODE, prop);
		// Float mode push the orientation
		if(_TheoreticalMode == MBEHAV::COMBAT_FLOAT)
		{
			// Get The property 'PROPERTY_ORIENTATION'.
			CCDBNodeLeaf *nodeOri = dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_ORIENTATION));
			if(nodeOri == 0)
			{
				nlwarning("CH::updtVPMode:%d: Cannot find the property 'PROPERTY_ORIENTATION(%d)'.", _Slot, CLFECOMMON::PROPERTY_ORIENTATION);
				return;
			}
			_Stages.addStage(gameCycle, CLFECOMMON::PROPERTY_ORIENTATION, nodeOri->getValue64());
		}
		// Any other mode push the position
		else
		{
			if(_TheoreticalMode != MBEHAV::MOUNT_NORMAL)
			{
				// Check the DB entry (the warning is already done in the build method).
				if(_DBEntry == 0)
					return;
				// Get The property 'PROPERTY_POSX'.
				CCDBNodeLeaf *nodeX	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSX));
				if(nodeX == 0)
				{
					nlwarning("CH::updtVPMode:%d: Cannot find the property 'PROPERTY_POSX(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSX);
					return;
				}
				// Get The property 'PROPERTY_POSY'.
				CCDBNodeLeaf *nodeY	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSY));
				if(nodeY == 0)
				{
					nlwarning("CH::updtVPMode:%d: Cannot find the property 'PROPERTY_POSY(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSY);
					return;
				}
				// Get The property 'PROPERTY_POSZ'.
				CCDBNodeLeaf *nodeZ	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSZ));
				if(nodeZ == 0)
				{
					nlwarning("CH::updtVPMode:%d: Cannot find the property 'PROPERTY_POSZ(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSZ);
					return;
				}
				// Add Stage.
				_Stages.addStage(gameCycle, CLFECOMMON::PROPERTY_POSX, nodeX->getValue64());
				_Stages.addStage(gameCycle, CLFECOMMON::PROPERTY_POSY, nodeY->getValue64());
				_Stages.addStage(gameCycle, CLFECOMMON::PROPERTY_POSZ, nodeZ->getValue64());
			}
		}
	}
}// updateVisualPropertyMode //

//-----------------------------------------------
// updateVisualPropertyBehaviour :
// New Behaviour received.
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyBehaviour(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	// New Behaviour Received.
	CBehaviour beh(prop);
	if(verboseVP(this))
		nlinfo("(%05d,%03d) CH::updateVPBeha:%d: '%s(%d)' received.", sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot, behaviourToString((EBehaviour)beh.Behaviour).c_str(), (sint)beh.Behaviour);

	// Add in right stage.
	_Stages.addStage(gameCycle, PROPERTY_BEHAVIOUR, prop);
}// updateVisualPropertyBehaviour //

void CCharacterCL::updateVisualPropertyTargetList(const NLMISC::TGameCycle &gameCycle, const sint64 &prop, uint listIndex)
{
	// Add in right stage.
	_Stages.addStage(gameCycle, PROPERTY_TARGET_LIST_0 + listIndex, prop);
}

void CCharacterCL::updateVisualPropertyVisualFX(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	_Stages.addStage(gameCycle, PROPERTY_VISUAL_FX, prop);
}
//-----------------------------------------------
// updateVisualPropertyName :
// Received the name Id.
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyName(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// Update the entity name (do not need to be managed with LCT).
	uint32 nameId = *(uint32 *)(&prop);

	// Store the name Id
	_NameId = nameId;

//	STRING_MANAGER::CStringManagerClient::instance()->waitString(nameId, this, &_Name);
	STRING_MANAGER::CStringManagerClient::instance()->waitString(nameId, this);

	//if(!getEntityName().empty())
	//	nlwarning("CH::updateVPName:%d: name Id '%d' received but no name allocated.", _Slot, nameId);
	//else if(verboseVP(this))
	//	nlinfo("(%05d,%03d) CH::updateVPName:%d: name '%s(%d)' received.", sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot, getEntityName().toString().c_str(), nameId);

	updateMissionTarget();
}// updateVisualPropertyName //

//-----------------------------------------------
// updateVisualPropertyTarget :
// Received the new target for the entity
// \todo GUIGUI : should be added in a stage.
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyTarget(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	// New target Received.
	sint targ = (sint)prop;

	if(verboseVP(this))
		nlinfo("(%05d,%03d) CH::updateVPTarget:%d: '%d' received.", sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot, targ);

	// New entity target
	_TargetSlotNoLag = (CLFECOMMON::TCLEntityId)targ;

	// Add in right stage.
	_Stages.addStage(gameCycle, PROPERTY_TARGET_ID, prop);
}// updateVisualPropertyTarget //

//-----------------------------------------------
// updateVisualPropertyVpa :
// Received the new target for the entity
// \todo GUIGUI : should be added in a stage.
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyVpa(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// VPA only useful for NPC
	if(isNPC()==false)
	{
		//nlwarning("CH:updtVPVpa:%d: VPA received but NOT an NPC. Sheet Id '%u(%s)'.", _Slot, _SheetId.asInt(), _SheetId.toString().c_str());
		return;
	}

	// NO SKELETON -> NO VPA
	if(_Skeleton.empty())
		return;

	// Get the alternative look property.
	SAltLookProp altLookProp = *(SAltLookProp *)(&prop);
	// Display debug infos
	if(verboseVP(this))
	{
		nlinfo("(%05d,%03d) CH:updtVPVpa:%d: TopColor(%d) BotColor(%d) RH(%d) LH(%d) Hat(%d) Seed(%d) HairColor(%d)",
			sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot,
			(uint)altLookProp.Element.ColorTop,        (uint)altLookProp.Element.ColorBot,
			(uint)altLookProp.Element.WeaponRightHand, (uint)altLookProp.Element.WeaponLeftHand,
			(uint)altLookProp.Element.Hat,             (uint)altLookProp.Element.Seed,
			(uint)altLookProp.Element.ColorHair);
	}

	// Dress the character.
	if(!_LookRdy)
	{
		// The entity is now visually ready.
		_LookRdy = true;

		// Generate a new random.
		NLMISC::CRandom rnd;
		rnd.srand((sint)altLookProp.Element.Seed);

		// Retrieve the right sheet for clothes.
		_ClothesSheet = _Sheet;
		if(_Sheet->IdAlternativeClothes.size() > 0)
		{
			sint32 num = rnd.rand()%(_Sheet->IdAlternativeClothes.size()+1);
			if(num > 0)
			{
				CSheetId altClothesId(_Sheet->getAlternativeClothes(num-1));
				const CEntitySheet *sheetAlt = SheetMngr.get(altClothesId);
				if(dynamic_cast<const CCharacterSheet *>(sheetAlt))
					_ClothesSheet = dynamic_cast<const CCharacterSheet *>(sheetAlt);
			}
		}

		// Eyes Color
		if(_Sheet->EyesColor >= SheetMngr.nbEyesColor())
		{
			if (SheetMngr.nbEyesColor() == 0)
				_EyesColor = 0;
			else
				_EyesColor = (sint8)(rnd.rand()%SheetMngr.nbEyesColor());
		}
		else
			_EyesColor = _Sheet->EyesColor;
		// Hair Color
		if (SheetMngr.nbHairColor() == 0)
			_HairColor = 0;
		else
			_HairColor = (sint8)altLookProp.Element.ColorHair%SheetMngr.nbHairColor();
		// Hair Index
		if(_Sheet->HairItemList.size() > 0)
		{
			sint32 num = rnd.rand()%_Sheet->HairItemList.size();
			if(num>=0 && num <_BadHairIndex)
				_HairIndex = (uint8)num;
			else
				nlwarning("CH:updtVPVpa:%d: Bad Hair Index '%d'", _Slot, num);
		}

		// -- Dress the character -- (all parts that should not change)
		/** tmp : remove all fx item
		  * \TODO delete an item only if changed
		  */
		buildEquipment(_ClothesSheet->Body,		SLOTTYPE::CHEST_SLOT,	altLookProp.Element.ColorTop);		// Chest
		buildEquipment(_ClothesSheet->Arms,		SLOTTYPE::ARMS_SLOT,	altLookProp.Element.ColorArm);		// Arms
		buildEquipment(_ClothesSheet->Hands,	SLOTTYPE::HANDS_SLOT,	altLookProp.Element.ColorGlove);	// Gloves
		buildEquipment(_ClothesSheet->Legs,		SLOTTYPE::LEGS_SLOT,	altLookProp.Element.ColorBot);		// Legs
		buildEquipment(_ClothesSheet->Feet,		SLOTTYPE::FEET_SLOT,	altLookProp.Element.ColorBoot);		// Boots
		// Face
		_FaceIdx = buildEquipment(_Sheet->Face,			SLOTTYPE::FACE_SLOT);

		// Entity is now dressed
		skeleton()->show();
	}

	// -- Manage the Head --
	// Display the helm.
	if(altLookProp.Element.Hat!=0 && !_ClothesSheet->Head.getItem().empty())
	{
		// Create the Helm.
		_HeadIdx = buildEquipment(_ClothesSheet->Head, SLOTTYPE::HEAD_SLOT, altLookProp.Element.ColorHair, _HeadIdx);
		// Hide the face.
		SInstanceCL *pInstFace = getFace();
		if(pInstFace)
		{
			if(pInstFace->Current.empty() == false)
				pInstFace->Current.hide();
			else
				pInstFace->KeepHiddenWhenLoaded = true;
		}
	}
	// Display the Hair.
	else
	{
		// Create the Hair.
		if(_HairIndex != _BadHairIndex)
			_HeadIdx = buildEquipment(_Sheet->HairItemList[_HairIndex], SLOTTYPE::HEAD_SLOT, altLookProp.Element.ColorHair, _HeadIdx);
		// Display the face.
		SInstanceCL *pInstFace = getFace();
		if(pInstFace)
			if(!pInstFace->Current.empty())
				pInstFace->Current.show();
	}

	// -- Manage weapons -- (weapons can change)

	// Right Hand
	const CItemSheet *newRightHand = SheetMngr.getItem(SLOTTYPE::RIGHT_HAND_SLOT, (uint)altLookProp.Element.WeaponRightHand);
	if (newRightHand != _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet) // item changed ?
	{
		// Remove the old Item in the right hand
		if(_RHandInstIdx != CEntityCL::BadIndex)
		{
			// remove shape
			_RHandInstIdx = addInstance("", "", -1, _RHandInstIdx);
			// remove fxs
			_Items[SLOTTYPE::RIGHT_HAND_SLOT].release();
		}
		// set new one
		_Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet = newRightHand;
		if (newRightHand)
		{
			if( newRightHand->ItemType != ITEM_TYPE::MAGICIAN_STAFF )
				_RHandInstIdx = createItemInstance(*newRightHand, _RHandInstIdx, SLOTTYPE::RIGHT_HAND_SLOT, "box_arme", -1, -1);
			else
				_RHandInstIdx = createItemInstance(*newRightHand, _RHandInstIdx, SLOTTYPE::RIGHT_HAND_SLOT, "", -1, -1);
		}
	}
	// update fx for right hand (trail may have been activated, or advantage fx)
	if (newRightHand)
	{
		SInstanceCL *instCLRH = idx2Inst(_RHandInstIdx);
		if(instCLRH)
		{
			NL3D::UInstance itemInstance = (!instCLRH->Loading.empty()) ? instCLRH->Loading : instCLRH->Current;
			if (!itemInstance.empty())
			{
				// update fxs
				_Items[SLOTTYPE::RIGHT_HAND_SLOT].enableAdvantageFX(itemInstance);
				if ( _CurrentBehaviour.Behaviour != MBEHAV::EXTRACTING )
					_Items[SLOTTYPE::RIGHT_HAND_SLOT].setTrailSize(altLookProp.Element.RTrail);
			}
		}
	}

	// Left Hand
	const CItemSheet *newLeftHand = SheetMngr.getItem(SLOTTYPE::LEFT_HAND_SLOT, (uint)altLookProp.Element.WeaponLeftHand);
	if (newLeftHand != _Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet) // item changed ?
	{
		// Remove the old Item in the left hand
		if(_LHandInstIdx != CEntityCL::BadIndex)
		{
			// remove shape
			_LHandInstIdx = addInstance("", "", -1, _LHandInstIdx);
			// remove fxs
			_Items[SLOTTYPE::LEFT_HAND_SLOT].release();
		}
		// set new one
		_Items[SLOTTYPE::LEFT_HAND_SLOT].Sheet = newLeftHand;
		if (newLeftHand)
		{
			string bindBone;
			if(newLeftHand->getAnimSet() == "s")
				bindBone = "Box_bouclier";
			else
				bindBone = "box_arme_gauche";
			_LHandInstIdx = createItemInstance(*newLeftHand, _LHandInstIdx, SLOTTYPE::LEFT_HAND_SLOT, bindBone, -1, -1);
		}
	}
	// update fx for left hand (trail may have been activated, or advantage fx)
	if (newLeftHand)
	{
		SInstanceCL *instCLLH = idx2Inst(_LHandInstIdx);
		if(instCLLH)
		{
			NL3D::UInstance itemInstance = (!instCLLH->Loading.empty()) ? instCLLH->Loading : instCLLH->Current;
			if (!itemInstance.empty())
			{
				// update fxs
				_Items[SLOTTYPE::LEFT_HAND_SLOT].enableAdvantageFX(itemInstance);
				_Items[SLOTTYPE::LEFT_HAND_SLOT].setTrailSize((uint) (2 * altLookProp.Element.LTrail));
			}
		}
	}


	// -- Update Animation --  (after all those changes animation could change).
	computeAnimSet();
	setAnim(animState(MOVE));
}// updateVisualPropertyVpb //

//-----------------------------------------------
// updateVisualPropertyVpb :
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyVpb(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// Get the alternative look property.
	SAltLookProp2 altLookProp = *(SAltLookProp2 *)(&prop);
	// Display debug infos
	if(verboseVP(this))
	{
		nlinfo("(%05d,%03d) CH:updtVPVpb:%d: Scale(%d)", sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot,
			(uint)altLookProp.PropertySubData.Scale);
	}
	// Save old scale
	float oldCustomScale = _CustomScale;
	// Set new scale
	if (altLookProp.PropertySubData.Scale==0)
		_CustomScale = 1.f;
	else
		_CustomScale = (float)altLookProp.PropertySubData.Scale/100.f;
	// Apply modification
	_CustomScalePos /= oldCustomScale;
	_CustomScalePos *= _CustomScale;
	// change the scale of the skeleton according to the new people
	USkeleton * skel = skeleton();
	if( skel )
	{
		skel->setScale(getScale(), getScale(), getScale());

		// modify the stick bone scale to not propagate scale to child
		sint boneID = skel->getBoneIdByName("stick_1");
		if( boneID != -1 )
		{
			UBone bone = skel->getBone(boneID);
			CVector newBoneScale = bone.getScale() * oldCustomScale/_CustomScale;
			bone.setScale( newBoneScale );
		}
	}
	// must set the new scale for the BotObject too
	else if(!_Instances.empty())
	{
		float	s= getScale();
		_Instances[0].setScale(CVector(s,s,s));
	}

}// updateVisualPropertyVpb //

//-----------------------------------------------
// updateVisualPropertyEntityMounted :
// Update Entity Mount
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyEntityMounted(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	// New target Received.
	sint mountSlot = (sint)prop;
	_TheoreticalMount = (CLFECOMMON::TCLEntityId)mountSlot;
	if(verboseVP(this))
		nlinfo("(%05d,%03d) CH::updateVPMount:%d: '%d' received.", sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot, mountSlot);

	// Add in right stage.
	_Stages.addStage(gameCycle, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID, prop);
}// updateVisualPropertyEntityMounted //

//-----------------------------------------------
// updateVisualPropertyRiderEntity :
// Update Entity Rider
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyRiderEntity(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	// New target Received.
	sint riderSlot = (sint)prop;
	_TheoreticalRider = (CLFECOMMON::TCLEntityId)riderSlot;
	if(verboseVP(this))
		nlinfo("(%05d,%03d) CH::updateVPRider:%d: '%d' received.", sint32(T1%100000), NetMngr.getCurrentServerTick(), _Slot, riderSlot);

	// Add in right stage.
	_Stages.addStage(gameCycle, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID, prop);
}// updateVisualPropertyRiderEntity //

//-----------------------------------------------
// updateVisualPropertyBars :
// Update Entity Bars
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyBars(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)	// virtual
{
	CBarManager::CBarInfo	barInfo;

	// Encode HP to 7 bits
	barInfo.Score[SCORES::hit_points]	= (sint8)((prop&0x7ff) * 127 / 1023);
	// NB: barInfo are sint8, but no problem, since anything following is 7 bits.
    barInfo.Score[SCORES::stamina]		= (uint8)((prop>>11)&0x7f);
    barInfo.Score[SCORES::sap]			= (uint8)((prop>>18)&0x7f);
    barInfo.Score[SCORES::focus]		= (uint8)((prop>>25)&0x7f);

	// update The Bar manager
	CBarManager	*pBM= CBarManager::getInstance();
	/* ***********
	  WHY gameCycle+1 ?????  (yoyo)
		It's because sometimes I have a bug With target DB update and VP update. This is the scenario
		where I suppose the problem rises:
		tick=320:	EGS::tickUpdate(): player.DBTargetHP.setProp(49)
		tick=321:	EGS::combat update, target ennemy receives a Hit, VPHp=10 => transmitted to client with timestamp=321
					EGS::databaseUpdate(), DB updated, with timestamp=321!!!

		Thus I receives on client:
			first the VP with Hp=10, timestamp=321
			second the DB with Hp=49, timestamp=321 too => replaced => BUG
		NB: DB is typically sent at low frequency by FrontEnd, thus received later on client.

		Since databaseUpdate() is called every 2 ticks, adding +1 to VP timestamps solve easily the problem.

		NB: the problem occurs because tickUpdate() and databaseUpdate() are called typically with 1 tick shift (tickUpdate()
			on even ticks, and databaseUpdate() on odd ticks for instance).

		NB: moreover, tickupdate() is called every 8 (or 16) ticks, and databaseUpdate() every 2 ticks. So there is one more
			possible bug:
			318:	EGS::tickUpdate(): player.DBTargetHP.setProp(49)
			319:	EGS::combat update, target ennemy receives a Hit, VPHp=10 => transmitted to client with timestamp=319
					EGS::databaseUpdate(), BUT decide to send only a small subset of DB (because lot of things to send)
						=> our TargetHP is not updated
			320:	nothing. tickupdate() is not called, since every 8 ticks
			321:	EGS::databaseUpdate(), update TargetHP, with timestamp=321 !!!!! => Bug

		(remind that we cannot store a timestamp for each DB property, else would be too big to store and to send...)

		BTW, this last bug should be very rare, so don't care.
	*********** */
	pBM->updateBars(dataSetId(), barInfo, gameCycle+1,
		CBarManager::HpFlag | CBarManager::StaFlag | CBarManager::SapFlag | CBarManager::FocusFlag);

}// updateVisualPropertyBars //

//-----------------------------------------------
// updateVisualPropertyGuildSymbol :
//
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyGuildSymbol(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	_GuildSymbol = prop;

	// maybe need to rebuild the in scene interface
	if(_InSceneUserInterface && _InSceneUserInterface->needGuildSymbolId())
		buildInSceneInterface();
} // updateVisualPropertyGuildSymbol //

//-----------------------------------------------
// updateVisualPropertyGuildNameID :
//
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyGuildNameID(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	_GuildNameId = uint32(prop);

	// maybe need to rebuild the in scene interface
	if(_InSceneUserInterface && _InSceneUserInterface->needGuildNameId())
		buildInSceneInterface();
} // updateVisualPropertyGuildNameID //

//-----------------------------------------------
// updateVisualPropertyEventFactionID :
//
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyEventFactionID(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// ODD Hack by Ulukyn
	//_EventFactionId = uint32(prop);
	_PvpMode = uint32(prop);

	buildInSceneInterface();

	if (isUser())
	{
		uint i;
		uint numEntity = (uint)EntitiesMngr.entities().size();
		for (i=0; i<numEntity; i++)
		{
			CEntityCL *entity = EntitiesMngr.entity(i);
			if (entity)
			{
				CCharacterCL *character = dynamic_cast<CCharacterCL*>(entity);
				if (character)
				{
					if( character->getPvpMode() != 0 && !character->isUser())
						character->buildInSceneInterface ();
				}
			}
		}
	}
} // updateVisualPropertyEventFactionID //

//-----------------------------------------------
// updateVisualPropertyPvpMode :
//
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyPvpMode(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	//_PvpMode = uint32(prop);
	//buildInSceneInterface();

} // updateVisualPropertyPvpMode //

//-----------------------------------------------
// updateVisualPropertyPvpClan :
//
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyPvpClan(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	_LeagueId = uint32(prop);

	buildInSceneInterface();

	if (isUser())
	{
		uint i;
		uint numEntity = (uint)EntitiesMngr.entities().size();
		for (i=0; i<numEntity; i++)
		{
			CEntityCL *entity = EntitiesMngr.entity(i);
			if (entity)
			{
				CCharacterCL *character = dynamic_cast<CCharacterCL*>(entity);
				if (character)
				{
					if( character->getPvpMode() != 0 && !character->isUser())
						character->buildInSceneInterface ();
				}
			}
		}
	}
} // updateVisualPropertyPvpClan //

//-----------------------------------------------
// updateVisualPropertyStatus :
// Update Entity Status
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyStatus(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */)	// virtual
{
	nlinfo("CH:updtVPStatus:%d: received.", _Slot);
}// updateVisualPropertyStatus //

//-----------------------------------------------
// updateVisualPropertyContextual :
// Update Entity Status
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyContextual(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	bool	precAttackable= _Properties.attackable();

	// call parent
	CEntityCL::updateVisualPropertyContextual(gameCycle, prop);

	// if attack modified, and npc/fauna, must rebuild the in scene interface,
	// cause sheets 'Attackable' property not always correclty filled
	if( (isNPC()||isFauna()) && precAttackable!=_Properties.attackable())
		buildInSceneInterface();
}


//-----------------------------------------------
// updateVisualPropertyOwnerPeople :
//
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyOwnerPeople(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	if( _OwnerPeople != MOUNT_PEOPLE::TMountPeople(prop) )
	{
		// reset scale pos
		float oldPeopleScaleFactor;
		switch( _OwnerPeople )
		{
			case MOUNT_PEOPLE::Fyros : oldPeopleScaleFactor = ClientCfg.FyrosScale; break;
			case MOUNT_PEOPLE::Matis : oldPeopleScaleFactor = ClientCfg.MatisScale; break;
			case MOUNT_PEOPLE::Tryker : oldPeopleScaleFactor = ClientCfg.TrykerScale; break;
			case MOUNT_PEOPLE::Zorai : oldPeopleScaleFactor = ClientCfg.ZoraiScale; break;
			default:
				oldPeopleScaleFactor = 1.f;
		}
		_CustomScalePos /= oldPeopleScaleFactor;

		// set the new scale pos
		float newPeopleScaleFactor;
		_OwnerPeople = MOUNT_PEOPLE::TMountPeople(prop);
		switch( _OwnerPeople )
		{
			case MOUNT_PEOPLE::Fyros : newPeopleScaleFactor = ClientCfg.FyrosScale; break;
			case MOUNT_PEOPLE::Matis : newPeopleScaleFactor = ClientCfg.MatisScale; break;
			case MOUNT_PEOPLE::Tryker : newPeopleScaleFactor = ClientCfg.TrykerScale; break;
			case MOUNT_PEOPLE::Zorai : newPeopleScaleFactor = ClientCfg.ZoraiScale; break;
			default:
				newPeopleScaleFactor = 1.f;
		}
		_CustomScalePos *= newPeopleScaleFactor;

		// change the scale of the skeleton according to the new people
		USkeleton * skel = skeleton();
		if( skel )
		{
			skel->setScale(getScale(), getScale(), getScale());

			// modify the stick bone scale to not propagate scale to child
			sint boneID = skel->getBoneIdByName("stick_1");
			if( boneID != -1 )
			{
				UBone bone = skel->getBone(boneID);
				CVector newBoneScale = bone.getScale() * oldPeopleScaleFactor/newPeopleScaleFactor;
				bone.setScale( newBoneScale );
			}
		}
	}

} // updateVisualPropertyOwnerPeople //



//-----------------------------------------------
// updateVisualPropertyOutpostInfos :
//
//-----------------------------------------------
void CCharacterCL::updateVisualPropertyOutpostInfos(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	_OutpostId = ((uint16)prop)&0x7FFF;
	uint16 side = (((uint16)prop)&0x8000)>>15;
	_OutpostSide = (OUTPOSTENUMS::TPVPSide)side;

	nldebug("<CCharacterCL::updateVisualPropertyOutpostInfos> prop = %d, id=%d side=%d",(uint16)prop,_OutpostId,_OutpostSide);

	buildInSceneInterface();

} // updateVisualPropertyOutpostInfos //


//-----------------------------------------------
// skin :
// Get The Entity Skin
//-----------------------------------------------
sint CCharacterCL::skin() const	// virtual
{
	return _Sheet->Skin;
}// skin //


//-----------------------------------------------
// initProperties :
// Initialize properties of the entity (according to the class).
//-----------------------------------------------
void CCharacterCL::initProperties()
{
	properties().selectable(_Sheet->Selectable);
	properties().talkableTo(_Sheet->Talkable);
	properties().attackable(_Sheet->Attackable);
	properties().givable(_Sheet->Givable);
	properties().mountable(_Sheet->Mountable);
	properties().invitable(false);										// You cannot group with a bot.
	properties().afk(false);

	switch(_Sheet->HLState)
	{
	case LHSTATE::LOOTABLE:
		properties().lootable(true);		// You can loot the creature
		properties().harvestable(false);	// You cannot harvest the creature
		break;
	case LHSTATE::HARVESTABLE:
		properties().lootable(false);		// You cannot loot the creature
		properties().harvestable(true);		// You can harvest the creature
		break;
	case LHSTATE::LOOTABLE_HARVESTABLE:
		properties().lootable(true);		// You can loot the creature
		properties().harvestable(true);		// You can harvest the creature
		break;

	default:
		properties().lootable(false);		// You cannot loot the creature
		properties().harvestable(false);	// You cannot harvest the creature
		break;
	}
}// initProperties //


//-----------------------------------------------
// computeTimeStep :
// Compute the elapsed time since last call.
// \param currentTime : current time in sec.
// \return double : elapsed time.
//-----------------------------------------------
double CCharacterCL::computeTimeStep(const double &currentTime)
{
	// Last Time greater than Current Time.
	if(_LastFrameTime > currentTime)
	{
		nlwarning("CCharacterCL::computeTimeStep : Time since last frame is negative (%f). Set _LastFrameTime with currentTime", _LastFrameTime - currentTime);
		_LastFrameTime = currentTime;
	}

	// Time since last Time >= 0
	return currentTime - _LastFrameTime;
}// computeTimeStep //


//-----------------------------------------------
// computeSpeed :
// \todo GUIGUI : to do an average speed, there is a problem if time become very small because small frame or _LastFrameTime increase when looping
//-----------------------------------------------
double CCharacterCL::computeSpeed()
{
	double spd;
	double t = _DestTime - _LastFrameTime;
	if(dist2Dest() <= 0.0)					// Already at destination.
		spd = 0.0;
	else if(_LastFrameTime == 0.0)			// First frame
		spd = 1.0;
	else if(t < 0.0001)	// We should already be at destination.
		spd = 1000.0;//-1.0;
	else
	{
		spd = dist2Dest()/t;
		if(spd>0 && spd<0.001)
			spd = 0.001;
		if(spd > 1000.0)
			spd = 1000.0;
	}
	speed(spd);
	return spd;
}// computeSpeed //

//-----------------------------------------------
// computeSpeedFactor :
// Compute and return the speed factor to apply to the animation.
// \param speedToDest : evaluted speed to destination.
// \return double : the speed factor to use for the current animation.
// \todo GUIGUI : review this scale problem and optimize it.
//-----------------------------------------------
double CCharacterCL::computeSpeedFactor(double speedToDest)
{
	double speedFactor = 1.0;

	// \todo GUIGUI : optimize emotes, currently it's badly designed.
	const CAnimationState *animStatePtr;
	// If the current animation is an emote, get the right animation state.
	if(animState(MOVE) == CAnimationStateSheet::Emote)
		animStatePtr = _CurrentAnimSet[MOVE]->getAnimationState(_SubStateKey);
	else
		animStatePtr = _CurrentAnimSet[MOVE]->getAnimationState(animState(MOVE));
	if(animStatePtr)
	{
		// Get the animation
		const CAnimation *anim = animStatePtr->getAnimation(animIndex(MOVE));
		if(anim)
		{
			// If the animation is a move (not idle, turn, etc.).
			if(_CurrentState)
			{
				// Move : adjust speed factor according to the animation.
				if(_CurrentState->Move)
				{
					// Check for oo speed.
					if(speedToDest != -1)
					{
						// Compute the animation average speed according to the scale.
						double animSpeed = EAM->getAnimationAverageSpeed(anim->id())*getSheetScale()*_CustomScalePos*_CharacterScalePos;
						if(animSpeed > 0.0)
							speedFactor = speedToDest / animSpeed;
						else
							nlwarning("The animation is a move but animation speed is %f !", animSpeed);
					}
					// \todo GUIGUI : unlimited speed, perhaps return a special value.
					// We should be arrived so speed is maximum.
					else
						speedFactor = 1000.0;
				}
				// The current animation is a rotation so adjust the rotation speed according to the angle.
				if(_CurrentState->Rotation && _RotationFactor > 0.0)
				{
					speedFactor /= _RotationFactor;
					speedFactor = std::min(speedFactor, 1.5);
					speedFactor = std::max(speedFactor, 0.5);
				}
				// This animation must be play in a maximum time when there is someting to do after.
				// (must be done after the rotation speed adjustment)
				if(_CurrentState->MaxAnimDuration > 0.00f)
				{
					if(dist2Dest() >= 0.0)
					{
						double animLength = EAM->getAnimationLength(anim->id());
						double speedFactorBackup = speedFactor;
						speedFactor = animLength/_CurrentState->MaxAnimDuration;
						// If the animation speed should have been greater, let it play faster.
						if(speedFactor < speedFactorBackup)
							speedFactor = speedFactorBackup;
					}
				}
				// Panic mode (too late => accelerate)
				if(!_CurrentState->Move && _ImportantStepTime!=0.0)
				{
					const float	beginPanic= 1.5f;		// start panic when too late of 1.5 seconds
					const float endPanic= 5.f;			// max panic factor at 5 seconds of interval
					const float	maxPanicSpeedFactor= 10.f;
					float	t= float(TimeInSec - _ImportantStepTime);
					if(t>beginPanic)
					{
						float	lerp= (t-beginPanic)/(endPanic-beginPanic);
						clamp(lerp, 0.f, 1.f);
						float	panicSF= lerp*maxPanicSpeedFactor;
						if(panicSF>speedFactor)
							speedFactor= panicSF;
					}
				}
				// Special panic mode because there is a position just after
				// NB: don't accelerate animations that can be breaked because of move
				// But still allow acceleration for mode animation transition (_Mode!=_ModeWanted)
				if( !_CurrentState->Move && _RunStartTimeNoPop!=INVALID_TIME &&
					(!_CurrentState->BreakableOnMove || _Mode!=_ModeWanted) )
				{
					const float maxPanicSpeedFactor= 1.5f;
					// compare time of rest of animation, to remain time to the first move
					float	remainAnimTime= float(EAM->getAnimationLength(anim->id()) - animOffset(MOVE));
					remainAnimTime= max(remainAnimTime, 0.f);
					float	panicSF;
					// if already too late, then maximize speed
					if(TimeInSec>=_RunStartTimeNoPop)
					{
						panicSF= maxPanicSpeedFactor;
					}
					// else target the animation speed so it ends at estimated start of move
					else
					{
						panicSF= float(remainAnimTime/(_RunStartTimeNoPop-TimeInSec));
						panicSF= min(panicSF, maxPanicSpeedFactor);
					}
					// only if greater than prec
					if(panicSF>speedFactor)
						speedFactor= panicSF;
				}

				// TestYoyo
				/*
				if(_Slot==WatchedEntitySlot && EntitiesMngr.isLogingStageChange())
				{
					sint64	refLT= EntitiesMngr.getLogStageChangeStartLocalTime();
					NLMISC::createDebug();
					NLMISC::DebugLog->displayRawNL("** Entity %d: (t=%3d) animState %s: %.2f/%.2f. move: %d. rstnp: %d. sf: %.2f",
						(sint32)_Slot, sint32(T1-refLT),
						CAnimationState::getAnimationStateName(animStatePtr->state()).c_str(),
						float(animOffset(MOVE)), float(EAM->getAnimationLength(anim->id())),
						uint(_CurrentState->Move), _RunStartTimeNoPop==INVALID_TIME?-1:sint32(sint64(_RunStartTimeNoPop*1000)-refLT),
						speedFactor
						);
				}
				*/
			}
		}
	}

	// \todo GUIGUI : corriger pour enlever le speed factor min.
	// Adjuste speed factor
	if(speedFactor < 0.5)
		speedFactor = 0.5;

	// Check negative or null speed factor.
	if(speedFactor <= 0.0)
	{
		nlwarning("CCharacterCL::computeSpeedFactor: speedFactor = %f and this should never be <= 0.", speedFactor);
		speedFactor = 1.0;
	}

	// Return the speed factor.
	return speedFactor;
}// computeSpeedFactor //

//-----------------------------------------------
// endAnimTransition :
// Call it at the end of the current animation to choose the next one.
//-----------------------------------------------
void CCharacterCL::endAnimTransition()
{
	// One more animation played.
	_NbLoopAnim++;

	// Hide the entity if needed.
	if(_HideSkin)
		hideSkin();

	// Debug Animation for the selection
	if(VerboseAnimSelection && _Slot == UserEntity->selection())
		nlinfo("CH:endAnimTransition:%d: current animation finished.", _Slot);
	// If the animation is a rotation, set the character direction he should have at the end of the animation.
	if(_CurrentState->Rotation)
	{
		if(isUser())
		{
			nldebug("<CCharacterCL::endAnimTransition> rotation : set dir as end anim dir");
		}

		dir(dirEndAnim());
	}
	// Fit the current direction to the target when attacking.
	if(_CurrentState->Attack)
	{
		dir(front());
	}
	// If user is in range attack and not moving, set dir to target
	if( isUser() )
	{
		if(_CurrentBehaviour.Behaviour == MBEHAV::RANGE_ATTACK && _Mode==MBEHAV::COMBAT && !UserControls.isMoving())
		{
			dir( dirToTarget() );

			float frontYawBefore = frontYaw();
			front(dir());
			float frontYawAfter = frontYaw();
			float	deltaYaw= frontYawAfter - frontYawBefore;
			UserControls.appendCameraDeltaYaw(-deltaYaw);

			if( ClientCfg.AutomaticCamera )
			{
				UserControls.resetSmoothCameraDeltaYaw();
			}
		}
	}

	// If the next mode in the automaton != Current Mode
	if(_CurrentState->NextMode != _Mode)
	{
		if(ClientCfg.UsePACSForAll && _Primitive)
			_Primitive->setCollisionMask(MaskColNone);
		//// AJOUT ////
		switch(_CurrentState->NextMode)
		{
		// Combat
		case MBEHAV::COMBAT:
		case MBEHAV::COMBAT_FLOAT:
			if(ClientCfg.UsePACSForAll && _Primitive)
				_Primitive->setCollisionMask(MaskColPlayer | MaskColNpc | MaskColDoor);	// Collision with player and npc.
			break;
		// Mount
		case MBEHAV::MOUNT_NORMAL:
			parent(_Mount);
			// Remove collisions if needed.
			if(_Mount != CLFECOMMON::INVALID_SLOT)
			{
				if(_Primitive)
					_Primitive->setOcclusionMask(MaskColNone);
			}
			break;
		// Death
		case MBEHAV::DEATH:
			setDead();
			break;
		// UNKNOWN
		case MBEHAV::UNKNOWN_MODE:
			if(ClientCfg.Check)
				nlstop;
			// NO BREAK is Normal here
		// Normal
		case MBEHAV::NORMAL:
//			_Mount = CLFECOMMON::INVALID_SLOT;
//			_Rider = CLFECOMMON::INVALID_SLOT;
			parent(CLFECOMMON::INVALID_SLOT);
			dir(front());
			/*
			front(CVector(1.f, 0.f, 0.f));
			dir(front());
			*/
			break;
		default:
			break;
		}
		// Change the current mode.
		if ( _ModeWanted != MBEHAV::UNKNOWN_MODE )
		{
			_Mode = _CurrentState->NextMode;
		}
		//else
		//	nlinfo( "NO mode change from %u to %u, %s S%hu", _Mode, _CurrentState->NextMode, _SheetId.toString().c_str(), _Slot );
		// Change the current automaton.
		computeAutomaton();
		// Update the animation set according to the new automaton.
		computeAnimSet();
	}
	// Select the Default Next Animation.
	setAnim(_CurrentState->NextState);
}// endAnimTransition //

//-----------------------------------------------
// onMove :
//-----------------------------------------------
CCharacterCL::TOnMove CCharacterCL::onMove(const CAutomatonStateSheet &curAnimState)
{
	// Animation is breakable if the distance to destination is long enough (at least > 0).
	if(curAnimState.BreakableOnMove && dist2Dest()>0.0)
	{
		// \todo GUIGUI : take the next position to current one (it could be possible this position was the same as the first).
		CVectorD dirToFirstPos = _FirstPos-pos();
		dirToFirstPos.z = 0.0;
		if(dirToFirstPos != CVectorD::Null)
			dirToFirstPos.normalize();
		else
		{
			nlwarning("CH:onMove:%d: First pos == pos -> take the dir(%f,%f,%f) instead.", _Slot, dir().x, dir().y, dir().z);
			dirToFirstPos = dir();
		}

		// Compute Angle between the front and the first position.
		double angToDest = computeShortestAngle(atan2(front().y, front().x), atan2(dirToFirstPos.y, dirToFirstPos.x));
		if(!_MaxLoop)
		{
			// Strafe Left
			if(curAnimState.OnMoveLeft     != CAnimationStateSheet::UnknownState)
				if((angToDest>Pi/3.0) && (angToDest<2.0*Pi/3.0))
					return OnMoveLeft;
			// Strafe Right
			if(curAnimState.OnMoveRight    != CAnimationStateSheet::UnknownState)
				if((angToDest<-Pi/3.0) && (angToDest>-2.0*Pi/3.0))
					return OnMoveRight;
			// Backward
			if(curAnimState.OnMoveBackward != CAnimationStateSheet::UnknownState)
				if(fabs(angToDest)>1.92)
					return OnMoveBackward;
		}
		// Forward (default)
		if(curAnimState.OnMoveForward  != CAnimationStateSheet::UnknownState)
		{
//			if(_MaxLoop || fabs(angToDest)<=1.92)
				return OnMoveForward;
		}
		else
			nlwarning("CH:onMove:%d: the current state is breakable on Move and the dist to dest is not Null, but there is no animation to move.", _Slot);
	}
	// No Move
	return OnMoveNone;
}// onMove //

//-----------------------------------------------
// onRotation :
//-----------------------------------------------
CCharacterCL::TOnRot CCharacterCL::onRotation(const CAutomatonStateSheet &curAnimState, CVector &dirEndA)
{
	// Turn and About Face
	if(curAnimState.BreakableOnRotation)
	{
		CVector bodyDir = dir();
		CVector destDir = front();

		// when user is attacking, his dest is his target
		if( isUser() && _CurrentBehaviour.Behaviour == MBEHAV::RANGE_ATTACK && _Mode==MBEHAV::COMBAT && !UserControls.isMoving() )
		{
			destDir = dirToTarget();
		}

		// Compute the angle between the current heading and computed heading.
		double angToDest = computeShortestAngle(atan2(bodyDir.y, bodyDir.x), atan2(destDir.y, destDir.x));
		// Rotation to the left.
		if(angToDest >=  (ClientCfg.AnimatedAngleThreshold*Pi/180.0))
		{
			dirEndA = destDir;
			return OnRotLeft;
		}
		// Rotation to the right.
		if(angToDest <= -(ClientCfg.AnimatedAngleThreshold*Pi/180.0))
		{
			dirEndA = destDir;
			return OnRotRight;
		}
	}
	// No Rot
	return OnRotNone;
}// onRotation //

//-----------------------------------------------
// onBigBend :
//-----------------------------------------------
CCharacterCL::TOnBigBend CCharacterCL::onBigBend(const CAutomatonStateSheet &curAnimState, CVector &dirEndA)
{
	// If the current direction is too different of the direction to the first destination -> play a rotation.
	if(curAnimState.BrkOnBigBend)
	{
		CVector dirToFirstPos;
		if(setVect(dirToFirstPos, _FirstPos - pos(), true, false))
		{
			dirToFirstPos = curAnimState.getMatrix()*dirToFirstPos;
			double angToDest = computeShortestAngle(atan2(dir().y, dir().x), atan2(dirToFirstPos.y, dirToFirstPos.x));
			// Rotation to the left.
			if(angToDest >= 1.5)
			{
				dirEndA = dirToFirstPos;
				return OnBendLeft;
			}
			// Rotation to the right.
			if(angToDest <= -1.5)
			{
				dirEndA = dirToFirstPos;
				return OnBendRight;
			}
			// Smooth dir
			if(fabs(angToDest) > 0.1)
			{
				double newAngle = atan2(dir().y, dir().x) + angToDest/2.0;
				dir(CVector((float)cos(newAngle), (float)sin(newAngle), dir().z));
			}
			else
				dir(dirToFirstPos);
		}
	}
	// No Bend
	return OnBendNone;
}// onBigBend //

//-----------------------------------------------
// onBadHeading :
//-----------------------------------------------
bool CCharacterCL::onBadHeading(const CAutomatonStateSheet &curAnimState)
{
	// Bad Heading
	if(curAnimState.BreakableOnBadHeading && !_MaxLoop)
	{
		CVector dirToFirstPos;
		if(setVect(dirToFirstPos, _FirstPos - pos(), true, false))
		{
			// Compute the angle between the front and the next body direction.
			double angToDest = computeShortestAngle(atan2(front().y, front().x), atan2(dirToFirstPos.y, dirToFirstPos.x));
			if(curAnimState.BadHeadingMin <= curAnimState.BadHeadingMax)
			{
				if((angToDest<curAnimState.BadHeadingMin) || (angToDest>curAnimState.BadHeadingMax))
					return true;
			}
			else
			{
				if((angToDest<curAnimState.BadHeadingMin) && (angToDest>curAnimState.BadHeadingMax))
					return true;
			}
		}
	}
	// No bad Heading
	return false;
}// onBadHeading //

//-----------------------------------------------
// setAnim :
// Select a new animation for the entity.
// \todo GUIGUI : better manage when there is no skeleton
// \todo GUIGUI : simplify head control
//-----------------------------------------------
ADD_METHOD(void CCharacterCL::setAnim(TAnimStateKey newKey, TAnimStateKey subKey, uint animID))
	if(ClientCfg.Light)
		return;

	// Debug Animation for the target
	if(VerboseAnimSelection && _Slot == UserEntity->selection())
		nlinfo("CH:setAnim:%d: state '%s'.", _Slot, CAnimationState::getAnimationStateName (newKey).c_str());


	// RELEASE THE OLD ANIMATION
	TAnimStateId lastAnimStateId = animState(MOVE);	// Bkup the current anim state Id.

	// \todo GUIGUI : Faire une fonction pour savoir si on a changer de type d'animation.
	// Reset loop counter
	if((newKey != animState(MOVE)) || (_OldAutomaton != _CurrentAutomaton))
		_NbLoopAnim = 0;

	// RESET THE CURRENT ANIMATION
	_HideSkin = false;
	CAnimation::TAnimId buIndex = animIndex(MOVE);
	animIndex (MOVE,           CAnimation::UnknownAnim);				// Reset the current animation Index
	animState (MOVE,	       CAnimationStateSheet::UnknownState);		// Reset the current animation state
	animOffset(MOVE,           0.0);									// Reset the current animation offset
	animIndex (MOVE_BLEND_OUT, CAnimation::UnknownAnim);				// Reset the current animation Index
	animState (MOVE_BLEND_OUT, CAnimationStateSheet::UnknownState);		// Reset the current animation state
	animOffset(MOVE_BLEND_OUT, 0.0);									// Reset the current animation offset
	_RotationFactor		= 1.0;											// New animation -> default rotation factor for the time.
	_RightFXActivated	= false;
	_LeftFXActivated	= false;
	_RotationFactor		= 1.0f;											// Rotation factor as the animation for the time.
	_SubStateKey		= CAnimationStateSheet::UnknownState;			// No SubStateKey for the time.
	dirEndAnim(dir());													// For the time the direction at the end of the animation is the current direction.

	// CHECK
	// If there is no animation set ->There is nothing we can do properly.
	if(_CurrentAnimSet[MOVE] == 0)
		return;


	// INITIALIZE
	if(!animationStateKey(MOVE, newKey))
	{
		nlwarning("CH:setAnim:%d: automaton '%s': animation state key '%s' asked is not valid -> trying Idle.", _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName (newKey).c_str());
		if(!animationStateKey(MOVE, CAnimationStateSheet::Idle))
			nlwarning("CH:setAnim:%d: Idle is not warking too", _Slot);
	}

	// Compute Speed
	double speedToDest = computeSpeed();
	// Compute the direction to the first position (direction = front if there is not first pos)
	CVector dirToFirstPos;
	if(dist2FirstPos() > 0.0)
		dirToFirstPos = (_FirstPos - pos()).normed();
	else
		dirToFirstPos = front();

	uint antiFreezeCounter = 0;
// Loop until the process find a steady state.
KeyChosen:
	// Get the state for the current animation.
	const CAutomatonStateSheet *state = EAM->mState(_CurrentAutomaton, animState(MOVE));
	if(state == 0)
	{
		nlwarning("CH:setAnim:%d: State '%s' not in the automaton '%s'.", _Slot, CAnimationStateSheet::getAnimationStateName(animState(MOVE)).c_str(), _CurrentAutomaton.c_str());
		// No animation playing
		if(_PlayList)
			_PlayList->setAnimation(MOVE, UPlayList::empty);
		return;
	}
	const CAutomatonStateSheet &curAnimState = *state;
	//--------------------//
	//--------------------//
	// ANTI-FREEZE SYSTEM //
	// If too many loop, display some infos
	if(antiFreezeCounter > 10)
	{
/*
		nlwarning("CH:setAnim:anitFreeze:%d: Automaton '%s'",           _Slot, _CurrentAutomaton.c_str());
		nlwarning("CH:setAnim:anitFreeze:%d: _IsThereAMode '%s'",       _Slot, _IsThereAMode?"true":"false");
		nlwarning("CH:setAnim:anitFreeze:%d: dist2Dest '%f'",           _Slot, dist2Dest());
		nlwarning("CH:setAnim:anitFreeze:%d: Mode '%s(%d)'",            _Slot, modeToString(_Mode).c_str(), _Mode);
		nlwarning("CH:setAnim:anitFreeze:%d: Mode Wanted '%s(%d)'",     _Slot, modeToString(_ModeWanted).c_str(), _ModeWanted);
		nlwarning("CH:setAnim:anitFreeze:%d: Anim State Move '%s(%d)'", _Slot, CAnimationStateSheet::getAnimationStateName(animState(MOVE)).c_str(), animState(MOVE));
*/
		// Once too many more time reached, leave the method.
		if(antiFreezeCounter > 20)
			return;
	}
	// Update antiFreezeCounter.
	++antiFreezeCounter;
	// ANTI-FREEZE SYSTEM //
	//--------------------//
	//--------------------//
	// Is there a mode in the queue
//	if(_IsThereAMode && (dist2Dest()==INVALID_DIST))
	{
		// Is the current mode not already the mode wanted.
		if((_Mode != _ModeWanted) && (dist2Dest()==INVALID_DIST))
		{
			TAnimStateKey transition;
			// Check Default Mode Connection.
			if(curAnimState.getModeConnection(_ModeWanted, transition))
			{
				// Mode Mount need to be synchro
				if(_ModeWanted == MBEHAV::MOUNT_NORMAL || _ModeWanted == MBEHAV::MOUNT_SWIM)
				{
					// The Mount
					if(_Rider != CLFECOMMON::INVALID_SLOT)
					{
						if(animationStateKey(MOVE, transition))
							goto KeyChosen;
						else
							nlwarning("CH:setAnim:%d: Mode Wanted '%d' : automaton '%s': state '%s': Transition '%s' is not valid.", _Slot, _ModeWanted, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName (curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName (transition).c_str());
					}
					// The Rider
					else if(_Mount != CLFECOMMON::INVALID_SLOT)
					{
						CEntityCL *mountTmp = EntitiesMngr.entity(_Mount);
						CCharacterCL *mount = dynamic_cast<CCharacterCL *>(mountTmp);
						if(mount)
						{
							if(mountTmp->mode() == _ModeWanted)
							{
								if(animationStateKey(MOVE, transition))
									goto KeyChosen;
								else
								{
									nlwarning("CH:setAnim:%d: automaton '%s': state '%s': MountDefaultModeTransition '%s' is not valid.", _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName (curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName (transition).c_str());
								}
							}
						}
						else
						{
							nlwarning("CH:setAnim:%d: Mode Wanted '%s' but the mount does not exist.", _Slot, MBEHAV::modeToString(_ModeWanted).c_str());
						}
					}
				}
				// Other modes have the same code.
				else
				{
					if(animationStateKey(MOVE, transition))
						goto KeyChosen;
					else
						nlwarning("CH:setAnim:%d: Mode Wanted '%d' : automaton '%s': state '%s': Transition '%s' is not valid.", _Slot, _ModeWanted, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName (curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName (transition).c_str());
				}
			}
		}
	}


	// Should we stop the animation once at destination.
	if(curAnimState.BrkAtDest && (dist2Dest() <= ClientCfg.DestThreshold))
	{
		_MaxLoop = false;
		if(animationStateKey(MOVE, CAnimationStateSheet::Idle))
			goto KeyChosen;
		else
			nlwarning("CH:setAnim:%d: automaton '%s': state '%s': BrkAtDest 'idle' is not valid.", _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName (curAnimState.MoveState).c_str());
	}

	// On Move
	switch(onMove(curAnimState))
	{
	// On Move Forward
	case OnMoveForward:
		if(animationStateKey(MOVE, curAnimState.OnMoveForward))
			goto KeyChosen;
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': OnMoveForeward '%s' is not valid.", _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName(curAnimState.OnMoveForward).c_str());
		break;
	// On Move Backward
	case OnMoveBackward:
		if(animationStateKey(MOVE, curAnimState.OnMoveBackward))
			goto KeyChosen;
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': OnMoveBackward '%s' is not valid.", _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName(curAnimState.OnMoveBackward).c_str());
		break;
	// On Move Left
	case OnMoveLeft:
		if(animationStateKey(MOVE, curAnimState.OnMoveLeft))
			goto KeyChosen;
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': OnMoveLeft '%s' is not valid.",     _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName(curAnimState.OnMoveLeft).c_str());
		break;
	// On Move Right
	case OnMoveRight:
		if(animationStateKey(MOVE, curAnimState.OnMoveRight))
			goto KeyChosen;
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': OnMoveRight '%s' is not valid.",    _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName(curAnimState.OnMoveRight).c_str());
		break;
	default:
		break;
	}

	// On Rotation/About Face
	CVector dirEndA(0.0f, 0.0f, 0.0f);
	switch(onRotation(curAnimState, dirEndA))
	{
	// On Rot Left
	case OnRotLeft:
		if(animationStateKey(MOVE, curAnimState.OnLeftRotation))
		{
			dirEndAnim(dirEndA);
			goto KeyChosen;
		}
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': OnLeftRotation '%s' is not valid.",     _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName(curAnimState.OnLeftRotation).c_str());
		break;
	// On Rot Right
	case OnRotRight:
		if(animationStateKey(MOVE, curAnimState.OnRightRotation))
		{
			dirEndAnim(dirEndA);
			goto KeyChosen;
		}
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': OnRightRotation '%s' is not valid.",    _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName(curAnimState.OnRightRotation).c_str());
		break;
	default:
		break;
	}

	// Max Loop
	if(curAnimState.MaxLoop && curAnimState.MaxLoop<=_NbLoopAnim)
	{
		if(animationStateKey(MOVE, CAnimationStateSheet::Idle))
		{
			_MaxLoop = true;
			_NbLoopAnim = 0;
			goto KeyChosen;
		}
	}

	// On Bad Heading
	if(onBadHeading(curAnimState))
	{
		if(animationStateKey(MOVE, CAnimationStateSheet::Idle))
			goto KeyChosen;
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': 'Idle' is not valid.",    _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str());
	}

	// On Big Bend
	switch(onBigBend(curAnimState, dirEndA))
	{
		// On Bend Left
	case OnBendLeft:
		if(animationStateKey(MOVE, curAnimState.OnBigBendLeft))
		{
			dirEndAnim(dirEndA);
			goto KeyChosen;
		}
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': OnBigBendLeft '%s' is not valid.",     _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName(curAnimState.OnBigBendLeft).c_str());
		break;
		// On Bend Right
	case OnBendRight:
		if(animationStateKey(MOVE, curAnimState.OnBigBendRight))
		{
			dirEndAnim(dirEndA);
			goto KeyChosen;
		}
		else
			nlwarning("CH::setAnim:%d: automaton '%s': state '%s': OnBigBendRight '%s' is not valid.",    _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName(curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName(curAnimState.OnBigBendRight).c_str());
		break;
	default:
		break;
	}

	// If the animation change according to a high speed and speed high enough or oo.
	if(ClientCfg.BlendForward == false)
	{
		if(curAnimState.OnMaxSpeed.Breakable)
		{
			if(speedToDest >= _CurrentAnimSet[MOVE]->speedToRun() ||  speedToDest == -1.0)
			{
				if(animationStateKey(MOVE, curAnimState.OnMaxSpeed.NextStateKey))
					goto KeyChosen;
				else
					nlwarning("CH::setAnim: Char '%d': automaton '%s': state '%s': OnMaxSpeed.NextStateKey '%s' is not valid.", _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName (curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName (curAnimState.OnMaxSpeed.NextStateKey).c_str());
			}
		}
	}

	// If the animation can change with a low speed.
	if(curAnimState.OnMinSpeed.Breakable)
	{
		// If the speed is low enough (check this is not -1 (infinite speed)) -> update the animation to play.
		if(speedToDest != -1.0 && speedToDest <= _CurrentAnimSet[MOVE]->speedToWalk())
		{
			if(animationStateKey(MOVE, curAnimState.OnMinSpeed.NextStateKey))
				goto KeyChosen;
			else
				 nlwarning("CH::setAnim: Char '%d': automaton '%s': state '%s': OnMinSpeed.NextStateKey '%s' is not valid.", _Slot, _CurrentAutomaton.c_str(), CAnimationState::getAnimationStateName (curAnimState.MoveState).c_str(), CAnimationState::getAnimationStateName (curAnimState.OnMinSpeed.NextStateKey).c_str());
		}
	}
	//// END LOOP /////


	// \todo GUIGUI : better manage automate change
	// Current animation is not of the same kind as the old one.
	bool sameAnim = lastAnimStateId == animState(MOVE) && _OldAutomaton == _CurrentAutomaton;
	if(!sameAnim)
	{
		// Stop FXs.
		stopItemAttackFXs();
		stopAttachedFXForCurrrentAnim(true); // stop all anim fx, including looping fxs
	}
	else
	{
		stopAttachedFXForCurrrentAnim(false); // stop all anim fxs, but do not stop looping fxs (because the same anim is repeating)
	}

	// Compute the current animation state.
	_CurrentState = EAM->mState(_CurrentAutomaton, animState(MOVE));
	// If the state does not exist.
	if(_CurrentState == 0)
	{
		nlwarning("CH:setAnim:%d: State '%s' not in the automaton '%s'.", _Slot, CAnimationStateSheet::getAnimationStateName (animState(MOVE)).c_str(), _CurrentAutomaton.c_str());

		// No animation playing
		if(_PlayList)
			_PlayList->setAnimation(MOVE, UPlayList::empty);
	}
	// The state is valid.
	else
	{
		double angToDest = 0.0;
		// If the new state is a rotation.
		if(_CurrentState->Rotation)
			angToDest = computeShortestAngle(atan2(dir().y, dir().x), atan2(dirEndAnim().y, dirEndAnim().x));
		// Get the right animation state and choose an animation.
		{
			// Get the animation state
			const CAnimationState *animationState = 0;
			if(animState(MOVE) == CAnimationStateSheet::Emote)
			{
				_SubStateKey = subKey;
				animationState = _CurrentAnimSet[MOVE]->getAnimationState(_SubStateKey);
			}
			else
				animationState = _CurrentAnimSet[MOVE]->getAnimationState(animState(MOVE));
			if(animationState)
			{
				// Choose the animation
				CAnimation::TAnimId index;
				if(sameAnim)
					index = buIndex;
				else
					index = CAnimation::UnknownAnim;
				animIndex(MOVE, animationState->chooseAnim(_AnimJobSpecialisation, people(), getGender(), angToDest, index));
				if(animID != NL3D::UAnimationSet::NotFound)
					animId(MOVE, animID);


				// Should the objects in hands be displayed ?
				_ObjectsVisible = animationState->areObjectsVisible();
				showOrHideBodyParts( _ObjectsVisible );

				// in case of user manage the internal view
				if( isUser() )
				{
					UserEntity->updateVisualDisplay();
				}
			}
		}
		// Initialize the animation
		if(animIndex(MOVE) != CAnimation::UnknownAnim)
		{
			// If the new state is a rotation.
			if(_CurrentState->Rotation)
			{
				// Get the animation rotation.
				double animAngle = CAnimationMisc::getAnimationRotation(EAM->getAnimationSet(), animId(MOVE));
				// Compute the rotation factor.
				if(animAngle != 0.0)
					_RotationFactor = fabs(angToDest/animAngle);
				else
					_RotationFactor = -1.0;	// \todo GUIGUI : see which value we should use if we have a rot anim without rot and which should rotate character
			}

			// If the animation is an atk or forage extraction -> Start all dynamic FXs.
			if (_CurrentBehaviour.Behaviour == MBEHAV::EXTRACTING)
			{
				// True Extract Animation only for Use AnimationState type
				// \todo yoyo: ugly?
				if( animState(MOVE)==CAnimationStateSheet::UseLoop )
				{
					_Items[SLOTTYPE::RIGHT_HAND_SLOT].setTrailSize(_CurrentBehaviour.ForageExtraction.Level);
					startItemAttackFXs(true, 1);
				}
			}
			else if (_CurrentState->Attack)
			{
				if(_CurrentBehaviour.Behaviour == MBEHAV::RANGE_ATTACK)
				{
					startItemAttackFXs(_CurrentBehaviour.Range.ImpactIntensity != 0, _CurrentBehaviour.Range.ImpactIntensity);
				}
				else
				{
					startItemAttackFXs(_CurrentBehaviour.Combat.ImpactIntensity != 0 && _CurrentBehaviour.Combat.HitType != HITTYPE::Failed, _CurrentBehaviour.Combat.ImpactIntensity);
				}
			}


			// Initialize the new animation.
			if(_PlayList)
			{
				// Blend the last animation and the new one.
				if(ClientCfg.BlendFrameNumber && _BlendRemaining <= 0								// Blend On ?
				&& animIndex(ACTION) != CAnimation::UnknownAnim										// Last Animation Valid ?
				&& ((lastAnimStateId != animState(MOVE)) || (_OldAutomaton != _CurrentAutomaton)
				   || (animState(MOVE) == CAnimationStateSheet::Emote))	// Last anim != or automaton != ?
				&& !isRiding())											// No Blend on a mount. \todo GUIGUI trouver un meilleur moyen.
				{
					// Get the rotation of the last animation for the blend.
					if(!_Skeleton.empty())
						_OldRotQuat = _Skeleton.getRotQuat();
					_BlendRemaining = ClientCfg.BlendFrameNumber;
					// Set animation
					_PlayList->setAnimation(ACTION, animId(ACTION));
					// Compute weight step.
					float w = 1.f/((float)(ClientCfg.BlendFrameNumber+1));
					// Set Old Anim Weight.
					_PlayList->setWeight(ACTION, 1.f-w);
					// Set New Anim Weight.
					_PlayList->setWeight(MOVE, w);
					// Copy Reverse
					_AnimReversed[ACTION] = _AnimReversed[MOVE];
				}
				// Set Animation.
				_PlayList->setAnimation (MOVE, animId(MOVE));
				// Blend between Walk and Run.
				if(ClientCfg.BlendForward && (animState(MOVE) == CAnimationStateSheet::Walk))
				{
					_CurrentAnimSet[MOVE_BLEND_OUT] = _CurrentAnimSet[MOVE];
					animState(MOVE_BLEND_OUT, CAnimationStateSheet::Run);
					const CAnimationState *animationBlendState = _CurrentAnimSet[MOVE_BLEND_OUT]->getAnimationState(animState(MOVE_BLEND_OUT));
					if(animationBlendState)
					{
						animIndex(MOVE_BLEND_OUT, animationBlendState->chooseAnim(_AnimJobSpecialisation, people(), getGender()));
						_PlayList->setAnimation(MOVE_BLEND_OUT, animId(MOVE_BLEND_OUT));
						_PlayList->setWeight(MOVE_BLEND_OUT, 1.0f);		// \todo GUIGUI : verify what is happening if animId is "empty".
					}
					else
						nlwarning("setAnim:%d: animationBlendState is Null.", _Slot);
				}

				// Set children animation.
				std::list<CEntityCL *>::iterator it = _Children.begin();
				while(it != _Children.end())
				{
					if(*it)
					{
						CCharacterCL *child = dynamic_cast<CCharacterCL *>(*it);
						if(child)
						{
							// Set the Child as the parent.
							child->computeAnimSet();
							child->currentAutomaton() = _CurrentAutomaton;	// \todo GUIGUI : CA VA PAS MARCHER A CAUSE DU TYPE !!!
							child->animOffset(MOVE, animOffset(MOVE));
							child->animState(MOVE, animState(MOVE));
							child->currentState(currentState());
							child->animIndex(MOVE, CAnimation::UnknownAnim);
							const CAnimationState *animStatePtr = child->currentAnimSet()[MOVE]->getAnimationState(child->animState(MOVE));
							if(animStatePtr)
							{
								child->animIndex(MOVE, animStatePtr->chooseAnim(_AnimJobSpecialisation, people(), getGender(), angToDest));
								child->playList()->setAnimation(MOVE, child->animId(MOVE));
							}
							// TEMP : \todo GUIGUI : Pour le moment on enlever le Blend sur les montures.
							{
								child->playList()->setAnimation(ACTION, UPlayList::empty);
								child->currentAnimSet()[ACTION]       = child->currentAnimSet()[MOVE];
								child->animState (ACTION, child->animState (MOVE));
								child->animIndex (ACTION, child->animIndex (MOVE));
								child->animOffset(ACTION, child->animOffset(MOVE));
							}

						}
						else
							nlwarning("CH:setAnim:%d: Child is not a 'CCharacterCL'.", _Slot);
					}
					else
						nlwarning("CH:setAnim:%d: Child not allocated.", _Slot);

					// Next Child
					++it;
				}
			}

			// Get the animation.
			const CAnimationState *animStatePtr = _CurrentAnimSet[MOVE]->getAnimationState((animState(MOVE) ==  CAnimationStateSheet::Emote)?subKey:animState(MOVE));
			if(animStatePtr)
			{
				const CAnimation *anim = animStatePtr->getAnimation(animIndex(MOVE));
				if(anim)
				{
					_HideSkin = anim->hideAtEndAnim();
					// Reverse ?
					_AnimReversed[MOVE] = anim->isReverse();
					// Is the head controlable.
					_TargetAnimCtrl.Enabled = anim->headControlable();
					// Select the sound ID
					_SoundId[MOVE] = anim->soundId();
					// look in behaviour if there's a spell to play
					if (_CurrentAttack)
					{
						if (_CurrentAttackInfo.Intensity >= 1 &&  _CurrentAttackInfo.Intensity <= MAGICFX::NUM_SPELL_POWER)
						{
							MAGICFX::TSpellCastStage attackStage;
							switch(newKey)
							{
								case CAnimationStateSheet::OffensiveCastBegin:
								case CAnimationStateSheet::CurativeCastBegin:
								case CAnimationStateSheet::MixedCastBegin:
								case CAnimationStateSheet::AcidCastInit:
								case CAnimationStateSheet::BlindCastInit:
								case CAnimationStateSheet::ColdCastInit:
								case CAnimationStateSheet::ElecCastInit:
								case CAnimationStateSheet::FearCastInit:
								case CAnimationStateSheet::FireCastInit:
								case CAnimationStateSheet::HealHPCastInit:
								case CAnimationStateSheet::MadCastInit:
								case CAnimationStateSheet::PoisonCastInit:
								case CAnimationStateSheet::RootCastInit:
								case CAnimationStateSheet::RotCastInit:
								case CAnimationStateSheet::ShockCastInit:
								case CAnimationStateSheet::SleepCastInit:
								case CAnimationStateSheet::SlowCastInit:
								case CAnimationStateSheet::StunCastInit:
									attackStage = MAGICFX::CastBegin;
								break;
								case CAnimationStateSheet::OffensiveCastLoop:
								case CAnimationStateSheet::CurativeCastLoop:
								case CAnimationStateSheet::MixedCastLoop:
								case CAnimationStateSheet::AcidCastLoop:
								case CAnimationStateSheet::BlindCastLoop:
								case CAnimationStateSheet::ColdCastLoop:
								case CAnimationStateSheet::ElecCastLoop:
								case CAnimationStateSheet::FearCastLoop:
								case CAnimationStateSheet::FireCastLoop:
								case CAnimationStateSheet::HealHPCastLoop:
								case CAnimationStateSheet::MadCastLoop:
								case CAnimationStateSheet::PoisonCastLoop:
								case CAnimationStateSheet::RootCastLoop:
								case CAnimationStateSheet::RotCastLoop:
								case CAnimationStateSheet::ShockCastLoop:
								case CAnimationStateSheet::SleepCastLoop:
								case CAnimationStateSheet::SlowCastLoop:
								case CAnimationStateSheet::StunCastLoop:
									attackStage = MAGICFX::CastLoop;
								break;
								case CAnimationStateSheet::OffensiveCastSuccess:
								case CAnimationStateSheet::CurativeCastSuccess:
								case CAnimationStateSheet::MixedCastSuccess:
								case CAnimationStateSheet::AcidCastEnd:
								case CAnimationStateSheet::BlindCastEnd:
								case CAnimationStateSheet::ColdCastEnd:
								case CAnimationStateSheet::ElecCastEnd:
								case CAnimationStateSheet::FearCastEnd:
								case CAnimationStateSheet::FireCastEnd:
								case CAnimationStateSheet::HealHPCastEnd:
								case CAnimationStateSheet::MadCastEnd:
								case CAnimationStateSheet::PoisonCastEnd:
								case CAnimationStateSheet::RootCastEnd:
								case CAnimationStateSheet::RotCastEnd:
								case CAnimationStateSheet::ShockCastEnd:
								case CAnimationStateSheet::SleepCastEnd:
								case CAnimationStateSheet::SlowCastEnd:
								case CAnimationStateSheet::StunCastEnd:
									attackStage = MAGICFX::CastEnd;
								break;
								case CAnimationStateSheet::OffensiveCastFail:
								case CAnimationStateSheet::OffensiveCastFumble:
								case CAnimationStateSheet::CurativeCastFail:
								case CAnimationStateSheet::CurativeCastFumble:
								case CAnimationStateSheet::MixedCastFail:
								case CAnimationStateSheet::MixedCastFumble:
								case CAnimationStateSheet::AcidCastFail:
								case CAnimationStateSheet::BlindCastFail:
								case CAnimationStateSheet::ColdCastFail:
								case CAnimationStateSheet::ElecCastFail:
								case CAnimationStateSheet::FearCastFail:
								case CAnimationStateSheet::FireCastFail:
								case CAnimationStateSheet::HealHPCastFail:
								case CAnimationStateSheet::MadCastFail:
								case CAnimationStateSheet::PoisonCastFail:
								case CAnimationStateSheet::RootCastFail:
								case CAnimationStateSheet::RotCastFail:
								case CAnimationStateSheet::ShockCastFail:
								case CAnimationStateSheet::SleepCastFail:
								case CAnimationStateSheet::SlowCastFail:
								case CAnimationStateSheet::StunCastFail:
									attackStage = MAGICFX::CastFail;
								break;
								// attacks
								case CAnimationStateSheet::DefaultAtkLow:
								case CAnimationStateSheet::DefaultAtkMiddle:
								case CAnimationStateSheet::DefaultAtkHigh:
								case CAnimationStateSheet::PowerfulAtkLow:
								case CAnimationStateSheet::PowerfulAtkMiddle:
								case CAnimationStateSheet::PowerfulAtkHigh:
								case CAnimationStateSheet::AreaAtkLow:
								case CAnimationStateSheet::AreaAtkMiddle:
								case CAnimationStateSheet::AreaAtkHigh:
								case CAnimationStateSheet::Attack1:
								case CAnimationStateSheet::Attack2:
								case CAnimationStateSheet::FirstPersonAttack:
									attackStage = MAGICFX::CastEnd;
								break;
								default:
									attackStage = MAGICFX::SpellCastStageCount;
								break;
							}
							const CAnimationFXSet *afs = NULL;
							switch(attackStage)
							{
								case MAGICFX::CastBegin: afs = &_CurrentAttack->AttackBeginFX; break;
								case MAGICFX::CastLoop:  afs = &_CurrentAttack->AttackLoopFX; break;
								case MAGICFX::CastEnd:   afs = &_CurrentAttack->AttackEndFX; break;
								case MAGICFX::CastFail:  afs = &_CurrentAttack->AttackFailFX; break;
								default: break;
							}
							playCastFX(afs, _CurrentAttackInfo.Intensity);
						}
					}
					// start forage prospection anim fx(s)
					if ( (behaviour() == MBEHAV::PROSPECTING) || (behaviour() == MBEHAV::PROSPECTING_END) )
					{
						const CAnimationFXSet& fxSet = anim->getFXSet();
						std::vector<UParticleSystemInstance> fxInstances;
						CAttachedFX::CBuildInfo bi;
						CAttachedFX::CTargeterInfo ti;
						for (uint k = 0; k < fxSet.FX.size(); ++k)
						{
							CAttachedFX::TSmartPtr fx = new CAttachedFX;
							bi.Sheet = &(fxSet.FX[k]);
							fx->create(*this, bi, ti);
							if (!fx->FX.empty())
							{

								attachFXInternal(fx,FXListCurrentAnim);
								if (k == 0)
								{
									// Set user param for size of prospection area
									{
										fx->FX.setUserParam( 0, ((float)(_CurrentBehaviour.ForageProspection.Range)) / 127.0f );
										float up [3];
										switch ( _CurrentBehaviour.ForageProspection.Angle )
										{
										case 0:	up[2] = up[1] = up[0] = 0.0f; break;
										case 1: up[0]=1.0f; up[1]=0.0f; up[2]=0.0f; break;
										case 2: up[0]=1.0f; up[1]=1.0f; up[2]=0.0f; break;
										default: up[0]=1.0f; up[1]=1.0f; up[2]=1.0f; break;
										}
										fx->FX.setUserParam( 1, up[0] );
										fx->FX.setUserParam( 2, up[1] );
										fx->FX.setUserParam( 3, up[2] );
										//nlinfo( "Prospection %s %u %u", behaviour()==MBEHAV::PROSPECTING?"PROSPTG":(behaviour()==MBEHAV::PROSPECTING_END?"PRO_END":"OTHER"), _CurrentBehaviour.ForageProspection.Range, _CurrentBehaviour.ForageProspection.Angle );
									}
								}
								else if (k == 1)
								{
									// Set user param for level of prospection
									float up [4];
									switch ( _CurrentBehaviour.ForageProspection.Level )
									{
									case 0: up[3] = up[2] = up[1] = up[0] = 0.0f; break;
									case 1: up[0]=0.0f; up[1]=1.0f; up[2]=0.0f; up[3]=0.0f; break;
									case 2: up[0]=1.0f; up[1]=1.0f; up[2]=0.0f; up[3]=0.0f; break;
									case 3: up[0]=1.0f; up[1]=1.0f; up[2]=1.0f; up[3]=0.0f; break;
									default: up[3] = up[2] = up[1] = up[0] = 1.0f; break;
									}
									fx->FX.setUserParam( 0, up[0] );
									fx->FX.setUserParam( 1, up[1] );
									fx->FX.setUserParam( 2, up[2] );
									fx->FX.setUserParam( 3, up[3] );
								}
							}
						}
					}
					// start standard anim fx
					else
					{
						CAttachedFX::CBuildInfo bi;
						CAttachedFX::CTargeterInfo ti;
						bi.MaxNumAnimCount = MAX_FX_ANIM_COUNT;
						for (uint k = 0; k < anim->getFXSet().FX.size(); ++k)
						{
							// if the fx is in looping mode, & the anim has already done a loop, then don't recreate it
							if (anim->getFXSet().FX[k].Sheet->RepeatMode == CAnimationFXSheet::Loop && sameAnim) continue;
							CAttachedFX::TSmartPtr fx = new CAttachedFX;
							bi.Sheet = &(anim->getFXSet().FX[k]);
							//bi.StickMode = &bi.Sheet->FX[k].StickMode;
							if (anim->getFXSet().FX[k].Sheet)
							{
								bi.StickMode = &(anim->getFXSet().FX[k].Sheet->StickMode);
							}
							fx->create(*this, bi, ti);
							if (!fx->FX.empty())
							{
								attachFXInternal(fx, FXListAuto);
							}
						}
					}
				}
			}
			else
				nlwarning("CH:setAnim:%d: cannot get the pointer on the animation.", _Slot);
		}
		// No animation found -> check if it is just a transition or is there really an animation missing
		else
		{
			// INFO : Verbose mode for Animations of the selection.
			if(VerboseAnimSelection && _Slot == UserEntity->selection())
				nlinfo("CH:setAnim:%d: Anim Not Found, state '%s'.", _Slot, CAnimationState::getAnimationStateName(animState(MOVE)).c_str());

			// If the next animation or automaton is not the same -> this is a transition -> no animation needed.
			if(_CurrentState->MoveState != _CurrentState->NextState || (_CurrentState->NextMode != _Mode))
			{
				// Choose the next animation.
				endAnimTransition();
			}
			// Else -> Animation Missing.
		}
	}
	// Set the LOD Animation.
	setAnimLOD(((lastAnimStateId != animState(MOVE)) || (_OldAutomaton != _CurrentAutomaton)));
}// setAnim //


//-----------------------------------------------
// playCastFX
//-----------------------------------------------
void CCharacterCL::playCastFX(const CAnimationFXSet *afs, uint power)
{
	if (!afs) return;
	if (power <= 0 || power > 5) return;
	static const float castFXUserParams[MAGICFX::NUM_SPELL_POWER][4] =
	{
		{ 0.f, 0.f, 0.f, 0.f},
		{ 1.f, 0.f, 0.f, 0.f},
		{ 1.f, 1.f, 0.f, 0.f},
		{ 1.f, 1.f, 1.f, 0.f},
		{ 1.f, 1.f, 1.f, 1.f}
	};
	CAttachedFX::CBuildInfo bi;
	CAttachedFX::CTargeterInfo ti;
	for (uint k = 0; k < afs->FX.size(); ++k)
	{
		CAttachedFX::TSmartPtr fx = new CAttachedFX;
		bi.Sheet = &afs->FX[k];
		fx->create(*this, bi, ti);
		if (!fx->FX.empty())
		{
				for(uint l = 0; l < 4; ++l)
				{
					fx->FX.setUserParam(l, castFXUserParams[power - 1][l]);
				}
				attachFXInternal(fx, FXListCurrentAnim);
		}
	}
}


//-----------------------------------------------
// showOrHideBodyParts
//-----------------------------------------------
void CCharacterCL::showOrHideBodyParts( bool objectsVisible )
{
	// UnHide all user body parts.
	for(uint i=0; i<_Instances.size(); ++i)
		if(!_Instances[i].Current.empty())
			_Instances[i].Current.show();

	// hide or show the face
	if( _Items[SLOTTYPE::HEAD_SLOT].Sheet && _Items[SLOTTYPE::HEAD_SLOT].Sheet->Family == ITEMFAMILY::ARMOR )
	{
		// Get the face
		SInstanceCL *face = getFace ();

		// hide if helmet
		if(face)
		{
			if(!face->Current.empty())
				face->Current.hide();
			else
				face->KeepHiddenWhenLoaded = true;
		}
	}
	else
	{
		// Get the face
		SInstanceCL *face = getFace ();

		// hide if helmet
		if(face)
		{
			if(!face->Current.empty())
				face->Current.show();
			else
				face->KeepHiddenWhenLoaded = false;
		}
	}

	// get the instance index for right hand and left hand
	uint32 rHandInstIdx;
	uint32 lHandInstIdx;
	if( isPlayer() || isUser() )
	{
		rHandInstIdx = SLOTTYPE::RIGHT_HAND_SLOT;
		lHandInstIdx = SLOTTYPE::LEFT_HAND_SLOT;

		// hide gloves(armor) if player has magician amplifier
		if( _Items[rHandInstIdx].Sheet && (_Items[rHandInstIdx].Sheet->ItemType == ITEM_TYPE::MAGICIAN_STAFF) )
		{
			if( !_Instances[SLOTTYPE::HANDS_SLOT].Current.empty() )
				_Instances[SLOTTYPE::HANDS_SLOT].Current.hide();
			else
				_Instances[SLOTTYPE::HANDS_SLOT].KeepHiddenWhenLoaded = true;
		}
	}
	else
	{
		rHandInstIdx = _RHandInstIdx;
		lHandInstIdx = _LHandInstIdx;
	}

	// if not already hidden and need to hide :
	if( !objectsVisible )
	{
		// Right Hand
		if(rHandInstIdx<_Instances.size())
			if( !(_Items[rHandInstIdx].Sheet && _Items[rHandInstIdx].Sheet->NeverHideWhenEquipped ) )
				if(!_Instances[rHandInstIdx].Current.empty())
				{
					_Instances[rHandInstIdx].Current.hide();
					_Instances[rHandInstIdx].hideStaticFXs();
				}
		// Left Hand
		if(lHandInstIdx <_Instances.size())
			if( !(_Items[lHandInstIdx].Sheet && _Items[lHandInstIdx].Sheet->NeverHideWhenEquipped ) )
				if(!_Instances[lHandInstIdx].Current.empty())
				{
					_Instances[lHandInstIdx].Current.hide();
					_Instances[lHandInstIdx].hideStaticFXs();
				}
	}
	else
	{
		// Right Hand
		if(rHandInstIdx<_Instances.size())
			if(!_Instances[rHandInstIdx].Current.empty())
			{
				_Instances[rHandInstIdx].Current.show();
				_Instances[rHandInstIdx].showStaticFXs();
			}
		// Left Hand
		if(lHandInstIdx <_Instances.size())
			if(!_Instances[lHandInstIdx].Current.empty())
			{
				_Instances[lHandInstIdx].Current.show();
				_Instances[lHandInstIdx].showStaticFXs();
			}
	}
}

//-----------------------------------------------
// setAnimLOD :
// Set the LOD animation.
//-----------------------------------------------
ADD_METHOD(void CCharacterCL::setAnimLOD(bool changed))
	// reset LOD animation.
	_LodCharacterAnimEnabled = false;
	// Setup new LOD animation.
	if(skeleton())
	{
		// if the entity has a lod Character
		sint lodId = skeleton()->getLodCharacterShape();
		if(lodId >= 0)
		{
			// Setup Lod anim.
			_LodCharacterAnimEnabled	= true;
			_LodCharacterMasterAnimSlot	= MOVE;
			_LodCharacterAnimTimeOffset	= 0;
			// do complex stuff only if the anim state has really changed.
			if(changed)
			{
				// get the anim state from the set.
				const CAnimationState *animStatePtr = _CurrentAnimSet[MOVE]->getAnimationState(animState(MOVE));
				// if animStatePtr found.
				if(animStatePtr)
				{
					// get Lod Character animation Name from the anim state.
					const string &lodAnimName = animStatePtr->getLodCharacterAnimation();
					// Find the anim in the UScene LodCharacterManager
					sint animId = Scene->getCLodAnimIdByName(lodId, lodAnimName);
					// if Anim not found, get the "idle" anim, with the Id 0.
					if(animId < 0)
						animId = 0;
					// setup the skeleton.
					skeleton()->setLodCharacterAnimId(animId);
				}
			}
		}
	}
}// setAnimLOD //


//-----------------------------------------------
// updateAnimationState :
// \todo GUIGUI : precalculate distance to destination when receiving Stages.
// \todo GUIGUI : improve, we are setting often 'idle' to recompute orientation at the end of animation instead of doing directly the right one.
//-----------------------------------------------
ADD_METHOD(void CCharacterCL::updateAnimationState())
	// If the current state is invalid -> return.
	if(_CurrentState == 0)
		return;

	// Get the Animation Length.
	double animLength = EAM->getAnimationLength(animId(MOVE));
	// Check Anim length
	if(animOffset(MOVE) >= animLength)
	{
		// Choose the next animation.
		endAnimTransition();
	}
	// Last animation not finished check for some chances to break current animation.
	else
	{
		// Check Modes.
//		if(_IsThereAMode && (dist2Dest()==INVALID_DIST))
		{
			// Is the current mode not already the mode wanted.
			if((_Mode != _ModeWanted) && (dist2Dest()==INVALID_DIST))
			{
				if((_ModeWanted != MBEHAV::MOUNT_NORMAL && _ModeWanted != MBEHAV::MOUNT_SWIM) || (_Rider != CLFECOMMON::INVALID_SLOT))
				{
					// Is there a possible connection with the mode wanted.
					TAnimStateKey transition;
					if(_CurrentState->getModeConnection(_ModeWanted, transition))
					{
						setAnim(transition);
						return;
					}
				}
				else
				{
					//InfoLog->displayRawNL("(%d)_Rider=%d  _Mount=%d  _ModeWanted=%s  _Mode=%s  offset=%f  length=%f",_Slot,_Rider,_Mount,MBEHAV::modeToString(_ModeWanted).c_str(),MBEHAV::modeToString(_Mode).c_str(),animOffset(MOVE),animLength);
					// anti-bug : sometimes _rider is not set yet and we stay in an infinite move
					_Rider = _TheoreticalRider;
				}
			}
		}

		// Should we stop the animation once at destination.
		if(_CurrentState->BrkAtDest && dist2Dest() <= ClientCfg.DestThreshold)
		{
			_MaxLoop = false;
			setAnim(CAnimationStateSheet::Idle);
			return;
		}

		// ON MOVE
		switch(onMove(*_CurrentState))
		{
		// On Move Forward
		case OnMoveForward:
			setAnim(_CurrentState->OnMoveForward);
			return;
		// On Move Backward
		case OnMoveBackward:
			setAnim(_CurrentState->OnMoveBackward);
			return;
		// On Move Left
		case OnMoveLeft:
			setAnim(_CurrentState->OnMoveLeft);
			return;
		// On Move Right
		case OnMoveRight:
			setAnim(_CurrentState->OnMoveRight);
			return;
		default:
			break;
		}

		// ON ROTATION
		CVector tmp;
		switch(onRotation(*_CurrentState, tmp))
		{
		// Rotation Left
		case OnRotLeft:
			setAnim(CAnimationStateSheet::Idle);
			return;
		// Rotation Right
		case OnRotRight:
			setAnim(CAnimationStateSheet::Idle);
			return;
		default:
			break;
		}

		// ON BAD HEADING
		if(onBadHeading(*_CurrentState))
		{
			setAnim(CAnimationStateSheet::Idle);
			return;
		}

		// ON BIG BEND
		switch(onBigBend(*_CurrentState, tmp))
		{
		// Big Bend Left and Right
		case OnBendLeft:
		case OnBendRight:
			setAnim(_CurrentState->MoveState);
			return;
		default:
			break;
		}

		// \todo GUIGUI : changer de place cette partie je pense.
		// Adjust the direction to fit the front
		if( !(isUser() && ClientCfg.AutomaticCamera==false) )
		{
			if(_CurrentState->AdjustOri)
			{
				// Adjust before the half of the attack animation.
				const double animL = animLength/2.0;
				// Half already reatch, dir should be as the front now
				if(animOffset(MOVE) > animL)
					dir(front());
				else
				{
					double ang = computeShortestAngle(atan2(dir().y, dir().x), atan2(front().y, front().x));
					if(ang)
					{
						double ang2 = (animOffset(MOVE)/animL)*ang/animL;
						double angleZ = ang2+atan2(dir().y, dir().x);
						dir(CVector((float)cos(angleZ), (float)sin(angleZ), 0.f));
					}
				}
			}
		}
	}
}// updateAnimationState //

//-----------------------------------------------
// computeMotion :
//-----------------------------------------------
ADD_METHOD(double CCharacterCL::computeMotion(const double &oldMovingTimeOffset, TAnimationType channel) const)
	H_AUTO_USE ( RZ_Client_Entity_CL_Update_Pos_Compute_Motion )

	// Check the state is valid.
	if(_CurrentState == 0)
		return 0.0;

	// Calculate movement for given animation segment.
	if(_CurrentState->Move)
	{
		// Animation is unknown, no move in it.
		if(animIndex(channel) == CAnimation::UnknownAnim)
			return 0.0;

		CVector	oldPos, newPos;
		uint animID = animId(channel);
		if(EAM->interpolate(animID, oldMovingTimeOffset, oldPos))
		{
			if(EAM->interpolate(animID, animOffset(channel), newPos))
			{
				CVector mov = newPos - oldPos;
				// Scale it by the CharacterScalePos, if needed, according to the animation.
				bool mustApplyCharacterScalePosFactor = true;
				const CAnimationState *animStatePtr = _CurrentAnimSet[channel]->getAnimationState(animState(channel));
				if(animStatePtr)
				{
					const CAnimation *anim = animStatePtr->getAnimation(animIndex(channel));
					if(anim)
						mustApplyCharacterScalePosFactor = anim->applyCharacterScalePosFactor();
				}
				// scale it according to the species.
				if(mustApplyCharacterScalePosFactor)
					mov*= _CharacterScalePos;
				// Scale according to the gabarit.
				mov *= _CustomScalePos;
				// Return a significant move.
				double distDone = mov.norm();
				if(distDone>0.0 && distDone<ClientCfg.SignificantDist)
					distDone = ClientCfg.SignificantDist;
				return distDone;
			}
		}

		// Miss the position's track.
		nlwarning("CCharacterCL::computeMotion : Animation should have a track for the position.");
		return 0.0;
	}
	// The animation is not one to move.
	else
		return 0.0;
}// computeMotion //

//-----------------------------------------------
// beginCast :
//-----------------------------------------------
void CCharacterCL::beginCast(const MBEHAV::CBehaviour &behaviour)
{
	// if the player has a target, force him to face this target
	CEntityCL *target = EntitiesMngr.entity(targetSlot());
	if(target && !target->isUser() )
	{
		CVectorD dirToTarget = target->pos() - pos();
		dirToTarget.z = 0;
		dirToTarget.normalize();
		front( dirToTarget );
		dir( dirToTarget );
	}

	switch(behaviour.Behaviour)
	{
	////////////////
	// BEGIN CAST //
	// OFFENSIVE CAST BEGIN
	case MBEHAV::CAST_OFF:
		setAnim(CAnimationStateSheet::OffensiveCastInit);
		break;
	// CURATIVE CAST BEGIN
	case MBEHAV::CAST_CUR:
		setAnim(CAnimationStateSheet::CurativeCastInit);
		break;
	// MIXED CAST BEGIN
	case MBEHAV::CAST_MIX:
		setAnim(CAnimationStateSheet::MixedCastInit);
		break;
	// IDLE
	case MBEHAV::IDLE:
		setAnim(CAnimationStateSheet::Idle);
		break;
	//
	case MBEHAV::CAST_ACID:
		setAnim(CAnimationStateSheet::AcidCastInit);
		break;
	case MBEHAV::CAST_BLIND:
		setAnim(CAnimationStateSheet::BlindCastInit);
		break;
	case MBEHAV::CAST_COLD:
		setAnim(CAnimationStateSheet::ColdCastInit);
		break;
	case MBEHAV::CAST_ELEC:
		setAnim(CAnimationStateSheet::ElecCastInit);
		break;
	case MBEHAV::CAST_FEAR:
		setAnim(CAnimationStateSheet::FearCastInit);
		break;
	case MBEHAV::CAST_FIRE:
		setAnim(CAnimationStateSheet::FireCastInit);
		break;
	case MBEHAV::CAST_HEALHP:
		setAnim(CAnimationStateSheet::HealHPCastInit);
		break;
	case MBEHAV::CAST_MAD:
		setAnim(CAnimationStateSheet::MadCastInit);
		break;
	case MBEHAV::CAST_POISON:
		setAnim(CAnimationStateSheet::PoisonCastInit);
		break;
	case MBEHAV::CAST_ROOT:
		setAnim(CAnimationStateSheet::RootCastInit);
		break;
	case MBEHAV::CAST_ROT:
		setAnim(CAnimationStateSheet::RotCastInit);
		break;
	case MBEHAV::CAST_SHOCK:
		setAnim(CAnimationStateSheet::ShockCastInit);
		break;
	case MBEHAV::CAST_SLEEP:
		setAnim(CAnimationStateSheet::SleepCastInit);
		break;
	case MBEHAV::CAST_SLOW:
		setAnim(CAnimationStateSheet::SlowCastInit);
		break;
	case MBEHAV::CAST_STUN:
		setAnim(CAnimationStateSheet::StunCastInit);
		break;
	default:
		break;
	}
}// beginCast //

//-----------------------------------------------
// endCast :
//-----------------------------------------------
void CCharacterCL::endCast(const MBEHAV::CBehaviour &behaviour, const MBEHAV::CBehaviour &lastBehaviour)
{
	if( !(isUser() && isSit()) )
	{
		switch(lastBehaviour.Behaviour)
		{
		case MBEHAV::CAST_ACID:
			setAnim(CAnimationStateSheet::AcidCastEnd);
			break;
		case MBEHAV::CAST_BLIND:
			setAnim(CAnimationStateSheet::BlindCastEnd);
			break;
		case MBEHAV::CAST_COLD:
			setAnim(CAnimationStateSheet::ColdCastEnd);
			break;
		case MBEHAV::CAST_ELEC:
			setAnim(CAnimationStateSheet::ElecCastEnd);
			break;
		case MBEHAV::CAST_FEAR:
			setAnim(CAnimationStateSheet::FearCastEnd);
			break;
		case MBEHAV::CAST_FIRE:
			setAnim(CAnimationStateSheet::FireCastEnd);
			break;
		case MBEHAV::CAST_HEALHP:
			setAnim(CAnimationStateSheet::HealHPCastEnd);
			break;
		case MBEHAV::CAST_MAD:
			setAnim(CAnimationStateSheet::MadCastEnd);
			break;
		case MBEHAV::CAST_POISON:
			setAnim(CAnimationStateSheet::PoisonCastEnd);
			break;
		case MBEHAV::CAST_ROOT:
			setAnim(CAnimationStateSheet::RootCastEnd);
			break;
		case MBEHAV::CAST_ROT:
			setAnim(CAnimationStateSheet::RotCastEnd);
			break;
		case MBEHAV::CAST_SHOCK:
			setAnim(CAnimationStateSheet::ShockCastEnd);
			break;
		case MBEHAV::CAST_SLEEP:
			setAnim(CAnimationStateSheet::SleepCastEnd);
			break;
		case MBEHAV::CAST_SLOW:
			setAnim(CAnimationStateSheet::SlowCastEnd);
			break;
		case MBEHAV::CAST_STUN:
			setAnim(CAnimationStateSheet::StunCastEnd);
			break;
		case MBEHAV::IDLE:
			setAnim(CAnimationStateSheet::Idle);
			break;
		// Old One
		default:
			{
				switch(behaviour.Behaviour)
				{
					//////////////
					// END CAST //
					// OFFENSIVE CAST END
				case MBEHAV::CAST_OFF_FAIL:
					setAnim(CAnimationStateSheet::OffensiveCastFail);
					break;
				case MBEHAV::CAST_OFF_FUMBLE:
					setAnim(CAnimationStateSheet::OffensiveCastFumble);
					break;
				case MBEHAV::CAST_OFF_SUCCESS:
					setAnim(CAnimationStateSheet::OffensiveCastSuccess);
					break;
				case MBEHAV::CAST_OFF_LINK:
					setAnim(CAnimationStateSheet::OffensiveCastLink);
					break;
					// CURATIVE CAST END
				case MBEHAV::CAST_CUR_FAIL:
					setAnim(CAnimationStateSheet::CurativeCastFail);
					break;
				case MBEHAV::CAST_CUR_FUMBLE:
					setAnim(CAnimationStateSheet::CurativeCastFumble);
					break;
				case MBEHAV::CAST_CUR_SUCCESS:
					setAnim(CAnimationStateSheet::CurativeCastSuccess);
					break;
				case MBEHAV::CAST_CUR_LINK:
					setAnim(CAnimationStateSheet::CurativeCastLink);
					break;
					// MIXED CAST END
				case MBEHAV::CAST_MIX_FAIL:
					setAnim(CAnimationStateSheet::MixedCastFail);
					break;
				case MBEHAV::CAST_MIX_FUMBLE:
					setAnim(CAnimationStateSheet::MixedCastFumble);
					break;
				case MBEHAV::CAST_MIX_SUCCESS:
					setAnim(CAnimationStateSheet::MixedCastSuccess);
					break;
				case MBEHAV::CAST_MIX_LINK:
					setAnim(CAnimationStateSheet::MixedCastLink);
					break;
				default:
					break;
				}
			}
			break;
		}
	}
}// endCast //


// *************************************************************************************************
void CCharacterCL::updateCurrentAttack()
{
	// This is a behaviour for the magic.
	if(_CurrentBehaviour.isMagic())
	{
		_CurrentAttackID.Type = CAttackIDSheet::Magic;
		_CurrentAttackID.SpellInfo.Mode = (MAGICFX::TSpellMode) _CurrentBehaviour.Spell.SpellMode;
		_CurrentAttackID.SpellInfo.ID = (MAGICFX::TMagicFx) _CurrentBehaviour.Spell.SpellId;
	}
	// This is a behaviour for the combat.
	else if(_CurrentBehaviour.Behaviour == MBEHAV::RANGE_ATTACK)
	{
		_CurrentAttackID.Type = CAttackIDSheet::Range;
		_CurrentAttackID.RangeWeaponType = (RANGE_WEAPON_TYPE::TRangeWeaponType) _CurrentBehaviour.Range.WeaponType;
	}
	else if (_CurrentBehaviour.isCombat())
	{
		_CurrentAttackID.Type = CAttackIDSheet::Melee;
	}
	else if (_CurrentBehaviour.isCreatureAttack())
	{
		_CurrentAttackID.Type = CAttackIDSheet::Creature;
		_CurrentAttackID.CreatureAttackIndex = _CurrentBehaviour.Behaviour == MBEHAV::CREATURE_ATTACK_0 ? 0 : 1;
	}
	else
	{
		// the behaviour does not generate an attack
		_CurrentAttack = NULL;
		_CurrentAttackID.Type = CAttackIDSheet::Unknown;
		return;
	}
	_CurrentAttack = getAttack(_CurrentAttackID);
	// update current attack infos
	if(_CurrentBehaviour.isMagic())
	{
		_CurrentAttackInfo.Intensity = _CurrentBehaviour.Spell.SpellIntensity;
		// physical damge are irrelevant for magic
		_CurrentAttackInfo.DamageType = DMGTYPE::UNDEFINED;
		_CurrentAttackInfo.HitType = HITTYPE::Undefined;
		_CurrentAttackInfo.Localisation = BODY::UnknownBodyPart;
		_CurrentAttackInfo.PhysicalImpactIntensity = 0;
	}
	// This is a behaviour for the combat.
	else if(_CurrentBehaviour.Behaviour == MBEHAV::RANGE_ATTACK)
	{
		_CurrentAttackInfo.Intensity = _CurrentBehaviour.Range.ImpactIntensity;
		_CurrentAttackInfo.DamageType = DMGTYPE::UNDEFINED;
		_CurrentAttackInfo.HitType = HITTYPE::Undefined;
		_CurrentAttackInfo.Localisation = (BODY::TBodyPart) _CurrentBehaviour.Range.Localisation;
		_CurrentAttackInfo.PhysicalImpactIntensity = 0;
	}
	else if (_CurrentBehaviour.isCombat())
	{
		_CurrentAttackInfo.Intensity = _CurrentBehaviour.Combat.ImpactIntensity;
		_CurrentAttackInfo.DamageType = (DMGTYPE::EDamageType) _CurrentBehaviour.Combat2.DamageType;
		_CurrentAttackInfo.HitType = (HITTYPE::THitType) _CurrentBehaviour.Combat.HitType;
		_CurrentAttackInfo.Localisation = (BODY::TBodyPart) _CurrentBehaviour.Combat.Localisation;
		_CurrentAttackInfo.PhysicalImpactIntensity = _CurrentBehaviour.Combat.ImpactIntensity;
	}
	else if (_CurrentBehaviour.isCreatureAttack())
	{
		// creature attack is the most general form
		_CurrentAttackInfo.Intensity = _CurrentBehaviour.CreatureAttack.MagicImpactIntensity;
		_CurrentAttackInfo.DamageType = (DMGTYPE::EDamageType) _CurrentBehaviour.CreatureAttack2.DamageType;
		_CurrentAttackInfo.HitType = (HITTYPE::THitType) _CurrentBehaviour.CreatureAttack2.HitType;
		_CurrentAttackInfo.Localisation = (BODY::TBodyPart) _CurrentBehaviour.CreatureAttack.Localisation;
		_CurrentAttackInfo.PhysicalImpactIntensity = _CurrentBehaviour.CreatureAttack.ImpactIntensity;
	}
	_CurrentAttackInfo.Side = (rand() & 1) ? BODY::Left : BODY::Right;
}


// utility function for performCurrentAttackEnd
inline static void getResistAndDistance(uint8 packedInfo, bool isDirectAttack, bool isCombat, bool &resist, uint &distance)
{
	// get the distance from attacker to defender (consider 0 if a direct attack)
	if (isDirectAttack)
		distance = 0;
	else
		distance = packedInfo & 127;
	// resisted?
	if(isCombat)
		resist= false;
	else
		resist = (packedInfo & 128) != 0;
}

// *********************************************************************************************
void CCharacterCL::performCurrentAttackEnd(const CBehaviourContext &bc, bool directOffensifSpell, vector<double> &targetHitDates, TAnimStateKey animForCombat)
{
	if (!_CurrentAttack) return;
	if (!_CurrentAttack->Sheet) return;
	if (bc.Targets.Targets.empty())
	{
		nlwarning ("No target available for current attack.");
		return;
	}
	nlassert(targetHitDates.size() == bc.Targets.Targets.size());

	CAttackInfo attackInfo = _CurrentAttackInfo;

	const CAttackSheet &sheet = *_CurrentAttack->Sheet;


	// should cast FX for static Objects like towers be used ?
	bool usesStaticCastFX = _Sheet && !_Sheet->ProjectileCastRay.empty();

	CProjectileBuild pb;


	// ray of cast for static object with several cast points
	CVector castWorldOrigin;
	CVector castWorldPos;
	CVector additionnalOffset = NLMISC::CVector::Null;
	bool castRayValid = false;
	uint mainCastFXIntensity = attackInfo.Intensity;


	// *** Compute castStartTime (time at which the projectile is casted)

	// *** Compute castStartTime (time at which the projectile is casted)

	float delay = 0.f; // by default, no delay before impact
	double timeFactor = 1;

	// Combat (range or melee) => no resist (yoyo: legacy code. Actually the server should set the resist bit to 0 in Target.Info)
	bool isCombat= _CurrentBehaviour.isCombat();

	// An attack is said 'direct' when no projectile is casted (Generally those are melee attack, but also pistol range attack for instance)
	// In this case, all targets are hit at the same time, no matter what date is supplied for them.
	bool isDirectAttack = sheet.ProjectileFX.empty();

	if (isDirectAttack)
	{
		///////////////////
		// DIRECT ATTACK //
		///////////////////
		// compute impact date. It is given by the trigger on right hand in the animation
		// Get the current animation id.
		if (sheet.ForceUseProjectileDelay)
		{
			delay = sheet.ProjectileDelay;
		}
		else
		if (!directOffensifSpell)
		{
			if ((_CurrentAttackID.Type != CAttackIDSheet::Range) && (_PlayList != NULL))
			{
				// default
				delay= 0.5f;
				// if the animation has been correctly chosen
				if(animForCombat!=CAnimationStateSheet::UnknownState)
				{
					// try to get the MeleeImpactDelay that is stored in the sheet (given by graphists)
					const CAnimationSet	*animSet= currentAnimSet()[MOVE];
					if(animSet)
					{
						const CAnimationState *animState  = animSet->getAnimationState(animForCombat);
						if(animState)
							delay= animState->getMeleeImpactDelay();
					}
				}
			}
			else
			{
				// see if weapon is known, and in this case, delay the impact by the same amount
				const CItemSheet *sheet = _Items[SLOTTYPE::RIGHT_HAND_SLOT].Sheet;
				if (sheet) delay = sheet->FX.ImpactFXDelay;
			}
		}
		// all target are reached at the same time
		timeFactor = 0;
	}
	else
	{
		// there's a projectile
		if (usesStaticCastFX)
		{
			delay = sheet.StaticObjectProjectileDelay;
		}
		else
		{
			if (!directOffensifSpell)
			{
				// if object has a list of cast rays, then we assume it is a static object (like guard towers)
				// projectile cast from a character
				delay = sheet.ProjectileDelay;
			}
		}
	}

	// then castStartTime is just start of behav + animation delay
	double	castStartTime;
	// Special for direct attack. because of lag sometimes behavTime<TimeInSec, which results
	// on too early played hits regarding animation played
	if(isDirectAttack)
	{
		// start at max(behavTime, TimeInSec), but with a limit of bc.BehavTime+0.5 sec
		if(TimeInSec > bc.BehavTime+0.5)
			castStartTime= bc.BehavTime + 0.5;
		else if(TimeInSec > bc.BehavTime)
			castStartTime= TimeInSec;
		else
			castStartTime= bc.BehavTime;
		// add the delay
		castStartTime+= delay;
	}
	else
	{
		// In case of magic or range (with projectile), this is not required because the projectilemanager should take cares of
		// cast start time and current time (and therfore advance the projectile if required)
		castStartTime= bc.BehavTime + delay;
	}

	// *** Casts projectiles and Impacts
	bool resist;
	uint distance;
	// there's a projectile
	switch(sheet.ProjectileMode)
	{
		// TODO: homin code for projectile look quite similar, maybe can merge ?
		case MAGICFX::Bomb:
		{
			getResistAndDistance(bc.Targets.Targets[0].Info, isDirectAttack, isCombat, resist, distance);
			double mainStartDate = castStartTime;
			double mainEndDate = mainStartDate + timeFactor * double(distance * MULTI_TARGET_DISTANCE_UNIT / MAGICFX::PROJECTILE_SPEED);
			CCharacterCL *mainTarget = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(bc.Targets.Targets[0].TargetSlot));
			if (mainTarget)
			{
				computeTargetStickMode(*_CurrentAttack->Sheet, attackInfo, pb.ProjectileAimingPoint, *mainTarget);
				if (usesStaticCastFX)
				{
					// compute the cast pos
					computeBestCastRay(*mainTarget, pb.ProjectileAimingPoint, castWorldOrigin, castWorldPos, additionnalOffset);
					castRayValid = true;
				}

				// the target hit date
				targetHitDates[0]= mainEndDate;

				// Add the projectile to queue
				if (createCurrentAttackEndPart(pb,
					                           _CurrentAttack,
											   *mainTarget,
											   NULL,
											   mainStartDate,
											   mainEndDate,
											   true,
											   sheet.PlayImpactAnim,
											   resist,
											   sheet.IsImpactLocalised,
											   attackInfo,
											   additionnalOffset
											  ))
				{
					CProjectileManager::getInstance().addProjectileToQueue(pb);

				}
				attackInfo.Intensity = attackInfo.Intensity != 0 ? 1 : 0;
				attackInfo.PhysicalImpactIntensity =  attackInfo.PhysicalImpactIntensity != 0 ? 1 : 0;
				// all subsequent projectiles are casted from the secondary target, from the first impact point, and have level 1
				for(uint k = 1; k < bc.Targets.Targets.size(); ++k)
				{
					getResistAndDistance(bc.Targets.Targets[k].Info, isDirectAttack, isCombat, resist, distance);
					double secondaryEndDate = mainEndDate + timeFactor * ((double) (distance * MULTI_TARGET_DISTANCE_UNIT / MAGICFX::PROJECTILE_SPEED));
					CCharacterCL *secondaryTarget = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(bc.Targets.Targets[k].TargetSlot));
					if (secondaryTarget)
					{
						// the target hit date
						targetHitDates[k]= secondaryEndDate;

						// Add the projectile to queue
						if (mainTarget->createCurrentAttackEndPart(pb,
							                                       _CurrentAttack,
																   *secondaryTarget,
																   &pb.ProjectileAimingPoint,
																   mainEndDate,
																   secondaryEndDate,
																   k != 0 ? !sheet.PlayImpactFXOnlyOnMainTarget : true,
																   sheet.PlayImpactAnim,
																   resist,
																   sheet.IsImpactLocalised,
																   attackInfo
																  ))
						{
							computeTargetStickMode(*_CurrentAttack->Sheet, attackInfo, pb.ProjectileAimingPoint, *secondaryTarget);
							CProjectileManager::getInstance().addProjectileToQueue(pb);
						}
					}
				}
			}
		}
		break;
		case MAGICFX::Chain:
		{
			double currDate = castStartTime;
			CCharacterCL *currCaster = this;
			const CFXStickMode *projectileStartPoint = NULL; // by default, start at caster hand
			CFXStickMode currStickMode;
			for(uint k = 0; k < bc.Targets.Targets.size(); ++k)
			{
				if (k == 1)
				{
					// for secondary impacts, intensity is 0 or 1
					attackInfo.Intensity = attackInfo.Intensity != 0 ? 1 : 0;
					attackInfo.PhysicalImpactIntensity =  attackInfo.PhysicalImpactIntensity != 0 ? 1 : 0;
				}
				getResistAndDistance(bc.Targets.Targets[k].Info, isDirectAttack, isCombat, resist, distance);
				double nextDate = currDate + timeFactor * ((double) (distance * MULTI_TARGET_DISTANCE_UNIT / MAGICFX::PROJECTILE_SPEED));
				CCharacterCL *currTarget = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(bc.Targets.Targets[k].TargetSlot));
				if (!currTarget) break;


				computeTargetStickMode(*_CurrentAttack->Sheet, attackInfo, pb.ProjectileAimingPoint, *currTarget);


				if (k == 0 && usesStaticCastFX)
				{
					// compute the initial cast pos for objects with multiple possible cast positions
					computeBestCastRay(*currTarget, pb.ProjectileAimingPoint, castWorldOrigin, castWorldPos, additionnalOffset);
					castRayValid = true;
				}

				// the target hit date
				targetHitDates[k]= nextDate;

				// Add the projectile to queue
				if (currCaster->createCurrentAttackEndPart(pb,
														   _CurrentAttack,
														   *currTarget,
														   projectileStartPoint,
														   currDate,
														   nextDate,
														   k != 0 ? !sheet.PlayImpactFXOnlyOnMainTarget : true,
														   sheet.PlayImpactAnim,
														   resist,
														   sheet.IsImpactLocalised,
														   attackInfo,
														   k == 0 ? additionnalOffset : CVector::Null
														  ))
				{
					CProjectileManager::getInstance().addProjectileToQueue(pb);
				}
				currStickMode = pb.ProjectileAimingPoint;
				projectileStartPoint = &currStickMode;
				currCaster = currTarget;
				if (!currCaster) break;
				currDate = nextDate;
			}
		}
		break;
		case MAGICFX::Spray:
		{
			for(uint k = 0; k < bc.Targets.Targets.size(); ++k)
			{
				getResistAndDistance(bc.Targets.Targets[k].Info, isDirectAttack, isCombat, resist, distance);
				double startDate = castStartTime;
				double endDate = startDate + timeFactor * ((double) (distance * MULTI_TARGET_DISTANCE_UNIT / MAGICFX::PROJECTILE_SPEED));
				CCharacterCL *currTarget = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(bc.Targets.Targets[k].TargetSlot));
				if (k == 1)
				{
					// for secondary impacts, intensity is 0 or 1
					attackInfo.Intensity = attackInfo.Intensity != 0 ? 1 : 0;
					attackInfo.PhysicalImpactIntensity =  attackInfo.PhysicalImpactIntensity != 0 ? 1 : 0;
				}
				if (currTarget)
				{
					computeTargetStickMode(*_CurrentAttack->Sheet, attackInfo, pb.ProjectileAimingPoint, *currTarget);
					if (k == 0 && usesStaticCastFX)
					{
						// compute the initial cast pos for objects with multiple possible cast positions
						computeBestCastRay(*currTarget, pb.ProjectileAimingPoint, castWorldOrigin, castWorldPos, additionnalOffset);
						castRayValid = true;
					}

					// the target hit date
					targetHitDates[k]= endDate;

					// nb : only main target display the spell with full power
					if (createCurrentAttackEndPart(pb,
						                           _CurrentAttack,
												   *currTarget,
												   NULL,
												   startDate,
												   endDate,
												   k != 0 ? !sheet.PlayImpactFXOnlyOnMainTarget : true,
												   sheet.PlayImpactAnim,
												   resist,
												   sheet.IsImpactLocalised,
												   attackInfo,
												   k == 0 ? additionnalOffset : CVector::Null
												  ))
					{
						CProjectileManager::getInstance().addProjectileToQueue(pb);
					}
				}
			}
		}
		break;
		default:
		break;
	}

	// if object has a list of cast rays, then we assume it is a static object (like guard towers)
	if (usesStaticCastFX && castRayValid)
	{
		buildStaticObjectCastFX(castWorldOrigin, castWorldPos, *_CurrentAttack->Sheet, mainCastFXIntensity);
	}


	// *** Play damage shields when melee attack is done
	/*
	// TODO: to finalize (server code not done).
	if (ClientCfg.DamageShieldEnabled && _CurrentBehaviour.isCombat() && _CurrentBehaviour.todoNotRange())
	{
		for(uint k = 0; k < bc.Targets.Targets.size(); ++k)
		{
			uint power = bc.Targets.Targets[k].Info & 7;
			if (power)
			{
				uint dmType = bc.Targets.Targets[k].Info >> 3;
				//
				CAttackIDSheet damageShieldID;
				damageShieldID.Type = CAttackIDSheet::DamageShield;
				damageShieldID.DamageShieldType = dmType;
				CCharacterCL *currTarget = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(bc.Targets.Targets[k].TargetSlot));
				if (!currTarget) continue;
				const CAttack *damageShieldReaction = currTarget->getAttack(damageShieldID);
				if (!damageShieldReaction) continue;
				CAttackInfo attackInfo;
				attackInfo.Intensity = power;
				attackInfo.PhysicalImpactIntensity = 0;
				attackInfo.Localisation = BODY::UnknownBodyPart;
				// no physical part
				const CAttackSheet &damageShieldSheet = *damageShieldReaction->Sheet;
				computeTargetStickMode(damageShieldSheet, attackInfo, pb.ProjectileAimingPoint, *this);
				if (currTarget->createCurrentAttackEndPart(pb,
					                                   damageShieldReaction,
													   *this,
													   NULL,
													   castStartTime,
													   castStartTime,
													   true,
													   damageShieldSheet.PlayImpactAnim,
													   false,
													   false,
													   attackInfo))
				{
					// play "CastEnd" at the good date (if any ...)
					pb.CastAspect = &damageShieldReaction->AttackEndFX;
					pb.CastPower = power;
					pb.ForcePlayImpact = true;
					CProjectileManager::getInstance().addProjectileToQueue(pb);
				}
			}
		}
	}
	*/
}

// *********************************************************************************************
void CCharacterCL::buildStaticObjectCastFX(const NLMISC::CVector &castWorldOrigin, NLMISC::CVector &castWorldPos, const CAttackSheet &/* sheet */, uint intensity)
{
	if (intensity == 0) return;
	const float *userParams = CProjectileManager::getProjectileFXUserParams(intensity);
	// create additionnal cast fxs on the tower or other static object (if any)
	// Build lookat matrix (with respect to axis) looking from castWorldOrigin to castWorldPos
	CVector dir = castWorldPos - castWorldOrigin;
	CVector I = dir.normed();
	CVector K = (CVector::K - (I * CVector::K) * I).normed();
	CMatrix castMat;
	castMat.setPos(castWorldPos);
	castMat.setRot(I, K ^ I, K);
	CAttachedFX::CBuildInfo bi;
	CAttachedFX::CTargeterInfo ti;
	bi.StaticMatrix = &castMat;
	const CAnimationFXSet &afs =_CurrentAttack->AttackStaticObjectCastFX;
	for (uint k = 0; k < afs.FX.size(); ++k)
	{
		// if the fx is in looping mode, & the anim has already done a loop, then don't recreate it
		CAttachedFX::TSmartPtr fx = new CAttachedFX;
		bi.Sheet = &afs.FX[k];
		fx->create(*this, bi, ti);
		if (!fx->FX.empty())
		{
			if (userParams)
			{
				for(uint l = 0; l < 4; ++l)
				{
					fx->FX.setUserParam(l, userParams[l]);
				}
			}
			attachFX(fx);
		}
	}
}

// *********************************************************************************************
void CCharacterCL::computeTargetStickMode(const CAttackSheet &sheet, const CAttackInfo &attackInfo, CFXStickMode &dest, CEntityCL &target)
{
	bool hasPhysicalImpact = false;
	if (attackInfo.Localisation != BODY::UnknownBodyPart &&
		attackInfo.PhysicalImpactIntensity >= 1 &&
		attackInfo.PhysicalImpactIntensity <= MAGICFX::NUM_SPELL_POWER &&
		attackInfo.HitType != HITTYPE::Failed &&
		attackInfo.HitType != HITTYPE::Undefined &&
		attackInfo.DamageType != DMGTYPE::UNDEFINED)
	{
		hasPhysicalImpact = true;
	}

	if (sheet.IsImpactLocalised || hasPhysicalImpact)
	{
		// get projectile impact point from localisation
		// & generate stick mode
		const char *targetBoneName = target.getBoneNameFromBodyPart(attackInfo.Localisation, attackInfo.Side);
		if (targetBoneName)
		{
			if (sheet.DefaultAimingPoint.Mode == CFXStickMode::UserBoneOrientedTowardTargeter)
			{
				dest.Mode = CFXStickMode::UserBoneOrientedTowardTargeter;
			}
			else
			{
				dest.Mode = CFXStickMode::UserBone;
			}
			dest.UserBoneName = CStringMapper::map(targetBoneName);
		}
	}
	else
	{
		// use default aiming point given in sheet
		dest = sheet.DefaultAimingPoint;
	}
}


// *********************************************************************************************
bool CCharacterCL::createCurrentAttackEndPart(CProjectileBuild &destPB,
											  const CAttack *currentAttack,
											  const CCharacterCL &target,
										      const CFXStickMode *sm,
										      double spawnDate,
										      double hitDate,
										      bool playImpactFX,
											  bool playImpactAnim,
										      bool magicResist,
										      bool /* mainImpactIsLocalised */,
										      const CAttackInfo &attackInfo,
											  const NLMISC::CVector &additionnalOffset /*= NLMISC::CVector::Null*/
										     )
{
	if (!currentAttack) return false;
	if (!currentAttack->Sheet) return false;


	const CAttackSheet &sheet = *currentAttack->Sheet;

	// dates
	destPB.StartDate = spawnDate;
	destPB.EndDate = hitDate;
	// choose fx for projectile
	if (attackInfo.Intensity >= 1 && attackInfo.Intensity <= MAGICFX::NUM_SPELL_POWER)
	{
		destPB.ProjectileAspect = &currentAttack->ProjectileFX;
	}
	else
	{
		destPB.ProjectileAspect = NULL;
	}
	// choose fx for impact
	if (!playImpactFX)
	{
		destPB.ImpactAspect = NULL;
	}
	else
	{
		if (attackInfo.Intensity  >= 1 && attackInfo.Intensity <= MAGICFX::NUM_SPELL_POWER) // impact has same intensity than projectile
		{
			destPB.ImpactAspect = &currentAttack->ImpactFX;
		}
		else
		{
			destPB.ImpactAspect = NULL;
		}
	}

	// FILL PROJECTILE BUILD
	destPB.AttackInfo = attackInfo;
	destPB.TargeterInfo.Slot = slot();
	//

	destPB.LocalizedImpact = sheet.IsImpactLocalised;
	// If this is a secondary projectile, it may start from another location, which is the impact point of the previous projectile
	// (so it doesn't start from the caster hand, or any around settings that is read from the spell sheet)
	if (sm) // start stickmode wanted ?
	{
		destPB.TargeterInfo.StickMode = *sm;
	}
	else
	{
		// if no stick mode is not forced, then use the one given in the  projectile sheet
		if (destPB.ProjectileAspect && !destPB.ProjectileAspect->FX.empty())
		{
			destPB.TargeterInfo.StickMode = destPB.ProjectileAspect->FX[0].Sheet->StickMode;
		}
		else
		{
			// if no projectile is given, then uses the default casting point
			destPB.TargeterInfo.StickMode = sheet.DefaultCastingPoint;
		}
	}
	destPB.Mode = sheet.ProjectileMode;
	destPB.Target.Slot = target.slot();
	destPB.TargeterInfo.StickOffset = CVector::Null;
	destPB.PlayImpactAnim = playImpactAnim;
	destPB.LetProjectileStickedOnTarget = sheet.LetProjectileStickedOnTarget;
	destPB.TargeterInfo.DefaultPos = pos().asVector();
	//
	destPB.MagicResist = magicResist;
	// offset if projectile is launched from a range weapon and projectile is sticked to box_arme
	if (destPB.TargeterInfo.StickMode.Mode == CFXStickMode::UserBone && sheet.ApplyItemOffsetToWeaponBone)
	{
		// should be fired from the 'box_arme' bone, which means it is fired from a weapon
		if (CStringMapper::unmap(destPB.TargeterInfo.StickMode.UserBoneName) == "box_arme")
		{
			NLMISC::CVector projectileOffset = NLMISC::CVector::Null;
			const CItemSheet *is = getRightHandItemSheet();
			if (is)
			{
				destPB.TargeterInfo.StickOffset = is->FX.AttackFXOffset;
			}
			destPB.TargeterInfo.StickOffset += additionnalOffset;
		}
	}
	destPB.TargeterInfo.StickOffset += sheet.AdditionnalStartOffset;
	return true;
}


// *********************************************************************************************
void CCharacterCL::computeBestCastRay(CEntityCL			 &targetEntity,
									  const CFXStickMode &targetStickMode,
									  NLMISC::CVector    &castWorldOrigin,
									  NLMISC::CVector    &castWorldPos,
									  NLMISC::CVector    &worldOffsetToCasterPivot
									 ) const
{
	// additionnal offset taken from sheet. Useful for towers that have no bones, but can fire projectiles anyway
	nlassert(_Sheet && !_Sheet->ProjectileCastRay.empty());
	// if several offsets are provided, then choose the one that has the smallest angle towards target
	CVector target;
	CProjectileManager::evalFXPosition(&targetStickMode, targetEntity, target);
	float   maxDP3 = -FLT_MAX;
	// NB : the offset is relative to object pivot, not to a bone
	CMatrix casterMatrix;
	buildAlignMatrix(casterMatrix);
	for(uint k = 0; k < _Sheet->ProjectileCastRay.size(); ++k)
	{
		CVector currCastWorldPos    = casterMatrix * _Sheet->ProjectileCastRay[k].Pos;
		CVector currCastWorldOrigin = casterMatrix * _Sheet->ProjectileCastRay[k].Origin;
		float dp3 = (target - currCastWorldPos).normed() * (currCastWorldPos - currCastWorldOrigin).normed();
		if (dp3 > maxDP3)
		{
			maxDP3 = dp3;
			worldOffsetToCasterPivot = casterMatrix.mulVector(_Sheet->ProjectileCastRay[k].Pos);
			castWorldOrigin = currCastWorldOrigin;
			castWorldPos = currCastWorldPos;
		}
	}
}

// *********************************************************************************************
bool CCharacterCL::isCurrentBehaviourAttackEnd() const
{
	switch(_CurrentBehaviour.Behaviour)
	{
		case MBEHAV::CAST_OFF_SUCCESS:
		case MBEHAV::CAST_OFF_LINK:
		case MBEHAV::CAST_CUR_SUCCESS:
		case MBEHAV::CAST_CUR_LINK:
		case MBEHAV::CAST_MIX_SUCCESS:
		case MBEHAV::CAST_MIX_LINK:
		case MBEHAV::RANGE_ATTACK:
		case MBEHAV::CREATURE_ATTACK_0:
		case MBEHAV::CREATURE_ATTACK_1:
		case MBEHAV::DEFAULT_ATTACK:
		case MBEHAV::POWERFUL_ATTACK:
		case MBEHAV::AREA_ATTACK:
			return true;
		default:
			break;
	}
	return false;
}


// ***************************************************************************
void CCharacterCL::applyBehaviourFlyingHPs(const CBehaviourContext &bc, const MBEHAV::CBehaviour &behaviour,
								 const vector<double> &targetHitDates)
{
	nlassert(targetHitDates.size()==bc.Targets.Targets.size());

	if(!bc.Targets.Targets.empty())
	{
		if(behaviour.DeltaHP != 0)
		{
			CRGBA deltaHPColor( 0, 0, 0 );
			// if it's a hit
			if( behaviour.DeltaHP < 0 )
			{
				// if the behaviour is casted by the user
				if( slot() == 0 )
				{
					deltaHPColor = ClientCfg.SystemInfoParams["dgm"].Color;
				}
				else
					// if the behaviour is casted by an entity that target the user
					if( targetSlot() == 0 )
					{
						CEntityCL *actor = EntitiesMngr.entity(slot());
						if( actor )
						{
							// if actor is player : use pvp color
							if( actor->isPlayer() )
								deltaHPColor = ClientCfg.SystemInfoParams["dgp"].Color;
							else
								deltaHPColor = ClientCfg.SystemInfoParams["dg"].Color;
						}
					}
					else
					{
						deltaHPColor = CRGBA(127,127,127);
					}
			}
			else
			{
				deltaHPColor = CRGBA(0,220,0);
			}

			// Set the delta HP
			for (size_t i=0; i<bc.Targets.Targets.size(); ++i)
			{
				CEntityCL *target2 = EntitiesMngr.entity(bc.Targets.Targets[i].TargetSlot);
				if(target2)
					target2->addHPOutput(behaviour.DeltaHP, deltaHPColor, float(targetHitDates[i]-TimeInSec));
			}
		}
	}
}


//-----------------------------------------------
// Apply the behaviour.
// \param behaviour : the behaviour to apply.
//-----------------------------------------------
void CCharacterCL::applyBehaviour(const CBehaviourContext &bc)	// virtual
{
	// Backup the current behaviour.
	CBehaviour previousBehaviour = _CurrentBehaviour;
	_CurrentBehaviour = bc.Behav;
	const CBehaviour &behaviour = bc.Behav;

	// check if self-target
	bool selfSpell = false;
	if (bc.Targets.Targets.size() == 1)
	{
		if (bc.Targets.Targets[0].TargetSlot == 0)
		{
			selfSpell = true;
		}
	}
	bool isOffensif;
	switch(behaviour.Behaviour)
	{
		case MBEHAV::CAST_OFF:
		case MBEHAV::CAST_OFF_FAIL:
		case MBEHAV::CAST_OFF_SUCCESS:
		case MBEHAV::CAST_OFF_LINK:
			isOffensif = true;
		break;
		default:
			isOffensif = false;
		break;
	}

	// Get a pointer on the target.
	CEntityCL *target = EntitiesMngr.entity(targetSlot());


	// ***** Choose Combat Animation to apply

	TAnimStateKey	combatAnimState= CAnimationStateSheet::UnknownState;
	bool			isMovingCombatAnimState= false;
	// if the behaviour is for combat, compute now the animation to apply
	if( !behaviour.isMagic() && ( behaviour.isCombat() || behaviour.isCreatureAttack() ) )
	{
		// Atk Animation when moving
		if(_CurrentState && _CurrentState->Move && (_CurrentState->OnAtk != CAnimationStateSheet::UnknownState))
		{
			combatAnimState= _CurrentState->OnAtk;
			isMovingCombatAnimState= true;
		}
		// Atk Animation when NOT moving
		else
		{
			// select the combat animation.
			switch(behaviour.Behaviour)
			{
			case CREATURE_ATTACK_0:
				combatAnimState= CAnimationStateSheet::Attack1;
				break;
			case CREATURE_ATTACK_1:
				combatAnimState= CAnimationStateSheet::Attack2;
				break;
			// Default Animation
			case DEFAULT_ATTACK:
				switch(getAttackHeight(target, (BODY::TBodyPart)behaviour.Combat.Localisation, BODY::Right))
				{
				// LOW
				case CCharacterCL::AtkLow:
					combatAnimState= CAnimationStateSheet::DefaultAtkLow;
					break;
				// HIGH
				case CCharacterCL::AtkHigh:
					combatAnimState= CAnimationStateSheet::DefaultAtkHigh;
					break;
				// MIDDLE or Default
				case CCharacterCL::AtkMiddle:
				default:
					combatAnimState= CAnimationStateSheet::DefaultAtkMiddle;
					break;
				}
				break;
			// Powerful Animation
			case POWERFUL_ATTACK:
				switch(getAttackHeight(target, (BODY::TBodyPart)behaviour.Combat.Localisation, BODY::Right))
				{
				// LOW
				case CCharacterCL::AtkLow:
					combatAnimState= CAnimationStateSheet::PowerfulAtkLow;
					break;
				// HIGH
				case CCharacterCL::AtkHigh:
					combatAnimState= CAnimationStateSheet::PowerfulAtkHigh;
					break;
				// MIDDLE or Default
				case CCharacterCL::AtkMiddle:
				default:
					combatAnimState= CAnimationStateSheet::PowerfulAtkMiddle;
					break;
				}
				break;
			// Area Animation
			case AREA_ATTACK:
				switch(getAttackHeight(target, (BODY::TBodyPart)behaviour.Combat.Localisation, BODY::Right))
				{
				// LOW
				case CCharacterCL::AtkLow:
					combatAnimState= CAnimationStateSheet::AreaAtkLow;
					break;
				// HIGH
				case CCharacterCL::AtkHigh:
					combatAnimState= CAnimationStateSheet::AreaAtkHigh;
					break;
				// MIDDLE or Default
				case CCharacterCL::AtkMiddle:
				default:
					combatAnimState= CAnimationStateSheet::AreaAtkMiddle;
					break;
				}
				break;
			// Range Animation
			case RANGE_ATTACK:
				combatAnimState= CAnimationStateSheet::Attack1;
				break;
			default:
				break;
			}
		}
	}


	// ***** Compute Impact delays and cast missiles

	// Default target hit dates
	static	vector<double>	targetHitDates;
	targetHitDates.clear();
	targetHitDates.resize(bc.Targets.Targets.size(), TimeInSec);

	// Update Attack Projectiles and FXs
	updateCurrentAttack();
	if (isCurrentBehaviourAttackEnd())
	{
		// retrieve target hit dates, so flying HPs have the correct ones
		performCurrentAttackEnd(bc, selfSpell && isOffensif, targetHitDates, combatAnimState);
	}

	// INFO : display some debug information.
	if((VerboseAnimUser && _Slot==0) || (VerboseAnimSelection && _Slot == UserEntity->selection()))
		nlinfo("CH:applyBeh:%d: '%d(%s)'", _Slot, (sint)behaviour.Behaviour, behaviourToString((EBehaviour)behaviour.Behaviour).c_str());


	// ***** Apply the behaviour according to type

	// This is a behaviour for the magic.
	if(behaviour.isMagic())
	{
		// Execute the magic behaviour.
		switch(behaviour.Behaviour)
		{
			////////////////
			// BEGIN CAST //
			case MBEHAV::CAST_OFF:
			case MBEHAV::CAST_CUR:
			case MBEHAV::CAST_MIX:
			case MBEHAV::CAST_ACID:
			case MBEHAV::CAST_BLIND:
			case MBEHAV::CAST_COLD:
			case MBEHAV::CAST_ELEC:
			case MBEHAV::CAST_FEAR:
			case MBEHAV::CAST_FIRE:
			case MBEHAV::CAST_HEALHP:
			case MBEHAV::CAST_MAD:
			case MBEHAV::CAST_POISON:
			case MBEHAV::CAST_ROOT:
			case MBEHAV::CAST_ROT:
			case MBEHAV::CAST_SHOCK:
			case MBEHAV::CAST_SLEEP:
			case MBEHAV::CAST_SLOW:
			case MBEHAV::CAST_STUN:
				beginCast(behaviour);
				break;
			//////////////
			// END CAST //
			case MBEHAV::CAST_OFF_FAIL:
			case MBEHAV::CAST_OFF_FUMBLE:
				if (!selfSpell) endCast(behaviour, previousBehaviour);
			break;
			case MBEHAV::CAST_OFF_SUCCESS:
			case MBEHAV::CAST_OFF_LINK:
				endCast(behaviour, previousBehaviour);
			break;
			case MBEHAV::CAST_CUR_FAIL:
			case MBEHAV::CAST_CUR_FUMBLE:
				endCast(behaviour, previousBehaviour);
			break;
			case MBEHAV::CAST_CUR_SUCCESS:
			case MBEHAV::CAST_CUR_LINK:
				endCast(behaviour, previousBehaviour);
			break;
			case MBEHAV::CAST_MIX_FAIL:
			case MBEHAV::CAST_MIX_FUMBLE:
				endCast(behaviour, previousBehaviour);
			break;
			case MBEHAV::CAST_MIX_SUCCESS:
			case MBEHAV::CAST_MIX_LINK:
				endCast(behaviour, previousBehaviour);
			break;
			default:
			break;
		}
		// DeltaHP
		applyBehaviourFlyingHPs(bc, behaviour, targetHitDates);
	}
	// This is a behaviour for the combat.
	else if(behaviour.isCombat() || behaviour.isCreatureAttack())
	{
		float frontYawBefore = 0.f;
		float frontYawAfter = 0.f;

		// Atk Animation when NOT moving?
		if(target && !isMovingCombatAnimState)
		{
			// orientate to target
			CVectorD dirToTarget = target->pos() - pos();
			dirToTarget.z = 0;
			dirToTarget.normalize();
			if( !(isUser() && ClientCfg.AutomaticCamera == false) )
			{
				// backup front yaw
				frontYawBefore = frontYaw();
				front( dirToTarget );
			}
			dir( dirToTarget );
		}

		// Apply the state animation chosen before
		if(combatAnimState!=CAnimationStateSheet::UnknownState)
			setAnim(combatAnimState);

		// move camera so view doesn't change
		if( isUser() && frontYawBefore != 0.f )
		{
			frontYawAfter = frontYaw();
			float deltaYaw = frontYawAfter - frontYawBefore;
			if( deltaYaw !=0 )
			{
				UserControls.appendCameraDeltaYaw(-deltaYaw);
			}
		}

		// reset yaw smoothly to center view behind user
		if( isUser() && target && !target->isUser() && ClientCfg.AutomaticCamera )
		{
			UserControls.resetSmoothCameraDeltaYaw();
		}

		// DeltaHP
		applyBehaviourFlyingHPs(bc, behaviour, targetHitDates);
	}
	// Emote
	else if(behaviour.isEmote())
	{
		if(ClientCfg.Light==false && ClientCfg.EAMEnabled)
		{
			TAnimStateId emot;
			if (EAM)
			{
				/*
				// old code : fxs attached to emotes
				uint emoteIndex = behaviour.Behaviour-EMOTE_BEGIN;
				CTextEmotListSheet *pTELS = dynamic_cast<CTextEmotListSheet*>(SheetMngr.get(CSheetId("list.text_emotes")));
				if (pTELS)
				{
					if (emoteIndex < pTELS->TextEmotList.size())
					{
						const CTextEmotListSheet::STextEmot &emot = pTELS->TextEmotList[emoteIndex];
						if (!emot.FXToSpawn.empty())
						{
							// Compute the direction Matrix
							CMatrix fxMatrix;
							CVector vi = dir() ^ CVector::K;
							CVector vk = vi ^ dir();
							fxMatrix.setRot(vi, UserEntity->dir(), vk, true);
							fxMatrix.setPos(pos().asVector() + fxMatrix.getJ() * emot.FXSpawnDist);
							FXMngr.deferFX(emot.FXToSpawn, fxMatrix, emot.FXSpawnDelay);
						}
					}
				}
				*/
				if(EAM->getEmot(behaviour.Behaviour-EMOTE_BEGIN, emot))
					setAnim(CAnimationStateSheet::Emote, emot);
				else
					nlwarning("CH:applyBeh:%d: Emot '%d' unknown.", _Slot, behaviour.Behaviour-EMOTE_BEGIN);
			}
		}
	}
	// Others
	else
	{
		switch(behaviour.Behaviour)
		{
		// Loot Begin
		case MBEHAV::LOOT_INIT:
			setAnim(CAnimationStateSheet::LootInit);
			break;
		// Loot End
		case MBEHAV::LOOT_END:
			setAnim(CAnimationStateSheet::LootEnd);
			break;
		// Prospecting Begin
		case MBEHAV::PROSPECTING:
			setAnim(CAnimationStateSheet::ProspectingInit);
			break;
		// Prospecting End
		case MBEHAV::PROSPECTING_END:
			setAnim(CAnimationStateSheet::ProspectingEnd);
			break;
		// Extracting Begin
		case MBEHAV::EXTRACTING:
			// DeltaHP
			if(target)
				if(behaviour.DeltaHP != 0)
					target->addHPOutput(behaviour.DeltaHP,CRGBA(0,220,0));
			// If receiving a new DeltaHP in the current extraction, don't reset the animation
			if ( previousBehaviour.Behaviour != _CurrentBehaviour.Behaviour )
				setAnim(CAnimationStateSheet::UseInit);
			break;
		// Extracting End
		case MBEHAV::EXTRACTING_END:
			setAnim(CAnimationStateSheet::UseEnd);
			break;
		// Care Begin
		case MBEHAV::CARE:
			setAnim(CAnimationStateSheet::CareInit);
			break;
		// Care End
		case MBEHAV::CARE_END:
			setAnim(CAnimationStateSheet::CareEnd);
			break;

		// Begin to use a tool
		case MBEHAV::HARVESTING:
		case MBEHAV::FABER:
		case MBEHAV::REPAIR:
		case MBEHAV::REFINE:
		case MBEHAV::TRAINING:
			setAnim(CAnimationStateSheet::UseInit);
			break;

		// End to use a tool
		case MBEHAV::HARVESTING_END:
		case MBEHAV::FABER_END:
		case MBEHAV::REPAIR_END:
		case MBEHAV::REFINE_END:
		case MBEHAV::TRAINING_END:
			setAnim(CAnimationStateSheet::UseEnd);
			break;

		// Begin Stun
		case MBEHAV::STUNNED:
			setAnim(CAnimationStateSheet::StunBegin);
			break;

		// End Stun
		case MBEHAV::STUN_END:
			setAnim(CAnimationStateSheet::StunEnd);
			break;

		// Idle
		case IDLE:
			break;

		// Unknown behaviour -> idle.
		case UNKNOWN_BEHAVIOUR:
		default:
			nlwarning("CH::computeBehaviour : Entity in slot %d has an unknown behaviour %d to manage.", _Slot, (sint)behaviour.Behaviour);
			break;
		}
	}
}// computeBehaviour //

//-----------------------------------------------
// impact :
// Play an impact on the entity
// \param impactType : 0=magic, 1=melee
// \param type : see behaviour for spell
// \param intensity : see behaviour for spell
// \param id : see behaviour for spell
//-----------------------------------------------
void CCharacterCL::impact(uint /* impactType */, uint type, uint id, uint intensity)	// virtual
{
	// Display Magic Debug Infos
	if(Verbose & VerboseMagic)
		nlinfo("CH:impact:%d: type: %d, id: %d, intensity: %d", _Slot, type, id, intensity);
	// No Intensity -> No Impact
	if(intensity==0)
		return;
	// Invalid Intensity -> No Impact
	else if(intensity>5)
	{
		nlwarning("CH:impact:%d: invalid intensity %u", _Slot, intensity);
		return;
	}
	// RESIST : temp until resist is in the enum.
	if(type==0)
	{
		// Create the FX
		NL3D::UInstance resistFX = Scene->createInstance("Sp_Resist_Lev5.ps");
		if(!resistFX.empty())
			resistFX.setPos(pos());
		return;
	}
	// Compute the impact name
	string impact;
	if(id < ClientCfg.OffImpactFX.size())
		impact = ClientCfg.OffImpactFX[id];
	// Create the FX
	if(!impact.empty())
	{

		NL3D::UInstance impactFX = Scene->createInstance(impact);
		if(!impactFX.empty())
		{
			impactFX.setPos(pos());
			UParticleSystemInstance instFX;
			instFX.cast (impactFX);
			if(!instFX.empty())
			{
				// UserParam  |  Intensity 1  |  Intensity 2  |  Intensity 3  |  Intensity 4  |  Intensity 5
				//     0      |       0       |       0       |       1       |       1       |       1
				//     1      |       0       |       1       |       1       |       1       |       1
				//     2      |       0       |       0       |       0       |       1       |       1
				//     3      |       0       |       0       |       0       |       0       |       1
				float userParam0 = 0.0f;
				float userParam1 = 0.0f;
				float userParam2 = 0.0f;
				float userParam3 = 0.0f;
				// WARNING : there is no break and this is correct.
				switch(intensity)
				{
				case 5:
					userParam3 = 1.0f;
				case 4:
					userParam2 = 1.0f;
				case 3:
					userParam0 = 1.0f;
				case 2:
					userParam1 = 1.0f;
				}
				instFX.setUserParam(0, userParam0);
				instFX.setUserParam(1, userParam1);
				instFX.setUserParam(2, userParam2);
				instFX.setUserParam(3, userParam3);
			}
		}
	}
}// impact //

//-----------------------------------------------
// meleeImpact ::
//-----------------------------------------------
void CCharacterCL::meleeImpact(const CAttackInfo &attack)
{
	if (_Skeleton.empty()) return;
	if (attack.PhysicalImpactIntensity < 1 || attack.PhysicalImpactIntensity > 5) return;
	if (attack.HitType == HITTYPE::Failed) return;
	const char *boneName = getBoneNameFromBodyPart(attack.Localisation, attack.Side);
	if (!boneName) return;
	sint boneId = _Skeleton.getBoneIdByName(std::string(boneName));
	if (boneId == -1) return;
	if (!Scene) return;
	// choose good fx depending on the kind of damage
	NL3D::UInstance instance;
	switch(attack.DamageType)
	{
		case DMGTYPE::BLUNT:    instance = Scene->createInstance("mel_impactblunt.ps"); break;
		case DMGTYPE::SLASHING: instance = Scene->createInstance("mel_impactslashing.ps"); break;
		case DMGTYPE::PIERCING: instance = Scene->createInstance("mel_impactpiercing.ps"); break;
		default:
			return; // other types not supported
		break;
	}
	if (instance.empty()) return;
	UParticleSystemInstance impact;
	impact.cast (instance);
	if (impact.empty())
	{
		Scene->deleteInstance(instance);
		return;
	}
	// the 2 first user params of the fx are used to modulate intensity
	static const float intensityUP[5][2] =
	{
		{ 0.f,  0.f},
		{ 0.f,  0.5f},
		{ 0.5f, 0.5f},
		{ 0.5f, 1.f},
		{ 1.f,  1.f}
	};
	impact.setUserParam(0, intensityUP[attack.PhysicalImpactIntensity - 1][0]);
	impact.setUserParam(1, intensityUP[attack.PhysicalImpactIntensity - 1][1]);
	impact.setUserParam(2, (attack.HitType == HITTYPE::CriticalHit || attack.HitType == HITTYPE::CriticalHitResidual) ? 1.f : 0.f);
	impact.setUserParam(3, (attack.HitType == HITTYPE::HitResidual || attack.HitType == HITTYPE::CriticalHitResidual) ? 1.f : 0.f);
	//
	_Skeleton.stickObject(impact, boneId);
	// delegate managment of the impact to the fx manager
	FXMngr.fx2remove(impact);
}// meleeImpact //


//-----------------------------------------------
// magicImpact :
// Play the magic impact on the entity
// \param type : type of the impact (host/good/neutral).
// \param intensity : intensity of the impact.
//-----------------------------------------------
void CCharacterCL::magicImpact(uint type, uint intensity)	// virtual
{
	// --- FX --- //
	// Choose the FX.
	string impact;
	switch(type)
	{
	// Resist
	case 0:
		impact = "Sp_Resist_Lev";
		break;
	// Good
	case 1:
		impact = "Sp_Bien_Cure_Lev";
		break;
	// Neutral
	case 2:
		impact = "Sp_Neutre_Protect_Lev";
		break;
	// Bad
	case 3:
		impact = "Sp_Host_Hurt_Lev";
		break;
	default:
		nlwarning("CH:magicImpact:%d: Unknown type '%d'.", _Slot, type);
		return;
	}
	// Intensity
	switch(intensity)
	{
	// Too weak
	case INTENSITY_TYPE::IMPACT_NONE:
		return;

	case INTENSITY_TYPE::IMPACT_INSIGNIFICANT:
		impact += "1.ps";
		break;
	case INTENSITY_TYPE::IMPACT_VERY_WEAK:
		impact += "2.ps";
		break;
	case INTENSITY_TYPE::IMPACT_WEAK:
		impact += "3.ps";
		break;
	case INTENSITY_TYPE::IMPACT_AVERAGE:
		impact += "4.ps";
		break;
	case INTENSITY_TYPE::IMPACT_STRONG:
		impact += "5.ps";
		break;

	// Unknown
	default:
		nlwarning("CH:magicImpact:%d: Unknown intensity '%d'.", _Slot, intensity);
	}
	// Create the FX
	if(!impact.empty())
	{
		NL3D::UInstance resistFX = Scene->createInstance(impact);
		if(!resistFX.empty())
			resistFX.setPos(pos());
	}
}// resist //


//-----------------------------------------------
// getMaxSpeed :
// Return the basic max speed for the entity in meter per sec
//-----------------------------------------------
double CCharacterCL::getMaxSpeed()	const // virtual
{
	return _Sheet->MaxSpeed;
}// getMaxSpeed //


//-----------------------------------------------
// mode :
// Method called to change the mode (Combat/Mount/etc.).
//-----------------------------------------------
bool CCharacterCL::mode(MBEHAV::EMode m)
{
	// DEBUG INFOS
	if(VerboseAnimSelection && _Slot == UserEntity->selection())
	{
		nlinfo("CH::mode:%d: m'%s(%d)', _ModeWanted'%s(%d)', _Mode'%s(%d)'", _Slot,
			MBEHAV::modeToString(m          ).c_str(), m,
			MBEHAV::modeToString(_ModeWanted).c_str(), _ModeWanted,
			MBEHAV::modeToString(_Mode      ).c_str(), _Mode);
	}
	// Is the mode wanted valid ?
	if(m == MBEHAV::UNKNOWN_MODE || m >= MBEHAV::NUMBER_OF_MODES)
	{
		nlwarning("CH::mode:%d: Invalid Mode Wanted '%s(%d)' -> keep '%s(%d)'.",
			_Slot, MBEHAV::modeToString(m).c_str(), m, MBEHAV::modeToString(_Mode).c_str(), _Mode);
		return false;
	}
	// Set the mode wanted.
	_ModeWanted = m;
	if(_CurrentState == 0)
		_Mode = _ModeWanted;
	return true;
}// mode //


//-----------------------------------------------
// applyStage :
// Apply stage modifications.
// \todo GUIGUI ; ameliorer gestion mode.
//-----------------------------------------------
bool CCharacterCL::applyStage(CStage &stage)
{
	bool stageDone = true;

	// If the Stage has a position, backup the position and the stage time.
	if(stage.getPos(_OldPos))
	{
		// Backup the time
		_OldPosTime = stage.time();
		// Remove the property.
		stage.removeProperty(PROPERTY_POSX);
		stage.removeProperty(PROPERTY_POSY);
		stage.removeProperty(PROPERTY_POSZ);
	}

	// Apply orientation.
	pair<bool, sint64> resultTeta = stage.property(PROPERTY_ORIENTATION);
	if(resultTeta.first)
	{
		float angleZ = *(float *)(&resultTeta.second);
		// server forces the entity orientation even if it cannot turn
		front(CVector((float)cos(angleZ), (float)sin(angleZ), 0.f), true, true, true);

		_TargetAngle = (float)angleZ;
		// Remove the property.
		stage.removeProperty(PROPERTY_ORIENTATION);
	}

	// Apply Mode.
	pair<bool, sint64> resultMode = stage.property(PROPERTY_MODE);
	if(resultMode.first)
	{
		// Get the mode from stage.
		uint8 mo = *(uint8 *)(&resultMode.second);
		// If the mode wanted is not the same, change the mode wanted.
		if(mo != _ModeWanted)
		{
			if(mode((MBEHAV::EMode)mo))
			{
				//stageDone = false;
				if(_Mode != _ModeWanted)
					stageDone = false;
				else
					stage.removeProperty(PROPERTY_MODE);
			}
			else
				stage.removeProperty(PROPERTY_MODE);
		}
		// If the mode wanted is not the same as the current mode -> Stage not done.
		else if(_Mode != _ModeWanted)
			stageDone = false;
		// Property applied -> Remove the property form stage.
		else
			stage.removeProperty(PROPERTY_MODE);
	}

	// Apply Behaviour.
	pair<bool, sint64> resultBehaviour = stage.property(PROPERTY_BEHAVIOUR);
	if(resultBehaviour.first)
	{
		CBehaviourContext bc;
		bc.Behav     = CBehaviour(resultBehaviour.second);
		bc.BehavTime = stage.time();
		// See if there's a list of target associated with that behaviour (for multitarget spells)
		uint64 spellTarget[4];
		uint numTargets = 0;
		for(uint k = 0; k < 4; ++k)
		{
			pair<bool, sint64> stProp = stage.property(PROPERTY_TARGET_LIST_0 + k);
			if (!stProp.first) break;
			spellTarget[k] = (uint64) stProp.second;
			++ numTargets;
			stage.removeProperty(PROPERTY_TARGET_LIST_0 + k);
		}

		if (numTargets > 0)
		{
			// get the list of targets from the visual properties
			bc.Targets.unpack(spellTarget, numTargets);
		}
		// Compute the beheviour.

		applyBehaviour(bc);
		// Remove the property.
		stage.removeProperty(PROPERTY_BEHAVIOUR);
	}

	// Apply the target.
	pair<bool, sint64> resultTarget = stage.property(PROPERTY_TARGET_ID);
	if(resultTarget.first)
	{
		// Change the entity target.
		targetSlot((CLFECOMMON::TCLEntityId)resultTarget.second);
		// Remove the property.
		stage.removeProperty(PROPERTY_TARGET_ID);
	}

	// The Mount
	pair<bool, sint64> resultParent = stage.property(CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
	if(resultParent.first)
	{
		_Mount = (CLFECOMMON::TCLEntityId)resultParent.second;

		// Remove the property.
		stage.removeProperty(CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
	}

	// The Rider
	pair<bool, sint64> resultRider = stage.property(CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);
	if(resultRider.first)
	{
		_Rider = (CLFECOMMON::TCLEntityId)resultRider.second;

		// Remove the property.
		stage.removeProperty(CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);
	}

	// visual fxs : links and auras
	pair<bool, sint64> resultVisualFX = stage.property(CLFECOMMON::PROPERTY_VISUAL_FX);
	if (resultVisualFX.first)
	{
		applyVisualFX(resultVisualFX.second);
		stage.removeProperty(CLFECOMMON::PROPERTY_VISUAL_FX);
	}


	return stageDone;
}// applyStage //

//-----------------------------------------------
// applyCurrentStage :
// Apply The Current Stage (first stage).
//-----------------------------------------------
bool CCharacterCL::applyCurrentStage()
{
	bool bRet = true;

	CStageSet::TStageSet::iterator it = _Stages._StageSet.begin();
	if(it != _Stages._StageSet.end())
	{
		// Apply the Stage and remove it if stage done.
		if(applyStage((*it).second))
			_Stages._StageSet.erase(it);
		else
			bRet = false;
	}
	else
		nlwarning("CCharacterCL::applyCurrentStage: there is no stage.");

	// Update information from remaining stages.
	updateStages();

	return bRet;
}// applyCurrentStage //


//-----------------------------------------------
// applyAllStagesToFirstPos :
// Apply all stages to the first stage with a position.
//-----------------------------------------------
void CCharacterCL::applyAllStagesToFirstPos()
{
	CVectorD stagePos;
	CStageSet::TStageSet::iterator it = _Stages._StageSet.begin();
	while(it != _Stages._StageSet.end() && !(*it).second.getPos(stagePos))
	{
		// Apply the Stage.
		if(!applyStage((*it).second))
		{
			updateStages();
			return;
		}

		// Backup the iterator to remove
		CStageSet::TStageSet::iterator itTmp = it;
		// Next Stage.
		++it;
		// Remove the stage done.
		_Stages._StageSet.erase(itTmp);
	}

	// Apply the stage with the position.
	if(it != _Stages._StageSet.end())
	{
		// Apply the Stage.
		if(applyStage((*it).second))
			// Remove the stage done.
			_Stages._StageSet.erase(it);
	}
	else
		nlwarning("CH:applyAllStagesToFirstPos:%d: There is no stage with a position.", _Slot);

	// Upate information from remaining stages.
	updateStages();
}// applyAllStagesToFirstPos //


//-----------------------------------------------
// playToEndAnim :
// Play the time step for the loop and truncate to End Anim if Time Step too big.
//-----------------------------------------------
ADD_METHOD(void CCharacterCL::playToEndAnim(const double &startTimeOffset, double &length))
	// Average Speed
	double speedToDest = speed();
	// Blend Walk/Run
	if(ClientCfg.BlendForward && (animState(MOVE) == CAnimationStateSheet::Walk))
	{
		uint animWalkId = animId(MOVE);
		uint animRunId  = animId(MOVE_BLEND_OUT);
		double animWalkSpeed = EAM->getAnimationAverageSpeed(animWalkId)*getSheetScale()*_CustomScalePos*_CharacterScalePos;
		double animRunSpeed  = EAM->getAnimationAverageSpeed(animRunId)*getSheetScale()*_CustomScalePos*_CharacterScalePos;
		if(animWalkSpeed<=animRunSpeed)
		{
			double startTimeOffRun = animOffset(MOVE_BLEND_OUT);
			double animWalkLength = EAM->getAnimationLength(animWalkId);
			double animRunLength  = EAM->getAnimationLength(animRunId);
			// Current Speed <= Walk Speed, so use the walk animation only.
			if(speed() <= animWalkSpeed)
			{
				runFactor(0.0);
				double speedFactor = speed()/animWalkSpeed;
				double animTimeOffWalk = animOffset(MOVE) + length*speedFactor;
				if(animTimeOffWalk > animWalkLength)
				{
					animOffset(MOVE,           animWalkLength);
					animOffset(MOVE_BLEND_OUT, animRunLength);
					length = (animWalkLength - startTimeOffset) / speedFactor;
				}
				// Adjust Time Offset for the Run Channel
				else
				{
					animOffset(MOVE,           animTimeOffWalk);
					animOffset(MOVE_BLEND_OUT, animRunLength*(animTimeOffWalk/animWalkLength));
				}
			}
			// Current Speed >= Run Speed, so use the run animation only.
			else if(speed() >= animRunSpeed)
			{
				runFactor(1.0);
				double speedFactor = speed()/animRunSpeed;
				double animTimeOffRun = animOffset(MOVE_BLEND_OUT) + length*speedFactor;
				if(animTimeOffRun > animRunLength)
				{
					animOffset(MOVE,           animWalkLength);
					animOffset(MOVE_BLEND_OUT, animRunLength);
					length = (animRunLength - startTimeOffRun) / speedFactor;
				}
				// Adjust Time Offset for the Walk Channel
				else
				{
					animOffset(MOVE,           animWalkLength*(animTimeOffRun/animRunLength));
					animOffset(MOVE_BLEND_OUT, animTimeOffRun);
				}
			}
			// Current Speed > Walk Speed & < Run Speed, so mix Walk and Run animation.
			else
			{
				double t1 = animRunSpeed-animWalkSpeed;
				double t2 =      speed()-animWalkSpeed;
				runFactor(t2/t1);
				double mixLength = runFactor()*animRunLength + (1.0-runFactor())*animWalkLength;
				double animTimeOffWalk = animOffset(MOVE) + animWalkLength/mixLength*length;
				if(animTimeOffWalk > animWalkLength)
				{
					animOffset(MOVE,           animWalkLength);
					animOffset(MOVE_BLEND_OUT, animRunLength);
					length = (animWalkLength - startTimeOffset) / (animWalkLength/mixLength);
				}
				else
				{
					animOffset(MOVE,           animTimeOffWalk);
					animOffset(MOVE_BLEND_OUT, animRunLength*animTimeOffWalk/animWalkLength);	// Same percentage in the animation than the Walk one.
				}
			}
			return;
		}
		//else
			//nlwarning("playToEndAnim:%d: animWalkSpeed > animRunSpeed", _Slot);
	}
	// No Mix between Walk and Run.
	runFactor(0.0);
	// Speed Factor
	double speedFactor = computeSpeedFactor(speedToDest);
	// Compute the desired new time offset.
	double animTimeOffMove = animOffset(MOVE) + length * speedFactor;
	// Truncate animation time offset if it over-runs end of animation and change the loopTimeOffset too.
	double animationLength = EAM->getAnimationLength(animId(MOVE));
	if(animTimeOffMove > animationLength)
	{
		animOffset(MOVE, animationLength);
		length = (animationLength - startTimeOffset) / speedFactor;
	}
	else
		animOffset(MOVE, animTimeOffMove);
}// playToEndAnim //

//-----------------------------------------------
// updateStages :
// Call this method to give a time for each stage, compute distance to destination and some more information.
// \todo GUIGUI : clean up
//-----------------------------------------------
void CCharacterCL::updateStages()
{
	H_AUTO ( RZ_Client_Entity_CL_updateStages );

	_FirstPos			= INVALID_POS;	// No First Position
	_FirstTime			= INVALID_TIME;	//- No First Position
	dist2FirstPos(INVALID_DIST);		// No First Position
	_DestPos			= INVALID_POS;	// No Destination
	_DestTime			= INVALID_TIME;	// No Destination
	dist2Dest(INVALID_DIST);			// No Destination
	CVectorD posTmp		= pos();
	_IsThereAMode		= false;
	_ImportantStepTime= 0.0;


	// ***** update predicted interval: if a new pos B is found after a pos A, then the interval B-A is known!
	CStageSet::TStageSet::iterator it = _Stages._StageSet.begin();
	CStageSet::TStageSet::iterator itPosPrec= _Stages._StageSet.end();
	bool	somePosFoundEarly= false;
	while(it != _Stages._StageSet.end())
	{
		// if this stage has a position
		if(it->second.isPresent(PROPERTY_POSITION))
		{
			somePosFoundEarly= true;
			// then it's cool we can set the new accurate interval to the prec stage which has a pos
			if(itPosPrec!=_Stages._StageSet.end())
			{
				uint dgc= it->first - itPosPrec->first;
				itPosPrec->second.predictedInterval(dgc);
			}
			// bkup
			itPosPrec= it;
		}
		it++;
	}


	// ***** Compute the current LCT Impact for this character
	// NB: used only in mode CClientConfig::StageUsePosOnlyLCT
	sint32	charLCTI;
	// If disabled, full LCT impact
	if(_StartDecreaseLCTImpact==0)
		charLCTI= 256;
	else
	{
		const	sint32	decreaseTick= 20;	// 2 seconds
		// blend according to the start of decrease
		sint32	dt= NetMngr.getCurrentServerTick() - _StartDecreaseLCTImpact;
		if(dt<=0)
			charLCTI= 256;
		else if(dt>=decreaseTick)
			charLCTI= 0;
		else
			charLCTI= ((decreaseTick-dt)*256)/decreaseTick;
		// hence, at end of blend, charLCTI is 0
	}


	// ***** Compute Stages to give them a time and get some information from those stages.
	// yoyo: use any stage with no LCT, until it is to be played AFTER a position
	bool	stageForceLCTFound= false;
	CStageSet::TStageSet::iterator itTmp;
	it = _Stages._StageSet.begin();
	while(it != _Stages._StageSet.end())
	{
		CStage &stage = (*it).second;

		// *** retrieve position in stage if any
		CVectorD	posInStage;
		bool		hasPos= stage.getPos(posInStage);
		// check the first pos is correct
		if(hasPos && dist2Dest()==INVALID_DIST)
		{
			// Compute the distance to the first position
			double distToFirst   = (CVectorD(posInStage.x, posInStage.y, 0.0) - CVectorD(          posTmp.x,           posTmp.y, 0.0)).norm();
			double distToLimiter = (CVectorD(posInStage.x, posInStage.y, 0.0) - CVectorD(_PositionLimiter.x, _PositionLimiter.y, 0.0)).norm();
			// Check if the first pos is Not the same as the current entity pos
			if((distToFirst   < ClientCfg.DestThreshold)
			|| (distToLimiter <=  ClientCfg.PositionLimiterRadius))
			{
				// The FIRST POSITION is the SAME as the CURRENT entity POSITION -> REMOVE POSITION in the stage
				stage.removeProperty(CLFECOMMON::PROPERTY_POSX);
				stage.removeProperty(CLFECOMMON::PROPERTY_POSY);
				stage.removeProperty(CLFECOMMON::PROPERTY_POSZ);
				hasPos= false;
				//
				if(VerboseAnimSelection && _Slot == UserEntity->selection())
					nlinfo("CH:updateStages:%d: Bad First, distToFirst(%f), ClientCfg.DestThreshold(%f), distToLimiter(%f), ClientCfg.PositionLimiterRadius(%f)", _Slot,
					distToFirst, ClientCfg.DestThreshold, distToLimiter, ClientCfg.PositionLimiterRadius);
			}
		}

		// stage has pos? => force LCT for it and after
		stageForceLCTFound= stageForceLCTFound || hasPos;


		// *** Compute the estimated Time for the Stage.
		// Compute difference in Game Cycle Between the current Game Cycle and the current Stage.
		sint32 t;
		if(ClientCfg.StageLCTUsage==CClientConfig::StageUseAllLCT)
			t= (*it).first - NetMngr.getCurrentClientTick();
		else if(ClientCfg.StageLCTUsage==CClientConfig::StageUseNoLCT)
			t= (*it).first - NetMngr.getCurrentServerTick();
		else
		{
			// Update LCTImpact for the stage
			if(stageForceLCTFound)
			{
				// Force full impact for stage after or including a POS
				stage.setLCTImpact(256);
			}
			else
			{
				/* minimize the LCT impact with the current char one
					Hence, if the stage had a lct impact lowered, but LCT decrease was canceled,
					the stage will keep its LCT impact
				*/
				stage.setLCTImpact(min(stage.getLCTImpact(), charLCTI));
			}

			sint32	lcti= stage.getLCTImpact();

			// if full impact
			if(lcti>=256)
				t= (*it).first - NetMngr.getCurrentClientTick();
			// if no impact
			else if(lcti<=0)
				t= (*it).first - NetMngr.getCurrentServerTick();
			// else blend
			else
			{
				sint32	twlct= (*it).first - NetMngr.getCurrentClientTick();
				sint32	twolct= (*it).first - NetMngr.getCurrentServerTick();
				t= (twlct*lcti + twolct*(256-lcti))>>8;
			}
		}
		// Compute the estimated Time for the Stage.
		stage.time( (double)(NetMngr.getMachineTimeAtTick() + t*NetMngr.getMsPerTick())/1000.0 );


		// *** Important step is used for "Panic mode" animation acceleration. skip the first stage
		if(_ImportantStepTime==0.0 && it!=_Stages._StageSet.begin())
		{
			// Important steps are ones that takes times (pos, orientation, mode, animation etc....)
			if(	stage.isPresent(PROPERTY_POSITION) ||
				stage.isPresent(PROPERTY_MODE) ||
				stage.isPresent(PROPERTY_ORIENTATION) ||
				stage.isPresent(PROPERTY_ENTITY_MOUNTED_ID) ||
				stage.isPresent(PROPERTY_RIDER_ENTITY_ID) ||
				stage.isPresent(PROPERTY_BEHAVIOUR))
				_ImportantStepTime= stage.time();
		}


		// *** Compute dist2dest (until a mode) if has pos
		if((_IsThereAMode==false) && hasPos)
		{
			// Set the destination pos and time.
			_DestPos= posInStage;
			_DestTime = stage.time() + (double)(stage.predictedInterval()*NetMngr.getMsPerTick())/1000.0;
			// Update First Pos.
			if(dist2Dest() == INVALID_DIST)
			{
				_FirstPos = _DestPos;
				_FirstTime = stage.time();
				// Compute the distance to the first position
				double distToFirst   = (CVectorD(_DestPos.x, _DestPos.y, 0.0) - CVectorD(posTmp.x,posTmp.y, 0.0)).norm();
				// Set the Distance to the Destination as the distance to the first position.
				dist2Dest(distToFirst);
				// Set the distance to First Stage.
				dist2FirstPos(distToFirst);
			}
			// Increase distance to destination.
			else
				dist2Dest(dist2Dest() + (CVectorD(_DestPos.x, _DestPos.y, 0.0) - CVectorD(posTmp.x, posTmp.y, 0.0)).norm());
			// Backup the last pos.
			posTmp = _DestPos;
		}
		// Stop if there is a mode in the stage.
		if(stage.isPresent(CLFECOMMON::PROPERTY_MODE))
			_IsThereAMode = true;


		// *** NEXT STAGE
		itTmp = it;
		++it;
		// REMOVE EMPTY STAGE (because only position and the same as the current one).
		if(stage.empty())
			_Stages._StageSet.erase(itTmp);
	}

	// If there is no mode in queue, mode wanted is the current mode
	// It must usually be the theorical one except for some mode used only by the client like SWIM
	if(!_IsThereAMode)
		_ModeWanted = _Mode;

	// ***** update _StartDecreaseLCTImpact
	// If a stage that force LCT has been found in the list
	if(stageForceLCTFound)
		// Decrease of LCT is disabled
		_StartDecreaseLCTImpact= 0;
	else if(_StartDecreaseLCTImpact==0)
		// Start to decrease LCT
		_StartDecreaseLCTImpact= NetMngr.getCurrentServerTick();


	// ***** compute _RunStartTimeNoPop (see _RunStartTimeNoPop)
	_RunStartTimeNoPop= INVALID_TIME;
	// only if have some pos in the queue
	if(somePosFoundEarly)
	{
		double	d2fp = 0.0;
		double	fpTime= INVALID_TIME;
		// if the first pos is computed, use it
		if(_FirstTime!=INVALID_TIME)
		{
			d2fp= dist2FirstPos();
			fpTime= _FirstTime;
		}
		// else try to compute the first pos, WIHTOUT regarding if there is a mode or not
		// (because even mode anim can be accelerated....)
		else
		{
			it = _Stages._StageSet.begin();
			while(it != _Stages._StageSet.end())
			{
				CStage &stage = (*it).second;
				CVectorD	firstPos;
				if(stage.getPos(firstPos))
				{
					fpTime = stage.time() + (double)(stage.predictedInterval()*NetMngr.getMsPerTick())/1000.0;
					d2fp= CVectorD(firstPos.x-pos().x, firstPos.y-pos().y, 0.0).norm();
					break;
				}
				it++;
			}
		}

		// with d2fp, fpTime, and maxSpeed, we can estimate the moment where the run should start
		if(fpTime!=INVALID_TIME)
		{
			float	maxSpeed= (float)getMaxSpeed();
			if(maxSpeed>0)
			{
				// compute at which time the first move should begin so it doesn't have to accelerate
				_RunStartTimeNoPop= fpTime - d2fp/maxSpeed;
			}
		}
	}

}// updateStages //

//-----------------------------------------------
// beginImpact :
// Return if the impact must be played.
// \param anim : pointer on the current animation (MUST NOT BE NULL).
// \param currentTime : current time in the animation.
// \param triggerName : name of the trigger to check.
// \param isActive : read (and can change) the state.
// \param timeFactor : when to activate the impact if there is no track (value has to be between 0 and 1 to be valid).
// \return bool : true if the trigger is valid.
// \warning This method does not check if the animation is Null.
//-----------------------------------------------
ADD_METHOD(bool CCharacterCL::beginImpact(NL3D::UAnimation *anim, NL3D::TAnimationTime currentTime, const std::string &triggerName, bool &isActive, float timeFactor))
	// Is the impact already activeated.
	if(isActive)
		return false;

	// Try to find the impact trigger in the animation.
	UTrack *Track = anim->getTrackByName(triggerName.c_str());
	// No track -> just check with 2/3 animation
	if(Track)
	{
		if(Track->interpolate(currentTime, isActive))
			return isActive;
		else
			nlwarning("CH:beginImpact:%d: Wrong type asked.", _Slot);
	}

	// No Track or pb with it so try with the animation length.
	float length = (float)(anim->getEndTime()-anim->getBeginTime());
	isActive = (animOffset(MOVE) >= length*timeFactor);
	return isActive;
}// beginImpact //

//-----------------------------------------------
// animEventsProcessing :
// Manage Events that could be created by the animation (like sound).
// \param startTime : time to start processing events from the current animation.
// \param stopTime : time to stop processing events from the current animation.
// \todo GUIGUI : Optimize FXs launch when we would have time
//-----------------------------------------------
void CCharacterCL::animEventsProcessing(double startTime, double stopTime)
{
	if (_CurrentState == 0)
		return;

	// \todo Vianney : temp le temps de savoir comment on joue les son pour le propre joueur
	// No sound for the player.
	if(_Slot != 0 || _Mode != MBEHAV::NORMAL)
	{
		// Retreive the surface material
		uint32 matId= getGroundType();
		// Set the material id var
		_SoundContext.Args[0] = matId;
		if(_Sheet)
		{
			// Set the sound family var
			_SoundContext.Args[2] = _Sheet->SoundFamily;
			// Set the sound variation var
			_SoundContext.Args[3] = _Sheet->SoundVariation;
		}
		else
		{
			// Set the sound family var
			_SoundContext.Args[2] = 0;
			// Set the sound variation var
			_SoundContext.Args[3] = 0;
		}
		// Sound Process.
		CSoundAnimManager* sndMngr = CSoundAnimManager::instance();
		if(sndMngr && (_SoundId[MOVE] != CSoundAnimationNoId))
		{
			_SoundContext.Position = pos();
			// Look for the cluster(s) containing this character...
			std::vector<NL3D::CCluster*>	clusters;
			if (!_Instance.empty())
			{
				// single meshed
				_Instance.getLastParentClusters(clusters);
			}
			else if (!_Skeleton.empty())
			{
				// Skel meshed
				_Skeleton.getLastParentClusters(clusters);
			}
			CCluster *pcluster = 0;
			// use the first cluster if at leat one available
			if (!clusters.empty())
				pcluster = clusters.front();
			sndMngr->playAnimation(_SoundId[MOVE], (float) startTime, (float) stopTime, pcluster, _SoundContext);
		}
	}

}// animEventsProcessing //

//-----------------------------------------------
// updatePreCollision :
// Method called each frame to manage the entity.
// \param time : current time of the frame.
// \parem target : pointer on the current entity target.
//-----------------------------------------------
void CCharacterCL::updatePreCollision(const TTime &currentTimeInMs, CEntityCL *target)	// virtual
{
	H_AUTO ( RZ_Client_Entity_CL_Update_Pre_Collision );
	// Set the Last frame PACS Pos.
	if(_Primitive)
		_Primitive->getGlobalPosition(_LastFramePACSPos, dynamicWI);
	// Set the previous position before changing the current one.
	_LastFramePos = _Position;

	// Turn towards the target when in COMBAT_FLOAT mode.
	if(_Mode == MBEHAV::COMBAT_FLOAT)
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Combat )

		// Check there is a valid target and it's not the entity itself.
		if(targetSlot() != CLFECOMMON::INVALID_SLOT
		&& targetSlot() != slot()
		&& target)
			// Set the new entity direction
			front(target->pos() - pos(), true, false);
	}

	// Update Position if not a child & displayable.
	if(parent() == CLFECOMMON::INVALID_SLOT && _Displayable)
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Pos )
		updatePos(currentTimeInMs, target);
	}
}// updatePreCollision //

//-----------------------------------------------
// updateFX :
// Apply track on fxs
// Check if some FX should be removed.
//-----------------------------------------------
inline void CCharacterCL::updateFX()
{
	updateAttachedFX();
}// updateFX //


//-----------------------------------------------
// updateAttachedFX :
// Apply track on animated fxs
// Remove those that should be removed
//-----------------------------------------------
void CCharacterCL::updateAttachedFX()
{
	// build align matrix
	CMatrix alignMatrix;
	buildAlignMatrix(alignMatrix);

	std::list<CAttachedFX::CBuildInfo>::iterator itAttachedFxToStart = _AttachedFXListToStart.begin();
	while(itAttachedFxToStart != _AttachedFXListToStart.end())
	{
		if ((*itAttachedFxToStart).DelayBeforeStart < (float)(TimeInSec - (*itAttachedFxToStart).StartTime))
		{
			uint index = (*itAttachedFxToStart).MaxNumAnimCount;
			(*itAttachedFxToStart).MaxNumAnimCount = 0;
			CAttachedFX::TSmartPtr fx = new CAttachedFX;
			fx->create(*this, (*itAttachedFxToStart), CAttachedFX::CTargeterInfo());
			if (!fx->FX.empty())
			{
				_AuraFX[index] = fx;
			}
			itAttachedFxToStart = _AttachedFXListToStart.erase(itAttachedFxToStart);
		}
		else
		{
			++itAttachedFxToStart;
		}
	}

	// update tracks & pos for anim attachedfxs
	std::list<CAttachedFX::TSmartPtr>::iterator itAttachedFx = _AttachedFXListForCurrentAnim.begin();
	while(itAttachedFx != _AttachedFXListForCurrentAnim.end())
	{
		nlassert(*itAttachedFx);
		CAttachedFX &attachedFX = **itAttachedFx;
		attachedFX.update(*this, alignMatrix);
		if (!attachedFX.FX.empty()) attachedFX.FX.setUserMatrix(alignMatrix);
		++itAttachedFx;
	}

	// Try to remove animation FXs still not removed.
	itAttachedFx = _AttachedFXListToRemove.begin();
	while(itAttachedFx != _AttachedFXListToRemove.end())
	{
		// If the FX is not present or valid -> remove the FX.
		bool mustDelete = false;
		CAttachedFX &attachedFX = **itAttachedFx;
		if (attachedFX.SpawnTime != TimeInSec)
		{
			if(attachedFX.FX.empty() || !attachedFX.FX.isSystemPresent() || !attachedFX.FX.isValid())
			{
				mustDelete = true;
			}
		}
		if (attachedFX.TimeOutDate != 0)
		{
			if (TimeInSec >= attachedFX.TimeOutDate)
			{
				mustDelete = true;
			}
		}
		if (mustDelete)
		{
			// Remove from the list.
			itAttachedFx = _AttachedFXListToRemove.erase(itAttachedFx);
		}
		else
		{
			attachedFX.update(*this, alignMatrix);
			if (!attachedFX.FX.empty()) attachedFX.FX.setUserMatrix(alignMatrix);
			++itAttachedFx;
		}
	}

	// update the aura fx
	for(uint k = 0; k < MaxNumAura; ++k)
	{
		if (_AuraFX[k])
		{
			if (_AuraFX[k]->TimeOutDate != 0.f) // we use that flag to mark the aura as 'shutting down'
			{
				if (TimeInSec >= _AuraFX[k]->TimeOutDate)
				{
					_AuraFX[k] = NULL;
				}
				else
				{
					float lifeRatio = (float) ((_AuraFX[k]->TimeOutDate - TimeInSec) / AURA_SHUTDOWN_TIME);
					if (!_AuraFX[k]->FX.empty()) _AuraFX[k]->FX.setUserParam(0, 1.f - lifeRatio);
				}
			}
			if (_AuraFX[k]) // not deleted yet ?
			{
				// update position & orientation
				_AuraFX[k]->update(*this, alignMatrix);
			}
		}
	}

	// update the link fx
	if (_LinkFX)
	{
		_LinkFX->update(*this, alignMatrix);
	}
	if (_StaticFX)
	{
		_StaticFX->FX->update(*this, alignMatrix);
	}
}




//-----------------------------------------------
// updateVisible :
//-----------------------------------------------
void CCharacterCL::updateVisible (const TTime &currentTimeInMs, CEntityCL *target)
{
	// Changes the skeleton state
	if(!_Skeleton.empty())
	{
		_Skeleton.show();
	}
	// Changes the instance position.
	else if(!_Instance.empty())
	{
		_Instance.show();
	}

	// Snap the entity to the ground.
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Snap_To_Ground )
		snapToGround();
	}

	// Apply the new entity position to the visual of the entity (apply x and z movement due to animation).
	if(parent() == CLFECOMMON::INVALID_SLOT)
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Display )
		updateDisplay();
	}

	// Change the cluster of the entity.
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Cluster )
		updateCluster();
	}

	// Update the LodCharacter Animation.
	if(_LodCharacterAnimEnabled)
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Pos_Lod_Animation )

		// set this value to the skeleton
		if(skeleton())
			skeleton()->setLodCharacterAnimTime(_LodCharacterAnimTimeOffset);
	}

	// Update FX
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_FX )
		updateFX();
	}

	// Update Modifiers
	if(!_HPModifiers.empty())
	{
		HPMD mod;
		mod.CHPModifier::operator= (*_HPModifiers.begin());
		mod.Time = TimeInSec + mod.DeltaT;
		_HPDisplayed.push_back(mod);
		_HPModifiers.erase(_HPModifiers.begin());
	}

	// Parent
	CEntityCL::updateVisible(currentTimeInMs, target);
}// updateVisible //

//-----------------------------------------------
// updateSomeClipped :
//-----------------------------------------------
void CCharacterCL::updateSomeClipped (const TTime &currentTimeInMs, CEntityCL *target)
{
	// Snap the entity to the ground.
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Snap_To_Ground )
		snapToGround();
	}

	// Changes the skeleton position.
	if(!_Skeleton.empty())
	{
		_Skeleton.setPos(pos());
		_Skeleton.hide();
	}
	// Changes the instance position.
	else if(!_Instance.empty())
	{
		_Instance.setPos(pos());
		_Instance.hide();
	}

	if(!ClientCfg.Light)
	{
		// Update texture Async Loading
		updateAsyncTexture();
		// Update lod Texture
		updateLodTexture();

		// Update FX
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_FX )
			updateFX();
		}
	}

	// Remove Modifiers.
	_HPModifiers.clear();
	_HPDisplayed.clear();

	// Parent
	CEntityCL::updateSomeClipped(currentTimeInMs, target);
}// updateSomeClipped //

//-----------------------------------------------
// updateClipped :
//-----------------------------------------------
void CCharacterCL::updateClipped (const TTime &currentTimeInMs, CEntityCL *target)
{
	// hide the scene interface
	if (_InSceneUserInterface)
	{
		if (_InSceneUserInterface->getActive())
			_InSceneUserInterface->setActive (false);
	}
	if (_CurrentBubble)
	{
		if (_CurrentBubble->getActive())
			_CurrentBubble->setActive (false);
	}

	// parent
	CEntityCL::updateClipped(currentTimeInMs, target);
}// updateClipped //

//-----------------------------------------------
// updateVisiblePostPos :
// Update the entity after all positions done.
//-----------------------------------------------
void CCharacterCL::updateVisiblePostPos(const NLMISC::TTime &currentTimeInMs, CEntityCL *target)	// virtual
{
	// Stuff to do only when alive.
	if(!isDead())
	{
		// Update the head direction.
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Head_Direction )
				updateHeadDirection(target);
		}
		// Update Blink.
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Blink )
				updateBlink(currentTimeInMs);
		}
	}

	// Update in scene interface
	if(_InSceneUserInterface || _CurrentBubble)
	{
		// Draw the entity Name if asked or under the cursor.
		bool showIS = mustShowInsceneInterface( (!_Sheet) || (_Sheet->DisplayOSD) );
		bool showBubble = true;

		// Don't show bubble if lod
		if (!_Skeleton.empty() && _Skeleton.isDisplayedAsLodCharacter())
		{
			showBubble = false;
		}

		// If the name of the character is unknown, no user info
		if (_EntityName.empty() && _Title.empty())
			showIS = false;

		// if mounted : don't display name
		if( _Rider != CLFECOMMON::INVALID_SLOT)
		{
			showIS = false;
		}


		// User Info
		if (_InSceneUserInterface)
		{
			// Activate
			_InSceneUserInterface->setActive (showIS);

			if (showIS)
			{
				// Update dynamic data
				_InSceneUserInterface->updateDynamicData ();

				NLMISC::CVectorD pos;
				if (getNamePos(pos))
				{
					// Check the pos validity
					if((isValidDouble(pos.x) && isValidDouble(pos.y) && isValidDouble(pos.z)) == false)
					{
						nlwarning("CH:updateVisiblePostPos:%d: invalid pos %f %f %f", _Slot, pos.x, pos.y, pos.z);
						nlstop;
					}
					_InSceneUserInterface->Position = pos;
				}
				else
				{
					pos = (box().getMin() + box().getMax())/2;
					pos.z = box().getMax().z;
					nlassert(isValidDouble(pos.x) && isValidDouble(pos.y) && isValidDouble(pos.z));
					_InSceneUserInterface->Position = pos;
				}
			}
		}

		// Bubble ?
		if (_CurrentBubble)
		{
			showBubble &= _CurrentBubble->canBeShown();

			// Activate
			if (_CurrentBubble->getActive() != showBubble)
				_CurrentBubble->setActive (showBubble);

			if (showBubble)
			{
				// Offset X
				sint offsetX = 0;
				if (_InSceneUserInterface)
					offsetX = - 10 -  (_InSceneUserInterface->getWReal() / 2);
				_CurrentBubble->setOffsetX (offsetX);

				NLMISC::CVectorD pos;
				if (!getNamePos(pos))
				{
					pos = (box().getMin() + box().getMax())/2;
					pos.z = box().getMax().z;
				}
				nlassert(isValidDouble(pos.x) && isValidDouble(pos.y) && isValidDouble(pos.z));
				_CurrentBubble->Position = pos;
			}
		}
	}

	// parent
	CEntityCL::updateVisiblePostPos(currentTimeInMs, target);
}// updateVisiblePostPos //


//-----------------------------------------------
// updatePostCollision :
// Method called each frame to manage the entity.
// \param time : current time of the frame.
// \parem target : pointer on the current entity target.
//-----------------------------------------------
void CCharacterCL::updatePostCollision(const TTime &/* currentTimeInMs */, CEntityCL * /* target */)	// virtual
{
	H_AUTO ( RZ_Client_Entity_CL_Update_Post_Collision )

	// Finalize PACS position
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Finalize_Move )
		pacsFinalizeMove();
		// \todo GUIGUI : fait rapidement pour voir les autres se baigner pour la video, faire mieux.

		// changed : Malkav , also do this for mektoub (as they can swim)
		if(PACS && _Primitive
			&& (isPlayer() || isNPC()
				|| (_Sheet && (_Sheet->Race == EGSPD::CPeople::MektoubMount || _Sheet->Race == EGSPD::CPeople::MektoubPacker))
				)
			)
		{
			// Is in water ?
			if(GR)
			{
				UGlobalPosition gPos;
				_Primitive->getGlobalPosition(gPos, dynamicWI);
				float waterHeight;
				if(GR->isWaterPosition(gPos, waterHeight))
				{
					if(isSwimming()==false)
					{
						if(isDead())
						{
							_Mode = MBEHAV::SWIM_DEATH;
						}
						else if (isRiding())
						{
							_Mode = MBEHAV::MOUNT_SWIM;

							// also change mounted entity mode
							if (_Mount != CLFECOMMON::INVALID_SLOT)
							{
								CCharacterCL *mount = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(_Mount));
								if(mount)
								{
									// Set the mount.
									mount->setMode(MBEHAV::MOUNT_SWIM);
									mount->computeAutomaton();
									mount->computeAnimSet();
									mount->setAnim(CAnimationStateSheet::Idle);
								}
							}
						}
						else
						{
							_Mode = MBEHAV::SWIM;
						}

						// Compute the current automaton
						computeAutomaton();
						// Update the animation set according to the mode.
						computeAnimSet();
						// Animset changed -> update current animation
						setAnim(CAnimationStateSheet::Idle);
					}
				}
				else
				{
					if(isSwimming())
					{
						if(isDead())
						{
							_Mode = MBEHAV::DEATH;
						}
						else if (isRiding())
						{
							_Mode = MBEHAV::MOUNT_NORMAL;
							// also change mounted entity mode
							if (_Mount != CLFECOMMON::INVALID_SLOT)
							{
								CCharacterCL *mount = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(_Mount));
								if(mount)
								{
									// Set the mount.
									mount->setMode(MBEHAV::MOUNT_NORMAL);
									mount->computeAutomaton();
									mount->computeAnimSet();
									mount->setAnim(CAnimationStateSheet::Idle);
								}
							}
						}
						else
						{
							_Mode = MBEHAV::NORMAL;
						}

						// Compute the current automaton
						computeAutomaton();
						// Update the animation set according to the mode.
						computeAnimSet();
						// Animset changed -> update current animation
						setAnim(CAnimationStateSheet::Idle);
					}
				}
			}
		}
	}
}// updatePostCollision //


//-----------------------------------------------
// getTheMove :
//-----------------------------------------------
double CCharacterCL::getTheMove(double loopTimeStep, double oldMovingTimeOffset, double oldMovingTimeOffsetRun) const
{
	double move;
	if(_CurrentState)
	{
		// A Real Move State
		if(_CurrentState->Move)
		{
			// Get the covered distance from the animation.
			move = getTheMove(loopTimeStep, oldMovingTimeOffset, MOVE);
			if(runFactor() > 0.0)
			{
				// Blend the 2 move (Walk & Run).
				move = move*(1.0-runFactor()) + getTheMove(loopTimeStep, oldMovingTimeOffsetRun, MOVE_BLEND_OUT)*runFactor();
				// The move must be significant.
				if((move>0.0) && (move<ClientCfg.SignificantDist))
					move = ClientCfg.SignificantDist;
			}
		}
		// Slide
		else if(_CurrentState->Slide)
		{
			move = speed() * loopTimeStep;
			// The move must be significant.
			if((move>0.0) && (move<ClientCfg.SignificantDist))
				move = ClientCfg.SignificantDist;
		}
		else
			move = 0.0;
	}
	else
	{
		move = speed() * loopTimeStep;
		// The move must be significant.
		if((move>0.0) && (move<ClientCfg.SignificantDist))
			move = ClientCfg.SignificantDist;
	}
	// Check the move is significant.
	CHECK(!((move>0.0) && (move<ClientCfg.SignificantDist)));
	// Return the move done by the entity since last time.
	return move;
}// getTheMove //
//-----------------------------------------------
// getTheMove :
//-----------------------------------------------
double CCharacterCL::getTheMove(double loopTimeStep, double oldMovingTimeOffset, TAnimationType channel) const
{
	double move;
	// Compute a linear motion when the animation is missing.
	if(animIndex(channel) == CAnimation::UnknownAnim)
	{
		double offsetT = _DestTime - _LastFrameTime;
		if(offsetT <= 0.0)
		{
			// \todo GUIGUI : in this case, 'loopTimeStep' should not decrease so FIX IT.
			move = dist2Dest();
		}
		else
		{
			move = dist2Dest() * (loopTimeStep / offsetT);
			// The move must be significant.
			if((move>0.0) && (move<ClientCfg.SignificantDist))
				move = ClientCfg.SignificantDist;
		}
	}
	// Get the motion done by the animation.
	else
		move = computeMotion(oldMovingTimeOffset, channel);

	CHECK(!(move>0.0 && move<ClientCfg.SignificantDist));
	return move;
}// getTheMove //

//-----------------------------------------------
// updatePosCombatFloat :
//-----------------------------------------------
void CCharacterCL::updatePosCombatFloat(double /* frameTimeRemaining */, CEntityCL *target)	// virtual
{
	H_AUTO_USE ( RZ_Client_Character_CL_Update_Pos_Combat_Float )

	// The target is valid
	if(target)
	{
		// Get the position where the attacker should go to attack his target according to the attack angle.
		CVectorD dirToTarget = target->pos() - pos();
		dirToTarget.z = 0.0;
		if(ClientCfg.Local
		|| ((dirToTarget != CVectorD::Null)
		&& fabs(target->pos().x-target->lastFramePos().x)>0.01
		&& fabs(target->pos().y-target->lastFramePos().y)>0.01))
		{
			double angToTarget = atan2(dirToTarget.y, dirToTarget.x);
			_DestPos = target->getAttackerPos(angToTarget, attackRadius() + ClientCfg.AttackDist);
		}
		else
			_DestPos = target->getAttackerPos(_TargetAngle, attackRadius() + ClientCfg.AttackDist);
		// Compute the distance to destination.
		CVectorD vectToDest = _DestPos - pos();
		vectToDest.z = 0.0;
		// Distance to destination is big enough.
		if(vectToDest.norm() > ClientCfg.DestThreshold)
		{
			dist2Dest(vectToDest.norm());
			// Compute the time to reach the destination at the max speed.
			double lengthOfTimeToDest = 0.0; // 0 = No Speed Limit
			_FirstPos	= _DestPos;
			_DestTime	= _LastFrameTime + lengthOfTimeToDest + ClientCfg.ChaseReactionTime;
			_FirstTime	= _DestTime;
/*
			// The time remaining will be enough to reach the destination
			if(frameTimeRemaining >= lengthOfTimeToDest)
			{
				_FirstPos	= _DestPos;
				_DestTime	= _LastFrameTime + lengthOfTimeToDest + ClientCfg.ChaseReactionTime;
				_FirstTime	= _DestTime;
			}
			// The time remaining is not enough to reach the destination at max speed -> compute a first pos possible to reach.
			else
			{
				_FirstPos	= pos() + vectToDest*frameTimeRemaining/lengthOfTimeToDest;
				_DestTime	= _LastFrameTime + lengthOfTimeToDest + ClientCfg.ChaseReactionTime;
				_FirstTime	= _LastFrameTime + frameTimeRemaining + ClientCfg.ChaseReactionTime;
			}
*/
			// Compute the distance to the first position.
			CVectorD tmp2computeDist2FirstPos = _FirstPos-pos();
			tmp2computeDist2FirstPos.z = 0.0;
			dist2FirstPos(tmp2computeDist2FirstPos.norm());

			updatePosCombatFloatChanged(target);
		}
		// Destination is too close (will consider to be at destination.
		else
		{
			_FirstPos		= _DestPos		= pos();
			dist2Dest(0.0);
			dist2FirstPos(0.0);
			_FirstTime		= _DestTime		= _LastFrameTime;
		}
	}
	// The target is not allocated.
	else
	{
		_FirstPos		= _DestPos		= pos();
		dist2Dest(0.0);
		dist2FirstPos(0.0);
		_FirstTime		= _DestTime		= _LastFrameTime;
	}
}// updatePosCombatFloat //

//-----------------------------------------------
// updatePos :
// Upadte the player position
// \param time : Time for the position of the entity after the motion.
// \param target : pointer on the current target.
// \todo GUIGUI : compute it when receiving a new stage instead of every frame (should be faster).
// \todo GUIGUI: recompute distance to destination even if the Stage not reached.
//-----------------------------------------------
ADD_METHOD(void CCharacterCL::updatePos(const TTime &currentTimeInMs, CEntityCL *target))
	_OldAutomaton = _CurrentAutomaton;
	// Compute the Time Step.
	double frameTimeRemaining = computeTimeStep(((double)currentTimeInMs)*0.001);
	// Update the LodCharacter Animation.
	if(_LodCharacterAnimEnabled)
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Pos_Lod_Animation );
		// \todo GUIGUI : replace 'getSpeedFactor' by the correct speed factor !!
		// update lod anim time. multiply by speed factor of the most important slot.
		_LodCharacterAnimTimeOffset += DT * _PlayList->getSpeedFactor(_LodCharacterMasterAnimSlot);
	}
	// BLEND
	if(_PlayList)
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Pos_Set_Play_List );
		// \todo GUIGUI : do something better for the blend (mounts there).
		if(isRiding() || ClientCfg.BlendFrameNumber == 0 || _BlendRemaining <= 0)
		{
			_BlendRemaining = 0;
			_PlayList->setAnimation(ACTION, UPlayList::empty);
			_PlayList->setWeight(ACTION, 0.0f);
			if(runFactor() < 0.5 || (_CurrentAnimSet[MOVE_BLEND_OUT]==0))
			{
				if(_CurrentAnimSet[MOVE])
				{
					_CurrentAnimSet[ACTION] = _CurrentAnimSet      [MOVE];
					animState (ACTION, animState (MOVE));
					animIndex (ACTION, animIndex (MOVE));			// This also call "animId" and set it.
					animOffset(ACTION, animOffset(MOVE));
				}
				else
				{
					_CurrentAnimSet[ACTION] = 0;
					animState (ACTION, CAnimationStateSheet::UnknownState);
					animIndex (ACTION, CAnimation::UnknownAnim);	// This also call "animId" and set it.
					animOffset(ACTION, 0.0);
				}
			}
			else
			{
				_CurrentAnimSet[ACTION] = _CurrentAnimSet      [MOVE_BLEND_OUT];
				animState (ACTION, animState (MOVE_BLEND_OUT));
				animIndex (ACTION, animIndex (MOVE_BLEND_OUT));	// This also call "animId" and set it.
				animOffset(ACTION, animOffset(MOVE_BLEND_OUT));
			}
			_AnimReversed[ACTION] = false;
		}
		else
		{
			double animLength = EAM->getAnimationLength(animId(ACTION));
			// Check Anim length
			if(animOffset(ACTION)+frameTimeRemaining > animLength)
				animOffset(ACTION, animLength);
			else
				animOffset(ACTION, animOffset(ACTION)+frameTimeRemaining);
			// Compute weight step.
			float w = (float)_BlendRemaining/(float)(ClientCfg.BlendFrameNumber+1);
			// Set Old Anim Weight.
			_PlayList->setWeight(ACTION, w);
			// Set New Anim Weight.
			_PlayList->setWeight(MOVE, 1.f-w);
		}
	}
	uint antiFreezeCounter = 0;
	// While the time Step is not Null.
	while(frameTimeRemaining > 0)
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Pos_WhileStep );

		//--------------------//
		//--------------------//
		// ANTI-FREEZE SYSTEM //
		// If too many loop, display some infos
		if(antiFreezeCounter > 50)
		{
/*
			nlwarning("CH:updatePos:antiFreeze:%d: frameTimeRemaining '%f'",  _Slot, frameTimeRemaining);
			nlwarning("CH:updatePos:antiFreeze:%d: Automaton '%s'",           _Slot, _CurrentAutomaton.c_str());
			nlwarning("CH:updatePos:antiFreeze:%d: _IsThereAMode '%s'",       _Slot, _IsThereAMode?"true":"false");
			nlwarning("CH:updatePos:antiFreeze:%d: dist2Dest '%f'",           _Slot, dist2Dest());
			nlwarning("CH:updatePos:antiFreeze:%d: Mode '%s(%d)'",            _Slot, modeToString(_Mode).c_str(), _Mode);
			nlwarning("CH:updatePos:antiFreeze:%d: Mode Wanted '%s(%d)'",     _Slot, modeToString(_ModeWanted).c_str(), _ModeWanted);
			nlwarning("CH:updatePos:antiFreeze:%d: Anim State Move '%s(%d)'", _Slot, CAnimationState::getAnimationStateName(animState(MOVE)).c_str(), animState(MOVE));
*/
			// Once too many more time reached, leave the method.
			if(antiFreezeCounter > 60)
				break;
		}
		// Update antiFreezeCounter.
		++antiFreezeCounter;
		// ANTI-FREEZE SYSTEM //
		//--------------------//
		//--------------------//
		// \todo GUIGUI : improve dist2first and dist2dest
		// Update Stages
		updateStages();
		// \todo GUIGUI : Bug with _TargetAngle in fight float, we overwrite here angle sent by the server ?
		// If the entity is too far (orientation not received yet), set the front vector as the moving direction.
		CVectorD distToUser = pos()-UserEntity->pos();
		distToUser.z = 0.0;
		if(distToUser.norm()*1000.0 > CLFECOMMON::THRESHOLD_ORIENTATION*0.9)
		{
			if(_FirstPos != INVALID_POS)
			{
				CVectorD dirToFirstP = _FirstPos-pos();
				dirToFirstP.z = 0.0;
				if(dirToFirstP != CVectorD::Null)
				{
					front(dirToFirstP.normed(), false, false);
					_TargetAngle = atan2(front().y, front().x);
				}
			}
		}
		// Mode Combat Float :
		if(!_IsThereAMode && (_Mode == MBEHAV::COMBAT_FLOAT))
		{
			// Update the position in combat float.
			updatePosCombatFloat(frameTimeRemaining, target);
		}
		// Compute the average speed to the destination.
		// double spd = 
		computeSpeed();


		bool stageReach = false;
		bool allToFirstPos = false;
		// Compute time to Stage or full Time Step if Stage too far.
		double loopTimeStep = frameTimeRemaining;
		double buLoopTimeStep = 0.0;
		double checkLoopTimeStep = loopTimeStep;
		// Update the animation used according to the speed/end anim/etc..
		updateAnimationState();
		// Backup the old time offset.
		double oldMovingTimeOffset    = animOffset(MOVE);
		double oldMovingTimeOffsetRun = animOffset(MOVE_BLEND_OUT);
		// WARNING -> Unknown Animation Selected.
		// Play the time step for the loop and truncate to End Anim if Time Step too big.
		if((_CurrentState != 0) && (animIndex(MOVE) != CAnimation::UnknownAnim))
			playToEndAnim(oldMovingTimeOffset, loopTimeStep);
		/////////////////
		// -- CHECK -- //
		if(loopTimeStep > checkLoopTimeStep)
		{
			nlwarning("CH:updtPos:%d: loopTimeStep(%f) > checkLoopTimeStep(%f).", _Slot, loopTimeStep, checkLoopTimeStep);
			if(ClientCfg.Check)
				nlstop;
			loopTimeStep = checkLoopTimeStep;
		}
		checkLoopTimeStep = loopTimeStep;
		// -- END CHECK -- //
		/////////////////////
		// (DEBUG) : Backup the Animation Time Offset after the adjustment with end anim to make some checks.
		double backupAnimTimeOff = animOffset(MOVE);
		//
		bool posInStage = false;
		double stageTime = -1.0;
		if(!_Stages._StageSet.empty())
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Pos_Move );
			// Get the reference on the current stage.
			CStageSet::TStageSet::iterator it = _Stages._StageSet.begin();
			CStage &stage = (*it).second;
			if(_Mode == MBEHAV::COMBAT_FLOAT && !_IsThereAMode)
				posInStage = false;
			else
				posInStage = stage.isPresent(CLFECOMMON::PROPERTY_POSITION);
			stageTime  = stage.time();
		}
		// dist2FirstPos() should not be Null if the destination is not Null (because of the code in updateStage).
		if(dist2FirstPos() > 0.0)
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Pos_dist2FirstPos_gt_0 );

			// Get the covered distance from the animation.
			double move = getTheMove(loopTimeStep, oldMovingTimeOffset, oldMovingTimeOffsetRun);
			// The move is big enough to reach the first step with motion.
			if(move >= dist2FirstPos())	// dist2FirstPos() > 0 -> move > 0.
			{
				double percent = dist2FirstPos() / move;
				// Adjust loopTimeStep
				loopTimeStep *= percent;
				if(loopTimeStep > checkLoopTimeStep)	// prevent bugs because of the double's precision
					loopTimeStep = checkLoopTimeStep;
				if(loopTimeStep < 0.0)
					loopTimeStep = 0.0;
				// Update Animation Time Offset (move greater than the dist to next stage; update animation time to get them equal).
				animOffset(MOVE,           oldMovingTimeOffset    + (animOffset(MOVE)          -oldMovingTimeOffset   )*percent);
				animOffset(MOVE_BLEND_OUT, oldMovingTimeOffsetRun + (animOffset(MOVE_BLEND_OUT)-oldMovingTimeOffsetRun)*percent);
				// \todo GUIGUI : check if the following line is necessary
				buLoopTimeStep = loopTimeStep;
				// First Position Reached
				pos(_FirstPos);
				dist2FirstPos(0.0);	// Current entity position is now the same as the First position so dis is Null.
				// Complete the Stage.
				if(_Mode != MBEHAV::COMBAT_FLOAT || _IsThereAMode)
				{
					if(posInStage)
						stageReach = true;
					else
						allToFirstPos = true;
				}
			}
			// Even if the movement is not enough to reach the first position, move the entity to this position.
			else if(move > 0.0)
			{
				// Compute the vector to the first stage with a position.
				CVectorD vectToFirstPos = _FirstPos - pos();
				vectToFirstPos.z = 0.0f;
				// Update entity position.
				if(vectToFirstPos != CVectorD::Null)
					pos(pos() + vectToFirstPos*(move/dist2FirstPos()));
			}
			// Else : There is no move.
		}
		else
		{
			CHECK(posInStage==false && dist2Dest()<=0.0);
		}

		// If there is no position in the next stage and the stage should be done already.
		if(!_Stages._StageSet.empty() && !posInStage && !stageReach && !allToFirstPos && ((_LastFrameTime+loopTimeStep) >= stageTime))
		{
			// Backup 'loopTimeStep' just in case of the stage could not be done.
			buLoopTimeStep = loopTimeStep;
			// Adjust loopTimeStep
			loopTimeStep = stageTime - _LastFrameTime;
			if(loopTimeStep > checkLoopTimeStep)	// prevent bugs because of the double's precision
				loopTimeStep = checkLoopTimeStep;
			if(loopTimeStep < 0.0)
				loopTimeStep = 0.0;
			//
			// \todo GUIGUI : adjust timeOffset, because we stopped the loop before
			//
			// Stage complete.
			stageReach = true;
		}
		/////////////////
		// -- CHECK -- //
		// Check the Animation Time Offset is not became greater than the old.
		if(animOffset(MOVE) > backupAnimTimeOff)
		{
			nlwarning("CH:updtPos:%d: backupAnimTimeOff(%f) < AnimationsTimeOffset(%f) animLen(%f) -> animOffset(MOVE) = backupAnimTimeOff",
				_Slot, backupAnimTimeOff, animOffset(MOVE), EAM->getAnimationLength(animId(MOVE)));
			if(ClientCfg.Check)
				nlstop;
			animOffset(MOVE, backupAnimTimeOff);
		}
		// Check loopTimeStep is not < 0;
		if(loopTimeStep < 0.0)
		{
			nlwarning("CH:updtPos:%d: loopTimeStep(%f) < 0 -> loopTimeStep=0.0.", _Slot, loopTimeStep);
			if(ClientCfg.Check)
				nlstop;
			loopTimeStep = 0.0;
		}
		// time spent could not be bigger than the time remaining
		if(loopTimeStep > frameTimeRemaining)
		{
			nlwarning("CH:updtPos:%d: loopTimeStep(%f) > frameTimeRemaining(%f) -> loopTimeStep=frameTimeRemaining.", _Slot, loopTimeStep, frameTimeRemaining);
			if(ClientCfg.Check)
				nlstop;
			loopTimeStep = frameTimeRemaining;
		}
		// -- END CHECK -- //
		/////////////////////
		// Manage Events that could be created by the animation (like sound).
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Pos_Anim_Event )
			animEventsProcessing(oldMovingTimeOffset, animOffset(MOVE));
		}
		// Apply all stages until the first stage with a pos.
		if(allToFirstPos)
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Pos_Apply_All_Stage_To_First_Pos );
			applyAllStagesToFirstPos();
		}
		// Stage is complete, apply modifications.
		else if(stageReach)
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Pos_Apply_Current_Stage );
			if(!applyCurrentStage())
				loopTimeStep = buLoopTimeStep;
		}
		// Compute the remaining Time Step.
		frameTimeRemaining -= loopTimeStep;
		// Update the last Time.
		_LastFrameTime += loopTimeStep;
	}// while(frameTimeRemaining > 0) //

	////////////////////////////////
	// UPDATE THE ENTITY POSITION //
	// Set the new position into PACS.
	{
		H_AUTO_USE ( RZ_Client_Entity_CL_Update_Pos_Pacs );
		pacsMove(pos());
	}

	/// (DEBUG) ///
	// Check frameTimeRemaining is perfectly equal to 0.
	if(frameTimeRemaining < 0.0)
		nlwarning("CCharacterCL::updatePos : frameTimeRemaining(%f) < 0 ! This should never happen.", frameTimeRemaining);
	/// (END DEBUG) ///

	// Update the children display.
	{
		H_AUTO ( RZ_Client_Entity_CL_Update_Pos_Children );
		std::list<CEntityCL *>::iterator itTmp, it = _Children.begin();
		while(it != _Children.end())
		{
			itTmp = it;
			// Next Child (done before just in case child is detached during his processFrame).
			it++;

			if(*itTmp)
			{
				CCharacterCL *child = dynamic_cast<CCharacterCL *>(*itTmp);
				if(child)
				{
					if ( ! ClientCfg.Light )
					{
						// Update the animation offset for the child.
						double animLength = EAM->getAnimationLength(animId(MOVE));
						if(animLength > 0.0)
						{
							double factor = animOffset(MOVE) / animLength;
							if(factor > 1.0)
								factor = 1.0;
							double childTimeOffset = factor*EAM->getAnimationLength(child->animId(MOVE));
							child->animOffset(MOVE, childTimeOffset);
						}
						else
							child->animOffset(MOVE, 0.0);
						child->processFrame(currentTimeInMs);
					}
					child->pacsMove(pos());	// Move the child at the same position than the parent.
				}
				else
					nlwarning("Character '%d': Child is not a 'CCharacterCL'.", _Slot);
			}
			else
				nlwarning("Character '%d': Child not allocated.", _Slot);
		}
	}
}// updatePos //

//-----------------------------------------------
// updateVisiblePostRender :
// Update the entity after the render like for the head offset.
//-----------------------------------------------
void CCharacterCL::updateVisiblePostRender()	// virtual
{
	// Compute the headoffset
	if(_HeadBoneId != -1 && !_Skeleton.empty())
	{
		if(_Skeleton.getLastClippedState() && _Skeleton.isBoneComputed(_HeadBoneId))
		{
			UBone headBone=_Skeleton.getBone(_HeadBoneId);
			const CMatrix &headMat = headBone.getLastWorldMatrixComputed();
			_HeadOffset = headMat.getPos()-pos();
			_HeadOffsetComputed= true;
		}
	}
}// updateVisiblePostRender //

//-----------------------------------------------

void CCharacterCL::updateAllPostRender()
{
}

//-----------------------------------------------
// processFrame :
//-----------------------------------------------
void CCharacterCL::processFrame(const TTime &currentTimeInMs)
{
	// Prepare stages and update information from them.
	updateStages();

	// Compute the time remaining until frame completely processed.
	double timeRemaining = computeTimeStep(((double)currentTimeInMs)*0.001);

	// Compute the time spent between 2 frames.
	while(timeRemaining > 0.0)
	{
		// Time already processed until now.
		double timeProcessed = timeRemaining;

		// Process Stages.
		if(!_Stages._StageSet.empty())
		{
			// Get the reference on the current stage.
			CStageSet::TStageSet::iterator it = _Stages._StageSet.begin();
			CStage &stage = (*it).second;

			// Check if the stage should be done already.
			if((_LastFrameTime+timeProcessed) >= stage.time())
			{
				// Stage Done during the Frame.
				if(stage.time() > _LastFrameTime)
					timeProcessed = stage.time() - _LastFrameTime;
				// This Stage should have been done already before last frame
				else
					timeProcessed = 0.0;

				// Process the stage.
				processStage(stage);

				// Stage complete.
				_Stages._StageSet.erase(it);
			}
		}

		// Compute the remaining Time Step.
		timeRemaining -= timeProcessed;
		// Update the last Time.
		_LastFrameTime += timeProcessed;
	}

	// Just to be sure, Last frame time = current time once all is done.
	_LastFrameTime = ((double)currentTimeInMs)*0.001;
}// processFrame //

//-----------------------------------------------
// processStage :
// Process the stage.
//-----------------------------------------------
void CCharacterCL::processStage(CStage &stage)
{
	// Apply Mode (if there is a mode, there is a position too).
	pair<bool, sint64> resultMode = stage.property(PROPERTY_MODE);
	if(resultMode.first)
	{
		uint8 mo = *(uint8 *)(&resultMode.second);
		MBEHAV::EMode mode = (MBEHAV::EMode)mo;
		if(mode != _Mode)
		{
			// release the old mode.
			if ( (_Mode == MBEHAV::MOUNT_NORMAL || _Mode == MBEHAV::MOUNT_SWIM)
				&& (mode != MBEHAV::MOUNT_NORMAL && mode != MBEHAV::MOUNT_SWIM)
				)
			{
				// Unlink the mount and the rider.
				parent(CLFECOMMON::INVALID_SLOT);
				_Mount = CLFECOMMON::INVALID_SLOT;
				_Rider = CLFECOMMON::INVALID_SLOT;

				// Restore collisions.
				if(_Primitive)
				{
					// \todo GUIGUI : do that without dynamic cast
					if(dynamic_cast<CPlayerCL *>(this))
						_Primitive->setOcclusionMask(MaskColPlayer);
					else
						_Primitive->setOcclusionMask(MaskColNpc);
				}

				// Get stage position.
				if(stage.getPos(_OldPos))
				{
					// Backup the time
					_OldPosTime = stage.time();
					// Unseat the entity at the position given in the stage.
					pacsMove(_OldPos);
				}
				else
					nlwarning("CH:processStage:%d: The stage should have a position with the mode.", _Slot);
			}

			// Set the new mode.
			_Mode				= mode;
			_ModeWanted			= mode;
			// Compute the automaton
			computeAutomaton();
			computeAnimSet();
			setAnim(CAnimationStateSheet::Idle);
		}
	}
	// Not a mode -> so search for a position.
}// processStage //


//-----------------------------------------------
// updateBlink :
// Update the player blink state
//-----------------------------------------------
void CCharacterCL::updateBlink(const TTime &currentTimeInMs)
{
	// Only for homine
	GSGENDER::EGender gender = getGender();
	if ((gender == GSGENDER::male) || (gender == GSGENDER::female))
	{
		float blend;

		// Some parameters
		static const double blinkTime = 100.f;
		static const double minBlinkLength = 500.f;
		static const double maxBlinkLength = 5000.f;

		// Next blink time is valid ?
		bool validTime = (_NextBlinkTime + blinkTime >= currentTimeInMs) && (_NextBlinkTime <= (currentTimeInMs + maxBlinkLength));

		// Blink end ?
		bool blinkEnd = (currentTimeInMs >= _NextBlinkTime + blinkTime);

		// Blink is finished or next blink time is invalid ?
		if ( blinkEnd || !validTime )
		{
			blend = 0;

			// Compute next time
			_NextBlinkTime = (TTime)(((double)rand () / (double)RAND_MAX) * (maxBlinkLength - minBlinkLength) + minBlinkLength + (double)currentTimeInMs);
		}
		else
		{
			// Blink time ?
			if (currentTimeInMs >= _NextBlinkTime)
			{
				blend = 100.f;
			}
			else
			{
				// Do nothing
				return;
			}
		}

		// Get the face
		SInstanceCL *face = getFace ();

		// Set the blend shape
		if(face && !face->Current.empty())
			face->Current.setBlendShapeFactor ("visage_100", blend, true);
	}
}// updateBlink //


//-----------------------------------------------
// getFace :
// Update eyes blink. For the moment, called by updatePos.
//-----------------------------------------------
CEntityCL::SInstanceCL *CCharacterCL::getFace ()
{
	// Implemented in CPlayerCL
	return idx2Inst(_FaceIdx);
}


//---------------------------------------------------
// updateDisplay :
// Get the entity position and set all visual stuff with it.
// \todo GUIGUI : put this method 'virtual' to have a different code for the user (no playlist).
// \todo GUIGUI : manage the parent better.
//---------------------------------------------------
ADD_METHOD(void CCharacterCL::updateDisplay(CEntityCL *parent))
	// Animable ?
	if(_PlayList)
	{
		// Reverse the animation if needed.
		const double animOffsetMOV = _AnimReversed[MOVE]           ? EAM->getAnimationLength(animId(MOVE))           - animOffset(MOVE)          : animOffset(MOVE);
		const double animOffsetACT = _AnimReversed[ACTION]         ? EAM->getAnimationLength(animId(ACTION))         - animOffset(ACTION)        : animOffset(ACTION);
		const double animOffsetBLE = _AnimReversed[MOVE_BLEND_OUT] ? EAM->getAnimationLength(animId(MOVE_BLEND_OUT)) - animOffset(MOVE_BLEND_OUT): animOffset(MOVE_BLEND_OUT);
		// Update Speed Factor
		_PlayList->setTimeOrigin(MOVE,           TimeInSec-animOffsetMOV);
		_PlayList->setTimeOrigin(ACTION,         TimeInSec-animOffsetACT);
		_PlayList->setTimeOrigin(MOVE_BLEND_OUT, TimeInSec-animOffsetBLE);
		float weight;
		if(_BlendRemaining)
			weight = _PlayList->getLocalWeight(MOVE, TimeInSec);
		else
			weight = 1.0f;
		_PlayList->setWeight(MOVE,           weight*(float)(1.0-runFactor()));
		_PlayList->setWeight(MOVE_BLEND_OUT, weight*(float)runFactor());
		// If the animation exist update the display.
		if(animIndex(MOVE) != CAnimation::UnknownAnim)
		{
			// POSITION //
			// Get the 3D position for the current time in the animation (Vector Null if animation has no move).
			CVector currentAnimPos;
			if(!EAM->interpolate(animId(MOVE), animOffsetMOV, currentAnimPos))
				currentAnimPos = CVector::Null;
			else
			{
				// If the current animation state is a move, do not take the Y move in the animation because it's the code that compute the move.
				if(_CurrentState && _CurrentState->Move)
				{
					CVector currentAnimPosStart;
					if(!EAM->interpolate(animId(MOVE), 0.0, currentAnimPosStart))
						currentAnimPosStart = CVector::Null;
					if(_CurrentState->XFactor) currentAnimPos.x = currentAnimPosStart.x;
					if(_CurrentState->YFactor) currentAnimPos.y = currentAnimPosStart.y;
					if(_CurrentState->ZFactor) currentAnimPos.z = currentAnimPosStart.z;
				}
				// Scale the animation with the Character Scale Pos if the animation need it.
				{
					bool applyCharacterScalePosFactor = true;
					// \todo GUIGUI : faire cette histoire de emote beaucoup mieux, C NULL.
					const CAnimationState *animStatePtr;
					// If the current animation is an emote, get the right animation state.
					if(animState(MOVE) == CAnimationStateSheet::Emote)
						animStatePtr = _CurrentAnimSet[MOVE]->getAnimationState(_SubStateKey);
					else
						animStatePtr = _CurrentAnimSet[MOVE]->getAnimationState(animState(MOVE));
					if(animStatePtr)
					{
						const CAnimation *anim = animStatePtr->getAnimation(animIndex(MOVE));
						if(anim)
							applyCharacterScalePosFactor = anim->applyCharacterScalePosFactor();
					}
					// scale it.
					if(applyCharacterScalePosFactor)
						currentAnimPos *= _CharacterScalePos;
				}
				// Scale according to the gabarit (choose at the character creation).
				currentAnimPos *= _CustomScalePos;
			}
			// Blend Walk/Run
			if(runFactor() > 0.0)
			{
				CVector currentAnimPosRun;
				if(!EAM->interpolate(animId(MOVE_BLEND_OUT), animOffsetBLE, currentAnimPosRun))
					currentAnimPosRun = CVector::Null;
				else
				{
					// If the current animation state is a move, do not take the Y move in the animation because it's the code that compute the move.
					if(_CurrentState && _CurrentState->Move)
					{
						CVector currentAnimPosStart;
						if(!EAM->interpolate(animId(MOVE_BLEND_OUT), 0.0, currentAnimPosStart))
							currentAnimPosStart = CVector::Null;
						if(_CurrentState->XFactor) currentAnimPosRun.x = currentAnimPosStart.x;
						if(_CurrentState->YFactor) currentAnimPosRun.y = currentAnimPosStart.y;
						if(_CurrentState->ZFactor) currentAnimPosRun.z = currentAnimPosStart.z;
					}
					// Scale it by the CharacterScalePos, if needed, according to the animation.
					bool applyCharacterScalePosFactor = true;
					const CAnimationState *animStatePtr = _CurrentAnimSet[MOVE_BLEND_OUT]->getAnimationState(animState(MOVE_BLEND_OUT));
					if(animStatePtr)
					{
						const CAnimation *anim = animStatePtr->getAnimation(animIndex(MOVE_BLEND_OUT));
						if(anim)
							applyCharacterScalePosFactor = anim->applyCharacterScalePosFactor();
					}
					// scale it.
					if(applyCharacterScalePosFactor)
						currentAnimPosRun *= _CharacterScalePos;
					// Scale according to the gabarit.
					currentAnimPosRun *= _CustomScalePos;
				}

				currentAnimPos = currentAnimPos*(float)(1.0-runFactor()) + currentAnimPosRun*(float)runFactor();
			}
			// ROTATION //
			// Get the rotation for the current time in the animation (Rotation Null if animation has no rotation).
			CQuat currentAnimRot;
			if(!EAM->interpolate(animId(MOVE), animOffsetMOV, currentAnimRot))
				currentAnimRot = CQuat::Identity;
			else
			{
				// If the animation is a rotation -> Do just a part of the animation.
				if(parent==0 && _CurrentState && _CurrentState->Rotation && _RotationFactor!=-1.0)
				{
					// Get the Rotation at the beginning of the animation.
					CQuat currentAnimRotStart;
					if(!EAM->interpolate(_PlayList->getAnimation(MOVE), 0.0, currentAnimRotStart))
						currentAnimRotStart = CQuat::Identity;

					double animLength = EAM->getAnimationLength(animId(MOVE));

					// Get the Rotation at the beginning of the animation.
					CQuat currentAnimRotEnd;
					if(!EAM->interpolate(_PlayList->getAnimation(MOVE), animLength, currentAnimRotEnd))
						currentAnimRotEnd = CQuat::Identity;

					// Get the angle done by the animation from the beginning
					CQuat rotStartTmp = currentAnimRotStart;
					rotStartTmp.invert();
					CQuat rotTmp =  rotStartTmp * currentAnimRot;
					float ang = rotTmp.getAngle();

					currentAnimRot = applyRotationFactor(currentAnimRot, (float)_RotationFactor, currentAnimRotStart, currentAnimRotEnd, (float)(animOffsetMOV/animLength));

					// Get the angle done once scaled.
					rotTmp =  rotStartTmp * currentAnimRot;
					CMatrix rotMat;
					rotMat.identity();
					rotMat.rotateZ(_CurrentState->RotFactor*(ang-rotTmp.getAngle()));

					// Apply the scaled rotation to the position.
					currentAnimPos = rotMat*currentAnimPos;
//// OLD ////
/*
					// Get the Rotation at the beginning of the animation.
					CQuat currentAnimRotStart;
					if(!EAM->interpolate(_PlayList->getAnimation(MOVE), 0.0, currentAnimRotStart))
						currentAnimRotStart = CQuat::Identity;

					// Find the closest quat.
//					currentAnimRotStart.makeClosest(currentAnimRot);

					// Get the angle done by the animation from the beginning
					CQuat rotStartTmp = currentAnimRotStart;
					rotStartTmp.invert();
					CQuat rotTmp =  rotStartTmp * currentAnimRot;
					float ang = rotTmp.getAngle();

					// Get the Rotation scaled.
					currentAnimRot = CQuat::slerp(currentAnimRotStart, currentAnimRot, (float)_RotationFactor);

					// Get the angle done once scaled.
					rotTmp =  rotStartTmp * currentAnimRot;
					CMatrix rotMat;
					rotMat.identity();
					rotMat.rotateZ(_CurrentState->RotFactor*(ang-rotTmp.getAngle()));

					// Apply the scaled rotation to the position.
					currentAnimPos = rotMat*currentAnimPos;
*/
//// FIN OLD ////
				}
			}
			// Blend Walk/Run
			if(runFactor() > 0.0)
			{
				// Get the rotation for the current time in the animation (Rotation Null if animation has no rotation).
				CQuat currentAnimRotRun;
				if(!EAM->interpolate(animId(MOVE_BLEND_OUT), animOffsetBLE, currentAnimRotRun))
					currentAnimRotRun = CQuat::Identity;
				currentAnimRotRun.makeClosest(currentAnimRot);
				currentAnimRot = CQuat::slerp(currentAnimRot, currentAnimRotRun, (float)runFactor());
			}

			// Animation Matrix
			CMatrix AnimMatrixRot;
			AnimMatrixRot.identity();
			AnimMatrixRot.setRot(currentAnimRot);

			// Rotation 180 degrees Matrix
			CMatrix rot180;
			rot180.identity();
			if(parent == 0)
				rot180.rotateZ((float)Pi);

			// Logical entity Matrix.
			CMatrix current;
			if(parent == 0)
				current = _DirMatrix;
			else
				current.identity();

			// Convert the anim position in a world position.
			currentAnimPos = (current*rot180)*currentAnimPos;

			rot180 *= AnimMatrixRot;
			current *= rot180;


			// Get the rotation for the current time in the animation (Rotation Null if animation has no rotation).
			if(ClientCfg.BlendFrameNumber && _BlendRemaining > 0)
			{
				CQuat tmpQuat;
				_OldRotQuat.makeClosest(current.getRot());
				tmpQuat = CQuat::slerp(current.getRot(), _OldRotQuat, ((float)_BlendRemaining/(float)(ClientCfg.BlendFrameNumber+1)));
				current.setRot(tmpQuat);
				// 1 more frame played.
				_BlendRemaining--;
			}


			// Compute the position for the instance.
			CVectorD tmpPos;
			if(parent == 0)
			{
				tmpPos = pos();
				tmpPos += currentAnimPos;
			}
			// If the entity is on a mount, just adjust the position with the animation.
			else
				tmpPos = currentAnimPos;
			// Set the skeleton position and rotation.
			if(!_Skeleton.empty())
			{
				_Skeleton.setRotQuat(current.getRot());
				_Skeleton.setPos(tmpPos);
			}
			// Only Instances with no skeleton (objects).
			else if(!_Instances.empty() && !_Instances[0].Current.empty())
			{
				_Instances[0].Current.setRotQuat(current.getRot());
				_Instances[0].Current.setPos(tmpPos);
			}
			// Set the instance position and rotation.
			else if(!_Instance.empty())
			{
				_Instance.setRotQuat(current.getRot());
				_Instance.setPos(tmpPos);
			}
			else
			{
				static bool once = false;

				if (!once)
				{
					nlwarning("CH::updtDisp:%d: no instance nor skeleton. Sheet Id '%d(%s)'.", _Slot, _SheetId.asInt(), _SheetId.toString().c_str());
					once = true;
				}
			}
		}
		// Else Keep the lastest correct display.
		else
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Display_Unknown_Anim )

			// Rotation 90 degrees Matrix
			CMatrix rot90;
			rot90.identity();
			if(parent == 0)
				rot90.rotateZ((float)(Pi/2.0));

			// Logical entity Matrix.
			CMatrix current;
			if(parent == 0)
				current = _DirMatrix;
			//			else
			//				current.identity();

			current *= rot90;

			// Changes the skeleton position.
			if(!_Skeleton.empty())
			{
				_Skeleton.setRotQuat(current.getRot());
				if(parent == 0)
					_Skeleton.setPos(pos());
//				else
//					_Skeleton.setPos(currentAnimPos);
			}
			// Only Instances with no skeleton (objects).
			else if(!_Instances.empty() && !_Instances[0].Current.empty())
			{
				_Instances[0].Current.setRotQuat(current.getRot());
				if(parent == 0)
					_Instances[0].Current.setPos(pos());
			}
		}
	}
	else
	{
		// Changes the skeleton position.
		if(!_Skeleton.empty())
		{
			_Skeleton.setPos(pos());
		}
		// Only Instances with no skeleton (objects).
		else if(!_Instances.empty() && !_Instances[0].Current.empty())
		{
			// Logical entity Matrix.
			CMatrix current;
			if(parent == 0)
				current = _DirMatrix;

			_Instances[0].Current.setRotQuat(current.getRot());
			_Instances[0].Current.setPos(pos());
		}
		// Changes the instance position.
		else if(!_Instance.empty())
		{
			_Instance.setPos(pos());
		}
	}

	if(!ClientCfg.Light)
	{
		// update texture Async Loading
		updateAsyncTexture();
		// update lod Texture
		updateLodTexture();

	}

	// Update the children display.
	std::list<CEntityCL *>::iterator it = _Children.begin();
	while(it != _Children.end())
	{
		// Update the display for the child
		(*it)->updateDisplay(this);
		// Next Child.
		++it;
	}
}// updateDisplay //


//---------------------------------------------------
// getHeadPos :
// Method to get the position of the head (in the world).
// \param headPos: will be set with the head position if succeed.
// \return 'true' if the param has been updated.
// \warning this method do NOT check if there is a skeleton.
//---------------------------------------------------
bool CCharacterCL::getHeadPos(NLMISC::CVector &headPos)
{
	// if never computed (eg: clipped or lod)
	if(!_HeadOffsetComputed)
	{
		// force compute the bone
		if(_HeadBoneId != -1 && !_Skeleton.empty())
		{
			_Skeleton.forceComputeBone(_HeadBoneId);
			UBone headBone=_Skeleton.getBone(_HeadBoneId);
			const CMatrix &headMat = headBone.getLastWorldMatrixComputed();
			_HeadOffset = headMat.getPos()-pos();
		}
		_HeadOffsetComputed= true;
	}

	// return the pos with the last head offset computed
	headPos = pos()+_HeadOffset;

	return true;
}// getHeadPos //

//---------------------------------------------------
// updateHeadDirection :
// Update the head Direction.
//---------------------------------------------------
void CCharacterCL::updateHeadDirection(CEntityCL *target)
{
	// Does the entity got a target to track with the head ?
	// No head targeting if the target slot is the same as the entity slot
	if(_TargetSlot!=CLFECOMMON::INVALID_SLOT && _TargetSlot!=_Slot)
	{
		// Is the target allocated.
		if(target != 0)
		{
			// Do not orientate the head to the target if too far.
			CVectorD vectDist = target->pos() - pos();
			if((fabs(vectDist.x) + fabs(vectDist.y)) <= ClientCfg.MaxHeadTargetDist && vectDist != CVectorD::Null)
			{
				// Do not orientate the head to the target if behind.
				vectDist.normalize();
				if(fabs(angleBetween2Vect(dir(), vectDist)) < Pi/3.0)
				{
					CVector targetheadPos;
					if(target->getHeadPos(targetheadPos))
					{
						_TargetAnimCtrl.Mode = CTargetAnimCtrl::TargetMode;
						_TargetAnimCtrl.WorldTarget = targetheadPos;
						return;
					}
				}
			}
		}
	}

	_TargetAnimCtrl.Mode = CTargetAnimCtrl::DirectionMode;
	CMatrix frontMat;
	frontMat.setRot(CVector::I, front(), CVector::K, true);
	frontMat.normalize(CMatrix::YZX);
	_TargetAnimCtrl.CurrentWorldDirection = frontMat.getRot();
}// updateHeadDirection //


//---------------------------------------------------
// displayName :
// Display the entity name.
//---------------------------------------------------
void CCharacterCL::displayName()
{
	// There is no Context -> Cannot display a name.
	if(TextContext == 0)
		return;

	NLMISC::CVector namePos;
	// There is a skeleton.
	if(!_Skeleton.empty() && !ClientCfg.Light)
	{
		// Only if the skeleton is visible.
		if(!isVisible())
			return;
		// Do not display the name in LoD.
		if(_Skeleton.isDisplayedAsLodCharacter())
			return;
		// If the entity was not displayed last frame (clipped) -> do not display the name.
		if(!_Skeleton.getLastClippedState())
			return;

		// Is there a Bone for the Name ?
		if(_NameBoneId != -1)
			namePos = _Skeleton.getBone(_NameBoneId).getLastWorldMatrixComputed().getPos();
		// No Bone for the name -> Draw it at a default position.
		else
			namePos = pos() + CVector(0.f, 0.f, 2.0f);
	}
	// If there is no skeleton -> compute a position for the name.
	else
	{
		namePos = pos() + CVector(0.f, 0.f, 2.0f);
	}

	// Create the matrix and set the orientation according to the camera.
	CMatrix matrix;
	matrix.identity();
	matrix.setRot(MainCam.getRotQuat());
	matrix.setPos(namePos);

	CVector distPos = MainCam.getPos()-pos();
	float scale = distPos.norm();

	// don't display too far names
	if (ClientCfg.Light && scale > 20)
		return;

	if(scale <= 1.0f)
		scale = 1.0f;
	else if(scale > ClientCfg.ConstNameSizeDist)
		scale = ClientCfg.ConstNameSizeDist;
	// Too Far to display a name.
	else if(scale > ClientCfg.MaxNameDist)
		return;
	// Compute the final scale.
	matrix.scale(ClientCfg.NameScale*scale);

	// Draw the name.
	drawName(matrix);
}// displayName //

//---------------------------------------------------
// drawName :
// Draw the name.
//---------------------------------------------------
void CCharacterCL::drawName(const NLMISC::CMatrix &mat)	// virtual
{
	const ucstring &ucname = getEntityName();
	if(!getEntityName().empty())
	{
		// If there is no extended name, just display the name
		if(_NameEx.empty())
			TextContext->render3D(mat, ucname);
		// If there is an extended name, display the extended name at the name place and the name above.
		else
		{
			// Display the Extended Name at right place.
			TextContext->render3D(mat, _NameEx);
			// Compute the position for the Name.
			CMatrix mat2;
			mat2.identity();
			mat2.setRot(MainCam.getRotQuat());
			CVector v = mat.getPos()+mat.getK().normed()*ClientCfg.NamePos*mat.getScaleUniform();
			mat2.setPos(v);
			mat2.scale(mat.getScaleUniform());
			// Diaplay the name.
			TextContext->render3D(mat2, ucname);
		}
	}
	// Name from Sheet
	else
	{
		if(_Sheet != 0)
		{
			const ucstring name(STRING_MANAGER::CStringManagerClient::getCreatureLocalizedName(_Sheet->Id));
			if (name.find(ucstring("<NotExist:")) != 0)
				TextContext->render3D(mat, name);
		}
	}
}// drawName //


//---------------------------------------------------
// displayModifiers :
// Display the Hp Bar
//---------------------------------------------------
void CCharacterCL::displayModifiers()	// virtual
{
	// if none, no op
	if(	_HPDisplayed.empty())
		return;

	// **** get the name pos
	NLMISC::CVectorD namePos;
	if(!getNamePos(namePos))
		namePos = pos() + CVector(0.f, 0.f, 2.0f);
	// remove but keep the Z to the ground
	float	currentDeltaZ= float(namePos.z - pos().z);
	CVector	 groundPos= namePos;
	groundPos.z-= currentDeltaZ;


	// **** compute the scale
	float	dist = (MainCam.getPos()-pos()).norm();
	float	scale= 1.f;
	if(dist > ClientCfg.MaxNameDist)
		return;
	if ( dist < ClientCfg.ConstNameSizeDist )
		scale = 1.0f;
	else
		scale = ClientCfg.ConstNameSizeDist / dist;


	// **** Display HP modifiers.
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	std::list<HPMD>::iterator itTmp;
	std::list<HPMD>::iterator it = _HPDisplayed.begin();
	while(it != _HPDisplayed.end())
	{
		HPMD &mod = *it;
		//
		const	float totalDuration= 3.f;
		const	float noFadeDuration= 1.f;
		const	float fadeDuration= totalDuration-noFadeDuration;
		if(TimeInSec > (mod.Time+totalDuration))
		{
			itTmp = it;
			++it;
			_HPDisplayed.erase(itTmp);
		}
		else if (TimeInSec >= mod.Time)
		{
			ucstring hpModifier;
			if (mod.Text.empty())
				hpModifier = ucstring(toString("%d", mod.Value));
			else
				hpModifier = mod.Text;
			double t = TimeInSec-mod.Time;
			// for character, keep the deltaZ the first time it is displayed, and apply the same each frame
			// (avoid Z movement of the flying text because of animation)
			if(mod.DeltaZ==-FLT_MAX)
				mod.DeltaZ= currentDeltaZ;
			// Compute the position for the Modifier.
			float	dynT= sqrtf((float)t/totalDuration);		// a sqrt just so it looks much more "jumpy"
			CVector		pos= groundPos + CVector(0.0f, 0.0f, mod.DeltaZ + dynT*1.f);
			// fade
			if(t<noFadeDuration)
				mod.Color.A= 255;
			else
				mod.Color.A= 255-(uint8)((t-noFadeDuration)*255.0/fadeDuration);

			// Display the hp modifier. display with a X offset according if user or not, for more readability
			sint	deltaX= -pIM->FlyingTextManager.getOffsetXForCharacter();
			if(UserEntity && UserEntity->slot()==slot())
				deltaX*= -1;
			pIM->FlyingTextManager.addFlyingText(&mod, hpModifier, pos, mod.Color, scale, deltaX);

			// Next
			++it;
		}
		else
		{
			// Next
			++it;
		}
	}
}// displayModifiers //


//---------------------------------------------------
// drawPath :
// Draw Pathes
//---------------------------------------------------
void CCharacterCL::drawPath()	// virtual
{
	// Pivot
	CLineColor line;
	CVector pl0 = pos();
	CVector pl1 = pos()+CVector(0.f, 0.f, 2.f);
	line = CLine(pl0, pl1);
	line.Color0 = CRGBA(150,0,255);
	line.Color1 = CRGBA(150,0,255);
	Driver->drawLine(line, GenericMat);

	line = CLine(_PositionLimiter, _PositionLimiter+CVector(0.f, 0.f, 2.f));
	line.Color0 = CRGBA(255,64,128);
	line.Color1 = CRGBA(255,64,128);
	Driver->drawLine(line, GenericMat);

	CVector p0 = pos();
	p0.z += 1.f;

	// Draw Front
	line = CLine(p0, p0+front());
	line.Color0 = CRGBA(0,255,0);
	line.Color1 = CRGBA(0,255,0);
	Driver->drawLine(line, GenericMat);

	// Draw Direction
	line = CLine(p0, p0+dir());
	line.Color0 = CRGBA(255,255,0);
	line.Color1 = CRGBA(255,255,0);
	Driver->drawLine(line, GenericMat);

	// Go to the First Stage.
	CStageSet::TStageSet::iterator it = _Stages._StageSet.begin();
	// Compute the distance over all Stages.
	while(it != _Stages._StageSet.end())
	{
		// Compute Distance.
		CVectorD stagePos;
		if((*it).second.getPos(stagePos))
		{
			CVector p1 = stagePos;
			CVector p2 = p0;
			p2.z = (float)stagePos.z;
			p2 = p2 + (stagePos-p2).normed();

			getCollisionEntity()->snapToGround(p1);
			p1.z += 0.05f;
			getCollisionEntity()->snapToGround(p2);
			p2.z += 0.03f;

			line = CLine(p0, p2);
			line.Color0 = CRGBA(0,0,255);
			line.Color1 = CRGBA(0,0,255);
			Driver->drawLine(line, GenericMat);

			p1.z += 0.03f;
			line = CLine(p0, p1);
			line.Color0 = CRGBA(255,0,0);
			line.Color1 = CRGBA(255,0,0);
			Driver->drawLine(line, GenericMat);

			p0 = p1;
		}

		// Next Stage.
		++it;
	}
}// drawPath //


//---------------------------------------------------
// drawBox :
// Draw the selection Box.
//---------------------------------------------------
void CCharacterCL::drawBox()	// virtual
{
	if(!ClientCfg.Light)
		::drawBox(_Aabbox.getMin(), _Aabbox.getMax(), CRGBA(0,250,0));

	// Draw the PACS box (adjust the color according to the PACS valid or not).
	NLMISC::CAABBox PACSBox = _Aabbox;
	CVector halfSize = PACSBox.getHalfSize();
	halfSize.x = 0; halfSize.y = 0;
	PACSBox.setCenter(_FinalPacsPos+halfSize);
	UGlobalPosition gPos;
	if(_Primitive)
		_Primitive->getGlobalPosition(gPos, dynamicWI);
	::drawBox(PACSBox.getMin(), PACSBox.getMax(), ((gPos.InstanceId == -1) && (T1%1000)>500)?CRGBA(255,0,0):CRGBA(0,250,250));

	if(!ClientCfg.Light)
	{
		::drawBox(selectBox().getMin(), selectBox().getMax(), CRGBA(250,250,0));
		// Draw the clip Sphere
		CVector clipPos = _Aabbox.getCenter();
		clipPos.z+= _ClipDeltaZ - _Aabbox.getHalfSize().z;	// _ClipDeltaZ is relative to pos on ground
		::drawSphere(clipPos, _ClipRadius, CRGBA(0,0,250));
	}
}// drawBox //

//---------------------------------------------------
// selectBox :
// Return the selection box.
//---------------------------------------------------
const NLMISC::CAABBox &CCharacterCL::selectBox()	// virtual
{
	// recompute the selection box?
	if(_LastSelectBoxComputeTime<T1)
	{
		_LastSelectBoxComputeTime=T1;
		bool	found= false;

		// if skeleton, compute aabox from precise skeleton method
		if(!_Skeleton.empty())
		{
			// Don't compute if in LOD form (else flick because sometimes valid because of shadow animation)
			if(!_Skeleton.isDisplayedAsLodCharacter() &&
				_Skeleton.computeRenderedBBoxWithBoneSphere(_SelectBox))
				found= true;
		}
		// else compute from static bot object
		else
		{
			UInstance	inst;
			// try with _Instances array first
			if(!_Instances.empty())
				inst= _Instances[0].Current;
			// Fallback to _Instance (??)
			if(inst.empty())
				inst= _Instance;

			// if static instance found
			if(!inst.empty())
			{
				CAABBox		bbox;
				inst.getShapeAABBox(bbox);
				// if supported (ie bbox not null)
				if(bbox.getHalfSize()!=CVector::Null)
				{
					// Transform bbox to world
					_SelectBox= CAABBox::transformAABBox(inst.getLastWorldMatrixComputed(), bbox);
					found= true;
				}
			}
		}

		// if not found, fallback to default bbox
		if(!found)
			_SelectBox = _Aabbox;
	}

	// Return the selection box.
	return _SelectBox;
}// selectBox //


//---------------------------------------------------
// updateAttachedFXListForSlotRemoved :
//---------------------------------------------------
void CCharacterCL::updateAttachedFXListForSlotRemoved(std::list<CAttachedFX::TSmartPtr> &fxList, const CLFECOMMON::TCLEntityId &slotRemoved)
{
	std::list<CAttachedFX::TSmartPtr>::iterator it = fxList.begin();
	while (it != fxList.end())
	{
		std::list<CAttachedFX::TSmartPtr>::iterator tmpIt = it;
		++ it;
		if ((*tmpIt)->StickMode &&
			((*tmpIt)->StickMode == CFXStickMode::OrientedTowardTargeter ||
			 (*tmpIt)->StickMode == CFXStickMode::UserBoneOrientedTowardTargeter ||
			 (*tmpIt)->StickMode == CFXStickMode::UserBoneRay
			)
		   )
		{
			if ((*tmpIt)->TargeterInfo.Slot == slotRemoved)
			{
				fxList.erase(tmpIt);
			}
		}
	}
}

//---------------------------------------------------
// slotRemoved :
// To Inform about an entity removed (to remove from selection for example).
// This will remove the entity from the target.
// \param slot : Slot of the entity that will be removed.
//---------------------------------------------------
void CCharacterCL::slotRemoved(const CLFECOMMON::TCLEntityId &slotRemoved)
{
	// If the target is the entity that will be removed -> no target
	if(_TargetSlot == slotRemoved)
		_TargetSlot = CLFECOMMON::INVALID_SLOT;
	// invalidate targeter slots in anim fxs
	updateAttachedFXListForSlotRemoved(_AttachedFXListForCurrentAnim, slotRemoved);
	updateAttachedFXListForSlotRemoved(_AttachedFXListToRemove, slotRemoved);
}// slotRemoved //

//---------------------------------------------------
// nbStage :
// Return number of stage remaining.
//---------------------------------------------------
uint CCharacterCL::nbStage()
{
	return (uint)_Stages._StageSet.size();
}// nbStage //


//---------------------------------------------------
// attackRadius :
// Method to return the attack radius of an entity (take the scale into account).
//---------------------------------------------------
double CCharacterCL::attackRadius() const
{
	return _Sheet->DistToFront * getScale();
}// attackRadius //

//---------------------------------------------------
// getAttackerPos :
// Return the position the attacker should have to combat according to the attack angle.
// \param ang : 0 = the front, >0 and <Pi = left side, <0 and >-Pi = right side.
// \todo : GUIGUI precalculate entity matrix
//---------------------------------------------------
CVectorD CCharacterCL::getAttackerPos(double ang, double dist) const
{
	// Compute the local angle
	ang = computeShortestAngle(atan2(front().y, front().x), ang);
	ang += Pi;
	if(ang > Pi)
		ang -= 2*Pi;

	// Compute the local position.
	CVectorD p;
	float distToSide, distToFront, distToBack;
	bool useComplexShape = false;
	if (useComplexShape) // Keep this code for when AIS become complex shape aware
	{
		distToSide = _Sheet->DistToSide;
		distToFront = _Sheet->DistToFront;
		distToBack = _Sheet->DistToBack;
	}
	else // use round shape here
	{
		distToSide = _Sheet->ColRadius;
		distToFront = _Sheet->ColRadius;
		distToBack = _Sheet->ColRadius;
	}
	p.x = getScale()*distToSide*sin(-ang) + dist*sin(-ang);	// or: pos.x = _Sheet->DistToSide*cos(ang) + dist*cos(ang); but 0 should be right side.
	p.y = dist*cos(ang);
	if(fabs(ang) <= Pi/2.0)
		p.y += getScale()*distToFront * cos(ang);
	else
		p.y += getScale()*distToBack  * cos(ang);
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

//---------------------------------------------------
// isPlacedToFight :
// Return true if the opponent is well placed.
//---------------------------------------------------
bool CCharacterCL::isPlacedToFight(const NLMISC::CVectorD &posAtk, const NLMISC::CVector &dirAtk, double attackerRadius) const	// virtual
{
	NLMISC::CVectorD vDist = pos()-posAtk;
	if(vDist != NLMISC::CVectorD::Null)
	{
		// Attacker Distance
		const double distToAttacker = vDist.norm();
		// Get the Ideal Position
		vDist.normalize();
		CVectorD rightPos = getAttackerPos(atan2(vDist.y, vDist.x), attackerRadius);
		// Vector from the Ideal Position
		NLMISC::CVectorD vDist2 = pos()-rightPos;
		// Check the Distance.
		if(distToAttacker <= vDist2.norm()+ClientCfg.FightAreaSize)
		{
			// Check the orientation.
			NLMISC::CVector vAng = dirAtk;
			vAng.z = 0.0f;
			vDist.z = 0.0;
			return (fabs(angleBetween2Vect(vAng, vDist)) <=  NLMISC::Pi/3.0);
		}
	}
	// User is on the target, do not check dist or angle
	else
		return true;
	// Something wrong
	return false;
//	NLMISC::CVectorD vDist = pos()-posAtk;
//	const double dist = vDist.norm();
//	double radius;
//	// Get current entity radius
//	if(_Primitive)
//		radius = _Primitive->getRadius();
//	else
//		radius = 0.0;
//	// Attack is possible if not too close or too far.
//	if(dist>=radius && dist<=(radius+ClientCfg.FightAreaSize))
//	{
//		// Check Angle
//		NLMISC::CVector vAng = dirAtk;
//		vAng.z = 0.0f;
//		vDist.z = 0.0;
//		return (fabs(angleBetween2Vect(vAng, vDist)) <=  NLMISC::Pi/3.0);
//	}
//	return false;
}// isPlacedToFight //


//---------------------------------------------------
// \param pos : result given in this variable. Only valid if return 'true'.
// \return bool : 'true' if the 'pos' has been filled.
//---------------------------------------------------
bool CCharacterCL::getNamePos(CVectorD &pos) // virtual
{
	// If there is no skeleton -> cannot display the name.
	if(_Skeleton.empty())
		return false;

	// If the entity was not displayed last frame (clipped) -> do not display the name.
	if(!_Skeleton.getLastClippedState())
		return false;

	if(_NameBoneId == -1)
		return false;

	// Take x and y in pos() else we a have a frame late.
	pos.x = this->pos().x;
	pos.y = this->pos().y;

	float namePosZ;
	if (ClientCfg.StaticNameHeight)
		namePosZ = getNamePosZ();
	else
		namePosZ = 0.f;

	// use bone position if no name position is given
	if (namePosZ == 0.f)
	{
		// if displayed as lod, the NameId bone may not be computed
		float skeletonZ = _Skeleton.getLastWorldMatrixComputed().getPos().z;
		if(_Skeleton.isDisplayedAsLodCharacter())
		{
			// if never computed
			if(_NameCLodDeltaZ==NameCLodDeltaZNotComputed)
			{
				_Skeleton.forceComputeBone(_NameBoneId);
				float	boneZ= _Skeleton.getBone(_NameBoneId).getLastWorldMatrixComputed().getPos().z;
				_NameCLodDeltaZ= boneZ - skeletonZ;
			}
			pos.z= skeletonZ + _NameCLodDeltaZ;
		}
		else
		{
			float	boneZ= _Skeleton.getBone(_NameBoneId).getLastWorldMatrixComputed().getPos().z;
			pos.z = boneZ;
			// update delta Z, for when enter in CLod form
			_NameCLodDeltaZ= boneZ - skeletonZ;
		}

		// reset name pos history
		if (_NamePosHistory.isInitialized())
		{
			_NamePosHistory.LastNamePosZ = 0.f;
			_NamePosHistory.LastBonePosZ = 0.f;
		}

		return true;
	}

	const float baseZ = float( this->pos().z );

	// if displayed as lod, skip smooth transition stuff
	if (_Skeleton.isDisplayedAsLodCharacter())
	{
		pos.z = baseZ + namePosZ;

		// reset name pos history
		if (_NamePosHistory.isInitialized())
		{
			_NamePosHistory.LastNamePosZ = 0.f;
			_NamePosHistory.LastBonePosZ = 0.f;
		}

		return true;
	}

	const float boneZ = _Skeleton.getBone(_NameBoneId).getLastWorldMatrixComputed().getPos().z;

	float deltaNamePosZ;
	float deltaBonePosZ;
	if (_NamePosHistory.isInitialized())
	{
		deltaNamePosZ = namePosZ - _NamePosHistory.LastNamePosZ;
		deltaBonePosZ = (boneZ - baseZ) - _NamePosHistory.LastBonePosZ;
	}
	else
	{
		deltaNamePosZ = 0.f;
		deltaBonePosZ = 0.f;
	}

	if (deltaNamePosZ != 0.f)
	{
		// generate a smooth transition following the bone movement
		if (deltaBonePosZ != 0.f && (deltaBonePosZ > 0.f) == (deltaNamePosZ > 0.f))
		{
			namePosZ = _NamePosHistory.LastNamePosZ + deltaBonePosZ;
		}
		else
		{
			const float defaultSpeed = 1.f; // in meters per sec.
			float deltaZ = defaultSpeed * DT;
			if (deltaNamePosZ < 0.f)
				deltaZ = -deltaZ;

			if ( fabs(deltaZ) < fabs(deltaNamePosZ) )
				namePosZ = _NamePosHistory.LastNamePosZ + deltaZ;
		}
	}

	pos.z = baseZ + namePosZ;

	// update history
	_NamePosHistory.LastNamePosZ = namePosZ;
	_NamePosHistory.LastBonePosZ = boneZ - baseZ;

	return true;
}// getNamePos //

//---------------------------------------------------
// Return name position on Z axis defined in sheet
//---------------------------------------------------
float CCharacterCL::getNamePosZ() const
{
	if (!_Sheet)
		return 0.f;

	float namePosZ;
	switch (_ModeWanted)
	{
	case MBEHAV::DEATH:
	case MBEHAV::SIT:
	case MBEHAV::REST:
		namePosZ = _Sheet->NamePosZLow;
		break;

	case MBEHAV::MOUNT_NORMAL:
	case MBEHAV::MOUNT_SWIM:
		namePosZ = _Sheet->NamePosZHigh;
		break;

	default:
		namePosZ = _Sheet->NamePosZNormal;
		break;
	}

	if (namePosZ == 0.f)
		namePosZ = _Sheet->NamePosZNormal;

	return namePosZ * getScale();
}// getNamePosZ //

//---------------------------------------------------
// \param pos : result given in this variable. Only valid if return 'true'.
// \return bool : 'true' if the 'pos' has been filled.
//---------------------------------------------------
bool CCharacterCL::getChestPos(CVectorD &pos) const	// virtual
{
	// If there is no skeleton -> cannot display the Chest.
	if(_Skeleton.empty())
		return false;

	// If the entity was not displayed last frame (clipped) -> do not display the Chest.
	if(!_Skeleton.getLastClippedState())
		return false;

	if(_ChestBoneId == -1)
		return false;

	// Take x and y in pos() else we a have a frame late.
	pos.x = this->pos().x;
	pos.y = this->pos().y;
	pos.z = _Skeleton.getBone(_ChestBoneId).getLastWorldMatrixComputed().getPos().z;
	return true;
}// getChestPos //


//---------------------------------------------------
// getSheetScale :
// Return the entity sheet scale. (return 1.0 if there is any problem).
//---------------------------------------------------
float CCharacterCL::getSheetScale() const	// virtual
{
	if(!_Sheet)
		return 1.f;
	else
		return _Sheet->Scale;

} // getSheetScale //


//---------------------------------------------------
// getColRadius :
// Return the entity collision radius. (return 0.5 if there is any problem).
//---------------------------------------------------
float CCharacterCL::getSheetColRadius() const 
{
	if(!_Sheet) 
		return 0.5f;
	else
		return _Sheet->ColRadius;
}
	

//---------------------------------------------------
// getScale :
// Return the entity scale. (return 1.0 if there is any problem).
//---------------------------------------------------
float CCharacterCL::getScale() const	// virtual
{
	switch( _OwnerPeople )
	{
		case MOUNT_PEOPLE::Fyros : return getSheetScale() * ClientCfg.FyrosScale * _CustomScale;
		case MOUNT_PEOPLE::Matis : return getSheetScale() * ClientCfg.MatisScale * _CustomScale;
		case MOUNT_PEOPLE::Tryker : return getSheetScale() * ClientCfg.TrykerScale * _CustomScale;
		case MOUNT_PEOPLE::Zorai : return getSheetScale() * ClientCfg.ZoraiScale * _CustomScale;
		default:
			return getSheetScale() * _CustomScale;
	}
}// getScale //





///////////
// DEBUG //
///////////
//---------------------------------------------------
// currentAnimationName :
// Return the current animation name.
//---------------------------------------------------
const std::string &CCharacterCL::currentAnimationName() const
{
	if(_PlayList)
	{
		uint idCurrentAnimation = _PlayList->getAnimation(MOVE);
		if(idCurrentAnimation != UPlayList::empty)
			if(EAM && EAM->getAnimationSet())
				return EAM->getAnimationSet()->getAnimationName(idCurrentAnimation);
	}

	// No animation yet.
	return CCharacterCL::_EmptyString;
}// currentAnimationName //

//---------------------------------------------------
// currentAnimationSetName :
// Return the current animation set name.
//---------------------------------------------------
std::string CCharacterCL::currentAnimationSetName(TAnimationType animType) const
{
	if( animType < animTypeCount )
	{
		if( uint(animType) < _CurrentAnimSet.size() )
		{
			if( _CurrentAnimSet[animType] )
			{
				return _CurrentAnimSet[animType]->getSheetName();
			}
		}
	}
	return CCharacterCL::_EmptyString;
}// currentAnimationSetName //

/////////////
// PRIVATE //
//---------------------------------------------------
// shapeFromItem :
// Return the shape pointer from tha item and according to the character gender.
// \param itemSheet : reference on the item sheet.
// \return string & : reference on the shape name.
//---------------------------------------------------
std::string CCharacterCL::shapeFromItem(const CItemSheet &itemSheet) const
{
	if(_Gender == GSGENDER::female && !itemSheet.getShapeFemale().empty())
		return itemSheet.getShapeFemale();
	else
		return itemSheet.getShape();
}// shapeFromItem //


//---------------------------------------------------
// createItemInstance :
// Create the instance from an item
//---------------------------------------------------
uint32 CCharacterCL::createItemInstance(const CItemSheet &itemSheet, uint32 instIdx, SLOTTYPE::EVisualSlot visualSlot, const string &bindBone, sint8 texture, sint color)
{
	uint32 idx = CEntityCL::BadIndex;
	// Get the right shape according to the gender of the character.
	const string &shape = shapeFromItem(itemSheet);

	// Check the shape.
	if(!shape.empty())
	{
		// Check the item need a shape.
		if(shape != "none.shape")
		{
			UInstance	instance;
			// Get the instance
			idx = addColoredInstance(shape, bindBone, texture, instIdx, color);
			SInstanceCL *pInst = idx2Inst(idx);
			nlassert( (pInst == NULL) || (pInst != NULL && !pInst->Loading.empty()) );
			if (pInst != NULL)
				instance = pInst->Loading;
			// Check the shape creation has been is well done.
			if(!instance.empty())
			{
				// Create the FX associated to the item in a given visual slot.
				_Items[visualSlot].initFXs(visualSlot, instance);
			}
			else
				nlwarning("CH:createItemInstance: cannot create the instance for the shape '%s'.", shape.c_str());
		}
	}
	else
		nlwarning("CH:createItemInstance: the item has no shape.");

	return idx;
}// createItemInstance //


//-----------------------------------------------
// setAlive :
// Method to Flag the character as alive and do everything needed.
//-----------------------------------------------
void CCharacterCL::setAlive()	// virtual
{
}// setAlive //



//---------------------------------------------------
// displayDebug :
// Display Debug Information.
//---------------------------------------------------
ADD_METHOD(void CCharacterCL::displayDebug(float x, float &y, float lineStep))	// virtual
	CInterfaceManager *IM = CInterfaceManager::getInstance ();

	CEntityCL::displayDebug(x, y, lineStep);
	// Mode Wanted
	// Display the Target Mode.
	TextContext->printfAt(x, y, "Mode Wanted: %d(%s)", (sint)_ModeWanted, MBEHAV::modeToString(_ModeWanted).c_str());
	y += lineStep;
	// Stage Remaining
	TextContext->printfAt(x, y, "Stages remaining: %d", _Stages._StageSet.size());
	y += lineStep;
	// Current Automaton
	TextContext->printfAt(x, y, "Automaton: %s", _CurrentAutomaton.c_str());
	y += lineStep;
	// Current Speed
	TextContext->printfAt(x, y, "Speed: %f (_DestTime(%f) - _LastFrameTime(%f)) = %f", speed(), _DestTime, _LastFrameTime, _DestTime-_LastFrameTime);
	y += lineStep;
	// Display the Run Factor.
	TextContext->printfAt(x, y, "(Walk)Run Factor: %f", runFactor());
	y += lineStep;
	// Display the current animation name(id)(offset)(nbloop) pour le channel MOVE.
	TextContext->printfAt(x, y, "Current Animation: %s(%u)(%lf)(%u loops)", animId(MOVE)==std::numeric_limits<uint>::max()?"[NONE]":currentAnimationName().c_str(), animId(MOVE), animOffset(MOVE), _NbLoopAnim);
	y += lineStep;
	// First Pos
	if(_First_Pos)
		TextContext->printfAt(x, y, "No Position Received", _First_Pos);
	else
		TextContext->printfAt(x, y, "At least 1 Position Received", _First_Pos);
	y += lineStep;
	// Primitive Ptr
	TextContext->printfAt(x, y, "Prim Ptr: %p", _Primitive);
	y += lineStep;
	// Primitive Position
	if(_Primitive)
	{
		CVectorD primFinalPos = _Primitive->getFinalPosition(dynamicWI);
		TextContext->printfAt(x, y, "Prim Pos: %f %f %f", primFinalPos.x, primFinalPos.y, primFinalPos.z);
		y += lineStep;
	}
	// Skeleton Ptr
	TextContext->printfAt(x, y, "Skel Ptr: %p", &_Skeleton);
	y += lineStep;
	// Animset Ptr
	TextContext->printfAt(x, y, "AnimSet Ptr: %p", _CurrentAnimSet[MOVE]);
	y += lineStep;
	// Current State Ptr
	TextContext->printfAt(x, y, "State Ptr: %p", _CurrentState);
	y += lineStep;
	// Display the target mount and rider.
	TextContext->printfAt(x, y, "Mount: %3u(Theoretical: %3u) Rider: %3u(Theoretical: %3u)", mount(), _TheoreticalMount, rider(), _TheoreticalRider);
	y += lineStep;
	// VPA
	sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", _Slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->getValue64();
	if(isPlayer() || isUser())
	{
		SPropVisualA visualA = *(SPropVisualA *)(&prop);
		TextContext->printfAt(x, y, "VPA: %"NL_I64"X : Chest(%d,%d) Legs(%d,%d) Arms(%d,%d) Hat(%d,%d) RH(%d) LH(%d)", prop,
			(uint)visualA.PropertySubData.JacketModel,	(uint)visualA.PropertySubData.JacketColor,
			(uint)visualA.PropertySubData.TrouserModel,	(uint)visualA.PropertySubData.TrouserColor,
			(uint)visualA.PropertySubData.ArmModel,		(uint)visualA.PropertySubData.ArmColor,
			(uint)visualA.PropertySubData.HatModel,		(uint)visualA.PropertySubData.HatColor,
			(uint)visualA.PropertySubData.WeaponRightHand,
			(uint)visualA.PropertySubData.WeaponLeftHand);
	}
	else
		TextContext->printfAt(x, y, "VPA: %"NL_I64"X", prop);
	y += lineStep;
	// VPB
	prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", _Slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->getValue64();
	if(isPlayer() || isUser())
	{
		SPropVisualB visualB = *(SPropVisualB *)(&prop);
		TextContext->printfAt(x, y, "VPB: %"NL_I64"X : Hands(%d,%d) Feet(%d,%d).", prop,
			(uint)visualB.PropertySubData.HandsModel,	(uint)visualB.PropertySubData.HandsColor,
			(uint)visualB.PropertySubData.FeetModel,	(uint)visualB.PropertySubData.FeetColor);
	}
	else
		TextContext->printfAt(x, y, "VPB: %"NL_I64"X", prop);
	y += lineStep;
	// VPC
	prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", _Slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPC))->getValue64();
	if(isPlayer() || isUser())
	{
		SPropVisualC visualC = *(SPropVisualC *)(&prop);
		TextContext->printfAt(x, y, "VPC: %"NL_I64"X : EyesColor(%d) Tattoo(%d).", prop, visualC.PropertySubData.EyesColor, visualC.PropertySubData.Tattoo);
	}
	else
		TextContext->printfAt(x, y, "VPC: %"NL_I64"X", prop);
	y += lineStep;
}// displayDebug //



//-----------------------------------------------
// displayDebugPropertyStages
//-----------------------------------------------
void CCharacterCL::displayDebugPropertyStages(float x, float &y, float lineStep)
{
	CStageSet::TStageSet::iterator	it= _Stages._StageSet.begin();
	for(;it!=_Stages._StageSet.end();it++)
	{
		CStage &stage= it->second;
		uint32	gc= it->first % 100;
		// build the string of props present in this stage
		string	strProps;
		for(uint i=0;i<CLFECOMMON::NB_VISUAL_PROPERTIES;i++)
		{
			if(i!=CLFECOMMON::PROPERTY_POSY && i!=CLFECOMMON::PROPERTY_POSZ && stage.isPresent(i))
				strProps+= string(CLFECOMMON::getPropShortText(i)) + " ";
		}
		TextContext->printfAt(x, y, "%02d %s", gc, strProps.c_str());
		y += lineStep;
	}
}


//---------------------------------------------------
// readWrite :
// Read/Write Variables from/to the stream.
//---------------------------------------------------
void CCharacterCL::readWrite(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CEntityCL::readWrite(f);

	// PUBLIC

	// PROTECTED
	f.serial(_Stages);
//	std::vector<CAnimation::TAnimId>			_Animations;
//	std::vector<NLSOUND::TSoundAnimId>			_SoundId;
//	NLSOUND::CSoundContext						_SoundContext;
//	std::vector<double>							_AnimationsTimeOffset;
//	std::vector<CAnimationState::TAnimStateId>	_AnimationsStateKey;
//	const CAnimationSet							*_CurrentAnimSet;
	f.serial(_LastFrameTime);
	f.serial(_LodCharacterAnimEnabled);
	f.serial(_LodCharacterAnimTimeOffset);
//	uint										_LodCharacterMasterAnimSlot;
	f.serial(_CharacterScalePos);
	f.serial(_FirstPos);
	f.serial(_FirstTime);
	f.serial(_DistToFirst);
	f.serial(_DestPos);
	f.serial(_DestTime);
	f.serial(_DistToDest);
	f.serial(_OldPos);
	f.serial(_OldPosTime);
//	GSGENDER::EGender							_Gender;
//	sint										_NameBoneId;
//	NL3D::UTransform							_NameTransform;
//	std::vector<CItemSheet *>					_Items;
	f.serial(_HeadIdx);
	f.serial(_FaceIdx);
//	sint	_HeadBoneId;
	f.serial(_RotationFactor);
	f.serial(_DirEndAnim);
	f.serial(_RotAngle);
	f.serial(_CurrentAutomaton);
//	const CAutomatonStateSheet					*_CurrentState;
//	MBEHAV::EMode								_ModeWanted;
//	sint										_BlendRemaining;
	f.serial(_OldAutomaton);
	f.serial(_OldRotQuat);
	f.serial(_CustomScalePos);
//	TTime										_NextBlinkTime;
	f.serial(_NbLoopAnim);
//	std::vector<CFXStruct *>					_FXs;
//	std::list<UParticleSystemInstance>			_CurrentAnimFXList;
//	std::list<UParticleSystemInstance>			_RemoveAnimFXList;
//	NL3D::UParticleSystemInstance				_CurrentAnimFX;
	f.serial(_RightFXActivated);
	f.serial(_LeftFXActivated);
//	sint										_IndexRightFX;
//	sint										_IndexLeftFX;
	f.serial(_Mount);
	f.serial(_Rider);
	f.serial(_IsThereAMode);
	f.serial(_HairColor);
	f.serial(_EyesColor);
	f.serial(_HairIndex);
	f.serial(_LookRdy);
	f.serial(_Speed);
	f.serial(_RunFactor);

	// PRIVATE
//	uint32						_RHandInstIdx;
//	uint32						_LHandInstIdx;
}// readWrite //

//---------------------------------------------------
// load :
// To call after a read from a stream to re-initialize the entity.
//---------------------------------------------------
void CCharacterCL::load()	// virtual
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

	if(_LookRdy)
	{
		_LookRdy = false;
		// Visual properties A
		_HeadIdx = CEntityCL::BadIndex;
		sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", _Slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->getValue64();
		updateVisualPropertyVpa(0, prop);
	}
}// load //

//---------------------------------------------------
// buildPlaylist :
//---------------------------------------------------
void CCharacterCL::buildPlaylist()
{
	computeAnimSet();
	// Release the old animation playlist.
	if(_PlayList)
	{
		EAM->deletePlayList(_PlayList);
		_PlayList = 0;
	}
	// Create the new animation playlist.
	_PlayList = EAM->createPlayList();
	if(!_PlayList)
	{
		nlwarning("Cannot create a playlist for the entity.");
		return;
	}
	// Register the skeleton to the playlist.
	_PlayList->registerTransform(_Skeleton);
	// Animation should not move alone.
	uint posChannel = EAM->getAnimationSet()->getChannelIdByName("pos");
	if(posChannel != NL3D::UAnimationSet::NotFound)
		_PlayList->enableChannel(posChannel, false);
	else
		nlwarning("Channel 'pos' not found.");
	// Animation should not rotate alone.
	uint rotquatChannel = EAM->getAnimationSet()->getChannelIdByName("rotquat");
	if(rotquatChannel != NL3D::UAnimationSet::NotFound)
		_PlayList->enableChannel(rotquatChannel, false);
	else
		nlwarning("Channel 'rotquat' not found.");
	// Initialize the new playlist.
	// MOVE Channel
	_PlayList->setSpeedFactor	(MOVE, 1.f);
	_PlayList->setWrapMode		(MOVE, NL3D::UPlayList::Clamp);
	// ACTION Channel
	_PlayList->setAnimation     (ACTION, NL3D::UPlayList::empty);
	_PlayList->setSpeedFactor	(ACTION, 1.f);
	_PlayList->setWrapMode		(ACTION, NL3D::UPlayList::Clamp);
	// Compute the current animation state.
	_CurrentState = EAM->mState(_CurrentAutomaton, animState(MOVE));
	if(_CurrentState == 0)
	{
		_PlayList->setAnimation(MOVE, NL3D::UPlayList::empty);
		return;
	}
	// Get the right animation state and choose an animation.
	{
		// Get the animation state
		const CAnimationState *animationState = 0;
		if(animState(MOVE) == CAnimationStateSheet::Emote)
			animationState = _CurrentAnimSet[MOVE]->getAnimationState(_SubStateKey);
		else
			animationState = _CurrentAnimSet[MOVE]->getAnimationState(animState(MOVE));
		if(animationState)
		{
			// Choose the animation
			animIndex(MOVE, animationState->chooseAnim(_AnimJobSpecialisation, people(), getGender(), 0.0));
			// Should the objects in hands be displayed ?
			_ObjectsVisible = animationState->areObjectsVisible();
		}
	}
	// Set animation
	_PlayList->setAnimation(MOVE, animId(MOVE));
}// buildPlaylist //

//--------------//
// ENTITY INFOS //
//--------------//
//-----------------------------------------------
// getSpeed :
// Return the entity speed
//-----------------------------------------------
double CCharacterCL::getSpeed() const	// virtual
{
	return speed();
}// getSpeed //

//-----------------------------------------------
// dist2FirstPos :
// Set the Distance from the current entity position to the First Position.
//-----------------------------------------------
void CCharacterCL::dist2FirstPos(double d2FP)
{
	CHECK((d2FP == INVALID_DIST) || (d2FP == 0.0) || (d2FP >= ClientCfg.DestThreshold));
	_DistToFirst = d2FP;
}// dist2FirstPos //
//-----------------------------------------------
// dist2FirstPos :
// Return the Distance from the current entity position to the First Position.
//-----------------------------------------------
double CCharacterCL::dist2FirstPos() const
{
	CHECK((_DistToFirst == INVALID_DIST) || (_DistToFirst == 0.0) || (_DistToFirst >= ClientCfg.DestThreshold));
	return _DistToFirst;
}// dist2FirstPos //

//-----------------------------------------------
// dist2Dest :
// Set the Distance from the current entity position to the Destination.
//-----------------------------------------------
void CCharacterCL::dist2Dest(double d2D)
{
	CHECK((d2D == INVALID_DIST) || (d2D == 0.0) || (d2D >= ClientCfg.DestThreshold));
	_DistToDest = d2D;
}// dist2Dest //

//-----------------------------------------------
// dist2Dest :
// Return the Distance from the current entity position to the Destination.
//-----------------------------------------------
double CCharacterCL::dist2Dest() const
{
	CHECK((_DistToDest == INVALID_DIST) || (_DistToDest == 0.0) || (_DistToDest >= ClientCfg.DestThreshold));
	return _DistToDest;
}// dist2Dest //

//-----------------------------------------------
// speed :
// Set the Entity Current Speed.
//-----------------------------------------------
void CCharacterCL::speed(double s)
{
	CHECK((s == -1.0) || (s == 0.0) || ((s >= 0.001) && (s <= 1000.0)));
	_Speed = s;
}// speed //

//-----------------------------------------------
// speed :
// Return the Entity Current Speed.
//-----------------------------------------------
double CCharacterCL::speed() const	// virtual
{
	CHECK((_Speed == -1.0) || (_Speed == 0.0) || ((_Speed >= 0.001) && (_Speed <= 1000.0)));
	return _Speed;
}// getSpeed //

//-----------------------------------------------
// Build the in scene interface
//-----------------------------------------------
void CCharacterCL::buildInSceneInterface ()
{
	// Delete previous interface
	releaseInSceneInterfaces();

	_InSceneUserInterface = CGroupInSceneUserInfo::build (this);

	// parent
	CEntityCL::buildInSceneInterface();
}

//-----------------------------------------------

void CCharacterCL::setBubble (CGroupInSceneBubble *bubble)
{
	if (_CurrentBubble != NULL)
	{
		CGroupInSceneBubble	*old = _CurrentBubble;
		_CurrentBubble = NULL;
		old->unlink();
	}
	nlassert (_CurrentBubble == NULL);
	_CurrentBubble = bubble;
}

//-----------------------------------------------

//-----------------------------------------------
// runFactor :
// Set the Factor between Walk & Run
//-----------------------------------------------
void CCharacterCL::runFactor(double factor)
{
	CHECK((factor >= 0.0) && (factor <= 1.0));
	CHECK((factor == 0.0) || (animState(MOVE) == CAnimationStateSheet::Walk));
	_RunFactor = factor;
}// runFactor //

//-----------------------------------------------
// runFactor :
// Get the Factor between Walk & Run
//-----------------------------------------------
double CCharacterCL::runFactor() const
{
	CHECK((_RunFactor >= 0.0) && (_RunFactor <= 1.0));
	CHECK((_RunFactor == 0.0) || (animState(MOVE) == CAnimationStateSheet::Walk));
	return _RunFactor;
}// runFactor //

//-----------------------------------------------
// animOffset :
// Set the animation time offset for an animation channel.
//-----------------------------------------------
void CCharacterCL::animOffset(TAnimationType channel, double timeOffset)
{
	if(ClientCfg.Light)
		return;
	// Check the channel
	CHECK((uint)channel < _AnimOffset.size());
	// Check Animation Time Offset is greater or equal to 0.
	CHECK(timeOffset >= 0.0);
	// Check the animation time offset is not greater to the animation length.
	CHECK(timeOffset <= EAM->getAnimationLength(animId(channel)));
	// Set the Value
	_AnimOffset[channel] = timeOffset;
}// animOffset //

//-----------------------------------------------
// animOffset :
// Return the animation time offset for an animation channel.
//-----------------------------------------------
double CCharacterCL::animOffset(TAnimationType channel) const
{
	if(ClientCfg.Light)
		return 0.0;
	// Check the channel
	CHECK((uint)channel < _AnimOffset.size());
	// Check Animation Time Offset is greater or equal to 0.
	CHECK(_AnimOffset[channel] >= 0.0);
	// Check the animation time offset is not greater to the animation length.
	CHECK(_AnimOffset[channel] <= EAM->getAnimationLength(animId(channel)));
	// Return the Value
	return _AnimOffset[channel];
}// animOffset //

//---------------------------------------------------
// animationStateKey :
// Set the animation state key.
// \todo GUIGUI : a remplacer par animState directement.
//---------------------------------------------------
bool CCharacterCL::animationStateKey(TAnimationType channel, TAnimStateId value)
{
	// Is the new key valid ?
	if(value == CAnimationStateSheet::UnknownState)
	{
		nlwarning("CH::animationStateKey: Char '%d': new state key is Null.", _Slot);
		return false;
	}
	// Set the new key.
	animState(channel, value);
	// Debug Animation for the selection
	if(VerboseAnimSelection && _Slot == UserEntity->selection())
		nlinfo("CH:animationStateKey:%d: state '%s'.", _Slot, CAnimationState::getAnimationStateName(value).c_str());
	//
	return true;
}// animationStateKey //

//-----------------------------------------------
// animState :
// Set the Animation 'State' for an animation channel.
//-----------------------------------------------
void CCharacterCL::animState(TAnimationType channel, TAnimStateId state)
{
	// Check the channel
	CHECK((uint)channel < _AnimState.size());
	// Set the new State
	_AnimState[channel] = state;
	// Reset the Run Factor when the state change.
	runFactor(0.0);
}// animState //

//-----------------------------------------------
// animState :
// Get the Animation 'State' for an animation channel.
//-----------------------------------------------
TAnimStateId CCharacterCL::animState(TAnimationType channel) const
{
	// Check the channel
	CHECK((uint)channel < _AnimState.size());
	// Return the Animation State
	return _AnimState[channel];
}// animState //

//-----------------------------------------------
// animIndex :
// Set the Animation 'Index' in the 'State' for an animation channel.
//-----------------------------------------------
void CCharacterCL::animIndex(TAnimationType channel, CAnimation::TAnimId index)
{
	// Check the channel
	CHECK((uint)channel < _AnimState.size());
	// Set the animation index in the state
	_AnimIndex[channel] = index;
	// Set the animation Id
	// If the current animation Index is not a valid one, return empty
	if(_AnimIndex[channel] == CAnimation::UnknownAnim)
		animId(channel, NL3D::UPlayList::empty);
	else
	{
		// Check the AnimSet needed to get the animation Id.
		CHECK(_CurrentAnimSet[channel]);
		// Get the Pointer on the animation state, if Null, return empty
		const CAnimationState *animStatePtr = _CurrentAnimSet[channel]->getAnimationState( (animState(channel)==CAnimationStateSheet::Emote)?_SubStateKey:animState(channel));
		if(animStatePtr == 0)
			animId(channel, NL3D::UPlayList::empty);
		else
		{
			// Get the Animation Pointer, if Null, return Empty
			const CAnimation *anim = animStatePtr->getAnimation(animIndex(channel));
			if(anim == 0)
				animId(channel, NL3D::UPlayList::empty);
			// Return The Animation ID
			else
				animId(channel, anim->id());
		}
	}
}// animIndex //

//-----------------------------------------------
// animIndex :
// Get the Animation 'Index' in the 'State' for an animation channel.
//-----------------------------------------------
CAnimation::TAnimId CCharacterCL::animIndex(TAnimationType channel) const
{
	// Check the channel
	CHECK((uint)channel < _AnimState.size());
	// Return the Animation Index in the State
	return _AnimIndex[channel];
}//animIndex //

//-----------------------------------------------
// animId :
// Set the Animation 'Id' among all the animations for an animation channel.
//-----------------------------------------------
void CCharacterCL::animId(TAnimationType channel, uint id)
{
	// Check the channel
	CHECK((uint)channel < _AnimId.size());
	// Set the Id
	_AnimId[channel] = id;
}// animId //

//-----------------------------------------------
// animId :
// Get the Animation 'Id' among all the animations for an animation channel.
//-----------------------------------------------
uint CCharacterCL::animId(TAnimationType channel) const
{
	// Check the channel
	CHECK((uint)channel < _AnimId.size());
	// Get the Id
	return _AnimId[channel];
}// animId //



//-----------------------------------------------
// Align the given FX so that it is oriented like that entity
//-----------------------------------------------
void CCharacterCL::alignFX(UParticleSystemInstance instance, float scale /* = 1.f */, const NLMISC::CVector &localOffset /*= NLMISC::CVector::Null*/) const
{
	// copy matrix from parent
	CMatrix fxMatrix;
	fxMatrix.identity();
	buildAlignMatrix(fxMatrix);
	alignFX(instance, fxMatrix, scale, localOffset);
}


void CCharacterCL::alignFX(UParticleSystemInstance instance, const CMatrix &matrix, float scale /*=1.f*/, const NLMISC::CVector &localOffset /*=NLMISC::CVector::Null*/) const
{
	if(instance.empty())
		return;
	CMatrix fxMatrix = matrix;
	if (scale != 1.f) fxMatrix.scale(scale);
	fxMatrix.setPos(fxMatrix.getPos() + fxMatrix.mulVector(localOffset));
	instance.setTransformMode(NL3D::UTransform::DirectMatrix);
	instance.setMatrix(fxMatrix);
	if(!_Skeleton.empty())
	{
		instance.setClusterSystem(_Skeleton.getClusterSystem());
	}
}


//------------------------------------------------------------
// build a matrix aligned on that entity (includes dir & pos)
//------------------------------------------------------------
void CCharacterCL::buildAlignMatrix(NLMISC::CMatrix &dest) const
{
	// if not in clod, pelvis bone has been computed -> use it to get the current orientation
	// use the dir matrix otherwise
	/*CVector forward;
	if (pelvisBone() != -1 && _Skeleton)
	{
		if (_Skeleton.isBoneComputed(pelvisBone()))
		{
			// the direction is given by the y axis
			NL3D::UBone pelvisBone = _Skeleton.getBone(pelvisBone());
			forward = pelvisBone->getMatrix().getJ();
			forward.z = 0.f; // project onto XY plane
			forward.normalize();
		}
		else
		{
			// must be in clod -> use the direction
			forward = _Front;
		}
	}
	else
	{
		// bone not found, use the direction
		forward = _Front;
	}
	dest.setRot(- forward, forward ^ CVector::K, CVector::K);
	*/
	dest.setRot(CVector::K ^ _Front, - _Front, CVector::K);
	dest.setPos(pos());
}


//-----------------------------------------------
// attachFXInternal
//-----------------------------------------------
void	CCharacterCL::attachFXInternal(const CAttachedFX::TSmartPtr fx, TAttachedFXList targetList)
{
	if (!fx) return;
	switch(targetList)
	{
		case FXListToRemove:
			fx->TimeOutDate += TimeInSec; // in remove list timeout date is absolute
			_AttachedFXListToRemove.push_front(fx);
		break;
		case FXListCurrentAnim:
			_AttachedFXListForCurrentAnim.push_front(fx);
		break;
		case FXListAuto:
		{
			if (fx->AniFX->Sheet)
			{
				switch(fx->AniFX->Sheet->RepeatMode)
				{
					case CAnimationFXSheet::Respawn:
						fx->TimeOutDate += TimeInSec; // in remove list timeout date is absolute
						_AttachedFXListToRemove.push_front(fx);
					break;
					default:
						_AttachedFXListForCurrentAnim.push_front(fx);
					break;
				}
			}
			else
			{
				fx->TimeOutDate += TimeInSec; // in remove list timeout date is absolute
				_AttachedFXListToRemove.push_front(fx);
			}
		}
		break;
		default:
			nlassert(0);
		break;
	}
}

//-----------------------------------------------
// attachFX
//-----------------------------------------------
void	CCharacterCL::attachFX(const CAttachedFX::TSmartPtr fx)
{
	attachFXInternal(fx, FXListToRemove);
}





// ***********************************************************************************************************************
void CCharacterCL::setAuraFX(uint index, const CAnimationFX *sheet)
{
	nlassert(index < MaxNumAura);
	// no-op if same aura
	if (_AuraFX[index] && _AuraFX[index]->AniFX == sheet) return;

	if (sheet == NULL)
	{
		std::list<CAttachedFX::CBuildInfo>::iterator itAttachedFxToStart = _AttachedFXListToStart.begin();
		while(itAttachedFxToStart != _AttachedFXListToStart.end())
		{
			if ((*itAttachedFxToStart).MaxNumAnimCount == index)
				itAttachedFxToStart = _AttachedFXListToStart.erase(itAttachedFxToStart);
			else
				++itAttachedFxToStart;
		}
		// if there's already an aura attached, and if it is not already shutting down
		if (_AuraFX[index] && _AuraFX[index]->TimeOutDate == 0.f)
		{
			_AuraFX[index]->TimeOutDate = TimeInSec + AURA_SHUTDOWN_TIME;
		}
	}
	else
	{
		std::list<CAttachedFX::CBuildInfo>::iterator itAttachedFxToStart = _AttachedFXListToStart.begin();
		while(itAttachedFxToStart != _AttachedFXListToStart.end())
		{
			if ((*itAttachedFxToStart).MaxNumAnimCount == index)
				return;
		}
		// remove previous aura
		_AuraFX[index] = NULL;
		CAttachedFX::CBuildInfo bi;
		bi.Sheet = sheet;
		bi.TimeOut =  0.f;

		if (sheet->Sheet->PSName == "misc_caravan_teleportout.ps")
		{
			bi.MaxNumAnimCount = index;
			bi.StartTime = TimeInSec;
			bi.DelayBeforeStart = 12.5f;
			_AttachedFXListToStart.push_front(bi);
		}
		else if (sheet->Sheet->PSName == "misc_kami_teleportout.ps")
		{
			bi.MaxNumAnimCount = index;
			bi.StartTime = TimeInSec;
			bi.DelayBeforeStart = 11.5f;
			_AttachedFXListToStart.push_front(bi);
		}
		else 
		{
			CAttachedFX::TSmartPtr fx = new CAttachedFX;
			fx->create(*this, bi, CAttachedFX::CTargeterInfo());
			if (!fx->FX.empty())
			{
				_AuraFX[index] = fx;
			}
		}
	}
}

// ***********************************************************************************************************************
void CCharacterCL::setLinkFX(const CAnimationFX *fx, const CAnimationFX *dispell)
{
	// no-op if same link
	if (_LinkFX && _LinkFX->AniFX == fx) return;
	if (_LinkFX)
	{
		if (dispell)
		{
			CAttachedFX::TSmartPtr fx = new CAttachedFX;
			CAttachedFX::CBuildInfo bi;
			bi.Sheet = dispell;
			fx->create(*this, bi, CAttachedFX::CTargeterInfo());
			attachFX(fx);
		}
	}
	_LinkFX = NULL;
	if (!fx) return;
	CAttachedFX::TSmartPtr linkFX = new CAttachedFX;
	CAttachedFX::CBuildInfo bi;
	bi.Sheet = fx;
	linkFX->create(*this, bi, CAttachedFX::CTargeterInfo());
	if (!linkFX->FX.empty())
	{
		_LinkFX = linkFX;
	}
}



// ***********************************************************************************************************************
void CCharacterCL::startItemAttackFXs(bool activateTrails, uint intensity)
{
	uint numItems = (uint)_Items.size();
	forceEvalAnim(); // force to eval bones at least once when fx are created
	for(uint k = 0; k < numItems; ++k)
	{
		_Items[k].startAttackFX(_Skeleton, intensity, (SLOTTYPE::EVisualSlot) k, activateTrails);
	}
}

// ***********************************************************************************************************************
void CCharacterCL::stopItemAttackFXs()
{
	uint numItems = (uint)_Items.size();
	for(uint k = 0; k < numItems; ++k)
	{
		if (_Items[k].Sheet)
		{
			_Items[k].stopAttackFX();
		}
	}
}


//-----------------------------------------------
// isNeutral :
//-----------------------------------------------
bool CCharacterCL::isNeutral() const
{
	return !isEnemy();
}

//-----------------------------------------------
// isFriend :
//-----------------------------------------------
bool CCharacterCL::isFriend () const
{
	return !isEnemy();
}

//-----------------------------------------------
// isEnemy :
//-----------------------------------------------
bool CCharacterCL::isEnemy () const
{
	// Suppose enemy if attackable
	if( properties().attackable() )
	{
		return true;
	}

	return isAnOutpostEnemy();
}


//-----------------------------------------------
// isAlly :
//-----------------------------------------------
bool CCharacterCL::isAlly () const
{
	return this->isAnOutpostAlly();
}


//-----------------------------------------------
// applyVisualFX
//-----------------------------------------------
void CCharacterCL::applyVisualFX(sint64 prop)
{
	CVisualFX vfx;
	vfx.unpack(prop);
	const CAnimationFX *auraFX = NULL;
	if (vfx.Aura != 0)
	{
		auraFX = CAttackListManager::getInstance().getAuras().getFX(vfx.Aura);
	}
	setAuraFX(0, auraFX);
	const CAnimationFX *auraReceiptFX = NULL;
	if (vfx.AuraReceipt)
	{
		auraReceiptFX = CAttackListManager::getInstance().getAuras().getFX(0);
	}
	setAuraFX(1, auraReceiptFX);
	const CAnimationFX *linkFX = NULL;
	if (vfx.Link != 0)
	{
		linkFX = CAttackListManager::getInstance().getLinks().getFX(vfx.Link);
	}
	const CAnimationFX *dispellFX = NULL;
	dispellFX = CAttackListManager::getInstance().getLinks().getFX(0);
	setLinkFX(linkFX, dispellFX);
}

// *********************************************************************************************
const char *CCharacterCL::getBoneNameFromBodyPart(BODY::TBodyPart part, BODY::TSide side) const
{
	if (!_Sheet) return CEntityCL::getBoneNameFromBodyPart(part, side);
	return _Sheet->BodyToBone.getBoneName(part, side);
}


// *********************************************************************************************
const CItemSheet *CCharacterCL::getRightHandItemSheet() const
{
	if (_RHandInstIdx == CEntityCL::BadIndex) return NULL;
	return _Items[_RHandInstIdx].Sheet;
}

// *********************************************************************************************
const CItemSheet *CCharacterCL::getLeftHandItemSheet() const
{
	if (_LHandInstIdx == CEntityCL::BadIndex) return NULL;
	return _Items[_LHandInstIdx].Sheet;
}

// ***************************************************************************
void	CCharacterCL::resetAllSoundAnimId()
{
	for(uint i=0;i<_SoundId.size();i++)
	{
		_SoundId[i]= CSoundAnimationNoId;
	}
}


/////////////////////////////
// CCharacterCL::CWornItem //
/////////////////////////////


// ***********************************************************************************************************************
void CCharacterCL::CWornItem::startAttackFX(NL3D::USkeleton skeleton, uint intensity, SLOTTYPE::EVisualSlot visualSlot, bool activateTrail)
{
	if (intensity < 1 || intensity > 5) return;
	// activate trail
	if (activateTrail && !Trail.empty())
	{
		Trail.start();
	}
	// create the attack fx
	if (Sheet)
	{
		const CItemFXSheet &fxSheet = Sheet->FX;
		std::string shapeName = fxSheet.getAttackFX();
		if (!shapeName.empty())
		{
			const char *stickPoint = NULL;
			if(!skeleton.empty())
			{
				switch(visualSlot)
				{
					case SLOTTYPE::RIGHT_HAND_SLOT:
						if( Sheet->ItemType != ITEM_TYPE::MAGICIAN_STAFF )
							stickPoint = "box_arme";
					break;
					case SLOTTYPE::LEFT_HAND_SLOT:
						if(Sheet && Sheet->getAnimSet()=="s")
							stickPoint = "Box_bouclier";
						else
							stickPoint = "box_arme_gauche";
					break;
					default:
					break;
				}
			}
			if (stickPoint)
			{
				sint boneId = skeleton.getBoneIdByName(std::string(stickPoint));
				if (boneId != -1)
				{
					NL3D::UInstance instance = Scene->createInstance(shapeName);
					if (!instance.empty())
					{
						instance.show();
						if (skeleton.getVisibility() == UTransform::Hide)
						{
							// force to compute the bone at least once
							skeleton.forceComputeBone(boneId);
						}
						UParticleSystemInstance atk;
						atk.cast (instance);
						if (!atk.empty())
						{
							// set the user params
							static const float up[5][4] =
							{
								{ 0.f, 0.f, 0.f, 0.f},
								{ 1.f, 0.f, 0.f, 0.f},
								{ 1.f, 1.f, 0.f, 0.f},
								{ 1.f, 1.f, 1.f, 0.f},
								{ 1.f, 1.f, 1.f, 1.f}
							};
							for(uint k = 0; k < 4; ++k)
							{
								atk.setUserParam(k, up[intensity - 1][k]);
							}
							atk.setTransformMode(UTransform::RotEuler);
							atk.setPos(fxSheet.AttackFXOffset);
							const float degreeToRad = (float) Pi / 180.f;
							atk.setRotEuler(fxSheet.AttackFXRot.x * degreeToRad, fxSheet.AttackFXRot.y * degreeToRad, fxSheet.AttackFXRot.z * degreeToRad);
							skeleton.stickObject(atk, boneId);
							// delegate mng of this object lifetime to the fx manager
							FXMngr.fx2remove(atk);
						}
						else
						{
							Scene->deleteInstance(atk);
						}
					}
				}
			}
		}
	}
}

// ***********************************************************************************************************************
void CCharacterCL::CWornItem::stopAttackFX()
{
	if (!Trail.empty()) Trail.stop();
}

// ***********************************************************************************************************************
void CCharacterCL::CWornItem::initFXs(SLOTTYPE::EVisualSlot /* visualSlot */, NL3D::UInstance parent)
{
	releaseFXs();
	if (!Sheet) return;
	// create trail fx
	const CItemFXSheet &sheet = Sheet->FX;
	std::string shapeName = sheet.getTrail();
	if (!shapeName.empty())
	{
		Trail = Scene->createInstance(shapeName);
		if (Trail.empty())
		{
			nlwarning("Cannot create instance %s", shapeName.c_str());
			return;
		}
		// Initialize the remanence. Must substract the object matrix since parent(parent)
		CMatrix mat = parent.getMatrix();
		mat.invert();
		mat *= Trail.getMatrix();
		Trail.setTransformMode(UTransformable::DirectMatrix);
		Trail.setMatrix(mat);
		// Initialize instance.
		Trail.parent(parent);
	}
}

// ***********************************************************************************************************************
void CCharacterCL::CWornItem::enableAdvantageFX(NL3D::UInstance parent)
{
	if (!Sheet) return;

	bool enabled = Sheet->HasFx;

	if ((enabled && !AdvantageFX.empty()) || (!enabled && AdvantageFX.empty()))
		return; // state did not change
	if (!enabled)
	{
		// well, it is unlikely that player will loses its ability to master an item after he gained it, but manage the case anyway.
		if (!AdvantageFX.removeByID(NELID("STOP")) && !AdvantageFX.removeByID(NELID("main")))
		{
			AdvantageFX.activateEmitters(false);
		}
		FXMngr.fx2remove(AdvantageFX);
		AdvantageFX = NULL;
	}
	else
	{
		std::string shapeName = Sheet->FX.getAdvantageFX();
		if (!shapeName.empty())
		{
			NL3D::UInstance fx = Scene->createInstance(shapeName);
			if (fx.empty()) return;
			AdvantageFX.cast (fx);
			if (AdvantageFX.empty())
			{
				Scene->deleteInstance(fx);
				return;
			}
			CMatrix mat = parent.getMatrix();
			mat.invert();
			mat *= AdvantageFX.getMatrix();
			AdvantageFX.setTransformMode(UTransformable::DirectMatrix);
			AdvantageFX.setMatrix(mat);
			AdvantageFX.parent(parent);
		}
	}
}

// ***********************************************************************************************************************
void CCharacterCL::CWornItem::releaseFXs()
{
	if (Scene)
	{
		if (!Trail.empty())
			Scene->deleteInstance(Trail);
		if (!AdvantageFX.empty())
			Scene->deleteInstance(AdvantageFX);
	}
}

// ***********************************************************************************************************************
void CCharacterCL::CWornItem::setTrailSize(uint size)
{
	if (Trail.empty()) return;
	if (!Sheet) return;
	clamp(size, (uint) 0, (uint) 15);
	if (size == 0)
	{
		Trail.setSliceTime(0.f);
	}
	else
	{
		float ratio = (size - 1) / 15.f;
		Trail.setSliceTime(ratio * Sheet->FX.TrailMaxSliceTime + (1.f - ratio) *  Sheet->FX.TrailMinSliceTime);
	}
}


// ***********************************************************************************************************************
const CAttack *CCharacterCL::getAttack(const CAttackIDSheet &id) const
{
	if (!_Sheet) return NULL;
	return getAttack(id, _Sheet->AttackLists);
}

// ***********************************************************************************************************************
const CAttack *CCharacterCL::getAttack(const CAttackIDSheet &id, const std::vector<NLMISC::TSStringId> &attackList) const
{
	for(std::vector<NLMISC::TSStringId>::const_reverse_iterator it = attackList.rbegin(); it != attackList.rend(); ++it)
	{
		const CAttackList *al = CAttackListManager::getInstance().getAttackList(ClientSheetsStrings.get(*it));
		if (al)
		{
			const CAttack *attk = al->getAttackFromID(id);
			if (attk) return attk;
		}
	}
	return NULL;
}


// ***********************************************************************************************************************
void CCharacterCL::initStaticFX()
{
	_StaticFX = NULL;
	if (ClientCfg.Light) return;
	std::string staticFX = _Sheet->getStaticFX();
	if (!staticFX.empty())
	{
		CEntitySheet *sheet = SheetMngr.get(NLMISC::CSheetId(staticFX));
		if (sheet)
		{
			if (sheet->Type == CEntitySheet::ANIMATION_FX)
			{
				CAnimationFXSheet *afs = NLMISC::safe_cast<CAnimationFXSheet *>(sheet);
				_StaticFX = new CStaticFX;
				_StaticFX->AF.init(afs, EAM ? EAM->getAnimationSet() : NULL);
				_StaticFX->FX = new CAttachedFX;
				CAttachedFX::CBuildInfo bi;
				bi.Sheet = &_StaticFX->AF;
				_StaticFX->FX->create(*this, bi, CAttachedFX::CTargeterInfo());
				if (_StaticFX->FX->FX.empty())
				{
					_StaticFX = NULL;
					return;
				}

			}
		}
		else
		{
			nlwarning("Static fx %s not found", staticFX.c_str());
		}
	}
}

// ***************************************************************************
EGSPD::CPeople::TPeople CCharacterCL::people() const
{
	if(_Sheet)
		return _Sheet->Race;
	else
		return EGSPD::CPeople::Unknown;
}

// ***************************************************************************
void CCharacterCL::setPeople(EGSPD::CPeople::TPeople /* people */)
{
}

// ***************************************************************************

#if !FINAL_VERSION

// temp : begin cast of projectile (start begin anim and loop anim)
NLMISC_COMMAND(beginCast, "Start spell cast", "<spell_id> <strenght> [<mode> <caster slot> <behaviour>]")
{
	if (args.size() < 2) return false;
	if (args.size() > 5) return false;
	uint casterSlot = 0;
	if (args.size() >= 4)
	{
		fromString(args[3], casterSlot);
	}
	CCharacterCL *ch = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(casterSlot));
	if (!ch) return false;
	CBehaviourContext bc;
	// TODO : the type of missile is contained in the spell id
	if (args.size() >= 5)
	{
		sint missileType;
		fromString(args[4], missileType);
		bc.Behav.Behaviour = (MBEHAV::EBehaviour) (MBEHAV::CAST_OFF + missileType);
	}
	else
	{
		bc.Behav.Behaviour = MBEHAV::CAST_OFF;
	}
	//
	uint16 spellId;
	fromString(args[0], spellId);	// spell id is unused
	bc.Behav.Spell.SpellId = spellId;
	//bc.Behav.Spell.Resist = false;
	uint16 spellIntensity;
	fromString(args[1], spellIntensity);
	bc.Behav.Spell.SpellIntensity = spellIntensity;
	if (args.size() == 3)
	{
		uint16 spellMode;
		fromString(args[2], spellMode);
		bc.Behav.Spell.SpellMode = spellMode;
	}
	else
	{
		bc.Behav.Spell.SpellMode = MAGICFX::Bomb; // 'bomb' is the default
	}
	ch->applyBehaviour(bc);
	return true;
}

// temp : test fail of a cast
NLMISC_COMMAND(failCast, "Simulate failure of a spell cast", "<spell_id> <strenght> [<mode>]")
{
	if (args.size() < 2) return false;
	if (args.size() > 3) return false;
	CCharacterCL *ch = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(0));
	if (!ch) return false;
	CBehaviourContext bc;
	// TODO : the type of missile is contained in the spell id
	bc.Behav.Behaviour = MBEHAV::CAST_OFF_FAIL;
	//
	uint16 spellId;
	fromString(args[0], spellId);	// spell id is unused
	bc.Behav.Spell.SpellId = spellId;
	//bc.Behav.Spell.Resist = false;
	uint16 spellIntensity;
	fromString(args[1], spellIntensity);
	bc.Behav.Spell.SpellIntensity = spellIntensity;
	if (args.size() == 3)
	{
		uint16 spellMode;
		fromString(args[2], spellMode);
		bc.Behav.Spell.SpellMode = spellMode;
	}
	else
	{
		bc.Behav.Spell.SpellMode = MAGICFX::Bomb; // 'bomb' is the default
	}
	ch->applyBehaviour(bc);
	return true;
}

// temp to test cast of a projectile on another entity
NLMISC_COMMAND(projectile, "Cast a projectile on another entity", "<spellID> <strenght> [<mode> <target entity> <resist> <source entity>]" )
{
	if (args.size() < 2) return false;
	if (args.size() > 6) return false;
	uint8 casterSlot = 0;
	if (args.size() > 5)
	{
		fromString(args[5], casterSlot);
	}
	CCharacterCL *ch = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(casterSlot));
	if (!ch) return false;
	// create a new behaviour to apply to the user
	CBehaviourContext bc;
	uint16 spellId, spellIntensity;
	fromString(args[0], spellId);
	bc.Behav.Spell.SpellId = spellId;
	fromString(args[1], spellIntensity);
	bc.Behav.Spell.SpellIntensity = spellIntensity;
	// TODO : the type of missile is contained in the spell id
	bc.Behav.Behaviour = MBEHAV::CAST_OFF_SUCCESS;
	//
	if (args.size() > 2)
	{
		uint16 spellMode;
		fromString(args[2], spellMode);
		bc.Behav.Spell.SpellMode = spellMode;
	}
	else
	{
		bc.Behav.Spell.SpellMode = MAGICFX::Bomb; // 'bomb' is the default
	}
	if (args.size() > 3)
	{
		uint targetSlot;
		fromString(args[3], targetSlot);
		if (targetSlot >= CLFECOMMON::INVALID_SLOT) return false;
		CEntityCL *target = EntitiesMngr.entity(targetSlot);
		if (!target) return false;
		double dist = (target->pos() - ch->pos()).norm();
		bool resist = false;
		if (args.size() > 4) fromString(args[4], resist);
		bc.Targets.Targets.push_back(CMultiTarget::CTarget(targetSlot, resist, (uint8) ceilf((float) (dist /  MULTI_TARGET_DISTANCE_UNIT))));
	}
	bc.BehavTime = TimeInSec;
	ch->applyBehaviour(bc);
	return true;
}

NLMISC_COMMAND(mtProjectile, "Cast a projectile on one or several entities", "<caster> <spellID> <strenght> <mode> <target0> [<target n>]*" )
{
	if (args.size() < 5) return false;
	uint8 casterSlot;
	fromString(args[0], casterSlot);
	CCharacterCL *ch = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(casterSlot));
	if (!ch) return false;
	// create a new behaviour to apply to the user
	CBehaviourContext bc;
	uint16 spellId, spellIntensity, spellMode;

	fromString(args[1], spellId);
	bc.Behav.Spell.SpellId = spellId;

	fromString(args[2], spellIntensity);
	bc.Behav.Spell.SpellIntensity = spellIntensity;

	fromString(args[3], spellMode);
	bc.Behav.Spell.SpellMode = spellMode;
	// get targets and their dist depending on the mode
	switch(bc.Behav.Spell.SpellMode)
	{
		case MAGICFX::Bomb:
		{
			uint mainTargetSlot;
			fromString(args[4], mainTargetSlot);
			if (mainTargetSlot >= CLFECOMMON::INVALID_SLOT) return false;
			CEntityCL *mainTarget = EntitiesMngr.entity(mainTargetSlot);
			if (mainTarget)
			{
				double dist = (mainTarget->pos() - ch->pos()).norm();
				bc.Targets.Targets.push_back(CMultiTarget::CTarget(mainTargetSlot, false, (uint8) ceilf((float) (dist /  MULTI_TARGET_DISTANCE_UNIT))));
				for(sint k = 1; k < (sint) (args.size() - 4); ++k)
				{
					uint secondaryTargetSlot;
					fromString(args[4 + k], secondaryTargetSlot);
					if (secondaryTargetSlot >= CLFECOMMON::INVALID_SLOT) return false;
					CEntityCL *secondaryTarget = EntitiesMngr.entity(secondaryTargetSlot);
					if (secondaryTarget)
					{
						dist = (secondaryTarget->pos() - mainTarget->pos()).norm();
						bc.Targets.Targets.push_back(CMultiTarget::CTarget(secondaryTargetSlot, false, (uint8) ceilf((float) (dist /  MULTI_TARGET_DISTANCE_UNIT))));
					}
				}
			}
		}
		break;
		case MAGICFX::Spray:
		{
			for(sint k = 0; k < (sint) (args.size() - 4); ++k)
			{
				uint targetSlot;
				fromString(args[4 + k], targetSlot);
				if (targetSlot >= CLFECOMMON::INVALID_SLOT) return false;
				CEntityCL *target = EntitiesMngr.entity(targetSlot);
				if (target)
				{
					double dist = (target->pos() - ch->pos()).norm();
					bc.Targets.Targets.push_back(CMultiTarget::CTarget(targetSlot, false, (uint8) ceilf((float) (dist /  MULTI_TARGET_DISTANCE_UNIT))));
				}
			}
		}
		break;
		case MAGICFX::Chain:
		{
			CEntityCL *startSlot = ch;
			for(sint k = 0; k < (sint) (args.size() - 4); ++k)
			{
				uint targetSlot;
				fromString(args[4 + k], targetSlot);
				if (targetSlot >= CLFECOMMON::INVALID_SLOT) return false;
				CEntityCL *target = EntitiesMngr.entity(targetSlot);
				if (target)
				{
					double dist = (target->pos() - startSlot->pos()).norm();
					bc.Targets.Targets.push_back(CMultiTarget::CTarget(targetSlot, false, (uint8) ceilf((float) (dist /  MULTI_TARGET_DISTANCE_UNIT))));
					startSlot = target;
				}
			}
		}
		break;
	}
	bc.BehavTime = TimeInSec;
	// TODO : the type of missile is contained in the spell id
	bc.Behav.Behaviour = MBEHAV::CAST_OFF_SUCCESS;
	//
	ch->applyBehaviour(bc);
	return true;
}

// temp to test cast of multitarget projectile on another entity
NLMISC_COMMAND(aura, "enable / disable aura on an entity", "<slot> <aura>")
{
	if (args.size() != 2) return false;
	uint slot;
	fromString(args[0], slot);
	CCharacterCL *ch = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(slot));
	if (!ch) return false;
	const CAnimationFX *fx = NULL;
	sint auraIndex;
	fromString(args[1], auraIndex);
	if (auraIndex != -1)
	{
		fx = CAttackListManager::getInstance().getAuras().getFX(auraIndex);
		if (!fx) return false;
	}
	ch->setAuraFX(0, fx);
	return true;
}

NLMISC_COMMAND(link, "enable / disable link on an entity", "<slot> <link>")
{
	if (args.size() != 2) return false;
	uint slot;
	fromString(args[0], slot);
	CCharacterCL *ch = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(slot));
	if (!ch) return false;
	const CAnimationFX *link = NULL;
	sint linkIndex;
	fromString(args[1], linkIndex);
	if (linkIndex != -1)
	{
		link = CAttackListManager::getInstance().getLinks().getFX(linkIndex + 1);
		if (!link) return false;
	}
	const CAnimationFX *linkBreak = CAttackListManager::getInstance().getLinks().getFX(0);
	if (!linkBreak) return false;
	ch->setLinkFX(link, linkBreak);
	return true;
}

NLMISC_COMMAND(auraReceipt, "enable / disable aura receipt on an entity", "<slot> <aura> <0=on/1=off>")
{
	if (args.size() != 2) return false;
	uint slot;
	fromString(args[0], slot);
	CCharacterCL *ch = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(slot));
	if (!ch) return false;
	const CAnimationFX *fx = NULL;
	bool enableAura;
	fromString(args[1], enableAura);
	if (enableAura)
	{
		fx = CAttackListManager::getInstance().getAuras().getFX(0); // 0 is special for aura receipt
		if (!fx) return false;
	}
	ch->setAuraFX(1, fx);
	return true;
}

////////////////////////////
// test for melee weapons //
////////////////////////////

// these are helpers (the same can be done with /vp, or altLook)
NLMISC_COMMAND(weapon, "change the weapon in hand", "<slot> <hand> <weapon>")
{
	if (args.size() != 3) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	uint slot;
	fromString(args[0], slot);
	CCDBNodeLeaf *propA = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:Entities:E%d:P%d", (int) slot, (int) PROPERTY_VPA), false);
	if (!propA) return false;
	sint64 valueA = propA->getValue64();

	CCDBNodeLeaf *propB = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:Entities:E%d:P%d", (int) slot, (int) PROPERTY_VPB), false);
	if (!propB) return false;
	sint64 valueB = propB->getValue64();

	uint hand;
	fromString(args[1], hand);

	// the VP is dependent of Entity actual type
	if(dynamic_cast<CPlayerCL*>(EntitiesMngr.entity(slot)))
	{
		SPropVisualA vpa = (SPropVisualA &) valueA;
		SPropVisualB vpb = (SPropVisualB &) valueB;
		if (hand == 0)
		{
			uint16 weaponRightHand;
			fromString(args[2], weaponRightHand);
			vpa.PropertySubData.WeaponRightHand = weaponRightHand;
			vpb.PropertySubData.RTrail = 1;
		}
		else
		{
			uint16 weaponLeftHand;
			fromString(args[2], weaponLeftHand);
			vpa.PropertySubData.WeaponLeftHand = weaponLeftHand;
			vpb.PropertySubData.LTrail = 1;
		}
		propA->setValue64((sint64) vpa.PropertyA);
		propB->setValue64((sint64) vpb.PropertyB);
	}
	// CharacterCL: use a SAltLook
	else
	{
		SAltLookProp vpalt = (SAltLookProp&) valueA;
		if (hand == 0)
		{
			uint16 weaponRightHand;
			fromString(args[2], weaponRightHand);
			vpalt.Element.WeaponRightHand = weaponRightHand;
			vpalt.Element.RTrail = 1;
		}
		else
		{
			uint16 weaponLeftHand;
			fromString(args[2], weaponLeftHand);
			vpalt.Element.WeaponLeftHand = weaponLeftHand;
			vpalt.Element.LTrail = 1;
		}
		propA->setValue64((sint64) vpalt.Summary);
	}

	// Force to update property
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPB);

	// display name of weapon sheet

	return true;
}

NLMISC_COMMAND(advantageFX, "turn on / off the advantage fx for an item in hand", "<slot> <hand> <on = 1/ off = 0>")
{
	if (args.size() != 3) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	uint slot;
	fromString(args[0], slot);

	/*
	CCDBNodeLeaf *prop = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:Entities:E%d:P%d", (int) slot, (int) PROPERTY_VPA), false);
	if (!prop) return false;
	sint64 value = prop->getValue64();
	uint hand;
	fromString(args[1], hand);
	// the VP is dependent of Entity actual type
	if(dynamic_cast<CPlayerCL*>(EntitiesMngr.entity(slot)))
	{
		SPropVisualA vpa = (SPropVisualA &) value;
		if (hand == 0)
		{
			fromString(args[2], vpa.PropertySubData.RWeaponFX);
		}
		else
		{
			fromString(args[2], vpa.PropertySubData.LWeaponFX);
		}
		prop->setValue64((sint64) vpa.PropertyA);
	}
	// CharacterCL: use a SAltLook
	else
	{
		SAltLookProp vpa = (SAltLookProp&) value;
		if (hand == 0)
		{
			fromString(args[2], vpa.Element.RWeaponFX);
		}
		else
		{
			fromString(args[2], vpa.Element.LWeaponFX);
		}
		prop->setValue64((sint64) vpa.Summary);
	}
	*/

	// Force to update property
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);

	return true;
}

NLMISC_COMMAND(trailLength, "set length of trail for one weapon in hand", "<slot> <hand> <power = 0..15>")
{
	if (args.size() != 3) return false;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	uint slot;
	fromString(args[0], slot);

	CCDBNodeLeaf *propA = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:Entities:E%d:P%d", (int) slot, (int) PROPERTY_VPA), false);
	if (!propA) return false;
	sint64 valueA = propA->getValue64();

	CCDBNodeLeaf *propB = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:Entities:E%d:P%d", (int) slot, (int) PROPERTY_VPB), false);
	if (!propB) return false;
	sint64 valueB = propB->getValue64();

	uint hand;
	fromString(args[1], hand);

	// the VP is dependent of Entity actual type
	if(dynamic_cast<CPlayerCL*>(EntitiesMngr.entity(slot)))
	{
		SPropVisualA vpa = (SPropVisualA &) valueA;
		SPropVisualB vpb = (SPropVisualB &) valueB;
		if (hand == 0)
		{
			uint16 rTrail;
			fromString(args[2], rTrail);
			vpb.PropertySubData.RTrail = rTrail;
			propB->setValue64((sint64) vpb.PropertyB);
		}
		else
		{
			uint16 lTrail;
			fromString(args[2], lTrail);
			vpb.PropertySubData.LTrail = lTrail / 2;
			propB->setValue64((sint64) vpb.PropertyB);
		}
	}
	else
	{
		SAltLookProp vpalt = (SAltLookProp &) valueA;
		if (hand == 0)
		{
			uint16 rTrail;
			fromString(args[2], rTrail);
			vpalt.Element.RTrail = rTrail;
		}
		else
		{
			uint16 lTrail;
			fromString(args[2], lTrail);
			vpalt.Element.LTrail = lTrail / 2;
		}
		propA->setValue64((sint64) vpalt.Summary);
	}

	// Force to update property
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPB);

	return true;
}


// simulate an attack behaviour for the given slot
NLMISC_COMMAND(attack, "simulate an attack", "<slot> <intensity> <hit_type> <localisation> <dmg_type> <damage_shield_power> <damage_shield_id>")
{
	if (args.size() < 2) return false;
	if (args.size() > 7) return false;
	CBehaviourContext bc;
	bc.Behav.Behaviour = MBEHAV::DEFAULT_ATTACK;
	bc.Behav.Combat.ActionDuration = 0;
	uint16 impactIntensity;
	fromString(args[1], impactIntensity);
	bc.Behav.Combat.ImpactIntensity = impactIntensity;
	bc.Behav.Combat.HitType = HITTYPE::Hit;
	bc.Behav.Combat.Localisation = BODY::HHead;
	bc.Behav.Combat2.DamageType = 0;
	if (args.size() > 2)
	{
		uint16 hitType;
		fromString(args[2], hitType);
		bc.Behav.Combat.HitType = hitType + 1;
	}
	if (args.size() > 3)
	{
		uint16 localisation;
		fromString(args[3], localisation);
		bc.Behav.Combat.Localisation = localisation;
	}
	if (args.size() > 4)
	{
		uint16 damageType;
		fromString(args[4], damageType);
		bc.Behav.Combat2.DamageType = damageType;
	}
	uint dsPower = 0;
	uint dsType = 0;
	if (args.size() > 5)
	{
		fromString(args[5], dsPower);
	}
	if (args.size() > 6)
	{
		fromString(args[6], dsType);
	}

	uint slot;
	fromString(args[0], slot);
	CCharacterCL *entity = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(slot));
	if (!entity) return false;
	// push current slection as main target
	CMultiTarget::CTarget target;
	target.TargetSlot = UserEntity->selection();
	target.Info = dsPower | (dsType << 3);
	bc.Targets.Targets.push_back(target);
	bc.BehavTime = TimeInSec;
	bc.Behav.DeltaHP = -20;
	entity->applyBehaviour(bc);
	return true;
}

// simulate a range attack from the current slor to the current selection
NLMISC_COMMAND(rangeAttack, "simulate a range attack", "<slot> [intensity] [localisation] [range_weapon_type_if_unequipped]")
{
	if (args.size() < 1) return false;
	if (args.size() > 4) return false;
	uint slot;
	fromString(args[0], slot);
	CCharacterCL *entity = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(slot));
	if (!entity) return false;
	const CItemSheet *weaponSheet = entity->getRightHandItemSheet();
	CBehaviourContext bc;
	if (!weaponSheet || weaponSheet->Family != ITEMFAMILY::RANGE_WEAPON)
	{
		//
		uint16 weaponType = 0;
		if (args.size() > 3)
		{
			fromString(args[3], weaponType);
		}
		bc.Behav.Range.WeaponType = weaponType;
	}
	else
	{
		bc.Behav.Range.WeaponType = weaponSheet->RangeWeapon.RangeWeaponType;
	}
	bc.Behav.Behaviour = MBEHAV::RANGE_ATTACK;
	bc.Behav.Range.ImpactIntensity = 1;
	bc.Behav.Range.Localisation = BODY::HHead;
	bc.BehavTime = TimeInSec;
	if (args.size() > 1)
	{
		uint16 impactIntensity;
		fromString(args[1], impactIntensity);
		bc.Behav.Range.ImpactIntensity = impactIntensity;
	}
	if (args.size() > 2)
	{
		uint16 localisation;
		fromString(args[2], localisation);
		bc.Behav.Range.Localisation = localisation;
	}
	// if not a generic range weapon, add a single target (this is the current selection)
	uint8 targetSlot = UserEntity->targetSlot();
	if (targetSlot >= CLFECOMMON::INVALID_SLOT) return false;
	CEntityCL *target = EntitiesMngr.entity(targetSlot);
	if (!target) return false;
	double dist = (target->pos() - entity->pos()).norm();
	bc.Targets.Targets.push_back(CMultiTarget::CTarget(targetSlot, false, (uint8) ceilf((float) (dist /  MULTI_TARGET_DISTANCE_UNIT))));
	bc.Behav.DeltaHP = -10;
	entity->applyBehaviour(bc);
	return true;
}


// simulate a creature attack
NLMISC_COMMAND(creatureAttack, "simulate a creature attack (2 attaques per creature)", "<casterSlot> <targetSlot> [attk=0/1] [magicIntensity] [physicalIntensity] [localisation] [damageType] [hitType] [resist=1/0]")
{
	if (args.size() < 2) return false;
	if (args.size() > 9) return false;
	CBehaviourContext bc;
	bc.Behav.Behaviour = MBEHAV::CREATURE_ATTACK_0;
	if (args.size() > 2)
	{
		uint attk;
		fromString(args[2], attk);
		bc.Behav.Behaviour = attk == 0 ? MBEHAV::CREATURE_ATTACK_0 : MBEHAV::CREATURE_ATTACK_1;
	}
	uint8 casterSlot;
	fromString(args[0], casterSlot);
	CCharacterCL *caster = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(casterSlot));
	if (!caster) return false;
	uint8 targetSlot;
	fromString(args[1], targetSlot);
	CCharacterCL *target = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(targetSlot));
	if (!target) return false;
	double dist = (target->pos() - caster->pos()).norm();
	bool resist = false;
	if (args.size() > 8)
	{
		fromString(args[8], resist);
	}
	bc.Targets.Targets.push_back(CMultiTarget::CTarget(targetSlot, false, (uint8) ceilf((float) (dist /  MULTI_TARGET_DISTANCE_UNIT))));
	bc.Behav.CreatureAttack.ActionDuration = 0;
	uint magicImpactIntensity = 1;
	if (args.size() > 3)
	{
		fromString(args[3], magicImpactIntensity);
	}
	bc.Behav.CreatureAttack.MagicImpactIntensity = magicImpactIntensity;
	uint physicalImpactIntensity = 0;
	if (args.size() > 4)
	{
		fromString(args[4], physicalImpactIntensity);
	}
	bc.Behav.CreatureAttack.ImpactIntensity = physicalImpactIntensity;
	BODY::TBodyPart localisation = BODY::HHead;
	if (args.size() > 5)
	{
		sint tmp;
		fromString(args[5], tmp);
		localisation = (BODY::TBodyPart) tmp;
	}
	bc.Behav.CreatureAttack.Localisation = localisation;
	DMGTYPE::EDamageType dmgType = DMGTYPE::BLUNT;
	if (args.size() > 6)
	{
		sint tmp;
		fromString(args[6], tmp);
		dmgType = (DMGTYPE::EDamageType) tmp;
	}
	bc.Behav.CreatureAttack2.DamageType = dmgType;
	HITTYPE::THitType hitType = HITTYPE::Hit;
	if (args.size() > 7)
	{
		sint tmp;
		fromString(args[7], tmp);
		hitType = (HITTYPE::THitType) tmp;
	}
	bc.Behav.CreatureAttack2.HitType = hitType;
	bc.BehavTime = TimeInSec;
	bc.Behav.DeltaHP = -15;
	caster->applyBehaviour(bc);
	return true;
}

NLMISC_COMMAND(setNamePosZ, "", "<low/high/normal> <value>")
{
	if (args.size() != 2) return false;

	CEntityCL *target = EntitiesMngr.entity(UserEntity->targetSlot());
	if (!target)
		return true;

	float *namePosZ = NULL;
	string sheetName, skelName;
	if (target->Type == CEntityCL::Player)
	{
		CPlayerCL *playerTarget = dynamic_cast<CPlayerCL*>(target);
		if (playerTarget)
		{
			CRaceStatsSheet *sheet = const_cast<CRaceStatsSheet*>(playerTarget->playerSheet());
			if (sheet)
			{
				if (toLower(args[0]) == "low")
					namePosZ = &sheet->GenderInfos[playerTarget->getGender()].NamePosZLow;
				else if (toLower(args[0]) == "normal")
					namePosZ = &sheet->GenderInfos[playerTarget->getGender()].NamePosZNormal;
				else if (toLower(args[0]) == "high")
					namePosZ = &sheet->GenderInfos[playerTarget->getGender()].NamePosZHigh;

				sheetName = sheet->Id.toString();
				skelName = sheet->GenderInfos[playerTarget->getGender()].Skelfilename;
			}
		}
	}
	else
	{
		CCharacterCL *creatureTarget = dynamic_cast<CCharacterCL*>(target);
		if (creatureTarget)
		{
			CCharacterSheet *sheet = const_cast<CCharacterSheet*>(creatureTarget->getSheet());
			if (sheet)
			{
				if (toLower(args[0]) == "low")
					namePosZ = &sheet->NamePosZLow;
				else if (toLower(args[0]) == "normal")
					namePosZ = &sheet->NamePosZNormal;
				else if (toLower(args[0]) == "high")
					namePosZ = &sheet->NamePosZHigh;

				sheetName = sheet->Id.toString();
				skelName = sheet->getSkelFilename();
			}
		}
	}

	if (namePosZ)
	{
		fromString(args[1], *namePosZ);
		nlinfo("NAMEPOSZ: sheet: %s, skel: %s, NamePosZ%s = %g", sheetName.c_str(), skelName.c_str(), args[0].c_str(), *namePosZ);
	}

	return true;
}

NLMISC_COMMAND(setMyNamePosZ, "", "<low/high/normal> <value>")
{
	if (args.size() != 2) return false;

	float *namePosZ = NULL;
	string sheetName, skelName;
	CRaceStatsSheet *sheet = const_cast<CRaceStatsSheet*>(UserEntity->playerSheet());
	if (sheet)
	{
		if (toLower(args[0]) == "low")
			namePosZ = &sheet->GenderInfos[UserEntity->getGender()].NamePosZLow;
		else if (toLower(args[0]) == "normal")
			namePosZ = &sheet->GenderInfos[UserEntity->getGender()].NamePosZNormal;
		else if (toLower(args[0]) == "high")
			namePosZ = &sheet->GenderInfos[UserEntity->getGender()].NamePosZHigh;

		sheetName = sheet->Id.toString();
		skelName = sheet->GenderInfos[UserEntity->getGender()].Skelfilename;
	}

	if (namePosZ)
	{
		fromString(args[1], *namePosZ);
		nlinfo("NAMEPOSZ: sheet: %s, skel: %s, NamePosZ%s = %g", sheetName.c_str(), skelName.c_str(), args[0].c_str(), *namePosZ);
	}

	return true;
}


NLMISC_COMMAND(pvpMode, "modify pvp mode", "[<pvp mode> <state>]")
{
	if (args.size() != 0 && args.size() != 2) return false;

	CInterfaceManager *IM = CInterfaceManager::getInstance();

	CEntityCL *target = EntitiesMngr.entity(UserEntity->targetSlot());
	if (!target)
	{
		IM->displaySystemInfo(toString("<pvpMode> no target"));
		return false;
	}
	if (target->Type != CEntityCL::Player && target->Type != CEntityCL::User)
	{
		IM->displaySystemInfo(toString("<pvpMode> target is not a player"));
		return false;
	}
	CPlayerCL *playerTarget = dynamic_cast<CPlayerCL*>(target);
	if (!playerTarget)
		return false;

	if( args.size() == 0 )
	{
		uint16 pvpMode = playerTarget->getPvpMode();
		string str;
		if( pvpMode&PVP_MODE::PvpDuel )
			str+="duel ";
		if( pvpMode&PVP_MODE::PvpChallenge)
			str+="challenge ";
		if( pvpMode&PVP_MODE::PvpZoneFree)
			str+="free ";
		if( pvpMode&PVP_MODE::PvpZoneFaction)
			str+="zone_faction ";
		if( pvpMode&PVP_MODE::PvpZoneGuild)
			str+="zone_guild ";
		if( pvpMode&PVP_MODE::PvpZoneOutpost)
			str+="outpost ";
		if( pvpMode&PVP_MODE::PvpFaction)
			str+="faction ";
		if( pvpMode&PVP_MODE::PvpFactionFlagged)
			str+="faction_flagged ";
		if( pvpMode&PVP_MODE::PvpZoneSafe)
			str+="in_safe_zone ";
		if( pvpMode&PVP_MODE::PvpSafe)
			str+="safe ";
		IM->displaySystemInfo(ucstring(str));
		nlinfo("<pvpMode> %s",str.c_str());
	}
	else
	{
		PVP_MODE::TPVPMode pvpMode = PVP_MODE::fromString(args[0]);
		bool state;
		fromString(args[1], state);
		if( state )
		{
			uint16 currentPVPMode = playerTarget->getPvpMode();
			currentPVPMode |= pvpMode;
			playerTarget->setPvpMode(currentPVPMode);
			IM->displaySystemInfo(toString("<pvpMode> adding pvp mode %s",args[0].c_str()));
		}
		else
		{
			uint16 currentPVPMode = playerTarget->getPvpMode();
			currentPVPMode &= ~pvpMode;
			playerTarget->setPvpMode(currentPVPMode);
			IM->displaySystemInfo(toString("<pvpMode> removing pvp mode %s",args[0].c_str()));
		}
	}
	playerTarget->buildInSceneInterface();
	return true;
}

/*
NLMISC_COMMAND(pvpClan, "modify pvp clan", "<pvp clan>")
{
	if (args.size() != 1) return false;

	CInterfaceManager *IM = CInterfaceManager::getInstance();

	CEntityCL *target = EntitiesMngr.entity(UserEntity->targetSlot());
	if (!target)
	{
		IM->displaySystemInfo(toString("<pvpClan> no target"));
		return false;
	}
	if (target->Type != CEntityCL::Player && target->Type != CEntityCL::User)
	{
		IM->displaySystemInfo(toString("<pvpMode> target is not a player"));
		return false;
	}
	CPlayerCL *playerTarget = dynamic_cast<CPlayerCL*>(target);
	if (!playerTarget)
		return false;

//	PVP_CLAN::TPVPClan clan = PVP_CLAN::fromString(args[0]);
//	playerTarget->setPvpClan(clan);
	playerTarget->buildInSceneInterface();

	return true;
}
*/
#endif // !FINAL_VERSION

#include "r2/editor.h"

//---------------------------------------------------
// setDead :
// Method to Flag the character as dead and do everything needed.
//---------------------------------------------------
void CCharacterCL::setDead()	// virtual
{
	// If the entity dead is the user -> switch to dead mode.
	if(_Slot == UserEntity->slot())
		UserControls.mode(CUserControls::DeathMode);

	// If the entity killed was the current user target, we update context cursor
	if(_Slot == UserEntity->selection())
	{
		bool nextContextSelected = false;
		// Quartering
		if (!R2::getEditor().isDMing())
		{
			if(_Properties.harvestable())
				nextContextSelected = ContextCur.context("QUARTER");
			// Loot
			else if(_Properties.lootable())
				nextContextSelected = ContextCur.context("LOOT");
			// Pick Up
			else if(_Properties.liftable())
				nextContextSelected = ContextCur.context("PICKUP");
			if( !nextContextSelected )
				ContextCur.context("STAND BY");
		}
	}

	// The character now won't be an obstacle anymore
	_Primitive->setOcclusionMask(MaskColNone);
}// setDead //

