// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
namespace NLDRIVERGL3 {

// ***************************************************************************
CDriverGLStates3::CDriverGLStates3()
{
	H_AUTO_OGL(CDriverGLStates3_CDriverGLStates)
	_CurrARBVertexBuffer = 0;
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;
	_CullMode = CCW;
}


// ***************************************************************************
void			CDriverGLStates3::init()
{
	H_AUTO_OGL(CDriverGLStates3_init)

	// By default all arrays are disabled.
	for (uint i = 0; i < CVertexBuffer::NumValue; ++i)
		_VertexAttribArrayEnabled[i] = false;
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;
}


// ***************************************************************************
void			CDriverGLStates3::forceDefaults(uint nbStages)
{
	H_AUTO_OGL(CDriverGLStates3_forceDefaults);

	// Enable / disable.
	_CurBlend = false;
	_CurCullFace = true;
	_CurZWrite = true;
	_CurStencilTest =false;

	// setup GLStates.
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);

	// Func.
	_CurBlendSrc = GL_SRC_ALPHA;
	_CurBlendDst = GL_ONE_MINUS_SRC_ALPHA;
	_CurDepthFunc = GL_LEQUAL;
	_CurStencilFunc = GL_ALWAYS;
	_CurStencilRef = 0;
	_CurStencilMask = std::numeric_limits<GLuint>::max();
	_CurStencilOpFail = GL_KEEP;
	_CurStencilOpZFail = GL_KEEP;
	_CurStencilOpZPass = GL_KEEP;
	_CurStencilWriteMask = std::numeric_limits<GLuint>::max();

	// setup GLStates.
	glBlendFunc(_CurBlendSrc, _CurBlendDst);
	glDepthFunc(_CurDepthFunc);
	glStencilFunc(_CurStencilFunc, _CurStencilRef, _CurStencilMask);
	glStencilOp(_CurStencilOpFail, _CurStencilOpZFail, _CurStencilOpZPass);
	glStencilMask(_CurStencilWriteMask);

	// setup GLStates.
	static const GLfloat one[4] = { 1, 1, 1, 1 };
	static const GLfloat zero[4] = { 0, 0, 0, 1 };

	// TexModes
	// FIXME GL3 TEXTUREMODE for (uint stage = 0; stage < nbStages; ++stage)
	// FIXME GL3 TEXTUREMODE {
		// disable texturing.
		// FIXME GL3 TEXTUREMODE nglActiveTexture(GL_TEXTURE0 + stage);
		// FIXME GL3 TEXTUREMODE glDisable(GL_TEXTURE_2D);
		// FIXME GL3 TEXTUREMODE glDisable(GL_TEXTURE_CUBE_MAP);
		// FIXME GL3 TEXTUREMODE glDisable(GL_TEXTURE_RECTANGLE);
		// FIXME GL3 TEXTUREMODE _TextureMode[stage]= TextureDisabled;
	// FIXME GL3 TEXTUREMODE }
	//	 etc

	// ActiveTexture current texture to 0.
	nglActiveTexture(GL_TEXTURE0);
	_CurrentActiveTexture = 0;

	// Depth range
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;

	glDepthRange (0, 1);

	// Cull order
	_CullMode = CCW;
	glCullFace(GL_BACK);
}

// ***************************************************************************
void			CDriverGLStates3::enableBlend(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableBlend)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurBlend)
#endif
	{
		// new state.
		_CurBlend= enabled;
		// Setup GLState.
		if (_CurBlend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}

// ***************************************************************************
void			CDriverGLStates3::enableCullFace(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableCullFace)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurCullFace)
#endif
	{
		// new state.
		_CurCullFace= enabled;
		// Setup GLState.
		if (_CurCullFace)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}
}

// ***************************************************************************
void			CDriverGLStates3::enableZWrite(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableZWrite)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurZWrite)
#endif
	{
		// new state.
		_CurZWrite= enabled;
		// Setup GLState.
		if (_CurZWrite)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
}


// ***************************************************************************
void			CDriverGLStates3::enableStencilTest(bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableStencilTest);

	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enable != _CurStencilTest)
#endif
	{
		// new state.
		_CurStencilTest= enable;
		// Setup GLState.
		if (_CurStencilTest)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}
}

// ***************************************************************************
void			CDriverGLStates3::blendFunc(GLenum src, GLenum dst)
{
	H_AUTO_OGL(CDriverGLStates3_blendFunc)
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (src!= _CurBlendSrc || dst!=_CurBlendDst)
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
void			CDriverGLStates3::depthFunc(GLenum zcomp)
{
	H_AUTO_OGL(CDriverGLStates3_depthFunc)
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zcomp != _CurDepthFunc)
#endif
	{
		// new state.
		_CurDepthFunc= zcomp;
		// Setup GLState.
		glDepthFunc(_CurDepthFunc);
	}
}

// ***************************************************************************
void			CDriverGLStates3::stencilFunc(GLenum func, GLint ref, GLuint mask)
{
	H_AUTO_OGL(CDriverGLStates3_stencilFunc)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if ((func!=_CurStencilFunc) || (ref!=_CurStencilRef) || (mask!=_CurStencilMask))
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
void			CDriverGLStates3::stencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	H_AUTO_OGL(CDriverGLStates3_stencilOp)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if ((fail!=_CurStencilOpFail) || (zfail!=_CurStencilOpZFail) || (zpass!=_CurStencilOpZPass))
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
void			CDriverGLStates3::stencilMask(GLuint mask)
{
	H_AUTO_OGL(CDriverGLStates3_stencilMask)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (mask!=_CurStencilWriteMask)
#endif
	{
		// new state
		_CurStencilWriteMask = mask;

		// setup function.
		glStencilMask(_CurStencilWriteMask);
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
void CDriverGLStates3::updateDepthRange()
{
	H_AUTO_OGL(CDriverGLStates3_updateDepthRange);

	float delta = _ZBias * (_DepthRangeFar - _DepthRangeNear);

	glDepthRange(delta + _DepthRangeNear, delta + _DepthRangeFar);

}

// ***************************************************************************
void		CDriverGLStates3::setZBias(float zbias)
{
	H_AUTO_OGL(CDriverGLStates3_setZBias)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zbias != _ZBias)
#endif
	{
		_ZBias = zbias;
		updateDepthRange();
	}
}


// ***************************************************************************
void CDriverGLStates3::setDepthRange(float znear, float zfar)
{
	H_AUTO_OGL(CDriverGLStates3_setDepthRange)
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
void			CDriverGLStates3::activeTexture(uint stage)
{
	H_AUTO_OGL(CDriverGLStates3_activeTexture);

	if (_CurrentActiveTexture != stage)
	{
		nglActiveTexture(GL_TEXTURE0 + stage);

		_CurrentActiveTexture = stage;
	}
}

// ***************************************************************************
void			CDriverGLStates3::forceActiveTexture(uint stage)
{
	H_AUTO_OGL(CDriverGLStates3_forceActiveTexture);

	nglActiveTexture(GL_TEXTURE0 + stage);

	_CurrentActiveTexture= stage;
}

// ***************************************************************************
void CDriverGLStates3::enableVertexAttribArrayARB(uint glIndex,bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableVertexAttribArrayARB);

	#ifndef NL3D_GLSTATE_DISABLE_CACHE
		if (_VertexAttribArrayEnabled[glIndex] != enable)
	#endif
	{
		if (enable)
			nglEnableVertexAttribArray(glIndex);
		else
			nglDisableVertexAttribArray(glIndex);

		_VertexAttribArrayEnabled[glIndex]= enable;
	}

}

// ***************************************************************************
void CDriverGLStates3::forceBindARBVertexBuffer(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindARBVertexBuffer)

	nglBindBuffer(GL_ARRAY_BUFFER, objectID);

	_CurrARBVertexBuffer = objectID;
}

// ***************************************************************************
void CDriverGLStates3::bindARBVertexBuffer(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates3_bindARBVertexBuffer)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (objectID != _CurrARBVertexBuffer)
#endif
	{
		forceBindARBVertexBuffer(objectID);
	}
}

// ***************************************************************************
void CDriverGLStates3::setCullMode(TCullMode cullMode)
{
	H_AUTO_OGL(CDriverGLStates3_setCullMode)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (cullMode != _CullMode)
#endif
	{
		glCullFace(cullMode == CCW ? GL_BACK : GL_FRONT);
		_CullMode = cullMode;
	}
}

// ***************************************************************************
CDriverGLStates3::TCullMode CDriverGLStates3::getCullMode() const
{
	H_AUTO_OGL(CDriverGLStates3_CDriverGLStates)
	return _CullMode;
}

} // NLDRIVERGL3
} // NL3D
