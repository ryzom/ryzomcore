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
// Sheets
#include "client_sheets/item_sheet.h"
// Misc
#include "nel/misc/vectord.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/bsphere.h"
// Interface 3D
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_material.h"
#include "nel/3d/u_visual_collision_manager.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_bone.h"
#include "nel/3d/u_track.h"
#include "nel/3d/u_instance_material.h"
#include "nel/3d/material.h" // for advanced material usage
// Pacs Interface
#include "nel/pacs/u_global_position.h"
// Client
#include "entity_cl.h"
#include "entity_animation_manager.h"
#include "pacs_client.h"
#include "ig_client.h"
#include "debug_client.h"
#include "time_client.h"
#include "entities.h"
#include "ingame_database_manager.h"
#include "debug_client.h"
#include "misc.h"
#include "client_cfg.h"
#include "nel/gui/action_handler.h"
#include "interface_v3/interface_manager.h"
#include "nel/gui/group_container.h"
#include "interface_v3/guild_manager.h"
#include "interface_v3/skill_manager.h"
#include "user_entity.h"
#include "interface_v3/people_interraction.h"
#include "view.h"
#include "color_slot_manager.h"
#include "string_manager_client.h"
#include "interface_v3/bar_manager.h"
#include "continent_manager.h"
#include "connection.h"
#include "weather.h"
#include "npc_icon.h"
// Game share
#include "game_share/mission_desc.h"
#include "game_share/inventories.h"
#include "game_share/animal_type.h"
#include "interface_v3/group_in_scene.h"
//
#include "r2/editor.h"

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;
using namespace std;
using namespace CLFECOMMON;



////////////
// EXTERN //
////////////
extern UDriver					*Driver;
extern UScene					*Scene;
extern UVisualCollisionManager	*CollisionManager;
extern CEntityAnimationManager	*EAM;
extern UCamera					MainCam;
extern UTextContext				*TextContext;
extern CLFECOMMON::TCLEntityId	SlotUnderCursor;
extern CContinentManager		ContinentMngr;
extern bool						ShowInterface;


////////////
// GLOBAL //
////////////

#define RYZOM_EPSILON_SQRT_POSITION (1.f*1.f)
NLMISC::CRGBA	CEntityCL::_EntitiesColor[TypeCount];
NLMISC::CRGBA	CEntityCL::_DeadColor;
NLMISC::CRGBA	CEntityCL::_TargetColor;
NLMISC::CRGBA	CEntityCL::_GroupColor;
NLMISC::CRGBA	CEntityCL::_GuildColor;
NLMISC::CRGBA	CEntityCL::_UserMountColor;
NLMISC::CRGBA	CEntityCL::_UserPackAnimalColor;
NLMISC::CRGBA	CEntityCL::_PvpEnemyColor;
NLMISC::CRGBA	CEntityCL::_PvpNeutralColor;
NLMISC::CRGBA	CEntityCL::_PvpAllyInTeamColor;
NLMISC::CRGBA	CEntityCL::_PvpAllyInLeagueColor;
NLMISC::CRGBA	CEntityCL::_PvpAllyColor;
NLMISC::CRGBA	CEntityCL::_GMTitleColor[CHARACTER_TITLE::EndGmTitle - CHARACTER_TITLE::BeginGmTitle + 1];
uint8 CEntityCL::_InvalidGMTitleCode = 0xFF;
NLMISC::CRefPtr<CCDBNodeLeaf> CEntityCL::_OpacityMinNodeLeaf;
NLMISC::CRefPtr<CCDBNodeLeaf> CEntityCL::_ShowReticleLeaf;


// Context help
extern void contextHelp (const std::string &help);

/////////////
// METHODS //
/////////////


//-----------------------------------------------
//	showStaticFXs
//
//-----------------------------------------------
void CEntityCL::SInstanceCL::showStaticFXs()
{
	for(std::vector<UInstance>::iterator it = StaticFXs.begin(); it != StaticFXs.end(); ++it)
	{
		if (!it->empty())
		{
			(*it).show();
		}
	}

} // showStaticFXs //


//-----------------------------------------------
//	hideStaticFXs
//
//-----------------------------------------------
void CEntityCL::SInstanceCL::hideStaticFXs()
{
	for(std::vector<UInstance>::iterator it = StaticFXs.begin(); it != StaticFXs.end(); ++it)
	{
		if (!it->empty())
		{
			(*it).hide();
		}
	}

} // hideStaticFXs //


//---------------------------------------------------
//---------------------------------------------------
bool CEntityCL::SInstanceCL::createLoading(const string &strShapeName, const string &strStickPoint, sint texture, bool clearIfFail)
{
	// create the new instance
	UInstance	newInst;
	if(!strShapeName.empty())
	{
		newInst= Scene->createInstance(strShapeName);
		// if fails to create and not clearIfFail, return
		if(newInst.empty() && !clearIfFail)
			return false;
	}

	// Remove the old loading instance.
	if(!Loading.empty())
	{
		Scene->deleteInstance(Loading);
		LoadingName = "";
	}

	// if the new instance is NULL, then clean ALL
	if( newInst.empty() )
	{
		if(!Current.empty())
		{
			releaseStaticFXs();
			Scene->deleteInstance(Current);
			CurrentName = "";
		}
	}
	// else setup into loading
	else
	{
		Loading = newInst;
		LoadingName = strShapeName;
		TextureSet = texture;
		StickPoint = strStickPoint;
		Loading.hide();

		// Select the texture.
		if(texture != -1)
		{
			// Set the right texture variation.
			Loading.selectTextureSet((uint) texture);
		}
		else
		{
			Loading.selectTextureSet(computeCurrSeason());
		}

		// Set Scale (relatively from scale exported by artists)
		Loading.setRelativeScale(_Scale);

		// Set async loading Texture Mode (faster loading)
		Loading.enableAsyncTextureMode(true);
	}

	// ok only if instance created or if shapeName empty
	return !newInst.empty() || strShapeName.empty();
}

//---------------------------------------------------
//---------------------------------------------------
void CEntityCL::SInstanceCL::setColors(sint skin, sint user, sint hair, sint eyes)
{
	ApplyColor = true;

	ACSkin = skin;
	ACUser = user;
	ACHair = hair;
	ACEyes = eyes;
}

//---------------------------------------------------
//---------------------------------------------------
void CEntityCL::SInstanceCL::setScale(const CVector &scale)
{
	// Bkup for new created loading instances
	_Scale= scale;

	// Set Scale to any created ones (relatively from scale exported by artists)
	if(!Current.empty())
		Current.setRelativeScale(_Scale);
	if(!Loading.empty())
		Loading.setRelativeScale(_Scale);
}

//---------------------------------------------------
//---------------------------------------------------
NL3D::UInstance CEntityCL::SInstanceCL::createLoadingFromCurrent()
{
	if (!Loading.empty()) return Loading;
	if (Current.empty()) return NULL;

	createLoading(CurrentName, StickPoint, TextureSet);
	if (ApplyColor)
	{
		CColorSlotManager::TIntCouple array[4];
		// Skin
		array[0].first = (uint)0; array[0].second = (uint)ACSkin;
		// User Color
		array[1].first = (uint)1; array[1].second = (uint)ACUser;
		// Hair Color
		array[2].first = (uint)2; array[2].second = (uint)ACHair;
		// Eyes Color
		array[3].first = (uint)3; array[3].second = (uint)ACEyes;
		if (!Loading.empty())
			ColorSlotManager.setInstanceSlot(Loading, array, 4);
	}

	// TODO : there is probably a bug for FX here : if we want to create a loading instance from the
	// current instance and there are some FXs binded to the current instance we have to :
	// 1 - recreate them onto the loading instance
	// or
	// 2 - unbind them from the current instance and rebind them to the loading instance

	return Loading;
}

//---------------------------------------------------
void CEntityCL::SInstanceCL::releaseStaticFXs()
{
	if (Scene)
	{
		for(std::vector<UInstance>::iterator it = StaticFXs.begin(); it != StaticFXs.end(); ++it)
		{
			if (!it->empty())
			{
				Scene->deleteInstance(*it);
			}
		}
	}
	StaticFXs.clear();
}

//---------------------------------------------------
bool CEntityCL::isAsyncLoading() const
{
	for(uint k = 0; k < _Instances.size(); ++k)
	{
		if (!_Instances[k].Loading.empty()) return true;
	}
	return false;
}


void CEntityCL::SInstanceCL::selectTextureSet(uint8 value, bool async)
{
	if (!Loading.empty())
	{
		Loading.selectTextureSet(value);
	}
	else if (!Current.empty())
	{
		Current.enableAsyncTextureMode(async);
		Current.selectTextureSet(value);
		if (async)
		{
			// NB: async case for this function not tested yet!
			Current.startAsyncTextureLoading();
		}
	}
}

//---------------------------------------------------
void CEntityCL::SInstanceCL::updateCurrentFromLoading(NL3D::USkeleton Skeleton)
{
	if(Loading.empty()) return;

	bool bShow = true;

	if (!Current.empty())
	{
		// copy temp info
		bShow = (Current.getVisibility() == UInstance::Show);
	}

	// Delete current instances
	if (!Current.empty())
	{
		Scene->deleteInstance(Current);
		CurrentName = "";
	}


	// Assign loading to current
	Current = Loading;
	CurrentName = LoadingName;
	Loading = NULL;
	LoadingName = "";


	// If there is a skeleton, bind the skin to the skeleton.
	if(!Skeleton.empty())
	{
		// No Stick Point, try to bind.
		if(StickPoint.empty())
		{
			if(!Skeleton.bindSkin(Current))
				nlwarning("Cannot bind the skin %s to the skeleton in the slot.", CurrentName.c_str());
		}
		// Try to Stick Object.
		else
		{
			sint stickID = Skeleton.getBoneIdByName(StickPoint);
			if(stickID != -1)
				Skeleton.stickObject(Current, stickID);
		}
	}

	// Show current instance
	if(bShow)
	{
		if(KeepHiddenWhenLoaded == false)
		{
			Current.show();
		}
		else
			KeepHiddenWhenLoaded = false;
	}

	releaseStaticFXs();

	// Add static fxs
	if (FXItemSheet && Scene)
	{
		uint numStaticFX = FXItemSheet->FX.getNumStaticFX();
		StaticFXs.reserve(numStaticFX);
		for(uint k = 0; k < numStaticFX; ++k)
		{
			std::string boneName = FXItemSheet->FX.getStaticFXBone(k);
			std::string name = FXItemSheet->FX.getStaticFXName(k);
			if (!boneName.empty() && !name.empty())
			{
				sint boneID = Skeleton.getBoneIdByName(boneName);
				if (boneID != -1)
				{
					UInstance instance = Scene->createInstance(name);
					if (!instance.empty())
					{
						instance.setTransformMode(UTransform::DirectMatrix);
						CMatrix mat;
						mat.setPos(FXItemSheet->FX.getStaticFXOffset(k));
						instance.setMatrix(mat);
						Skeleton.stickObject(instance, boneID);
						StaticFXs.push_back(instance);
					}
					else
					{
						nlwarning("Can't create static fx %s sticked on bone %s", name.c_str(), boneName.c_str());
					}
				}
				else
				{
					nlwarning("Can't find bone %s for static fx %s", boneName.c_str(), name.c_str());
				}
			}
		}
	}
}

//---------------------------------------------------
void	CEntityCL::SInstanceCL::setEmissive(NLMISC::CRGBA emit)
{
	// Do it on both Loading and Current, to avoid any problem
	UInstance	insts[2];
	insts[0]= Current;
	insts[1]= Loading;
	for(uint i=0;i<2;i++)
	{
		// test if instance is valid
		UInstance	inst= insts[i];
		if(inst.empty())
			continue;
		UShape		shape= inst.getShape();
		if(shape.empty())
			continue;
		uint	numMats= shape.getNumMaterials();
		if(numMats==0)
			continue;
		if(numMats!=inst.getNumMaterials())
			continue;

		// For all materials
		for(uint j=0;j<numMats;j++)
			// set emit
			inst.getMaterial(j).setEmissive(emit);
	}
}

//---------------------------------------------------
void	CEntityCL::SInstanceCL::restoreEmissive()
{
	// Do it on both Loading and Current, to avoid any problem
	UInstance	insts[2];
	insts[0]= Current;
	insts[1]= Loading;
	for(uint i=0;i<2;i++)
	{
		// test if instance is valid
		UInstance	inst= insts[i];
		if(inst.empty())
			continue;
		UShape		shape= inst.getShape();
		if(shape.empty())
			continue;
		uint	numMats= shape.getNumMaterials();
		if(numMats==0)
			continue;
		if(numMats!=inst.getNumMaterials())
			continue;

		// For all materials
		for(uint j=0;j<numMats;j++)
			// restore UShape value
			inst.getMaterial(j).setEmissive(shape.getMaterial(j).getEmissive());
	}
}

//---------------------------------------------------
void	CEntityCL::SInstanceCL::makeInstanceTransparent(uint8 opacity, uint8 opacityMin)
{
	// ensure correct opacity
	opacity= max(opacity, opacityMin);

	// choose to disable ZWrite only when opacity == opacityMin (this that what do actually previous code)
	bool	disableZWrite= (opacity!=255) && (opacity==opacityMin);

	// Do it on both Loading and Current, to avoid any problem
	UInstance	insts[2];
	insts[0]= Current;
	insts[1]= Loading;
	for(uint i=0;i<2;i++)
	{
		// test if instance is valid
		UInstance	inst= insts[i];
		if(inst.empty())
			continue;
		::makeInstanceTransparent(inst, opacity, disableZWrite);
	}
}

//---------------------------------------------------
void CEntityCL::SInstanceCL::setDiffuse(bool onOff, NLMISC::CRGBA diffuse)
{
	// test if instance is valid
	::setDiffuse(Current, onOff, diffuse);
	::setDiffuse(Loading, onOff, diffuse);
}



/////////////
// METHODS //
/////////////
//---------------------------------------------------
// CEntityCL :
// Dafault constructor
//---------------------------------------------------
CEntityCL::CEntityCL()
{
	// Initialize the object.
	init();
	Type = Entity;
	_SelectionFX = NULL;
	_MouseOverFX = NULL;
	_StateFX = NULL;
	_StateFXName = "";
	_GMTitle = _InvalidGMTitleCode;
	_LastLocalSelectBoxComputeTime = 0;
	_InSceneInterfaceEnabled = true;
}// CEntityCL //

//---------------------------------------------------
// ~CEntityCL :
// Destructor
//---------------------------------------------------
CEntityCL::~CEntityCL()
{
	// Remove the visual collision entity (snaped).
	if(CollisionManager && _CollisionEntity)
	{
		CollisionManager->deleteEntity(_CollisionEntity);
		_CollisionEntity = 0;
	}

	// If there still is a scene -> delete pointers.
	if(Scene)
	{
		// Remove skeleton.
		if(!_Skeleton.empty())
		{
			Scene->deleteSkeleton(_Skeleton);
			_Skeleton = 0;
		}
		// Remove Instance.
		if(!_Instance.empty())
		{
			Scene->deleteInstance(_Instance);
			_Instance = 0;
		}

		// Delete Instances.
		for(uint i = 0; i < _Instances.size(); ++i)
		{
			if(!_Instances[i].Loading.empty())
			{
				Scene->deleteInstance(_Instances[i].Loading);
			}
			if(!_Instances[i].Current.empty())
			{
				Scene->deleteInstance(_Instances[i].Current);
			}
		}
	}
	// No more scene -> reset pointers.
	else
	{
		_Skeleton = 0;
		_Instance = 0;
	}

	// Remove the collision entity.
	removeCollisionEntity();
	// Remove the primitive.
	removePrimitive();

	// Release the animation playlist
	if(_PlayList)
	{
		EAM->deletePlayList(_PlayList);
		_PlayList = 0;
	}

	if (!_StateFX.empty() && Scene)
	{
		Scene->deleteInstance(_StateFX);
		_StateFXName = "";
	}
	if (!_SelectionFX.empty() && Scene)
	{
		Scene->deleteInstance(_SelectionFX);
	}
	if (!_MouseOverFX.empty() && Scene)
	{
		Scene->deleteInstance(_MouseOverFX);
	}
	// Free the link with the parent
	parent(CLFECOMMON::INVALID_SLOT);
	// Remove Link with children
	TChildren::iterator it = _Children.begin();
	while(it != _Children.end())
	{
		// Remove the child from the list if found.
		if(skeleton() && (*it) && (*it)->skeleton())
			skeleton()->detachSkeletonSon(*((*it)->skeleton()));
		// Next Child
		it++;
	}
	// Remove the list
	_Children.clear();
}// ~CEntityCL //

//-----------------------------------------------
// init :
// Initialize the Object with this function for all constructors.
//-----------------------------------------------
void CEntityCL::init()
{
	// No parent.
	_Parent = CLFECOMMON::INVALID_SLOT;
	// No entry for the moment.
	_DBEntry = 0;

//	_Name = NULL;

	// Bot Objects flags
	_DisplayInRadar= true;
	_DisplayOSDName= true;
	_DisplayOSDBars= true;
	_DisplayOSDForceOver= false;
	_Traversable= true;
	_CanTurn = true; // must be initialized beforce calling front()
	_ForbidClipping = false;

	// Entity Up.
	up(CVector(0.f, 0.f, 1.f));
	// Orientation of the entity.
	front(CVector(0.f, 1.f, 0.f));
	// Current direction for the entity.
	_DirMatrix.identity();
	dir(CVector(0.f, 1.f, 0.f));
	// Position of the entity.
	_Position = CVectorD(0.f, 0.f, 0.f);
	// Initialize the position limiter
	_PositionLimiter = CVectorD(0.0, 0.0, 0.0);

	// Entity is not flyer at the beginning.
	_Flyer = false;
	// Initialize the mode.
	_Mode            = MBEHAV::UNKNOWN_MODE;
	_TheoreticalMode = MBEHAV::UNKNOWN_MODE;
	// Initialize the behaviour.
	_CurrentBehaviour = MBEHAV::IDLE;

	// No skeleton at the beginning.
	_Skeleton = 0;
	// No Instance at the beginning.
	_Instance = 0;
	// No primitive at the beginning.
	_Primitive = 0;
	// No collision Entity at the beginning.
	_CollisionEntity = 0;
	// No PlayList at the beginning.
	_PlayList		= 0;
	_FacePlayList	= 0;

	// AABBox
	CAABBox aabbox;
	CVector min = CVector(-0.5f, -0.5f, 0);
	CVector max = CVector( 0.5f,  0.5f, 2);
	aabbox.setMinMax(min, max);
	box(aabbox);

	// Default Clip Sphere
	_ClipRadius= aabbox.getRadius();
	_ClipDeltaZ= aabbox.getCenter().z;

	// 'true' as long as the entity has not received any position.
	_First_Pos = true;
	// No position managed for the time so the next one will be the first one.
	_FirstPosManaged = true;

	// No DataSetId initiliazed
	_DataSetId= CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;
	_NPCAlias = 0;
	// The entity is not in any slot for the time.
	_Slot		= CLFECOMMON::INVALID_SLOT;
	// The entity has no target for the time.
	_TargetSlot	= CLFECOMMON::INVALID_SLOT;
	_TargetSlotNoLag = CLFECOMMON::INVALID_SLOT;

	// init _LogicInfo3D
	_LogicInfo3D.Self= this;

	// Async texture
	_AsyncTextureLoading= false;
	_LodTextureDirty= false;

	// Angle to have with the target when in combat mode with.
	_TargetAngle = 0.0;

	// Must do a setGlobalPosition
	_SetGlobalPositionDone = false;
	_SnapToGroundDone = false;

	// Entity are not displayed at the beginning.
	_Displayable = true;
	//
	_Clipped = false;

	if (ClientCfg.Local)
	{
		_Title = "Newbie";
		_HasReservedTitle = false;
		_EntityName = "Name";
	}
	_NameId = 0;

	_PermanentStatutIcon.clear();

	// Not a mission target by default
	_MissionTarget = false;
	// The entity has not moved for the time.
	_HasMoved = false;
	_TranspFactor = 0.0f;
	_IsInTeam= false;
	_IsUserMount= false;
	_IsUserPackAnimal= false;

	// GroundTypeCache
	_GroundTypeCachePos= CVectorD::Null;
	_GroundTypeCache= 0;

	_StateFX = NULL;
	_StateFXName = "";
	_SelectionFX = NULL;
	_MouseOverFX = NULL;

	_SomeInstanceCastShadowMap= false;
	_ShadowMapZDirClamp= ClientCfg.ShadowZDirClampLandscape;
	_ShadowMapMaxDepth= ClientCfg.ShadowMaxDepthLandscape;
	_ShadowMapPropertyLastUpdate= 0;

#ifdef TMP_DEBUG_GUIGUI
	_TheoreticalPosition = CVectorD(0.f, 0.f, 0.f);
	_TheoreticalOrientation = -10.0f;	// Init value to know if it has been changed.
#endif // TMP_DEBUG_GUIGUI

	_VisualSelectionTime= 0;
	_VisualSelectionBlinked= false;
	_InSceneInterfaceEnabled = true;
}// init //

//-----------------------------------------------
// initialize :
// Method to call to initialize all members of the right class.
//-----------------------------------------------
void CEntityCL::initialize()
{
	// Initialize the primitive.
	initPrimitive(0.5f, 2.0f, 0.0f, 0.0f, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, MaskColNone, MaskColNone);
	// Create the collision entity (used to snap the entity to the ground).
	computeCollisionEntity();
	// Initialize properties of the client.
	initProperties();
}// initialize //

//-----------------------------------------------
// initPrimitive :
//
//-----------------------------------------------
bool CEntityCL::initPrimitive(float radius, float height, float length, float width, UMovePrimitive::TReaction reactionType, UMovePrimitive::TTrigger triggerType, UMovePrimitive::TCollisionMask occlusionMask, UMovePrimitive::TCollisionMask collisionMask, float clipRadius, float clipHeight)
{
	// Check primitive, there should not be a primitive because it could have been created with another PACS and crash(continent changed).
	if(_Primitive)
	{
		nlwarning("ENT:initPrimitive:%d: There is already a primitive -> _Primitive = 0.", _Slot);
		_Primitive = 0;
	}

	// **** Create the primitive
	bool	primitiveOk= false;
	if(PACS)
	{
		_Primitive = PACS->addCollisionablePrimitive(dynamicWI, 1);
		if(_Primitive)
		{
			_Primitive->setReactionType(reactionType);
			_Primitive->setTriggerType(triggerType);
			_Primitive->setAbsorbtion(0);
			// Set the collision if the radius is > 0
			if(radius > 0.0f)
			{
				_Primitive->setPrimitiveType(UMovePrimitive::_2DOrientedCylinder);
				_Primitive->setRadius( std::min(radius, (float)(RYZOM_ENTITY_SIZE_MAX/2)) );
				_Primitive->setHeight(height+ClientCfg.PrimitiveHeightAddition);
				CVector min = CVector(-radius, -radius,   0.0f);
				CVector max = CVector( radius,  radius, height);
				_Aabbox.setMinMax(min, max);
				_Primitive->setOcclusionMask(occlusionMask);	// This is an npc.
			}
			// Maybe be a Box
			else if(length > 0.0f)
			{
				_Primitive->setPrimitiveType(UMovePrimitive::_2DOrientedBox);
				_Primitive->setSize(std::min(length, (float)(RYZOM_ENTITY_SIZE_MAX/2)), std::min(width, (float)(RYZOM_ENTITY_SIZE_MAX/2)));
				_Primitive->setHeight(height+ClientCfg.PrimitiveHeightAddition);
				if(length > width)
				{
					CVector min = CVector(-length, -length,   0.0f);
					CVector max = CVector( length,  length, height);
					_Aabbox.setMinMax(min, max);
				}
				else
				{
					CVector min = CVector(-width, -width,   0.0f);
					CVector max = CVector( width,  width, height);
					_Aabbox.setMinMax(min, max);
				}
				_Primitive->setOcclusionMask(occlusionMask);	// This is an npc.
			}
			// Non-collisionnable entity
			else
			{
				_Primitive->setOcclusionMask(MaskColNone);	// This is an npc.
				// still have some _Aabbox for ???? use width for radius
				float	maxRad= max(width, 0.5f);
				CVector min = CVector(-maxRad, -maxRad,   0.0f);
				CVector max = CVector( maxRad,  maxRad, height);
				_Aabbox.setMinMax(min, max);
			}
			_Primitive->setCollisionMask(collisionMask);
			_Primitive->setObstacle(true);
			_Primitive->UserData = UserDataEntity;

			if (_DataSetId != CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
				_Primitive->UserData += (((uint64)_DataSetId)<<16);

			primitiveOk= true;
		}
		else
			nlwarning("CEntityCL::initPrimitive:%d: Cannot create the _Primitive.", _Slot);
	}
	else
		nlwarning("CEntityCL::initPrimitive:%d: PACS not allocated -> _Primitive not created.", _Slot);

	// **** setup the 3D Clip Sphere.
	// if clip radius not specified, deduce from collision
	if(clipRadius<=0.f)
	{
		// Cylinder Colision?
		if(radius > 0.0f)
		{
			clipRadius= radius;
		}
		// Box Colision?
		else if(length > 0.0f)
		{
			clipRadius= max(width, length);
		}
		// Non-collisionnable entity
		else
		{
			// Backward compatibility: use width for radius
			clipRadius= width;
		}

		// at least 0.5f clip radius
		clipRadius= max(clipRadius, 0.5f);
	}
	// if clip height not specified
	if(clipHeight<=0.f)
	{
		// at least 0.5f clip height, with height from collision
		clipHeight= max(height, 0.5f);
	}
	// Mount the sphere around the Clip cylinder (over estimate)
	CAABBox		clipBox;
	clipBox.setMinMax(CVector(-clipRadius, -clipRadius,   0.0f), CVector( clipRadius,  clipRadius, clipHeight));
	// Compute the local Sphere info
	_ClipRadius= clipBox.getRadius();
	_ClipDeltaZ= clipBox.getCenter().z;		// NB different from radius, since radius>center.z


	return primitiveOk;
}// initPrimitive //


//-----------------------------------------------
// initProperties :
// Initialize properties of the entity (according to the class).
//-----------------------------------------------
void CEntityCL::initProperties()
{
	properties().selectable(true);
}// initProperties //


//-----------------------------------------------
// updateVisualProperty :
// Update a visual property from the database.
// \param gameCycle : when this was sent.
// \param prop : the property to udapte.
//-----------------------------------------------
void CEntityCL::updateVisualProperty(const NLMISC::TGameCycle &gameCycle, const uint &prop, const NLMISC::TGameCycle &predictedInterval)
{
	CCDBNodeBranch *nodePtr = IngameDbMngr.getNodePtr();
	if (nodePtr)
	{
		CCDBNodeBranch	*nodeRoot = dynamic_cast<CCDBNodeBranch*>(nodePtr->getNode(0));
		if(!nodeRoot)
		{
			nlwarning("CEntityCL::updateVisualProperty : There is no entry in the DB for entities (current slot %d).", _Slot);
			return;
		}

		CCDBNodeBranch *nodGrp = dynamic_cast<CCDBNodeBranch*>(nodeRoot->getNode(_Slot));
		if(nodGrp == 0)
		{
			nlwarning("CEntityCL::updateVisualProperty : Cannot find the entity '%d' in the database.", _Slot);
			return;
		}

		// Get The property ptr.
		CCDBNodeLeaf	*nodeProp	= dynamic_cast<CCDBNodeLeaf*>(nodGrp->getNode(prop));
		if(nodeProp == 0)
		{
			nlwarning("CEntityCL::updateVisualProperty : Cannot find the property '%d' for the slot %d.", prop, _Slot);
			return;
		}

		switch(prop)
		{
		case PROPERTY_POSITION:
			updateVisualPropertyPos(gameCycle, nodeProp->getValue64(), predictedInterval);
			break;

		case PROPERTY_ORIENTATION:
			updateVisualPropertyOrient(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_BEHAVIOUR:
			updateVisualPropertyBehaviour(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_NAME_STRING_ID:
			updateVisualPropertyName(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_TARGET_ID:
			updateVisualPropertyTarget(gameCycle, nodeProp->getValue64());

			break;

		case PROPERTY_MODE:
			updateVisualPropertyMode(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_VPA:
			updateVisualPropertyVpa(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_VPB:
			updateVisualPropertyVpb(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_VPC:
			updateVisualPropertyVpc(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_ENTITY_MOUNTED_ID:
			updateVisualPropertyEntityMounted(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_RIDER_ENTITY_ID:
			updateVisualPropertyRiderEntity(gameCycle, nodeProp->getValue64());
			break;

		case PROPERTY_TARGET_LIST_0:
		case PROPERTY_TARGET_LIST_1:
		case PROPERTY_TARGET_LIST_2:
		case PROPERTY_TARGET_LIST_3:
			updateVisualPropertyTargetList(gameCycle, nodeProp->getValue64(), prop - PROPERTY_TARGET_LIST_0);
		break;

		case PROPERTY_VISUAL_FX:
			updateVisualPropertyVisualFX(gameCycle, nodeProp->getValue64());
		break;

		// Property to update the contextual menu, and some important status
		case PROPERTY_CONTEXTUAL:
			updateVisualPropertyContextual(gameCycle, nodeProp->getValue64());
		break;

		case PROPERTY_BARS:
			updateVisualPropertyBars(gameCycle, nodeProp->getValue64());
		break;

		case PROPERTY_GUILD_SYMBOL:
			updateVisualPropertyGuildSymbol(gameCycle, nodeProp->getValue64());
		break;

		case PROPERTY_GUILD_NAME_ID:
			updateVisualPropertyGuildNameID(gameCycle, nodeProp->getValue64());
		break;

		case PROPERTY_EVENT_FACTION_ID:
			updateVisualPropertyEventFactionID(gameCycle, nodeProp->getValue64());
		break;

		case PROPERTY_PVP_MODE:
			updateVisualPropertyPvpMode(gameCycle, nodeProp->getValue64());
		break;

		case PROPERTY_PVP_CLAN:
			updateVisualPropertyPvpClan(gameCycle, nodeProp->getValue64());
		break;

		case PROPERTY_OWNER_PEOPLE:
			updateVisualPropertyOwnerPeople(gameCycle, nodeProp->getValue64());
		break;

		case PROPERTY_OUTPOST_INFOS:
			updateVisualPropertyOutpostInfos(gameCycle, nodeProp->getValue64());
		break;

			//	case PROPERTY_STATUS:
			//		updateVisualPropertyStatus(gameCycle, nodeProp->getValue64());
			//		break;

		default:
			nlwarning("CEntityCL::updateVisualProperty : Unknown Property '%d' for the entity in the slot '%d'.", prop, _Slot);
			break;
		}
	}
}// updateVisualProperty //


//-----------------------------------------------
// updateVisualPropertyContextual
//-----------------------------------------------
void CEntityCL::updateVisualPropertyContextual(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	_Properties.selectable( CProperties( (uint16) prop ).selectable() );
	_Properties.talkableTo( CProperties( (uint16) prop ).talkableTo() );
	_Properties.attackable( CProperties( (uint16) prop ).attackable() );
	_Properties.mountable( CProperties( (uint16) prop ).mountable() );
	_Properties.lootable( CProperties( (uint16) prop ).lootable() );
	_Properties.harvestable( CProperties( (uint16) prop ).harvestable() );
	_Properties.afk( CProperties( (uint16) prop ).afk() );
}

//-----------------------------------------------
// skeleton :
// Set the skeleton for the entity or an empty string to remove a skeleton.
// \param filename : file with the skeleton to apply to the entity or empty string to remove a skeleton.
//-----------------------------------------------
USkeleton *CEntityCL::skeleton(const string &filename)
{
	// Check there is a scene.
	if(!Scene)
	{
		pushDebugStr("No scene allocated -> Cannot create the skeleton.");
		_Skeleton = 0;
		return skeleton();
	}

	// Remove the old skeleton.
	if(!_Skeleton.empty())
	{
		Scene->deleteSkeleton(_Skeleton);
	}

	// If the filename is not empty -> changes the skeleton.
	if(filename.empty())
	{
		pushDebugStr("skeleton filename is empty -> Cannot create the skeleton.");
		return skeleton();
	}

	// Create the skeleton.
	_Skeleton = Scene->createSkeleton(filename);
	if(!_Skeleton.empty())
	{
		// Set default Transform mode -> RotQuat.
		_Skeleton.setTransformMode(UTransformable::RotQuat);
		// Initialize the playlist a bit.
		if(_PlayList)
		{
			// Register the skeleton to the playlist.
			_PlayList->registerTransform(_Skeleton);

			// \todo GUIGUI : is this location right for this ?
			// Animation should not move alone.
			uint posTmp = EAM->getAnimationSet()->getChannelIdByName("pos");
			if(posTmp != UAnimationSet::NotFound)
				_PlayList->enableChannel(posTmp, false);
			else
				pushDebugStr("Channel 'pos' not found.");

			// Animation should not rotate alone.
			uint rotquatTmp = EAM->getAnimationSet()->getChannelIdByName("rotquat");
			if(rotquatTmp != UAnimationSet::NotFound)
				_PlayList->enableChannel(rotquatTmp, false);
			else
				pushDebugStr("Channel 'rotquat' not found.");
		}
		//
		else
			pushDebugStr("Playlist no allocated -> skeleton not registered.");

		// Attach Light Information request to the skeleton
		_Skeleton.setLogicInfo(&_LogicInfo3D);

		// Enable user clipping
		_Skeleton.setUserClipping (true);

		// Update Shadow
		updateCastShadowMap();
	}
	else
		pushDebugStr(toString("Cannot create the Skeleton '%s', the file probably not exist.", filename.c_str()));

	// Return the skeleton pointer.
	return skeleton();
}// skeleton //


//-----------------------------------------------
// addInstance :
// Add an instance to the list of instance composing the entity.
// \param shapeName : shape filename.
// \param stickPoint : Name of the bone to stick on.
// \param texture : texture to use (in multi texture) or -1 for default texture.
// \param instIdx : if not CEntityCL::BadIndex, the instance will replace the one at this index.
// \return uint32 : index of the instance created, or CEntityCL::BadIndex.
//-----------------------------------------------
uint32 CEntityCL::addInstance(const string &shapeName, const std::string &stickPoint, sint texture, uint32 instIdx)
{
	// Future Index for the instance.
	uint32 idx;
	// Replace Instance
	if(instIdx < _Instances.size()) // And so instIdx != CEntityCL::BadIndex
	{
		// Index given from parameters so do not change.
		idx = instIdx;
	}
	else
	{
		if(instIdx != CEntityCL::BadIndex)
			nlwarning("CEntityCL::addInst:%d: given index '%d' is not valid.", _Slot, instIdx);
		idx = instIdx = CEntityCL::BadIndex;
	}

	// If the shape is empty, just clean and leave.
	if(shapeName.empty())
	{
		if(instIdx == CEntityCL::BadIndex)
			nlwarning("CEntityCL::addInst:%d: shape empty but given index was invalid, so nothing to clean and cannot add an empty shape.", _Slot);

		// clean the instance
		if(instIdx<_Instances.size())
			_Instances[instIdx].createLoading(string(), stickPoint, texture);

		return idx;
	}
	// else create
	else
	{
		// Create new instance slot?
		if(instIdx == CEntityCL::BadIndex)
		{
			idx= (uint32)_Instances.size();
			_Instances.push_back(SInstanceCL());
		}

		// load, but dont replace instance if fails (yoyo: I keep the same behavior than before )
		if( !_Instances[idx].createLoading(shapeName, stickPoint, texture, false) )
		{
			// If fails to load....
			nlwarning("CEntityCL::addInstance : Cannot create the instance '%s' for the slot %d.", shapeName.c_str(), _Slot);

			// if the index was just being created, erase array entry
			if(instIdx == CEntityCL::BadIndex)
			{
				_Instances.pop_back();
				idx= CEntityCL::BadIndex;
			}
		}
	}

	// Return the instance index.
	return idx;
}// addInstance //


//---------------------------------------------------
// show :
// Show or Hide the entity.
// \param s : if 'true' = entity visible, else invisible.
//---------------------------------------------------
void CEntityCL::show(bool s)
{
	if(!_Skeleton.empty())
	{
		if(s)
			_Skeleton.show();
		else
			_Skeleton.hide();
	}
	else if(!_Instance.empty())
	{
		if(s)
			_Instance.show();
		else
			_Instance.hide();
	}
	else
	{
		if (s)
		{
			for(std::vector<SInstanceCL>::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
			{
				if (!it->Current.empty()) it->Current.show();
			}
		}
		else
		{
			for(std::vector<SInstanceCL>::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
			{
				if (!it->Current.empty()) it->Current.hide();
			}
		}
	}
}// show //

//-----------------------------------------------
// hideSkin :
// hide the entity Skin (all entity instances).
// todo GUIGUI : for video, it shouldn't be a problem, but we should improve with the current which could not be ready yet while loading not done
//-----------------------------------------------
void CEntityCL::hideSkin()
{
	const uint nbInst = (uint)_Instances.size();
	for(uint i = 0; i<nbInst; ++i)
	{
		if(!_Instances[i].Current.empty())
			_Instances[i].Current.hide();
	}
}// hideSkin //


//---------------------------------------------------
// up :
// Set the vector up of the entity and normalize.
//---------------------------------------------------
void CEntityCL::up(const CVector &vect)
{
	// If the param is vector Null -> problem so do not changed the vector.
	if(vect == CVector::Null)
	{
		nlwarning("CEntityCL::up : attempt to set the vector up with a vector Null. Up not changed !");
		return;
	}

	// Compute the up vector
	_Up = vect;
	_Up.normalize();
}// up //

//---------------------------------------------------
// front :
// Change the entity front vector.
// \param vect : new vector to use for the front.
// \param compute : adjust the param 'vect' to be valid or leave the old front unchanged if impossible.
// \param check : warning if the param 'vect' is not valid to be the front (vector Null) even with compute=true.
// \param forceTurn : set front even if the entity cannot turn
// \return bool : 'true' if the front has been filled, else 'false'.
//---------------------------------------------------
bool CEntityCL::front(const CVector &vect, bool compute, bool check, bool forceTurn)
{
	if (!forceTurn && !_CanTurn)
		return false;
	return setVect(_Front, vect, compute, check);
}// front //

//---------------------------------------------------
// dir :
// Change the entity direction vector.
// \param vect : new vector to use for the direction.
// \param compute : adjust the param 'vect' to be valid or leave the old direction unchanged if impossible.
// \param check : warning if the param 'vect' is not valid to be the direction (vector Null) even with compute=true.
// \return bool : 'true' if the direction has been filled, else 'false'.
//---------------------------------------------------
bool CEntityCL::dir(const CVector &vect, bool compute, bool check)
{
	if(setVect(_Dir, vect, compute, check))
	{
		// Compute the direction Matrix
		CVector vi = _Dir^up();
		CVector vk = vi^_Dir;
		_DirMatrix.setRot(vi, _Dir, vk, true);
		return true;
	}
	return false;
}// dir //


//---------------------------------------------------
// posBox :
// Change the box position.
// \param pos contains the new box position.
//---------------------------------------------------
void CEntityCL::posBox(const CVector &pos)
{
	CVector halfSize = _Aabbox.getHalfSize();
	halfSize.x = 0; halfSize.y = 0;
	_Aabbox.setCenter(pos+halfSize);
}// posBox //

//---------------------------------------------------
// box :
// Set the box.
// \param box : an axis aligned bounding box to apply to the entity.
//---------------------------------------------------
void CEntityCL::box(const CAABBox &box)
{
	// Copy the Box.
	_Aabbox = box;
	// position the box around the entity.
	posBox(pos());
}// box //


//-----------------------------------------------
// lastFramePACSPos :
// Return the last frame PACS position.
//-----------------------------------------------
bool CEntityCL::lastFramePACSPos(NLPACS::UGlobalPosition &result) const
{
	if(_Primitive)
	{
		result = _LastFramePACSPos;
		if(result.InstanceId != -1)
			return true;
		else
			return false;
	}
	else
		return false;
}// lastFramePACSPos //

//-----------------------------------------------
// currentPACSPos :
// Return the current PACS position.
//-----------------------------------------------
bool CEntityCL::currentPACSPos(NLPACS::UGlobalPosition &result) const
{
	if(_Primitive)
	{
		_Primitive->getGlobalPosition(result, dynamicWI);
		if(result.InstanceId != -1)
			return true;
		else
			return false;
	}
	else
		return false;
}// currentPACSPos //


//---------------------------------------------------
// pacsPos :
// Change the PACS position and the entity position too.
// \todo GUIGUI : do we have to get the position from pacs or from the parameter ?
//---------------------------------------------------
void CEntityCL::pacsPos(const CVectorD &vect, const UGlobalPosition &globPos)
{
	// Set then entity position
	pos(vect);

	if(!_Primitive)
		return;

	if(!GR)
	{
		nlwarning("CEntityCL::pacsPos : Global Retriever not allocated.");
		return;
	}

	// Use the precise PACS position if possible
	if( globPos.InstanceId != -1 /*&& ContinentMngr.getCurrentContinentSelectName() == "newbieland"*/ )
	{
		_Primitive->setGlobalPosition(globPos, dynamicWI);
	}
	// NB: else use the 3D position. => the Z may be different than the Z wanted! (according to the pacs pos found)
	else
	{
		_Primitive->setGlobalPosition(vect, dynamicWI, UGlobalPosition::Unspecified);
	}


	// Get the global position
	UGlobalPosition gPos;
	_Primitive->getGlobalPosition(gPos, dynamicWI);
	_FinalPacsPos = GR->getDoubleGlobalPosition(gPos);

	// Check the new position is valid
	if (gPos.InstanceId != -1)
	{
		_SetGlobalPositionDone = true;
		_LastRetrievedPosition = vect;
		_LastRetrievedPacsPosition = gPos;
	}

	// If the entity do not fly, update Z value (X,Y should be the same or close and to avoid a pb with a too small move, just keep them)
	if(!flyer())
		pos().z = _FinalPacsPos.z;
}// pacsPos //



//---------------------------------------------------
// pacsMove :
// Move the PACS position and the entity position too.
//---------------------------------------------------
void CEntityCL::pacsMove(const CVectorD &vect)
{
	// Entity has not moved.
	_HasMoved = false;
	// If there is a PACS primitive -> changes the PACS position.
	if(!_Primitive)
	{
		// Set then entity position
		pos(vect);

		return;
	}

	if(!GR)
	{
		// Set then entity position
		pos(vect);

		nlwarning("CEntityCL::pacsMove : Global Retriever not allocated.");
		return;
	}

	if ( _SetGlobalPositionDone )
	{
		// Change the position of the entity in PACS and apply the new position to the entity.
		CVectorD deltaPos ((vect.x - _FinalPacsPos.x)/ DT, (vect.y - _FinalPacsPos.y)/ DT, 0);
		if ((fabs (deltaPos.x) > 0.05) || (fabs (deltaPos.y) > 0.05))
		{
			_HasMoved = true;
			_Primitive->move (deltaPos, dynamicWI);
		}
	}
	else
	{
		_Primitive->setGlobalPosition(vect, dynamicWI);

		// Check the new position is valid
		UGlobalPosition gPos;
		_Primitive->getGlobalPosition(gPos, dynamicWI);
		if (gPos.InstanceId != -1)
		{
			_SetGlobalPositionDone = true;
			_LastRetrievedPosition = vect;
			_LastRetrievedPacsPosition = gPos;
		}
	}

	// Set the entity position
	pos(vect);

}// pacsMove //

//-----------------------------------------------
// updateDisplay :
// Update the PACS position after the evalCollision. The entity position is set too. This is fast.
// If the entity position is too far from its PACS position, setGlobalPosition is called.
// After this call, the position.z is valid.
//-----------------------------------------------
void CEntityCL::pacsFinalizeMove()	// virtual
{
	if(_Primitive == 0)
	{
		return;
	}

	// Get the global position
	_FinalPacsPos = _Primitive->getFinalPosition (dynamicWI);

	// Get the global position
	UGlobalPosition gPos;
	_Primitive->getGlobalPosition(gPos, dynamicWI);
	if (gPos.InstanceId != -1)
	{
		if(ClientCfg.UsePACSForAll && (_Mode == MBEHAV::COMBAT || _Mode == MBEHAV::COMBAT_FLOAT))
		{
			pos(_FinalPacsPos);
		}
		else
		{
			// Delta between real and evaluated position
			CVectorD deltaPos = pos();
			deltaPos -= _FinalPacsPos;

			// Too far from real value ?
			if ( ((deltaPos.x*deltaPos.x+deltaPos.y*deltaPos.y) > (double)RYZOM_EPSILON_SQRT_POSITION) )
			{
				pacsPos (pos ());
			}

			// If the entity do not fly, update Z value (X,Y should be the same or close and to avoid a pb with a too small move, just keep them)
			if(!flyer())
				pos().z = _FinalPacsPos.z;
		}
	}
	else
	{
		_SetGlobalPositionDone = false;
		_SnapToGroundDone = false;
	}
}// pacsFinalizeMove //

//---------------------------------------------------
// updateDisplay :
// Get the entity position and set all visual stuff with it.
// \todo GUIGUI : put this method 'virtual' to have a different code for the user (no playlist).
// \todo GUIGUI : manage the parent better.
//---------------------------------------------------
void CEntityCL::updateDisplay(CEntityCL * /* parent */)
{
}// updateDisplay //


//-----------------------------------------------
// setCluster :
// Set the cluster system for the current entity and all of its chidren.
//-----------------------------------------------
void CEntityCL::setClusterSystem(UInstanceGroup *cluster)
{
	// Place the skeleton in the right cluster.
	if(!_Skeleton.empty())
		_Skeleton.setClusterSystem(cluster);
	else if(!_Instance.empty())
		_Instance.setClusterSystem(cluster);
	else
	{
		for(std::vector<SInstanceCL>::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
		{
			if (!it->Current.empty())
			{
				it->Current.setClusterSystem(cluster);
			}
		}
	}

	// Update the children display.
	std::list<CEntityCL *>::iterator it = _Children.begin();
	while(it != _Children.end())
	{
		// Update the display for the child
		(*it)->setClusterSystem(cluster);
		// Next Child.
		++it;
	}
}// setCluster //

// *****************************************************************************************************
NL3D::UInstanceGroup *CEntityCL::getClusterSystem()
{
	if (!_Skeleton.empty()) return _Skeleton.getClusterSystem();
	if (!_Instance.empty()) return _Instance.getClusterSystem();
	return NULL;
}

//-----------------------------------------------
// updateCluster :
// Choose the right cluster according to the entity position.
//-----------------------------------------------
void CEntityCL::updateCluster()
{
	// Check there is a primitive.
	if(_Primitive)
	{
		// Get the global position.
		UGlobalPosition gPos;

		_Primitive->getGlobalPosition(gPos, dynamicWI);
		// Set the new cluster.
		setClusterSystem(getCluster(gPos));
	}
}// updateCluster //


//---------------------------------------------------
// snapToGround :
// Snap entity to the ground.
//---------------------------------------------------
void CEntityCL::snapToGround()
{
	// While PACS not done, do not try to snap.
	if(_SetGlobalPositionDone == false)
		return;

	// If the entity is a flyer no snap to ground will be done.
	if(!flyer())
	{
		CVector vect;
		bool needSnap = true;

		// Check there is a primitive.
		if(_Primitive && GR)
		{
			// Get the right pos from PACS.
			UGlobalPosition gPos;
			_Primitive->getGlobalPosition(gPos, dynamicWI);

			// Get the final position from pacs and after the snap if on landscape.
			vect = GR->getGlobalPosition(gPos);
			// if on interior, no need to snap
			if(GR->isInterior(gPos))
				needSnap = false;
			// if on water, snap on water
			else
			{
				float waterHeight = 0.0f;
				if(GR->isWaterPosition(gPos, waterHeight))
				{
					if ( isUser() || isPlayer() || isNPC())
					{
						
						float waterOffset = ClientCfg.WaterOffset;
						switch(people())
						{
							case EGSPD::CPeople::Unknown :
								break;
							case EGSPD::CPeople::Fyros :
								waterOffset = ClientCfg.FyrosWaterOffset;
								break;
							case EGSPD::CPeople::Matis :
								waterOffset = ClientCfg.MatisWaterOffset;
								break;
							case EGSPD::CPeople::Tryker :
								waterOffset = ClientCfg.TrykerWaterOffset;
								break;
							case EGSPD::CPeople::Zorai :
								waterOffset = ClientCfg.ZoraiWaterOffset;
								break;
							default:
								break;
						}

						vect.z = waterHeight + waterOffset;
					}
					else // creature
					{
						vect.z = waterHeight + ClientCfg.WaterOffsetCreature;
					}

					needSnap= false;
				}
			}
		}
		// No primitive, vect = current pos.
		else
			vect = pos();

		// Snap to the ground.
		if(needSnap && _CollisionEntity)
		{
			// for Snap on landscape, use always current 3d position, for snap cache performance.
			vect = pos();

			// Have i to force the snap to ground ?
			if (!_SnapToGroundDone)
			{
				CVector vectNull = CVector::Null;
				_CollisionEntity->snapToGround(vectNull);
			}
			_SnapToGroundDone = _CollisionEntity->snapToGround(vect);
		}

		// Change the entity position.
		pos().z = vect.z;
	}

	// Set the box position.
	posBox(pos());
}// snapToGround //


//---------------------------------------------------
// chooseRandom :
//
//---------------------------------------------------
string chooseRandom( const vector<string>& sounds, uint32& previousIndex )
{
	uint32 randomIndex = rand()%sounds.size();
	if( randomIndex == previousIndex )
	{
		randomIndex = (randomIndex+1)%sounds.size();
	}
	previousIndex = randomIndex;


	return sounds[randomIndex];
}// chooseRandom //


//---------------------------------------------------
// CEntityLogicInfo3D::getStaticLightSetup() callBack
//---------------------------------------------------
void		CEntityLogicInfo3D::getStaticLightSetup(NLMISC::CRGBA sunAmbient, std::vector<NL3D::CPointLightInfluence> &pointLightList,
		uint8 &sunContribution, NLMISC::CRGBA &localAmbient)
{
	// Is the instance in a InstanceGroup???
	UInstanceGroup			*igUnderInstance= NULL;
	const UMovePrimitive	*movePrim= Self->getPrimitive();
	if(Self->skeleton() && movePrim)
		igUnderInstance= Self->skeleton()->getClusterSystem();

	// if it is, retrieve static light from the IG.
	if(igUnderInstance)
	{
		// get the logic position of the entity.
		UGlobalPosition gPos;
		movePrim->getGlobalPosition(gPos, dynamicWI);

		// retrieve collision identifier from Pacs.
		uint	localRetrieverId= GR->getLocalRetrieverId(gPos);

		// get static light info from ig
		igUnderInstance->getStaticLightSetup(sunAmbient, localRetrieverId, gPos.LocalPosition.Surface,
			gPos.LocalPosition.Estimation, pointLightList, sunContribution, localAmbient);
	}
	// else retrieve from landscape
	else
	{
		// Use VisualCollisionEntity to retrieve info from landscape.
		UVisualCollisionEntity	*colEnt= Self->getCollisionEntity();
		if(colEnt)
		{
			colEnt->getStaticLightSetup(sunAmbient, Self->pos(), pointLightList, sunContribution, localAmbient);
		}
	}
}



//-----------------------------------------------
// computePrimitive :
// Create (or re-create) a primitive.
//-----------------------------------------------
void CEntityCL::computePrimitive()
{
	// Initialize the primitive.
	initPrimitive(0.5f, 2.0f, 0.0f, 0.0f, UMovePrimitive::DoNothing, UMovePrimitive::NotATrigger, MaskColNone, MaskColNone);
	// Set the position.
	pacsPos(pos());
}// computePrimitive //

//---------------------------------------------------
// removePrimitive :
// remove the entity primitive from PACS
//---------------------------------------------------
void CEntityCL::removePrimitive()
{
	// If there is a Primitive -> remove it.
	if(_Primitive)
	{
		// Remove the primitive if PACS allocated
		if(PACS)
			PACS->removePrimitive(_Primitive);
		// Primitive removed
		_Primitive = 0;
	}
}// removePrimitive //


//-----------------------------------------------
// computeCollisionEntity :
// Create the collision entity.
//-----------------------------------------------
void CEntityCL::computeCollisionEntity()
{
	// Check for an old collision entity not removed.
	if(_CollisionEntity)
	{
		nlwarning("CEntityCL::computeCollisionEntity: There is already a collision entity in the slot %d.", _Slot);
		_CollisionEntity = 0;
	}

	// Is there a collision manager.
	if(CollisionManager)
	{
		// Create the collision entity.
		_CollisionEntity = CollisionManager->createEntity();
		if(_CollisionEntity == 0)
			nlwarning("CEntityCL::computeCollisionEntity : Cannot create the _CollisionEntity for the slot %d.", _Slot);
		else
			_CollisionEntity->setSnapToRenderedTesselation(false);
	}
}// computeCollisionEntity //

//-----------------------------------------------
// removeCollisionEntity :
// Remove the collision entity.
//-----------------------------------------------
void CEntityCL::removeCollisionEntity()
{
	// If there is a collision entity -> remove it.
	if(_CollisionEntity)
	{
		// Remove it if there is a collision manager.
		if(CollisionManager)
			CollisionManager->deleteEntity(_CollisionEntity);
		// Collision Entity Removed.
		_CollisionEntity = 0;
	}
}// removeCollisionEntity //


//-----------------------------------------------
// attackRadius :
// Method to return the attack radius of an entity
//-----------------------------------------------
double CEntityCL::attackRadius() const
{
	return 0.5;
}// attackRadius //

//-----------------------------------------------
//-----------------------------------------------
CVectorD CEntityCL::getAttackerPos(double /* ang */, double /* dist */) const
{
	return pos();
}


///////////////
// 3D SYSTEM //
///////////////
//-----------------------------------------------
// updateAsyncTexture
//-----------------------------------------------
float	CEntityCL::updateAsyncTexture()
{
	H_AUTO ( RZ_Client_Entity_CL_Update_Async_Texture )

	uint	i;

	// Check all instance to know if they need to start async load their textures
	for (i = 0; i < _Instances.size(); i++)
	{
		if (!_Instances[i].Loading.empty())
		{
			UParticleSystemInstance psi;
			psi.cast (_Instances[i].Loading);
			// dirty?
			if (_Instances[i].Loading.isAsyncTextureDirty() || !psi.empty())
			{
				// reset instance state.
				_Instances[i].Loading.setAsyncTextureDirty(false);
				// must start loading for this isntance
				_Instances[i].Loading.startAsyncTextureLoading();
				// the entity is now currently loading.
				_AsyncTextureLoading= true;
				// The LodTexture need to be recomputed
				_LodTextureDirty= true;
			}
		}
	}


	// Update Async Texture loading of all instances.
	if (_AsyncTextureLoading)
	{
		bool	allLoaded= true;
		// update loading for all instances.
		for (i  = 0; i < _Instances.size(); i++)
		{
			if(!_Instances[i].Loading.empty())
			{
				// update async texture loading
				allLoaded= allLoaded && _Instances[i].Loading.isAsyncTextureReady();
			}
		}

		// if all are loaded, then End! don't need to check all instances every frame.
		if(allLoaded)
		{
			_AsyncTextureLoading= false;

			// Transfert InstancesLoading to Instances
			for(i = 0; i < _Instances.size(); i++)
			{
				_Instances[i].updateCurrentFromLoading(_Skeleton);
				if(_Skeleton.empty() && !_Instances[i].Current.empty())
				{
					_Instances[i].Current.setPos(pos()); // force update right now (else the object would be at 0, 0, 0 for first frame)
				}

			}

			// additionally, if their is no skeleton (bot objects), may enable cast shadow here
			updateCastShadowMap();
		}
	}


	// compute distance to camera.
	// For lesser update, take the refine position (else in 3th person view, rotation will cause lot of update)
	// \todo GUIGUI : make sure pos() will return the world position all the time even when a child.
	float distToCam = (View.refinePos() - pos()).norm();

	// For LOD texture, must update the "texture distance"
	for(i = 0; i < _Instances.size(); i++)
	{
		if(!_Instances[i].Current.empty())
		{
			// update async texture loading
			_Instances[i].Current.setAsyncTextureDistance(distToCam);
		}
	}

	return distToCam;
}


//-----------------------------------------------
// updateLodTexture
//-----------------------------------------------
void	CEntityCL::updateLodTexture()
{
	H_AUTO ( RZ_Client_Entity_CL_Update_Lod_Texture )

	// if need to recompute, and if Async loading ended
	if( _LodTextureDirty && !_AsyncTextureLoading )
	{
		// clean
		_LodTextureDirty= false;
		// compute
		if(!_Skeleton.empty())
			_Skeleton.computeLodTexture();
	}
}



//-----------------------------------------------
// addChild :
// Add a new child pointer.
//-----------------------------------------------
void CEntityCL::addChild(CEntityCL *c)
{
	if(c == 0)
	{
		nlwarning("ENT:addChild:%d: Try to add a child with a Null Pointer.", _Slot);
		return;
	}

	delChild(c);
	_Children.push_back(c);

	if(skeleton() && c->skeleton())
	{
		const string stickBone = "stick_1";
		sint idBones = skeleton()->getBoneIdByName(stickBone);
		if(idBones != -1)
		{
			// Child position is relative to the parent so set its position to 0 0 0.
			c->skeleton()->setPos(CVector::Null);
			skeleton()->stickObjectEx(*(c->skeleton()), idBones, true);
		}
		else
			nlwarning("ENT:addChild:%d: the Bone '%s' does not exist", _Slot, stickBone.c_str());
	}
}// addChild //

//-----------------------------------------------
// delChild :
// Remove a new child pointer.
//-----------------------------------------------
void CEntityCL::delChild(CEntityCL *c)
{
	if(c == 0)
	{
		nlwarning("ENT:delChild:%d: Try to remove a child with a Null Pointer.", _Slot);
		return;
	}

	TChildren::iterator it = _Children.begin();
	while(it != _Children.end())
	{
		// Remove the child from the list if found.
		if((*it) == c)
		{
			if(skeleton() && c->skeleton())
				skeleton()->detachSkeletonSon(*(c->skeleton()));
			_Children.erase(it);
			break;
		}

		// Next Child
		it++;
	}
}// delChild //

//-----------------------------------------------
// parent :
// Set the new parent
//-----------------------------------------------
void CEntityCL::parent(CLFECOMMON::TCLEntityId p)
{
	// Remove the entity from the old parent.
	CEntityCL *parent = EntitiesMngr.entity(_Parent);
	if(parent)
		parent->delChild(this);

	// Initialize the new parent.
	_Parent = p;

	// Remove the entity from the old parent.
	parent = EntitiesMngr.entity(_Parent);
	if(parent)
		parent->addChild(this);
}// parent //

bool CEntityCL::clipped (const std::vector<NLMISC::CPlane> &clippingPlanes, const CVector &camPos)
{
	// If the entity still have no position, and still have no mode, entity should not be displayed.
	if(mode() == MBEHAV::UNKNOWN_MODE || _First_Pos)
		return true;

	// If the entity is not displayable, count it as clipped.
	if(!_Displayable)
		return true;

	// Use Clip position (sphere clip)
	CVector clipPos = _Aabbox.getCenter();
	clipPos.z+= _ClipDeltaZ - _Aabbox.getHalfSize().z;	// _ClipDeltaZ is relative to pos on ground

	if (!_ForbidClipping)
	{
		// Speed Clip: clip just the sphere.

		// if out of only plane, entirely out.
		const uint count = (uint)clippingPlanes.size ();
		uint i;
		for(i=0;i<count;i++)
		{
			// We are sure that pyramid has normalized plane normals.
			// if SpherMax OUT return true.
			float	d= clippingPlanes[i]*clipPos;
			if(d>_ClipRadius)
				return true;
		}
	}

	// Character clip
	float sqrdist = (camPos - clipPos).sqrnorm();
	return (sqrdist > (ClientCfg.CharacterFarClip*ClientCfg.CharacterFarClip));
}


//---------------------------------------------------
// setName :
// Set the name of the entity. Handle replacement tag if any
// to insert NPC task translated.
//---------------------------------------------------
void CEntityCL::setEntityName(const ucstring &name)
{
	_EntityName = name;
}



//---------------------------------------------------
// displayDebug :
// Display Debug Information.
//---------------------------------------------------
void CEntityCL::displayDebug(float x, float &y, float lineStep)	// virtual
{
	// Type
	TextContext->printfAt(x, y, "Type: %d", Type);
	y += lineStep;
	// Slot
	TextContext->printfAt(x, y, "Slot: %d", _Slot);
	y += lineStep;
	// Outpost
	TextContext->printfAt(x, y, "Outpost id:%d side:%s",this->getOutpostId(),OUTPOSTENUMS::toString(this->getOutpostSide()).c_str() );
	y += lineStep;
	// Name
	if(!getEntityName().empty())
		TextContext->printAt(x, y, getEntityName());
	else
		TextContext->printfAt(x, y, "Name not received");
	y += lineStep;
	// Target
	TextContext->printfAt(x, y, "Target: %d", _TargetSlot);
	y += lineStep;
	// DataSet Id
	TextContext->printfAt(x, y, "DataSet Id: %u", _DataSetId);
	y += lineStep;
	// Sheet Id
	TextContext->printfAt(x, y, "Sheet: %d(%s)", _SheetId.asInt(), _SheetId.toString().c_str());
	y += lineStep;
	// NPC Alias
	TextContext->printfAt(x, y, "NPCAlias: %u", _NPCAlias);
	y += lineStep;
	// Position
#ifndef TMP_DEBUG_GUIGUI
	TextContext->printfAt(x, y, "Pos: %f %f %f", pos().x, pos().y, pos().z);
#else
	TextContext->printfAt(x, y, "Pos: %f %f %f (Theoretical : %f %f %f)", pos().x, pos().y, pos().z, _TheoreticalPosition.x, _TheoreticalPosition.y, _TheoreticalPosition.z);
#endif
	y += lineStep;
	// Display the Target Mode.
	TextContext->printfAt(x, y, "Mode: %d(%s) (Theoretical : %d(%s))", (sint)mode(), MBEHAV::modeToString(mode()).c_str(), (sint)_TheoreticalMode, MBEHAV::modeToString(_TheoreticalMode).c_str());
	y += lineStep;
	// Display the Target Behaviour.
	TextContext->printfAt(x, y, "Behaviour: %d(%s)", (sint)behaviour(), MBEHAV::behaviourToString(behaviour()).c_str());
	y += lineStep;
	// Front
#ifndef TMP_DEBUG_GUIGUI
	TextContext->printfAt(x, y, "%f(%f,%f,%f) front", frontYaw(), front().x, front().y, front().z);
#else
	TextContext->printfAt(x, y, "%f(%f,%f,%f) front (Theoretical : %f)", frontYaw(), front().x, front().y, front().z, _TheoreticalOrientation);
#endif
	y += lineStep;
	// Direction
	TextContext->printfAt(x, y, "%f(%f,%f,%f) dir", atan2(dir().y, dir().x), dir().x, dir().y, dir().z);
	y += lineStep;
	// Last Retrieved pacs
	TextContext->printfAt(x, y, "Last PACS: (%.4f,%.4f,%.4f) (%d,%d,%.4f,%.4f,%.4f)", _LastRetrievedPosition.x, _LastRetrievedPosition.y, _LastRetrievedPosition.z,
																					_LastRetrievedPacsPosition.InstanceId, _LastRetrievedPacsPosition.LocalPosition.Surface,
																					_LastRetrievedPacsPosition.LocalPosition.Estimation.x, _LastRetrievedPacsPosition.LocalPosition.Estimation.y, _LastRetrievedPacsPosition.LocalPosition.Estimation.z);
	y += lineStep;
	// Current pacs
	UGlobalPosition	gp;
	if (_Primitive)
		_Primitive->getGlobalPosition(gp, dynamicWI);
	float	waterHeight = 0.0f;
	bool	water = (GR ? GR->isWaterPosition(gp, waterHeight) : false);
	TextContext->printfAt(x, y, "Current PACS: (%d,%d,%.4f,%.4f,%.4f) %s %.1f", gp.InstanceId, gp.LocalPosition.Surface,
																		   gp.LocalPosition.Estimation.x, gp.LocalPosition.Estimation.y, gp.LocalPosition.Estimation.z,
																		   (water ? "WATER" : "LANDSCAPE"), waterHeight);
	y += lineStep;
}// displayDebug //


//-----------------------------------------------
//-----------------------------------------------
void CEntityCL::displayDebugPropertyStages(float /* x */, float &y, float /* lineStep */)
{
}


//-----------------------------------------------
// serial :
// Serialize entity.
//-----------------------------------------------
void CEntityCL::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	readWrite(f);
	if(f.isReading())
		load();
}// serial //

//-----------------------------------------------
// readWrite :
// Read/Write Variables from/to the stream.
//-----------------------------------------------
void CEntityCL::readWrite(class NLMISC::IStream &f) throw(NLMISC::EStream)	// virtual
{
	f.serialVersion(4);

	// PUBLIC

	// PROTECTED
	f.serial(_DataSetId);
	f.serial(_SheetId);
//	NL3D::UPlayList					*_PlayList;
//	NL3D::UPlayList					*_FacePlayList;
//	NL3D::UVisualCollisionEntity	*_CollisionEntity;
//	NL3D::USkeleton					*_Skeleton;
	f.serial(_Slot);
	f.serial(_TargetSlot);
	f.serial(_Position);
	f.serialEnum(_Mode);
	f.serial(_CurrentBehaviour);
	f.serial(_Properties);
	f.serial(_EntityName);
	f.serial(_PermanentStatutIcon);
//	f.serial(_NameEx);
//	NLPACS::UMovePrimitive			*_Primitive;
//	CEntityLogicInfo3D				_LogicInfo3D;
	bool asyncTextureLoading = _AsyncTextureLoading;
	f.serial(asyncTextureLoading);
	_AsyncTextureLoading = asyncTextureLoading;
	bool lodTextureDirty = _LodTextureDirty;
	f.serial(lodTextureDirty);
	_LodTextureDirty = lodTextureDirty;
	f.serial(_Aabbox);
	f.serial(_Parent);
//	TChildren						_Children;
//	UInstance						_Instance;
//	std::vector<UInstance>			_Instances;
	f.serial(_Front);
	f.serial(_Up);
	f.serial(_Dir);
	f.serial(_DirMatrix);
	f.serial(_LastFramePos);
	bool firstPos = _First_Pos;
	f.serial(firstPos);
	_First_Pos = firstPos;
	bool firstPosManaged = _FirstPosManaged;
	f.serial(firstPosManaged);
	_FirstPosManaged = firstPosManaged;

	bool flyier = _Flyer;
	f.serial(flyier);
	_Flyer = flyier;
	f.serial(_TargetAngle);

	// PRIVATE
}// readWrite //

//-----------------------------------------------
// load :
// To call after a read from a stream to re-initialize the entity.
//-----------------------------------------------
void CEntityCL::load()	// virtual
{
}// load //


//-----------------------------------------------
// onStringAvailable :
// Callback when the name is arrived.
//-----------------------------------------------
void CEntityCL::onStringAvailable(uint /* stringId */, const ucstring &value)
{
	_EntityName = value;

	// remove the shard name if possible
	_EntityName= removeShardFromName(_EntityName);

	// New title
	ucstring newtitle;

	_HasReservedTitle = false;

	// check if there is any replacement tag in the string
	ucstring::size_type p1 = _EntityName.find('$');
	if (p1 != ucstring::npos)
	{
		// we found a replacement point begin tag
		ucstring::size_type p2 = _EntityName.find('$', p1+1);
		if (p2 != ucstring::npos)
		{
			// ok, we have the second replacement point!
			// extract the replacement id
			ucstring id = _EntityName.substr(p1+1, p2-p1-1);
			// retrieve the translated string
			_TitleRaw = id.toString();
//			ucstring replacement = CI18N::get(strNewTitle);
			bool womanTitle = false;
			CCharacterCL * c = dynamic_cast<CCharacterCL*>(this);
			if(c)
			{
				womanTitle = ( c->getGender() == GSGENDER::female );
			}
			
			ucstring replacement(STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(_TitleRaw, womanTitle));
			// Sometimes translation contains another title
			{
				ucstring::size_type pos = replacement.find('$');
				if (pos != ucstring::npos)
				{
					ucstring sn = replacement;
					_EntityName = sn.substr(0, pos);
					ucstring::size_type pos2 = sn.find('$', pos + 1);
					_TitleRaw = sn.substr(pos+1, pos2 - pos - 1);
					replacement = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(_TitleRaw, womanTitle);
				}
			}
			
			_Tags = STRING_MANAGER::CStringManagerClient::getTitleInfos(_TitleRaw, womanTitle);

			if (!replacement.empty() || !ClientCfg.DebugStringManager)
			{
				// build the final name
				p1 = _EntityName.find('$');
				_EntityName   = _EntityName.substr(0, p1);	// + _Name.substr(p2+1)
				// Get extended name
				_NameEx = replacement;
				newtitle = _NameEx;
			}
			CHARACTER_TITLE::ECharacterTitle titleEnum = CHARACTER_TITLE::toCharacterTitle( _TitleRaw.toString() );
			if ( titleEnum >= CHARACTER_TITLE::BeginGmTitle && titleEnum <= CHARACTER_TITLE::EndGmTitle )
			{
				_GMTitle = titleEnum - CHARACTER_TITLE::BeginGmTitle;
				_HasReservedTitle = true;
			}
			else
			{
				_GMTitle = _InvalidGMTitleCode;
				if ( titleEnum == CHARACTER_TITLE::FBT )
					_HasReservedTitle = true;
			}
		}
	}

	// Is the first title or a new title ?
	if ( !_Title.empty() && (_Slot==0) )
	{
		// Context help
		contextHelp ("title");
	}
	_Title = newtitle;

	// Update interface with new title
	if (_Slot == 0)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:player:header_opened:player_title"));
		if (pVT != NULL) pVT->setText(_Title);

		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:player"));
		if (pGC != NULL) pGC->setUCTitle(_EntityName);

		CSkillManager *pSM = CSkillManager::getInstance();
		pSM->setPlayerTitle(_TitleRaw.toString());
	}

	// Must rebuild the in scene interface 'cause name has changed
	buildInSceneInterface ();

}// onStringAvailable //

//-----------------------------------------------
// getTitleFromName
//-----------------------------------------------
ucstring CEntityCL::getTitleFromName(const ucstring &name)
{
	ucstring::size_type p1 = name.find('$');
	if (p1 != ucstring::npos)
	{
		ucstring::size_type p2 = name.find('$', p1 + 1);
		if (p2 != ucstring::npos)
			return name.substr(p1+1, p2-p1-1);
	}

	return ucstring("");
}// getTitleFromName //

//-----------------------------------------------
// removeTitleFromName
//-----------------------------------------------
ucstring CEntityCL::removeTitleFromName(const ucstring &name)
{
	ucstring::size_type p1 = name.find('$');
	if (p1 == ucstring::npos)
	{
		return name;
	}
	else
	{
		ucstring::size_type p2 = name.find('$', p1 + 1);
		if (p2 != ucstring::npos)
		{
			return name.substr(0, p1) + name.substr(p2 + 1);
		}
		else
		{
			return name.substr(0, p1);
		}
	}
}// removeTitleFromName //

//-----------------------------------------------
// removeShardFromName
//-----------------------------------------------
ucstring CEntityCL::removeShardFromName(const ucstring &name)
{
	// The string must contains a '(' and a ')'
	ucstring::size_type	p0= name.find('(');
	ucstring::size_type	p1= name.find(')');
	if(p0==ucstring::npos || p1==ucstring::npos || p1<=p0)
		return name;

	// if it is the same as the shard name of the user, remove it
	if(ucstrnicmp(name, (uint)p0+1, (uint)(p1-p0-1), PlayerSelectedHomeShardName)==0)
		return name.substr(0,p0) + name.substr(p1+1);
	// else don't modify
	else
		return name;
}

//-----------------------------------------------
// removeTitleAndShardFromName
//-----------------------------------------------
ucstring CEntityCL::removeTitleAndShardFromName(const ucstring &name)
{
	return removeTitleFromName(removeShardFromName(name));
}


//-----------------------------------------------

class CUpdateEntitiesColor : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CEntityCL::_EntitiesColor[CEntityCL::User] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:USER")->getValueRGBA();
		CEntityCL::_EntitiesColor[CEntityCL::Player] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:PLAYER")->getValueRGBA();
		CEntityCL::_EntitiesColor[CEntityCL::NPC] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:NPC")->getValueRGBA();
		CEntityCL::_EntitiesColor[CEntityCL::Fauna] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:FAUNA")->getValueRGBA();
		CEntityCL::_EntitiesColor[CEntityCL::ForageSource] = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:SOURCE")->getValueRGBA();
		CEntityCL::_DeadColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:DEAD")->getValueRGBA();
		CEntityCL::_TargetColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:TARGET")->getValueRGBA();
		CEntityCL::_GroupColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:GROUP")->getValueRGBA();
		CEntityCL::_GuildColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:GUILD")->getValueRGBA();
		CEntityCL::_UserMountColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:MOUNT")->getValueRGBA();
		CEntityCL::_UserPackAnimalColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:BEAST")->getValueRGBA();
		CEntityCL::_PvpEnemyColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:PVPENEMY")->getValueRGBA();
		CEntityCL::_PvpAllyColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:PVPALLY")->getValueRGBA();
		CEntityCL::_PvpAllyInTeamColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:PVPALLYINTEAM")->getValueRGBA();
		CEntityCL::_PvpNeutralColor = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ENTITY:COLORS:PVPNEUTRAL")->getValueRGBA();

		// don't save these colors in .icfg because players can't change them
		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::SGM - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:SGM")->getValueRGBA();
		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::GM - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:GM")->getValueRGBA();
		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::VG - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:VG")->getValueRGBA();
		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::SG - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:SG")->getValueRGBA();
		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::G - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:G")->getValueRGBA();

		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::CM - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:CM")->getValueRGBA();
		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::EM - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:EM")->getValueRGBA();
		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::EG - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:EG")->getValueRGBA();
		CEntityCL::_GMTitleColor[ CHARACTER_TITLE::OBSERVER - CHARACTER_TITLE::BeginGmTitle ] = NLGUI::CDBManager::getInstance()->getDbProp("UI:INTERFACE:ENTITY:COLORS:OBSERVER")->getValueRGBA();
	}
};
REGISTER_ACTION_HANDLER (CUpdateEntitiesColor, "update_entities_color");

//-----------------------------------------------

bool CEntityCL::isTarget () const
{
	return UserEntity && UserEntity->selection() == _Slot;
}

//-----------------------------------------------

bool CEntityCL::isInSameGuild () const
{
	if (Type != Player && Type != User)
		return false;

	const uint32 guildNameId = this->getGuildNameID();
	if (guildNameId != 0 && UserEntity && guildNameId == UserEntity->getGuildNameID())
		return true;

	return false;
}

//-----------------------------------------------

bool CEntityCL::oneInLeague () const
{
	if (Type != Player && Type != User)
		return false;

	const uint32 leagueID = getLeagueID();
	if ((UserEntity && (UserEntity->getLeagueID() != 0)) || leagueID != 0)
		return true;

	return false;
}

//-----------------------------------------------

bool CEntityCL::isInSameLeague () const
{
	if (Type != Player && Type != User)
		return false;

	const uint32 leagueID = getLeagueID();
	if ((leagueID != 0) && UserEntity && (leagueID == UserEntity->getLeagueID()))
		return true;

	return false;
}
//-----------------------------------------------

NLMISC::CRGBA	CEntityCL::getColor () const
{
	// target
	if (isTarget())
		return _TargetColor;
	// dead
	if (isReallyDead())
		return _DeadColor;
	// mount
	if (isUserMount())
		return _UserMountColor;
	// pack animal
	if (isUserPackAnimal())
		return _UserPackAnimalColor;
	// GM
	if ( ( Type == Player || Type == User ) && _GMTitle != _InvalidGMTitleCode )
	{
		return _GMTitleColor[_GMTitle];
	}

	if (Type == Player || Type == NPC)
	{
		// enemy
		if( Type == NPC )
		{
			if (isAnOutpostEnemy())
			{
				return _PvpEnemyColor;
			}
		}
		else
		{
			if (isEnemy())
			{
				if (getPvpMode()&PVP_MODE::PvpFaction)
					return CRGBA(min(255, _PvpEnemyColor.R+150), min(255, _PvpEnemyColor.G+150), min(255, _PvpEnemyColor.B+150),_PvpEnemyColor.A);
				else
					return _PvpEnemyColor;
			}
		}

		// ally
		if (isAlly())
		{
			if (getPvpMode() & PVP_MODE::PvpFactionFlagged)
			{
				if(isInSameLeague())
					return CRGBA(max(0, _PvpAllyColor.R-100), max(0, _PvpAllyColor.G-100), max(0, _PvpAllyColor.B-100),_PvpAllyColor.A);
				return CRGBA(max(0, _PvpAllyInTeamColor.R-100), max(0, _PvpAllyInTeamColor.G-100), max(0, _PvpAllyInTeamColor.B-100),_PvpAllyInTeamColor.A);
			}
			else
			{
				if(isInSameLeague())
					return _PvpAllyColor;
				return _PvpAllyInTeamColor;
			}
		}

		// neutral pvp
		if (isNeutralPVP())
			return _PvpNeutralColor;

		// neutral
		if (isInTeam())
			return _GroupColor;

		// neutral
		if (isInSameLeague())
			return CRGBA(min(255, _GroupColor.R+50), min(255, _GroupColor.G+50), min(255, _GroupColor.B+50),_GroupColor.A);

		if (isInSameGuild())
			return _GuildColor;
	}
	return _EntitiesColor[Type];
}


//---------------------------------------------------
// indoor :
// Return true if the current position is an indoor position.
//---------------------------------------------------
bool CEntityCL::indoor() const
{
	if(_Primitive && GR)
	{
		UGlobalPosition gPos;
		_Primitive->getGlobalPosition(gPos, dynamicWI);
		return GR->isInterior(gPos);
	}
	// Return false if any problem
	return false;
}// indoor //


//-----------------------------------------------
// pos :
//-----------------------------------------------
void CEntityCL::pos(const NLMISC::CVectorD &vect)
{
	CVectorD checkDist = _PositionLimiter-vect;
	checkDist.z = 0.0;
	if(checkDist != CVectorD::Null)
	{
		if(checkDist.norm() > ClientCfg.PositionLimiterRadius)
		{
			checkDist.normalize();
			_PositionLimiter = vect + checkDist*ClientCfg.PositionLimiterRadius/2.0;
		}
	}
	_Position = vect;
}// pos //


//-----------------------------------------------

void CEntityCL::updateMissionTarget()
{
	// Update the mission target flag
	_MissionTarget = false;
	if (_NameId)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		uint i,j;
		for (i=0; i<MAX_NUM_MISSIONS; i++)
			for (j=0; j<MAX_NUM_MISSION_TARGETS; j++)
			{
				// Get the db prop
				CCDBNodeLeaf *prop = EntitiesMngr.getMissionTargetTitleDB(i, j); // NLGUI::CDBManager::getInstance()->getDbProp("SERVER:MISSIONS:"+toString(i)+":TARGET"+toString(j)+":TITLE", false);
				if (prop)
				{
					_MissionTarget = _NameId == (uint32)prop->getValue32();
					if (_MissionTarget)
						return;
				}
			}
	}
}

//-----------------------------------------------
uint CEntityCL::getGroundType() const
{
	// default: 1 meter
	const	float	srqCacheDistLimit= 1;

	// If the user is not too far from cached pos, return same value
	if((_GroundTypeCachePos-pos()).sqrnorm() < srqCacheDistLimit)
	{
		return _GroundTypeCache;
	}

	uint	gt;

	if (indoor())
	{
		if(GR)
		{
			UGlobalPosition gPos;
			getPrimitive()->getGlobalPosition(gPos, dynamicWI);
			gt= GR->getMaterial(gPos);
		}
		else
			gt= 0;
	}
	else
	{
		// outside
		NL3D::CSurfaceInfo si;
		_CollisionEntity->getSurfaceInfo(pos(), si);
		gt= si.UserSurfaceData;
	}

	// store in cache
	_GroundTypeCachePos= pos();
	_GroundTypeCache= gt;

	return gt;
}

//---------------------------------------------------
void CEntityCL::makeTransparent(bool t)
{
	if (t == true)
		_TranspFactor += ((float)(DT)) / ((float)RZ_TIME_TO_BECOME_TRANSPARENT_IN_SECOND);
	else
		_TranspFactor -= ((float)(DT)) / ((float)RZ_TIME_TO_BECOME_TRANSPARENT_IN_SECOND);

	if (_TranspFactor < 0.0) _TranspFactor = 0.0;
	if (_TranspFactor > 1.0) _TranspFactor = 1.0;

	uint32	opaMin= getOpacityMin();
	uint8 opacity = (uint8)(opaMin + (255-opaMin) * (1.0 - _TranspFactor));

	for (uint32 i = 0; i < _Instances.size(); ++i)
	{
		_Instances[i].makeInstanceTransparent(opacity, (uint8)opaMin);
	}

}// makeTransparent //

//---------------------------------------------------
void CEntityCL::makeTransparent(float factor)
{
	clamp(factor, 0.f, 1.f);
	_TranspFactor = 1.f - factor;
	uint32	opaMin= getOpacityMin();
	uint8 opacity = (uint8)(opaMin + (255-opaMin) * (1.0 - _TranspFactor));

	for (uint32 i = 0; i < _Instances.size(); ++i)
	{
		_Instances[i].makeInstanceTransparent(opacity, (uint8)opaMin);
	}
}

// ***************************************************************************
void CEntityCL::setDiffuse(bool onOff, NLMISC::CRGBA diffuse)
{
	for (uint32 i = 0; i < _Instances.size(); ++i)
	{
		_Instances[i].setDiffuse(onOff, diffuse);
	}
}


// ***************************************************************************
CCDBNodeLeaf *CEntityCL::getOpacityDBNode()
{
	if (!_OpacityMinNodeLeaf)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		_OpacityMinNodeLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:USER_CHAR_OPA_MIN", false);
	}
	return _OpacityMinNodeLeaf;
}

// ***************************************************************************
uint32 CEntityCL::getOpacityMin()
{
	CCDBNodeLeaf *pNL = getOpacityDBNode();
	if(pNL)
	{
		uint32	val= pNL->getValue32();
		clamp(val, (uint32)0, (uint32)255);
		return val;
	}
	else
		// this is an error case...
		return 128;
}

// ***************************************************************************
void CEntityCL::setOpacityMin(uint32 value)
{
	CCDBNodeLeaf *pNL = getOpacityDBNode();
	if(pNL)
	{
		clamp(value, (uint32)0, (uint32)255);
		pNL->setValue32(value);
	}
}


// ***************************************************************************
/*
 * Return true if the in-scene interface must be shown
 */
bool CEntityCL::mustShowInsceneInterface( bool enabledInSheet ) const
{
	return 	(
				(enabledInSheet /*&& !CNPCIconCache::getInstance().getNPCIcon(this).getTextureMain().empty()*/) && 
				(_InSceneInterfaceEnabled) &&
				(	ClientCfg.Names ||
					isUser () ||
					((getDisplayOSDForceOver() || ClientCfg.ShowNameUnderCursor) && slot()==SlotUnderCursor) ||
					(ClientCfg.ShowNameSelected && UserEntity && slot()==UserEntity->selection()) ||
					(isInTeam() && !isUser ()) ||
					(UserEntity && ((UserEntity->pos()-pos()).sqrnorm() < ClientCfg.ShowNameBelowDistanceSqr) && !isUser ())
				)
			);

}


//-----------------------------------------------
const char *CEntityCL::getBoneNameFromBodyPart(BODY::TBodyPart part, BODY::TSide side) const
{
	BODY::TBodyPart hominPart = BODY::getMatchingHominBodyPart(part);
	switch(hominPart)
	{
		case BODY::HHead:  return "Bip01 Head";
		case BODY::HChest: return "Bip01 Spine2";
		case BODY::HArms:  return side == BODY::Left ? "Bip01 L UpperArm" : "Bip01 R UpperArm";
		case BODY::HHands: return side == BODY::Left ? "Bip01 L Hand" : "Bip01 R Hand";
		case BODY::HLegs:  return side == BODY::Left ? "Bip01 L Calf" : "Bip01 R Calf";
		case BODY::HFeet:  return side == BODY::Left ? "Bip01 L Foot" : "Bip01 R Foot";
		default: break;
	}
	return NULL;
}

//-----------------------------------------------
// idx2Inst :
// Return a pointer on an instance structure.
//-----------------------------------------------
CEntityCL::SInstanceCL *CEntityCL::idx2Inst(uint idx)
{
	if(idx < _Instances.size()) // CEntityCL::BadIndex is the max so idx < ... mean idx != CEntityCL::BadIndex too
		return &_Instances[idx];

	return NULL;
}// idx2Inst //



//-----------------------------------------------
void CEntityCL::updateIsInTeam ()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	_IsInTeam= false;

	// if UId not set, false
	if(dataSetId()==CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
		return;

	for (uint i=0; i<MaxNumPeopleInTeam; i++)
	{
		// Get the db prop
		CCDBNodeLeaf *uidProp = EntitiesMngr.getGroupMemberUidDB(i);
		CCDBNodeLeaf *presentProp = EntitiesMngr.getGroupMemberNameDB(i);
		// If same Entity uid than the one in the Database, ok the entity is in the Player TEAM!!
		if (uidProp && uidProp->getValue32() == (sint32)dataSetId() &&
			presentProp && presentProp->getValueBool() )
		{
			_IsInTeam= true;
			buildInSceneInterface();
			return;
		}
	}
	buildInSceneInterface();
}

//-----------------------------------------------
void CEntityCL::updateIsUserAnimal ()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	_IsUserMount= false;
	_IsUserPackAnimal= false;

	// if UId not set, false
	if(dataSetId()==CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
		return;

	// Am i a pack animal?
	for (uint i=0; i<MAX_INVENTORY_ANIMAL; i++)
	{
		// Get the db prop
		CCDBNodeLeaf *uidProp = EntitiesMngr.getBeastUidDB(i);
		CCDBNodeLeaf *statusProp  = EntitiesMngr.getBeastStatusDB(i);
		CCDBNodeLeaf *typeProp  = EntitiesMngr.getBeastTypeDB(i);
		// I must have the same Id, and the animal entry must be ok.
		if(uidProp && statusProp && typeProp && uidProp->getValue32() == (sint32)dataSetId() &&
			ANIMAL_STATUS::isSpawned((ANIMAL_STATUS::EAnimalStatus)(statusProp->getValue32()) ))
		{
			switch(typeProp->getValue16())
			{
				case ANIMAL_TYPE::Mount:	_IsUserMount = true; break;

				default:
				case ANIMAL_TYPE::Packer:	_IsUserPackAnimal = true; break;

			}
			return;
		}
	}
}

//-----------------------------------------------
ANIMAL_STATUS::EAnimalStatus	CEntityCL::getPackAnimalStatus() const
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(! (isUserPackAnimal() || isUserMount()))
		return 0;

	// Am i a pack animal?
	for (uint i=0; i<MAX_INVENTORY_ANIMAL; i++)
	{
		// Get the db prop
		CCDBNodeLeaf *uidProp = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:UID", i), false);
		CCDBNodeLeaf *statusProp  = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:STATUS", i), false);
		// I must have the same Id, and the animal entry must be ok.
		if(uidProp && statusProp && uidProp->getValue32() == (sint32)dataSetId())
			return (ANIMAL_STATUS::EAnimalStatus)(statusProp->getValue32());
	}

	return 0;
}

//-----------------------------------------------
bool CEntityCL::getPackAnimalIndexInDB(sint &dbIndex) const
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	if(! (isUserPackAnimal() || isUserMount()))
		return false;

	// Am i a pack animal?
	for (uint i=0; i<MAX_INVENTORY_ANIMAL; i++)
	{
		// Get the db prop
		CCDBNodeLeaf *uidProp = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:UID", i), false);
		CCDBNodeLeaf *statusProp  = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:STATUS", i), false);
		// I must have the same Id, and the animal entry must be ok.
		if(uidProp && statusProp && uidProp->getValue32() == (sint32)dataSetId())
		{
			dbIndex= i;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------
void CEntityCL::dataSetId(CLFECOMMON::TClientDataSetIndex dataSet)
{
	_DataSetId = dataSet;

	if (_Primitive &&  _Primitive->UserData == UserDataEntity)
		_Primitive->UserData |= (((uint64)_DataSetId)<<16);

	// additionaly, on a UID change, must check the IsInTeam and IsAniml flags
	updateIsInTeam();
	updateIsUserAnimal();

	// and update Bar Manager, only if correctly init
	if(_Slot!=CLFECOMMON::INVALID_SLOT && _DataSetId!=CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
		CBarManager::getInstance()->addEntity(_Slot, _DataSetId);
}

// ***************************************************************************
void	CEntityCL::updateVisible(const NLMISC::TTime &/* time */, CEntityCL * /* target */)
{
	// update some shadowmap properties
	updateShadowMapProperties();

	// For Skeleton Spawn Animation, must update some pos
	if(!_Skeleton.empty())
	{
		_Skeleton.setSSSWOPos(pos());
		_Skeleton.setSSSWODir(front());
	}

	if (!R2::isEditionCurrent())
	{
		// NB : if editor is enabled, then highlighting is managed in another fashion (there may be multiple-semihighlights)

		// Selection Highlight
		if(_VisualSelectionTime != 0)
		{
			bool blinkOn = false;
			sint64	t= T1 -_VisualSelectionTime;
			// hardcoded blink animation
			if(t>150 && t<300)
				blinkOn= true;
			if(t>450)
				_VisualSelectionTime = 0;

			setVisualSelectionBlink(blinkOn, CRGBA::White);
		}
		else
		{
			// If i am the target
			if(isTarget())
				// set semi-highlight
				setVisualSelectionBlink(true, CRGBA(100,100,100));
			// else If i am under cursor
			else  if(ShowInterface && isUnderCursor())
				// highlight
				setVisualSelectionBlink(true, CRGBA::White);
			else
				// disable hightlight
				setVisualSelectionBlink(false, CRGBA::White);
		}
	}
}

// ***************************************************************************
void	CEntityCL::updateSomeClipped(const NLMISC::TTime &/* time */, CEntityCL * /* target */)
{
	// update some shadowmap properties
	updateShadowMapProperties();
}

// ***************************************************************************
void	CEntityCL::updateClipped(const NLMISC::TTime &/* currentTimeInMs */, CEntityCL * /* target */)
{
	// hide only if I am not the target
	if (! isTarget())
	{
		if (!_SelectionFX.empty() && Scene)
		{
			Scene->deleteInstance(_SelectionFX);
			_SelectionFX = NULL;
		}
		if (!_MouseOverFX.empty() && Scene)
		{
			Scene->deleteInstance(_MouseOverFX);
			_MouseOverFX = NULL;
		}
	}
}


// ***************************************************************************
void	CEntityCL::updateVisiblePostPos(const NLMISC::TTime &/* currentTimeInMs */, CEntityCL * /* target */)
{
	if (R2::isEditionCurrent())	return; // selection managed by r2 editor in edition mode

	bool bShowReticle = true;

	CCDBNodeLeaf *node = (CCDBNodeLeaf *)_ShowReticleLeaf ? &*_ShowReticleLeaf
		: &*(_ShowReticleLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:SHOW_RETICLE", false));

	if (node)
	{
		bShowReticle = node->getValueBool();
	}

	// No-op if I am not the current UserEntity Target
	if(bShowReticle && isTarget())
	{
		// activate selection fx
		if (_SelectionFX.empty())
		{
			// Keep a static instance of the selection FX
			NL3D::UInstance instance = Scene->createInstance(ClientCfg.SelectionFX);
			if (!instance.empty())
			{
				_SelectionFX.cast (instance);
				if (_SelectionFX.empty())
				{
					// shape found, but not a particle system
					Scene->deleteInstance(instance);
				}
			}
		}
	}
	else
	{
		// No selection FX
		if (!_SelectionFX.empty() && Scene)
		{
			Scene->deleteInstance(_SelectionFX);
		}
	}

	// Mouse over SFX, only if the entity is selectable
	if (bShowReticle && ShowInterface && !isTarget() && isUnderCursor() && properties().selectable())
	{
		// activate selection fx
		if (_MouseOverFX.empty())
		{
			// Keep a static instance of the selection FX
			NL3D::UInstance instance = Scene->createInstance(ClientCfg.MouseOverFX);
			if (!instance.empty())
			{
				_MouseOverFX.cast (instance);
				if (_MouseOverFX.empty())
				{
					// shape found, but not a particle system
					Scene->deleteInstance(instance);
				}
			}
		}
	}
	else
	{
		// No selection FX
		if (!_MouseOverFX.empty() && Scene)
		{
			Scene->deleteInstance(_MouseOverFX);
		}
	}

	if (!_StateFX.empty())
	{
		// Build a matrix for the fx
		NLMISC::CMatrix mat;
		mat.identity();

		// Scale
		const CVector &boxes = _SelectBox.getHalfSize ();
		// take mean of XY and Z
		float halfwidth = std::max(boxes.x, boxes.y);
		halfwidth = (halfwidth + boxes.z)/2;
		mat.setScale (halfwidth);

		// Pos. Avoid Flick in XY, but take precise Z
		CVector position;
		position = pos().asVector();
		position.z= _SelectBox.getMin().z;
		mat.setPos(position);
		mat.setRot(dirMatrix());

		_StateFX.setTransformMode(NL3D::UTransformable::DirectMatrix);
		_StateFX.setMatrix(mat);
		if (skeleton())
			_StateFX.setClusterSystem(skeleton()->getClusterSystem());
	}
	
	if (!_SelectionFX.empty() || !_MouseOverFX.empty())
	{
		// Build a matrix for the fx
		NLMISC::CMatrix mat;
		mat.identity();

		// Scale
		const CVector &boxes = _SelectBox.getHalfSize ();
		// take mean of XY and Z
		float halfwidth = std::max(boxes.x, boxes.y);
		halfwidth = (halfwidth + boxes.z)/2;
		mat.setScale (halfwidth * 2 * ClientCfg.SelectionFXSize);

		// Pos. Avoid Flick in XY, but take precise Z
		CVector position;
		position = pos().asVector();
		position.z= _SelectBox.getCenter().z;
		mat.setPos(position);

		if (!_SelectionFX.empty())
		{
			_SelectionFX.setTransformMode(NL3D::UTransformable::DirectMatrix);
			_SelectionFX.setMatrix(mat);
			if (skeleton())
				_SelectionFX.setClusterSystem(skeleton()->getClusterSystem());
			// Colorize the selection depending of the level of the creature
			{
				CRGBA col = CRGBA(0,0,0);
				uint8 nForce = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TARGET:FORCE_RATIO")->getValue8();
				_SelectionFX.setUserParam(0, 0.1f*nForce + 0.1f);
			}
		}
		if (!_MouseOverFX.empty())
		{
			_MouseOverFX.setTransformMode(NL3D::UTransformable::DirectMatrix);
			_MouseOverFX.setMatrix(mat);
			if (skeleton())
				_MouseOverFX.setClusterSystem(skeleton()->getClusterSystem());
		}
	}
}


// ***************************************************************************
void CEntityCL::buildInSceneInterface ()
{
}

// ***************************************************************************
void CEntityCL::doSetVisualSelectionBlink(bool bOnOff, CRGBA emitColor)
{
	// blink all instances
	for(uint i = 0; i < instances().size(); ++i)
	{
		SInstanceCL	&inst = instances()[i];
		if(bOnOff)
			inst.setEmissive(emitColor);
		else
			inst.restoreEmissive();
	}

	// also blink skeleton in CLod form
	if(!_Skeleton.empty())
	{
		if(bOnOff)
			_Skeleton.setLodEmit(emitColor);
		else
			_Skeleton.setLodEmit(CRGBA::Black);
	}
}

// ***************************************************************************
void CEntityCL::visualSelectionStart()
{
	_VisualSelectionTime= T1;
}

// ***************************************************************************
void CEntityCL::visualSelectionStop()
{
	_VisualSelectionTime= 0;
}

// ***************************************************************************
bool CEntityCL::canCastShadowMap() const
{
	return ClientCfg.Shadows;
}

// ***************************************************************************
void CEntityCL::updateCastShadowMap()
{
	bool	shadowOn= canCastShadowMap();

	// if the entity has a skeleton
	if(skeleton())
	{
		// then shadow will be done through the skeleton
		skeleton()->enableCastShadowMap(shadowOn);
		// disable shadows on instances
		if(_SomeInstanceCastShadowMap)
		{
			for(uint i = 0; i < _Instances.size(); i++)
			{
				if(!_Instances[i].Current.empty())
					_Instances[i].Current.enableCastShadowMap(false);
			}
			_SomeInstanceCastShadowMap= false;
		}
	}
	else
	{
		// enable/disable cast shadow on instances instead (eg: on bot objects)
		// NB: must do it at each updateCastShadowMap(), cause don't know if an instance has been added or not
		for(uint i = 0; i < _Instances.size(); i++)
		{
			if(!_Instances[i].Current.empty())
			{
				_Instances[i].Current.enableCastShadowMap(shadowOn);
			}
		}

		_SomeInstanceCastShadowMap= shadowOn;
	}

	// if shadow enabled, update the shadow properties
	if(shadowOn)
	{
		if(skeleton())
		{
			skeleton()->setShadowMapDirectionZThreshold(_ShadowMapZDirClamp);
			skeleton()->setShadowMapMaxDepth(_ShadowMapMaxDepth);
		}
		else
		{
			for(uint i = 0; i < _Instances.size(); i++)
			{
				if(!_Instances[i].Current.empty())
				{
					_Instances[i].Current.setShadowMapDirectionZThreshold(_ShadowMapZDirClamp);
					_Instances[i].Current.setShadowMapMaxDepth(_ShadowMapMaxDepth);
				}
			}
		}
	}
}

// ***************************************************************************
void CEntityCL::updateShadowMapProperties()
{
	/*
		Choose the z clamp direction whether or not the player is on "interior" stuff.
		In "interior" stuff, the ZClamp direction is lesser, to avoid some problems of
		"cast shadow behind the walls"

		Also choose the MaxDepth of shadow map  whether or not the player is on "interior" stuff.
		In "interior" stuff, the MaxDepth is lesser to, to avoid some problems with the "bud":
			when the player go up stairs and when in the "bud", the shadow still appears on landscape
	*/
	float	zDirClampWanted= ClientCfg.ShadowZDirClampLandscape;
	float	maxDepth= ClientCfg.ShadowMaxDepthLandscape;
	if(indoor())
	{
		zDirClampWanted= ClientCfg.ShadowZDirClampInterior;
		maxDepth= ClientCfg.ShadowMaxDepthInterior;
	}

	// smooth over time
	if(_ShadowMapZDirClamp!=zDirClampWanted || _ShadowMapMaxDepth!=maxDepth)
	{
		// if some time passed since last update
		sint64	dt= T1-_ShadowMapPropertyLastUpdate;
		if(dt!=0)
		{
			// update _ShadowMapZDirClamp
			if(_ShadowMapZDirClamp<zDirClampWanted)
			{
				_ShadowMapZDirClamp+= ClientCfg.ShadowZDirClampSmoothSpeed * 0.001f * dt;
				if(_ShadowMapZDirClamp>zDirClampWanted)
					_ShadowMapZDirClamp= zDirClampWanted;
			}
			else if(_ShadowMapZDirClamp>zDirClampWanted)
			{
				_ShadowMapZDirClamp-= ClientCfg.ShadowZDirClampSmoothSpeed * 0.001f * dt;
				if(_ShadowMapZDirClamp<zDirClampWanted)
					_ShadowMapZDirClamp= zDirClampWanted;
			}

			// update _ShadowMapMaxDepth
			if(_ShadowMapMaxDepth<maxDepth)
			{
				_ShadowMapMaxDepth+= ClientCfg.ShadowMaxDepthSmoothSpeed * 0.001f * dt;
				if(_ShadowMapMaxDepth>maxDepth)
					_ShadowMapMaxDepth=maxDepth;
			}
			else if(_ShadowMapMaxDepth>maxDepth)
			{
				_ShadowMapMaxDepth-= ClientCfg.ShadowMaxDepthSmoothSpeed * 0.001f * dt;
				if(_ShadowMapMaxDepth<maxDepth)
					_ShadowMapMaxDepth=maxDepth;
			}

			// update shadowMap, to update the clamp zdirection, and the shadow depth
			updateCastShadowMap();
		}
	}

	// bkup last time of update
	_ShadowMapPropertyLastUpdate= T1;
}

// ***************************************************************************
void CEntityCL::setOrderingLayer(uint layer)
{
	if (!_Skeleton.empty()) _Skeleton.setOrderingLayer(layer);
}


void CEntityCL::displayable(bool d)
{
	_Displayable = d;
	// :KLUDGE: Hide selection FX
	if (!_Displayable && !_SelectionFX.empty() && Scene)
	{
		Scene->deleteInstance(_SelectionFX);
	}
}

// ***************************************************************************
EGSPD::CPeople::TPeople CEntityCL::people() const
{
	return EGSPD::CPeople::Unknown;
}

// ***************************************************************************
void CEntityCL::setPeople(EGSPD::CPeople::TPeople /* people */)
{
}

// ***************************************************************************
void CEntityCL::forceEvalAnim()
{
	if (getLastClip())
	{
		// find highest father in the hierarchy
		CEntityCL *parentEntity = this;
		for(;;)
		{
			CEntityCL *nextParent = EntitiesMngr.entity(parentEntity->parent());
			if (!nextParent) break;
			parentEntity = nextParent;
		}

		// Snap the parent entity to the ground.
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Snap_To_Ground )
			parentEntity->snapToGround();
		}

		// Animate the parent entity (and also child entities)
		{
			H_AUTO ( RZ_Client_Entity_CL_Update_Display )
			parentEntity->updateDisplay();
		}
	}
}


// ***************************************************************************
const NLMISC::CAABBox &CEntityCL::localSelectBox()
{
	// recompute the selection box?
	if(_LastLocalSelectBoxComputeTime<T1)
	{
		_LastLocalSelectBoxComputeTime=T1;
		bool	found= false;

		// if skeleton, compute aabox from precise skeleton method
		if(!_Skeleton.empty())
		{
			// Don't compute if in LOD form (else flick because sometimes valid because of shadow animation)
			if(!_Skeleton.isDisplayedAsLodCharacter())
			{
				/*
				bool computed;
				static volatile bool useBoneSphere = true;
				if (useBoneSphere) computed = _Skeleton.computeRenderedBBoxWithBoneSphere(_LocalSelectBox, false);
				else computed = _Skeleton.computeRenderedBBox(_LocalSelectBox);
				if (computed)
				{
					found= true;
					// apply local offset due to skeleton animation
					CMatrix invMat = _DirMatrix;
					invMat.setPos(pos());
					invMat.invert();
					CVector localOffset = invMat * _Skeleton.getPos();
					_LocalSelectBox.setCenter(_LocalSelectBox.getCenter() + localOffset);
					static volatile float rot = -1.f;
					CMatrix rotMat;
					rotMat.rotateZ((float) Pi * rot / 2);
					CVector newHalfSize = rotMat * _LocalSelectBox.getHalfSize();
					newHalfSize.x = fabsf(newHalfSize.x);
					newHalfSize.y = fabsf(newHalfSize.y);
					newHalfSize.z = fabsf(newHalfSize.z);
					_LocalSelectBox.setHalfSize(newHalfSize);
				}
				*/
				if (_Skeleton.computeRenderedBBoxWithBoneSphere(_LocalSelectBox, false))
				{
					found= true;
					// apply local offset due to skeleton animation
					CMatrix invMat = _DirMatrix;
					invMat.setPos(pos());
					invMat.invert();
					CMatrix localMat = invMat * _Skeleton.getMatrix();
					_LocalSelectBox = CAABBox::transformAABBox(localMat, _LocalSelectBox);
				}
			}
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
				inst.getShapeAABBox(_LocalSelectBox);
				found= true;
			}
		}


		// if not found, fallback to default bbox
		if(!found)
		{
			_LocalSelectBox.setCenter(_Aabbox.getCenter() - pos().asVector());
			_LocalSelectBox.setHalfSize(_Aabbox.getHalfSize());
		}
		else
		{
			CVector scale;
			getMeshDefaultScale(scale);
			const CVector &halfSize = _LocalSelectBox.getHalfSize();
			const CVector &center = _LocalSelectBox.getCenter();
			_LocalSelectBox.setHalfSize(CVector(halfSize.x * scale.x, halfSize.y * scale.y, halfSize.z * scale.z));
			_LocalSelectBox.setCenter(CVector(center.x * scale.x, center.y * scale.y, center.z * scale.z));
		}
	}

	// Return the selection box.
	return _LocalSelectBox;
}

//----------------------------------------------------------------------
void CEntityCL::getMeshDefaultScale(NLMISC::CVector &scale) const
{
	scale.set(1.f, 1.f, 1.f);
	if (!_Skeleton.empty()) return;  // scale already applied to skeleton
	if (!_Instance.empty())
	{
		const_cast<UInstance &>(_Instance).getScale(scale);
	}
	else if (!_Instances.empty())
	{
		if (!_Instances[0].Current.empty())
		{
			const_cast<UInstance &>(_Instances[0].Current).getScale(scale);
		}
		else if (!_Instances[0].Loading.empty())
		{
			const_cast<UInstance &>(_Instances[0].Loading).getScale(scale);
		}
	}
}


//----------------------------------------------------------------------
bool CEntityCL::isAnOutpostEnemy() const
{
	if ( getOutpostId() != 0 )
	{
		if( UserEntity->getOutpostId() == getOutpostId() )
		{
			if( UserEntity->getOutpostSide() != getOutpostSide() )
			{
				// same outpost but different side
				return true;
			}
		}
	}
	return false;
}


//----------------------------------------------------------------------
bool CEntityCL::isAnOutpostAlly() const
{
	if ( getOutpostId() != 0 )
	{
		if( UserEntity->getOutpostId() == getOutpostId() )
		{
			if( UserEntity->getOutpostSide() == getOutpostSide() )
			{
				// same outpost and same side
				return true;
			}
		}
	}
	return false;
}


//----------------------------------------------------------------------
CVector CEntityCL::dirToTarget() const
{
	CVector dir2Target;
	CEntityCL * target = EntitiesMngr.entity( targetSlot() );
	if( target )
	{
		dir2Target = target->pos() - pos();
		dir2Target.z = 0;
		dir2Target.normalize();
	}
	else
	{
		dir2Target = dir();
	}
	return dir2Target;
}

//----------------------------------------------------------------------
void CEntityCL::setStateFx(const std::string &fxName)
{
	if (fxName != _StateFXName)
	{
		if (!_StateFX.empty() && Scene)
		{
			Scene->deleteInstance(_StateFX);
		}

		NL3D::UInstance instance = Scene->createInstance(fxName);

		if (!instance.empty())
		{
			_StateFX.cast (instance);
			if (_StateFX.empty())
			{
				// shape found, but not a particle system
				Scene->deleteInstance(instance);
			}
			else
			{
				_StateFX.setScale(0.5, 0.5, 0.5);
				_StateFXName = fxName;
			}
		}
	}
}

//----------------------------------------------------------------------
void CEntityCL::removeStateFx()
{
	if (!_StateFX.empty() && Scene)
	{
		Scene->deleteInstance(_StateFX);
		_StateFXName = "";
	}
}

