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
#include "nel/gui/db_manager.h"
#include "nel/gui/view_text_id.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/algo.h"

using namespace std;
using NLMISC::CCDBNodeLeaf;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

NLMISC_REGISTER_OBJECT(CViewBase, CViewTextID, std::string, "text_id");

namespace NLGUI
{

	CViewTextID::IViewTextProvider* CViewTextID::textProvider = NULL;

	// ***************************************************************************
	CViewTextID::CViewTextID(const std::string& id,const std::string &/* idDBPath */, sint FontSize /*=12*/,NLMISC::CRGBA Color /*=NLMISC::CRGBA(255,255,255)*/,bool Shadow /*=false*/)
				: CViewText (id, std::string(""), FontSize, Color, Shadow)
	{
		_StringModifier= NULL;
		_IsDBLink = true;
		_DBTextId.link(id.c_str());
		_Initialized = false;
		_DynamicString = true;
		_IsTextFormatTaged= false;
	}

	// ***************************************************************************
	CViewTextID::~CViewTextID()
	{
	}

	std::string CViewTextID::getProperty( const std::string &name ) const
	{
		if( name == "textid" )
		{
			if( _DBTextId.getNodePtr() != NULL )
				return _DBTextId.getNodePtr()->getFullName();
			else
				return NLMISC::toString( _TextId );
		}
		else
		if( name == "dynamic_string" )
		{
			return NLMISC::toString( _DynamicString );
		}
		else
		if( name == "format_taged" )
		{
			return NLMISC::toString( _IsTextFormatTaged );
		}
		else
			return CViewText::getProperty( name );
	}

	void CViewTextID::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "textid" )
		{
			uint32 i;
			if( NLMISC::fromString( value, i ) )
			{
				_TextId = i;
				_IsDBLink = false;
			}
			else
			{
				_DBTextId.link( value.c_str() );
				_IsDBLink = true;
			}
			return;
		}
		else
		if( name == "dynamic_string" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_DynamicString = b;
			return;
		}
		else
		if( name == "format_taged" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				_IsTextFormatTaged = b;
			return;
		}
		else
			CViewText::setProperty( name, value );
	}


	xmlNodePtr CViewTextID::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CViewText::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "text_id" );

		if( _DBTextId.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "textid", BAD_CAST _DBTextId.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "textid", BAD_CAST "" );

		xmlSetProp( node, BAD_CAST "dynamic_string",
			BAD_CAST NLMISC::toString( _DynamicString ).c_str() );
		
		xmlSetProp( node, BAD_CAST "format_taged",
			BAD_CAST NLMISC::toString( _IsTextFormatTaged ).c_str() );

		return node;
	}

	// ***************************************************************************
	bool CViewTextID::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
	{
		if (!CViewText::parse(cur, parentGroup))
			return false;

		if(!parseTextIdOptions(cur))
			return false;

		return true;
	}

	// ***************************************************************************
	bool CViewTextID::parseTextIdOptions(xmlNodePtr cur)
	{
		_Initialized = false;

		CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"textid" ));
		_IsDBLink = false;
		_TextId = 0xFFFFFFFF;
		if (prop)
		{
			if ( isdigit(*prop.getDatas()) || *(prop.getDatas())=='-')
			{
				NLMISC::fromString((const char*)prop, _TextId);
			}
			else
			{
				if (_DBTextId.link(prop))
				{
					_TextId = (uint32)_DBTextId.getSInt64();
					_IsDBLink = true;
				}
				else
				{
					return false;
				}
			}
		}

		prop= (char*) xmlGetProp( cur, (xmlChar*)"dynamic_string" );
		_DynamicString = true;
		if (prop)
			_DynamicString = convertBool(prop);

		// format_taged
		prop= (char*) xmlGetProp( cur, (xmlChar*)"format_taged" );
		_IsTextFormatTaged= false;
		if(prop)
			_IsTextFormatTaged= convertBool(prop);

		return true;
	}

	// ***************************************************************************
	void CViewTextID::checkCoords()
	{
		if (_IsDBLink)
		{
			uint32 newId = (uint32)_DBTextId.getSInt64();
			setTextId (newId);
		}

		if (!_Initialized)
		{
			// String result
			string result;

			if( textProvider != NULL )
			{
				// Get the string
				if( _DynamicString )
					_Initialized = textProvider->getDynString( _TextId, result );
				else
					_Initialized = textProvider->getString( _TextId, result );
			}

			// Remove all {break}
			for(;;)
			{
				string::size_type index = result.find("{break}");
				if (index == string::npos) break;
				result = result.substr (0, index) + result.substr(index+7, result.size());
			}


			// Remove all {ros_exit}
			while(NLMISC::strFindReplace(result,   "{ros_exit}",   ""));

			// Modify the text?
			if (_StringModifier)
			{
				_StringModifier->onReceiveTextId(result);
			}

			// Set the Text
			if(_IsTextFormatTaged)
				setTextFormatTaged(result);
			else
				setText(result);
		}
		CViewText::checkCoords();
	}

	// ***************************************************************************
	uint32 CViewTextID::getTextId () const
	{
		return _TextId;
	}

	// ***************************************************************************
	void CViewTextID::setTextId (uint32 newId)
	{
		if (newId != _TextId)
			_Initialized = false;
		_TextId = newId;
		if (_IsDBLink)
			_DBTextId.setSInt64(_TextId);
	}

	// ***************************************************************************
	bool CViewTextID::setDBTextID(const std::string &dbPath)
	{
		if (_DBTextId.link(dbPath.c_str()))
		{
			_TextId = (uint32)_DBTextId.getSInt64();
			_IsDBLink = true;
			_Initialized = false;
			#ifdef NL_DEBUG
				_DBPath =dbPath;
			#endif
			return true;
		}
		else
		{
			_IsDBLink = false;
			_Initialized = false;
			#ifdef NL_DEBUG
				_DBPath="Bad db path : " + dbPath;
			#endif
			return false;
		}
	}

	// ***************************************************************************
	void CViewTextID::setDBLeaf(CCDBNodeLeaf *leaf)
	{
		if (!leaf)
		{
			_IsDBLink = false;
			_TextId = 0;
			return;
		}
		_IsDBLink = true;
		_DBTextId.setNodePtr(leaf);
	}

	// ***************************************************************************
	string CViewTextID::getTextIdDbLink() const
	{
		if (!_IsDBLink) return "";
		if (_DBTextId.getNodePtr() == NULL) return "";
		return _DBTextId.getNodePtr()->getFullName();
	}

	// ***************************************************************************
	void CViewTextID::setTextIdDbLink(const string &link)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(link,false);
		if (pNL == NULL)
		{
			nlwarning("cant set textidlink for %s",link.c_str());
			return;
		}
		setDBLeaf(pNL);
	}

}

