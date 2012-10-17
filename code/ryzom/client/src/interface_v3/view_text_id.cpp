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

#include "interface_manager.h"
#include "../string_manager_client.h"
#include "view_text_id.h"
#include "game_share/xml_auto_ptr.h"
#include "../client_cfg.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace STRING_MANAGER;

NLMISC_REGISTER_OBJECT(CViewBase, CViewTextID, std::string, "text_id");

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
		ucstring result;
		CStringManagerClient *pSMC = CStringManagerClient::instance();

		// Get the string
		if (_DynamicString)
			_Initialized = pSMC->getDynString (_TextId, result);
		else
			_Initialized = pSMC->getString (_TextId, result);

		// Remove all {break}
		for(;;)
		{
			ucstring::size_type index = result.find (ucstring("{break}"));
			if (index == ucstring::npos) break;
			result = result.substr (0, index) + result.substr(index+7, result.size());
		}


		// Remove all {ros_exit}
		while(NLMISC::strFindReplace(result,   "{ros_exit}",   ""));

		// Modify the text?
		if(_StringModifier)
			_StringModifier->onReceiveTextId(result);

		// Set the Text
		if(_IsTextFormatTaged)
			setTextFormatTaged(result);
		else
			setText (result);
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
	CCDBNodeLeaf *pNL = CInterfaceManager::getInstance()->getDbProp(link,false);
	if (pNL == NULL)
	{
		nlwarning("cant set textidlink for %s",link.c_str());
		return;
	}
	setDBLeaf(pNL);
}

