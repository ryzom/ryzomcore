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

#ifndef SCHEME_MANAGER_H
#define SCHEME_MANAGER_H

#include <nel/misc/stream.h>

#include <string>
#include <map>
#include <algorithm>

namespace NL3D
{
	class CPSAttribMakerBase;
}

namespace NLQT
{

class CSchemeManager
{
public:
	/// dtor
	~CSchemeManager();

	typedef std::pair<std::string, NL3D::CPSAttribMakerBase *> TSchemeInfo;
	// insert a new scheme in the collection. The scheme is then owned by this object
	void insertScheme(const std::string &name, NL3D::CPSAttribMakerBase *scheme);
	// get all the schemes with the given type
	void getSchemes(const std::string &type, std::vector<TSchemeInfo> &dest);
	// serial this collection
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	// swap this collection with another one
	void	swap(CSchemeManager &other);
	// remove a scheme from the bank, given a pointer on it
	void    remove(NL3D::CPSAttribMakerBase *am);
	// rename a scheme, given a pointer on it
	void    rename(NL3D::CPSAttribMakerBase *am, const std::string &newName);
protected:	
	// typedef std::pair<std::string, NL3D::CPSAttribMakerBase *> TSchemeInfo;
	typedef std::multimap<std::string, TSchemeInfo> TSchemeMap;
	TSchemeMap		_SchemeMap;	
};

} /* namespace NLQT */

#endif
