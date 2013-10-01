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
#include "nel/3d/dynamic_material.h"
#include "nel/3d/usr_shader_manager.h"
#include "nel/3d/usr_shader_program.h"

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
	bool CDriverGL3::compileVertexProgram( CGLSLVertexProgram *program )
	{
		// Already compiled
		if( program->getProgramId() != 0 )
			return true;

		const char *s = program->getSource().c_str();
		glGetError();
		unsigned int id = nglCreateShaderProgramv( GL_VERTEX_SHADER, 1, &s );

		if( id == 0 )
			return false;

		GLint ok;
		nglGetProgramiv( id, GL_LINK_STATUS, &ok );
		if( ok == 0 )
		{
			char errorLog[ 1024 ];
			nglGetProgramInfoLog( id, 1024, NULL, errorLog );
			nlinfo( "%s", errorLog );
			return false;
		}


		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
			return false;

		program->setProgramId( id );

		return true;
	}

	bool CDriverGL3::activeVertexProgram( CGLSLVertexProgram *program )
	{
		if( program->getProgramId() == 0 )		
			return false;

		glGetError();

		nglUseProgramStages( ppoId, GL_VERTEX_SHADER_BIT, program->getProgramId() );

		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
		{
			
			return false;
		}

		vertexProgram = program;

		return true;
	}

	bool CDriverGL3::compilePixelProgram( CGLSLPixelProgram *program )
	{
		// Already compiled
		if( program->getProgramId() != 0 )
			return true;

		const char *s = program->getSource().c_str();
		glGetError();
		unsigned int id = nglCreateShaderProgramv( GL_FRAGMENT_SHADER, 1, &s );
		if( id == 0 )
			return false;

		GLint ok;
		nglGetProgramiv( id, GL_LINK_STATUS, &ok );
		if( ok == 0 )
		{
			char errorLog[ 1024 ];
			nglGetProgramInfoLog( id, 1024, NULL, errorLog );
			nlinfo( "%s", errorLog );
			return false;
		}

		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
			return false;

		program->setProgramId( id );

		return true;
	}

	bool CDriverGL3::activePixelProgram( CGLSLPixelProgram *program )
	{
		if( program->getProgramId() == 0 )
			return false;

		glGetError();

		nglUseProgramStages( ppoId, GL_FRAGMENT_SHADER_BIT, program->getProgramId() );

		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
		{
			return false;
		}

		pixelProgram = program;

		return true;
	}

	IProgram* CDriverGL3::createVertexProgram() const
	{
		return new CGLSLVertexProgram();
	}

	IProgram* CDriverGL3::createPixelProgram() const
	{
		return new CGLSLPixelProgram();
	}

	int CDriverGL3::getUniformLocation( uint32 programType, const char *name )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			return nglGetUniformLocation( vertexProgram->getProgramId(), name );
			break;

		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			return nglGetUniformLocation( pixelProgram->getProgramId(), name );
			break;
		}

		return -1;
	}


	void CDriverGL3::setUniform1f( uint32 programType, uint index, float f )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniform1f( vertexProgram->getProgramId(), index, f );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniform1f( pixelProgram->getProgramId(), index, f );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniform3f( uint32 programType, uint index, float f1, float f2, float f3 )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniform3f( vertexProgram->getProgramId(), index, f1, f2, f3 );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniform3f( pixelProgram->getProgramId(), index, f1, f2, f3 );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniform4f( uint32 programType, uint index, float f1, float f2, float f3, float f4 )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniform4f( vertexProgram->getProgramId(), index, f1, f2, f3, f4 );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniform4f( pixelProgram->getProgramId(), index, f1, f2, f3, f4 );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniform1i( uint32 programType, uint index, int i )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniform1i( vertexProgram->getProgramId(), index, i );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniform1i( pixelProgram->getProgramId(), index, i );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniform4i( uint32 programType, uint index, int i1, int i2, int i3, int i4 )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniform4i( vertexProgram->getProgramId(), index, i1, i2, i3, i4 );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniform4i( pixelProgram->getProgramId(), index, i1, i2, i3, i4 );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniform1u( uint32 programType, uint index, uint u )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniform1ui( vertexProgram->getProgramId(), index, u );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniform1ui( pixelProgram->getProgramId(), index, u );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniform4u( uint32 programType, uint index, uint u1, uint u2, uint u3, uint u4 )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniform4ui( vertexProgram->getProgramId(), index, u1, u2, u3, u4 );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniform4ui( pixelProgram->getProgramId(), index, u1, u2, u3, u4 );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniformMatrix2fv( uint32 programType, uint index, uint count, bool transpose, const float *values )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniformMatrix2fv( vertexProgram->getProgramId(), index, count, transpose, values );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniformMatrix2fv( pixelProgram->getProgramId(), index, count, transpose, values );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniformMatrix3fv( uint32 programType, uint index, uint count, bool transpose, const float *values )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniformMatrix3fv( vertexProgram->getProgramId(), index, count, transpose, values );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniformMatrix3fv( pixelProgram->getProgramId(), index, count, transpose, values );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::setUniformMatrix4fv( uint32 programType, uint index, uint count, bool transpose, const float *values )
	{
		switch( programType )
		{
		case IProgram::VERTEX_PROGRAM:
			nlassert( vertexProgram != NULL );
			nglProgramUniformMatrix4fv( vertexProgram->getProgramId(), index, count, transpose, values );
			break;
		case IProgram::PIXEL_PROGRAM:
			nlassert( pixelProgram != NULL );
			nglProgramUniformMatrix4fv( pixelProgram->getProgramId(), index, count, transpose, values );
			break;

		default:
			nlassert( false );
			break;
		}
	}

	void CDriverGL3::generateShaderDesc( CShaderDesc &desc, CMaterial &mat )
	{
		desc.setShaderType( mat.getShader() );
		desc.setVBFlags( _CurrentVertexBufferHard->VB->getVertexFormat() );
		
		if( mat.getShader() == CMaterial::LightMap )
			desc.setNLightMaps( mat._LightMaps.size() );
		
		int i = 0;

		if( mat.getShader() == CMaterial::Normal )
		{
			int maxTextures = std::min( int( SHADER_MAX_TEXTURES ), int( IDRV_MAT_MAXTEXTURES ) );
			for( i = 0; i < maxTextures; i++ )
			{
				desc.setTexEnvMode( i, mat.getTexEnvMode( i ) );
			}
		}

		if( mat.getAlphaTest() )
		{
			desc.setAlphaTest( true );
			desc.setAlphaTestThreshold( mat.getAlphaTestThreshold() );
		}

		if( fogEnabled() )
		{
			desc.setFog( true );
			desc.setFogMode( CShaderDesc::Linear );
		}

		int maxLights = std::min( int( SHADER_MAX_LIGHTS ), int( MaxLight ) );
		bool enableLights = false;
		for( int i = 0; i < maxLights; i++ )
		{
			if( !_UserLightEnable[ i ] )
				continue;

			enableLights = true;
			
			switch( _LightMode[ i ] )
			{
			case CLight::DirectionalLight:
				desc.setLight( i, CShaderDesc::Directional );
				break;
			
			case CLight::PointLight:
				desc.setLight( i, CShaderDesc::Point );
				break;
			
			case CLight::SpotLight:
				desc.setLight( i, CShaderDesc::Spot );
				break;
			}
		
		}

		desc.setLighting( enableLights );			
	}


	bool CDriverGL3::setupProgram( CMaterial &mat )
	{
		if( mat.getDynMat() != NULL )
			return true;
		
		CGLSLVertexProgram *vp = NULL;
		CGLSLPixelProgram *pp = NULL;
		SShaderPair sp;

		CShaderDesc desc;

		generateShaderDesc( desc, mat );

		sp = shaderCache.findShader( desc );

		if( !sp.empty() )
		{
			if( !activeVertexProgram( sp.vp ) )
				return false;

			if( !activePixelProgram( sp.pp ) )
				return false;
		}
		else
		{
			std::string vs;
			std::string ps;

			shaderGenerator->reset();
			shaderGenerator->setMaterial( &mat );
			shaderGenerator->setVBFormat( _CurrentVertexBufferHard->VB->getVertexFormat() );
			shaderGenerator->setShaderDesc( &desc );
			shaderGenerator->generateVS( vs );
			shaderGenerator->generatePS( ps );

			vp = new CGLSLVertexProgram();
			vp->setSource( vs.c_str() );
			if( !compileVertexProgram( vp ) )
			{
				delete vp;
				vp = NULL;
				return false;
			}
		
			pp = new CGLSLPixelProgram();
			pp->setSource( ps.c_str() );
			if( !compilePixelProgram( pp ) )
			{
				delete vp;
				vp = NULL;
				delete pp;
				pp = NULL;
				return false;
			}

			if( !activeVertexProgram( vp ) )
			{
				delete vp;
				vp = NULL;
				delete pp;
				pp = NULL;
				return false;
			}

			if( !activePixelProgram( pp ) )
			{
				delete vp;
				vp = NULL;
				delete pp;
				pp = NULL;
				return false;
			}
			
			vp->cacheUniforms();
			pp->cacheUniforms();
			sp.vp = vp;
			sp.pp = pp;
			desc.setShaders( sp );
			shaderCache.cacheShader( desc );
		}

		setupUniforms();

		return true;
	}

	bool CDriverGL3::setupDynMatProgram( CMaterial& mat, uint pass )
	{
		CDynMaterial *m = mat.getDynMat();
		const SRenderPass *rp = m->getPass( pass );
		std::string shaderRef;
		rp->getShaderRef( shaderRef );

		NL3D::CUsrShaderProgram prg;

		if( !usrShaderManager->getShader( shaderRef, &prg ) )
			return false;
		
		CGLSLVertexProgram *vp = new CGLSLVertexProgram();
		CGLSLPixelProgram *pp = new CGLSLPixelProgram();

		std::string shaderSource;
		std::string log;
		prg.getVP( shaderSource );
		vp->setSource( shaderSource.c_str() );
		if( !compileVertexProgram( vp ) )
		{
			delete vp;
			delete pp;
			return false;
		}

		prg.getFP( shaderSource );
		pp->setSource( shaderSource.c_str() );
		if( !compilePixelProgram( pp ) )
		{
			delete vp;
			delete pp;
			return false;
		}

		if( !activeVertexProgram( vp ) )
		{
			delete vp;
			delete pp;
			return false;
		}

		if( !activePixelProgram( pp ) )
		{
			delete vp;
			delete pp;
			return false;
		}

		if( dynMatVP != NULL )
			delete dynMatVP;
		dynMatVP = vp;

		if( dynMatPP != NULL )
			delete dynMatPP;
		dynMatPP = pp;

		vp->cacheUniforms();
		pp->cacheUniforms();

		return true;
	}


	void CDriverGL3::setupUniforms()
	{
		setupUniforms( vertexProgram );
		setupUniforms( pixelProgram );
	}

	void CDriverGL3::setupUniforms( CGLSLProgram *program )
	{
		CMaterial &mat = *_CurrentMaterial;
		CGLSLProgram *currentProgram = program;
		uint32 type = program->getType();

		int mvpIndex = currentProgram->getUniformIndex( IProgram::MVPMatrix );
		if( mvpIndex != -1 )
		{
			CMatrix mat = _GLProjMat * _ModelViewMatrix;
			setUniformMatrix4fv( type, mvpIndex, 1, false, mat.get() );
		}

		int mvIndex = currentProgram->getUniformIndex( IProgram::MVMatrix );
		if( mvIndex != -1 )
		{
			setUniformMatrix4fv( type, mvIndex, 1, false, _ModelViewMatrix.get() );
		}

		/*
		int nmIdx = currentProgram->getUniformIndex( IProgram::NormalMatrix );
		if( nmIdx != -1 )
		{
		}
		*/

		int fogStartIdx = currentProgram->getUniformIndex( IProgram::FogStart );
		if( fogStartIdx != -1 )
		{
			setUniform1f( type, fogStartIdx, getFogStart() );
		}

		int fogEndIdx = currentProgram->getUniformIndex( IProgram::FogEnd );
		if( fogEndIdx != -1 )
		{
			setUniform1f( type, fogEndIdx, getFogEnd() );
		}

		int fogColorIdx = currentProgram->getUniformIndex( IProgram::FogColor );
		if( fogColorIdx != -1 )
		{
			GLfloat glCol[ 4 ];
			CRGBA col = getFogColor();
			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;
			setUniform4f( type, fogColorIdx, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
		}

		int colorIndex = currentProgram->getUniformIndex( IProgram::Color );
		if( colorIndex != -1 )
		{
			GLfloat glCol[ 4 ];
			CRGBA col = mat.getColor();
			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;

			setUniform4f( type, colorIndex, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
		}

		int diffuseIndex = currentProgram->getUniformIndex( IProgram::Diffuse );
		if( diffuseIndex != -1 )
		{
			GLfloat glCol[ 4 ];
			CRGBA col = mat.getDiffuse();
			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;

			setUniform4f( type, diffuseIndex, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
		}


		int maxLights = std::min( int( MaxLight ), int( SHADER_MAX_LIGHTS ) );
		for( int i = 0; i < maxLights; i++ )
		{
			if( !_UserLightEnable[ i ] )
				continue;
			
			////////////////// Temporary insanity  ///////////////////////////////
			if( ( _LightMode[ i ] != CLight::DirectionalLight ) && ( _LightMode[ i ] != CLight::PointLight ) )
				continue;
			//////////////////////////////////////////////////////////////////////
			
			int ld = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0Dir + i ) );
			if( ld != -1 )
			{
				CVector v = _UserLight[ i ].getDirection();
				setUniform3f( type, ld, v.x, v.y, v.z );
			}

			int lp = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0Pos + i ) );
			if( lp != -1 )
			{
				CVector v = _UserLight[ i ].getPosition();
				setUniform3f( type, lp, v.x, v.y, v.z );
			}

			int ldc = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0ColDiff + i ) );
			if( ldc != -1 )
			{
				GLfloat glCol[ 4 ];
				CRGBA col = _UserLight[ i ].getDiffuse();
				glCol[ 0 ] = col.R / 255.0f;
				glCol[ 1 ] = col.G / 255.0f;
				glCol[ 2 ] = col.B / 255.0f;
				glCol[ 3 ] = col.A / 255.0f;
				setUniform4f( type, ldc, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
			}

			int lsc = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0ColSpec + i ) );
			if( lsc != -1 )
			{
				GLfloat glCol[ 4 ];
				CRGBA col = _UserLight[ i ].getSpecular();
				glCol[ 0 ] = col.R / 255.0f;
				glCol[ 1 ] = col.G / 255.0f;
				glCol[ 2 ] = col.B / 255.0f;
				glCol[ 3 ] = col.A / 255.0f;
				setUniform4f( type, lsc, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
			}

			int shl = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0Shininess + i ) );
			if( shl != -1 )
			{
				setUniform1f( type, shl, mat.getShininess() );
			}

			int lac = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0ColAmb + i ) );
			if( lac != -1 )
			{
				GLfloat glCol[ 4 ];
				CRGBA col;
				if( mat.getShader() == CMaterial::LightMap )
					col = _UserLight[ i ].getAmbiant();
				else
					col.add( _UserLight[ i ].getAmbiant(), mat.getEmissive() );

				glCol[ 0 ] = col.R / 255.0f;
				glCol[ 1 ] = col.G / 255.0f;
				glCol[ 2 ] = col.B / 255.0f;
				glCol[ 3 ] = col.A / 255.0f;
				setUniform4f( type, lac, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
			}

			int lca = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0ConstAttn + i ) );
			if( lca != -1 )
			{
				setUniform1f( type, lca, _UserLight[ i ].getConstantAttenuation() );
			}

			int lla = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0LinAttn + i ) );
			if( lla != -1 )
			{
				setUniform1f( type, lla, _UserLight[ i ].getLinearAttenuation() );
			}

			int lqa = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Light0QuadAttn + i ) );
			if( lqa != -1 )
			{
				setUniform1f( type, lqa, _UserLight[ i ].getQuadraticAttenuation() );
			}
		}


		// Lightmaps have special constants
		if( mat.getShader() != CMaterial::LightMap )
		{
		
			for( int i = 0; i < IDRV_MAT_MAXTEXTURES; i++ )
			{
				int cl = currentProgram->getUniformIndex( IProgram::EUniform( IProgram::Constant0 + i ) );
				if( cl != -1 )
				{
					CRGBA col = mat._TexEnvs[ i ].ConstantColor;
					GLfloat glCol[ 4 ];
					glCol[ 0 ] = col.R / 255.0f;
					glCol[ 1 ] = col.G / 255.0f;
					glCol[ 2 ] = col.B / 255.0f;
					glCol[ 3 ] = col.A / 255.0f;

					setUniform4f( type, cl, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ] );
				}
			}

		}
	}

	bool CDriverGL3::initPipeline()
	{
		ppoId = 0;

		nglGenProgramPipelines( 1, &ppoId );
		if( ppoId == 0 )
			return false;

		nglBindProgramPipeline( ppoId );

		return true;
	}
}


