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


#ifndef GLSL_PROGRAM_H
#define GLSL_PROGRAM_H

#include <vector>
#include <string>

namespace NL3D
{
	class CGLSLShaderBase;

	/// Wrapper class for OpenGL shader program object
	class CGLSLProgram
	{
	public:
		CGLSLProgram();
		~CGLSLProgram();

		bool attachVertexProgram( CGLSLShaderBase *shader );
		bool attachPixelProgram( CGLSLShaderBase *shader );

		bool detachVertexProgram( CGLSLShaderBase *shader );
		bool detachPixelProgram( CGLSLShaderBase *shader );		

		bool link( std::string &log );

		unsigned int getProgramId() const{ return programId; }
		bool isLinked() const{ return linked; }

	private:
		void deleteShaders();

		unsigned int programId;
		bool linked;

		std::vector< CGLSLShaderBase* > vertexPrograms;
		std::vector< CGLSLShaderBase* > pixelPrograms;
	};
}

#endif

