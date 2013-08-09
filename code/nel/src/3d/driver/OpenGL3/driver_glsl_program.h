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
#include "nel/3d/i_program_object.h"

namespace NL3D
{
	/// Wrapper class for OpenGL shader program object
	class CGLSLProgram : public IProgramObject
	{
	public:
		CGLSLProgram();
		~CGLSLProgram();

		bool attachVertexProgram( IProgram *shader );
		bool attachPixelProgram( IProgram *shader );

		bool detachVertexProgram( IProgram *shader );
		bool detachPixelProgram( IProgram *shader );		

		bool link( std::string &log );

	private:
		void deleteShaders();

		std::vector< IProgram* > vertexPrograms;
		std::vector< IProgram* > pixelPrograms;
	};
}

#endif

