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

#ifndef NL_PARTICLE_SYSTEM_MODEL_H
#define NL_PARTICLE_SYSTEM_MODEL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/plane.h"
#include "nel/misc/contiguous_block_allocator.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/particle_system_manager.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/load_balancing_trav.h"
#include "nel/3d/scene.h"

// tmp
#include "nel/misc/hierarchical_timer.h"

#include <vector>

namespace NL3D {

/////////////////////////////////////////////////////////
// THE FOLLOWING CLASS IS FOR INSERTION OF A PARTICLE  //
// SYSTEM IN A MOT MODEL							   //
// SEE also CParticleSystemshape, CParticleSystem	   //
/////////////////////////////////////////////////////////

class CParticleSystem;
class CScene;
class CParticleSystemShape;


/** A particle system model : it is built using a CParticleSystemShape.
  * You should forgot to call the animate() method of the CScene it is part of
  * if you want motion to be performed
  */

class CParticleSystemModel : public CTransformShape
{
public:
	///\name Object
	//@{
		/// ctor
		CParticleSystemModel();
		/// dtor
		~CParticleSystemModel();

		/// register the basic models
		static	void				registerBasic();
	//@}

	///\name Embedded particle system
	//@{

		/** Get the particle system contained in this transform shape. (NB: This is shared by a smart ptr)
		  * \return pointer to the system, or NULL if no system is currently hold by this model.
		  *			this may happen when the system is not visible and that it has been deleted
		  */
		CParticleSystem				*getPS(void)
		{
			return _ParticleSystem;
		}

		/** Get the particle system (NB : This is shared by a smart ptr) contained in this transform shape.
		  * This may be null if the model is not visible.
		  */
		const CParticleSystem		*getPS(void) const
		{
			return _ParticleSystem;
		}

		/** Set the particle system for this transform shape after it has been instanciated (from a memory stream, or by sharing)
		 *  see CParticleSystemShape
		 */
		void						setParticleSystem(CParticleSystem *ps)
		{
			nlassert(!_ParticleSystem);
			_ParticleSystem = ps;
			updateOpacityInfos();
		}
	//@}

	///\name Life managment
		//@{
		/**
		 * test whether the system has become invalid. The condition for a system to be invalid
		 * are encoded in the system itself (no more particles for example). When a system has become invalid, you may want to remove it most of the time
		 */
		bool isInvalid(void) const { return _Invalidated; }

		/// interface for object that observe this model. They will be notified when it becomes invalid
		struct IPSModelObserver
		{
			virtual ~IPSModelObserver() {}
			/// called when a system has been invalidated
			virtual void invalidPS(CParticleSystemModel *psm) = 0;
		};

		/// register an observer that will be notified when this model becomes invalid
		void registerPSModelObserver(IPSModelObserver *obs);

		/** remove an observer
		  * \see registerPSModelObserver
		  */
		void removePSModelObserver(IPSModelObserver *obs);
		/// test whether obs observe this model
		bool isPSModelObserver(IPSModelObserver *obs);
	//@}


	//\name Time managment
	//@{

		/** when called with true, this force the model to querry himself the ellapsed time to the scene.
		  * This is the default. Otherwise, setEllapsedTime must be called
		  */
		void						enableAutoGetEllapsedTime(bool enable = true)
		{
			_AutoGetEllapsedTime = enable;
		}
		/** This apply a ratio on the ellapsed time. This can be used to slow down a system
		  * This must be in the >= 0.
		  * 1 means the system run at normal speed
		  */
		void						setEllapsedTimeRatio(float value)
		{
			nlassert(value >= 0);
			_EllapsedTimeRatio = value;
		}
		//
		float						getEllapsedTimeRatio() const { return _EllapsedTimeRatio; }
		/// tells whether the model will querry himself for the ellapsed time
		bool						isAutoGetEllapsedTimeEnabled(void) const
		{
			return _AutoGetEllapsedTime;
		}
		/// set the ellapsed time (in second) used for animation.
		void						setEllapsedTime(TAnimationTime ellapsedTime)
		{
			_EllapsedTime = ellapsedTime;
		}
		/// get the ellapsed time used for animation
		TAnimationTime				getEllapsedTime(void) const
		{
			return _EllapsedTime;
		}
	//@}

	///\name Edition related methods
	//@{
		/// activate the display of tool (for edition purpose)
		void						enableDisplayTools(bool enable = true);

		// check whether the display of tools is enabled
		bool						isToolDisplayEnabled(void) const
		{
			return _ToolDisplayEnabled;
		}
		/** force the edition mode : this will prevent the system from being removed when it is out of range.
		 * When the model is first allocated, the system resource are not allocated until it becomes visible.
		 * This also forces the resources to be allocated.
		 * when there are no more particles in it etc. (this also mean that you can safely keep a pointer on it)
		 * This flag is not saved.
		 */
		void						setEditionMode(bool enable = true) ;
		/// test if edition mode is activated
		bool						getEditionMode(void) const
		{
			return _EditionMode;
		}
		/// edition purpose : touch the system to tell that the transparency state of the system has changed (added/removed opaque/tansparent faces )
		void						touchTransparencyState(void)
		{
			_TransparencyStateTouched = true;
		}
		/// edition purpose : touch the system to tell that the lightable state of the system has changed (added/removed lightable faces )
		void						touchLightableState(void)
		{
			_LightableStateTouched = true;
		}
	//@}

	///\name User params / animation
	//@{
		/// for now, we have 4 animatables value in a system
		enum	TAnimValues
		{
			OwnerBit= CTransformShape::AnimValueLast,
			PSParam0,
			PSParam1,
			PSParam2,
			PSParam3,
			PSTrigger, // trigger the instanciation of the system
			AnimValueLast,
		};
		virtual IAnimatedValue		*getValue (uint valueId);
		virtual const char			*getValueName (uint valueId) const;
		static const char			*getPSParamName (uint valueId);
		virtual ITrack				*getDefaultTrack (uint valueId);
		virtual	void				registerToChannelMixer(CChannelMixer *chanMixer,
														   const std::string &prefix = "");
		// Bypass a global user param.
		void  bypassGlobalUserParamValue(uint userParamIndex, bool byPass = true);
		bool  isGlobalUserParamValueBypassed(uint userParamIndex) const;
	//@}

	/** This update the infos about opacity (e.g are there solid faces and / or transparent faces in the system).
	  * This must be called when the system is instanciated, or when attributes have changed, such as the blending mode
	  */
	void						updateOpacityInfos(void);
	// Update the lighted/not lighted flag of the system
	void						updateLightingInfos(void);

	virtual void				getAABBox(NLMISC::CAABBox &bbox) const;
	/// inherited from CTransformShape. Returns the number of triangles wanted depeneding on the distance
	virtual float				getNumTriangles (float distance);
	/// create an instance of this class
	static CTransform				*creator()
	{
		return new CParticleSystemModel;
	}


	/// \name CTransform traverse specialisation
	// @{
	/** Very special clip for Particle System (because of the complexity of not rendered, but still detail-animated...)
	 */
	virtual void	traverseClip();
	// no-op clip() because all done in special traverse()
	virtual	bool	clip();
	/**
	 *	 - call CTransformShape::traverseAnimDetail()
	 *	 - Detail animation for a particle system. It perform motion of the particles
	 *		(so, motion occurs only when the system has not be clipped)
     */
	virtual void	traverseAnimDetail();
	virtual void	traverseRender();
	// @}

	// activate / deactivate all emitters
	void	activateEmitters(bool active);
	// test if there are active emitters in the system
	bool    hasActiveEmitters() const;

	// user color
	void			setUserColor(NLMISC::CRGBA userColor);
	NLMISC::CRGBA	getUserColor() const { return _UserColor; }

	/** Set user matrix of the system.
	   *
	   * Particle can be located in various coordinate system :
	   * - in world (identity matrix)
	   * - local to the particle system (matrix of the particle system)
	   * - local to the coord. sys. defined by the user matrix
	   *
	   */
	void setUserMatrix(const NLMISC::CMatrix &userMatrix) { _UserMatrix = userMatrix; }
	// Set the user matrix, with instant update of the CParticleSystem pointed by that model (if instanciated)
	void forceSetUserMatrix(const NLMISC::CMatrix &userMatrix);
	const NLMISC::CMatrix &getUserMatrix() const { return _UserMatrix; }

	void forceInstanciate();

	// Set z-bias. Value is in world coordinates. Value remains even if ps isn't present (rsc not allocated)
	void setZBias(float value);

	// sound on / off
	void stopSound();
	void reactivateSound();

	// from CTransform
	virtual void update();

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	friend class CParticleSystemShape;
	friend class CParticleSystemManager;


	/// Called when the resource (attached system) for this system must be reallocated
	void reallocRsc();
	/// Called by the particle system manager to release this model resources (if it is too far for example)
	void refreshRscDeletion(const std::vector<CPlane>	&worldFrustumPyramid,  const NLMISC::CVector &viewerPos);
	// Release the resources (attached system) of this model, but doesn't make it invalid.
	void releaseRsc();
	// Mark this system model as invalid, delete the attached system, and calls his observers
	void releaseRscAndInvalidate();
	/// Return true if the system is in the given world pyramid
	bool checkAgainstPyramid(const std::vector<CPlane>	&worldFrustumPyramid) const;
	// Release PS and backup system params
	void releasePSPointer();

	void invalidateAutoAnimatedHandle();


	// insert the model in the Clip/AnimDetail/LoadBalacing visible list.
	void				insertInVisibleList()
	{
		// if not already not inserted
		if (!_InsertedInVisibleList)
		{
			_Visible = true;
			_InsertedInVisibleList = true;
			// add to clip/load Trav.
			getOwnerScene()->getClipTrav().addVisibleModel(this);
			getOwnerScene()->getLoadBalancingTrav().addVisibleModel(this);

			// Add only if no ancestor skeleton model
			if( _AncestorSkeletonModel==NULL )
			{
				// need to test isLightable(), because most of PS are not lightable
				// NB: don't insert if has an _AncestorSkeletonModel, because in this case,
				// result is driven by the _LightContribution of the _AncestorSkeletonModel.
				if( isLightable() )
					getOwnerScene()->getLightTrav().addLightedModel(this);
				// no need to test isAnimDetailable()... for PS, always add them
				// NB: don't insert if has an _AncestorSkeletonModel, because in this case, this ancestor will
				// animDetail through the hierarchy...
				getOwnerScene()->getAnimDetailTrav().addVisibleModel(this);
			}
		}
	}
	bool checkDestroyCondition(CParticleSystem *ps);
	// perform actual animation of the particles
	void	doAnimate();

private:
	CParticleSystemManager::TModelHandle    _ModelHandle; /** a handle to say when the resources
															* of the model (_ParticleSystem) are deleted
															*/
	CParticleSystemManager::TAlwaysAnimatedModelHandle    _AnimatedModelHandle; // handle for permanenlty animated models
	NLMISC::CSmartPtr<CParticleSystem>		_ParticleSystem;
	CScene							  	   *_Scene;
	TAnimationTime						    _EllapsedTime;
	float									_EllapsedTimeRatio;

	// old animation mode for the system
	CParticleSystem::TAnimType				_AnimType;

	bool									_AutoGetEllapsedTime        : 1;
	bool									_ToolDisplayEnabled         : 1;
	bool									_TransparencyStateTouched   : 1;
	bool									_LightableStateTouched      : 1;
	bool									_EditionMode                : 1;
	bool									_Invalidated 				: 1;  /// if false, system should be recreated
	bool									_InsertedInVisibleList      : 1;
	bool									_InClusterAndVisible        : 1;
	bool                                    _EmitterActive			    : 1;
	bool									_SoundActive				: 1;

	std::vector<IPSModelObserver *>			_Observers;
	CAnimatedValueBool						_TriggerAnimatedValue;
	/// user params of the system
	CAnimatedValueFloat						_UserParam[MaxPSUserParam];
	uint8                                   _BypassGlobalUserParam;  // mask to bypass a global user param. This state is not serialized
	NLMISC::CRGBA							_UserColor;
	NLMISC::CMatrix							_UserMatrix;
	float									_ZBias;
	CHrcTrav::TVisibility					_LastVisibility;

	#ifdef PS_FAST_ALLOC
		// for fast allocation of ps resources
		NLMISC::CContiguousBlockAllocator		_Allocator;
	#endif
};


// tmp
class CMiniTimer
{
public:
	NLMISC::CSimpleClock SC;
	uint64 &Target;
	uint64 StartDate;
	CMiniTimer(uint64 &target) : Target(target)
	{
		SC.start();
	}
	~CMiniTimer()
	{
		SC.stop();
		Target += SC.getNumTicks();
	}
};

#define MINI_TIMER(name)

/*
#define MINI_TIMER(name) CMiniTimer mt(name);


extern uint64 PSStatsRegisterPSModelObserver;
extern uint64 PSStatsRemovePSModelObserver;
extern uint64 PSStatsUpdateOpacityInfos;
extern uint64 PSStatsUpdateLightingInfos;
extern uint64 PSStatsGetAABBox;
extern uint64 PSStatsReallocRsc;
extern uint64 PSStatsReleasePSPointer;
extern uint64 PSStatsRefreshRscDeletion;
extern uint64 PSStatsReleaseRsc;
extern uint64 PSStatsReleaseRscAndInvalidate;
extern uint64 PSStatsGetNumTriangles;
extern uint64 PSStatsCheckAgainstPyramid;
extern uint64 PSStatsTraverseAnimDetail;
extern uint64 PSStatsTraverseAnimDetailPart1;
extern uint64 PSStatsTraverseAnimDetailPart2;
extern uint64 PSStatsTraverseAnimDetailPart3;
extern uint64 PSStatsTraverseAnimDetailPart4;
extern uint64 PSStatsDoAnimate;
extern uint64 PSStatsDoAnimatePart1;
extern uint64 PSStatsDoAnimatePart2;
extern uint64 PSStatsDoAnimatePart3;
extern uint64 PSStatsTraverseRender;
extern uint64 PSStatsTraverseClip;
extern uint64 PSStatsClipSystemInstanciated;
extern uint64 PSStatsClipSystemNotInstanciated;
extern uint64 PSStatsClipSystemCheckAgainstPyramid;
extern uint64 PSStatsInsertInVisibleList;
extern uint64 PSStatsCheckDestroyCondition;
extern uint64 PSStatsForceInstanciate;
extern uint64 PSAnim1;
extern uint64 PSAnim2;
extern uint64 PSAnim3;
extern uint64 PSAnim4;
extern uint64 PSAnim5;
extern uint64 PSAnim6;
extern uint64 PSAnim7;
extern uint64 PSAnim8;
extern uint64 PSAnim9;
extern uint64 PSAnim10;
extern uint64 PSAnim11;
extern uint PSStatsNumDoAnimateCalls;;
extern float PSMaxET;
extern uint PSMaxNBPass;
extern uint64 PSStatsZonePlane;
extern uint64 PSStatsZoneSphere;
extern uint64 PSStatsZoneDisc;
extern uint64 PSStatsZoneRectangle;
extern uint64 PSStatsZoneCylinder;
extern uint64 PSMotion1;
extern uint64 PSMotion2;
extern uint64 PSMotion3;
extern uint64 PSMotion4;
extern uint64 PSStatCollision;
extern uint64 PSStatEmit;
extern uint64 PSStatRender;

*/





} // NL3D






#endif // NL_PARTICLE_SYSTEM_MODEL_H

/* End of particle_system_model.h */





















