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
#include "ground_fx_manager.h"
#include "pacs_client.h"
#include "sheet_manager.h"
#include "misc.h"
#include "entities.h"
#include "user_entity.h"
#include "time_client.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_camera.h"
#include "nel/misc/vector.h"
#include "client_sheets/race_stats_sheet.h"
#include "client_sheets/character_sheet.h"
#include "client_sheets/ground_fx_sheet.h"
#include "continent_manager.h"
#include "network_connection.h"

#ifdef NL_DEBUG
	#define DEBUG_FX
#endif

#ifdef DEBUG_FX
	#define CHECK_INTEGRITY checkIntegrity();
#else
	#define CHECK_INTEGRITY
#endif


H_AUTO_DECL(RZ_GroundFXManager)

extern NL3D::UTextContext		*TextContext;
extern NL3D::UDriver			*Driver;
extern NL3D::UScene				*Scene;
extern CContinentManager		ContinentMngr;


// max dist to reuse an old fx
static const float MAX_DIST_TO_REUSE_OLD_FX = 1.5f;

using namespace NLMISC;

// *****************************************************************************
CGroundFXManager::CGroundFXManager() :
					   _MinSpeed(1.5f),
					   _MaxSpeed(6.f),
					   _SpeedWaterWalkFast(3.f),
					   _SpeedWaterSwimFast(2.f),
					   _MaxDist(50.f),
					   _MaxNumFX(10),
					   _NumFX(0),
					   _MaxNumCachedFX(10),
					   _NumCachedFX(0),
					   _NumInstances(0),
					   _Scene(NULL)
{
	H_AUTO_USE(RZ_GroundFXManager)
	// Construct
}

// *****************************************************************************
CGroundFXManager::~CGroundFXManager()
{
	H_AUTO_USE(RZ_GroundFXManager)
	reset();
	CHECK_INTEGRITY
}

// *****************************************************************************
void CGroundFXManager::reset()
{
	H_AUTO_USE(RZ_GroundFXManager)
	CHECK_INTEGRITY
	if (_Scene)
	{
		TGroundFXList::iterator it;
		// release all active FXs
		for(it = _ActiveFXs.begin(); it != _ActiveFXs.end(); ++it)
		{
			if (!it->FX.empty()) _Scene->deleteInstance(it->FX);
			if (!it->FXUnderWater.empty()) _Scene->deleteInstance(it->FXUnderWater);
		}
		// release all inactive FXs
		for(it = _InactiveFXs.begin(); it != _InactiveFXs.end(); ++it)
		{
			if (!it->FX.empty()) _Scene->deleteInstance(it->FX);
			if (!it->FXUnderWater.empty()) _Scene->deleteInstance(it->FXUnderWater);

		}
		// release all cached FXs
		for(it = _CachedFXs.begin(); it != _CachedFXs.end(); ++it)
		{
			if (!it->FX.empty()) _Scene->deleteInstance(it->FX);
			if (!it->FXUnderWater.empty()) _Scene->deleteInstance(it->FXUnderWater);
		}
	}
	_ActiveFXs.clear();
	_InactiveFXs.clear();
	_CachedFXs.clear();
	_InstancesList.clear();
	_SortedInstances.clear();
	_NumFX = 0;
	_NumCachedFX = 0;
	_NumInstances = 0;
	_Scene = NULL;
}

// *****************************************************************************
void CGroundFXManager::init(NL3D::UScene *scene, float maxDist, uint maxNumFX, uint fxCacheSize)
{
	H_AUTO_USE(RZ_GroundFXManager)
	nlassert(_Scene == NULL); // init already called
	_Scene = scene;
	_MaxDist = maxDist;
	_MaxNumFX = maxNumFX;
	_MaxNumCachedFX = fxCacheSize;
}

// *****************************************************************************
CGroundFXManager::TEntityHandle CGroundFXManager::add(CEntityCL *entity)
{
	H_AUTO_USE(RZ_GroundFXManager)
	CHECK_INTEGRITY
	nlassert(entity);
	#ifdef NL_DEBUG
		for(TInstanceList::iterator it = _InstancesList.begin(); it != _InstancesList.end(); ++it)
		{
			if (it->Entity == entity)
			{
				nlerror("entity added twice"); //
			}
		}
	#endif
	_InstancesList.push_front(CInstance());
	CInstance &i = _InstancesList.front();
	i.Entity = entity;
	i.HasFX  = false;
	i.EmittersActive = false;
	i.EmittersUnderWaterActive = false;
	i.InstanciateDelay = 2;
	++ _NumInstances;
	CHECK_INTEGRITY
	return _InstancesList.begin();
}

// *****************************************************************************
void CGroundFXManager::remove(TEntityHandle handle)
{
	H_AUTO_USE(RZ_GroundFXManager)
	CHECK_INTEGRITY
	if (handle->HasFX)
	{
		// shutdown fx
		if (!handle->FXHandle->FX.empty())
		{
			handle->FXHandle->FX.activateEmitters(false);
			handle->FXHandle->FX.stopSound();
		}
		if (!handle->FXHandle->FXUnderWater.empty())
		{
			handle->FXHandle->FXUnderWater.activateEmitters(false);
			handle->FXHandle->FXUnderWater.stopSound();
		}
		// put in inactive list
		_InactiveFXs.splice(_InactiveFXs.begin(), _ActiveFXs, handle->FXHandle);
		nlassert(_NumFX > 0);
		-- _NumFX;
		handle->EmittersActive = false;
		handle->EmittersUnderWaterActive = false;
	}
	_InstancesList.erase(handle);
	nlassert(_NumInstances != 0);
	-- _NumInstances;
	CHECK_INTEGRITY
}

// predicate to test a ground id

// *****************************************************************************
// Predicate for binary search in a vector of sorted ground fx sheets
struct CCmpGroundIDPred
{
	inline bool operator()(const CGroundFXSheet &lhs, const CGroundFXSheet &rhs) const
	{
		return lhs.GroundID < rhs.GroundID;
	}
};

// *****************************************************************************
void	CGroundFXManager::CInstance::getFXNameFromGroundType(uint32 groundID, std::string &fxName) const
{
	H_AUTO_USE(RZ_GroundFXManager)
	fxName= "";
	if (!Entity) return;
	const CEntitySheet *es;
	es = SheetMngr.get(Entity->sheetId());
	if (!es) return;
	const std::vector<CGroundFXSheet> *gfx = Entity->getGroundFX();
	if (!gfx) return;
	CGroundFXSheet tmp;
	tmp.GroundID = groundID;
	std::vector<CGroundFXSheet>::const_iterator it = std::lower_bound(gfx->begin(), gfx->end(), tmp, CCmpGroundIDPred());
	if (it ==gfx->end()) return;
	if (it->GroundID != groundID) return;
	fxName= it->getFXName();
}

// *****************************************************************************
/** Predicate to sort instances by distances.
  */
struct CSortInstancePred
{
	bool operator()(CGroundFXManager::TEntityHandle lhs, CGroundFXManager::TEntityHandle rhs) const
	{
		return lhs->Dist2 < rhs->Dist2;
	}
};

// *****************************************************************************
void CGroundFXManager::invalidateFX(TEntityHandle instance)
{
	H_AUTO_USE(RZ_GroundFXManager)
	bool present = false;
	if (!instance->FXHandle->FX.empty())
	{
		present = instance->FXHandle->FX.isSystemPresent();
		instance->FXHandle->FX.activateEmitters(false);
		instance->FXHandle->FX.stopSound();
	}
	if (!instance->FXHandle->FXUnderWater.empty())
	{
		present &= instance->FXHandle->FXUnderWater.isSystemPresent();
		instance->FXHandle->FXUnderWater.activateEmitters(false);
		instance->FXHandle->FXUnderWater.stopSound();
	}
	//
	instance->EmittersActive = false;
	instance->EmittersUnderWaterActive = false;
	instance->HasFX = false;
	if (present)
	{
		// put in inactive list
		_InactiveFXs.splice(_InactiveFXs.begin(), _ActiveFXs, instance->FXHandle);
		nlassert(_NumFX > 0);
		-- _NumFX;
	}
	else
	{
		// directly put in cache without (fx already shut down)
		moveFXInCache(_ActiveFXs, instance->FXHandle);
		-- _NumFX;
	}

}

// *****************************************************************************
void CGroundFXManager::moveFXInCache(TGroundFXList &ownerList, TGroundFXHandle fx)
{
	H_AUTO_USE(RZ_GroundFXManager)
	if (!fx->FX.empty()) fx->FX.hide();
	// cache full ?
	nlassert(_NumCachedFX <= _MaxNumCachedFX);
	if (_NumCachedFX == _MaxNumCachedFX)
	{
		// remove least recently added fx
		if (!_CachedFXs.back().FX.empty()) _Scene->deleteInstance(_CachedFXs.back().FX);
		if (!_CachedFXs.back().FXUnderWater.empty()) _Scene->deleteInstance(_CachedFXs.back().FXUnderWater);
		_CachedFXs.pop_back();
	}
	else
	{
		++ _NumCachedFX;
	}
	// move in cache front
	_CachedFXs.splice(_CachedFXs.begin(), ownerList, fx);
}


// *******************************************************************************************
void CGroundFXManager::checkIntegrity()
{
	H_AUTO_USE(RZ_GroundFXManager)
	for(TGroundFXList::iterator it = _InactiveFXs.begin(); it != _InactiveFXs.end(); ++it)
	{
		if (!it->FX.empty()) nlassert(!it->FX.hasActiveEmitters());
		if (!it->FXUnderWater.empty()) nlassert(!it->FXUnderWater.hasActiveEmitters());
	}
	for(TGroundFXList::iterator it = _CachedFXs.begin(); it != _CachedFXs.end(); ++it)
	{
		if (!it->FX.empty())
		{
			nlassert(!it->FX.hasActiveEmitters());
			nlassert(!it->FX.hasParticles());
		}
		if (!it->FXUnderWater.empty())
		{
			nlassert(!it->FXUnderWater.hasActiveEmitters());
			nlassert(!it->FXUnderWater.hasParticles());
			if (it->FXUnderWater.hasParticles())
			{
				nlinfo(it->FXName.c_str());
			}
		}
	}
	uint numFX = (uint)_ActiveFXs.size();
	nlassert(numFX == _NumFX);
	nlassert(numFX <= _MaxNumFX);
	uint numCachedFX = (uint)_CachedFXs.size();
	nlassert(numCachedFX == _NumCachedFX);
	nlassert(numCachedFX <= _MaxNumCachedFX);
}


// *****************************************************************************
void CGroundFXManager::update(const NLMISC::CVectorD &camPos)
{
	H_AUTO_USE(RZ_GroundFXManager)
	if (!_Scene) return;
	CHECK_INTEGRITY
	// check all inactive fxs, and move in cache those that have no more particles
	TGroundFXList::iterator currIt, nextIt;
	for(TGroundFXList::iterator currIt = _InactiveFXs.begin(); currIt != _InactiveFXs.end();)
	{
		nextIt = currIt;
		++ nextIt;
		bool somethingVisible = false;
		if (!currIt->FX.empty() && currIt->FX.isSystemPresent() && currIt->FX.hasParticles())
		{
			somethingVisible = true;
		}
		if (!currIt->FXUnderWater.empty() && currIt->FXUnderWater.isSystemPresent() && currIt->FXUnderWater.hasParticles())
		{
			somethingVisible = true;
		}
		if (!somethingVisible)
		{
			moveFXInCache(_InactiveFXs, currIt);
		}
		currIt = nextIt;
	}
	float maxDist2 = _MaxDist * _MaxDist;
	//
	_SortedInstances.clear();
	_SortedInstances.reserve(_NumInstances);
	// check all candidates entities
	for(TInstanceList::iterator instanceIt = _InstancesList.begin(); instanceIt != _InstancesList.end(); ++instanceIt)
	{
		const NLMISC::CVectorD &entityPos = instanceIt->Entity->pos();
		if (instanceIt->InstanciateDelay != 0)
		{
			-- instanceIt->InstanciateDelay;
			continue;
		}
		if (instanceIt->Entity->getLastClip())
		{
			// entity is clipped
			// if a fx was attached, put it in the inactive list
			// When the entity is not visible, we can't update its ground fx :
			// As a matter of fact, snapToGround is not called at each frame for entities that are not visible, so this causes a
            // very noticeable shift on z for the ground fx when the entity enters the view pyramid.
			if (instanceIt->HasFX)
			{
				invalidateFX(instanceIt);
			}
		}
		else
		{
			// entity is not clipped
			instanceIt->Dist2 = (float) (entityPos - camPos).sqrnorm();
			if (instanceIt->Dist2 > maxDist2)
			{
				// entity not eligible for fx (too far), remove any attached fx
				if (instanceIt->HasFX)
				{
					invalidateFX(instanceIt);
				}
			}
			else
			{
				instanceIt->Mode = CInstance::Ground;
				// if entity is in water, fx can be generated even if the entity hasn't moved
				if (instanceIt->Entity->mode() == MBEHAV::SWIM || instanceIt->Entity->mode() == MBEHAV::MOUNT_SWIM)
				{
					instanceIt->Mode = CInstance::Swim;
					// retrieve water height
					CContinent *cont = ContinentMngr.cur();
					if (cont)
					{
						bool splashEnabled;
						if (cont->WaterMap.getWaterHeight(CVector2f((float) entityPos.x, (float) entityPos.y), instanceIt->WaterHeight, splashEnabled))
						{
							if (splashEnabled)
							{
								// put in candidate list, even if not moving
								_SortedInstances.push_back(instanceIt);
							}
							else
							{
								// there's water, but with no splashs
								instanceIt->Mode = CInstance::Ground;
							}
						}
						else
						{
							// water height not retrieved ? -> set ground mode
							instanceIt->Mode = CInstance::Ground;
						}
					}
				}
				else
				{
					// check if entity walking in water
					CContinent *cont = ContinentMngr.cur();
					if (cont)
					{
						bool splashEnabled;
						if (cont->WaterMap.getWaterHeight(CVector2f((float) entityPos.x, (float) entityPos.y), instanceIt->WaterHeight, splashEnabled))
						{
							if (splashEnabled)
							{
								// see if feet are underwater
								if (entityPos.z < instanceIt->WaterHeight)
								{
									instanceIt->Mode = CInstance::Water;
									// put in candidate list, even if not moving
									_SortedInstances.push_back(instanceIt);
								}
							}
						}
					}
				}

				if (instanceIt->Mode == CInstance::Ground)
				{
					// entity not in water
					if (instanceIt->Entity->getSpeed() >= _MinSpeed)
					{
						// put in sort list
						_SortedInstances.push_back(instanceIt);
					}
					else
					{
						if (instanceIt->HasFX)
						{
							invalidateFX(instanceIt);
						}
					}
				}
			}
		}
	}
	CHECK_INTEGRITY
	// sort instances by distance (number of chosen instances is likely to be a lot less than 200, so quick sort is ok)
	std::sort(_SortedInstances.begin(), _SortedInstances.end(), CSortInstancePred());
	uint numValidInstances = std::min(_MaxNumFX, uint(_SortedInstances.size()));
	uint k;
	for(k = 0; k < numValidInstances; ++k)
	{
		std::string fxName;
		std::string stdFXName; // standardized name for retrieval in cache
		std::string fxNameUnderWater;
		bool createNewFx = false;
		//
		float fxZ; // z at which the instance must be put
		static float fxZBias = 0.2f;
		//
		double speed = _SortedInstances[k]->Entity->getSpeed();
		//
		switch(_SortedInstances[k]->Mode)
		{
			case CInstance::Water:
				if (speed == 0.f) fxName = "StepSwimIdle.ps";
				else if (speed > _SpeedWaterWalkFast) fxName = "StepSwimRun.ps";
				else fxName = "StepSwimWalk.ps";
				fxZ = _SortedInstances[k]->WaterHeight;
			break;
			case CInstance::Swim:
				if (speed == 0.f) fxName = "StepSwimIdle.ps";
				else if (speed > _SpeedWaterSwimFast)
				{
					fxName = "StepSwimSpeed.ps";
					fxNameUnderWater = "StepSwimSpeedUnderWater.ps";
				}
				else
				{
					fxName = "StepSwim.ps";
					fxNameUnderWater = "StepSwimUnderWater.ps";
				}
				fxZ = _SortedInstances[k]->WaterHeight;
			break;
			case CInstance::Ground:
			{
				uint32 groundId = (uint32) _SortedInstances[k]->Entity->getGroundType();
				_SortedInstances[k]->getFXNameFromGroundType(groundId, fxName);
				fxZ = (float) _SortedInstances[k]->Entity->pos().z;
			}
			break;
			default:
				nlassert(0);
			break;
		}
		if (!fxName.empty())
		{
			stdFXName = NLMISC::strlwr(NLMISC::CFile::getFilenameWithoutExtension(fxName));
		}
		// is an fx already attached to the entity ?
		if (_SortedInstances[k]->HasFX)
		{
			if (_SortedInstances[k]->FXHandle->FXName == stdFXName)
			{
				// name is the same
				// ok, activate emitters
				if (!_SortedInstances[k]->EmittersActive)
				{
					// NB : we dont activate emitters has soon as the fx is allocated by an entity, because of the way the fx works.
					// As a matter of fact, if an object move from A to B, the fx may spawn several particles on [AB] in a single frame
					// So if the FX was previously used by another entity, a trail of particles may appear between the 2 entities when
					// fx is deallocated from first entity and allocated by the new one
					if (!_SortedInstances[k]->FXHandle->FX.empty())
					{
						if (_SortedInstances[k]->FXHandle->FX.isSystemPresent())
						{
							_SortedInstances[k]->FXHandle->FX.activateEmitters(true);
							_SortedInstances[k]->FXHandle->FX.reactivateSound();
							_SortedInstances[k]->EmittersActive = true;
						}
					}
				}
				// underwater fx
				if (!_SortedInstances[k]->EmittersUnderWaterActive)
				{
					// NB : we dont activate emitters has soon as the fx is allocated by an entity, because of the way the fx works.
					// As a matter of fact, if an object move from A to B, the fx may spawn several particles on [AB] in a single frame
					// So if the FX was previously used by another entity, a trail of particles may appear between the 2 entities when
					// fx is deallocated from first entity and allocated by the new one
					if (!_SortedInstances[k]->FXHandle->FXUnderWater.empty())
					{
						if (_SortedInstances[k]->FXHandle->FXUnderWater.isSystemPresent())
						{
							_SortedInstances[k]->FXHandle->FXUnderWater.activateEmitters(true);
							_SortedInstances[k]->FXHandle->FXUnderWater.reactivateSound();
							_SortedInstances[k]->EmittersUnderWaterActive = true;
						}
					}
				}
			}
			else
			{
				// name has changed
				// put in inactive list
				invalidateFX(_SortedInstances[k]);
				if (!fxName.empty())
				{
					// must create a new fx
					createNewFx = true;
				}
			}
		}
		else
		{
			if (!fxName.empty())
			{
				// must create a new fx
				createNewFx = true;
			}
		}

		if (createNewFx)
		{
			nlassert(!fxName.empty());
			bool instanciate = true; // should we instanciate a new fx ?
			if (!_InactiveFXs.empty())
			{
				// simple linear search should suffice
				for(TGroundFXList::iterator it = _InactiveFXs.begin(); it != _InactiveFXs.end(); ++it)
				{
					if (it->FXName == stdFXName)
					{
						// fx must be not too far because of transparancy sorting issues
						if ((_SortedInstances[k]->Entity->pos() - it->FX.getMatrix().getPos()).sqrnorm() < MAX_DIST_TO_REUSE_OLD_FX * MAX_DIST_TO_REUSE_OLD_FX)
						{
							// put fx in active list
							_ActiveFXs.splice(_ActiveFXs.begin(), _InactiveFXs, it);
							++ _NumFX;
							_SortedInstances[k]->HasFX = true;
							nlassert(!_SortedInstances[k]->EmittersActive);
							nlassert(!_SortedInstances[k]->EmittersUnderWaterActive);
							_SortedInstances[k]->FXHandle = _ActiveFXs.begin();
							instanciate = false;
							break;
						}
					}
				}
			}

			// Look for a matching fx in the cache.
			// We can't reuse inactive fx (those that are shuttingdown), because we wait for all particles to disappear, so that the bbox is null
			if (instanciate && !_CachedFXs.empty())
			{
				for(TGroundFXList::iterator it = _CachedFXs.begin(); it != _CachedFXs.end(); ++it)
				{
					if (it->FXName == stdFXName)
					{
						//
						if (!it->FX.empty()) it->FX.show();
						// put fx in active list
						_ActiveFXs.splice(_ActiveFXs.begin(), _CachedFXs, it);
						nlassert(_NumCachedFX > 0);
						-- _NumCachedFX;
						++ _NumFX;
						//
						_SortedInstances[k]->HasFX = true;
						nlassert(!_SortedInstances[k]->EmittersActive);
						nlassert(!_SortedInstances[k]->EmittersUnderWaterActive);
						_SortedInstances[k]->FXHandle = _ActiveFXs.begin();
						instanciate = false;
						break;
					}
				}
			}


			if (instanciate)
			{
				// must create new fx
				_ActiveFXs.push_front(CGroundFX());
				++ _NumFX;
				CGroundFX &gfx = _ActiveFXs.front();
				gfx.FX = NULL;
				gfx.FXUnderWater = NULL;
				NL3D::UInstance fxInstance = _Scene->createInstance(fxName);
				if (!fxInstance.empty())
				{
					gfx.FX.cast (fxInstance);
					if (gfx.FX.empty())
					{
						// not a particle system instance
						_Scene->deleteInstance(fxInstance);
					}
					else
					{
						if (_SortedInstances[k]->Mode != CInstance::Ground)
						{
							// add z-bias for fx in water
							gfx.FX.setZBias(-fxZBias);
						}
					}
				}
				if (!fxNameUnderWater.empty())
				{
					fxInstance = _Scene->createInstance(fxNameUnderWater);
					if (!fxInstance.empty())
					{
						gfx.FXUnderWater.cast (fxInstance);
						if (gfx.FXUnderWater.empty())
						{
							// not a particle system instance
							_Scene->deleteInstance(fxInstance);
						}
						else
						{
							gfx.FXUnderWater.setZBias(fxZBias);
						}
					}
				}
				gfx.FXName = stdFXName;
				_SortedInstances[k]->HasFX = true;
				nlassert(!_SortedInstances[k]->EmittersActive);
				nlassert(!_SortedInstances[k]->EmittersUnderWaterActive);
				_SortedInstances[k]->FXHandle = _ActiveFXs.begin();
				if (!gfx.FX.empty())
				{
					gfx.FX.setTransformMode(NL3D::UTransform::DirectMatrix);
					gfx.FX.setOrderingLayer(2);
				}
				if (!gfx.FXUnderWater.empty())
				{
					gfx.FXUnderWater.setTransformMode(NL3D::UTransform::DirectMatrix);
					gfx.FXUnderWater.setOrderingLayer(0);
				}
			}
		}
		// update Pos & matrix
		if (_SortedInstances[k]->HasFX && !_SortedInstances[k]->FXHandle->FX.empty())
		{
			NLMISC::CMatrix mat = _SortedInstances[k]->Entity->dirMatrix();
			const NLMISC::CVector &front = _SortedInstances[k]->Entity->front();
			mat.setRot(CVector::K ^ front, - front, CVector::K);
			const NLMISC::CVector &entityPos = _SortedInstances[k]->Entity->pos();
			mat.setPos(NLMISC::CVector(entityPos.x, entityPos.y, (float) fxZ));
			_SortedInstances[k]->FXHandle->FX.setMatrix(mat);
			_SortedInstances[k]->FXHandle->FX.setClusterSystem(_SortedInstances[k]->Entity->getClusterSystem());
			if (!_SortedInstances[k]->FXHandle->FXUnderWater.empty())
			{
				_SortedInstances[k]->FXHandle->FXUnderWater.setMatrix(mat);
				_SortedInstances[k]->FXHandle->FXUnderWater.setClusterSystem(_SortedInstances[k]->Entity->getClusterSystem());
			}
			if (_SortedInstances[k]->Mode == CInstance::Ground)
			{
				float intensity;
				if (_MinSpeed == _MaxSpeed)
				{
					intensity = 0.f;
				}
				else
				{
					intensity = (float) ((speed - _MinSpeed) / (_MaxSpeed - _MinSpeed));
				}
				clamp(intensity, 0.f, 1.f);
				_SortedInstances[k]->FXHandle->FX.setUserParam(0, intensity);
			}
			/*
			else
			{
				// adjust z-bias (tmp)
				if (_SortedInstances[k]->FXHandle->FXUnderWater) _SortedInstances[k]->FXHandle->FXUnderWater.setZBias(-fxZBias);
				if (_SortedInstances[k]->FXHandle->FX) _SortedInstances[k]->FXHandle->FX.setZBias(fxZBias);
			}*/
		}
	}
	// remove fx for instances that are too far / not taken in account
	for (; k < _SortedInstances.size(); ++k)
	{
		if (_SortedInstances[k]->HasFX)
		{
			invalidateFX(_SortedInstances[k]);
		}
	}
	CHECK_INTEGRITY
}

//////////////////////////////////
// tmp tmp : test of ground fxs //
//////////////////////////////////
#if !FINAL_VERSION

#include "interface_v3/interface_manager.h"

using NLMISC::toString;

CTestGroundFX TestGroundFX;

// ***********************************************************************************************
void CTestGroundFX::update()
{
	H_AUTO_USE(RZ_GroundFXManager)
	for(uint k = 0; k < Entities.size(); ++k)
	{
		if (MoveAll || Entities[k].Move)
		{
			Entities[k].Entity->snapToGround();
			NLMISC::CVectorD newPos = NLMISC::CVectorD(DT * Entities[k].Dir) + Entities[k].Entity->pos();
			NLMISC::CVectorD startToPos = newPos - NLMISC::CVectorD(Entities[k].StartPos);
			if (startToPos.norm() > 10.0)
			{
				newPos = NLMISC::CVectorD(Entities[k].StartPos) + 10.0 * startToPos.normed();
			}
			Entities[k].Entity->pacsPos(newPos);
			Entities[k].Duration -= DT;
			if (Entities[k].Duration <= 0.f)
			{
				if (MoveAll)
				{
					Entities[k].Move = true;
					Entities[k].Duration = 5.f;
					Entities[k].Dir.set(NLMISC::frand(1.f), NLMISC::frand(1.f), 0.f);
					Entities[k].Dir.normalize();
				}
				else
				{
					Entities[k].Move = false;
				}
			}
		}
	}
}



// ***********************************************************************************************
void CTestGroundFX::displayFXBoxes() const
{
	H_AUTO_USE(RZ_GroundFXManager)
	Driver->setViewMatrix(Scene->getCam().getMatrix().inverted());
	NL3D::CFrustum fr;
	Scene->getCam().getFrustum(fr.Left, fr.Right, fr.Bottom, fr.Top, fr.Near, fr.Far);
	fr.Perspective = true;
	Driver->setFrustum(fr);
	TextContext->setColor(CRGBA::Green);
	TextContext->setShaded(false);
	TextContext->setShadeOutline(false);
	TextContext->setFontSize(12);
	//
	float size = 0.4f;
	float textSize = 2.5f;
	// display fx to add
	for(uint k = 0; k < TestGroundFX.Entities.size(); ++k)
	{
		NLMISC::CMatrix mat;
		mat.identity();
		CVector pos = (CVector) TestGroundFX.Entities[k].Entity->pos();
		mat.setPos(pos);
		Driver->setModelMatrix(mat);
		drawBox(CVector(- size, - size, - size),
			    CVector(size, size, size),
				CRGBA::Magenta);
		mat.identity();
		mat.setRot(Scene->getCam().getRotQuat());
		mat.setPos(pos + 5.f * size * CVector::K);
		CVector distVect = pos - Scene->getCam().getPos();
		mat.scale(textSize * distVect.norm());
		TextContext->render3D(mat, NLMISC::toString("gfx %d, dist = %.1f", (int) k, (pos - Scene->getCam().getMatrix().getPos()).norm()));
	}
}


// *******************************************************************************************
void CGroundFXManager::setMinSpeed(float minSpeed)
{
	H_AUTO_USE(RZ_GroundFXManager)
	nlassert(minSpeed >= 0.f);
	_MinSpeed = minSpeed;
}

// *******************************************************************************************
void CGroundFXManager::setMaxSpeed(float maxSpeed)
{
	H_AUTO_USE(RZ_GroundFXManager)
	nlassert(maxSpeed >= 0.f);
	_MaxSpeed = maxSpeed;
}

// *******************************************************************************************
void CGroundFXManager::setSpeedWaterWalkFast(float speed)
{
	H_AUTO_USE(RZ_GroundFXManager)
	nlassert(speed >= 0.f);
	_SpeedWaterWalkFast = speed;
}

// *******************************************************************************************
void CGroundFXManager::setSpeedWaterSwimFast(float speed)
{
	H_AUTO_USE(RZ_GroundFXManager)
	nlassert(speed >= 0.f);
	_SpeedWaterSwimFast = speed;
}



// *******************************************************************************************
// temp, for debug

// add an entity for test
NLMISC_COMMAND(gfxAdd, "gfxAdd", "<>")
{
	uint slot;
	for(slot = 0; slot < EntitiesMngr.nbEntitiesAllocated(); ++slot)
	{
		if (EntitiesMngr.entity(slot) == NULL)
		{
			break;
		}
	}
	int category = 0;
	uint32 form;
	if (args.size() == 1)
	{
		if (fromString(args[0], category))
		{
			switch(category)
			{
				case 1:
				{
					NLMISC::CSheetId sheet("dag_for_lvl_01.creature");
					form = sheet.asInt();
				}
				break;
				default:
				{
					NLMISC::CSheetId sheet("fyros.race_stats");
					form = sheet.asInt();
				}
				break;
			}
		}
		else
		{
			NLMISC::CSheetId sheet(args[0]);
			form = sheet.asInt();
		}
	}
	
	TNewEntityInfo emptyEntityInfo;
	emptyEntityInfo.reset();
	CEntityCL *entity = EntitiesMngr.create(slot, form, emptyEntityInfo);
	if(entity)
	{
		sint64       *prop = 0;
		CCDBNodeLeaf *node = 0;

		CInterfaceManager *IM = CInterfaceManager::getInstance();
		// Set Position
		node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+NLMISC::toString("%d", slot)+":P"+NLMISC::toString("%d", CLFECOMMON::PROPERTY_POSX), false);
		NLMISC::CVectorD pos;
		pos = UserEntity->pos() + 2.f * UserEntity->front();
		if(node)
		{
			sint64 x = (sint64)(pos.x * 1000.0);
			sint64 y = (sint64)(pos.y * 1000.0);
			sint64 z = (sint64)(pos.z * 1000.0);
			node->setValue64(x);
			node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+NLMISC::toString("%d", slot)+":P"+NLMISC::toString("%d", CLFECOMMON::PROPERTY_POSY), false);
			if(node)
			{
				node->setValue64(y);
				node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+NLMISC::toString("%d", slot)+":P"+NLMISC::toString("%d", CLFECOMMON::PROPERTY_POSZ), false);
				if(node)
				{
					node->setValue64(z);
					EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_POSITION);
				}
			}
		}
		// Set Direction
		entity->front(UserEntity->front());
		entity->dir(UserEntity->front());
		// Set Mode
		node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+NLMISC::toString("%d", slot)+":P"+NLMISC::toString("%d", CLFECOMMON::PROPERTY_MODE), false);
		if(node)
		{
			MBEHAV::EMode m = MBEHAV::NORMAL;
			prop = (sint64 *)&m;
			node->setValue64(*prop);
			EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_MODE);
		}
		// Set Visual Properties
		if(dynamic_cast<CPlayerCL *>(entity))
		{
			SPropVisualA visualA;
			visualA.PropertySubData.Sex = ClientCfg.Sex;
			SPropVisualB visualB;
			// Initialize the Visual Property C (Default parameters).
			SPropVisualC visualC;
			visualC.PropertySubData.CharacterHeight = 7;
			visualC.PropertySubData.ArmsWidth       = 7;
			visualC.PropertySubData.LegsWidth       = 7;
			visualC.PropertySubData.TorsoWidth      = 7;
			visualC.PropertySubData.BreastSize		= 7;
			// Set The Database
			prop = (sint64 *)&visualB;
			NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+NLMISC::toString("%d", slot)+":P"+NLMISC::toString("%d", CLFECOMMON::PROPERTY_VPB))->setValue64(*prop);
			prop = (sint64 *)&visualC;
			NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+NLMISC::toString("%d", slot)+":P"+NLMISC::toString("%d", CLFECOMMON::PROPERTY_VPC))->setValue64(*prop);
			prop = (sint64 *)&visualA;
			NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+NLMISC::toString("%d", slot)+":P"+NLMISC::toString("%d", CLFECOMMON::PROPERTY_VPA))->setValue64(*prop);
			// Apply Changes.
			EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);
		}
		TestGroundFX.Entities.push_back(CTestGroundFX::CEntity());
		TestGroundFX.Entities.back().Entity = entity;
		TestGroundFX.Entities.back().Slot = slot;
		TestGroundFX.Entities.back().Move = false;
		TestGroundFX.Entities.back().Dir.set(0.f, 0.f, 0.f);
		TestGroundFX.Entities.back().Duration = 0;
		TestGroundFX.Entities.back().StartPos = pos;

		entity->pacsPos(pos);
	}
	return true;
}


// remove all test entities
NLMISC_COMMAND(gfxReset, "gfxReset", "<>")
{
	for(uint k = 0; k < TestGroundFX.Entities.size(); ++k)
	{
		EntitiesMngr.remove(TestGroundFX.Entities[k].Slot, false);
	}
	TestGroundFX.Entities.clear();
	return true;
}

// move one entity
NLMISC_COMMAND(gfxMove, "gfxMove", "<>")
{
	if (args.size() != 1) return false;
	uint index;
	fromString(args[0], index);
	if (index > TestGroundFX.Entities.size()) return false;
	TestGroundFX.Entities[index].Move = true;
	TestGroundFX.Entities[index].Dir.set(NLMISC::frand(1.f), NLMISC::frand(1.f), 0.f);
	TestGroundFX.Entities[index].Dir.normalize();
	TestGroundFX.Entities[index].Duration = 5.f;
	return true;
}

// move all entities
NLMISC_COMMAND(gfxMoveAll, "gfxMoveAll", "<>")
{
	for(uint k = 0; k < TestGroundFX.Entities.size(); ++k)
	{
		TestGroundFX.Entities[k].Dir.set(NLMISC::frand(1.f), NLMISC::frand(1.f), 0.f);
		TestGroundFX.Entities[k].Dir.normalize();
		TestGroundFX.Entities[k].Duration = 5.f;
	}
	TestGroundFX.MoveAll = true;
	return true;
}

// stop all entities
NLMISC_COMMAND(gfxStopAll, "gfxStopAll", "<>")
{
	for(uint k = 0; k < TestGroundFX.Entities.size(); ++k)
	{
		TestGroundFX.Entities[k].Move = false;
	}
	TestGroundFX.MoveAll = false;
	return true;
}

#endif
