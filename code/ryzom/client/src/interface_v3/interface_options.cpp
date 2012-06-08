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



#include "nel/misc/xml_auto_ptr.h"
#include "interface_options.h"
#include "interface_element.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
const CInterfaceOptionValue	CInterfaceOptionValue::NullValue;

// ***************************************************************************
void		CInterfaceOptionValue::init(const std::string &str)
{
	_Str= str;
	fromString(str, _Int);
	fromString(str, _Float);
	_Color= CInterfaceElement::convertColor (str.c_str());
	_Boolean= CInterfaceElement::convertBool(str.c_str());
}


// ----------------------------------------------------------------------------
// CInterfaceOptions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
CInterfaceOptions::CInterfaceOptions()
{
}

// ----------------------------------------------------------------------------
CInterfaceOptions::~CInterfaceOptions()
{
}

// ----------------------------------------------------------------------------
bool CInterfaceOptions::parse (xmlNodePtr cur)
{
	cur = cur->children;
	bool ok = true;
	while (cur)
	{
		if ( !stricmp((char*)cur->name,"param") )
		{
			CXMLAutoPtr ptr, val;
			ptr = xmlGetProp (cur, (xmlChar*)"name");
			val = xmlGetProp (cur, (xmlChar*)"value");
			if (!ptr || !val)
			{
				nlinfo("param with no name or no value");
				ok = false;
			}
			else
			{
				string name = NLMISC::toLower(string((const char*)ptr));
				string value = (string((const char*)val));
				_ParamValue[name].init(value);
			}
		}
		cur = cur->next;
	}
	return ok;
}

// ***************************************************************************
void	CInterfaceOptions::copyBasicMap(const CInterfaceOptions &other)
{
	_ParamValue= other._ParamValue;
}

// ***************************************************************************
const CInterfaceOptionValue	 &CInterfaceOptions::getValue(const string &sParamName) const
{
	string sLwrParamName = strlwr (sParamName);
	std::map<std::string, CInterfaceOptionValue>::const_iterator it = _ParamValue.find (sLwrParamName);
	if (it != _ParamValue.end())
		return it->second;
	else
		return CInterfaceOptionValue::NullValue;
}

// ***************************************************************************
const std::string		&CInterfaceOptions::getValStr(const std::string &sParamName) const
{
	return getValue(sParamName).getValStr();
}
// ***************************************************************************
sint32					CInterfaceOptions::getValSInt32(const std::string &sParamName) const
{
	return getValue(sParamName).getValSInt32();
}
// ***************************************************************************
float					CInterfaceOptions::getValFloat(const std::string &sParamName) const
{
	return getValue(sParamName).getValFloat();
}
// ***************************************************************************
NLMISC::CRGBA			CInterfaceOptions::getValColor(const std::string &sParamName) const
{
	return getValue(sParamName).getValColor();
}
// ***************************************************************************
bool					CInterfaceOptions::getValBool(const std::string &sParamName) const
{
	return getValue(sParamName).getValBool();
}

