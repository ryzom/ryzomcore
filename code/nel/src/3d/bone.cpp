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

#include "nel/3d/bone.h"
#include "nel/3d/anim_ctrl.h"
#include "nel/misc/hierarchical_timer.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// CBoneBase
// ***************************************************************************
// ***************************************************************************


static	const CVector	UnitScale(1,1,1);


// ***************************************************************************
CBoneBase::CBoneBase() : DefaultPos(CVector(0,0,0)), DefaultRotEuler(CVector(0,0,0)),
	DefaultScale(UnitScale), DefaultPivot(CVector(0,0,0)), SkinScale(UnitScale)
{
	FatherId= -1;
	UnheritScale= true;
	// Default: never disable.
	LodDisableDistance= 0.f;
}


// ***************************************************************************
void			CBoneBase::serial(NLMISC::IStream &f)
{
	/*
	Version 2:
		- SkinScale
	Version 1:
		- LodDisableDistance
	*/
	sint	ver= f.serialVersion(2);

	f.serial(Name);
	f.serial(InvBindPos);
	f.serial(FatherId);
	f.serial(UnheritScale);

	if(ver>=1)
		f.serial(LodDisableDistance);
	else
	{
		// Default: never disable.
		LodDisableDistance= 0.f;
	}

	f.serial(DefaultPos);
	f.serial(DefaultRotEuler);
	f.serial(DefaultRotQuat);
	f.serial(DefaultScale);
	f.serial(DefaultPivot);

	if(ver>=2)
		f.serial(SkinScale);
}


// ***************************************************************************
// ***************************************************************************
// CBone
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CBone::CBone(CBoneBase *boneBase)
{
	nlassert(boneBase);
	_BoneBase= boneBase;

	// IAnimatable.
	IAnimatable::resize(AnimValueLast);

	ITransformable::setTransformMode(ITransformable::RotQuat);
	ITransformable::setPos( _BoneBase->DefaultPos.getDefaultValue()  );
	ITransformable::setRotQuat( _BoneBase->DefaultRotQuat.getDefaultValue() );
	ITransformable::setScale( _BoneBase->DefaultScale.getDefaultValue()  );
	ITransformable::setPivot( _BoneBase->DefaultPivot.getDefaultValue()  );

	// By default, the bone is not binded to a channelMixer.
	_PosChannelId= -1;
	_RotEulerChannelId= -1;
	_RotQuatChannelId= -1;
	_ScaleChannelId= -1;
	_PivotChannelId= -1;

	// No animCtrl by default
	_AnimCtrl= NULL;

	// Get default BoneBase SkinScale
	_SkinScale= _BoneBase->SkinScale;
}

// ***************************************************************************
ITrack* CBone::getDefaultTrack (uint valueId)
{
	nlassert(_BoneBase);

	// what value ?
	switch (valueId)
	{
	case PosValue:			return &_BoneBase->DefaultPos;
	case RotEulerValue:		return &_BoneBase->DefaultRotEuler;
	case RotQuatValue:		return &_BoneBase->DefaultRotQuat;
	case ScaleValue:		return &_BoneBase->DefaultScale;
	case PivotValue:		return &_BoneBase->DefaultPivot;
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getDefaultTrack(valueId);

	return NULL;
}

// ***************************************************************************
void	CBone::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	// For CBone, channels are detailled.
	// Bkup each channelId (for disable).
	_PosChannelId= addValue(chanMixer, PosValue, OwnerBit, prefix, true);
	_RotEulerChannelId= addValue(chanMixer, RotEulerValue, OwnerBit, prefix, true);
	_RotQuatChannelId= addValue(chanMixer, RotQuatValue, OwnerBit, prefix, true);
	_ScaleChannelId= addValue(chanMixer, ScaleValue, OwnerBit, prefix, true);
	_PivotChannelId= addValue(chanMixer, PivotValue, OwnerBit, prefix, true);

	// Deriver note: if necessary, call	BaseClass::registerToChannelMixer(chanMixer, prefix);
}

// ***************************************************************************
void	CBone::compute(CBone *parent, const CMatrix &rootMatrix, CSkeletonModel *skeletonForAnimCtrl)
{
	// compute is called typically 800 time per frame.
#ifdef NL_DEBUG
	nlassert(_BoneBase);
#endif

	// get/compute our local matrix
	const CMatrix	&localMatrix= getMatrix();

	// Compute LocalSkeletonMatrix.
	// Root case?
	if(!parent)
	{
		_LocalSkeletonMatrix= localMatrix;
	}
	// Else, son case, take world matrix from parent.
	else
	{
		// UnheritScale case.
		if(_BoneBase->UnheritScale)
		{
			CMatrix		invScaleComp;
			CVector		fatherScale;
			CVector		trans;

			/* Optim note:
				the scale is rarely ==1, so don't optimize case where it may be.
			*/

			// retrieve our translation
			localMatrix.getPos(trans);
			// retrieve scale from our father.
			parent->getScale(fatherScale);
			// inverse this scale.
			fatherScale.x= 1.0f / fatherScale.x;
			fatherScale.y= 1.0f / fatherScale.y;
			fatherScale.z= 1.0f / fatherScale.z;

			// Compute InverseScale compensation:
			// with UnheritScale, formula per bone should be  T*Sf-1*P*R*S*P-1.
			// But getMatrix() return T*P*R*S*P-1.
			// So we must compute T*Sf-1*T-1, in order to get wanted result.
			invScaleComp.setScale(fatherScale);
			// Faster compute of the translation part: just "trans + fatherScale MUL -trans" where MUL is comp mul
			trans.x-= fatherScale.x * trans.x;
			trans.y-= fatherScale.y * trans.y;
			trans.z-= fatherScale.z * trans.z;
			invScaleComp.setPos(trans);


			// And finally, we got ParentWM * T*Sf-1*P*R*S*P-1.
			// Do: _LocalSkeletonMatrix= parent->_LocalSkeletonMatrix * invScaleComp * localMatrix
			static	CMatrix	tmp;
			tmp.setMulMatrixNoProj( parent->_LocalSkeletonMatrix, invScaleComp );
			_LocalSkeletonMatrix.setMulMatrixNoProj( tmp, localMatrix );
		}
		// Normal case.
		else
		{
			// Do: _LocalSkeletonMatrix= parent->_LocalSkeletonMatrix * localMatrix
			_LocalSkeletonMatrix.setMulMatrixNoProj( parent->_LocalSkeletonMatrix, localMatrix );
		}
	}

	// Compute WorldMatrix. Do: _WorldMatrix= rootMatrix * _LocalSkeletonMatrix
	_WorldMatrix.setMulMatrixNoProj( rootMatrix, _LocalSkeletonMatrix );

	// Compute BoneSkinMatrix. Easier of no SkinScale
	if(_SkinScale==UnitScale)
	{
		// Do: _BoneSkinMatrix= _LocalSkeletonMatrix * _BoneBase->InvBindPos
		_BoneSkinMatrix.setMulMatrixNoProj( _LocalSkeletonMatrix, _BoneBase->InvBindPos );
	}
	else
	{
		// Do: _BoneSkinMatrix= _LocalSkeletonMatrix * SkinScale * _BoneBase->InvBindPos
		CMatrix		tmp;
		CMatrix		scaleMat;
		scaleMat.setScale(_SkinScale);
		tmp.setMulMatrixNoProj(_LocalSkeletonMatrix, scaleMat);
		_BoneSkinMatrix.setMulMatrixNoProj( tmp, _BoneBase->InvBindPos );
	}

	// When compute is done, do extra user ctrl?
	if(_AnimCtrl && skeletonForAnimCtrl)
		_AnimCtrl->execute(skeletonForAnimCtrl, this);
}


// ***************************************************************************
void	CBone::interpolateBoneSkinMatrix(const CMatrix &otherMatrix, float interp)
{
	CMatrix		&curMatrix= _BoneSkinMatrix;

	// interpolate rot/scale. Just interpolate basis vectors
	CVector		fatherI= otherMatrix.getI();
	CVector		curI= curMatrix.getI();
	curI= fatherI*(1-interp) + curI*interp;
	CVector		fatherJ= otherMatrix.getJ();
	CVector		curJ= curMatrix.getJ();
	curJ= fatherJ*(1-interp) + curJ*interp;
	CVector		fatherK= otherMatrix.getK();
	CVector		curK= curMatrix.getK();
	curK= fatherK*(1-interp) + curK*interp;
	// replace rotation
	curMatrix.setRot(curI, curJ, curK);

	// interpolate pos
	CVector		fatherPos= otherMatrix.getPos();
	CVector		curPos= curMatrix.getPos();
	curPos= fatherPos*(1-interp) + curPos*interp;
	curMatrix.setPos(curPos);
}


// ***************************************************************************
void	CBone::lodEnableChannels(CChannelMixer *chanMixer, bool enable)
{
	nlassert(chanMixer);

	// Lod Enable channels if they are correclty registered to the channelMixer.
	if( _PosChannelId>=0 )
		chanMixer->lodEnableChannel(_PosChannelId, enable);
	if( _RotEulerChannelId>=0 )
		chanMixer->lodEnableChannel(_RotEulerChannelId, enable);
	if( _RotQuatChannelId>=0 )
		chanMixer->lodEnableChannel(_RotQuatChannelId, enable);
	if( _ScaleChannelId>=0 )
		chanMixer->lodEnableChannel(_ScaleChannelId, enable);
	if( _PivotChannelId>=0 )
		chanMixer->lodEnableChannel(_PivotChannelId, enable);

}

// ***************************************************************************
void	CBone::setSkinScale(CVector &skinScale)
{
	_SkinScale= skinScale;
}


} // NL3D
