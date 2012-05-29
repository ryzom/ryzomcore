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



#ifndef NL_ACTION_HANDLER_H
#define NL_ACTION_HANDLER_H

#include "nel/misc/types_nl.h"
#include <libxml/parser.h>
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "game_share/xml_auto_ptr.h"
#include <map>

extern bool game_exit;
extern bool ryzom_exit;
extern bool game_exit_request;
extern bool ryzom_exit_request;
extern bool paying_account_request;
extern bool paying_account_already_request;
extern bool game_exit_after_paying_account_request;


/**
 * interface for action handlers
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */

class IInputControl;
class CCtrlBase;

class IActionHandler
{
public:
	// Execute the answer to the action
	// Params has the following form : paramName=theParam|paramName2=theParam2|...
	virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */) { }

	virtual ~IActionHandler() {}

	static std::string getParam (const std::string &Params, const std::string &ParamName);

	static void getAllParams (const std::string &Params, std::vector< std::pair<std::string,std::string> > &AllParams);
};


/**
 * interface for action handlers factory
 * no release in this factory : a handler must be destroyed by the control that created it
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */

class CActionHandlerFactoryManager
{
public:
	typedef std::map< std::string, IActionHandler* > TFactoryMap;
	typedef std::map< IActionHandler*, std::string > TNameMap;

	static CActionHandlerFactoryManager* getInstance()
	{
		if (_GlobalInstance == NULL)
			_GlobalInstance = new CActionHandlerFactoryManager;
		return _GlobalInstance;
	}

	/// return pointer to action handler or null if it doesn't exist
	IActionHandler *getActionHandler(const std::string &name) const
	{
		TFactoryMap::const_iterator it = FactoryMap.find(name);
		return it != FactoryMap.end() ? it->second : NULL;
	}

	/// Return the name of the action handler given its pointer
	const std::string &getActionHandlerName(IActionHandler *pAH) const
	{
		TNameMap::const_iterator it = NameMap.find(pAH);
		return it != NameMap.end() ? it->second : EmptyName;
	}

	/// map of action handler factories
	TFactoryMap		FactoryMap;
	TNameMap		NameMap;
	std::string		EmptyName;

private:
	CActionHandlerFactoryManager(){}
	static CActionHandlerFactoryManager	*_GlobalInstance;

};

/// return the Action Handler 'name'. if name is of form 'ah:params', then params are filled (NB: else not changed)
IActionHandler *getAH(const std::string &name, std::string &params);
IActionHandler *getAH(const std::string &name, class CStringShared &params);

/** common method to parse Action Handler from a xml node
 *	\param ahId eg: "onclick_l"
 *	\param paramId eg: "params_l".
 *	\param params returned parameters.
 *	NB: if paramId is NULL, empty or does not exist in the xmlNode, then the optional param in ahId (eg: "show:phrase_book")
 *	is taken
 *	NB: if none of the optional param in ahId, or the specified param are filled/found, then params is not changed
 */
void	parseAH(xmlNodePtr cur, const char *ahId, const char *paramId, IActionHandler *&ahRet, std::string &params);
void	parseAH(xmlNodePtr cur, const char *ahId, const char *paramId, IActionHandler *&ahRet, class CStringShared &params);

/// Get the AH name from ptr
inline const std::string &getAHName(IActionHandler *pAH) { return CActionHandlerFactoryManager::getInstance()->getActionHandlerName(pAH); }

/// Ah name must all be lower case
#define REGISTER_ACTION_HANDLER(handler ,name) \
class handler##Factory : public handler\
{\
public:\
	handler##Factory ()\
	{\
		nlassert(name!=NULL);																		\
		const char *c= name;																		\
		while(*c!='\0')																				\
		{																							\
			nlassert(islower(*c) || !isalpha(*c));													\
			c++;																					\
		}																							\
		CActionHandlerFactoryManager *pAHFM = CActionHandlerFactoryManager::getInstance(); \
		pAHFM->FactoryMap.insert(CActionHandlerFactoryManager::TFactoryMap::value_type(name,this));\
		pAHFM->NameMap.insert(CActionHandlerFactoryManager::TNameMap::value_type(this,name));\
	};\
}; \
	handler##Factory handler##FactoryInstance ;\
\

#endif //NL_ACTION_HANDLER_H
