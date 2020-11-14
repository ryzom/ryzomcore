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

#include "stdmisc.h"

#include "nel/misc/class_registry.h"
#include "nel/misc/debug.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{


// ======================================================================================================
CClassRegistry::TClassMap		*CClassRegistry::RegistredClasses = NULL;


// ======================================================================================================
void		CClassRegistry::init()
{
	if (RegistredClasses == NULL)
		RegistredClasses = new TClassMap;
}

// ======================================================================================================
void		CClassRegistry::release()
{
	if( RegistredClasses )
		delete RegistredClasses;
	RegistredClasses = NULL;
}

// ======================================================================================================
IClassable	*CClassRegistry::create(const string &className)
{
	init();

	TClassMap::iterator	it;

	it=RegistredClasses->find(className);

	if(it==RegistredClasses->end())
		return NULL;
	else
	{
		IClassable	*ptr;
		ptr=it->second.Creator();
		#ifdef NL_DEBUG
			nlassert(CClassRegistry::checkObject(ptr));
		#endif
		return ptr;
	}

}

// ======================================================================================================
void		CClassRegistry::registerClass(const string &className, IClassable* (*creator)(), const string &typeidCheck)
{
	init();

	CClassNode	node;
	node.Creator=creator;
	node.TypeIdCheck= typeidCheck;
	std::pair<TClassMap::iterator, bool> result;
	result = RegistredClasses->insert(TClassMap::value_type(className, node));
	if(!result.second)
	{
		nlstop;
		throw ERegisteredClass();
	}
}

// ======================================================================================================
bool		CClassRegistry::checkObject(IClassable* obj)
{
	init();

	TClassMap::iterator	it;
	it=RegistredClasses->find(obj->getClassName());
	if(it==RegistredClasses->end())
		return false;

	if( it->second.TypeIdCheck != string(typeid(*obj).name()) )
		return false;

	return true;
}



}






















