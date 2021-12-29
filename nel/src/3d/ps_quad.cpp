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

#include "nel/3d/ps_quad.h"
#include "nel/3d/ps_macro.h"
#include "nel/3d/driver.h"
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/texture_grouped.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_iterator.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{



///////////////////////////
// constant definition   //
///////////////////////////


static const uint dotBufSize = 1024; // size used for point particles batching

////////////////////
// vertex buffers //
////////////////////

/// the various kind of vertex buffers we need for quads
CVertexBuffer CPSQuad::_VBPos;
CVertexBuffer CPSQuad::_VBPosCol;
CVertexBuffer CPSQuad::_VBPosTex1;
CVertexBuffer CPSQuad::_VBPosTex1Col;
CVertexBuffer CPSQuad::_VBPosTex1Anim;
CVertexBuffer CPSQuad::_VBPosTex1AnimCol;
//==========
CVertexBuffer CPSQuad::_VBPosTex1Tex2;
CVertexBuffer CPSQuad::_VBPosTex1ColTex2;
CVertexBuffer CPSQuad::_VBPosTex1AnimTex2;
CVertexBuffer CPSQuad::_VBPosTex1AnimColTex2;
//==========
CVertexBuffer CPSQuad::_VBPosTex1Tex2Anim;
CVertexBuffer CPSQuad::_VBPosTex1ColTex2Anim;
CVertexBuffer CPSQuad::_VBPosTex1AnimTex2Anim;
CVertexBuffer CPSQuad::_VBPosTex1AnimColTex2Anim;

/// tmp : ram
CVertexBuffer RAMVBPos;
CVertexBuffer RAMVBPosCol;
CVertexBuffer RAMVBPosTex1;
CVertexBuffer RAMVBPosTex1Col;
CVertexBuffer RAMVBPosTex1Anim;
CVertexBuffer RAMVBPosTex1AnimCol;
//==========
CVertexBuffer RAMVBPosTex1Tex2;
CVertexBuffer RAMVBPosTex1ColTex2;
CVertexBuffer RAMVBPosTex1AnimTex2;
CVertexBuffer RAMVBPosTex1AnimColTex2;
//==========
CVertexBuffer RAMVBPosTex1Tex2Anim;
CVertexBuffer RAMVBPosTex1ColTex2Anim;
CVertexBuffer RAMVBPosTex1AnimTex2Anim;
CVertexBuffer RAMVBPosTex1AnimColTex2Anim;




CVertexBuffer    * const CPSQuad::_VbTab[] =
{
  // tex1 only
  &_VBPos, &_VBPosCol, &_VBPosTex1,  &_VBPosTex1Col,
  NULL,   NULL,		 &_VBPosTex1Anim, &_VBPosTex1AnimCol,
  // tex1 & tex2
  NULL, NULL, &_VBPosTex1Tex2, &_VBPosTex1ColTex2,
  NULL,   NULL,		 &_VBPosTex1AnimTex2, &_VBPosTex1AnimColTex2,
  // tex2 & !tex1 (invalid)
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  // tex2 & !tex1 (invalid)
  // tex1 & tex2
  NULL, NULL, &_VBPosTex1Tex2Anim, &_VBPosTex1ColTex2Anim,
  NULL,   NULL,		 &_VBPosTex1AnimTex2Anim, &_VBPosTex1AnimColTex2Anim,
};

CVertexBuffer    * const RAMVbTab[] =
{
	// tex1 only
		&RAMVBPos, &RAMVBPosCol, &RAMVBPosTex1,  &RAMVBPosTex1Col,
		NULL,   NULL,		 &RAMVBPosTex1Anim, &RAMVBPosTex1AnimCol,
		// tex1 & tex2
		NULL, NULL, &RAMVBPosTex1Tex2, &RAMVBPosTex1ColTex2,
		NULL,   NULL,		 &RAMVBPosTex1AnimTex2, &RAMVBPosTex1AnimColTex2,
		// tex2 & !tex1 (invalid)
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		// tex2 & !tex1 (invalid)
		// tex1 & tex2
		NULL, NULL, &RAMVBPosTex1Tex2Anim, &RAMVBPosTex1ColTex2Anim,
		NULL,   NULL,		 &RAMVBPosTex1AnimTex2Anim, &RAMVBPosTex1AnimColTex2Anim,
};




//////////////////////////////////
// CPSQuad implementation       //
//////////////////////////////////


///==================================================================================
/// fill textures coordinates for a quad vb
static void SetupQuadVBTexCoords(CVertexBuffer &vb, uint texCoordSet)
{
	NL_PS_FUNC(SetupQuadVBTexCoords)
	nlassert(texCoordSet < 2);
	CVertexBufferReadWrite vba;
	vb.lock (vba);
	// the size used for buffer can't be higher than quad buf size
	// to have too large buffer will broke the cache
	for (uint32 k = 0; k < CPSQuad::quadBufSize; ++k)
	{

		vba.setTexCoord(k * 4, texCoordSet, CUV(0, 0));
		vba.setTexCoord(k * 4 + 1, texCoordSet, CUV(1, 0));
		vba.setTexCoord(k * 4 + 2, texCoordSet, CUV(1, 1));
		vba.setTexCoord(k * 4 + 3, texCoordSet, CUV(0, 1));
	}
}

///==================================================================================
/// this static method init vertex buffers
void CPSQuad::initVertexBuffers()
{
	NL_PS_FUNC(CPSQuad_initVertexBuffers)
	for (uint k = 0; k < 32; ++k)
	{
		CVertexBuffer *vb = _VbTab[k];
		if (vb) // valid vb ?
		{
			vb->setName("CPSQuad");
			vb->setPreferredMemory(CVertexBuffer::AGPVolatile, true);
			uint32 vf = CVertexBuffer::PositionFlag;
			/// setup vertex format
			if (k & (uint) VBCol) vf |= CVertexBuffer::PrimaryColorFlag;
			if (k & (uint) VBTex  || k & (uint) VBTexAnimated) vf |= CVertexBuffer::TexCoord0Flag;
			if (k & (uint) VBTex2 || k & (uint) VBTex2Animated) vf |= CVertexBuffer::TexCoord1Flag;
			vb->setVertexFormat(vf);
			vb->setNumVertices(quadBufSize << 2);

			if ((k & (uint) VBTex) && !(k & (uint) VBTexAnimated))
			{
				SetupQuadVBTexCoords(*vb, 0);
			}

			if ((k & (uint) VBTex2) && !(k & (uint) VBTex2Animated))
			{
				SetupQuadVBTexCoords(*vb, 1);
			}

		}
	}
	/*
	for (uint k = 0; k < 32; ++k)
	{
		CVertexBuffer *vb = RAMVbTab[k];
		if (vb) // valid vb ?
		{
			vb->setName("CPSQuadRAM");
			vb->setPreferredMemory(CVertexBuffer::RAMPreferred, false);
			uint32 vf = CVertexBuffer::PositionFlag;
			/// setup vertex format
			if (k & (uint) VBCol) vf |= CVertexBuffer::PrimaryColorFlag;
			if (k & (uint) VBTex  || k & (uint) VBTexAnimated) vf |= CVertexBuffer::TexCoord0Flag;
			if (k & (uint) VBTex2 || k & (uint) VBTex2Animated) vf |= CVertexBuffer::TexCoord1Flag;
			vb->setVertexFormat(vf);
			vb->setNumVertices(quadBufSize << 2);

			if ((k & (uint) VBTex) && !(k & (uint) VBTexAnimated))
			{
				SetupQuadVBTexCoords(*vb, 0);
			}

			if ((k & (uint) VBTex2) && !(k & (uint) VBTex2Animated))
			{
				SetupQuadVBTexCoords(*vb, 1);
			}

		}
	}
	*/
}

// tmp
//volatile bool TestWantAGPCPSQuad = true;

///==================================================================================
/// choose the vertex buffex that we need
CVertexBuffer &CPSQuad::getNeededVB(IDriver &drv)
{
	NL_PS_FUNC(CPSQuad_getNeededVB)
	uint flags = 0;
	if (_ColorScheme) flags |= (uint) VBCol;
	if (_TexGroup)
	{
		flags |= VBTex | VBTexAnimated;
	}
	else if (_Tex)
	{
		flags |= VBTex;
	}

	if (flags & VBTex)
	{
		/// check if multitexturing is enabled, and which texture are enabled and / or animated
		if (CPSMultiTexturedParticle::isMultiTextureEnabled())
		{
			if (isAlternateTextureUsed(drv) && _AlternateTexture2)
			{
				if ((flags & VBTex) && (_TexScrollAlternate[0].x != 0 || _TexScrollAlternate[0].y	!= 0)) flags |= VBTexAnimated;
				if (_AlternateOp != Decal && (_TexScrollAlternate[1].x != 0 || _TexScrollAlternate[1].y != 0))
				{
					flags |= VBTex2 | VBTex2Animated;
				}
				else
				{
					flags |= VBTex2;
				}
			}
			else
			{
				if ((flags & VBTex) && (_TexScroll[0].x != 0 || _TexScroll[0].y	!= 0)) flags |= VBTexAnimated;
				if (_Texture2)
				{
					if (_MainOp != Decal && (_TexScroll[1].x != 0 || _TexScroll[1].y != 0))
					{
						flags |= VBTex2 | VBTex2Animated;
					}
					else
					{
						flags |= VBTex2;
					}
				}
			}
		}
	}
	/*if (TestWantAGPCPSQuad)
	{*/
		nlassert((flags & ~VBFullMask) == 0); // check for overflow
		nlassert(_VbTab[flags] != NULL);
		return *(_VbTab[flags]); // get the vb
	/*}
	else
	{
		nlassert((flags & ~VBFullMask) == 0); // check for overflow
		nlassert(RAMVbTab[flags] != NULL);
		return *(RAMVbTab[flags]); // get the vb
	}*/
}

///==================================================================================
CPSQuad::CPSQuad(CSmartPtr<ITexture> tex)
{
	NL_PS_FUNC(CPSQuad_CPSQuad)
	setTexture(tex);
	init();
	// we don't init the _IndexBuffer for now, as it will be when resize is called
	if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("quad");
}


///==================================================================================
CPSQuad::~CPSQuad()
{
	NL_PS_FUNC(CPSQuad_CPSQuadDtor)
}

///==================================================================================
uint32 CPSQuad::getNumWantedTris() const
{
	NL_PS_FUNC(CPSQuad_getNumWantedTris)
	nlassert(_Owner);
	//return _Owner->getMaxSize() << 1;
	return _Owner->getSize() << 1;
}

///==================================================================================
bool CPSQuad::hasTransparentFaces(void)
{
	NL_PS_FUNC(CPSQuad_hasTransparentFaces)
	return getBlendingMode() != CPSMaterial::alphaTest ;
}

///==================================================================================
bool CPSQuad::hasOpaqueFaces(void)
{
	NL_PS_FUNC(CPSQuad_hasOpaqueFaces)
	return !hasTransparentFaces();
}

///==================================================================================
void CPSQuad::init(void)
{
	NL_PS_FUNC(CPSQuad_init)
	_Mat.setLighting(false);
	_Mat.setDoubleSided(true);


	updateMatAndVbForColor();
	updateMatAndVbForTexture();
}

///==================================================================================
void CPSQuad::updateMatAndVbForTexture(void)
{
	NL_PS_FUNC(CPSQuad_updateMatAndVbForTexture)
	if (CPSMultiTexturedParticle::isMultiTextureEnabled())
	{
		touch();
	}
	else
	{
		_Mat.setTexture(0, _TexGroup ? (ITexture *) _TexGroup : (ITexture *) _Tex);
	}
}

///==================================================================================
bool CPSQuad::completeBBox(NLMISC::CAABBox &box) const
{
	NL_PS_FUNC(CPSQuad_completeBBox)
	if (!_SizeScheme)
	{
		CPSUtil::addRadiusToAABBox(box, _ParticleSize);
	}
	else
	{
		CPSUtil::addRadiusToAABBox(box, std::max(fabsf(_SizeScheme->getMaxValue()), fabsf(_SizeScheme->getMinValue())));
	}
	return true ;
}

///==================================================================================
void CPSQuad::resize(uint32 size)
{
	NL_PS_FUNC(CPSQuad_resize)
	nlassert(size < (1 << 16));
	resizeSize(size);
	resizeColor(size);
	resizeTextureIndex(size);
}

//==============================================================
void CPSQuad::updateMatAndVbForColor(void)
{
	NL_PS_FUNC(CPSQuad_updateMatAndVbForColor)
	// no vb to setup, now..
/*	if (!_ColorScheme)
	{
		_Mat.setColor(_Color);
	}
	else
	{
		_Mat.setColor(CRGBA::White);
	}	*/
}

//==============================================================
void CPSQuad::serial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSQuad_serial)
	sint ver = f.serialVersion(2);
	CPSParticle::serial(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSColoredParticle::serialColorScheme(f);
	CPSTexturedParticle::serialTextureScheme(f);
	serialMaterial(f);
	if (ver > 1)
	{
		CPSMultiTexturedParticle::serialMultiTex(f);
	}
}

//==============================================================
// static func used to fill texture coordinates of quads, with global time
static void FillQuadCoords(uint8 *dest, uint stride, const NLMISC::CVector2f &speed, float time, uint num)
{
	NL_PS_FUNC(FillQuadCoords)
	if (!num) return;
	const float topV    = speed.y * time;
	const float bottomV = topV + 1.f;
	const float leftU   = speed.x * time;
	const float rightU  = leftU + 1.f;


	do
	{
		((NLMISC::CUV *) dest)->set(leftU, topV);
		((NLMISC::CUV *) (dest + stride))->set(rightU, topV);
		((NLMISC::CUV *) (dest + (stride << 1)))->set(rightU, bottomV);
		((NLMISC::CUV *) (dest + (stride * 3)))->set(leftU, bottomV);


		dest += stride << 2;
	}
	while (--num);
}

//==============================================================
// static func used to fill texture coordinates of quads, with local time
static void FillQuadCoordsLocalTime(uint8 *dest, uint stride, const NLMISC::CVector2f &speed, CPSLocated &srcLoc, uint startIndex, uint num, uint32 srcStep)
{
	NL_PS_FUNC(FillQuadCoordsLocalTime)
	if (!num) return;

	if (srcStep == (1 << 16)) // step = 1.0 ?
	{
		TPSAttribTime::iterator timePt = srcLoc.getTime().begin() + startIndex;
		do
		{
			const float topV    = speed.y * *timePt;
			const float bottomV = topV + 1.f;
			const float leftU   = speed.x * *timePt;
			const float rightU  = leftU + 1.f;

			((NLMISC::CUV *) dest)->set(leftU, topV);
			((NLMISC::CUV *) (dest + stride))->set(rightU, topV);
			((NLMISC::CUV *) (dest + (stride << 1)))->set(rightU, bottomV);
			((NLMISC::CUV *) (dest + (stride * 3)))->set(leftU, bottomV);

			dest += stride << 2;
			++timePt;
		}
		while (--num);
	}
	else
	{
		TIteratorTimeStep1616 timePt(srcLoc.getTime().begin(), startIndex, srcStep);
		do
		{
			const float topV    = speed.y * *timePt;
			const float bottomV = topV + 1.f;
			const float leftU   = speed.x * *timePt;
			const float rightU  = leftU + 1.f;

			((NLMISC::CUV *) dest)->set(leftU, topV);
			((NLMISC::CUV *) (dest + stride))->set(rightU, topV);
			((NLMISC::CUV *) (dest + (stride << 1)))->set(rightU, bottomV);
			((NLMISC::CUV *) (dest + (stride * 3)))->set(leftU, bottomV);

			dest += stride << 2;
			++timePt;
		}
		while (--num);
	}
}

//==============================================================
void CPSQuad::updateVbColNUVForRender(CVertexBuffer &vb, uint32 startIndex, uint32 size, uint32 srcStep, IDriver &drv)
{
	NL_PS_FUNC(CPSQuad_updateVbColNUVForRender)
	nlassert(_Owner);

	if (!size) return;

	CVertexBufferReadWrite vba;
	vb.lock (vba);

	if (_ColorScheme)
	{
		// compute the colors, each color is replicated 4 times
		// todo hulud d3d vertex color RGBA / BGRA
		_ColorScheme->make4(_Owner, startIndex, vba.getColorPointer(), vb.getVertexSize(), size, srcStep);
	}


	if (_TexGroup) // if it has a constant texture we are sure it has been setupped before...
	{
		sint32 textureIndex[quadBufSize];
		const uint32 stride = vb.getVertexSize(), stride2 = stride << 1, stride3 = stride2 + stride, stride4 = stride2 << 1;
		uint8 *currUV = (uint8 *) vba.getTexCoordPointer();


		const sint32 *currIndex;
		uint32 currIndexIncr;

		if (_TextureIndexScheme)
		{
			currIndex = (sint32 *) _TextureIndexScheme->make(_Owner, startIndex, textureIndex, sizeof(sint32), size, true, srcStep);
			currIndexIncr = 1;
		}
		else
		{
			currIndex = &_TextureIndex;
			currIndexIncr  = 0;
		}

		uint32 left = size;
		while (left--)
		{
			// for now, we don't make texture index wrapping
			const CTextureGrouped::TFourUV &uvGroup = _TexGroup->getUVQuad((uint32) *currIndex);

			// copy the 4 uv's for this face
			*(CUV *) currUV = uvGroup.uv0;
			*(CUV *) (currUV + stride) = uvGroup.uv1;
			*(CUV *) (currUV + stride2) = uvGroup.uv2;
			*(CUV *) (currUV + stride3) = uvGroup.uv3;

			// point the next face
			currUV += stride4;
			currIndex += currIndexIncr;
		}
	}

	nlassert(_Owner && _Owner->getOwner());
	const float date= (float) _Owner->getOwner()->getSystemDate();


	/// todo: vertex program optimisation (& choose the vb accordingly)

	if (CPSMultiTexturedParticle::isMultiTextureEnabled() && (_Tex || _TexGroup))
	{

		// perform tex1 animation if needed
		if (!_TexGroup) // doesn't work with texGroup enabled
		{
			if (!isAlternateTextureUsed(drv) || !isAlternateTexEnabled() || !_AlternateTexture2)
			{
				if (_Tex && (_TexScroll[0].x != 0 || _TexScroll[0].y != 0))
				{
					// animation of texture 1 with main speed
					if (!getUseLocalDate())
					{
						FillQuadCoords((uint8 *) vba.getTexCoordPointer(0, 0), vb.getVertexSize(), _TexScroll[0], date, size);
					}
					else
					{
						FillQuadCoordsLocalTime((uint8 *) vba.getTexCoordPointer(0, 0), vb.getVertexSize(), _TexScroll[0], *_Owner, startIndex, size, srcStep);
					}
				}
			}
			else
			{
				if (_Tex && (_TexScrollAlternate[0].x != 0 || _TexScrollAlternate[0].y != 0))
				{
					// animation of texture 1 with alternate speed
					if (!getUseLocalDateAlt())
					{
						FillQuadCoords((uint8 *) vba.getTexCoordPointer(0, 0), vb.getVertexSize(), _TexScrollAlternate[0], date, size);
					}
					else
					{
						FillQuadCoordsLocalTime((uint8 *) vba.getTexCoordPointer(0, 0), vb.getVertexSize(), _TexScrollAlternate[0], *_Owner, startIndex, size, srcStep);
					}
				}
			}
		}

		// perform tex2 animation if needed
		if (!isAlternateTextureUsed(drv))
		{
			if (_Texture2 && (_TexScroll[1].x != 0 || _TexScroll[1].y != 0))
			{
				// animation of texture 2 with main speed
				if (!getUseLocalDate())
				{
					FillQuadCoords((uint8 *) vba.getTexCoordPointer(0, 1), vb.getVertexSize(), _TexScroll[1], date, size);
				}
				else
				{
					FillQuadCoordsLocalTime((uint8 *) vba.getTexCoordPointer(0, 1), vb.getVertexSize(), _TexScroll[1], *_Owner, startIndex, size, srcStep);
				}
			}
		}
		else
		{
			if (_AlternateTexture2 && isAlternateTexEnabled() && (_TexScrollAlternate[1].x != 0 || _TexScrollAlternate[1].y != 0))
			{
				// animation of texture 2 with alternate speed
				if (!getUseLocalDateAlt())
				{
					FillQuadCoords((uint8 *) vba.getTexCoordPointer(0, 1), vb.getVertexSize(), _TexScrollAlternate[1], date, size);
				}
				else
				{
					FillQuadCoordsLocalTime((uint8 *) vba.getTexCoordPointer(0, 1), vb.getVertexSize(), _TexScrollAlternate[1], *_Owner, startIndex, size, srcStep);
				}
			}
		}

	}
}


///==================================================================================
void CPSQuad::updateMatBeforeRendering(IDriver *drv, CVertexBuffer &vb)
{
	NL_PS_FUNC(CPSQuad_updateMatBeforeRendering)
	nlassert(_Owner && _Owner->getOwner());
	CParticleSystem &ps = *(_Owner->getOwner());
	vb.setUVRouting(0, 0);
	vb.setUVRouting(1, 1); // default uv routing
	if (isMultiTextureEnabled())
	{
		setupMaterial(_Tex, drv, _Mat, vb);
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{
			_Mat.setColor(ps.getGlobalColorLighted());
		}
		else
		{
			_Mat.setColor(ps.getGlobalColor());
		}
	}
	else
	{
		/// update the material if the global color of the system is variable
		if (_ColorScheme != NULL &&
			(ps.getColorAttenuationScheme() != NULL ||
			 ps.isUserColorUsed() ||
			 ps.getForceGlobalColorLightingFlag() ||
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
			NLMISC::CRGBA col;
			if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
			{
				col.modulateFromColor(ps.getGlobalColorLighted(), _Color);
			}
			else if (ps.getColorAttenuationScheme() != NULL || ps.isUserColorUsed())
			{
				col.modulateFromColor(ps.getGlobalColor(), _Color);
			}
			else
			{
				col = _Color;
			}
			_Mat.setColor(col);
		}
	}
}

// *****************************************************************************************************
void CPSQuad::enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv)
{
	NL_PS_FUNC(CPSQuad_enumTexs)
	CPSTexturedParticle::enumTexs(dest);
	CPSMultiTexturedParticle::enumTexs(dest, drv);
}

// *****************************************************************************************************
void CPSQuad::setZBias(float value)
{
	NL_PS_FUNC(CPSQuad_setZBias)
	CPSMaterial::setZBias(value);
}


// *****************************************************************************************************
void CPSQuad::setTexture(CSmartPtr<ITexture> tex)
{
	NL_PS_FUNC(CPSQuad_setTexture)
	CPSTexturedParticle::setTexture(tex);
	CPSMultiTexturedParticle::touch();
}

// *****************************************************************************************************
void CPSQuad::setTextureGroup(NLMISC::CSmartPtr<CTextureGrouped> texGroup)
{
	NL_PS_FUNC(CPSQuad_setTextureGroup)
	CPSTexturedParticle::setTextureGroup(texGroup);
	CPSMultiTexturedParticle::touch();
}

// *****************************************************************************************************
void CPSQuad::setTexture2(ITexture *tex)
{
	NL_PS_FUNC(CPSQuad_setTexture2)
	CPSMultiTexturedParticle::setTexture2(tex);
}

// *****************************************************************************************************
void CPSQuad::setTexture2Alternate(ITexture *tex)
{
	NL_PS_FUNC(CPSQuad_setTexture2Alternate)
	CPSMultiTexturedParticle::setTexture2Alternate(tex);
}

// *****************************************************************************************************
void CPSQuad::updateTexWrapMode(IDriver &drv)
{
	NL_PS_FUNC(CPSQuad_updateTexWrapMode)
	if (isMultiTextureEnabled())
	{
		if (_Tex)
		{
			if (isAlternateTextureUsed(drv))
			{
				_Tex->setWrapS(_TexScroll[0].x == 0 ? ITexture::Clamp : ITexture::Repeat);
				_Tex->setWrapT(_TexScroll[0].y == 0 ? ITexture::Clamp : ITexture::Repeat);
			}
			else
			{
				_Tex->setWrapS(_TexScrollAlternate[0].x == 0 ? ITexture::Clamp : ITexture::Repeat);
				_Tex->setWrapT(_TexScrollAlternate[0].y == 0 ? ITexture::Clamp : ITexture::Repeat);
			}
		}
		ITexture *tex2 = isAlternateTextureUsed(drv) ? getTexture2Alternate() : getTexture2();
		if (tex2)
		{
			tex2->setWrapS(_TexScroll[1].x == 0 ? ITexture::Clamp : ITexture::Repeat);
			tex2->setWrapT(_TexScroll[1].y == 0 ? ITexture::Clamp : ITexture::Repeat);
		}
	}
	else
	{
		if (_Tex)
		{
			_Tex->setWrapS(ITexture::Clamp);
			_Tex->setWrapT(ITexture::Clamp);
		}
		// nb for grouped texture we assume that no mipmapping is used, and that the border is black (or white for modulate material mode)
	}
}

} // NL3D
