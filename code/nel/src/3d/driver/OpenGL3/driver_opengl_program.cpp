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


#include "driver_opengl.h"
#include "driver_glsl_program.h"
#include "driver_glsl_vertex_program.h"
#include "driver_glsl_pixel_program.h"
#include "driver_glsl_shader_generator.h"
#include "driver_opengl_vertex_buffer_hard.h"
#include "nel/3d/i_program_object.h"

namespace
{
	const char *constNames[ NL3D::IDRV_MAT_MAXTEXTURES ] =
	{
		"constant0",
		"constant1",
		"constant2",
		"constant3"
	};

}

namespace NL3D
{
	bool CDriverGL3::activeProgramObject( IProgramObject *program )
	{
		if( !program )
		{
#ifndef GLSL
			_VertexProgramEnabled = false;
#endif
			currentProgram = NULL;
			return true;
		}

		if( !program->isLinked() )
			return false;

		// Release previous program
		releaseProgram();
		
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

	static IProgram *vp;
	static IProgram *pp;
	static IProgramObject *p;


	void CDriverGL3::generateShaderDesc( CShaderDesc &desc, CMaterial &mat )
	{
		desc.setShaderType( mat.getShader() );
		desc.setVBFlags( _CurrentVertexBufferHard->VB->getVertexFormat() );
		
		if( mat.getShader() == CMaterial::LightMap )
			desc.setNLightMaps( mat._LightMaps.size() );
		
		int i = 0;

		if( mat.getShader() == CMaterial::Normal )
		{
			for( i = 0; i < MAX_TEXTURES; i++ )
			{
				desc.setTexEnvMode( i, mat.getTexEnvMode( i ) );
			}
		}

		desc.setAlphaTest( mat.getAlphaTest() );
		desc.setAlphaTestThreshold( mat.getAlphaTestThreshold() );
	}


	bool CDriverGL3::setupProgram( CMaterial &mat )
	{

#ifdef GLSL
		CShaderDesc desc;

		generateShaderDesc( desc, mat );

		p = shaderCache.findShader( desc );

		if( p != NULL )
		{
			if( !activeProgramObject( p ) )
				return false;
		}
		else
		{
			std::string vs;
			std::string ps;

			shaderGenerator->reset();
			shaderGenerator->setMaterial( &mat );
			shaderGenerator->setVBFormat( _CurrentVertexBufferHard->VB->getVertexFormat() );
			shaderGenerator->generateVS( vs );
			shaderGenerator->generatePS( ps );

			vp = createVertexProgram();
			std::string log;

			vp->shaderSource( vs.c_str() );
			if( !vp->compile( log ) )
			{
				delete vp;
				vp = NULL;
				nlinfo( "%s", log.c_str() );
				return false;
			}
		
			pp = createPixelProgram();
			pp->shaderSource( ps.c_str() );
			if( !pp->compile( log ) )
			{
				delete vp;
				vp = NULL;
				delete pp;
				pp = NULL;
				nlinfo( "%s", log.c_str() );
				return false;
			}

			p = createProgramObject();
			p->attachVertexProgram( vp );
			p->attachPixelProgram( pp );
			if( !p->link( log ) )
			{
				vp = NULL;
				pp = NULL;
				delete p;
				p = NULL;
				nlinfo( "%s", log.c_str() );
				return false;
			}

			if( !activeProgramObject( p ) )
				return false;
			
			p->cacheUniformIndices();
			desc.setProgram( p );
			shaderCache.cacheShader( desc );
		}

		setupUniforms( mat );

#endif

		return true;
	}

	void CDriverGL3::setupUniforms( CMaterial& mat )
	{

#ifdef GLSL

		int mvpIndex = currentProgram->getUniformIndex( IProgramObject::MVPMatrix );
		if( mvpIndex != -1 )
		{
			CMatrix mat = _GLProjMat * _ModelViewMatrix;
			setUniformMatrix4fv( mvpIndex, 1, false, mat.get() );
		}

		int colorIndex = currentProgram->getUniformIndex( IProgramObject::Color );
		if( colorIndex != -1 )
		{
			GLfloat glCol[ 4 ];
			CRGBA col = mat.getColor();
			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;

			setUniform4f( colorIndex, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
		}

		int diffuseIndex = currentProgram->getUniformIndex( IProgramObject::Diffuse );
		if( diffuseIndex != -1 )
		{
			GLfloat glCol[ 4 ];
			CRGBA col = mat.getDiffuse();
			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;

			setUniform4f( diffuseIndex, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
		}


		// Lightmaps have special constants
		if( mat.getShader() != CMaterial::LightMap )
		{
		
			for( int i = 0; i < IDRV_MAT_MAXTEXTURES; i++ )
			{
				int cl = currentProgram->getUniformIndex( IProgramObject::EUniform( IProgramObject::Constant0 + i ) );
				if( cl != -1 )
				{
					CRGBA col = mat._TexEnvs[ i ].ConstantColor;
					GLfloat glCol[ 4 ];
					glCol[ 0 ] = col.R / 255.0f;
					glCol[ 1 ] = col.G / 255.0f;
					glCol[ 2 ] = col.B / 255.0f;
					glCol[ 3 ] = col.A / 255.0f;

					setUniform4f( cl, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
				}
			}

		}

#endif
	}

	void CDriverGL3::releaseProgram()
	{
		currentProgram = NULL;

#ifndef GLSL
		_VertexProgramEnabled = false;
#endif
	}

}


