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



#ifndef CL_GROUND_FX_MANAGER_H
#define CL_GROUND_FX_MANAGER_H


#include "nel/misc/vectord.h"
#include "nel/3d/u_particle_system_instance.h"

namespace NL3D
{
	class UParticleSystemInstance;
	class UScene;
}

namespace NLMISC
{
	class CVectorD;
}

class CEntityCL;

/** Manager of 'ground fxs' : fx that are displayed when character walk over sand (actually sawdust), or grass.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CGroundFXManager
{
private:
	struct  CGroundFX
	{
		NL3D::UParticleSystemInstance	FX;
		NL3D::UParticleSystemInstance	FXUnderWater; // underwater part of fx
		std::string						FXName;
	};
	typedef std::list<CGroundFX> TGroundFXList;
	typedef std::list<CGroundFX>::iterator  TGroundFXHandle;
	class CInstance
	{
	public:
		enum TMode { Ground = 0, Water, Swim }; // if the entity is on the ground, then fx is created whn it moves
		                                        // if the entity is in water, then fx is played even if it doesn't move
		                                        // the same goes if entity is swimming
	public:
		CEntityCL			*Entity;			// the managed entity
		float				Dist2;				// square dist to the camera
		TGroundFXHandle		FXHandle;			// the FX that is played by that entity
		uint32				GroundID;			// ground type of current fx
		uint                InstanciateDelay;   // delay before to instanciate the fx
		bool				HasFX;				// is the FXHandle valid ?
		bool				EmittersActive;		// are the emitters active ?
		bool				EmittersUnderWaterActive;		// are the emitters active ?
		TMode				Mode;
		float				WaterHeight;        // if entity is in water, give its height
		bool				Idle;
	public:
		// get name of ground fx associated with a ground material id, or empty
		void		getFXNameFromGroundType(uint32 groundID, std::string &fxName) const;

	};
	typedef std::list<CInstance>	   TInstanceList;
	friend struct CSortInstancePred;
public:

	typedef TInstanceList::iterator    TEntityHandle;
	typedef std::vector<TEntityHandle> TInstancePtrVect;

	// ctor
	CGroundFXManager();
	// dtor
	~CGroundFXManager();
	/** Init the manager to work with the given scene.
	  * \param scene			 The scene from which instance are created
	  * \param maxDist           The max dist at which ground fxs are generated
	  * \param maxActiveFX       The max number of fxs that are holded by an entity
	  * \param maxInactiveFX     The max number of inactive FXs (shuting down fxs & fx ready to use)
	  */
	void		  init(NL3D::UScene *scene, float maxDist, uint maxNumFX, uint fxCacheSize);
	// get the max number of fxs.
	uint		  getMaxNumFX() const { return _MaxNumFX; }
	// Get size of cache for FXs
	uint		  getFXCacheSize() const { return _MaxNumCachedFX; }
	// get the max dist at which ground FXs are played
	float		  getMaxDist() const { return _MaxDist; }
	// register an entity to be managed
	TEntityHandle add(CEntityCL *entity);
	// remove a managed entity from its handle
	void		  remove(TEntityHandle handle);
	// update current manager state
	void		  update(const NLMISC::CVectorD &camPos);
	// reset the manager (must call init again for reuse). All handle allocated from add(CEntityCL *entity) becomes invalid after the call.
	void		  reset();
	// set min speed for walk/run (speed at which fx starts)
	void		  setMinSpeed(float minSpeed);
	// set max speed for walk/run (speed at which fx is at its max intensity)
	void		  setMaxSpeed(float maxSpeed);
	// set speed for fast walk in water
	void		  setSpeedWaterWalkFast(float speed);
	// set speed for fast swim in water
	void		  setSpeedWaterSwimFast(float speed);
private:
	float						_MinSpeed;
	float						_MaxSpeed;
	float						_SpeedWaterWalkFast;
	float						_SpeedWaterSwimFast;
	float						_MaxDist;
	TGroundFXList				_ActiveFXs;
	TGroundFXList				_InactiveFXs; // Shutting down FXs
	TGroundFXList				_CachedFXs;	  // Cached fxs FXs
	uint						_MaxNumFX;    // max number of fxs ( active fxs)
	uint						_NumFX;
	uint						_MaxNumCachedFX;
	uint						_NumCachedFX;
	//
	TInstanceList				_InstancesList;
	uint						_NumInstances;
	TInstancePtrVect			_SortedInstances; // sorted entities by distance
	NL3D::UScene				*_Scene;
private:
	void invalidateFX(TEntityHandle handle);
	void moveFXInCache(TGroundFXList &ownerList,  TGroundFXHandle fx);
	// for debug only
	void checkIntegrity();

};


/////////////////////////////////////////
// tmp : test class to test ground fxs //
/////////////////////////////////////////

#if 1

/** temp class to test ground fxs
  */
struct CTestGroundFX
{
	struct CEntity
	{
		CEntityCL	*Entity;
		uint		Slot;
		bool		Move;
		NLMISC::CVector		Dir;
		NLMISC::CVector     StartPos;
		float		Duration;
	};
	std::vector<CEntity> Entities;
	bool                 MoveAll;
	void			update();
	void			displayFXBoxes() const;
	CTestGroundFX() : MoveAll(false) {}
};

extern CTestGroundFX TestGroundFX;

#endif

#endif










































