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

#include "nel/3d/ps_macro.h"
#include "nel/3d/ps_shockwave.h"
#include "nel/3d/driver.h"
#include "nel/3d/texture_grouped.h"
#include "nel/3d/ps_iterator.h"
#include "nel/3d/particle_system.h"


namespace NL3D
{

///////////////////////////
// constant definition   //
///////////////////////////

// max number of shockwave to be processed at once
static const uint ShockWaveBufSize = 128;

// the number of vertices we want in a vertex buffer
static const uint NumVertsInBuffer = 8 * ShockWaveBufSize;


CPSShockWave::TPBMap CPSShockWave::_PBMap; // the primitive blocks
CPSShockWave::TVBMap CPSShockWave::_VBMap; // vb ith unanimated texture
CPSShockWave::TVBMap CPSShockWave::_AnimTexVBMap; // vb ith unanimated texture
CPSShockWave::TVBMap CPSShockWave::_ColoredVBMap; // vb ith unanimated texture
CPSShockWave::TVBMap CPSShockWave::_ColoredAnimTexVBMap; // vb ith unanimated texture
/////////////////////////////////
// CPSShockWave implementation //
/////////////////////////////////


/** Well, we could have put a method template in CPSShockWave, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSShockWaveHelper
{
public:
	template <class T>
	static void drawShockWave(T posIt, CPSShockWave &s, uint size, uint32 srcStep)
	{
		NL_PS_FUNC(drawShockWave_drawShockWave)
		PARTICLES_CHECK_MEM;
		nlassert(s._Owner);

		// get / build the vertex buffer and the primitive block
		CVertexBuffer *vb;
		CIndexBuffer *pb;
		s.getVBnPB(vb, pb);

		const uint32 vSize = vb->getVertexSize();
		IDriver *driver = s.getDriver();
		if (s._ColorScheme)
		{
			s._ColorScheme->setColorType(driver->getVertexColorFormat());
		}
		s._Owner->incrementNbDrawnParticles(size); // for benchmark purpose
		s.setupDriverModelMatrix();
		const uint numShockWaveToDealWith = std::min(ShockWaveBufSize, s.getNumShockWavesInVB());


		static CPlaneBasis planeBasis[ShockWaveBufSize];
		float       sizes[ShockWaveBufSize];
		float       angles[ShockWaveBufSize];

		uint leftToDo  = size, toProcess;
		T endIt;
		uint8 *currVertex;
		uint k ;

		const float angleStep = 256.f / s._NbSeg;
		float currAngle;

		CPlaneBasis *ptCurrBasis;
		uint32	ptCurrBasisIncrement = s._PlaneBasisScheme ? 1 : 0;

		float *ptCurrSize;
		uint32 ptCurrSizeIncrement = s._SizeScheme ? 1 : 0;

		float *ptCurrAngle;
		uint32 ptCurrAngleIncrement = s._Angle2DScheme ? 1 : 0;

		CVector radVect, innerVect;
		float radiusRatio;


		do
		{
			toProcess = leftToDo > numShockWaveToDealWith ? numShockWaveToDealWith : leftToDo;
			vb->setNumVertices((toProcess * (s._NbSeg + 1)) << 1);
			{
				CVertexBufferReadWrite vba;
				vb->lock (vba);
				currVertex = (uint8 *) vba.getVertexCoordPointer();
				endIt = posIt + toProcess;
				if (s._SizeScheme)
				{
					ptCurrSize  = (float *) (s._SizeScheme->make(s._Owner, size - leftToDo, (void *) sizes, sizeof(float), toProcess, true, srcStep));
				}
				else
				{
					ptCurrSize = &s._ParticleSize;
				}

				if (s._PlaneBasisScheme)
				{
					ptCurrBasis  = (CPlaneBasis *) (s._PlaneBasisScheme->make(s._Owner, size - leftToDo, (void *) planeBasis, sizeof(CPlaneBasis), toProcess, true, srcStep));
				}
				else
				{
					ptCurrBasis = &s._PlaneBasis;
				}

				if (s._Angle2DScheme)
				{
					ptCurrAngle  = (float *) (s._Angle2DScheme->make(s._Owner, size - leftToDo, (void *) angles, sizeof(float), toProcess, true, srcStep));
				}
				else
				{
					ptCurrAngle = &s._Angle2D;
				}


				s.updateVbColNUVForRender(size - leftToDo, toProcess, srcStep, *vb, *driver);
				do
				{
					currAngle = *ptCurrAngle;
					if (fabsf(*ptCurrSize) > 10E-6)
					{
						radiusRatio = (*ptCurrSize - s._RadiusCut) / *ptCurrSize;
					}
					else
					{
						radiusRatio = 0.f;
					}

					for (k = 0; k <= s._NbSeg; ++k)
					{
						radVect = *ptCurrSize * (CPSUtil::getCos((sint32) currAngle) * ptCurrBasis->X + CPSUtil::getSin((sint32) currAngle) * ptCurrBasis->Y);
						innerVect = radiusRatio * radVect;
						CHECK_VERTEX_BUFFER(*vb, currVertex);
						* (CVector *) currVertex = *posIt + radVect;
						currVertex += vSize;
						CHECK_VERTEX_BUFFER(*vb, currVertex);
						* (CVector *) currVertex = *posIt + innerVect;
						currVertex += vSize;
						currAngle += angleStep;
					}

					++posIt;
					ptCurrBasis +=  ptCurrBasisIncrement;
					ptCurrSize  +=  ptCurrSizeIncrement;
					ptCurrAngle  +=  ptCurrAngleIncrement;
				}
				while (posIt != endIt);
			}

			const uint numTri = 2 * toProcess * s._NbSeg;
			pb->setNumIndexes(3 * numTri);
			driver->activeIndexBuffer(*pb);
			driver->activeVertexBuffer(*vb);
			driver->renderTriangles(s._Mat, 0, numTri);
			leftToDo -= toProcess;
		}
		while (leftToDo);
		PARTICLES_CHECK_MEM;
	}
};

///=================================================================================
CPSShockWave::CPSShockWave(uint nbSeg, float radiusCut, CSmartPtr<ITexture> tex)
		:  _NbSeg(nbSeg)
		   , _RadiusCut(radiusCut)
		   , _UFactor(1.f)

{
	NL_PS_FUNC(CPSShockWave_CPSShockWave)
	nlassert(nbSeg > 2 && nbSeg <= 64);
	setTexture(tex);
	init();
	if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("ShockWave");
}

///=================================================================================
uint32 CPSShockWave::getNumWantedTris() const
{
	NL_PS_FUNC(CPSShockWave_getNumWantedTris)
	nlassert(_Owner);
	//return (_Owner->getMaxSize() * _NbSeg) << 1 ;
	return (_Owner->getSize() * _NbSeg) << 1 ;
}

///=================================================================================
bool CPSShockWave::hasTransparentFaces(void)
{
	NL_PS_FUNC(CPSShockWave_hasTransparentFaces)
	return getBlendingMode() != CPSMaterial::alphaTest ;
}

///=================================================================================
bool CPSShockWave::hasOpaqueFaces(void)
{
	NL_PS_FUNC(CPSShockWave_hasOpaqueFaces)
	return !hasTransparentFaces();
}

///=================================================================================
void CPSShockWave::setNbSegs(uint nbSeg)
{
	NL_PS_FUNC(CPSShockWave_setNbSegs)
	nlassert(nbSeg > 2 && nbSeg <= 64);
	_NbSeg = nbSeg;
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
		//notifyOwnerMaxNumFacesChanged();
	}
}

///=================================================================================
void CPSShockWave::setRadiusCut(float radiusCut)
{
	NL_PS_FUNC(CPSShockWave_setRadiusCut)
	_RadiusCut = radiusCut;
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}
}

///=================================================================================
void	CPSShockWave::setUFactor(float value)
{
	NL_PS_FUNC(CPSShockWave_setUFactor)
	nlassert(_Owner); // must be attached to an owner before to call this method
	_UFactor = value;
	resize(_Owner->getSize()); // resize also recomputes the UVs..
}

///=================================================================================
void CPSShockWave::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSShockWave_serial)
	sint ver  = f.serialVersion(2);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSTexturedParticle::serialTextureScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	serialMaterial(f);
	f.serial(_NbSeg, _RadiusCut);
	if (ver > 1)
	{
		f.serial(_UFactor);
	}
	init();
}

///=================================================================================
inline void CPSShockWave::setupUFactor()
{
	NL_PS_FUNC(CPSShockWave_setupUFactor)
	if (_UFactor != 1.f)
	{
		_Mat.enableUserTexMat(0);
		CMatrix texMat;
		texMat.setRot(_UFactor  * NLMISC::CVector::I,
					  NLMISC::CVector::J,
					  NLMISC::CVector::K
					 );
		_Mat.setUserTexMat(0, texMat);
	}
	else
	{
		_Mat.enableUserTexMat(0, false);
	}
}

///=================================================================================
void CPSShockWave::draw(bool opaque)
{
//	if (!FilterPS[7]) return;
	NL_PS_FUNC(CPSShockWave_draw)
	PARTICLES_CHECK_MEM;
	if (!_Owner->getSize()) return;

	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);
	if (!numToProcess) return;



	/// update the material if the global color of the system is variable
	CParticleSystem &ps = *(_Owner->getOwner());
	/// update the material if the global color of the system is variable
	if (_ColorScheme != NULL &&
		(ps.getColorAttenuationScheme() != NULL ||
		 ps.isUserColorUsed() ||
		 ps.getForceGlobalColorLightingFlag()   ||
		 usesGlobalColorLighting()
		)
	   )
	{
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{
			CPSMaterial::forceModulateConstantColor(true, ps.getGlobalColorLighted());
		}
		else
		{
			CPSMaterial::forceModulateConstantColor(true, ps.getGlobalColor());
		}
	}
	else
	{
		forceModulateConstantColor(false);
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{
			NLMISC::CRGBA col;
			col.modulateFromColor(ps.getGlobalColorLighted(), _Color);
			_Mat.setColor(col);
		}
		else
		if (!ps.getColorAttenuationScheme() || ps.isUserColorUsed())
		{
			_Mat.setColor(_Color);
		}
		else
		{
			NLMISC::CRGBA col;
			col.modulateFromColor(ps.getGlobalColor(), _Color);
			_Mat.setColor(col);
		}
	}
	//////

	setupUFactor();

	if (step == (1 << 16))
	{
		CPSShockWaveHelper::drawShockWave(_Owner->getPos().begin(),
										  *this,
										  numToProcess,
										  step
										 );
	}
	else
	{
		CPSShockWaveHelper::drawShockWave(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
										  *this,
										  numToProcess,
										  step
										 );
	}

	PARTICLES_CHECK_MEM;
}

///=================================================================================
bool CPSShockWave::completeBBox(NLMISC::CAABBox &box) const
{
	NL_PS_FUNC(CPSShockWave_completeBBox)
	// TODO : implement this
	return false;
}

///=================================================================================
void CPSShockWave::init(void)
{
	NL_PS_FUNC(CPSShockWave_init)
	_Mat.setLighting(false);
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);
	updateMatAndVbForColor();
	updateMatAndVbForTexture();
}

///=================================================================================
void CPSShockWave::updateVbColNUVForRender(uint32 startIndex, uint32 size, uint32 srcStep, CVertexBuffer &vb, IDriver &drv)
{
	NL_PS_FUNC(CPSShockWave_updateVbColNUVForRender)
	nlassert(_Owner);
	CVertexBufferReadWrite vba;
	vb.lock (vba);
	if (!size) return;
	if (_ColorScheme)
	{
		// compute the colors, each color is replicated n times...
		_ColorScheme->makeN(_Owner, startIndex, vba.getColorPointer(), vb.getVertexSize(), size, (_NbSeg + 1) << 1, srcStep);
	}

	if (_TexGroup) // if it has a constant texture we are sure it has been setupped before...
	{
		sint32 textureIndex[ShockWaveBufSize];
		const uint32 stride = vb.getVertexSize(), stride2 = stride << 1;
		uint8 *currUV = (uint8 *) vba.getTexCoordPointer();
		uint k;

		uint32 currIndexIncr;
		const sint32 *currIndex;

		if (_TextureIndexScheme)
		{
			currIndex  = (sint32 *) (_TextureIndexScheme->make(_Owner, startIndex, textureIndex, sizeof(sint32), size, true, srcStep));
			currIndexIncr = 1;
		}
		else
		{
			currIndex = &_TextureIndex;
			currIndexIncr = 0;
		}

		while (size--)
		{
			// for now, we don't make texture index wrapping
			const CTextureGrouped::TFourUV &uvGroup = _TexGroup->getUVQuad((uint32) *currIndex);

			for (k = 0; k <= _NbSeg; ++k)
			{

				*(CUV *) currUV = uvGroup.uv0 + CUV(k * _UFactor, 0);
				*(CUV *) (currUV + stride) = uvGroup.uv3 + CUV(k * _UFactor, 0);
				// point the next quad
				currUV += stride2;
			}

			currIndex += currIndexIncr;
		}
	}
}

///=================================================================================
void CPSShockWave::updateMatAndVbForColor(void)
{
	NL_PS_FUNC(CPSShockWave_updateMatAndVbForColor)
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}
}

///=================================================================================
void CPSShockWave::updateMatAndVbForTexture(void)
{
	NL_PS_FUNC(CPSShockWave_updateMatAndVbForTexture)
	_Mat.setTexture(0, _TexGroup ? (ITexture *) _TexGroup : (ITexture *) _Tex);
}

///=================================================================================
void CPSShockWave::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSShockWave_newElement)
	newColorElement(info);
	newTextureIndexElement(info);
	newSizeElement(info);
	newAngle2DElement(info);
}

///=================================================================================
void CPSShockWave::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSShockWave_deleteElement)
	deleteColorElement(index);
	deleteTextureIndexElement(index);
	deleteSizeElement(index);
	deleteAngle2DElement(index);
}

///=================================================================================
void CPSShockWave::resize(uint32 aSize)
{
	NL_PS_FUNC(CPSShockWave_resize)
	nlassert(aSize < (1 << 16));
	resizeColor(aSize);
	resizeTextureIndex(aSize);
	resizeSize(aSize);
	resizeAngle2D(aSize);
}

///=================================================================================
void CPSShockWave::getVBnPB(CVertexBuffer *&retVb, CIndexBuffer *&retPb)
{
	NL_PS_FUNC(CPSShockWave_getVBnPB)
	TVBMap &vbMap = _ColorScheme == NULL  ? (_TexGroup == NULL ?  _VBMap : _AnimTexVBMap)
										  : (_TexGroup == NULL ?  _ColoredVBMap : _ColoredAnimTexVBMap);


	TVBMap::iterator vbIt = vbMap.find(_NbSeg);
	if (vbIt != vbMap.end())
	{
		retVb = &(vbIt->second);
		TPBMap::iterator pbIt = _PBMap.find(_NbSeg);
		nlassert(pbIt != _PBMap.end());
		retPb = &(pbIt->second);
	}
	else // we need to create the vb
	{
		// create an entry (we setup the primitive block at the same time, this could be avoided, but doesn't make much difference)
		CVertexBuffer &vb = vbMap[_NbSeg]; // create a vb
		CIndexBuffer &pb = _PBMap[_NbSeg]; // eventually create a pb
		const uint32 size = getNumShockWavesInVB();
		vb.setVertexFormat(CVertexBuffer::PositionFlag |
						   CVertexBuffer::TexCoord0Flag |
						   (_ColorScheme != NULL ?  CVertexBuffer::PrimaryColorFlag : 0)
						  );
		vb.setNumVertices((size * (_NbSeg + 1)) << 1 );
		vb.setPreferredMemory(CVertexBuffer::AGPVolatile, true);
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		pb.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
		pb.setNumIndexes(2 * 3 * size * _NbSeg);
		CIndexBufferReadWrite ibaWrite;
		pb.lock (ibaWrite);
		uint finalIndex = 0;
		for (uint32 k = 0; k < size; ++k)
		{
			for (uint32 l = 0; l < _NbSeg; ++l)
			{
				const uint32 index = ((k * (_NbSeg + 1)) + l) << 1;
				ibaWrite.setTri(finalIndex, index + 1 , index + 3, index + 2);
				finalIndex+=3;
				ibaWrite.setTri(finalIndex, index + 1, index + 2, index + 0);
				finalIndex+=3;
				vba.setTexCoord(index, 0, CUV((float) l, 0));
				vba.setTexCoord(index + 1, 0, CUV((float) l, 1));
			}
			const uint32 index = ((k * (_NbSeg + 1)) + _NbSeg) << 1;
			vba.setTexCoord(index, 0, CUV((float) _NbSeg, 0));
			vba.setTexCoord(index + 1, 0, CUV((float) _NbSeg, 1));
		}
		retVb = &vb;
		retPb = &pb;
		vb.setName("CPSShockWave");
		NL_SET_IB_NAME(pb, "CPSShockWave");
	}
}

///=================================================================================
uint CPSShockWave::getNumShockWavesInVB() const
{
	NL_PS_FUNC(CPSShockWave_getNumShockWavesInVB)
	const uint numRib = NumVertsInBuffer / ((_NbSeg + 1) << 1);
	return std::max(1u, numRib);
}

///=================================================================================
void CPSShockWave::enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv)
{
	NL_PS_FUNC(CPSShockWave_enumTexs)
	CPSTexturedParticle::enumTexs(dest);
}

} // NL3D
