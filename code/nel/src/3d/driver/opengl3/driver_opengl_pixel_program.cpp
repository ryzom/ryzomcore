// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  by authors
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

#include <sstream>

#include "nel/3d/vertex_program.h"
#include "nel/3d/light.h"

#include "driver_opengl.h"
#include "driver_opengl_program.h"
#include "driver_opengl_vertex_buffer_hard.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

namespace /* anonymous */ {

uint maxTextures(CMaterial::TShader shader)
{
	switch (shader)
	{
	case CMaterial::Specular:
	case CMaterial::UserColor: // UserColor has the same texture set up twice
		return 2;
	default:
		return IDRV_MAT_MAXTEXTURES;
	}
}

bool useTexEnv(CMaterial::TShader shader)
{
	return shader == CMaterial::Normal
		|| shader == CMaterial::UserColor;
}

bool useTex(const CPPBuiltin &desc, uint stage)
{
	return (desc.TextureActive & (1 << stage)) != 0;
}

} /* anonymous namespace */

bool operator<(const CPPBuiltin &left, const CPPBuiltin &right)
{	
	// Material state
	if (left.Shader != right.Shader)
		return left.Shader < right.Shader;
	if (left.Flags != right.Flags)
		return left.Flags < right.Flags;
	if (left.TextureActive != right.TextureActive)
		return left.TextureActive < right.TextureActive;
	uint maxTex = maxTextures(left.Shader);
	if (left.TextureActive)
		for (uint stage = 0; stage < maxTex; ++stage)
			if (useTex(left, stage))
				if (left.TexSamplerMode[stage] != right.TexSamplerMode[stage])
					return left.TexSamplerMode[stage] < right.TexSamplerMode[stage];
	if (useTexEnv(left.Shader))
		for (uint stage = 0; stage < maxTex; ++stage)
			if (left.TexEnvMode[stage] != right.TexEnvMode[stage])
				return left.TexEnvMode[stage] < right.TexEnvMode[stage];

	// Driver state
	if (left.VertexFormat != right.VertexFormat)
		return left.VertexFormat < right.VertexFormat;
	if (left.Fog != right.Fog)
		return right.Fog;

	return false;
}

namespace /* anonymous */ {

const char *s_ShaderNames[] = 
{
	"Normal",
	"Bump",
	"Usercolor",
	"Lightmap",
	"Specular",
	"Caustics",
	"Per-Pixel Lighting",
	"Per-Pixel Lighting, no specular",
	"Cloud",
	"Water"
};

CMaterial::TShader getSupportedShader(CMaterial::TShader shader)
{
	switch (shader)
	{
	case CMaterial::Normal:
	case CMaterial::UserColor:
	case CMaterial::Specular:
	case CMaterial::LightMap:
		return shader;
	default:
		return CMaterial::Normal;
	}
}
	
void ppTexEnv(std::stringstream &ss, const CPPBuiltin &desc)
{
	uint maxTex = maxTextures(desc.Shader);
	CMaterial::CTexEnv texEnv;
	for (uint stage = 0; stage < maxTex; ++stage)
	{
		if (useTex(desc, stage))
		{
			texEnv.EnvPacked = desc.TexEnvMode[stage];
			for (uint arg = 0; arg < 3; ++arg)
			{
				// Texop arg
				ss << "vec4 texop" << stage << "arg" << arg << ";" << std::endl;

				// RGB
				uint rgbArg = texEnv.getColorArg(arg);
				uint rgbOp = texEnv.getColorOperand(arg);
				std::stringstream rgbArgVec;
				switch (rgbArg)
				{
				case CMaterial::Texture:
					rgbArgVec << "texel" << stage;
					break;
				case CMaterial::Previous:
					if (stage > 0)
					{
						rgbArgVec << "texop" << (stage - 1);
						break;
					}
				case CMaterial::Diffuse:
					rgbArgVec << "fragColor";
					break;
				case CMaterial::Constant:
					rgbArgVec << "constant" << stage;
					break;
				}
				ss << "texop" << stage << "arg" << arg << ".rgb = ";
				switch (rgbOp) // SrcColor=0, InvSrcColor, SrcAlpha, InvSrcAlpha
				{
				case CMaterial::SrcColor:
					ss << rgbArgVec.str() << ".rgb";
					break;
				case CMaterial::InvSrcColor:
					ss << "vec3(1.0, 1.0, 1.0) - " << rgbArgVec.str() << ".rgb";
					break;
				case CMaterial::SrcAlpha:
					ss << rgbArgVec.str() << ".aaa";
					break;
				case CMaterial::InvSrcAlpha:
					ss << "(1.0 - " << rgbArgVec.str() << ").aaa";
					break;
				}
				ss << ";" << std::endl;

				// Alpha
				uint alphaArg = texEnv.getAlphaArg(arg);
				uint alphaOp = texEnv.getAlphaOperand(arg);
				std::stringstream alphaArgVec;
				switch (alphaArg)
				{
				case CMaterial::Texture:
					alphaArgVec << "texel" << stage;
					break;
				case CMaterial::Previous:
					if (stage > 0)
					{
						alphaArgVec << "texop" << (stage - 1);
						break;
					}
				case CMaterial::Diffuse:
					alphaArgVec << "fragColor";
					break;
				case CMaterial::Constant:
					alphaArgVec << "constant" << stage;
					break;
				}
				ss << "texop" << stage << "arg" << arg << ".a = ";
				switch (alphaOp) // SrcColor=0, InvSrcColor, SrcAlpha, InvSrcAlpha
				{
				case CMaterial::SrcColor:
					ss << alphaArgVec.str() << ".r";
					break;
				case CMaterial::InvSrcColor:
					ss << "1.0 - " << alphaArgVec.str() << ".r";
					break;
				case CMaterial::SrcAlpha:
					ss << alphaArgVec.str() << ".a";
					break;
				case CMaterial::InvSrcAlpha:
					ss << "1.0 - " << alphaArgVec.str() << ".a";
					break;
				}
				ss << ";" << std::endl;
			}
			ss << "vec4 texop" << stage << ";" << std::endl;

			// RGB
			switch (texEnv.Env.OpRGB)
			{
			case CMaterial::InterpolateConstant:
				ss << "float texop" << stage << "rgbAs = constant" << stage << ".a;" << std::endl;
				break;
			case CMaterial::InterpolatePrevious:
				if (stage > 0)
				{
					ss << "float texop" << stage << "rgbAs = texop" << (stage - 1) << ".a;" << std::endl;
					break;
				}
			case CMaterial::InterpolateDiffuse:
				ss << "float texop" << stage << "rgbAs = fragColor.a;" << std::endl;
				break;
			case CMaterial::InterpolateTexture:
				ss << "float texop" << stage << "rgbAs = texel" << stage << ".a;" << std::endl;
				break;
			}
			ss << "texop" << stage << ".rgb = ";
			switch (texEnv.Env.OpRGB)
			{
			case CMaterial::Replace:
				ss << "texop" << stage << "arg0.rgb";
				break;
			case CMaterial::Modulate:
				ss << "texop" << stage << "arg0.rgb * texop" << stage << "arg1.rgb";
				break;
			case CMaterial::Add:
				ss << "texop" << stage << "arg0.rgb + texop" << stage << "arg1.rgb";
				break;
			case CMaterial::AddSigned:
				ss << "texop" << stage << "arg0.rgb + texop" << stage << "arg1.rgb - vec3(0.5, 0.5, 0.5)";
				break;
			case CMaterial::InterpolateConstant:
			case CMaterial::InterpolateDiffuse:
			case CMaterial::InterpolatePrevious:
			case CMaterial::InterpolateTexture:
				ss << "texop" << stage << "arg0.rgb * texop" << stage << "rgbAs + texop" << stage << "arg1.rgb * (1.0 - texop" << stage << "rgbAs)";
				break;
			case CMaterial::Mad:
				ss << "texop" << stage << "arg0.rgb * texop" << stage << "arg1.rgb + texop" << stage << "arg2.rgb";
				break;
			}
			ss << ";" << std::endl;

			// Alpha
			switch (texEnv.Env.OpAlpha)
			{
			case CMaterial::InterpolateConstant:
				ss << "float texop" << stage << "alphaAs = constant" << stage << ".a;" << std::endl;
				break;
			case CMaterial::InterpolatePrevious:
				if (stage > 0)
				{
					ss << "float texop" << stage << "alphaAs = texop" << (stage - 1) << ".a;" << std::endl;
					break;
				}
			case CMaterial::InterpolateDiffuse:
				ss << "float texop" << stage << "alphaAs = fragColor.a;" << std::endl;
				break;
			case CMaterial::InterpolateTexture:
				ss << "float texop" << stage << "alphaAs = texel" << stage << ".a;" << std::endl;
				break;
			}
			ss << "texop" << stage << ".a = ";
			switch (texEnv.Env.OpAlpha)
			{
			case CMaterial::Replace:
				ss << "texop" << stage << "arg0.a";
				break;
			case CMaterial::Modulate:
				ss << "texop" << stage << "arg0.a * texop" << stage << "arg1.a";
				break;
			case CMaterial::Add:
				ss << "texop" << stage << "arg0.a + texop" << stage << "arg1.a";
				break;
			case CMaterial::AddSigned:
				ss << "texop" << stage << "arg0.a + texop" << stage << "arg1.a - 0.5";
				break;
			case CMaterial::InterpolateConstant:
			case CMaterial::InterpolateDiffuse:
			case CMaterial::InterpolatePrevious:
			case CMaterial::InterpolateTexture:
				ss << "texop" << stage << "arg0.a * texop" << stage << "rgbAs + texop" << stage << "arg1.a * (1.0 - texop" << stage << "rgbAs)";
				break;
			case CMaterial::Mad:
				ss << "texop" << stage << "arg0.a * texop" << stage << "arg1.a + texop" << stage << "arg2.a";
				break;
			}
			ss << ";" << std::endl;
		}
		else if (stage == 0)
		{
			ss << "vec4 texop" << stage << " = fragColor; // no active texture in stage" << std::endl;
		}
		else
		{
			ss << "vec4 texop" << stage << " = texop" << (stage - 1) << "; // no active texture in stage" << std::endl;
		}
	}
	ss << "fragColor = texop" << (maxTex - 1) << ";" << std::endl;
}

void ppSpecular(std::stringstream &ss, const CPPBuiltin &desc)
{
	if (useTex(desc, 0))
	{
		ss << "vec3 specop0 = texel0.rgb * fragColor.rgb;" << std::endl;
		if (useTex(desc, 1))
		{
			ss << "vec4 specop1 = vec4(texel1.rgb * texel0.a + specop0, fragColor.a);" << std::endl;
		}
		else
		{
			nlwarning("Texture stage 1 (reflection) missing in Specular shader");
			ss << "vec4 specop1 = vec4(specop0, fragColor.a);" << std::endl;
		}
		ss << "fragColor = specop1;" << std::endl;
	}
	else if (useTex(desc, 1))
	{
		nlwarning("Texture stage 0 (color) missing in Specular shader");
		ss << "vec4 specop1 = vec4(texel1.rgb + fragColor.rgb, 1.0);" << std::endl;
		ss << "fragColor = specop1;" << std::endl;
	}
	else
	{
		nlwarning("PP: No textures defined in Specular shader");
		// do nothing
	}
}

void ppLightmap(std::stringstream &ss, const CPPBuiltin &desc)
{
	uint nstages;
	for (nstages = 0; nstages < IDRV_MAT_MAXTEXTURES; ++nstages)
		if (!useTex(desc, nstages))
			break;
	if (nstages == 0)
	{
		// do nothing
		nlwarning("PP: Lightmap without textures setup");
	}
	else if (nstages == 1)
	{
		// single map (doesn't support alpha)
		nlwarning("PP: Lightmap material without lightmaps setup %i %i %i %i", useTex(desc, 0), useTex(desc, 1), useTex(desc, 2), useTex(desc, 3));
		ss << "fragColor = vec4(1.0, 1.0, 1.0, 1.0);" << std::endl; // HACK FIXME GL3
		ss << "fragColor = texel" << (nstages - 1) << " * fragColor;" << std::endl;
	}
	else
	{
		ss << "fragColor = vec4(1.0, 1.0, 1.0, 1.0);" << std::endl; // HACK FIXME GL3
		ss << "vec4 lightmapop = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl;
		for (uint stage = 0; stage < (nstages - 1); ++stage)
			ss << "lightmapop = lightmapop + texel" << stage << " * constant" << stage << " * texel" << (nstages - 1) << ";" << std::endl;
		ss << "fragColor = fragColor * lightmapop;" << std::endl;
	}
}

void ppGenerate(std::string &result, const CPPBuiltin &desc)
{
	std::stringstream ss;
	ss << "// Builtin Pixel Shader: " << s_ShaderNames[desc.Shader] << std::endl;
	ss << std::endl;

	ss << "#version 330" << std::endl;
	ss << "#extension GL_ARB_separate_shader_objects : enable" << std::endl;
	ss << std::endl;

	ss << "out vec4 fragColor;" << std::endl;

	for (int i = Weight; i < NumOffsets; i++)
	{
		if (hasFlag(desc.VertexFormat, g_VertexFlags[i]))
		{
			ss << "smooth in vec4 ";
			ss << g_AttribNames[i] << ";" << std::endl;
		}
	}
	ss << std::endl;
	
	uint maxTex = maxTextures(desc.Shader);
	for (uint stage = 0; stage < maxTex; ++stage)
	{
		if (stage == 1 && desc.Shader == CMaterial::UserColor)
		{
			ss << "// user color" << std::endl;
		}
		if (useTex(desc, stage))
		{
			ss << "uniform "
				<< ((desc.TexSamplerMode[stage] == SamplerCube) ? "samplerCube" : "sampler2D")
				<< " sampler" << stage << ";" << std::endl;
		}
	}
	ss << std::endl;

	// ???
	ss << "uniform vec4 materialColor;" << std::endl; // ?! what is this doing in PP
	ss << std::endl;

	// TexEnv
	ss << "uniform vec4 constant0;" << std::endl; // FIXME: we must optimize this by texenv!...
	ss << "uniform vec4 constant1;" << std::endl;
	ss << "uniform vec4 constant2;" << std::endl;
	ss << "uniform vec4 constant3;" << std::endl;
	ss << std::endl;

	// Alpha test
	if (desc.Flags & IDRV_MAT_ALPHA_TEST)
	{
		ss << "uniform float alphaRef;" << std::endl;
		ss << std::endl;
	}

	// Fog
	if (desc.Fog) // FIXME: FogMode!
	{
		ss << "uniform vec2 fogParams;" << std::endl; // s = start, t = end
		ss << "uniform vec4 fogColor;" << std::endl;

		/*if (desc->getFogMode() == CShaderDesc::Linear)
		{*/
			//ss << "uniform float fogDensity;" << std::endl;
		/*}*/

		ss << "smooth in vec4 ecPos;" << std::endl;
		
		/*switch(desc->getFogMode())
		{*/
		//case CShaderDesc::Linear:
			ss << "vec4 applyFog(vec4 col)" << std::endl;
			ss << "{" << std::endl;
			ss << "float z = ecPos.z / ecPos.w;" << std::endl;
			ss << "z = abs(z);" << std::endl;
			ss << "float fogFactor = (fogParams.t - z) / (fogParams.t - fogParams.s);" << std::endl;
			ss << "fogFactor = clamp(fogFactor, 0.0, 1.0);" << std::endl;
			ss << "vec4 fColor = mix(fogColor, col, fogFactor);" << std::endl;
			ss << "return fColor;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;
		//	break;
		/*}*/

		ss << std::endl;
	}

	ss << "smooth in vec4 vertexColor;" << std::endl;
	ss << std::endl;
	
	ss << "void main(void)" << std::endl;
	ss << "{" << std::endl;

	// Vertex color (light or unlit diffuse, primary and secondary)
	ss << "fragColor = vertexColor;" << std::endl;

	for (uint stage = 0; stage < maxTex; ++stage)
	{
		if (stage == 1 && desc.Shader == CMaterial::UserColor)
		{
			ss << "vec4 texel1 = texel0;" << std::endl; // UserColor has one single texture set up in two textures, we optimize this away here
		}
		else if (useTex(desc, stage))
		{
			ss << "vec4 texel" << stage << " = texture(sampler" << stage << ", ";	
			if (desc.Shader == CMaterial::LightMap && stage != (maxTex - 1) && useTex(desc, stage + 1) && hasFlag(desc.VertexFormat, g_VertexFlags[TexCoord1]))
			{
				ss << g_AttribNames[TexCoord1];
			}
			else if (desc.Shader == CMaterial::LightMap && hasFlag(desc.VertexFormat, g_VertexFlags[TexCoord0]))
			{
				ss << g_AttribNames[TexCoord0];
			}
			else if (hasFlag(desc.VertexFormat, g_VertexFlags[TexCoord0 + stage]))
			{
				ss << g_AttribNames[TexCoord0 + stage];
			}
			else if (hasFlag(desc.VertexFormat, g_VertexFlags[TexCoord0]))
			{
				ss << g_AttribNames[TexCoord0];
			}
			else
			{
				nlwarning("GL3: Pixel Program generated for material with coordinateless texture");
				ss << "vec4(0.0, 0.0, 0.0, 0.0)";
			}
			ss << ((desc.TexSamplerMode[stage] == SamplerCube) ? ".stp);" : ".st);");
			ss << std::endl;
		}
	}

	switch (desc.Shader)
	{
	case CMaterial::Normal:
	case CMaterial::UserColor:
		ppTexEnv(ss, desc);
		break;
	case CMaterial::Specular:
		ppSpecular(ss, desc);
		break;
	case CMaterial::LightMap:
		ppLightmap(ss, desc);
		break;
	default:
		nlwarning("GL3: Try to generate unknown shader type (%s)", s_ShaderNames[desc.Shader]);
		// ss << "fragColor = vec(1.0, 0.0, 0.5, 1.0);" << std::endl;
		break;
	}

	if (desc.Flags & IDRV_MAT_ALPHA_TEST)
	{
		ss << "if (fragColor.a < alphaRef) discard;" << std::endl; // TODO: VERIFY < or <= ?
	}

	if (desc.Fog)
	{
		ss << "fragColor = applyFog(fragColor);" << std::endl;
	}

	ss << "}" << std::endl;

	result = ss.str();
}

} /* anonymous namespace */

void CDriverGL3::generateBuiltinPixelProgram(CMaterial &mat)
{
	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
	nlassert(matDrv);

	std::set<CPPBuiltin>::iterator it = m_PPBuiltinCache.find(matDrv->PPBuiltin);
	if (it != m_PPBuiltinCache.end())
	{
		matDrv->PPBuiltin.PixelProgram = it->PixelProgram;
		return;
	}

	std::string result;
	ppGenerate(result, matDrv->PPBuiltin);

	CPixelProgram *program = new CPixelProgram();
	IProgram::CSource *src = new IProgram::CSource();
	src->Profile = IProgram::glsl330f;
	src->DisplayName = "Builtin Pixel Program (" + NLMISC::toString(m_PPBuiltinCache.size()) + ")";
	src->setSource(result);
	program->addSource(src);

	nldebug("GL3: Generate '%s'", src->DisplayName.c_str());

	if (!compilePixelProgram(program))
	{
		delete program;
		program = NULL;
	}

	matDrv->PPBuiltin.PixelProgram = program;
	m_PPBuiltinCache.insert(matDrv->PPBuiltin);
}

void CPPBuiltin::checkDriverStateTouched(CDriverGL3 *driver) // MUST NOT depend on any state set by checkMaterialStateTouched
{
	// Add generated texture coordinates to vertex format // TODO: Eliminate unused flags
	uint16 vertexFormat = driver->m_VPBuiltinCurrent.VertexFormat;
	for (sint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
		if (driver->m_VPBuiltinCurrent.TexGenMode[stage] >= 0)
			vertexFormat |= g_VertexFlags[TexCoord0 + stage];
	vertexFormat &= ~g_VertexFlags[PrimaryColor];
	vertexFormat &= ~g_VertexFlags[SecondaryColor];

	// Compare values
	if (VertexFormat != vertexFormat)
	{
		VertexFormat = vertexFormat;
		Touched = true;
	}
	if (Fog != driver->m_VPBuiltinCurrent.Fog)
	{
		Fog = driver->m_VPBuiltinCurrent.Fog;
		Touched = true;
	}
}

void CPPBuiltin::checkDriverMaterialStateTouched(CDriverGL3 *driver, CMaterial &mat)
{
	CMaterial::TShader shader = getSupportedShader(mat.getShader());
	switch (shader)
	{
	case CMaterial::LightMap:
		// Use Textures from current driver state
		// NB: Might be necessary to use this for every material type and remove the one from the other function
		uint maxTex = maxTextures(shader);
		uint8 textureActive = 0x00;
		for (uint stage = 0; stage < maxTex; ++stage)
		{
			NL3D::ITexture *tex = driver->_CurrentTexture[stage];
			if (tex)
			{
				textureActive |= (1 << stage);

				uint8 texSamplerMode = tex->isTextureCube() ? SamplerCube : Sampler2D;
				if (TexSamplerMode[stage] != texSamplerMode)
				{
					TexSamplerMode[stage] = texSamplerMode;
					Touched = true;
				}
			}
		}
		if (TextureActive != textureActive)
		{
			TextureActive = textureActive;
			Touched = true;
		}
		// nldebug("TextureActive: %i", TextureActive);
		break;
	}
}

void CPPBuiltin::checkMaterialStateTouched(CMaterial &mat) // MUST NOT depend on any state set by checkDriverStateTouched
{
	// Optimize
	uint32 touched = !PixelProgram ? IDRV_TOUCHED_ALL : mat.getTouched();
	if (touched == 0) return;

	// Compare values
	CMaterial::TShader shader = getSupportedShader(mat.getShader());
	if (Shader != shader)
	{
		Shader = shader;
		Touched = true;
	}
	uint32 flags = mat.getFlags();
	flags &= IDRV_MAT_ALPHA_TEST; // TODO: |= with the wanted flags from the VP when flags are added to the VP
	if (Flags != flags)
	{
		Flags = flags;
		Touched = true;
	}
	uint maxTex = maxTextures(shader);
	if (touched & IDRV_TOUCHED_ALLTEX) // Note: There is a case where textures are provided where no texture coordinates are provided, this is handled gracefully by the pixel program generation (it will use a vec(0) texture coordinate). The inverse is an optimization issue
	{
		switch (shader)
		{
		case CMaterial::LightMap:
		default:
			// Use textures directly from the CMaterial
			uint8 textureActive = 0x00;
			for (uint stage = 0; stage < maxTex; ++stage)
			{
				NL3D::ITexture *tex = mat._Textures[stage];
				if (tex)
				{
					textureActive |= (1 << stage);

					// Issue: Due to the IDRV_TOUCHED_ALLTEX check, the sampler mode of an ITexture cannot be modified after it has been added to the CMaterial
					uint8 texSamplerMode = tex->isTextureCube() ? SamplerCube : Sampler2D;
					if (TexSamplerMode[stage] != texSamplerMode)
					{
						TexSamplerMode[stage] = texSamplerMode;
						Touched = true;
					}
				}
			}
			if (TextureActive != textureActive)
			{
				TextureActive = textureActive;
				Touched = true;
			}
			break;
		}
	}
	if (useTexEnv(shader) && (touched & IDRV_TOUCHED_TEXENV))
	{
		for (uint stage = 0; stage < maxTex; ++stage)
		{
			if (TexEnvMode[stage] != mat._TexEnvs[stage].EnvPacked)
			{
				TexEnvMode[stage] = mat._TexEnvs[stage].EnvPacked;
				Touched = true;
			}
		}
	}

	// Optimize
	mat.clearTouched(0xFFFFFFFF);
}

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

