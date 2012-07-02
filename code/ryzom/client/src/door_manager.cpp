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
#include <memory>
#include "door_manager.h"
#include "pacs_client.h"
#include "time_client.h"
#include "entity_cl.h"
#include "entities.h"

#include "nel/pacs/u_primitive_block.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_global_position.h"
#include "nel/pacs/u_collision_desc.h"

using namespace NLMISC;
using namespace NLPACS;
using namespace NL3D;

// GLOBALS

CIGDoorAddedCallback		IGDoorCallback;

CDoorManager *CDoorManager::_Instance = NULL;

extern CEntityManager		EntitiesMngr;

uint32 CDoorManager::s_nextId = 0;

// ***************************************************************************
void CDoorManager::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

// ***************************************************************************
// SDoor
// ***************************************************************************

#define TIME_OPEN_CLOSE_DOOR 0.7f
#define SIZE_DOOR 3.0f
#define SIZE_DOOR_3PARTS_LR 1.0f
#define SIZE_DOOR_3PARTS_DN 1.5f
#define SIZE_DOOR_2PARTS_LR 1.0f

// ***************************************************************************
void CDoorManager::SDoor::anim()
{
	switch(AnimType)
	{
		case Matis3Part:
		case Matis3PartBourgeon:
		{
			if (Instances[0] != 0xFFFFFFFF) // Left Part
			{
				UInstance inst = InstanceGroup->getInstance(Instances[0]);
				if (!inst.empty())
				{
					CMatrix mat = InstMat[0];
					if (AnimType == Matis3Part)
						mat.setPos (mat.getPos()+ OCState*mat.getK()*SIZE_DOOR_3PARTS_LR
												+ OCState*mat.getJ()*SIZE_DOOR_3PARTS_LR);
					else
						mat.setPos (mat.getPos()+ OCState*mat.getK()*SIZE_DOOR_3PARTS_LR
												- OCState*mat.getI()*SIZE_DOOR_3PARTS_LR);
					inst.setMatrix(mat);
				}
			}

			if (Instances[1] != 0xFFFFFFFF) // Right Part
			{
				UInstance inst = InstanceGroup->getInstance(Instances[1]);
				if (!inst.empty())
				{
					CMatrix mat = InstMat[1];
					if (AnimType == Matis3Part)
						mat.setPos (mat.getPos()+ OCState*mat.getK()*SIZE_DOOR_3PARTS_LR
												- OCState*mat.getJ()*SIZE_DOOR_3PARTS_LR);
					else
						mat.setPos (mat.getPos()+ OCState*mat.getK()*SIZE_DOOR_3PARTS_LR
												+ OCState*mat.getI()*SIZE_DOOR_3PARTS_LR);
					inst.setMatrix(mat);
				}
			}

			if (Instances[2] != 0xFFFFFFFF) // Down Part
			{
				UInstance inst = InstanceGroup->getInstance(Instances[2]);
				if (!inst.empty())
				{
					CMatrix mat = InstMat[2];
					mat.setPos (mat.getPos() - OCState*mat.getK()*SIZE_DOOR_3PARTS_DN);
					inst.setMatrix(mat);
				}
			}
		}
		break;

		case Zorai2Part:
		{
			if (Instances[0] != 0xFFFFFFFF) // Left Part
			{
				UInstance inst = InstanceGroup->getInstance(Instances[0]);
				if (!inst.empty())
				{
					CMatrix mat = InstMat[0];
					mat.setPos (mat.getPos()- OCState*mat.getI()*SIZE_DOOR_2PARTS_LR);
					inst.setMatrix(mat);
				}
			}

			if (Instances[1] != 0xFFFFFFFF) // Right Part
			{
				UInstance inst = InstanceGroup->getInstance(Instances[1]);
				if (!inst.empty())
				{
					CMatrix mat = InstMat[1];
					mat.setPos (mat.getPos()+ OCState*mat.getI()*SIZE_DOOR_2PARTS_LR);
					inst.setMatrix(mat);
				}
			}
		}
		break;

		case Normal:
		default:
		{
			for (uint i = 0; i < Instances.size(); ++i)
			{
				UInstance inst = InstanceGroup->getInstance(Instances[i]);
				if (!inst.empty())
				{
					CMatrix mat = InstMat[i];
					mat.setPos(mat.getPos()+OCState*mat.getK()*SIZE_DOOR);
					inst.setMatrix(mat);
				}
			}
		}
		break;
	}
}

// ***************************************************************************
bool CDoorManager::SDoor::open()
{
	if (OCState >= 1.0f) return true;

	if (!Opened)
	{
		InstanceGroup->setDynamicPortal (Name, true);
		Opened = true;
	}

	OCState += DT / TIME_OPEN_CLOSE_DOOR;
	if (OCState > 1.0f) OCState = 1.0f;
	anim();

	return false;
}

// ***************************************************************************
bool CDoorManager::SDoor::close()
{
	if (OCState <= 0.0f) return true;

	OCState -= DT / TIME_OPEN_CLOSE_DOOR;
	if (OCState < 0.0f)
	{
		OCState = 0.0f;
		if (Opened)
		{
			InstanceGroup->setDynamicPortal (Name, false);
			Opened = false;
		}
	}
	anim();

	return false;
}

// ***************************************************************************
void CDoorManager::SDoor::entityCollide(CEntityCL *pE)
{
	for (uint i = 0; i < Entities.size(); ++i)
		if (Entities[i] == pE)
		{
			// The entity has moved in the trigger
			EntitiesMoved[i] = 1;
			return;
		}
	// New entity entered the trigger
	Entities.push_back(pE);
	EntitiesMoved.push_back(1);
}

// ***************************************************************************
void CDoorManager::SDoor::checkToClose()
{
	for (sint i = 0; i < (sint)Entities.size(); ++i)
	{
		if (EntitiesMoved[i] == 0) // No trigger moved
		{
			if (Entities[i]->hasMoved()) // But the entity has moved (so not in the trigger)
			{
				// The entity has leaved the trigger
				Entities.erase(Entities.begin()+i);
				EntitiesMoved.erase(EntitiesMoved.begin()+i);
				i--;
			}
		}
		else // Trigger move ok lets reset for next update
		{
			EntitiesMoved[i] = EntitiesMoved[i] - 1;
		}
	}

	if (Entities.size() == 0)
		close();
	else
		open();
}


// ***************************************************************************
// CDoorManager
// ***************************************************************************

// ***************************************************************************
std::string CDoorManager::transformName (uint /* index */, const std::string &/* instanceName */, const std::string &shapeName)
{
	if (shapeName.rfind('.') == string::npos) return shapeName;
	string sExt = shapeName.substr(shapeName.rfind('.')+1,shapeName.size());
	sExt = strlwr(sExt);
	if (sExt != "pacs_prim") return shapeName;
	return ""; // Do not load a pacs prim as a mesh...
}

// ***************************************************************************
void CDoorManager::loadedCallback (NL3D::UInstanceGroup *ig)
{
	uint i;
	// Check all instances and look if there are some pacs_prim to instanciate
	uint numInstances = ig->getNumInstance();
	for(uint k = 0; k < numInstances; ++k)
	{
		// Check if the shape name is a pacs_prim
		string sShapeName = ig->getShapeName(k);
		if (sShapeName.rfind('.') == string::npos) continue;

		string sExt = sShapeName.substr(sShapeName.rfind('.')+1,sShapeName.size());
		sExt = strlwr(sExt);
		if (sExt != "pacs_prim") continue;

		// Check if the pacs_prim is a door detection

		sShapeName = CPath::lookup(sShapeName,false);
		if (!sShapeName.empty())
		{
			std::auto_ptr<NLPACS::UPrimitiveBlock> pb(NLPACS::UPrimitiveBlock::createPrimitiveBlockFromFile(sShapeName));
			NLPACS::UPrimitiveBlock *pPB = pb.release();

			bool bDoorDetectPresent = false;
			for (i = 0; i < pPB->getNbPrimitive(); ++i)
			{
				if ((pPB->getUserData(i) & 0xffff) == UserDataDoor)
				{
					bDoorDetectPresent = true;
					break;
				}
			}

			if (!bDoorDetectPresent)
			{
				delete pPB;
				continue;
			}

			// Instanciate the pacs_prim and create a door structure associated
			SDoor *pDoor = new SDoor;
			pDoor->InstanceGroup = ig;
			// compute orientation and position
			NLMISC::CMatrix instanceMatrix;
			ig->getInstanceMatrix(k, instanceMatrix);
			NLMISC::CVector pos;
			float			angle;
			NLMISC::CVector scale = ig->getInstanceScale(k);
			NLPACS::UMoveContainer::getPACSCoordsFromMatrix(pos, angle, instanceMatrix);
			PACS->addCollisionnablePrimitiveBlock(pPB, 0, 1, &pDoor->Prims, angle, pos, true, scale);

			// Complete the user data of all the 'door primitives' with a pointer to the door structure
			for (i = 0; i < pDoor->Prims.size(); ++i)
			{
				UMovePrimitive *pPrim = pDoor->Prims[i];
				if ((pPrim->UserData&0xffff) == UserDataDoor)
				{
					// First byte is for type (2 for door 1 for ascensor)
					pPrim->UserData |= ((uint64)pDoor->ID << 16);
				}
			}

			// Link with all door 3d objects (depending on the structure of the door)
			pDoor->Name = ig->getInstanceName(k);
			pDoor->AnimType = SDoor::Normal;

			if (strnicmp(pDoor->Name.c_str(),"ma_asc_3portes_bourgeons",24)==0)
			{
				pDoor->AnimType = SDoor::Matis3PartBourgeon;
				pDoor->Instances.resize(3, 0xFFFFFFFF);
			}
			else if (strnicmp(pDoor->Name.c_str(),"ma_asc_3portes",14)==0)
			{
				pDoor->AnimType = SDoor::Matis3Part;
				pDoor->Instances.resize(3, 0xFFFFFFFF);
			}
			else if (strnicmp(pDoor->Name.c_str(),"zo_asc_2portes",14)==0)
			{
				pDoor->AnimType = SDoor::Zorai2Part;
				pDoor->Instances.resize(2, 0xFFFFFFFF);
			}

			for(i = 0; i < numInstances; ++i)
			if (i != k)
			{
				string sInstName = ig->getInstanceName(i);
				if (sInstName == pDoor->Name)
				{
					switch (pDoor->AnimType)
					{
						case SDoor::Matis3Part:
						case SDoor::Matis3PartBourgeon:
						{
							string sDebug = ig->getShapeName(i);
							sDebug = strlwr(sDebug.substr(sDebug.rfind('_')+1,sDebug.size()));
							if (sDebug == "gauche")
								pDoor->Instances[0] = i;
							else if (sDebug == "droite")
								pDoor->Instances[1] = i;
							else if (sDebug == "bas")
								pDoor->Instances[2] = i;
						}
						break;

						case SDoor::Zorai2Part:
						{
							string sDebug = ig->getShapeName(i);
							sDebug = strlwr(sDebug.substr(sDebug.rfind('_')+1,sDebug.size()));
							if (sDebug == "gauche")
								pDoor->Instances[0] = i;
							else if (sDebug == "droite")
								pDoor->Instances[1] = i;
						}
						break;

						case SDoor::Normal:
						default:
						{
							string sDebug = ig->getShapeName(i);
							pDoor->Instances.push_back(i);
						}
						break;
					}
				}
			}

			bool bAllInit = true;
			for (i = 0; i < pDoor->Instances.size(); ++i)
				if (pDoor->Instances[i] == 0xFFFFFFFF)
					bAllInit = false;

			if (!bAllInit)
			{
				nlwarning("All the door part are not well initialized");
				for (sint j = 0; j < (sint)pDoor->Prims.size(); ++j)
				{
					if (PACS != NULL)
						PACS->removePrimitive(pDoor->Prims[j]);
					else
						nlwarning("PACS should not be NULL at this point");
				}
				delete pDoor;
			}
			else
			{
				// Get matrices
				pDoor->InstMat.resize(pDoor->Instances.size());
				for (i = 0; i < pDoor->Instances.size(); ++i)
				{
					CMatrix mat;
					ig->getInstanceMatrix (pDoor->Instances[i], mat);
					pDoor->InstMat[i] = mat;
				}

				// Close the door/portal by default
				pDoor->Opened = false;
				ig->setDynamicPortal(pDoor->Name, false);

				// Add the door to the door manager
				_Doors.push_back(pDoor);
				ig->setTransformNameCallback(this);
			}
		}
	}
}

// ***************************************************************************
void CDoorManager::addedCallback (NL3D::UInstanceGroup *ig)
{
	sint i, j;
	for (i = 0; i < (sint)_Doors.size(); ++i)
	{
		SDoor *pDoor = _Doors[i];
		if (pDoor->InstanceGroup == ig)
		{
			for (j = 0; j < (sint)pDoor->Instances.size(); ++j)
			{
				UInstance inst = pDoor->InstanceGroup->getInstance(pDoor->Instances[j]);
				if (!inst.empty())
				{
					inst.unfreezeHRC();
					inst.setTransformMode(UInstance::DirectMatrix);
				}
			}
		}
	}
}

// ***************************************************************************
void CDoorManager::removedCallback (NL3D::UInstanceGroup *ig)
{
	// Remove all doors corresponding to the instance group ig
	sint i, j;
	for (i = 0; i < (sint)_Doors.size(); ++i)
	{
		SDoor *pDoor = _Doors[i];
		if (pDoor->InstanceGroup == ig)
		{
			// Remove Pacs
			for (j = 0; j < (sint)pDoor->Prims.size(); ++j)
			{
				if (PACS != NULL)
					PACS->removePrimitive(pDoor->Prims[j]);
				else
					nlwarning("PACS should not be NULL at this point");
			}
			delete pDoor;
			_Doors.erase(_Doors.begin()+i);
			i--;
		}
	}
}

// Copy triggers to be used in update
// ***************************************************************************
void CDoorManager::getPACSTriggers()
{
	uint nNbTrig = PACS->getNumTriggerInfo();
	for (uint i = 0; i < nNbTrig; ++i)
	{
		const UTriggerInfo &rTI = PACS->getTriggerInfo(i);
		// Does one of the 2 objects is a door detection object
		if (((rTI.Object0 & 0xffff) == UserDataDoor) || ((rTI.Object1 & 0xffff) == UserDataDoor))
		{
			uint64 nUserDataDoor = 0;
			uint64 nUserDataEntity = 0;
			if ((rTI.Object0 & 0xffff) == UserDataDoor)
			{
				nUserDataDoor = rTI.Object0;
				nUserDataEntity = rTI.Object1;
			}

			if ((rTI.Object1 & 0xffff) == UserDataDoor)
			{
				nUserDataDoor = rTI.Object1;
				nUserDataEntity = rTI.Object0;
			}

			if (rTI.CollisionType != UTriggerInfo::Inside)
				continue;

			// Retrieve the door pointer
			SDoor *pDoor = NULL;
			CEntityCL *pEntity = NULL;

			uint32 doorId = ((nUserDataDoor >> 16) & 0xffffffff);
			uint32 entityId = ((nUserDataEntity >> 16) & 0xffffffff);

			for(uint i = 0; i < _Doors.size(); ++i)
			{
				pDoor = _Doors[i];

				if (pDoor && pDoor->ID == doorId)
				{
					pEntity = EntitiesMngr.getEntityByCompressedIndex(entityId);

					if (pEntity)
						pDoor->entityCollide(pEntity);

					break;
				}
			}
		}
	}
}

// ***************************************************************************
void CDoorManager::update ()
{
	// Check all doors if we have to close
	for (uint i = 0; i < _Doors.size(); ++i)
	{
		SDoor *pDoor = _Doors[i];
		pDoor->checkToClose();
	}

}
