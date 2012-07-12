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



#ifndef RZ_INTERFACE_PARSER_H
#define RZ_INTERFACE_PARSER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_texture.h"
#include "nel/gui/ctrl_sheet_selection.h"
#include "nel/gui/interface_link.h"
#include "nel/misc/smart_ptr.h"
#include "nel/gui/lua_helper.h"
#include "nel/gui/proc.h"
#include "nel/gui/widget_manager.h"

namespace NLGUI
{
	class CInterfaceElement;
	class CInterfaceGroup;
	class CInterfaceOptions;
	class CInterfaceLink;
	class CCtrlBase;
	class CGroupList;
	class CGroupContainer;
	class CInterfaceAnim;
	class CViewPointer;

	// ***************************************************************************
	/**
	 * class managing the interface parsing
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */

	// this is the base class for CInterfaceManager
	class CInterfaceParser : public IParser
	{

	public:
		
		class IParserModule
		{
		public:
			enum ParsingStage
			{
				None			= 0,
				Unresolved		= 1,
				Resolved		= 2,
				GroupChildren	= 4
			};

			IParserModule(){
				parser = NULL;
				parsingStage = None;
			}
			virtual ~IParserModule(){}

			bool canParseInStage( ParsingStage stage )
			{
				if( ( parsingStage & static_cast< uint >( stage ) ) != 0 )
					return true;
				else
					return false;
			}

			virtual bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup ) = 0;
			void setParser( CInterfaceParser *p ){ parser = p; }
		
		protected:
			CInterfaceParser *parser;
			uint parsingStage;
		};

		CInterfaceParser();
		virtual ~CInterfaceParser();

	public:

		/**
		 * Parsing methods
		 */

		/** Load a set of xml files
		  * \param isFilename true if xmlFileNames array contains the names of the xml file, false, if each
		  *                   array is a script itself
		  */
		bool parseInterface (const std::vector<std::string> &xmlFileNames, bool reload, bool isFilename = true, bool checkInData = false);

		bool parseXMLDocument (xmlNodePtr root, bool reload);
		bool parseTemplateNode (xmlNodePtr node,xmlNodePtr instance,xmlNodePtr templ);
		bool parseInstance(xmlNodePtr cur);
		bool parseVector (xmlNodePtr cur);
		bool parseVariable (xmlNodePtr cur, CInterfaceGroup * parentGroup);
		bool parseOptions (xmlNodePtr cur, CInterfaceGroup * parentGroup);
		bool parseGroup (xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload);
		bool parseGroupChildren(xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload);
		bool parseControl (xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload);
		bool parseLink (xmlNodePtr cur, CInterfaceGroup * parentGroup);
		bool parseView (xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload);
		bool parseTreeNode (xmlNodePtr cur, CGroupContainer *parentGroup);
		bool parseTree (xmlNodePtr cur, CWidgetManager::SMasterGroup *parentGroup);
		bool parseDefine(xmlNodePtr cur);
		bool parseProcedure(xmlNodePtr cur, bool reload);
		bool parseSheetSelection(xmlNodePtr cur);
		bool parseAnim(xmlNodePtr cur, CInterfaceGroup * parentGroup);
		bool parseStyle(xmlNodePtr cur);
		bool parseLUAScript (xmlNodePtr cur);
		bool setupTree (xmlNodePtr cur, CWidgetManager::SMasterGroup *parentGroup);
		bool setupTreeNode (xmlNodePtr cur, CGroupContainer *parentGroup);
		
		void addModule( std::string name, IParserModule *module );
		IParserModule* getModuleFor( std::string name ) const;
		void removeAllModules();

		// Called by each parse in parseXMLDocument
		bool solveDefine(xmlNodePtr cur);
		bool solveStyle(xmlNodePtr cur);

		// Solve All define in a string. return false if some define not founs (defError contains this define)
		bool solveDefine(const std::string &propVal, std::string &newPropVal, std::string &defError);

		// Called after template & options parsing
		virtual void setupOptions() { }

		/**
		 * Initializer
		 */

		bool initCoordsAndLuaScript ();

		/// Association builders : associate an element of the interface with the string ID of
		/// another element used as reference for position values
		void addParentPositionAssociation (CInterfaceElement *element, const std::string &parentID);
		void addParentSizeAssociation (CInterfaceElement *element, const std::string &parentID);
		void addParentSizeMaxAssociation (CInterfaceElement *element, const std::string &parentID);

		/// LUA Class Association builder :  associate a lua script to a group (called for each group after every document parsed)
		void addLuaClassAssociation(CInterfaceGroup *group, const std::string &luaScript);

		/**
		 * Accessors
		 */
		// access to control sheet selection
		CCtrlSheetSelection	&getCtrlSheetSelection() { return _CtrlSheetSelection; }

		/// \name Parameter variable
		// @{
		const std::string&  getDefine(const std::string &id) const;
		bool				isDefineExist(const std::string &id) const;
		void				setDefine(const std::string &id, const std::string &value);
		// @}

		/// \name Dynamic links mgt
		// @{
			/** Associate the given dynamic link with an ID
			  * \return true if succesful
			  */
			bool addLink(CInterfaceLink *link, const std::string &id);
			/** remove the given link from its ID
			  * \return true if succesful
			  */
			bool removeLink(const std::string &id);
		// @}

		/** create a template from an instance consisting of a single group
		  * \param templateName name of the template in the xml
		  * \param templateParams array containing each template parameter and its name
		  * \param number of template parameters in the array
		  */
		CInterfaceGroup *createGroupInstance(const std::string &templateName, const std::string &parentID, const std::pair<std::string, std::string> *templateParams, uint numParams, bool updateLinks = true);
		CInterfaceGroup *createGroupInstance(const std::string &templateName, const std::string &parentID, std::vector<std::pair<std::string, std::string> > &templateParams, bool updateLinks = true)
		{
			if (templateParams.size() > 0)
				return createGroupInstance(templateName, parentID, &templateParams[0], (uint)templateParams.size(), updateLinks);
			else
				return createGroupInstance(templateName, parentID, NULL, 0, updateLinks);
		}

		/** create a template from an instance consisting of a single control or group
		  * \param templateName name of the template in the xml
		  * \param templateParams array containing each template parameter and its name
		  * \param number of template parameters in the array
		  */
		CInterfaceElement *createUIElement(const std::string &templateName, const std::string &parentID, const std::pair<std::string,std::string> *templateParams, uint numParams, bool updateLinks /* = true */);
		CInterfaceElement *createUIElement(const std::string &templateName, const std::string &parentID, std::vector<std::pair<std::string, std::string> > &templateParams, bool updateLinks = true)
		{
			if (templateParams.size() > 0)
				return createUIElement(templateName, parentID, &templateParams[0], (uint)templateParams.size(), updateLinks);
			else
				return createUIElement(templateName, parentID, NULL, 0, updateLinks);
		}

		static void freeXMLNodeAndSibblings(xmlNodePtr node);

		// search a "tree" node in the hierarchy that match node. may return root! NULL if not found
		static xmlNodePtr searchTreeNodeInHierarchy(xmlNodePtr root, const char *node);

		/// \name Clearing mgt
		// @{
			void removeAllLinks();
			void removeAllProcedures();
			void removeAllDefines();
			void removeAllTemplates();
			void removeAllAnims();
			void removeAll();
		// @}

		// get info on procedure. return 0 if procedure not found
		uint getProcedureNumActions( const std::string &procName ) const;

		// return false if procedure not found, or if bad action index. return false if has some param variable (@0...)
		bool getProcedureAction( const std::string &procName, uint actionIndex, std::string &ah, std::string &params ) const;

		void setCacheUIParsing( bool b ){ cacheUIParsing = b; }

		CInterfaceAnim* getAnim( const std::string &name ) const;

		CProcedure* getProc( const std::string &name );

	protected:

		/**
		 * Temporary data for init
		 */

		/// vector storing parsed templates during init. At the end of init, only used template are kept
		std::vector<xmlNodePtr> _Templates;


		// map linking an element to its parent position used during init only
		std::map<CInterfaceElement*,std::string> _ParentPositionsMap;
		std::map<CInterfaceElement*,std::string> _ParentSizesMap;
		std::map<CInterfaceElement*,std::string> _ParentSizesMaxMap;

		// map linking a group to its lua script. used during init only
		std::map<CInterfaceGroup*,std::string> _LuaClassAssociation;

		/**
		 * Data of initialized interface
		 */

		/// Define Variable list
		typedef	std::map<std::string, std::string>		TVarMap;
		typedef	TVarMap::iterator			ItVarMap;
		typedef	TVarMap::const_iterator		CstItVarMap;
		TVarMap								_DefineMap;

		bool	validDefineChar(char c) const;

		class	CStyleProperty
		{
		public:
			std::string				Name;
			std::string				Value;
		};
		class	CStyle
		{
		public:
			std::vector<CStyleProperty>		Properties;
		};


		/// Procedure list
		typedef	std::map<std::string,CProcedure>		TProcedureMap;
		typedef	TProcedureMap::iterator			ItProcedureMap;
		typedef	TProcedureMap::const_iterator	CstItProcedureMap;
		TProcedureMap							_ProcedureMap;

		// mgt of sheet selections (inventory, buy, sell..)
		CCtrlSheetSelection		  _CtrlSheetSelection;

		// Map of dynamic links
		typedef std::map<std::string, NLMISC::CSmartPtr<CInterfaceLink> > TLinkMap;
		TLinkMap _LinkMap;

		// Map of anims
		typedef std::map<std::string, CInterfaceAnim *> TAnimMap;
		TAnimMap _AnimMap;

		// Map of styles.
		typedef	std::map<std::string, CStyle>			TStyleMap;
		TStyleMap										_StyleMap;

	protected:
		std::map< std::string, IParserModule* > moduleMap;
		// LUA
		// ----------------------------------------------------------------------------------
		// LUA Interface State. NB: The LUA environnement is not shared between Login/OutGame/InGame
		NLMISC::CSmartPtr<CLuaState> _LuaState;
		virtual void initLUA();
		void uninitLUA();
		// List of script loaded (for reloadLua command)
		std::set<std::string>	_LuaFileScripts;
		// Load A .lua. false if parse error. string 'error' contains the eventual error desc (but warning still displayed)
		bool				loadLUA(const std::string &luaFile, std::string &error);
		bool cacheUIParsing;
	};

}

#endif // RZ_INTERFACE_PARSER_H
