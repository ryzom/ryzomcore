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
	case CMaterial::UserColor:
	case CMaterial::Normal:
	case CMaterial::Specular:
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
					rgbArgVec << "diffuse";
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
					alphaArgVec << "diffuse";
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
					ss << "float texop" << stage << "rgbAs = texop" << stage << ".a;" << std::endl;
					break;
				}
			case CMaterial::InterpolateDiffuse:
				ss << "float texop" << stage << "rgbAs = diffuse.a;" << std::endl;
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
					ss << "float texop" << stage << "alphaAs = texop" << stage << ".a;" << std::endl;
					break;
				}
			case CMaterial::InterpolateDiffuse:
				ss << "float texop" << stage << "alphaAs = diffuse.a;" << std::endl;
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
			ss << "vec4 texop" << stage << " = diffuse; // no active texture in stage" << std::endl;
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
	ss << "vec3 specop0 = texel0.rgb * diffuse.rgb;" << std::endl;
	ss << "vec4 specop1 = vec4(texel1.rgb * texel0.a + specop0, diffuse.a);" << std::endl;
	ss << "fragColor = specop1;" << std::endl;
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
		if (useTex(desc, stage))
		{
			ss << "uniform "
				<< ((desc.TexSamplerMode[stage] == SamplerCube) ? "samplerCube" : "sampler2D")
				<< " sampler" << stage << ";" << std::endl;
		}
	}

	// ???
	ss << "uniform vec4 materialColor;" << std::endl; // ?! what is this doing in PP

	// TexEnv
	ss << "uniform vec4 constant0;" << std::endl; // todo: we can optimize this by env!...
	ss << "uniform vec4 constant1;" << std::endl;
	ss << "uniform vec4 constant2;" << std::endl;
	ss << "uniform vec4 constant3;" << std::endl;

	// Alpha test
	ss << "uniform float alphaTreshold;" << std::endl; // FIXME: only when driver state has alpha test.... oooh

	// Fog
	if (desc.Fog) // FIXME: FogMode!
	{
		ss << "uniform float fogStart;" << std::endl;
		ss << "uniform float fogEnd;" << std::endl;
		ss << "uniform vec4 fogColor;" << std::endl;

		/*if (desc->getFogMode() == CShaderDesc::Linear)
		{
			ss << "uniform float fogDensity;" << std::endl;
		}*/

		ss << "smooth in vec4 ecPos;" << std::endl;
	}
	ss << std::endl;

	/*if (desc->lightingEnabled())
	{
		addLightUniformsFS();
		addLightInsFS();
		ss << std::endl;
		
		addLightsFunctionFS();
		ss << std::endl;
	}

	if (desc->fogEnabled())
		addFogFunction();*/
	
	ss << "void main(void)" << std::endl;
	ss << "{" << std::endl;

	// Light color
	/*ss << "vec4 diffuse = vec4(1.0, 1.0, 1.0, 1.0);" << std::endl;
	if (desc->lightingEnabled())
	{
		ss << "diffuse = applyLights(diffuse);" << std::endl;
		ss << "diffuse.a = 1.0;" << std::endl;
	}
	if (hasFlag(desc->vbFlags, g_VertexFlags[PrimaryColor]))
		ss << "diffuse = color * diffuse;" << std::endl; // TODO: If this is the correct location, we should premultiply light and color in VS.

	bool textures = false;
	for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		if (desc->getUseTexStage(i))
		{
			ss << "vec4 texel" << i << " = texture(sampler" << i << ", ";				
			if (desc->hasVBFlags(g_VertexFlags[TexCoord0 + i]))
				ss << g_AttribNames[TexCoord0 + i];
			else if (desc->hasVBFlags(g_VertexFlags[TexCoord0]))
				ss << g_AttribNames[TexCoord0];
			else
			{
				nlwarning("GL3: Pixel Program generated for material with coordinateless texture");
				ss << "vec4(0.0, 0.0, 0.0, 0.0)";
			}
			ss << ((desc->textureSamplerMode[i] == SamplerCube) ? ".str);" : ".st);");
			ss << std::endl;
			textures = true;
		}
	}*/

	/*switch (material->getShader())
	{
	case CMaterial::Specular:
		generateSpecular();
		break;
	default:
		generateTexEnv();
		break;
	}

	if (desc->fogEnabled())
		addFog();

	addAlphaTest();*/

	// ss << "fragColor = fragColor + vec4(0.0, 0.25, 0.0, 0.0);" << std::endl;

	// ss << "fragColor.b = diffuse.b;" << std::endl;

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
	// Add generated texture coordinates to vertex format
	uint16 vertexFormat = driver->m_VPBuiltinCurrent.VertexFormat;
	for (sint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
		if (driver->m_VPBuiltinCurrent.TexGenMode[stage] >= 0)
			vertexFormat |= g_VertexFlags[TexCoord0 + stage];

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
	uint maxTex = maxTextures(shader);
	if (touched & IDRV_TOUCHED_ALLTEX) // Note: There is a case where textures are provided where no texture coordinates are provided, this is handled gracefully by the pixel program generation (it will use a vec(0) texture coordinate). The inverse is an optimization issue
	{
		uint8 textureActive = 0x00;
		for (uint stage = 0; stage < maxTex; ++stage)
		{
			NL3D::ITexture *tex = mat.getTexture(stage);
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
	}
	if (useTexEnv(shader) && (touched & IDRV_TOUCHED_TEXENV))
	{
		for (uint stage = 0; stage < maxTex; ++stage)
		{
			if (TexEnvMode[stage] != mat.getTexEnvMode(stage))
			{
				TexEnvMode[stage] = mat.getTexEnvMode(stage);
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

