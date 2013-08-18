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
	}

	void CGLSLShaderGenerator::generateVS( std::string &vs )
	{
		std::stringstream ss;

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
		
		vs.assign( ss.str() );
	}

	void CGLSLShaderGenerator::generatePS( std::string &ps )
	{
		std::stringstream ss;

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

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "fragColor = ";

		//////////////////// obviously this is just temporary /////////
		ss << "vec4( 1.0, 0.0, 0.0, 1.0 );" << std::endl;
		/////////////////////////////////////////////////////

		ss << "}" << std::endl;

		ps.assign( ss.str() );
	}

}


