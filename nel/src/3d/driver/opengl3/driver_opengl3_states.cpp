// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014-2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdopengl3.h"

#include "driver_opengl3.h"

// Define for debug: bypasses all cache checks, forcing every GL call through.
// #define NL3D_GLSTATE_DISABLE_CACHE

namespace NL3D {
namespace NLDRIVERGL3 {

// ===========================================================================
// Lifecycle
// ===========================================================================

CDriverGLStates3::CDriverGLStates3()
{
	H_AUTO_OGL(CDriverGLStates3_CDriverGLStates)

	// --- Rendering state (reset by forceDefaults) ---

	// Rasterizer
	m_CurCullFace = true;
	m_CullMode = CCW;
	m_CurPolygonMode = GL_FILL;

	// Fragment operations
	m_CurBlend = false;
	m_CurBlendSrc = GL_SRC_ALPHA;
	m_CurBlendDst = GL_ONE_MINUS_SRC_ALPHA;
	m_CurBlendColor.set(0, 0, 0, 0);
	m_CurZWrite = true;
	m_CurDepthFunc = GL_LEQUAL;
	m_CurStencilTest = false;
	m_CurStencilFunc = GL_ALWAYS;
	m_CurStencilRef = 0;
	m_CurStencilMask = std::numeric_limits<GLuint>::max();
	m_CurStencilOpFail = GL_KEEP;
	m_CurStencilOpZFail = GL_KEEP;
	m_CurStencilOpZPass = GL_KEEP;
	m_CurStencilWriteMask = std::numeric_limits<GLuint>::max();
	m_CurColorMaskR = m_CurColorMaskG = m_CurColorMaskB = m_CurColorMaskA = true;

	// Scissor test
	m_CurScissorTest = false;

	// Depth range
	m_DepthRangeNear = 0.f;
	m_DepthRangeFar = 1.f;
	m_ZBias = 0.f;

	// Clip planes
	for (uint i = 0; i < MaxClipDistances; ++i)
		m_CurClipDistance[i] = false;
	m_PPClipPlanes = false;

	// --- Geometry state (NOT reset by forceDefaults) ---

	// Viewport and scissor rect
	m_CurViewportX = m_CurViewportY = 0;
	m_CurViewportWidth = m_CurViewportHeight = 0;
	m_CurScissorX = m_CurScissorY = 0;
	m_CurScissorWidth = m_CurScissorHeight = 0;

	// --- Binding state (NOT reset by forceDefaults) ---

	// Vertex input (single default VAO)
	m_DefaultVAO = 0;
	m_CurVAO = 0;
	for (uint i = 0; i < CVertexBuffer::NumValue; ++i)
		m_VertexAttribArrayEnabled[i] = false;
	m_CurrArrayBuffer = 0;

	// Texture state
	m_CurrentActiveTexture = 0;
	for (uint i = 0; i < MaxTextureUnits; ++i)
	{
		m_CurTextureTarget[i] = 0;
		m_CurTexture[i] = 0;
	}

	// Framebuffer
	m_CurFramebuffer = 0;

	// Shader programs
	m_CurProgram = 0;
	m_CurProgramPipeline = 0;

	// Buffer objects
	m_CurUniformBuffer = 0;
	for (uint i = 0; i < MaxUBOBindings; ++i)
		m_CurUniformBufferBase[i] = 0;
	m_CurPixelUnpackBuffer = 0;
}

void CDriverGLStates3::init()
{
	H_AUTO_OGL(CDriverGLStates3_init)

	// Create and bind the single default VAO used for the driver's entire lifetime.
	// GL 3.3 core profile requires a VAO for all vertex operations.
	// A new VAO starts with all attrib arrays disabled, matching the cache defaults.
	nglGenVertexArrays(1, &m_DefaultVAO);
	nglBindVertexArray(m_DefaultVAO);
	m_CurVAO = m_DefaultVAO;
}

void CDriverGLStates3::release()
{
	H_AUTO_OGL(CDriverGLStates3_release)

	if (m_DefaultVAO)
	{
		nglDeleteVertexArrays(1, &m_DefaultVAO);
		m_DefaultVAO = 0;
	}
}

void CDriverGLStates3::forceDefaults()
{
	H_AUTO_OGL(CDriverGLStates3_forceDefaults);

	// Rasterizer
	m_CurCullFace = true;
	m_CullMode = CCW;
	m_CurPolygonMode = GL_FILL;
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // TODO GLES: not available

	// Fragment operations
	m_CurBlend = false;
	m_CurBlendSrc = GL_SRC_ALPHA;
	m_CurBlendDst = GL_ONE_MINUS_SRC_ALPHA;
	m_CurBlendColor.set(0, 0, 0, 0);
	m_CurZWrite = true;
	m_CurDepthFunc = GL_LEQUAL;
	m_CurStencilTest = false;
	m_CurStencilFunc = GL_ALWAYS;
	m_CurStencilRef = 0;
	m_CurStencilMask = std::numeric_limits<GLuint>::max();
	m_CurStencilOpFail = GL_KEEP;
	m_CurStencilOpZFail = GL_KEEP;
	m_CurStencilOpZPass = GL_KEEP;
	m_CurStencilWriteMask = std::numeric_limits<GLuint>::max();
	m_CurColorMaskR = m_CurColorMaskG = m_CurColorMaskB = m_CurColorMaskA = true;

	glDisable(GL_BLEND);
	glBlendFunc(m_CurBlendSrc, m_CurBlendDst);
	nglBlendColor(0.f, 0.f, 0.f, 0.f);
	glDepthMask(GL_TRUE);
	glDepthFunc(m_CurDepthFunc);
	glDisable(GL_STENCIL_TEST);
	glStencilFunc(m_CurStencilFunc, m_CurStencilRef, m_CurStencilMask);
	glStencilOp(m_CurStencilOpFail, m_CurStencilOpZFail, m_CurStencilOpZPass);
	glStencilMask(m_CurStencilWriteMask);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Scissor test (not rect -- rect is caller-specific)
	m_CurScissorTest = false;
	glDisable(GL_SCISSOR_TEST);

	// Depth range
	m_DepthRangeNear = 0.f;
	m_DepthRangeFar = 1.f;
	m_ZBias = 0.f;
	glDepthRange(0, 1); // TODO GLES: glDepthRangef(0.f, 1.f);

	// Clip planes
	for (uint i = 0; i < MaxClipDistances; ++i)
	{
		m_CurClipDistance[i] = false;
		if (!m_PPClipPlanes)
			glDisable(GL_CLIP_DISTANCE0 + i);
	}

}

// ===========================================================================
// ===========================================================================
// Rendering state -- reset by forceDefaults()
// ===========================================================================
// ===========================================================================

// ===========================================================================
// Rasterizer state
// ===========================================================================

void CDriverGLStates3::enableCullFace(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableCullFace)
	bool enabled = (enable != 0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != m_CurCullFace)
#endif
	{
		m_CurCullFace = enabled;
		if (m_CurCullFace)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}
}

void CDriverGLStates3::setCullMode(TCullMode cullMode)
{
	H_AUTO_OGL(CDriverGLStates3_setCullMode)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (cullMode != m_CullMode)
#endif
	{
		m_CullMode = cullMode;
		glCullFace(cullMode == CCW ? GL_BACK : GL_FRONT);
	}
}

CDriverGLStates3::TCullMode CDriverGLStates3::getCullMode() const
{
	return m_CullMode;
}

void CDriverGLStates3::polygonMode(GLenum mode)
{
	H_AUTO_OGL(CDriverGLStates3_polygonMode)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (mode != m_CurPolygonMode)
#endif
	{
		m_CurPolygonMode = mode;
		glPolygonMode(GL_FRONT_AND_BACK, mode); // TODO GLES: not available
	}
}

// ===========================================================================
// Fragment operations
// ===========================================================================

void CDriverGLStates3::enableBlend(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableBlend)
	bool enabled = (enable != 0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != m_CurBlend)
#endif
	{
		m_CurBlend = enabled;
		if (m_CurBlend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}

void CDriverGLStates3::blendFunc(GLenum src, GLenum dst)
{
	H_AUTO_OGL(CDriverGLStates3_blendFunc)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (src != m_CurBlendSrc || dst != m_CurBlendDst)
#endif
	{
		m_CurBlendSrc = src;
		m_CurBlendDst = dst;
		glBlendFunc(m_CurBlendSrc, m_CurBlendDst);
	}
}

void CDriverGLStates3::blendColor(NLMISC::CRGBA color)
{
	H_AUTO_OGL(CDriverGLStates3_blendColor)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (color != m_CurBlendColor)
#endif
	{
		m_CurBlendColor = color;
		static const float OO255 = 1.0f / 255.0f;
		nglBlendColor(color.R * OO255, color.G * OO255, color.B * OO255, color.A * OO255);
	}
}

void CDriverGLStates3::enableZWrite(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableZWrite)
	bool enabled = (enable != 0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != m_CurZWrite)
#endif
	{
		m_CurZWrite = enabled;
		if (m_CurZWrite)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
}

void CDriverGLStates3::depthFunc(GLenum zcomp)
{
	H_AUTO_OGL(CDriverGLStates3_depthFunc)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zcomp != m_CurDepthFunc)
#endif
	{
		m_CurDepthFunc = zcomp;
		glDepthFunc(m_CurDepthFunc);
	}
}

void CDriverGLStates3::enableStencilTest(bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableStencilTest)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enable != m_CurStencilTest)
#endif
	{
		m_CurStencilTest = enable;
		if (m_CurStencilTest)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}
}

void CDriverGLStates3::stencilFunc(GLenum func, GLint ref, GLuint mask)
{
	H_AUTO_OGL(CDriverGLStates3_stencilFunc)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (func != m_CurStencilFunc || ref != m_CurStencilRef || mask != m_CurStencilMask)
#endif
	{
		m_CurStencilFunc = func;
		m_CurStencilRef = ref;
		m_CurStencilMask = mask;
		glStencilFunc(m_CurStencilFunc, m_CurStencilRef, m_CurStencilMask);
	}
}

void CDriverGLStates3::stencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	H_AUTO_OGL(CDriverGLStates3_stencilOp)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (fail != m_CurStencilOpFail || zfail != m_CurStencilOpZFail || zpass != m_CurStencilOpZPass)
#endif
	{
		m_CurStencilOpFail = fail;
		m_CurStencilOpZFail = zfail;
		m_CurStencilOpZPass = zpass;
		glStencilOp(m_CurStencilOpFail, m_CurStencilOpZFail, m_CurStencilOpZPass);
	}
}

void CDriverGLStates3::stencilMask(GLuint mask)
{
	H_AUTO_OGL(CDriverGLStates3_stencilMask)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (mask != m_CurStencilWriteMask)
#endif
	{
		m_CurStencilWriteMask = mask;
		glStencilMask(m_CurStencilWriteMask);
	}
}

void CDriverGLStates3::colorMask(bool red, bool green, bool blue, bool alpha)
{
	H_AUTO_OGL(CDriverGLStates3_colorMask)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (red != m_CurColorMaskR || green != m_CurColorMaskG || blue != m_CurColorMaskB || alpha != m_CurColorMaskA)
#endif
	{
		m_CurColorMaskR = red;
		m_CurColorMaskG = green;
		m_CurColorMaskB = blue;
		m_CurColorMaskA = alpha;
		glColorMask(red, green, blue, alpha);
	}
}

// ===========================================================================
// Scissor test
// ===========================================================================

void CDriverGLStates3::enableScissorTest(bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableScissorTest)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enable != m_CurScissorTest)
#endif
	{
		m_CurScissorTest = enable;
		if (m_CurScissorTest)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
	}
}

// ===========================================================================
// Depth range
// ===========================================================================

void CDriverGLStates3::updateDepthRange()
{
	H_AUTO_OGL(CDriverGLStates3_updateDepthRange)
	float delta = m_ZBias * (m_DepthRangeFar - m_DepthRangeNear);
	glDepthRange(delta + m_DepthRangeNear, delta + m_DepthRangeFar); // TODO GLES: glDepthRangef
}

void CDriverGLStates3::setDepthRange(float znear, float zfar)
{
	H_AUTO_OGL(CDriverGLStates3_setDepthRange)
	nlassert(znear != zfar);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (znear != m_DepthRangeNear || zfar != m_DepthRangeFar)
#endif
	{
		m_DepthRangeNear = znear;
		m_DepthRangeFar = zfar;
		updateDepthRange();
	}
}

void CDriverGLStates3::setZBias(float zbias)
{
	H_AUTO_OGL(CDriverGLStates3_setZBias)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zbias != m_ZBias)
#endif
	{
		m_ZBias = zbias;
		updateDepthRange();
	}
}

// ===========================================================================
// Clip planes
// ===========================================================================

void CDriverGLStates3::enableClipDistance(uint index, bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableClipDistance)
	nlassert(index < MaxClipDistances);
	if (m_PPClipPlanes)
		return;
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enable != m_CurClipDistance[index])
#endif
	{
		m_CurClipDistance[index] = enable;
		if (enable)
			glEnable(GL_CLIP_DISTANCE0 + index);
		else
			glDisable(GL_CLIP_DISTANCE0 + index);
	}
}

// ===========================================================================
// ===========================================================================
// Geometry state -- NOT reset by forceDefaults()
// ===========================================================================
// ===========================================================================

// ===========================================================================
// Viewport and scissor rect
// ===========================================================================

void CDriverGLStates3::viewport(sint x, sint y, sint width, sint height)
{
	H_AUTO_OGL(CDriverGLStates3_viewport)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (x != m_CurViewportX || y != m_CurViewportY || width != m_CurViewportWidth || height != m_CurViewportHeight)
#endif
	{
		m_CurViewportX = x;
		m_CurViewportY = y;
		m_CurViewportWidth = width;
		m_CurViewportHeight = height;
		glViewport(x, y, width, height);
	}
}

void CDriverGLStates3::scissor(sint x, sint y, sint width, sint height)
{
	H_AUTO_OGL(CDriverGLStates3_scissor)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (x != m_CurScissorX || y != m_CurScissorY || width != m_CurScissorWidth || height != m_CurScissorHeight)
#endif
	{
		m_CurScissorX = x;
		m_CurScissorY = y;
		m_CurScissorWidth = width;
		m_CurScissorHeight = height;
		glScissor(x, y, width, height);
	}
}

// ===========================================================================
// ===========================================================================
// Binding state -- NOT reset by forceDefaults()
// ===========================================================================
// ===========================================================================

// ===========================================================================
// Vertex input
// ===========================================================================

void CDriverGLStates3::forceBindVertexArray(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindVertexArray)
	nglBindVertexArray(id);
	m_CurVAO = id;
}

void CDriverGLStates3::bindVertexArray(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_bindVertexArray)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (id != m_CurVAO)
#endif
	{
		forceBindVertexArray(id);
	}
}

void CDriverGLStates3::enableVertexAttribArray(uint glIndex, bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableVertexAttribArray)

	// Attrib enable cache is only valid for the default VAO.
	// Rebind it if a different VAO is currently active.
	if (m_CurVAO != m_DefaultVAO)
		forceBindVertexArray(m_DefaultVAO);

#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (m_VertexAttribArrayEnabled[glIndex] != enable)
#endif
	{
		if (enable)
			nglEnableVertexAttribArray(glIndex);
		else
			nglDisableVertexAttribArray(glIndex);
		m_VertexAttribArrayEnabled[glIndex] = enable;
	}
}

void CDriverGLStates3::forceBindArrayBuffer(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindArrayBuffer)
	nglBindBuffer(GL_ARRAY_BUFFER, objectID);
	m_CurrArrayBuffer = objectID;
}

void CDriverGLStates3::bindArrayBuffer(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates3_bindArrayBuffer)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (objectID != m_CurrArrayBuffer)
#endif
	{
		forceBindArrayBuffer(objectID);
	}
}

// ===========================================================================
// Texture state
// ===========================================================================

void CDriverGLStates3::activeTexture(uint stage)
{
	H_AUTO_OGL(CDriverGLStates3_activeTexture)
	if (m_CurrentActiveTexture != stage)
	{
		nglActiveTexture(GL_TEXTURE0 + stage);
		m_CurrentActiveTexture = stage;
	}
}

void CDriverGLStates3::forceBindTexture(GLenum target, GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindTexture)
	glBindTexture(target, id);
	nlassert(m_CurrentActiveTexture < MaxTextureUnits);
	m_CurTextureTarget[m_CurrentActiveTexture] = target;
	m_CurTexture[m_CurrentActiveTexture] = id;
}

void CDriverGLStates3::bindTexture(GLenum target, GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_bindTexture)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	nlassert(m_CurrentActiveTexture < MaxTextureUnits);
	if (target != m_CurTextureTarget[m_CurrentActiveTexture] || id != m_CurTexture[m_CurrentActiveTexture])
#endif
	{
		forceBindTexture(target, id);
	}
}

// ===========================================================================
// Framebuffer
// ===========================================================================

void CDriverGLStates3::forceBindFramebuffer(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindFramebuffer)
	nglBindFramebuffer(GL_FRAMEBUFFER, id);
	m_CurFramebuffer = id;
}

void CDriverGLStates3::bindFramebuffer(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_bindFramebuffer)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (id != m_CurFramebuffer)
#endif
	{
		forceBindFramebuffer(id);
	}
}

// ===========================================================================
// Shader programs
// ===========================================================================

void CDriverGLStates3::forceUseProgram(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_forceUseProgram)
	nglUseProgram(id);
	m_CurProgram = id;
}

void CDriverGLStates3::useProgram(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_useProgram)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (id != m_CurProgram)
#endif
	{
		forceUseProgram(id);
	}
}

void CDriverGLStates3::forceBindProgramPipeline(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindProgramPipeline)
	nglBindProgramPipeline(id);
	m_CurProgramPipeline = id;
}

void CDriverGLStates3::bindProgramPipeline(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_bindProgramPipeline)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (id != m_CurProgramPipeline)
#endif
	{
		forceBindProgramPipeline(id);
	}
}

// ===========================================================================
// Buffer objects
// ===========================================================================

void CDriverGLStates3::forceBindUniformBuffer(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindUniformBuffer)
	nglBindBuffer(GL_UNIFORM_BUFFER, id);
	m_CurUniformBuffer = id;
}

void CDriverGLStates3::forceBindUniformBufferBase(GLuint binding, GLuint bufferId)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindUniformBufferBase)
	nglBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId);
	nlassert(binding < MaxUBOBindings);
	m_CurUniformBufferBase[binding] = bufferId;
}

void CDriverGLStates3::bindUniformBufferBase(GLuint binding, GLuint bufferId)
{
	H_AUTO_OGL(CDriverGLStates3_bindUniformBufferBase)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	nlassert(binding < MaxUBOBindings);
	if (bufferId != m_CurUniformBufferBase[binding])
#endif
	{
		forceBindUniformBufferBase(binding, bufferId);
	}
}

void CDriverGLStates3::forceBindPixelUnpackBuffer(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindPixelUnpackBuffer)
	nglBindBuffer(GL_PIXEL_UNPACK_BUFFER, id);
	m_CurPixelUnpackBuffer = id;
}

void CDriverGLStates3::bindPixelUnpackBuffer(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_bindPixelUnpackBuffer)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (id != m_CurPixelUnpackBuffer)
#endif
	{
		forceBindPixelUnpackBuffer(id);
	}
}

} // NLDRIVERGL3
} // NL3D

/* end of file */
