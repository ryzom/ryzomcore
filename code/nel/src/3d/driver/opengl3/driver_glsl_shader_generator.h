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


#ifndef GLSL_SHADER_GENERATOR
#define GLSL_SHADER_GENERATOR

#include <string>
#include <sstream>

namespace NL3D
{
	class CMaterial;
	class CShaderDesc;

	/// GLSL 330+ shader program generator
	class CGLSLShaderGenerator
	{
	public:
		CGLSLShaderGenerator();
		~CGLSLShaderGenerator();

		/// Resets the generator to 0.
		void reset();

		/// Generate Pixel Shader based on the data provided in material, descriptor and vertexbuffer flags
		void generatePS(std::string &ps);

		void setMaterial(CMaterial *mat) { material = mat; }
		void setVBFormat(uint16 format) { vbFormat = format; }
		void setShaderDesc(CShaderDesc *d) { desc = d; }

	private:
		/// Adds ambient color constant uniform declaration to the program
		void addAmbient();

		/// Adds diffuse constant uniform declaration  to the program
		void addDiffuse();

		/// Adds specular color constant uniform declaration to the program
		void addSpecular();

		/// Adds Color constant uniform declaration to the program
		void addColor();

		/// Adds constant uniform declarations to the program
		void addConstants();

		/// Adds the normal matrix declaration to the program
		void addNormalMatrix();

		void addViewMatrix();

		//////////////////////////// Alpha Threshold //////////////////
		
		/// Adds the alpha threshold uniform to the program
		void addAlphaTreshold();

		/// Adds the actual alpha test to the program (discards fragment if below threshold)
		void addAlphaTest();

		//////////////////////////////////////////////////////////////

		/////////////////////////// Fog ///////////////////////////////
		
		/// Adds the fog uniforms to the program
		void addFogUniform();

		/// Adds the fog function to the program
		void addFogFunction();

		/// Adds the fog call to the program
		void addFog();

		///////////////////////////////////////////////////////////////

		//////////////////////////// Lights ///////////////////////////
		
		/// Adds the Vertex Shader light uniforms to the program
		void addLightUniformsVS();

		/// Adds the Pixel Shader light uniforms to the program
		void addLightUniformsFS();

		/// Adds the Pixel Shader light output variables to the program
		void addLightInsFS();

		/// Adds the appropriate light functions to the Pixel Shader
		void addLightsFunctionFS();

		/// Adds the lights to the Fragment Shader (calls the appropriate functions)
		void addLightsFS();

		//////////////////////////////////////////////////////////////



		//////////////////////////////////////// Vertex Shader generation ////////////////////////////////////


		void generateSpecularVS();

		/// Per-Pixel Lighting
		void generatePPLVS();

		void generateWaterVS();


		///////////////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////// Pixel Shader generation ///////////////////////////////////////
		
		void generateInvalidPS();
		void generateNormalPS();
		

		void generateTexEnv();

		void generateLightMapPS();
		void generateSpecularPS();
		
		/// Per-Pixel Lighting
		void generatePPLPS();

		void generateWaterPS();

		void generateCloudPS();

		////////////////////////////////////////////////////////////////////////////////////////////////////////

		std::stringstream ss;
		uint16 vbFormat;
		CMaterial const *material;
		CShaderDesc const *desc;
	};
}

#endif


