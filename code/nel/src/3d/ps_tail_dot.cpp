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

#include "nel/3d/ps_tail_dot.h"
#include "nel/3d/ps_macro.h"
#include "nel/3d/driver.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/texture_mem.h"
#include "nel/misc/smart_ptr.h"

#include <memory>

namespace NL3D
{
static NLMISC::CRGBA GradientB2W[] = {NLMISC::CRGBA(0, 0, 0, 0), NLMISC::CRGBA(255, 255, 255, 255) };

/// private use : this create a gradient texture that goew from black to white
static ITexture *CreateGradientTexture()
{
	NL_PS_FUNC(CreateGradientTexture)
	std::auto_ptr<CTextureMem> tex(new CTextureMem((uint8 *) &GradientB2W,
												   sizeof(GradientB2W),
												   false, /* dont delete */
												   false, /* not a file */
												   2, 1)
								  );
	tex->setWrapS(ITexture::Clamp);
	tex->setShareName("#GradBW");
	return tex.release();
}


///////////////////////////////
// CPSTailDot implementation //
///////////////////////////////

CPSTailDot::TVBMap			CPSTailDot::_VBMap;			  // index / vertex buffers with no color
CPSTailDot::TVBMap			CPSTailDot::_FadedVBMap;	  // index / vertex buffers for constant color with fading
CPSTailDot::TVBMap			CPSTailDot::_ColoredVBMap;    // index / vertex buffer + colors
CPSTailDot::TVBMap			CPSTailDot::_FadedColoredVBMap;    // index / vertex buffer + faded colors

//=======================================================
CPSTailDot::CPSTailDot() : _ColorFading(false),
						   _GlobalColor(false),
						   _Lighted(false),
						   _ForceLighted(false),
						   _Touch(true)
{
	NL_PS_FUNC(CPSTailDot_CPSTailDot)
	setInterpolationMode(Linear);
	setSegDuration(0.06f);
	if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("TailDot");
}

//=======================================================
CPSTailDot::~CPSTailDot()
{
	NL_PS_FUNC(CPSTailDot_CPSTailDotDtor)
//	delete _DyingRibbons;
}

//=======================================================
void CPSTailDot::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSTailDot_serial)

	sint ver = f.serialVersion(3);
	if (ver == 1)
	{
		nlassert(f.isReading());

		/// we had CPSParticle::serial(f), but this is not the base class anymore, so we emulate this...
		/// version 2 : auto-lod saved
		sint ver2 = f.serialVersion(2);

		// here is CPSLocatedBindable::serial(f)
		sint ver3 = f.serialVersion(4);
		f.serialPtr(_Owner);
		if (ver3 > 1) f.serialEnum(_LOD);
		if (ver3 > 2) f.serial(_Name);
		if (ver3 > 3)
		{
			if (f.isReading())
			{
				uint32 id;
				f.serial(id);
				setExternID(id);
			}
			else
			{
				f.serial(_ExternID);
			}
		}

		if (ver2 >= 2)
		{
			bool bDisableAutoLOD;
			f.serial(bDisableAutoLOD);
			disableAutoLOD(bDisableAutoLOD);
		}

		uint32 tailNbSegs;
		bool   colorFading;
		bool   systemBasisEnabled;

		CPSColoredParticle::serialColorScheme(f);
		f.serial(tailNbSegs, colorFading, systemBasisEnabled);

		_ColorFading = colorFading;
		_NbSegs = tailNbSegs >> 1;
		if (_NbSegs < 2) _NbSegs = 2;
		setInterpolationMode(Linear);
		serialMaterial(f);


		nlassert(_Owner);
		resize(_Owner->getMaxSize());
		initDateVect();
		resetFromOwner();
	}

	if (ver >= 2)
	{
		CPSRibbonBase::serial(f);
		CPSColoredParticle::serialColorScheme(f);
		CPSMaterial::serialMaterial(f);
		bool colorFading = _ColorFading;
		f.serial(colorFading);
		_ColorFading = colorFading;
		if (ver >= 3)
		{
			uint32 tailNbSegs = _NbSegs;
			f.serial(tailNbSegs);
		}
		if (f.isReading())
		{
			setTailNbSeg(_NbSegs);
			touch();
		}
	}
}


//=======================================================
void CPSTailDot::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSTailDot_step)
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
void CPSTailDot::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSTailDot_newElement)
	CPSRibbonBase::newElement(info);
	newColorElement(info);
}


//=======================================================
void CPSTailDot::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSTailDot_deleteElement)
	CPSRibbonBase::deleteElement(index);
	deleteColorElement(index);
}


//=======================================================
void CPSTailDot::resize(uint32 size)
{
	NL_PS_FUNC(CPSTailDot_resize)
	nlassert(size < (1 << 16));
	CPSRibbonBase::resize(size);
	resizeColor(size);
}

//=======================================================
void CPSTailDot::updateMatAndVbForColor(void)
{
	NL_PS_FUNC(CPSTailDot_updateMatAndVbForColor)
	touch();
}

//==========================================================================
void CPSTailDot::displayRibbons(uint32 nbRibbons, uint32 srcStep)
{
//	if (!FilterPS[8]) return;
	NL_PS_FUNC(CPSTailDot_displayRibbons)
	if (!nbRibbons) return;
	nlassert(_Owner);
	CPSRibbonBase::updateLOD();
	if (_UsedNbSegs < 2) return;
	const float date = _Owner->getOwner()->getSystemDate();
	uint8						*currVert;
	CVBnPB						&VBnPB = getVBnPB(); // get the appropriate vb (built it if needed)
	CVertexBuffer				&VB = VBnPB.VB;
	CIndexBuffer				&PB = VBnPB.PB;
	const uint32				vertexSize  = VB.getVertexSize();
	uint						colorOffset=0;

	IDriver *drv = this->getDriver();
	#ifdef NL_DEBUG
		nlassert(drv);
	#endif
	drv->setupModelMatrix(getLocalToWorldTrailMatrix());
	_Owner->incrementNbDrawnParticles(nbRibbons); // for benchmark purpose
	const uint numRibbonBatch = getNumRibbonsInVB(); // number of ribons to process at once
	if (_UsedNbSegs == 0) return;

	////////////////////
	// Material setup //
	////////////////////
		CParticleSystem &ps = *(_Owner->getOwner());
		bool useGlobalColor = ps.getColorAttenuationScheme() != NULL || ps.isUserColorUsed();
		if (useGlobalColor != _GlobalColor)
		{
			_GlobalColor = useGlobalColor;
			touch();
		}
		if (usesGlobalColorLighting() != _Lighted)
		{
			_Lighted = usesGlobalColorLighting();
			touch();
		}
		if (ps.getForceGlobalColorLightingFlag() != _ForceLighted)
		{
			_ForceLighted = ps.getForceGlobalColorLightingFlag();
			touch();
		}
		updateMaterial();
		setupGlobalColor();
		//
		if (_ColorScheme)
		{
			colorOffset = VB.getColorOff();
		}

	/////////////////////
	// Compute ribbons //
	/////////////////////

	uint toProcess;
	uint ribbonIndex = 0; // index of the first ribbon in the batch being processed
	uint32 fpRibbonIndex = 0; // fixed point index in source
	if (_ColorScheme)
	{
		_ColorScheme->setColorType(drv->getVertexColorFormat());
	}
	do
	{
		toProcess = std::min((uint) (nbRibbons - ribbonIndex) /* = left to do */, numRibbonBatch);
		VB.setNumVertices((_UsedNbSegs + 1) * toProcess);
		{
			CVertexBufferReadWrite vba;
			VB.lock (vba);
			currVert = (uint8 *) vba.getVertexCoordPointer();

			/// compute colors
			if (_ColorScheme)
			{
				_ColorScheme->makeN(this->_Owner, ribbonIndex, currVert + colorOffset, vertexSize, toProcess, _UsedNbSegs + 1, srcStep);
			}
			uint k = toProcess;
			//////////////////////////////////////////////////////////////////////////////////////
			// interpolate and project points the result is directly setup in the vertex buffer //
			//////////////////////////////////////////////////////////////////////////////////////
			if (!_Parametric)
			{

				//////////////////////
				// INCREMENTAL CASE //
				//////////////////////
				do
				{
					// the parent class has a method to get the ribbons positions
					computeRibbon((uint) (fpRibbonIndex >> 16), (CVector *) currVert, vertexSize);
					currVert += vertexSize * (_UsedNbSegs + 1);
					fpRibbonIndex += srcStep;
				}
				while (--k);
			}
			else
			{
				//////////////////////
				// PARAMETRIC  CASE //
				//////////////////////
				do
				{
					// we compute each pos thanks to the parametric curve
					_Owner->integrateSingle(date - _UsedSegDuration * (_UsedNbSegs + 1), _UsedSegDuration, _UsedNbSegs + 1, (uint) (fpRibbonIndex >> 16),
											(NLMISC::CVector *) currVert, vertexSize);
					currVert += vertexSize * (_UsedNbSegs + 1);
					fpRibbonIndex += srcStep;
				}
				while (--k);

			}
		}
		const uint numLine = _UsedNbSegs * toProcess;
		PB.setNumIndexes(2 * numLine);
		// display the result
		drv->activeIndexBuffer(PB);
		drv->activeVertexBuffer(VB);
		drv->renderLines (_Mat, 0, numLine);
		ribbonIndex += toProcess;
	}
	while (ribbonIndex != nbRibbons);
}

//==========================================================================
bool CPSTailDot::hasTransparentFaces(void)
{
	NL_PS_FUNC(CPSTailDot_hasTransparentFaces)
	return getBlendingMode() != CPSMaterial::alphaTest ;
}


//==========================================================================
bool CPSTailDot::hasOpaqueFaces(void)
{
	NL_PS_FUNC(CPSTailDot_hasOpaqueFaces)
	return !hasTransparentFaces();
}

//==========================================================================
uint32 CPSTailDot::getNumWantedTris() const
{
	NL_PS_FUNC(CPSTailDot_getNumWantedTris)
	nlassert(_Owner);
	//return _Owner->getMaxSize() * _NbSegs;
	return _Owner->getSize() * _NbSegs;
}



//==========================================================================
CPSTailDot::CVBnPB &CPSTailDot::getVBnPB()
{
	NL_PS_FUNC(CPSTailDot_getVBnPB)
	/// choose the right vb
	TVBMap &map = _ColorScheme ? (_ColorFading ? _FadedColoredVBMap : _ColoredVBMap)		// per ribbon color
							   : (_ColorFading ? _FadedVBMap : _VBMap);     // global color
	TVBMap::iterator it = map.find(_UsedNbSegs + 1);
	if (it != map.end())
	{
		return it->second;
	}
	else	// must create this vb, with few different size, it is still interseting, though they are only destroyed at exit
	{
		const uint numRibbonInVB = getNumRibbonsInVB();
		CVBnPB &VBnPB = map[_UsedNbSegs + 1]; // make an entry

		/// set the vb format & size
		/// In the case of a ribbon with color and fading, we encode the fading in a texture
		/// If the ribbon has fading, but only a global color, we encode it in the primary color
		CVertexBuffer &vb = VBnPB.VB;
		vb.setPreferredMemory(CVertexBuffer::AGPVolatile, true);
		vb.setVertexFormat(CVertexBuffer::PositionFlag
						   |(_ColorScheme || _ColorFading ? CVertexBuffer::PrimaryColorFlag : 0)
						   | (_ColorScheme && _ColorFading ? CVertexBuffer::TexCoord0Flag : 0));

		vb.setNumVertices((_UsedNbSegs + 1) * numRibbonInVB ); // 1 seg = 1 line + terminal vertices

		// set the primitive block size
		CIndexBuffer &pb = VBnPB.PB;
		pb.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
		pb.setNumIndexes(2 * _UsedNbSegs * numRibbonInVB);
		/// Setup the pb and vb parts. Not very fast but executed only once
		uint vbIndex = 0;
		uint pbIndex = 0;
		CIndexBufferReadWrite ibaWrite;
		pb.lock (ibaWrite);
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		for (uint i = 0; i < numRibbonInVB; ++i)
		{
			for (uint k = 0; k < (_UsedNbSegs + 1); ++k)
			{

				if (_ColorScheme && _ColorFading)
				{
					vba.setTexCoord(vbIndex, 0, 0.5f - 0.5f * ((float) k / _UsedNbSegs), 0);
				}
				else if (_ColorFading)
				{
					uint8 intensity = (uint8) (255 * (1.f - ((float) k / _UsedNbSegs)));
					NLMISC::CRGBA col(intensity, intensity, intensity, intensity);
					vba.setColor(vbIndex, col);
				}

					/// add 1 line in the primitive block
				if (k != _UsedNbSegs)
				{
					ibaWrite.setLine(pbIndex, vbIndex, vbIndex + 1);
					pbIndex+=2;
				}
				++vbIndex;
			}
		}
		vb.setName("CPSTailDot");
		NL_SET_IB_NAME(pb, "CPSTailDot");
		return VBnPB;
	}
}

//==========================================================================
uint	CPSTailDot::getNumRibbonsInVB() const
{
	NL_PS_FUNC(CPSTailDot_getNumRibbonsInVB)
	/// approximation of the max number of vertices we want in a vb
	const uint vertexInVB = 256;
	return std::max(1u, (uint) (vertexInVB / (_UsedNbSegs + 1)));
}


//==========================================================================
void	CPSTailDot::updateMaterial()
{
	NL_PS_FUNC(CPSTailDot_updateMaterial)
	if (!_Touch) return;

	static NLMISC::CRefPtr<ITexture> ptGradTexture;

	CParticleSystem &ps = *(_Owner->getOwner());
	if (_ColorScheme)
	{	// PER RIBBON COLOR
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting() || ps.getColorAttenuationScheme() || ps.isUserColorUsed())
		{
			if (_ColorFading) // global color + fading + per ribbon color
			{
				// the first stage is used to get fading * global color
				// the second stage multiply the result by the diffuse colot
				if (ptGradTexture == NULL) // have we got a gradient texture ?
				{
					ptGradTexture = CreateGradientTexture();
				}
				_Mat.setTexture(0, ptGradTexture);
				CPSMaterial::forceTexturedMaterialStages(2); // use constant color 0 * diffuse, 1 stage needed
				SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Constant);
				SetupModulatedStage(_Mat, 1, CMaterial::Previous, CMaterial::Diffuse);
			}
			else // per ribbon color with global color
			{
				CPSMaterial::forceTexturedMaterialStages(1); // use constant color 0 * diffuse, 1 stage needed
				SetupModulatedStage(_Mat, 0, CMaterial::Diffuse, CMaterial::Constant);
			}
		}
		else
		{
			if (_ColorFading) // per ribbon color, fading
			{
				if (ptGradTexture == NULL) // have we got a gradient texture ?
				{
					ptGradTexture = CreateGradientTexture();
				}
				_Mat.setTexture(0, ptGradTexture);
				CPSMaterial::forceTexturedMaterialStages(1);
				SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Diffuse);
			}
			else // per color ribbon with no fading, and no global color
			{
				CPSMaterial::forceTexturedMaterialStages(0); // no texture use constant diffuse only
			}
		}
	}
	else // GLOBAL COLOR
	{
		if (_ColorFading)
		{
			CPSMaterial::forceTexturedMaterialStages(1); // use constant color 0 * diffuse, 1 stage needed
			SetupModulatedStage(_Mat, 0, CMaterial::Diffuse, CMaterial::Constant);
		}
		else // constant color
		{
			CPSMaterial::forceTexturedMaterialStages(0); // no texture use constant diffuse only
		}
	}

	_Touch = false;
}

//==========================================================================
void	CPSTailDot::setupGlobalColor()
{
	NL_PS_FUNC(CPSTailDot_setupGlobalColor)
	/// setup the global color if it is used
	CParticleSystem &ps = *(_Owner->getOwner());
	if (_ColorScheme)
	{
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{
			_Mat.texConstantColor(0, ps.getGlobalColorLighted());
		}
		else
		{
			_Mat.texConstantColor(0, ps.getGlobalColor());
		}
	}
	else // GLOBAL COLOR with / without fading
	{
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{
			NLMISC::CRGBA col;
			col.modulateFromColor(ps.getGlobalColorLighted(), _Color);
			if (_ColorFading)
			{
				_Mat.texConstantColor(0, col);
			}
			else // color attenuation, no fading :
			{
				_Mat.setColor(col);
			}
		}
		else
		if (ps.getColorAttenuationScheme() || ps.isUserColorUsed())
		{
			NLMISC::CRGBA col;
			col.modulateFromColor(ps.getGlobalColor(), _Color);
			if (_ColorFading)
			{
				_Mat.texConstantColor(0, col);
			}
			else // color attenuation, no fading :
			{
				_Mat.setColor(col);
			}
		}
		else
		{
			if (_ColorFading)
			{
				_Mat.texConstantColor(0, _Color);
			}
			else // constant color
			{
				_Mat.setColor(_Color);
			}
		}
	}
}

} // NL3D
