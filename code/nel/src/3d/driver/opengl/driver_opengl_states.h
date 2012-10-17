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

#ifndef NL_DRIVER_OPENGL_STATES_H
#define NL_DRIVER_OPENGL_STATES_H

#include "nel/misc/types_nl.h"
#include "nel/3d/vertex_buffer.h"

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

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
			- GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_ARB/OES.
			- GL_TEXTURE_GEN_S, GL_TEXTURE_GEN_T, GL_TEXTURE_GEN_R
			- GL_COLOR_MATERIAL
			- GL_FOG
			- GL_MULTISAMPLE_ARB
		- glActiveTextureARB()
		- glClientActiveTextureARB()
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
class CDriverGLStates
{
public:
	/// Constructor. no-op.
	CDriverGLStates();
	// init. Do it just after setDisplay()
	void			init(bool supportTextureCubeMap, bool supportTextureRectangle, uint maxLight);

	/// Reset all OpenGL states of interest to default, and update caching. This don't apply to light.
	void			forceDefaults(uint nbTextureStages);

	/// \name enable if !0
	// @{
	void			enableBlend(uint enable);
	void			enableFog(uint enable);
	void			enableCullFace(uint enable);
	/// enable and set good AlphaFunc.
	void			enableAlphaTest(uint enable);
	void			enableZWrite(uint enable);
	// overall lighting enabled
	void			enableLighting(uint enable);
	bool			isLightingEnabled() const { return _CurLighting; }
	/// enable/disable specific light. num must be < "maxLight" param set in init()
	void			enableLight(uint num, uint enable);
	bool			isLightEnabled(uint num) const;
	/// enable/disable stencil test
	void			enableStencilTest(bool enable);
	bool			isStencilTestEnabled() const { return _CurStencilTest; }
	/// enable/disable multisample
	void			enableMultisample(bool enable);
	bool			isMultisampleEnabled() const { return _CurMultisample; }
	// @}

	/// glBlendFunc.
	void			blendFunc(GLenum src, GLenum dst);
	/// glDepthFunc.
	void			depthFunc(GLenum zcomp);
	/// glAlphaFunc
	void			alphaFunc(float threshold);
	/// glStencilFunc
	void			stencilFunc(GLenum stencilFunc, GLint ref, GLuint mask);
	/// glStencilOp
	void			stencilOp(GLenum fail, GLenum zfail, GLenum zpass);
	/// glStencilMask
	void			stencilMask(uint mask);

	/// \name Material setting.
	/// Each f() get an uint32 for fast comparison, and OpenGL colors.
	// @{
	void			setEmissive(uint32 packedColor, const GLfloat color[4]);
	void			setAmbient(uint32 packedColor, const GLfloat color[4]);
	void			setDiffuse(uint32 packedColor, const GLfloat color[4]);
	void			setSpecular(uint32 packedColor, const GLfloat color[4]);
	void			setShininess(float shin);
	void			setVertexColorLighted(bool enable);
	void			setDepthRange (float znear, float zfar);
	void			getDepthRange(float &znear, float &zfar) const { znear = _DepthRangeNear; zfar = _DepthRangeFar; }
	/** Set z-bias
      * NB : this is done in window coordinate, not in world coordinate as with CMaterial
	  */
	void			setZBias(float zbias);
	// NB: set 0 to reset TexGen.
	void			setTexGenMode (uint stage, GLint mode);
	// @}



	/// \name Texture Mode setting.
	// @{
	enum			TTextureMode {TextureDisabled, Texture2D, TextureRect, TextureCubeMap, TextureModeCount};
	/// same as glActiveTextureARB(). useful for setTextureMode.
	void			activeTextureARB(uint stage);
	/// same as active texture arb, but with no cache check
	void			forceActiveTextureARB(uint stage);
	/// get active texture
	uint			getActiveTextureARB() const { return _CurrentActiveTextureARB; }
	/** change if needed the texture mode of the current active Texture ARB.
	 *	NB: if CubeMap extension not supported, TextureCubeMap <=> TextureDisabled.
	 */
	void			setTextureMode(TTextureMode texMode);
	TTextureMode	getTextureMode() const { return _TextureMode[_CurrentActiveTextureARB]; }
	// reset texture mode to the default (disabled) for the current stage. It forces the state (useful after texture shaders)
	void			resetTextureMode();
	// @}

	/// \name Vertex Array enabling.
	/// equivalent to glEnableClientState() / glDisableClientState(). NB: Not modified by forceDefaults()
	// @{
	void			enableVertexArray(bool enable);
	void			enableNormalArray(bool enable);
	void			enableWeightArray(bool enable);
	void			enableColorArray(bool enable);
	void			enableSecondaryColorArray(bool enable);
	/// same as glClientActiveTextureARB(). useful for enableTexCoordArray.
	void			clientActiveTextureARB(uint stage);
	/// NB: caller must call correct clientActiveTextureARB() before.
	void			enableTexCoordArray(bool enable);
	/** For vertexProgram. do not check if supported or not.
	  */
	void			enableVertexAttribArray(uint glIndex, bool enable);
	/** Same as enableVertexAttribArray, but for EXTVertexShader (need variant ID)
	  * \param firstVariantSymbol the first variant symbol
	  */
	void			enableVertexAttribArrayForEXTVertexShader(uint glIndex, bool enable, uint *variants);

	// special version for ARB_vertex_program used with ARB_vertex_buffer or ATI_vertex_attrib_array_object
	void			enableVertexAttribArrayARB(uint glIndex, bool enable);


	// @}


	// ARB_vertex_buffer_object buffer binding
	void			bindARBVertexBuffer(uint objectID);
	void			forceBindARBVertexBuffer(uint objectID);
	uint			getCurrBoundARBVertexBuffer() const { return _CurrARBVertexBuffer; }

	enum TCullMode  { CCW = 0, CW };
	void			setCullMode(TCullMode cullMode);
	TCullMode       getCullMode() const;

private:
	bool			_CurBlend;
	bool			_CurFog;
	bool			_CurCullFace;
	bool			_CurAlphaTest;
	bool			_CurLighting;
	bool			_CurZWrite;
	bool			_CurStencilTest;
	bool			_CurMultisample;

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
	float			_CurAlphaTestThreshold;

	uint32			_CurEmissive;
	uint32			_CurAmbient;
	uint32			_CurDiffuse;
	uint32			_CurSpecular;
	float			_CurShininess;
	bool			_VertexColorLighted;

	bool			_TextureCubeMapSupported;
	bool			_TextureRectangleSupported;
	uint			_CurrentActiveTextureARB;
	TTextureMode	_TextureMode[8];

	bool			_VertexArrayEnabled;
	bool			_NormalArrayEnabled;
	bool			_WeightArrayEnabled;
	bool			_ColorArrayEnabled;
	bool			_SecondaryColorArrayEnabled;
	uint			_CurrentClientActiveTextureARB;
	bool			_TexCoordArrayEnabled[8];
	bool			_VertexAttribArrayEnabled[CVertexBuffer::NumValue];

	GLint			_TexGenMode[8];

	uint			_CurrARBVertexBuffer;

	float			_DepthRangeNear;
	float			_DepthRangeFar;
	float			_ZBias; // NB : zbias is in window coordinates

	TCullMode		_CullMode;

private:
	void updateDepthRange();

	// Mirror of glEnable() and GL_LIGHT0+i
	enum	{MaxLight=8};
	uint			_MaxDriverLight;
	bool			_CurLight[MaxLight];
};

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D


#endif // NL_DRIVER_OPENGL_STATES_H

/* End of driver_opengl_states.h */
