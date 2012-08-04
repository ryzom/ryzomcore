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

#include "nel/gui/view_text_formated.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/i18n.h"

NLMISC_REGISTER_OBJECT(CViewBase, CViewTextFormated, std::string, "text_formated");

namespace NLGUI
{

	CViewTextFormated::IViewTextFormatter *CViewTextFormated::textFormatter = NULL;

	std::string CViewTextFormated::getProperty( const std::string &name ) const
	{
		if( name == "format" )
		{
			return getFormatString().toString();
		}
		else
			return CViewText::getProperty( name );
	}

	void CViewTextFormated::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "format" )
		{
			setFormatString( value );
			return;
		}
		else
			CViewText::setProperty( name, value );
	}

	// ****************************************************************************
	bool CViewTextFormated::parse(xmlNodePtr cur,CInterfaceGroup * parentGroup)
	{
		if (!CViewText::parse(cur, parentGroup)) return false;
		CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"format" ));
		if (prop)
			setFormatString(ucstring((const char *) prop));
		else
			setFormatString(ucstring("$t"));
		return true;
	}

	// ****************************************************************************
	void CViewTextFormated::checkCoords()
	{
		if (!getActive()) return;
		ucstring formatedResult;
		formatedResult = formatString(_FormatString, ucstring(""));

		//
		setText (formatedResult);
		CViewText::checkCoords();
	}

	// ****************************************************************************
	void CViewTextFormated::setFormatString(const ucstring &format)
	{
		_FormatString = format;
		if ( (_FormatString.size()>2) && (_FormatString[0] == 'u') && (_FormatString[1] == 'i'))
			_FormatString = NLMISC::CI18N::get (format.toString());
	}

	// ****************************************************************************
	ucstring CViewTextFormated::formatString(const ucstring &inputString, const ucstring &paramString)
	{
		ucstring formatedResult;

		if( textFormatter == NULL )
			formatedResult = inputString;
		else
			formatedResult = CViewTextFormated::textFormatter->formatString( inputString, paramString );

		return formatedResult;
	}

}

