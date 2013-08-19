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

	class CGLSLShaderGenerator
	{
	public:
		CGLSLShaderGenerator();
		~CGLSLShaderGenerator();
		void reset();

		void generateVS( std::string &vs );
		void generatePS( std::string &ps );

		void setMaterial( CMaterial *mat ){ material = mat; }
		void setVBFormat( uint16 format ){ vbFormat = format; }

	private:
		void generateNormalPS();
		void generateTexEnv();
		void generateTexEnvRGB( unsigned int stage );
		void generateTexEnvAlpha( unsigned int stage );
		void buildArg( unsigned int stage, unsigned int n, bool alpha, std::string &arg );

		void generateLightMapPS();

		std::stringstream ss;
		uint16 vbFormat;
		CMaterial const *material;
	};
}

#endif


