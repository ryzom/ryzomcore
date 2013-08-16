#include "driver_opengl.h"
#include "driver_glsl_program.h"
#include "driver_glsl_vertex_program.h"
#include "driver_glsl_pixel_program.h"
#include "driver_glsl_shader_generator.h"
#include "driver_opengl_vertex_buffer_hard.h"
#include "nel/3d/i_program_object.h"

namespace NL3D
{
	bool CDriverGL3::activeProgramObject( IProgramObject *program )
	{
		if( !program )
		{
			_VertexProgramEnabled = false;
			currentProgram = NULL;
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
		currentProgram = program;

		return true;
	}


	IProgramObject* CDriverGL3::createProgramObject() const
	{
		return new CGLSLProgram();
	}

	IProgram* CDriverGL3::createVertexProgram() const
	{
		return new CGLSLVertexProgram();
	}

	IProgram* CDriverGL3::createPixelProgram() const
	{
		return new CGLSLPixelProgram();
	}

	int CDriverGL3::getUniformLocation( const char *name )
	{
		if( currentProgram == NULL )
			return -1;

		return nglGetUniformLocation( currentProgram->getProgramId(), name );
	}


	void CDriverGL3::setUniform1f( uint index, float f )
	{
		nglUniform1f( index, f );
	}

	void CDriverGL3::setUniform4f( uint index, float f1, float f2, float f3, float f4 )
	{
		nglUniform4f( index, f1, f2, f3, f4 );
	}

	void CDriverGL3::setUniform1i( uint index, int i )
	{
		nglUniform1i( index, i );
	}

	void CDriverGL3::setUniform4i( uint index, int i1, int i2, int i3, int i4 )
	{
		nglUniform4i( index, i1, i2, i3, i4 );
	}

	void CDriverGL3::setUniform1u( uint index, uint u )
	{
		nglUniform1ui( index, u );
	}

	void CDriverGL3::setUniform4u( uint index, uint u1, uint u2, uint u3, uint u4 )
	{
		nglUniform4ui( index, u1, u2, u3, u4 );
	}

	void CDriverGL3::setUniformMatrix2fv( uint index, uint count, bool transpose, const float *values )
	{
		nglUniformMatrix2fv( index, count, transpose, values );
	}

	void CDriverGL3::setUniformMatrix3fv( uint index, uint count, bool transpose, const float *values )
	{
		nglUniformMatrix3fv( index, count, transpose, values );
	}

	void CDriverGL3::setUniformMatrix4fv( uint index, uint count, bool transpose, const float *values )
	{
		nglUniformMatrix4fv( index, count, transpose, values );
	}

	bool CDriverGL3::renderRawTriangles2( CMaterial &mat, uint32 startIndex, uint32 numTris )
	{
		glDrawArrays( GL_TRIANGLES, startIndex * 3, numTris * 3 );

		return true;
	}

	bool CDriverGL3::renderTriangles2( CMaterial &mat, uint32 startIndex, uint32 numTris )
	{
		if( !setupProgram( mat ) )
			return false;

		glDrawArrays( GL_TRIANGLES, startIndex * 3, numTris * 3 );

		return true;
	}

	bool CDriverGL3::setupProgram( CMaterial &mat )
	{
		std::string vs;
		std::string ps;

		shaderGenerator->reset();
		shaderGenerator->setMaterial( &mat );
		shaderGenerator->setVBFormat( _CurrentVertexBufferHard->VB->getVertexFormat() );
		shaderGenerator->generateVS( vs );
		shaderGenerator->generatePS( ps );

		return true;
	}
}


