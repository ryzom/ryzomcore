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



#ifndef CL_TIMED_FX_MANAGER_H
#define CL_TIMED_FX_MANAGER_H

#include "nel/misc/vector.h"
#include "nel/misc/noise_value.h"
#include "game_share/season.h"
#include "time_client.h"
#include "fx_manager.h"

class CSeasonFXSheet;

namespace NL3D
{
	class UParticleSystemInstance;
	class UScene;
}

/** A fx that must be spawn at a given season and hour
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CTimedFX
{
public:
	NLMISC::CVector			SpawnPosition;
	NLMISC::CQuat			Rot;
	NLMISC::CVector			Scale;
	const CSeasonFXSheet   *FXSheet;
#if !FINAL_VERSION
		bool				FromIG; // true if the fx comes from an ig, or false if it was generated dynamically
	#endif
public:
	CTimedFX() : SpawnPosition(0.f, 0.f, 0.f), FXSheet(NULL)
	{
#if !FINAL_VERSION
			FromIG = true;
		#endif
	}
	NLMISC::CMatrix			getInstanceMatrix() const;
};

typedef std::vector<CTimedFX> TTimedFXGroup;

/** Manager to spawn fxs at a given season and hour.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CTimedFXManager
{
private:
	class  CManagedFX;
	struct CTimeStampedFX
	{
		CClientDate    Date; // the date at which the FX must be added / removed
		//sint32         DebugDay;
		CManagedFX	   *FX;  // managed FX
		bool operator < (const CTimeStampedFX &rhs) const
		{
			if (Date == rhs.Date) return FX < rhs.FX;
			else return Date < rhs.Date;
		} // we want to deal with early dates sooner
		bool operator == (const CTimeStampedFX &rhs) const { return Date == rhs.Date && FX == rhs.FX; }
		CTimeStampedFX() : FX(NULL) {}
	};
	typedef std::set<CTimeStampedFX> TTimeStampedFXPtrSet;
	struct CManagedFXGroup;
	struct CCandidateFXListHead;
	/** A managed FX. Unless it is flagged as 'always intanciated', or is in a group that is shutting down,
	  * such a fx can only be in one set : _FXToAdd if it is not currently instanciated (or is shutting down), and _FXToRemove if it is instanciated and is not being shut down.
	  * and will be removed in the future.
	  */
	class CManagedFX : public CTimedFX
	{
		public:
			enum TState { Unknown = 0, Permanent, InAddList, InRemoveList };
			#ifdef NL_DEBUG
				CManagedFXGroup				  *OwnerGroup;
			#endif
			NL3D::UParticleSystemInstance	   Instance;  // Pointer to the actual FX. NULL if failed to instanciate or if not instanciated
			TState							   State;
			TTimeStampedFXPtrSet::iterator	   SetHandle;              // if the fx is in a list (see its state), then it is an iterator into that list (for removal)
			//
			CManagedFX						   **_PrevCandidateFX;     // if the fx has asked to be instanciated, it is inserted in a list of candidate fxs. points the previous "Next" pointer
			                                                           // even if the fx is currently instanciated, it remains in that list (those are the potnetially instanciated fxs)
			CManagedFX						   *_NextCandidateFX;      // next candidate FX
			//
			CManagedFX					       **_PrevInstanciatedFX;        // link into instanciated fx list, prev (is also a candidate)
			CManagedFX					       *_NextInstanciatedFX;         // link into instanciated fx list, next
			// Tmp for debug
			#ifdef NL_DEBUG
				uint32 Magic;
			#endif
		public:
			// ctor
			CManagedFX()
			{
				Instance = NULL;
				State = Unknown;
				#ifdef NL_DEBUG
					OwnerGroup = NULL;
					Magic = 0xbaadcafe;
				#endif
				_PrevCandidateFX = NULL;
				_NextCandidateFX = NULL;
				//
				_PrevInstanciatedFX = NULL;
				_NextInstanciatedFX = NULL;
			}
			// compute start hour of that fx for the given day
			void computeStartHour(sint32 cycle, CClientDate &resultDate, float cycleLength, float dayLength, const NLMISC::CNoiseValue &nv) const;
			// compute end hour of that fx for the given day
			void computeEndHour(sint32 cycle, CClientDate &resultDate, float cycleLength, float dayLength, const NLMISC::CNoiseValue &nv) const;
			// unlink from list of candidate fxs
			void unlinkFromCandidateFXList();
			// unlink from list of instanciated fx. NB : this doesn't remove the model!
			void unlinkFromInstanciatedFXList();
			/** Shutdown the fx
			  */
			void shutDown(NL3D::UScene *scene, CFXManager &fxManager);
		private:
			void computeHour(sint32 cycle, float bias, CClientDate &resultDate, float cycleLength, float dayLength, const NLMISC::CNoiseValue &nv, float minHour, float maxHour) const;
	};
	// A group of managed fx. We never resize these vectors after creation so we can keep pointers in them.
	typedef std::vector<CManagedFX> TManagedFXGroup;
	struct CManagedFXGroup
	{
		TManagedFXGroup  Group;
		#ifdef NL_DEBUG
			EGSPD::CSeason::TSeason Season;
		#endif
	};
	// a list of group of managed fxs.
	typedef std::list<CManagedFXGroup> TManagedFXGroupList;


public:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//\name USER INTERFACE
//@{
	// mode of display for debug
	enum TDebugDisplayMode { NoText = 0, PSName, SpawnDate, DebugModeCount };
	// handle to add/remove a group of managed fxs (see add() / remove())
	typedef TManagedFXGroupList::iterator TFXGroupHandle;
	// dtor
	~CTimedFXManager();
	/** Init the manager. Must be called before first use
	  * \param scene                 The scene from which instances are created
	  * \param startDate             Date of init
	  * \param dayLength             Length of a day, in hours
	  * \param noiseFrequency        Frequency of noise used to compute pseudo random spwaning dates of fxs (depends on their spawn position and on the date)
	  * \param maxNumberOfFXInstance Max number of fx instances that are allowed to be instanciated at a given time. Nearest fx are instanciated first
	  * \param sortDistanceInterval  precision with which fx are sorted in distance. For example, with 1meter precision, fx at 0.2 & 0.5 from the user are considered
	  *                              to be at the same distance
	  * \param maxDist               Max dist at which fxs are sorted. For efficiency, it is important to keep maxDist / sortDistanceInterval
	  */
	void					init(NL3D::UScene *scene,
		                         const CClientDate &startDate,
								 float dayLength,
								 float noiseFrequency,
								 uint  maxNumberOfFXInstances,
								 float sortDistanceInterval,
								 float maxDist
								);
	// Reset all manager content
	void					reset();
	/** Register a set of fxs to be managed.
	  * A handle is returned to remove it later.
	  */
	TFXGroupHandle			add(const std::vector<CTimedFX> &fxs, EGSPD::CSeason::TSeason season);
	/** Remove a FX group that has previously been added
	  * All FX instances are deleted.
	  */
	void					remove(TFXGroupHandle handle);
	/** Delayed removal of a fx group that has previously been added.
	  * All FXs are shutdown by removing their emitters.
	  * This is useful to switch from one season to another.
	  */
	void					shutDown(TFXGroupHandle handle);
	// Update the manager state to match the new date. This add / removes FX as necessary
	void					update(const CClientDate &date, EGSPD::CSeason::TSeason currSeason, const NLMISC::CVector &camPos);
	// Set a new date, but do not update current fx. The new hour will be taken in account during the next call to 'add'
	void					setDate(const CClientDate &date);
	// get the current date
	const CClientDate	   &getDate() const { return _CurrDate; }
	// Access to the unique instance of this class
	static CTimedFXManager &getInstance();
	// for debug only
	void					dumpFXToAdd() const;
	void					dumpFXToRemove() const;
	void					dumpFXInfo() const;
	// debug bbox and dates of FXs to remove and to add.
	void					displayFXBoxes(TDebugDisplayMode displayMode) const;
	// set max number of fx to be instanciated at a time
	void					setMaxNumFXInstances(uint maxNumInstaces);
//@}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	// Set of fx to be instanciated in the future. It is managed as a priority queue (sorted by date of instanciation).
	TTimeStampedFXPtrSet		_FXToAdd;
	// Set of fx to remove in the future, and that are currently instanciated (and are not shutting done). It is managed as a priority queue (sorted by date of removal).
	TTimeStampedFXPtrSet		_FXToRemove;
	// List of the groups of FXs that are currenlty managed.
	TManagedFXGroupList			_FXGroups;
	// The scene from which to add / remove instances
	NL3D::UScene				*_Scene;
	// Current date of the manager
	CClientDate					_CurrDate;
	// List of group that are shutting down (that is, shutDown(TFXGroupHandle handle) was called on that group)
	std::list<TFXGroupHandle>	_ShuttingDownGroups;
	// Length of a day, in hours
	float						_DayLength;
	// Noise function used to compute start/end date of fxs
	NLMISC::CNoiseValue			_Noise;
	// Init
	bool						_InitDone;

	//\name DISTANCE SORTING
	//@{
		// FX to be displayed can be limited in number -> the nearest fx are instanciated in priority
		// To achieve this, we keep a list of candidate fx for each interval of distance (thus achieving linear approximate sorting)

		typedef std::vector<CManagedFX *>  TCandidateFXListSortedByDist;
		TCandidateFXListSortedByDist	   _CandidateFXListSortedByDist;      // roughly sorted list of candidate fx. Should be rebuilt completely when players moves (done in linear time)
		TCandidateFXListSortedByDist	   _CandidateFXListSortedByDistTmp;   // already allocated vect for fast resorting of list when player moves

		                                                                 // NB : this vector nevers grows
		bool							   _CandidateFXListTouched;      // the list of candidate has been modified, so the manager should see which fxs should be instanciated, and which fxs should be removed
		float							   _SortDistance;                // length of each distance interval, in meters
		uint							   _MaxNumberOfFXInstances;       // max number of instances that can be 'alive' at a time
	//@}

	// Linked list of currently instanciated FXs (nearest candidates)
	CManagedFX *_InstanciatedFXs;
	NLMISC::CVector	_LastCamPos;

	// fx manager to deals with shutting down fxs
	CFXManager _FXManager;

private:
	// ctor
	CTimedFXManager();
	// for debug
	void checkIntegrity();
	// setup user params for a fx
	void setupUserParams(CManagedFX &fi, uint cycle);
	// insert a fx in list of candidate for instanciation (also unlink from a previous list)
	void linkCandidateFX(TCandidateFXListSortedByDist &targetList, float dist, CManagedFX *fx);
	// link a fx into a list of instances (also unlink from a previous list)
	void linkInstanciatedFX(CManagedFX *&listHead, CManagedFX *fx);
	// update list of instanciated fxs (create / removes fxs as necessary)
	void updateInstanciatedFXList();
	// re-sort list of candidate fx by distance (this is necessary when player moves)
	void updateCandidateFXListSorting();
public:
	// convert a cycle and an hour to a date
	static void   cycleToDate(sint32 cycle, float hour, float cycleLength, float dayLength, CClientDate &result);
	// give the matching cycle for a date
	static sint32 dateToCycle(const CClientDate &date, float cycleLength, float dayLength);
};



#endif
