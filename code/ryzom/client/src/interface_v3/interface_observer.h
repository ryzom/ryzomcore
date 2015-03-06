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



#ifndef NL_INTERFACE_OBSERVER_H
#define NL_INTERFACE_OBSERVER_H

#include "nel/misc/types_nl.h"
#include "interface_manager.h"
#include "nel/misc/xml_auto_ptr.h"


/**
 * interface to define a simple observer
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class IInterfaceObserver : public NLMISC::ICDBNode::IPropertyObserver
{
public:

	/**
	 * parse the element and initalize it
	 * \param cur : pointer to the node describing this element
	 * \param ctrl : pointer to the parent group
	 * \return true if success
	 */
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)=0;

	/**
	 * observer update
	 */
	virtual void update (NLMISC::CCDBNodeLeaf* leaf)=0;


};


class IInterfaceObserverFactory;

/**
 * manager for observer factory
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class CInterfaceObserverFactoryManager
{
public:
	typedef std::map< std::string, IInterfaceObserverFactory* > TFactoryMap;
	static CInterfaceObserverFactoryManager* getInstance()
	{
		if (_GlobalInstance == NULL)
			_GlobalInstance = new CInterfaceObserverFactoryManager;
		return _GlobalInstance;
	}

	/// map of action handler factories
	TFactoryMap		FactoryMap;

private:
	CInterfaceObserverFactoryManager(){}
	static CInterfaceObserverFactoryManager	*_GlobalInstance;

};


/**
 * interface for observer factory
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class IInterfaceObserverFactory
{
public:


	/**
	* build the desired observer from script
	*\param idHandler : the string identifying the handler
	*\return a pointer to the factory instance to be used
	*/
	static IInterfaceObserver *create(xmlNodePtr node,CInterfaceGroup* parentGroup)
	{
		CInterfaceManager* iMngr = CInterfaceManager::getInstance();
		CXMLAutoPtr idObserver((const char*)xmlGetProp(node,(char unsigned*)"action"));
		if (!idObserver)
		{
			nlinfo("action not found in a observer node");
			return NULL;
		}
		CXMLAutoPtr ptr((const char*) xmlGetProp (node, (xmlChar*)"data"));
		if (!ptr)
		{
			nlinfo ("no data in a observer tag");
			return NULL;

		}


		IInterfaceObserver* obs =NULL;

		char * end = ptr.getDatas() + strlen( ptr.getDatas() );
		char * dataTok = strtok( ptr.getDatas()," ,");
		NLMISC::ICDBNode::CTextId textId;

		while(dataTok)
		{
			std::string data (dataTok);
			if (NLGUI::CDBManager::getInstance()->getDbProp(data,false) == NULL) // Full path provided ? No.
			{
				CInterfaceGroup *parent = parentGroup;
				while (parent != NULL)
				{
					std::string sTmp;
					if (data[0] == ':')
						sTmp = parent->getId() + data;
					else
						sTmp = parent->getId() + ":" + data;
					if (NLGUI::CDBManager::getInstance()->getDbProp(sTmp,false) != NULL)
					{
						data = sTmp;
						break;
					}
					parent = parent->getParent();
				}
				if (NLGUI::CDBManager::getInstance()->getDbProp(data,false) == NULL)
				{
					std::string sTmp = std::string("data (")+std::string(dataTok)+std::string(") in a observer tag do not exist, CREATING!");
					nlinfo (sTmp.c_str());
					data = (const char*)dataTok;
					NLGUI::CDBManager::getInstance()->getDbProp (data, true);
				}
			}
			if ( !obs )
			{
				CInterfaceObserverFactoryManager::TFactoryMap::const_iterator it = CInterfaceObserverFactoryManager::getInstance()->FactoryMap.find((const char *) idObserver);
				if (it== CInterfaceObserverFactoryManager::getInstance()->FactoryMap.end())
				{
					nlinfo("undefined observer : %s", (const char*)idObserver);
					return NULL;
				}
				obs = it->second->createObserver(node,parentGroup);
				if (!obs)
				{
					return NULL;
				}
			}
			textId = NLMISC::ICDBNode::CTextId( data );
			if ( ! NLGUI::CDBManager::getInstance()->getDB()->addObserver(obs,textId ) )
			{
				return NULL;
			}
			dataTok+= strlen(dataTok ) +1;
			if (dataTok >= end)
				break;
			dataTok = strtok( dataTok," ,");
		}


		return obs;
	}

	/**
	* Create an action handler
	*\param node : a pointer to the XML node containing the handlers parameters
	*\return a pointer to the action handler created
	*/
	virtual IInterfaceObserver *createObserver(xmlNodePtr node,CInterfaceGroup* parent)=0;

};

/**
 * register a new observer by defining the associated factory and insert the handler name in the global action handler map
 * \param handler : name of the action handler class
 * \param name : name of the action handler in the map (same as the "onaction" value in the XML script)
 */

#define REGISTER_OBSERVER(observer ,name) \
class observer##Factory : public IInterfaceObserverFactory\
{\
public:\
	observer##Factory ()\
	{\
	CInterfaceObserverFactoryManager::getInstance()->FactoryMap.insert(make_pair(std::string(name),this));\
	};\
protected:\
	IInterfaceObserver *createObserver(xmlNodePtr node,CInterfaceGroup* parent)\
	{\
		observer * buf = new observer; \
		buf->parse(node,parent);\
		return buf;\
	}\
}; \
observer##Factory observer##FactoryInstance ;\



#endif // NL_INTERFACE_OBSERVER_H

/* End of interface_observer.h */

