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


#include "nel/3d/shader_program.h"

namespace NL3D
{
	CShaderProgram::CShaderProgram()
	{
		vpId = 0;
		fpId = 0;
		pId  = 0;
	}

	CShaderProgram::~CShaderProgram()
	{
	}

	void CShaderProgram::serial( NLMISC::IStream &f )
	{
		f.xmlPush( "ShaderProgram" );

		int version = f.serialVersion( 1 );

		f.xmlPush( "Name" );
		f.serial( name );
		f.xmlPop();

		f.xmlPush( "Description" );
		f.serial( description );
		f.xmlPop();

		f.xmlPush( "VertexProgram" );
		f.serial( vertexProgram );
		f.xmlPop();

		f.xmlPush( "FragmentProgram" );
		f.serial( fragmentProgram );
		f.xmlPop();

		f.xmlPop();
	}
}


