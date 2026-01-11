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

#ifndef NL_PS_FLOAT_H
#define NL_PS_FLOAT_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_attrib_maker_template.h"
#include "nel/3d/ps_attrib_maker_bin_op.h"
#include "nel/3d/ps_attrib_maker_helper.h"
#include "nel/3d/animation_time.h"
#include <algorithm>
#include "nel/misc/vector_h.h"

namespace NL3D {

template <>
inline const char *CPSAttribMaker<float>::getType() { return "float"; }

/// these are some attribute makers for float
/// This is a float blender class. It just blend between 2 values

class CPSFloatBlender : public CPSValueBlender<float>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatBlender);
	CPSFloatBlender(float startFloat = 0.1f , float endFloat = 1.f, float nbCycles = 1.0f) : CPSValueBlender<float>(nbCycles)
	{
		_F.setValues(startFloat, endFloat);
	}
	CPSAttribMakerBase *clone() const { return new CPSFloatBlender(*this); }
	// F is serialized by base classes...

};


/// This is a float gradient class
class CPSFloatGradient : public CPSValueGradient<float>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatGradient);


	CPSFloatGradient() : CPSValueGradient<float>(1.f) {}

	/**
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSFloatGradient(const float *floatTab, uint32 nbValues, uint32 nbStages, float nbCycles = 1.0f);


	CPSAttribMakerBase *clone() const { return new CPSFloatGradient(*this); }
	// F is serialized by base classes...
};

/** this memorize float by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSFloatMemory : public CPSAttribMakerMemory<float>
{
public:
	CPSFloatMemory() { setDefaultValue(0.f); }
	NLMISC_DECLARE_CLASS(CPSFloatMemory);
	CPSAttribMakerBase *clone() const { return new CPSFloatMemory(*this); }

};

/** An attribute maker whose output is the result of a binary op on floats
  *
  */
class CPSFloatBinOp : public CPSAttribMakerBinOp<float>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatBinOp);
	CPSAttribMakerBase *clone() const { return new CPSFloatBinOp(*this); }
};

/** this functor produce float based on a hermite curve
  * NB : must be init before use, or assert occurs
  */
class CPSFloatCurveFunctor
{
	public:
		struct CCtrlPoint
		{
			CCtrlPoint() {}
			CCtrlPoint(float date, float value) : Date(date), Value(value) { nlassert(Date >= 0 && Date <= 1); }
			float Date;
			float Value;
			void serial(NLMISC::IStream &f)
			{
				f.serial(Date, Value);
			}
		};

		/** ctor. The default is a cst function whose value is .5
		  * NB : must be init before use, or assert occurs
		  */
		CPSFloatCurveFunctor();

		/** Add a control point. There is a sorted insertion based on the date
		  * \param index renge from 0 to 3
		  */
		void							addControlPoint(const CCtrlPoint &ctrlPoint);

		/// retrieve the number of control points
		uint							getNumCtrlPoints(void) const { return (uint)_CtrlPoints.size(); }

		/** get a control point.
		  * \return a <date, value> std::pair
		  */
		const CCtrlPoint				&getControlPoint(uint index) const;

		/// modify the given ctrl point
		void							setCtrlPoint(uint index, const CCtrlPoint &ctrlPoint);

		///remove the ctrl point at the given index
		void							removeCtrlPoint(uint index);

		/// set the number of samples used with this curb
		void							setNumSamples(uint32 numSamples);

		/// get the numer of samples used with this curb
		uint32							getNumSamples(void) const { return _NumSamples; }

		/// Enable / diable smoothing. This cause to use hermite / linear curves.
		void							enableSmoothing(bool enable = true);

		/// test whether smoothing is enabled
		bool							hasSmoothing(void) const			{ return _Smoothing;}

		/** This return a sampled value from the hermite curb. The more steps there are, the more accurate it is
		  * You can also get an 'exact value'.
		  * This must be called between beginFastFloor() and endFastFloor() statements
		  * \see getValue
		  */
		#ifdef NL_OS_WINDOWS
			__forceinline
		#endif
		float operator()(TAnimationTime time) const
		{
			#ifdef NL_DEBUG
				nlassert(time >= 0.f && time <= 1.f);
			#endif
			return _Tab[NLMISC::OptFastFloor(time * _NumSamples)];
		}

		/// compute an exact value at the given date, which must be in [0, 1[
		float							getValue(float date) const;

		/// serialization
		void serial(NLMISC::IStream &f);

		float	getMinValue() const { return _MinValue; }
		float	getMaxValue() const { return _MaxValue; }

	protected:
		/// get the tangent (slope in our case) for the key at the given position
		float getSlope(uint index) const ;
		/// sort the ctrl points in increasing order
		void						sortPoints(void);
		/// update the value tab
		void						updateTab(void);
		CPSVector<CCtrlPoint>::V	_CtrlPoints;
		uint32						_NumSamples;
		CPSVector<float>::V			_Tab; // sampled version of the curve
		bool						_Smoothing;
		float						_MinValue;
		float						_MaxValue;
};


inline bool operator<(const CPSFloatCurveFunctor::CCtrlPoint &lhs, const CPSFloatCurveFunctor::CCtrlPoint &rhs)
{
	return lhs.Date < rhs.Date;
}


class CPSFloatCurve : public CPSAttribMakerT<float, CPSFloatCurveFunctor>
{
public:
	CPSFloatCurve() : CPSAttribMakerT<float, CPSFloatCurveFunctor>(1) {}
	NLMISC_DECLARE_CLASS(CPSFloatCurve);
	CPSAttribMakerBase *clone() const { return new CPSFloatCurve(*this); }
	virtual float getMinValue(void) const { return _F.getMinValue(); }
	virtual float getMaxValue(void) const { return _F.getMaxValue(); }
};



} // NL3D


#endif // NL_PS_FLOAT_H

/* End of ps_size.h */
