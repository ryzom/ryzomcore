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

#include "stdopengl.h"
#include "driver_glsl_shader_generator.h"

#include <sstream>

#include "nel/3d/vertex_buffer.h"
#include "driver_opengl_program.h"
#include "driver_opengl_shader_desc.h"

namespace NL3D
{
	const char *shaderNames[] = 
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

	CGLSLShaderGenerator::CGLSLShaderGenerator()
	{
		reset();
	}

	CGLSLShaderGenerator::~CGLSLShaderGenerator()
	{

	}

	void CGLSLShaderGenerator::reset()
	{
		material = NULL;
		vbFormat = 0;
		desc = NULL;
		ss.str("");
		ss.clear();
	}

	void CGLSLShaderGenerator::generatePS(std::string &ps)
	{
		ss.str("");
		ss.clear();

		ss << "// " << shaderNames[ material->getShader() ] << " Pixel Shader" << std::endl;
		ss << std::endl;

		ss << "#version 330" << std::endl;
		ss << std::endl;

		ss << "out vec4 fragColor;" << std::endl;

		for (int i = Weight; i < NumOffsets; i++)
		{
			if (hasFlag(vbFormat, g_VertexFlags[i]))
			{
				ss << "smooth in vec4 ";
				ss << g_AttribNames[i] << ";" << std::endl;
			}
		}
		ss << std::endl;
		
#if 0 // LIGHTING DEBUG
		if (desc->lightingEnabled() || material->getShader() != CMaterial::Normal)
		{
			generateInvalidPS();
		}
		else
		{
			generateNormalPS();
		}
#else
		switch(material->getShader())
		{
		case CMaterial::Normal:
		case CMaterial::UserColor:
			generateNormalPS();
			break;

		case CMaterial::LightMap:
			generateLightMapPS();
			break;

		case CMaterial::Specular:
			generateSpecularPS();
			break;

		case CMaterial::PerPixelLighting:
		case CMaterial::PerPixelLightingNoSpec:
			generatePPLPS();
			break;

		case CMaterial::Water:
			generateWaterPS();
			break;

		case CMaterial::Cloud:
			generateCloudPS();
			break;
		}
#endif

		ps.assign(ss.str());
	}

	void CGLSLShaderGenerator::addAmbient()
	{
		ss << "uniform vec4 ambientColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addDiffuse()
	{
		ss << "uniform vec4 diffuseColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addSpecular()
	{
		ss << "uniform vec4 specularColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addColor()
	{
		ss << "uniform vec4 materialColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addConstants()
	{
		ss << "uniform vec4 constant0;" << std::endl;
		ss << "uniform vec4 constant1;" << std::endl;
		ss << "uniform vec4 constant2;" << std::endl;
		ss << "uniform vec4 constant3;" << std::endl;
	}

	void CGLSLShaderGenerator::addNormalMatrix()
	{
		ss << "uniform mat3 normalMatrix;" << std::endl;
	}

	void CGLSLShaderGenerator::addViewMatrix()
	{
		ss << "uniform mat4 viewMatrix;" << std::endl;
	}

	void CGLSLShaderGenerator::addAlphaTreshold()
	{
		ss << "uniform float alphaTreshold;" << std::endl;
	}

	void CGLSLShaderGenerator::addAlphaTest()
	{
		if (material->getAlphaTest())
			ss << "if (fragColor.a <= (alphaTreshold - 0.0001)) discard;" << std::endl;
	}

	void CGLSLShaderGenerator::addFogUniform()
	{
		if (!desc->fogEnabled())
			return;

		ss << "uniform float fogStart;" << std::endl;
		ss << "uniform float fogEnd;" << std::endl;
		ss << "uniform vec4 fogColor;" << std::endl;

		if (desc->getFogMode() == CShaderDesc::Linear)
			return;

		ss << "uniform float fogDensity;" << std::endl;
	}

	void CGLSLShaderGenerator::addFogFunction()
	{
		if (!desc->fogEnabled())
			return;

		switch(desc->getFogMode())
		{
		case CShaderDesc::Linear:
			ss << "vec4 applyFog(vec4 col)" << std::endl;
			ss << "{" << std::endl;
			ss << "float z = ecPos.z / ecPos.w;" << std::endl;
			ss << "z = abs(z);" << std::endl;
			ss << "float fogFactor = (fogEnd - z) / (fogEnd - fogStart);" << std::endl;
			ss << "fogFactor = clamp(fogFactor, 0.0, 1.0);" << std::endl;
			ss << "vec4 fColor = mix(fogColor, col, fogFactor);" << std::endl;
			ss << "return fColor;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;
			break;
		}
	}

	void CGLSLShaderGenerator::addFog()
	{
		ss << "fragColor = applyFog(fragColor);" << std::endl;
	}


	/////////////////////////////////////////////////////////// Lights ////////////////////////////////////////////////////////////////////

	void CGLSLShaderGenerator::addLightUniformsFS()
	{
		for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; i++)
		{
			switch(desc->getLight(i))
			{
			case CShaderDesc::Nolight:
				continue;
				break;

			case CShaderDesc::Directional:
				break;
			}
		}

	}

	void CGLSLShaderGenerator::addLightInsFS()
	{
		ss << "smooth in vec4 lightColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addLightsFunctionFS()
	{
		ss << "vec4 applyLights(vec4 col)" << std::endl;
		ss << "{" << std::endl;
		ss << "return col * lightColor;" << std::endl;
		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::addLightsFS()
	{
		ss << "fragColor = applyLights(fragColor);" << std::endl;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*void CGLSLShaderGenerator::generateSpecularVS()
	{
		
		ss << "uniform mat4 modelView;" << std::endl;
		ss << "uniform mat4 texMatrix0;" << std::endl;
		ss << "smooth out vec3 cubeTexCoords;" << std::endl;

		if (desc->fogEnabled() || desc->hasPointLight())
		{
			ss << "vec4 ecPos4;" << std::endl;
		}
		
		if (desc->fogEnabled())
			ss << "smooth out vec4 ecPos;" << std::endl;
		ss << std::endl;

		if (desc->lightingEnabled())
		{
			addViewMatrix();
			addNormalMatrix();
			addLightUniformsVS();
			addLightOutsVS();
			ss << std::endl;

			addLightsFunctionVS();
			ss << std::endl;
		}

		ss << "vec3 ReflectionMap(const in vec3 eyePos, const in vec3 normal)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 u = normalize(eyePos);" << std::endl;
		ss << "return reflect(u, normal);" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 eyePosition = modelView * v" << g_AttribNames[ 0 ] << ";" << std::endl;

		if (desc->hasPointLight())
			ss << "ecPos4 = eyePosition;" << std::endl;
		
		if (desc->fogEnabled())
			ss << "ecPos = eyePosition;" << std::endl;

		ss << "vec3 ep = eyePosition.xyz / eyePosition.w;" << std::endl;
		ss << "vec3 n = vnormal.xyz;" << std::endl;
		ss << "cubeTexCoords = ReflectionMap(ep, n);" << std::endl;
		ss << "vec4 t = vec4(cubeTexCoords, 1.0);" << std::endl;
		ss << "t = t * texMatrix0;" << std::endl;
		ss << "cubeTexCoords = t.xyz;" << std::endl;
		ss << "gl_Position = modelViewProjection * v" << g_AttribNames[ 0 ] << ";" << std::endl;

		if (desc->lightingEnabled())
			addLightsVS();

		for (int i = Weight; i < NumOffsets; i++)
		{
			if (hasFlag(vbFormat, g_VertexFlags[i]))
			{
				ss << g_AttribNames[i];
				ss << " = ";
				ss << "v" << g_AttribNames[i] << ";" << std::endl;
			}
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generatePPLVS()
	{
		ss << "uniform vec4 lightPosition;" << std::endl;
		
		if (material->getShader() == CMaterial::PerPixelLighting)
		{
			ss << "uniform vec4 viewerPos;" << std::endl;
			ss << "uniform mat4 invModelMat;" << std::endl;
			
		}
		ss << std::endl;

		ss << "smooth out vec3 cubeTexCoords0;" << std::endl;

		if (material->getShader() == CMaterial::PerPixelLighting)
			ss << "smooth out vec3 cubeTexCoords2;" << std::endl;

		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;
		ss << "gl_Position = modelViewProjection * v" << g_AttribNames[ 0 ] << ";" << std::endl;
		
		for (int i = Weight; i < NumOffsets; i++)
		{
			if (hasFlag(vbFormat, g_VertexFlags[i]))
			{
				ss << g_AttribNames[i];
				ss << " = ";
				ss << "v" << g_AttribNames[i] << ";" << std::endl;
			}
		}

		ss << "vec4 n = normalize(vnormal); //normalized normal" << std::endl;
		ss << "vec4 T; // second basis, Tangent" << std::endl;
		ss << "T = texCoord1;" << std::endl;
		ss << "T = T - n * dot(N, T); // Gramm-Schmidt process" << std::endl;
		ss << "T = normalize(T);" << std::endl;
		ss << "vec4 B;" << std::endl;
		ss << "B.xyz = cross(n.xyz, T.xyz); // B = N x T" << std::endl;
		ss << "vec4 L = lightPos - vposition; //Inverse light vector" << std::endl;
		ss << "L = normalize(L);" << std::endl;
		ss << "L * [ T B N ]" << std::endl;
		ss << "cubeTexCoords0.x = dot(T.xyz, L.xyz);" << std::endl;
		ss << "cubeTexCoords0.y = dot(B.xyz, L.xyz);" << std::endl;
		ss << "cubeTexCoords0.z = dot(n.xyz, L.xyz);" << std::endl;

		if (material->getShader() == CMaterial::PerPixelLighting)
		{
			ss << "vec4 V = invModelMat * viewerPos - vposition;" << std::endl;
			ss << "V = normalize(V);" << std::endl;
			ss << "vec4 H = L + V; // half-angle" << std::endl;
			ss << "vec4 H = normalize(H);" << std::endl;
			ss << "cubeTexCoords2.x = dot(T, H);" << std::endl;
			ss << "cubeTexCoords2.y = dot(B, H);" << std::endl;
			ss << "cubeTexCoords2.w = dot(n, H);" << std::endl;
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateWaterVS()
	{
		bool diffuse = false;
		if (material->getTexture(3))
			diffuse = true;

		ss << "smooth out vec4 texCoord0;" << std::endl;
		ss << "smooth out vec4 texCoord1;" << std::endl;
		ss << "smooth out vec4 texCoord2;" << std::endl;
		ss << "flat out vec4 bump0ScaleBias;" << std::endl;
		ss << "flat out vec4 bump1ScaleBias;" << std::endl;

		if (diffuse)
			ss << "smooth out vec4 texCoord3;" << std::endl;

		ss << std::endl;

		if (diffuse)
		{
			ss << "uniform vec4 diffuseMapVector0;" << std::endl;
			ss << "uniform vec4 diffuseMapVector1;" << std::endl;
		}
		
		ss << "uniform vec4 bumpMap0Scale;" << std::endl;
		ss << "uniform vec4 bumpMap1Scale;" << std::endl;
		ss << "uniform vec4 bumpMap0Offset;" << std::endl;
		ss << "uniform vec4 bumpMap1Offset;" << std::endl;

		// no fog yet
		//ss << "uniform mat4 modelView;" << std::endl;
		ss << std::endl;

		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 eye = vposition;" << std::endl; // FIXME
		ss << "vec4 position = vposition;" << std::endl; // FIXME
		ss << "gl_Position = modelViewProjection * position;" << std::endl;
		ss << "bump0ScaleBias = bumpMap0Scale;" << std::endl;
		ss << "bump1ScaleBias = bumpMap1Scale;" << std::endl;
		
		// no fog yet
		//ss << "vec4 v = modelView[ 3 ];" << std::endl;
		//fog.x = dot(position, v);

		ss << "texCoord0 = position * bumpMap0Scale + bumpMap0Offset;" << std::endl;
		ss << "texCoord1 = position * bumpMap1Scale + bumpMap1Offset;" << std::endl;
		ss << "vec4 eyeDirection = normalize(eye - position);" << std::endl;

		ss << "texCoord2 = -1 * eyeDirection * vec4(0.5, 0.05, 0.0, 1.0) + vec4(0.5, 0.05, 0.0, 1.0);" << std::endl;

		if (diffuse)
		{
			ss << "texCoord3.x = dot(position, diffuseMapVector0);" << std::endl;
			ss << "texCoord3.y = dot(position, diffuseMapVector1);" << std::endl;
		}

		ss << "}" << std::endl;
	}*/

	void CGLSLShaderGenerator::generateInvalidPS()
	{
		/*uint sampler = 0;
		for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
		{
			if (desc->getUseTexStage(i))
				ss << "uniform sampler2D sampler" << sampler << ";" << std::endl;
			sampler++;
		}

		addColor();
		addConstants();
		addAlphaTreshold();
		addFogUniform();*/

		if (desc->fogEnabled())
			ss << "smooth in vec4 ecPos;" << std::endl;

		ss << std::endl;

		if (desc->lightingEnabled())
		{
			addLightUniformsFS();
			addLightInsFS();
			ss << std::endl;
			
			addLightsFunctionFS();
			ss << std::endl;
		}

		/*if (desc->fogEnabled())
			addFogFunction();*/
		
		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;

		/*bool textures = false;
		sampler = 0;
		for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
		{
			if (desc->getUseTexStage(i))
			{
				ss << "vec4 texel" << sampler << " = texture(sampler" << sampler << ",";
				
				if (!desc->getUseFirstTexCoords())
					ss << g_AttribNames[ TexCoord0 + i ] << ".st);";
				else
					ss << g_AttribNames[ TexCoord0 ] << ".st);";

				ss << std::endl;

				textures = true;
			}
			sampler++;
		}

		bool vertexColor = false;
		if (hasFlag(vbFormat, g_VertexFlags[ PrimaryColor ]))
			vertexColor = true;

		if (textures && !vertexColor)
			ss << "vec4 texel = vec4(1.0, 1.0, 1.0, 1.0);" << std::endl;
		else
		if (vertexColor)
			ss << "vec4 texel = color;" << std::endl;
		else
			ss << "vec4 texel = vec4(0.5, 0.5, 0.5, 1.0);" << std::endl;

		generateTexEnv();

		// This is just an idea I had, but it seems to be working.
		// Unfortunately it's not documented anywhere I looked in the GL spec, but if I don't have this modulation here,
		// the Ryzom UI looks horrific.
		if (vertexColor)
			ss << "texel = color * texel;" << std::endl;*/

		ss << "fragColor = vec4(1.0, 1.0, 1.0, 1.0);" << std::endl;

		if (desc->lightingEnabled())
			addLightsFS();

		ss << "fragColor = fragColor + vec4(0.25, 0.0, 0.0, 0.0);" << std::endl;

		/*if (desc->fogEnabled())
			addFog();

		addAlphaTest();*/

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateNormalPS()
	{
		uint sampler = 0;
		for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
		{
			if (desc->getUseTexStage(i))
				ss << "uniform sampler2D sampler" << sampler << ";" << std::endl;
			sampler++;
		}

		addColor();
		addConstants();
		addAlphaTreshold();
		addFogUniform();

		if (desc->fogEnabled())
			ss << "smooth in vec4 ecPos;" << std::endl;

		ss << std::endl;

		if (desc->lightingEnabled())
		{
			addLightUniformsFS();
			addLightInsFS();
			ss << std::endl;
			
			addLightsFunctionFS();
			ss << std::endl;
		}

		if (desc->fogEnabled())
			addFogFunction();
		
		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;

		// Light color
		ss << "vec4 diffuse = vec4(1.0, 1.0, 1.0, 1.0);" << std::endl; // FIXME: vertex color?
		if (desc->lightingEnabled())
			ss << "diffuse = applyLights(diffuse);" << std::endl;

		bool textures = false;
		for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
		{
			if (desc->getUseTexStage(i))
			{
				ss << "vec4 texel" << i << " = texture(sampler" << i << ", ";				
				if (!desc->getUseFirstTexCoords()) // FIXME: What is this???
					ss << g_AttribNames[ TexCoord0 + i ] << ".st);";
				else
					ss << g_AttribNames[ TexCoord0 ] << ".st);";
				ss << std::endl;
				textures = true;
			}
		}

		bool vertexColor = false;
		if (hasFlag(vbFormat, g_VertexFlags[ PrimaryColor ]))
			vertexColor = true;

		/*if (textures && !vertexColor)
			ss << "vec4 texel = vec4(1.0, 1.0, 1.0, 1.0);" << std::endl;
		else if (vertexColor)
			ss << "vec4 texel = color;" << std::endl;
		else
			ss << "vec4 texel = vec4(0.5, 0.5, 0.5, 1.0);" << std::endl;*/

		generateTexEnv();

		ss << "fragColor = texop" << (IDRV_MAT_MAXTEXTURES - 1) << ";" << std::endl;

		// This is just an idea I had, but it seems to be working.
		// Unfortunately it's not documented anywhere I looked in the GL spec, but if I don't have this modulation here,
		// the Ryzom UI looks horrific.
		if (vertexColor)
			ss << "fragColor = color * fragColor;" << std::endl;

		//if (desc->lightingEnabled())
		//	addLightsFS();

		if (desc->fogEnabled())
			addFog();

		addAlphaTest();

		// ss << "fragColor = fragColor + vec4(0.0, 0.25, 0.0, 0.0);" << std::endl;

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateTexEnv()
	{
		for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
		{
			if (desc->getUseTexStage(stage))
			{
				for (uint arg = 0; arg < 3; ++arg)
				{
					// Texop arg
					ss << "vec4 texop" << stage << "arg" << arg << ";" << std::endl;

					// RGB
					uint rgbArg = material->_TexEnvs[stage].getColorArg(arg);
					uint rgbOp = material->_TexEnvs[stage].getColorOperand(arg);
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
					uint alphaArg = material->_TexEnvs[stage].getAlphaArg(arg);
					uint alphaOp = material->_TexEnvs[stage].getAlphaOperand(arg);
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
				ss << "texop" << stage << ".rgb = ";
				switch (material->_TexEnvs[stage].Env.OpRGB)
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
					ss << "texop" << stage << "arg0.rgb * vec3(0.5, 0.5, 0.5)"; // TODO !!!!
					break;
				case CMaterial::Mad:
					ss << "texop" << stage << "arg0.rgb * texop" << stage << "arg1.rgb + texop" << stage << "arg2.rgb";
					break;
				}
				ss << ";" << std::endl;

				// Alpha
				ss << "texop" << stage << ".a = ";
				switch (material->_TexEnvs[stage].Env.OpAlpha)
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
					ss << "texop" << stage << "arg0.a * 0.5"; // TODO !!!!
					break;
				case CMaterial::Mad:
					ss << "texop" << stage << "arg0.a * texop" << stage << "arg1.a + texop" << stage << "arg2.a";
					break;
				}
				ss << ";" << std::endl;
			}
			else if (stage == 0)
			{
				ss << "vec4 texop" << stage << " = diffuse;" << std::endl;
			}
			else
			{
				ss << "vec4 texop" << stage << " = texop" << (stage - 1) << ";" << std::endl;
			}
		}
	}


	void CGLSLShaderGenerator::generateLightMapPS()
	{
		int ls = material->_LightMaps.size();
		ls++; // lightmaps + color texture

		int ntextures = 0;
		for (int i = TexCoord0; i < TexCoord4; i++)
		{
			if (hasFlag(vbFormat, g_VertexFlags[i]))
				ntextures++;
		}

		ntextures = std::max(ntextures, ls);

		for (int i = 0; i < ntextures; i++)
			ss << "uniform sampler2D sampler" << i << ";" << std::endl;

		addConstants();

		addDiffuse();

		addAlphaTreshold();

		addFogUniform();

		if (desc->fogEnabled())
			ss << "smooth in vec4 ecPos;" << std::endl;

		ss << std::endl;

		if (desc->lightingEnabled())
		{
			addLightUniformsFS();
			addLightInsFS();
			ss << std::endl;
			
			addLightsFunctionFS();
			ss << std::endl;
		}

		if (desc->fogEnabled())
			addFogFunction();

		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;

		// Lightmap UV coords are at position 1
		for (int i = 0; i < ntextures - 1; i++)
		{
			ss << "vec4 texel" << i;
			ss << " = texture(sampler" << i;
			ss << ", " << g_AttribNames[ TexCoord1 ] << ".st);" << std::endl;
		}

		// Color map UV coords are at position 0
		ss << "vec4 texel" << ntextures - 1 << " = texture(sampler" << ntextures - 1 << ", " << g_AttribNames[ TexCoord0 ] << ".st);" << std::endl;
		
		//ss << "vec4 texel = diffuseColor;" << std::endl;
		//ss << "vec4 texel = vec4(1.0, 1.0, 1.0, 1.0);" << std::endl;
		ss << "vec4 texel = vec4(0.0, 0.0, 0.0, 1.0);" << std::endl;

		// Lightmaps
		for (int i = 0; i < ntextures - 1; i++)
		{
			ss << "texel.rgb = " << g_TexelNames[i] << ".rgb * " << g_ConstantNames[i] << ".rgb + texel.rgb;" << std::endl;
			ss << "texel.a = " << g_TexelNames[i] << ".a * texel.a + texel.a;" << std::endl;
		}

		// Texture
		ss << "texel.rgb = " << g_TexelNames[ ntextures - 1 ] << ".rgb * texel.rgb;" << std::endl;
		ss << "texel.a = " << g_TexelNames[ ntextures - 1] << ".a;" << std::endl;

		if (material->_LightMapsMulx2)
		{
			ss << "texel.rgb = texel.rgb * 2.0;" << std::endl;
			ss << "texel.a = texel.a * 2.0;" << std::endl;
		}

		ss << "fragColor = texel;" << std::endl;

		if (desc->lightingEnabled())
			addLightsFS();
		
		if (desc->fogEnabled())
			addFog();

		addAlphaTest();

		ss << "}" << std::endl;
		ss << std::endl;
	}

	void CGLSLShaderGenerator::generateSpecularPS()
	{
		ss << "smooth in vec3 cubeTexCoords;" << std::endl;
		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform samplerCube sampler1;" << std::endl;
		addDiffuse();
		addAlphaTreshold();
		addFogUniform();

		if (desc->lightingEnabled())
		{
			addLightUniformsFS();
			addLightInsFS();
			ss << std::endl;
			
			addLightsFunctionFS();
			ss << std::endl;
		}

		if (desc->fogEnabled())
			ss << "smooth in vec4 ecPos;" << std::endl;

		ss << std::endl;

		if (desc->fogEnabled())
			addFogFunction();

		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;		
		ss << "vec4 texel0 = texture(sampler0, texCoord0.st);" << std::endl;
		ss << "vec4 texel1 = texture(sampler1, cubeTexCoords);" << std::endl;
		ss << "vec4 texel;" << std::endl;
		ss << "texel.rgb = texel0.rgb * diffuseColor.rgb;" << std::endl;
		ss << "texel.a = texel0.a;" << std::endl;
		ss << "texel.rgb = texel1.rgb * texel.a + texel.rgb;" << std::endl;
		ss << "texel.a = texel1.a;" << std::endl;
		ss << "fragColor = texel;" << std::endl;

		if (desc->lightingEnabled())
			addLightsFS();
		
		if (desc->fogEnabled())
			addFog();
		
		addAlphaTest();

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generatePPLPS()
	{
		ss << "smooth in vec3 cubeTexCoords0;" << std::endl;
		if (material->getShader() == CMaterial::PerPixelLighting)
			ss << "smooth in vec3 cubeTexCoords2;" << std::endl;
		ss << std::endl;

		ss << "uniform samplerCube sampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;

		if (material->getShader() == CMaterial::PerPixelLighting)
			ss << "uniform samplerCube sampler2;" << std::endl;

		addDiffuse();

		addConstants();

		addAlphaTreshold();

		ss << std::endl;

		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;
		
		ss << "vec4 texel0 = texture(sampler0, cubeTexCoords0);" << std::endl;
		ss << "vec4 texel1 = texture(sampler1, texCoord1.st);" << std::endl;

		if (material->getShader() == CMaterial::PerPixelLighting)
			ss << "vec4 texel2 = texture(sampler2, cubeTexCoords2);" << std::endl;

		ss << "vec4 texel;" << std::endl;

		if (material->getShader() == CMaterial::PerPixelLighting)
		{
			ss << "texel.rgb = texel0.rgb * constant0.rgb + constant0.rgb;" << std::endl;
			ss << "texel.rgb = texel1.rgb * texel.rgb;" << std::endl;
			ss << "texel.rgb = texel2.rgb * constant2.rgb + texel.rgb;" << std::endl;
			ss << "texel.a = texel0.a * diffuseColor.a" << std::endl;
			ss << "texel.a = texel1.a * texel.a;" << std::endl;
		}
		else
		{
			ss << "texel.rgb = texel0.rgb * constant0.rgb + color.rgb;" << std::endl;
			ss << "texel.rgb = texel1.rgb * texel.rgb;" << std::endl;
			ss << "texel.a = texel0.a * diffuseColor.a;" << std::endl;
			ss << "texel.a = texel1.a * diffuseColor.a;" << std::endl;
		}

		ss << "fragColor = texel;" << std::endl;
		addAlphaTest();
		ss << "}" << std::endl;
	}


	void CGLSLShaderGenerator::generateWaterPS()
	{
		bool diffuse = false;
		if (material->getTexture(3) != NULL)
			diffuse = true;

		ss << "smooth in vec4 texCoord0;" << std::endl;
		ss << "smooth in vec4 texCoord1;" << std::endl;
		ss << "smooth in vec4 texCoord2;" << std::endl;

		if (diffuse)
			ss << "smooth in vec4 texCoord3;" << std::endl;

		ss << "flat in vec4 bump0ScaleBias;" << std::endl;
		ss << "flat in vec4 bump1ScaleBias;" << std::endl;

		ss << std::endl;

		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;
		ss << "uniform sampler2D sampler2;" << std::endl;

		if (diffuse)
			ss << "uniform sampler2D sampler3;" << std::endl;

		addAlphaTreshold();

		ss << std::endl;

		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 texel0 = texture(sampler0, texCoord0.st);" << std::endl;
		ss << "texel0 = texel0 * bump0ScaleBias.xxxx + bump0ScaleBias.yyzz;" << std::endl;
		ss << "texel0 = texel0 + texCoord1;" << std::endl;
		ss << "vec4 texel1 = texture(sampler1, texel0.st);" << std::endl;
		ss << "texel1 = texel1 * bump1ScaleBias.xxxx + bump1ScaleBias.yyzz;" << std::endl;
		ss << "texel1 = texel1 + texCoord2;" << std::endl;
		ss << "vec4 texel2 = texture(sampler2, texel1.st);" << std::endl;

		if (diffuse)
		{
			ss << "vec4 texel3 = texture(sampler3, texCoord3.st);" << std::endl;
			ss << "texel3 = texel3 * texel2;" << std::endl;
		}
		
		// No fog yet, so for later
		//vec4 tmpFog = clamp(fogValue.x * fogFactor.x + fogFactor.y);
		//vec4 fragColor = mix(texel3, fogColor, tmpFog.x);
		
		if (diffuse)
			ss << "fragColor = texel3;" << std::endl;
		else
			ss << "fragColor = texel2;" << std::endl;

		addAlphaTest();

		ss << "}" << std::endl;

		ss << std::endl;
	}

	void CGLSLShaderGenerator::generateCloudPS()
	{
		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;
		addDiffuse();
		addAlphaTreshold();
		ss << std::endl;

		ss << "void main(void)" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 tex0 = texture(sampler0, texCoord0.st);" << std::endl;
		ss << "vec4 tex1 = texture(sampler1, texCoord1.st);" << std::endl;
		ss << "vec4 tex = mix(tex0, tex1, diffuse.a);" << std::endl;
		ss << "tex.a = 0;" << std::endl;
		ss << "fragColor = tex;" << std::endl;
		addAlphaTest();
		ss << "}" << std::endl;
		ss << std::endl;

	}
}


