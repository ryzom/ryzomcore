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


#ifndef OPENGL_SHADER_CACHE
#define OPENGL_SHADER_CACHE

#include "driver_opengl_shader_desc.h"
#include <vector>

namespace NL3D
{
	class CShaderCache
	{
	public:
		CShaderCache();
		~CShaderCache();

		IProgramObject* findShader( const CShaderDesc &desc ) const;
		void cacheShader( CShaderDesc &desc );
		void clearCache();

	private:
		std::vector< CShaderDesc > shaders;

	};
}


#endif

