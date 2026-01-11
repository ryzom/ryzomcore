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

#ifndef NL_DRIVER_MATERIAL_INLINE_H
#define NL_DRIVER_MATERIAL_INLINE_H

#include "nel/misc/debug.h"

namespace NL3D
{

// --------------------------------------------------

inline bool CMaterial::texturePresent(uint8 n) const
{
	nlassert(n<IDRV_MAT_MAXTEXTURES);
	if (_Textures[n])
	{
		return(true);
	}
	return(false);
}

inline ITexture*		CMaterial::getTexture(uint8 n) const
{
	nlassert(n<IDRV_MAT_MAXTEXTURES);
	return(_Textures[n]);
}

inline void CMaterial::setSrcBlend(TBlend val)
{
	_SrcBlend=val;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}

inline void CMaterial::setDstBlend(TBlend val)
{
	_DstBlend=val;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}

inline void CMaterial::setBlend(bool active)
{
	if (active)	_Flags|=IDRV_MAT_BLEND;
	else		_Flags&=~IDRV_MAT_BLEND;
	_Touched|=IDRV_TOUCHED_BLEND;
}

inline void CMaterial::setDoubleSided(bool active)
{
	if (active)	_Flags|=IDRV_MAT_DOUBLE_SIDED;
	else		_Flags&=~IDRV_MAT_DOUBLE_SIDED;
	_Touched|=IDRV_TOUCHED_DOUBLE_SIDED;
}

inline void CMaterial::setAlphaTest(bool active)
{
	if (active)	_Flags|=IDRV_MAT_ALPHA_TEST;
	else		_Flags&=~IDRV_MAT_ALPHA_TEST;
	_Touched|=IDRV_TOUCHED_ALPHA_TEST;
}

inline void CMaterial::setAlphaTestThreshold(float thre)
{
	_AlphaTestThreshold= thre;
	_Touched|=IDRV_TOUCHED_ALPHA_TEST_THRE;
}

inline void	CMaterial::setBlendFunc(TBlend src, TBlend dst)
{
	_SrcBlend=src;
	_DstBlend=dst;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}


inline void CMaterial::setZFunc(ZFunc val)
{
	_ZFunction=val;
	_Touched|=IDRV_TOUCHED_ZFUNC;
}

inline void	CMaterial::setZWrite(bool active)
{
	if (active)	_Flags|=IDRV_MAT_ZWRITE;
	else		_Flags&=~IDRV_MAT_ZWRITE;
	_Touched|=IDRV_TOUCHED_ZWRITE;
}

inline void CMaterial::setZBias(float val)
{
	_ZBias=val;
	_Touched|=IDRV_TOUCHED_ZBIAS;
}

inline void CMaterial::setColor(NLMISC::CRGBA rgba)
{
	if (_Color != rgba)
	{
		_Color=rgba;
		_Touched|=IDRV_TOUCHED_COLOR;
	}
}

inline void CMaterial::setLighting(	bool active,
									NLMISC::CRGBA emissive,
									NLMISC::CRGBA ambient,
									NLMISC::CRGBA diffuse,
									NLMISC::CRGBA specular,
									float	shininess)
{
	if (active)
	{
		_Flags|=IDRV_MAT_LIGHTING;
	}
	else
	{
		_Flags&=~IDRV_MAT_LIGHTING;
	}
	_Emissive=emissive;
	_Ambient=ambient;
	_Diffuse=diffuse;
	_Specular=specular;
	_Shininess= shininess;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}


// ***************************************************************************
inline void	CMaterial::setEmissive( CRGBA emissive)
{
	_Emissive= emissive;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setAmbient( CRGBA ambient)
{
	_Ambient= ambient;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setDiffuse( CRGBA diffuse)
{
	// Keep opacity.
	_Diffuse.R= diffuse.R;
	_Diffuse.G= diffuse.G;
	_Diffuse.B= diffuse.B;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setOpacity( uint8	opa )
{
	_Diffuse.A= opa;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setSpecular( CRGBA specular)
{
	_Specular= specular;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setShininess( float shininess )
{
	_Shininess= shininess;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setLightedVertexColor( bool useLightedVertexColor )
{
	if (useLightedVertexColor)
		_Flags |= IDRV_MAT_LIGHTED_VERTEX_COLOR;
	else
		_Flags &= ~IDRV_MAT_LIGHTED_VERTEX_COLOR;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}

// ***************************************************************************
inline bool		CMaterial::getLightedVertexColor () const
{
	return (_Flags & IDRV_MAT_LIGHTED_VERTEX_COLOR) != 0;
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
inline void					CMaterial::texEnvOpRGB(uint stage, TTexOperator ope)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.OpRGB= ope;
	_Touched|=IDRV_TOUCHED_TEXENV;
}

// ***************************************************************************
inline CMaterial::TTexOperator	    CMaterial::getTexEnvOpRGB(uint stage) const
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	return (TTexOperator) _TexEnvs[stage].Env.OpRGB;
}

// ***************************************************************************
inline void					CMaterial::texEnvArg0RGB(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.SrcArg0RGB= src;
	_TexEnvs[stage].Env.OpArg0RGB= oper;
	_Touched|=IDRV_TOUCHED_TEXENV;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg1RGB(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.SrcArg1RGB= src;
	_TexEnvs[stage].Env.OpArg1RGB= oper;
	_Touched|=IDRV_TOUCHED_TEXENV;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg2RGB(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.SrcArg2RGB= src;
	_TexEnvs[stage].Env.OpArg2RGB= oper;
}

// ***************************************************************************
inline void					CMaterial::texEnvOpAlpha(uint stage, TTexOperator ope)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.OpAlpha= ope;
	_Touched|=IDRV_TOUCHED_TEXENV;
}

// ***************************************************************************
inline CMaterial::TTexOperator	    CMaterial::getTexEnvOpAlpha(uint stage) const
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	return (TTexOperator) _TexEnvs[stage].Env.OpAlpha;
}

// ***************************************************************************
inline void					CMaterial::texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	nlassert(oper==SrcAlpha || oper==InvSrcAlpha);
	_TexEnvs[stage].Env.SrcArg0Alpha= src;
	_TexEnvs[stage].Env.OpArg0Alpha= oper;
	_Touched|=IDRV_TOUCHED_TEXENV;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	nlassert(oper==SrcAlpha || oper==InvSrcAlpha);
	_TexEnvs[stage].Env.SrcArg1Alpha= src;
	_TexEnvs[stage].Env.OpArg1Alpha= oper;
	_Touched|=IDRV_TOUCHED_TEXENV;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg2Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	nlassert(oper==SrcAlpha || oper==InvSrcAlpha);
	_TexEnvs[stage].Env.SrcArg2Alpha= src;
	_TexEnvs[stage].Env.OpArg2Alpha= oper;
}



// ***************************************************************************
inline void					CMaterial::texConstantColor(uint stage, CRGBA color)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].ConstantColor= color;
	_Touched|=IDRV_TOUCHED_TEXENV;
}


// ***************************************************************************
inline uint32				CMaterial::getTexEnvMode(uint stage)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	return _TexEnvs[stage].EnvPacked;
}
// ***************************************************************************
inline void					CMaterial::setTexEnvMode(uint stage, uint32 packed)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].EnvPacked= packed;
	_Touched|=IDRV_TOUCHED_TEXENV;
}
// ***************************************************************************
inline CRGBA				CMaterial::getTexConstantColor(uint stage)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	return _TexEnvs[stage].ConstantColor;
}
// ***************************************************************************
inline bool					CMaterial::getTexCoordGen(uint stage) const
{
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	return (_Flags&(IDRV_MAT_GEN_TEX_0<<stage))!=0;
}
// ***************************************************************************
inline void					CMaterial::setTexCoordGen(uint stage, bool generate)
{
	nlassert(stage<IDRV_MAT_MAXTEXTURES);
	if (generate)
		_Flags|=(IDRV_MAT_GEN_TEX_0<<stage);
	else
		_Flags&=~(IDRV_MAT_GEN_TEX_0<<stage);
	_Touched|=IDRV_TOUCHED_TEXGEN;
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
inline void					CMaterial::setUserColor(CRGBA userColor)
{
	nlassert(_ShaderType==CMaterial::UserColor);
	// setup stage 0 constant color (don't use texConstantColor() because of assert).
	_TexEnvs[0].ConstantColor= userColor;
	_Touched|=IDRV_TOUCHED_TEXENV;
}

// ***************************************************************************
inline CRGBA				CMaterial::getUserColor() const
{
	nlassert(_ShaderType==CMaterial::UserColor);
	// setup stage 0 constant color (don't use getTexConstantColor() because of assert).
	return _TexEnvs[0].ConstantColor;
}

// ***************************************************************************
inline void                 CMaterial::enableUserTexMat(uint stage, bool enabled /*= true*/)
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	if (enabled)
	{
		if (!(_Flags & IDRV_MAT_USER_TEX_MAT_ALL)) // not usr tex mat setupped before?
		{
			nlassert(_TexUserMat.get() == NULL);
			_TexUserMat.reset(new CUserTexMat);
		}
		_Flags |= (IDRV_MAT_USER_TEX_0_MAT << stage);
		_TexUserMat->TexMat[stage].identity();
	}
	else
	{
		if (!(_Flags & IDRV_MAT_USER_TEX_MAT_ALL)) return; // nothing to do
		_Flags &= ~(IDRV_MAT_USER_TEX_0_MAT << stage);     // clear the stage flag
		if (!(_Flags & IDRV_MAT_USER_TEX_MAT_ALL))		   // no more user textures used ?
		{
			_TexUserMat.reset();
		}
	}
}

// ***************************************************************************
inline bool               CMaterial::isUserTexMatEnabled(uint stage) const
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return (_Flags & (IDRV_MAT_USER_TEX_0_MAT << stage)) != 0;
}

// ***************************************************************************
inline void				  CMaterial::setUserTexMat(uint stage, const NLMISC::CMatrix &m)
{
	nlassert(isUserTexMatEnabled(stage));
	nlassert(_TexUserMat.get() != NULL);
	_TexUserMat->TexMat[stage] = m;
}

// ***************************************************************************
inline const NLMISC::CMatrix  &CMaterial::getUserTexMat(uint stage) const
{
	nlassert(isUserTexMatEnabled(stage));
	nlassert(_TexUserMat.get() != NULL);
	return _TexUserMat->TexMat[stage];
}

// ***************************************************************************
inline void	CMaterial::setTexCoordGenMode(uint stage, TTexCoordGenMode mode)
{
	if(stage>=IDRV_MAT_MAXTEXTURES)
		return;
	_TexCoordGenMode&= ~ (IDRV_MAT_TEX_GEN_MASK << (stage*IDRV_MAT_TEX_GEN_SHIFT));
	_TexCoordGenMode|=   ((mode&IDRV_MAT_TEX_GEN_MASK) << (stage*IDRV_MAT_TEX_GEN_SHIFT));
	_Touched|=IDRV_TOUCHED_TEXGEN;
}

}

#endif

