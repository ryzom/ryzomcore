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

#include "sstream"
#include "driver_glsl_shader_generator.h"
#include "nel/3d/vertex_buffer.h"

namespace
{
	inline bool hasFlag( uint32 data, uint32 flag )
	{
		if( ( data & flag ) != 0 )
			return true;
		else
			return false;
	}

	enum AttribOffset
	{
		Position,
		Weight,
		Normal,
		PrimaryColor,
		SecondaryColor,
		Fog,
		PaletteSkin,
		Empty,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,
		NumOffsets
	};
}

namespace NL3D
{
	uint16 vertexFlags[ CVertexBuffer::NumValue ] = 
	{
		CVertexBuffer::PositionFlag,
		CVertexBuffer::WeightFlag,
		CVertexBuffer::NormalFlag,
		CVertexBuffer::PrimaryColorFlag,
		CVertexBuffer::SecondaryColorFlag,
		CVertexBuffer::FogFlag,
		CVertexBuffer::PaletteSkinFlag,
		0,
		CVertexBuffer::TexCoord0Flag,
		CVertexBuffer::TexCoord1Flag,
		CVertexBuffer::TexCoord2Flag,
		CVertexBuffer::TexCoord3Flag,
		CVertexBuffer::TexCoord4Flag,
		CVertexBuffer::TexCoord5Flag,
		CVertexBuffer::TexCoord6Flag,
		CVertexBuffer::TexCoord7Flag
	};

	const char *attribNames[ CVertexBuffer::NumValue ] =
	{
		"position",
		"weight",
		"normal",
		"color",
		"color2",
		"fog",
		"paletteSkin",
		"none",
		"texCoord0",
		"texCoord1",
		"texCoord2",
		"texCoord3",
		"texCoord4",
		"texCoord5",
		"texCoord6",
		"texCoord7"
	};

	const char *texelNames[ 4 ] =
	{
		"texel0",
		"texel1",
		"texel2",
		"texel3"
	};

	const char *constantNames[ 4 ] =
	{
		"constant0",
		"constant1",
		"constant2",
		"constant3"
	};

	CGLSLShaderGenerator::CGLSLShaderGenerator()
	{
		reset();
	}

	CGLSLShaderGenerator::~CGLSLShaderGenerator()
	{
	}

	void CGLSLShaderGenerator::reset()
	{
		material = NULL;
		vbFormat = 0;
		ss.str( "" );
		ss.clear();
	}

	void CGLSLShaderGenerator::generateVS( std::string &vs )
	{
		ss.str( "" );
		ss.clear();

		ss << "#version 330" << std::endl;
		ss << "uniform mat4 mvpMatrix;" << std::endl;
		ss << std::endl;

		for( int i = Position; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "layout ( location = ";
				ss << i;
				ss << " ) ";
				ss << "in vec4 ";
				ss << "v" << attribNames[ i ];
				ss << ";";
				ss << std::endl;
			}
		}
		ss << std::endl;

		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "smooth out vec4 ";
				ss << attribNames[ i ] << ";" << std::endl;
			}
		}
		ss << std::endl;

		switch( material->getShader() )
		{
		case CMaterial::Normal:
		case CMaterial::UserColor:
		case CMaterial::LightMap:
		case CMaterial::Cloud:
			generateNormalVS();
			break;

		case CMaterial::Specular:
			generateSpecularVS();
			break;

		case CMaterial::PerPixelLighting:
		case CMaterial::PerPixelLightingNoSpec:
			generatePPLVS();
			break;

		case CMaterial::Water:
			generateWaterVS();
			break;
		}
		
		vs.assign( ss.str() );
	}

	void CGLSLShaderGenerator::generatePS( std::string &ps )
	{
		ss.str( "" );
		ss.clear();

		ss << "#version 330" << std::endl;
		ss << std::endl;

		ss << "out vec4 fragColor;" << std::endl;

		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "smooth in vec4 ";
				ss << attribNames[ i ] << ";" << std::endl;
			}
		}
		ss << std::endl;
		
		switch( material->getShader() )
		{
		case CMaterial::Normal:
		case CMaterial::UserColor:
			generateNormalPS();
			break;

		case CMaterial::LightMap:
			generateLightMapPS();
			break;

		case CMaterial::Specular:
			generateSpecularPS();
			break;

		case CMaterial::PerPixelLighting:
		case CMaterial::PerPixelLightingNoSpec:
			generatePPLPS();
			break;

		case CMaterial::Water:
			generateWaterPS();
			break;

		case CMaterial::Cloud:
			generateCloudPS();
			break;
		}

		ps.assign( ss.str() );
	}

	void CGLSLShaderGenerator::addDiffuse()
	{
		ss << "float diffuse  = vec4( ";
		ss << float( material->getDiffuse().R / 255.0f ) << ", ";
		ss << float( material->getDiffuse().G / 255.0f ) << ", ";
		ss << float( material->getDiffuse().B / 255.0f ) << ", ";
		ss << float( material->getDiffuse().A / 255.0f ) << " );";
		ss << std::endl;
	}

	void CGLSLShaderGenerator::addConstants()
	{
		int j = 0;
		for( int i = TexCoord0; i < TexCoord4; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "vec4 " << constantNames[ j ] << " = vec4( ";
				ss << float( material->_TexEnvs[ j ].ConstantColor.R / 255.0f ) << ", ";
				ss << float( material->_TexEnvs[ j ].ConstantColor.G / 255.0f ) << ", ";
				ss << float( material->_TexEnvs[ j ].ConstantColor.B / 255.0f ) << ", ";
				ss << float( material->_TexEnvs[ j ].ConstantColor.A / 255.0f ) << " );";
				ss << std::endl;
			}
			j++;
		}
	}

	void CGLSLShaderGenerator::generateNormalVS()
	{
		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "gl_Position = mvpMatrix * " << "v" << attribNames[ 0 ] << ";" << std::endl;

		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << attribNames[ i ];
				ss << " = ";
				ss << "v" << attribNames[ i ] << ";" << std::endl;
			}
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateSpecularVS()
	{
		
		ss << "uniform mat4 mvMatrix;" << std::endl;
		ss << "uniform mat4 texMatrix;" << std::endl;
		ss << "smooth out vec3 cubeTexCoords;" << std::endl;
		ss << std::endl;

		ss << "vec3 ReflectionMap( const in vec3 eyePos, const in vec3 normal )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 u = normalize( eyePos );" << std::endl;
		ss <<"return reflect( u, normal );" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 eyePosition = mvMatrix * v" << attribNames[ 0 ] << ";" << std::endl;
		ss << "cubeTexCoords = ReflectionMap( eyePosition, v" << attribNames[ 2 ] << " );" << std::endl;
		ss << "vec4 t = vec4( cubeTexCoords, 1.0 );" << std::endl;
		ss << "t = t * texMatrix;" << std::endl;
		ss << "cubeTexCoords = t.xyz;" << std::endl;
		ss << "gl_Position = mvpMatrix * v" << attribNames[ 0 ] << ";" << std::endl;

		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << attribNames[ i ];
				ss << " = ";
				ss << "v" << attribNames[ i ] << ";" << std::endl;
			}
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generatePPLVS()
	{
		ss << "uniform vec4 lightPosition;" << std::endl;
		
		if( material->getShader() == CMaterial::PerPixelLighting )
		{
			ss << "uniform vec4 viewerPos;" << std::endl;
			ss << "uniform mat4 invModelMat;" << std::endl;
			
		}
		ss << std::endl;

		ss << "smooth out vec3 cubeTexCoords0;" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
			ss << "smooth out vec3 cubeTexCoords2;" << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "gl_Position = mvpMatrix * v" << attribNames[ 0 ] << ";" << std::endl;
		
		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << attribNames[ i ];
				ss << " = ";
				ss << "v" << attribNames[ i ] << ";" << std::endl;
			}
		}

		ss << "vec4 n = normalize( vnormal ); //normalized normal" << std::endl;
		ss << "vec4 T; // second basis, Tangent" << std::endl;
		ss << "T = texCoord1;" << std::endl;
		ss << "T = T - n * dot( N, T ); // Gramm-Schmidt process" << std::endl;
		ss << "T = normalize( T );" << std::endl;
		ss << "vec4 B;" << std::endl;
		ss << "B.xyz = cross( n.xyz, T.xyz ); // B = N x T" << std::endl;
		ss << "vec4 L = lightPos - vposition; //Inverse light vector" << std::endl;
		ss << "L = normalize( L );" << std::endl;
		ss << "L * [ T B N ]" << std::endl;
		ss << "cubeTexCoords0.x = dot( T.xyz, L.xyz );" << std::endl;
		ss << "cubeTexCoords0.y = dot( B.xyz, L.xyz );" << std::endl;
		ss << "cubeTexCoords0.z = dot( n.xyz, L.xyz );" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
		{
			ss << "vec4 V = invModelMat * viewerPos - vposition;" << std::endl;
			ss << "V = normalize( V );" << std::endl;
			ss << "vec4 H = L + V; // half-angle" << std::endl;
			ss << "vec4 H = normalize( H );" << std::endl;
			ss << "cubeTexCoords2.x = dot( T, H );" << std::endl;
			ss << "cubeTexCoords2.y = dot( B, H );" << std::endl;
			ss << "cubeTexCoords2.w = dot( n, H );" << std::endl;
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateWaterVS()
	{
		bool diffuse = false;
		if( material->getTexture( 3 ) )
			diffuse = true;

		ss << "smooth out vec4 texCoord0;" << std::endl;
		ss << "smooth out vec4 texCoord1;" << std::endl;
		ss << "smooth out vec4 texCoord2;" << std::endl;
		ss << "flat out vec4 bump0ScaleBias;" << std::endl;
		ss << "flat out vec4 bump1ScaleBias;" << std::endl;

		if( diffuse )
			ss << "smooth out vec4 texCoord3;" << std::endl;

		ss << std::endl;

		if( diffuse )
		{
			ss << "uniform vec4 diffuseMapVector0;" << std::endl;
			ss << "uniform vec4 diffuseMapVector1;" << std::endl;
		}
		
		ss << "uniform vec4 bumpMap0Scale;" << std::endl;
		ss << "uniform vec4 bumpMap1Scale;" << std::endl;
		ss << "uniform vec4 bumpMap0Offset;" << std::endl;
		ss << "uniform vec4 bumpMap1Offset;" << std::endl;

		// no fog yet
		//ss << "uniform mat4 mvMatrix;" << std::endl;
		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "position = vposition;" << std::endl;
		ss << "gl_Position = mvpMatrix * position;" << std::endl;
		ss << "bump0ScaleBias = bumpMap0Scale;" << std::endl;
		ss << "bump1ScaleBias = bumpMap1Scale;" << std::endl;
		
		// no fog yet
		//ss << "vec4 v = mvMatrix[ 3 ];" << std::endl;
		//fog.x = dot( position, v );

		ss << "texCoord0 = position * bumpMap0Scale + bumpMap0Offset;" << std::endl;
		ss << "texCoord1 = position * bumpMap1Scale + bumpMap1Offset;" << std::endl;
		ss << "vec4 eyeDirection = normalize( eye - position );" << std::endl;

		ss << "texCoord2 = -1 * eyeDirection * vec4( 0.5, 0.05, 0.0, 1.0 ) + vec4( 0.5, 0.05, 0.0, 1.0 );" << std::endl;

		if( diffuse )
		{
			ss << "texCoord3.x = dot( position, diffuseMapVector0 );" << std::endl;
			ss << "texCoord3.y = dot( position, diffuseMapVector1 );" << std::endl;
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateNormalPS()
	{
		uint sampler = 0;
		for( int i = TexCoord0; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "uniform sampler2D sampler" << sampler;
				ss << ";";
				ss << std::endl;
			}
			sampler++;
		}
		ss << std::endl;
		
		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;

		addDiffuse();

		sampler = 0;
		for( int i = TexCoord0; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "vec4 texel" << sampler;
				ss << " = texture2D( sampler" << sampler << ",";
				ss << attribNames[ i ] << " );" << std::endl;
			}
			sampler++;
		}

		ss << "vec4 texel = vec4( 0.0, 0.0, 0.0, 1.0 );" << std::endl;

		generateTexEnv();
	
		ss << "fragColor = texel;" << std::endl;
		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateTexEnv()
	{
		addConstants();

		uint32 stage = 0;
		for( int i = TexCoord0; i < TexCoord4; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				generateTexEnvRGB( stage );
				generateTexEnvAlpha( stage );
			}

			stage++;
		}
	}

	void CGLSLShaderGenerator::generateTexEnvRGB( unsigned int stage )
	{
		std::string arg0;
		std::string arg1;
		std::string arg2;
	
		switch( material->_TexEnvs[ stage ].Env.OpRGB )
		{
		case CMaterial::Replace:
			{
				buildArg( stage, 0, false, arg0 );
				ss << "texel.rgb = " << arg0 << ";" << std::endl;
			}
			break;
		
		case CMaterial::Modulate:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );
				ss << "texel.rgb = " << arg0 << ";" << std::endl;
				ss << "texel.rgb = texel.rgb * " << arg1 << ";" << std::endl;
			}
			break;
		
		case CMaterial::Add:
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );
				ss << "texel.rgb = " << arg0 << ";" << std::endl;
				ss << "texel.rgb = texel.rgb + " << arg1 << ";" << std::endl;
			break;
		
		case CMaterial::AddSigned:
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );
				ss << "texel.rgb = " << arg0 << ";" << std::endl;
				ss << "texel.rgb = texel.rgb + " << arg1 << ";" << std::endl;
				ss << "texel.rgb = texel.rgb - vec3( 0.5, 0.5, 0.5 );" << std::endl;
			break;

		case CMaterial::Mad:
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );
				buildArg( stage, 2, false, arg2 );
				ss << "texel.rgb = " << arg0 << ";" << std::endl;
				ss << "texel.rgb = texel.rgb * " << arg1 << ";" << std::endl;
				ss << "texel.rgb = texel.rgb + " << arg2 << ";" << std::endl;
			break;

		case CMaterial::InterpolateTexture:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );

				std::string As = texelNames[ stage ];
				As.append( ".a" );

				ss << "texel.rgb = " << arg0 << " * " << As << ";";
				ss << "texel.rgb = texel.rgb + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolatePrevious:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );

				std::string As = "texel.a";

				ss << "texel.rgb = " << arg0 << " * " << As << ";";
				ss << "texel.rgb = texel.rgb + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolateDiffuse:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );

				std::string As = "diffuse.a";

				ss << "texel.rgb = " << arg0 << " * " << As << ";";
				ss << "texel.rgb = texel.rgb + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolateConstant:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );

				std::string As = constantNames[ stage ];
				As.append( ".a" );

				ss << "texel.rgb = " << arg0 << " * " << As << ";";
				ss << "texel.rgb = texel.rgb + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;
		}
	}

	void CGLSLShaderGenerator::generateTexEnvAlpha( unsigned int stage )
	{
		std::string arg0;
		std::string arg1;
		std::string arg2;
	
		switch( material->_TexEnvs[ stage ].Env.OpRGB )
		{
		case CMaterial::Replace:
			{
				buildArg( stage, 0, true, arg0 );
				ss << "texel.a = " << arg0 << ";" << std::endl;
			}
			break;
		
		case CMaterial::Modulate:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );
				ss << "texel.a = " << arg0 << ";" << std::endl;
				ss << "texel.a = texel.a * " << arg1 << ";" << std::endl;
			}
			break;
		
		case CMaterial::Add:
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );
				ss << "texel.a = " << arg0 << ";" << std::endl;
				ss << "texel.a = texel.a + " << arg1 << ";" << std::endl;
			break;
		
		case CMaterial::AddSigned:
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );
				ss << "texel.rgb = " << arg0 << ";" << std::endl;
				ss << "texel.rgb = texel.rgb + " << arg1 << ";" << std::endl;
				ss << "texel.rgb = texel.rgb - vec3( 0.5, 0.5, 0.5 );" << std::endl;
			break;

		case CMaterial::Mad:
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );
				buildArg( stage, 2, true, arg2 );
				ss << "texel.a = " << arg0 << ";" << std::endl;
				ss << "texel.a = texel.a * " << arg1 << ";" << std::endl;
				ss << "texel.a = texel.a + " << arg2 << ";" << std::endl;
			break;

		case CMaterial::InterpolateTexture:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );

				std::string As = texelNames[ stage ];
				As.append( ".a" );

				ss << "texel.a = " << arg0 << " * " << As << ";";
				ss << "texel.a = texel.a + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolatePrevious:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );

				std::string As = "texel.a";

				ss << "texel.a = " << arg0 << " * " << As << ";";
				ss << "texel.a = texel.a + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolateDiffuse:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );

				std::string As = "diffuse.a";

				ss << "texel.a = " << arg0 << " * " << As << ";";
				ss << "texel.a = texel.a + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolateConstant:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );

				std::string As = constantNames[ stage ];
				As.append( ".a" );

				ss << "texel.a = " << arg0 << " * " << As << ";";
				ss << "texel.a = texel.a + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;
		}
	}


	void CGLSLShaderGenerator::buildArg( unsigned int stage, unsigned int n, bool alpha, std::string &arg )
	{
		uint32 src;
		uint32 op;
		
		if( !alpha )
		{
			src = material->_TexEnvs[ n ].getColorArg( n );
			op  = material->_TexEnvs[ n ].getColorOperand( n );
		}
		else
		{
			src = material->_TexEnvs[ n ].getAlphaArg( n );
			op  = material->_TexEnvs[ n ].getAlphaOperand( n );
		}

		std::stringstream ds;

		switch( src )
		{
		case CMaterial::Texture:
			{

				switch( op )
				{
				case CMaterial::SrcColor:
					ds << texelNames[ stage ] << ".rgb";
					break;

				case CMaterial::InvSrcColor:
					ds << "( vec3( 1.0, 1.0, 1.0 ) - ";
					ds << texelNames[ stage ] << ".rgb )";
					break;

				case CMaterial::SrcAlpha:
					ds << texelNames[ stage ] << ".a";
					break;

				case CMaterial::InvSrcAlpha:
					ds << "( 1.0 - ";
					ds << texelNames[ stage ] << ".a )";
					break;
				}
			}
			break;

		case CMaterial::Previous:

				switch( op )
				{
				case CMaterial::SrcColor:
					ds << "texel.rgb";
					break;

				case CMaterial::InvSrcColor:
					ds << "( vec3( 1.0, 1.0, 1.0 ) - texel.rgb )";
					break;

				case CMaterial::SrcAlpha:
					ds << "texel.a;";
					break;

				case CMaterial::InvSrcAlpha:
					ds << "( 1.0 - texel.a )";
					break;
				}
			break;

		case CMaterial::Diffuse:

				switch( op )
				{
				case CMaterial::SrcColor:
					ds << "diffuse.rgb";
					break;

				case CMaterial::InvSrcColor:
					ds << "( vec3( 1.0, 1.0, 1.0 ) - diffuse.rgb )";
					break;

				case CMaterial::SrcAlpha:
					ds << "diffuse.a";
					break;

				case CMaterial::InvSrcAlpha:
					ds << "( 1.0 - diffuse.a )";
					break;
				}
			break;

		case CMaterial::Constant:

				switch( op )
				{
				case CMaterial::SrcColor:
					ss << constantNames[ stage ] << ".rgb";
					break;

				case CMaterial::InvSrcColor:
					ss << "( vec3( 1.0, 1.0, 1.0 ) - " << constantNames[ stage ] << ".rgb )";
					break;

				case CMaterial::SrcAlpha:
					ss << constantNames[ stage ] << ".a";
					break;

				case CMaterial::InvSrcAlpha:
					ss << "( 1.0 - " << constantNames[ stage ] << ".a )";
					break;
				}
			break;
		}

		arg.assign( ds.str() );
	}


	void CGLSLShaderGenerator::generateLightMapPS()
	{
		uint sampler = 0;
		for( int i = TexCoord0; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "uniform sampler2D sampler" << sampler;
				ss << ";";
				ss << std::endl;
			}
			sampler++;
		}
		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;

		addDiffuse();

		addConstants();
		
		sampler = 0;
		for( int i = TexCoord0; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "vec4 texel" << sampler;
				ss << " = texture2D( sampler" << sampler;
				ss << ", " << attribNames[ i ] << " );" << std::endl;
			}
			sampler++;
		}

		ss << "vec4 texel = diffuse;" << std::endl;

		sampler = 0;
		for( int i = TexCoord0; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "texel = ";
				ss << texelNames[ sampler ] << " * " << constantNames[ sampler ] << " + texel;" << std::endl;
			}
			sampler++;
		}

		ss << "fragColor = texel;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;
	}

	void CGLSLShaderGenerator::generateSpecularPS()
	{
		ss << "smooth in vec3 cubeTexCoords;" << std::endl;
		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform samplerCube cubeSampler;" << std::endl;
		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		
		addDiffuse();

		ss << "vec4 texel0 = texture2D( sampler0, texCoord0 );" << std::endl;
		ss << "vec4 texel1 = textureCube( cubeSampler, cubeTexCoords );" << std::endl;
		ss << "vec4 texel;" << std::endl;
		ss << "texel.rgb = texel0.rgb * diffuse;" << std::endl;
		ss << "texel.a = texel0.a;" << std::endl;
		ss << "texel.rgb = texel1.rgb * texel.alpha + texel.rgb;" << std::endl;
		ss << "texel.a = color.a;" << std::endl;
		ss << "fragColor = texel;" << std::endl;
		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generatePPLPS()
	{
		ss << "smooth in vec3 cubeTexCoords0;" << std::endl;
		if( material->getShader() == CMaterial::PerPixelLighting )
			ss << "smooth in vec3 cubeTexCoords2;" << std::endl;
		ss << std::endl;

		ss << "uniform samplerCube cubeSampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
			ss << "uniform samplerCube cubeSampler2;" << std::endl;

		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		
		addConstants();

		addDiffuse();

		ss << "vec4 texel0 = textureCube( cubeSampler0, cubeTexCoords0 );" << std::endl;
		ss << "vec4 texel1 = texture2D( sampler1, texCoord1 );" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
			ss << "vec4 texel2 = textureCube( cubeSampler2, cubeTexCoords2 );" << std::endl;

		ss << "vec4 texel;" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
		{
			ss << "texel.rgb = texel0.rgb * constant0.rgb + constant0.rgb;" << std::endl;
			ss << "texel.rgb = texel1.rgb * texel.rgb;" << std::endl;
			ss << "texel.rgb = texel2.rgb * constant2.rgb + texel.rgb;" << std::endl;
			ss << "texel.a = texel0.a * diffuse.a" << std::endl;
			ss << "texel.a = texel1.a * texel.a;" << std::endl;
		}
		else
		{
			ss << "texel.rgb = texel0.rgb * constant0.rgb + color.rgb;" << std::endl;
			ss << "texel.rgb = texel1.rgb * texel.rgb;" << std::endl;
			ss << "texel.a = texel0.a * diffuse.a;" << std::endl;
			ss << "texel.a = texel1.a * diffuse.a;" << std::endl;
		}

		ss << "fragColor = texel;" << std::endl;
		ss << "}" << std::endl;
	}


	void CGLSLShaderGenerator::generateWaterPS()
	{
		bool diffuse = false;
		if( material->getTexture( 3 ) != NULL )
			diffuse = true;

		ss << "smooth in texCoord0;" << std::endl;
		ss << "smooth in texCoord1;" << std::endl;
		ss << "smooth in texCoord2;" << std::endl;

		if( diffuse )
			ss << "smooth in texCoord3;" << std::endl;

		ss << "flat in vec4 bump0ScaleBias;" << std::endl;
		ss << "flat in vec4 bump1ScaleBias;" << std::endl;

		ss << std::endl;

		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;
		ss << "uniform sampler2D sampler2;" << std::endl;

		if( diffuse )
			ss << "uniform sampler2D sampler3;" << std::endl;

		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 texel0 = texture2D( sampler0, texCoord0 );" << std::endl;
		ss << "texel0 = texel0 * bump0ScaleBias.xxxx + bump0ScaleBias.yyzz;" << std::endl;
		ss << "texel0 = texel0 + texCoord1;" << std::endl;
		ss << "vec4 texel1 = texture2D( sampler1, texel0 );" << std::endl;
		ss << "texel1 = texel1 * bump1ScaleBias.xxxx + bump1ScaleBias.yyzz;" << std::endl;
		ss << "texel1 = texel1 + texCoord2;" << std::endl;
		ss << "vec4 texel2 = texture2D( sampler2, texel1 );" << std::endl;

		if( diffuse )
		{
			ss << "vec4 texel3 = texture2D( sampler3, texCoord3 );" << std::endl;
			ss << "texel3 = texel3 * texel2;" << std::endl;
		}
		
		// No fog yet, so for later
		//vec4 tmpFog = clamp( fogValue.x * fogFactor.x + fogFactor.y );
		//vec4 fragColor = mix( texel3, fogColor, tmpFog.x );
		
		if( diffuse )
			ss << "fragColor = texel3;" << std::endl;
		else
			ss << "fragColor = texel2" << std::endl;

		ss << "}" << std::endl;

		ss << std::endl;
	}

	void CGLSLShaderGenerator::generateCloudPS()
	{
		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;
		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;

		addDiffuse();

		ss << "vec4 tex0 = texture2D( sampler0, texCoord0 );" << std::endl;
		ss << "vec4 tex1 = texture2D( sampler1, texCoord1 );" << std::endl;
		ss << "vec4 tex = mix( tex0, tex1, diffuse.a );" << std::endl;
		ss << "tex.a = 0;" << std::endl;
		ss << "fragColor = tex;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

	}
}


