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

#ifndef NL_DRIVER_OPENGL_STATES_H
#define NL_DRIVER_OPENGL_STATES_H

#include "nel/misc/types_nl.h"
#include "nel/3d/vertex_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

// ***************************************************************************
/**
 * Class for optimizing calls to openGL states, by caching old ones.
 *	All following call with OpenGL must be done with only one instance of this class:
		- glEnable() glDisable() with:
			- GL_BLEND
			- GL_CULL_FACE
			- GL_ALPHA_TEST
			- GL_LIGHTING
			- GL_LIGHT0 + i .....
			- GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP/OES.
			- GL_TEXTURE_GEN_S, GL_TEXTURE_GEN_T, GL_TEXTURE_GEN_R
			- GL_COLOR_MATERIAL
			- GL_FOG
			- GL_MULTISAMPLE_ARB
		- glActiveTexture()
		- glClientActiveTexture()
		- glEnableClientState() glDisableClientState() with:
			- GL_VERTEX_ARRAY
			- GL_NORMAL_ARRAY
			- GL_VERTEX_WEIGHTING_EXT
			- GL_COLOR_ARRAY
			- GL_SECONDARY_COLOR_ARRAY_EXT
			- GL_TEXTURE_COORD_ARRAY
			- GL_VERTEX_ATTRIB_ARRAY0_NV + i.
		- glDepthMask()
		- glAlphaFunc()
		- glBlendFunc()
		- glDepthFunc()
		- glMaterialf() and glMaterialfv() for:
			- GL_EMISSION
			- GL_AMBIENT
			- GL_DIFFUSE
			- GL_SPECULAR
			- GL_SHININESS
		- glDepthRange()
		- glColorMaterial()
		- glTexGeni()
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CDriverGLStates3
{
public:
	/// Constructor. no-op.
	CDriverGLStates3();
	// init. Do it just after setDisplay()
	void			init();

	/// Reset all OpenGL states of interest to default, and update caching. This don't apply to light.
	void			forceDefaults(uint nbTextureStages);

	/// \name enable if !0
	// @{
	void			enableBlend(uint enable);
	void			enableCullFace(uint enable);
	/// enable and set good AlphaFunc.
	void			enableZWrite(uint enable);
	/// enable/disable stencil test
	void			enableStencilTest(bool enable);
	bool			isStencilTestEnabled() const { return _CurStencilTest; }
	// @}

	/// glBlendFunc.
	void			blendFunc(GLenum src, GLenum dst);
	/// glDepthFunc.
	void			depthFunc(GLenum zcomp);
	/// glStencilFunc
	void			stencilFunc(GLenum stencilFunc, GLint ref, GLuint mask);
	/// glStencilOp
	void			stencilOp(GLenum fail, GLenum zfail, GLenum zpass);
	/// glStencilMask
	void			stencilMask(uint mask);

	/// \name Material setting.
	/// Each f() get an uint32 for fast comparison, and OpenGL colors.
	// @{
	void			setDepthRange (float znear, float zfar);
	void			getDepthRange(float &znear, float &zfar) const { znear = _DepthRangeNear; zfar = _DepthRangeFar; }
	/** Set z-bias
      * NB : this is done in window coordinate, not in world coordinate as with CMaterial
	  */
	void			setZBias(float zbias);
	// @}



	/// \name Texture Mode setting.
	// @{
	/// same as glActiveTexture(). useful for setTextureMode.
	void			activeTexture(uint stage);
	/// same as active texture arb, but with no cache check
	void			forceActiveTexture(uint stage);
	/// get active texture
	uint			getActiveTexture() const { return _CurrentActiveTexture; }
	// @}

	// special version for ARB_vertex_program used with ARB_vertex_buffer or ATI_vertex_attrib_array_object
	void			enableVertexAttribArrayARB(uint glIndex, bool enable);


	// ARB_vertex_buffer_object buffer binding
	void			bindARBVertexBuffer(uint objectID);
	void			forceBindARBVertexBuffer(uint objectID);
	uint			getCurrBoundARBVertexBuffer() const { return _CurrARBVertexBuffer; }

	enum TCullMode  { CCW = 0, CW };
	void			setCullMode(TCullMode cullMode);
	TCullMode       getCullMode() const;

private:
	bool			_CurBlend;
	bool			_CurCullFace;
	bool			_CurZWrite;
	bool			_CurStencilTest;

	GLenum			_CurBlendSrc;
	GLenum			_CurBlendDst;
	GLenum			_CurDepthFunc;
	GLenum			_CurStencilFunc;
	GLint			_CurStencilRef;
	GLuint			_CurStencilMask;
	GLenum			_CurStencilOpFail;
	GLenum			_CurStencilOpZFail;
	GLenum			_CurStencilOpZPass;
	GLuint			_CurStencilWriteMask;

	uint			_CurrentActiveTexture;

	bool			_VertexAttribArrayEnabled[CVertexBuffer::NumValue];

	uint			_CurrARBVertexBuffer;

	float			_DepthRangeNear;
	float			_DepthRangeFar;
	float			_ZBias; // NB : zbias is in window coordinates

	TCullMode		_CullMode;

private:
	void			updateDepthRange();
};

} // NLDRIVERGL3
} // NL3D


#endif // NL_DRIVER_OPENGL_STATES_H

/* End of driver_opengl_states.h */
