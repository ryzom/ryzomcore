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



#include "nel/misc/path.h"
#include "nel/misc/matrix.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/sheet_id.h"

#include "nel/pacs/u_primitive_block.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_global_position.h"

#include "nel/3d/u_scene.h"
#include "nel/3d/u_instance.h"


#include "ig_callback.h"
#include "fix_season_data.h"
#include "sheet_manager.h"
#include "ig_enum.h"
#include "weather.h"
#include "pacs_client.h"
// Client Sheets
#include "client_sheets/plant_sheet.h"
#include <memory>


#define RZ_PRIM_ZEXT_BLOCK_AVOIDANCE	2.0f


H_AUTO_DECL(RZ_IGCallback)

///===================================================================================
CIGCallback::CIGCallback() : _MoveContainer(NULL)
{
	H_AUTO_USE(RZ_IGCallback)
	//nlinfo("**** YOYO: CREATING IG CALLBACK: %x", this);
}

///===================================================================================
CIGCallback::~CIGCallback()
{
	//nlinfo("**** YOYO: DELETING IG CALLBACK: %x", this);
	H_AUTO_USE(RZ_IGCallback)
	deleteIGs();
}

///===================================================================================
void CIGCallback::resetContainer()
{
	H_AUTO_USE(RZ_IGCallback)
	if (!_MoveContainer) return;
	deleteIGs();
	_MoveContainer = NULL;
}

///===================================================================================
void CIGCallback::setMoveContainer(NLPACS::UMoveContainer *mc)
{
	H_AUTO_USE(RZ_IGCallback)
	nlassert(!_MoveContainer);
	_MoveContainer = mc;
}

///===================================================================================
void CIGCallback::addIG(NL3D::UInstanceGroup *ig)
{
	H_AUTO_USE(RZ_IGCallback)
	nlassert(_MoveContainer);
	CIGInstance *igi;
	try
	{
		igi = new CIGInstance(ig, this);
		_IGInstances.push_back(igi);
		ig->setAddRemoveInstanceCallback(igi);
		ig->setTransformNameCallback(igi);
		ig->setIGAddBeginCallback(igi);
	}
	catch(...)
	{
		delete igi;
		throw;
	}
}

///===================================================================================
void CIGCallback::addIGWithNumZC(NL3D::UInstanceGroup *ig, sint numZC)
{
	H_AUTO_USE(RZ_IGCallback)
	// Check the ig is valid.
	if(ig == 0)
		return;

	nlassert(_MoveContainer);
	CIGInstance *igi;
	try
	{
		igi = new CIGInstance(ig, this);
		igi->numZC(numZC);
		_IGInstances.push_back(igi);
		ig->setAddRemoveInstanceCallback(igi);
		ig->setTransformNameCallback(igi);
		ig->setIGAddBeginCallback(igi);
	}
	catch(...)
	{
		delete igi;
		throw;
	}
}


///===================================================================================
void CIGCallback::addIGs(const std::vector<NL3D::UInstanceGroup *> &igs)
{
	H_AUTO_USE(RZ_IGCallback)
	_IGInstances.reserve(_IGInstances.size() + igs.size());
	for(uint k = 0; k < igs.size(); ++k)
	{
		addIG(igs[k]);
	}
}

//-----------------------------------------------
// addIGsWithNumZC :
// Add a vector of instance groups with the num ZC associated.
//-----------------------------------------------
void CIGCallback::addIGsWithNumZC(std::vector<std::pair<NL3D::UInstanceGroup *, sint> > &igs)
{
	H_AUTO_USE(RZ_IGCallback)
	_IGInstances.reserve(_IGInstances.size() + igs.size());
	for(uint k = 0; k < igs.size(); ++k)
	{
		addIGWithNumZC(igs[k].first, igs[k].second);
	}
}


///===================================================================================
CIGCallback::CIGInstance::CIGInstance(NL3D::UInstanceGroup *ig, CIGCallback	*owner)
	: _Owner(owner), _IG(ig), _HasManagedFXs(false)
{
	H_AUTO_USE(RZ_IGCallback)
	nlassert(owner);
	nlassert(ig);
}

///===================================================================================
CIGCallback::CIGInstance::~CIGInstance()
{
	H_AUTO_USE(RZ_IGCallback)
	releaseMovePrimitives();
	if (_HasManagedFXs)
	{
		CTimedFXManager::getInstance().remove(_ManagedFXHandle);
	}
	if (_IG)
	{
		_IG->setAddRemoveInstanceCallback(NULL);
		_IG->setTransformNameCallback(NULL);
		_IG->setIGAddBeginCallback(NULL);
	}
}

///===================================================================================
void CIGCallback::CIGInstance::instanceGroupAdded()
{
	H_AUTO_USE(RZ_IGCallback)
	// Check the pointer on the IG.
	nlassert(_IG);
	nlassert(_Owner);

	// See what objects need collisions primitives
	if (!_MovePrimitives.empty()) return; // already added
	std::vector<NLPACS::UMovePrimitive *> addedPrims;
	uint numInstances = _IG->getNumInstance();
	for(uint k = 0; k < numInstances; ++k)
	{
		TPacsPrimMap::iterator pbIt = PacsPrims.find(NLMISC::strlwr(NLMISC::CFile::getFilenameWithoutExtension(_IG->getShapeName(k))));
		if (pbIt != PacsPrims.end())
		{
			// compute orientation and position
			NLMISC::CMatrix instanceMatrix;
			_IG->getInstanceMatrix(k, instanceMatrix);
			NLMISC::CVector pos;
			float			angle;
			NLMISC::CVector scale = _IG->getInstanceScale(k);
			NLPACS::UMoveContainer::getPACSCoordsFromMatrix(pos, angle, instanceMatrix);
			// insert the matching primitive block
			addedPrims.clear();
			_Owner->_MoveContainer->addCollisionnablePrimitiveBlock(pbIt->second, 0, 1, &addedPrims, angle, pos, true, scale);
			// Yoyo: For each primitive, increment its height, to avoid blocking bugs
			for(uint i=0;i<addedPrims.size();i++)
			{
				// do it only for obstacle, non trigerrable(?) prims
				if(addedPrims[i] && addedPrims[i]->getObstacle() && addedPrims[i]->getTriggerType()==NLPACS::UMovePrimitive::NotATrigger)
				{
					addedPrims[i]->setHeight(addedPrims[i]->getHeight() + RZ_PRIM_ZEXT_BLOCK_AVOIDANCE);
				}
			}
			// for future remove
			_MovePrimitives.insert(_MovePrimitives.end(), addedPrims.begin(), addedPrims.end());
		}
	}
	// update additionnal datas from sheets
	updateFromSheets();
	updateManagedFXs();
	// free mem for sheet pointers
	NLMISC::contReset(_EntitySheets);
	_Owner->notifyIGAdded(_IG);
}

///===================================================================================
void CIGCallback::CIGInstance::instanceGroupRemoved()
{
	H_AUTO_USE(RZ_IGCallback)
	// If this zone is a ZC.
	releaseMovePrimitives();
	if (_HasManagedFXs)
	{
		CTimedFXManager::getInstance().remove(_ManagedFXHandle);
		_HasManagedFXs = false;
	}
}


///===================================================================================
void CIGCallback::CIGInstance::releaseMovePrimitives()
{
	H_AUTO_USE(RZ_IGCallback)
	nlassert(_Owner);
	// remove all primitives from the move container
	nlassert(_Owner->_MoveContainer);
	for(TMovePrimitiveVect::iterator it = _MovePrimitives.begin(); it != _MovePrimitives.end(); ++it)
	{
		_Owner->_MoveContainer->removePrimitive(*it);
	}
	NLMISC::contReset(_MovePrimitives);
}

///===================================================================================
void CIGCallback::deleteIGs()
{
	H_AUTO_USE(RZ_IGCallback)
	for(TIGInstanceList::iterator it = _IGInstances.begin(); it != _IGInstances.end(); ++it)
	{
		delete *it;
	}
	_IGInstances.clear();
}

///===================================================================================
void CIGCallback::forceAddAll()
{
	H_AUTO_USE(RZ_IGCallback)
	for(TIGInstanceList::iterator it = _IGInstances.begin(); it != _IGInstances.end(); ++it)
	{
		(*it)->forceAdd();
	}
}

///===================================================================================
void CIGCallback::CIGInstance::startAddingIG(uint numInstances)
{
	H_AUTO_USE(RZ_IGCallback)
	// make room for sheets ptr
	_EntitySheets.resize(numInstances);
	std::fill(_EntitySheets.begin(), _EntitySheets.end(), (CEntitySheet *) NULL);
}

///===================================================================================
void CIGCallback::CIGInstance::buildSheetVector()
{
	H_AUTO_USE(RZ_IGCallback)
	uint numInstances = _IG->getNumInstance();
	_EntitySheets.resize(numInstances);
	for(uint k = 0; k < numInstances; ++k)
	{
		_EntitySheets[k] = NULL;
		const std::string &name = _IG->getInstanceName(k);
		if (NLMISC::nlstricmp(NLMISC::CFile::getExtension(name), "plant") == 0)
		{
			NLMISC::CSheetId sheetId;
			if (sheetId.buildSheetId(name))
			{
				_EntitySheets[k] = SheetMngr.get(sheetId);
			}
		}
	}
}

///===================================================================================
void CIGCallback::CIGInstance::eraseSheetVector()
{
	H_AUTO_USE(RZ_IGCallback)
	NLMISC::contReset(_EntitySheets);
}

///===================================================================================
std::string CIGCallback::CIGInstance::transformName(uint instanceIndex, const std::string &instanceName, const std::string &shapeName)
{
	H_AUTO_USE(RZ_IGCallback)
	/** we look if there'is a matching form for this instance name
	  * If this is the case we can choose the shape we want to instanciate
	  */

	string ext = NLMISC::CFile::getExtension(instanceName);
	if (NLMISC::nlstricmp(ext, "pacs_prim") == 0)
	{
		return "";		// Don't instanciate pacs_prim
	}
	if (NLMISC::nlstricmp(ext, "plant") != 0)
	{
		return shapeName; // if there's no attached sheet, use the shape name
	}
	// We cache the last id
	static std::string lastName;
	static CEntitySheet *lastSheet = NULL;
	CEntitySheet *sh;
	if (instanceName == lastName)
	{
		sh = lastSheet;
	}
	else
	{
		NLMISC::CSheetId sheetId;
		if (sheetId.buildSheetId(instanceName))
		{
			sh = SheetMngr.get(sheetId);
			lastSheet = sh;
			lastName = instanceName;
		}
		else
		{
			return shapeName;
		}
	}
	if (!sh) return shapeName;
	if (sh->type() == CEntitySheet::PLANT)
	{
		// store sheet in the sheet list
		_EntitySheets[instanceIndex] = sh;
		return ((CPlantSheet *) sh)->getShapeName();
	}
	else
	{
		return shapeName;
	}
}

///===================================================================================
void CIGCallback::CIGInstance::updateFromSheets()
{
	H_AUTO_USE(RZ_IGCallback)
	nlassert(_EntitySheets.size() == _IG->getNumInstance());
	// See for which objects distance should be overriden (object which use a .PLANT sheet)
	uint numInstances = (uint)_EntitySheets.size();
	for(uint k = 0; k < numInstances; ++k)
	{
		if (_EntitySheets[k] && _EntitySheets[k]->Type == CEntitySheet::PLANT)
		{
			CPlantSheet *ps = NLMISC::safe_cast<CPlantSheet *>(_EntitySheets[k]);
			if (ps->getMaxDist() != -1) _IG->setDistMax(k, ps->getMaxDist());
			if (ps->getCoarseMeshDist() != -1) _IG->setCoarseMeshDist(k, ps->getCoarseMeshDist());
		}
	}
}

///===================================================================================
void CIGCallback::CIGInstance::shutDownFXs()
{
	H_AUTO_USE(RZ_IGCallback)
	if (!_HasManagedFXs) return;
	CTimedFXManager::getInstance().shutDown(_ManagedFXHandle);
	_HasManagedFXs = false;
}

///===================================================================================
void CIGCallback::CIGInstance::updateManagedFXs()
{
	H_AUTO_USE(RZ_IGCallback)
	nlassert(_EntitySheets.size() == _IG->getNumInstance());
	// See for which objects distance should be overriden (object which use a .PLANT sheet)
	uint numInstances = (uint)_EntitySheets.size();
	// vector of fx that should be managed by the dedicated manager. static for malloc perf
	static std::vector<CTimedFX> timedFXs;
	timedFXs.clear();
	for(uint k = 0; k < numInstances; ++k)
	{
		if (_EntitySheets[k] && _EntitySheets[k]->Type == CEntitySheet::PLANT)
		{
			CPlantSheet *ps = NLMISC::safe_cast<CPlantSheet *>(_EntitySheets[k]);
			// check for managed fxs for the current season
			if (CurrSeason < EGSPD::CSeason::Invalid)
			{
				if (!ps->getFXSheet(CurrSeason).FXName.empty())
				{
					timedFXs.push_back(CTimedFX());
					timedFXs.back().SpawnPosition = _IG->getInstancePos(k);
					timedFXs.back().Rot = _IG->getInstanceRot(k);
					timedFXs.back().Scale = _IG->getInstanceScale(k);
					timedFXs.back().FXSheet = &ps->getFXSheet(CurrSeason);
				}
			}
		}
	}
	if (!timedFXs.empty())
	{
		_ManagedFXHandle = CTimedFXManager::getInstance().add(timedFXs, CurrSeason);
		_HasManagedFXs = true;
	}
}



///===================================================================================
bool CIGCallback::enumIGs(IIGEnum *callback)
{
	H_AUTO_USE(RZ_IGCallback)
	nlassert(callback);
	for(TIGInstanceList::iterator it = _IGInstances.begin(); it != _IGInstances.end(); ++it)
	{
		if ((*it)->getIG() != NULL && (*it)->getIG() != (NL3D::UInstanceGroup *)-1)
		{
			bool res = callback->enumIG((*it)->getIG());
			if (!res) return false;
		}
	}
	return true;
}

///===================================================================================
void CIGCallback::changeSeason()
{
	H_AUTO_USE(RZ_IGCallback)
	// for now, this only update managed fxs so that they are displayed the same way on both clients
	for(TIGInstanceList::iterator it = _IGInstances.begin(); it != _IGInstances.end(); ++it)
	{
		if ((*it)->getIG() != NULL && (*it)->getIG() != (NL3D::UInstanceGroup *)-1)
		{
			(*it)->buildSheetVector(); // the sheet vector is deleted after use, so need to rebuild it
			(*it)->shutDownFXs();
			(*it)->updateManagedFXs();
			(*it)->eraseSheetVector();
		}
	}
}

///===================================================================================
///===================================================================================


void createInstancesFromMoveContainer(NL3D::UScene *scene, NLPACS::UMoveContainer *mc, std::vector<NL3D::UInstance> *instances /*=NULL*/)
{
	H_AUTO_USE(RZ_IGCallback)
	nlassert(scene);
	nlassert(mc);
	mc->evalCollision(0.1f, 0);
	// get the primitives
	std::vector<const NLPACS::UMovePrimitive *> prims;
	mc->getPrimitives(prims);
	if (instances)
	{
		instances->reserve(prims.size());
	}
	for(uint k = 0; k < prims.size(); ++k)
	{
		NL3D::UInstance newInstance;
		float angle = 0.f;
		NLMISC::CVector scale;
		switch(prims[k]->getPrimitiveType())
		{
			case NLPACS::UMovePrimitive::_2DOrientedBox:
				newInstance = scene->createInstance("unit_box.shape");
				angle = (float) prims[k]->getOrientation(prims[k]->getFirstWorldImageV());
				prims[k]->getSize(scale.x, scale.y);
			break;
			case NLPACS::UMovePrimitive::_2DOrientedCylinder:
				newInstance = scene->createInstance("unit_cylinder.shape");
				scale.x = scale.y = prims[k]->getRadius();
			break;
			default:
				nlwarning("createInstancesFromMoveContainer : unsupported type encountered");
				continue;
			break;
		}
		if (!newInstance.empty())
		{
			scale.z = prims[k]->getHeight();
			NLMISC::CVectorD worldPos = prims[k]->getFinalPosition(prims[k]->getFirstWorldImageV());
			newInstance.setPos(NLMISC::CVector((float) worldPos.x, (float) worldPos.y, (float) worldPos.z));
			newInstance.setScale(scale);
			newInstance.setRotQuat(NLMISC::CQuat(NLMISC::CVector::K, angle));
			if (instances)
			{
				instances->push_back(newInstance);
			}
		}
		else
		{
			nlwarning("createInstancesFromMoveContainer : couldn't create shape");
		}
	}
}


