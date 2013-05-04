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

#ifndef NL_PS_FORCE_H
#define NL_PS_FORCE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_util.h"
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/ps_edit.h"
#include "nel/3d/ps_direction.h"
#include "nel/3d/particle_system.h"


namespace NL3D {

class CPSEmitterInfo;

/**
 * All forces in the system derive from this class
 * It has a list with all located on which the force can apply.
 * Only the motion and toolRender passes are supported for a force.
 * Not sharable accross systems.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSForce : public CPSTargetLocatedBindable
{
public:

	/// Constructor
	CPSForce();

	/// return this bindable type
	uint32				getType(void) const { return PSForce; }


	/// return priority for forces

	virtual uint32		getPriority(void) const { return 4000; }

	/// Override of CPSLocatedBindable::doesProduceBBox. forces usually are not part of the bbox
	virtual bool		doesProduceBBox(void) const { return false; }

	/**
	 * process one pass for the force
	 */
	virtual void		step(TPSProcessPass pass);


	/// Compute the force on the targets. To be called inside the sim loop
	virtual void		computeForces(CPSLocated &target) = 0;

	/// Show the force (edition mode)
	virtual void		show() = 0;

	/// Serial the force definition. MUST be called by deriver during their serialisation
	virtual void		serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// check whether this force is integrable over time. The default is false
	virtual bool		isIntegrable(void) const { return false; }

	/// inherited from   CPSLocatedBindableTarget, we use that to tell whether this force is integrable or not
	virtual void		attachTarget(CPSLocated *ptr);

	/// inherited from   CPSLocatedBindableTarget
	void				releaseTargetRsc(CPSLocated *target);


	/** Integrate this force on the given located. If 'accumulate' is set to true, it just add the effect of this force on position
	  * otherwise, it must also integrate from the initial speed, and add this force effect. The first call to this must be done with
	  * 'accumulate' set to false.
	  * NB : works only with integrable forces
	  */
	 virtual void integrate(float /* date */, CPSLocated * /* src */, uint32 /* startIndex */, uint32 /* numObjects */, NLMISC::CVector * /* destPos */ = NULL, NLMISC::CVector * /* destSpeed */ = NULL,
							bool /* accumulate */ = false,
							uint /* posStride */ = sizeof(NLMISC::CVector), uint /* speedStride */ = sizeof(NLMISC::CVector)
							) const
	 {
		 nlassert(0); // not an integrable force
	 }


	/** Compute a trajectory on several steps for a single object, rather than a step for several object.
	  * If the start date is lower than the creation date, the initial position is used
	  * NB : works only with integrable forces
	  */
	virtual void integrateSingle(float /* startDate */, float /* deltaT */, uint /* numStep */,
								 const CPSLocated * /* src */, uint32 /* indexInLocated */,
								 NLMISC::CVector * /* destPos */,
								 bool /* accumulate */ = false,
								 uint /* posStride */ = sizeof(NLMISC::CVector)) const
	{
		 nlassert(0); // not an integrable force
	}






protected:
	friend class CPSLocated;
	friend class CPSForceIntensity;

	/// register integrable and non-integrable forces to the targets
	void				registerToTargets(void);

	/// if this force is not integrable anymore, it tells that to its targets
	void				 cancelIntegrable(void);

	/// if this force has become integrable again, this method tells it to the target
	void				 renewIntegrable(void);

	/** inherited from	 CPSLocatedBindable. When we deal with integrable forces,
	  * they must be in the same basis than their target. If this change, we must notify the target of it.
	  */
	virtual void		 basisChanged(TPSMatrixMode systemBasis);

	virtual void newElement(const CPSEmitterInfo &info) = 0;

	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) = 0;

	/** Resize the bindable attributes containers DERIVERS SHOULD CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLOcated::resize instead
	 */
	virtual void resize(uint32 size) = 0;
};


/// this is a class to set force instensity (acceleration for gravity, k coefficient for springs...)
class CPSForceIntensity
{
public:

	// ctor
	CPSForceIntensity() : _IntensityScheme(NULL)
	{
	}

	virtual ~CPSForceIntensity();


	/// get the constant intensity that was set for the force
	float getIntensity(void) const  { return _K; }

	/// set a constant intensity for the force. this discard any previous call to setIntensityScheme
	virtual void setIntensity(float value);

	/// set a non-constant intensity
	virtual void setIntensityScheme(CPSAttribMaker<float> *scheme);

	// deriver have here the opportunity to setup the functor object. The default does nothing
	virtual void setupFunctor(uint32 /* indexInLocated */) { }

	/// get the attribute maker for a non constant intensity
	CPSAttribMaker<float> *getIntensityScheme(void) { return _IntensityScheme; }
	const CPSAttribMaker<float> *getIntensityScheme(void) const { return _IntensityScheme; }
	void serialForceIntensity(NLMISC::IStream &f) throw(NLMISC::EStream);

protected:

	/// deriver must return the located that own them here
	virtual CPSLocated *getForceIntensityOwner(void) = 0;


	// the intensity ...
	float _K;
	CPSAttribMaker<float> *_IntensityScheme;

	void newForceIntensityElement(const CPSEmitterInfo &info)
	{
		if (_IntensityScheme && _IntensityScheme->hasMemory()) _IntensityScheme->newElement(info);
	}
	void deleteForceIntensityElement(uint32 index)
	{
		if (_IntensityScheme && _IntensityScheme->hasMemory()) _IntensityScheme->deleteElement(index);
	}
	void resizeForceIntensity(uint32 size)
	{
		if (_IntensityScheme && _IntensityScheme->hasMemory()) _IntensityScheme->resize(size, getForceIntensityOwner()->getSize());
	}
};


/**
  * this class defines the newElement, deleteElement, and resize method of a class that derives from CPSForceIntensity
  * And that don't add per paerticle attribute
  */
class CPSForceIntensityHelper : public CPSForce, public CPSForceIntensity
{
public:
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

protected:
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner; }
	virtual void newElement(const CPSEmitterInfo &info) { newForceIntensityElement(info); }
	virtual void deleteElement(uint32 index) { deleteForceIntensityElement(index); }
	virtual void resize(uint32 size) { resizeForceIntensity(size); }

};




/** a helper class to create isotropic force : they are independant of the basis, and have no position
 *  (fluid friction for example)
 *  To use this class you should provide to it a functor class that define the () operator with 3 parameters
 *  param1 = a const reference to the position of the particle
 *  param2 = a reference to the position, that must be updated
 *  param3 =  a float giving the inverse of the mass
 *  param4 = the ellapsed time, in second (has the TAnimationTime type).
 *  Example of use :
 *  class MyForceFunctor
 *  {
 *    public:
 *      /// it is strongly recommended to have your operator inlined
 *      void operator() (const NLMISC::CVector &pos, NLMISC::CVector &speed, float invMass , CanimationTime ellapsedTime)
 *      {
 *			// perform the speed update there
 *		}
 *
 *      // you can provide a serialization method. Note that that if the functor parameters are set before each use,
 *      // it useless to serial something ...
 *		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
 *
 *    protected:
 *      ...
 *  };
 *
 *
 * because of the serialization process, you must proceed like the following. (but you don't need to redefine serial, which
 * will serilize the functor object you passed for you
 *
 *	class MyForce : public CHomogenousForceT<MyForceFunctor>
 *  {
 *		public:
 *         MyForce();
 *		   NLMISC_DECLARE_CLASS(Myforce);
 *
 *      protected:
 *			...
 *
 *  };
 *
 *
 *  not that each functor may have its own parameter. the setupFunctor method will be called each time
 */

template <class T> class CIsotropicForceT : public CPSForce
{
public:

	/// Compute the force on the targets
	virtual void computeForces(CPSLocated &target);


	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1);
		CPSForce::serial(f);
		f.serial(_F); // serial the functor object 5does nothing most of the time ...)
	}


	/** Show the force (edition mode). The default does nothing
	 *  TODO later
	 */

	 void show()  {}


	 /// setup the functor object. The default does nothing

	 virtual void setupFunctor(uint32 /* index */) {}

protected:

	/// the functor object
	T _F;



	virtual void newElement(const CPSEmitterInfo &/* info */) {}
	virtual void deleteElement(uint32 /* index */) {}
	virtual void resize(uint32 /* size */) {}


};

//////////////////////////////////////////////////////////////////////
// implementation of method of the template class  CHomogenousForceT //
//////////////////////////////////////////////////////////////////////


template <class T> void CIsotropicForceT<T>::computeForces(CPSLocated &target)
{
	nlassert(CParticleSystem::InsideSimLoop);
	for (uint32 k = 0; k < _Owner->getSize(); ++k)
	{
		setupFunctor(k);
		TPSAttribVector::iterator speedIt = target.getSpeed().begin(), endSpeedIt = target.getSpeed().end();
		TPSAttribVector::const_iterator posIt = target.getPos().begin();
		TPSAttribFloat::const_iterator invMassIt = target.getInvMass().begin();

		for (; speedIt != endSpeedIt; ++speedIt, ++posIt, ++invMassIt)
		{
			_F(*posIt, *speedIt, *invMassIt);
		}
	}
}


/**
 *  a force that has the same direction everywhere. Mass is also taken in account (which is not the case for gravity)
 */

class CPSDirectionnalForce : public CPSForceIntensityHelper, public CPSDirection
{
	public:
	/// Compute the force on the targets
	virtual void computeForces(CPSLocated &target);

	/// Show the force (edition mode)
	virtual void show();



	CPSDirectionnalForce(float i = 1.f)
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("DirectionnalForce");
		setIntensity(i);
		_Dir = NLMISC::CVector(0, 0, -1);
	}

	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);


	NLMISC_DECLARE_CLASS(CPSDirectionnalForce);

	///\name From CPSDirection
	//@{
		/// set the direction of the force
		virtual void setDir(const NLMISC::CVector &dir) { _Dir = dir; }
		/// get the direction of the force
		virtual NLMISC::CVector getDir(void) const  { return _Dir; }
		// Tells that a global vector value of the system can be used as a direction
		virtual bool supportGlobalVectorValue() const { return true; }
		/// Bind the direction to a global vaariable (e.g "WIND" ). The value can be changed in CParticleSystem::setGlobalVectorValue
		virtual void				enableGlobalVectorValue(const std::string &name);
		// See if the direction is bound to a global variable. Return an empty string if not
		virtual std::string         getGlobalVectorValueName() const;
	//@}
protected:
	NLMISC::CVector								_Dir;
	CParticleSystem::CGlobalVectorValueHandle   _GlobalValueHandle; // a global vector value may override the value of _Dir (example of use : global direction for wind)
};


/// a gravity class. Mass isn't taken in account (true with a uniform gravity model, near earth )
class CPSGravity : public CPSForceIntensityHelper
{
public:
	/// Compute the force on the targets
	virtual void computeForces(CPSLocated &target);

	/// Show the force (edition mode)
	virtual void show();



	CPSGravity(float g = 9.8f)
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("Gravity");
		setIntensity(g);
	}

	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	NLMISC_DECLARE_CLASS(CPSGravity);


	virtual bool		 isIntegrable(void) const;

	/// inherited from CPSForce
	virtual void integrate(float date, CPSLocated *src, uint32 startIndex, uint32 numObjects, NLMISC::CVector *destPos = NULL, NLMISC::CVector *destSpeed = NULL,
							bool accumulate = false,
							uint posStride = sizeof(NLMISC::CVector), uint speedStride = sizeof(NLMISC::CVector)
							) const;

	virtual void integrateSingle(float startDate, float deltaT, uint numStep,
								 const CPSLocated *src, uint32 indexInLocated,
								 NLMISC::CVector *destPos,
								 bool accumulate = false,
								 uint posStride = sizeof(NLMISC::CVector)) const;

protected:
	/// inherited from CPSForceIntensityHelper
	virtual void setIntensity(float value);
	/// inherited from CPSForceIntensityHelper
	virtual void setIntensityScheme(CPSAttribMaker<float> *scheme);
};


/// a central gravity class. Mass is taken in account here
class CPSCentralGravity : public CPSForceIntensityHelper
{
public:
	/// Compute the force on the targets
	virtual void computeForces(CPSLocated &target);

	/// Show the force (edition mode)
	virtual void show();



	CPSCentralGravity(float i = 1.f)
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("CentralGravity");
		setIntensity(i);
	}

	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);


	NLMISC_DECLARE_CLASS(CPSCentralGravity);
};


/// a spring class
class CPSSpring : public CPSForceIntensityHelper
{
public:

	/// ctor : k is the coefficient of the spring
	CPSSpring(float k = 1.0f)
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("Spring");
		setIntensity(k);
	}


	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);


	/// Compute the force on the targets
	virtual void computeForces(CPSLocated &target);

	/// Show the force (edition mode)
	virtual void show();


	NLMISC_DECLARE_CLASS(CPSSpring);

};



/// a fluid friction functor, it is used by the fluid friction class
class CPSFluidFrictionFunctor
{
public:
	CPSFluidFrictionFunctor() : _K(1.f)
	{
	}

	virtual ~CPSFluidFrictionFunctor() {}

	#ifdef NL_OS_WINDOWS
		__forceinline
	#endif
	void operator() (const NLMISC::CVector &/* pos */, NLMISC::CVector &speed, float invMass)
	{
		speed -= (CParticleSystem::EllapsedTime * _K * invMass * speed);
	}

	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1);
		// we don't save intensity info : it is saved by the owning object (and set before each use of this functor)
	}

	// get the friction coefficient
	float getK(void) const { return _K; }

	// set the friction coefficient
	void setK(float coeff) { _K = coeff; }
protected:
	// the friction coeff
	float _K;
};


/** the fluid friction force. We don't derive from CPSForceIntensityHelper (which derives from CPSForce
  * , because CIsotropicForceT also derives from CPSForce, and we don't want to use virtual inheritance
  */


class CPSFluidFriction : public CIsotropicForceT<CPSFluidFrictionFunctor>, public CPSForceIntensity
{
public:
	// create the force with a friction coefficient
	CPSFluidFriction(float frictionCoeff = 1.f)
	{
		setIntensity(frictionCoeff);
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("FluidFriction");
	}

	// inherited from CIsotropicForceT
	virtual void setupFunctor(uint32 index)
	{
		_F.setK(_IntensityScheme ? _IntensityScheme->get(_Owner, index) : _K);
	}

	NLMISC_DECLARE_CLASS(CPSFluidFriction)


	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1);
		CIsotropicForceT<CPSFluidFrictionFunctor>::serial(f);
		serialForceIntensity(f);
		if (f.isReading())
		{
			registerToTargets();
		}
	}


protected:
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner; }
	virtual void newElement(const CPSEmitterInfo &info) { newForceIntensityElement(info); }
	virtual void deleteElement(uint32 index) { deleteForceIntensityElement(index); }
	virtual void resize(uint32 size) { resizeForceIntensity(size); }
};


/** A Brownian motion
  */


class CPSBrownianForce : public CPSForceIntensityHelper
{
public:
	// create the force with a friction coefficient
	CPSBrownianForce(float intensity = 1.f);

	NLMISC_DECLARE_CLASS(CPSBrownianForce)

	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// We provide a kind of integration on a predefined sequence
	virtual bool		 isIntegrable(void) const;

	virtual void integrate(float date, CPSLocated *src, uint32 startIndex, uint32 numObjects, NLMISC::CVector *destPos = NULL, NLMISC::CVector *destSpeed = NULL,
							bool accumulate = false,
							uint posStride = sizeof(NLMISC::CVector), uint speedStride = sizeof(NLMISC::CVector)
							) const;

	virtual void integrateSingle(float startDate, float deltaT, uint numStep,
								 const CPSLocated *src, uint32 indexInLocated,
								 NLMISC::CVector *destPos,
								 bool accumulate = false,
								 uint posStride = sizeof(NLMISC::CVector)) const;

	/// perform initialisations
	static void initPrecalc();

	void setIntensity(float value);
	void setIntensityScheme(CPSAttribMaker<float> *scheme);

	/// Compute the force on the targets
	virtual void computeForces(CPSLocated &target);

	void show()  {}


	/** When used with parametric integration, this tells factor tells how fast the force acts on particle
	  * (how fast it go through the lookup table in fact)
	  */
	void	setParametricFactor(float factor) {  _ParametricFactor = factor; }
	float   getParametricFactor() const { return _ParametricFactor; }

protected:
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner; }
	virtual void newElement(const CPSEmitterInfo &info) { newForceIntensityElement(info); }
	virtual void deleteElement(uint32 index) { deleteForceIntensityElement(index); }
	virtual void resize(uint32 size) { resizeForceIntensity(size); }

	float  _ParametricFactor; // tells how fast this force act on a particle when parametric motion is used
	static NLMISC::CVector PrecomputedPos[]; // after the sequence we must be back to the start position
	static NLMISC::CVector PrecomputedSpeed[];

	/// various impulsion for normal motion
	static NLMISC::CVector PrecomputedImpulsions[];

};


/// a turbulence force functor

struct CPSTurbulForceFunc
{
	virtual ~CPSTurbulForceFunc() {}

	#ifdef NL_OS_WINDOWS
		__forceinline
	#endif
	void operator() (const NLMISC::CVector &/* pos */, NLMISC::CVector &/* speed */, float /* invMass */)
	{
		nlassert(0);

		// TODO : complete that

	/*	static const NLMISC::CVector v1(1.235f, - 45.32f, 157.5f);
		static const NLMISC::CVector v2(-0.35f, 7.77f, 220.77f);


		speed += ellapsedTime * _Intensity
			   * NLMISC::CVector(2.f * (-0.5f + CPSUtil::buildPerlinNoise(_Scale * pos, _NumOctaves))
						 , 2.f * (-0.5f +  CPSUtil::buildPerlinNoise(_Scale * (pos +  v1) , _NumOctaves))
						 , 2.f * (-0.5f +  CPSUtil::buildPerlinNoise(_Scale * (pos +  v2) , _NumOctaves))
						 );
						 */
	}

	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	 {
		f.serialVersion(1);
		f.serial(_Scale, _NumOctaves);
	 }

	float _Scale;
	float _Intensity;
	uint32 _NumOctaves;
};



// the turbulence force

class CPSTurbul : public CIsotropicForceT<CPSTurbulForceFunc>, public CPSForceIntensity
{
public:
	// create the force with a friction coefficient
	CPSTurbul(float scale = 1.f , uint numOctaves = 4)
	{
		nlassert(numOctaves > 0);
		setScale(scale);
		setNumOctaves(numOctaves);
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("Turbulence");
	}


	float getScale(void) const { return _F._Scale; }
	void setScale(float scale) { _F._Scale = scale; }


	uint getNumOctaves(void) const { return _F._NumOctaves; }
	void setNumOctaves(uint numOctaves) { _F._NumOctaves = numOctaves; }


	NLMISC_DECLARE_CLASS(CPSTurbul)

	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1);
		CIsotropicForceT<CPSTurbulForceFunc>::serial(f);
		serialForceIntensity(f);
		if (f.isReading())
		{
			registerToTargets();
		}
	}

	// inherited from CIsotropicForceT
	virtual void setupFunctor(uint32 index)
	{
		_F._Intensity = (_IntensityScheme ? _IntensityScheme->get(_Owner, index) : _K);
	}

protected:
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner; }
	virtual void newElement(const CPSEmitterInfo &info) { newForceIntensityElement(info); }
	virtual void deleteElement(uint32 index) { deleteForceIntensityElement(index); }
	virtual void resize(uint32 size) { resizeForceIntensity(size); }
};




/** a cylindric vortex. It has a limited extend
  * It has unlimited extension in the z direction
  * The model is aimed at tunability rather than realism
  */

class CPSCylindricVortex : public CPSForceIntensityHelper, public IPSMover
{
public:
	/// Compute the force on the targets
	virtual void computeForces(CPSLocated &target);

	/// Show the force (edition mode)
	virtual void show();


	CPSCylindricVortex(float intensity = 1.f) : _RadialViscosity(.1f), _TangentialViscosity(.1f)
	{
		setIntensity(intensity);
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("Cylindric Vortex");
	}

	// inherited from IPSMover
	virtual bool supportUniformScaling(void) const { return true; }
	virtual bool supportNonUniformScaling(void) const { return false; }
	virtual void setScale(uint32 k, float scale) { _Radius[k] = scale; }
	virtual NLMISC::CVector getScale(uint32 k) const { return NLMISC::CVector(_Radius[k], _Radius[k], _Radius[k]); }
	virtual bool onlyStoreNormal(void) const { return true; }
	virtual NLMISC::CVector getNormal(uint32 index) { return _Normal[index]; }
	virtual void setNormal(uint32 index, NLMISC::CVector n) { _Normal[index] = n; }

	virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);
	virtual NLMISC::CMatrix getMatrix(uint32 index) const;


	void setRadialViscosity(float v) { _RadialViscosity = v; }
	float getRadialViscosity(void) const { return _RadialViscosity; }

	void setTangentialViscosity(float v) { _TangentialViscosity = v; }
	float getTangentialViscosity(void) const { return _TangentialViscosity; }

	NLMISC_DECLARE_CLASS(CPSCylindricVortex);



	// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);





protected:

	/// inherited from CPSForceIntensity
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner; }

	// the normal of the vortex
	CPSAttrib<NLMISC::CVector> _Normal;
	// radius of the vortex
	TPSAttribFloat _Radius;

	// radial viscosity : when it is near of 1, if tends to set the radial componenent of speed to 0
	float _RadialViscosity;

	// tangential viscosity : when set to 1, the tangential speed immediatly reach what it would be in a real vortex (w = 1 / r2)
	float _TangentialViscosity;

	virtual void newElement(const CPSEmitterInfo &info);
	virtual void deleteElement(uint32 index);
	virtual void resize(uint32 size);

};




/**
 *  a magnetic field that has the given direction
 */
class CPSMagneticForce : public CPSDirectionnalForce
{
	public:
	CPSMagneticForce(float i = 1.f)  : CPSDirectionnalForce(i)
	{
		if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("MagneticForce");
	}
	virtual void computeForces(CPSLocated &target);
	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CPSMagneticForce);
};





} // NL3D


#endif // NL_PS_FORCE_H

/* End of ps_force.h */
