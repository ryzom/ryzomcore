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

#ifndef NL_PS_EMITTER_H
#define NL_PS_EMITTER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/ps_edit.h"
#include "nel/3d/ps_plane_basis.h"
#include "nel/3d/ps_direction.h"
#include "nel/3d/particle_system.h"


namespace NL3D {


/**
 * Base class for all emitters in a particle system.
 * Derivers should at least define the emit method which is called each time an emission is needed.
 * Not sharable accross systems.
 *
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSEmitter : public CPSLocatedBindable
{
public:
	/// \name Object
	//@{
		/// Constructor
		CPSEmitter();

		// dtor
		virtual ~CPSEmitter();
	//@}

	/// Return this bindable type
	uint32							getType(void) const { return PSEmitter; }


	/// Return priority for emitters
	virtual uint32					getPriority(void) const { return 500; }

	/// Return true if this located bindable derived class holds alive emitters
	virtual bool					hasEmitters(void) { nlassert(_Owner); return _Owner->getSize() != 0; }


	virtual void					step(TPSProcessPass pass);

	/**
	* Process the emissions.
	* The standard behaviour will call "emit" each time is needed.
	* So you don't need to redefine this most of the time
	*
	*/
	void							computeSpawns(uint firstInstanceIndex);
	/// This is called inside the sim step, and triggers emitter that where flagged as 'emit once'
	void							doEmitOnce(uint firstInstanceIndex);



	/// Display the emitter in edition mode
	virtual void					showTool(void);

	/** Set the type of located to be emitted. The default is NULL which mean that no emission will occur
	  * \return true if the operation could be performed. It can fail when this cause the system the system to last forever,
	  *              which is incompatible with the 'BypassMaxNumIntegrationSteps' flag in CParticleSystem
	  */
	bool							setEmittedType(CPSLocated *et);

	/** Inherited from CPSLocatedBindable
	 *  We register to the emitted type (when setEmittedType is called), so, this, this will be called when it is destroyed
	 */
	virtual void					notifyTargetRemoved(CPSLocated *ptr);

	/// Get emitted type.
	CPSLocated						*getEmittedType(void) { return _EmittedType; }
	/// Get const ptr on emitted type
	const CPSLocated				*getEmittedType(void) const { return _EmittedType; }


	/** The type of emission.
	 *  regular     : means use Period, and generation number (the number of particle to generate when an emission occurs)
	 *  onDeath     : emit when the emitter is destroyed
	 *  once        : emit when the emitter is created
	 *  onBounce    : emit when the emitter bounce
	 *  externEmit  : emitted explicitly by the system user. A 4 letters ID must be used to identify this kind of emitters
	 *                the default ID is NONE
	 */
	enum TEmissionType { regular = 0, onDeath = 1,  once = 2, onBounce = 3, externEmit = 4, numEmissionType };

	/** Set the emission type. Please note that if the type is externEmit, this located need to have been attached to the system (the system is holding the ID-Located map)
	  * \return true if the operation could be performed. It can fail when this cause the system the system to last forever,
	  *              which is incompatible with the 'BypassMaxNumIntegrationSteps' in CParticleSystem
	  */
	bool							setEmissionType(TEmissionType freqType);

	/// Get the frequency type
	TEmissionType					getEmissionType(void) const { return _EmissionType; }

	/** Set a constant period for emission (expressed in second)
	 *  any previous period scheme is discarded
	 */
	void							setPeriod(float period);

	/// Retrieve the period for emission, valid only if a period scheme is used
	float							getPeriod(void) const { return _Period; }

	/// Indicate whether a period scheme is used or not
	bool							usePeriodScheme(void) { return _PeriodScheme != NULL; }

	/// Set a period scheme
	void							setPeriodScheme(CPSAttribMaker<float> *scheme);

	// Retrieve the period scheme, or null, if there'isnt
	CPSAttribMaker<float>			*getPeriodScheme(void) { return _PeriodScheme; }

	// Retrieve the period scheme, or null, if there'isnt (const version)
	const CPSAttribMaker<float>		*getPeriodScheme(void) const  { return _PeriodScheme; }

	/// Set a delay in seconds before the first emission (regular emitter only)
	void							setEmitDelay(float delay);

	/// Get the delay in seconds before the first emission (regular emitter only)
	float							getEmitDelay() const { return _EmitDelay; }

	/** Set a max. number of particle emission (0 means no limit and is the default). Applies with regular emitter only.
	  * NB : the emitter should be inserted in a system for this call to work
	  * \return true if the operation could be performed. It can fail when this cause the system the system to last forever,
	  *              which is incompatible with the 'BypassMaxNumIntegrationSteps' in CParticleSystem
	  */
	bool							setMaxEmissionCount(uint8 count);

	/// Get the max. number of particle emission (0 means no limit and is the default). Applies with regular emitter only.
	uint8							getMaxEmissionCount() const { return _MaxEmissionCount; }

	/** Set a constant number of particle to be generated at once
	 *  any previous scheme is discarded
	 */
	void							setGenNb(uint32 GenNb);

	/// Retrieve the GenNb for emission, valid only if a GenNb scheme is used
	uint							getGenNb(void) const { return _GenNb; }

	/// Indicate whether a GenNb scheme is used or not
	bool							useGenNbScheme(void) { return _GenNbScheme != NULL; }

	/// Set a GenNb scheme
	void							setGenNbScheme(CPSAttribMaker<uint32> *scheme);

	/// Retrieve the GenNb scheme, or null, if there'isnt
	CPSAttribMaker<uint32>			*getGenNbScheme(void) { return _GenNbScheme; }

	/// Retrieve the GenNb scheme, or null, if there'isnt (const version)
	const CPSAttribMaker<uint32>	*getGenNbScheme(void) const  { return _GenNbScheme; }

	/// Serialization
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	///\name Speed vector options
	//@{
		/** Set a factor, to add the emitter speed to the emittee creation speed. this can be < 0
		 *  The default is 0
		 */
		void							setSpeedInheritanceFactor(float fact)
		{
			_SpeedInheritanceFactor = fact;
		}

		/// Get the Speed Inheritance factor
		float							getSpeedInheritanceFactor(void) const
		{
			return _SpeedInheritanceFactor;
		}

		/** Align the direction of emission on the emitter speed.
		  * NB This also implies that the coord. system in which the speed vector is expressed if the same than the one of the emitter
		  * so the calls to enableUserMatrixModeForEmissionDirection() & setUserMatrixModeForEmissionDirection() have no effects (but their value is retained)
		  */
		void							enableSpeedBasisEmission(bool enabled = true);

		/** Check if the direction of emission is aligned on the emitter speed.
		 *  \see enableSpeedBasisEmission()
		 */
		bool							isSpeedBasisEmissionEnabled(void) const { return _SpeedBasisEmission; }

		/** By default, the direction of emission is supposed to be expressed in the same coordinate system than the one of the emitter.
		  * Enabling a user matrix mode for the direction of emission allows to change that behaviour.
		  * example of use :
		  * A fire p.s is linked to a torch, but the torch doesn't point to the top. So particles are emitted in the axis aligned to the torch
		  * If matrix mode for direction emission is set to PSIdentityMatrix, then the direction is interpreted to be in world, and is thus independant from
		  * the torch orientation : particles are always spawned in the +K direction.
		  *
		  * NB  : if isSpeedBasisEmissionEnabled() == true then this flag is meaningless
          */
		void							enableUserMatrixModeForEmissionDirection(bool enable = true);
		bool							isUserMatrixModeForEmissionDirectionEnabled() const { return _UserMatrixModeForEmissionDirection; }
		/** Set the coord. system in with the direction is expressed. This value is taken in account only
		  * if enableUserMatrixModeForEmissionDirection(true) has been called.
		  * NB  : if isSpeedBasisEmissionEnabled() == true then this value is meaningless
          */
		void							setUserMatrixModeForEmissionDirection(TPSMatrixMode matrixMode);
		TPSMatrixMode					getUserMatrixModeForEmissionDirection() const {	return _UserDirectionMatrixMode; }
	//@}

	/// Process a single emission. For external use (in the user interface layer)
	void							singleEmit(uint32 index, uint quantity);

	/** Enable consistent emission. The default is false. This try to keep the number of emitted particle constant, by allowing
	  * more than one emission cycle per iteration. This is useful to deal with poor frmerate. This has several drawbacks though :
	  * - collisions are not properly supported in this case (may be resolved later). RESOLVED
	  * - The motion is in straight lines.
	  * - It assumes that emitter has no motion (for now). RESOLVED
	  * NB nico : this is the default now ...
	  * In fact, this should be used when there can't be collisions with the emitted particles, and with main emitters only.
	  * NB : this has no effect if the emission period is 0 (which mean emit at each frame)
	  */
	void							enableConsistenEmission(bool enable) { _ConsistentEmission = enable; }

	bool						    isConsistentEmissionEnabled() const { return _ConsistentEmission; }

	/** Release any reference this obj may have on the given process.
	  * For example, this is used when detaching a located bindable from a system.
	  */
	virtual	void			 releaseRefTo(const CParticleSystemProcess *other);

	/** Release any reference this obj may have to other process of the system
	  * For example, this is used when detaching a located bindable from a system.
	  */
	virtual void			 releaseAllRef();

	// bypass the auto-LOD : no auto-LOD will be applied to that emitter
	void					 setBypassAutoLOD(bool bypass) { _BypassAutoLOD = bypass; }
	bool					 getBypassAutoLOD() const {	return _BypassAutoLOD; }

	/** For edition only : avoid that a call to CPSLocated::deleteElement() causes emitters flagged with 'emitOnDeath' to emit
	  */
	static void				setBypassEmitOnDeath(bool bypass) { _BypassEmitOnDeath = bypass; }
	static bool				getBypassEmitOnDeath() { return _BypassEmitOnDeath; }

	/** check if there's a loop with that emitter e.g. A emit B emit A
	  * NB : the emitter should be inserted in a system, otherwise -> assert
	  */
	bool					checkLoop() const;


	/** Test is the emitter will emit an infinite amount of particles (e.g it doesn't stop after a while)
	  * NB : If the emitter isn't inserted in a CPSLocated instance, an assertion will be reaised
	  */
	bool					testEmitForever() const;

	// from CPSLocated
	virtual void setOwner(CPSLocated *psl);

	// from from CPSLocated
	virtual bool			getUserMatrixUsageCount() const;

	// Set the emit trigger. At the next sim step, all particles will emit at the start of the step.
	void					setEmitTrigger() { _EmitTrigger = true; }
	// Update emit trigger. To Be called at the start of the sim step.
	// If emit trigger is set, all emitter emit a single time, and then the falg is cleared
	void					updateEmitTrigger();

protected:
	friend class CPSLocated;

	/// This will call emit, and will add additionnal features (speed addition and so on). This must be called inside the sim loop.
	void						processEmit(uint32 index, sint nbToGenerate);
	// The same than process emit, but can be called outside the sim loop.
	void						processEmitOutsideSimLoop(uint32 index, sint nbToGenerate);

	/// The same as processEmit, but can also add a time delta. This must be called inside the sim loop.
	void						processEmitConsistent(const NLMISC::CVector &emitterPos,
													  uint32 emitterIndex,
													  sint nbToGenerate,
													  TAnimationTime deltaT);

	/// Regular emission processing
	void							processRegularEmission(uint firstInstanceIndex, float emitLOD);
	void							processRegularEmissionWithNoLOD(uint firstInstanceIndex);

	/** Regular emission processing, with low-framrate compensation
	  */
	void							processRegularEmissionConsistent(uint firstInstanceIndex, float emitLOD, float inverseEmitLOD);
	void							processRegularEmissionConsistentWithNoLOD(uint firstInstanceIndex);


	// test if user matrix is needed to compute direction of emission
	bool							isUserMatrixUsed() const;

	/** The particle system maintains a ref counter to see how many object requires the user matrix for their computation
	  * (if it isn't required, a significant amount of memory used for maintenance can be saved)
	  * This tool function helps increasing / decreasing that count by seeing if the matrix is still required or not
	  */
	void							updatePSRefCountForUserMatrixUsage(bool matrixIsNeededNow, bool matrixWasNeededBefore);


	/** This method is called each time one (and only one) located must be emitted.
	 *  DERIVERS MUST DEFINE THIS
	 *  \param srcPos the source position of the emitter (with eventually a correction)
	 *  \param index the index of the emitter in the tab that generated a located
	 *  \param pos the resulting pos of the particle, expressed in the emitter basis
	 *  \param speed the reulting speed of the emitter, expressed in the emitter basis
	 */
	virtual void					emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed) = 0;

	/**	Generate a new element for this bindable. They are generated according to the propertie of the class
	 */
	virtual void					newElement(const CPSEmitterInfo &info);

	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */
	virtual void					deleteElement(uint32 index);
	// version of delete element that is called by the sim loop
	virtual void					deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep);

	/** Resize the bindable attributes containers. DERIVERS SHOULD CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void					resize(uint32 size);
	virtual void					bounceOccured(uint32 index, TAnimationTime timeToNextSimStep);
	void							updateMaxCountVect();



	/// A pointer on the type to be emitted.
	CPSLocated						*_EmittedType;

	/** The phase (  0 < phase  < period of emission). This is the time ellapsed since the last emission
	 */
	TPSAttribFloat					_Phase;
	TPSAttribUInt8					_NumEmission; // used only if MaxEmissionCount is != 0

	float							_SpeedInheritanceFactor;
	TEmissionType					_EmissionType;
	float _Period;
	CPSAttribMaker<float>			*_PeriodScheme;
	uint32 _GenNb;
	CPSAttribMaker<uint32>			*_GenNbScheme;
	float							_EmitDelay;
	uint8							_MaxEmissionCount;
	bool							_SpeedBasisEmission                 : 1;
	bool							_ConsistentEmission                 : 1;
	bool							_BypassAutoLOD                      : 1;
	bool							_UserMatrixModeForEmissionDirection : 1; // true when the direction of emission is expressed in a user defined coordinate system (otherwise it is expressed in this object coordinate system, as specified by setMatrixMode)
	bool							_EmitTrigger						: 1;
	TPSMatrixMode					_UserDirectionMatrixMode;
	static bool						_BypassEmitOnDeath; // for edition only
private:
	// common op of both versions of deleteElement
	void							deleteElementBase(uint32 index);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** This class helps tuning the emission strenght.
 *  It modulate the speed of emitted particle by a coeeficient produced by an attribute maker
 */

class CPSModulatedEmitter
{
	public:

		/// ctor : the default doesn't alter speed
		CPSModulatedEmitter() : _EmitteeSpeed(1.f), _EmitteeSpeedScheme(NULL)
		{
		}

		/// dtor
		virtual ~CPSModulatedEmitter()
		{
			delete _EmitteeSpeedScheme;
		}

		/** Set a new scheme for speed modulation.
		 *  It must have been allocated with new, and will be destriyed by this object
		 */
		void setEmitteeSpeedScheme(CPSAttribMaker<float> *scheme)
		{
			delete _EmitteeSpeedScheme;
			_EmitteeSpeedScheme = scheme;
			if (getModulatedEmitterOwner() && scheme->hasMemory())
				scheme->resize(getModulatedEmitterOwner()->getMaxSize(), getModulatedEmitterOwner()->getSize());
		}

		/// set a constant speed modulation for emittee
		void setEmitteeSpeed(float speed)
		{
			delete _EmitteeSpeedScheme;
			_EmitteeSpeedScheme = NULL;
			_EmitteeSpeed = speed;

		}

		/// get the modulation speed (valid only if no scheme is used)
		float getEmitteeSpeed(void) const { return _EmitteeSpeed; }

		/// get the speed modulation shceme, or NULL if no one is set
		CPSAttribMaker<float> *getEmitteeSpeedScheme(void) { return _EmitteeSpeedScheme; }

		/// get the speed modulation shceme, or NULL if no one is set (const version)
		const CPSAttribMaker<float> *getEmitteeSpeedScheme(void) const { return _EmitteeSpeedScheme; }

		/// check whether a speed modulation scheme is being used
		bool useEmitteeSpeedScheme(void) const { return _EmitteeSpeedScheme != NULL; }

		/// serialization
		void serialEmitteeSpeedScheme(NLMISC::IStream &f) throw(NLMISC::EStream);

	protected:

		// emitter must define this in order to allow this class to access the located owner
		virtual CPSLocated *getModulatedEmitterOwner(void) = 0;

		void newEmitteeSpeedElement(const CPSEmitterInfo &info)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->newElement(info);
		}

		void deleteEmitteeSpeedElement(uint32 index)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->deleteElement(index);
		}

		void resizeEmitteeSpeed(uint32 capacity)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->resize(capacity, getModulatedEmitterOwner()->getSize());
		}


		float _EmitteeSpeed;
		CPSAttribMaker<float> *_EmitteeSpeedScheme;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Emit in one direction. This can be the 0, 0, 0 vector
class CPSEmitterDirectionnal : public CPSEmitter, public CPSModulatedEmitter
							   ,public CPSDirection
{

public:


	CPSEmitterDirectionnal() : _Dir(NLMISC::CVector::K)
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("DirectionnalEmitter");
	}

	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);


	NLMISC_DECLARE_CLASS(CPSEmitterDirectionnal);

	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);

	void setDir(const NLMISC::CVector &v) { _Dir = v; }

	NLMISC::CVector getDir(void) const { return _Dir; }


protected:

	NLMISC::CVector _Dir;

	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner; }
	virtual void newElement(const CPSEmitterInfo &info);
	virtual void deleteElement(uint32 index);
	void deleteElementBase(uint32 index);
	virtual void deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep);
	virtual void resize(uint32 capacity);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// A radial emitter. The direction gives the normal to the plane of emission
class CPSRadialEmitter : public CPSEmitterDirectionnal
{
	public:
	CPSRadialEmitter()
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("RadialEmitter");
	}
	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CPSRadialEmitter);
	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Emit randomly in all direction
class CPSEmitterOmni : public CPSEmitter, public CPSModulatedEmitter
{

public:

	CPSEmitterOmni()
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("EmitterOmni");
	}

	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	NLMISC_DECLARE_CLASS(CPSEmitterOmni);


	/// Emission of located
	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);
protected:
	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner; }
	virtual void newElement(const CPSEmitterInfo &info);
	virtual void deleteElement(uint32 index);
	void deleteElementBase(uint32 index);
	virtual void deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep);
	virtual void resize(uint32 capacity);


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Emit directionnally in a rectangle (useful to produce snow, drop of water ...)
class CPSEmitterRectangle : public CPSEmitter, public CPSModulatedEmitter, public IPSMover, public CPSDirection
{
	public:

		// Ctor

		CPSEmitterRectangle() : _Dir(-NLMISC::CVector::K)
		{
			if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("EmitterRectangle");
		}

		/// Serialisation
 		virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

		NLMISC_DECLARE_CLASS(CPSEmitterRectangle);


		/// Emission of located

		virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);

		virtual void setDir(const NLMISC::CVector &v) { _Dir = v; }

		NLMISC::CVector getDir(void) const { return _Dir; }


		void showTool(void);



		// Inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true; }
		virtual bool supportNonUniformScaling(void) const { return true; }
		virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);
		virtual NLMISC::CMatrix getMatrix(uint32 index) const;
		virtual void setScale(uint32 index, float scale);
		virtual void setScale(uint32 index, const NLMISC::CVector &s);
		NLMISC::CVector getScale(uint32 index) const;

	protected:

		virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner; }

		CPSAttrib<CPlaneBasis> _Basis;

		//  Width
		TPSAttribFloat _Width;

		//  Height
		TPSAttribFloat _Height;

		// Direction of emission (in each plane basis)
		NLMISC::CVector _Dir;

		/**	Generate a new element for this bindable. They are generated according to the propertie of the class
		 */
		virtual void newElement(const CPSEmitterInfo &info);

		/** Delete an element given its index
		 *  Attributes of the located that hold this bindable are still accessible for of the index given
		 *  index out of range -> nl_assert
		 */
		virtual void deleteElement(uint32 index);
		void deleteElementBase(uint32 index);
		virtual void deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep);

		/** Resize the bindable attributes containers. DERIVERS SHOULD CALL THEIR PARENT VERSION
		 * should not be called directly. Call CPSLocated::resize instead
		 */
		virtual void resize(uint32 size);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// The same as a directionnel emitter, but you can also specify the radius for emission
class CPSEmitterConic : public CPSEmitterDirectionnal
{
public:

	CPSEmitterConic() : _Radius(1.f)
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("EmitterConic");
	}

	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	NLMISC_DECLARE_CLASS(CPSEmitterConic);


	/// Emission of located
	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);

	/// Set a new radius for emission
	void setRadius(float r) { _Radius = r; }

	/// Get the emission radius
	float getRadius(void) const { return _Radius; }

	/// Set the direction for emission
	virtual void setDir(const NLMISC::CVector &v);

protected:

	// The radius for emission
	float _Radius;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A spherical emitter
class CPSSphericalEmitter : public CPSEmitter, public CPSModulatedEmitter, public IPSMover
{
public:
	// Ctor

	CPSSphericalEmitter()
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("spherical emitter");
	}

	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	NLMISC_DECLARE_CLASS(CPSSphericalEmitter);


	/// Emission of located

	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);



	void showTool(void);



	// Inherited from IPSMover
	virtual bool supportUniformScaling(void) const { return true; }
	virtual bool supportNonUniformScaling(void) const { return false; }
	virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);
	virtual NLMISC::CMatrix getMatrix(uint32 index) const;
	virtual void setScale(uint32 index, float scale) { _Radius[index] = scale; }
	NLMISC::CVector getScale(uint32 index) const { return NLMISC::CVector(_Radius[index], _Radius[index], _Radius[index]); }

protected:
	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner; }
	TPSAttribFloat _Radius;
	virtual void newElement(const CPSEmitterInfo &info);
	virtual void deleteElement(uint32 index);
	void deleteElementBase(uint32 index);
	virtual void deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep);
	virtual void resize(uint32 size);
};


} // NL3D


namespace NLMISC
{
	// toString implementation for NL3D::CPSEmitter::TEmissionType
	std::string toString(NL3D::CPSEmitter::TEmissionType type);
}

#endif // NL_PS_EMITTER_H

/* End of ps_emitter.h */
