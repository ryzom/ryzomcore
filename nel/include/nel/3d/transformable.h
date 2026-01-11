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

#ifndef NL_TRANSFORMABLE_H
#define NL_TRANSFORMABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/quat.h"
#include "nel/3d/animatable.h"
#include "nel/3d/animated_value.h"
#include "nel/3d/track.h"


namespace NL3D
{

class	CChannelMixer;

using NLMISC::CMatrix;
using NLMISC::CVector;
using NLMISC::CQuat;


/**
 * Something which can be transformed in 3D space / animated.
 * By default Transformmode is RotQuat.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class ITransformable : public IAnimatable
{
public:

	enum	TTransformMode
	{
		DirectMatrix=0,		// DirectMatrixMode.
		RotEuler,			// Matrix is computed from sperated composantes, with euler rotation.
		RotQuat,			// Matrix is computed from sperated composantes, with quat rotation (default).

		TransformModeCount
	};


public:

	/// Constructor. By default, RotQuat mode.
	ITransformable();
	virtual ~ITransformable() {}


	/// Get the matrix, compute her if necessary (work in all modes).
	const CMatrix	&getMatrix() const {updateMatrix(); return _LocalMatrix;}


	/** Tells if the LocalMatrix is newer than what caller except.
	 * This return true either if the matrix components (pos/rot etc...) are touched, or if matrix is newer than caller date.
	 */
	bool			compareMatrixDate(uint64 callerDate) const
	{
		return callerDate<_LocalMatrixDate || needCompute();
	}


	/** return the last date of computed matrix. updateMatrix() if necessary.
	 */
	uint64			getMatrixDate() const
	{
		updateMatrix();
		return _LocalMatrixDate;
	}


	/// \name Transform Mode.
	//@{
	/// Change the transform mode. Components or matrix are not reseted.
	void	setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro= CMatrix::ZXY)
	{
		_Mode= mode;
		_RotOrder= ro;
		// just for information.
		touch(PosValue, OwnerBit);
	}
	//@}


	/// \name Matrix operations.
	//@{

	/// Work only in Rot* mode(nlassert).
	void	setPos(const CVector &pos)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		_Pos.Value= pos;
		touch(PosValue, OwnerBit);
	}
	/// Work only in Rot* mode(nlassert).
	void	setPos(float x, float y, float z)
	{
		setPos(CVector(x,y,z));
	}
	/// Work only in RotEuler mode(nlassert).
	void	setRotEuler(const CVector &rot)
	{
		nlassert(_Mode==RotEuler);
		_RotEuler.Value= rot;
		touch(RotEulerValue, OwnerBit);
	}
	/// Work only in RotEuler mode(nlassert).
	void	setRotEuler(float rotX, float rotY, float rotZ)
	{
		setRotEuler(CVector(rotX, rotY, rotZ));
	}
	/// Work only in RotQuat mode (nlassert).
	void	setRotQuat(const CQuat &quat)
	{
		nlassert(_Mode==RotQuat);
		_RotQuat.Value= quat;
		touch(RotQuatValue, OwnerBit);
	}
	/// Work only in Rot* mode (nlassert).
	void	setScale(const CVector &scale)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		_Scale.Value= scale;
		touch(ScaleValue, OwnerBit);
	}
	/// Work only in Rot* mode (nlassert).
	void	setScale(float scaleX, float scaleY, float scaleZ)
	{
		setScale(CVector(scaleX, scaleY, scaleZ));
	}
	/// Work only in Rot* mode (nlassert).
	void	setScale(float scale)
	{
		setScale(CVector(scale, scale, scale));
	}
	/// Work only in Rot* mode (nlassert).
	void	setPivot(const CVector &pivot)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		_Pivot.Value= pivot;
		touch(PivotValue, OwnerBit);
	}
	/// Work only in Rot* mode (nlassert).
	void	setPivot(float x, float y, float z)
	{
		setPivot(CVector(x, y, z));
	}
	/// Work only in DirecTMatrix mode (nlassert).
	void	setMatrix(const CMatrix &mat)
	{
		nlassert(_Mode==DirectMatrix);
		_LocalMatrix= mat;
		// The matrix has changed.
		_LocalMatrixDate++;
	}

	//@}


	/// \name Matrix Get operations.
	//@{

	/// get the current transform mode.
	TTransformMode	getTransformMode()
	{
		return _Mode;
	}
	/// get the current rotorder (information vlaid only when RotEuler mode).
	CMatrix::TRotOrder	getRotOrder()
	{
		return _RotOrder;
	}

	/// Work only in Rot* mode(nlassert).
	void	getPos(CVector &pos)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		pos= _Pos.Value;
	}
	/// Work only in RotEuler mode(nlassert).
	void	getRotEuler(CVector &rot)
	{
		nlassert(_Mode==RotEuler);
		rot= _RotEuler.Value;
	}
	/// Work only in RotQuat mode (nlassert).
	void	getRotQuat(CQuat &quat)
	{
		nlassert(_Mode==RotQuat);
		quat= _RotQuat.Value;
	}
	/// Work only in Rot* mode (nlassert).
	void	getScale(CVector &scale)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		scale= _Scale.Value;
	}
	/// Work only in Rot* mode (nlassert).
	void	getPivot(CVector &pivot)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		pivot= _Pivot.Value;
	}

	/// Work only in Rot* mode(nlassert).
	CVector	getPos()
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		return _Pos.Value;
	}
	/// Work only in RotEuler mode(nlassert).
	CVector	getRotEuler()
	{
		nlassert(_Mode==RotEuler);
		return _RotEuler.Value;
	}
	/// Work only in RotQuat mode (nlassert).
	CQuat	getRotQuat()
	{
		nlassert(_Mode==RotQuat);
		return _RotQuat.Value;
	}
	/// Work only in Rot* mode (nlassert).
	CVector	getScale()
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		return _Scale.Value;
	}
	/// Work only in Rot* mode (nlassert).
	CVector getPivot()
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		return _Pivot.Value;
	}
	//@}


	/// \name Misc
	// @{
	/**
	  * Setup Matrix by the lookAt method. Work only in DirectMatrix mode and RotQuat mode (not euler...).
	  *
	  * \param eye is the coordinate of the object.
	  * \param target is the point the object look at.
	  * \param roll is the roll angle in radian along the object's Y axis.
	  */
	void		lookAt (const CVector& eye, const CVector& target, float roll=0.f);
	// @}



	/// \name Herited from IAnimatable
	// @{
	/// Added values.
	enum	TAnimValues
	{
		OwnerBit= IAnimatable::AnimValueLast,
		PosValue,
		RotEulerValue,
		RotQuatValue,
		ScaleValue,
		PivotValue,
		AnimValueLast
	};

	/// From IAnimatable
	virtual IAnimatedValue* getValue (uint valueId);

	/// From IAnimatable
	virtual const char *getValueName (uint valueId) const;

	/// From IAnimatable. Deriver must implement this.
	virtual ITrack* getDefaultTrack (uint valueId) =0;

	/// From IAnimatable. Deriver must implement this (channels may be detail-ed or not).
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix) =0;

	// @}


	static const char *getPosValueName ();
	static const char *getRotEulerValueName();
	static const char *getRotQuatValueName();
	static const char *getScaleValueName();
	static const char *getPivotValueName();


private:
	// The computed matrix.
	mutable CMatrix			_LocalMatrix;
	TTransformMode			_Mode;
	CMatrix::TRotOrder		_RotOrder;
	mutable	uint64			_LocalMatrixDate;

	// For animation, Pos, rot scale pivot animated values
	CAnimatedValueVector	_Pos;
	CAnimatedValueVector	_RotEuler;
	CAnimatedValueQuat		_RotQuat;
	CAnimatedValueVector	_Scale;
	CAnimatedValueVector	_Pivot;

	// clear transform flags.
	void	clearTransformFlags() const;

	// compute the matrix.
	void	updateMatrix() const;

	/// Tells if the matrix needs to be computed, ie, if data are modified.
	bool			needCompute() const
	{
		return  _Mode!=DirectMatrix && isTouched(OwnerBit);
	}

};


} // NL3D


#endif // NL_TRANSFORMABLE_H

/* End of transformable.h */
