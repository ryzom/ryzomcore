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

#include "stdopengl.h"

#include "driver_opengl.h"

// ***************************************************************************
// define it For Debug purpose only. Normal use is to hide this line
//#define		NL3D_GLSTATE_DISABLE_CACHE

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

// ***************************************************************************
CDriverGLStates::CDriverGLStates()
{
	H_AUTO_OGL(CDriverGLStates_CDriverGLStates)
	_TextureCubeMapSupported= false;
	_CurrARBVertexBuffer = 0;
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;
	_MaxDriverLight= 0;
	_CullMode = CCW;
}


// ***************************************************************************
void			CDriverGLStates::init(bool supportTextureCubeMap, bool supportTextureRectangle, uint maxLight)
{
	H_AUTO_OGL(CDriverGLStates_init)
	_TextureCubeMapSupported= supportTextureCubeMap;
	_TextureRectangleSupported= supportTextureRectangle;
	_MaxDriverLight= maxLight;
	_MaxDriverLight= std::min(_MaxDriverLight, uint(MaxLight));

	// By default all arrays are disabled.
	_VertexArrayEnabled= false;
	_NormalArrayEnabled= false;
	_WeightArrayEnabled= false;
	_ColorArrayEnabled= false;
	_SecondaryColorArrayEnabled= false;
	uint	i;
	for(i=0; i<sizeof(_TexCoordArrayEnabled)/sizeof(_TexCoordArrayEnabled[0]); i++)
	{
		_TexCoordArrayEnabled[i]= false;
	}
	for(i=0; i<CVertexBuffer::NumValue; i++)
	{
		_VertexAttribArrayEnabled[i]= false;
	}
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;
	// by default all lights are disabled (not reseted in forceDefaults)
	for(i=0; i<MaxLight; i++)
	{
		_CurLight[i]= false;
	}
}


// ***************************************************************************
void			CDriverGLStates::forceDefaults(uint nbStages)
{
	H_AUTO_OGL(CDriverGLStates_forceDefaults);

	// Enable / disable.
	_CurFog= false;
	_CurBlend= false;
	_CurCullFace= true;
	_CurAlphaTest= false;
	_CurLighting= false;
	_CurZWrite= true;
	_CurStencilTest=false;
	_CurMultisample= false;

	// setup GLStates.
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
	glDisable(GL_MULTISAMPLE_ARB);

	// Func.
	_CurBlendSrc= GL_SRC_ALPHA;
	_CurBlendDst= GL_ONE_MINUS_SRC_ALPHA;
	_CurDepthFunc= GL_LEQUAL;
	_CurStencilFunc = GL_ALWAYS;
	_CurStencilRef = 0;
	_CurStencilMask = std::numeric_limits<GLuint>::max();
	_CurStencilOpFail = GL_KEEP;
	_CurStencilOpZFail = GL_KEEP;
	_CurStencilOpZPass = GL_KEEP;
	_CurStencilWriteMask = std::numeric_limits<GLuint>::max();
	_CurAlphaTestThreshold= 0.5f;

	// setup GLStates.
	glBlendFunc(_CurBlendSrc, _CurBlendDst);
	glDepthFunc(_CurDepthFunc);
	glStencilFunc(_CurStencilFunc, _CurStencilRef, _CurStencilMask);
	glStencilOp(_CurStencilOpFail, _CurStencilOpZFail, _CurStencilOpZPass);
	glStencilMask(_CurStencilWriteMask);
	glAlphaFunc(GL_GREATER, _CurAlphaTestThreshold);



	// Materials.
	uint32			packedOne= (CRGBA(255,255,255,255)).getPacked();
	uint32			packedZero= (CRGBA(0,0,0,255)).getPacked();
	_CurEmissive= packedZero;
	_CurAmbient= packedOne;
	_CurDiffuse= packedOne;
	_CurSpecular= packedZero;
	_CurShininess= 1;

	// Lighted vertex color
	_VertexColorLighted=false;
	glDisable(GL_COLOR_MATERIAL);

	// setup GLStates.
	static const GLfloat		one[4]= {1,1,1,1};
	static const GLfloat		zero[4]= {0,0,0,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, _CurShininess);

	// TexModes
	uint stage;
	for(stage=0;stage<nbStages; stage++)
	{
		// disable texturing.
#ifdef USE_OPENGLES
		glActiveTexture(GL_TEXTURE0+stage);
#else
		nglActiveTextureARB(GL_TEXTURE0_ARB+stage);
#endif

		glDisable(GL_TEXTURE_2D);

		if(_TextureCubeMapSupported)
		{
			glDisable(GL_TEXTURE_CUBE_MAP_ARB);
#ifdef USE_OPENGLES
			glDisable(GL_TEXTURE_GEN_STR_OES);
#endif
		}

		_TextureMode[stage]= TextureDisabled;

		// Tex gen init
		_TexGenMode[stage] = 0;

#ifndef USE_OPENGLES
		if(_TextureRectangleSupported)
			glDisable(GL_TEXTURE_RECTANGLE_NV);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);
#endif
	}

	// ActiveTexture current texture to 0.
#ifdef USE_OPENGLES
	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
#else
	nglActiveTextureARB(GL_TEXTURE0_ARB);
	nglClientActiveTextureARB(GL_TEXTURE0_ARB);
#endif

	_CurrentActiveTextureARB= 0;
	_CurrentClientActiveTextureARB= 0;

	// Depth range
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;

#ifdef USE_OPENGLES
	glDepthRangef (0.f, 1.f);
#else
	glDepthRange (0, 1);
#endif

	// Cull order
	_CullMode = CCW;
	glCullFace(GL_BACK);
}

// ***************************************************************************
void			CDriverGLStates::enableBlend(uint enable)
{
	H_AUTO_OGL(CDriverGLStates_enableBlend)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurBlend )
#endif
	{
		// new state.
		_CurBlend= enabled;
		// Setup GLState.
		if(_CurBlend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableCullFace(uint enable)
{
	H_AUTO_OGL(CDriverGLStates_enableCullFace)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurCullFace )
#endif
	{
		// new state.
		_CurCullFace= enabled;
		// Setup GLState.
		if(_CurCullFace)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableAlphaTest(uint enable)
{
	H_AUTO_OGL(CDriverGLStates_enableAlphaTest)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurAlphaTest )
#endif
	{
		// new state.
		_CurAlphaTest= enabled;

		// Setup GLState.
		if(_CurAlphaTest)
		{
			glEnable(GL_ALPHA_TEST);
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
		}
	}
}



// ***************************************************************************
void			CDriverGLStates::enableLighting(uint enable)
{
	H_AUTO_OGL(CDriverGLStates_enableLighting)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurLighting )
#endif
	{
		// new state.
		_CurLighting= enabled;
		// Setup GLState.
		if(_CurLighting)
			glEnable(GL_LIGHTING);
		else
		{
			glDisable(GL_LIGHTING);
		}
	}
}

// ***************************************************************************
void			CDriverGLStates::enableLight(uint num, uint enable)
{
	H_AUTO_OGL(CDriverGLStates_enableLight)
	if(num>=_MaxDriverLight)
		return;

	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurLight[num] )
#endif
	{
		// new state.
		_CurLight[num]= enabled;
		// Setup GLState.
		if(_CurLight[num])
			glEnable ((GLenum)(GL_LIGHT0+num));
		else
			glDisable ((GLenum)(GL_LIGHT0+num));
	}
}

// ***************************************************************************
bool			CDriverGLStates::isLightEnabled(uint num) const
{
	H_AUTO_OGL(CDriverGLStates_isLightEnabled)
	if(num>=_MaxDriverLight)
		return false;
	else
		return _CurLight[num];
}


// ***************************************************************************
void			CDriverGLStates::enableZWrite(uint enable)
{
	H_AUTO_OGL(CDriverGLStates_enableZWrite)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurZWrite )
#endif
	{
		// new state.
		_CurZWrite= enabled;
		// Setup GLState.
		if(_CurZWrite)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
}


// ***************************************************************************
void			CDriverGLStates::enableStencilTest(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableStencilTest);

	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enable != _CurStencilTest )
#endif
	{
		// new state.
		_CurStencilTest= enable;
		// Setup GLState.
		if(_CurStencilTest)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableMultisample(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableMultisample);

	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enable != _CurMultisample )
#endif
	{
		// new state.
		_CurMultisample= enable;

		// Setup GLState.
		if(_CurMultisample)
			glEnable(GL_MULTISAMPLE_ARB);
		else
			glDisable(GL_MULTISAMPLE_ARB);
	}
}

// ***************************************************************************
void			CDriverGLStates::blendFunc(GLenum src, GLenum dst)
{
	H_AUTO_OGL(CDriverGLStates_blendFunc)
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( src!= _CurBlendSrc || dst!=_CurBlendDst )
#endif
	{
		// new state.
		_CurBlendSrc= src;
		_CurBlendDst= dst;
		// Setup GLState.
		glBlendFunc(_CurBlendSrc, _CurBlendDst);
	}
}

// ***************************************************************************
void			CDriverGLStates::depthFunc(GLenum zcomp)
{
	H_AUTO_OGL(CDriverGLStates_depthFunc)
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( zcomp != _CurDepthFunc )
#endif
	{
		// new state.
		_CurDepthFunc= zcomp;
		// Setup GLState.
		glDepthFunc(_CurDepthFunc);
	}
}


// ***************************************************************************
void			CDriverGLStates::alphaFunc(float threshold)
{
	H_AUTO_OGL(CDriverGLStates_alphaFunc)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if(threshold != _CurAlphaTestThreshold)
#endif
	{
		// new state
		_CurAlphaTestThreshold= threshold;
		// setup function.
		glAlphaFunc(GL_GREATER, _CurAlphaTestThreshold);
	}
}


// ***************************************************************************
void			CDriverGLStates::stencilFunc(GLenum func, GLint ref, GLuint mask)
{
	H_AUTO_OGL(CDriverGLStates_stencilFunc)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if((func!=_CurStencilFunc) || (ref!=_CurStencilRef) || (mask!=_CurStencilMask))
#endif
	{
		// new state
		_CurStencilFunc = func;
		_CurStencilRef = ref;
		_CurStencilMask = mask;

		// setup function.
		glStencilFunc(_CurStencilFunc, _CurStencilRef, _CurStencilMask);
	}
}


// ***************************************************************************
void			CDriverGLStates::stencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	H_AUTO_OGL(CDriverGLStates_stencilOp)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if((fail!=_CurStencilOpFail) || (zfail!=_CurStencilOpZFail) || (zpass!=_CurStencilOpZPass))
#endif
	{
		// new state
		_CurStencilOpFail = fail;
		_CurStencilOpZFail = zfail;
		_CurStencilOpZPass = zpass;

		// setup function.
		glStencilOp(_CurStencilOpFail, _CurStencilOpZFail, _CurStencilOpZPass);
	}
}

// ***************************************************************************
void			CDriverGLStates::stencilMask(GLuint mask)
{
	H_AUTO_OGL(CDriverGLStates_stencilMask)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if(mask!=_CurStencilWriteMask)
#endif
	{
		// new state
		_CurStencilWriteMask = mask;

		// setup function.
		glStencilMask(_CurStencilWriteMask);
	}
}


// ***************************************************************************
void			CDriverGLStates::setEmissive(uint32 packedColor, const GLfloat color[4])
{
	H_AUTO_OGL(CDriverGLStates_setEmissive)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurEmissive )
#endif
	{
		_CurEmissive= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setAmbient(uint32 packedColor, const GLfloat color[4])
{
	H_AUTO_OGL(CDriverGLStates_setAmbient)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurAmbient )
#endif
	{
		_CurAmbient= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setDiffuse(uint32 packedColor, const GLfloat color[4])
{
	H_AUTO_OGL(CDriverGLStates_setDiffuse)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurDiffuse )
#endif
	{
		_CurDiffuse= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setSpecular(uint32 packedColor, const GLfloat color[4])
{
	H_AUTO_OGL(CDriverGLStates_setSpecular)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurSpecular )
#endif
	{
		_CurSpecular= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setShininess(float shin)
{
	H_AUTO_OGL(CDriverGLStates_setShininess)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( shin != _CurShininess )
#endif
	{
		_CurShininess= shin;
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shin);
	}
}


// ***************************************************************************
static void	convColor(CRGBA col, GLfloat glcol[4])
{
	H_AUTO_OGL(convColor)
	static	const float	OO255= 1.0f/255;
	glcol[0]= col.R*OO255;
	glcol[1]= col.G*OO255;
	glcol[2]= col.B*OO255;
	glcol[3]= col.A*OO255;
}

// ***************************************************************************
void			CDriverGLStates::setVertexColorLighted(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_setVertexColorLighted)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enable != _VertexColorLighted)
#endif
	{
		_VertexColorLighted= enable;

		if (_VertexColorLighted)
		{
			glEnable (GL_COLOR_MATERIAL);
#ifndef USE_OPENGLES
			glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
#endif
		}
		else
		{
			glDisable (GL_COLOR_MATERIAL);
			// Since we leave glColorMaterial mode, GL diffuse is now scracth. reset him to current value.
			CRGBA	diffCol;
			diffCol.R= (uint8)((_CurDiffuse >> 24) & 255);
			diffCol.G= (uint8)((_CurDiffuse >> 16) & 255);
			diffCol.B= (uint8)((_CurDiffuse >>  8) & 255);
			diffCol.A= (uint8)((_CurDiffuse      ) & 255);
			GLfloat	glColor[4];
			convColor(diffCol, glColor);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glColor);
		}
	}
}


// ***************************************************************************
void CDriverGLStates::updateDepthRange()
{
	H_AUTO_OGL(CDriverGLStates_updateDepthRange);

	float delta = _ZBias * (_DepthRangeFar - _DepthRangeNear);

#ifdef USE_OPENGLES
	glDepthRangef(delta + _DepthRangeNear, delta + _DepthRangeFar);
#else
	glDepthRange(delta + _DepthRangeNear, delta + _DepthRangeFar);
#endif
}

// ***************************************************************************
void		CDriverGLStates::setZBias(float zbias)
{
	H_AUTO_OGL(CDriverGLStates_setZBias)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zbias != _ZBias)
#endif
	{
		_ZBias = zbias;
		updateDepthRange();
	}
}


// ***************************************************************************
void CDriverGLStates::setDepthRange(float znear, float zfar)
{
	H_AUTO_OGL(CDriverGLStates_setDepthRange)
	nlassert(znear != zfar);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (znear != _DepthRangeNear || zfar != _DepthRangeFar)
#endif
	{
		_DepthRangeNear = znear;
		_DepthRangeFar = zfar;
		updateDepthRange();
	}
}

// ***************************************************************************
void		CDriverGLStates::setTexGenMode (uint stage, GLint mode)
{
	H_AUTO_OGL(CDriverGLStates_setTexGenMode);

#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (mode != _TexGenMode[stage])
#endif
	{
		_TexGenMode[stage] = mode;

		if (!_TextureCubeMapSupported) return;

		if(mode==0)
		{
#ifdef USE_OPENGLES
			glDisable(GL_TEXTURE_GEN_STR_OES);
#else
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );
#endif
		}
		else
		{
#ifdef USE_OPENGLES
			nglTexGeniOES(GL_TEXTURE_GEN_STR_OES, GL_TEXTURE_GEN_MODE_OES, mode);
#else
			glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, mode);
			glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, mode);
			glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, mode);
#endif

			/* Object or Eye Space ? => enable W generation. VERY IMPORTANT because
				was a bug with VegetableRender and ShadowRender:
					- Vegetable use the TexCoord1.w in his VertexProgram
					- Shadow Render don't use any TexCoord in VB (since projected)
					=> TexCoord1.w dirty!!
			*/
#ifdef USE_OPENGLES
//			if(mode==GL_OBJECT_LINEAR || mode==GL_EYE_LINEAR)
//			{
//				nglTexGeniOES(GL_TEXTURE_GEN_STR_OES, GL_TEXTURE_GEN_MODE_OES, mode);
//				glEnable(GL_TEXTURE_GEN_STR_OES);
//			}
//			else
//			{
//				glDisable(GL_TEXTURE_GEN_STR_OES);
//			}
#else
			if(mode==GL_OBJECT_LINEAR || mode==GL_EYE_LINEAR)
			{
				glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, mode);
				glEnable( GL_TEXTURE_GEN_Q );
			}
			else
			{
				glDisable( GL_TEXTURE_GEN_Q );
			}
#endif

			// Enable All.
#ifdef USE_OPENGLES
			glEnable(GL_TEXTURE_GEN_STR_OES);
#else
			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_R );
#endif
		}
	}
}

// ***************************************************************************
void			CDriverGLStates::resetTextureMode()
{
	H_AUTO_OGL(CDriverGLStates_resetTextureMode);

	glDisable(GL_TEXTURE_2D);

	if (_TextureCubeMapSupported)
	{
		glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	}

#ifndef USE_OPENGLES
	if (_TextureRectangleSupported)
	{
		glDisable(GL_TEXTURE_RECTANGLE_NV);
	}
#endif

	_TextureMode[_CurrentActiveTextureARB]= TextureDisabled;
}


// ***************************************************************************
void			CDriverGLStates::setTextureMode(TTextureMode texMode)
{
	H_AUTO_OGL(CDriverGLStates_setTextureMode)
	TTextureMode	oldTexMode = _TextureMode[_CurrentActiveTextureARB];
	if(oldTexMode != texMode)
	{
		// Disable first old mode.
		if (oldTexMode == Texture2D)
		{
			glDisable(GL_TEXTURE_2D);
		}
		else if(oldTexMode == TextureRect)
		{
#ifndef USE_OPENGLES
			if(_TextureRectangleSupported)
			{
				glDisable(GL_TEXTURE_RECTANGLE_NV);
			}
			else
#endif
			{
				glDisable(GL_TEXTURE_2D);
			}
		}
		else if(oldTexMode == TextureCubeMap)
		{
			if(_TextureCubeMapSupported)
			{
				glDisable(GL_TEXTURE_CUBE_MAP_ARB);
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
			}
		}

		// Enable new mode.
		if(texMode == Texture2D)
		{
			glEnable(GL_TEXTURE_2D);
		}
		else if(texMode == TextureRect)
		{
#ifndef USE_OPENGLES
			if(_TextureRectangleSupported)
			{
				glEnable(GL_TEXTURE_RECTANGLE_NV);
			}
			else
#endif
			{
				glEnable(GL_TEXTURE_2D);
			}
		}
		else if(texMode == TextureCubeMap)
		{
			if(_TextureCubeMapSupported)
			{
				glEnable(GL_TEXTURE_CUBE_MAP_ARB);
			}
			else
			{
				glEnable(GL_TEXTURE_2D);
			}
		}

		// new mode.
		_TextureMode[_CurrentActiveTextureARB]= texMode;
	}
}


// ***************************************************************************
void			CDriverGLStates::activeTextureARB(uint stage)
{
	H_AUTO_OGL(CDriverGLStates_activeTextureARB);

	if( _CurrentActiveTextureARB != stage )
	{
#ifdef USE_OPENGLES
		glActiveTexture(GL_TEXTURE0+stage);
#else
		nglActiveTextureARB(GL_TEXTURE0_ARB+stage);
#endif

		_CurrentActiveTextureARB= stage;
	}
}

// ***************************************************************************
void			CDriverGLStates::forceActiveTextureARB(uint stage)
{
	H_AUTO_OGL(CDriverGLStates_forceActiveTextureARB);

#ifdef USE_OPENGLES
	glActiveTexture(GL_TEXTURE0+stage);
#else
	nglActiveTextureARB(GL_TEXTURE0_ARB+stage);
#endif

	_CurrentActiveTextureARB= stage;
}

// ***************************************************************************
void			CDriverGLStates::enableVertexArray(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableVertexArray);

	if(_VertexArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_VERTEX_ARRAY);
		else
			glDisableClientState(GL_VERTEX_ARRAY);

		_VertexArrayEnabled= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableNormalArray(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableNormalArray)
	if(_NormalArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_NORMAL_ARRAY);
		else
			glDisableClientState(GL_NORMAL_ARRAY);
		_NormalArrayEnabled= enable;


	}
}
// ***************************************************************************
void			CDriverGLStates::enableWeightArray(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableWeightArray);

	if(_WeightArrayEnabled != enable)
	{
#ifndef USE_OPENGLES
		if(enable)
			glEnableClientState(GL_VERTEX_WEIGHTING_EXT);
		else
			glDisableClientState(GL_VERTEX_WEIGHTING_EXT);
#endif

		_WeightArrayEnabled= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableColorArray(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableColorArray);

	if(_ColorArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_COLOR_ARRAY);
		else
			glDisableClientState(GL_COLOR_ARRAY);
		_ColorArrayEnabled= enable;


	}
}


// ***************************************************************************
void			CDriverGLStates::enableSecondaryColorArray(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableSecondaryColorArray);

	if(_SecondaryColorArrayEnabled != enable)
	{
#ifndef USE_OPENGLES
		if(enable)
			glEnableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
		else
			glDisableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
#endif

		_SecondaryColorArrayEnabled= enable;

#ifndef USE_OPENGLES
		// If disable
		if(!enable)
		{
			// GeForceFx Bug: Must reset Secondary color to 0 (if comes from a VP), else bugs
			nglSecondaryColor3ubEXT(0,0,0);
		}
#endif
	}
}

// ***************************************************************************
void			CDriverGLStates::clientActiveTextureARB(uint stage)
{
	H_AUTO_OGL(CDriverGLStates_clientActiveTextureARB);

	if( _CurrentClientActiveTextureARB != stage )
	{
#ifdef USE_OPENGLES
		glClientActiveTexture(GL_TEXTURE0+stage);
#else
		nglClientActiveTextureARB(GL_TEXTURE0_ARB+stage);
#endif
		_CurrentClientActiveTextureARB= stage;
	}
}

// ***************************************************************************
void			CDriverGLStates::enableTexCoordArray(bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableTexCoordArray);

	if(_TexCoordArrayEnabled[_CurrentClientActiveTextureARB] != enable)
	{
		if(enable)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		else
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);


		_TexCoordArrayEnabled[_CurrentClientActiveTextureARB]= enable;
	}
}

// ***************************************************************************
void			CDriverGLStates::enableVertexAttribArray(uint glIndex, bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableVertexAttribArray);

	if(_VertexAttribArrayEnabled[glIndex] != enable)
	{
#ifndef USE_OPENGLES
		if(enable)
			glEnableClientState(glIndex+GL_VERTEX_ATTRIB_ARRAY0_NV);
		else
			glDisableClientState(glIndex+GL_VERTEX_ATTRIB_ARRAY0_NV);
#endif

		_VertexAttribArrayEnabled[glIndex]= enable;
	}
}

// ***************************************************************************
void CDriverGLStates::enableVertexAttribArrayARB(uint glIndex,bool enable)
{
	H_AUTO_OGL(CDriverGLStates_enableVertexAttribArrayARB);

	#ifndef NL3D_GLSTATE_DISABLE_CACHE
		if(_VertexAttribArrayEnabled[glIndex] != enable)
	#endif
	{
#ifndef USE_OPENGLES
		if(enable)
			nglEnableVertexAttribArrayARB(glIndex);
		else
			nglDisableVertexAttribArrayARB(glIndex);
#endif

		_VertexAttribArrayEnabled[glIndex]= enable;
	}
}

// ***************************************************************************
void CDriverGLStates::enableVertexAttribArrayForEXTVertexShader(uint glIndex, bool enable, uint *variants)
{
	H_AUTO_OGL(CDriverGLStates_enableVertexAttribArrayForEXTVertexShader);

	if(_VertexAttribArrayEnabled[glIndex] != enable)
	{
		switch(glIndex)
		{
			case 0: // position
				enableVertexArray(enable);
			break;
			case 1: // skin weight
#ifndef USE_OPENGLES
				if(enable)
					nglEnableVariantClientStateEXT(variants[CDriverGL::EVSSkinWeightVariant]);
				else
					nglDisableVariantClientStateEXT(variants[CDriverGL::EVSSkinWeightVariant]);
#endif
			break;
			case 2: // normal
				enableNormalArray(enable);
			break;
			case 3: // color
				enableColorArray(enable);
			break;
			case 4: // secondary color
#ifndef USE_OPENGLES
				if(enable)
					nglEnableVariantClientStateEXT(variants[CDriverGL::EVSSecondaryColorVariant]);
				else
					nglDisableVariantClientStateEXT(variants[CDriverGL::EVSSecondaryColorVariant]);
#endif
			break;
			case 5: // fog coordinate
#ifndef USE_OPENGLES
				if(enable)
					nglEnableVariantClientStateEXT(variants[CDriverGL::EVSFogCoordsVariant]);
				else
					nglDisableVariantClientStateEXT(variants[CDriverGL::EVSFogCoordsVariant]);
#endif
			break;
			case 6: // palette skin
#ifndef USE_OPENGLES
				if(enable)
					nglEnableVariantClientStateEXT(variants[CDriverGL::EVSPaletteSkinVariant]);
				else
					nglDisableVariantClientStateEXT(variants[CDriverGL::EVSPaletteSkinVariant]);
#endif
			break;
			case 7: // empty
				nlstop;
			break;
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				clientActiveTextureARB(glIndex - 8);
				enableTexCoordArray(enable);
			break;
			default:
				nlstop; // invalid value
			break;
		}
		_VertexAttribArrayEnabled[glIndex]= enable;
	}
}



// ***************************************************************************
void			CDriverGLStates::enableFog(uint enable)
{
	H_AUTO_OGL(CDriverGLStates_enableFog)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurFog )
#endif
	{
		// new state.
		_CurFog= enabled;
		// Setup GLState.
		if(_CurFog)
			glEnable(GL_FOG);
		else
			glDisable(GL_FOG);
	}
}

// ***************************************************************************
void CDriverGLStates::forceBindARBVertexBuffer(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates_forceBindARBVertexBuffer)

#ifdef USE_OPENGLES
	glBindBuffer(GL_ARRAY_BUFFER, objectID);
#else
	nglBindBufferARB(GL_ARRAY_BUFFER_ARB, objectID);
#endif

	_CurrARBVertexBuffer = objectID;
}

// ***************************************************************************
void CDriverGLStates::bindARBVertexBuffer(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates_bindARBVertexBuffer)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (objectID != _CurrARBVertexBuffer)
#endif
	{
		forceBindARBVertexBuffer(objectID);
	}
}

// ***************************************************************************
void CDriverGLStates::setCullMode(TCullMode cullMode)
{
	H_AUTO_OGL(CDriverGLStates_setCullMode)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (cullMode != _CullMode)
#endif
	{
		glCullFace(cullMode == CCW ? GL_BACK : GL_FRONT);
		_CullMode = cullMode;
	}
}

// ***************************************************************************
CDriverGLStates::TCullMode CDriverGLStates::getCullMode() const
{
	H_AUTO_OGL(CDriverGLStates_CDriverGLStates)
	return _CullMode;
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
