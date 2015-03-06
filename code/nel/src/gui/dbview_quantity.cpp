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
#include "nel/gui/dbview_quantity.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/i18n.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


NLMISC_REGISTER_OBJECT(CViewBase, CDBViewQuantity, std::string, "text_quantity");

namespace NLGUI
{

	// ***************************************************************************
	CDBViewQuantity::CDBViewQuantity(const TCtorParam &param)
		: CViewText(param)
	{
	}


	std::string CDBViewQuantity::getProperty( const std::string &name ) const
	{
		if( name == "value" )
		{
			if( _Number.getNodePtr() != NULL )
				return _Number.getNodePtr()->getFullName();
			else
				return "";
		}
		else
		if( name == "valuemax" )
		{
			if( _NumberMax.getNodePtr() != NULL )
				return _NumberMax.getNodePtr()->getFullName();
			else
				return "";
		}
		else
		if( name == "emptytext" )
		{
			return _EmptyText.toString();
		}
		else
			return CViewText::getProperty( name );
	}


	void CDBViewQuantity::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "value" )
		{
			_Number.link( value.c_str() );
			return;
		}
		else
		if( name == "valuemax" )
		{
			_NumberMax.link( value.c_str() );
			return;
		}
		else
		if( name == "emptytext" )
		{
			_EmptyText = value;
			return;
		}
		else
			CViewText::setProperty( name, value );
	}


	xmlNodePtr CDBViewQuantity::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewText::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "text_quantity" );

		if( _Number.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "value", BAD_CAST _Number.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "value", BAD_CAST "" );

		if( _NumberMax.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "valuemax", BAD_CAST _NumberMax.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "valuemax", BAD_CAST "" );

		xmlSetProp( node, BAD_CAST "emptytext", BAD_CAST _EmptyText.toString().c_str() );

		return node;
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
		if( _Number.hasValue() && _NumberMax.hasValue() )
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

	void CDBViewQuantity::forceLink()
	{
	}
}
