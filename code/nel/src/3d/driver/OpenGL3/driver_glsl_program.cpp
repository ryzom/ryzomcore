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
#include "driver_glsl_shader_base.h"
#include "stdopengl.h"
#include "driver_opengl_extension.h"

#define MAX_PROGRAM_LINK_ERROR_LOG 1024

namespace NL3D
{
	CGLSLProgram::CGLSLProgram()
	{
		programId = nglCreateProgram();
		nlassert( programId != 0 );
		linked = false;

	}

	CGLSLProgram::~CGLSLProgram()
	{
		nglDeleteProgram( programId );
		deleteShaders();
		programId = 0;
	}

	bool CGLSLProgram::attachVertexProgram( CGLSLShaderBase *shader )
	{
		if( !shader->isVertexProgram() )
			return false;

		if( !shader->isCompiled() )
			return false;

		std::vector< CGLSLShaderBase* >::const_iterator itr =
			std::find( vertexPrograms.begin(), vertexPrograms.end(), shader );
		if( itr != vertexPrograms.end() )
			return false;

		nglAttachShader( programId, shader->getShaderId() );
		GLenum error = glGetError();

		if( error != 0 )
			return false;

		vertexPrograms.push_back( shader );

		return true;
	}

	bool CGLSLProgram::attachPixelProgram( CGLSLShaderBase *shader )
	{
		if( !shader->isPixelProgram() )
			return false;

		if( !shader->isCompiled() )
			return false;

		std::vector< CGLSLShaderBase* >::const_iterator itr =
			std::find( pixelPrograms.begin(), pixelPrograms.end(), shader );
		if( itr != pixelPrograms.end() )
			return false;

		nglAttachShader( programId, shader->getShaderId() );
		GLenum error = glGetError();

		if( error != GL_NO_ERROR )
			return false;

		pixelPrograms.push_back( shader );

		return true;
	}

	bool CGLSLProgram::detachVertexProgram( CGLSLShaderBase *shader )
	{
		if( !shader->isVertexProgram() )
			return false;

		std::vector< CGLSLShaderBase* >::iterator itr =
			std::find( vertexPrograms.begin(), vertexPrograms.end(), shader );
		if( itr == vertexPrograms.end() )
			return false;

		nglDetachShader( programId, shader->getShaderId() );
		GLenum error = glGetError();

		if( error != GL_NO_ERROR )
			return false;

		vertexPrograms.erase( itr );

		return true;
	}


	bool CGLSLProgram::detachPixelProgram( CGLSLShaderBase *shader )
	{
		if( !shader->isPixelProgram() )
			return false;

		std::vector< CGLSLShaderBase* >::iterator itr =
			std::find( pixelPrograms.begin(), pixelPrograms.end(), shader );
		if( itr == pixelPrograms.end() )
			return false;

		nglDetachShader( programId, shader->getShaderId() );
		GLenum error = glGetError();

		if( error != GL_NO_ERROR )
			return false;

		pixelPrograms.erase( itr );

		return true;
	}

	bool CGLSLProgram::link( std::string &log )
	{
		if( vertexPrograms.empty() || pixelPrograms.empty() )
			return false;

		nglLinkProgram( programId );

		GLint ok;
		nglGetProgramiv( programId, GL_LINK_STATUS, &ok );
		if( ok == 0 )
		{
			char errorLog[ MAX_PROGRAM_LINK_ERROR_LOG ];
			nglGetProgramInfoLog( programId, MAX_PROGRAM_LINK_ERROR_LOG, NULL, errorLog );
			log.assign( errorLog );
			return false;
		}

		linked = true;

		return true;
	}


	void CGLSLProgram::deleteShaders()
	{
		std::vector< CGLSLShaderBase* >::iterator itr;
		
		itr = vertexPrograms.begin();
		while( itr != vertexPrograms.end() )
		{
			delete *itr;
			++itr;
		}

		itr = pixelPrograms.begin();
		while( itr != pixelPrograms.end() )
		{
			delete *itr;
			++itr;
		}
	}
}


