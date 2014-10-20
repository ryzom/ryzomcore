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
#include "nel/gui/dbview_number.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/common.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


NLMISC_REGISTER_OBJECT(CViewBase, CDBViewNumber, std::string, "text_number");

// ***************************************************************************

namespace NLGUI
{

	CDBViewNumber::CDBViewNumber(const TCtorParam &param)
		:CViewText(param)
	{
		_Positive = false;
		_Cache= 0;
		setText(ucstring("0"));
		_Divisor = 1;
		_Modulo = 0;
	}

	std::string CDBViewNumber::getProperty( const std::string &name ) const
	{
		if( name == "value" )
		{
			if( _Number.getNodePtr() != NULL )
				return _Number.getNodePtr()->getFullName();
			else
				return "";
		}
		else
		if( name == "positive" )
		{
			return toString( _Positive );
		}
		else
		if( name == "format" )
		{
			return toString( _Format );
		}
		else
		if( name == "divisor" )
		{
			return toString( _Divisor );
		}
		else
		if( name == "modulo" )
		{
			return toString( _Modulo );
		}
		else
		if( name == "suffix" )
		{
			return _Suffix.toString();
		}
		else
		if( name == "prefix" )
		{
			return _Prefix.toString();
		}
		else
			return CViewText::getProperty( name );
	}

	void CDBViewNumber::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "value" )
		{
			_Number.link( value.c_str() );
			return;
		}
		else
		if( name == "positive" )
		{
			bool b;
			if( fromString( value, b ) )
				_Positive = b;
			return;
		}
		else
		if( name == "format" )
		{
			bool b;
			if( fromString( value, b ) )
				_Format = b;
			return;
		}
		else
		if( name == "divisor" )
		{
			sint64 i;
			if( fromString( value, i ) )
				_Divisor = i;
			return;
		}
		else
		if( name == "modulo" )
		{
			sint64 i;
			if( fromString( value, i ) )
				_Divisor = i;
			return;
		}
		else
		if( name == "suffix" )
		{
			_Suffix = value;
			return;
		}
		else
		if( name == "prefix" )
		{
			_Prefix = value;
			return;
		}
		else
			CViewText::setProperty( name, value );
	}

	xmlNodePtr CDBViewNumber::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewText::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "text_number" );
		
		if( _Number.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "value", BAD_CAST _Number.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "value", BAD_CAST "" );

		xmlSetProp( node, BAD_CAST "positive", BAD_CAST toString( _Positive ).c_str() );
		xmlSetProp( node, BAD_CAST "format", BAD_CAST toString( _Format ).c_str() );
		xmlSetProp( node, BAD_CAST "divisor", BAD_CAST toString( _Divisor ).c_str() );
		xmlSetProp( node, BAD_CAST "modulo", BAD_CAST toString( _Modulo ).c_str() );
		xmlSetProp( node, BAD_CAST "suffix", BAD_CAST _Suffix.toString().c_str() );
		xmlSetProp( node, BAD_CAST "prefix", BAD_CAST _Prefix.toString().c_str() );

		return node;
	}

	// ***************************************************************************
	bool CDBViewNumber::parse (xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if(!CViewText::parse(cur, parentGroup))
			return false;

		// link to the db
		CXMLAutoPtr ptr;
		ptr = xmlGetProp (cur, (xmlChar*)"value");
		if ( ptr )
		{
			// Yoyo: verify doesn't entered a direct number :). MUST BE A CORRECT DATABASE ENTRY
			const	char	*serverDb= "SERVER:";
			const	char	*localDb= "LOCAL:";
			const	char	*uiDb= "UI:";
			if( strncmp((const char*)ptr, serverDb, strlen(serverDb))==0 ||
				strncmp((const char*)ptr, localDb, strlen(localDb))==0 ||
				strncmp((const char*)ptr, uiDb, strlen(uiDb))==0 )
			{
				// OK? => Link.
				_Number.link ( ptr );
			}
			else
			{
				nlinfo ("bad value in %s", _Id.c_str());
				return false;
			}
		}
		else
		{
			nlinfo ("no value in %s", _Id.c_str());
			return false;
		}

		ptr = xmlGetProp (cur, (xmlChar*)"positive");
		if (ptr) _Positive = convertBool(ptr);
		else _Positive = false;

		ptr = xmlGetProp (cur, (xmlChar*)"format");
		if (ptr) _Format = convertBool(ptr);
		else _Format = false;

		ptr = xmlGetProp (cur, (xmlChar*)"divisor");
		if (ptr) fromString((const char*)ptr, _Divisor);

		ptr = xmlGetProp (cur, (xmlChar*)"modulo");
		if (ptr) fromString((const char*)ptr, _Modulo);

		ptr = xmlGetProp (cur, (xmlChar*)"suffix");
		if (ptr) _Suffix = (const char*)ptr;

		ptr = xmlGetProp (cur, (xmlChar*)"prefix");
		if (ptr) _Prefix = (const char*)ptr;

		// init cache.
		_Cache= 0;
		setText(ucstring("0"));

		return true;
	}

	// ***************************************************************************
	void CDBViewNumber::checkCoords()
	{
		// change text
		sint64	val = getVal();
		if (_Cache != val)
		{
			_Cache= val;
			ucstring value = _Format ? NLMISC::formatThousands(toString(val)) : toString(val);
			if (_Positive) setText(val >= 0 ? ( ucstring(_Prefix) + value + ucstring(_Suffix) ) : ucstring("?"));
			else setText( ucstring(_Prefix) + value + ucstring(_Suffix) );
		}
	}

	// ***************************************************************************
	void CDBViewNumber::draw ()
	{
		// parent call
		CViewText::draw();
	}

	void CDBViewNumber::forceLink()
	{
	}

	sint64 CDBViewNumber::getVal()
	{
		if( !_Number.hasValue() )
			return 0;

		if( _Modulo == 0 )
			return _Number.getSInt64() / _Divisor;
		else
			return ( _Number.getSInt64() / _Divisor ) % _Modulo;
	}

}

