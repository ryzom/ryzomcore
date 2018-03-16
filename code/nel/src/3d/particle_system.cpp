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



#include "nel/3d/particle_system.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/driver.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/nelu.h"
#include "nel/3d/ps_util.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_emitter.h"
#include "nel/3d/ps_sound.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/ps_located.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/file.h"
#include "nel/misc/stream.h"

// tmp
#include "nel/3d/particle_system_model.h"


#ifdef NL_DEBUG
	#define CHECK_INTEGRITY checkIntegrity();
#else
	#define CHECK_INTEGRITY
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif



namespace NL3D
{

uint32										CParticleSystem::NbParticlesDrawn = 0;
UPSSoundServer *							CParticleSystem::_SoundServer = NULL;
CParticleSystem::TGlobalValuesMap			CParticleSystem::_GlobalValuesMap;
CParticleSystem::TGlobalVectorValuesMap		CParticleSystem::_GlobalVectorValuesMap;

// sim step infos
TAnimationTime CParticleSystem::EllapsedTime = 0.f;
TAnimationTime CParticleSystem::InverseTotalEllapsedTime = 0.f;
TAnimationTime CParticleSystem::RealEllapsedTime = 0.f;
float CParticleSystem::RealEllapsedTimeRatio = 1.f;
bool CParticleSystem::InsideSimLoop = false;
bool CParticleSystem::InsideRemoveLoop = false;
bool CParticleSystem::InsideNewElementsLoop = false;;
std::vector<NLMISC::CVector> CParticleSystem::_SpawnPos;



//bool FilterPS[10] = { false, false, false, false, false, false, false, false, false, false };


#ifdef NL_DEBUG
uint	CParticleSystem::_NumInstances = 0;
#endif


static const float PS_MIN_TIMEOUT = 1.f; // the test that check if there are no particles left
#if defined(NL_DEBUG) ||  defined(NL_PS_DEBUG)
	bool CParticleSystem::_SerialIdentifiers = true;
#else
	bool CParticleSystem::_SerialIdentifiers = false;
#endif
bool CParticleSystem::_ForceDisplayBBox = false;
CParticleSystemModel *CParticleSystem::OwnerModel = NULL;





///////////////////////////////////
// CPaticleSystem implementation //
///////////////////////////////////


/// the default max distance of view for particle systems
const float PSDefaultMaxViewDist = 300.f;

/*
 * Constructor
 */
CParticleSystem::CParticleSystem() : _Driver(NULL),
	_FontGenerator(NULL),
	_FontManager(NULL),
	_UserCoordSystemInfo(NULL),
	_Date(0),
	_LastUpdateDate(-1),
	_CurrEditedElementLocated(NULL),
	_CurrEditedElementLocatedBindable(NULL),
	_CurrEditedElementIndex(0),
	_Scene(NULL),
	_TimeThreshold(0.15f),
	_SystemDate(0.f),
	_MaxNbIntegrations(2),
	_LODRatio(0.5f),
	_OneMinusCurrentLODRatio(0),
	_MaxViewDist(PSDefaultMaxViewDist),
	_MaxDistLODBias(0.05f),
	_InvMaxViewDist(1.f / PSDefaultMaxViewDist),
	_InvCurrentViewDist(1.f / PSDefaultMaxViewDist),
	_AutoLODEmitRatio(0.f),
	_DieCondition(none),
	_DelayBeforeDieTest(-1.f),
	_NumWantedTris(0),
	_AnimType(AnimInCluster),
	_UserParamGlobalValue(NULL),
	_BypassGlobalUserParam(0),
	_PresetBehaviour(UserBehaviour),
	_ColorAttenuationScheme(NULL),
	_GlobalColor(NLMISC::CRGBA::White),
	_GlobalColorLighted(NLMISC::CRGBA::White),
	_LightingColor(NLMISC::CRGBA::White),
	_UserColor(NLMISC::CRGBA::White),
	_ComputeBBox(true),
	_BBoxTouched(true),
	_AccurateIntegration(true),
	_CanSlowDown(true),
	_DestroyModelWhenOutOfRange(false),
	_DestroyWhenOutOfFrustum(false),
	_Sharing(false),
	_AutoLOD(false),
	_KeepEllapsedTimeForLifeUpdate(false),
	_AutoLODSkipParticles(false),
	_EnableLoadBalancing(true),
	_EmitThreshold(true),
	_BypassIntegrationStepLimit(false),
	_ForceGlobalColorLighting(false),
	_AutoComputeDelayBeforeDeathTest(true),
	_AutoCount(false),
	_HiddenAtCurrentFrame(true),
	_HiddenAtPreviousFrame(true),
	_AutoLODStartDistPercent(0.1f),
	_AutoLODDegradationExponent(1)
{
	NL_PS_FUNC_MAIN(CParticleSystem_CParticleSystem)
	std::fill(_UserParam, _UserParam + MaxPSUserParam, 0.0f);
	#ifdef NL_DEBUG
		++_NumInstances;
	#endif

}


std::vector<NLMISC::CSmartPtr<CParticleSystem::CSpawnVect> > CParticleSystem::_Spawns; // spawns for the current system being processed
std::vector<uint> CParticleSystem::_ParticleToRemove;
std::vector<sint> CParticleSystem::_ParticleRemoveListIndex;




///=======================================================================================
/// immediatly shut down all the sound in this system
void CParticleSystem::stopSound()
{
	NL_PS_FUNC_MAIN(CParticleSystem_stopSound)
	for (uint k = 0; k < this->getNbProcess(); ++k)
	{
		CPSLocated *psl = dynamic_cast<NL3D::CPSLocated *>(this->getProcess(k));
		if (psl)
		{
			for (uint l = 0; l < psl->getNbBoundObjects(); ++l)
			{
				if (psl->getBoundObject(l)->getType() == PSSound)
				{
					static_cast<CPSSound *>(psl->getBoundObject(l))->stopSound();
				}
			}
		}
	}
}

///=======================================================================================
void CParticleSystem::reactivateSound()
{
	NL_PS_FUNC_MAIN(CParticleSystem_reactivateSound)
	for (uint k = 0; k < this->getNbProcess(); ++k)
	{
		CPSLocated *psl = dynamic_cast<NL3D::CPSLocated *>(this->getProcess(k));
		if (psl)
		{
			for (uint l = 0; l < psl->getNbBoundObjects(); ++l)
			{
				if (psl->getBoundObject(l)->getType() == PSSound)
				{
					static_cast<CPSSound *>(psl->getBoundObject(l))->reactivateSound();
				}
			}
		}
	}
}


///=======================================================================================
void CParticleSystem::enableLoadBalancing(bool enabled /*=true*/)
{
	NL_PS_FUNC_MAIN(CParticleSystem_enableLoadBalancing)
	if (enabled)
	{
		//notifyMaxNumFacesChanged();
	}
	_EnableLoadBalancing = enabled;
}

///=======================================================================================
/*
void CParticleSystem::notifyMaxNumFacesChanged(void)
{
	if (!_EnableLoadBalancing) return;
	_MaxNumFacesWanted = 0;
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		_MaxNumFacesWanted += (*it)->querryMaxWantedNumFaces();
	}
}
*/

///=======================================================================================
void CParticleSystem::updateNumWantedTris()
{
	NL_PS_FUNC_MAIN(CParticleSystem_updateNumWantedTris)
	_NumWantedTris = 0;
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		_NumWantedTris += (*it)->getNumWantedTris();
	}
}

///=======================================================================================
float CParticleSystem::getWantedNumTris(float dist)
{
	NL_PS_FUNC_MAIN(CParticleSystem_getWantedNumTris)
	if (!_EnableLoadBalancing) return 0; // no contribution to the load balancing
	if (dist > _MaxViewDist) return 0;
	float retValue = ((1.f - dist * _InvMaxViewDist) * _NumWantedTris);
	///nlassertex(retValue >= 0 && retValue < 10000, ("dist = %f, _MaxViewDist = %f, _MaxNumFacesWanted = %d, retValue = %f",  dist, _MaxViewDist, _MaxNumFacesWanted, retValue));
	return retValue;
}


///=======================================================================================
void CParticleSystem::setNumTris(uint numFaces)
{
	NL_PS_FUNC_MAIN(CParticleSystem_setNumTris)
	if (_EnableLoadBalancing)
	{
		float modelDist = (getSysMat().getPos() - _InvertedViewMat.getPos()).norm();
		/*uint numFaceWanted = (uint) getWantedNumTris(modelDist);*/

		const float epsilon = 10E-5f;


		uint wantedNumTri = (uint) getWantedNumTris(modelDist);
		if (numFaces >= wantedNumTri || wantedNumTri == 0 || _NumWantedTris == 0 || modelDist < epsilon)
		{
			_InvCurrentViewDist = _InvMaxViewDist;
		}
		else
		{

			_InvCurrentViewDist = (_NumWantedTris - numFaces) / (_NumWantedTris * modelDist);
		}
	}
	else
	{
		// always take full detail when there's no load balancing
		_InvCurrentViewDist = _InvMaxViewDist;
	}
}


///=======================================================================================
/// dtor
CParticleSystem::~CParticleSystem()
{
	NL_PS_FUNC_MAIN(CParticleSystem_CParticleSystemDtor)
 	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		delete *it;
	}
	if (_ColorAttenuationScheme)
		delete _ColorAttenuationScheme;
	if (_UserParamGlobalValue)
		delete _UserParamGlobalValue;
	delete _UserCoordSystemInfo;
	#ifdef NL_DEBUG
		--_NumInstances;
	#endif
}

///=======================================================================================
void CParticleSystem::setViewMat(const NLMISC::CMatrix &m)
{
	NL_PS_FUNC_MAIN(CParticleSystem_setViewMat)
	_ViewMat = m;
	_InvertedViewMat = m.inverted();
}

///=======================================================================================
bool CParticleSystem::hasEmitters() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasEmitters)
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->hasEmitters()) return true;
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasParticles() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasParticles)
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->hasParticles()) return true;
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasTemporaryParticles() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasTemporaryParticles)
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(*it);
			if (loc->hasParticles()) return true;
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::stepLocated(TPSProcessPass pass)
{
	NL_PS_FUNC_MAIN(CParticleSystem_stepLocated)
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		(*it)->step(pass);
	}
}

///=======================================================================================
#ifndef NL_DEBUG
	inline
#endif
float CParticleSystem::getDistFromViewer() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getDistFromViewer)
	const CVector d = getSysMat().getPos() - _InvertedViewMat.getPos();
	return d.norm();
}

///=======================================================================================
#ifndef NL_DEBUG
	inline
#endif
float CParticleSystem::updateLODRatio()
{
	NL_PS_FUNC_MAIN(CParticleSystem_updateLODRatio)
	float dist = getDistFromViewer();
	_OneMinusCurrentLODRatio = 1.f - (dist * _InvCurrentViewDist);
	NLMISC::clamp(_OneMinusCurrentLODRatio, 0.f, 1.f);
	return dist;
}

///=======================================================================================
inline void CParticleSystem::updateColor(float distFromViewer)
{
	NL_PS_FUNC_MAIN(CParticleSystem_updateColor)
	if (_ColorAttenuationScheme)
	{
		float ratio = distFromViewer * _InvMaxViewDist;
		NLMISC::clamp(ratio, 0.f, 1.f);
		_GlobalColor = 	_ColorAttenuationScheme->get(ratio);
	}
	else
	{
		_GlobalColor = NLMISC::CRGBA::White;
	}
	_GlobalColor.modulateFromColor(_GlobalColor, _UserColor);
	_GlobalColorLighted.modulateFromColor(_GlobalColor, _LightingColor);
}


/*
static void displaySysPos(IDriver *drv, const CVector &pos, CRGBA col)
{
	if (!drv) return;
	drv->setupModelMatrix(CMatrix::Identity);
	CPSUtil::displayArrow(drv, pos, CVector::K, 1.f, CRGBA::White, col);
}
*/


///=======================================================================================
void CParticleSystem::step(TPass pass, TAnimationTime ellapsedTime, CParticleSystemShape &shape, CParticleSystemModel &model)
{
	NL_PS_FUNC_MAIN(CParticleSystem_step)
	CHECK_INTEGRITY
	OwnerModel = &model;
	if (!_CoordSystemInfo.Matrix)
	{
		nlwarning("3D: BUG: CParticleSystem::step -> !_CoordSystemInfo.Matrix");
		return;
	}
	nlassert(_CoordSystemInfo.Matrix); // matrix not set for position of system
	if (_UserCoordSystemInfo)
	{
		nlassert(_CoordSystemInfo.Matrix);
	}
	switch (pass)
	{
		case SolidRender:
			EllapsedTime = RealEllapsedTime = ellapsedTime;
			RealEllapsedTimeRatio = 1.f;
			/// When shared, the LOD ratio must be computed there
			if (_Sharing)
			{
				float dist = updateLODRatio();
				updateColor(dist);
			}
			else
			{
				updateColor(getDistFromViewer());
			}
			// update time
			++_Date;
			// update global color
			stepLocated(PSSolidRender);

		break;
		case BlendRender:
			EllapsedTime = RealEllapsedTime = ellapsedTime;
			RealEllapsedTimeRatio = 1.f;
			/// When shared, the LOD ratio must be computed there
			/// When shared, the LOD ratio must be computed there
			if (_Sharing)
			{
				float dist = updateLODRatio();
				updateColor(dist);
			}
			else
			{
				updateColor(getDistFromViewer());
			}
			// update time
			++_Date;
			// update global color
			stepLocated(PSBlendRender);
			if (_ForceDisplayBBox)
			{
				NLMISC::CAABBox box;
				computeBBox(box);
				getDriver()->setupModelMatrix(*_CoordSystemInfo.Matrix);
				CPSUtil::displayBBox(getDriver(), box);
			}
		break;
		case ToolRender:
			EllapsedTime = RealEllapsedTime = ellapsedTime;
			RealEllapsedTimeRatio = 1.f;
			stepLocated(PSToolRender);
		break;
		case Anim:
		{
			if (ellapsedTime <= 0.f) return;
			// update user param from global value if needed, unless this behaviour is bypassed has indicated by a flag in _BypassGlobalUserParam
			if (_UserParamGlobalValue)
			{
				nlctassert(MaxPSUserParam < 8); // there should be less than 8 parameters because of mask stored in a byte
				uint8 bypassMask = 1;
				for(uint k = 0; k < MaxPSUserParam; ++k)
				{
					if (_UserParamGlobalValue[k] && !(_BypassGlobalUserParam & bypassMask)) // if there is a global value for this param and if the update is not bypassed
					{
						_UserParam[k] = _UserParamGlobalValue[k]->second;
					}
					bypassMask <<= 1;
				}
			}
			//
			uint nbPass = 1;
			EllapsedTime = ellapsedTime;
			_BBoxTouched = true;
			if (_AccurateIntegration)
			{
				if (EllapsedTime > _TimeThreshold)
				{
					nbPass = (uint32) ceilf(EllapsedTime / _TimeThreshold);
					if (!_BypassIntegrationStepLimit && nbPass > _MaxNbIntegrations)
					{
						nbPass = _MaxNbIntegrations;
						if (_CanSlowDown)
						{
							EllapsedTime = _TimeThreshold;
							nlassert(_TimeThreshold != 0);
							InverseTotalEllapsedTime = 1.f / (_TimeThreshold * nbPass);
						}
						else
						{
							EllapsedTime = ellapsedTime / nbPass;
							InverseTotalEllapsedTime = ellapsedTime != 0 ? 1.f / ellapsedTime : 0.f;
						}
					}
					else
					{
						EllapsedTime = ellapsedTime / nbPass;
						InverseTotalEllapsedTime = ellapsedTime != 0 ? 1.f / ellapsedTime : 0.f;
					}
				}
				else
				{
					InverseTotalEllapsedTime = ellapsedTime != 0 ? 1.f / ellapsedTime : 0.f;
				}
			}
			else
			{
				InverseTotalEllapsedTime = ellapsedTime != 0 ? 1.f / ellapsedTime : 0.f;
			}
			updateLODRatio();
			{
				MINI_TIMER(PSAnim3)
				if (_AutoLOD && !_Sharing)
				{
					float currLODRatio = 1.f - _OneMinusCurrentLODRatio;
					if (currLODRatio <= _AutoLODStartDistPercent)
					{
						_AutoLODEmitRatio = 1.f; // no LOD applied
					}
					else
					{
						float lodValue = (currLODRatio - 1.f) / (_AutoLODStartDistPercent - 1.f);
						NLMISC::clamp(lodValue, 0.f, 1.f);
						float finalValue = lodValue;
						for(uint l = 1; l < _AutoLODDegradationExponent; ++l)
						{
							finalValue *= lodValue;
						}
						_AutoLODEmitRatio = (1.f - _MaxDistLODBias) * finalValue + _MaxDistLODBias;
						if (_AutoLODEmitRatio < 0.f) _AutoLODEmitRatio = 0.f;
					}
				}
			}
			{
				MINI_TIMER(PSAnim4)
				// set start position. Used by emitters that emit from Local basis to world
				if (!_HiddenAtPreviousFrame && !_HiddenAtCurrentFrame)
				{
					_CoordSystemInfo.CurrentDeltaPos = _CoordSystemInfo.OldPos - _CoordSystemInfo.Matrix->getPos();
					if (_UserCoordSystemInfo)
					{
						CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
						csi.CurrentDeltaPos = csi.OldPos - csi.Matrix->getPos();
					}
				}
				else
				{
					_CoordSystemInfo.CurrentDeltaPos = NLMISC::CVector::Null;
					_CoordSystemInfo.OldPos = _CoordSystemInfo.Matrix->getPos();
					if (_UserCoordSystemInfo)
					{
						CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
						csi.CurrentDeltaPos = NLMISC::CVector::Null;
						csi.OldPos = csi.Matrix->getPos();
					}
				}
				//displaySysPos(_Driver, _CurrentDeltaPos + _OldSysMat.getPos(), CRGBA::Red);
				// process passes
			}
			RealEllapsedTime = _KeepEllapsedTimeForLifeUpdate ? (ellapsedTime / nbPass)
														  : EllapsedTime;
			RealEllapsedTimeRatio = RealEllapsedTime / EllapsedTime;
			/*PSMaxET = std::max(PSMaxET, ellapsedTime);
			PSMaxNBPass = std::max(PSMaxNBPass, nbPass);*/



			// Sort by emission depth. We assume that the ps is a directed acyclic graph (so no loop will be encountered)
			if (shape._ProcessOrder.empty())
			{
				getSortingByEmitterPrecedence(shape._ProcessOrder);
			}
			// nodes sorted by degree
			InsideSimLoop = true;
			// make enough room for spawns
			uint numProcess = (uint)_ProcessVect.size();
			if (numProcess > _Spawns.size())
			{
				uint oldSize = (uint)_Spawns.size();
				_Spawns.resize(numProcess);
				for(uint k = oldSize; k < numProcess; ++k)
				{
					_Spawns[k] = new CSpawnVect;
				}
			}
			for(uint k = 0; k < numProcess; ++k)
			{
				if (!_ProcessVect[k]->isLocated()) continue;
				CPSLocated *loc = static_cast<CPSLocated *>(_ProcessVect[k]);
				if (loc->hasLODDegradation())
				{
					loc->doLODDegradation();
				}
				_Spawns[k]->MaxNumSpawns = loc->getMaxSize();
			}
			do
			{
				{
					MINI_TIMER(PSAnim5)
					// position of the system at the end of the integration
					_CoordSystemInfo.CurrentDeltaPos += (_CoordSystemInfo.Matrix->getPos() - _CoordSystemInfo.OldPos) * (EllapsedTime * InverseTotalEllapsedTime);
					if (_UserCoordSystemInfo)
					{
						CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
						csi.CurrentDeltaPos += (csi.Matrix->getPos() - csi.OldPos) * (EllapsedTime * InverseTotalEllapsedTime);
					}
				}
				for(uint k = 0; k < shape._ProcessOrder.size(); ++k)
				{
					if (!_ProcessVect[shape._ProcessOrder[k]]->isLocated()) continue;
					CPSLocated *loc = static_cast<CPSLocated *>(_ProcessVect[shape._ProcessOrder[k]]);
					if (_ParticleRemoveListIndex.size() < loc->getMaxSize())
					{
						_ParticleRemoveListIndex.resize(loc->getMaxSize(), -1);
					}
					if (loc->getSize() != 0)
					{
						#ifdef NL_DEBUG
							loc->checkLife();
						#endif
						if (loc->hasCollisionInfos())
						{
							loc->resetCollisions(loc->getSize());
						}
						// compute motion (including collisions)
						if (!loc->isParametricMotionEnabled()) loc->computeMotion();
						// Update life and mark particles that must be removed
						loc->updateLife();
						// Spawn particles. Emitters date is updated only after so we check in CPSLocated::postNewElement
						// if the emitter was still alive at this date, otherwise we discard the post
						loc->computeSpawns(0, false);
						if (loc->hasCollisionInfos()) loc->updateCollisions();
						// Remove too old particles, making room for new ones
						if (!_ParticleToRemove.empty())
						{
							loc->removeOldParticles();
						}
						#ifdef NL_DEBUG
							loc->checkLife();
						#endif
					}
					if (!_Spawns[shape._ProcessOrder[k]]->SpawnInfos.empty())
					{
						uint insertionIndex = loc->getSize(); // index at which new particles will be inserted
						// add new particles that where posted by ancestor emitters, and also mark those that must already be deleted
						loc->addNewlySpawnedParticles();
						if (insertionIndex != loc->getSize())
						{
							// Compute motion for spawned particles. This is useful only if particle can collide because
							if (loc->hasCollisionInfos())
							{
								loc->resetCollisions(loc->getSize());
								loc->computeNewParticleMotion(insertionIndex);
							}
							loc->computeSpawns(insertionIndex, true);
							if (loc->hasCollisionInfos()) loc->updateCollisions();
							// Remove too old particles among the newly created ones.
							if (!_ParticleToRemove.empty())
							{
								loc->removeOldParticles();
							}
							#ifdef NL_DEBUG
								loc->checkLife();
							#endif
						}
					}
					if (!loc->isParametricMotionEnabled()) loc->computeForces();
				}
				_SystemDate += RealEllapsedTime;
				for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
				{
					#ifdef NL_DEBUG
						if ((*it)->isLocated())
						{
							CPSLocated *loc = static_cast<CPSLocated *>(*it);
							loc->checkLife();
						}
					#endif
					{
						MINI_TIMER(PSAnim2)
						(*it)->step(PSMotion);
					}
				}
			}
			while (--nbPass);
			{
				MINI_TIMER(PSAnim10)
				// perform parametric motion if present
				for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
				{
					if ((*it)->isParametricMotionEnabled()) (*it)->performParametricMotion(_SystemDate);
				}
			}

			updateNumWantedTris();

			InsideSimLoop = false;

			{
				MINI_TIMER(PSAnim11)
				// memorize position of matrix for next frame (becomes old position)
				_CoordSystemInfo.OldPos = _CoordSystemInfo.Matrix->getPos();
				if (_UserCoordSystemInfo)
				{
					CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
					csi.OldPos =  csi.Matrix->getPos();
				}

				_HiddenAtPreviousFrame = _HiddenAtCurrentFrame;
			}
		}
	}
	RealEllapsedTimeRatio = 0.f;
	CHECK_INTEGRITY
}


///=======================================================================================
void CParticleSystem::serial(NLMISC::IStream &f)
{
	CHECK_INTEGRITY
	NL_PS_FUNC_MAIN(CParticleSystem_serial)
	sint version =  f.serialVersion(19);

	// version 19: sysmat no more serialized (useless)
	// version 18: _AutoComputeDelayBeforeDeathTest
	// version 17: _ForceGlobalColorLighting flag
	// version 16: _BypassIntegrationStepLimit flag
	// version 14: emit threshold
	// version 13: max dist lod bias for auto-LOD
	// version 12: global userParams
	// version 11: enable load balancing flag
	// version 9: Sharing flag added
	//            Auto-lod parameters
	//            New integration flag
	//			  Global color attenuation
	// version 8: Replaced the attribute '_PerformMotionWhenOutOfFrustum' by a _AnimType field which allow more precise control

	//f.serial(_ViewMat);

	// patch for old fx : force to recompute duration when fx is saved to avoid prbs
	if (!f.isReading())
	{
		if (_AutoComputeDelayBeforeDeathTest)
		{
			_DelayBeforeDieTest = evalDuration();
		}
	}

	if (version < 19)
	{
		NLMISC::CMatrix dummy;
		f.serial(dummy);
	}
	f.serial(_Date);
	if (f.isReading())
	{
		delete _ColorAttenuationScheme;
		// delete previous multimap
		_LBMap.clear();
		// delete previously attached process
		TProcessVect::iterator it;
		for (it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
		{
			delete (*it);
		}

		_ProcessVect.clear();

		f.serialContPolyPtr(_ProcessVect);

		_FontGenerator = NULL;
		_FontManager = NULL;
		if (_UserParamGlobalValue)
			delete _UserParamGlobalValue;
		_UserParamGlobalValue = NULL;
		_BypassGlobalUserParam = 0;
		// see if some process need to access the user matrix
		delete _UserCoordSystemInfo;
		_UserCoordSystemInfo = NULL;
		for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
		{
			addRefForUserSysCoordInfo((*it)->getUserMatrixUsageCount());
		}
	}
	else
	{
		f.serialContPolyPtr(_ProcessVect);
	}

	if (version > 1) // name of the system
	{
		if (f.isReading() && !getSerializeIdentifierFlag())
		{
			// just skip the name
			sint32 len;
			f.serial(len);
			f.seek(len, NLMISC::IStream::current);
		}
		else
		{
			f.serial(_Name);
		}
	}

	if (version > 2) // infos about integration, and LOD
	{
		bool accurateIntegration = _AccurateIntegration; // read from bitfield
		f.serial(accurateIntegration);
		_AccurateIntegration = accurateIntegration;
		if (_AccurateIntegration)
		{
			bool canSlowDown = _CanSlowDown;
			f.serial(canSlowDown);
			_CanSlowDown = canSlowDown;
			f.serial(_TimeThreshold, _MaxNbIntegrations);
		}
		f.serial(_InvMaxViewDist, _LODRatio);
		_MaxViewDist = 1.f / _InvMaxViewDist;
		_InvCurrentViewDist = _InvMaxViewDist;
	}

	if (version > 3) // tell whether the system must compute his bbox, hold a precomputed bbox
	{
		bool computeBBox = _ComputeBBox;
		f.serial(computeBBox);
		_ComputeBBox = computeBBox;
		if (!computeBBox)
		{
			f.serial(_PreComputedBBox);
		}
	}

	if (version > 4) // lifetime information
	{
		bool destroyModelWhenOutOfRange = _DestroyModelWhenOutOfRange; // read from bitfield
		f.serial(destroyModelWhenOutOfRange);
		_DestroyModelWhenOutOfRange = destroyModelWhenOutOfRange;
		f.serialEnum(_DieCondition);
		if (_DieCondition != none)
		{
			f.serial(_DelayBeforeDieTest);
		}
	}

	if (version > 5)
	{
		bool destroyWhenOutOfFrustum = _DestroyWhenOutOfFrustum; // read from bitfield
		f.serial(destroyWhenOutOfFrustum);
		_DestroyWhenOutOfFrustum = destroyWhenOutOfFrustum;
	}

	if (version > 6 && version < 8)
	{
		bool performMotionWOOF;
		if (f.isReading())
		{
			f.serial(performMotionWOOF);
			performMotionWhenOutOfFrustum(performMotionWOOF);
		}
		else
		{
			performMotionWOOF = doesPerformMotionWhenOutOfFrustum();
			f.serial(performMotionWOOF);
		}
	}

	if (version > 7)
	{
		f.serialEnum(_AnimType);
		f.serialEnum(_PresetBehaviour);
	}

	if (version > 8)
	{
		bool sharing = _Sharing; // read from bitfield
		f.serial(sharing);
		_Sharing = sharing;
		bool autoLOD = _AutoLOD; // read from bitfield
		f.serial(autoLOD);
		_AutoLOD = autoLOD;

		if (_AutoLOD)
		{
			f.serial(_AutoLODStartDistPercent, _AutoLODDegradationExponent);
			bool autoLODSkipParticles = _AutoLODSkipParticles; // read from bitfield
			f.serial(autoLODSkipParticles);
			_AutoLODSkipParticles = autoLODSkipParticles;
		}
		bool keepEllapsedTimeForLifeUpdate = _KeepEllapsedTimeForLifeUpdate;
		f.serial(keepEllapsedTimeForLifeUpdate);
		_KeepEllapsedTimeForLifeUpdate = keepEllapsedTimeForLifeUpdate;
		f.serialPolyPtr(_ColorAttenuationScheme);
	}

	if (version >= 11)
	{
		bool enableLoadBalancing = _EnableLoadBalancing; // read from bitfield
		f.serial(enableLoadBalancing);
		_EnableLoadBalancing = enableLoadBalancing;
	}

	if (version >= 12)
	{
		// serial infos about global user params
		nlctassert(MaxPSUserParam < 8); // In this version mask of used global user params are stored in a byte..
		if (f.isReading())
		{
			uint8 mask;
			f.serial(mask);
			if (mask)
			{
				std::string globalValueName;
				uint8 testMask = 1;
				for(uint k = 0; k < MaxPSUserParam; ++k)
				{
					if (mask & testMask)
					{
						f.serial(globalValueName);
						bindGlobalValueToUserParam(globalValueName.c_str(), k);
					}
					testMask <<= 1;
				}
			}
		}
		else
		{
			uint8 mask = 0;
			if (_UserParamGlobalValue)
			{
				for(uint k = 0; k < MaxPSUserParam; ++k)
				{
					if (_UserParamGlobalValue[k]) mask |= (1 << k);
				}
			}
			f.serial(mask);
			if (_UserParamGlobalValue)
			{
				for(uint k = 0; k < MaxPSUserParam; ++k)
				{
					if (_UserParamGlobalValue[k])
					{
						std::string valueName = _UserParamGlobalValue[k]->first;
						f.serial(valueName);
					}
				}
			}
		}
	}
	if (version >= 13)
	{
		if (_AutoLOD && !_Sharing)
		{
			f.serial(_MaxDistLODBias);
		}
	}
	if (version >= 14)
	{
		bool emitThreshold = _EmitThreshold; // read from bitfiled
		f.serial(emitThreshold);
		_EmitThreshold = emitThreshold;
	}

	if (version >= 15)
	{
		bool bypassIntegrationStepLimit = _BypassIntegrationStepLimit; // read from bitfield
		f.serial(bypassIntegrationStepLimit);
		_BypassIntegrationStepLimit = bypassIntegrationStepLimit;
	}

	if (version >= 17)
	{
		bool forceGlobalColorLighting = _ForceGlobalColorLighting; // read from bitfield
		f.serial(forceGlobalColorLighting);
		_ForceGlobalColorLighting = forceGlobalColorLighting;
	}

	if (version >= 18)
	{
		bool autoComputeDelayBeforeDeathTest = _AutoComputeDelayBeforeDeathTest; // read from bitfield
		f.serial(autoComputeDelayBeforeDeathTest);
		_AutoComputeDelayBeforeDeathTest = autoComputeDelayBeforeDeathTest;
	}
	else
	{
		nlassert(f.isReading());
		// for all previously created system, force to eval the system duration in an automatyic way
		setDelayBeforeDeathConditionTest(-1.f);
		_AutoComputeDelayBeforeDeathTest = true;
	}

	if (f.isReading())
	{
		//notifyMaxNumFacesChanged();
		updateNumWantedTris();
		activatePresetBehaviour(_PresetBehaviour); // apply behaviour changes
		updateProcessIndices();
	}
	CHECK_INTEGRITY
	// tmp
	//if (f.isReading())
	//	{
	//		dumpHierarchy();
	//	}

}

///=======================================================================================
bool CParticleSystem::attach(CParticleSystemProcess *ptr)
{
	NL_PS_FUNC_MAIN(CParticleSystem_attach)
	nlassert(ptr);
	nlassert(std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr) == _ProcessVect.end() ); // can't attach twice
	//nlassert(ptr->getOwner() == NULL);
	_ProcessVect.push_back(ptr);
	ptr->setOwner(this);
	ptr->setIndex((uint32)_ProcessVect.size() - 1);
	//notifyMaxNumFacesChanged();
	if (getBypassMaxNumIntegrationSteps())
	{
		if (!canFinish())
		{
			remove(ptr);
			nlwarning("<void CParticleSystem::attach> Can't attach object : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. Object is not attached");
			return false;
		}
	}
	systemDurationChanged();
	return true;
}

///=======================================================================================
void CParticleSystem::remove(CParticleSystemProcess *ptr)
{
	NL_PS_FUNC_MAIN(CParticleSystem_remove)
	TProcessVect::iterator it = std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr);
	nlassert(it != _ProcessVect.end() );
	ptr->setOwner(NULL);
	_ProcessVect.erase(it);
	delete ptr;
	systemDurationChanged();
	updateProcessIndices();
}

///=======================================================================================
void CParticleSystem::forceComputeBBox(NLMISC::CAABBox &aabbox)
{
	NL_PS_FUNC_MAIN(CParticleSystem_forceComputeBBox)
	bool foundOne = false;
	NLMISC::CAABBox tmpBox;
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->computeBBox(tmpBox))
		{
			// rotate the aabbox so that it is in the correct basis
			const CMatrix &convMat = CPSLocated::getConversionMatrix(*this, PSFXWorldMatrix, (*it)->getMatrixMode());
			tmpBox = NLMISC::CAABBox::transformAABBox(convMat, tmpBox);

			if (foundOne)
			{
				aabbox = NLMISC::CAABBox::computeAABBoxUnion(aabbox, tmpBox);
			}
			else
			{
				aabbox = tmpBox;
				foundOne = true;
			}
		}
	}

	if (!foundOne)
	{
		aabbox.setCenter(NLMISC::CVector::Null);
		aabbox.setHalfSize(NLMISC::CVector::Null);
	}
}

///=======================================================================================
void CParticleSystem::computeBBox(NLMISC::CAABBox &aabbox)
{
	NL_PS_FUNC_MAIN(CParticleSystem_computeBBox)
	if (!_ComputeBBox || !_BBoxTouched)
	{
		aabbox = _PreComputedBBox;
		return;
	}
	forceComputeBBox(aabbox);
	_BBoxTouched = false;
	_PreComputedBBox = aabbox;
}

///=======================================================================================
void CParticleSystem::setSysMat(const CMatrix *m)
{
	NL_PS_FUNC_MAIN(CParticleSystem_setSysMat)
	_CoordSystemInfo.Matrix = m;
	if (_SystemDate == 0.f)
	{
		_CoordSystemInfo.OldPos = m ? m->getPos() : CVector::Null;
	}
	if (!m) return;
	_CoordSystemInfo.InvMatrix = _CoordSystemInfo.Matrix->inverted();
}

///=======================================================================================
void CParticleSystem::setUserMatrix(const NLMISC::CMatrix *m)
{
	NL_PS_FUNC_MAIN(CParticleSystem_setUserMatrix)
	if (!_UserCoordSystemInfo) return; // no process in the system references the user matrix
	CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
	csi.Matrix = m;
	if (_SystemDate == 0.f)
	{
		csi.OldPos = m ? m->getPos() : getSysMat().getPos(); // _CoordSystemInfo.Matrix is relevant if at least one call to setSysMat has been performed before
	}
	if (!m) return;
	csi.InvMatrix = csi.Matrix->inverted();
	// build conversion matrix between father user matrix & fx matrix
	// TODO : lazy evaluation for this ?
	_UserCoordSystemInfo->UserBasisToFXBasis = _CoordSystemInfo.InvMatrix * *(csi.Matrix);
	_UserCoordSystemInfo->FXBasisToUserBasis =  csi.InvMatrix * getSysMat();
}

///=======================================================================================
bool CParticleSystem::hasOpaqueObjects(void) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasOpaqueObjects)
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(*it);
			for (uint k = 0; k < loc->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = loc->getBoundObject(k);
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasOpaqueFaces()) return true;
				}
			}
		}
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasTransparentObjects(void) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasTransparentObjects)
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(*it);
			for (uint k = 0; k < loc->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = loc->getBoundObject(k);
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasTransparentFaces()) return true;
				}
			}
		}
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasLightableObjects() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasLightableObjects)
	if (_ForceGlobalColorLighting) return true;
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(*it);
			for (uint k = 0; k < loc->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = loc->getBoundObject(k);
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasLightableFaces()) return true;
					if (((CPSParticle *) lb)->usesGlobalColorLighting()) return true;
				}
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::getLODVect(NLMISC::CVector &v, float &offset,  TPSMatrixMode matrixMode)
{
	NL_PS_FUNC_MAIN(CParticleSystem_getLODVect)
	switch(matrixMode)
	{
		case PSFXWorldMatrix:
		{
			const CVector tv = getInvertedSysMat().mulVector(_InvertedViewMat.getJ());
			const CVector org = getInvertedSysMat() * _InvertedViewMat.getPos();
			v = _InvCurrentViewDist * tv;
			offset = - org * v;
		}
		break;
		case PSIdentityMatrix:
		{
			v = _InvCurrentViewDist * _InvertedViewMat.getJ();
			offset = - _InvertedViewMat.getPos() * v;
		}
		break;
		case PSUserMatrix:
		{
			const CVector tv = getInvertedUserMatrix().mulVector(_InvertedViewMat.getJ());
			const CVector org = getInvertedUserMatrix() * _InvertedViewMat.getPos();
			v = _InvCurrentViewDist * tv;
			offset = - org * v;
		}
		break;
		default:
			nlassert(0);
		break;
	}
}

///=======================================================================================
TPSLod CParticleSystem::getLOD(void) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getLOD)
	const float dist = fabsf(_InvCurrentViewDist * (getSysMat().getPos() - _InvertedViewMat.getPos()) * _InvertedViewMat.getJ());
	return dist > _LODRatio ? PSLod2 : PSLod1;
}


///=======================================================================================
void CParticleSystem::registerLocatedBindableExternID(uint32 id, CPSLocatedBindable *lb)
{
	NL_PS_FUNC_MAIN(CParticleSystem_registerLocatedBindableExternID)
	nlassert(lb);
	nlassert(lb->getOwner() && lb->getOwner()->getOwner() == this); // the located bindable must belong to that system
	#ifdef NL_DEBUG
		// check that this lb hasn't been inserted yet
		TLBMap::iterator lbd = _LBMap.lower_bound(id), ubd = _LBMap.upper_bound(id);
		nlassert(std::find(lbd, ubd, TLBMap::value_type (id, lb)) == ubd);
		nlassert(std::find(lbd, ubd, TLBMap::value_type (id, lb)) == ubd );

	#endif
		_LBMap.insert(TLBMap::value_type (id, lb) );
}

///=======================================================================================
void CParticleSystem::unregisterLocatedBindableExternID(CPSLocatedBindable *lb)
{
	NL_PS_FUNC_MAIN(CParticleSystem_unregisterLocatedBindableExternID)
	nlassert(lb);
	nlassert(lb->getOwner() && lb->getOwner()->getOwner() == this); // the located bindable must belong to that system
	uint32 id = lb->getExternID();
	if (!id) return;
	TLBMap::iterator lbd = _LBMap.lower_bound(id), ubd = _LBMap.upper_bound(id);
	TLBMap::iterator el = std::find(lbd, ubd, TLBMap::value_type (id, lb));
	nlassert(el != ubd);
	_LBMap.erase(el);
}

///=======================================================================================
uint CParticleSystem::getNumLocatedBindableByExternID(uint32 id) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getNumLocatedBindableByExternID)
	return (uint)_LBMap.count(id);
}

///=======================================================================================
CPSLocatedBindable *CParticleSystem::getLocatedBindableByExternID(uint32 id, uint index)
{
	NL_PS_FUNC_MAIN(CParticleSystem_getLocatedBindableByExternID)
	if (index >= _LBMap.count(id))
	{
		return NULL;
	}
	TLBMap::const_iterator el = _LBMap.lower_bound(id);
	uint left = index;
	while (left--) ++el;
	return  el->second;

}

///=======================================================================================
const CPSLocatedBindable *CParticleSystem::getLocatedBindableByExternID(uint32 id, uint index) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getLocatedBindableByExternID)
	if (index >= _LBMap.count(id))
	{
		return NULL;
	}
	TLBMap::const_iterator el = _LBMap.lower_bound(id);
	uint left = index;
	while (left--) ++el;
	return  el->second;
}

///=======================================================================================
bool CParticleSystem::merge(CParticleSystemShape *pss)
{
	NL_PS_FUNC_MAIN(CParticleSystem_merge)
	nlassert(pss);
	nlassert(_Scene);
	CParticleSystem *duplicate = pss->instanciatePS(*this->_Scene); // duplicate the p.s. to merge
	// now we transfer the located of the duplicated ps to this object...
	for (TProcessVect::iterator it = duplicate->_ProcessVect.begin(); it != duplicate->_ProcessVect.end(); ++it)
	{
		if (!attach(*it))
		{
			for (TProcessVect::iterator clearIt = duplicate->_ProcessVect.begin(); clearIt != it; ++it)
			{
				detach(getIndexOf(**it));
			}
			nlwarning("<CParticleSystem::merge> Can't do the merge : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. Merge is not done.");
			return false;
		}
	}
	//
	if (getBypassMaxNumIntegrationSteps())
	{
		if (!canFinish())
		{
			for (TProcessVect::iterator it = duplicate->_ProcessVect.begin(); it != duplicate->_ProcessVect.end(); ++it)
			{
				detach(getIndexOf(**it));
			}
			nlwarning("<CParticleSystem::merge> Can't do the merge : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. Merge is not done.");
			return false;
		}
	}
	//
	duplicate->_ProcessVect.clear();
	delete duplicate;
	systemDurationChanged();
	CHECK_INTEGRITY
	return true;
}

///=======================================================================================
void CParticleSystem::activatePresetBehaviour(TPresetBehaviour behaviour)
{
	NL_PS_FUNC_MAIN(CParticleSystem_activatePresetBehaviour)
	switch(behaviour)
	{
		case EnvironmentFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimVisible);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case RunningEnvironmentFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimInCluster);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case SpellFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimAlways);
			setBypassMaxNumIntegrationSteps(true);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case LoopingSpellFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			// setAnimType(AnimInCluster); // TODO : AnimAlways could be better ?
			setAnimType(AnimVisible);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case MinorFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(true);
			setAnimType(AnimVisible);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case MovingLoopingFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimVisible);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = true;
		break;
		case SpawnedEnvironmentFX:
			setDestroyModelWhenOutOfRange(true);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimAlways);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = false;
		break;
		case GroundFX:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(none);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimAlways);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = true;
		break;
		case Projectile:
			setDestroyModelWhenOutOfRange(false);
			setDestroyCondition(noMoreParticles);
			destroyWhenOutOfFrustum(false);
			setAnimType(AnimVisible);
			setBypassMaxNumIntegrationSteps(false);
			_KeepEllapsedTimeForLifeUpdate = true;
		break;
		default: break;
	}
	_PresetBehaviour = behaviour;
}


///=======================================================================================
CParticleSystemProcess *CParticleSystem::detach(uint index)
{
	NL_PS_FUNC_MAIN(CParticleSystem_detach)
	nlassert(index < _ProcessVect.size());
	CParticleSystemProcess *proc = _ProcessVect[index];
	// release references other process may have to this system
	for(TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		(*it)->releaseRefTo(proc);
	}
	// erase from the vector
	_ProcessVect.erase(_ProcessVect.begin() + index);
	proc->setOwner(NULL);
	//
	systemDurationChanged();
	// not part of this system any more
	return proc;
}

///=======================================================================================
bool CParticleSystem::isProcess(const CParticleSystemProcess *process) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_isProcess)
	for(TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if (*it == process) return true;
	}
	return false;
}

///=======================================================================================
uint CParticleSystem::getIndexOf(const CParticleSystemProcess &process) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getIndexOf)
	#ifdef NL_DEBUG
		nlassert(isProcess(&process));
	#endif
		return process.getIndex();
}

///=======================================================================================
uint CParticleSystem::getNumID() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getNumID)
	return (uint)_LBMap.size();
}

///=======================================================================================
uint32 CParticleSystem::getID(uint index) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getID)
	TLBMap::const_iterator it = _LBMap.begin();
	for(uint k = 0; k < index; ++k)
	{
		if (it == _LBMap.end()) return 0;
		++it;
	}
	return it->first;
}

///=======================================================================================
void CParticleSystem::getIDs(std::vector<uint32> &dest) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getIDs)
	dest.resize(_LBMap.size());
	uint k = 0;
	for(TLBMap::const_iterator it = _LBMap.begin(); it != _LBMap.end(); ++it)
	{
		dest[k] = it->first;
		++k;
	}
}

///=======================================================================================
void CParticleSystem::interpolateFXPosDelta(NLMISC::CVector &dest, TAnimationTime deltaT)
{
	NL_PS_FUNC_MAIN(CParticleSystem_interpolateFXPosDelta)
	nlassert(_CoordSystemInfo.Matrix);
	dest = _CoordSystemInfo.CurrentDeltaPos - (deltaT * InverseTotalEllapsedTime) * (_CoordSystemInfo.Matrix->getPos() - _CoordSystemInfo.OldPos);
}

///=======================================================================================
void CParticleSystem::interpolateUserPosDelta(NLMISC::CVector &dest, TAnimationTime deltaT)
{
	NL_PS_FUNC_MAIN(CParticleSystem_interpolateUserPosDelta)
	if (!_UserCoordSystemInfo)
	{
		interpolateFXPosDelta(dest, deltaT);
	}
	else
	{
		CCoordSystemInfo &csi = _UserCoordSystemInfo->CoordSystemInfo;
		dest = csi.CurrentDeltaPos - (deltaT * InverseTotalEllapsedTime) * (csi.Matrix->getPos() - csi.OldPos);
	}
}

///=======================================================================================
void CParticleSystem::bindGlobalValueToUserParam(const std::string &globalValueName, uint userParamIndex)
{
	NL_PS_FUNC_MAIN(CParticleSystem_bindGlobalValueToUserParam)
	nlassert(userParamIndex < MaxPSUserParam);
	if (globalValueName.empty()) // disable a user param global value
	{
		if (!_UserParamGlobalValue) return;
		_UserParamGlobalValue[userParamIndex] = NULL;
		for(uint k = 0; k < MaxPSUserParam; ++k)
		{
			if (_UserParamGlobalValue[k] != NULL) return;
		}
		// no more entry used
		delete _UserParamGlobalValue;
		_UserParamGlobalValue = NULL;
	}
	else // enable a user param global value
	{
		if (!_UserParamGlobalValue)
		{
			// no table has been allocated yet, so create one
			_UserParamGlobalValue = new const TGlobalValuesMap::value_type *[MaxPSUserParam];
			std::fill(_UserParamGlobalValue, _UserParamGlobalValue + MaxPSUserParam, (TGlobalValuesMap::value_type *) NULL);
		}
		// has the global value be created yet ?
		TGlobalValuesMap::const_iterator it = _GlobalValuesMap.find(globalValueName);
		if (it != _GlobalValuesMap.end())
		{
			// yes, make a reference on it
			_UserParamGlobalValue[userParamIndex] = &(*it);
		}
		else
		{
			// create a new entry
			std::pair<TGlobalValuesMap::iterator, bool> itPair = _GlobalValuesMap.insert(TGlobalValuesMap::value_type(globalValueName, 0.f));
			_UserParamGlobalValue[userParamIndex] = &(*(itPair.first));
		}
	}
}

///=======================================================================================
void CParticleSystem::setGlobalValue(const std::string &name, float value)
{
	NL_PS_FUNC(CParticleSystem_setGlobalValue)
	nlassert(!name.empty());
	NLMISC::clamp(value, 0.f, 1.f);
	_GlobalValuesMap[name] = value;
}

///=======================================================================================
float CParticleSystem::getGlobalValue(const std::string &name)
{
	NL_PS_FUNC(CParticleSystem_getGlobalValue)
	TGlobalValuesMap::const_iterator it = _GlobalValuesMap.find(name);
	if (it != _GlobalValuesMap.end()) return it->second;
	return 0.f; // not a known value
}

///=======================================================================================
std::string CParticleSystem::getGlobalValueName(uint userParamIndex) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getGlobalValueName)
	nlassert(userParamIndex < MaxPSUserParam);
	if (!_UserParamGlobalValue) return "";
	if (!_UserParamGlobalValue[userParamIndex]) return "";
	return _UserParamGlobalValue[userParamIndex]->first;
}

///=======================================================================================
void CParticleSystem::setGlobalVectorValue(const std::string &name, const NLMISC::CVector &value)
{
	NL_PS_FUNC(CParticleSystem_setGlobalVectorValue)
	nlassert(!name.empty());
	_GlobalVectorValuesMap[name] = value;
}


///=======================================================================================
NLMISC::CVector CParticleSystem::getGlobalVectorValue(const std::string &name)
{
	NL_PS_FUNC(CParticleSystem_getGlobalVectorValue)
	nlassert(!name.empty());
	TGlobalVectorValuesMap::const_iterator it = _GlobalVectorValuesMap.find(name);
	if (it != _GlobalVectorValuesMap.end()) return it->second;
	return NLMISC::CVector::Null; // not a known value
}

///=======================================================================================
CParticleSystem::CGlobalVectorValueHandle CParticleSystem::getGlobalVectorValueHandle(const std::string &name)
{
	NL_PS_FUNC(CParticleSystem_getGlobalVectorValueHandle)
	nlassert(!name.empty());
	TGlobalVectorValuesMap::iterator it = _GlobalVectorValuesMap.find(name);
	if (it == _GlobalVectorValuesMap.end())
	{
		it = _GlobalVectorValuesMap.insert(TGlobalVectorValuesMap::value_type(name, NLMISC::CVector::Null)).first;
	}
	CGlobalVectorValueHandle handle;
	handle._Value = &it->second;
	handle._Name = &it->first;
	return handle;
}

///=======================================================================================
void CParticleSystem::setMaxDistLODBias(float lodBias)
{
	NL_PS_FUNC_MAIN(CParticleSystem_setMaxDistLODBias)
	NLMISC::clamp(lodBias, 0.f, 1.f);
	_MaxDistLODBias = lodBias;
}

///=======================================================================================
bool CParticleSystem::canFinish(CPSLocatedBindable **lastingForeverObj /*= NULL*/) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_canFinish)
	if (hasLoop(lastingForeverObj)) return false;
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		if (_ProcessVect[k]->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(_ProcessVect[k]);
			if (loc->getLastForever())
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					CPSEmitter *em = dynamic_cast<CPSEmitter *>(loc->getBoundObject(l));
					if (em && em->testEmitForever())
					{
						if (lastingForeverObj) *lastingForeverObj = em;
						return false;
					}
					CPSParticle *p = dynamic_cast<CPSParticle *>(loc->getBoundObject(l));
					if (p)
					{
						if (lastingForeverObj) *lastingForeverObj = p;
						return false; // particles shouldn't live forever, too
					}
				}
			}
		}
	}
	return true;
}

///=======================================================================================
bool CParticleSystem::hasLoop(CPSLocatedBindable **loopingObj /*= NULL*/) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasLoop)
	// we want to check for loop like A emit B emit A
	// NB : there's room for a smarter algo here, but should not be useful for now
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		if (_ProcessVect[k]->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(_ProcessVect[k]);
			for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
			{
				CPSEmitter *em = dynamic_cast<CPSEmitter *>(loc->getBoundObject(l));
				if (em)
				{
					if (em->checkLoop())
					{
						if (loopingObj) *loopingObj = em;
						return true;
					}
				}
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::systemDurationChanged()
{
	NL_PS_FUNC_MAIN(CParticleSystem_systemDurationChanged)
	if (getAutoComputeDelayBeforeDeathConditionTest())
	{
		setDelayBeforeDeathConditionTest(-1.f);
	}
}

///=======================================================================================
void CParticleSystem::setAutoComputeDelayBeforeDeathConditionTest(bool computeAuto)
{
	NL_PS_FUNC_MAIN(CParticleSystem_setAutoComputeDelayBeforeDeathConditionTest)
	if (computeAuto == _AutoComputeDelayBeforeDeathTest) return;
	_AutoComputeDelayBeforeDeathTest = computeAuto;
	if (computeAuto) setDelayBeforeDeathConditionTest(-1.f);
}

///=======================================================================================
TAnimationTime CParticleSystem::getDelayBeforeDeathConditionTest() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getDelayBeforeDeathConditionTest)
	if (_DelayBeforeDieTest < 0.f)
	{
		_DelayBeforeDieTest = evalDuration();
	}
	return std::max(PS_MIN_TIMEOUT, _DelayBeforeDieTest);
}

///=======================================================================================
// struct to eval duration of an emitter chain
struct CToVisitEmitter
{
	float		Duration; // cumuled duration of this emitter parent emitters
	const CPSLocated *Located;
};

///=======================================================================================
float CParticleSystem::evalDuration() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_evalDuration)
	std::vector<const CPSLocated *> visitedEmitter;
	std::vector<CToVisitEmitter> toVisitEmitter;
	float maxDuration = 0.f;
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		if (_ProcessVect[k]->isLocated())
		{
			bool emitterFound = false;
			const CPSLocated *loc = static_cast<const CPSLocated *>(_ProcessVect[k]);
			for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
			{
				const CPSLocatedBindable *bind = loc->getBoundObject(l);
				if (loc->getSize() > 0)
				{
					switch(bind->getType())
					{
						case  PSParticle:
						{
							if (loc->getLastForever())
							{
								return -1;
							}
							else
							{
								maxDuration = std::max(maxDuration, loc->evalMaxDuration());
							}
						}
						break;
						case PSEmitter:
						{
							if (!emitterFound)
							{
								CToVisitEmitter tve;
								tve.Located = loc;
								tve.Duration = 0.f;
								toVisitEmitter.push_back(tve);
								emitterFound = true;
							}
						}
						break;
					}
				}
			}
			visitedEmitter.clear();
			while (!toVisitEmitter.empty())
			{
				const CPSLocated *loc = toVisitEmitter.back().Located;
				float duration = toVisitEmitter.back().Duration;
				toVisitEmitter.pop_back();
				visitedEmitter.push_back(loc);
				bool emitterFound = false;
				for(uint m = 0; m < loc->getNbBoundObjects(); ++m)
				{
					const CPSLocatedBindable *bind = loc->getBoundObject(m);
					if (bind->getType() == PSEmitter)
					{
						const CPSEmitter *em = NLMISC::safe_cast<const CPSEmitter *>(loc->getBoundObject(m));
						const CPSLocated *emittedType = em->getEmittedType();
						// continue if there's no loop
						if (std::find(visitedEmitter.begin(), visitedEmitter.end(), emittedType) == visitedEmitter.end())
						{
							if (emittedType != NULL)
							{
								emitterFound = true;
								CToVisitEmitter tve;
								tve.Located = emittedType;
								// if emitter has limited lifetime, use it
								if (!loc->getLastForever())
								{
									tve.Duration = duration + loc->evalMaxDuration();
								}
								else
								{
									// try to eval duration depending on type
									switch(em->getEmissionType())
									{
										case CPSEmitter::regular:
										{
											if (em->getMaxEmissionCount() != 0)
											{
												float period = em->getPeriodScheme() ? em->getPeriodScheme()->getMaxValue() : em->getPeriod();
												tve.Duration = duration + em->getEmitDelay() + 	period * em->getMaxEmissionCount();
											}
											else
											{
												tve.Duration = duration + em->getEmitDelay();
											}
										}
										break;
										case CPSEmitter::onDeath:
										case CPSEmitter::once:
										case CPSEmitter::onBounce:
										case CPSEmitter::externEmit:
											tve.Duration = duration; // can't eval duration ..
										break;
										default:
											break;
									}
								}
								toVisitEmitter.push_back(tve);
							}
						}
					}
				}
				if (!emitterFound)
				{
					if (!loc->getLastForever())
					{
						duration += loc->evalMaxDuration();
					}
					maxDuration = std::max(maxDuration, duration);
				}
			}
		}
	}
	return maxDuration;
}

///=======================================================================================
bool CParticleSystem::isDestroyConditionVerified() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_isDestroyConditionVerified)
	if (getDestroyCondition() != CParticleSystem::none)
	{
		if (getSystemDate() > getDelayBeforeDeathConditionTest())
		{
			switch (getDestroyCondition())
			{
				case CParticleSystem::noMoreParticles: return !hasParticles();
				case CParticleSystem::noMoreParticlesAndEmitters: return !hasParticles() && !hasEmitters();
				default: nlassert(0); return false;
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::setSystemDate(float date)
{
	NL_PS_FUNC_MAIN(CParticleSystem_setSystemDate)
	if (date == _SystemDate) return;
	_SystemDate = date;
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		_ProcessVect[k]->systemDateChanged();
	}
}

///=======================================================================================
void CParticleSystem::registerSoundServer(UPSSoundServer *soundServer)
{
	NL_PS_FUNC(CParticleSystem_registerSoundServer)
	if (soundServer == _SoundServer) return;
	if (_SoundServer)
	{
		CParticleSystemManager::stopSoundForAllManagers();
	}
	_SoundServer = soundServer;
	if (_SoundServer)
	{
		CParticleSystemManager::reactivateSoundForAllManagers();
	}
}

///=======================================================================================
void CParticleSystem::activateEmitters(bool active)
{
	NL_PS_FUNC_MAIN(CParticleSystem_activateEmitters)
	for(uint k = 0; k < getNbProcess(); ++k)
	{
		if (getProcess(k)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSEmitter)
						loc->getBoundObject(l)->setActive(active);
				}
			}
		}
	}
}

///=======================================================================================
bool CParticleSystem::hasActiveEmitters() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasActiveEmitters)
	for(uint k = 0; k < getNbProcess(); ++k)
	{
		if (getProcess(k)->isLocated())
		{
			const CPSLocated *loc = static_cast<const CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSEmitter)
					{
						if (loc->getBoundObject(l)->isActive()) return true;
					}
				}
			}
		}
	}
	return false;
}

///=======================================================================================
bool CParticleSystem::hasEmittersTemplates() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_hasEmittersTemplates)
	for(uint k = 0; k < getNbProcess(); ++k)
	{
		if (getProcess(k)->isLocated())
		{
			const CPSLocated *loc = static_cast<const CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSEmitter)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

///=======================================================================================
void CParticleSystem::matchArraySize()
{
	NL_PS_FUNC_MAIN(CParticleSystem_matchArraySize)
	for(uint k = 0; k < getNbProcess(); ++k)
	{
		if (getProcess(k)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(getProcess(k));
			loc->resize(loc->getSize()); // match the max size with the number of instances
		}
	}
}

///=======================================================================================
uint CParticleSystem::getMaxNumParticles() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getMaxNumParticles)
	uint numParts = 0;
	for(uint k = 0; k < getNbProcess(); ++k)
	{
		if (getProcess(k)->isLocated())
		{
			const CPSLocated *loc = static_cast<const CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSParticle)
					{
						numParts += loc->getMaxSize();
					}
				}
			}
		}
	}
	return numParts;
}

///=======================================================================================
uint CParticleSystem::getCurrNumParticles() const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getCurrNumParticles)
	uint numParts = 0;
	for(uint k = 0; k < getNbProcess(); ++k)
	{
		if (getProcess(k)->isLocated())
		{
			const CPSLocated *loc = static_cast<const CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					if (loc->getBoundObject(l)->getType() == PSParticle)
					{
						numParts += loc->getSize();
					}
				}
			}
		}
	}
	return numParts;
}

///=======================================================================================
void CParticleSystem::getTargeters(const CPSLocated *target, std::vector<CPSTargetLocatedBindable *> &targeters)
{
	NL_PS_FUNC_MAIN(CParticleSystem_getTargeters)
	nlassert(target);
	nlassert(isProcess(target));
	targeters.clear();
	for(uint k = 0; k < getNbProcess(); ++k)
	{
		if (getProcess(k)->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(getProcess(k));
			if (loc)
			{
				for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
				{
					CPSTargetLocatedBindable *targeter = dynamic_cast<CPSTargetLocatedBindable *>(loc->getBoundObject(l));
					if (targeter)
					{
						for(uint m = 0; m < targeter->getNbTargets(); ++m)
						{
							if (targeter->getTarget(m) == target)
							{
								targeters.push_back(targeter);
								break;
							}
						}
					}
				}
			}
		}
	}
}

///=======================================================================================
void CParticleSystem::matrixModeChanged(CParticleSystemProcess *proc, TPSMatrixMode oldMode, TPSMatrixMode newMode)
{
	NL_PS_FUNC_MAIN(CParticleSystem_matrixModeChanged)
	nlassert(proc);
	// check that the located belong to that system
	nlassert(isProcess(proc));
	if (oldMode != PSUserMatrix && newMode == PSUserMatrix)
	{
		addRefForUserSysCoordInfo();
	}
	else if (oldMode == PSUserMatrix && newMode != PSUserMatrix)
	{
		releaseRefForUserSysCoordInfo();
	}
}

///=======================================================================================
void CParticleSystem::addRefForUserSysCoordInfo(uint numRefs)
{
	NL_PS_FUNC_MAIN(CParticleSystem_addRefForUserSysCoordInfo)
	if (!numRefs) return;
	if (!_UserCoordSystemInfo)
	{
		_UserCoordSystemInfo = new CUserCoordSystemInfo;
	}
	nlassert(_UserCoordSystemInfo);
	_UserCoordSystemInfo->NumRef += numRefs;

}

///=======================================================================================
void CParticleSystem::releaseRefForUserSysCoordInfo(uint numRefs)
{
	NL_PS_FUNC_MAIN(CParticleSystem_releaseRefForUserSysCoordInfo)
	if (!numRefs) return;
	nlassert(_UserCoordSystemInfo);
	nlassert(numRefs <= _UserCoordSystemInfo->NumRef);
	_UserCoordSystemInfo->NumRef -= numRefs;
	if (_UserCoordSystemInfo->NumRef == 0)
	{
		delete _UserCoordSystemInfo;
		_UserCoordSystemInfo = NULL;
	}
}

///=======================================================================================
void CParticleSystem::checkIntegrity()
{
	NL_PS_FUNC_MAIN(CParticleSystem_checkIntegrity)
	// do some checks
	uint userMatrixUsageCount = 0;
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		userMatrixUsageCount += (*it)->getUserMatrixUsageCount();
	}
	if (userMatrixUsageCount == 0)
	{
		nlassert(_UserCoordSystemInfo == NULL);
	}
	else
	{
		nlassert(_UserCoordSystemInfo != NULL);
		nlassert(_UserCoordSystemInfo->NumRef == userMatrixUsageCount);
	}
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		nlassert(_ProcessVect[k]->getOwner() == this);
	}
}

///=======================================================================================
void CParticleSystem::enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv)
{
	NL_PS_FUNC_MAIN(CParticleSystem_enumTexs)
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		(*it)->enumTexs(dest, drv);
	}
}

///=======================================================================================
void CParticleSystem::setZBias(float value)
{
	NL_PS_FUNC_MAIN(CParticleSystem_setZBias)
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		(*it)->setZBias(value);
	}
}

///=======================================================================================
void CParticleSystem::getSortingByEmitterPrecedence(std::vector<uint> &result) const
{
	NL_PS_FUNC_MAIN(CParticleSystem_getSortingByEmitterPrecedence)
	#ifdef NL_DEBUG
		nlassert(!hasLoop()); // should be an acyclic graph, otherwise big problem....
	#endif
	typedef std::list<CParticleSystemProcess *> TProcessList;
	std::vector<TProcessList> degreeToNodes;
	std::vector<TProcessList::iterator> nodeToIterator(_ProcessVect.size());
	//
	std::vector<uint> inDegree(_ProcessVect.size(), 0); // degree for each node
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		if (_ProcessVect[k]->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(_ProcessVect[k]);
			for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
			{
				if (loc->getBoundObject(l)->getType() == PSEmitter)
				{
					CPSEmitter *pEmit = NLMISC::safe_cast<CPSEmitter *>(loc->getBoundObject(l));
					if (pEmit->getEmittedType())
					{
						++ inDegree[getIndexOf(*pEmit->getEmittedType())];
					}
				}
			}
		}
	}
	// make enough room in degreeToNodes.
	for(uint k = 0; k < inDegree.size(); ++k)
	{
		if (degreeToNodes.size() <= inDegree[k])
		{
			degreeToNodes.resize(inDegree[k] + 1);
		}
	}
	// sort nodes by degree
	for(uint k = 0; k < inDegree.size(); ++k)
	{
		// NB : could not do resize there because we keep iterators in the list, so it's done in the previous loop
		degreeToNodes[inDegree[k]].push_front(_ProcessVect[k]);
		nodeToIterator[k] = degreeToNodes[inDegree[k]].begin();
	}
	//
	#ifdef  NL_DEBUG
		#define DO_SBEP_CHECK  \
		{  for(uint k = 0; k < degreeToNodes.size(); ++k)	                                                         \
		{                                                                                                            \
			for(TProcessList::const_iterator it = degreeToNodes[k].begin(); it != degreeToNodes[k].end(); ++it)      \
			{                                                                                                        \
				nlassert(inDegree[(*it)->getIndex()] == k);                                                          \
			}			                                                                                             \
		}}
	#else
		#define DO_SBEP_CHECK
	#endif
	//
	DO_SBEP_CHECK
	result.reserve(_ProcessVect.size());
	result.clear();
	if (degreeToNodes.empty()) return;
	// now, do the sort -> add each node with a degree of 0, and removes arc to their son (and insert in new good list according to their degree)
	while (!degreeToNodes[0].empty())
	{
		DO_SBEP_CHECK
		CParticleSystemProcess *pr = degreeToNodes[0].front();
		degreeToNodes[0].pop_front();
		result.push_back(getIndexOf(*pr));
		if (pr->isLocated())
		{
			CPSLocated *loc = static_cast<CPSLocated *>(pr);
			for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
			{
				if (loc->getBoundObject(l)->getType() == PSEmitter)
				{
					CPSEmitter *pEmit = NLMISC::safe_cast<CPSEmitter *>(loc->getBoundObject(l));
					// update degree of node
					if (pEmit->getEmittedType())
					{
						uint emittedLocIndex  = getIndexOf(*pEmit->getEmittedType());
						uint degree = inDegree[emittedLocIndex];
						nlassert(degree != 0);
						degreeToNodes[degree - 1].splice(degreeToNodes[degree - 1].begin(), degreeToNodes[degree], nodeToIterator[emittedLocIndex]);
						nodeToIterator[emittedLocIndex] = degreeToNodes[degree - 1].begin(); // update iterator
						-- inDegree[emittedLocIndex];
						DO_SBEP_CHECK
					}
				}
			}
		}
	}
}

///=======================================================================================
void CParticleSystem::updateProcessIndices()
{
	NL_PS_FUNC_MAIN(CParticleSystem_updateProcessIndices)
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		_ProcessVect[k]->setIndex(k);
	}
}


///=======================================================================================
void CParticleSystem::dumpHierarchy()
{
	NL_PS_FUNC_MAIN(CParticleSystem_dumpHierarchy)
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		CPSLocated *loc = dynamic_cast<CPSLocated *>(_ProcessVect[k]);
		if (loc)
		{
			nlinfo("Located k : %s @%p", loc->getName().c_str(), loc);
			for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
			{
				CPSEmitter *emitter = dynamic_cast<CPSEmitter *>(loc->getBoundObject(l));
				if (emitter)
				{
					nlinfo("    emitter %s : emit %s @%p", emitter->getName().c_str(), emitter->getEmittedType() ? emitter->getEmittedType()->getName().c_str() : "none", emitter->getEmittedType());
				}
			}
		}
	}
}

///=======================================================================================
void CParticleSystem::onShow(bool shown)
{
	NL_PS_FUNC_MAIN(CParticleSystem_onShow)
	for(uint k = 0; k < _ProcessVect.size(); ++k)
	{
		_ProcessVect[k]->onShow(shown);
	}
}


} // NL3D
