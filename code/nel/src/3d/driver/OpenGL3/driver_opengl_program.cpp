#include "driver_opengl.h"
#include "driver_glsl_program.h"

namespace NL3D
{
	bool CDriverGL3::activeGLSLProgram( CGLSLProgram *program )
	{
		if( !program )
		{
			_VertexProgramEnabled = false;
			return true;
		}

		if( program->isLinked() )
			return false;
		
		nglValidateProgram( program->getProgramId() );

		GLint ok;
		nglGetProgramiv( program->getProgramId(), GL_VALIDATE_STATUS, &ok );
		if( ok != GL_TRUE )
			return false;

		nglUseProgram( program->getProgramId() );

		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
			return false;

		_VertexProgramEnabled = true;

		return true;
	}
}


