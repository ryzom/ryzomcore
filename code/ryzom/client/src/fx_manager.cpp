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
#include "stdpch.h"	// First include for pre-compiled headers.
// 3D
#include "nel/3d/u_particle_system_instance.h"
#include "nel/pacs/u_global_position.h"
// Client
#include "fx_manager.h"
#include "time_client.h"
#include "pacs_client.h"


H_AUTO_DECL(RZ_FXManager)

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
extern UScene	*Scene;


/////////////
// GLOBALS //
/////////////
// FX manager.
CFXManager	FXMngr;


/////////////
// MEMBERS //
/////////////


/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CFXManager :
// Constructor.
//-----------------------------------------------
CFXManager::CFXManager()
{
	H_AUTO_USE(RZ_FXManager)
	_NumFXToRemove = 0;
}// CFXManager //

//-----------------------------------------------
// CFXManager :
// Destructor.
//-----------------------------------------------
CFXManager::~CFXManager()
{
	H_AUTO_USE(RZ_FXManager)
	reset();
}// CFXManager //


//-----------------------------------------------
// instantFX :
// create an FX that will be removed as soon as possible.
//-----------------------------------------------
NL3D::UParticleSystemInstance CFXManager::instantFX(const std::string &fxName, float timeOut /* = 0.f*/)
{
	H_AUTO_USE(RZ_FXManager)
	// Check FX Name.
	if(fxName.empty())
		return 0;

	NL3D::UParticleSystemInstance fx;
	fx.cast (Scene->createInstance(fxName));
	if(!fx.empty())
	{
		_FX2RemoveList.push_back(CFX2Remove(fx, timeOut));
		++_NumFXToRemove;
	}
	return fx;
}// instantFX //


//-----------------------------------------------
// addFX :
//-----------------------------------------------
void CFXManager::addFX(NL3D::UParticleSystemInstance fx, float timeOut, bool testNoMoreParticles /*= false*/)
{
	H_AUTO_USE(RZ_FXManager)
	if (fx.empty()) return;
	_FX2RemoveList.push_back(CFX2Remove(fx, timeOut, testNoMoreParticles));
	++_NumFXToRemove;
}// addFX //

//-----------------------------------------------
// fx2remove :
//-----------------------------------------------
void CFXManager::fx2remove(NL3D::UParticleSystemInstance fx, float timeOut)
{
	H_AUTO_USE(RZ_FXManager)
	if(!fx.empty())
	{
		_FX2RemoveList.push_back(CFX2Remove(fx, timeOut));
		++_NumFXToRemove;
	}
}// fx2remove //

//-----------------------------------------------
// update :
//-----------------------------------------------
void CFXManager::update()
{
	H_AUTO_USE(RZ_FXManager)

	// deferred fx

	while (!_DeferredFXByDate.empty())
	{

		if (T1 < (uint)_DeferredFXByDate.begin()->first) break;
		const CDeferredFX &fx = _DeferredFXByDate.begin()->second;
		NL3D::UParticleSystemInstance fxInstance = instantFX(fx.FXName, fx.TimeOut);
		if (!fxInstance.empty())
		{
			fxInstance.setTransformMode(UTransform::DirectMatrix);
			fxInstance.setMatrix(fx.Matrix);
			UGlobalPosition gPos;
			GR->retrievePosition(fx.Matrix.getPos());
			UInstanceGroup *clusterSystem = getCluster(gPos);
			fxInstance.setClusterSystem(clusterSystem);
			fxInstance.forceInstanciate(); // accurate timing wanted
		}
		_DeferredFXByDate.erase(_DeferredFXByDate.begin());
	}


	#ifdef NL_DEBUG
		nlassert(_NumFXToRemove == _FX2RemoveList.size());
	#endif
	// Try to remove Animation FXs still not removed.
	std::list<CFX2Remove>::iterator itFx = _FX2RemoveList.begin();
	while(itFx != _FX2RemoveList.end())
	{

		// Backup the old iterator
		std::list<CFX2Remove>::iterator itTmp = itFx;
		// Iterator on the Next FX.
		itFx++;

		// If the FX is not present or valid -> remove the FX.
		if(!itTmp->Instance.isSystemPresent() ||
		   !itTmp->Instance.isValid() ||
		   (itTmp->TestNoMoreParticles && !itTmp->Instance.hasParticles())
		  )
		{
			// Delete the FX.
			Scene->deleteInstance(itTmp->Instance);
			// Remove from the list.
			_FX2RemoveList.erase(itTmp);
			-- _NumFXToRemove;
		}
		else
		if (itTmp->TimeOut != 0.f)
		{
			itTmp->TimeOut -= DT;
			if (itTmp->TimeOut < 0.f)
			{
				// Delete the FX.
				Scene->deleteInstance(itTmp->Instance);
				// Remove from the list.
				_FX2RemoveList.erase(itTmp);
				-- _NumFXToRemove;
			}
		}
	}
}// update //

//-----------------------------------------------
// reset :
//-----------------------------------------------
void CFXManager::reset()
{
	H_AUTO_USE(RZ_FXManager)
	// Remove Animation FXs still not removed.
	std::list<CFX2Remove>::iterator itFx = _FX2RemoveList.begin();
	while(itFx != _FX2RemoveList.end())
	{
		// Delete the FX.
		if(Scene)
			Scene->deleteInstance(itFx->Instance);
		// Next FX
		itFx++;
	}
	// clear FX list
	_FX2RemoveList.clear();
	_DeferredFXByDate.clear();
	_NumFXToRemove = 0;
}

//-----------------------------------------------
// deferedFX :
//-----------------------------------------------
void CFXManager::deferFX(const std::string &fxName, const NLMISC::CMatrix &matrix, float delayInSeconds, float timeOut /*=FX_MANAGER_DEFAULT_TIMEOUT*/)
{
	CDeferredFX fx;
	fx.FXName  = fxName;
	fx.Matrix  = matrix;
	fx.TimeOut = timeOut;
	_DeferredFXByDate.insert(std::make_pair(T1 + uint64(1000.f * delayInSeconds), fx));
}


