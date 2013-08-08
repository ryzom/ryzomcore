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


#ifndef GLSL_SHADER_BASE_H
#define GLSL_SHADER_BASE_H

namespace NL3D
{
	/// Base class for OpenGL shader objects
	class CGLSLShaderBase
	{
	public:
		CGLSLShaderBase(){
			shaderId = 0;
			compiled = false;
		}

		virtual ~CGLSLShaderBase(){}

		void shaderSource( const char *source );
		bool compile( std::string &log );

		unsigned int getShaderId() const{ return shaderId; }
		bool isCompiled() const{ return compiled; }

	protected:
		unsigned int shaderId;

	private:
		bool compiled;
	};
}


#endif


