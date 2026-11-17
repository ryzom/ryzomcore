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
	{
		m_VertexAttribArrayEnabled[i] = false;
		m_VertexAttribPointer[i].Buffer = 0;
	}
	m_CurrArrayBuffer = 0;
	m_CurElementArrayBuffer = 0;

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
	{
		m_CurUniformBufferBase[i] = 0;
		m_CurUniformBufferOffset[i] = 0;
		m_CurUniformBufferSize[i] = 0;
		m_WantUniformBufferBase[i] = 0;
		m_WantUniformBufferOffset[i] = 0;
		m_WantUniformBufferSize[i] = 0;
	}
	m_UniformBufferDirty = 0;
	m_MultiBind = false;
	m_CurPixelUnpackBuffer = 0;

#ifdef NL3D_GLSTATE_CHECK_BINDINGS
	m_BindingCheckWarned = 0;
	m_ProgramPipelineUsed = false;
#endif
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

	// The context is new, nothing is bound in it. What the cache has of a context
	// before it is forgotten: a cached bind would skip the first bind of an object
	// whose name the cache still has.
	for (uint i = 0; i < CVertexBuffer::NumValue; ++i)
	{
		m_VertexAttribArrayEnabled[i] = false;
		m_VertexAttribPointer[i].Buffer = 0;
	}
	m_CurrArrayBuffer = 0;
	m_CurElementArrayBuffer = 0;
	m_CurrentActiveTexture = 0;
	for (uint i = 0; i < MaxTextureUnits; ++i)
	{
		m_CurTextureTarget[i] = 0;
		m_CurTexture[i] = 0;
	}
	m_CurFramebuffer = 0;
	m_CurProgram = 0;
	m_CurProgramPipeline = 0;
	m_CurUniformBuffer = 0;
	for (uint i = 0; i < MaxUBOBindings; ++i)
	{
		m_CurUniformBufferBase[i] = 0;
		m_CurUniformBufferOffset[i] = 0;
		m_CurUniformBufferSize[i] = 0;
		m_WantUniformBufferBase[i] = 0;
		m_WantUniformBufferOffset[i] = 0;
		m_WantUniformBufferSize[i] = 0;
	}
	m_UniformBufferDirty = 0;
	m_CurPixelUnpackBuffer = 0;

	// The viewport and the scissor rectangle of a new context are those of its window,
	// not what the context before had last: the first of each is issued
	m_CurViewportWidth = m_CurViewportHeight = -1;
	m_CurScissorWidth = m_CurScissorHeight = -1;

#ifdef NL3D_GLSTATE_CHECK_BINDINGS
	m_BindingCheckWarned = 0;
	m_ProgramPipelineUsed = false;
#endif
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

#ifdef NL3D_GLSTATE_CHECK_BINDINGS

namespace /* anonymous */ {

// The kinds of mismatch that checkBindings() tells apart, one bit each
enum TBindingCheck
{
	BindingCheckProgram = 0,
	BindingCheckProgramPipeline,
	BindingCheckVertexArray,
	BindingCheckArrayBuffer,
	BindingCheckElementArrayBuffer,
	BindingCheckAttribEnabled,
	BindingCheckAttribBuffer,
	BindingCheckAttribFormat,
	BindingCheckAttribPointer,
	BindingCheckActiveTexture,
	BindingCheckTexture,
	BindingCheckUniformBuffer,
	BindingCheckUniformBufferIndexed
};

} /* anonymous namespace */

bool CDriverGLStates3::warnBinding(uint kind)
{
	if (m_BindingCheckWarned & (1 << kind))
		return false;
	m_BindingCheckWarned |= (1 << kind);
	return true;
}

void CDriverGLStates3::checkBindings()
{
	H_AUTO_OGL(CDriverGLStates3_checkBindings)

	GLint value = 0;

	// --- Programs
	glGetIntegerv(GL_CURRENT_PROGRAM, &value);
	if ((GLuint)value != m_CurProgram && warnBinding(BindingCheckProgram))
		nlwarning("3D: state cache: the program in use is %u in the cache and %u in GL", (uint)m_CurProgram, (uint)value);

	// (a context without program pipelines does not have the query)
	if (m_ProgramPipelineUsed)
	{
		value = 0;
		glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, &value);
		if ((GLuint)value != m_CurProgramPipeline && warnBinding(BindingCheckProgramPipeline))
			nlwarning("3D: state cache: the program pipeline is %u in the cache and %u in GL", (uint)m_CurProgramPipeline, (uint)value);
	}

	// --- Vertex input
	GLint vertexArray = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray);
	if ((GLuint)vertexArray != m_CurVAO && warnBinding(BindingCheckVertexArray))
		nlwarning("3D: state cache: the vertex array object is %u in the cache and %u in GL", (uint)m_CurVAO, (uint)vertexArray);

	value = 0;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &value);
	if ((GLuint)value != m_CurrArrayBuffer && warnBinding(BindingCheckArrayBuffer))
		nlwarning("3D: state cache: the array buffer is %u in the cache and %u in GL", (uint)m_CurrArrayBuffer, (uint)value);

	// The element array buffer and the attributes are state of the vertex array object,
	// and the cache has those of the default one
	if ((GLuint)vertexArray == m_DefaultVAO)
	{
		value = 0;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &value);
		if ((GLuint)value != m_CurElementArrayBuffer && warnBinding(BindingCheckElementArrayBuffer))
			nlwarning("3D: state cache: the element array buffer is %u in the cache and %u in GL", (uint)m_CurElementArrayBuffer, (uint)value);

		for (uint i = 0; i < CVertexBuffer::NumValue; ++i)
		{
			value = 0;
			nglGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &value);
			if ((value != 0) != m_VertexAttribArrayEnabled[i] && warnBinding(BindingCheckAttribEnabled))
				nlwarning("3D: state cache: the array of attribute %u is %s in the cache and %s in GL", i,
				    m_VertexAttribArrayEnabled[i] ? "enabled" : "disabled", value ? "enabled" : "disabled");

			// An attribute that no draw reads, or whose buffer the cache does not know (it was deleted), is not compared
			const CVertexAttribPointer &cached = m_VertexAttribPointer[i];
			if (!m_VertexAttribArrayEnabled[i] || !cached.Buffer)
				continue;

			value = 0;
			nglGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &value);
			if ((GLuint)value != cached.Buffer && warnBinding(BindingCheckAttribBuffer))
				nlwarning("3D: state cache: attribute %u reads buffer %u in the cache and buffer %u in GL", i, (uint)cached.Buffer, (uint)value);

			GLint size = 0, type = 0, normalized = 0, stride = 0;
			nglGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
			nglGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
			nglGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);
			nglGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
			if ((size != cached.Size || (GLenum)type != cached.Type || (normalized != 0) != (cached.Normalized != 0) || stride != cached.Stride)
			    && warnBinding(BindingCheckAttribFormat))
			{
				nlwarning("3D: state cache: attribute %u has size %d, type 0x%x, normalized %d, stride %d in the cache and size %d, type 0x%x, normalized %d, stride %d in GL",
				    i, (sint)cached.Size, (uint)cached.Type, (sint)(cached.Normalized != 0), (sint)cached.Stride,
				    (sint)size, (uint)type, (sint)(normalized != 0), (sint)stride);
			}

			GLvoid *pointer = nullptr;
			nglGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &pointer);
			if (pointer != cached.Pointer && warnBinding(BindingCheckAttribPointer))
				nlwarning("3D: state cache: attribute %u starts at offset %p in the cache and at offset %p in GL", i, cached.Pointer, pointer);
		}
	}

	// --- Textures
	GLint activeTexture = GL_TEXTURE0;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
	if ((uint)(activeTexture - GL_TEXTURE0) != m_CurrentActiveTexture && warnBinding(BindingCheckActiveTexture))
		nlwarning("3D: state cache: the active texture unit is %u in the cache and %u in GL", m_CurrentActiveTexture, (uint)(activeTexture - GL_TEXTURE0));

	// The binding of a unit can only be asked of the active unit. A unit that never
	// had a texture has no target in the cache and is not compared.
	bool unitChanged = false;
	for (uint i = 0; i < MaxTextureUnits; ++i)
	{
		GLenum query;
		switch (m_CurTextureTarget[i])
		{
		case GL_TEXTURE_2D: query = GL_TEXTURE_BINDING_2D; break;
		case GL_TEXTURE_CUBE_MAP: query = GL_TEXTURE_BINDING_CUBE_MAP; break;
		case GL_TEXTURE_RECTANGLE: query = GL_TEXTURE_BINDING_RECTANGLE; break;
		default: continue;
		}
		nglActiveTexture(GL_TEXTURE0 + i);
		unitChanged = true;
		value = 0;
		glGetIntegerv(query, &value);
		if ((GLuint)value != m_CurTexture[i] && warnBinding(BindingCheckTexture))
			nlwarning("3D: state cache: unit %u has texture %u on target 0x%x in the cache and texture %u in GL", i, (uint)m_CurTexture[i], (uint)m_CurTextureTarget[i], (uint)value);
	}
	// back to the unit that GL had, which is the one of the cache unless the line above was logged
	if (unitChanged)
		nglActiveTexture((GLenum)activeTexture);

	// --- Uniform buffers
	value = 0;
	glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &value);
	if ((GLuint)value != m_CurUniformBuffer && warnBinding(BindingCheckUniformBuffer))
		nlwarning("3D: state cache: the uniform buffer is %u in the cache and %u in GL", (uint)m_CurUniformBuffer, (uint)value);

	for (uint i = 0; i < MaxUBOBindings; ++i)
	{
		// (start and size are 0 for a buffer that is bound whole)
		GLint buffer = 0, start = 0, size = 0;
		nglGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, i, &buffer);
		nglGetIntegeri_v(GL_UNIFORM_BUFFER_START, i, &start);
		nglGetIntegeri_v(GL_UNIFORM_BUFFER_SIZE, i, &size);
		if (((GLuint)buffer != m_CurUniformBufferBase[i] || (GLintptr)start != m_CurUniformBufferOffset[i] || (GLsizeiptr)size != m_CurUniformBufferSize[i])
		    && warnBinding(BindingCheckUniformBufferIndexed))
		{
			nlwarning("3D: state cache: uniform buffer binding %u has buffer %u, offset %u, size %u in the cache and buffer %u, offset %u, size %u in GL",
			    i, (uint)m_CurUniformBufferBase[i], (uint)m_CurUniformBufferOffset[i], (uint)m_CurUniformBufferSize[i],
			    (uint)buffer, (uint)start, (uint)size);
		}
	}
}

#endif // NL3D_GLSTATE_CHECK_BINDINGS

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

void CDriverGLStates3::arrayBufferDeleted(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates3_arrayBufferDeleted)
	if (!objectID)
		return;
	if (m_CurrArrayBuffer == objectID)
		m_CurrArrayBuffer = 0;
	// With another VAO bound, GL leaves the buffer in the attributes of the default VAO.
	// The cache forgets it all the same: the name is free for another buffer, and the
	// next call for the attribute is then issued, which is right in both cases.
	for (uint i = 0; i < CVertexBuffer::NumValue; ++i)
	{
		if (m_VertexAttribPointer[i].Buffer == objectID)
			m_VertexAttribPointer[i].Buffer = 0;
	}
}

void CDriverGLStates3::vertexAttribPointer(uint glIndex, GLuint buffer, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
	H_AUTO_OGL(CDriverGLStates3_vertexAttribPointer)
	nlassert(glIndex < CVertexBuffer::NumValue);

	// The cache is the one of the default VAO
	if (m_CurVAO != m_DefaultVAO)
		forceBindVertexArray(m_DefaultVAO);

	CVertexAttribPointer &current = m_VertexAttribPointer[glIndex];
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (!buffer || current.Buffer != buffer || current.Pointer != pointer || current.Stride != stride
	    || current.Type != type || current.Size != size || current.Normalized != normalized)
#endif
	{
		// The attribute takes the buffer that is bound to GL_ARRAY_BUFFER at the call
		bindArrayBuffer(buffer);
		nglVertexAttribPointer(glIndex, size, type, normalized, stride, pointer);
		current.Buffer = buffer;
		current.Size = size;
		current.Type = type;
		current.Normalized = normalized;
		current.Stride = stride;
		current.Pointer = pointer;
	}
}

void CDriverGLStates3::forceBindElementArrayBuffer(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindElementArrayBuffer)

	// The binding belongs to the bound VAO, and the cache is the one of the default VAO
	if (m_CurVAO != m_DefaultVAO)
		forceBindVertexArray(m_DefaultVAO);

	nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	m_CurElementArrayBuffer = id;
}

void CDriverGLStates3::bindElementArrayBuffer(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_bindElementArrayBuffer)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (id != m_CurElementArrayBuffer || m_CurVAO != m_DefaultVAO)
#endif
	{
		forceBindElementArrayBuffer(id);
	}
}

void CDriverGLStates3::elementArrayBufferDeleted(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_elementArrayBufferDeleted)
	// With another VAO bound, GL leaves the buffer in the default VAO. The cache
	// forgets it all the same: the next bind is then issued, which is right in both cases.
	if (id && m_CurElementArrayBuffer == id)
		m_CurElementArrayBuffer = 0;
}

// ===========================================================================
// Texture state
// ===========================================================================

void CDriverGLStates3::activeTexture(uint stage)
{
	H_AUTO_OGL(CDriverGLStates3_activeTexture)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (m_CurrentActiveTexture != stage)
#endif
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

bool CDriverGLStates3::isTextureBound(uint stage, GLenum target, GLuint id) const
{
	nlassert(stage < MaxTextureUnits);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	return target == m_CurTextureTarget[stage] && id == m_CurTexture[stage];
#else
	return false;
#endif
}

void CDriverGLStates3::textureDeleted(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_textureDeleted)
	if (!id)
		return;
	for (uint i = 0; i < MaxTextureUnits; ++i)
	{
		// The target stays: the unit has nothing bound to it
		if (m_CurTexture[i] == id)
			m_CurTexture[i] = 0;
	}
}

void CDriverGLStates3::unbindTexture(uint stage)
{
	H_AUTO_OGL(CDriverGLStates3_unbindTexture)
	nlassert(stage < MaxTextureUnits);
	if (m_CurTexture[stage] != 0)
	{
		activeTexture(stage);
		glBindTexture(m_CurTextureTarget[stage], 0);
		m_CurTexture[stage] = 0;
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
#ifdef NL3D_GLSTATE_CHECK_BINDINGS
	if (id)
		m_ProgramPipelineUsed = true;
#endif
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

void CDriverGLStates3::bindUniformBuffer(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_bindUniformBuffer)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (id != m_CurUniformBuffer)
#endif
	{
		forceBindUniformBuffer(id);
	}
}

void CDriverGLStates3::forceBindUniformBufferBase(GLuint binding, GLuint bufferId)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindUniformBufferBase)
	nglBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId);
	nlassert(binding < MaxUBOBindings);
	m_CurUniformBufferBase[binding] = bufferId;
	m_CurUniformBufferOffset[binding] = 0;
	m_CurUniformBufferSize[binding] = 0;
	// the binding point has what it is to have: a range that was noted for it is dropped
	m_WantUniformBufferBase[binding] = bufferId;
	m_WantUniformBufferOffset[binding] = 0;
	m_WantUniformBufferSize[binding] = 0;
	m_UniformBufferDirty &= ~(1u << binding);
	// the indexed bind commands bind the buffer to the general binding point as well
	m_CurUniformBuffer = bufferId;
}

void CDriverGLStates3::bindUniformBufferBase(GLuint binding, GLuint bufferId)
{
	H_AUTO_OGL(CDriverGLStates3_bindUniformBufferBase)
	nlassert(binding < MaxUBOBindings);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (bufferId != m_CurUniformBufferBase[binding] || m_CurUniformBufferSize[binding] != 0)
#endif
	{
		forceBindUniformBufferBase(binding, bufferId);
	}
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	else
	{
		// The binding point has the buffer already. A range that was noted for it since is
		// dropped: the last one asked for is the one it is to have.
		m_WantUniformBufferBase[binding] = bufferId;
		m_WantUniformBufferOffset[binding] = 0;
		m_WantUniformBufferSize[binding] = 0;
		m_UniformBufferDirty &= ~(1u << binding);
	}
#endif
}

void CDriverGLStates3::forceBindUniformBufferRange(GLuint binding, GLuint bufferId, GLintptr offset, GLsizeiptr size)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindUniformBufferRange)
	nglBindBufferRange(GL_UNIFORM_BUFFER, binding, bufferId, offset, size);
	nlassert(binding < MaxUBOBindings);
	m_CurUniformBufferBase[binding] = bufferId;
	m_CurUniformBufferOffset[binding] = offset;
	m_CurUniformBufferSize[binding] = size;
	m_WantUniformBufferBase[binding] = bufferId;
	m_WantUniformBufferOffset[binding] = offset;
	m_WantUniformBufferSize[binding] = size;
	m_UniformBufferDirty &= ~(1u << binding);
	// the indexed bind commands bind the buffer to the general binding point as well
	m_CurUniformBuffer = bufferId;
}

void CDriverGLStates3::bindUniformBufferRange(GLuint binding, GLuint bufferId, GLintptr offset, GLsizeiptr size)
{
	H_AUTO_OGL(CDriverGLStates3_bindUniformBufferRange)
	nlassert(binding < MaxUBOBindings);

	// Noted, and issued by flushUniformBufferBindings() when it is not what the binding point has
	m_WantUniformBufferBase[binding] = bufferId;
	m_WantUniformBufferOffset[binding] = offset;
	m_WantUniformBufferSize[binding] = size;
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (bufferId != m_CurUniformBufferBase[binding]
	    || offset != m_CurUniformBufferOffset[binding]
	    || size != m_CurUniformBufferSize[binding])
#endif
	{
		m_UniformBufferDirty |= (1u << binding);
	}
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	else
	{
		m_UniformBufferDirty &= ~(1u << binding);
	}
#endif
}

void CDriverGLStates3::flushUniformBufferBindings()
{
	if (!m_UniformBufferDirty)
		return;

	H_AUTO_OGL(CDriverGLStates3_flushUniformBufferBindings)

	for (uint i = 0; i < MaxUBOBindings; ++i)
	{
		if (!(m_UniformBufferDirty & (1u << i)))
			continue;

#if NL3D_GL3_MULTI_BIND && !defined(USE_OPENGLES3)
		if (m_MultiBind)
		{
			// The binding points from this one to the last one that changes, as far as each
			// of them is to have a range of a buffer: glBindBuffersRange takes no whole
			// buffer, and for no buffer it wants a size all the same
			uint last = i;
			for (uint j = i; j < MaxUBOBindings; ++j)
			{
				if (!m_WantUniformBufferBase[j] || m_WantUniformBufferSize[j] <= 0)
					break;
				if (m_UniformBufferDirty & (1u << j))
					last = j;
			}
			if (last > i)
			{
				// One call for them. The binding points between them that do not change are
				// given the range they have. The general binding point stays as it is.
				nglBindBuffersRange(GL_UNIFORM_BUFFER, i, (GLsizei)(last - i + 1),
				    &m_WantUniformBufferBase[i], &m_WantUniformBufferOffset[i], &m_WantUniformBufferSize[i]);
				for (uint j = i; j <= last; ++j)
				{
					m_CurUniformBufferBase[j] = m_WantUniformBufferBase[j];
					m_CurUniformBufferOffset[j] = m_WantUniformBufferOffset[j];
					m_CurUniformBufferSize[j] = m_WantUniformBufferSize[j];
					m_UniformBufferDirty &= ~(1u << j);
				}
				i = last;
				continue;
			}
		}
#endif

		// (clears the bit)
		forceBindUniformBufferRange(i, m_WantUniformBufferBase[i], m_WantUniformBufferOffset[i], m_WantUniformBufferSize[i]);
	}
}

void CDriverGLStates3::uniformBufferDeleted(GLuint id)
{
	H_AUTO_OGL(CDriverGLStates3_uniformBufferDeleted)
	if (!id)
		return;
	if (m_CurUniformBuffer == id)
		m_CurUniformBuffer = 0;
	for (uint i = 0; i < MaxUBOBindings; ++i)
	{
		if (m_CurUniformBufferBase[i] == id)
		{
			m_CurUniformBufferBase[i] = 0;
			m_CurUniformBufferOffset[i] = 0;
			m_CurUniformBufferSize[i] = 0;
		}
		if (m_WantUniformBufferBase[i] == id)
		{
			// A range of the buffer that is noted for the binding point is not issued:
			// the binding point keeps what it has
			m_WantUniformBufferBase[i] = m_CurUniformBufferBase[i];
			m_WantUniformBufferOffset[i] = m_CurUniformBufferOffset[i];
			m_WantUniformBufferSize[i] = m_CurUniformBufferSize[i];
			m_UniformBufferDirty &= ~(1u << i);
		}
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
