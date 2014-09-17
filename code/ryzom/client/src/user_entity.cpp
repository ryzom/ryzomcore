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
// Misc.
#include "nel/misc/vectord.h"
#include "nel/misc/matrix.h"
#include "nel/misc/quat.h"
// 3D Interface.
#include "nel/3d/u_scene.h"
#include "nel/3d/u_visual_collision_manager.h"
#include "nel/3d/viewport.h"
#include "nel/3d/u_bone.h"
#include "nel/3d/u_instance_material.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_point_light.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_camera.h"
// Pacs Interface
#include "nel/pacs/u_global_position.h"
// Client.
#include "user_entity.h"
#include "motion/user_controls.h"
#include "pacs_client.h"
#include "net_manager.h"
#include "time_client.h"
#include "entity_animation_manager.h"
#include "sheet_manager.h"
#include "sound_manager.h"
#include "interface_v3/interface_manager.h"
#include "entities.h"
#include "debug_client.h"
#include "misc.h"
#include "interface_v3/bot_chat_manager.h"
#include "fx_manager.h"
#include "main_loop.h"
#include "interface_v3/group_in_scene_bubble.h"
#include "interface_v3/inventory_manager.h"
#include "nel/gui/group_html.h"
#include "interface_v3/people_interraction.h"
#include "init_main_loop.h"
#include "view.h"
#include "interface_v3/sphrase_manager.h"
#include "interface_v3/sbrick_manager.h"
#include "interface_v3/action_phrase_faber.h"
#include "interface_v3/bar_manager.h"
#include "interface_v3/skill_manager.h"
#include "far_tp.h"
#include "npc_icon.h"
// game share
#include "game_share/slot_types.h"
#include "game_share/player_visual_properties.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/inventories.h"
#include "game_share/animal_type.h"
#include "game_share/bot_chat_types.h"
// Sound animation
#include "nel/sound/sound_anim_manager.h"
#include "nel/sound/sound_animation.h"
#include "nel/sound/sound_anim_marker.h"
// r2
#include "r2/editor.h"

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NLPACS;
using namespace std;
using NL3D::UScene;
using NL3D::UVisualCollisionManager;
using NL3D::UTextContext;


////////////
// EXTERN //
////////////
extern UScene								*Scene;
extern UVisualCollisionManager				*CollisionManager;
extern CEntityAnimationManager				*EAM;
extern UTextContext							*TextContext;
extern NL3D::UCamera								MainCam;

// Context help
extern void contextHelp (const std::string &help);

extern void beastOrder (const std::string &orderStr, const std::string &beastIndexStr, bool confirmFree = true);

// Out game received position
NLMISC::CVectorD	UserEntityInitPos;
NLMISC::CVector		UserEntityInitFront;
CUserEntity			*UserEntity = NULL;

uint32				CharFirstConnectedTime = 0;
uint32				CharPlayedTime = 0;

const double		MaxExtractionDistance = 1.0f;

////////////
// GLOBAL //
////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Update_Sound )

//////////////
// FUNCTION //
//////////////
//string chooseRandom( const vector<string>& sounds, uint32& previousIndex );

//-----------------------------------------------
// CUserEntity :
// Constructor.
//-----------------------------------------------
CUserEntity::CUserEntity()
: CPlayerCL()
{
	Type				= User;
	_Run				= false;
	_RunWhenAble		= false;
	_WalkVelocity		= 1.0f;
	_RunVelocity		= 2.0f;
	_CurrentVelocity	= _WalkVelocity;

	_FrontVelocity		= 0.0f;
	_LateralVelocity	= 0.0f;

	// \todo GUIGUI : do it more generic.
	_First_Pos = false;

	// No selection, trader, interlocutor at the beginning.
	_Selection	= CLFECOMMON::INVALID_SLOT;
	_Trader		= CLFECOMMON::INVALID_SLOT;
	_Interlocutor = CLFECOMMON::INVALID_SLOT;

	// Not selectable at the beginning.
	_Selectable = false;

	// Your are not on a mount at the beginning.
	_OnMount = false;

	_AnimAttackOn = false;

	_ViewMode = FirstPV;
	_PermanentDeath = false;
	_FollowMode = false;

	_CheckPrimitive = 0;
	// The user is not in collision with someone else.
	_ColOn = false;
	// Collisions are not removed.
	_ColRemoved = false;

	// No Move To at the beginning.
	_MoveToSlot = CLFECOMMON::INVALID_SLOT;
	_MoveToAction= CUserEntity::None;
	_MoveToDist= 0.0;
	_MoveToColStartTime= 0;

	_FollowForceHeadPitch= false;

	_ForceLookSlot= CLFECOMMON::INVALID_SLOT;
	_LastExecuteCombatSlot= CLFECOMMON::INVALID_SLOT;

	_R2CharMode= R2::TCharMode::Player;

}// CUserEntity //


//-----------------------------------------------
// ~CUserEntity :
// Destructor.
//-----------------------------------------------
CUserEntity::~CUserEntity()
{
	// Remove observers
	_SpeedFactor.release();
	_MountHunger.release();
	_MountSpeeds.release();

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	
	{
		CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:IS_INVISIBLE", false);
		if (node)
		{
			ICDBNode::CTextId textId;
			node->removeObserver(&_InvisibleObs, textId);
		}
	}

	for(uint i=0;i<EGSPD::CSPType::EndSPType;i++)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:USER:SKILL_POINTS_%d:VALUE", i), false);
		if(node)
		{
			ICDBNode::CTextId textId;
			node->removeObserver(_SkillPointObs+i, textId);
		}
	}

	for( uint i=0; i<_FamesObs.size(); ++i )
	{
		uint32 factionIndex = _FamesObs[i]->FactionIndex;
		uint32 fameIndexInDatabase = CStaticFames::getInstance().getDatabaseIndex(factionIndex);
		string sDBPath = toString("SERVER:FAME:PLAYER%d:VALUE",fameIndexInDatabase);

		CCDBNodeLeaf * node = NLGUI::CDBManager::getInstance()->getDbProp(sDBPath, false);
		if(node)
		{
			ICDBNode::CTextId textId;
			node->removeObserver(_FamesObs[i], textId);
		}
	}
	contReset(_FamesObs);

	CNPCIconCache::getInstance().removeObservers();

	// Remove the Primitive used for check (because ~CEntityCL() will call CEntityCL::removePrimitive(), not CUserEntity::removePrimitive())
	removeCheckPrimitive();

	CNPCIconCache::release();

}// ~CUserEntity //

//-----------------------------------------------
// initProperties :
// Initialize properties of the entity (according to the class).
//-----------------------------------------------
void CUserEntity::initProperties()
{
	properties().selectable(true);
}// initProperties //


//-----------------------------------------------
// build :
// Build the entity from a sheet.
//-----------------------------------------------
bool CUserEntity::build(const CEntitySheet *sheet)	// virtual
{
	// Init received position
	pos(UserEntityInitPos);
	front(UserEntityInitFront);
	dir(front());
	setHeadPitch(0);

	// Cast the sheet in the right type.
	_PlayerSheet = dynamic_cast<const CRaceStatsSheet *>(sheet);
	if(_PlayerSheet == 0)
	{
		pushDebugStr("User Sheet is not a valid '.race_stats'.");
		return false;
	}
	else
		pushInfoStr("User Sheet is a valid '.race_stats'.");
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

	disableFollow();
	// Walk/Run ?
	if(ClientCfg.RunAtTheBeginning != _Run)
		switchVelocity();

	// Set the up of the user.
	up(CVector(0,0,1));

	// Init User infos.
	eyesHeight(ClientCfg.EyesHeight);
	walkVelocity(ClientCfg.Walk);
	runVelocity(ClientCfg.Run);

	// Compute the first automaton.
	_CurrentAutomaton = automatonType() + "_normal.automaton";

	// Build the PACS Primitive.
	if(initPrimitive(0.5f, 2.0f, 0.0f, 0.0f, UMovePrimitive::Slide, (UMovePrimitive::TTrigger)(UMovePrimitive::OverlapTrigger | UMovePrimitive::EnterTrigger), MaskColPlayer, MaskColPlayer | MaskColNpc | MaskColDoor))
		_Primitive->insertInWorldImage(dynamicWI);

	// Compute the element to be able to snap the entity to the ground.
	computeCollisionEntity();

	// Initialize properties of the client.
	initProperties();

	// Initialize the observer for the speed factor and mount stuff
	_SpeedFactor.init();
	_MountHunger.init();
	_MountSpeeds.init();

	// Create the user playlist
	createPlayList();

	// Initialize the internal time.
	_LastFrameTime = ((double)T1) * 0.001;

	// Set the gender in local mode.
	if(ClientCfg.Local)
	{
		_Mode       = MBEHAV::NORMAL;
		_ModeWanted = MBEHAV::NORMAL;
		_Gender = ClientCfg.Sex;
		SPropVisualA visualA = buildPropVisualA(_PlayerSheet->GenderInfos[_Gender]);
		SPropVisualB visualB = buildPropVisualB(_PlayerSheet->GenderInfos[_Gender]);
		SPropVisualC visualC;
		visualA.PropertySubData.Sex             = _Gender;
		visualC.PropertyC                       = 0;
		visualC.PropertySubData.CharacterHeight = 0;
		visualC.PropertySubData.ArmsWidth       = 7;
		visualC.PropertySubData.LegsWidth       = 7;
		visualC.PropertySubData.TorsoWidth      = 7;
		visualC.PropertySubData.BreastSize      = 7;
		// Set the Database
		sint64 *prop = (sint64 *)&visualA;
		NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E0:P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->setValue64(*prop);	// Set the database
		prop = (sint64 *)&visualB;
		NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E0:P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->setValue64(*prop);	// Set the database
		prop = (sint64 *)&visualC;
		NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E0:P"+toString("%d", CLFECOMMON::PROPERTY_VPC))->setValue64(*prop);	// Set the database
		// Apply Changes.
		updateVisualProperty(0, CLFECOMMON::PROPERTY_VPA);
	}
	// \todo GUIGUI Retrieve the player's appearence during the avatar selection.
	// Get Visual Properties From the character selection window.
	else
	{
	}

	// Rebuild interface
	buildInSceneInterface ();

	// Add observer on invisible property
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	{
		CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:IS_INVISIBLE", false);
		if (node)
		{
			ICDBNode::CTextId textId;
			node->addObserver(&_InvisibleObs, textId);
		}
	}

	// Add an observer on skill points
	for(uint i=0;i<EGSPD::CSPType::EndSPType;i++)
	{
		_SkillPointObs[i].SpType= i;
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:USER:SKILL_POINTS_%d:VALUE", i), false);
		if(node)
		{
			ICDBNode::CTextId textId;
			node->addObserver(_SkillPointObs+i, textId);
		}
	}

	// Add an observer on Fames
	for( uint i=(uint)PVP_CLAN::BeginClans; i<=(uint)PVP_CLAN::EndClans; ++i )
	{
		uint32 factionIndex = PVP_CLAN::getFactionIndex((PVP_CLAN::TPVPClan)i);
		uint32 fameIndexInDatabase = CStaticFames::getInstance().getDatabaseIndex(factionIndex);
		string sDBPath = toString("SERVER:FAME:PLAYER%d:VALUE",fameIndexInDatabase);

		CFameObserver * fameObs = new CFameObserver();
		if( fameObs )
		{
			fameObs->FactionIndex = factionIndex;
			CCDBNodeLeaf * node = NLGUI::CDBManager::getInstance()->getDbProp(sDBPath, false);
			if(node)
			{
				ICDBNode::CTextId textId;
				node->addObserver(fameObs, textId);
			}
			_FamesObs.push_back(fameObs);
		}
	}

	// Add an observer on Mission Journal
	CNPCIconCache::getInstance().addObservers();

	// Initialize the camera distance.
	View.cameraDistance(ClientCfg.CameraDistance);

	// char and account time properties
	CSkillManager *pSM = CSkillManager::getInstance();
	if( pSM )
	{
		pSM->tryToUnblockTitleFromCharOldness( CharFirstConnectedTime );
		pSM->tryToUnblockTitleFromCharPlayedTime( CharPlayedTime );
	}

	// Entity created.
	return true;
}// build //


//-----------------------------------------------
// eyesHeight :
// \todo GUIGUI : do it better in mount mode
//-----------------------------------------------
float CUserEntity::eyesHeight()
{
	if(!_OnMount)
		return _EyesHeight * _CharacterScalePos;
	else
		return _EyesHeight * _CharacterScalePos;
}// eyesHeight //


/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////// VISUAL PROPERTIES ///////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
//-----------------------------------------------
// updateVisualPropertyPos :
// Update Entity Position.
//-----------------------------------------------
void CUserEntity::updateVisualPropertyPos(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */, const NLMISC::TGameCycle &/* pI */)
{
}// updateVisualPropertyPos //

//-----------------------------------------------
// updateVisualPropertyOrient :
// Update Entity Orientation.
//-----------------------------------------------
void CUserEntity::updateVisualPropertyOrient(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */)
{
}// updateVisualPropertyOrient //


void CUserEntity::updateVisualPropertyTargetList(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */, uint /* listIndex */)
{
}

void CUserEntity::updateVisualPropertyVisualFX(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	applyVisualFX(prop);
}

//-----------------------------------------------
// updateVisualPropertyBehaviour :
// Update Entity Behaviour.
//-----------------------------------------------
void CUserEntity::updateVisualPropertyBehaviour(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// Compute the behaviour.
	CBehaviourContext bc;
	bc.Behav     = MBEHAV::CBehaviour(prop);
	bc.BehavTime = TimeInSec;
	if(VerboseAnimUser)
	{
		nlinfo("UE::updateVPBeha: '%d(%s)'.", (sint)bc.Behav.Behaviour, MBEHAV::behaviourToString(bc.Behav.Behaviour).c_str());
	}
	CCDBNodeLeaf *targetList0 = dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_TARGET_LIST_0));
	CCDBNodeLeaf *targetList1 = dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_TARGET_LIST_1));
	CCDBNodeLeaf *targetList2 = dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_TARGET_LIST_1));
	CCDBNodeLeaf *targetList3 = dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_TARGET_LIST_1));
	if (targetList0 && targetList1 && targetList2 && targetList3)
	{
		uint64 vp[4] =
		{
			(uint64) targetList0->getValue64(),
			(uint64) targetList1->getValue64(),
			(uint64) targetList2->getValue64(),
			(uint64) targetList3->getValue64()
		};
		bc.Targets.unpack(vp, 4);
	}
	applyBehaviour(bc);
}// updateVisualPropertyBehaviour //

//-----------------------------------------------
// updateVisualPropertyName :
// Update Entity Name.
//-----------------------------------------------
void CUserEntity::updateVisualPropertyName(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	uint32 oldNameId = _NameId;

	CPlayerCL::updateVisualPropertyName(gameCycle, prop);

	// Name changed ?
/*	if (oldNameId != _NameId)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceElement *element = CWidgetManager::getInstance()->getElementFromId("ui:interface:mailbox:content:html");
		if (element)
		{
			CGroupHTML *html = dynamic_cast<CGroupHTML*>(element);
			if (html)
				html->browse("home");
		}
	}
*/	
}// updateVisualPropertyName //

//-----------------------------------------------
// updateVisualPropertyTarget :
// Update Entity Target.
//-----------------------------------------------
void CUserEntity::updateVisualPropertyTarget(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &/* prop */)
{
	// Don't override the Player Target, cause client side entirely => no lag.
	//targetSlot((CLFECOMMON::TCLEntityId)prop);
}// updateVisualPropertyTarget //

//-----------------------------------------------
// updateVisualPropertyMode :
// New mode received -> immediately change the mode for the user.
// \warning Read the position or orientation from the database when reading the mode (no more updated in updateVisualPropertyPos and updateVisualPropertyOrient).
//-----------------------------------------------
void CUserEntity::updateVisualPropertyMode(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// Combat Float Check
	if((MBEHAV::EMode)prop == MBEHAV::COMBAT_FLOAT)
	{
		nlwarning("UE:updateVPMode: the user should never have the COMBAT_FLOAT mode.");
		return;
	}
	// New Mode Received.
	_TheoreticalMode = (MBEHAV::EMode)prop;
	// Change the user mode.
	mode(_TheoreticalMode);
}// updateVisualPropertyMode //

//-----------------------------------------------
// updateVisualPropertyVpa :
// Update Entity Visual Property A
//-----------------------------------------------
void CUserEntity::updateVisualPropertyVpa(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	CPlayerCL::updateVisualPropertyVpa(gameCycle, prop);

	// Special for user: Disable Character Lod, because always want it at full rez.
	if(!_Skeleton.empty())
	{
		_Skeleton.setLodCharacterShape(-1);
	}

	updateVisualDisplay();
}// updateVisualPropertyVpa //

//-----------------------------------------------
// updateVisualPropertyVpb :
// Update Entity Visual Property B
//-----------------------------------------------
void CUserEntity::updateVisualPropertyVpb(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	CPlayerCL::updateVisualPropertyVpb(gameCycle, prop);
	updateVisualDisplay();
}// updateVisualPropertyVpb //

//-----------------------------------------------
// updateVisualPropertyVpc :
// Update Entity Visual Property C
//-----------------------------------------------
void CUserEntity::updateVisualPropertyVpc(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	CPlayerCL::updateVisualPropertyVpc(gameCycle, prop);
	updateVisualDisplay();
}// updateVisualPropertyVpc //

//-----------------------------------------------
// updateVisualPropertyEntityMounted :
// Update Entity Mount
//-----------------------------------------------
void CUserEntity::updateVisualPropertyEntityMounted(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	if(isFighting())
		CPlayerCL::updateVisualPropertyEntityMounted(gameCycle, prop);
	else
		_Mount = (CLFECOMMON::TCLEntityId)prop;
}// updateVisualPropertyEntityMounted //

//-----------------------------------------------
// updateVisualPropertyRiderEntity :
// Update Entity Rider
//-----------------------------------------------
void CUserEntity::updateVisualPropertyRiderEntity(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	if(isFighting())
		CPlayerCL::updateVisualPropertyRiderEntity(gameCycle, prop);
	else
		_Rider = (CLFECOMMON::TCLEntityId)prop;
}// updateVisualPropertyRiderEntity //

//-----------------------------------------------
//-----------------------------------------------
void CUserEntity::updateVisualPropertyPvpMode(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	CPlayerCL::updateVisualPropertyPvpMode(gameCycle, prop);
	// Additionaly, inform interface of the change
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	// For PVP ZoneFaction
	CCDBNodeLeaf *pDB= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:TRACK_PVP_CHANGE_MODE");
	if(pDB)
	{
		sint32	val= pDB->getValue32();
		pDB->setValue32(val+1);
	}
	// For Any PVP change
	pDB= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:TRACK_PVP_CHANGE_ANY");
	if(pDB)
	{
		sint32	val= pDB->getValue32();
		pDB->setValue32(val+1);
	}
}

//-----------------------------------------------
//-----------------------------------------------
void CUserEntity::updateVisualPropertyOutpostInfos(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	CPlayerCL::updateVisualPropertyOutpostInfos(gameCycle, prop);
	// For Any PVP change
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf *pDB= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:TRACK_PVP_CHANGE_ANY");
	if(pDB)
	{
		sint32	val= pDB->getValue32();
		pDB->setValue32(val+1);
	}
}

//-----------------------------------------------
//-----------------------------------------------
void CUserEntity::updateVisualPropertyPvpClan(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{
	CPlayerCL::updateVisualPropertyPvpClan(gameCycle, prop);
	// For Any PVP change
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf *pDB= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:TRACK_PVP_CHANGE_ANY");
	if(pDB)
	{
		sint32	val= pDB->getValue32();
		pDB->setValue32(val+1);
	}
}


/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////


//-----------------------------------------------
// mode :
// Method called to change the mode (Combat/Mount/etc.).
// \todo GUIGUI : apply stage in combat modes instead of just removing them.
// \todo GUIGUI : go or teleport the player to the mode position (see how to manage it).
//-----------------------------------------------
bool CUserEntity::mode(MBEHAV::EMode m)
{
	if(Verbose & VerboseAnim)
		nlinfo("UE::mode: old mode '%s(%d)' new mode '%s(%d)'.", MBEHAV::modeToString(_Mode).c_str(), _Mode, MBEHAV::modeToString(m).c_str(), m);
	// Nothing to do if the mode is the same as the previous one.
	if(m == _Mode)
		return true;
	// Release the old Mode.
	switch(_Mode)
	{
		// Leave COMBAT Mode
		case MBEHAV::COMBAT:
		case MBEHAV::COMBAT_FLOAT:
		{
			// If there are some stage not complete -> remove them
			if(_Stages._StageSet.size() != 0)
				_Stages._StageSet.clear();
		}
		break;
		// Leave MOUNTED Mode
		case MBEHAV::MOUNT_NORMAL:
		case MBEHAV::MOUNT_SWIM:
		{
			if ( m == MBEHAV::REST )
			{
				// can't go afk while mounting
				return false;
			}

			// if changing mode for another mount mode, do nothing
			if (m != MBEHAV::MOUNT_NORMAL && m != MBEHAV::MOUNT_SWIM)
			{
				// Display the mount again.
				CCharacterCL *mount = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(_Mount));
				if(mount)
				{
					// Set the mount.
					mount->rider(CLFECOMMON::INVALID_SLOT);
					mount->_Stages._StageSet.clear();
					mount->setMode(MBEHAV::NORMAL);
					mount->computeAutomaton();
					mount->computeAnimSet();
					mount->setAnim(CAnimationStateSheet::Idle);
					if(mount->getPrimitive())
						mount->getPrimitive()->setOcclusionMask(MaskColNpc);	// the mount is an npc
					mount->_ForbidClipping = false;
				}
				//
				_Mount = CLFECOMMON::INVALID_SLOT;
				// Restore the user Primitive
				if(_Primitive)
				{
					_Primitive->setRadius( std::min(0.5f, (float)(RYZOM_ENTITY_SIZE_MAX/2)) );
					_Primitive->setHeight(2);
				}
				_OnMount = false;

				// Shift the player position (not to stand inside the mount)
				CVectorD unmountShift;
				unmountShift.sphericToCartesian(1.0, frontYaw() + NLMISC::Pi/2, 0);
				pacsPos(pos() + unmountShift);

				// Restore running if necessary
				if (!_Run && _RunWhenAble)
				{
					switchVelocity();
				}
			}

			if (_Mode == MBEHAV::MOUNT_SWIM && ( m == MBEHAV::COMBAT || m == MBEHAV::COMBAT_FLOAT))
			{
				//TODO : display "you can't fight while swimming"
				return true;
			}
		}
		break;
		// Leave DEATH Mode
		case MBEHAV::DEATH:
			// Restaure the last view.
			viewMode(viewMode());
			break;
		case MBEHAV::SWIM:
			if( m == MBEHAV::COMBAT || m == MBEHAV::COMBAT_FLOAT)
			{
				//TODO : display "you can't fight while swimming"
				return true;
			}
		break;
		default:
			nlwarning("Invalid behaviour change.");
	}

	// Reset Parent, unless we stay in mount mode
	if ((_Mode != MBEHAV::MOUNT_SWIM && _Mode != MBEHAV::MOUNT_NORMAL)
		|| (m != MBEHAV::MOUNT_SWIM && m != MBEHAV::MOUNT_NORMAL)
		)
	{
		parent(CLFECOMMON::INVALID_SLOT);
	}

	// Change the Mode for the user ( if user sits down or stands up we wait in order to play the sit/stand transition anim)
	if( m != MBEHAV::SIT && _Mode != MBEHAV::SIT )
		_Mode = m;
	_ModeWanted	= m;

	// Initialize the new Mode.
	switch(m)
	{
	// Combat mode
	case MBEHAV::COMBAT:
	case MBEHAV::COMBAT_FLOAT:
	{
		// Compute the angle
		const string propName = toString("SERVER:Entities:E%d:P%d", _Slot, CLFECOMMON::PROPERTY_ORIENTATION);
		sint64 ang = NLGUI::CDBManager::getInstance()->getDbProp(propName)->getValue64();
		_TargetAngle = *(float *)(&ang);

		// Initialize controls for the combat.
		UserControls.startCombat();

		// Context help
		contextHelp ("action_bar");
	}
	break;

	// Mount Normal or mount swim
	case MBEHAV::MOUNT_NORMAL:
		{
			CCharacterCL *mount = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(parent()));
			if(mount)
			{
				mount->_Stages.removePosWithNoMode();
				dir(mount->dir());
			}
		}
	case MBEHAV::MOUNT_SWIM:
	{
		// Hide the mount unless we come from another mounted mode
		if (_Mode != MBEHAV::MOUNT_SWIM && _Mode != MBEHAV::MOUNT_NORMAL)
		{
			if(_Mount != CLFECOMMON::INVALID_SLOT) // if _Mount is still invalid, the following code will be done in updatePos()
			{
				setOnMount();
			}
		}
		// refresh target
		UserEntity->selection(_Selection);
	}
	break;

	// Dead mode.
	case MBEHAV::DEATH:
		setDead();
		if(isSwimming())
			_Mode = MBEHAV::SWIM_DEATH;
		break;

	// Normal or Default mode.
	case MBEHAV::NORMAL:
		_CurrentBehaviour.Behaviour = MBEHAV::IDLE;
	default:
		//
		setAlive();
		viewMode(viewMode());
		break;
	}

	bool doSetAnim = true;
	// if user sits down or stands up we set transition anim before changing animset
	if( m == MBEHAV::SIT )
	{
		setAnim(CAnimationStateSheet::SitMode);
		_Mode = m;
		doSetAnim = false;
	}
	else
	if( _Mode == MBEHAV::SIT && m!=MBEHAV::DEATH )
	{
		setAnim(CAnimationStateSheet::SitEnd);
		_Mode = m;
		doSetAnim = false;
	}

	// Show/Hide all or parts of the body.
	updateVisualDisplay();
	if( ClientCfg.AutomaticCamera )
	{
		// Set the direction as the front.
		dir(front());
	}
	// Compute the current automaton
	computeAutomaton();
	// Update the animation set according to the mode.
	computeAnimSet();

	if( doSetAnim )
	{
		// Animset changed -> update current animation
		setAnim(CAnimationStateSheet::Idle);
	}

	// Changing the mode well done.
	return true;
}// mode //


/*
 * Mount the mount in _Mount
 */
void	CUserEntity::setOnMount()
{
	CCharacterCL *mount = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(_Mount));
	if(mount)
	{
		// Update primitives for the mount and the rider.
		if(_Primitive && mount->getPrimitive())
		{
			_Primitive->setRadius( std::min(mount->getPrimitive()->getRadius(), (float)(RYZOM_ENTITY_SIZE_MAX/2)) );
			_Primitive->setHeight(mount->getPrimitive()->getHeight());
			mount->getPrimitive()->setOcclusionMask(MaskColNone);		// Remove collisions.
		}
		// Now on a mount
		_OnMount = true;
		// Link the mount and the user.
		parent(_Mount);
		// Refresh the View Mode
		viewMode(viewMode());
		// Close the crafting window if open
		closeFaberCastWindow();

		// Keep run in mind
		_RunWhenAble = _Run;

		mount->_ForbidClipping = true;
	}
}



//-----------------------------------------------
// getVelocity :
// compute and return the entity velocity
//-----------------------------------------------
CVector CUserEntity::getVelocity() const
{
	static const CVector lateral(0,0,1);
	static CVector velocity;
	velocity =  front() * _FrontVelocity + ( lateral ^ front()) * _LateralVelocity;
	velocity.normalize();
	// User is Dead
	if(isDead())
	{
		velocity *= 0.0;
	}
	// User is sitting
	else if(isSit())
	{
		velocity *= 0.0;
	}
	// User is swimming
	else if(isSwimming())
	{
				// We are a Ring DM so speed up a litle


		// Forward Run or Walk
		if(_FrontVelocity > 0.0f)
		{
			if(_Run)
				velocity *= 3.0;
			else
				velocity *= 1.0;
		}
		// Lateral or Backward Walk
		else
			velocity *= 1.0;

		if (_R2CharMode == R2::TCharMode::Editer || _R2CharMode == R2::TCharMode::Dm)
		{
			velocity *= (float(ClientCfg.DmRun) / 6.0f); // velocity max = max run / 2
		}

	}
	else if(isRiding())
	{
		// Forward Run or Walk
		if(_FrontVelocity > 0.0f)
		{
			if(_Run)
				velocity *= getMountRunVelocity();
			else
				velocity *= getMountWalkVelocity();
		}
		// Lateral or Backward Walk (currently, not used)
		else
			velocity *= 0.66f;//getMountWalkVelocity();
	}
	else
	{
		// Forward Run or Walk
		if(_FrontVelocity > 0.0f)
			velocity *= currentVelocity();
		// Lateral or Backward Walk
		else
			velocity *= _WalkVelocity;
	}
	return velocity;
}// getVelocity //

//-----------------------------------------------
// speed :
// Return the Entity Current Speed.
//-----------------------------------------------
double CUserEntity::speed() const	// virtual
{
	return CPlayerCL::speed();
//	return (double)getVelocity().norm();
}// speed //

//-----------------------------------------------
// applyMotion :
// Apply the motion to the entity.
//-----------------------------------------------
void CUserEntity::applyMotion(CEntityCL *target)
{
	// default each frame
	_ForceLookSlot= CLFECOMMON::INVALID_SLOT;

	bool lastHasMoved = _HasMoved;
	_HasMoved = false;
	// Remove Positions in stages
	_Stages.removePosWithNoMode();
	// Remove Positions in stages for the mount.
	CCharacterCL *mount = 0;
	if(parent() != CLFECOMMON::INVALID_SLOT)
	{
		mount = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(parent()));
		if(mount)
			mount->_Stages.removePosWithNoMode();
	}
	// NO PRIMITIVE -> NO MOVE
	if(_Primitive == 0)
		return;
	// BAD CONNECTION -> NO MOVE
	if(NetMngr.getConnectionQuality() == false)
		return;
	// MS per TICK <=0 -> NO MOVE
	if(NetMngr.getMsPerTick() <= 0)
		return;
	// Compute Speed Vector
	NLMISC::CVectorD speed;
	if(_MoveToSlot != CLFECOMMON::INVALID_SLOT)
	{
		// Check the Target.
		if(target == 0 || target == this)
			return;
		// Compute the vector between the user and the target.
		CVectorD dir2targ = target->pos() - pos();
		dir2targ.z = 0.0;
		if(dir2targ == CVectorD::Null)
			dir2targ = front();
		const double angToTarget = atan2(dir2targ.y, dir2targ.x);
		CVectorD aimingPos = target->getAttackerPos(angToTarget, attackRadius() + ClientCfg.AttackDist);
		// Aiming Position
		CVectorD dirToAimingPos = aimingPos-pos();
		dirToAimingPos.z = 0.0;
		const double distToAimingPos = dirToAimingPos.norm();

		// Decide if the target is reached or not
		bool	targetReached= false;
		if(distToAimingPos > 0.5)
		{
			// Because of Tryker Counter, may increase the Threshold, when the player is stalled
			if(distToAimingPos<_MoveToDist)
			{
				// If the player is stalled too much time, abort the move and launch the action
				float	actualSpeed= float((_Position - _LastFramePos).norm() / DT);

				// if player effectively runs twice slower, start colision timer
				if( actualSpeed*2 < getMaxSpeed() )
				{
					if(!_MoveToColStartTime)
						_MoveToColStartTime= T1;
				}
				// else ok, reset colision timer
				else
					_MoveToColStartTime= 0;

				// if too much time stalled, stop run.
				if(_MoveToColStartTime && (T1 - _MoveToColStartTime >= ClientCfg.MoveToTimeToStopStall) )
					targetReached= true;
			}
			else
				_MoveToColStartTime= 0;
		}
		else
			targetReached= true;

		// If the target is reached
		if(targetReached)
		{
			// stop and execute action
			speed = CVectorD::Null;
			moveToAction(target);
		}
		// else continue follow
		else
		{
			// Look at the entity. delay it after pacs evalCollision(), for correct orientation
			_ForceLookSlot= target->slot();
			// but still estimate now an approximative front (may be used between now and applyForceLook())
			forceLookEntity(dir2targ, false);

			// Normalize
			dirToAimingPos.normalize();
			// Set the Velocity Direction
			speed = dirToAimingPos*distToAimingPos;
			speed /= DT;
			if(speed.norm() > getMaxSpeed())
				speed = dirToAimingPos*getMaxSpeed();
		}
	}
	else if(follow())
	{
		// Check the Target.
		if(target == 0 || target == this)
			return;
		// If the target is moving, orientate the user to the target.
//		if(target->hasMoved())
		{
			// Compute the vector between the user and the target.
			CVectorD dir2targ = target->pos() - pos();
			dir2targ.z = 0.0;
			if(dir2targ != CVectorD::Null)
			{
				// Look at the entity. delay it after pacs evalCollision(), for correct orientation
				_ForceLookSlot= target->slot();
				// but still estimate now an approximative front (may be used between now and applyForceLook())
				forceLookEntity(dir2targ, false);
			}
		}
		// Compute the angle in the world to attack the target.
		const double angToTarget = frontYaw();
		// Get the best position to attack the target with the given angle.
		const CVectorD aimingPos = target->getAttackerPos(angToTarget, attackRadius() + ClientCfg.AttackDist);
		// Aiming Position
		CVectorD dirToAimingPos = aimingPos-pos();
		dirToAimingPos.z = 0.0;
		const double distToAimingPos = dirToAimingPos.norm();
		// If the User was not moving and the distance to re-move is not enough big, stay idle
		if(lastHasMoved
		|| (isFighting() && distToAimingPos >= 0.5)
		|| (!isFighting() && distToAimingPos >= 3.0))
		{
			dirToAimingPos.normalize();
			// User is in combat mode -> follow as close as possible.
			if(isFighting())
			{
				// Target is moving
				if(target->hasMoved())
				{
					// Get closer if too far.
					if(distToAimingPos >= 0.2)
					{
						// Set the Velocity Direction
						speed = dirToAimingPos*distToAimingPos;
						speed /= DT;
						if(speed.norm() > getMaxSpeed())
							speed = dirToAimingPos*getMaxSpeed();
					}
					else
					{
						// Try to have the same speed as the target.
						if(target->getSpeed() > getMaxSpeed())
							speed = target->dir()* ((float)getMaxSpeed());
						else
							speed = target->dir()* ((float)target->getSpeed());
					}
				}
				// Target is not moving.
				else
				{
					// Get closer if too far.
					if(distToAimingPos >= 0.1)
					{
						// Set the Velocity Direction
						speed = dirToAimingPos*distToAimingPos;
						speed /= DT;
						if(speed.norm() > getMaxSpeed())
							speed = dirToAimingPos*getMaxSpeed();
					}
					else
						speed = CVectorD::Null;
				}
			}
			// User is not in combat mode -> follow not so close.
			else
			{
				// Too far, get closer as fast as possible
				if(distToAimingPos >= 3.0)
				{
					// Set the Velocity Direction
					speed = dirToAimingPos*distToAimingPos;
					speed /= DT;
					if(speed.norm() > getMaxSpeed())
						speed = dirToAimingPos*getMaxSpeed();
				}
				// Just far enough, adjust the user speed to the target
				else if(target->hasMoved() && distToAimingPos >= 1.0)
				{
					// Try to have the same speed as the target.
					if(target->getSpeed() > getMaxSpeed())
						speed = target->dir()* ((float)getMaxSpeed());
					else
						speed = target->dir()* ((float)target->getSpeed());
				}
				// Too close, Stop
				else
					speed = CVectorD::Null;
			}
		}
		// User was stop and the next pos is to close to begin to move.
		else
			speed = CVectorD::Null;
	}
	else
		speed = getVelocity()*_SpeedFactor.getValue();
	// SPEED VECTOR NULL -> NO MOVE
	if(speed == CVectorD::Null)
		return;

	// First Person View
	if(UserControls.isInternalView())
	{
		// If the server is slow, the client move slower too (only needed in FPV).
		double modif = (100.0f/(float)NetMngr.getMsPerTick());
		// don't increase speed
		clamp(modif, 0.0, 1.0);
		speed *= modif;
		// Move
		_HasMoved = true;
		_Primitive->move(speed, dynamicWI);
		if(mount && mount->getPrimitive())
			mount->getPrimitive()->move(speed, dynamicWI);
	}
	// Third Person View
	else
	{
		speed += pos();
		sint64 x = (sint64)((sint32)(speed.x * 1000.0));
		sint64 y = (sint64)((sint32)(speed.y * 1000.0));
		sint64 z = (sint64)((sint32)(speed.z * 1000.0));
		const uint time = 10;	// = 1sec because the speed is in Meter per Sec.
		_Stages.addStage(NetMngr.getCurrentClientTick()+time, CLFECOMMON::PROPERTY_POSX, x, 0);
		_Stages.addStage(NetMngr.getCurrentClientTick()+time, CLFECOMMON::PROPERTY_POSY, y);
		_Stages.addStage(NetMngr.getCurrentClientTick()+time, CLFECOMMON::PROPERTY_POSZ, z);
		// Move the Mount
		if(mount)
		{
			mount->_Stages.addStage(NetMngr.getCurrentClientTick()+time, CLFECOMMON::PROPERTY_POSX, x, 0);
			mount->_Stages.addStage(NetMngr.getCurrentClientTick()+time, CLFECOMMON::PROPERTY_POSY, y);
			mount->_Stages.addStage(NetMngr.getCurrentClientTick()+time, CLFECOMMON::PROPERTY_POSZ, z);
		}
	}
}// applyMotion //


//---------------------------------------------------
//---------------------------------------------------
void CUserEntity::applyForceLook()
{
	if(_ForceLookSlot!=CLFECOMMON::INVALID_SLOT)
	{
		CEntityCL *target= EntitiesMngr.entity(_ForceLookSlot);
		if(target && target!=this)
		{
			// Compute the vector between the user and the target.
			CVectorD dir2targ = target->pos() - pos();
			dir2targ.z = 0.0;
			if(dir2targ == CVectorD::Null)
				dir2targ = front();
			// Look at the entity
			forceLookEntity(dir2targ, true);
		}
	}
}


//---------------------------------------------------
// updatePosCombatFloatChanged :
//---------------------------------------------------
void CUserEntity::updatePosCombatFloatChanged(CEntityCL * /* target */)	// virtual
{
	if(viewMode() == FirstPV)
	{
		pos(_FirstPos);
	}
}// updatePosCombatFloatChanged //


//-----------------------------------------------
// forceIndoorFPV //
// Return true if the user is indoor and the CFG want to force the FPV Indoor.
//-----------------------------------------------
bool CUserEntity::forceIndoorFPV()
{
	return (ClientCfg.ForceIndoorFPV && indoor());
}// forceIndoorFPV //


//-----------------------------------------------
// enableFollow :
//-----------------------------------------------
void CUserEntity::disableFollow()
{
	if (_FollowMode==false)
		return;

	_FollowMode = false;

	// Send the message to the server.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("TARGET:NO_FOLLOW", out))
		NetMngr.push(out);
	else
		nlwarning("UE:follow: unknown message named 'TARGET:NO_FOLLOW'.");

}// follow //


//-----------------------------------------------
// enableFollow :
//-----------------------------------------------
void CUserEntity::enableFollow(bool resetCameraRot)
{
	if (_FollowMode == true)
		return;

	if( _Mode == MBEHAV::DEATH )
		return;

	_FollowMode = true;

	// Send the message to the server.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("TARGET:FOLLOW", out))
		NetMngr.push(out);
	else
		nlwarning("UE:follow: unknown message named 'TARGET:FOLLOW'.");

	// Disable any autowalk (else when follow re-disabled, it will effect, which is weird)
	UserControls.autowalkState(false);

	// disable afk mode
	setAFK(false);

	// if want to reset camera rotation
	if(resetCameraRot)
		startForceLookEntity(targetSlot());

}// follow //


//-----------------------------------------------
// resetAnyMoveTo
//-----------------------------------------------
void CUserEntity::resetAnyMoveTo()
{
	// if we cancel a MoveToPhrase action, MUST dec the action counter, and hide the Action Icon
	if(_MoveToAction==CUserEntity::CombatPhrase || _MoveToAction==CUserEntity::ExtractRM)
	{
		// the clientExecute has not been called in case of "ExtractRM autoFind"
		bool	autoFindExtractRM= _MoveToAction==CUserEntity::ExtractRM && _MoveToPhraseMemoryLine == std::numeric_limits<uint>::max();
		if(!autoFindExtractRM)
		{
			CSPhraseManager	*pPM= CSPhraseManager::getInstance();
			pPM->cancelClientExecute(_MoveToPhraseCyclic);
		}
	}

	// reset to no moveTo
	_MoveToSlot = CLFECOMMON::INVALID_SLOT;
	_MoveToAction = CUserEntity::None;
	_MoveToDist = 0.0;
	_MoveToColStartTime= 0;
}

//-----------------------------------------------
// moveToCheckStartDist :
// Check if the user is not already well placed.
//-----------------------------------------------
void CUserEntity::moveToCheckStartDist(CLFECOMMON::TCLEntityId slot, double dist, TMoveToAction /* action */)
{
	if(_MoveToSlot != CLFECOMMON::INVALID_SLOT)
	{
		// Start a new Force Look entity
		startForceLookEntity(_MoveToSlot);

		// Disable any autowalk (else when moveTo re-disabled, it will effect, which is weird)
		UserControls.autowalkState(false);

		// disable afk mode
		setAFK(false);

		// if sufficiently near, launch the action
		CEntityCL *target = EntitiesMngr.entity(slot);
		if(target)
		{
			CVectorD dir2targ = target->pos() - pos();
			dir2targ.z = 0.0;
			if((dir2targ==CVectorD::Null) || (dir2targ.norm() < dist))
			{
				moveToAction(target);
			}
		}
	}
}// moveToCheckStartDist //

//-----------------------------------------------
// moveTo :
// Method to move to someone else.
//-----------------------------------------------
void CUserEntity::moveTo(CLFECOMMON::TCLEntityId slot, double dist, TMoveToAction action)
{
	resetAnyMoveTo();

	// setup new state
	_MoveToSlot = slot;
	_MoveToDist = dist;
	_MoveToAction = action;

	moveToCheckStartDist(_MoveToSlot, _MoveToDist, _MoveToAction);
}// moveTo //

//-----------------------------------------------
// moveToMission :
// Method to move to someone else for a mission. NB: if prec action was a CombatPhrase action, action counter are decremented
//-----------------------------------------------
void CUserEntity::moveToMission(CLFECOMMON::TCLEntityId slot, double dist, uint32 id)
{
	resetAnyMoveTo();

	// setup new state
	_MoveToSlot = slot;
	_MoveToDist = dist;
	_MoveToAction = CUserEntity::Mission;
	_MoveToMissionId = id;

	moveToCheckStartDist(_MoveToSlot, _MoveToDist, _MoveToAction);
}// moveToMission //

//-----------------------------------------------
// moveToMissionRing :
// Method to move to someone else for a ring mission. NB: if prec action was a CombatPhrase action, action counter are decremented
//-----------------------------------------------
void CUserEntity::moveToMissionRing(CLFECOMMON::TCLEntityId slot, double dist, uint32 id)
{
	resetAnyMoveTo();

	// setup new state
	_MoveToSlot = slot;
	_MoveToDist = dist;
	_MoveToAction = CUserEntity::MissionRing;
	_MoveToMissionId = id;

	moveToCheckStartDist(_MoveToSlot, _MoveToDist, _MoveToAction);
}// moveToMissionRing //

//-----------------------------------------------
// moveTo :
// Method to move to someone else.
//-----------------------------------------------
void CUserEntity::moveToCombatPhrase(CLFECOMMON::TCLEntityId slot, double dist, uint phraseMemoryLine, uint phraseMemorySlot, bool phraseCyclic)
{
	resetAnyMoveTo();

	// setup new state
	_MoveToSlot = slot;
	_MoveToDist = dist;
	_MoveToAction = CUserEntity::CombatPhrase;
	_MoveToPhraseMemoryLine= phraseMemoryLine;
	_MoveToPhraseMemorySlot= phraseMemorySlot;
	_MoveToPhraseCyclic= phraseCyclic;

	moveToCheckStartDist(_MoveToSlot, _MoveToDist, _MoveToAction);
}

//-----------------------------------------------
// Method to move to someone else, for foraging extraction
// The caller MUST call after CSPhraseManager::clientExecute(), to increment action counter
//-----------------------------------------------
void CUserEntity::moveToExtractionPhrase(CLFECOMMON::TCLEntityId slot, double dist, uint phraseMemoryLine, uint phraseMemorySlot, bool cyclic)
{
	// Test if forage tool in hand, otherwise auto-equip with it
	bool validForageToolInHand = false;
	CInventoryManager * inv = CInventoryManager::getInstance();
	if ( !inv )
	{
		return;
	}
	uint32 rightHandSheet = inv->getRightHandItemSheet();
	if ( rightHandSheet )
	{
		validForageToolInHand = inv->isForageToolItem( rightHandSheet );
	}
	if ( !validForageToolInHand )
	{
		// Find a forage tool in the bag
		uint i;
		for ( i=0; i<MAX_BAGINV_ENTRIES; ++i )
		{
			uint32 itemSheet = inv->getBagItem(i).getSheetID();
			if ( itemSheet && inv->isForageToolItem(itemSheet) )
			{
				break;
			}
		}
		if ( i != MAX_BAGINV_ENTRIES && ClientCfg.AutoEquipTool )
		{
			// remember last used weapon(s)
			rememberWeaponsInHand();

			// Clear hands
			inv->unequip( "LOCAL:INVENTORY:HAND:1" );
			inv->unequip( "LOCAL:INVENTORY:HAND:0" );

			// Equip hands
			string bagPath = toString( "LOCAL:INVENTORY:BAG:%u", i );
			inv->equip( bagPath, "LOCAL:INVENTORY:HAND:0" );
		}
		else
		{
			// No forage tool in bag
			CInterfaceManager::getInstance()->displaySystemInfo( CI18N::get("uiForageToolMissing"), "CHK" );
			return;
		}
	}

	resetAnyMoveTo();

	// setup new state
	_MoveToSlot = slot;
	_MoveToDist = dist;
	_MoveToAction = CUserEntity::ExtractRM;
	_MoveToPhraseMemoryLine= phraseMemoryLine;
	_MoveToPhraseMemorySlot= phraseMemorySlot;
	_MoveToPhraseCyclic= cyclic;

	moveToCheckStartDist(_MoveToSlot, _MoveToDist, _MoveToAction);
}

//-----------------------------------------------
// Method to begin a spire construction
// The caller MUST call after CSPhraseManager::clientExecute(), to increment action counter
//-----------------------------------------------
void CUserEntity::moveToTotemBuildingPhrase(CLFECOMMON::TCLEntityId slot, double dist, uint phraseMemoryLine, uint phraseMemorySlot, bool cyclic)
{
	resetAnyMoveTo();

	// setup new state
	_MoveToSlot = slot;
	_MoveToDist = dist;
	_MoveToAction = CUserEntity::BuildTotem;
	_MoveToPhraseMemoryLine= phraseMemoryLine;
	_MoveToPhraseMemorySlot= phraseMemorySlot;
	_MoveToPhraseCyclic= cyclic;

	moveToCheckStartDist(_MoveToSlot, _MoveToDist, _MoveToAction);
}

//-----------------------------------------------
// moveToAction :
// Launch the Action Once the Move is done.
// \param CEntityCL * : pointer on the destination entity.
// \warning entity pointer must be valid(allocated).
//-----------------------------------------------
void CUserEntity::moveToAction(CEntityCL *ent)
{
	// Check entity pointer
	nlassert(ent);

	UserControls.needReleaseForward();

	// Get the interface instance.
	CInterfaceManager *IM = CInterfaceManager::getInstance();
	switch(_MoveToAction)
	{
	// Attack
	case CUserEntity::Attack:
		UserEntity->attack();
		break;
	// Quartering
	case CUserEntity::Quarter:
		if((ent->properties()).harvestable())
			CAHManager::getInstance()->runActionHandler("context_quartering", 0);
		break;
	// Loot
	case CUserEntity::Loot:
		if((ent->properties()).lootable())
			CAHManager::getInstance()->runActionHandler("context_loot", 0);
		break;
	// Pick Up
	case CUserEntity::PickUp:
		nlwarning("UE:checkMoveTo: not yet implemented");
		break;
	case CUserEntity::ExtractRM:
		extractRM();
		break;
	// Trade Item
	case CUserEntity::TradeItem:
		CAHManager::getInstance()->runActionHandler("context_trade_item", 0);
		break;
	// Trade Phrase
	case CUserEntity::TradePhrase:
		CAHManager::getInstance()->runActionHandler("context_trade_phrase", 0);
		break;
	// Trade Pact
	case CUserEntity::TradePact:
		CAHManager::getInstance()->runActionHandler("context_trade_pact", 0);
		break;
	// Mission
	case CUserEntity::Mission:
		{
			string param = toString("id=%d", _MoveToMissionId);
			CAHManager::getInstance()->runActionHandler("mission_option", 0, param);
		}
		break;
	// Dynamic Mission
	case CUserEntity::DynamicMission:
		CAHManager::getInstance()->runActionHandler("context_dynamic_mission", 0);
		break;
	// Static Mission
	case CUserEntity::StaticMission:
		CAHManager::getInstance()->runActionHandler("context_choose_mission", 0);
		break;
	// Mission
	case CUserEntity::MissionRing:
		{
			string param = toString("id=%d", _MoveToMissionId);
			CAHManager::getInstance()->runActionHandler("mission_ring", 0, param);
		}
		break;
	// Create Guild
	case CUserEntity::CreateGuild:
		CAHManager::getInstance()->runActionHandler("context_create_guild", 0);
		break;
	// News
	case CUserEntity::News:
		CAHManager::getInstance()->runActionHandler("context_talk", 0);
		break;
	// Trade Teleport
	case CUserEntity::TradeTeleport:
		CAHManager::getInstance()->runActionHandler("context_trade_teleport", 0);
		break;
	// Trade Faction items
	case CUserEntity::TradeFaction:
		CAHManager::getInstance()->runActionHandler("context_trade_faction", 0);
		break;
	// Trade Cosmetic
	case CUserEntity::TradeCosmetic:
		CAHManager::getInstance()->runActionHandler("context_trade_cosmetic", 0);
		break;
	// Talk
	case CUserEntity::Talk:
		nlwarning("UE:checkMoveTo: not yet implemented");
		break;
	// CombatPhrase
	case CUserEntity::CombatPhrase:
		UserEntity->attackWithPhrase();
		break;
	// Mount
	case CUserEntity::Mount:
		{
			string orderStr = "mount";
			string beastIndexStr = "@UI:GCM_BEAST_SELECTED";
			bool confirmFree = true;
			beastOrder(orderStr,beastIndexStr,confirmFree);
			break;
		}
	// WebPage
	case CUserEntity::WebPage:
		CAHManager::getInstance()->runActionHandler("context_web_page", 0);
		break;
	// Outpost
	case CUserEntity::Outpost:
		CLuaManager::getInstance().executeLuaScript("game:outpostBCOpenStateWindow()", 0);
		break;
	// BuildTotem
	case CUserEntity::BuildTotem:
		buildTotem();
		break;
	default:
		break;
	}

	// Move To Done.
	resetAnyMoveTo();
}// moveToAction //


//-----------------------------------------------
// sendToServer :
// Send the position and orientation to the server.
//-----------------------------------------------
bool CUserEntity::sendToServer(CBitMemStream &out)
{
	if(GenericMsgHeaderMngr.pushNameToStream("POSITION", out))
	{
		// Backup the position sent.
		if (_Primitive) _Primitive->getGlobalPosition(_LastGPosSent, dynamicWI);
		// Send Position & Orientation
		CPositionMsg positionMsg;
		positionMsg.X = (sint32)(pos().x * 1000);
		positionMsg.Y = (sint32)(pos().y * 1000);
		positionMsg.Z = (sint32)(pos().z * 1000);
		positionMsg.Heading = frontYaw();
		out.serial(positionMsg);
		return true;
	}
	else
	{
		nlwarning("UE:sendToServer: unknown message named 'POSITION'.");
		return false;
	}
}// sendToServer //

//-----------------------------------------------
// msgForCombatPos :
// Fill the msg to know if the user is well placed to fight.
//-----------------------------------------------
bool CUserEntity::msgForCombatPos(NLMISC::CBitMemStream &out)
{
	static bool wellPosition = false;
	bool wellP = false;
//	if(isFighting())
	{
		// Is the user well Placed
		CEntityCL *target = EntitiesMngr.entity(UserEntity->targetSlot());
		if(target)
			wellP = target->isPlacedToFight(UserEntity->pos(), front(), attackRadius() + ClientCfg.AttackDist);
	}
	// If different from the last time
	if(wellPosition != wellP)
	{
		wellPosition = wellP;
		// Send state to server.
		if(GenericMsgHeaderMngr.pushNameToStream("COMBAT:VALIDATE_MELEE", out))
		{
			uint8 flag = wellP?1:0;
			out.serial(flag);
			return true;
		}
		else
			nlwarning("UE:msgForCombatPos: unknown message named 'COMBAT:TOGGLE_COMBAT_FLOAT_MODE'.");
	}
	// Do not send the msg.
	return false;
}// msgForCombatPos //


//-----------------------------------------------
// checkPos :
// Check the User Position according to the server code.
// \todo GUIGUI : put checks on GPos
// \todo GUIGUI : refact the check primitive when creating a PACS again
//-----------------------------------------------
void CUserEntity::checkPos()
{
	if(PACS && _Primitive)
	{
		// Is in water ?
		if(GR)
		{
			UGlobalPosition gPos;
			_Primitive->getGlobalPosition(gPos, dynamicWI);
			float waterHeight;
			if(GR->isWaterPosition(gPos, waterHeight))
			{
				if(isSwimming() == false)
				{
					// Player is Dead set the right mode (swim and dead)
					if(isDead())
						mode(MBEHAV::SWIM_DEATH);
					else
					{
						// Do not swim when in combat mode.
						if(isFighting())
						{
							_HasMoved = false;
							pacsPos(_LastPositionValidated, _LastGPosValidated);
						}
						// \todo GUIGUI : if the move was cancelled, do not switch to swimming mode.
						else if (isRiding())
						{
							mode(MBEHAV::MOUNT_SWIM);
							// also change mounted entity mode
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
						else
						{
							mode(MBEHAV::SWIM);
						}
					}
				}
			}
			else
			{
				if(isSwimming())
				{
					if(isDead())
					{
						mode(MBEHAV::DEATH);
					}
					else if (isRiding())
					{
						mode(MBEHAV::MOUNT_NORMAL);
						// also change mounted entity mode
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
					else
					{
						mode(MBEHAV::NORMAL);
					}
				}
			}
		}

		// Create the Primitive used to check if the move will be accepted by the server
		if(_CheckPrimitive == 0)
		{
			_Primitive->getGlobalPosition(_LastGPosSent, dynamicWI);
			_Primitive->getGlobalPosition(_LastGPosValidated, dynamicWI);
			_CheckPrimitive = PACS->addNonCollisionablePrimitive(_Primitive);
			_CheckPrimitive->setOcclusionMask(MaskColNone);	// Collide with nothing
			_CheckPrimitive->setCollisionMask(MaskColNone);	// Collide with nothing
			_LastPositionValidated = pos();
		}
		if(_CheckPrimitive)
		{
			_CheckPrimitive->setGlobalPosition(_LastGPosSent, dynamicWI);
			CVectorD speed = _Primitive->getFinalPosition(dynamicWI) - _CheckPrimitive->getFinalPosition(dynamicWI);

			_CheckPrimitive->move(speed, dynamicWI);
			if(PACS->evalNCPrimitiveCollision(1.0, _CheckPrimitive, dynamicWI) == false)
				nlwarning("UE:checkPos: _CheckPrimitive is a collisionable primitive !!!");

			CVectorD vectDist = _Primitive->getFinalPosition(dynamicWI) - _CheckPrimitive->getFinalPosition(dynamicWI);
			if(vectDist.norm() > 0.001)
			{
				// The server will not be able to reproduce this move, restoring the last one.
				_HasMoved = false;
				pacsPos(_LastPositionValidated,_LastGPosValidated);
			}
			else
			{
				_LastPositionValidated = _Primitive->getFinalPosition(dynamicWI);
				_Primitive->getGlobalPosition(_LastGPosValidated, dynamicWI);
			}
		}
	}
}// checkPos //


//-----------------------------------------------
// testPacsPos :
// test the move from posToTest to current pos
//-----------------------------------------------
bool CUserEntity::testPacsPos( CVectorD& posToTest )
{
	if(PACS && _Primitive && _CheckPrimitive)
	{
		_CheckPrimitive->setGlobalPosition(posToTest, dynamicWI);
		CVectorD speed = _Primitive->getFinalPosition(dynamicWI) - _CheckPrimitive->getFinalPosition(dynamicWI);

		_CheckPrimitive->move(speed, dynamicWI);
		if(PACS->evalNCPrimitiveCollision(1.0, _CheckPrimitive, dynamicWI) == false)
			nlwarning("UE:testPacsPos: _CheckPrimitive is a collisionable primitive !!!");

		CVectorD vectDist = _Primitive->getFinalPosition(dynamicWI) - _CheckPrimitive->getFinalPosition(dynamicWI);
		if(vectDist.norm() > 0.001)
		{
			return false;
		}
		else
			return true;
	}
	return false;

} // testPacsPos //


//-----------------------------------------------
// tp :
// Teleport the player (remove selection, re-init checkPos, etc.).
//-----------------------------------------------
void CUserEntity::tp(const CVectorD &dest)
{
	// Remove the selection.
	UserEntity->selection(CLFECOMMON::INVALID_SLOT);
	// Update PACS
	pacsPos(dest);
	// Update the primitive useful to check the user position.
	_LastPositionValidated = dest;
	// Get the Global position
	if(_Primitive)
	{
		// this is the last PACS position validated too
		_Primitive->getGlobalPosition(_LastGPosValidated, dynamicWI);
		// consider this is the last position sent to server (since actually received!)
		_Primitive->getGlobalPosition(_LastGPosSent, dynamicWI);
		// Set the new position of the 'check' primitive
		if(_CheckPrimitive)
			_CheckPrimitive->setGlobalPosition(_LastGPosSent, dynamicWI);
	}
	else
		nlwarning("UE:tp: the entity has a Null primitive.");
	// Return to interface mode.
	viewMode(UserEntity->viewMode());
	// User well oriented.
	dir(UserEntity->front());
	// Set Normal Mode after a teleport.
	mode(MBEHAV::NORMAL);
	// Set animation with idle.
	setAnim(CAnimationStateSheet::Idle);
}// tp //

//-----------------------------------------------
// correctPos :
// Teleport the player to correct his position.
//-----------------------------------------------
void CUserEntity::correctPos(const NLMISC::CVectorD &dest)
{
	nlinfo("UE:correctPos: new user position %f %f %f", dest.x, dest.y, dest.z);
	// Change the user poisition.
	UserEntity->pacsPos(dest);
	// Update the primitive useful to check the user position.
	_LastPositionValidated = dest;
	// Get the Global position
	if(_Primitive)
	{
		// this is the last PACS position validated too
		_Primitive->getGlobalPosition(_LastGPosValidated, dynamicWI);
		// consider this is the last position sent to server (since actually received!)
		_Primitive->getGlobalPosition(_LastGPosSent, dynamicWI);
		// Set the new position of the 'check' primitive
		if(_CheckPrimitive)
			_CheckPrimitive->setGlobalPosition(_LastGPosSent, dynamicWI);
	}
	else
		nlwarning("UE:correctPos: the entity has a Null primitive.");

	// Correct the pos of the mount, if riding
	if ( isRiding() )
	{
		if ( _Mount < EntitiesMngr.entities().size() )
		{
			CEntityCL *mount = EntitiesMngr.entities()[_Mount];
			if ( mount )
				mount->pacsPos( dest );
		}
	}
}// correctPos //


/*
 * Check if the mount is able to run, and force walking mode if not
 * (if the player clicked on run, set back to walk).
 */
void CUserEntity::checkMountAbleToRun()
{
	if ( isRiding() )
	{
		if ( running() )
		{
			// Make the mount walk if she's hungry
			if ( ! _MountHunger.canRun() )
				switchVelocity( false );
		}
		else
		{
			// Make the mount run if she's not hungry anymore
			if ( _RunWhenAble && _MountHunger.canRun() )
				switchVelocity( false );
		}
	}
}


//-----------------------------------------------
// Update the position of the entity after the motion.
// \param t : Time for the position of the entity after the motion.
// \param target : pointer on the current target.
//-----------------------------------------------
void CUserEntity::updatePos(const TTime &t, CEntityCL *target)
{
	// Update Mount if mounting (if _Mount was still not valid when mode received)
	if((_Mount != CLFECOMMON::INVALID_SLOT && isRiding()) && _OnMount==false)
	{
		setOnMount();
	}
	// External view are managed like other entities.
	if(!UserControls.isInternalView())
	{
		// Update position according to the animations.
		CPlayerCL::updatePos(t, target);
		_LastFrameTime = t*0.001;
		// Set The FPV when indoor.
		if(forceIndoorFPV())
			viewMode(FirstPV, false);
		return;
	}
	/*
	else
	{
		// update anim state at least (needed when a spell is cast to play the cast fx at the right time, because they may be visible)
		updateAnimationState();
	}*/

	// Compute the Time Step.
	double frameTimeRemaining = computeTimeStep(((double)t)*0.001);
	// Do not update animation if Client Light
	if (!ClientCfg.Light)
	{
		// Attack Animation.
		if(_AnimAttackOn)
		{
			if(animIndex(MOVE) != ::CAnimation::UnknownAnim)
			{
				if(animOffset(MOVE) >= EAM->getAnimationLength(animId(MOVE)))
				{
					_AnimAttackOn = false;
					updateVisualDisplay();
				}
			}
		}

		// Increase the time in the current animation.
		double previousTimeOffset = animOffset(MOVE);
		double offset = previousTimeOffset + frameTimeRemaining;
		// Check Anim length
		double animLength = EAM->getAnimationLength(animId(MOVE));
		if(offset > animLength)
			animOffset(MOVE, animLength);
		else
			animOffset(MOVE, offset);
		// Manage Events that could be created by the animation (like sound).
		animEventsProcessing(previousTimeOffset, animOffset(MOVE));
	}
	// Get the right pos from PACS.
	if(_Primitive && GR)
	{
		UGlobalPosition gPos;
		_Primitive->getGlobalPosition(gPos, dynamicWI);

		// Set the new current pos.
		pos(GR->getGlobalPosition(gPos));
	}
	// Set the direction.
	if( ClientCfg.AutomaticCamera )
	{
		//dir(front());
	}
	// Reset the TPV when outdoor if needed.
	if(_Primitive && GR)
	{
		UGlobalPosition gPos;
		_Primitive->getGlobalPosition(gPos, dynamicWI);
		if(!GR->isInterior(gPos))
			if(!ClientCfg.FPV)
				viewMode(ThirdPV, false);
	}
	// Current time is now the entity time too.
	_LastFrameTime = t*0.001;
}// updatePos //

//-----------------------------------------------
// updateDisplay :
// Update the PACS position after the evalCollision. The entity position is set too. This is fast.
// If the entity position is too far from its PACS position, setGlobalPosition is called.
// After this call, the position.z is valid.
//-----------------------------------------------
void CUserEntity::pacsFinalizeMove()	// virtual
{
	if(_Primitive == 0)
		return;

	// Get the global position
	_FinalPacsPos = _Primitive->getFinalPosition(dynamicWI);

	// Get the global position
	UGlobalPosition gPos;
	_Primitive->getGlobalPosition(gPos, dynamicWI);
	if(gPos.InstanceId != -1)
	{
		pos(_FinalPacsPos);
		if(_Mount != CLFECOMMON::INVALID_SLOT)
		{
			CEntityCL *mount = EntitiesMngr.entity(_Mount);
			if(mount)
				mount->pacsPos(pos());
		}
	}
	else
		_SetGlobalPositionDone = false;
}// pacsFinalizeMove //


//-----------------------------------------------
// applyBehaviour :
// Apply the behaviour for the user.
//-----------------------------------------------
void CUserEntity::applyBehaviour(const CBehaviourContext &behaviourContext)	// virtual
{
	const MBEHAV::CBehaviour &behaviour = behaviourContext.Behav;
	// Special code for the First Person View
	if((viewMode() == FirstPV) && behaviour.isCombat())
	{
		// Backup the current behaviour.
		_CurrentBehaviour = behaviourContext.Behav;
		dir(front());

		// check if self-target
		bool selfSpell = false;
		if (behaviourContext.Targets.Targets.size() == 1)
		{
			if (behaviourContext.Targets.Targets[0].TargetSlot == 0)
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

		// Default target hit dates
		static	vector<double>	targetHitDates;
		targetHitDates.clear();
		targetHitDates.resize(behaviourContext.Targets.Targets.size(), TimeInSec);

 		// cast projectiles/impact linked to behaviour
		updateCurrentAttack();
		if (isCurrentBehaviourAttackEnd())
		{
			// In First Person (don't care), use a default animation => will choose a default delay of 0.5
			performCurrentAttackEnd(behaviourContext, selfSpell && isOffensif,
				targetHitDates, CAnimationStateSheet::UnknownState);
		}
		//
		_RightFXActivated	= false;
		_LeftFXActivated	= false;
		if(EAM)
		{
			animState (MOVE, CAnimationStateSheet::FirstPersonAttack);
			animIndex (MOVE, CAnimation::UnknownAnim);
			animOffset(MOVE, 0.0);
			_CurrentState = EAM->mState(_CurrentAutomaton, animState(MOVE));
			if(_CurrentState && _CurrentAnimSet[MOVE])
			{
				const CAnimationState *animStatePtr = _CurrentAnimSet[MOVE]->getAnimationState(animState(MOVE));
				if(animStatePtr)
				{
					animIndex(MOVE, animStatePtr->chooseAnim(_AnimJobSpecialisation, people(), getGender(), 0.0));
					if(animIndex(MOVE) != CAnimation::UnknownAnim)
					{
						if(_PlayList)
						{
							_PlayList->setAnimation  (MOVE, animId(MOVE));
							_PlayList->setTimeOrigin (MOVE, ryzomGetLocalTime ()*0.001);//_LastFrameTime);
							_AnimAttackOn = true;
							updateVisualDisplay();
						}
					}
				}
			}
		}
		// Start FX to play at the animation beginning like trails.
		if(_CurrentBehaviour.Behaviour == MBEHAV::RANGE_ATTACK)
		{
			startItemAttackFXs(_CurrentBehaviour.Range.ImpactIntensity != 0, _CurrentBehaviour.Range.ImpactIntensity);
		}
		else
		{
			startItemAttackFXs(_CurrentBehaviour.Combat.ImpactIntensity != 0 && _CurrentBehaviour.Combat.HitType != HITTYPE::Failed, _CurrentBehaviour.Combat.ImpactIntensity);
		}
		// DeltaHP
		applyBehaviourFlyingHPs(behaviourContext, behaviour, targetHitDates);
	}
	// In third person view (or camera mode), play the same way than for the others.
	else
		CPlayerCL::applyBehaviour(behaviourContext);
}// applyBehaviour //

//---------------------------------------------------
// setDead :
// Method to Flag the character as dead and do everything needed.
//---------------------------------------------------
void CUserEntity::setDead()	// virtual
{
	// User is dead -> NO FOLLOW
	disableFollow();
	// Remove the selection.
	UserEntity->selection(CLFECOMMON::INVALID_SLOT);
	// Death Mode Control and view.
	UserControls.mode(CUserControls::DeathMode);
	// Play the FX for the death
	NL3D::UParticleSystemInstance deathFX = FXMngr.instantFX(ClientCfg.DeadFXName);
	if(!deathFX.empty())
		deathFX.setPos(selectBox().getCenter());

	// Last teleport is a death

	// get player's kami fame
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	sint8 kamiFame = 0;
	uint kamiFameIndex = CStaticFames::getInstance().getFactionIndex("kami");
	if (pIM && kamiFameIndex != CStaticFames::INVALID_FACTION_INDEX)
	{
		CCDBNodeLeaf *pLeafKamiFame = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:FAME:PLAYER%d:VALUE", kamiFameIndex - 1), false);
		if (pLeafKamiFame != NULL)
			kamiFame = pLeafKamiFame->getValue8();
	}

	// get player's karavan fame
	sint8 karavanFame = 0;
	uint karavanFameIndex = CStaticFames::getInstance().getFactionIndex("karavan");
	if (pIM && karavanFameIndex != CStaticFames::INVALID_FACTION_INDEX)
	{
		CCDBNodeLeaf *pLeafKaravanFame = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:FAME:PLAYER%d:VALUE", karavanFameIndex - 1), false);
		if (pLeafKaravanFame != NULL)
			karavanFame = pLeafKaravanFame->getValue8();
	}

	// set loading background depending on player's faction fame
	if (kamiFame > karavanFame)
		LoadingBackground = ResurectKamiBackground;
	else
		LoadingBackground = ResurectKaravanBackground;

	// disable or enable shadowing
	updateCastShadowMap();

	// enable death warning popup
	//CInterfaceManager * pIM = CInterfaceManager::getInstance();
	if( pIM )
	{
		CAHManager::getInstance()->runActionHandler("set",NULL,"dblink=UI:VARIABLES:DEATH_WARNING_WANTED|value=1");
	}
}// setDead //

//-----------------------------------------------
// skillUp :
// Skill Up
//-----------------------------------------------
void CUserEntity::skillUp()
{
	// Play the FX for the death
	NL3D::UParticleSystemInstance skillUpFX = FXMngr.instantFX(ClientCfg.SkillUpFXName);
	skillUpFX.setClusterSystem(getClusterSystem());
	if(!skillUpFX.empty())
		skillUpFX.setPos(pos());
}// skillUp //

//-----------------------------------------------
// startColTimer :
// After a few time, if the user is still in collision with someone else, remove collisions with other entitites.
//-----------------------------------------------
void CUserEntity::startColTimer()
{
	if(_ColOn)
	{
		if(_ColRemoved==false)
		{
			if((T1-_ColStartTime) > ClientCfg.TimeToRemoveCol)
			{
				EntitiesMngr.removeColUserOther();
				_ColRemoved = true;
			}
		}
	}
	else
	{
		_ColOn = true;
		_ColStartTime = T1;
	}
}// startColTimer //

//-----------------------------------------------
// stopColTimer :
// Called when the user is no more in collision with another entity.
//-----------------------------------------------
void CUserEntity::stopColTimer()
{
	// Restore collisions.
	if(_ColRemoved)
	{
		EntitiesMngr.restoreColUserOther();
		_ColRemoved = false;
	}
	_ColOn = false;
}// stopColTimer //



//-----------------------------------------------
// getMaxSpeed
// Return the current max speed for the entity in meter per sec
// The method return a max according to the speed factor (given by the server)
// It's also return a value according to the landscape (water)
// Also managed mounts
//-----------------------------------------------
double CUserEntity::getMaxSpeed()	const // virtual
{
	// Max Defined speed according to the current factor (slow, or speed increased)
	double maxSpeed = _SpeedFactor.getValue();
	// User is Dead
	if(isDead())
	{
		maxSpeed *= 0.0;
	}
	// User is sitting
	else if(isSit())
	{
		maxSpeed *= 0.0;
	}
	// User is swimming
	else if(isSwimming())
	{

		// We are a Ring DM so speed up a litle
		if (_R2CharMode == R2::TCharMode::Editer || _R2CharMode == R2::TCharMode::Dm)
		{
			maxSpeed *= ClientCfg.DmRun / 2;
		}
		else
		{
			// Run
			maxSpeed *= 3.0;
		}
	}
	else if(isRiding())
	{
		// Run if mount not hungry, otherwise, use walk velocity
		if (_MountHunger.canRun())
			maxSpeed *= (double)getMountRunVelocity();
		else
			maxSpeed *= (double)getMountWalkVelocity();
	}
	else
	{
		// Run
		maxSpeed *= runVelocity();
	}
	// Return the current max
	return maxSpeed;
//	return ClientCfg.Run * _SpeedFactor.getValue();
}// getMaxSpeed //


//-----------------------------------------------
// snapToGround :
// Snap the user to the ground.
//-----------------------------------------------
void CUserEntity::snapToGround()
{
	CEntityCL::snapToGround();

	updateSound (ryzomGetLocalTime ());
}// // snapToGround //


//-----------------------------------------------
// updateSound :
//-----------------------------------------------
void CUserEntity::updateSound(const TTime &time)
{
	H_AUTO_USE ( RZ_Client_Update_Sound );

	// no sound manager, no need to update sound
	if (SoundMngr == 0)
		return;

	if (!(StereoHMD && true)) // TODO: ClientCfg.Headphone
	{
		// NOTE: StereoHMD+Headphone impl in main_loop.cpp
		SoundMngr->setListenerPos(pos());
		const CMatrix &camMat = MainCam.getMatrix();
		SoundMngr->setListenerOrientation(camMat.getJ(), camMat.getK());
	}

	if (ClientCfg.Light)
		return;

	// step sound of self : refind the sound animations associated to the walk and run animation

	static bool		computeAnim = true;
	static bool		lastMode = false;
	static TTime	previousTime = 0;
	static TTime	stepTime = 0;
	static bool		leftRight = false;
	static NLSOUND::CSoundAnimMarker	*leftStep = 0;
	static NLSOUND::CSoundAnimMarker	*rightStep = 0;

	// TODO : Remove when bug corrected:
	if (_Gender == GSGENDER::unknown)
	{
		nlwarning("CUserEntity::updateSound : The gender is unknown, forcing it to male");
		_Gender = GSGENDER::male;
	}

	// force recompute of anim if walk/run change.
	computeAnim = computeAnim || (lastMode != _Run);

	// Check the sound animation to find the time between to step
	if(computeAnim && SoundMngr && _CurrentAnimSet[MOVE] != 0) // && _SoundId[MOVE] != NLSOUND::CSoundAnimationNoId)
	{
		lastMode = _Run;
		TAnimStateId mode = _Run ? CAnimationStateSheet::Run : CAnimationStateSheet::Walk;
		const CAnimationState *animStatePtr = _CurrentAnimSet[MOVE]->getAnimationState (mode);
		if (animStatePtr)
		{
			::CAnimation::TAnimId animId = animStatePtr->chooseAnim (_AnimJobSpecialisation, people(), getGender(), 0);
			if (animId != ::CAnimation::UnknownAnim)
			{
				const ::CAnimation *anim = animStatePtr->getAnimation (animId);
				if(anim)
				{
					// Select the sound ID
					_SoundId[MOVE] = anim->soundId();

					if (_SoundId[MOVE] != NLSOUND::CSoundAnimationNoId)
					{
						// retrieve the anim
						NLSOUND::CSoundAnimManager	*mgr = NLSOUND::CSoundAnimManager::instance();

						string name = mgr->idToName(_SoundId[MOVE]);

						if (!name.empty())
						{
							NLSOUND::CSoundAnimation *sanim = mgr->findAnimation(name);
							if (sanim->countMarkers() != 2)
							{
								static set<string> warnOnce;
								if (warnOnce.find(sanim->getName()) == warnOnce.end())
								{
									nlwarning("Sound animation '%s' has not 2 markers, not a biped ? (Display Once)", sanim->getName().c_str());
									warnOnce.insert(sanim->getName());
								}
							}
							else
							{
								stepTime = TTime((sanim->getMarker(1)->getTime() - sanim->getMarker(0)->getTime()) * 1000);
								rightStep = sanim->getMarker(0);
								leftStep = sanim->getMarker(1);
								computeAnim = false;
							}
						}
					}
				}
			}
		}
	}

	if( SoundMngr && _Mode == MBEHAV::NORMAL)
 	{
		if (!getVelocity().isNull())
		{
			if (stepTime > 0 && time-previousTime>=stepTime)
			{
				previousTime = time;

				// set the sound 1 meter below listener
				_SoundContext.Position = pos() + CVector(0,0,-1);
				_SoundContext.RelativeGain = SoundMngr->getUserEntitySoundLevel();;

				uint32 matId= getGroundType();
				//nldebug("Current material = %u", matId);
				_SoundContext.Args[0] = matId;

				if (leftRight)
				{
					if (leftStep)
						// TODO : find the correct cluster
						leftStep->play(SoundMngr->getMixer(), NULL, _SoundContext);
				}
				else
				{
					if (rightStep)
						// TODO : find the correct cluster
						rightStep->play(SoundMngr->getMixer(), NULL, _SoundContext);

					// recompute a new sound anim
					computeAnim = true;
				}

				leftRight = !leftRight;
			}
		}
		else
		{
			// resets the counter
			previousTime = 0;
		}
	}
}// updateSound //


//-----------------------------------------------
// rotate :
// rotate the body on the left or right (front changes).
//-----------------------------------------------
void CUserEntity::rotate(float ang)
{
	// Rotate the body.
	CMatrix m;
	m.identity();
	m.rotateZ(ang);
	front(m * front().normed());
}// rotate //


//-----------------------------------------------
// rotHeadVertically :
// rotate the head vertically.
//-----------------------------------------------
void CUserEntity::rotHeadVertically(float ang)
{
	setHeadPitch(_HeadPitch+ang);
}// rotHeadVertically //


//-----------------------------------------------
// setHeadPitch(double hp)
//-----------------------------------------------
void CUserEntity::setHeadPitch(double hp)
{
	_HeadPitch= hp;
	const double bound= Pi/2 - 0.01;	//  epsilon to avoid gimbal lock
	clamp(_HeadPitch, -bound, bound);
}

//---------------------------------------------------
// slotRemoved :
// To Inform about an entity removed (to remove from selection for example).
// This will remove the entity from the target.
// \param slot : Slot of the entity that will be removed.
//---------------------------------------------------
void CUserEntity::slotRemoved(const CLFECOMMON::TCLEntityId &slot)
{
	// parent call
	CPlayerCL::slotRemoved(slot);

	// reset also selection
	if(selection() == slot)
		selection(CLFECOMMON::INVALID_SLOT);
}// slotRemoved //

//---------------------------------------------------
// selection :
// Change the entity selected.
// \param slot : slot now selected (CLFECOMMON::INVALID_SLOT for an empty selection).
// \warning Can be different from the entity targeted (in combat mode for example).
//---------------------------------------------------
void CUserEntity::selection(const CLFECOMMON::TCLEntityId &slot)	// virtual
{
	//allows reselection in Ring client: even if the selected slots is equal to the selection,
	//the client must send the messages.
	if ((_Selection == slot) && !ClientCfg.R2EDEnabled)
		return;

	// The selection will be the entity to watch
	WatchedEntitySlot = slot;
	disableFollow();

	// Send the entity selected to the server.
	NetMngr.pushTarget(slot);


	// Target the slot on client, don't wait NetWork response
	targetSlot(slot);
	_TargetSlotNoLag= slot;

	if (ClientCfg.R2EDEnabled)
	{
		R2::getEditor().inGameSelection(slot);
	}


	// Change the current selection so un color the current selection.
	CEntityCL *sel = EntitiesMngr.entity(_Selection);
	if(sel != NULL)
		sel->visualSelectionStop(); // Blink off == restore to normal

	// Set the entity selected
	_Selection = slot;

	// Update visual selection and interface
	if ( sel && sel->isForageSource() )
		sel->buildInSceneInterface(); // remove focus on previous selected source
	sel = EntitiesMngr.entity(_Selection);
	if(sel != NULL)
	{
		sel->visualSelectionStart();
		if ( sel->isForageSource() )
			sel->buildInSceneInterface(); // set focus on new selected source
	}


	// **** Update Target interface
	//get the new target slot and set it in the database
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:TARGET:SLOT")->setValue64(slot);

	// Get the new target UID, and set in Database
	uint	tgtSlot= _Selection;
	uint32	tgtEntityId= CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;
	CEntityCL *entity = NULL;
	if (tgtSlot!=CLFECOMMON::INVALID_SLOT)
	{
		entity = EntitiesMngr.entity(tgtSlot);
		if (entity)
			tgtEntityId= entity->dataSetId();
	}

	// Set the User Target
	CCDBNodeLeaf *prop = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:TARGET:UID", false);
	if(prop)
		prop->setValue32(tgtEntityId);

	// Bar Manager. Update now the Target View (so it takes VP if data available or 0... but result is immediate)
	CBarManager::getInstance()->setLocalTarget(tgtEntityId);

	// **** Update DB for InGameMenu
	// clear the entries for mission option
	for(uint k = 0; k < NUM_MISSION_OPTIONS; ++k)
	{
		CCDBNodeLeaf *missionOption = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%d:TITLE", (int) k), false);
		if (missionOption)
		{
			missionOption->setValue32(0);
		}
		CCDBNodeLeaf *playerGiftNeeded = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%d:PLAYER_GIFT_NEEDED", (int) k), false);
		if (playerGiftNeeded)
		{
			playerGiftNeeded->setValue32(0);
		}
		//
		missionOption = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%d:TITLE", (int) k), false);
		if (missionOption)
		{
			missionOption->setValue32(0);
		}
		playerGiftNeeded = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%d:PLAYER_GIFT_NEEDED", (int) k), false);
		if (playerGiftNeeded)
		{
			playerGiftNeeded->setValue32(0);
		}
	}
/* TODO ULU : Add RP tags */

	// update pvp tags
	if ((tgtSlot!=CLFECOMMON::INVALID_SLOT) && entity)
	{
		CPlayerCL *pPlayer = dynamic_cast<CPlayerCL*>(entity);

		if (pPlayer)
		{
			/*// Pvp Mode
			CViewBitmap * tagMode = dynamic_cast<CViewBitmap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:target:pvp_tags:mode"));
			if (tagMode)
			{
				if (pPlayer->getPvpMode()&PVP_MODE::PvpFaction)
					tagMode->setTexture("pvp_orange.tga");
				else if (pPlayer->getPvpMode()&PVP_MODE::PvpFactionFlagged)
					tagMode->setTexture("pvp_red.tga");
				else
					tagMode->setTexture("alpha_10.tga");
			}
*/
			/*// Pvp available actions (attack, heal, both)
			CViewBitmap * tagMode = dynamic_cast<CViewBitmap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:target:pvp_tags:actions"));
			if (tagMode)
			{
				if (pPlayer->getPvpMode()&PVP_MODE::PvpFaction)
					tag->setTexture("pvp_orange.tga");
				else if (pPlayer->getPvpMode()&PVP_MODE::PvpFactionFlagged)
					tag->setTexture("pvp_red.tga");
				else
					tag->setTexture("alpha_10.tga");
			}*/

		}
	}

	// clear web page
	prop= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:WEB_PAGE_URL", false);
	if(prop)	prop->setValue32(0);
	prop= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:WEB_PAGE_TITLE", false);
	if(prop)	prop->setValue32(0);

	// clear mission ring
	for(uint k = 0; k < BOTCHATTYPE::MaxR2MissionEntryDatabase; ++k)
	{
		prop= NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:TARGET:CONTEXT_MENU:MISSION_RING:%d:TITLE", k), false);
		if(prop)	prop->setValue32(0);
	}

	// clear programs
	prop= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:PROGRAMMES", false);
	if(prop)	prop->setValue32(0);
	prop= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TARGET:CONTEXT_MENU:PROGRAMMES");
	if(prop)	prop->setValue32(0);
	// increment db counter for context menu
	pIM->incLocalSyncActionCounter();

}// selection //

//---------------------------------------------------
// moveToAttack :
// Method to place the user to attack the target and attack.
//---------------------------------------------------
void CUserEntity::moveToAttack()
{
	// **** For clarity, try to launch a "default attack" found in the memory bar instead of an "anonymous" action
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();
	uint32	memLine, memSlot;
	CEntityCL	*target= EntitiesMngr.entity(selection());

	CInventoryManager *inv = CInventoryManager::getInstance();

	// auto-equip with valid weapon
	if( ClientCfg.AutoEquipTool )
	{
		if(inv)
		{
			// if no valid weapons in had -> auto-equip with last used weapons
			bool validWeaponInHand = true;
			uint32 rightHandSheet = inv->getRightHandItemSheet();
			if(rightHandSheet)
			{
				validWeaponInHand = inv->isMeleeWeaponItem(rightHandSheet) || inv->isRangeWeaponItem(rightHandSheet);
			}
			if( !validWeaponInHand )
			{
				autoEquipWithLastUsedWeapons();
			}

			// remember last used weapon(s)
			rememberWeaponsInHand();
		}
	}

	if(target && pPM->findDefaultAttack(memLine, memSlot))
	{
		// launch instead a phrase execution with this phrase
		executeCombatWithPhrase(target, memLine, memSlot, true);
	}
	// **** Else launch an anonymous "default attack"
	else
	{
		// melee or range?
		bool melee = true;
		if(inv)
		{
			uint32 rightHandSheet = inv->getRightHandItemSheet();
			if(rightHandSheet)
				melee = inv->isMeleeWeaponItem(rightHandSheet);
		}

		// Move to target if melee weapon
		if(melee)
			moveTo(selection(), 2.0, CUserEntity::Attack);
		// Just attack if range weapon.
		else
			attack();
	}
}// moveToAttack //

//---------------------------------------------------
// attack :
// Method to attack the target.
//---------------------------------------------------
void CUserEntity::attack()
{
	// execute the default attack
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();
	pPM->executeDefaultAttack();

	bool melee = true;
	CInventoryManager *inv = CInventoryManager::getInstance();
	if(inv)
	{
		uint32 rightHandSheet = inv->getRightHandItemSheet();
		if(rightHandSheet)
			melee = inv->isMeleeWeaponItem(rightHandSheet);
	}

	// If option ON, follow when attacking.
	if(ClientCfg.FollowOnAtk)
	{
		// Follow only if attacking with a melee weapon
		if(melee)
			// enable, but don't reset camera rotation
			enableFollow(false);
	}
}// attack //

//---------------------------------------------------
// attack :
// Method to attack the target, with a special phrase
//---------------------------------------------------
void CUserEntity::attackWithPhrase()
{
	if( !canEngageCombat() )
		return;

	CSPhraseManager		*pPM= CSPhraseManager::getInstance();

	// validate the execution on server
	pPM->sendExecuteToServer(_MoveToPhraseMemoryLine, _MoveToPhraseMemorySlot, _MoveToPhraseCyclic);

	// If the Attack is a "Next", and not a cycle, do a default attack
	if(!_MoveToPhraseCyclic)
		pPM->executeDefaultAttack();

	// If option ON, follow when attacking.
	if(ClientCfg.FollowOnAtk)
	{
		bool melee = true;
		CInventoryManager *inv = CInventoryManager::getInstance();
		if(inv)
		{
			uint32 rightHandSheet = inv->getRightHandItemSheet();
			if(rightHandSheet)
				melee = inv->isMeleeWeaponItem(rightHandSheet);
		}
		// Follow only if attacking with a melee weapon
		if(melee)
			// enable, but don't reset camera rotation
			enableFollow(false);
	}

	// Because sendExecuteToServer() has been called, must NOT cancelClientExecute() at resetAnyMoveTo()
	_MoveToAction= CUserEntity::None;
}

//-----------------------------------------------
// assist :
// your current target become the target of your current one.
//-----------------------------------------------
void CUserEntity::assist()
{
	assist(targetSlot());
}// assist //

//-----------------------------------------------

void CUserEntity::assist(uint slot)
{
	// Check the current target
	if(slot == CLFECOMMON::INVALID_SLOT || slot == _Slot)
		return;
	// Check the target
	CEntityCL *target = EntitiesMngr.entity(slot);
	if(target == 0)
		return;
	// Check the new slot.
	CLFECOMMON::TCLEntityId newSlot = target->targetSlot();
	if(newSlot == CLFECOMMON::INVALID_SLOT || newSlot == _Slot)
		return;
	// Select the new target.
	selection(newSlot);
}

//---------------------------------------------------
// disengage :
// Method to disengage the target.
//---------------------------------------------------
void CUserEntity::disengage()
{
	// Set the database in local.
	if(ClientCfg.Local)
	{
		IngameDbMngr.setProp("Entities:E0:P" + toString(CLFECOMMON::PROPERTY_MODE),      MBEHAV::NORMAL);	// Mode
		IngameDbMngr.setProp("Entities:E0:P" + toString(CLFECOMMON::PROPERTY_TARGET_ID), _Selection);		// Target
		UserEntity->updateVisualProperty(0, CLFECOMMON::PROPERTY_MODE);
		UserEntity->updateVisualProperty(0, CLFECOMMON::PROPERTY_TARGET_ID);
		return;
	}

	// Disengage MSG.
	const string msgName = "COMBAT:DISENGAGE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		NetMngr.push(out);
	else
		nlwarning("UE::disengage: unknown message named '%s'.", msgName.c_str());

	// Change the current mode.
	mode(MBEHAV::NORMAL);
}// disengage //

//-----------------------------------------------
// sit :
// Ask for the client to sit/stand ('true' to sit).
//-----------------------------------------------
bool CUserEntity::sit(bool s)
{
	bool	ok= false;

	// SIT
	if(s)
	{
		if(canSit() == true)
		{
			// disable afk mode
			setAFK(false);

			// Sit MSG.
			if(mode(MBEHAV::SIT))
			{
				// autowalk disabled
				UserControls.autowalkState(false);

				const string msgName = "COMMAND:SIT";
				CBitMemStream out;
				if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
				{
					out.serial(s);
					NetMngr.push(out);
				}
				else
					nlwarning("UE:sit: unknown message named '%s'.", msgName.c_str());

				// mode changed
				ok= true;

				// display sit msg
				CInterfaceManager	*pIM= CInterfaceManager::getInstance();
				ucstring msg = CI18N::get("msgUserIsSitting");
				string cat = getStringCategory(msg, msg);
				pIM->displaySystemInfo(msg, cat);
			}
		}
	}
	// STAND
	else
	{
		if(_Mode == MBEHAV::SIT)
		{
			if(mode(MBEHAV::NORMAL))
			{
				const string msgName = "COMMAND:SIT";
				CBitMemStream out;
				if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
				{
					out.serial(s);
					NetMngr.push(out);
				}
				else
					nlwarning("UE:sit: unknown message named '%s'.", msgName.c_str());

				// mode changed
				ok= true;

				// display stand msg
				CInterfaceManager	*pIM= CInterfaceManager::getInstance();
				ucstring msg = CI18N::get("msgUserIsStanding");
				string cat = getStringCategory(msg, msg);
				pIM->displaySystemInfo(msg, cat);
			}
		}
	}

	// if mode changed, Write to the UI database, to update views
	if(ok)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:PLAYER_STAND", false);
		if(node)
			node->setValue32(_Mode != MBEHAV::SIT);
	}

	// mode changed
	return ok;
}// sit //

//-----------------------------------------------
// canSit :
// Return true if the user can sit.
//-----------------------------------------------
bool CUserEntity::canSit() const
{
	// If the user is not already sitting or is on a mount
	if(!isSit() && (!isRiding()) && !isDead() && !isSwimming())
	{
		return true;
	}
	else
		return false;
}// canSit //

//-----------------------------------------------
// setAFK
//-----------------------------------------------
void CUserEntity::setAFK(bool b, string afkTxt)
{
	if( isAFK() == b ) return;

	if (b)
	{
		if( isDead() || isRiding() || moveTo() || follow() )
			return;

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		//sint64 start = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_TSTART")->getValue64();
		//sint64 end = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_TEND")->getValue64();
		//sint64 type = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_TYPE")->getValue64();
		//sint64 num = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_NUMBER")->getValue64();
		if( pIM )
		{
			if( NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:ACT_TYPE")->getValue64() != 0 )
				return;
		}

		if( !isSit() && !isSwimming() )
		{
			if( !mode(MBEHAV::REST) )
				return;
		}
	}
	else
	{
		if( !isSit() && !isSwimming() )
		{
			if (isDead())
				return;
			else
				mode(MBEHAV::NORMAL);
		}
	}

	// send afk state
	string msgName = "COMMAND:AFK";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		out.serial(b);
		NetMngr.push(out);
	}
	else
		nlwarning("CUserEntity:setAFK: unknown message named '%s'.", msgName.c_str());

	// custom afk txt
	ucstring ucstr;
	ucstr.fromUtf8( afkTxt );
	CBitMemStream outTxt;
	msgName = "STRING:AFK_TXT";
	if( GenericMsgHeaderMngr.pushNameToStream(msgName,outTxt) )
	{
		outTxt.serial( ucstr );
		NetMngr.push( outTxt );
	}
	else
	{
		nlwarning("CUserEntity:setAFK: unknown message named '%s'.", msgName.c_str());
	}


}// setAFK //

//-----------------------------------------------
// rollDice
//-----------------------------------------------
void CUserEntity::rollDice(sint16 min, sint16 max)
{
	const string msgName = "COMMAND:RANDOM";
	CBitMemStream out;
	if (GenericMsgHeaderMngr.pushNameToStream(msgName, out))
	{
		out.serial(min);
		out.serial(max);
		NetMngr.push(out);
	}
	else
		nlwarning("CUserEntity:rollDice: unknown message named '%s'.", msgName.c_str());
}// rollDice //

//---------------------------------------------------
// canEngageCombat :
// return true if user can engage melee combat, else return false and display system msg
//---------------------------------------------------
bool CUserEntity::canEngageCombat()
{
	if( isSit() )
	{
		// display "you can't fight while sitting" message)
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		ucstring msg = CI18N::get("msgCantFightSit");
		string cat = getStringCategory(msg, msg);
		pIM->displaySystemInfo(msg, cat);

		return false;
	}

	if( isSwimming() )
	{
		// display "you can't fight while swiming" message)
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		ucstring msg = CI18N::get("msgCantFightSwim");
		string cat = getStringCategory(msg, msg);
		pIM->displaySystemInfo(msg, cat);

		return false;
	}

	if ( isRiding() )
	{
		// display "you can't fight while swimming" message)
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		ucstring msg = CI18N::get("msgCantFightRide");
		string cat = getStringCategory(msg, msg);
		pIM->displaySystemInfo(msg, cat);

		return false;
	}

	return true;
} // canEngageCombat //


//---------------------------------------------------
// viewMode :
// Change the View (First/Third Person View).
//---------------------------------------------------
void CUserEntity::viewMode(CUserEntity::TView viewMode, bool changeView)
{
	switch(viewMode)
	{
	// First Person View
	case FirstPV:
		if(changeView)
			ClientCfg.FPV = true;
		if(_Mount != CLFECOMMON::INVALID_SLOT)
		{
			CEntityCL *mount = EntitiesMngr.entity(_Mount);
			if(mount)
				mount->displayable(false);
		}
		// Change Controls.
		if( isRiding() )
		{
			bool autoWalk = UserControls.autowalkState();
			UserControls.mode(CUserControls::MountMode);
			if( autoWalk )
				UserControls.autowalkState( true );
		}
		else
			UserControls.mode(CUserControls::InterfaceMode);
		break;

	// Third Person View
	case ThirdPV:
		if(changeView)
			ClientCfg.FPV = false;
		if(_Mount != CLFECOMMON::INVALID_SLOT)
		{
			CEntityCL *mount = EntitiesMngr.entity(_Mount);
			if(mount)
				mount->displayable(true);
		}
		// Change Controls.
		UserControls.mode(CUserControls::ThirdMode);
		break;

	// Unknown
	default:
		nlwarning("UE:viewMode: Unknown View Asked '%d'.", (sint)viewMode);
		return;
	}

	// Change the current View like asked.
	_ViewMode = viewMode;

	// disable or enable shadowing
	updateCastShadowMap();
}// viewMode //

//-----------------------------------------------
// toggleCamera :
// Toggle Camera (First/Third Person)
//-----------------------------------------------
void CUserEntity::toggleCamera()
{
	// You cannot change the camera view when dead.
	if(isDead())
		return;
	// Only if not inside a building.
	if(!UserEntity->forceIndoorFPV())
	{
		// Leave the 1st Person Mode -> Enter the 3rd Person View Mode
		if     (UserEntity->viewMode() == CUserEntity::FirstPV)
			UserEntity->viewMode(CUserEntity::ThirdPV);
		// Leave the 3rd Person Mode -> Enter the 1st Person View Mode
		else
			UserEntity->viewMode(CUserEntity::FirstPV);
	}
}// toggleCamera //

//---------------------------------------------------
// getScale :
// Return the entity scale. (return 1.0 if there is any problem).
// \todo GUIGUI : do we have to take care of the user's race kwnowing it can favour him ?
//---------------------------------------------------
float CUserEntity::getScale() const	// virtual
{
	// Default Scale.
	return 1.0f;
}// getScale //

//---------------------------------------------------
// removeCheckPrimitive :
// Remove the check primitive
//---------------------------------------------------
void CUserEntity::removeCheckPrimitive()
{
	if(PACS && _CheckPrimitive)
		PACS->removePrimitive(_CheckPrimitive);
	_CheckPrimitive = 0;
}

//---------------------------------------------------
// removePrimitive :
// Remove the primitive
//---------------------------------------------------
void CUserEntity::removePrimitive()	// virtual (will not be called by ~CEntityCL())
{
	// Remove the Primitive used for check
	removeCheckPrimitive();

	// Remove the other primitive
	CPlayerCL::removePrimitive();
}// removePrimitive //

//---------------------------------------------------
// computePrimitive :
// Create a primitive for the entity.
//---------------------------------------------------
void CUserEntity::computePrimitive() // virtual
{
	// Initialize the primitive.
	if(initPrimitive(0.5f, 2.0f, 0.0f, 0.0f, UMovePrimitive::Slide, (UMovePrimitive::TTrigger)(UMovePrimitive::OverlapTrigger | UMovePrimitive::EnterTrigger), MaskColPlayer, MaskColPlayer | MaskColNpc | MaskColDoor))
		_Primitive->insertInWorldImage(dynamicWI);
	// Set the position.
	pacsPos(pos());
}// computePrimitive //


//---------------------------------------------------
// isBusy :
// Return if the user is already busy (combat/bo chat/loot/ etc.).
//---------------------------------------------------
bool CUserEntity::isBusy() const
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	// Check Trade.

	// TODO : put the right DB entry !

	CCDBNodeLeaf *nod = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:EXCHANGE:BEGUN", false);
	if(nod)
	{
		if(nod->getValueBool())
			return true;
	}
//	else
//		nlwarning("UE:isBusy: Cannot get the nod 'SERVER:INVENTORY:EXCHANGE:BEGUN'.");

/*
	// Check Loot
	static const uint nbSlot = 4;
	uint i;
	for(i=0; i<nbSlot; ++i)
	{
		nod = NLGUI::CDBManager::getInstance()->getDbProp(NLMISC::toString("SERVER:INVENTORY:%d:%d:SHEET", INVENTORIES::pickup, i), false);
		if(nod)
		{
			if(nod->getValue32() != 0)
				return true;
		}
		else
			nlwarning("UE:isBusy: Cannot get the nod 'SERVER:INVENTORY:%d:%d:SHEET'.", INVENTORIES::pickup, i);
	}

	// Check Harvest
	for(i=0; i<nbSlot; ++i)
	{
		nod = NLGUI::CDBManager::getInstance()->getDbProp(NLMISC::toString("SERVER:INVENTORY:%d:%d:SHEET", INVENTORIES::harvest, i), false);
		if(nod)
		{
			if(nod->getValue32() != 0)
				return true;
		}
		else
			nlwarning("UE:isBusy: Cannot get the nod 'SERVER:INVENTORY:%d:%d:SHEET'.", INVENTORIES::harvest, i);
	}
*/

	// Check Bot chat.
	CBotChatPage * currPage = CBotChatManager::getInstance()->getCurrPage();
	if( currPage!= NULL )
	{
		return true;
	}

	// Not Busy
	return false;
}// isBusy //


//---------------------------------------------------
// updateVisualDisplay :
// Show/Hide all or parts of the user body.
// todo GUIGUI : it's bad for the _Face to be a separated instance
//---------------------------------------------------
void CUserEntity::updateVisualDisplay()
{
	// We need a skeleton.
	if(_Skeleton.empty())
		return;

	// 1st person View
	if(UserControls.isInternalView() || View.forceFirstPersonView())
	{
		// Hide the mount
		CCharacterCL *mount = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(_Mount));
		if(mount)
			mount->displayable(false);
		// Hide all user body parts.
		for(uint i=0; i<_Instances.size(); ++i)
			if(!_Instances[i].Current.empty())
			{
				_Instances[i].Current.hide();
				_Instances[i].hideStaticFXs();
			}
		// Hide the face
		if(!_Face.Current.empty())
			_Face.Current.hide();

		// We want to display weapons in 1st person view when attacking.
		if(modeWithHiddenItems() == false)
		{
			if( _Mode == MBEHAV::COMBAT_FLOAT || _Mode == MBEHAV::COMBAT )
			{
				if( _ObjectsVisible )
				{
					// Show just Few parts
					if(!_Instances[SLOTTYPE::HANDS_SLOT].Current.empty())
					{
						_Instances[SLOTTYPE::HANDS_SLOT].Current.show();
						_Instances[SLOTTYPE::HANDS_SLOT].showStaticFXs();
					}
					if(!_Instances[SLOTTYPE::ARMS_SLOT].Current.empty())
					{
						_Instances[SLOTTYPE::ARMS_SLOT].Current.show();
						_Instances[SLOTTYPE::ARMS_SLOT].showStaticFXs();
					}
					if(!_Instances[SLOTTYPE::RIGHT_HAND_SLOT].Current.empty())
					{
						_Instances[SLOTTYPE::RIGHT_HAND_SLOT].Current.show();
						_Instances[SLOTTYPE::RIGHT_HAND_SLOT].showStaticFXs();
					}
					if(!_Instances[SLOTTYPE::LEFT_HAND_SLOT].Current.empty())
					{
						_Instances[SLOTTYPE::LEFT_HAND_SLOT].Current.show();
						_Instances[SLOTTYPE::LEFT_HAND_SLOT].showStaticFXs();
					}
				}
			}
		}
	}
	else
	{
		// Show the mount
		CCharacterCL *mount = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(_Mount));
		if(mount)
		{
			mount->displayable(true);

			showOrHideBodyParts( objectsVisible() );
		}

		// Show the face
		/*
		if(!_Face.Current.empty())
			_Face.Current.show();
		*/
	}
}// updateVisualDisplay //

//---------------------------------------------------
// light:
// Show/Hide the user light.
//---------------------------------------------------
void CUserEntity::light()
{
	// Create the light
	if(_Light.empty())
	{
		// Check there is a skeleton and a bone to stick the light before to create it.
		if(!_Skeleton.empty() && _NameBoneId!=-1)
		{
			_Light = Scene->createPointLight();
			if(!_Light.empty())
			{
				// front of the player
				_Light.setPos(0.f,0.3f,0.f);
				// Setup the light
				_Light.setupAttenuation(12.0f, 20.0f);
				// Attach the light
				_Skeleton.stickObject(_Light, _NameBoneId);
				// The player light is the only one who can interact with Lightmapped objects
				_Light.setInfluenceLightMap(true);

				// TestYoyo
				/*
				NL3D::UInstance	inst;
				inst= Scene->createInstance("box.shape");
				if(!inst.empty())
				{
					inst.setScale(0.2f, 0.2f, 0.2f);
					inst.parent(_Light);
				}
				*/
			}
		}
		else
			nlwarning("UE:light: there is no skeleton or Name Bone to stick the light.");
	}
	// Turn On/Off the Light
	_LightOn = !_LightOn;
	if(!_Light.empty())
	{
		if(_LightOn)
			_Light.show();
		else
			_Light.hide();
	}
}// light //

//---------------------------------------------------
// CSpeedFactor::init :
// Initialize the Observer for the Speed Factor.
//---------------------------------------------------
void CUserEntity::CSpeedFactor::init()
{
	_Value = 1.0f; // Default speed factor is 1.
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	CCDBNodeLeaf *pNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:SPEED_FACTOR", false);
	if(pNodeLeaf)
	{
		/* if(!pNodeLeaf->addToLeaves(this))
			nlwarning("UE:SP:init: cannot add the observer");*/
		ICDBNode::CTextId textId;
		pNodeLeaf->addObserver(this, textId);
		if ( pNodeLeaf->getValue64() != 0 )
			_Value = ((float)pNodeLeaf->getValue64())/100.0f; // may have been received before
	}
	else
		nlwarning("UE:SP:init: 'SERVER:USER:SPEED_FACTOR' does not exist.");
}// CSpeedFactor::init //

//---------------------------------------------------
// CMountHunger::init :
//---------------------------------------------------
void CUserEntity::CMountHunger::init()
{}

//---------------------------------------------------
// CMountSpeeds::init :
//---------------------------------------------------
void CUserEntity::CMountSpeeds::init()
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	CCDBNodeLeaf *pNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp( "SERVER:USER:MOUNT_WALK_SPEED", false );
	BOMB_IF( ! pNodeLeaf, "MOUNT_WALK_SPEED not found", return );
	if(pNodeLeaf)
	{
		ICDBNode::CTextId textId;
		pNodeLeaf->addObserver(this, textId);
		_WalkSpeed = ((float)pNodeLeaf->getValue32()) / 1000.0f; // may have been received before
	}
	pNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp( "SERVER:USER:MOUNT_RUN_SPEED", false );
	BOMB_IF( ! pNodeLeaf, "MOUNT_RUN_SPEED not found", return );
	if(pNodeLeaf)
	{
		ICDBNode::CTextId textId;
		pNodeLeaf->addObserver(this, textId);
		_RunSpeed = ((float)pNodeLeaf->getValue32()) / 1000.0f; // may have been received before
	}
}

//---------------------------------------------------

void CUserEntity::CSpeedFactor::release()
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	CCDBNodeLeaf *pNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:SPEED_FACTOR", false);
	if(pNodeLeaf)
	{
		/* if(!pNodeLeaf->addToLeaves(this))
			nlwarning("UE:SP:init: cannot add the observer");*/
		ICDBNode::CTextId textId;
		pNodeLeaf->removeObserver(this, textId);
	}
	else
		nlwarning("UE:SP:init: 'SERVER:USER:SPEED_FACTOR' does not exist.");
}// CSpeedFactor::init //

void CUserEntity::CMountHunger::release()
{}

void CUserEntity::CMountSpeeds::release()
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	CCDBNodeLeaf *pNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp( "SERVER:USER:MOUNT_WALK_SPEED", false );
	BOMB_IF( ! pNodeLeaf, "MOUNT_WALK_SPEED not found", return );
	if(pNodeLeaf)
	{
		ICDBNode::CTextId textId;
		pNodeLeaf->removeObserver(this, textId);
	}
	pNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp( "SERVER:USER:MOUNT_RUN_SPEED", false );
	BOMB_IF( ! pNodeLeaf, "MOUNT_RUN_SPEED not found", return );
	if(pNodeLeaf)
	{
		ICDBNode::CTextId textId;
		pNodeLeaf->removeObserver(this, textId);
	}
}


//---------------------------------------------------
// CSpeedFactor::update :
// Callback called to update the speed factor.
//---------------------------------------------------
void CUserEntity::CSpeedFactor::update(ICDBNode *node) // virtual
{
	CCDBNodeLeaf *leaf = safe_cast<CCDBNodeLeaf *>(node);
	_Value = ((float)leaf->getValue64())/100.0f;
	//nlinfo("SpeedFactor changed to %f / %"NL_I64"u", _Value, leaf->getValue64());
	
	// clamp the value (2.0 is the egg item or the level 6 speed up power up, nothing should be faster)
	// commented because ring editor speed is in fact faster
	//if(_Value > 2.0f)
	//{
		//nlwarning("HACK: you try to change the speed factor to %f", _Value);
		//nlstop;
		//_Value = 2.0f;
	//}
}// CSpeedFactor::update //


/*
 * Return true if the mount can run. Precondition: UserEntity->isRiding().
 */
bool CUserEntity::CMountHunger::canRun() const
{
	CEntityCL *mountEntity = UserEntity->getMountEntity();
	if ( ! mountEntity )
		return false;

	// Find the mount's db leaf and check hunger
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeBranch *animalsNode = safe_cast<CCDBNodeBranch*>(NLGUI::CDBManager::getInstance()->getDB()->getNode( ICDBNode::CTextId( "SERVER:PACK_ANIMAL" ), false ));
	BOMB_IF( ! animalsNode, "! animalsNode", return false; );
	uint nbAnimals = (uint)animalsNode->getNbNodes();
	for ( uint i=0; i!=nbAnimals; ++i )
	{
		ICDBNode *beastNode = animalsNode->getNode( i );
		CCDBNodeLeaf *uidLeaf = safe_cast<CCDBNodeLeaf*>(beastNode->getNode( ICDBNode::CTextId( "UID" ) ));
		if ( ((CLFECOMMON::TClientDataSetIndex)uidLeaf->getValue32()) == mountEntity->dataSetId() )
		{
			CCDBNodeLeaf *hungerLeaf = safe_cast<CCDBNodeLeaf*>(beastNode->getNode( ICDBNode::CTextId( "HUNGER" ) ));
			return (hungerLeaf->getValue32() != (sint)ANIMAL_TYPE::DbHungryValue);
		}
	}
	return false;
}



//---------------------------------------------------
// CMountSpeeds::update :
// Callback called to update the mount speed.
//---------------------------------------------------
void CUserEntity::CMountSpeeds::update(ICDBNode * /* node */) // virtual
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	_WalkSpeed = ((float)(NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:MOUNT_WALK_SPEED")->getValue32())) / 1000.0f;
	_RunSpeed = ((float)(NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:MOUNT_RUN_SPEED")->getValue32())) / 1000.0f;
}


/*
 * Return the mount entity if the user is riding, otherwise NULL
 */
CEntityCL* CUserEntity::getMountEntity()
{
	if ( _Mount < EntitiesMngr.entities().size() )
	{
		return EntitiesMngr.entities()[_Mount];
	}
	return NULL;
}

/*
 * Return the DB entry for the specified user's animal (NULL if not found)
 */
CCDBNodeBranch *CUserEntity::getBeastDBEntry( CLFECOMMON::TClientDataSetIndex uid )
{
	// Find animal entry corresponding to datasetId
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeBranch *animalsNode = safe_cast<CCDBNodeBranch*>(NLGUI::CDBManager::getInstance()->getDB()->getNode( ICDBNode::CTextId( "SERVER:PACK_ANIMAL" ), false ));
	BOMB_IF( ! animalsNode, "! animalsNode", return NULL );
	uint nbAnimals = (uint)animalsNode->getNbNodes();
	for ( uint i=0; i!=nbAnimals; ++i )
	{
		ICDBNode *beastNode = animalsNode->getNode( i );
		CCDBNodeLeaf *pNodeLeaf = safe_cast<CCDBNodeLeaf*>(beastNode->getNode( ICDBNode::CTextId( "UID" ) ));
		if ( pNodeLeaf && (pNodeLeaf->getValue32() == (sint32)uid) )
			return (CCDBNodeBranch*)beastNode;
	}
	return NULL;
}


//---------------------------------------------------
// displayDebug :
// Display Debug Information.
//---------------------------------------------------
void CUserEntity::displayDebug(float x, float &y, float lineStep)	// virtual
{
	CPlayerCL::displayDebug(x, y, lineStep);
}// displayDebug //

//---------------------------------------------------
// displayModifiers :
// Display dmg/heal numbers above the head.
//---------------------------------------------------
void CUserEntity::displayModifiers()	// virtual
{
	if(!UserControls.isInternalView())
		CPlayerCL::displayModifiers();
}// displayModifiers //

//---------------------------------------------------
// isVisible :
// Return 'true' is the entity is displayed.
//---------------------------------------------------
bool CUserEntity::isVisible() const	// virtual
{
	return !UserControls.isInternalView();
}// isVisible //






//---------------------------------------------------
// readWrite :
// Read/Write Variables from/to the stream.
//---------------------------------------------------
void CUserEntity::readWrite(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPlayerCL::readWrite(f);

	// PROTECTED
	f.serial(_SpeedFactor);
	f.serial(_FrontVelocity);
	f.serial(_LateralVelocity);
	CVector	dummyHead;
	f.serial(dummyHead);
	f.serial(_HeadPitch);
	f.serial(_EyesHeight);
	f.serial(_Run);
	f.serial(_WalkVelocity);
	f.serial(_RunVelocity);
	f.serial(_CurrentVelocity);
	f.serial(_Selection);
	f.serial(_Trader);
	f.serial(_Interlocutor);
	f.serial(_Selectable);

	// PRIVATE
	f.serial(_OnMount);
	f.serial(_AnimAttackOn);
//	f.serialEnum(_ViewMode);
}// readWrite //

//---------------------------------------------------
// load :
// To call after a read from a stream to re-initialize the entity.
//---------------------------------------------------
void CUserEntity::load()	// virtual
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	// Insert the user into PACS at his saved position
	pacsPos(pos());

	// update
	if(!_WaitForAppearance)
	{
		// Visual properties A
		sint64 prop = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", _Slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->getValue64();
		updateVisualPropertyVpa(0, prop);	// Vpa udapte vpb and vpc too.
	}
}// load //


//---------------------------------------------------
void CUserEntity::CInvisibleObserver::update(ICDBNode* node)
{
	UserEntity->buildInSceneInterface();
}

//---------------------------------------------------
void CUserEntity::CSkillPointsObserver::update(ICDBNode* node )
{
	if (FarTP.isFarTPInProgress() || IngameDbMngr.initInProgress()) // prevent from displaying at the beginning of a FarTP (due to RESET_BANK or CDB resetData())
		return;

	CInterfaceManager *pIM = CInterfaceManager::getInstance ();
	CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf*>(node);
	if (leaf)
	{
		sint32 oldValue = leaf->getOldValue32();
		if (oldValue != 0)
		{
			sint delta = leaf->getValue32()-oldValue;
			string deltaStr = toString("%+d", delta);

			// get the sp title
			ucstring	spTitle;
			spTitle= CI18N::get(toString("uiSkillPointsBold%d",SpType));

			// run the popup
			CAHManager::getInstance()->runActionHandler("message_popup", NULL, "text1="+deltaStr+"|text0="+spTitle.toUtf8());

			// Context help
			contextHelp ("skill_point");
		}
	}
}


//---------------------------------------------------
//	CFameObserver::update
//---------------------------------------------------
void CUserEntity::CFameObserver::update(ICDBNode* node )
{
	CSkillManager *pSM = CSkillManager::getInstance();
	CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf*>(node);
	if (leaf)
	{
		sint32 fameValue = leaf->getValue32();
		pSM->tryToUnblockTitleFromMinFames( FactionIndex, fameValue );
		pSM->tryToUnblockTitleFromMaxFames( FactionIndex, fameValue );
	}
}


//---------------------------------------------------
void CUserEntity::makeTransparent(bool t)
{
	CPlayerCL::makeTransparent(t);

	uint32	opaMin= getOpacityMin();
	uint8 opacity = (uint8)(opaMin + (255-opaMin) * (1.0 - _TranspFactor));

	getFace()->makeInstanceTransparent(opacity, (uint8)opaMin);
}// makeTransparent //

//---------------------------------------------------
void CUserEntity::setDiffuse(bool onOff, NLMISC::CRGBA diffuse)
{
	CPlayerCL::setDiffuse(onOff, diffuse);
	getFace()->setDiffuse(onOff, diffuse);
}




// Helper for CUserEntity::extractRM()
bool findExtractionActionInMemory( CSPhraseManager *pm, CSBrickManager *bm, uint memoryLine, uint& index )
{
	uint x;
	for ( x=0; x!=PHRASE_MAX_MEMORY_SLOT; ++x )
	{
		uint32 phraseSlot = pm->getMemorizedPhrase( memoryLine, x );
		const CSPhraseCom& phraseCom = pm->getPhrase( phraseSlot );
		if ( ! phraseCom.empty() )
		{
			 CSBrickSheet *brickSheet = bm->getBrick( phraseCom.Bricks[0] );
			 if ( brickSheet->isForageExtraction() && (!brickSheet->MandatoryFamilies.empty()) ) // assumes care root bricks have not mandatories
			 {
				 index = x;
				 return true;
			 }
		}
	}
	return false;
}

//---------------------------------------------------
void CUserEntity::extractRM()
{
	CSPhraseManager *pm = CSPhraseManager::getInstance();
	uint index;
	uint memoryLine;
	bool autoFindPhrase = (_MoveToPhraseMemoryLine == std::numeric_limits<uint>::max());
	if ( ! autoFindPhrase )
	{
		// Use clicked phrase
		memoryLine = _MoveToPhraseMemoryLine;
		index = _MoveToPhraseMemorySlot;
	}
	else
	{
		// Find the first extraction phrase in the memory bar
		CSBrickManager *bm = CSBrickManager::getInstance();
		memoryLine = pm->getSelectedMemoryLineDB();
		if ( ! findExtractionActionInMemory( pm, bm, memoryLine, index ) )
		{
			// Search in other memory bar lines (because the auto-equip does not set the current line at once)
			memoryLine = std::numeric_limits<uint>::max();
			uint nbLines = pm->getNbMemoryLines();
			for ( uint j=0; j!=nbLines; ++j )
			{
				if ( j == memoryLine )
					continue;
				if ( findExtractionActionInMemory( pm, bm, j, index ) )
				{
					memoryLine = j;
					break;
				}
			}
		}
	}

	if ( memoryLine != std::numeric_limits<uint>::max() )
	{
		// Open the forage (but not for care actions). Necessary for the case of redoing an extraction after a Drop All on the same source.
		uint32 phraseId = pm->getMemorizedPhrase( memoryLine, index );
		if ( phraseId != 0 )
		{
			const CSPhraseCom& phraseCom = pm->getPhrase( phraseId );
			if ( ! phraseCom.empty() )
			{
				CSBrickSheet *rootBrick = CSBrickManager::getInstance()->getBrick( phraseCom.Bricks[0] );
				if ( rootBrick )
				{
					if ( rootBrick->IndexInFamily == 1 ) // only extracting actions
						CTempInvManager::getInstance()->open( TEMP_INV_MODE::Forage );
				}
			}
		}

		// Cast the extraction. if autoFindPhrase, clientExecute() not already called.
		if ( autoFindPhrase )
		{
			// decide now if cyclic or not
			_MoveToPhraseCyclic= true;
			if(pm->avoidCyclicForPhrase(pm->getMemorizedPhrase(memoryLine, index)))
				_MoveToPhraseCyclic= false;

			// execute on client now
			pm->clientExecute( memoryLine, index, _MoveToPhraseCyclic);
		}

		// execute on server
		pm->sendExecuteToServer( memoryLine, index, _MoveToPhraseCyclic );

		// Because sendExecuteToServer() has been called, must NOT cancelClientExecute() at resetAnyMoveTo()
		_MoveToAction= CUserEntity::None;
	}
	else
	{
		CInterfaceManager::getInstance()->displaySystemInfo( CI18N::get("uiExtractionPhraseMissing"), "CHK" );
		return;
	}
}


// ***************************************************************************
bool	CUserEntity::canCastShadowMap() const
{
	if(!CCharacterCL::canCastShadowMap())
		return false;

	// don't cast shadow in first person, but in death mode (third person actually...)
	return viewMode() != FirstPV || UserControls.mode() == CUserControls::DeathMode;
}


// ***************************************************************************
void	CUserEntity::forceLookEntity(const NLMISC::CVectorD &dir2targIn, bool updateHeadPitch, bool /* start */)
{
	CVectorD	dir2targ= dir2targIn;
	float		frontYawBefore = 0.f;
	float		frontYawAfter;

	// Third person: bkup current yaw
	if(viewMode()==ThirdPV)
	{
		frontYawBefore = frontYaw();
	}


	// **** Look at the entity
	dir2targ.normalize();
	front(dir2targ, false, false);


	// **** FirstPerson
	if(viewMode() == FirstPV)
	{
		if(updateHeadPitch && _FollowForceHeadPitch)
		{
			// rotate the head to the target
			CEntityCL *target = EntitiesMngr.entity(targetSlot());
			if(target)
			{
				// Both Z must be correct
				snapToGround();
				target->snapToGround();

				// don't update to the real head position each frame (else jitter too much cause of target anim)
				CVector targetPos= target->pos() + CVector(0,0,_FollowHeadOffset);

				// then look at this target
				CVector dirToTarget = targetPos - (pos()+CVector(0,0, UserEntity->eyesHeight()));
				if((dirToTarget.x != 0.0f) || (dirToTarget.y!=0.0f))
				{
					dirToTarget.normalize();
					setHeadPitch(atan2(dirToTarget.z, sqrt(sqr(dirToTarget.x)+sqr(dirToTarget.y))));
				}

				// TestYoyo
				/*if(ClientCfg.Fly!=0.f)
				{
					nlinfo("Uy: %.3f. Hp: %.3f. UPos:(%.3f,%.3f,%.3f). TPos:(%.3f,%.3f,%.3f)",
						UserEntity->frontYaw(),	UserEntity->getHeadPitch(), pos().x, pos().y, pos().z,
						targetPos.x, targetPos.y, targetPos.z);
					static	float	uy=0.f;
					static	float	hp=0.f;
					if( fabs(fmod(UserEntity->frontYaw()-uy, 2*Pi))>ClientCfg.Fly ||
						fabs(fmod(UserEntity->getHeadPitch()-hp, 2*Pi))>ClientCfg.Fly )
					{
						nlinfo("YOYOBREAK: ^^^^^^^^^^");
					}
					uy=UserEntity->frontYaw();
					hp=UserEntity->getHeadPitch();
				}*/
			}
		}
	}
	// **** Third person
	else if(viewMode()==ThirdPV)
	{
		// keep the current effective yaw. ONLY if no SmoothResetCameraYaw forced
		if(!UserControls.isResetSmoothCDYForced())
		{
			frontYawAfter = frontYaw();
			float	deltaYaw= frontYawAfter - frontYawBefore;

			// compensate rotation (NB: it stops also any SmoothReset)
			UserControls.appendCameraDeltaYaw(-deltaYaw);
		}
	}

	// when looking to an entity, if automatic camera, center the view on it.
	if( ClientCfg.AutomaticCamera /*&& start*/ )
	{
		UserControls.resetSmoothCameraDeltaYaw();
	}
}


// ***************************************************************************
void CUserEntity::startForceLookEntity(CLFECOMMON::TCLEntityId slot)
{
	// Start a new follow: force head pitch to follow by default
	_FollowForceHeadPitch= true;
	// if a follow is started in first person, reset CameraYaw
	if(viewMode()==FirstPV)
		UserControls.resetCameraDeltaYaw();

	// reorient now (important else may have a time shift because of resetCameraDeltaYaw above)
	CEntityCL *target = EntitiesMngr.entity(slot);
	if(target)
	{
		/* For complex reason, the target may not be still snapped on the ground. snap it now
			- this is because in common case, entities are snap only if they are visible (for speed up)
				=> depends on camera
			- but in this case, the camera depends on real entity position (headPitch in first person)
		*/
		target->snapToGround();

		// For FirstPerson targeting. Get the current target head offset
		_FollowHeadOffset= 0.f;
		CVector		headPos;
		if(target->getHeadPos(headPos))
		{
			_FollowHeadOffset= headPos.z - float(target->pos().z);
		}

		// Look at the entity
		CVectorD dir2targ = target->pos() - pos();
		dir2targ.z = 0.0;
		if(dir2targ!=CVectorD::Null)
		{
			forceLookEntity(dir2targ, true, true);
		}
	}
}


// ***************************************************************************
void CUserEntity::stopForceHeadPitchInFollow()
{
	_FollowForceHeadPitch= false;
}

// ***************************************************************************
void CUserEntity::switchVelocity(bool userRequest)
{
	if (ClientCfg.R2EDEnabled && R2::getEditor().getMode() == R2::CEditor::EditionMode)
	{
		// when in the R2 editor, force to run all the time
		_Run = true;
	}
	else
	{
		_Run = !_Run;
	}
	_CurrentVelocity = _Run ? runVelocity() : walkVelocity();

	if (userRequest)
	{
		_RunWhenAble = _Run;
	}

	// display message : your are running, you are walking
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	ucstring msg;
	if( _Run )
		msg = CI18N::get("msgUserIsRunning");
	else
		msg = CI18N::get("msgUserIsWalking");
	string cat = getStringCategory(msg, msg);
	pIM->displaySystemInfo(msg, cat);

	// Write to the UI database, to update views
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:PLAYER_RUNNING", false);
	if(node)
		node->setValue32(_Run);
}

//-----------------------------------------------
//	autoEquipWithLastUsedWeapons
//
//-----------------------------------------------
void	CUserEntity::autoEquipWithLastUsedWeapons()
{
	CInventoryManager *inv = CInventoryManager::getInstance();
	if ( !inv )
	{
		return;
	}

	// Clear hands
	inv->unequip( "LOCAL:INVENTORY:HAND:1" );
	inv->unequip( "LOCAL:INVENTORY:HAND:0" );

	uint ir,il;
	// Equip right hand
	if( _PreviousRightHandItem.Sheet != 0 )
	{
		// find item in bag with same properties than last used one in right hand
		for ( ir=0; ir<MAX_BAGINV_ENTRIES; ++ir )
		{
			if( _PreviousRightHandItem.Sheet == inv->getBagItem(ir).getSheetID() &&
				_PreviousRightHandItem.Quality == inv->getBagItem(ir).getQuality() &&
				_PreviousRightHandItem.Weight == inv->getBagItem(ir).getWeight() &&
				_PreviousRightHandItem.NameId == inv->getBagItem(ir).getNameId() )
			{
				break;
			}
		}
		if ( ir != MAX_BAGINV_ENTRIES )
		{
			// Equip right hand
			string bagPath = toString( "LOCAL:INVENTORY:BAG:%u", ir );
			inv->equip( bagPath, "LOCAL:INVENTORY:HAND:0" );

			// Equip left hand if needed
			if( _PreviousLeftHandItem.Sheet != 0 )
			{
				for ( il=0; il<MAX_BAGINV_ENTRIES; ++il )
				{
					if( il != ir &&
						_PreviousLeftHandItem.Sheet == inv->getBagItem(il).getSheetID() &&
						_PreviousLeftHandItem.Quality == inv->getBagItem(il).getQuality() &&
						_PreviousLeftHandItem.Weight == inv->getBagItem(il).getWeight() &&
						_PreviousLeftHandItem.NameId == inv->getBagItem(il).getNameId() )
					{
						break;
					}
				}
				if ( il != MAX_BAGINV_ENTRIES )
				{
					bagPath = toString( "LOCAL:INVENTORY:BAG:%u", il );
					inv->equip( bagPath, "LOCAL:INVENTORY:HAND:1" );
				}
			}
			return;
		}
	}


	// TODO : choose the best one

}


// ***************************************************************************
void	CUserEntity::executeCombatWithPhrase(CEntityCL	*target, uint32 memoryLine, uint32 memoryIndex, bool cyclic)
{
	nlassert(target);
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();

	// is a melee combat?
	bool meleeCombat = false;
	// empty hand => yes!
	meleeCombat= true;
	uint32 rightHandSheet = getInventory().getRightHandItemSheet();
	if(rightHandSheet)
		meleeCombat = getInventory().isMeleeWeaponItem(rightHandSheet);

	// If melee combat, and if the user entity is not well placed for fight, or if it has changed his target
	if( meleeCombat &&
		(
		!target->isPlacedToFight(pos(), front(), attackRadius() + ClientCfg.AttackDist) ||
		target->slot()!=_LastExecuteCombatSlot
		)
		)
	{
		_LastExecuteCombatSlot= target->slot();

		// Cancel any follow
		disableFollow();

		// Launch the moveToCombatPhrase, canceling any old action client execution.
		// NB: this will also force him to look at the entity
		moveToCombatPhrase(target->slot(), 2.0, memoryLine, memoryIndex, cyclic);

		// And after (order is important), start the phrase execution on client
		pPM->clientExecute(memoryLine, memoryIndex, cyclic);
	}
	else
	{
		// Cancel any moveTo(), because don't want to continue reaching the prec entity
		// VERY important if previous MoveTo was a SPhrase MoveTo (because cancelClientExecute() must be called)
		resetAnyMoveTo();

		// start client execution: NB: start client execution even if it
		pPM->clientExecute(memoryLine, memoryIndex, cyclic);

		// inform Server of phrase cast
		pPM->sendExecuteToServer(memoryLine, memoryIndex, cyclic);

		if( !meleeCombat && !cyclic )
		{
			pPM->executeDefaultAttack();
		}
	}
}

// ***************************************************************************
void	CUserEntity::beginCast(const MBEHAV::CBehaviour &behaviour)
{
	if(viewMode()==ThirdPV)
	{
		// backup front yaw
		float		frontYawBefore = frontYaw();
		// begin cast
		CCharacterCL::beginCast( behaviour );
		// compensate the front change using a camera move
		float frontYawAfter = frontYaw();
		float	deltaYaw= frontYawAfter - frontYawBefore;
		UserControls.appendCameraDeltaYaw(-deltaYaw);
		// if automatic camera, center the view behind the user
		if( ClientCfg.AutomaticCamera )
		{
			UserControls.resetSmoothCameraDeltaYaw();
		}
	}
	else
	{
		// in first person mode, reset the delta yaw
		UserControls.resetCameraDeltaYaw();
		CCharacterCL::beginCast( behaviour );
	}
}

// ***************************************************************************
void	CUserEntity::updatePreCollision(const NLMISC::TTime &time, CEntityCL *target)
{
	CPlayerCL::updatePreCollision(time, target);

	// test each frame if the mode has changed
	if(SoundMngr)
	{
		string	deadMusic= "death.ogg";
		// Play/stop music if comes from or goes to dead
		bool	isDead= _Mode==MBEHAV::DEATH || _Mode==MBEHAV::SWIM_DEATH;

		// must start music?
		if( isDead && SoundMngr->getEventMusicPlayed()!=deadMusic )
		{
			SoundMngr->playEventMusic(deadMusic, 0, true);
		}

		// must end music?
		if( !isDead && SoundMngr->getEventMusicPlayed()==deadMusic )
		{
			SoundMngr->stopEventMusic(deadMusic, CSoundManager::LoadingMusicXFade);
		}
	}
}

// ***************************************************************************
void	CUserEntity::buildTotem()
{
	const string msgName = "TOTEM:BUILD";
	CBitMemStream out;
	if( GenericMsgHeaderMngr.pushNameToStream( msgName, out ) )
	{
		NetMngr.push( out );
		nlinfo( "sending TOTEM:build message to server" );
	}
}

// ***************************************************************************
void	CUserEntity::setR2CharMode(R2::TCharMode mode)
{
	if (mode == R2::TCharMode::Editer || mode == R2::TCharMode::Dm)
	{
		_R2CharMode= mode;
		walkVelocity(ClientCfg.DmWalk);
		runVelocity(ClientCfg.DmRun);
		View.setCameraDistanceMaxForDm();
		CEntityCL *user = EntitiesMngr.entity(0);
		NLPACS::UMovePrimitive* prim = user?user->getPrimitive():0;
		if (prim)
		{
			prim->setObstacle(false);
		}

	}
	else if (mode == R2::TCharMode::Player || mode == R2::TCharMode::Tester)
	{
		_R2CharMode= mode;
		walkVelocity(ClientCfg.Walk);
		runVelocity(ClientCfg.Run);
		View.setCameraDistanceMaxForPlayer();
		CEntityCL *user = EntitiesMngr.entity(0);
		NLPACS::UMovePrimitive* prim = user?user->getPrimitive():0;
		if (prim)
		{
			prim->setObstacle(true);
		}
	}
	else
	{
		nlwarning("R2Ed: Error Char Mode not handled %u", (uint32)mode.getValue());
	}
}

bool CUserEntity::isInNpcControl() const
{
	CInterfaceManager* pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf	*sheet = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:NPC_CONTROL:SHEET", false);
	return sheet && NLMISC::CSheetId(sheet->getValue32())!=NLMISC::CSheetId::Unknown;
}


void	CUserEntity::updateNpcContolSpeed()
{
	CInterfaceManager* pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf	*sheet = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:NPC_CONTROL:SHEET", false);
	CCDBNodeLeaf	*walk  = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:NPC_CONTROL:WALK",  false);
	CCDBNodeLeaf	*run   = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:NPC_CONTROL:RUN",   false);
	if (!sheet || !walk || !run)
	{
		return;
	}

	static NLMISC::CSheetId oldSheet = NLMISC::CSheetId::Unknown;
	static float oldRun=0.f;
	static float oldWalk=0.f;

	NLMISC::CSheetId sheetId(sheet->getValue32());
	float newRun = float(run->getValue32()) / 1000.0f;
	float newWalk =  float(walk->getValue32()) / 1000.0f;

	if (sheetId == oldSheet && oldRun == newRun && oldWalk == newWalk )
	{
		return;
	}

	oldSheet = sheetId;
	oldRun = newRun;
	oldWalk = newWalk;

	if (sheetId != NLMISC::CSheetId::Unknown)
	{
		walkVelocity(newWalk);
		runVelocity(newRun);
	}
	else
	{
		setR2CharMode(_R2CharMode);
	}

}

//-----------------------------------------------
//		cancelAllPhrases
//-----------------------------------------------
void CUserEntity::cancelAllPhrases()
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("PHRASE:CANCEL_ALL", out))
	{
		NetMngr.push(out);
	}
	else
	{
		nlwarning("<CUserEntity::cancelAllPhrases> unknown message name '%s'", "PHRASE:CANCEL_ALL");
	}
}


//-----------------------------------------------
//		canChangeFront
//-----------------------------------------------
bool CUserEntity::canChangeFront()
{
	return !(_CurrentBehaviour.Behaviour == MBEHAV::EXTRACTING
		|| (_CurrentBehaviour.Behaviour == MBEHAV::RANGE_ATTACK && _Mode==MBEHAV::COMBAT && !UserControls.isMoving())
		|| (_CurrentBehaviour.Behaviour >= MBEHAV::MAGIC_CASTING_BEHAVIOUR_BEGIN && _CurrentBehaviour.Behaviour <= MBEHAV::MAGIC_CASTING_BEHAVIOUR_END));
}


//-----------------------------------------------
//	rememberWeaponsInHand
//
//-----------------------------------------------
void	CUserEntity::rememberWeaponsInHand()
{
	CInventoryManager * inv = CInventoryManager::getInstance();
	if( !inv )
	{
		return;
	}

	// keep right hand item
	CItemImage * rightItemImg = inv->getHandItem(0);
	if( rightItemImg )
	{
		if( inv->isMeleeWeaponItem(rightItemImg->getSheetID()) || inv->isRangeWeaponItem(rightItemImg->getSheetID()) )
		{
			_PreviousRightHandItem = CItemSnapshot(*rightItemImg);

			// keep left hand item too (could be ammo, second weapon, etc ..)
			CItemImage * leftItemImg = inv->getHandItem(1);
			if( leftItemImg )
			{
				_PreviousLeftHandItem = CItemSnapshot(*leftItemImg);
			}
			else
			{
				_PreviousLeftHandItem = CItemSnapshot();
			}
		}
	}
}


//-----------------------------------------------
// snapshot of a CItemImage
//
//-----------------------------------------------
CUserEntity::CItemSnapshot::CItemSnapshot( const CItemImage& i )
{
	Sheet = i.getSheetID();
	Quality = i.getQuality();
	Quantity = i.getQuantity();
	UserColor = i.getUserColor();
	Price = i.getPrice();
	Weight = i.getWeight();
	NameId = i.getNameId();
	InfoVersion = i.getInfoVersion();
}

sint CUserEntity::getLevel() const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	sint level = -1;
	for(uint i=0;i<EGSPD::CSPType::EndSPType;i++)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:USER:SKILL_POINTS_%d:VALUE", i), false);
		if(node)
		{
			level = std::max(level, (sint) node->getValue32());
		}
	}
	return level;
}

//-----------------------------------------------
//		interlocutor
//-----------------------------------------------
void CUserEntity::interlocutor( const CLFECOMMON::TCLEntityId &slot)
{
	CLFECOMMON::TCLEntityId prevInterlocutor = _Interlocutor;
	_Interlocutor = slot;

	// Refresh (hide or unhide) the icon for the interlocutor NPC
	if (prevInterlocutor != CLFECOMMON::INVALID_SLOT)
		EntitiesMngr.refreshInsceneInterfaceOfFriendNPC(prevInterlocutor);
	if (_Interlocutor != CLFECOMMON::INVALID_SLOT)
		EntitiesMngr.refreshInsceneInterfaceOfFriendNPC(_Interlocutor);
}

//-----------------------------------------------
//		trader
//-----------------------------------------------
void CUserEntity::trader(const CLFECOMMON::TCLEntityId &slot)
{
	CLFECOMMON::TCLEntityId prevTrader = _Trader;
	_Trader = slot;

	// Refresh (hide or unhide) the icon for the trader NPC
	if (prevTrader != CLFECOMMON::INVALID_SLOT)
		EntitiesMngr.refreshInsceneInterfaceOfFriendNPC(prevTrader);
	if (_Trader != CLFECOMMON::INVALID_SLOT)
		EntitiesMngr.refreshInsceneInterfaceOfFriendNPC(_Trader);
}

