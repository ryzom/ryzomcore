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

#ifndef SHADER_DESC
#define SHADER_DESC

#include "nel/misc/types_nl.h"

#define MAX_TEXTURES 4

namespace NL3D
{
	class IProgramObject;

	class CShaderDesc
	{
	public:
		CShaderDesc(){
			for( int i = 0; i < MAX_TEXTURES; i++ )
				texEnvMode[ i ] = 0;
			shaderType = 0;
			program = NULL;
			vbFlags = 0;
		}

		~CShaderDesc(){
		}

		bool operator==( const CShaderDesc &o ) const
		{
			if( shaderType != o.shaderType )
				return false;

			if( vbFlags != o.vbFlags )
				return false;

			for( int i = 0; i < MAX_TEXTURES; i++ )
				if( texEnvMode[ i ] != o.texEnvMode[ i ] )
					return false;
		
			return true;
		}

		void setTexEnvMode( uint32 index, uint32 mode ){ texEnvMode[ index ] = mode; }
		void setVBFlags( uint32 flags ){ vbFlags = flags; }
		void setShaderType( uint32 type ){ shaderType = type; }
		void setProgram( IProgramObject *p ){ program = p; }
		IProgramObject* getProgram() const{ return program; }

	private:
		uint32 texEnvMode[ MAX_TEXTURES ];
		uint32 vbFlags;
		uint32 shaderType;
		IProgramObject *program;
	};
}

#endif

