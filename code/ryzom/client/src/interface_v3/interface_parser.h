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
#include "ctrl_sheet_selection.h"
#include "interface_link.h"
#include "nel/misc/smart_ptr.h"
#include "game_share/brick_types.h"
#include "lua_helper.h"

// ***************************************************************************
class CInterfaceElement;
class CInterfaceGroup;
class CGroupContainer;
class CGroupList;
class CInterfaceOptions;
class CInterfaceAnim;
class CViewPointer;
class CInterfaceLink;
class CBrickJob;
class CCtrlBase;

// ***************************************************************************

#define WIN_PRIORITY_MAX	8
#define WIN_PRIORITY_WORLD_SPACE	0
#define WIN_PRIORITY_LOWEST		1
#define WIN_PRIORITY_LOW		2
#define WIN_PRIORITY_NORMAL		3
#define WIN_PRIORITY_HIGH		4
#define WIN_PRIORITY_HIGHEST	5

// ***************************************************************************
/**
 * class managing the interface parsing
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2002
 */

// this is the base class for CInterfaceManager
class CInterfaceParser
{

public:
	CInterfaceParser();
	virtual ~CInterfaceParser();

	struct SMasterGroup
	{
		SMasterGroup()
		{
			Group= NULL;
			LastTopWindowPriority= WIN_PRIORITY_NORMAL;
		}

		CInterfaceGroup *Group;
		std::list<CInterfaceGroup*> PrioritizedWindows[WIN_PRIORITY_MAX];

		void addWindow(CInterfaceGroup *pIG, uint8 nPrio = WIN_PRIORITY_NORMAL);
		void delWindow(CInterfaceGroup *pIG);
		CInterfaceGroup *getWindowFromId(const std::string &winID);
		bool isWindowPresent(CInterfaceGroup *pIG);
		// Set a window top in its priority queue
		void setTopWindow(CInterfaceGroup *pIG);
		void setBackWindow(CInterfaceGroup *pIG);
		void deactiveAllContainers();
		void centerAllContainers();
		void unlockAllContainers();

		// Sort the world space group
		void sortWorldSpaceGroup ();

		uint8		LastTopWindowPriority;
	};

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

	// bool parseDynamicList (xmlNodePtr cur, CInterfaceGroup * parentGroup);

	bool parseVector (xmlNodePtr cur);

	bool parseObserver (xmlNodePtr cur, CInterfaceGroup * parentGroup);

	bool parseVariable (xmlNodePtr cur, CInterfaceGroup * parentGroup);

	bool parseOptions (xmlNodePtr cur, CInterfaceGroup * parentGroup);

	bool parseGroup (xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload);

	bool parseGroupChildren(xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload);

	bool parseControl (xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload);

	bool parseLink (xmlNodePtr cur, CInterfaceGroup * parentGroup);

	bool parseView (xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload);

	bool parseTreeNode (xmlNodePtr cur, CGroupContainer *parentGroup);

	bool parseTree (xmlNodePtr cur, SMasterGroup *parentGroup);

	bool parseDefine(xmlNodePtr cur);

	bool parseProcedure(xmlNodePtr cur, bool reload);

	bool parseSheetSelection(xmlNodePtr cur);

	bool parseCareerGenerator(xmlNodePtr cur);

	bool parseAnim(xmlNodePtr cur, CInterfaceGroup * parentGroup);

	bool parseScene3D (xmlNodePtr cur, CInterfaceGroup * parentGroup);

	bool parseActionCategory (xmlNodePtr cur);

	bool parseKey(xmlNodePtr cur);

	bool parseMacro(xmlNodePtr cur);

	bool parseCommand(xmlNodePtr cur);

	bool parseBrickCareerGenerator(xmlNodePtr cur);

	bool parseBrickSuffixGenerator(xmlNodePtr cur);

	bool parseStyle(xmlNodePtr cur);

	bool parseDDX (xmlNodePtr cur, CInterfaceGroup * parentGroup);

	bool parseLUAScript (xmlNodePtr cur);

	bool setupTree (xmlNodePtr cur, SMasterGroup *parentGroup);
	bool setupTreeNode (xmlNodePtr cur, CGroupContainer *parentGroup);

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

	CInterfaceGroup* getMasterGroupFromId (const std::string &MasterGroupName);
	const std::vector<SMasterGroup> &getAllMasterGroup() { return _MasterGroups; }
	SMasterGroup& getMasterGroup(uint8 i) { return _MasterGroups[i]; }
	CInterfaceGroup* getWindowFromId (const std::string & groupId);
	void addWindowToMasterGroup (const std::string &sMasterGroupName, CInterfaceGroup *pIG);
	void removeWindowFromMasterGroup (const std::string &sMasterGroupName, CInterfaceGroup *pIG);
	// access to control sheet selection
	CCtrlSheetSelection	&getCtrlSheetSelection() { return _CtrlSheetSelection; }

	/// \name Parameter variable
	// @{
	const std::string	&getDefine(const std::string &id) const;
	bool				isDefineExist(const std::string &id) const;
	void				setDefine(const std::string &id, const std::string &value);
	// @}

	/** From a target name of a link, retrieve the target element and its target target property
	  * \return true if the target is valid
	  */
	static bool splitLinkTarget(const std::string &target, CInterfaceGroup *parentGroup, std::string &propertyName, CInterfaceElement *&targetElm);

	/** From several target names of a link (seprated by ','), retrieve the target elements and their target properties
	  * \return true if all targets are valid
	  */
	static bool splitLinkTargets(const std::string &targets, CInterfaceGroup *parentGroup, std::vector<CInterfaceLink::CTargetInfo> &targetsVect);

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
		void removeAllOptions();
		void removeAllProcedures();
		void removeAllDefines();
		void removeAllTemplates();
		void removeAllAnims();
		void removeAllMasterGroups();
		void removeAll();
	// @}

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

	CViewPointer *_Pointer;

	// Master groups encapsulate all windows
	std::vector<SMasterGroup> _MasterGroups;

	// Options description
	std::map<std::string, NLMISC::CSmartPtr<CInterfaceOptions> > _OptionsMap;

	/// Define Variable list
	typedef	std::map<std::string, std::string>		TVarMap;
	typedef	TVarMap::iterator			ItVarMap;
	typedef	TVarMap::const_iterator		CstItVarMap;
	TVarMap								_DefineMap;

	bool	validDefineChar(char c) const;

	/// Procedure def
	class	CParamBlock
	{
	public:
		// -1 if not a param id, but a string
		sint32		NumParam;
		std::string		String;

		CParamBlock()
		{
			NumParam= -1;
		}
	};
	class	CAction
	{
	public:
		// a condition to launch this action handler (is an expression)
		std::vector<CParamBlock>	CondBlocks;

		// the action handler (may be proc!!)
		std::string					Action;
		// A list of string/or param number => to build the final params at execution
		std::vector<CParamBlock>	ParamBlocks;

		// build a paramBlock from a string
		void		buildParamBlock (const std::string &params);
		// from ParamBlock, and a paramList (skip the 0th), build params.
		void		buildParams (const std::vector<std::string> &paramList, std::string &params) const;

		void		buildCondBlock (const std::string &params);

		void		buildCond (const std::vector<std::string> &paramList, std::string &cond) const;

		static void buildBlocks (const std::string &in, std::vector<CParamBlock> &out);
		static void eval (const std::vector<std::string> &inArgs, const std::vector<CParamBlock> &inBlocks, std::string &out);

	};
	class	CProcedure
	{
	public:
		// List of the actions
		std::vector<CAction>	Actions;
	};
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

	bool	parseCareerGeneratorParams(xmlNodePtr cur,
		std::string	&templateCareer,
		std::string	&templateJob,
		std::string	&careerWindow,
		std::string	&jobWindow,
		xmlNodePtr	&rootTreeNode,
		bool		&brickTypeFilter,
		BRICK_TYPE::EBrickType	&brickType
		);

	void	createJobBricks(BRICK_TYPE::EBrickType	brickType, xmlNodePtr &nextSibling, xmlNodePtr parentTreeNode,
		const CBrickJob &job, const std::string &templateBrick, const std::string &baseWindowId, sint32 xstart);

	bool	parseGeneratorRootContainer(xmlNodePtr cur, xmlNodePtr	&rootTreeNode);


protected:
	// LUA
	// ----------------------------------------------------------------------------------
	// LUA Interface State. NB: The LUA environnement is not shared between Login/OutGame/InGame
	NLMISC::CSmartPtr<CLuaState> _LuaState;
	void				initLUA();
	void				uninitLUA();
	// List of script loaded (for reloadLua command)
	std::set<std::string>	_LuaFileScripts;
	// Load A .lua. false if parse error. string 'error' contains the eventual error desc (but warning still displayed)
	bool				loadLUA(const std::string &luaFile, std::string &error);
};

#endif // RZ_INTERFACE_PARSER_H
