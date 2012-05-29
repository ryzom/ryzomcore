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



#ifndef PRIMITIVE_CFG_H
#define PRIMITIVE_CFG_H

#include "nel/misc/types_nl.h"

class CPrimitiveCfg
{
//	static std::vector<std::string>			_AllPrimitives;
	static std::vector<std::string>			_MapNames;
	static std::map<std::string, std::vector<std::string> >	_Maps;
	static std::map<std::string, std::vector<std::string> >	_ContinentFiles;
public:

	static	void readPrimitiveCfg(bool forceReload = false);

	static const std::vector<std::string> &getMapNames()
	{
		return _MapNames;
	}

	static const std::vector<std::string> &getMap(const std::string mapName)
	{
		static std::vector<std::string> emptyVector;
		std::map<std::string, std::vector<std::string> >::iterator it(_Maps.find(mapName));

		if (it == _Maps.end())
			return emptyVector;
		else
			return it->second;
	}

	static	std::string getContinentNameOf(const std::string &fileName);

	static	void addPrimitive(std::vector<std::string>	&list, const	std::string &str);

//	static const std::vector<std::string> &getAllPrimitives()
//	{
//		return _AllPrimitives;
//	}

};


#endif //PRIMITIVE_CFG_H
