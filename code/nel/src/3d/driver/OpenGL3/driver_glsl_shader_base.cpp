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


#include "driver_glsl_shader_base.h"
#include "stdopengl.h"
#include "driver_opengl_extension.h"

#define MAX_SHADER_COMPILE_INFOLOG 1024


namespace NL3D
{

	void CGLSLShaderBase::shaderSource( const char *source )
	{
		nlassert( shaderId != 0 );

		const GLchar *p[1];
		p[ 0 ] = source;
		GLint lengths[ 1 ];
		lengths[ 0 ] = strlen( source );

		nglShaderSource( shaderId, 1, p, lengths );
	}

	bool CGLSLShaderBase::compile( std::string &log )
	{
		nglCompileShader( shaderId );

		GLint ok;
		nglGetShaderiv( shaderId, GL_COMPILE_STATUS, &ok );

		if( ok != 0 )
		{
			char infoLog[ MAX_SHADER_COMPILE_INFOLOG ];
			nglGetShaderInfoLog( shaderId, MAX_SHADER_COMPILE_INFOLOG, NULL, infoLog );
			log.assign( infoLog );
			return false;
		}

		compiled = true;

		return true;
	}

}




