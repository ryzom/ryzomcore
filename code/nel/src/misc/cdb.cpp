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

#include "stdmisc.h"

//////////////
// Includes //
//////////////
#include "nel/misc/cdb.h"
#include "nel/misc/cdb_branch.h"
#include "nel/misc/bit_mem_stream.h"

////////////////
// Namespaces //
////////////////
using namespace std;

namespace NLMISC{

CStringMapper *ICDBNode::_DBSM = NULL;
bool ICDBNode::verboseDatabase = false;


std::string ICDBNode::getFullName()
{
	CSString sTmp;
	sTmp.reserve(256);
	_buildFullName(sTmp);
	return sTmp;
//	CCDBNodeBranch *pParent = getParent();
//	if (pParent == NULL)
//		return _DBSM->localUnmap(_Name);
//	CSString sTmp;
//	_buildFullName(sTmp);
//	sTmp << ":" << sTmp;
//	return sTmp;

//	// climb the parent hierarchie
//	while (pParent->getParent() != NULL)
//		pParent = pParent->getParent();
//
//	// now build the full name in desc order
//	while (p)
//
//	while (pParent->getParent() != NULL)
//	{
//		sTmp = *pParent->getName()) << ":" << sTmp;
//		pParent = pParent->getParent();
//	}
//	return sTmp;
}

void ICDBNode::_buildFullName(CSString &fullName)
{
	// we do not want to recurse up to the ROOT node - we stop 1 level down from the root
	if (getParent() != NULL && getParent()->getParent() != NULL)
	{
		getParent()->_buildFullName(fullName);
		fullName << ":" << _DBSM->localUnmap(_Name);
	}
	else
		fullName = _DBSM->localUnmap(_Name);
}

void ICDBNode::releaseStringMapper()
{
	if( _DBSM )
		delete _DBSM;
	_DBSM = NULL;
}

}

