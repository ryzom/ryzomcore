// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/view_text_formated.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/i18n.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

NLMISC_REGISTER_OBJECT(CViewBase, CViewTextFormated, std::string, "text_formated");

namespace NLGUI
{

	CViewTextFormated::IViewTextFormatter *CViewTextFormated::textFormatter = NULL;

	std::string CViewTextFormated::getProperty(const std::string &name) const
    {
		if (name == "format")
		{
			return getFormatString();
		}
		else
			return CViewText::getProperty(name);
    }

    void CViewTextFormated::setProperty(const std::string &name, const std::string &value)
    {
	    if (name == "format")
	    {
		    setFormatString(value);
		    return;
	    }
	    else
		    CViewText::setProperty(name, value);
    }

	xmlNodePtr CViewTextFormated::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewText::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "text_formated" );
		xmlSetProp( node, BAD_CAST "format", BAD_CAST getFormatString().c_str() );

		return NULL;
	}

	// ****************************************************************************
	bool CViewTextFormated::parse(xmlNodePtr cur,CInterfaceGroup * parentGroup)
	{
		if (!CViewText::parse(cur, parentGroup)) return false;
		CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"format" ));
		if (prop)
			setFormatString((const char *)prop);
		else
			setFormatString("$t");
		return true;
	}

	// ****************************************************************************
	void CViewTextFormated::checkCoords()
	{
		if (!getActive()) return;
		std::string formatedResult;
		formatedResult = formatString(_FormatString, std::string());

		//
		setText (formatedResult);
		CViewText::checkCoords();
	}

	// ****************************************************************************
	void CViewTextFormated::setFormatString(const std::string &format)
	{
		if (NLMISC::startsWith(format, "ui"))
			_FormatString = NLMISC::CI18N::get(format);
		else
			_FormatString = format;
	}

	// ****************************************************************************
	std::string CViewTextFormated::formatString(const std::string &inputString, const std::string &paramString)
	{
		std::string formatedResult;

		if( textFormatter == NULL )
			formatedResult = inputString;
		else
			formatedResult = CViewTextFormated::textFormatter->formatString( inputString, paramString );

		return formatedResult;
	}

}

