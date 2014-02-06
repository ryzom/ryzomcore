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
#include "timed_fx_manager.h"
#include "ig_callback.h"
#include "client_sheets/plant_sheet.h"
#include "nel/3d/u_transform.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_camera.h"
#include "nel/misc/fast_floor.h"
#include "misc.h"
#include "fx_manager.h"
#include "nel/misc/check_fpu.h"


#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
	#include <crtdbg.h>
#endif

using namespace std::rel_ops;
using namespace NLMISC;

extern NL3D::UTextContext		*TextContext;
extern NL3D::UDriver			*Driver;
extern NL3D::UScene				*Scene;



//#define	DEBUG_FX

#ifdef DEBUG_FX
	#ifndef NL_DEBUG
		#error Flag DEBUG_FX can only be used in debug mode
	#endif
#endif

#ifdef DEBUG_FX
	#define CHECK_INTEGRITY checkIntegrity();
#else
	#define CHECK_INTEGRITY
#endif

CTimedFXManager &CTimedFXManager::getInstance()
{
	FPU_CHECKER
	static CTimedFXManager manager;
	return manager;
}

const float DEFAULT_SPAWNED_FX_TIMED_OUT = 100.f;

H_AUTO_DECL(RZ_TimedFX)


// *******************************************************************************************
NLMISC::CMatrix CTimedFX::getInstanceMatrix() const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	NLMISC::CMatrix result;
	result.identity();
	result.translate(SpawnPosition);
	if (FXSheet && FXSheet->InheritRot) result.rotate(Rot);
	if (FXSheet && FXSheet->InheritScale) result.scale(Scale);
	return result;
}


// *******************************************************************************************
CTimedFXManager::CTimedFXManager()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	_Scene = NULL,
	_DayLength = 24.f,
	_InitDone = false;
	_InstanciatedFXs = NULL;
	_CandidateFXListTouched = false;
	_SortDistance = 0.f;
	_MaxNumberOfFXInstances = 0;
}

// *******************************************************************************************
CTimedFXManager::~CTimedFXManager()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	reset();
}

// *******************************************************************************************
void CTimedFXManager::init(NL3D::UScene *scene,
						   const CClientDate &startDate,
						   float /* dayLength */,
						   float noiseFrequency,
						   uint  maxNumberOfFXInstances,
						   float sortDistanceInterval,
						   float maxDist)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(!_InitDone);
	if (!scene)
	{
		nlwarning("Scene is NULL");
	}
	_Scene	  = scene;
	_CurrDate = startDate;
	_Noise.Frequency = noiseFrequency;
	_Noise.Abs  = 0.f;
	_Noise.Rand = 1.f;
	CHECK_INTEGRITY
	_InitDone = true;
	nlassert(maxDist > 0.f);
	nlassert(sortDistanceInterval > 0.f);
	_CandidateFXListSortedByDist.resize((int) ceilf(maxDist / sortDistanceInterval));
	_CandidateFXListSortedByDistTmp.resize((int) ceilf(maxDist / sortDistanceInterval));
	_MaxNumberOfFXInstances = maxNumberOfFXInstances;
	_SortDistance = sortDistanceInterval;
}


// *******************************************************************************************
void CTimedFXManager::reset()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (_InitDone)
	{
		nlassert(_InitDone);
		CHECK_INTEGRITY
		while (!_FXGroups.empty())
		{
			remove(_FXGroups.begin());
		}
		_Scene = NULL;
		_CurrDate = CClientDate();
		_InitDone = false;
		_FXManager.reset();
	}
}

// *******************************************************************************************
void CTimedFXManager::setDate(const CClientDate &date)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	_CurrDate = date;
}

// *******************************************************************************************
CTimedFXManager::TFXGroupHandle CTimedFXManager::add(const std::vector<CTimedFX> &fxs, EGSPD::CSeason::TSeason /* season */)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(_InitDone);
	CHECK_INTEGRITY
	_FXGroups.push_front(CManagedFXGroup());
	#ifdef DEBUG_FX
		_FXGroups.front().Season = season;
	#endif
	TManagedFXGroup &newGroup = _FXGroups.begin()->Group;
	newGroup.resize(fxs.size());
	for(uint k = 0; k < fxs.size(); ++k)
	{
		CManagedFX &fi = newGroup[k];
		fi.FXSheet			= fxs[k].FXSheet;
		fi.SpawnPosition	= fxs[k].SpawnPosition;
		fi.Scale			= fxs[k].Scale;
		fi.Rot				= fxs[k].Rot;
		fi.Instance         = NULL;
#if !FINAL_VERSION
			fi.FromIG = fxs[k].FromIG;
		#endif
		#ifdef DEBUG_FX
			fi.OwnerGroup = &(_FXGroups.front());
		#endif
		// compute start and end hour to see in which list insertion should occurs
		bool instanciate = false;
		CClientDate startDate;
		CClientDate endDate;
		//sint32 debugDay;
		if (!(fi.FXSheet && fi.FXSheet->Mode == CSeasonFXSheet::AlwaysStarted))
		{
			if (fi.FXSheet->Mode == CSeasonFXSheet::Spawn)
			{
				// compute next spawn date
				float cycleLength = fi.FXSheet ? fi.FXSheet->CycleDuration : _DayLength;
				sint32 cycle = dateToCycle(_CurrDate, cycleLength, _DayLength);
				fi.computeStartHour(cycle - 1, startDate, cycleLength, _DayLength, _Noise);
				if (startDate < _CurrDate)
				{
					fi.computeStartHour(cycle, startDate, cycleLength, _DayLength, _Noise);
					if (startDate < _CurrDate)
					{
						fi.computeStartHour(cycle + 1, startDate, cycleLength, _DayLength, _Noise);
					}
				}
			}
			else
			{
				// the system is not always instanciated, so must check if it currently is.
				float cycleLength = fi.FXSheet ? fi.FXSheet->CycleDuration : _DayLength;
				sint32 cycle = dateToCycle(_CurrDate, cycleLength, _DayLength);
				fi.computeStartHour(cycle, startDate, cycleLength, _DayLength, _Noise);
				//debugDay = _CurrDate.Day;
				if (startDate > _CurrDate)
				{
					// Let see if it did not start the previous cycle and then ended that cycle
					fi.computeEndHour(cycle - 1, endDate, cycleLength, _DayLength, _Noise);
					if (endDate > _CurrDate)
					{
						CClientDate prevStartDate;
						fi.computeStartHour(cycle - 1, prevStartDate, cycleLength, _DayLength, _Noise);
						if (prevStartDate <= _CurrDate)
						{
							instanciate = true;
						}
						else
						{
							startDate = prevStartDate;
							//debugDay = _CurrDate.Day - 1;
						}
					}
				}
				else
				{
					fi.computeEndHour(cycle, endDate, cycleLength, _DayLength, _Noise);
					if (endDate > _CurrDate)
					{
						// the fx is currently running
						instanciate = true;
					}
					else
					{
						fi.computeStartHour(_CurrDate.Day + 1, endDate, cycleLength, _DayLength, _Noise);
						// the fx will start only during the next day
						//debugDay = _CurrDate.Day + 1;
					}
				}
			}
		}
		else
		{
			if (fi.FXSheet && fi.FXSheet->Mode == CSeasonFXSheet::AlwaysStarted)
			{
				fi.State = CManagedFX::Permanent;
				instanciate = true;
			}
		}
		if (instanciate)
		{
			if (!_Scene || !fi.FXSheet)
			{
				fi.Instance = NULL;
			}
			else
			{
				// insert in candidate fx list
				float dist = (fi.SpawnPosition - _LastCamPos).norm();
				linkCandidateFX(_CandidateFXListSortedByDist, dist, &fi);
				_CandidateFXListTouched = true;
			}
			// Add the system to the queue of systems to be removed, only if it is not always started
			if (!(fi.FXSheet && fi.FXSheet->Mode == CSeasonFXSheet::AlwaysStarted))
			{
				// As the system is currenlty instanciated, we won't deals with it again until it shutdowns, so, we save it in the priority queue.
				CTimeStampedFX tsf;
				tsf.Date = endDate;
				//tsf.DebugDay = 666;
				tsf.FX = &fi; // yes, we keep a pointer on a vector element, but we will never grow that vector
				fi.SetHandle = _FXToRemove.insert(tsf).first;
				fi.State = CManagedFX::InRemoveList;
			}
		}
		else
		{
			// The system is not instanciated yet, but will be later, so add to the queue of systems that "will be instanciated later"
			CTimeStampedFX tsf;
			tsf.Date = startDate;
			tsf.FX = &fi; // yes, we keep a pointer on a vector element, but we will never grow that vector
			//tsf.DebugDay = debugDay;
			nlassert(fi.State != CManagedFX::InAddList);
			fi.SetHandle = _FXToAdd.insert(tsf).first;
			fi.State = CManagedFX::InAddList;
			#ifdef DEBUG_FX
				nlinfo("==== added fx %s, start day = %day, start hour = %f, pos=(%f, %f, %f)", fi.FXSheet ? fi.FXSheet->FXName.c_str() : "???", (int) startDate.Day, startDate.Hour,
					   fi.SpawnPosition.x, fi.SpawnPosition.y, fi.SpawnPosition.z);
			#endif
		}
	}
	CHECK_INTEGRITY
	return _FXGroups.begin();
}



// *******************************************************************************************
void CTimedFXManager::remove(TFXGroupHandle handle)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(_InitDone);
	CHECK_INTEGRITY
	TManagedFXGroup &group = handle->Group;
	for(uint k = 0; k < group.size(); ++k)
	{
		CManagedFX &fi = group[k];
		if (!fi.Instance.empty())
		{
			nlassert(_Scene);
			_Scene->deleteInstance(fi.Instance);
		}
		switch(fi.State)
		{
			case CManagedFX::InAddList:	   _FXToAdd.erase(fi.SetHandle); break;
			case CManagedFX::InRemoveList: _FXToRemove.erase(fi.SetHandle); break;
			default: break;
		}
		fi.unlinkFromCandidateFXList();
		fi.unlinkFromInstanciatedFXList();
	}
	_FXGroups.erase(handle);
	CHECK_INTEGRITY
}


// *******************************************************************************************
void CTimedFXManager::update(const CClientDate &date, EGSPD::CSeason::TSeason /* season */, const NLMISC::CVector &camPos)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(_InitDone);
	#ifdef DEBUG_FX
		#ifdef NL_OS_WINDOWS
			_CrtCheckMemory();
		#endif
	#endif
	CHECK_INTEGRITY

	_FXManager.update();

	// check for instanciated fxs that should be removed
	while(!_FXToRemove.empty())
	{

		const CTimeStampedFX &tsf = *_FXToRemove.begin();
		#ifdef DEBUG_FX
			nlassert(tsf.FX->OwnerGroup);
			//nlassert(tsf.FX->OwnerGroup->Season == season);
		#endif
		nlassert(tsf.FX->State == CManagedFX::InRemoveList);
		if (tsf.Date > date) break; // all following fx will be remove in the future
		// removes from candidate & instanciated list (this causes fx to be shutdown at the end of the pass)
		tsf.FX->unlinkFromCandidateFXList();
		_CandidateFXListTouched = true;

		// compute new activation date, and insert in _FXToAdd queue
		CTimeStampedFX newTsf;
		newTsf.FX = tsf.FX;
		float cycleLength = newTsf.FX->FXSheet ? newTsf.FX->FXSheet->CycleDuration : _DayLength;
		sint32 cycle = dateToCycle(tsf.Date, cycleLength, _DayLength);
		tsf.FX->computeStartHour(cycle, newTsf.Date, cycleLength, _DayLength, _Noise);
		if (newTsf.Date <= tsf.Date)
		{
			// already started for that day, so compute start hour for next day.
			tsf.FX->computeStartHour(cycle + 1, newTsf.Date, cycleLength, _DayLength, _Noise);
		}
		tsf.FX->SetHandle = _FXToAdd.insert(newTsf).first;
		tsf.FX->State = CManagedFX::InAddList;
		// remove from current queue
		_FXToRemove.erase(_FXToRemove.begin());
	}

	// check for fxs that should be instanciated
	while (!_FXToAdd.empty())
	{

		const CTimeStampedFX &tsf = *_FXToAdd.begin();
		#ifdef DEBUG_FX
			nlassert(tsf.FX->OwnerGroup);
			//nlassert(tsf.FX->OwnerGroup->Season == season);
		#endif
		nlassert(tsf.FX->State == CManagedFX::InAddList);
		if (tsf.Date >= date) break; // all following fx will be instancied in the future
		// activate current fx
		if (!_Scene || !tsf.FX->FXSheet)
		{
			tsf.FX->Instance = NULL;
		}
		else
		{
			// insert in candidate fx list
			float dist = (tsf.FX->SpawnPosition - _LastCamPos).norm();
			linkCandidateFX(_CandidateFXListSortedByDist, dist, tsf.FX);
			_CandidateFXListTouched = true;
		}
		// compute next shutdown date
		#ifdef DEBUG_FX
			nlinfo("===== compute end date for %s, debug day = %d", tsf.FX->FXSheet ? tsf.FX->FXSheet->FXName.c_str() : "???", /*tsf.DebugDay*/ 666);
		#endif
		CTimeStampedFX newTsf;
		newTsf.FX = tsf.FX;
		if (tsf.FX->FXSheet && tsf.FX->FXSheet->Mode == CSeasonFXSheet::Spawn)
		{
			 // for a spawned fx, it is shutdown as soon the frame after it is created, so we add it to the remove list with the current date
			newTsf.Date = date;
		}
		else
		{
			#ifdef DEBUG_FX
				nlinfo("start day = %d, start hour = %f", (int) tsf.Date.Day, tsf.Date.Hour);
			#endif
			float cycleLength = tsf.FX->FXSheet ? tsf.FX->FXSheet->CycleDuration : _DayLength;
			sint32 cycle = dateToCycle(tsf.Date, cycleLength, _DayLength);
			tsf.FX->computeEndHour(cycle - 1, newTsf.Date, cycleLength, _DayLength, _Noise);
			#ifdef DEBUG_FX
				nlinfo("try 0 : end day = %d, end hour = %f", (int) newTsf.Date.Day, newTsf.Date.Hour);
			#endif
			if (newTsf.Date < tsf.Date)
			{
				tsf.FX->computeEndHour(cycle, newTsf.Date, cycleLength, _DayLength, _Noise);
				#ifdef DEBUG_FX
					nlinfo("try 1 : end day = %d, end hour = %f", (int) newTsf.Date.Day, newTsf.Date.Hour);
				#endif
				if (newTsf.Date < tsf.Date)
				{
					tsf.FX->computeEndHour(cycle + 1, newTsf.Date, cycleLength, _DayLength, _Noise);
					#ifdef DEBUG_FX
						nlinfo("try 2 : end day = %d, end hour = %f", (int) newTsf.Date.Day, newTsf.Date.Hour);
					#endif
				}
			}
		}
		newTsf.FX->SetHandle = _FXToRemove.insert(newTsf).first;
		newTsf.FX->State = CManagedFX::InRemoveList;
		// remove from current queue
		_FXToAdd.erase(_FXToAdd.begin());

	}

	CHECK_INTEGRITY
	_CurrDate = date;

	// if cam pos has moved too much, must completely resort fx by distance
	if ((_CurrDate.Day == 0 && _CurrDate.Hour == 0.f) ||(camPos - _LastCamPos).sqrnorm() > _SortDistance * _SortDistance)
	{
		_LastCamPos = camPos;
		updateCandidateFXListSorting();
		_CandidateFXListTouched = true;
	}
	// update instanciated fxs
	updateInstanciatedFXList();
}


// *******************************************************************************************
void CTimedFXManager::shutDown(TFXGroupHandle handle)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(_InitDone);
	CHECK_INTEGRITY
	TManagedFXGroup &fxGroup = handle->Group;
	// remove all fxs of that group from both queues, and shutdown those that are instanciated
	for(uint k = 0; k < fxGroup.size(); ++k)
	{
		fxGroup[k].shutDown(_Scene, _FXManager);
	}
	CHECK_INTEGRITY
		remove(handle);
}


// *******************************************************************************************
void CTimedFXManager::CManagedFX::unlinkFromCandidateFXList()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (_PrevCandidateFX)
	{
		*_PrevCandidateFX = _NextCandidateFX;
		if (_NextCandidateFX)
		{
			_NextCandidateFX->_PrevCandidateFX = _PrevCandidateFX;
		}
		_NextCandidateFX = NULL;
		_PrevCandidateFX = NULL;
	}
}

// *******************************************************************************************
void CTimedFXManager::CManagedFX::unlinkFromInstanciatedFXList()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (_PrevInstanciatedFX)
	{
		*_PrevInstanciatedFX = _NextInstanciatedFX;
		if (_NextInstanciatedFX)
		{
			_NextInstanciatedFX->_PrevInstanciatedFX = _PrevInstanciatedFX;
		}
		_NextInstanciatedFX = NULL;
		_PrevInstanciatedFX = NULL;
	}
}

// *******************************************************************************************
void CTimedFXManager::CManagedFX::shutDown(NL3D::UScene *scene, CFXManager &fxManager)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (!Instance.empty())
	{
		// should not be already shutting down
		// if the fx is spaned, it will end by itself, so we just add it to the shutdown list
		if (FXSheet && FXSheet->Mode == CSeasonFXSheet::Spawn)
		{
			// delegate to fx manager
			fxManager.addFX(Instance, DEFAULT_SPAWNED_FX_TIMED_OUT);
		}
		else
		{
			// fx isn't spwaned, so must tell fx to stop its emitters
			if (Instance.isSystemPresent() && Instance.isValid())
			{
				if (!Instance.removeByID(NELID("main")) && !Instance.removeByID(NELID("STOP")))
				{
					// if a specific emitter has not be tagged, just stop all emitters if there's no better solution
					Instance.activateEmitters(false);
				}
				fxManager.addFX(Instance, FX_MANAGER_DEFAULT_TIMEOUT, true);
			}
			else
			{
				// the system is not present yet -> delete it immediatly
				if (scene) scene->deleteInstance(Instance);
			}
		}
		Instance = NULL;
	}
}

// *******************************************************************************************
void CTimedFXManager::cycleToDate(sint32 cycle, float hour, float cycleLength, float dayLength, CClientDate &result)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (dayLength == 0.f)
	{
		result.Day = 0;
		result.Hour = 0;
	}
	double resultHour = (double) cycle * (double) cycleLength + (double) hour;
	result.Day = (sint32) floor(resultHour / dayLength);
	result.Hour = (float) (resultHour - (double) result.Day * (double) dayLength);
}

// *******************************************************************************************
sint32 CTimedFXManager::dateToCycle(const CClientDate &date, float cycleLength, float dayLength)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (cycleLength == 0.f) return 0;
	if (dayLength == cycleLength) return date.Day;
	double hour = (double) date.Day * (double) dayLength + (double) date.Hour;
	return (sint32) floor(hour / cycleLength);
}


// *******************************************************************************************
void CTimedFXManager::CManagedFX::computeHour(sint32 cycle, float bias, CClientDate &resultDate, float cycleLength, float dayLength, const NLMISC::CNoiseValue &nv, float minHour, float maxHour) const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	// add an offset in z to get a different noise value each day
	CVector pos = SpawnPosition + (float) cycle * CVector::K + bias * nv.Frequency * CVector::I;
	float delta = computeUniformNoise(nv, pos);
	if (minHour <= maxHour)
	{
		// convert cycle:hour to day:hour (a cycle may be something other than 24 hour)
		cycleToDate(cycle, minHour + delta * (maxHour - minHour), cycleLength, dayLength, resultDate);
	}
	else
	{
		float hourDiff = dayLength - minHour + maxHour;
		float hour = minHour + delta * hourDiff;
		if (hour >= cycleLength)
		{
			cycleToDate(cycle + 1, hour - dayLength, cycleLength, dayLength, resultDate);
		}
		else
		{
			cycleToDate(cycle, hour, cycleLength, dayLength, resultDate);
		}
	}
}

// *******************************************************************************************
void CTimedFXManager::CManagedFX::computeStartHour(sint32 cycle, CClientDate &resultDate, float cycleLength, float dayLength, const NLMISC::CNoiseValue &nv) const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (!FXSheet)
	{
		resultDate = CClientDate();
		return;
	}
	computeHour(cycle, 0.f, resultDate, cycleLength, dayLength, nv, FXSheet->StartHourMin, FXSheet->StartHourMax);
}

// *******************************************************************************************
void CTimedFXManager::CManagedFX::computeEndHour(sint32 cycle, CClientDate &resultDate, float cycleLength, float dayLength, const NLMISC::CNoiseValue &nv) const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (!FXSheet)
	{
		resultDate = CClientDate();
		return;
	}
	if (!(FXSheet->Mode == CSeasonFXSheet::UseDuration))
	{
		if (FXSheet->EndHourMax < FXSheet->StartHourMin)
		{
			computeHour(cycle + 1, 1.f, resultDate, cycleLength, dayLength, nv, FXSheet->EndHourMin, FXSheet->EndHourMax);
		}
		else
		{
			computeHour(cycle, 1.f, resultDate, cycleLength, dayLength, nv, FXSheet->EndHourMin, FXSheet->EndHourMax);
		}
	}
	else
	{
		// compute start hour and add duration
        computeHour(cycle, 0.f, resultDate, cycleLength, dayLength, nv, FXSheet->StartHourMin, FXSheet->StartHourMax);
		#ifdef DEBUG_FX
			nlinfo("computeEndHour for day %d: start day = %d, start hour = %f, pos=(%f, %f, %f)", (int) day, (int) resultDate.Day, resultDate.Hour, SpawnPosition.x, SpawnPosition.y, SpawnPosition.z);
		#endif
		// add duration
		const float bias = 0.5656f; // dummy bias to get another value
		CVector pos = SpawnPosition + (float) cycle * CVector::K + bias * nv.Frequency * CVector::I;
		float delta = computeUniformNoise(nv, pos);
		resultDate.Hour += delta * (FXSheet->MaxDuration - FXSheet->MinDuration) + FXSheet->MinDuration;
		if (resultDate.Hour > dayLength)
		{
			resultDate.Hour -= dayLength;
			++resultDate.Day;
		}
	}
}

// *******************************************************************************************
void CTimedFXManager::dumpFXToAdd() const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(_InitDone);
	nlinfo("FX to add");
	nlinfo("=========");
	for(TTimeStampedFXPtrSet::const_iterator it = _FXToAdd.begin(); it != _FXToAdd.end(); ++it)
	{
		nlinfo("%s : day=%d, hour=%f, pos=(%.1f, %.1f, %.1f)",
			   it->FX->FXSheet ? it->FX->FXSheet->FXName.c_str() : "???",
			   (int) it->Date.Day,
			   it->Date.Hour,
			   it->FX->SpawnPosition.x,
			   it->FX->SpawnPosition.y,
			   it->FX->SpawnPosition.z
			  );
	}
}

// *******************************************************************************************
void CTimedFXManager::dumpFXToRemove() const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(_InitDone);
	nlinfo("FX to add");
	nlinfo("=========");
	for(TTimeStampedFXPtrSet::const_iterator it = _FXToRemove.begin(); it != _FXToRemove.end(); ++it)
	{
		nlinfo("%s : day=%d, hour=%f, pos=(%.1f, %.1f, %.1f)",
			   it->FX->FXSheet ? it->FX->FXSheet->FXName.c_str() : "???",
			   (int) it->Date.Day,
			   it->Date.Hour,
			   it->FX->SpawnPosition.x,
			   it->FX->SpawnPosition.y,
			   it->FX->SpawnPosition.z
			  );
	}
}

// *******************************************************************************************
void CTimedFXManager::dumpFXInfo() const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(_InitDone);
	nlinfo("FX Infos");
	nlinfo("=========");
	nlinfo("Num FX to add = %d", (int) _FXToAdd.size());
	nlinfo("Num FX to remove = %d", (int) _FXToRemove.size());
	uint numInstance = 0;
	for(TManagedFXGroupList::const_iterator it = _FXGroups.begin(); it != _FXGroups.end(); ++it)
	{
		const TManagedFXGroup &group = it->Group;
		for(uint k = 0; k < group.size(); ++k)
		{
			if (!group[k].Instance.empty()) ++ numInstance;
		}

	}
	nlinfo("Num Intanciated FX = %d", (int) numInstance);
}

// *******************************************************************************************
void CTimedFXManager::checkIntegrity()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)

	// count number of instances that are not always instanciated
	uint numInstance = 0;
	for(TManagedFXGroupList::const_iterator it = _FXGroups.begin(); it != _FXGroups.end(); ++it)
	{
		const TManagedFXGroup &group = it->Group;
		for(uint k = 0; k < group.size(); ++k)
		{
			if (!group[k].Instance.empty())
			{
				if (!(group[k].FXSheet && group[k].FXSheet->Mode == CSeasonFXSheet::AlwaysStarted))
				{
					++ numInstance;
				}
			}
		}
	}
	#ifdef NL_DEBUG
		for(TTimeStampedFXPtrSet::iterator it = _FXToAdd.begin(); it != _FXToAdd.end(); ++it)
		{
			nlassert(it->FX->Magic == 0xbaadcafe);
		}
		for(TTimeStampedFXPtrSet::iterator it = _FXToRemove.begin(); it != _FXToRemove.end(); ++it)
		{
			nlassert(it->FX->Magic == 0xbaadcafe);
		}
	#endif
}


// *******************************************************************************************
void CTimedFXManager::setupUserParams(CManagedFX &fi, uint cycle)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (fi.FXSheet)
	{
		for(uint k = 0; k < 4; ++k)
		{
			if (fi.FXSheet->UserParamsMax[k] != fi.FXSheet->UserParamsMin[k])
			{
				float delta = computeUniformNoise(_Noise, fi.SpawnPosition + (float) 1.23564f * k * CVector::I + (float) cycle * 2.564848f * CVector::J);
				fi.Instance.setUserParam(k, fi.FXSheet->UserParamsMin[k] + delta * (fi.FXSheet->UserParamsMax[k] - fi.FXSheet->UserParamsMin[k]));
			}
			else
			{
				fi.Instance.setUserParam(k, fi.FXSheet->UserParamsMin[k]);
			}
		}
	}
	else
	{
		for(uint k = 0; k < 4; ++k)
		{
			fi.Instance.setUserParam(k, 0.f);
		}
	}
}


// *******************************************************************************************
// from a fx mode, get a description string
const char *timedFXModeToStr(CSeasonFXSheet::TMode mode)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	switch(mode)
	{
		case CSeasonFXSheet::AlwaysStarted: return "always started";
		case CSeasonFXSheet::UseEndHour:    return "use end hour";
		case CSeasonFXSheet::UseDuration:   return "use duration";
		case CSeasonFXSheet::Spawn:			return "spawn";
		default:							break;
	}
	return "???";
}

// *******************************************************************************************
void CTimedFXManager::displayFXBoxes(TDebugDisplayMode displayMode) const
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	Driver->setViewMatrix(Scene->getCam().getMatrix().inverted());
	NL3D::CFrustum fr;
	Scene->getCam().getFrustum(fr.Left, fr.Right, fr.Bottom, fr.Top, fr.Near, fr.Far);
	fr.Perspective = true;
	Driver->setFrustum(fr);
	TextContext->setColor(CRGBA::Blue);
	TextContext->setShaded(false);
	TextContext->setFontSize(10);
	//
	float size = 0.4f;
	float textSize = 2.5f;
	// display fx to add
	for(TManagedFXGroupList::const_iterator groupIt = _FXGroups.begin(); groupIt != _FXGroups.end(); ++groupIt)
	{
		for(uint k = 0; k < groupIt->Group.size(); ++k)
		{
			const CManagedFX &mf = groupIt->Group[k];
			NLMISC::CMatrix mat = mf.getInstanceMatrix();
			Driver->setModelMatrix(mat);
			NLMISC::CRGBA color = CRGBA::Black;
			switch(mf.State)
			{
				case CManagedFX::Permanent: color = (!mf.Instance.empty()) ? CRGBA::Magenta : CRGBA::Cyan; break;
				case CManagedFX::InAddList: color = CRGBA::Blue; break;
				case CManagedFX::InRemoveList: color = (!mf.Instance.empty()) ? CRGBA::Red : CRGBA::Yellow; break;
				case CManagedFX::Unknown: break;
			}
			drawBox(CVector(- size, - size, - size),
					CVector(size, size, size),
					color);
			std::string textToDisplay;
			switch(displayMode)
			{
				case NoText:
				break;
				case PSName:
					textToDisplay = mf.FXSheet->FXName;
				break;
				case SpawnDate:
					switch(mf.State)
					{
						case CManagedFX::Permanent: textToDisplay = NLMISC::toString("PERMANENT"); break;
						case CManagedFX::InAddList:
						case CManagedFX::InRemoveList:
							textToDisplay = NLMISC::toString("day:%d, hour:%d:%d", (int) mf.SetHandle->Date.Day, (int) mf.SetHandle->Date.Hour, (int) (60.f * fmodf(mf.SetHandle->Date.Hour, 1.f)));
						break;
						default:
						break;
					}
				break;
				default:
					nlassert(0);
				break;
			}
#if !FINAL_VERSION
				if (mf.FromIG)
				{
					textToDisplay = "*" + textToDisplay;
				}
			#endif
			if (!textToDisplay.empty())
			{
				TextContext->setColor(color);
				mat.identity();
				mat.setRot(Scene->getCam().getRotQuat());
				mat.setPos(mf.SpawnPosition + 2.5f * size * CVector::K);
				CVector distPos = mf.SpawnPosition - Scene->getCam().getPos();
				mat.scale(textSize * distPos.norm());
				TextContext->render3D(mat, textToDisplay);
			}
		}
	}
}


// *******************************************************************************************
void CTimedFXManager::linkCandidateFX(TCandidateFXListSortedByDist &targetList, float dist, CManagedFX *fx)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (!fx) return;
	uint rank = (uint) (dist / _SortDistance);
	fx->unlinkFromCandidateFXList(); // unlink from previous list
	rank = std::min(rank, (uint) (_CandidateFXListSortedByDist.size() - 1));
	fx->_NextCandidateFX = targetList[rank];
	if (targetList[rank] != NULL)
	{
		targetList[rank]->_PrevCandidateFX = &fx->_NextCandidateFX;
	}
	fx->_PrevCandidateFX = &targetList[rank]; // NB the vector won't grow so no prb
	targetList[rank] = fx;
}

// *******************************************************************************************
void CTimedFXManager::linkInstanciatedFX(CManagedFX *&listHead, CManagedFX *fx)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	if (!fx) return;
	fx->unlinkFromInstanciatedFXList();
	fx->_NextInstanciatedFX = listHead;
	if (listHead)
	{
		listHead->_PrevInstanciatedFX = &fx->_NextInstanciatedFX;
	}
	fx->_PrevInstanciatedFX = &listHead;
	listHead = fx;
}

// *******************************************************************************************
void CTimedFXManager::updateInstanciatedFXList()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	nlassert(_InitDone);
	// if the list has not been modified since last time, no-op
	if (!_CandidateFXListTouched) return;
	// algo : we take at most '_MaxNumberOfFXInstances' from the start of the updated list of candidate fxs (they are roughly sorted by distance)
	// these fx are inserted in new list of instanciated fxs. All fx that haven't been inserted (they remains in the previous list)
	// are discarded. At the end the new list replaces the previous one
	CManagedFX *toInstanciateListHead = NULL;
	CManagedFX *alreadyInstanciatedListHead = NULL;
	uint numInstances = 0;
	uint numDistanceRanges = (uint)_CandidateFXListSortedByDist.size();
	sint maxNumPossibleInstance = (sint) (_MaxNumberOfFXInstances - _FXManager.getNumFXtoRemove());
	if (maxNumPossibleInstance > 0)
	{
		// NB: Ideally _CandidateFXListSortedByDist is small as a vector, so it's ok to traverse it in its entirety.
		for(uint k = 0; k < numDistanceRanges; ++k)
		{
			CManagedFX *currCandidate = _CandidateFXListSortedByDist[k];
			while(currCandidate)
			{
				nlassert(currCandidate->_PrevCandidateFX);
				// if fx already instanciated, put in special list
				if (currCandidate->_PrevInstanciatedFX != NULL)
				{
					linkInstanciatedFX(alreadyInstanciatedListHead, currCandidate);
				}
				else
				{
					// not already instanciated
					linkInstanciatedFX(toInstanciateListHead, currCandidate);
				}
				++ numInstances;
				if (numInstances == (uint) maxNumPossibleInstance) break; // max number of instances reached
				currCandidate = currCandidate->_NextCandidateFX;
			}
			if (numInstances == (uint) maxNumPossibleInstance) break; // max number of instances reached
		}
	}
	// removes old instances (those that were instanciated at previous frame, but are not anymore)
	CManagedFX *currFXToRemove = _InstanciatedFXs;
	while (currFXToRemove)
	{
		CManagedFX *tmp = currFXToRemove;
		nlassert(tmp->_PrevInstanciatedFX);
		currFXToRemove = currFXToRemove->_NextInstanciatedFX;

		// shut the fx down
		tmp->shutDown(_Scene, _FXManager);

		// fast unlink (all instance are removed from that list)
		tmp->_PrevInstanciatedFX = NULL;
		tmp->_NextInstanciatedFX = NULL;
	}
	// create new instances
	CManagedFX *prevFXToInstanciate = NULL;
	CManagedFX *currFXToInstanciate = toInstanciateListHead;
	if (maxNumPossibleInstance > 0)
	{
		while (currFXToInstanciate)
		{
			nlassert(currFXToInstanciate->_PrevInstanciatedFX);
			bool oktoCreate = true;
			 // special case : if the fx must be spawned, then it must be a valid candidate at the frame it is inserted, otherwise
			 // it is not created at all (this is to avoid it to spawn at a later date, when it's too late)
			 if (currFXToInstanciate->FXSheet && currFXToInstanciate->FXSheet->Mode == CSeasonFXSheet::Spawn)
			 {
				 if (currFXToInstanciate->SetHandle->Date != _CurrDate)
				 {
					oktoCreate = false;
					// nb : the fx will be removed from the 'instanciated list' at next pass, because spawned fx are shutdown one frame after their creation (see CTimedFXManager::update)
				 }
			 }
			 if (oktoCreate)
			 {
				 nlassert(currFXToInstanciate->Instance.empty());
				 // if fx was in shutting down state, delete the instance
				 NL3D::UInstance ts = _Scene->createInstance(currFXToInstanciate->FXSheet->FXName);
				 if (ts.empty())
				 {
					currFXToInstanciate->Instance.detach();
				 }
				 else
				 {
					 currFXToInstanciate->Instance.cast (ts);
					 if (currFXToInstanciate->Instance.empty())
					 {
 						// bad type, removes the shape
						_Scene->deleteInstance(ts);
					 }
					 else
					 {
 						currFXToInstanciate->Instance.setTransformMode(NL3D::UTransform::DirectMatrix);
 						currFXToInstanciate->Instance.setMatrix(currFXToInstanciate->getInstanceMatrix());
						if (currFXToInstanciate->State == CManagedFX::Permanent)
						{
							setupUserParams(*currFXToInstanciate,0);
						}
						else
						{
 							setupUserParams(*currFXToInstanciate, currFXToInstanciate->SetHandle->Date.Day);
						}
						currFXToInstanciate->Instance.freezeHRC();
					 }
				}
			 }
			 prevFXToInstanciate = currFXToInstanciate;
			 currFXToInstanciate = currFXToInstanciate->_NextInstanciatedFX;
		}
	}
	// merge toInstanciateListHead & alreadyInstanciatedListHead together
	// this becomes the new list of instanciated fxs
	if (prevFXToInstanciate)
	{
		nlassert(prevFXToInstanciate->_NextInstanciatedFX == NULL);
		prevFXToInstanciate->_NextInstanciatedFX = alreadyInstanciatedListHead;
		if (alreadyInstanciatedListHead)
		{
			alreadyInstanciatedListHead->_PrevInstanciatedFX = &prevFXToInstanciate->_NextInstanciatedFX;
		}
		// set new list
		_InstanciatedFXs = toInstanciateListHead;
		toInstanciateListHead->_PrevInstanciatedFX = &_InstanciatedFXs;
	}
	else
	{
		// set new list
		_InstanciatedFXs = alreadyInstanciatedListHead;
		if (alreadyInstanciatedListHead)
		{
			alreadyInstanciatedListHead->_PrevInstanciatedFX = &_InstanciatedFXs;
		}
	}
	_CandidateFXListTouched = false;
}

// *******************************************************************************************
void CTimedFXManager::updateCandidateFXListSorting()
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	// re-sort all instances by distance
	nlassert(_CandidateFXListSortedByDist.size() == _CandidateFXListSortedByDistTmp.size());
	uint numDistanceRanges = (uint)_CandidateFXListSortedByDist.size();
	for(uint k = 0; k < numDistanceRanges; ++k)
	{
		CManagedFX *currCandidate = _CandidateFXListSortedByDist[k];
		while(currCandidate)
		{
			CManagedFX *tmp = currCandidate;
			currCandidate = currCandidate->_NextCandidateFX;
			float dist = (tmp->SpawnPosition - _LastCamPos).norm();
			// unlink & relink in new list
			linkCandidateFX(_CandidateFXListSortedByDistTmp, dist, tmp);
		}
	}
	// swap the 2 list
	_CandidateFXListSortedByDist.swap(_CandidateFXListSortedByDistTmp);
}

// *******************************************************************************************
void CTimedFXManager::setMaxNumFXInstances(uint maxNumInstances)
{
	FPU_CHECKER
	H_AUTO_USE(RZ_TimedFX)
	_MaxNumberOfFXInstances = maxNumInstances;
	_CandidateFXListTouched = true;
}

// *******************************************************************************************
// temp, for debug
NLMISC_COMMAND(dumpFXToAdd, "dumpFXToAdd", "<>")
{
	if (!args.empty()) return false;
	CTimedFXManager::getInstance().dumpFXToAdd();
	return true;
}

// *******************************************************************************************
// temp, for debug
NLMISC_COMMAND(dumpFXToRemove, "dumpFXToRemove", "<>")
{
	if (!args.empty()) return false;
	CTimedFXManager::getInstance().dumpFXToRemove();
	return true;
}


// *******************************************************************************************
// temp, for debug
NLMISC_COMMAND(dumpFXInfo, "dumpFXInfo", "<>")
{
	if (!args.empty()) return false;
	CTimedFXManager::getInstance().dumpFXInfo();
	return true;
}


extern class CIGCallback		*IGCallbacks;

// *******************************************************************************************
// temp, for debug
NLMISC_COMMAND(updateSeason, "updateSeason", "<>")
{
	if (!args.empty()) return false;
	if (IGCallbacks) IGCallbacks->changeSeason();
	return true;
}

// *******************************************************************************************
// temp, for debug
NLMISC_COMMAND(setMaxNumTimedFXs, "set the max number of timed fx that are visible at a time", "<numInst>")
{
	if (args.size() != 1) return false;
	uint maxNumInstances;
	fromString(args[0], maxNumInstances);
	CTimedFXManager::getInstance().setMaxNumFXInstances(maxNumInstances);
	return true;
}
