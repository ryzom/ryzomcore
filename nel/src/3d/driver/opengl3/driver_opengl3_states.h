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

#ifndef NL_DRIVER_OPENGL3_STATES_H
#define NL_DRIVER_OPENGL3_STATES_H

#include "nel/misc/types_nl.h"
#include "nel/3d/vertex_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

/**
 * @brief GL3 core profile state cache.
 *
 * Optimizes GL state changes by caching previous values and skipping
 * redundant calls. All GL state listed below must go through this class;
 * direct GL calls will desynchronize the cache.
 *
 * Define @c NL3D_GLSTATE_DISABLE_CACHE to bypass all cache checks
 * (forces every GL call through, useful for debugging).
 *
 * @par Managed GL calls
 * - Rasterizer: glEnable/glDisable(GL_CULL_FACE), glCullFace(), glPolygonMode()
 * - Fragment ops: glEnable/glDisable(GL_BLEND), glBlendFunc(),
 *   glDepthMask(), glDepthFunc(),
 *   glEnable/glDisable(GL_STENCIL_TEST), glStencilFunc(), glStencilOp(), glStencilMask(),
 *   glColorMask()
 * - Viewport/scissor: glViewport(), glEnable/glDisable(GL_SCISSOR_TEST), glScissor()
 * - Depth range: glDepthRange()
 * - Clip planes: glEnable/glDisable(GL_CLIP_DISTANCE0..5)
 * - Vertex input: glEnableVertexAttribArray(), glDisableVertexAttribArray(),
 *   glBindBuffer(GL_ARRAY_BUFFER)
 * - Textures: glActiveTexture(), glBindTexture()
 * - Framebuffer: glBindFramebuffer(GL_FRAMEBUFFER)
 * - Shader programs: glUseProgram(), glBindProgramPipeline()
 * - Buffer objects: glBindBuffer(GL_UNIFORM_BUFFER),
 *   glBindBufferBase(GL_UNIFORM_BUFFER),
 *   glBindBuffer(GL_PIXEL_UNPACK_BUFFER)
 */
class CDriverGLStates3
{
public:
	/// @name Lifecycle
	/// @{

	CDriverGLStates3();

	/**
	 * @brief Create and bind the single default VAO required by core profile.
	 *
	 * Must be called once after the GL context is current and extensions are loaded.
	 * The driver uses exactly one VAO for its entire lifetime; vertex attrib
	 * enable/disable state (which is per-VAO in GL 3.3) is cached here as if
	 * it were context state since there is only ever one VAO.
	 */
	void init();

	/// Delete the default VAO. Call while the GL context is still current.
	void release();

	/**
	 * @brief Reset cached rendering state to defaults and update the GL context.
	 *
	 * Resets: blend, cull, depth write, stencil, scissor test, color mask,
	 * polygon mode, clip distances, blend/depth/stencil functions,
	 * depth range, and cull order.
	 *
	 * Does @b not reset: active texture unit, viewport, scissor rect,
	 * vertex attrib arrays, buffer bindings, texture bindings, framebuffer,
	 * or shader programs (these are caller-specific or managed by their
	 * own subsystem).
	 */
	void forceDefaults();

	/// @}

	// =========================================================================
	// Rendering state -- reset by forceDefaults()
	// =========================================================================

	/// @name Rasterizer state
	/// @{

	enum TCullMode { CCW = 0, CW };

	/// glEnable/glDisable(GL_CULL_FACE).
	void enableCullFace(uint enable);

	/// glCullFace(). CCW = GL_BACK, CW = GL_FRONT.
	void setCullMode(TCullMode cullMode);
	TCullMode getCullMode() const;

	/// glPolygonMode(GL_FRONT_AND_BACK, mode).
	void polygonMode(GLenum mode);

	/// @}

	/// @name Fragment operations
	/// @{

	/// glEnable/glDisable(GL_BLEND).
	void enableBlend(uint enable);

	/// glBlendFunc().
	void blendFunc(GLenum src, GLenum dst);

	/// glBlendColor().
	void blendColor(NLMISC::CRGBA color);
	NLMISC::CRGBA getBlendColor() const { return m_CurBlendColor; }

	/// glDepthMask().
	void enableZWrite(uint enable);

	/// glDepthFunc().
	void depthFunc(GLenum zcomp);

	/// glEnable/glDisable(GL_STENCIL_TEST).
	void enableStencilTest(bool enable);
	bool isStencilTestEnabled() const { return m_CurStencilTest; }

	/// glStencilFunc().
	void stencilFunc(GLenum stencilFunc, GLint ref, GLuint mask);

	/// glStencilOp().
	void stencilOp(GLenum fail, GLenum zfail, GLenum zpass);

	/// glStencilMask().
	void stencilMask(uint mask);

	/// glColorMask().
	void colorMask(bool red, bool green, bool blue, bool alpha);

	/// @}

	/// @name Scissor test
	/// @{

	/// glEnable/glDisable(GL_SCISSOR_TEST).
	void enableScissorTest(bool enable);

	/// @}

	/// @name Depth range
	/// @{

	/// glDepthRange(). Applies z-bias offset internally.
	void setDepthRange(float znear, float zfar);

	void getDepthRange(float &znear, float &zfar) const
	{
		znear = m_DepthRangeNear;
		zfar = m_DepthRangeFar;
	}

	/// Set z-bias in window coordinates (not world coordinates as with CMaterial).
	void setZBias(float zbias);

	/// @}

	/// @name Clip planes
	/// @{

	/// Set PP clip plane mode. When true, all GL_CLIP_DISTANCE calls are skipped
	/// because clip planes are handled in the pixel program via discard.
	void setPPClipPlanes(bool pp) { m_PPClipPlanes = pp; }

	/// glEnable/glDisable(GL_CLIP_DISTANCE0 + index). Index must be < 6.
	/// Skipped when PP clip plane mode is active.
	void enableClipDistance(uint index, bool enable);

	/// @}

	// =========================================================================
	// Geometry state -- NOT reset by forceDefaults()
	// Caller-specific; tied to the window or render target dimensions.
	// =========================================================================

	/// @name Viewport and scissor rect
	/// @{

	/// glViewport().
	void viewport(sint x, sint y, sint width, sint height);

	/// glScissor().
	void scissor(sint x, sint y, sint width, sint height);

	/// @}

	// =========================================================================
	// Binding state -- NOT reset by forceDefaults()
	// These are caller-specific or managed by their own subsystem.
	// =========================================================================

	/// @name Vertex input
	/// @{
	/// @note Vertex attrib enable/disable is per-VAO state in GL 3.3 core profile,
	/// but is cached here as context state because the driver uses a single default VAO.

	/// glBindVertexArray(). Bind a VAO other than the default.
	/// Attrib enable cache is only valid for the default VAO.
	void bindVertexArray(GLuint id);
	void forceBindVertexArray(GLuint id);

	/// glEnableVertexAttribArray() / glDisableVertexAttribArray().
	/// Operates on the default VAO only. If another VAO is currently bound,
	/// the default VAO will be rebound before modifying attrib state.
	void enableVertexAttribArray(uint glIndex, bool enable);

	/// glBindBuffer(GL_ARRAY_BUFFER). Context state, not per-VAO.
	void bindArrayBuffer(uint objectID);
	void forceBindArrayBuffer(uint objectID);
	uint getCurrBoundArrayBuffer() const { return m_CurrArrayBuffer; }

	/// @}

	/// @name Texture state
	/// @{
	/// @note activeTexture() is an API selector (determines which unit glBindTexture
	/// targets), not rendering state. All callers explicitly set the unit before
	/// texture work.

	/// glActiveTexture(GL_TEXTURE0 + stage).
	void activeTexture(uint stage);
	uint getActiveTexture() const { return m_CurrentActiveTexture; }

	/// glBindTexture(). Operates on the current active texture unit.
	/// Cache is keyed by both target and id per unit.
	void bindTexture(GLenum target, GLuint id);
	void forceBindTexture(GLenum target, GLuint id);

	/// @}

	/// @name Framebuffer
	/// @{

	/// glBindFramebuffer(GL_FRAMEBUFFER).
	void bindFramebuffer(GLuint id);
	void forceBindFramebuffer(GLuint id);
	GLuint getCurrBoundFramebuffer() const { return m_CurFramebuffer; }

	/// @}

	/// @name Shader programs
	/// @{

	/// glUseProgram().
	void useProgram(GLuint id);
	void forceUseProgram(GLuint id);
	GLuint getCurrUsedProgram() const { return m_CurProgram; }

	/// glBindProgramPipeline().
	void bindProgramPipeline(GLuint id);
	void forceBindProgramPipeline(GLuint id);
	GLuint getCurrBoundProgramPipeline() const { return m_CurProgramPipeline; }

	/// @}

	/// @name Buffer objects
	/// @{

	/// glBindBuffer(GL_UNIFORM_BUFFER). For data upload; not an indexed binding point.
	void forceBindUniformBuffer(GLuint id); // TODO: Not needed here in state, see pixel and some other upload bindings
	GLuint getCurrBoundUniformBuffer() const { return m_CurUniformBuffer; }

	/// glBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId). Indexed binding points.
	void bindUniformBufferBase(GLuint binding, GLuint bufferId);
	void forceBindUniformBufferBase(GLuint binding, GLuint bufferId);
	GLuint getCurrBoundUniformBufferBase(GLuint binding) const { nlassert(binding < MaxUBOBindings); return m_CurUniformBufferBase[binding]; }

	/// glBindBuffer(GL_PIXEL_UNPACK_BUFFER). For texture upload via PBO.
	void bindPixelUnpackBuffer(GLuint id);
	void forceBindPixelUnpackBuffer(GLuint id);
	GLuint getCurrBoundPixelUnpackBuffer() const { return m_CurPixelUnpackBuffer; }

	/// @}

private:
	// =========================================================================
	// Rendering state (reset by forceDefaults)
	// =========================================================================

	// Rasterizer
	bool m_CurCullFace;
	TCullMode m_CullMode;
	GLenum m_CurPolygonMode;

	// Fragment operations
	bool m_CurBlend;
	GLenum m_CurBlendSrc;
	GLenum m_CurBlendDst;
	NLMISC::CRGBA m_CurBlendColor;

	bool m_CurZWrite;
	GLenum m_CurDepthFunc;

	bool m_CurStencilTest;
	GLenum m_CurStencilFunc;
	GLint m_CurStencilRef;
	GLuint m_CurStencilMask;
	GLenum m_CurStencilOpFail;
	GLenum m_CurStencilOpZFail;
	GLenum m_CurStencilOpZPass;
	GLuint m_CurStencilWriteMask;

	bool m_CurColorMaskR, m_CurColorMaskG, m_CurColorMaskB, m_CurColorMaskA;

	// Scissor test
	bool m_CurScissorTest;

	// Depth range
	float m_DepthRangeNear;
	float m_DepthRangeFar;
	float m_ZBias;

	void updateDepthRange();

	// Clip planes
	enum { MaxClipDistances = 6 };
	bool m_CurClipDistance[MaxClipDistances];
	bool m_PPClipPlanes;

	// =========================================================================
	// Geometry state (NOT reset by forceDefaults)
	// =========================================================================

	// Viewport and scissor rect
	sint m_CurViewportX, m_CurViewportY, m_CurViewportWidth, m_CurViewportHeight;
	sint m_CurScissorX, m_CurScissorY, m_CurScissorWidth, m_CurScissorHeight;

	// =========================================================================
	// Binding state (NOT reset by forceDefaults)
	// =========================================================================

	// Vertex input
	GLuint m_DefaultVAO;
	GLuint m_CurVAO;
	bool m_VertexAttribArrayEnabled[CVertexBuffer::NumValue];
	uint m_CurrArrayBuffer;

	// Texture state
	uint m_CurrentActiveTexture;
	enum { MaxTextureUnits = 8 };
	GLenum m_CurTextureTarget[MaxTextureUnits];
	GLuint m_CurTexture[MaxTextureUnits];

	// Framebuffer
	GLuint m_CurFramebuffer;

	// Shader programs
	GLuint m_CurProgram;
	GLuint m_CurProgramPipeline;

	// Buffer objects
	GLuint m_CurUniformBuffer;
	enum { MaxUBOBindings = 8 };
	GLuint m_CurUniformBufferBase[MaxUBOBindings];
	GLuint m_CurPixelUnpackBuffer;
};

} // NLDRIVERGL3
} // NL3D

#endif // NL_DRIVER_OPENGL3_STATES_H

/* end of file */
