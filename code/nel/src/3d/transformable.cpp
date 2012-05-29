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

#include "nel/3d/transformable.h"
#include "nel/3d/channel_mixer.h"


namespace NL3D
{


// ***************************************************************************
ITransformable::ITransformable()
{
	// Set number of animated values.
	IAnimatable::resize (AnimValueLast);

	// Deriver note: just copy this line in each ctor.

	// Init default values.
	_Mode= RotQuat;
	// matrix init to identity.
	_Pos.Value= CVector::Null;
	_RotEuler.Value= CVector::Null;
	_RotQuat.Value= CQuat::Identity;
	_Scale.Value= CVector(1,1,1);
	_Pivot.Value= CVector::Null;

	_LocalMatrixDate= 0;
}


// ***************************************************************************
IAnimatedValue*		ITransformable::getValue (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case PosValue:			return &_Pos;
	case RotEulerValue:		return &_RotEuler;
	case RotQuatValue:		return &_RotQuat;
	case ScaleValue:		return &_Scale;
	case PivotValue:		return &_Pivot;
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getValue(valueId);

	return NULL;
}
// ***************************************************************************
const char 	*ITransformable::getValueName (uint valueId) const
{
	// what value ?
	switch (valueId)
	{
	case PosValue:			return getPosValueName ();
	case RotEulerValue:		return getRotEulerValueName();
	case RotQuatValue:		return getRotQuatValueName();
	case ScaleValue:		return getScaleValueName();
	case PivotValue:		return getPivotValueName();
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getValueName(valueId);

	return "";
}

// ***************************************************************************
const char	*ITransformable::getPosValueName ()
{
	return "pos";
}
// ***************************************************************************
const char	*ITransformable::getRotEulerValueName()
{
	return "roteuler";
}
// ***************************************************************************
const char	*ITransformable::getRotQuatValueName()
{
	return "rotquat";
}
// ***************************************************************************
const char	*ITransformable::getScaleValueName()
{
	return "scale";
}
// ***************************************************************************
const char	*ITransformable::getPivotValueName()
{
	return "pivot";
}


// ***************************************************************************
void	ITransformable::clearTransformFlags() const
{
	ITransformable	*self= const_cast<ITransformable*>(this);

	// clear my falgs.
	self->clearFlag(PosValue);
	self->clearFlag(RotEulerValue);
	self->clearFlag(RotQuatValue);
	self->clearFlag(ScaleValue);
	self->clearFlag(PivotValue);

	// We are OK!
	self->clearFlag(OwnerBit);
}


// ***************************************************************************
void	ITransformable::updateMatrix() const
{
	// should we update?
	if(needCompute())
	{
		clearTransformFlags();
		// update scale date (so sons are informed of change).
		_LocalMatrixDate++;

		// update the matrix.
		_LocalMatrix.identity();

		// father scale will be herited.
		// T*P
		_LocalMatrix.translate(_Pos.Value+_Pivot.Value);

		// R*S*P-1.
		if(_Mode==RotEuler)
			_LocalMatrix.rotate(_RotEuler.Value, _RotOrder);
		else
			_LocalMatrix.rotate(_RotQuat.Value);
		_LocalMatrix.scale(_Scale.Value);
		_LocalMatrix.translate(-_Pivot.Value);
	}
}


// ***************************************************************************
void		ITransformable::lookAt (const CVector& eye, const CVector& target, float roll)
{
	nlassert(_Mode==RotQuat || _Mode==DirectMatrix);

	// Roll matrix
	CMatrix rollMT;
	rollMT.identity();
	if (roll!=0.f)
		rollMT.rotateY (roll);

	// Make the target base
	CVector j=target;
	j-=eye;
	j.normalize();
	CVector i=j^CVector (0,0,1.f);
	CVector k=i^j;
	k.normalize();
	i=j^k;
	i.normalize();

	// Make the target matrix
	CMatrix targetMT;
	targetMT.identity();
	targetMT.setRot (i, j, k);
	targetMT.setPos (eye);

	// Compose matrix
	targetMT*=rollMT;

	// Set the matrix
	if(_Mode==DirectMatrix)
		setMatrix (targetMT);
	else
	{
		// transfrom to quaternion mode.
		setScale(CVector(1,1,1));
		setPivot(CVector::Null);
		setPos(targetMT.getPos());
		setRotQuat(targetMT.getRot());
	}
}



} // NL3D
