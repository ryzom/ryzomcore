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

#ifndef NL_U_TRANSFORMABLE_H
#define NL_U_TRANSFORMABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/quat.h"


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CQuat;


// ***************************************************************************
/**
 * Base interface for manipulating Movable Objects: camera, bones, lights, instances etc...
 * By default Transformmode is RotQuat.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UTransformable
{
protected:

public:
	// Enum should be the same than in ITransformable.

	// Matrix mode.
	enum	TTransformMode
	{
		DirectMatrix,		// DirectMatrixMode .
		RotEuler,			// Matrix is computed from sperated composantes, with euler rotation.
		RotQuat,			// Matrix is computed from sperated composantes, with quat rotation (default).

		TransformModeCount
	};


public:


	/// \name Position set
	// @{
	/// Change the transform mode. Components or matrix are not reseted.
	void			setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro= CMatrix::ZXY);
	/// Work only in Rot* mode(nlassert).
	void			setPos(const CVector &pos);
	/// Work only in Rot* mode(nlassert).
	void			setPos(float px, float py, float pz) {setPos(CVector(px, py, pz));}
	/// Work only in RotEuler mode(nlassert).
	void			setRotEuler(const CVector &rot);
	/// Work only in RotEuler mode(nlassert).
	void			setRotEuler(float rx, float ry, float rz) {setRotEuler(CVector(rx, ry, rz));}
	/// Work only in RotQuat mode (nlassert).
	void			setRotQuat(const CQuat &quat);
	/** Work only in RotQuat mode (nlassert).
	 * Build a quaternion from a forward direction (a J vector). there is no roll... jdir do not need to be noramlized.
	 */
	void			setRotQuat(const CVector &jdir);
	/** Work only in RotQuat mode (nlassert).
	 * Build a quaternion from a forward direction (a J vector). the roll is determined with help of the vector up vup... vectors do not need to be noramlized.
	 */
	void			setRotQuat(const CVector &jdir, const CVector &vup);
	/// Work only in Rot* mode (nlassert).
	void			setScale(const CVector &scale);
	/// Work only in Rot* mode (nlassert).
	void			setScale(float sx, float sy, float sz) {setScale(CVector(sx, sy, sz));}
	/// Work only in Rot* mode (nlassert).
	void			setPivot(const CVector &pivot);
	/// Work only in Rot* mode (nlassert).
	void			setPivot(float px, float py, float pz) {setPivot(CVector(px, py, pz));}

	/// Work only in DirecTMatrix mode (nlassert).
	void			setMatrix(const CMatrix &mat);
	// @}


	/// \name Position get
	// @{

	/// get the current transform mode.
	TTransformMode		getTransformMode();
	/// get the current rotorder (information vlaid only when RotEuler mode).
	CMatrix::TRotOrder	getRotOrder();

	/// Get the matrix, compute her if necessary (work in all modes).
	const CMatrix	&getMatrix() const	;

	/// Work only in Rot* mode(nlassert).
	void			getPos(CVector &pos);
	/// Work only in RotEuler mode(nlassert).
	void			getRotEuler(CVector &rot);
	/// Work only in RotQuat mode (nlassert).
	void			getRotQuat(CQuat &quat);
	/// Work only in Rot* mode (nlassert).
	void			getScale(CVector &scale);
	/// Work only in Rot* mode (nlassert).
	void			getPivot(CVector &pivot);

	/// Work only in Rot* mode(nlassert).
	CVector			getPos();
	/// Work only in RotEuler mode(nlassert).
	CVector			getRotEuler();
	/// Work only in RotQuat mode (nlassert).
	CQuat			getRotQuat();
	/// Work only in Rot* mode (nlassert).
	CVector			getScale();
	/// Work only in Rot* mode (nlassert).
	CVector			getPivot();
	// @}


	/// \name Misc
	// @{
	/**
	  * Setup Matrix by the lookAt method. Work only in DirectMatrix mode and RotQuat mode (not euler...).
	  *
	  * \param eye is the coordinate of the object.
	  * \param target is the point the object look at.
	  * \param roll is the roll angle in radian along the object's Y axis.
	  */
	void			lookAt (const CVector& eye, const CVector& target, float roll=0.f);
	// @}



	/// \name Channel name
	// @{
	static const char *getPosValueName ();
	static const char *getRotEulerValueName();
	static const char *getRotQuatValueName();
	static const char *getScaleValueName();
	static const char *getPivotValueName();
	// @}

	/// Proxy interface

	/// Constructors
	UTransformable() {_Object = NULL;}
	UTransformable(class ITransformable *object) : _Object (object) {}
	/// Attach an object to this proxy
	void			attach(class ITransformable *object) { _Object = object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class ITransformable	*getObjectPtr() const {return _Object;}

protected:
	ITransformable	*_Object;
};


} // NL3D


#endif // NL_U_TRANSFORMABLE_H

/* End of u_transformable.h */
