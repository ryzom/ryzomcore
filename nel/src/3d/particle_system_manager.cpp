// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std3d.h"

#include "nel/3d/particle_system_manager.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/scene.h"
#include "nel/3d/skeleton_model.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

CParticleSystemManager::TManagerList *CParticleSystemManager::ManagerList = NULL;

CParticleSystemManager::TManagerList     &CParticleSystemManager::getManagerList()
{
	if (ManagerList == NULL)
	{
		ManagerList = new TManagerList;
	}
	return *ManagerList;
}


// release memory
void CParticleSystemManager::release()
{
	if( ManagerList )
		delete ManagerList;
	ManagerList = NULL;
}

///=========================================================
CParticleSystemManager::CParticleSystemManager() : _NumModels(0)
{
	NL_PS_FUNC(CParticleSystemManager_CParticleSystemManager)
	_CurrListIterator = _ModelList.end();
	getManagerList().push_front(this);
	_GlobalListHandle = getManagerList().begin();
}

///=========================================================
CParticleSystemManager::~CParticleSystemManager()
{
	NL_PS_FUNC(CParticleSystemManager_CParticleSystemManagerDtor)
	// remove from global list
	getManagerList().erase(_GlobalListHandle);
}

///=========================================================
void	CParticleSystemManager::refreshModels(const std::vector<NLMISC::CPlane>	&worldFrustumPyramid,  const NLMISC::CVector &viewerPos)
{
	NL_PS_FUNC(CParticleSystemManager_refreshModels)
	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif

	if (_NumModels == 0) return;
	const uint toProcess = std::min(_NumModels, (uint) NumProcessToRefresh);
	TModelList::iterator nextIt;
	for (uint k = 0; k < toProcess; ++k)
	{
		if (_CurrListIterator == _ModelList.end())
		{
			_CurrListIterator = _ModelList.begin();
			nlassert(_CurrListIterator!=_ModelList.end());
		}
		nextIt = _CurrListIterator;
		++ nextIt;
		(*_CurrListIterator)->refreshRscDeletion(worldFrustumPyramid, viewerPos);
		_CurrListIterator = nextIt;
		if (_NumModels == 0) break;
	}

	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif
}

///=========================================================
CParticleSystemManager::TModelHandle CParticleSystemManager::addSystemModel(CParticleSystemModel *model)
{
	NL_PS_FUNC(CParticleSystemManager_addSystemModel)
	#ifdef NL_DEBUG
		nlassert(std::find(_ModelList.begin(), _ModelList.end(), model) == _ModelList.end());
	#endif
	_ModelList.push_front(model);
	++_NumModels;

	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif

	TModelHandle handle;
	handle.Valid = true;
	handle.Iter = _ModelList.begin();
	return handle;
}


///=========================================================
void		CParticleSystemManager::removeSystemModel(TModelHandle &handle)
{
	NL_PS_FUNC(CParticleSystem_removeSystemModel)
	nlassert(handle.Valid);
	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif
	nlassert(_NumModels != 0);
	if (handle.Iter == _CurrListIterator)
	{
		++_CurrListIterator;
	}

	_ModelList.erase(handle.Iter);
	--_NumModels;
	handle.Valid = false;
	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif
}


///=========================================================
CParticleSystemManager::TAlwaysAnimatedModelHandle	CParticleSystemManager::addPermanentlyAnimatedSystem(CParticleSystemModel *ps)
{
	NL_PS_FUNC(CParticleSystemManager_addPermanentlyAnimatedSystem)
	#ifdef NL_DEBUG
		for(TAlwaysAnimatedModelList::iterator it = _PermanentlyAnimatedModelList.begin(); it != _PermanentlyAnimatedModelList.end(); ++it)
		{
			nlassert(it->Model != ps);
		}
	#endif
	CAlwaysAnimatedPS aaps;
	aaps.Model = ps;
	aaps.HasAncestorSkeleton = false; // even if there's an ancestor skeleton yet, force the manager to recompute relative pos of the system when clipped
	_PermanentlyAnimatedModelList.push_front(aaps);


	TAlwaysAnimatedModelHandle handle;
	handle.Valid = true;
	handle.Iter = _PermanentlyAnimatedModelList.begin();
	return handle;
}

///=========================================================
void			CParticleSystemManager::removePermanentlyAnimatedSystem(CParticleSystemManager::TAlwaysAnimatedModelHandle &handle)
{
	NL_PS_FUNC(CParticleSystemManager_removePermanentlyAnimatedSystem)
	nlassert(handle.Valid);
	_PermanentlyAnimatedModelList.erase(handle.Iter);
	handle.Valid = false;
}

///=========================================================
void	CParticleSystemManager::processAnimate(TAnimationTime deltaT)
{
	NL_PS_FUNC(CParticleSystemManager_processAnimate)
	for (TAlwaysAnimatedModelList::iterator it = _PermanentlyAnimatedModelList.begin(); it != _PermanentlyAnimatedModelList.end();)
	{
		CParticleSystemModel &psm = *(it->Model);
		CParticleSystem		 *ps  = psm.getPS();
		TAlwaysAnimatedModelList::iterator nextIt = it;
		nextIt++;
		if (ps)
		{
			// test if already auto-animated
			if (ps->getAnimType() != CParticleSystem::AnimAlways)
			{
				psm.invalidateAutoAnimatedHandle();
				it = _PermanentlyAnimatedModelList.erase(it);
				continue;
			}
			// special case for sticked fxs :
			// When a fx is sticked as a son of a skeleton model, the skeleton matrix is not updated
			// when the skeleton is not visible (clipped)
			// This is a concern when fx generate trails because when the skeleton becomes visible again,
			// a trail will appear between the previous visible pos and the new visible pos
			// to solve this :
			// When the ancestor skeleton is visible, we backup the relative position to the ancestor skeleton.
			//			- if it is not visible at start, we must evaluate the position of the stick point anyway
			// When the father skeleton is clipped, we use the relative position
			if (psm.getAncestorSkeletonModel())
			{
				if(psm.getAncestorSkeletonModel()->getVisibility() != CHrcTrav::Hide) // matrix of ancestor is irrelevant if the system is hidden (because sticked to hidden parent for example)
				{
					if (!psm.isClipVisible()) // the system may not be visible because of clod
					{
						if (!it->IsRelMatrix) // relative matrix already computed ?
						{
							if (!it->HasAncestorSkeleton)
							{
								psm.forceCompute();
							}
							it->OldAncestorMatOrRelPos = it->OldAncestorMatOrRelPos.inverted() * psm.getWorldMatrix();
							it->IsRelMatrix = true;
						}
						psm.setWorldMatrix(psm.getAncestorSkeletonModel()->getWorldMatrix() * it->OldAncestorMatOrRelPos);
					}
					else
					{
						// backup ancestor position matrix relative to the ancestor skeleton
						it->HasAncestorSkeleton = true;
						it->OldAncestorMatOrRelPos = psm.getAncestorSkeletonModel()->getWorldMatrix();
						it->IsRelMatrix = false;
					}
				}
			}
			psm.doAnimate();
			if (!psm.getEditionMode())
			{
				// test deletion condition (no more particle, no more particle and emitters)
				if (ps->isDestroyConditionVerified())
				{
					psm.releaseRscAndInvalidate();
				}
			}

		}
		it = nextIt;
	}
}

///=========================================================
void CParticleSystemManager::stopSound()
{
	NL_PS_FUNC(CParticleSystemManager_stopSound)
	for(TModelList::iterator it = _ModelList.begin(); it != _ModelList.end(); ++it)
	{
		CParticleSystemModel &psm = *(*it);
		CParticleSystem		 *ps  = psm.getPS();
		if (ps)
		{
			ps->stopSound();
		}
	}
}

///=========================================================
void CParticleSystemManager::reactivateSound()
{
	NL_PS_FUNC(CParticleSystemManager_reactivateSound)
	for(TModelList::iterator it = _ModelList.begin(); it != _ModelList.end(); ++it)
	{
		CParticleSystemModel &psm = *(*it);
		CParticleSystem		 *ps  = psm.getPS();
		if (ps)
		{
			ps->reactivateSound();
		}
	}
}

///=========================================================
void CParticleSystemManager::stopSoundForAllManagers()
{
	NL_PS_FUNC(CParticleSystemManager_stopSoundForAllManagers)
	for(TManagerList::iterator it = getManagerList().begin(); it != getManagerList().end(); ++it)
	{
		nlassert(*it);
		(*it)->stopSound();
	}
}

///=========================================================
void CParticleSystemManager::reactivateSoundForAllManagers()
{
	NL_PS_FUNC(CParticleSystemManager_reactivateSoundForAllManagers)
	for(TManagerList::iterator it = getManagerList().begin(); it != getManagerList().end(); ++it)
	{
		nlassert(*it);
		(*it)->reactivateSound();
	}
}

} // NL3D
