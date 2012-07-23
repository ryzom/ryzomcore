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

#ifndef NL_PLANE_BASIS_MAKER_H
#define NL_PLANE_BASIS_MAKER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_attrib_maker_template.h"
#include "nel/3d/ps_attrib_maker_bin_op.h"
#include "nel/3d/ps_plane_basis.h"
#include "nel/misc/fast_floor.h"
#include "nel/misc/object_vector.h"


namespace NL3D {

template <>
inline const char *CPSAttribMaker<CPlaneBasis>::getType() { return "CPlaneBasis";}

/** these are some attribute makers for plane_basis
 * This is a plane basis class. It just blend between 2 plane by linearly interpolating the normal
 * (non sampled version)
 */

class CPSPlaneBasisBlender : public CPSValueBlender<CPlaneBasis>
{
public:
	NLMISC_DECLARE_CLASS(CPSPlaneBasisBlender);

	CPSPlaneBasisBlender(const CPlaneBasis &startBasis = CPlaneBasis(NLMISC::CVector::I), const CPlaneBasis &endBasis = CPlaneBasis(NLMISC::CVector::J), float nbCycles = 1.0f) : CPSValueBlender<CPlaneBasis>(nbCycles)
	{
		_F.setValues(startBasis, endBasis);
	}
	CPSAttribMakerBase *clone() const { return new CPSPlaneBasisBlender(*this); }
};


/// This is a PlaneBasis gradient class
class CPSPlaneBasisGradient : public CPSValueGradient<CPlaneBasis>
{
public:
	NLMISC_DECLARE_CLASS(CPSPlaneBasisGradient);

	/**
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSPlaneBasisGradient(const CPlaneBasis *basisTab = CPSPlaneBasisGradient::DefaultPlaneBasisTab
		, uint32 nbValues = 2, uint32 nbStages = 16, float nbCycles = 1.0f) : CPSValueGradient<CPlaneBasis>(nbCycles)
	{
		_F.setValues(basisTab, nbValues, nbStages);
	}
	CPSAttribMakerBase *clone() const { return new CPSPlaneBasisGradient(*this); }
	static CPlaneBasis DefaultPlaneBasisTab[];
};



/** this is a 'follow direction' plane basis maker
 * It set the plane basis to have its normal in the same direction than speed of the located
 * The cycle param has no effect o the direction
 */
class CPSPlaneBasisFollowSpeed : public CPSAttribMaker<CPlaneBasis>
{
	public:
		enum TProjectionPlane { NoProjection = 0, XY, XZ, YZ, ProjectionPlaneLast /* enum counter */ };
	public:
		CPSPlaneBasisFollowSpeed() : CPSAttribMaker<CPlaneBasis>(1), _ProjectionPlane(NoProjection) {}

		/// compute one value of the attribute for the given index
		virtual CPlaneBasis get(CPSLocated *loc, uint32 index);
		virtual CPlaneBasis get(const CPSEmitterInfo &infos);

		/** Fill tab with an attribute by using the given stride. It fills numAttrib attributes.
		 *  \param loc the 'located' that hold the 'located bindable' that need an attribute to be filled
		 *  \param startIndex usually 0, it gives the index of the first element in the located
		 */

		virtual void *make(CPSLocated *loc,
						   uint32 startIndex,
						   void *tab,
						   uint32 stride,
						   uint32 numAttrib,
						   bool enableNoCopy = false,
						   uint32 srcStep = (1 << 16),
						   bool forceClampEntry = false
						  ) const;

		/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
		 *  \see make()
		 */
		virtual void make4(CPSLocated *loc,
						   uint32 startIndex,
						   void *tab,
						   uint32 stride,
						   uint32 numAttrib,
						   uint32 srcStep = (1 << 16)
						  ) const;


		/** the same as make4, but with nbReplicate replication isntead of 4
		 *  \see make4
		 */
		virtual void makeN(CPSLocated *loc,
						   uint32 startIndex,
						   void *tab,
						   uint32 stride,
						   uint32 numAttrib,
						   uint32 nbReplicate,
						   uint32 srcStep = (1 << 16)
						  ) const;

		NLMISC_DECLARE_CLASS(CPSPlaneBasisFollowSpeed);

		/// serialization
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			// version 2 : added projection plane
			// version 1 : nothing to save here
			sint ver = f.serialVersion(2);
			if (ver >= 2)
			{
				f.serialEnum(_ProjectionPlane);
			}
			else
			{
				_ProjectionPlane = NoProjection;
			}
		}
		CPSAttribMakerBase *clone() const { return new CPSPlaneBasisFollowSpeed(*this); }
		//
		TProjectionPlane getProjectionPlane() const { return _ProjectionPlane; }
		void			 setProjectionPlane(TProjectionPlane pp) { _ProjectionPlane = pp; }
	private:
		TProjectionPlane _ProjectionPlane;
};




/** this memorize value by applying some function based on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generated
  */
class CPSPlaneBasisMemory : public CPSAttribMakerMemory<CPlaneBasis>
{
public:
	CPSPlaneBasisMemory() { setDefaultValue(CPlaneBasis(NLMISC::CVector::K)); }
	NLMISC_DECLARE_CLASS(CPSPlaneBasisMemory);
	CPSAttribMakerBase *clone() const { return new CPSPlaneBasisMemory(*this); }
};


/** An attribute maker whose output if the result of a binary op on plane basis
  *
  */
class CPSPlaneBasisBinOp : public CPSAttribMakerBinOp<CPlaneBasis>
{
public:
	NLMISC_DECLARE_CLASS(CPSPlaneBasisBinOp);
	CPSAttribMakerBase *clone() const { return new CPSPlaneBasisBinOp(*this); }
};


// a functor object that produce basis by applying a rotation over a fixed axis
class CSpinnerFunctor
{
public:
	CSpinnerFunctor();
	const CPlaneBasis &operator()(float date) const { return _PBTab[NLMISC::OptFastFloor(date * _NbSamples)]; }
	/// set the rotation axis
	void					setAxis(const NLMISC::CVector &axis);
	/// get the rotation axis
	const NLMISC::CVector   getAxis(void) const { return _Axis;}
	/// set the number of samples for the rotation
	void setNumSamples(uint32 nbSamples);
	/// get the number of samples for the rotation
	uint32 getNumSamples(void) const;
	/// serial this object
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
protected:
	CPSVector<CPlaneBasis>::V   _PBTab;
	uint32						_NbSamples;
	NLMISC::CVector				_Axis;
	/// update the samples tab
	void						updateSamples(void);
};



/// this is a spinner : this compute a basis by applying a rotation over the given axis
// nb : default init is done with nb samples = 0, so must set a number of samples that is > 0 before use
class CPSBasisSpinner : public CPSAttribMakerT<CPlaneBasis, CSpinnerFunctor>
{
public:
	CPSBasisSpinner() : CPSAttribMakerT<CPlaneBasis, CSpinnerFunctor>(1) {}
	NLMISC_DECLARE_CLASS(CPSBasisSpinner);
	CPSAttribMakerBase *clone() const { return new CPSBasisSpinner(*this); }
};



} // NL3D


#endif // NL_PLANE_BASIS_MAKER_H

/* End of plane_basis_maker.h */
