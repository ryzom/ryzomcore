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


#ifndef I_PROGRAM_H
#define I_PROGRAM_H

#include <string>

namespace NL3D
{
	/// Interface class for all NEL GPU ( vertex, pixel, geometry, etc ) programs
	class IProgram
	{
	public:
		enum TProgramType
		{
			INVALID_PROGRAM,
			VERTEX_PROGRAM,
			PIXEL_PROGRAM,
			GEOMETRY_PROGRAM
		};

		enum EUniform
		{
			MVPMatrix,
			MVMatrix,
			NormalMatrix,
			TexMatrix0,
			TexMatrix1,
			TexMatrix2,
			TexMatrix3,
			Constant0,
			Constant1,
			Constant2,
			Constant3,
			Diffuse,
			Color,
			Sampler0,
			Sampler1,
			Sampler2,
			Sampler3,
			AlphaTreshold,
			FogStart,
			FogEnd,
			FogColor,
			FogDensity,
			Light0Dir,
			Light1Dir,
			Light2Dir,
			Light3Dir,
			Light4Dir,
			Light5Dir,
			Light6Dir,
			Light7Dir,
			Light0ColDiff,
			Light1ColDiff,
			Light2ColDiff,
			Light3ColDiff,
			Light4ColDiff,
			Light5ColDiff,
			Light6ColDiff,
			Light7ColDiff,
			Light0ColAmb,
			Light1ColAmb,
			Light2ColAmb,
			Light3ColAmb,
			Light4ColAmb,
			Light5ColAmb,
			Light6ColAmb,
			Light7ColAmb,
			Light0ColSpec,
			Light1ColSpec,
			Light2ColSpec,
			Light3ColSpec,
			Light4ColSpec,
			Light5ColSpec,
			Light6ColSpec,
			Light7ColSpec,
			Light0Shininess,
			Light1Shininess,
			Light2Shininess,
			Light3Shininess,
			Light4Shininess,
			Light5Shininess,
			Light6Shininess,
			Light7Shininess,
			Light0Pos,
			Light1Pos,
			Light2Pos,
			Light3Pos,
			Light4Pos,
			Light5Pos,
			Light6Pos,
			Light7Pos,
			Light0ConstAttn,
			Light1ConstAttn,
			Light2ConstAttn,
			Light3ConstAttn,
			Light4ConstAttn,
			Light5ConstAttn,
			Light6ConstAttn,
			Light7ConstAttn,
			Light0LinAttn,
			Light1LinAttn,
			Light2LinAttn,
			Light3LinAttn,
			Light4LinAttn,
			Light5LinAttn,
			Light6LinAttn,
			Light7LinAttn,
			Light0QuadAttn,
			Light1QuadAttn,
			Light2QuadAttn,
			Light3QuadAttn,
			Light4QuadAttn,
			Light5QuadAttn,
			Light6QuadAttn,
			Light7QuadAttn,
			NUM_UNIFORMS
		};

		IProgram(){
			type = INVALID_PROGRAM;
		}

		virtual ~IProgram(){}

		void setSource( const char *source ){ src.assign( source ); }
		const std::string& getSource() const{ return src; }

		virtual void cacheUniforms() = 0;
		virtual int getUniformIndex( uint32 id ) const = 0;

		TProgramType getType() const{ return type; }

	protected:
		std::string src;
		static const char *uniformNames[ NUM_UNIFORMS ];
		TProgramType type;
	};
}

#endif


