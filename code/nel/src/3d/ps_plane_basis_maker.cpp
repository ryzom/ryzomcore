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


#include "nel/3d/ps_plane_basis_maker.h"
#include "nel/3d/ps_register_plane_basis_attribs.h"

namespace NL3D
{


CPlaneBasis CPSPlaneBasisGradient::DefaultPlaneBasisTab[] = { CPlaneBasis(NLMISC::CVector::I), CPlaneBasis(NLMISC::CVector::J) };

/////////////////////////////////////////////
// CPSPlaneBasisFollowSpeed implementation //
/////////////////////////////////////////////

///============================================================================
CPlaneBasis CPSPlaneBasisFollowSpeed::get(CPSLocated *loc, uint32 index)
{
   return (CPlaneBasis(loc->getSpeed()[index]));
}
CPlaneBasis CPSPlaneBasisFollowSpeed::get(const CPSEmitterInfo &infos)
{
	return (CPlaneBasis(infos.Speed));
}

///============================================================================
void *CPSPlaneBasisFollowSpeed::make(CPSLocated *loc,
									 uint32 startIndex,
									 void *tab, uint32 stride,
									 uint32 numAttrib,
									 bool enableNoCopy /* = false*/,
									 uint32 srcStep /*= (1 << 16)*/,
									 bool forceClampEntry /* = false */
									) const
{
	nlassert(numAttrib);
	if (srcStep == (1 << 16))
	{
		TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
										, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib;
		uint8 *ptDat  = (uint8 *) tab;
		switch(_ProjectionPlane)
		{
			case NoProjection:
				do
				{
					*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt);
					++ speedIt;
					ptDat += stride;
				}
				while (speedIt != endSpeedIt);
			break;
			case XY:
				do
				{
					float norm = sqrtf(speedIt->x * speedIt->x + speedIt->y * speedIt->y);
					float invNorm = (norm != 0.f) ? 1.f / norm : 0.f;
					CPlaneBasis &pb = *(CPlaneBasis *) ptDat;
					pb.X.set(invNorm * speedIt->x, invNorm * speedIt->y, 0.f);
					pb.Y.set(- pb.X.y, pb.X.x, 0.f);
					++ speedIt;
					ptDat += stride;
				}
				while (speedIt != endSpeedIt);
			break;
			case XZ:
				do
				{
					float norm = sqrtf(speedIt->x * speedIt->x + speedIt->z * speedIt->z);
					float invNorm = (norm != 0.f) ? 1.f / norm : 0.f;
					CPlaneBasis &pb = *(CPlaneBasis *) ptDat;
					pb.X.set(invNorm * speedIt->x, 0.f, invNorm * speedIt->z);
					pb.Y.set(- pb.X.z, 0.f, pb.X.x);
					++ speedIt;
					ptDat += stride;
				}
				while (speedIt != endSpeedIt);
			break;
			case YZ:
				do
				{
					float norm = sqrtf(speedIt->y * speedIt->y + speedIt->z * speedIt->z);
					float invNorm = (norm != 0.f) ? 1.f / norm : 0.f;
					CPlaneBasis &pb = *(CPlaneBasis *) ptDat;
					pb.X.set(0.f, invNorm * speedIt->y, invNorm * speedIt->z);
					pb.Y.set(0.f, - pb.X.z, pb.X.y);
					++ speedIt;
					ptDat += stride;
				}
				while (speedIt != endSpeedIt);
			break;
			default:
				nlstop; // unknow projection mode
			break;
		}
		return tab;
	}
	else
	{
		uint32 fpIndex = startIndex * srcStep;
		const TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin();
		uint8 *ptDat  = (uint8 *) tab;
		switch(_ProjectionPlane)
		{
			case NoProjection:
				while (numAttrib --)
				{
					*(CPlaneBasis *) ptDat = CPlaneBasis(*(speedIt + (fpIndex >> 16)));
					ptDat += stride;
					fpIndex += srcStep;
				}
			break;
			case XY:
				while (numAttrib --)
				{
					const CVector *speedVect = &(*(speedIt + (fpIndex >> 16)));
					float norm = sqrtf(speedVect->x * speedVect->x + speedVect->y * speedVect->y);
					float invNorm = (norm != 0.f) ? 1.f / norm : 0.f;
					CPlaneBasis &pb = *(CPlaneBasis *) ptDat;
					pb.X.set(invNorm * speedVect->x, invNorm * speedVect->y, 0.f);
					pb.Y.set(- pb.X.y, pb.X.x, 0.f);
					ptDat += stride;
					fpIndex += srcStep;
				}
			break;
			case XZ:
				while (numAttrib --)
				{
					const CVector *speedVect = &(*(speedIt + (fpIndex >> 16)));
					float norm = sqrtf(speedVect->x * speedVect->x + speedVect->z * speedVect->z);
					float invNorm = (norm != 0.f) ? 1.f / norm : 0.f;
					CPlaneBasis &pb = *(CPlaneBasis *) ptDat;
					pb.X.set(invNorm * speedVect->x, 0.f, invNorm * speedVect->z);
					pb.Y.set(- pb.X.z, 0.f, pb.X.x);
					ptDat += stride;
					fpIndex += srcStep;
				}
			break;
			case YZ:
				while (numAttrib --)
				{
					const CVector *speedVect = &(*(speedIt + (fpIndex >> 16)));
					float norm = sqrtf(speedVect->y * speedVect->y + speedVect->z * speedVect->z);
					float invNorm = (norm != 0.f) ? 1.f / norm : 0.f;
					CPlaneBasis &pb = *(CPlaneBasis *) ptDat;
					pb.X.set(0.f, invNorm * speedVect->y, invNorm * speedVect->z);
					pb.Y.set(0.f, - pb.X.z, pb.X.y);
					ptDat += stride;
					fpIndex += srcStep;
				}
			break;
			default:
				nlstop; // unknow projection mode
			break;
		}
		return tab;
	}
}

///============================================================================
void CPSPlaneBasisFollowSpeed::make4(CPSLocated *loc,
									 uint32 startIndex,
									 void *tab,
									 uint32 stride,
									 uint32 numAttrib,
									 uint32 srcStep /*= (1 << 16)*/
									) const
{
	nlassert(numAttrib);
	if (srcStep == (1 << 16))
	{
		TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
										, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib;
		uint8 *ptDat  = (uint8 *) tab;
		do
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt);
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride;
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride;
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride << 1;
			++ speedIt;
		}
		while (speedIt != endSpeedIt);
	}
	else
	{
		uint32 fpIndex = startIndex * srcStep;
		const TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin();
		uint8 *ptDat  = (uint8 *) tab;
		while (numAttrib --)
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*(speedIt + (fpIndex >> 16)));
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride;
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride;
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride << 1;
			fpIndex += srcStep;
		}
	}
}

///============================================================================
void CPSPlaneBasisFollowSpeed::makeN(CPSLocated *loc,
									 uint32 startIndex,
									 void *tab,
									 uint32 stride,
									 uint32 numAttrib,
									 uint32 nbReplicate,
									 uint32 srcStep /*= (1 << 16) */
									) const
{
	nlassert(numAttrib);
	if (srcStep == (1 << 16))
	{
		nlassert(nbReplicate > 1);
		TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
										, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib;
		uint8 *ptDat  = (uint8 *) tab;
		uint k;
		do
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt);

			k = nbReplicate - 1;

			do
			{
				*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
				ptDat += stride;
			}
			while (--k);
			ptDat += stride;

			++ speedIt;
		}
		while (speedIt != endSpeedIt);
	}
	else
	{
		uint32 fpIndex = startIndex * srcStep;
		nlassert(nbReplicate > 1);
		const TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin();
		uint8 *ptDat  = (uint8 *) tab;
		uint k;
		while (numAttrib --)
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*(speedIt + (fpIndex >> 16)));

			k = nbReplicate - 1;

			do
			{
				*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
				ptDat += stride;
			}
			while (--k);
			ptDat += stride;

			fpIndex += srcStep;
		}
	}
}


/////////////////////////////////////////////
// CSpinnerFunctor implementation		   //
/////////////////////////////////////////////


///============================================================================
CSpinnerFunctor::CSpinnerFunctor() : _NbSamples(0), _Axis(NLMISC::CVector::K)
{
}

///============================================================================
void CSpinnerFunctor::setAxis(const NLMISC::CVector &axis)
{
	_Axis = axis;
	updateSamples();
}

///============================================================================
void CSpinnerFunctor::setNumSamples(uint32 nbSamples)
{
	nlassert(nbSamples > 0);
	_NbSamples = nbSamples;
	updateSamples();
}

///============================================================================
uint32 CSpinnerFunctor::getNumSamples(void) const
{
	return _NbSamples;
}

///============================================================================
void CSpinnerFunctor::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	f.serial(_Axis, _NbSamples);
	if (f.isReading()) updateSamples();
}

///============================================================================
void CSpinnerFunctor::updateSamples(void)
{
	if (_NbSamples == 0) return;
	// compute step between each angle
	const float angInc = (float) (2.f * NLMISC::Pi / _NbSamples);
	_PBTab.resize(_NbSamples + 1);
	NLMISC::CMatrix mat;
	CPSUtil::buildSchmidtBasis(_Axis, mat);
	NLMISC::CVector I = mat.getI();
	NLMISC::CVector J = mat.getJ();
	// compute basis for rotation
	for (uint32 k = 0; k < _NbSamples; ++k)
	{
		float ca = cosf(k * angInc);
		float sa = sinf(k * angInc);
		_PBTab[k].X.set(ca * I.x + sa * J.x,
						ca * I.y + sa * J.y,
						ca * I.z + sa * J.z);

		_PBTab[k].Y.set(- sa * I.x + ca * J.x,
					    - sa * I.y + ca * J.y,
						- sa * I.z + ca * J.z);
	}
}

///============================================================================
void PSRegisterPlaneBasisAttribs()
{
	NLMISC_REGISTER_CLASS(CPSPlaneBasisBlender);
	NLMISC_REGISTER_CLASS(CPSPlaneBasisGradient);
	NLMISC_REGISTER_CLASS(CPSPlaneBasisMemory);
	NLMISC_REGISTER_CLASS(CPSPlaneBasisBinOp);
	NLMISC_REGISTER_CLASS(CPSPlaneBasisFollowSpeed);
	NLMISC_REGISTER_CLASS(CPSBasisSpinner);
}

} // NL3D
