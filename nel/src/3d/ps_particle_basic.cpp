// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/ps_macro.h"
#include "nel/3d/driver.h"
#include "nel/3d/texture_grouped.h"
#include "nel/3d/texture_bump.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/particle_system.h"


#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{



/////////////////////////////////
// CPSParticle implementation  //
/////////////////////////////////

//=======================================
CPSParticle::CPSParticle() : _DisableAutoLOD(false),
							 _UsesGlobalColorLighting(false)
{
	NL_PS_FUNC(CPSParticle_CPSParticle)
}

//=======================================
void CPSParticle::showTool()
{
	NL_PS_FUNC(CPSParticle_showTool)
	PARTICLES_CHECK_MEM;

	CVector I = CVector::I;
	CVector J = CVector::J;

	const CVector tab[] = { 2 * J, I + J
							, I + J, 2 * I + J
							, 2 * I + J, I
							, I,  2 * I - J
							, 2 * I - J, - .5f * J
							, - .5f * J, -2 * I - J
							, -2 * I - J, - I
							, - I, -2 * I + J
							, -2 * I + J, - I + J
							, - I + J, 2 * J
						};
	const uint tabSize = sizeof(tab) / (2 * sizeof(CVector));

	const float sSize = 0.1f;
	displayIcon2d(tab, tabSize, sSize);

	PARTICLES_CHECK_MEM;
}

//=======================================
void CPSParticle::computeSrcStep(uint32 &step, uint &numToProcess)
{
	NL_PS_FUNC(CPSParticle_computeSrcStep)
	nlassert(_Owner && _Owner->getOwner());
	const CParticleSystem &ps = *(_Owner->getOwner());
	if (_DisableAutoLOD || !ps.isAutoLODEnabled() || !ps.isSharingEnabled() || _Owner->getSize() == 0) // Should Auto-LOD be used ?
	{
		step = (1 << 16);
		numToProcess = _Owner->getSize();
	}
	else
	{
		float oneMinusLODRatio = ps.getOneMinusCurrentLODRatio();
		float LODRatio = 1.f - oneMinusLODRatio;
		if (LODRatio > ps.getAutoLODStartDistPercent())
		{
			float factor = (LODRatio - 1.f) / (ps.getAutoLODStartDistPercent() - 1.f);
			NLMISC::clamp(factor, 0.f, 1.f);
			float r = factor;
			for (uint k = 1; k < ps.getAutoLODDegradationExponent(); ++k)
			{
				r *= factor;
			}
			numToProcess = (uint) (_Owner->getSize() * r);
			if (numToProcess < 1) { numToProcess = 1; }

			step =	 ps.getAutoLODMode() ?				   // skip or limit number, depending on the mode
				(_Owner->getSize() << 16) / numToProcess : // skip particles
				(1 << 16);							   // just display less particles
		}
		else
		{
			step = (1 << 16);
			numToProcess = _Owner->getSize();
		}
	}

}

//////////////////////////////////////
// coloured particle implementation //
//////////////////////////////////////

//=======================================
void CPSColoredParticle::setColorScheme(CPSAttribMaker<CRGBA> *col)
{
	NL_PS_FUNC(CPSColoredParticle_setColorScheme)
	nlassert(col);
	delete _ColorScheme;
	_ColorScheme = col;
	if (getColorOwner() && col->hasMemory()) col->resize(getColorOwner()->getMaxSize(), getColorOwner()->getSize());
	updateMatAndVbForColor();
}

//=======================================
void CPSColoredParticle::setColor(NLMISC::CRGBA col)
{
	NL_PS_FUNC(CPSColoredParticle_setColor)
	delete _ColorScheme;
	_ColorScheme = NULL;
	_Color = col;
	updateMatAndVbForColor();
}

//=======================================
CPSColoredParticle::CPSColoredParticle() : _Color(CRGBA(255, 255, 255)), _ColorScheme(NULL)
{
	NL_PS_FUNC(CPSColoredParticle_CPSColoredParticle)
}

//=======================================
CPSColoredParticle::~CPSColoredParticle()
{
	NL_PS_FUNC(CPSColoredParticle_CPSColoredParticleDtor)
	delete _ColorScheme;
}

//=======================================
void CPSColoredParticle::serialColorScheme(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSColoredParticle_IStream )
	f.serialVersion(1);
	if (f.isReading())
	{
		if (_ColorScheme)
		{
			delete _ColorScheme;
			_ColorScheme = NULL;
		}
	}
	bool useColorScheme = _ColorScheme != NULL;
	f.serial(useColorScheme);
	if (useColorScheme)
	{
		f.serialPolyPtr(_ColorScheme);
	}
	else
	{
		f.serial(_Color);
	}
}

///////////////////////////////////
// sized particle implementation //
///////////////////////////////////

//=======================================
void CPSSizedParticle::setSizeScheme(CPSAttribMaker<float> *size)
{
	NL_PS_FUNC(CPSSizedParticle_setSizeScheme)
	nlassert(size != NULL);
	delete _SizeScheme;
	_SizeScheme = size;
	if (getSizeOwner() && size->hasMemory()) size->resize(getSizeOwner()->getMaxSize(), getSizeOwner()->getSize());
}

//=======================================
void CPSSizedParticle::setSize(float size)
{
	NL_PS_FUNC(CPSSizedParticle_setSize)
	delete _SizeScheme;
	_SizeScheme = NULL;
	_ParticleSize = size;
}

//=======================================
CPSSizedParticle::CPSSizedParticle() : _ParticleSize(0.3f), _SizeScheme(NULL)
{
	NL_PS_FUNC(CPSSizedParticle_CPSSizedParticle)
}

//=======================================
CPSSizedParticle::~CPSSizedParticle()
{
	NL_PS_FUNC(CPSSizedParticle_CPSSizedParticleDtor)
	delete _SizeScheme;
}

//=======================================
void CPSSizedParticle::serialSizeScheme(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSSizedParticle_serialSizeScheme)
	f.serialVersion(1);
	if (f.isReading())
	{
		if (_SizeScheme)
		{
			delete _SizeScheme;
			_SizeScheme = NULL;
		}
	}
	bool useSizeScheme = _SizeScheme != NULL;
	f.serial(useSizeScheme);
	if (useSizeScheme)
	{
		f.serialPolyPtr(_SizeScheme);
	}
	else
	{
		f.serial(_ParticleSize);
	}
};

/////////////////////////////////////
// rotated particle implementation //
/////////////////////////////////////

float CPSRotated2DParticle::_RotTable[256 * 4];
bool CPSRotated2DParticle::_InitializedRotTab = false;

///===================================================================================
void CPSRotated2DParticle::setAngle2DScheme(CPSAttribMaker<float> *angle2DScheme)
{
	NL_PS_FUNC(CPSRotated2DParticle_setAngle2DScheme)
	nlassert(angle2DScheme);
	delete _Angle2DScheme;
	_Angle2DScheme = angle2DScheme;
	if (getAngle2DOwner() && angle2DScheme->hasMemory()) angle2DScheme->resize(getAngle2DOwner()->getMaxSize(), getAngle2DOwner()->getSize());
}

///===================================================================================
void CPSRotated2DParticle::setAngle2D(float angle2DScheme)
{
	NL_PS_FUNC(CPSRotated2DParticle_setAngle2D)
	delete _Angle2DScheme;
	_Angle2DScheme = NULL;
	_Angle2D = angle2DScheme;
}

///===================================================================================
CPSRotated2DParticle::CPSRotated2DParticle() : _Angle2D(0), _Angle2DScheme(NULL)
{
	NL_PS_FUNC(CPSRotated2DParticle_CPSRotated2DParticle)
}

///===================================================================================
CPSRotated2DParticle::~CPSRotated2DParticle()
{
	NL_PS_FUNC(CPSRotated2DParticle_CPSRotated2DParticleDtor)
	delete _Angle2DScheme;
}

///===================================================================================
void CPSRotated2DParticle::serialAngle2DScheme(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSRotated2DParticle_serialAngle2DScheme)
	f.serialVersion(1);
	if (f.isReading())
	{
		if (_Angle2DScheme)
		{
			delete _Angle2DScheme;
			_Angle2DScheme = NULL;
		}
	}
	bool useAngle2DScheme = _Angle2DScheme != NULL;
	f.serial(useAngle2DScheme);
	if (useAngle2DScheme)
	{
		f.serialPolyPtr(_Angle2DScheme);
	}
	else
	{
		f.serial(_Angle2D);
	}
}

///===================================================================================
void CPSRotated2DParticle::initRotTable(void)
{
	NL_PS_FUNC(CPSRotated2DParticle_initRotTable)
	float *ptFloat = _RotTable;
	for (uint32 k = 0; k < 256; ++k)
	{
		const float ca = (float) cos(k * (1.0f / 256.0f) * 2.0f * NLMISC::Pi);
		const float sa = (float) sin(k * (1.0f / 256.0f) * 2.0f * NLMISC::Pi);

		*ptFloat++ = -ca - sa;
		*ptFloat++ = -sa + ca;

		*ptFloat++ = ca - sa;
		*ptFloat++ = sa + ca;
	}
	_InitializedRotTab = true;
}

//////////////////////////////////////
// textured particle implementation //
//////////////////////////////////////

///===================================================================================
void CPSTexturedParticle::setTextureIndexScheme(CPSAttribMaker<sint32> *animOrder)
{
	NL_PS_FUNC(CPSTexturedParticle_setTextureIndexScheme)
	nlassert(animOrder);
	nlassert(_TexGroup); // setTextureGroup must have been called before this
	delete _TextureIndexScheme;
	_TextureIndexScheme = animOrder;
	if (getTextureIndexOwner() && animOrder->hasMemory()) animOrder->resize(getTextureIndexOwner()->getMaxSize(), getTextureIndexOwner()->getSize());


	updateMatAndVbForTexture();
}

///===================================================================================
void CPSTexturedParticle::setTextureIndex(sint32 index)
{
	NL_PS_FUNC(CPSTexturedParticle_setTextureIndex)
	delete _TextureIndexScheme;
	_TextureIndexScheme = NULL;
	_TextureIndex = index;
}

///===================================================================================
void CPSTexturedParticle::setTextureGroup(NLMISC::CSmartPtr<CTextureGrouped> texGroup)
{
	NL_PS_FUNC(CPSTexturedParticle_setTextureGroup)
	nlassert(texGroup);
	if (_Tex)
	{
		_Tex = NULL;
	}
	_TexGroup = texGroup;
	updateMatAndVbForTexture();
}

///===================================================================================
void CPSTexturedParticle::setTexture(CSmartPtr<ITexture> tex)
{
	NL_PS_FUNC(CPSTexturedParticle_setTexture)
	delete _TextureIndexScheme;
	_TextureIndexScheme = NULL;
	_Tex = tex;
	_TexGroup = NULL; // release any grouped texture if one was set before
	updateMatAndVbForTexture();
}

///===================================================================================
CPSTexturedParticle::CPSTexturedParticle() : _TexGroup(NULL),
											 _TextureIndexScheme(NULL),
											 _TextureIndex(0)
{
	NL_PS_FUNC(CPSTexturedParticle_CPSTexturedParticle)
}

///===================================================================================
CPSTexturedParticle::~CPSTexturedParticle()
{
	NL_PS_FUNC(CPSTexturedParticle_CPSTexturedParticleDtor)
	delete _TextureIndexScheme;
}

///===================================================================================
void CPSTexturedParticle::serialTextureScheme(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSTexturedParticle_serialTextureScheme)
	f.serialVersion(1);
	if (f.isReading())
	{
		if (_TextureIndexScheme)
		{
			delete _TextureIndexScheme;
			_TextureIndexScheme = NULL;
			_Tex = NULL;
			_TexGroup = NULL;
		}
	}

	bool useAnimatedTexture;
	if (!f.isReading())
	{
		useAnimatedTexture = (_TexGroup != NULL);
	}
	f.serial(useAnimatedTexture);
	if (useAnimatedTexture)
	{
		if (f.isReading())
		{
			CTextureGrouped *ptTex = NULL;
			f.serialPolyPtr(ptTex);
			_TexGroup = ptTex;
		}
		else
		{
			CTextureGrouped *ptTex = _TexGroup;
			f.serialPolyPtr(ptTex);
		}

		bool useTextureIndexScheme = _TextureIndexScheme != NULL;
		f.serial(useTextureIndexScheme);
		if (useTextureIndexScheme)
		{
			f.serialPolyPtr(_TextureIndexScheme);
			_TextureIndex = 0;
		}
		else
		{
			f.serial(_TextureIndex);
		}
	}
	else
	{
		if (f.isReading())
		{
			ITexture *ptTex = NULL;
			f.serialPolyPtr(ptTex);
			_Tex = ptTex;
		}
		else
		{
			ITexture *ptTex = _Tex;
			f.serialPolyPtr(ptTex);
		}
	}
}

////////////////////////////////////////////////////////
//       CPSRotated3DPlaneParticle  implementation    //
////////////////////////////////////////////////////////

///===================================================================================
void CPSRotated3DPlaneParticle::setPlaneBasisScheme(CPSAttribMaker<CPlaneBasis> *basisMaker)
{
	NL_PS_FUNC(CPSRotated3DPlaneParticle_setPlaneBasisScheme)
	nlassert(basisMaker);
	delete _PlaneBasisScheme;
	_PlaneBasisScheme = basisMaker;
	if (getPlaneBasisOwner() && basisMaker->hasMemory()) basisMaker->resize(getPlaneBasisOwner()->getMaxSize(), getPlaneBasisOwner()->getSize());
}

///===================================================================================
void CPSRotated3DPlaneParticle::setPlaneBasis(const CPlaneBasis &basis)
{
	NL_PS_FUNC(CPSRotated3DPlaneParticle_setPlaneBasis)
	delete _PlaneBasisScheme;
	_PlaneBasisScheme = NULL;
	_PlaneBasis = basis;
}

///===================================================================================
CPSRotated3DPlaneParticle::CPSRotated3DPlaneParticle() : _PlaneBasisScheme(NULL)
{
	NL_PS_FUNC(CPSRotated3DPlaneParticle_CPSRotated3DPlaneParticle)
	_PlaneBasis.X = CVector::I;
	_PlaneBasis.Y = CVector::J;
}

///===================================================================================
CPSRotated3DPlaneParticle::~CPSRotated3DPlaneParticle()
{
	NL_PS_FUNC(CPSRotated3DPlaneParticle_CPSRotated3DPlaneParticleDtor)
	delete _PlaneBasisScheme;
}

///===================================================================================
void CPSRotated3DPlaneParticle::serialPlaneBasisScheme(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSRotated3DPlaneParticle_serialPlaneBasisScheme)
	f.serialVersion(1);
	f.serialPolyPtr(_PlaneBasisScheme);
	bool usePlaneBasisScheme = _PlaneBasisScheme != NULL;
	if (!usePlaneBasisScheme)
	{
		f.serial(_PlaneBasis);
	}
}

////////////////////////////////
// CPSMaterial implementation //
////////////////////////////////

///===================================================================================
CPSMaterial::CPSMaterial()
{
	NL_PS_FUNC(CPSMaterial_CPSMaterial)
	_Mat.setBlend(true);
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one);
	_Mat.setZWrite(false);
}

///===================================================================================
void CPSMaterial::serialMaterial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSMaterial_IStream )
	// version 3 : added zbias
	sint ver = f.serialVersion(3);
	TBlendingMode m = getBlendingMode();
	f.serialEnum(m);
	setBlendingMode(m);
	if (ver == 2)
	{
		bool zTest = isZTestEnabled();
		f.serial(zTest);
		enableZTest(zTest);
	}
	else
	if (ver >= 3)
	{
		// bit 0 : ztest enabled
		// bit 1 : 1 if zbias is not 0, in this case zbias follows
		uint8 flags = (isZTestEnabled() ? 1 : 0) | (getZBias() != 0.f ? 2 : 0);
		f.serial(flags);
		enableZTest((flags & 1) != 0);
		if (flags & 2)
		{
			float zBias = getZBias();
			f.serial(zBias);
			setZBias(zBias);
		}
	}
}


///===================================================================================
void CPSMaterial::enableZTest(bool enabled)
{
	NL_PS_FUNC(CPSMaterial_enableZTest)
	_Mat.setZFunc(enabled ? CMaterial::less : CMaterial::always);
}

///===================================================================================
bool CPSMaterial::isZTestEnabled() const
{
	NL_PS_FUNC(CPSMaterial_isZTestEnabled)
	return _Mat.getZFunc() != CMaterial::always;
}

///===================================================================================
void CPSMaterial::setBlendingMode(CPSMaterial::TBlendingMode mode)
{
	NL_PS_FUNC(CPSMaterial_setBlendingMode)
	switch (mode)
	{
		case add:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::one, CMaterial::one);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case modulate:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::zero, CMaterial::srccolor);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case alphaBlend:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case alphaTest:
			_Mat.setBlend(false);
			_Mat.setZWrite(true);
			_Mat.setAlphaTest(true);
		break;
	}
}

///===================================================================================
CPSMaterial::TBlendingMode CPSMaterial::getBlendingMode(void) const
{
	NL_PS_FUNC(CPSMaterial_getBlendingMode)
	if (_Mat.getBlend())
	{
		CMaterial::TBlend srcBlend = _Mat.getSrcBlend();
		CMaterial::TBlend destBlend = _Mat.getDstBlend();

		if (srcBlend == CMaterial::one && destBlend == CMaterial::one) return add;
		if (srcBlend == CMaterial::zero && destBlend == CMaterial::srccolor) return modulate;
		if (srcBlend == CMaterial::srcalpha && destBlend == CMaterial::invsrcalpha) return alphaBlend;

		// unrecognized mode
		nlassert(0);
		return alphaTest; // to avoid a warning only ...
	}
	else
	{
		return alphaTest;
	}
}

///===================================================================================
void CPSMaterial::forceModulateConstantColor(bool force, const NLMISC::CRGBA &col)
{
	NL_PS_FUNC(CPSMaterial_forceModulateConstantColor)
	if (force)
	{
		/// TODO : caching..
		_Mat.texConstantColor(1, col);
		_Mat.texEnvOpRGB(1, CMaterial::Modulate);
		_Mat.texEnvOpAlpha(1, CMaterial::Modulate);
		_Mat.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
		_Mat.texEnvArg1RGB(1, CMaterial::Constant, CMaterial::SrcColor);
		_Mat.texEnvArg0Alpha(1, CMaterial::Previous, CMaterial::SrcAlpha);
		_Mat.texEnvArg1Alpha(1, CMaterial::Constant, CMaterial::SrcAlpha);
		forceTexturedMaterialStages(2);
	}
	else
	{
		if (_Mat.getTexture(1) != NULL)
		{
			_Mat.setTexture(1, NULL);
		}
	}
}


///===================================================================================
void CPSMaterial::forceTexturedMaterialStages(uint numStages)
{
	NL_PS_FUNC(CPSMaterial_forceTexturedMaterialStages)
	ITexture *blankTex = NULL;
	uint k;
	for (k = 0; k < numStages; ++k)
	{
		if (_Mat.getTexture(k) == NULL)
		{
			if (!blankTex)
			{
				blankTex = CTextureMem::Create1x1WhiteTex();
			}
			_Mat.setTexture(k, blankTex);
		}
	}
	for (; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		if (_Mat.getTexture(k) != NULL)
		{
			_Mat.setTexture(k, NULL);
		}
	}
}


/////////////////////////////////////////////
// CPSMultiTexturedParticle implementation //
/////////////////////////////////////////////

//=======================================
bool CPSMultiTexturedParticle::_ForceBasicCaps  = false;

//=======================================
CPSMultiTexturedParticle::CPSMultiTexturedParticle() : _MainOp(Add), _AlternateOp(Add), _MultiTexState(TouchFlag), _BumpFactor(1.f)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_CPSMultiTexturedParticle)
}

//=======================================
void	CPSMultiTexturedParticle::enableMultiTexture(bool enabled /*= true*/)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_enableMultiTexture)
	if (isMultiTextureEnabled() == enabled) return;
	if (!enabled)
	{
		_Texture2		   = NULL;
		_AlternateTexture2 = NULL;
		_MultiTexState = 0;
	}
	else
	{
		_MainOp = Modulate;
		_TexScroll[0].set(0, 0);
		_TexScroll[1].set(0, 0);
		_MultiTexState = (uint8) MultiTextureEnabled;
	}
	touch();
}

//=======================================
void	CPSMultiTexturedParticle::enableAlternateTex(bool enabled /*= true*/)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_enableAlternateTex)
	nlassert(isMultiTextureEnabled()); // multitexturing must have been enabled first
	if (enabled == isAlternateTexEnabled()) return;

	if (enabled)
	{
		_TexScrollAlternate[0].set(0, 0);
		_TexScrollAlternate[1].set(0, 0);
		_AlternateOp = Modulate;
		_MultiTexState |= (uint8) AlternateTextureEnabled;
	}
	else
	{
		_Texture2 = NULL;
		_MultiTexState &= ~(uint8) AlternateTextureEnabled;
	}
	touch();
}

//=======================================
void	CPSMultiTexturedParticle::serialMultiTex(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_serialMultiTex)
	/// version 1 : bump factor
	sint ver = f.serialVersion(1);
	f.serial(_MultiTexState);
	if (isMultiTextureEnabled())
	{
		f.serialEnum(_MainOp);
		if (_MainOp == EnvBumpMap && ver >= 1)
		{
			f.serial(_BumpFactor);
		}
		ITexture *tex = NULL;
		if (f.isReading())
		{
			f.serialPolyPtr(tex);
			_Texture2 = tex;
		}
		else
		{
			tex = _Texture2;
			f.serialPolyPtr(tex);
		}
		f.serial(_TexScroll[0], _TexScroll[1]);

		if (isAlternateTexEnabled())
		{
			f.serialEnum(_AlternateOp);
			if (f.isReading())
			{
				f.serialPolyPtr(tex);
				_AlternateTexture2 = tex;
			}
			else
			{
				tex = _AlternateTexture2;
				f.serialPolyPtr(tex);
			}
			f.serial(_TexScrollAlternate[0], _TexScrollAlternate[1]);
		}
		else
		{
			_AlternateTexture2 = NULL;
		}
	}
	else
	{
		if (f.isReading())
		{
			_Texture2		   = NULL;
			_AlternateTexture2 = NULL;
		}
	}
}

//=======================================
void CPSMultiTexturedParticle::setupMaterial(ITexture *primary, IDriver *driver, CMaterial &mat, CVertexBuffer &vb)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_setupMaterial)
	/// if bump is used, the matrix must be setupped each time (not a material field)
	if (!_ForceBasicCaps && isMultiTextureEnabled() && _MainOp  == EnvBumpMap)
	{
		if (driver->supportTextureAddrMode(CMaterial::OffsetTexture))
		{
			CTextureBump *tb = dynamic_cast<CTextureBump *>((ITexture *) _Texture2);
			if (tb != NULL)
			{
				float normFactor = tb->getNormalizationFactor();
				if (normFactor == 0.f) // have we computed the normalization factor ?
				{
					/// todo : caching of this value
					tb->generate();
					normFactor = tb->getNormalizationFactor();
					tb->release();
				}
				const float bMat[] = { _BumpFactor * normFactor, 0.f, 0.f, _BumpFactor * normFactor};
				driver->setMatrix2DForTextureOffsetAddrMode(1, bMat);
			}
			vb.setUVRouting(0, 1);
			vb.setUVRouting(1, 0);
		}
		else if (driver->supportEMBM())
		{
			uint embmTex = 0;
			const uint numTexStages = driver->getNbTextureStages();
			for(uint k = 0; k < numTexStages; ++k)
			{
				vb.setUVRouting(k, 0);
				if (driver->isEMBMSupportedAtStage(k))
				{
					vb.setUVRouting(k, 1);
					if (k != (numTexStages - 1))
					{
						vb.setUVRouting(k + 1, 0);
					}
					embmTex = k;
					break;
				}
			}
			CTextureBump *tb = dynamic_cast<CTextureBump *>((ITexture *) _Texture2);
			if (tb != NULL)
			{
				float normFactor = tb->getNormalizationFactor();
				if (normFactor == 0.f) // have we computed the normalization factor ?
				{
					/// todo : caching of this value
					tb->generate();
					normFactor = tb->getNormalizationFactor();
					tb->release();
				}
				const float bMat[] = { _BumpFactor * normFactor, 0.f, 0.f, _BumpFactor * normFactor};
				driver->setEMBMMatrix(embmTex, bMat);
			}
		}
	}

	if (!isTouched() && areBasicCapsForcedLocal() == areBasicCapsForced()) return;
	forceBasicCapsLocal(areBasicCapsForced());
	if (!isMultiTextureEnabled())
	{
		mat.setTexture(0, primary);
		mat.texEnvOpRGB(0, CMaterial::Modulate);
		mat.setTexture(1, NULL);
	}
	else
	{
		if (_MainOp  != EnvBumpMap)
		{
			setupMultiTexEnv(_MainOp, primary, _Texture2, mat, *driver);
			_MultiTexState &= ~(uint8) EnvBumpMapUsed;
			_MultiTexState &= ~(uint8) AlternateTextureUsed;
		}
		else
		{
			if (!_ForceBasicCaps && (driver->supportTextureAddrMode(CMaterial::OffsetTexture) || driver->supportEMBM())) // envbumpmap supported ?
			{
				CTextureBump *tb = dynamic_cast<CTextureBump *>((ITexture *) _Texture2);
				if (tb != NULL)
				{
					float normFactor = tb->getNormalizationFactor();
					if (normFactor == 0.f) // have we computed the normalization factor ?
					{
						/// todo : caching of this value
						tb->generate();
						normFactor = tb->getNormalizationFactor();
						tb->release();
					}
					setupMultiTexEnv(_MainOp, primary, _Texture2, mat, *driver);
				}
				_MultiTexState &= ~(uint8) AlternateTextureUsed;
				_MultiTexState |= (uint8) EnvBumpMapUsed;
			}
			else // switch to alternate
			{
				_MultiTexState |= (uint8) AlternateTextureUsed;
				if (isAlternateTexEnabled() && _AlternateTexture2)
				{
					setupMultiTexEnv(_AlternateOp, primary, _AlternateTexture2, mat, *driver);
				}
				else
				{
					mat.setTexture(0, primary);
					mat.texEnvOpRGB(0, CMaterial::Modulate);
				}
				_MultiTexState &= ~(uint8) EnvBumpMapUsed;
			}
		}
	}
	updateTexWrapMode(*driver);
	unTouch();
}

//=======================================
void	CPSMultiTexturedParticle::setupMultiTexEnv(TOperator op, ITexture *tex1, ITexture *tex2, CMaterial &mat, IDriver &drv)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_setupMultiTexEnv)
	switch (op)
	{
		case Add:
			mat.setTexture(0, tex1);
			mat.setTexture(1, tex2);
			mat.texEnvOpRGB(0, CMaterial::Modulate);
			mat.texEnvOpRGB(1, CMaterial::Add);
			mat.enableTexAddrMode(false);
			break;
		case Modulate:
			mat.setTexture(0, tex1);
			mat.setTexture(1, tex2);
			mat.texEnvOpRGB(0, CMaterial::Modulate);
			mat.texEnvOpRGB(1, CMaterial::Modulate);
			mat.enableTexAddrMode(false);
			break;
		case EnvBumpMap:
			if (drv.supportTextureAddrMode(CMaterial::OffsetTexture))
			{
				mat.setTexture(0, tex2);
				mat.setTexture(1, tex1);
				mat.texEnvOpRGB(0, CMaterial::Replace);
				mat.texEnvOpRGB(1, CMaterial::Modulate);
				mat.enableTexAddrMode(true);
				mat.setTexAddressingMode(0, CMaterial::FetchTexture);
				mat.setTexAddressingMode(1, CMaterial::OffsetTexture);
			}
			else if (drv.supportEMBM())
			{
				const uint numTexStages = drv.getNbTextureStages();
				// if embm unit is at last stage, it operates on texture at previous stage
				// otherwise it operates on texture at next stage
				for(uint k = 0; k < numTexStages; ++k)
				{
					if (drv.isEMBMSupportedAtStage(k))
					{
						if (k == (numTexStages - 1))
						{
							mat.setTexture(k, tex2);
							mat.texEnvOpRGB(k, CMaterial::EMBM);
							mat.texEnvOpAlpha(k, CMaterial::EMBM);
							mat.setTexture(0, tex1);
							mat.texEnvOpRGB(0, CMaterial::Modulate);
							mat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
							mat.texEnvArg1RGB(0, CMaterial::Diffuse, CMaterial::SrcColor);
							mat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);
							mat.texEnvArg1Alpha(0, CMaterial::Diffuse, CMaterial::SrcAlpha);
						}
						else
						{
							// stage before the bump map must not be empty so fill them
							for(uint l = 0; l < k; ++l)
							{
								mat.setTexture(l, tex1);
								mat.texEnvOpRGB(l, CMaterial::Modulate);
							}
							mat.setTexture(k, tex2);
							mat.texEnvOpRGB(k, CMaterial::EMBM);
							mat.setTexture(k + 1, tex1);
							mat.texEnvOpRGB(k + 1, CMaterial::Modulate);
							mat.texEnvArg0RGB(k + 1, CMaterial::Texture, CMaterial::SrcColor);
							mat.texEnvArg1RGB(k + 1, CMaterial::Diffuse, CMaterial::SrcColor);
							mat.texEnvArg0Alpha(k + 1, CMaterial::Texture, CMaterial::SrcAlpha);
							mat.texEnvArg1Alpha(k + 1, CMaterial::Diffuse, CMaterial::SrcAlpha);
						}
						break;
					}
				}
			}
		break;
		case Decal:
			mat.setTexture(0, tex1);
			mat.texEnvOpRGB(0, CMaterial::Replace);
			mat.setTexture(1, NULL);
			mat.enableTexAddrMode(false);
			break;
		default: break;
	}
}

//=====static func to convert a texture to a bumpmap
static void ConvertToBumpMap(NLMISC::CSmartPtr<ITexture> &ptr)
{
	NL_PS_FUNC(ConvertToBumpMap)
	if (!dynamic_cast<CTextureBump *>( (ITexture *) ptr))
	{
		// convert to a bumpmap
		CTextureBump *tb = new CTextureBump;
		tb->setHeightMap((ITexture *) ptr);
		tb->enableSharing(true);
		ptr = tb;
	}
}

//=====static func to convert a bumpmap to a texture (its heightmap)
static void ConvertFromBumpMap(NLMISC::CSmartPtr<ITexture> &ptr)
{
	NL_PS_FUNC(ConvertFromBumpMap)
	CTextureBump *bm = dynamic_cast<CTextureBump *>( (ITexture *) ptr);
	if (bm)
	{
		// retrieve the heightmap from the bumpmap
		NLMISC::CSmartPtr<ITexture> hm = bm->getHeightMap();
		ptr = hm;
	}
}

//=========================================
void	CPSMultiTexturedParticle::setTexture2Alternate(ITexture *tex)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_setTexture2Alternate)
	_AlternateTexture2 = tex;
	if (_AlternateOp != EnvBumpMap)
	{
		ConvertFromBumpMap(_AlternateTexture2);
	}
	else
	{
		ConvertToBumpMap(_AlternateTexture2);
	}
	touch();
}

//==========================================
void	CPSMultiTexturedParticle::setTexture2(ITexture *tex)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_setTexture2)
	_Texture2 = tex;
	if (_MainOp != EnvBumpMap)
	{
		ConvertFromBumpMap(_Texture2);
	}
	else
	{
		if (!dynamic_cast<NL3D::CTextureBump *>((ITexture *) _Texture2))
		{
			ConvertToBumpMap(_Texture2);
		}
	}
	touch();
}

//==========================================
void	CPSMultiTexturedParticle::setMainTexOp(TOperator op)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_setMainTexOp)
	_MainOp = op;
	if (_MainOp == EnvBumpMap)
	{
		ConvertToBumpMap(_Texture2);
	}
	else
	{
		ConvertFromBumpMap(_Texture2);
	}
	touch();
}

//==========================================
void	CPSMultiTexturedParticle::setAlternateTexOp(TOperator op)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_setAlternateTexOp)
	_AlternateOp = op;
	if (_AlternateOp == EnvBumpMap)
	{
		ConvertToBumpMap(_AlternateTexture2);
	}
	else
	{
		ConvertFromBumpMap(_AlternateTexture2);
	}
	touch();
}



//==========================================
void	CPSMultiTexturedParticle::setUseLocalDate(bool use)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_setUseLocalDate)
	if (use) _MultiTexState |= ScrollUseLocalDate;
	else _MultiTexState &= ~ ScrollUseLocalDate;
}


//==========================================
void	CPSMultiTexturedParticle::setUseLocalDateAlt(bool use)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_setUseLocalDateAlt)
	if (use) _MultiTexState |= ScrollUseLocalDateAlternate;
	else _MultiTexState &= ~ ScrollUseLocalDateAlternate;
}

//==========================================
void CPSTexturedParticle::enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest)
{
	NL_PS_FUNC(CPSTexturedParticle_enumTexs)
	if (_Tex)
	{
		dest.push_back(_Tex);
	}
	if (getTextureGroup())
	{
		dest.push_back(getTextureGroup());
	}
}

//==========================================
void CPSMultiTexturedParticle::enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv)
{
	NL_PS_FUNC(CPSMultiTexturedParticle_enumTexs)
	if (_MainOp  == EnvBumpMap && !_ForceBasicCaps)
	{
		if (drv.supportTextureAddrMode(CMaterial::OffsetTexture) || drv.supportEMBM())
		{
			if (_Texture2) dest.push_back(_Texture2);
		}
		else
		{
			if (_AlternateTexture2) dest.push_back(_AlternateTexture2);
		}
		return;
	}
	if (_Texture2) dest.push_back(_Texture2);
}

// *****************************************************************************************************
bool CPSMultiTexturedParticle::isAlternateTextureUsed(IDriver &driver) const
{
	NL_PS_FUNC(CPSMultiTexturedParticle_isAlternateTextureUsed)
	if (!isTouched() && areBasicCapsForcedLocal() == areBasicCapsForced()) return (_MultiTexState & AlternateTextureUsed) != 0;
	if (_MainOp  != EnvBumpMap) return false;
	return _ForceBasicCaps || (!driver.supportTextureAddrMode(CMaterial::OffsetTexture) && !driver.supportEMBM());
}

} // NL3D
