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

#ifndef NL_PS_RIBBON_BASE_H
#define NL_PS_RIBBON_BASE_H


#include "nel/3d/ps_particle_basic.h"
#include "nel/misc/object_vector.h"


namespace NL3D
{

/** Base class for ribbons. If can be used to compute ribbons trajectory.
  * It can perform hermitte or linear interpolation.
  * to get the ribbon shape. It can also be used to have fixed size ribbons.
  * NB : Ribbons that don't herit from this are deprecated but may be kept for compatibility.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CPSRibbonBase : public CPSParticle, public CPSTailParticle
{
public:
	// Coord. system in which trail will reside
	enum TMatrixMode
	{
		FXWorldMatrix = 0,
		IdentityMatrix,
		UserMatrix,
		FatherMatrix, // father located matrix (introduced for backward compatibility)
		MatrixModeCount
	};
	enum TRibbonMode		{ VariableSize = 0, FixedSize, RibbonModeLast };
	enum TInterpolationMode { Linear = 0, Hermitte, InterpModeLast };

	///\name Object
	///@{
		CPSRibbonBase();
		/// serialisation. Derivers must override this, and call their parent version
		virtual void			serial(NLMISC::IStream &f);
	///@}

	///\name Behaviour
	///@{
		/// NB : a fixed size isn't applied with parametric motion.
		void					setRibbonMode(TRibbonMode mode);
		TRibbonMode				getRibbonMode() const { return _RibbonMode; }
		void					setInterpolationMode(TInterpolationMode mode);
		TInterpolationMode		getInterpolationMode() const { return _InterpolationMode; }
		// Set the coordinate system in which the trail will be created
		void					setMatrixMode(TMatrixMode matrixMode);
		TMatrixMode				getMatrixMode() const { return _MatrixMode; }
	///@}

	///\name Geometry
	///@{
		/// set the number of segments used with this particle. In this case, it can't be lower than 2
		void				setTailNbSeg(uint32 nbSegs);
		/// get the number of segments used with this particle
		uint32				getTailNbSeg(void) const { return _NbSegs; }
		/** Set how many seconds need a seg to be traversed. Long times will create longer ribbons. Default is 0.02.
		  * It gives the sampling rate for each type of ribbon
		  */
		void				setSegDuration(TAnimationTime ellapsedTime);
		TAnimationTime		getSegDuration(void) const { return _SegDuration; }

		/** The length in meter of the ribbon. This is used only if the ribbon mode is set to FixedSize.
		* These kind of ribbon are usually slower than variable size ribbons.
		* The default is one metter.
		*/
		void					setRibbonLength(float length);
		float			        getRibbonLength() const { return _RibbonLength; }
	///@}

	/** Allow degradation of ribbons with distance of the system (may not be suited when theit paths have wicked angles)
	  * \param percent 1 mean no degradation, 0 mean nothing will be draw when the system is at its max dist. 1 is the default
	  */
	void					setLODDegradation(float percent)
	{
		nlassert(percent >= 0 && percent <= 1);
		_LODDegradation = percent;
	}
	float					getLODDegradation() const { return _LODDegradation; }

protected:
	typedef CPSVector<NLMISC::CVector>::V	TPosVect;
	typedef	CPSVector<float>::V				TFloatVect; // all positions for each ribbons packed in a single vector

	uint32							  _NbSegs;
	TAnimationTime					  _SegDuration;
	bool							  _Parametric; // if this is set to true, then the owner has activated parametric motion.


	/// inherited from CPSLocatedBindable
	virtual void					newElement(const CPSEmitterInfo &info);
	/// inherited from CPSLocatedBindable
	virtual void					deleteElement(uint32 index);
	/// inherited from CPSLocatedBindable
	virtual void					resize(uint32 size);
	/// called when the motion type has changed, this allow us to draw smoother ribbons when parametric anim is used
	virtual	void					motionTypeChanged(bool parametric);

	/** Get position of the i-th ribbon and store them in a table of vector.
	  * It uses the interpolation setting of this object.
	  * The dest tab must have at least nbSegs + 1 entries.
	  */
	void							computeRibbon( uint index,
												   NLMISC::CVector *dest,
												   uint stride = sizeof(NLMISC::CVector)
												  );

	/// Called each time the time of the system change in order to update the ribbons positions
	void							updateGlobals();

	/// must be called for the lod to apply (updates UsedNbSegs)
	void                            updateLOD();

	// get index of the ribbons head in the sampling vect
	uint32							getRibbonIndex() const { return _RibbonIndex; }
	// get sampling date for each pos of the ribbon
	const TFloatVect				&getSamplingDate() const { return _SamplingDate; }

	/// value to use after lod computation
	uint32							  _UsedNbSegs;
	TAnimationTime					  _UsedSegDuration;
	float							  _UsedSegLength;

private:

	TFloatVect					      _SamplingDate;
	uint							  _RibbonIndex;  // indicate which is the first index for the ribbons head
	TMatrixMode						  _MatrixMode;
	TPosVect						  _Ribbons;
	TAnimationTime					  _LastUpdateDate;
	TRibbonMode						  _RibbonMode;
	TInterpolationMode				  _InterpolationMode;
	float							  _RibbonLength; // used if _RibbonMode == FixedSize
	float							  _SegLength;
	float							  _LODDegradation;

protected: // should be call by derivers for backward compatibility only
	void					initDateVect();
	void					resetFromOwner();
	inline const NLMISC::CMatrix  &getLocalToWorldTrailMatrix() const;
	// Convert matrix mode to the TPSMatrixMode enum.
	inline TPSMatrixMode convertMatrixMode() const;
private:
	void					resetSingleRibbon(uint index, const NLMISC::CVector &pos);


	/// copy datas from one ribbon to another
	void					dupRibbon(uint dest, uint src);

	/// Compute the ribbon points using linear interpolation between each sampling point.
	void					computeLinearRibbon( uint index,
											     NLMISC::CVector *dest,
										         uint stride = sizeof(NLMISC::CVector)
										       );
	/// The same as compute linear ribbon but try to make its length constant
	void					computeLinearCstSizeRibbon( uint index,
											     NLMISC::CVector *dest,
										         uint stride = sizeof(NLMISC::CVector)
										       );
	/// Compute the ribbon points using hermitte splines between each sampling point.
	void					computeHermitteRibbon( uint index,
											     NLMISC::CVector *dest,
										         uint stride = sizeof(NLMISC::CVector)
										       );

	/** Compute the ribbon points using hermitte splines between each sampling point,
	  * and make a rough approximation to get a constant length
	  */
	void					computeHermitteCstSizeRibbon( uint index,
											     NLMISC::CVector *dest,
										         uint stride = sizeof(NLMISC::CVector)
										       );
	// called by the system when its date has been manually changed
	virtual void			systemDateChanged();
};

/////////////
// INLINES //
/////////////

//=======================================================
inline const NLMISC::CMatrix &CPSRibbonBase::getLocalToWorldTrailMatrix() const
{
	NL_PS_FUNC(CPSRibbonBase_getLocalToWorldTrailMatrix)
	#ifdef NL_DEBUG
		nlassert(_Owner);
		nlassert(_Owner->getOwner());
	#endif
	return CPSLocated::getConversionMatrix(*_Owner->getOwner(), PSIdentityMatrix, convertMatrixMode());
}


//=======================================================
// Convert matrix mode to the TPSMatrixMode enum.
inline TPSMatrixMode CPSRibbonBase::convertMatrixMode() const
{
	NL_PS_FUNC(CPSRibbonBase_convertMatrixMode)
	if (_MatrixMode == FatherMatrix)
	{
		#ifdef NL_DEBUG
			nlassert(_Owner);
		#endif
		return _Owner->getMatrixMode();
	}
	return (TPSMatrixMode) _MatrixMode;
}


} // NL3D


#endif // NL_PS_RIBBON_BASE_H

/* End of ps_ribbon_base.h */
