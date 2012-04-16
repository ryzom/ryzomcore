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

#include "dbview_quantity.h"
#include "interface_manager.h"
#include "nel/misc/xml_auto_ptr.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


NLMISC_REGISTER_OBJECT(CViewBase, CDBViewQuantity, std::string, "text_quantity");

// ***************************************************************************
CDBViewQuantity::CDBViewQuantity(const TCtorParam &param)
	: CViewText(param)
{
}



// ***************************************************************************
bool CDBViewQuantity::parse (xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	if(!CViewText::parse(cur, parentGroup))
		return false;

	// link to the db
	CXMLAutoPtr ptr;
	ptr = xmlGetProp (cur, (xmlChar*)"value");
	if ( ptr )
		_Number.link ( ptr );
	else
	{
		nlinfo ("no value in %s", _Id.c_str());
		return false;
	}
	ptr = xmlGetProp (cur, (xmlChar*)"valuemax");
	if ( ptr )
		_NumberMax.link ( ptr );
	else
	{
		nlinfo ("no max value in %s", _Id.c_str());
		return false;
	}

	// empty opt
	ptr = xmlGetProp (cur, (xmlChar*)"emptytext");
	if(ptr)
	{
		const char *propPtr = ptr;
		_EmptyText = ucstring(propPtr);
		if ((strlen(propPtr)>2) && (propPtr[0] == 'u') && (propPtr[1] == 'i'))
			_EmptyText = CI18N::get (propPtr);
	}

	// init cache.
	_Cache= 0;
	_CacheMax= 0;
	buildTextFromCache();

	return true;
}

// ***************************************************************************
void CDBViewQuantity::draw ()
{
	// change text
	sint32	val= _Number.getSInt32();
	sint32	valMax= _NumberMax.getSInt32();
	if(_Cache!=val || _CacheMax!=valMax)
	{
		_Cache= val;
		_CacheMax=valMax;
		buildTextFromCache();
	}

	// parent call
	CViewText::draw();
}

// ***************************************************************************
void	CDBViewQuantity::buildTextFromCache()
{
	if(_Cache==0 && !_EmptyText.empty())
	{
		setText(_EmptyText);
	}
	else
	{
		char	buf[256];
		smprintf(buf, 256, "%d/%d", _Cache, _CacheMax);
		setText(toString((const char*)buf));
	}
}
