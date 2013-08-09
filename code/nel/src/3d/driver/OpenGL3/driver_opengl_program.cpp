#include "driver_opengl.h"
#include "driver_glsl_program.h"
#include "driver_glsl_vertex_program.h"
#include "driver_glsl_pixel_program.h"

namespace NL3D
{
	bool CDriverGL3::activeGLSLProgram( CGLSLProgram *program )
	{
		if( !program )
		{
			_VertexProgramEnabled = false;
			return true;
		}

		if( !program->isLinked() )
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



	const char *vs =
		"#version 330\n"
		"in vec4 vertex;\n"
		"void main( void )\n"
		"{\n"
		"gl_Position = vertex;\n"
		"}\n";
	
	const char *fs =
		"#version 330\n"
		"out vec4 color;\n"
		"void main( void )\n"
		"{\n"
		"color = vec4( 1.0, 0.0, 0.0, 1.0 );\n"
		"}\n";
	
	bool CDriverGL3::renderRawTriangles2( CMaterial &mat, uint32 startIndex, uint32 numTris )
	{
		std::string log;

		CGLSLProgram program;
		CGLSLVertexProgram *vp = new CGLSLVertexProgram();
		CGLSLPixelProgram *pp = new CGLSLPixelProgram();

		vp->shaderSource( vs );
		if( !vp->compile( log ) )
		{
			nlinfo( "%s", log.c_str() );
			return false;
		}

		pp->shaderSource( fs );
		if( !pp->compile( log ) )
		{
			nlinfo( "%s", log.c_str() );
			return false;
		}

		if( !program.attachVertexProgram( vp ) )
			return false;

		if( !program.attachPixelProgram( pp ) )
			return false;

		if( !program.link( log ) )
		{
			nlinfo( "%s", log.c_str() );
			return false;
		}

		if( !activeGLSLProgram( &program ) )
			return false;
		
		glDrawArrays( GL_TRIANGLES, startIndex * 3, numTris * 3 );

		activeGLSLProgram( NULL );

		return true;
	}

}


