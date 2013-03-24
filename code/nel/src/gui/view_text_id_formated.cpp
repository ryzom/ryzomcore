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
#include "nel/gui/view_text_id_formated.h"
#include "nel/gui/view_text_formated.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/i18n.h"

NLMISC_REGISTER_OBJECT(CViewBase, CViewTextIDFormated, std::string, "text_id_formated");

namespace NLGUI
{

	std::string CViewTextIDFormated::getProperty( const std::string &name ) const
	{
		if( name == "format" )
		{
			return getFormatString().toString();
		}
		else
		return CViewTextID::getProperty( name );
	}


	void CViewTextIDFormated::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "format" )
		{
			setFormatString( value );
			return;
		}
		else
			CViewTextID::setProperty( name, value );
	}

	xmlNodePtr CViewTextIDFormated::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewTextID::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "text_id_formated" );
		xmlSetProp( node, BAD_CAST "format", BAD_CAST getFormatString().c_str() );

		return node;
	}

	// *********************************************************************************
	bool CViewTextIDFormated::parse(xmlNodePtr cur,CInterfaceGroup * parentGroup)
	{
		if (!CViewTextID::parse(cur, parentGroup)) return false;
		CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"format" ));
		if (prop)
			setFormatString(ucstring((const char *) prop));
		else
			setFormatString(ucstring("$t"));
		return true;
	}

	// *********************************************************************************
	void CViewTextIDFormated::checkCoords()
	{
		if (_IsDBLink)
		{
			uint32 newId = (uint32)_DBTextId.getSInt64();
			setTextId (newId);
		}

		if (!_Initialized)
		{
			ucstring result, formatedResult;
			bool bValid;

			if( CViewTextID::getTextProvider() == NULL )
			{
				if(!_DBPath.empty())
					result = ucstring(_DBPath);
				else
					result = ucstring("Text ID = " + NLMISC::toString(_TextId));
				bValid = true;
			}
			else
			{
				bValid = CViewTextID::getTextProvider()->getDynString (_TextId, result);
			}
			formatedResult = CViewTextFormated::formatString(_FormatString, result);
			//
			setText (formatedResult);
			//
			if (bValid)
			{
				_Initialized = true;
			}
		}
		CViewText::checkCoords();
	}

	// ****************************************************************************
	void CViewTextIDFormated::setFormatString(const ucstring &format)
	{
		_Initialized = false;
		_FormatString = format;
		if ( (_FormatString.size()>2) && (_FormatString[0] == 'u') && (_FormatString[1] == 'i'))
			_FormatString = NLMISC::CI18N::get (format.toString());
	}

}
