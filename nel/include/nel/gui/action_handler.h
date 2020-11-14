// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include "nel/misc/debug.h"
#include "nel/misc/xml_auto_ptr.h"

#include <map>

// Forward declarations for libxml2
typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;

namespace NLGUI
{

	class CCtrlBase;


	/**
	 * interface for action handlers
	 * \author Nicolas Brigand
	 * \author Nevrax France
	 * \date 2002
	 */
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
	 interface for action handlers factory
	 no release in this factory : a handler must be destroyed by the control that created it
	 */
	class CAHManager
	{
	public:
		typedef std::map< std::string, IActionHandler* > TFactoryMap;
		typedef std::map< IActionHandler*, std::string > TNameMap;

		static CAHManager* getInstance()
		{
			if (_GlobalInstance == NULL && !s_Deleted)
				_GlobalInstance = new CAHManager;
			return _GlobalInstance;
		}

		void getActionHandlers( std::vector< std::string > &handlers );

		/// return pointer to action handler or null if it doesn't exist
		IActionHandler *getActionHandler(const std::string &name) const
		{
			if( name.empty() )
				return NULL;

			TFactoryMap::const_iterator it = FactoryMap.find(name);
			if( it == FactoryMap.end() )
			{
				nlwarning( "Couldn't find action handler %s", name.c_str() );
				return NULL;
			}
			else
				return it->second;
		}

		/// Return the name of the action handler given its pointer
		const std::string &getActionHandlerName(IActionHandler *pAH) const
		{
			TNameMap::const_iterator it = NameMap.find(pAH);
			return it != NameMap.end() ? it->second : EmptyName;
		}

		/// map of action handler factories
		TFactoryMap FactoryMap;
		TNameMap    NameMap;
		std::string EmptyName;
		
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
		void parseAH(xmlNodePtr cur, const char *ahId, const char *paramId, IActionHandler *&ahRet, std::string &params);
		void parseAH(xmlNodePtr cur, const char *ahId, const char *paramId, IActionHandler *&ahRet, class CStringShared &params);
		
		/// Get the AH name from ptr
		const std::string &getAHName(IActionHandler *pAH){ return getActionHandlerName(pAH); }

		void runActionHandler(const std::string &AHName, CCtrlBase *pCaller, const std::string &Params=std::string("") );
		void runActionHandler(IActionHandler *ah, CCtrlBase *pCaller, const std::string &Params=std::string("") );

		// Submit a generic event
		void submitEvent( const std::string &evt );
		static void setEditorMode( bool b ){ editorMode = b; }

	private:
		CAHManager(){}
		static CAHManager *_GlobalInstance;
		static bool editorMode;

		class CDeleter
		{
		public:
			~CDeleter()
			{
				delete _GlobalInstance;
				_GlobalInstance = NULL;
				s_Deleted = true;
			}
		};
		static CDeleter s_Deleter;
		static bool s_Deleted;

	};

	/// Ah name must all be lower case
	#define REGISTER_ACTION_HANDLER(handler ,name)                                                      \
	class handler##Factory : public handler                                                             \
	{                                                                                                   \
	public:                                                                                             \
		handler##Factory ()                                                                             \
		{                                                                                               \
			nlassert(name!=NULL);																		\
			const char *c= name;																		\
			while(*c!='\0')																				\
			{																							\
				nlassert(islower(*c) || !isalpha(*c));													\
				c++;																					\
			}																							\
			CAHManager *pAHFM = CAHManager::getInstance();                                              \
			pAHFM->FactoryMap.insert(CAHManager::TFactoryMap::value_type(name,this));                   \
			pAHFM->NameMap.insert(CAHManager::TNameMap::value_type(this,name));                         \
		};                                                                                              \
	};                                                                                                  \
		handler##Factory handler##FactoryInstance ;                                                     \
		\


}

#endif //NL_ACTION_HANDLER_H
