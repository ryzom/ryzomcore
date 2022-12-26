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


#include "nel/misc/types_nl.h"

#include <string>
#include <vector>
#include <map>


class CxmlNode
{
public:
	bool read(const std::string &filename);
	bool parseInput(char * &ptr);

	inline const std::string &txt() const		{ return _txt;  }
	inline const std::string &type() const		{ return _name; }
	inline const std::string &arg(const std::string &argName) const
	{
		static std::string emptyString;

		std::map<std::string,std::string>::const_iterator it;
		it=_args.find(argName);

		if (it!=_args.end())
			return (*it).second;
		else
			return emptyString;
	}

	inline uint childCount() const				{ return _children.size(); }
	inline CxmlNode *child(uint index) const	{ return _children[index]; }

private:
	std::string _txt;
	std::string _name;
	std::map<std::string,std::string> _args;
	std::vector<CxmlNode *> _children;
};

