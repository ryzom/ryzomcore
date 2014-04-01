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

} /* anonymous namespace */

bool operator<(const CPPBuiltin &left, const CPPBuiltin &right)
{	
	// Material state
	if (left.Shader != right.Shader)
		return left.Shader < right.Shader;
	if (left.TextureActive != right.TextureActive)
		return left.TextureActive < right.TextureActive;
	uint maxTex = maxTextures(left.Shader);
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

bool useTex(CPPBuiltin &desc, uint stage)
{
	return (desc.TextureActive & (1 << stage)) != 0;
}
	
void ppTexEnv(std::stringstream &ss, CPPBuiltin &desc)
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

void ppSpecular(std::stringstream &ss, CPPBuiltin &desc)
{
	ss << "vec3 specop0 = texel0.rgb * diffuse.rgb;" << std::endl;
	ss << "vec4 specop1 = vec4(texel1.rgb * texel0.a + specop0, diffuse.a);" << std::endl;
	ss << "fragColor = specop1;" << std::endl;
}

void ppGenerate(std::string &result, CPPBuiltin &desc)
{

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
	if (touched & IDRV_TOUCHED_ALLTEX) // Note: There is a case where textures are provided where no texture coordinates are provided, this is handled gracefully by the pixel program generation (it will use a vec(0) texture coordinate). The inverse is an optimization issue.
	{
		uint8 textureActive = 0x00;
		for (uint i = 0; i < maxTex; ++i)
			if (mat.getTexture(i))
				textureActive |= (1 << i);
		TextureActive = textureActive;
		Touched = true;
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

