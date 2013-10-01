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


#include "driver_glsl_program.h"
#include <algorithm>
#include "driver_opengl_extension.h"

namespace NL3D
{
	CGLSLProgram::CGLSLProgram() :
	IProgram()
	{
		std::fill( uniformIndices, uniformIndices + NUM_UNIFORMS, -1 );
		programId = 0;
	}

	CGLSLProgram::~CGLSLProgram()
	{
	}

	void CGLSLProgram::cacheUniforms()
	{
		nlassert( programId != 0 );

		for( int i = 0; i < NUM_UNIFORMS; i++ )
		{
			uniformIndices[ i ] = nglGetUniformLocation( programId, uniformNames[ i ] );
		}
	}

}


