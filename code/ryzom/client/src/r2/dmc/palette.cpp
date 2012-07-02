// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"

#include "palette.h"
#include "game_share/object.h"

#include <assert.h>

using namespace R2;

namespace R2
{

CObject* CPalette::getPaletteElement(const std::string& key) const
{
	//H_AUTO(R2_CPalette_getPaletteElement)
	TMap::const_iterator find(_Map.find(key));
	if (find != _Map.end())
	{
		return find->second;
	}
	return 0;
}

void CPalette::addPaletteElement(const std::string& key, CObject* paletteElement)
{
	//H_AUTO(R2_CPalette_addPaletteElement)
	std::pair< TMap::iterator, bool> result;
	result = _Map.insert( std::pair<std::string, CObject*>(key, paletteElement));


	if (!result.second)
	{
		nlwarning("Palette element added twice : %s", key.c_str());
		delete paletteElement;
	}
}

bool CPalette::isInPalette(const std::string &key) const
{
	//H_AUTO(R2_CPalette_isInPalette)
	TMap::const_iterator found(_Map.find(key));
	if (found != _Map.end())
		return true;
	return false;
}

CPalette::~CPalette()
{
	TMap::iterator first(_Map.begin()), last(_Map.end());
	for (; first != last; ++first)
	{
		delete(first->second);
	}
	_Map.clear();
}

} // R2

