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
#include "driver_opengl_vertex_program.h"

#include <sstream>

#include "nel/3d/vertex_program.h"
#include "nel/3d/light.h"

#include "driver_opengl.h"
#include "driver_opengl_vertex_buffer_hard.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

bool operator<(const CVPBuiltin &left, const CVPBuiltin &right)
{
	if (left.VertexFormat != right.VertexFormat)
		return left.VertexFormat < right.VertexFormat;
	if (left.Lighting != right.Lighting)
		return right.Lighting;
	if (left.Lighting)
		for (sint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
			if (left.LightMode[i] != right.LightMode[i])
				return left.LightMode[i] < right.LightMode[i];
	if (left.Fog != right.Fog)
		return right.Fog;
	if (left.VertexColorLighted != right.VertexColorLighted)
		return right.VertexColorLighted;

	return false;
}

namespace
{
	inline bool hasFlag(uint32 data, uint32 flag)
	{
		if ((data & flag) != 0)
			return true;
		else
			return false;
	}

	enum TAttribOffset
	{
		Position,
		Weight,
		Normal,
		PrimaryColor,
		SecondaryColor,
		Fog,
		PaletteSkin,
		Empty,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,
		NumOffsets
	};

	const uint16 s_VertexFlags[CVertexBuffer::NumValue] = 
	{
		CVertexBuffer::PositionFlag,
		CVertexBuffer::WeightFlag,
		CVertexBuffer::NormalFlag,
		CVertexBuffer::PrimaryColorFlag,
		CVertexBuffer::SecondaryColorFlag,
		CVertexBuffer::FogFlag,
		CVertexBuffer::PaletteSkinFlag,
		0,
		CVertexBuffer::TexCoord0Flag,
		CVertexBuffer::TexCoord1Flag,
		CVertexBuffer::TexCoord2Flag,
		CVertexBuffer::TexCoord3Flag,
		CVertexBuffer::TexCoord4Flag,
		CVertexBuffer::TexCoord5Flag,
		CVertexBuffer::TexCoord6Flag,
		CVertexBuffer::TexCoord7Flag
	};

	const char *s_AttribNames[ CVertexBuffer::NumValue ] =
	{
		"position",
		"weight",
		"normal",
		"color",
		"color2",
		"fog",
		"paletteSkin",
		"none",
		"texCoord0",
		"texCoord1",
		"texCoord2",
		"texCoord3",
		"texCoord4",
		"texCoord5",
		"texCoord6",
		"texCoord7"
	};

	const char *s_TexelNames[ SHADER_MAX_TEXTURES ] =
	{
		"texel0",
		"texel1",
		"texel2",
		"texel3"
	};

	const char *s_ConstantNames[ 4 ] =
	{
		"constant0",
		"constant1",
		"constant2",
		"constant3"
	};

	void vpLightUniforms(std::stringstream &ss, const CVPBuiltin &desc, int i)
	{
		switch (desc.LightMode[i])
		{
		case CLight::DirectionalLight:
			ss << "uniform vec3 light" << i << "DirOrPos;" << std::endl;
			ss << "uniform vec4 light" << i << "ColDiff;" << std::endl;
			ss << "uniform vec4 light" << i << "ColAmb;" << std::endl;
			ss << "uniform vec4 light" << i << "ColSpec;" << std::endl;
			ss << "uniform float light" << i << "Shininess;" << std::endl;
			break;
		case CLight::PointLight:
			ss << "uniform vec3 light" << i << "DirOrPos;" << std::endl;
			ss << "uniform vec4 light" << i << "ColDiff;" << std::endl;
			ss << "uniform vec4 light" << i << "ColAmb;" << std::endl;
			ss << "uniform vec4 light" << i << "ColSpec;" << std::endl;
			ss << "uniform float light" << i << "Shininess;" << std::endl;
			ss << "uniform float light" << i << "ConstAttn;" << std::endl;
			ss << "uniform float light" << i << "LinAttn;" << std::endl;
			ss << "uniform float light" << i << "QuadAttn;" << std::endl;
			break;
		}
	}

	void vpLightFunctions(std::stringstream &ss, const CVPBuiltin &desc, int i)
	{
		switch (desc.LightMode[i])
		{
		case CLight::DirectionalLight:
			ss << "float getIntensity" << i << "(vec3 normal3, vec3 lightDir)" << std::endl;
			ss << "{" << std::endl;
			ss << "float angle = dot(lightDir, normal3);" << std::endl;
			ss << "angle = max(0.0, angle);" << std::endl;
			ss << "return angle;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;

			ss << "float getSpecIntensity" << i << "(vec3 normal3, vec3 lightDir)" << std::endl;
			ss << "{" << std::endl;
			ss << "vec3 halfVector = normalize(lightDir + normal3);" << std::endl;
			ss << "float angle = dot(normal3, halfVector);" << std::endl;
			ss << "angle = max(0.0, angle);" << std::endl;
			ss << "float si = pow(angle, light" << i << "Shininess);" << std::endl;
			ss << "return si;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;

			ss << "vec4 getLight" << i << "Color()" << std::endl;
			ss << "{" << std::endl;
			ss << "vec4 lightDir4 = viewMatrix * vec4(light" << i << "DirOrPos, 1.0);" << std::endl;
			ss << "vec3 lightDir = lightDir4.xyz / lightDir4.w;" << std::endl;
			ss << "lightDir = normalize(lightDir);" << std::endl;
			ss << "vec3 normal3 = vnormal.xyz / vnormal.w;" << std::endl;
			ss << "normal3 = normalMatrix * normal3;" << std::endl;
			ss << "normal3 = normalize(normal3);" << std::endl;

			//if (desc->useTextures() || (material->getShader() == CMaterial::LightMap))
			//{
				ss << "vec4 lc = getIntensity" << i << "(normal3, lightDir) * light" << i << "ColDiff + ";
				ss << "getSpecIntensity" << i << "(normal3, lightDir) * light" << i << "ColSpec + ";
				ss << "light" << i << "ColAmb;" << std::endl;
			//} // FIXME: Ambient color is not correctly implemented
			/*else
			{
				ss << "vec4 lc = getIntensity" << num << "(normal3, lightDir) * light" << num << "ColDiff * diffuseColor + ";
				ss << "getSpecIntensity" << num << "(normal3, lightDir) * light" << num << "ColSpec * specularColor + ";
				ss << "light" << num << "ColAmb * ambientColor;" << std::endl;
			}*/

			ss << "return lc;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;
			break;
		case CLight::PointLight:
			ss << "float getIntensity" << i << "(vec3 normal3, vec3 direction3)" << std::endl;
			ss << "{" << std::endl;
			ss << "float angle = dot(direction3, normal3);" << std::endl;
			ss << "angle = max(0.0, angle);" << std::endl;
			ss << "return angle;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;

			ss << "float getSpecIntensity" << i << "(vec3 normal3, vec3 direction3)" << std::endl;
			ss << "{" << std::endl;
			ss << "vec3 halfVector = normalize(direction3 + normal3);" << std::endl;
			ss << "float angle = dot(normal3, halfVector);" << std::endl;
			ss << "angle = max(0.0, angle);" << std::endl;
			ss << "float si = pow(angle, light" << i << "Shininess);" << std::endl;
			ss << "return si;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;

			ss << "vec4 getLight" << i << "Color()" << std::endl;
			ss << "{" << std::endl;
			ss << "vec3 ecPos3 = ecPos4.xyz / ecPos4.w;" << std::endl;
			ss << "vec4 lightPos4 = viewMatrix * vec4(light" << i << "DirOrPos, 1.0);" << std::endl;
			ss << "vec3 lightPos = lightPos4.xyz / lightPos4.w;" << std::endl;
			ss << "vec3 lightDirection = lightPos - ecPos3;" << std::endl;
			ss << "float lightDistance = length(lightDirection);" << std::endl;
			ss << "lightDirection = normalize(lightDirection);" << std::endl;

			ss << "float attenuation = light" << i << "ConstAttn + ";
			ss << "light" << i << "LinAttn * lightDistance +";
			ss << "light" << i << "QuadAttn * lightDistance * lightDistance;" << std::endl;
			
			ss << "vec3 normal3 = vnormal.xyz / vnormal.w;" << std::endl;
			ss << "normal3 = normalMatrix * normal3;" << std::endl;
			ss << "normal3 = normalize(normal3);" << std::endl;

			/*if (desc->useTextures() || (material->getShader() == CMaterial::LightMap))
			{*/
				ss << "vec4 lc = getIntensity" << i << "(normal3, lightDirection) * light" << i << "ColDiff + ";
				ss << "getSpecIntensity" << i << "(normal3, lightDirection) * light" << i << "ColSpec + ";
				ss << "light" << i << "ColAmb;" << std::endl;
				// FIXME: Ambient stuff
			/*}
			else
			{
				ss << "vec4 lc = getIntensity" << num << "(normal3, lightDirection) * light" << num << "ColDiff * diffuseColor+ ";
				ss << "getSpecIntensity" << num << "(normal3, lightDirection) * light" << num << "ColSpec * specularColor + ";
				ss << "light" << num << "ColAmb * ambientColor;" << std::endl;
			}*/

			ss << "lc = lc / attenuation;" << std::endl;
			ss << "return lc;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;
			break;
		}
	}

	void vsLighting(std::stringstream &ss, const CVPBuiltin &desc, int i)
	{

	}

	void vpGenerate(std::string &result, const CVPBuiltin &desc)
	{
		std::stringstream ss;
		ss << "// Builtin Vertex Shader" << std::endl;
		ss << std::endl;
		ss << "#version 330" << std::endl;
		ss << "#extension GL_ARB_separate_shader_objects : enable" << std::endl;
		ss << "out gl_PerVertex" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 gl_Position;" << std::endl;
		ss << "};" << std::endl;
		ss << std::endl;
		ss << "uniform mat4 modelViewProjection;" << std::endl;
		ss << std::endl;

		for (int i = Position; i < NumOffsets; ++i)
			if (hasFlag(desc.VertexFormat, s_VertexFlags[i]))
				ss << "layout (location = " << i << ") " << "in vec4 " << "v" << s_AttribNames[i] << ";" << std::endl;
		ss << std::endl;

		for (int i = Weight; i < NumOffsets; ++i)
			if (hasFlag(desc.VertexFormat, s_VertexFlags[i]))
				ss << "smooth out vec4 " << s_AttribNames[i] << ";" << std::endl;
		ss << std::endl;
		
		// if (!useTextures) {
		ss << "uniform vec4 ambientColor;" << std::endl;
		ss << "uniform vec4 diffuseColor;" << std::endl;
		ss << "uniform vec4 specularColor;" << std::endl; // }

		if (desc.Fog || desc.Lighting)
			ss << "uniform mat4 modelView;" << std::endl;
		if (desc.Lighting)
			ss << "uniform mat4 viewMatrix;" << std::endl;
		if (desc.Fog || desc.Lighting)
			ss << "vec4 ecPos4;" << std::endl;
		if (desc.Fog)
			ss << "smooth out vec4 ecPos;" << std::endl;
		ss << std::endl;

		if (desc.Lighting)
		{
			ss << "uniform mat3 normalMatrix;" << std::endl;
			for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
				vpLightUniforms(ss, desc, i);
			ss << "smooth out vec4 lightColor;" << std::endl;
			ss << std::endl;
			
			for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
				vpLightFunctions(ss, desc, i);
			ss << std::endl;
		}
	
		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;
		ss << "gl_Position = modelViewProjection * " << "v" << s_AttribNames[0] << ";" << std::endl;

		if (desc.Fog || desc.Lighting)
			ss << "ecPos4 = modelView * v" << s_AttribNames[0] << ";" << std::endl;
		if (desc.Fog)
			ss << "ecPos = ecPos4;" << std::endl;

		if (desc.Lighting)
		{
			ss << "lightColor = vec4(0.0, 0.0, 0.0, 1.0);" << std::endl;
			for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; i++)
				if (desc.LightMode[i] == CLight::DirectionalLight || desc.LightMode[i] == CLight::PointLight)
					ss << "lightColor = lightColor + getLight" << i << "Color();" << std::endl;
		}

		for (int i = Weight; i < NumOffsets; i++)
		{
			if (hasFlag(desc.VertexFormat, s_VertexFlags[i]))
			{
				ss << s_AttribNames[i] << " = " << "v" << s_AttribNames[i] << ";" << std::endl;
			}
		}

		ss << "}" << std::endl;
		result = ss.str();
	}
}

CVertexProgram *CDriverGL3::generateBuiltinVertexProgram()
{
	CVPBuiltin desc;
	desc.VertexFormat = _CurrentVertexBufferHard->VB->getVertexFormat();
	desc.Fog = _DriverGLStates.isFogEnabled();
	desc.Lighting = _DriverGLStates.isLightingEnabled();
	if (desc.Lighting)
		for (sint i = 0; i < MaxLight; ++i)
			desc.LightMode[i] = _UserLightEnable[i] ? _LightMode[i] : -1;
	desc.VertexColorLighted = false; // _DriverGLStates._VertexColorLighted;

	std::set<CVPBuiltin>::iterator it = m_VPBuiltinCache.find(desc);
	if (it != m_VPBuiltinCache.end())
		return it->VertexProgram;

	std::string result;
	vpGenerate(result, desc);

	CVertexProgram *vertexProgram = new CVertexProgram();
	IProgram::CSource *src = new IProgram::CSource();
	src->Profile = IProgram::glsl330v;
	src->DisplayName = "Builtin Vertex Program (" + NLMISC::toString(m_VPBuiltinCache.size()) + ")";
	src->setSource(result);
	vertexProgram->addSource(src);

	nldebug("GL3: Generate '%s'", src->DisplayName.c_str());

	if (!compileVertexProgram(vertexProgram))
	{
		delete vertexProgram; vertexProgram = NULL;
	}

	desc.VertexProgram = vertexProgram;
	m_VPBuiltinCache.insert(desc);

	return desc.VertexProgram;
}

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

