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

#include "stdpch.h"
#include "scheme_manager.h"
#include "nel/3d/ps_attrib_maker.h"

namespace NLQT
{

CSchemeManager::~CSchemeManager()
{
	for(TSchemeMap::iterator it = _SchemeMap.begin(); it != _SchemeMap.end(); ++it)
	{
		delete it->second.second;
	}
}


void CSchemeManager::insertScheme(const std::string &name, NL3D::CPSAttribMakerBase *scheme)
{
	nlassert(scheme);
	TSchemeInfo si(std::string(name), scheme);
	_SchemeMap.insert(TSchemeMap::value_type(std::string(scheme->getType()), si));
}

void CSchemeManager::getSchemes(const std::string &type, std::vector<TSchemeInfo> &dest)
{
	TSchemeMap::const_iterator lbd = _SchemeMap.lower_bound(type), ubd = _SchemeMap.upper_bound(type);
	dest.clear();
	for (TSchemeMap::const_iterator it = lbd; it != ubd; ++it)
	{
		dest.push_back(it->second);
	}
}

void CSchemeManager::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{

	f.serialCheck(NELID("_GNM"));
	f.serialCheck(NELID("MHCS"));
	f.serialVersion(1);
	if (!f.isReading())
	{
		sint32 size = (sint32)_SchemeMap.size();
		f.serial(size);
		for (TSchemeMap::iterator smIt = _SchemeMap.begin(); smIt != _SchemeMap.end(); ++smIt)
		{
			f.serial(smIt->second.first);		  // name
			f.serialPolyPtr(smIt->second.second); // scheme
		}
	}
	else
	{
		_SchemeMap.clear();

		std::string name;
		NL3D::CPSAttribMakerBase *scheme = NULL;
		sint32 size;
		f.serial(size);
		for (sint32 k = 0; k < size; ++k)
		{
			f.serial(name);
			f.serialPolyPtr(scheme);
			insertScheme(name, scheme);
		}
	}
}

void CSchemeManager::swap(CSchemeManager &other)
{
	this->_SchemeMap.swap(other._SchemeMap);
}

void CSchemeManager::remove(NL3D::CPSAttribMakerBase *am)
{
	TSchemeMap::iterator smIt;
	for (smIt = _SchemeMap.begin(); smIt != _SchemeMap.end(); ++smIt)
	{
		if (smIt->second.second == am) break;
	}
	if (smIt != _SchemeMap.end())
	{
		delete smIt->second.second; // delete the scheme
		// delet from the collection
		_SchemeMap.erase(smIt);
	}
}

// rename a scheme, given a pointer on it
void CSchemeManager::rename(NL3D::CPSAttribMakerBase *am, const std::string &newName)
{
	TSchemeMap::iterator smIt;
	for (smIt = _SchemeMap.begin(); smIt != _SchemeMap.end(); ++smIt)
	{
		if (smIt->second.second == am) break;
	}
	if (smIt != _SchemeMap.end())
	{
		smIt->second.first = newName;
	}
}

}