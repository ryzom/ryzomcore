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
// Client
#include "entities.h"
#include "entity_cl.h"
#include "fx_cl.h"
#include "forage_source_cl.h"
#include "item_cl.h"
#include "pacs_client.h"
#include "time_client.h"
#include "view.h"
#include "user_entity.h"
#include "sheet_manager.h"
#include "motion/user_controls.h"
#include "net_manager.h"
#include "debug_client.h"
#include "ingame_database_manager.h"
#include "interface_v3/interface_manager.h"
#include "door_manager.h"
#include "projectile_manager.h"
#include "client_chat_manager.h"
#include "interface_v3/people_interraction.h"
#include "interface_v3/bar_manager.h"
#include "interface_v3/group_compas.h"
// 3D
#include "nel/3d/quad_tree.h"
// Interface 3D
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/u_material.h"
// Misc
#include "nel/misc/stream.h"
#include "nel/misc/common.h"
// Game share
#include "game_share/mission_desc.h"
#include "game_share/inventories.h"
// PACS
#include "nel/pacs/u_collision_desc.h"
// UI
#include "interface_v3/group_compas.h"

#include "player_r2_cl.h"
#include "r2/editor.h"

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;


////////////
// EXTERN //
////////////
extern UDriver					*Driver;
extern UScene					*Scene;
extern UTextContext				*TextContext;
extern UCamera					MainCam;
extern CLFECOMMON::TCLEntityId	SlotUnderCursor;


////////////
// GLOBAL //
////////////
CEntityManager			EntitiesMngr;

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Entity_Mngr_Update )
H_AUTO_DECL ( RZ_Client_Update_Post_Render )
H_AUTO_DECL ( RZ_Client_Entity_Mngr_Update_Apply_Motion )
H_AUTO_DECL ( RZ_Client_Entity_Mngr_Update_Count )



/////////////
// METHODS //
/////////////
//---------//
// PRIVATE //
//---------//

// ***************************************************************************
class CMissionTargetObserver : public ICDBNode::IPropertyObserver
{
public :

	// From ICDBNode::IPropertyObserver
	virtual void update(ICDBNode* node )
	{
		CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf*>(node);
		if (leaf)
		{
			// Get the target
			uint32 oldTarget = leaf->getOldValue32();
			uint32 target = leaf->getValue32();

			// Scan all entities
			CEntityCL *entity = NULL;
			if (oldTarget)
				entity = EntitiesMngr.getEntityByName(oldTarget);
			if (entity)
				entity->updateMissionTarget();

			entity = NULL;
			if (target)
				entity = EntitiesMngr.getEntityByName(target);
			if (entity)
				entity->updateMissionTarget();

			CInterfaceManager *im = CInterfaceManager::getInstance();
			CGroupCompas *gc = dynamic_cast<CGroupCompas *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:compass"));
			// if new target title is not NULL, then show the compass and make it blink to indicate new location
			// please note that the first time the player login, a target has not been saved in his config file, so
			// we permit the first (and only one) mission that is received to become the new compass direction (chiang the strong ...)
			if (!IngameDbMngr.initInProgress() || (gc && !gc->isSavedTargetValid()))
			{
				if (target)
				{
					_PendingMissionTitle.push_back(leaf);
				}
			}
		}
	}
	// When a mission name has been retrieved, update the compass to point it
	void update()
	{
		std::list<CCDBNodeLeaf *>::iterator it = _PendingMissionTitle.begin();
		while (it != _PendingMissionTitle.end())
		{
			std::list<CCDBNodeLeaf *>::iterator tmpIt = it;
			++ it;
			CCDBNodeLeaf *leaf = *tmpIt;

			// If the previous title is not empty we probably have to clear the compass
			if (leaf->getOldValue32() != 0)
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				CGroupCompas *pGC = dynamic_cast<CGroupCompas*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:compass"));
				if (pGC == NULL)
				{
					nlwarning("Can't retrieve compass group");
					return;
				}
				CCompassTarget ct = pGC->getTarget();

				STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
				ucstring oldName;
				if (!pSMC->getDynString(leaf->getOldValue32(), oldName))
				{
					nlwarning("Can't get compass target name");
					return;
				}

				if (ct.Name == oldName)
				{
					CCompassTarget north;
					pGC->setTarget(north);
				}
			}

			// see if mission name has been retrieved
			if ((*tmpIt)->getValue32() == 0)
			{
				_PendingMissionTitle.erase(tmpIt);
			}
			else
			{
				// TODO : maybe the following code could be include in CGroupMap::checkCoords, but it is not called when the map is not visible...
				STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
				ucstring name;
				if (pSMC->getDynString((*tmpIt)->getValue32(), name))
				{
//					if (_AlreadyReceived.count(name) == 0)
//					{
//						_AlreadyReceived.insert(name);
						CInterfaceManager *im = CInterfaceManager::getInstance();
						CGroupCompas *gc = dynamic_cast<CGroupCompas *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:compass"));
						if (!gc)
						{
							nlwarning("Can't retrieve compass group");
						}
						else
						{
							CCompassTarget ct;
							CCDBNodeLeaf *leaf = *tmpIt;
							CCDBNodeBranch *parent = leaf->getParent();
							if (parent)
							{
								CCDBNodeLeaf *x = dynamic_cast<CCDBNodeLeaf *>(parent->getNode(ICDBNode::CTextId("X")));
								CCDBNodeLeaf *y = dynamic_cast<CCDBNodeLeaf *>(parent->getNode(ICDBNode::CTextId("Y")));
								if (x && y)
								{
									CSmartPtr<CNamedEntityPositionState> tracker = new CNamedEntityPositionState;
									tracker->build(*tmpIt, x, y);
									ct.setPositionState(tracker);
									ct.Name = name;
									// make the compass appear and blink
									gc->setActive(true);
									gc->setTarget(ct);
									gc->blink();
									gc->enableBlink(2);
									CWidgetManager::getInstance()->setTopWindow(gc);
								}
							}
						}
//					}
					_PendingMissionTitle.erase(tmpIt);
				}
			}
		}

	}
private:
	std::list<CCDBNodeLeaf *> _PendingMissionTitle;
//	std::set<ucstring> _AlreadyReceived;
};

//-----------------------------------------------
CMissionTargetObserver	MissionTargetObserver;



// ***************************************************************************
class CTeamUIDObserver : public ICDBNode::IPropertyObserver
{
public :

	// From ICDBNode::IPropertyObserver
	virtual void update(ICDBNode* node )
	{
		CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf*>(node);
		if (leaf)
		{
			// Get the uid
			CLFECOMMON::TClientDataSetIndex oldEntityId = leaf->getOldValue32();
			CLFECOMMON::TClientDataSetIndex entityId = leaf->getValue32();

			// Scan all entities.
			// check if removed from team
			CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(oldEntityId);
			if (entity)
				entity->updateIsInTeam();

			// check if added in team
			entity = EntitiesMngr.getEntityByCompressedIndex(entityId);
			if (entity)
				entity->updateIsInTeam();
		}
	}
};

//-----------------------------------------------

CTeamUIDObserver		TeamUIDObserver;


// ***************************************************************************
class CTeamPresentObserver : public ICDBNode::IPropertyObserver
{
public :

	// From ICDBNode::IPropertyObserver
	virtual void update(ICDBNode* node )
	{
		CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf*>(node);
		if (leaf)
		{
			// Must get the NAME leaf
			CCDBNodeBranch	*parent= leaf->getParent();
			if(parent)
			{
				leaf= dynamic_cast<CCDBNodeLeaf*>(parent->getNode(ICDBNode::CTextId("UID"), false));
				// Get the name id
				CLFECOMMON::TClientDataSetIndex entityId = CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;
				if(leaf)
					entityId= leaf->getValue32();

				// Scan all entities.
				// check if added/removed in team
				CEntityCL	*entity= EntitiesMngr.getEntityByCompressedIndex(entityId);
				if (entity)
					entity->updateIsInTeam();
			}
		}
	}
};

//-----------------------------------------------

CTeamPresentObserver		TeamPresentObserver;


// ***************************************************************************
class CAnimalUIDObserver : public ICDBNode::IPropertyObserver
{
public :

	// From ICDBNode::IPropertyObserver
	virtual void update(ICDBNode* node )
	{
		CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf*>(node);
		if (leaf)
		{
			// Get the uid
			CLFECOMMON::TClientDataSetIndex oldEntityId = leaf->getOldValue32();
			CLFECOMMON::TClientDataSetIndex entityId = leaf->getValue32();

			// Scan all entities.
			// check if removed from animal list
			CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(oldEntityId);
			if (entity)
				entity->updateIsUserAnimal();

			// check if added in animal list
			entity = EntitiesMngr.getEntityByCompressedIndex(entityId);
			if (entity)
				entity->updateIsUserAnimal();
		}
	}
};

//-----------------------------------------------

CAnimalUIDObserver		AnimalUIDObserver;


// ***************************************************************************
class CAnimalStatusObserver : public ICDBNode::IPropertyObserver
{
public :

	// From ICDBNode::IPropertyObserver
	virtual void update(ICDBNode* node )
	{
		CCDBNodeLeaf *leaf = dynamic_cast<CCDBNodeLeaf*>(node);
		if (leaf)
		{
			// Must get the NAME leaf
			CCDBNodeBranch	*parent= leaf->getParent();
			if(parent)
			{
				leaf= dynamic_cast<CCDBNodeLeaf*>(parent->getNode(ICDBNode::CTextId("UID"), false));
				// Get the name id
				CLFECOMMON::TClientDataSetIndex entityId = CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;
				if(leaf)
					entityId= leaf->getValue32();

				// Scan all entities.
				// check if added/removed in animal list
				CEntityCL	*entity= EntitiesMngr.getEntityByCompressedIndex(entityId);
				if (entity)
					entity->updateIsUserAnimal();
			}
		}
	}
};

//-----------------------------------------------

CAnimalStatusObserver		AnimalStatusObserver;


// ***************************************************************************
//--------//
// PUBLIC //
//--------//
//-----------------------------------------------
// CEntityManager :
// Constructor.
//-----------------------------------------------
CEntityManager::CEntityManager()
{
	_NbMaxEntity		= 0;
	_EntitiesAllocated	= 0;
	_NbUser				= 0;
	_NbPlayer			= 0;
	_NbChar				= 0;
	_LastEntityUnderPos= NULL;
}// CEntityManager //

//-----------------------------------------------
// ~CEntityManager :
// Destructor.
//-----------------------------------------------
CEntityManager::~CEntityManager()
{
	release();
}// ~CEntityManager //


//-----------------------------------------------
// initialize :
//
//-----------------------------------------------
void CEntityManager::initialize(uint nbMaxEntity)
{
	// Set the maximum number of entities.
	_NbMaxEntity = nbMaxEntity;
	// if
	if(_NbMaxEntity)
	{
		_Entities.resize(_NbMaxEntity, 0);
		_EntityGroundFXHandle.resize(_NbMaxEntity);
	}

	ICDBNode::CTextId textId;

	// Add an observer on the mission database
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	uint i,j;
	for (i=0; i<MAX_NUM_MISSIONS; i++)
	for (j=0; j<MAX_NUM_MISSION_TARGETS; j++)
	{
		std::string text = toString("SERVER:MISSIONS:%d:TARGET%d:TITLE", i, j);
		textId = ICDBNode::CTextId(text);
		NLGUI::CDBManager::getInstance()->getDB()->addObserver(&MissionTargetObserver, textId );
		_MissionTargetTitleDB[i][j] = NLGUI::CDBManager::getInstance()->getDbProp(text, false);
		nlassert(_MissionTargetTitleDB[i][j]);
	}

	// Add an Observer to the Team database
	for (i=0; i<MaxNumPeopleInTeam; i++)
	{
		std::string text = toString(TEAM_DB_PATH ":%d:UID", i);
		textId = ICDBNode::CTextId(text);
		NLGUI::CDBManager::getInstance()->getDB()->addObserver(&TeamUIDObserver, textId );
		_GroupMemberUidDB[i] = NLGUI::CDBManager::getInstance()->getDbProp(text, false);
		nlassert(_GroupMemberUidDB[i]);
		
		text = toString(TEAM_DB_PATH ":%d:NAME", i);
		textId = ICDBNode::CTextId(text);
		NLGUI::CDBManager::getInstance()->getDB()->addObserver(&TeamPresentObserver, textId );
		_GroupMemberNameDB[i] = NLGUI::CDBManager::getInstance()->getDbProp(text, false);
		nlassert(_GroupMemberNameDB[i]);
	}

	// Add an Observer to the Animal database
	for (i=0; i<MAX_INVENTORY_ANIMAL; i++)
	{
		std::string text = toString("SERVER:PACK_ANIMAL:BEAST%d:UID", i);
		textId = ICDBNode::CTextId(text);
		NLGUI::CDBManager::getInstance()->getDB()->addObserver(&AnimalUIDObserver, textId);
		_BeastUidDB[i] = NLGUI::CDBManager::getInstance()->getDbProp(text, false);
		nlassert(_BeastUidDB[i]);
		
		text = toString("SERVER:PACK_ANIMAL:BEAST%d:STATUS", i);
		textId = ICDBNode::CTextId(text);
		NLGUI::CDBManager::getInstance()->getDB()->addObserver(&AnimalStatusObserver, textId);
		_BeastStatusDB[i] = NLGUI::CDBManager::getInstance()->getDbProp(text, false);
		nlassert(_BeastStatusDB[i]);
		
		text = toString("SERVER:PACK_ANIMAL:BEAST%d:TYPE", i);
		_BeastTypeDB[i] = NLGUI::CDBManager::getInstance()->getDbProp(text, false);
		nlassert(_BeastTypeDB[i]);
	}

}// initialize //


//-----------------------------------------------
// release :
// Free the class and all the components.
//-----------------------------------------------
void CEntityManager::release()
{
	_LastEntityUnderPos= NULL;

	// Remove all entities.
	for(uint i=0; i<_Entities.size(); ++i)
	{
		if(_Entities[i])
		{
			// remove from fx manager
			if (_Entities[i]->supportGroundFX())
			{
				_GroundFXManager.remove(_EntityGroundFXHandle[i]);
			}
			delete _Entities[i];
			_Entities[i] = 0;
		}
	}

	UserEntity = NULL;

	// Clear the list.
	_Entities.clear();

	// Clean the backuped list.
	_BackupedChanges.clear();

	_GroundFXManager.reset();
}// release //

//-----------------------------------------------
void CEntityManager::reinit()
{
	release();
	initialize(_NbMaxEntity);
}

CShapeInstanceReference CEntityManager::createInstance(const string& shape, const CVector &pos, const string& text, const string& url, bool bbox_active)
{
	CShapeInstanceReference nullinstref(UInstance(), string(""), string(""));
	if (!Scene) return nullinstref;

	UInstance instance = Scene->createInstance(shape);
	if (text.empty())
		bbox_active = false;
	CShapeInstanceReference instref = CShapeInstanceReference(instance, text, url, bbox_active);
	if(!instance.empty())
	{
		_ShapeInstances.push_back(instref);
	}
	return instref;
}

bool CEntityManager::removeInstances()
{
	if (!Scene) return false;
	// Remove all instances.
	for(uint i=0; i<_ShapeInstances.size(); ++i)
	{
		if (!_ShapeInstances[i].Instance.empty())
			Scene->deleteInstance(_ShapeInstances[i].Instance);
	}
	_ShapeInstances.clear();
	_InstancesRemoved = true;
	return true;
}

bool CEntityManager::instancesRemoved()
{
	bool instRemoved = _InstancesRemoved;
	_InstancesRemoved = false;
	return instRemoved;
}

CShapeInstanceReference CEntityManager::getShapeInstanceUnderPos(float x, float y)
{
	CShapeInstanceReference selectedInstance(UInstance(), string(""), string(""));
	_LastInstanceUnderPos= NULL;

	// If not initialised, return
	if (_ShapeInstances.empty())
		return selectedInstance;

	// build the ray
	CMatrix camMatrix = MainCam.getMatrix();
	CFrustum camFrust = MainCam.getFrustum();
	CViewport viewport = Driver->getViewport();

	// Get the Ray made by the mouse.
	CVector pos, dir;
	viewport.getRayWithPoint(x, y, pos, dir, camMatrix, camFrust);
	// Normalize the direction.
	dir.normalize();

	// **** Get instances with box intersecting the ray.
	float bestDist = 255;
	for(uint i=0; i<_ShapeInstances.size(); i++)
	{
		if (_ShapeInstances[i].BboxActive)
		{
			H_AUTO(RZ_Client_GEUP_box_intersect)

			// if intersect the bbox
			NLMISC::CAABBox bbox;
			//= _ShapeInstances[i].SelectionBox;
			_ShapeInstances[i].Instance.getShapeAABBox(bbox);
			if (bbox.getCenter() == CVector::Null)
			{
				bbox.setMinMax(CVector(-0.3f, -0.3f, -0.3f)+_ShapeInstances[i].Instance.getPos(), CVector(0.3f, 0.3f, 0.3f)+_ShapeInstances[i].Instance.getPos());
			}
			else
			{
				bbox.setMinMax((bbox.getMin()*_ShapeInstances[i].Instance.getScale().x)+_ShapeInstances[i].Instance.getPos(), (bbox.getMax()*_ShapeInstances[i].Instance.getScale().x)+_ShapeInstances[i].Instance.getPos());
			}
			if(bbox.intersect(pos, pos+dir*15.0f))
			{
				float dist = (bbox.getCenter()-pos).norm();
				if (dist < bestDist)
				{
					selectedInstance = _ShapeInstances[i];
					bestDist = dist;
				}
			}
		}
	}
	return	selectedInstance;
}


//-----------------------------------------------
// Create an entity according to the slot and the form.
// \param uint slot : slot for the entity.
// \param uint32 form : form to create the entity.
// \param TClientDataSetIndex : persitent id while the entity is connected.
// \return CEntityCL * : pointer on the new entity.
//-----------------------------------------------
CEntityCL *CEntityManager::create(uint slot, uint32 form, const TNewEntityInfo& newEntityInfo)
{
	// DEBUG
	if(verboseVP(NULL, form))
		nlinfo("(%05d,%03d) EM:create: slot '%u': %s", sint32(T1%100000), NetMngr.getCurrentServerTick(), slot, CSheetId(form).toString().c_str());
	// Check parameter : slot.
	if(slot >= _NbMaxEntity)
	{
		nlwarning("EM:create: Cannot create the entity, the slot '%u' is invalid.", slot);
		return 0;
	}
	else
	{
		// Slot 0 is for the user and so should be allocated only once (at beginning of main loop).
		if( slot == 0 && _Entities[0] )
		{
			if (newEntityInfo.DataSetIndex != CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
			{
				// Store the dataSetId received
				_Entities[0]->dataSetId(newEntityInfo.DataSetIndex);
			}
			// Store the alias (although there should not be one for the slot 0!)
			_Entities[0]->npcAlias(newEntityInfo.Alias);
			return 0;
		}
	}

	// Remove the old one (except the user).
	if(_Entities[slot])
	{
		nlwarning("EM:create: There is already an entity in the slot '%u' ! Old entity will be removed.", slot);
		// remove from ground fx manager
		// TODO : test if entity has ground fxs
		if (_Entities[slot]->supportGroundFX())
		{
			_GroundFXManager.remove(_EntityGroundFXHandle[slot]);
		}
		delete _Entities[slot];
		_Entities[slot] = 0;
	}

	// Check parameter : form.
	CEntitySheet *entitySheet = SheetMngr.get((CSheetId)form);
	if(entitySheet == 0)
	{
		nlwarning("EM:create: Attempt on create an entity with a bad form number %d (%s) for the slot '%d' trying to compute the default one.", form, ((CSheetId)form).toString().c_str(), slot);
		CSheetId defaultEntity;
		if(defaultEntity.buildSheetId(ClientCfg.DefaultEntity)==false)
		{
			nlwarning("EM:create: The default entity (%s) is not in the sheetid.bin.", ClientCfg.DefaultEntity.c_str());
			return 0;
		}
		entitySheet = SheetMngr.get(defaultEntity);
		if(entitySheet == 0)
		{
			nlwarning("EM:create: The default entity (%s) is not in the sheet manager.", ClientCfg.DefaultEntity.c_str());
			return 0;
		}
	}

	// Create the entity according to the type.


	switch(entitySheet->type())
	{
	case CEntitySheet::RACE_STATS:
	case CEntitySheet::CHAR:
		if (slot == 0)
		{
			nlassert (UserEntity == NULL);
			UserEntity = new CUserEntity;
			_Entities[slot] = UserEntity;
		}
		else
		{
			_Entities[slot] = new CPlayerCL;
		}
		break;

	case CEntitySheet::FAUNA:
	{
		CCharacterSheet *sheet = NLMISC::safe_cast<CCharacterSheet *>(entitySheet);
		if (!sheet->R2Npc) _Entities[slot] = new CCharacterCL;
		else _Entities[slot] = new CPlayerR2CL;
	}
	break;
	case CEntitySheet::FLORA:
		_Entities[slot] = new CCharacterCL;
		break;

	case CEntitySheet::FX:
		_Entities[slot] = new CFxCL;
		break;

	case CEntitySheet::ITEM:
		_Entities[slot] = new CItemCL;
		break;

	case CEntitySheet::FORAGE_SOURCE:
		_Entities[slot] = new CForageSourceCL;
		break;

	default:
		pushDebugStr(NLMISC::toString("Unknown Form Type '%d' -> entity not created.", entitySheet->type()));
		break;
	}

	// If the entity has been right created.
	if(_Entities[slot])
	{
		// Set the sheet Id.
		_Entities[slot]->sheetId((CSheetId)form);
		// Set the slot.
		_Entities[slot]->slot(slot);
		// Set the DataSet Index. AFTER slot(), so bar manager is correctly init
		_Entities[slot]->dataSetId(newEntityInfo.DataSetIndex);
		// Set the Mission Giver Alias
		_Entities[slot]->npcAlias(newEntityInfo.Alias);
		// Build the entity from a sheet.
		if(_Entities[slot]->build(entitySheet))
		{
			// Apply properties backuped;
			applyBackupedProperties(slot);
			// register to the ground fx manager
			if(_Entities[slot]->supportGroundFX())
			{
				_EntityGroundFXHandle[slot] = _GroundFXManager.add(_Entities[slot]);
			}
		}
		// Entity is not valid -> REMOVE IT
		else
		{
			// Everyone except the User
			if(slot != 0)
			{
				nlwarning("EM:%d: Cannot build the Entity -> REMOVE IT", slot);
				delete _Entities[slot];
				_Entities[slot] = 0;
			}
			// The User
			else
				nlerror("EM: Cannot build the User");
		}
	}
	// Entity Not Allocated
	else
		pushDebugStr(NLMISC::toString("Cannot Allocated the Entity in the slot '%d'.", slot));
	// Log problems about the entity creation.
	flushDebugStack(NLMISC::toString("Create Entity in slot '%d' with Form '%s' :", slot, ((CSheetId)form).toString().c_str()));
	// Return a pointer on the entity created.
	return _Entities[slot];
}// create //

//-----------------------------------------------
// remove :
// Delete an entity.
// \todo GUIGUI : rename into free.
// \todo GUIGUI : finish the function.
//-----------------------------------------------
bool CEntityManager::remove(uint slot, bool warning)
{
	// DEBUG
	if(verboseVP(NULL))
		nlinfo("EM:remove: slot '%u'.", slot);
	// Check parameter : slot.
	if(slot >= _NbMaxEntity)
	{
		nlwarning("CEntityManager::remove : Attempt on delete a bad slot (slot %d)", slot);
		return false;
	}

	// Do not delete the user.
	if(slot == 0)
	{
		nlwarning("CEntityManager::remove : Cannot remove the entity in the slot 0 (user slot).");
		return false;
	}

	// Slot not allocated.
	if(_Entities[slot] == 0)
	{
		if(warning)
		{
			nlwarning("CEntityManager::remove : Attempt on delete the slot '%d' that is not allocated.", slot);
			return false;
		}
	}

	// Remove the entity from others target.
	for(uint i=0; i<_Entities.size(); ++i)
	{
		// This entity is not allocated.
		if(_Entities[i] == 0)
			continue;

		// Inform about the slot of the entity that will be removed.
		_Entities[i]->slotRemoved(slot);
	}

	// remove ground fx
	if(_Entities[slot] != 0)
	{
		if (_Entities[slot]->supportGroundFX())
		{
			_GroundFXManager.remove(_EntityGroundFXHandle[slot]);
		}
	}

	// notify the projectile manager that entity has been removed
	CProjectileManager::getInstance().entityRemoved(slot);

	// notify the Bar Manager
	if(_Entities[slot])
		CBarManager::getInstance()->delEntity(_Entities[slot]->slot());

	// previous UnderPos?
	if(_LastEntityUnderPos==_Entities[slot])
		_LastEntityUnderPos= NULL;

	// Free the slot.
	delete _Entities[slot];
	_Entities[slot] = 0;

	// Done.
	return true;
}// remove //


//-----------------------------------------------
// removeCollision :
// Remove the collision for all entities.
//-----------------------------------------------
void CEntityManager::removeCollision()
{
	const uint nbEntities = (uint)_Entities.size();
	for(uint i=0; i<nbEntities; ++i)
	{
		// Is the entity allocated.
		if(_Entities[i] == 0)
			continue;

		// Remove the entity primitive.
		_Entities[i]->removePrimitive();

		// Remove the collision entity.
		_Entities[i]->removeCollisionEntity();
	}
}// removeCollision //

//-----------------------------------------------
// reloadAnims :
// Re-load animations (remove and load).
//-----------------------------------------------
void CEntityManager::reloadAnims()
{
	for(uint i=0; i<_Entities.size(); ++i)
	{
		if(_Entities[i])
		{
			// Get a reference on the current entity.
			CEntityCL &entity = *(_Entities[i]);
			// Change the playlist
			entity.buildPlaylist();
		}
	}
}// reloadAnims //


//-----------------------------------------------
// entity :
// Get a pointer on an entity according to the asked slot.
// \param uint slot : the asked slot.
// \return CEntityCL * : pointer on the entity or 0.
//-----------------------------------------------
CEntityCL *CEntityManager::entity(uint slot)
{
	// Return 0 if the slot is the INVALID_SLOT
	if(slot==CLFECOMMON::INVALID_SLOT)
		return 0;
	// Check parameter : slot.
	if(slot >= _Entities.size())
	{
		nlwarning("EM:entity: slot '%u' is invalid.", slot);
		if(ClientCfg.Check)
			nlstop;
		return 0;
	}
	// Return the entity pointer.
	return _Entities[slot];
}// entity //

//-----------------------------------------------
// entitiesNearDoors :
// Return if there is an entity near a door.
// \param float openingDist : near is when you are under the 'openingDist'.
// \param const CVector& posDoor1 : first door position.
// \param const CVector& posDoor2 : second door position.
// \return bool ; 'true' if any entity is near one of the door.
//-----------------------------------------------
bool CEntityManager::entitiesNearDoors(float openingDist, const CVector& posDoor1, const CVector& posDoor2)
{
	for(uint i=0; i<_NbMaxEntity; ++i)
	{
		// Is the entity allocated.
		if(_Entities[i] == 0)
			continue;

		// Get a reference on the current entity.
		CEntityCL &entity = *(_Entities[i]);

		// If the entity is close enough from the door -> return true.
		if( ((entity.pos() - posDoor1).sqrnorm() < openingDist)
		||  ((entity.pos() - posDoor2).sqrnorm() < openingDist) )
			return true;
	}

	// No Entity near the door.
	return false;
}// entitiesNearDoors //

//-----------------------------------------------
// getEntityListForSelection
//-----------------------------------------------
void	CEntityManager::getEntityListForSelection(std::vector<CEntityCL*> &entities, uint flags)
{
	// According to the view (first or third person), the user can or cannot be selected.
	entities.clear();
	uint firstEntity = (flags&CEntityFilterFlag::NotUser)?1:0;
	for(uint i=firstEntity; i<_NbMaxEntity; ++i)
	{
		// Is the entity allocated and not user mount.
		if(_Entities[i] == 0 || i==UserEntity->mount())
			continue;
		// If entity unselectable, skip
		if(!_Entities[i]->properties().selectable())
			continue;

		// Apply each filter
		if ( (flags&CEntityFilterFlag::Friend) && !_Entities[i]->isFriend() )
			continue;
		if ( (flags&CEntityFilterFlag::Enemy) && !_Entities[i]->isEnemy() )
			continue;
		if ( (flags&CEntityFilterFlag::Alive) && _Entities[i]->isReallyDead() )
			continue;
		if ( (flags&CEntityFilterFlag::Dead) && !_Entities[i]->isReallyDead() )
			continue;
		if ( (flags&CEntityFilterFlag::Player) && !_Entities[i]->isPlayer() )
			continue;
		if ( (flags&CEntityFilterFlag::NonPlayer) && _Entities[i]->isPlayer() )
			continue;

		// Insert every entity in the valid list.
		entities.push_back(_Entities[i]);
	}
}

//-----------------------------------------------
// getEntityUnderPos :
// Get the entity under the (2d) position. Return NULL if not entity under this position.
//-----------------------------------------------
struct CSortEntity
{
	CEntityCL	*Entity;
	float		Depth;

	bool	operator<(const CSortEntity &o) const
	{
		return Depth<o.Depth;
	}
};
CEntityCL *CEntityManager::getEntityUnderPos(float x, float y, float distSelection, bool &isPlayerUnderCursor)
{
	H_AUTO (RZ_Client_getEntityUnderPos )
	uint	i;

	// valid only if bbox still intersect
	CEntityCL	*precEntityUnderPos= _LastEntityUnderPos;
	bool		precEntityUnderPosValid= false;

	// reset result
	isPlayerUnderCursor= false;
	_LastEntityUnderPos= NULL;

	// If not initialised, return
	if (_Entities.empty())
		return NULL;


	// **** list of valid entities to test
	static	vector<CEntityCL*>		validEntities;
	uint	filterFlags= CEntityFilterFlag::NoFilter;
	getEntityListForSelection(validEntities, filterFlags);

	// build the ray
	CMatrix camMatrix = MainCam.getMatrix();
	CFrustum camFrust = MainCam.getFrustum();
	CViewport viewport = Driver->getViewport();

	// Get the Ray made by the mouse.
	CVector pos, dir;
	viewport.getRayWithPoint(x, y, pos, dir, camMatrix, camFrust);
	// Normalize the direction.
	dir.normalize();


	// **** Get entities with box intersecting the ray.
	static	vector<CSortEntity>		intersectedEntities;
	intersectedEntities.clear();
	for(i=0;i<validEntities.size();i++)
	{
		H_AUTO(RZ_Client_GEUP_box_intersect)
		CEntityCL *entity = validEntities[i];
		// if entity not visible, skip
		if(entity->getLastClip())
			continue;

		// if intersect the bbox
		NLMISC::CAABBox bbox = entity->selectBox();
		if(bbox.intersect(pos, pos+dir*distSelection))
		{
			// add this entity to the list of possible entities
			CSortEntity	e;
			e.Entity= entity;
			e.Depth= (bbox.getCenter()-pos).norm();
			intersectedEntities.push_back(e);

			// is it the last entity under pos?
			if(entity==precEntityUnderPos)
				precEntityUnderPosValid= true;
		}
	}

	// if no intersected entities, quit
	if(intersectedEntities.empty())
		return NULL;

	// Compute startDistBox: nearest entity distance, but the user
	float	startDistBox;
	if(intersectedEntities[0].Entity==UserEntity)
	{
		// if the nearest entity is the user, set res
		isPlayerUnderCursor= true;
		// if only player intersected, return NULL!
		if(intersectedEntities.size()==1)
			return NULL;
		// so take the second for startDistBox
		startDistBox= intersectedEntities[1].Depth;
	}
	else
	{
		// ok, take it.
		startDistBox= intersectedEntities[0].Depth;
	}


	// **** get best entity according to distance face-camera or box-ray if no face intersection
	CEntityCL	*entitySelected= NULL;
	float		bestDistBox= FLT_MAX;
	float		bestDistZ= FLT_MAX;
	for(i=0;i<intersectedEntities.size();i++)
	{
		CEntityCL *entity = intersectedEntities[i].Entity;
		const NLMISC::CAABBox &bbox = entity->selectBox();

		// If this entity is the UserEntity, skip!!
		if(entity==UserEntity)
			continue;

		// if entity skeleton model was clipped, skip
		USkeleton	*skeleton= entity->skeleton();
		if(!ClientCfg.Light && skeleton && !skeleton->getLastClippedState())
			continue;

		H_AUTO(RZ_Client_GEUP_face_intersect)


		// *** Try get face-intersection, result in distZ
		// if the entity support fast and precise intersection (and if it succeeds)
		bool	trueIntersectComputed= false;
		float	dist2D, distZ;
		if(!ClientCfg.Light)
		{
			if(skeleton)
			{
				if(skeleton->supportFastIntersect() && skeleton->fastIntersect(pos, dir, dist2D, distZ, false))
					trueIntersectComputed= true;
			}
			// get the intersection with the instance (bot object)
			else if(!entity->instances().empty() && !entity->instances()[0].Current.empty())
			{
				UInstance	inst= entity->instances()[0].Current;
				if(inst.supportFastIntersect() && inst.fastIntersect(pos, dir, dist2D, distZ, false))
					trueIntersectComputed= true;
			}
		}

		// if true face-intersection not found
		if(!trueIntersectComputed)
		{
			/*
				this happens especially for Forage Source. but could happens for anyhting else
				In this case, estimate face-instersection, with box:
				Suppose full intersection, if the ray is in the 1/3 of the bbox
			*/

			// clip the ray with the box
			CVector	a= pos, b= pos+dir*distSelection;
			if(!bbox.clipSegment(a, b))
				continue;
			// take the middle of the clipped segment. suppose that this middle is the "nearest ray point to center"
			// This is false, but gives better results.
			CVector	m= (a+b)/2;

			// Suppose full intersection, if the ray is in the 1/3 of the bbox
			CVector	itToCenter= m-bbox.getCenter();
			itToCenter.maxof(itToCenter, -itToCenter);
			CVector	smallBoxHS= bbox.getHalfSize()*0.3f;
			smallBoxHS.maxof(smallBoxHS, -smallBoxHS);
			if(itToCenter.x<=smallBoxHS.x && itToCenter.y<=smallBoxHS.y && itToCenter.z<=smallBoxHS.z)
			{
				dist2D= 0;
				distZ= (m-pos).norm();
			}
			else
			{
				// no intersection
				dist2D= FLT_MAX;
				distZ= 0;
			}
		}
		// else it's ok, dist2D and distZ are computed


		// *** if intersect face, then take the best face-intersection, else use box-ray cost
		// true face-col found?
		if(dist2D==0)
		{
			// yes, get the nearest
			if(distZ<bestDistZ)
			{
				bestDistBox= 0;
				bestDistZ= distZ;
				entitySelected= entity;
			}
		}
		// else
		else
		{
			// if a true face-intersection has not been found for others entities
			if(bestDistZ==FLT_MAX)
			{
				// get the "distance to camera" contribution.
				CVector			c= bbox.getCenter();
				float			distCamCost= intersectedEntities[i].Depth;
				// get relative to the nearest intersected entity
				distCamCost-= startDistBox;

				// get the ratio "how many the ray is in the bbox"
				CVector	a= pos, b= pos+dir*distSelection;
				bbox.clipSegment(a, b);
				// take the middle of the clipped segment. suppose that this middle is the "nearest ray point to center"
				// This is false, but gives better results.
				CVector	m= (a+b)/2;
				// get the distance to center. NB: small entities are preferred since smaller mean lower cost
				float			outBBoxCost= (m-c).norm();

				// the final cost is a weighted sum of the both. NB: distCamCost is in meter,
				// and outBBBoxCost is meters. Hence ClientCfg.SelectionOutBBoxWeight is a factor
				float	boxCost= distCamCost + outBBoxCost * ClientCfg.SelectionOutBBoxWeight;

				// take the lowest cost
				if(boxCost<bestDistBox)
				{
					entitySelected= entity;
					bestDistBox= boxCost;
				}
			}
		}
	}

	// If precise intersection not found
	if(bestDistZ==FLT_MAX)
	{
		// if the last entity under pos is valid, prefer it among all other approximate ones
		if(precEntityUnderPos && precEntityUnderPosValid)
			entitySelected= precEntityUnderPos;
	}

	// return the best entity
	_LastEntityUnderPos= entitySelected;
	return	entitySelected;
}// getEntityUnderPos //


//-----------------------------------------------
// getEntityInCamera
//-----------------------------------------------
CEntityCL *CEntityManager::getEntityInCamera(uint flags, float distSelection, CLFECOMMON::TCLEntityId	precEntity)
{
	H_AUTO (RZ_Client_getEntityInCamera )

	// If not initialised, return
	if (_Entities.empty())
		return NULL;

	// list of valid entities
	static	vector<CEntityCL*>		validEntitiesTmp, validEntities;
	getEntityListForSelection(validEntitiesTmp, flags);

	// Remove entities not selectable by space key
	uint i;
	validEntities.clear();
	for (i=0 ; i<validEntitiesTmp.size() ; i++)
	{
		CCharacterCL *entity = dynamic_cast<CCharacterCL*>(validEntitiesTmp[i]);
		if ((entity == NULL) || (entity && entity->isSelectableBySpace()))
			validEntities.push_back(entity);
	}

	// Build the camera pyramid
	CMatrix camMatrix = MainCam.getMatrix();
	CFrustum camFrust = MainCam.getFrustum();
	static	vector<CPlane>		camPyramid;
	// No need to use worldMatrix. NB: not setuped if ClientLight.
	MainCam.buildCameraPyramid(camPyramid, false);

	// list of entities in screen
	static vector<CSortEntity>	screenEntities;
	screenEntities.clear();

	// compute distance related to the user pos (not camera one).
	CVector	userPos = UserEntity->pos();

	// prefer take the direction of the camera (can select backward for instance with camera rotation)
	CVector	userDir = View.currentView().normed();

	// Get all entity in this pyramid, and in the dist selection
	for(i=0;i<validEntities.size();i++)
	{
		CEntityCL	*entity= validEntities[i];
		const NLMISC::CAABBox &b = entity->selectBox();
		bool	isIn= true;
		for(uint j=0;j<camPyramid.size();j++)
		{
			if( !b.clipBack(camPyramid[j]) )
			{
				isIn= false;
				break;
			}
		}
		// if In the pyramid
		if(isIn)
		{
			CVector		dirToEntity= b.getCenter()-userPos;
			CSortEntity	eSelect;
			eSelect.Entity= entity;
			eSelect.Depth= dirToEntity.norm();
			// if in max distance
			if(eSelect.Depth<distSelection)
			{
				// The lower, the more the influence of direction (minimum should be 1)
				const float	dirInfluence= 1.1f;

				// modulate the depth with dot3: force take the most in front of user.
				if(eSelect.Depth>0)
					dirToEntity/= eSelect.Depth;
				eSelect.Depth*= dirInfluence-dirToEntity*userDir;

				// append to sort list
				screenEntities.push_back(eSelect);
			}
		}
	}

	// No one in screen?
	if(screenEntities.empty())
		return NULL;

	// sort them increasingly
	sort(screenEntities.begin(), screenEntities.end());

	// Try to find the precEntity in this list
	uint	entitySelected= 0;
	if(precEntity!=CLFECOMMON::INVALID_SLOT)
	{
		for(i=0;i<screenEntities.size();i++)
		{
			// if found the precEntity, get the farther one
			if(screenEntities[i].Entity->slot()==precEntity)
			{
				entitySelected= i+1;
				break;
			}
		}
		// reset to 0 if: no more entities, or if the max cycle is reached
		if(entitySelected>=screenEntities.size() || entitySelected>=ClientCfg.SpaceSelectionMaxCycle)
			entitySelected= 0;
	}

	// found!
	return screenEntities[entitySelected].Entity;
}

//-----------------------------------------------
// changeContinent :
// Continent has changed.
//-----------------------------------------------
void CEntityManager::changeContinent()
{
	// Re-create entities primitive.
	for(uint i=0; i<_NbMaxEntity; ++i)
	{
		// Is the entity allocated.
		if(_Entities[i] == 0)
			continue;

		// Compute the new primitive.
		_Entities[i]->computePrimitive();

		// Compute the new collision entity.
		_Entities[i]->computeCollisionEntity();
	}
}// changeContinent //


//-----------------------------------------------
// updatePreCamera :
// Update entites before the camera position is computed.
// This update the entites position. Evaluate collisions. Compte final world position.
//-----------------------------------------------
void CEntityManager::updatePreCamera()
{
	H_AUTO ( RZ_Client_Entity_Mngr_Update_Pre_Cam )
	uint i;
	// Build an entity list..
	_ActiveEntities.reserve (_Entities.size ());
	_ActiveEntities.clear ();
	// Reset Counters
	resetCounters();
	// Update entities position.
	for(i=0; i<_NbMaxEntity; ++i)
	{
		// Is the entity allocated.
		CEntityCL *entity = _Entities[i];
		if(entity == 0)
			continue;
		// Count Entities
		++_EntitiesAllocated;
		switch(entity->Type)
		{
		case CEntityCL::User:
			++_NbUser;   break;
		case CEntityCL::Player:
			++_NbPlayer; break;
		/*case CEntityCL::NPC:
		case CEntityCL::Fauna:
		case CEntityCL::Entity:
		case CEntityCL::ForageSource:*/
		default:
			++_NbChar;   break;
		}
		// Update the list of Active Entities
		_ActiveEntities.push_back (CEntityReference (i, entity));
	}
	// Adjust the orientation of the NPC in trade with the user.
	if(UserEntity->trader() != CLFECOMMON::INVALID_SLOT)
	{
		CEntityCL * trader = _Entities[UserEntity->trader()];
		if(trader)
			trader->front(UserEntity->pos() - trader->pos());
	}
	// Adjust the orientation of the NPC in dyn chat with the user.
	if(UserEntity->interlocutor() != CLFECOMMON::INVALID_SLOT)
	{
		CEntityCL * interlocutor = _Entities[UserEntity->interlocutor()];
		if(interlocutor)
			interlocutor->front(UserEntity->pos() - interlocutor->pos());
	}

	// Update entities position except the User
	for(i=1; i<_EntitiesAllocated; ++i)
	{
		CEntityReference &activeEntity = _ActiveEntities[i];

		// Get a poiner on the entity target
		CEntityCL *target = entity(activeEntity.Entity->targetSlot());

		// Update the entity.
		activeEntity.Entity->updatePreCollision(T1, target);
	}
	// USER
	{
		// Get a poiner on the entity target
		CEntityCL *target = entity(UserEntity->targetSlot());
		// update user behaviour/speed/heading/vectorUp/position/bodyHeading
		UserEntity->applyMotion(target);
		// Update the entity.
		UserEntity->updatePreCollision(T1, target);
	}
	// Update PACS
	if(PACS)
	{
		// Time since last Frame
		double DTEval = ((float)(T1-T0))*0.001f;
		PACS->evalCollision(DTEval, staticWI);		// Eval the static world.
		PACS->evalCollision(DTEval, dynamicWI);		// Eval the dynamic world.
		getDoorManager().getPACSTriggers();			// Copy triggers to be used in update
		managePACSTriggers();
		UserEntity->checkPos();
	}
	// Update entities position.
	for(i=0; i<_EntitiesAllocated; ++i)
	{
		CEntityReference &activeEntity = _ActiveEntities[i];
		// Get a poiner on the entity target
		CEntityCL *target = entity(activeEntity.Entity->targetSlot());
		// Update the entity.
		activeEntity.Entity->updatePostCollision(T1, target);
	}
	// User Orientation
	UserEntity->applyForceLook();


	getDoorManager().update();					// Check for trigger to open/close doors

	MissionTargetObserver.update();

}// updatePreCamera //


//-----------------------------------------------
// updatePostCamera :
// Update the entity (position\animation).
// Clip the primitives
// Update visual entites parameters for clipped and non-clipped primitives
// This update the entites position.
//-----------------------------------------------
void CEntityManager::updatePostCamera(uint clippedUpdateTime, const std::vector<CPlane> &clippingPlanes, const CVector &camPos)
{
	H_AUTO ( RZ_Client_Entity_Mngr_Update_Post_Cam )

	// Build a non clipped entity list..
	_VisibleEntities.reserve (_Entities.size ());
	_VisibleEntities.clear ();

	static bool firstTime = true;

	// Clip entities position.
	uint i;
	for(i=0; i<_EntitiesAllocated; ++i)
	{
		CEntityReference &activeEntity = _ActiveEntities[i];

		// Get a poiner on the entity target
		CEntityCL *target = entity(activeEntity.Entity->targetSlot());

		// Clip it
		if (!activeEntity.Entity->clipped(clippingPlanes, camPos)
			|| (R2::getEditor().getSelectedInstance() && R2::getEditor().getSelectedInstance()->getEntity()==activeEntity.Entity))
		{
			// Add to visible primitives
			_VisibleEntities.push_back (activeEntity);
			activeEntity.Entity->setLastClip(false);
			activeEntity.Entity->updateVisible (T1, target);
		}
		else
		{
			activeEntity.Entity->setLastClip(true);
			if (firstTime)
			{
				// Update texture Async Loading
				activeEntity.Entity->updateAsyncTexture();

				// Update lod Texture
				activeEntity.Entity->updateLodTexture();

			}

			// Update this clipped primitive at this time ?
			if ((activeEntity.Slot&RZ_CLIPPED_UPDATE_TIME_MASK) == clippedUpdateTime)
			{
				activeEntity.Entity->updateSomeClipped (T1, target);
			}

			// Update clipped primitives
			activeEntity.Entity->updateClipped (T1, target);
		}
	}

	// Update visible entities post positions.
	const uint count = (uint)_VisibleEntities.size ();
	for(i=0; i<count; ++i)
	{
		CEntityReference &visibleEntity = _VisibleEntities[i];
		// Get a poiner on the entity target
		CEntityCL *target = entity(visibleEntity.Entity->targetSlot());
		//
		visibleEntity.Entity->updateVisiblePostPos(T1, target);
	}

	// update ground fx
	_GroundFXManager.update(NLMISC::CVectorD(camPos));

	firstTime = false;
}// updatePostCamera //

//-----------------------------------------------
// updatePostRender :
// Update entites after the render 3D.
//-----------------------------------------------
void CEntityManager::updatePostRender()
{
	H_AUTO_USE ( RZ_Client_Update_Post_Render )

	TextContext->setHotSpot(UTextContext::MiddleMiddle);
	TextContext->setFontSize(ClientCfg.NameFontSize);
	CRGBA color;

	const uint activeCount = (uint)_ActiveEntities.size ();
	uint i;
	for(i=0; i<activeCount; i++)
	{
		CEntityReference &visibleEntity = _ActiveEntities[i];

		// Update in-scene interface
		visibleEntity.Entity->updateAllPostRender ();
	}

	const uint count = (uint)_VisibleEntities.size ();
	for(i=0; i<count; ++i)
	{
		CEntityReference &visibleEntity = _VisibleEntities[i];
		// Update Visible Entities after the render.
		visibleEntity.Entity->updateVisiblePostRender();

		// Draw the entity Path.
		if(ClientCfg.ShowPath)
			visibleEntity.Entity->drawPath();
		// Draw the selection box.
		if(ClientCfg.DrawBoxes)
			visibleEntity.Entity->drawBox();
		// Display Modifiers (Dmgs/heals).
		if(1)
			visibleEntity.Entity->displayModifiers();
	}

	// Flush any no more used Flying text. Must do it before interface display (to be sure texts are hid)
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	pIM->FlyingTextManager.releaseNotUsedFlyingText();

}// updatePostRender //


//-----------------------------------------------
// updateVisualProperty :
// Method to update the visual property 'prop' for the entity in 'slot'.
// \param uint slot : slot of the entity to update.
// \param uint prop : the property to udapte.
//-----------------------------------------------
void CEntityManager::updateVisualProperty(const NLMISC::TGameCycle &gameCycle, const uint &slot, const uint &prop, const NLMISC::TGameCycle &predictedInterval)
{
	// INFO : log some debug information about visual properties.
	if(verboseVP(NULL))
		nlinfo("EM:updateVP: received prop '%d' for the slot '%d'.", prop, slot);

	// Check parameter : slot.
	if(slot >= _NbMaxEntity)
	{
		nlwarning("CEntityManager::updateVisualProperty : Slot '%d' is not valid.", slot);
		return;
	}

	// Entity still not allocated -> backup values received for the entity.
	if(_Entities[slot] == 0)
	{
		// INFO : log some debug information about visual properties.
		if(verboseVP(NULL))
			nlinfo("EM:updateVP: backup the property as long as the entity is not allocated.", prop, slot);

		string propName = toString("SERVER:Entities:E%d:P%d", slot, prop);
		TProperty propty;
		propty.GC    = gameCycle;
		propty.Value = 0;
//		propty.Value = IngameDbMngr.getProp(propName);


		TBackupedChanges::iterator it = _BackupedChanges.find(slot);
		// Entity does not have any changes backuped for the time.
		if(it == _BackupedChanges.end())
		{
			TProperties propMap;
			propMap.insert(make_pair(prop, propty));
			_BackupedChanges.insert(make_pair(slot, propMap));
		}
		// Entity already have some changes backuped.
		else
		{
			TProperties &properties = (*it).second;
			TProperties::iterator itProp = properties.find(prop);
			// This properties is still not backuped for this entity.
			if(itProp == properties.end())
				properties.insert(make_pair(prop, propty));
			// There is already a backuped value
			else
			{
				nlwarning("EM:updateVP:%d: property '%d' already backuped.", slot, prop);
				(*itProp).second = propty;
			}
		}
	}
	// Entity already allocated -> apply values.
	else
	{
		// Call the method from the entity to update the visual property.
		_Entities[slot]->updateVisualProperty(gameCycle, prop, predictedInterval);
	}
}// updateVisualProperty //

//-----------------------------------------------
// applyBackupedProperties :
//-----------------------------------------------
void CEntityManager::applyBackupedProperties(uint slot)
{
	TBackupedChanges::iterator it = _BackupedChanges.find(slot);
	if(it != _BackupedChanges.end())
	{
		TProperties &properties = (*it).second;
		TProperties::iterator itProp = properties.begin();
		while(itProp != properties.end())
		{
			_Entities[slot]->updateVisualProperty((*itProp).second.GC, (*itProp).first, 0);
			++itProp;
		}

		_BackupedChanges.erase(it);
	}
}// applyBackupedProperties //



//-----------------------------------------------
// writeEntities :
// Write a file with the position of all entities.
//-----------------------------------------------
void CEntityManager::writeEntities()
{
	COFile f;
	if(!f.open("entities.txt", false, true))
		return;

	string strTmp = "StartCommands = {\n";
	f.serialBuffer((uint8*)strTmp.c_str(), (uint)strTmp.size());

	const uint nb = (uint)_Entities.size();
	for(uint i=1; i<nb; ++i)
	{
		if(_Entities[i])
		{
			strTmp = toString("\"%s\",\t\"%f\", \"%f\", \"%f\", \"%f\", \"%f\", \"%f\",\t// %3d\n", _Entities[i]->sheetId().toString().c_str(), _Entities[i]->pos().x, _Entities[i]->pos().y, _Entities[i]->pos().z, _Entities[i]->front().x, _Entities[i]->front().y, _Entities[i]->front().z, i);
			f.serialBuffer((uint8*)strTmp.c_str(), (uint)strTmp.size());
		}
	}

	strTmp = "};\n";
	f.serialBuffer((uint8*)strTmp.c_str(), (uint)strTmp.size());

	// Close the File.
	f.close();
}// writeEntities //

//-----------------------------------------------
// serial
// Serialize entities.
//-----------------------------------------------
void CEntityManager::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Get nb max entities possible.
	f.serial(_NbMaxEntity);
	if(f.isReading())
	{
		release();
		initialize(_NbMaxEntity);
	}

//	f.serial(_EntitiesAllocated);	no need to serialize this one except maybe to check.

	// Serialize each entity.
	const uint nb = (uint)_Entities.size();
	for(uint i=0; i<nb; ++i)
	{
		NLMISC::CSheetId si;
		if(!f.isReading())
		{
			if(_Entities[i])
				si = _Entities[i]->sheetId();
			else
				si = NLMISC::CSheetId::Unknown;
		}

		// ...
		f.serial(si);

		// Create the entity.
		if(f.isReading() && (si != CSheetId::Unknown))
		{
			TNewEntityInfo emptyEntityInfo;
			emptyEntityInfo.reset();
			create(i, si.asInt(), emptyEntityInfo);
		}

		// Get/Set entity state.
		if(_Entities[i])
			_Entities[i]->serial(f);
	}
}// serial //


//-----------------------------------------------
// dump :
// // Dump entities state.
//-----------------------------------------------
void CEntityManager::dump(class NLMISC::IStream &f)
{
	// Serialize the class.
	serial(f);
}// dump //

//-----------------------------------------------
// dumpXML :
// Dump entities state (XML Format).
//-----------------------------------------------
void CEntityManager::dumpXML(class NLMISC::IStream &f)
{
	// Start the opening of a new node named Identity
	f.xmlPush("Entities");

		const uint nb = (uint)_Entities.size();
		for(uint i=0; i<nb; ++i)
		{
			// Add a comment
//			f.xmlComment();//toString("Describ the entity in the slot %d.", i).c_str());
			// Start the opening of a new node named Identity
			f.xmlPush(toString("Entity%d", i).c_str());

				if(_Entities[i])
				{
					// Open a new node header named Address
					f.xmlPushBegin("Name");
					// Set a property name
					f.xmlSetAttrib ("string");
					ucstring n = _Entities[i]->getEntityName();
					f.serial(n);
					// Close the new node header
					f.xmlPushEnd();
					// Close the address node
					f.xmlPop();

					// Open a new node header named Address
					f.xmlPushBegin("Sheet");
					// Set a property name
					f.xmlSetAttrib ("name");
					string sheetName = _Entities[i]->sheetId().toString();
					f.serial(sheetName);
					// Close the new node header
					f.xmlPushEnd();
					// Close the address node
					f.xmlPop();

					// Open a new node header named Address
					f.xmlPushBegin("Position");
					// Close the new node header
					f.xmlPushEnd();
					f.serial(_Entities[i]->pos());
					// Close the address node
					f.xmlPop();

					// Open a new node header named Address
					f.xmlPushBegin("Front");
					// Close the new node header
					f.xmlPushEnd();
					NLMISC::CVector front = _Entities[i]->front();
					f.serial(front);
					// Close the address node
					f.xmlPop();

					// Open a new node header named Address
					f.xmlPushBegin("Mode");
					// Set a property name
					f.xmlSetAttrib ("name");
					string mode = MBEHAV::modeToString(_Entities[i]->mode());
					f.serial(mode);
					// Set a property name
					f.xmlSetAttrib ("num");
					uint8 m = _Entities[i]->mode();
					f.serial(m);
					// Close the new node header
					f.xmlPushEnd();
					// Close the address node
					f.xmlPop();

					// Open a new node header named Address
					f.xmlPushBegin("LastBehaviourPlayed");
					// Set a property name
					f.xmlSetAttrib ("name");
					string beh = MBEHAV::behaviourToString(_Entities[i]->behaviour());
					f.serial(beh);
					// Set a property name
					f.xmlSetAttrib ("num");
					uint8 b = _Entities[i]->behaviour();
					f.serial(b);
					// Close the new node header
					f.xmlPushEnd();
					// Close the address node
					f.xmlPop();
				}

			// Close the address node
			f.xmlPop();
		}

	// Close the identity node
	f.xmlPop();
}// dumpXML //

//-----------------------------------------------

CEntityCL *CEntityManager::getEntityByName (uint32 stringId) const
{
	if (stringId)
	{
		uint i;
		const uint count = (uint)_Entities.size();
		for (i=0; i<count; i++)
		{
			if(_Entities[i])
				if(_Entities[i]->getNameId() == stringId)
					return _Entities[i];
		}
	}
	return NULL;
}

//-----------------------------------------------

CEntityCL *CEntityManager::getEntityByName (const ucstring &name, bool caseSensitive, bool complete) const
{
	ucstring source = name;
	const uint size = (uint)source.size();
	if (!caseSensitive)
	{
		uint j;
		for (j=0; j<size; j++)
			source[j] = tolower (source[j]);
	}

	uint i;
	const uint count = (uint)_Entities.size();
	uint selectedEntityId = 0;
	float selectedEntityDist = FLT_MAX; // No selected Entity

	for (i=0; i<count; i++)
	{
		if(_Entities[i])
		{
			ucstring value = _Entities[i]->getDisplayName();
			bool foundEntity = false;

			uint j;
			if (!caseSensitive)
			{
				for (j=0; j<value.size(); j++)
					value[j] = tolower (value[j]);
			}

			// Complete test ?
			if (complete)
			{
				if(value == source)
					foundEntity = true;
			}
			else
			{
				if (value.size() >= size)
				{
					if (std::operator==(source, value.substr (0, size)))
						foundEntity = true;
				}
			}

			if (foundEntity)
			{
				const NLMISC::CVectorD &targetPosD = _Entities[i]->pos();
				const NLMISC::CVectorD &userPosD = UserEntity->pos();

				float deltaX = (float) targetPosD.x - (float) userPosD.x;
				float deltaY = (float) targetPosD.y - (float) userPosD.y;
				float dist = (float)sqrt(deltaX * deltaX + deltaY * deltaY);
				if (dist < selectedEntityDist)
				{
					selectedEntityDist = dist;
					selectedEntityId = i;
				}
			}
		}
	}
	if (selectedEntityDist != FLT_MAX) // Entity found
		return _Entities[selectedEntityId];
	else
		return NULL;
}

//-----------------------------------------------

CEntityCL *CEntityManager::getEntityByCompressedIndex(TDataSetIndex compressedIndex) const
{
	if (compressedIndex != INVALID_DATASET_ROW)
	{
		uint i;
		const uint count = (uint)_Entities.size();
		for (i=0; i<count; i++)
		{
			if(_Entities[i])
				if(_Entities[i]->dataSetId() == compressedIndex)
					return _Entities[i];
		}
	}
	return NULL;
}

//-----------------------------------------------
// managePACSTriggers :
// Manage PACS Triggers.
//-----------------------------------------------
void CEntityManager::managePACSTriggers()
{
	uint i;
	const uint nNbTrig = PACS->getNumTriggerInfo();
	for(i=0; i<nNbTrig; ++i)
	{
		const NLPACS::UTriggerInfo &rTI = PACS->getTriggerInfo(i);
		// Detect collisions between user and other entities, to not be block (only the user is a trigger so no need to check).
		if(((rTI.Object0 & 0xFFFF) == UserDataEntity)
		&& ((rTI.Object1 & 0xFFFF) == UserDataEntity))
		{
			UserEntity->startColTimer();
			break;
		}
	}
	// Stop Collision.
	if(i >= nNbTrig)
		UserEntity->stopColTimer();
}// managePACSTriggers //


//-----------------------------------------------
// removeColUserOther :
//
//-----------------------------------------------
void CEntityManager::removeColUserOther()
{
	uint i;
	const uint count = (uint)_Entities.size();
	for(i=1; i<count; i++)
	{
		if(_Entities[i])
		{
			if(_Entities[i]->getPrimitive())
			{
				// remove collision only if the entity is Traversable (bot objects may not)
				if(_Entities[i]->getTraversable())
					_Entities[i]->getPrimitive()->setObstacle(false);
			}
		}
	}
}// removeColUserOther //

//-----------------------------------------------
// restoreColUserOther :
//
//-----------------------------------------------
void CEntityManager::restoreColUserOther()
{
	uint i;
	const uint count = (uint)_Entities.size();
	for(i=1; i<count; i++)
	{
		if(_Entities[i])
		{
			if(_Entities[i]->getPrimitive())
				_Entities[i]->getPrimitive()->setObstacle(true);
		}
	}
}// restoreColUserOther //

//-----------------------------------------------
void CEntityManager::removeAllAttachedFX()
{
	for(TEntities::iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		if (*it) (*it)->removeAllAttachedFX();
	}

}

// ***************************************************************************
void	CEntityManager::resetAllSoundAnimId()
{
	for(uint i=0;i<_Entities.size();i++)
	{
		CEntityCL	*ent= _Entities[i];
		if(ent)
		{
			ent->resetAllSoundAnimId();
		}
	}
}

// ***************************************************************************
#define nldebugraw NLMISC::createDebug(), NLMISC::DebugLog->displayRawNL

// ***************************************************************************
void	CEntityManager::startLogStageChange(sint32 currentGameCycle, sint64 currentLocalTime)
{
	// first stop
	stopLogStageChange();

	// enable
	_LogStageChange.Enabled= true;
	_LogStageChange.StartGameCycle= currentGameCycle;
	_LogStageChange.StartLocalTime= currentLocalTime;
	_LogStageChange.LastEntityLoged= CLFECOMMON::INVALID_SLOT;
	_LogStageChange.StageSet.clear();
	nldebugraw("*** Start Loging Stage changes");
}


// ***************************************************************************
void	CEntityManager::logStageChange(sint64 currentLocalTime)
{
	if(!_LogStageChange.Enabled)
		return;

	// if still exist
	CCharacterCL	*ent= dynamic_cast<CCharacterCL*>(entity(WatchedEntitySlot));
	if(ent)
	{
		// if the watched entity has been changed
		if(WatchedEntitySlot!=_LogStageChange.LastEntityLoged)
		{
			_LogStageChange.LastEntityLoged= WatchedEntitySlot;
			// backup set
			_LogStageChange.StageSet= ent->_Stages._StageSet;
			nldebugraw("*** Start Loging Stage changes for Entity %d", WatchedEntitySlot);
		}
		else
		{
			// can log it
			sint32		recGcRef= _LogStageChange.StartGameCycle;
			sint64		recTimeRef= _LogStageChange.StartLocalTime;
			// compare 2 logs and display differences
			CStageSet::TStageSet		&oldStageSet= _LogStageChange.StageSet;
			const CStageSet::TStageSet	&newStageSet= ent->_Stages._StageSet;

			// for pos log detail
			CVectorD	precNewPos= ent->pos();
			CVectorD	precOldPos= ent->pos();

			// compare each new/old stage
			CStageSet::TStageSet::const_iterator	itOld= oldStageSet.begin();
			CStageSet::TStageSet::const_iterator	itNew= newStageSet.begin();
			while(itOld!=oldStageSet.end() || itNew!=newStageSet.end())
			{
				// compare 2 iterators
				sint signNewMinusOld;
				if(itNew==newStageSet.end())
					signNewMinusOld= +1;
				else if(itOld==oldStageSet.end())
					signNewMinusOld= -1;
				else
				{
					if(itNew->first > itOld->first)
						signNewMinusOld= +1;
					else if(itNew->first < itOld->first)
						signNewMinusOld= -1;
					else
						signNewMinusOld= 0;
				}

				// if signNewMinusOld= +1, it means an old exist, without a new (=> the stage has been removed)
				if(signNewMinusOld==+1)
				{
					logPropertyChange(WatchedEntitySlot, itOld->second, CStage(), precOldPos, precNewPos, (sint32)itOld->first-recGcRef, currentLocalTime-recTimeRef);
					// new prec pos (if any)
					itOld->second.getPos(precOldPos);
					itOld++;
				}
				// if signNewMinusOld= -1, it means an new exist, without an old (=> the stage has been added)
				else if(signNewMinusOld==-1)
				{
					logPropertyChange(WatchedEntitySlot, CStage(), itNew->second, precOldPos, precNewPos, (sint32)itNew->first-recGcRef, currentLocalTime-recTimeRef);
					// new prec pos (if any)
					itNew->second.getPos(precNewPos);
					itNew++;
				}
				// if ==0, means the stage exist in both, but properties set may be different
				else
				{
					logPropertyChange(WatchedEntitySlot, itOld->second, itNew->second, precOldPos, precNewPos, (sint32)itNew->first-recGcRef, currentLocalTime-recTimeRef);
					// new prec pos (if any)
					itOld->second.getPos(precOldPos);
					itNew->second.getPos(precNewPos);
					itOld++;
					itNew++;
				}
			}

			// bkup the new stage set
			oldStageSet= newStageSet;
		}
	}
	// this entity might have been deleted, stop its log
	else
	{
		_LogStageChange.LastEntityLoged= CLFECOMMON::INVALID_SLOT;
		_LogStageChange.StageSet.clear();
	}
}

// ***************************************************************************
void	CEntityManager::logPropertyChange(CLFECOMMON::TCLEntityId who, const CStage &oldStage, const CStage &newStage,
			const CVectorD &precOldPos, const CVectorD &precNewPos, sint32 relGameCycle, sint64 relLocalTime)
{
	// For all properties of interest
	CLFECOMMON::TPropIndex	propLoged[]= {CLFECOMMON::PROPERTY_POSITION, CLFECOMMON::PROPERTY_ORIENTATION,
		CLFECOMMON::PROPERTY_MODE, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID,
		CLFECOMMON::PROPERTY_BEHAVIOUR, CLFECOMMON::PROPERTY_TARGET_ID,
		/*CLFECOMMON::PROPERTY_VISUAL_FX,
		CLFECOMMON::PROPERTY_TARGET_LIST_0, CLFECOMMON::PROPERTY_TARGET_LIST_1,
		CLFECOMMON::PROPERTY_TARGET_LIST_2, CLFECOMMON::PROPERTY_TARGET_LIST_3*/};
	uint32	numProps= sizeof(propLoged) / sizeof(propLoged[0]);
	for(uint i=0;i<numProps;i++)
	{
		pair<bool, sint64>	oldProp= oldStage.property(propLoged[i]);
		pair<bool, sint64>	newProp= newStage.property(propLoged[i]);
		// if change of the prop, log it
		if((oldProp.first || newProp.first) && oldProp!=newProp)
		{
			// get the change reason
			string	reason;
			if(!oldProp.first)
				reason= "ADD";
			else if(!newProp.first)
				reason= "DEL";
			else
				reason= "CHG";

			// get the value
			sint64	value= newProp.second;
			if(!newProp.first)	value= oldProp.second;
			string	valStr;
			// mode?
			if(propLoged[i]==CLFECOMMON::PROPERTY_MODE)
			{
				valStr= MBEHAV::TMode((uint64)value).toString();
			}
			// behaviour
			else if(propLoged[i]==CLFECOMMON::PROPERTY_BEHAVIOUR)
			{
				valStr= MBEHAV::CBehaviour((uint64)value).toString();
			}
			// mount
			else if(propLoged[i]==CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID)
			{
				valStr= NLMISC::toString(value);
			}
			else if(propLoged[i]==CLFECOMMON::PROPERTY_RIDER_ENTITY_ID)
			{
				valStr= NLMISC::toString(value);
			}
			// Target
			else if(propLoged[i]==CLFECOMMON::PROPERTY_TARGET_ID)
			{
				valStr= NLMISC::toString(value);
			}
			// Position
			else if(propLoged[i]==CLFECOMMON::PROPERTY_POSITION)
			{
				// get the delta of move from previous pos stage
				CVectorD	pos;
				float		dist= 0.f;
				if(newProp.first)
				{
					if(newStage.getPos(pos))
						dist= float(CVectorD(pos.x-precNewPos.x, pos.y-precNewPos.y,0).norm());
					valStr= toString("dst=%.1f pi=%d", dist, newStage.predictedInterval());
				}
				else
				{
					if(oldStage.getPos(pos))
						dist= float(CVectorD(pos.x-precOldPos.x, pos.y-precOldPos.y,0).norm());
					valStr= toString("dst=%.1f pi=%d", dist, oldStage.predictedInterval());
				}
			}
			// Orientation
			else if(propLoged[i]==CLFECOMMON::PROPERTY_ORIENTATION)
			{
				float	rot= *(float*)(&value);
				valStr= toString("%d", sint32(rot*180/Pi));
			}


			// display log
			nldebugraw("** Entity %d: (gc=%3d,t=%3d) %s: %s %s", (sint32)who, relGameCycle, (sint32)relLocalTime, reason.c_str(), CLFECOMMON::getPropShortText(propLoged[i]), valStr.c_str());
		}
	}
}


// ***************************************************************************
void	CEntityManager::stopLogStageChange()
{
	_LogStageChange.Enabled= false;
	nldebugraw("*** Stop Loging Stage changes");
}

// ***************************************************************************
bool	CEntityManager::isLogingStageChange() const
{
	return _LogStageChange.Enabled;
}

// ***************************************************************************
sint32	CEntityManager::getLogStageChangeStartCycle() const
{
	if(isLogingStageChange())
		return _LogStageChange.StartGameCycle;
	else
		return 0;
}

// ***************************************************************************
sint64	CEntityManager::getLogStageChangeStartLocalTime() const
{
	if(isLogingStageChange())
		return _LogStageChange.StartLocalTime;
	else
		return 0;
}

// ***************************************************************************
void	CEntityManager::refreshInsceneInterfaceOfFriendNPC(uint slot)
{
	CCharacterCL *entity = dynamic_cast<CCharacterCL*>(_Entities[slot]);
	if (!entity)
		return;
	if (entity->canHaveMissionIcon()
		&& entity->isFriend() // only valid once the Contextual property is received
		)
	{
		entity->releaseInSceneInterfaces();
		entity->buildInSceneInterface();
	}
}
