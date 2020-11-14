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
#include "nel/misc/xml_auto_ptr.h"

#include <libxml/parser.h>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

//=======================================
void CXMLAutoPtr::destroy()
{
	if (_Value)
	{
		xmlFree(const_cast<char *>(_Value));
		_Value = NULL;
	}
}

//=======================================
CXMLAutoPtr::~CXMLAutoPtr()
{
	destroy();
}

//=======================================
CXMLAutoPtr &CXMLAutoPtr::operator = (const char *other)
{
	if (other == _Value) return *this;
	destroy();
	_Value = other;
	return *this;
}
