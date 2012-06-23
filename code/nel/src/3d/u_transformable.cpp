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

#include "std3d.h"

#include "nel/3d/u_transformable.h"
#include "nel/3d/transformable.h"


namespace NL3D
{

// ***************************************************************************

const char *UTransformable::getPosValueName ()
{
	return ITransformable::getPosValueName ();
}

// ***************************************************************************

const char *UTransformable::getRotEulerValueName()
{
	return ITransformable::getRotEulerValueName();
}

// ***************************************************************************

const char *UTransformable::getRotQuatValueName()
{
	return ITransformable::getRotQuatValueName();
}

// ***************************************************************************

const char *UTransformable::getScaleValueName()
{
	return ITransformable::getScaleValueName();
}

// ***************************************************************************

const char *UTransformable::getPivotValueName()
{
	return ITransformable::getPivotValueName();
}

// ***************************************************************************

void UTransformable::setMatrix(const CMatrix &mat)
{
	_Object->setMatrix(mat);
}

// ***************************************************************************

const CMatrix &UTransformable::getMatrix() const
{
	return _Object->getMatrix();
}

// ***************************************************************************

void UTransformable::setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro)
{
	_Object->setTransformMode((ITransformable::TTransformMode)(uint)mode, ro);
}

// ***************************************************************************

void UTransformable::setPos(const CVector &pos)
{
	_Object->setPos(pos);
}

// ***************************************************************************

void UTransformable::setRotEuler(const CVector &rot)
{
	_Object->setRotEuler(rot);
}

// ***************************************************************************

void UTransformable::setRotQuat(const CQuat &quat)
{
	_Object->setRotQuat(quat);
}

// ***************************************************************************

void UTransformable::setRotQuat(const CVector &jdir)
{
	CMatrix	mat;
	mat.setRot(CVector::I, jdir, CVector::K);
	mat.normalize(CMatrix::YZX);
	setRotQuat(mat.getRot());
}

// ***************************************************************************

void UTransformable::setRotQuat(const CVector &jdir, const CVector &vup)
{
	CMatrix	mat;
	mat.setRot(CVector::I, jdir, vup);
	mat.normalize(CMatrix::YZX);
	setRotQuat(mat.getRot());
}

// ***************************************************************************

void UTransformable::setScale(const CVector &scale)
{
	_Object->setScale(scale);
}

// ***************************************************************************

void UTransformable::setPivot(const CVector &pivot)
{
	_Object->setPivot(pivot);
}

// ***************************************************************************

UTransformable::TTransformMode UTransformable::getTransformMode()
{
	return (TTransformMode)(uint)_Object->getTransformMode();
}

// ***************************************************************************

CMatrix::TRotOrder UTransformable::getRotOrder()
{
	return _Object->getRotOrder();
}

// ***************************************************************************

void UTransformable::getPos(CVector &pos)
{
	_Object->getPos(pos);
}

// ***************************************************************************

void UTransformable::getRotEuler(CVector &rot)
{
	_Object->getRotEuler(rot);
}

// ***************************************************************************

void UTransformable::getRotQuat(CQuat &quat)
{
	_Object->getRotQuat(quat);
}

// ***************************************************************************

void UTransformable::getScale(CVector &scale)
{
	_Object->getScale(scale);
}

// ***************************************************************************

void UTransformable::getPivot(CVector &pivot)
{
	_Object->getPivot(pivot);
}


// ***************************************************************************

CVector	UTransformable::getPos()
{
	return _Object->getPos();
}

// ***************************************************************************

CVector	UTransformable::getRotEuler()
{
	return _Object->getRotEuler();
}

// ***************************************************************************

CQuat UTransformable::getRotQuat()
{
	return _Object->getRotQuat();
}

// ***************************************************************************

CVector	UTransformable::getScale()
{
	return _Object->getScale();
}

// ***************************************************************************

CVector	UTransformable::getPivot()
{
	return _Object->getPivot();
}

// ***************************************************************************

void UTransformable::lookAt (const CVector& eye, const CVector& target, float roll)
{
	_Object->lookAt(eye, target, roll);
}

// ***************************************************************************


} // NL3D
