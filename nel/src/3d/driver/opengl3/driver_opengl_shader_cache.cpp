// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#if 0

#include "driver_opengl_shader_cache.h"

namespace NL3D
{
	CShaderCache::CShaderCache()
	{
	}

	CShaderCache::~CShaderCache()
	{
		clearCache();
	}

	SShaderPair CShaderCache::findShader(const CShaderDesc &desc) const
	{
		for (int i = 0; i < shaders.size(); i++)
		{
			if (shaders[ i ] == desc)
				return shaders[ i ].getShaders();
		}

		return SShaderPair();
	}

	void CShaderCache::cacheShader(CShaderDesc &desc)
	{
		shaders.push_back(desc);
	}

	void CShaderCache::clearCache()
	{
		std::vector< CShaderDesc >::iterator itr = shaders.begin();
		while (itr != shaders.end())
		{
			SShaderPair sp;
			sp = itr->getShaders();
			delete sp.vp;
			delete sp.pp;
			++itr;
		}

		shaders.clear();
	}
}

#endif
