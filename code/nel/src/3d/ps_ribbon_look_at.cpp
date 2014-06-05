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

#include "nel/3d/ps_ribbon_look_at.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_macro.h"
#include "nel/3d/driver.h"

namespace NL3D
{

////////////////////////////////////
// CPSRibbonLookAt implementation //
////////////////////////////////////

const float ZEpsilon = 10E-3f;
const float NormEpsilon = 10E-8f;


struct CVectInfo
{
	NLMISC::CVector Interp;
	NLMISC::CVector Proj;
};
typedef std::vector<CVectInfo> TRibbonVect; // a vector used for intermediate computations

CPSRibbonLookAt::TVBMap		CPSRibbonLookAt::_VBMap;			// index buffers with no color
CPSRibbonLookAt::TVBMap		CPSRibbonLookAt::_ColoredVBMap;  // index buffer + colors

//=======================================================
CPSRibbonLookAt::CPSRibbonLookAt()
{
	NL_PS_FUNC(CPSRibbonLookAt_CPSRibbonLookAt)
}

//=======================================================
CPSRibbonLookAt::~CPSRibbonLookAt()
{
	NL_PS_FUNC(CPSRibbonLookAt_CPSRibbonLookAtDtor)
//	delete _DyingRibbons;
}

//=======================================================
void CPSRibbonLookAt::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSRibbonLookAt_serial)
	/** Version 4 : added CPSRibbonBase has a base class instead of CPSParticle
	  *
	  */
	sint ver = f.serialVersion(4);
	if (ver > 3)
	{
		CPSRibbonBase::serial(f);
	}
	else
	{
		CPSParticle::serial(f);
	}
	CPSColoredParticle::serialColorScheme(f);
	CPSSizedParticle::serialSizeScheme(f);
	serialMaterial(f);
	uint32 dummy = 0; /* _NbDyingRibbons */
	if (ver <= 3)
	{
		f.serial(_SegDuration, _NbSegs, dummy /*_NbDyingRibbons*/);
	}
	ITexture *tex = NULL;

	if (ver > 2)
	{
		f.serial(_Parametric);
	}


	if (!f.isReading())
	{
		tex = _Tex;
		f.serialPolyPtr(tex);
	}
	else
	{
		f.serialPolyPtr(tex);
		setTexture(tex);
		_Tex = tex;
		if (_Tex)
		{
			_Tex->setWrapS(ITexture::Clamp);
			_Tex->setWrapT(ITexture::Clamp);
		}
		setTailNbSeg(_NbSegs); // force to build the vb
	}
}


//=======================================================
void CPSRibbonLookAt::setTexture(CSmartPtr<ITexture> tex)
{
	NL_PS_FUNC(CPSRibbonLookAt_setTexture)
	_Tex = tex;
	if (_Tex)
	{
		_Tex->setWrapS(ITexture::Clamp);
		_Tex->setWrapT(ITexture::Clamp);
	}
	updateMatAndVbForColor();
}


//=======================================================
void CPSRibbonLookAt::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSRibbonLookAt_step)
	if (pass == PSMotion)
	{
		if (!_Parametric)
		{
			updateGlobals();
		}
	}
	else
	if (
		(pass == PSBlendRender && hasTransparentFaces())
		|| (pass == PSSolidRender && hasOpaqueFaces())
		)
	{
		uint32 step;
		uint   numToProcess;
		computeSrcStep(step, numToProcess);
		if (!numToProcess) return;

		/// update the material color
		CParticleSystem &ps = *(_Owner->getOwner());
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{
			_Mat.setColor(ps.getGlobalColorLighted());
		}
		else
		{
			_Mat.setColor(ps.getGlobalColor());
		}
		/** We support Auto-LOD for ribbons, although there is a built-in LOD (that change the geometry rather than the number of ribbons)
		  * that gives better result (both can be used simultaneously)
		  */
		displayRibbons(numToProcess, step);
	}
	else
	if (pass == PSToolRender) // edition mode only
	{
		//showTool();
	}
}


//=======================================================
void CPSRibbonLookAt::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSRibbonLookAt_newElement)
	CPSRibbonBase::newElement(info);
	newColorElement(info);
	newSizeElement(info);
}


//=======================================================
void CPSRibbonLookAt::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSRibbonLookAt_deleteElement)
	CPSRibbonBase::deleteElement(index);
	deleteColorElement(index);
	deleteSizeElement(index);
}


//=======================================================
void CPSRibbonLookAt::resize(uint32 size)
{
	NL_PS_FUNC(CPSRibbonLookAt_resize)
	nlassert(size < (1 << 16));
	CPSRibbonBase::resize(size);
	resizeColor(size);
	resizeSize(size);
}

//=======================================================
void CPSRibbonLookAt::updateMatAndVbForColor(void)
{
	NL_PS_FUNC(CPSRibbonLookAt_updateMatAndVbForColor)
	_Mat.setTexture(0, _Tex);
	_Mat.setDoubleSided(true);
}

//=======================================================
static inline void MakeProj(NLMISC::CVector &dest, const NLMISC::CVector &src)
{
	NL_PS_FUNC(MakeProj)
	if (fabsf(src.y) > NormEpsilon * NormEpsilon)
	{
		dest.x = src.x / src.y;
		dest.z = src.z / src.y;
		dest.y = src.y;
	}
}

static inline void BuildSlice(const NLMISC::CMatrix &mat, CVertexBuffer &vb, uint8 *currVert, uint32 vertexSize,
							  const NLMISC::CVector &I,
							  const NLMISC::CVector &K,
  							  TRibbonVect::iterator  pos,
							  TRibbonVect::iterator  prev,
							  TRibbonVect::iterator  next,
							  float ribSize)
/// TODO: some optimisation to get a better speed
{
	NL_PS_FUNC(BuildSlice)
	CHECK_VERTEX_BUFFER(vb, currVert);
	CHECK_VERTEX_BUFFER(vb, currVert);
	NLMISC::CVector tangent;

	float invTgNorm; // inverse of the' norm of the projected segment
	float tgNorm;

	if (prev->Proj.y > ZEpsilon && next->Proj.y > ZEpsilon) // the 2 points are in front of the camera
	{
		tangent = next->Proj - prev->Proj;
		tangent.y = 0;
		tgNorm = tangent.norm();
		if (fabs(tgNorm) > 10E-8)
		{
			invTgNorm = 1.f / tgNorm;
		}
		else
		{
			invTgNorm = 1.f;
		}
		// build orthogonals vectors to tangent
		*(NLMISC::CVector *) currVert = pos->Interp + ribSize * invTgNorm * (tangent.x * K - tangent.z * I);
		*(NLMISC::CVector *) (currVert + vertexSize) = pos->Interp + ribSize * invTgNorm * (- tangent.x * K + tangent.z * I);
	}
	else if (prev->Proj.y > ZEpsilon) // second point cross the near plane
	{
		// compute intersection point
		NLMISC::CVector inter;
		NLMISC::CVector tInter = CVector::Null;
		if (fabsf(prev->Proj.y - next->Proj.y) > NormEpsilon)
		{
				float lambda = (next->Proj.y - ZEpsilon) / (next->Proj.y - prev->Proj.y);
				inter = lambda * prev->Interp + (1.f - lambda) * next->Interp;
				MakeProj(tInter, mat * inter);
		}
		else //
		{
			*(NLMISC::CVector *) currVert = pos->Interp;
			*(NLMISC::CVector *) (currVert + vertexSize) = pos->Interp;
			return;
		}

		tangent = tInter - prev->Proj;
		tangent.y = 0;

		tgNorm = tangent.norm();
		if (fabs(tgNorm) > 10E-8)
		{
			invTgNorm = 1.f / tgNorm;
		}
		else
		{
			invTgNorm = 1.f;
		}
		// build orthogonals vectors to tangent

		*(NLMISC::CVector *) currVert = inter + ribSize *  invTgNorm * (tangent.x * K - tangent.z * I);
		*(NLMISC::CVector *) (currVert + vertexSize) = inter + ribSize * invTgNorm * (- tangent.x * K + tangent.z * I);
	}
	else if (next->Proj.y > ZEpsilon) // first point cross the near plane
	{
		// compute intersection point
		NLMISC::CVector inter;
		NLMISC::CVector tInter = NLMISC::CVector::Null;
		if (fabsf(prev->Proj.y - next->Proj.y) > NormEpsilon)
		{
				float lambda = (next->Proj.y - ZEpsilon) / (next->Proj.y - prev->Proj.y);
				inter = lambda * prev->Interp + (1.f - lambda) * next->Interp;
				MakeProj(tInter, mat * inter);
		}
		else //
		{
			*(NLMISC::CVector *) currVert = pos->Interp;
			*(NLMISC::CVector *) (currVert + vertexSize) = pos->Interp;
			return;
		}

		tangent = next->Proj - tInter;
		tangent.y = 0;
		tgNorm = tangent.norm();
		if (fabs(tgNorm) > 10E-8)
		{
			invTgNorm = 1.f / tgNorm;
		}
		else
		{
			invTgNorm = 1.f;
		}
		// build orthogonals vectors to tangent

		*(NLMISC::CVector *) currVert = inter + ribSize * invTgNorm * (tangent.x * K - tangent.z * I);
		*(NLMISC::CVector *) (currVert + vertexSize) = inter + ribSize * invTgNorm * (- tangent.x * K + tangent.z * I);

	}
	else // two points are not visible
	{
		*(NLMISC::CVector *) currVert = pos->Interp;
		*(NLMISC::CVector *) (currVert + vertexSize) = pos->Interp;
	}

}


//==========================================================================
void CPSRibbonLookAt::displayRibbons(uint32 nbRibbons, uint32 srcStep)
{
//	if (!FilterPS[6]) return;
	NL_PS_FUNC(CPSRibbonLookAt_displayRibbons)
	if (!nbRibbons) return;
	nlassert(_Owner);
	CPSRibbonBase::updateLOD();
	if (_UsedNbSegs < 2) return;
	const float date = _Owner->getOwner()->getSystemDate();
	uint8						*currVert;
	CVBnPB						&VBnPB = getVBnPB(); // get the appropriate vb (build it if needed)
	CVertexBuffer				&VB = VBnPB.VB;
	CIndexBuffer				&PB = VBnPB.PB;
	const uint32				vertexSize  = VB.getVertexSize();
	uint						colorOffset=0;
	const uint32				vertexSizeX2  = vertexSize << 1;
	const NLMISC::CVector       I = _Owner->computeI();
	const NLMISC::CVector       K = _Owner->computeK();
	const NLMISC::CMatrix &localToWorldMatrix = getLocalToWorldTrailMatrix();
	const NLMISC::CMatrix &mat =  getViewMat() * localToWorldMatrix;
	IDriver *drv = this->getDriver();
	#ifdef NL_DEBUG
		nlassert(drv);
	#endif
	drv->setupModelMatrix(localToWorldMatrix);
	_Owner->incrementNbDrawnParticles(nbRibbons); // for benchmark purpose
	const uint numRibbonBatch = getNumRibbonsInVB(); // number of ribbons to process at once
	static TRibbonVect				   currRibbon;
	static std::vector<float>		   sizes;
	static std::vector<NLMISC::CRGBA>  colors;

	if (_UsedNbSegs == 0) return;

	currRibbon.resize(_UsedNbSegs + 1);
	sizes.resize(numRibbonBatch);


	/// update material color
	CParticleSystem &ps = *(_Owner->getOwner());
	if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
	{
		CPSMaterial::forceModulateConstantColor(true, ps.getGlobalColorLighted());
	}
	else
	if (ps.getColorAttenuationScheme() != NULL || ps.isUserColorUsed())
	{
		CPSMaterial::forceModulateConstantColor(true, ps.getGlobalColor());
	}
	else
	{
		forceModulateConstantColor(false);
		_Mat.setColor(ps.getGlobalColor());
	}

	if (_ColorScheme)
	{
		colorOffset = VB.getColorOff();
		colors.resize(numRibbonBatch);
	}



	uint toProcess;
	uint ribbonIndex = 0; // index of the first ribbon in the batch being processed
	uint32 fpRibbonIndex = 0;
	if (_ColorScheme)
	{
		_ColorScheme->setColorType(drv->getVertexColorFormat());
	}
	do
	{
		toProcess = std::min((uint) (nbRibbons - ribbonIndex) /* = left to do */, numRibbonBatch);
		/// setup sizes
		const float	*ptCurrSize;
		uint32  ptCurrSizeIncrement;
		if (_SizeScheme)
		{
			ptCurrSize = (float *) _SizeScheme->make(this->_Owner, ribbonIndex, &sizes[0], sizeof(float), toProcess, true, srcStep);
			ptCurrSizeIncrement = 1;
		}
		else
		{
			ptCurrSize = &_ParticleSize;
			ptCurrSizeIncrement = 0;
		}


		/// setup colors
		NLMISC::CRGBA	*ptCurrColor=0;
		if (_ColorScheme)
		{
			colors.resize(nbRibbons);
			ptCurrColor = (NLMISC::CRGBA *) _ColorScheme->make(this->_Owner, ribbonIndex, &colors[0], sizeof(NLMISC::CRGBA), toProcess, true, srcStep);
		}
		VB.setNumVertices(2 * (_UsedNbSegs + 1) * toProcess);
		{
			CVertexBufferReadWrite vba;
			VB.lock (vba);
			currVert = (uint8 *) vba.getVertexCoordPointer();
			for (uint k = ribbonIndex; k < ribbonIndex + toProcess; ++k)
			{

				TRibbonVect::iterator rIt = currRibbon.begin(), rItEnd = currRibbon.end(), rItEndMinusOne = rItEnd - 1;

				////////////////////////////////////
				// interpolate and project points //
				////////////////////////////////////

					if (!_Parametric)
					{

						//////////////////////
						// INCREMENTAL CASE //
						//////////////////////

						// the parent class has a method to get the ribbons positions
						computeRibbon((uint) (fpRibbonIndex >> 16), &rIt->Interp, sizeof(CVectInfo));
						do
						{
							MakeProj(rIt->Proj, mat * rIt->Interp);
							++rIt;
						}
						while (rIt != rItEnd);
					}
					else
					{
						//////////////////////
						// PARAMETRIC  CASE //
						//////////////////////
						// we compute each pos thanks to the parametric curve
						_Owner->integrateSingle(date - _UsedSegDuration * (_UsedNbSegs + 1), _UsedSegDuration, _UsedNbSegs + 1, (uint) (fpRibbonIndex >> 16),
												 &rIt->Interp, sizeof(CVectInfo) );
						// project each position now
						do
						{
							MakeProj(rIt->Proj, mat * rIt->Interp);
							++rIt;
						}
						while (rIt != rItEnd);
					}

					rIt = currRibbon.begin();


					// setup colors
					if (_ColorScheme)
					{
						uint8 *currColVertex = currVert + colorOffset;
						uint colCount = (_UsedNbSegs + 1) << 1;
						do
						{
							* (CRGBA *) currColVertex = *ptCurrColor;
							currColVertex += vertexSize;
						}
						while (--colCount);

						++ptCurrColor;
					}

					/// build the ribbon in vb
					// deals with first point
					BuildSlice(mat, VB, currVert, vertexSize, I, K, rIt, rIt, rIt + 1, *ptCurrSize);
					currVert += vertexSizeX2;
					++rIt;


					// deals with other points
					for (;;) // we assume at least 2 segments, so we must have a middle point
					{
						// build 2 vertices with the right tangent. /* to project 2 */ is old projected point
						BuildSlice(mat, VB, currVert, vertexSize, I, K, rIt, rIt - 1, rIt + 1, *ptCurrSize);
						// next position
						++rIt;
						if (rIt == rItEndMinusOne) break;
						// next vertex
						currVert += vertexSizeX2;
					}
					currVert += vertexSizeX2;
					// last point.
					BuildSlice(mat, VB, currVert, vertexSize, I, K, rIt , rIt - 1, rIt, *ptCurrSize);
					ptCurrSize += ptCurrSizeIncrement;
					currVert += vertexSizeX2;

					fpRibbonIndex += srcStep;

			}
		}
		PB.setNumIndexes((_UsedNbSegs << 1) * toProcess * 3);
		drv->activeVertexBuffer(VB);
		// display the result
		drv->activeIndexBuffer (PB);
		drv->renderTriangles (_Mat, 0, PB.getNumIndexes()/3);
		ribbonIndex += toProcess;
	}
	while (ribbonIndex != nbRibbons);
}

//==========================================================================
bool CPSRibbonLookAt::hasTransparentFaces(void)
{
	NL_PS_FUNC(CPSRibbonLookAt_hasTransparentFaces)
	return getBlendingMode() != CPSMaterial::alphaTest ;
}


//==========================================================================
bool CPSRibbonLookAt::hasOpaqueFaces(void)
{
	NL_PS_FUNC(CPSRibbonLookAt_hasOpaqueFaces)
	return !hasTransparentFaces();
}

//==========================================================================
uint32 CPSRibbonLookAt::getNumWantedTris() const
{
	NL_PS_FUNC(CPSRibbonLookAt_getNumWantedTris)
	nlassert(_Owner);
	//return _Owner->getMaxSize() * _NbSegs * 2;
	return _Owner->getSize() * _NbSegs * 2;
}



//==========================================================================
CPSRibbonLookAt::CVBnPB &CPSRibbonLookAt::getVBnPB()
{
	NL_PS_FUNC(CPSRibbonLookAt_getVBnPB)
	TVBMap &map = _ColorScheme ? _VBMap : _ColoredVBMap;
	TVBMap::iterator it = map.find(_UsedNbSegs + 1);
	if (it != map.end())
	{
		return it->second;
	}
	else	// must create this vb
	{
		const uint numRibbonInVB = getNumRibbonsInVB();
		CVBnPB &VBnPB = map[_UsedNbSegs + 1]; // make an entry

		/// set the vb format & size
		CVertexBuffer &vb = VBnPB.VB;
		vb.setVertexFormat(CVertexBuffer::PositionFlag |
						   CVertexBuffer::TexCoord0Flag |
						   (_ColorScheme ? CVertexBuffer::PrimaryColorFlag : 0));
		vb.setNumVertices(2 * (_UsedNbSegs + 1) * numRibbonInVB );
		vb.setPreferredMemory(CVertexBuffer::AGPVolatile, true);
		CVertexBufferReadWrite vba;
		vb.lock (vba);

		// set the primitive block size
		CIndexBuffer &pb = VBnPB.PB;
		pb.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
		pb.setNumIndexes((_UsedNbSegs << 1) * numRibbonInVB * 3);
		pb.setPreferredMemory(CIndexBuffer::AGPVolatile, false);
		CIndexBufferReadWrite iba;
		pb.lock (iba);
		/// Setup the pb and vb parts. Not very fast but executed only once
		uint vbIndex = 0;
		uint pbIndex = 0;
		for (uint i = 0; i < numRibbonInVB; ++i)
		{
			for (uint k = 0; k < (_UsedNbSegs + 1); ++k)
			{
				vba.setTexCoord(vbIndex, 0, CUV((1.f - k / (float) _UsedNbSegs), 0)); /// top vertex
				vba.setTexCoord(vbIndex + 1, 0, CUV((1.f - k / (float) _UsedNbSegs), 1)); /// bottom vertex
				if (k != _UsedNbSegs)
				{
					/// add 2 tri in the primitive block
					iba.setTri(pbIndex, vbIndex + 1, vbIndex + 2, vbIndex);
					iba.setTri(pbIndex+3, vbIndex + 1, vbIndex + 3, vbIndex + 2);
					pbIndex+=6;
				}
				vbIndex += 2;
			}
		}
		return VBnPB;
	}
}

//==========================================================================
uint	CPSRibbonLookAt::getNumRibbonsInVB() const
{
	NL_PS_FUNC(CPSRibbonLookAt_getNumRibbonsInVB)
	/// approximation of the max number of vertices we want in a vb
	const uint vertexInVB = 256;
	return std::max(1u, (uint) (vertexInVB / (_UsedNbSegs + 1)));
}

//==========================================================================
void CPSRibbonLookAt::enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv)
{
	NL_PS_FUNC(CPSRibbonLookAt_enumTexs)
	if (_Tex)
	{
		dest.push_back(_Tex);
		_Tex->getShareName();
	}
}


} // NL3D
