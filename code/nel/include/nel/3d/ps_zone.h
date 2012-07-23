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

#ifndef NL_PS_ZONE_H
#define NL_PS_ZONE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_edit.h"
#include "nel/3d/ps_attrib.h"
#include "nel/3d/ps_plane_basis.h"


namespace NL3D {


/** This epsilon is in meter and give a thickness to surfaces for tests. It must be above above 0
* for the system to work correctly
*/
const float PSCollideEpsilon = 10E-3f;



/**
 * This class hold any entity that has an effect over located : a sink , a bouncing zone etc
 * This is a kind a specialized force, and it has an attached list of the targets
 * Not sharable accross systems.
 *
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSZone : public CPSTargetLocatedBindable
{
public:

	/// behaviour when a collision occurs

	enum TCollisionBehaviour { bounce = 0, destroy = 1 };

	/// Constructor
	CPSZone();

	/**
	*  Gives the type for this bindable.
	*/
	virtual uint32 getType(void) const { return PSZone; }

	/**
	* Get the priority of the bindable
	* The more high it is, the earlier it is dealt with
	*/
	virtual uint32 getPriority(void) const { return 3500; }

	/**
	 * Process one pass for the zone
	 * The default behaviour call performMotion or show depending on the pass being processed
	 */
	virtual void step(TPSProcessPass pass);


	/// Show the zone (edition mode).
	virtual void show() = 0;


	/// Add a new type of located for this zone to apply on. nlassert if already present
	virtual void attachTarget(CPSLocated *ptr);


	/// serialization, DERIVER must override this, and call the parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);


	/** Inherited from CPSTargetLocatedBindable. It's called when one of the targets has been detroyed or detached
	 *  The default behaviour, release collision infos from the located
	 */

	virtual void releaseTargetRsc(CPSLocated *target);


	/// set the bounce factor. It has meaning only if the behaviour is set to bounce...
	void setBounceFactor(float bounceFactor) { _BounceFactor = bounceFactor; }

	/// get the bounce factor. It has meaning only if the behaviour is set to bounce...
	float getBounceFactor(void) const { return  _BounceFactor; }


	void setCollisionBehaviour(TCollisionBehaviour behaviour) { _CollisionBehaviour = behaviour; }

	TCollisionBehaviour getCollisionBehaviour(void) const { return _CollisionBehaviour; }

	/** Compute collisions for the given target. This will update the collisions infos.
	  * The caller must provide pointer to arrays positions before and after time step.
	  */
	virtual	void computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter) = 0;

protected:


	// the bounce factor. 1.f mean no energy loss
	float _BounceFactor;


	TCollisionBehaviour _CollisionBehaviour;

	/**
	 * This set speed of a located so that it looks like bouncing on a surface
	 * \param locatedIndex the index
	 * \param bouncePoint the position where the collision occured
	 * \param surfNormal  the normal of the surface at the collision point (this must be a unit vector)
	 * \elasticity  1 = full bounce, 0 = no bounce (contact)
	 * \ellapsedTime the time ellapsed
	 */

//	void bounce(uint32 locatedIndex, const NLMISC::CVector &bouncePoint, const NLMISC::CVector &surfNormal, float elasticity, float ellapsedTime);




};


/** A plane over which particles bounce
 * It has an interface to move each plane individually
 */

class CPSZonePlane : public CPSZone, public IPSMover
{
	public:
		virtual	void computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter);
		virtual void show();


		NLMISC_DECLARE_CLASS(CPSZonePlane);



		virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);
		virtual NLMISC::CMatrix getMatrix(uint32 index) const;
		virtual bool onlyStoreNormal(void) const { return true; }
		virtual NLMISC::CVector getNormal(uint32 index);
		virtual void setNormal(uint32 index, NLMISC::CVector n);

		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	protected:
		TPSAttribVector _Normal;
		NLMISC::CMatrix buildBasis(uint32 index) const;

		virtual void resize(uint32 size);

		virtual void newElement(const CPSEmitterInfo &info);

		virtual void deleteElement(uint32 index);
};




/// a radius and its suare in the same struct
struct CRadiusPair
{
	// the adius, and the square radius
	float R, R2;
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(R, R2);
	}
};


typedef CPSAttrib<CRadiusPair> TPSAttribRadiusPair;

/** A sphere
 */


class CPSZoneSphere : public CPSZone, public IPSMover
{
	public:
		virtual	void computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter);
		virtual void show();


		NLMISC_DECLARE_CLASS(CPSZoneSphere);

		CPSZoneSphere()
		{
			if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("ZoneSphere");
		}



		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);


		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true; }
		virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);
		virtual NLMISC::CMatrix getMatrix(uint32 index) const;
		virtual void setScale(uint32 k, float scale);
		virtual NLMISC::CVector getScale(uint32 k) const;


	protected:



		TPSAttribRadiusPair _Radius;

		NLMISC::CMatrix buildBasis(uint32 index) const;

		virtual void resize(uint32 size);

		virtual void newElement(const CPSEmitterInfo &info);

		virtual void deleteElement(uint32 index);
};

/// a disc

class CPSZoneDisc : public CPSZone, public IPSMover
{
	public:
		virtual	void computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter);
		virtual void show();

		CPSZoneDisc()
		{
			if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("ZoneDisc");
		}

		NLMISC_DECLARE_CLASS(CPSZoneDisc);


		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true; }
		virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);
		virtual NLMISC::CMatrix getMatrix(uint32 index) const;
		virtual void setScale(uint32 k, float scale);
		virtual NLMISC::CVector getScale(uint32 k) const;
		virtual bool onlyStoreNormal(void) const { return true; }
		virtual NLMISC::CVector getNormal(uint32 index);
		virtual void setNormal(uint32 index, NLMISC::CVector n);

		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);



	protected:
		TPSAttribVector _Normal;
		TPSAttribRadiusPair _Radius;

		NLMISC::CMatrix buildBasis(uint32 index) const;

		virtual void resize(uint32 size);

		virtual void newElement(const CPSEmitterInfo &info);

		virtual void deleteElement(uint32 index);

};



/// a caped cylinder


class CPSZoneCylinder : public CPSZone, public IPSMover
{
	public:
		virtual	void computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter);
		virtual void show();

		CPSZoneCylinder()
		{
			if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("ZoneCylinder");
		}

		NLMISC_DECLARE_CLASS(CPSZoneCylinder);

		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true; }
		virtual bool supportNonUniformScaling(void) const { return true; }
		virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);
		virtual NLMISC::CMatrix getMatrix(uint32 index) const;
		virtual void setScale(uint32 k, float scale);
		virtual void setScale(uint32 k, const NLMISC::CVector &s);
		virtual NLMISC::CVector getScale(uint32 k) const;

		// serialization
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);





	protected:

		// the I and J vector of the cylinder
		CPSAttrib<CPlaneBasis> _Basis;

		// dimension of cylinder in each direction, encoded in a vector
		TPSAttribVector _Dim;

		NLMISC::CMatrix buildBasis(uint32 index) const;

		virtual void resize(uint32 size);

		virtual void newElement(const CPSEmitterInfo &info);

		virtual void deleteElement(uint32 index);
};



/**
 *	The same as a plane, but with a rectangle. We don't encode the plane by its normal, however...
 */

class CPSZoneRectangle : public CPSZone, public IPSMover
{
	public:
		virtual	void computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter);
		virtual void show();

		CPSZoneRectangle()
		{
			if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("CPSZoneRectangle");
		}

		NLMISC_DECLARE_CLASS(CPSZoneRectangle);





		// serialization
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true; }
		virtual bool supportNonUniformScaling(void) const { return true; }
		virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);
		virtual NLMISC::CMatrix getMatrix(uint32 index) const;
		virtual void setScale(uint32 index, float scale);
		virtual void setScale(uint32 index, const NLMISC::CVector &s);
		virtual NLMISC::CVector getScale(uint32 index) const;

	protected:


		CPSAttrib<CPlaneBasis> _Basis;

		//  width
		TPSAttribFloat _Width;

		//  Height
		TPSAttribFloat _Height;


		NLMISC::CMatrix buildBasis(uint32 index) const;

		virtual void resize(uint32 size);

		virtual void newElement(const CPSEmitterInfo &info);

		virtual void deleteElement(uint32 index);

};




} // NL3D


#endif // NL_PS_ZONE_H

/* End of ps_zone.h */
