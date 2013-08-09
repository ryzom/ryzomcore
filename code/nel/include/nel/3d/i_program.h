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


#ifndef I_PROGRAM_H
#define I_PROGRAM_H

#include <string>

namespace NL3D
{
	/// Interface class for all NEL GPU ( vertex, pixel, geometry, etc ) programs
	class IProgram
	{
	public:
		enum EProgramType
		{
			VERTEX_PROGRAM,
			PIXEL_PROGRAM,
			GEOMETRY_PROGRAM,
			UNKNOWN_PROGRAM
		};

		IProgram(){
			shaderId = 0;
			compiled = false;
			type = UNKNOWN_PROGRAM;
		}

		virtual ~IProgram(){}

		virtual void shaderSource( const char *source ) = 0;

		virtual bool compile( std::string &log ) = 0;

		unsigned int getShaderId() const{ return shaderId; }

		bool isCompiled() const{ return compiled; }

		bool isVertexProgram() const{
			if( type == VERTEX_PROGRAM )
				return true;
			else
				return false;
		}

		bool isPixelProgram() const{
			if( type == PIXEL_PROGRAM )
				return true;
			else
				return false;
		}

		bool isGeometryProgram() const{
			if( type == GEOMETRY_PROGRAM )
				return true;
			else
				return false;
		}

	protected:
		unsigned int shaderId;
		bool compiled;
		EProgramType type;
	};
}

#endif


