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



#ifndef NL_INTERFACE_MANAGER_H
#define NL_INTERFACE_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/cdb_manager.h"
#include "nel/3d/u_texture.h"
#include "nel/3d/u_text_context.h"
#include "interface_group.h"
#include "interface_link.h"
#include "group_list.h"
#include "view_base.h"
#include "view_pointer.h"

#include "ctrl_base.h"
#include "ctrl_scroll.h"

#include "nel/gui/view_renderer.h"

// InterfaceV3
#include "interface_parser.h"
#include "ctrl_sheet_selection.h"
#include "interface_options.h"
#include "interface_config.h"
#include "interface_pointer.h"
#include "flying_text_manager.h"

#include "nel/gui/input_event_listener.h"
#include "nel/gui/db_manager.h"

// CLIENT
#include "../string_manager_client.h"
#include "yubo_chat.h"

#include "../ingame_database_manager.h"

static const float ROLLOVER_MIN_DELTA_PER_MS = 0.28f;
static const float ROLLOVER_MAX_DELTA_PER_MS = 0.12f;

//the network database node
extern CCDBSynchronised IngameDbMngr;

///\todo nico remove that
extern bool g_hidden;

///max botchat distance
#define MAX_BOTCHAT_DISTANCE_SQUARE 25

#define MAX_NUM_MODES 4

#define RZ_CATEGORY_EDIT "edit"

// #define AJM_DEBUG_TRACK_INTERFACE_GROUPS

class CGroupContainer;
class CInterfaceOptions;
class CInterfaceAnim;
class CGroupMenu;

/**
 * class managing the interface
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2002
 */
class CInterfaceManager : public CInterfaceParser, public NLGUI::IInputEventListener
{
public:

#ifdef AJM_DEBUG_TRACK_INTERFACE_GROUPS
	void DebugTrackGroupsCreated( CInterfaceGroup *pIG );
	void DebugTrackGroupsDestroyed( CInterfaceGroup *pIG );
	void DebugTrackGroupsDump();
	int DebugTrackGroupsGetId( CInterfaceGroup *pIG );

	typedef std::set<CInterfaceGroup *> setInterfaceGroupPtr;
	typedef std::map<CInterfaceGroup *, int> mapInterfaceGroupPtr2Int;

	setInterfaceGroupPtr _DebugTrackGroupSet;
	mapInterfaceGroupPtr2Int _DebugTrackGroupMap;
	int _DebugTrackGroupCreateCount;
	int _DebugTrackGroupDestroyCount;
#endif
	enum TSystemInfoMode
	{
		InfoMsg,
		WarningMsg,
		ErrorMsg
	};

	// Icon to use for validMessageBox() method
	enum TValidMessageIcon
	{
		NoIconMsg=0,
		QuestionIconMsg,
		WarningIconMsg,
		ErrorIconMsg
	};

public:

	/// Singleton method : Get the unique interface loader instance
	static CInterfaceManager* getInstance()
	{
		nlassert( _Instance != NULL );
		return _Instance;
	}

	/// Create singleton
	static void create(  NL3D::UDriver *driver, NL3D::UTextContext *textcontext  );

	/// Destroy singleton
	static void destroy ();

	/// Destructor
	~CInterfaceManager();

	/**
	 * High level
	 */

	void createLocalBranch (const std::string &fileName, NLMISC::IProgressCallback &progressCallBack);

	void reset();

	// release all of the global db autocopy observers
	void releaseServerToLocalAutoCopyObservers();

	bool isInGame() const { return _InGame; }

	/// initialize the whole login interface
	void initLogin();

	/// unload login interface
	void uninitLogin();

	/// initialize the whole out game interface
	void initOutGame();

	/// unload out game interface
	void uninitOutGame();

	/// initialize the whole in game interface
	void initInGame();

	/// Part of initInGame()
	void loadIngameInterfaceTextures();

	/// Part of initInGame()
	void loadUI();

	/// Configure the Quit dialog box
	void configureQuitDialogBox();

	/// Part of initInGame()
	void loadKeys();

	/// Part of initInGame()
	void loadInterfaceConfig();

	/// Save the game interface (keys and interface config). Called before continent and entity manager are destroyed.
	void uninitInGame0();

	/// Uninit game interface. Called after continent and entity manager are destroyed.
	void uninitInGame1();

	/// update a frame Event: update the input handler manager
	void updateFrameEvents();

	// force to flush the debug window (also done when calling updateFrameEvents)
	void flushDebugWindow();

	/// update a frame View: check coordinates and draw view. The camera is used to draw in-scene interfaces. Can be NULL.
	void updateFrameViews(NL3D::UCamera camera);


	/**
	 * Config file loaders
	 */

	/// Load texture grouping a set of small texture
	void loadTextures (const std::string &textFileName, const std::string &uvFileName, bool uploadDXTC= false);

	/// Load texts
	void loadTexts (const std::string &fileName);

	/// Load a set of xml files
	bool parseInterface (const std::vector<std::string> &xmlFileNames, bool reload, bool isFilename = true);

	// Load/Save position, size, etc.. of windows
	bool loadConfig (const std::string &filename);
	bool saveConfig (const std::string &filename);
	// delete the user config (give the player ident fileName)
	bool deletePlayerConfig (const std::string &playerFileIdent);

	// Save the keys config file
	bool saveKeys (const std::string &filename);
	// delete the user Keysconfig (give the player ident fileName)
	bool deletePlayerKeys (const std::string &playerFileIdent);

	// Log system (all chat/tell
	void setLogState(bool state) { _LogState = state; }
	bool getLogState() const { return _LogState; }
	void log(const ucstring &str);

	/// Text from here and from server

	class IStringProcess
	{
	public:
		virtual bool cbIDStringReceived(ucstring &inOut) = 0; // called when string or id is received (return true if valid the change)
	};

	void addServerString (const std::string &sTarget, uint32 id, IStringProcess *cb = NULL);
	void addServerID (const std::string &sTarget, uint32 id, IStringProcess *cb = NULL);
	void processServerIDString();

	/**
	 * get a window from its  Id of its group.
	 *	NB: "ctrl_launch_modal" is a special Id which return the last ctrl which has launch a modal. NULL if modal closed.
	 * \param groupId : the Id of the window group
	 */
	/// get an element from a define ID. shortcut for getElementFromId(getDefine(define))
	CInterfaceElement* getElementFromDefine (const std::string &defineId);

	/// Control specific

	/// Enable/Disable window movement
	//void enableMoveWindow (CInterfaceGroup *pWin);
	//void disableMoveWindow ();

	/// Enable/Disable the window resizing (0,TopLeft)(1,T)(2,TR)(3,R)(4,BR)(5,B)(6,BL)(7,L)
	//void enableResizeWindow (CInterfaceGroup *pWin, uint8 nType, sint32 nMinW, sint32 nMaxW, sint32 nMinH, sint32 nMaxH,
	//						sint32 nStepW, sint32 nStepH);
	//void disableResizeWindow ();
	//void moveWindow (CInterfaceGroup *pWin, sint32 dx, sint32 dy);

	/// Enable/Disbale capture of a control (combo box for example)
	/// When capture is lost (by clicking outside of the control, the given property is toggled)
	//void enableCaptureElement(CInterfaceElement *pElem, CInterfaceProperty *captureFlag);
	//void disableCaptureElement();

	/** Enable/Disable a single modal window (pointer cannot get out of the window).
	  * NB : the keyboard capture is released on both calls.
	  * NB : cascaded modal windows are disabled by the call
	  */


	/// Handle The Event. return true if the interfaceManager catch it and if must not send to the Game Action Manager
	bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);
	bool handleMouseMoveEvent( const NLGUI::CEventDescriptor &eventDesc );
	// execute a procedure. give a list of parameters. NB: the first param is the name of the proc (skipped)...
	void runProcedure(const std::string &procName, CCtrlBase *pCaller, const std::vector<std::string> &paramList);
	// replace an action in a procedure (if possible)
	void setProcedureAction(const std::string &procName, uint actionIndex, const std::string &ah, const std::string &params);
	// get info on procedure. return 0 if procedure not found
	uint getProcedureNumActions(const std::string &procName) const;
	// return false if procedure not found, or if bad action index. return false if has some param variable (@0...)
	bool getProcedureAction(const std::string &procName, uint actionIndex, std::string &ah, std::string &params) const;
	// Execute a anim
	void startAnim(const std::string &animId);
	void stopAnim(const std::string &animId);


	// InGame ContextMenu
	void launchContextMenuInGame (const std::string &nameOfCM);


	/**
	 * Draw views
	 */
	void drawViews (NL3D::UCamera camera);
	void drawAutoAdd ();
	void drawContextHelp ();
	//void drawContextMenu ();

	void setGlobalColor (NLMISC::CRGBA col);
	NLMISC::CRGBA getGlobalColor() { return _GlobalColor; }
	void setContentAlpha(uint8 alpha);
	uint8 getContentAlpha() const { return _ContentAlpha; }
	void setContainerAlpha(uint8 alpha);
	uint8 getContainerAlpha() const { return _ContainerAlpha; }
	NLMISC::CRGBA getGlobalColorForContent() { return _GlobalColorForContent; }
	//	these values are updated from the DB
	uint8 getGlobalContentAlpha() const { return _GlobalContentAlpha; }
	uint8 getGlobalContainerAlpha() const { return _GlobalContainerAlpha; }
	uint8 getGlobalRolloverFactorContent() const { return _GlobalRolloverFactorContent; }
	uint8 getGlobalRolloverFactorContainer() const { return _GlobalRolloverFactorContainer; }

	/// Update all the elements
	void updateAllLocalisedElements ();

	// display a debug info
	void		  displayDebugInfo(const ucstring &str, TSystemInfoMode mode = InfoMsg);
	// get the color associated with the given system info mode
	NLMISC::CRGBA getDebugInfoColor(TSystemInfoMode mode);

	// display a system info string
	void		  displaySystemInfo(const ucstring &str, const std::string &Category = "SYS");
	NLMISC::CRGBA getSystemInfoColor(const std::string &Category = "SYS");
	/// \name Global Interface Options
	// @{

	/// Get options by name
	CInterfaceOptions		*getOptions (const std::string &optName);

	// List of system options
	enum	TSystemOption
	{
		OptionCtrlSheetGrayColor=0,
		OptionCtrlTextGrayColor,
		OptionCtrlSheetRedifyColor,
		OptionCtrlTextRedifyColor,
		OptionCtrlSheetGreenifyColor,
		OptionCtrlTextGreenifyColor,
		OptionViewTextOverBackColor,
		OptionFont,
		OptionAddCoefFont,
		OptionMulCoefAnim,
		OptionTimeoutBubbles,
		OptionTimeoutMessages,
		OptionTimeoutContext,
		OptionTimeoutContextHtml,

		NumSystemOptions,
	};

	virtual void setupOptions();

	/** Get a system option by its enum (faster than getOptions() and getVal())
	 *	NB: array updated after each parseInterface()
	 */
	const CInterfaceOptionValue	&getSystemOption(TSystemOption o) const {return _SystemOptions[o];}

	// @}


	/** Open a MessageBox. this is a simple ModalWindow with a Ok button
	 *	ui:interface:message_box must be defined in xml, with a "text" ViewText son
	 */
	void	messageBox(const ucstring &text, const std::string &masterGroup="ui:interface", TCaseMode caseMode = CaseFirstSentenceLetterUp);
	/** Open a MessageBox. this is a simple ModalWindow with a Ok and a HELP button.
	 *  The help button with open a browser on ryzom.com faq
	 *	ui:interface:message_box_with_help must be defined in xml, with a "text" ViewText son
	 */
	void	messageBoxWithHelp(const ucstring &text, const std::string &masterGroup="ui:interface",
							   const std::string &ahOnOk = std::string(), const std::string &paramsOnOk= std::string(),
							   TCaseMode caseMode = CaseFirstSentenceLetterUp);

	/** Open a MessageBox with validation question. this is a simple ModalWindow with a Ok / Cancel button
	 *	ui:interface:valid_message_box must be defined in xml, with a "text" ViewText son, and a "ok" button
	 *	\param ahOnOk => the action handler to call if ok is pressed. NB: you don't have to call leave_modal in this ah (auto done).
	 *	\param paramsOnOk => params passed to ahOnOk.
	 *	\param ahOnCancel => the action handler to call if cancel is pressed. NB: you don't have to call leave_modal in this ah (auto done).
	 *	\param paramsOnCancel => params passed to ahOnCancel.
	 */
	void	validMessageBox(TValidMessageIcon icon, const ucstring &text, const std::string &ahOnOk, const std::string &paramsOnOk= std::string(),
		const std::string &ahOnCancel= std::string(), const std::string &paramsOnCancel= std::string(), const std::string &masterGroup="ui:interface");

	/** Get the current running validMessageBox OnOk action. empty if no validMessageBox currently opened
	 *	One can use it to know if it match its system and so if it needs to be closed (with disableModalWindow())
	 */
	bool	getCurrentValidMessageBoxOnOk(std::string &ahOnOk, const std::string &masterGroup="ui:interface");

	/// force disable the context help
	void	disableContextHelp();
	/// force disable the context help, if it is issued from the given control
	void	disableContextHelpForControl(CCtrlBase *pCtrl);
	/// for ContextHelp action handler only: set the result name
	void	setContextHelpText(const ucstring &text);

	void	setContextHelpActive(bool active);

	bool	isMouseOverWindow() const {return  _MouseOverWindow;}

	// Enable mouse Events to interface. if false, release Captures.
	void	enableMouseHandling(bool handle);
	bool    isMouseHandlingEnabled() const { return _MouseHandlingEnabled; }

	// Modes
	void	setMode(uint8 newMode);
	uint8	getMode() const { return _CurrentMode; }
	void	resetMode(uint8 newMode);
	// Update image of a single group container in a virtual desktop.
	// This is faster than switching to a new desktop using 'setMode' to update the changes
	// \param mode Index of the virtual desktop
	void	updateGroupContainerImage(CGroupContainer &gc, uint8 mode);
	// Remove a group container from a virtual desktop image
	// \param mode Index of the virtual desktop
	void	removeGroupContainerImage(const std::string &groupName, uint8 mode);




	// debug : dump all the interface windows in the console
	void	dumpUI(bool indent);

	// debug : display box of all elements in the interface (plus hotspot)
	void	displayUIViewBBoxs(const std::string &uiFilter);
	void	displayUICtrlBBoxs(const std::string &uiFilter);
	void	displayUIGroupBBoxs(const std::string &uiFilter);

	// Get the User DblClick Delay (according to save...), in milisecond
	uint	getUserDblClickDelay();

	// Submit a generic event
	void	submitEvent (const std::string &event);

	// visit all elements of the interface manager
	void	visit(CInterfaceElementVisitor *visitor);

	// Display the only one web window at end of ryzom
	void    displayWebWindow(const std::string &name, const std::string &url);

	// Initialize emote from TextEmotListSheet (create command and insert the GCM entry)
	void	initEmotes();
	void	uninitEmotes();
	void	updateEmotes();

	// For Drag And drop, return true if the "CopyDrag" key is pressed (actually the ctrl key!)
	bool	testDragCopyKey();

	// Reset the view text index
	void	resetTextIndex();

	// test if the config has been loaded in initInGame()
	bool	isConfigLoaded() const {return _ConfigLoaded;}

	/** connect or reconnect to the yubo chat (if was kicked for instance).
	 *	The YuboChat is a special telnet chat for Game Masters, same channel as the Yubo Klient
	 */
	void	connectYuboChat();
	/// send a string to the yubo chat
	void	sendStringToYuboChat(const ucstring &str);


	/// Manager for flying text. use it to add
	CFlyingTextManager		FlyingTextManager;


	/// \name LUA
	// @{
	/// Execute a lua script (smallScript for speed optimisation, see lua_helper). return false if parse/execute error (warning/sysinfo displayed)
	bool	executeLuaScript(const std::string &luaScript, bool smallScript= false);
	/// Reload all LUA scripts inserted through <lua>
	void	reloadAllLuaFileScripts();
	/// For debug: dump in the sysinfo and nlwarning state of lua. detail range from 0 to 2 (clamped).
	void		dumpLuaState(uint detail);
	/// For debug: force a garbage collector
	void		luaGarbageCollect();
	// @}

	// Get the list of InGame XML Interface files, with any AddOn ones
	static std::vector<std::string>		getInGameXMLInterfaceFiles();

	// hide all the windows
	void		hideAllWindows();
	void		hideAllNonSavableWindows();

	/// \name Action Counter sync
	// @{

	void			incLocalSyncActionCounter();

	uint8			getLocalSyncActionCounter() const {return _LocalSyncActionCounter;}
	uint8			getLocalSyncActionCounterMask() const {return _LocalSyncActionCounterMask;}

	bool			localActionCounterSynchronizedWith(NLMISC::CCDBNodeLeaf *leaf)
	{
		if (!leaf) return false;
		uint	srvVal= leaf->getValue32();
		uint	locVal= _LocalSyncActionCounter ;
		srvVal&= _LocalSyncActionCounterMask;
		locVal&= _LocalSyncActionCounterMask;
		return srvVal == locVal;
	}

	void			resetShardSpecificData();

	// @}


	// Get the alpha roll over speed
	float getAlphaRolloverSpeed()
	{
		if (!_AlphaRolloverSpeedDB)
			_AlphaRolloverSpeedDB = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:ALPHA_ROLLOVER_SPEED");
		float fTmp = ROLLOVER_MIN_DELTA_PER_MS + (ROLLOVER_MAX_DELTA_PER_MS - ROLLOVER_MIN_DELTA_PER_MS) * 0.01f * (100 - _AlphaRolloverSpeedDB->getValue32());
		return fTmp*fTmp*fTmp;
	}

	// For single lined ViewText that are clipped: on over of viewText too big, the text is drawn on top. A CRefPtr is kept
	void	setOverExtendViewText(CViewText *vt, NLMISC::CRGBA backGround);

	// Item Carac Test, get the value
	bool	isItemCaracRequirementMet(CHARACTERISTICS::TCharacteristics type, sint32 value)
	{
		// carac requirement?
		if( type < CHARACTERISTICS::NUM_CHARACTERISTICS)
			return value <= _CurrentPlayerCharac[type];
		// no carac requirement => ok
		else
			return true;
	}

	// get Player Carac
	sint32	getCurrentPlayerCarac(CHARACTERISTICS::TCharacteristics type)
	{
		if( type < CHARACTERISTICS::NUM_CHARACTERISTICS)
			return _CurrentPlayerCharac[type];
		else
			return 0;
	}

	// Description of the last key event that called an action handler
	const NLGUI::CEventDescriptorKey&	getLastEventKeyDesc() const { return _LastEventKeyDesc; }

	void	notifyMailAvailable();
	void	notifyForumUpdated();

	void updateTooltipCoords();

	/** Returns a human readable timestamp with the given format.
	 */
	static char* getTimestampHuman(const char* format = "[%H:%M:%S] ");

	/** Parses any tokens in the ucstring like $t$ or $g()$
	 */
	static bool parseTokens(ucstring& ucstr);

	/// Sets the current TextContext.
	void setTextContext( NL3D::UTextContext *textcontext );

	/// Retrueves the current TextContext
	inline NL3D::UTextContext* getTextContext() const{ return textcontext; };

// ------------------------------------------------------------------------------------------------
private:

	// Observer for copying db branch changes
	class CServerToLocalAutoCopy
	{
	public:
		CServerToLocalAutoCopy();
		~CServerToLocalAutoCopy()	{ release(); }

		// init the AutoCopy
		void	init(const std::string &dbPath);
		// unhook from everything we are tangled up in
		void	release();

		// When something in the SERVER DB changes
		void	onServerChange(NLMISC::ICDBNode *serverNode);
		// When something in the LOCAL DB changes
		void	onLocalChange(NLMISC::ICDBNode *localNode);

	private:
		class CLocalDBObserver : public NLMISC::ICDBNode::IPropertyObserver
		{
		public:
			CServerToLocalAutoCopy	&_Owner;
			CLocalDBObserver(CServerToLocalAutoCopy	&owner) : _Owner(owner) {}
			virtual void	update(NLMISC::ICDBNode *node)	{_Owner.onLocalChange(node);}
		};
		class CServerDBObserver : public NLMISC::ICDBNode::IPropertyObserver
		{
		public:
			CServerToLocalAutoCopy	&_Owner;
			CServerDBObserver(CServerToLocalAutoCopy	&owner) : _Owner(owner) {}
			virtual void	update(NLMISC::ICDBNode *node)	{_Owner.onServerChange(node);}
		};

		// A node here is a pair Server<->Local
		struct CNode
		{
			NLMISC::CCDBNodeLeaf	*ServerNode;
			NLMISC::CCDBNodeLeaf	*LocalNode;
			bool			InsertedInUpdateList;
			CNode()
			{
				ServerNode= NULL;
				LocalNode= NULL;
				InsertedInUpdateList= false;
			}
		};
		// Struct for comparing nodes, by either Local or Server pointer
		struct CNodeLocalComp
		{
			CNode	*Node;
			bool	operator<=(const CNodeLocalComp &o) const	{return Node->LocalNode <= o.Node->LocalNode;}
			bool	operator<(const CNodeLocalComp &o) const	{return Node->LocalNode < o.Node->LocalNode;}
		};
		struct CNodeServerComp
		{
			CNode	*Node;
			bool	operator<=(const CNodeServerComp &o) const	{return Node->ServerNode <= o.Node->ServerNode;}
			bool	operator<(const CNodeServerComp &o) const	{return Node->ServerNode < o.Node->ServerNode;}
		};

	private:
		// Counter Node
		NLMISC::CCDBNodeLeaf			*_ServerCounter;
		// updaters
		CLocalDBObserver		_LocalObserver;
		CServerDBObserver		_ServerObserver;
		// avoid reentrance
		bool					_LocalUpdating;

		// Array of Nodes that have to be synchronized
		std::vector<CNode>			_Nodes;
		// Sorting of Nodes, by Server Node
		std::vector<CNodeServerComp>	_ServerNodeMap;
		// Sorting of Nodes, by Local Node
		std::vector<CNodeLocalComp>	_LocalNodeMap;
		// List of nodes to update until next synchonized client-server counter
		std::vector<CNode*>			_UpdateList;

		void	buildRecursLocalLeaves(NLMISC::CCDBNodeBranch *branch, std::vector<NLMISC::CCDBNodeLeaf*> &leaves);
	};

	// Database management stuff
	class CDBLandmarkObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode *node);
	};

	// EMOTES
	// ----------------------------------------------------------------------------------
	class CEmoteCmd : public NLMISC::ICommand
	{
	public:
		CEmoteCmd(const char *cmdName, const char *cmdHelp, const char *cmdArgs)
			: NLMISC::ICommand("emotes", cmdName, cmdHelp, cmdArgs)
		{
		}

		bool execute(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human=true);

		uint32 Behaviour; // State id from list.emot
		uint32 EmoteNb;
	};

// ------------------------------------------------------------------------------------------------
public:
	// cache and expose some commonly used db nodes
	NLMISC::CCDBNodeBranch *_DBB_UI_DUMMY;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_QUANTITY;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_QUALITY;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_SHEET;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_NAMEID;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_ENCHANT;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_SLOT_TYPE;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_PHRASE;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_WORNED;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_PREREQUISIT_VALID;
	NLMISC::CCDBNodeLeaf *_DB_UI_DUMMY_FACTION_TYPE;

private:

	NLMISC::CCDBNodeLeaf *_CheckMailNode;
	NLMISC::CCDBNodeLeaf *_CheckForumNode;
	sint64 _UpdateWeatherTime;

	// @}

	/// \name Check Yubo Chat (special telnet chat for Game Masters, same channel as the Yubo Klient)
	// @{
	CYuboChat	_YuboChat;
	void		checkYuboChat();
	// @}


	/** This is the GLOBAL Action counter used to synchronize some systems (including INVENTORY) with the server.
	 */
	uint8			_LocalSyncActionCounter;

	/// This is the Mask (4bits)
	uint8			_LocalSyncActionCounterMask;


	uint8 _ContentAlpha;
	uint8 _ContainerAlpha;
	NLMISC::CRGBA _GlobalColorForContent;
	//
	uint8 _GlobalContentAlpha;
	uint8 _GlobalContainerAlpha;
	uint8 _GlobalRolloverFactorContent;
	uint8 _GlobalRolloverFactorContainer;

	/// Constructor
	CInterfaceManager( NL3D::UDriver *driver, NL3D::UTextContext *textcontext );

	///the singleton's instance
	static CInterfaceManager* _Instance;

	NLMISC::CCDBNodeLeaf	   *_DescTextTarget;

	bool		_MouseOverWindow;

	// Context Help
	bool					_ContextHelpActive;
	//CCtrlBasePtr			_CurCtrlContextHelp;
	//Delay before displaying ContextHelp on a ctrl having wantInstantContextHelp set to false (in seconds)
	float					_MaxTimeStopingContextHelp;
	sint					_LastXContextHelp;
	sint					_LastYContextHelp;
	ucstring				_ContextHelpText;
	CCtrlBase				*getNewContextHelpCtrl();

	/// Current waiting id and string from server
	struct SIDStringWaiter
	{
		STRING_MANAGER::IStringWaiterRemover SWR;
		bool IdOrString; // true == id, false == string
		uint32 Id;
		std::string Target;
		IStringProcess *Cb;
	};
	std::vector<SIDStringWaiter*> _IDStringWaiters;

	uint32			_ScreenW, _ScreenH; // Change res detection
	NLMISC::CRGBA	_GlobalColor;
	sint32			_LastInGameScreenW, _LastInGameScreenH; // Resolution used for last InGame interface

	// List of active Anims
	std::vector<CInterfaceAnim*> _ActiveAnims;

	bool isControlInWindow (CCtrlBase *ctrl, CInterfaceGroup *pNewCurrentWnd);
	uint getDepth (CCtrlBase *ctrl, CInterfaceGroup *pNewCurrentWnd);

	// System Options
	CInterfaceOptionValue	_SystemOptions[NumSystemOptions];

	bool			_MouseHandlingEnabled;


	// Modes
	CInterfaceConfig::CDesktopImage	_Modes[MAX_NUM_MODES];
	uint8				_CurrentMode;

	// true when interface manager is running 'ingame' content
	bool				_InGame;

	// Does the interface config file as been loaded ?
	bool				_ConfigLoaded;
	bool				_LogState;

	// Does the keys config file for as been loaded ?
	bool				_KeysLoaded;

	// clear all edit box in the ui
	void    clearAllEditBox();
	// restore all backuped positions for containers
	void    restoreAllContainersBackupPosition();

	// Some Node leaf
	NLMISC::CCDBNodeLeaf *_NeutralColor;
	NLMISC::CCDBNodeLeaf *_WarningColor;
	NLMISC::CCDBNodeLeaf *_ErrorColor;
	NLMISC::CCDBNodeLeaf *_RProp;
	NLMISC::CCDBNodeLeaf *_GProp;
	NLMISC::CCDBNodeLeaf *_BProp;
	NLMISC::CCDBNodeLeaf *_AProp;
	NLMISC::CCDBNodeLeaf *_AlphaRolloverSpeedDB;

	// The next ViewText to draw for Over
	NLMISC::CRefPtr<CInterfaceElement>	_OverExtendViewText;
	NLMISC::CRGBA						_OverExtendViewTextBackColor;
	void			drawOverExtendViewText();

	CInterfaceGroup	*getWindowForActiveMasterGroup(const std::string &windowName);

	CDBLandmarkObs _LandmarkObs;

	/// \name LUA
	// @{
	// str= printable version of value at stack index
	void	getLuaValueInfo(std::string &str, sint index);
	// display a string in SysInfo and nlinfo
	void	dumpLuaString(const std::string &str);
	// dump printable version of pair key-index in top of stack. if value is a table, recurs (up to recursTableLevel times)
	void	dumpLuaKeyValueInfo(uint recursTableLevel, uint tabLevel);
	// @}

	bool						_EmotesInitialized;
	std::vector<CEmoteCmd*>		_EmoteCmds;

	// Item Carac requirement
	sint32		_CurrentPlayerCharac[CHARACTERISTICS::NUM_CHARACTERISTICS];

	// Description of the last key event that called an action handler
	NLGUI::CEventDescriptorKey	_LastEventKeyDesc;

	// observers for copying database branch changes
	CServerToLocalAutoCopy ServerToLocalAutoCopyInventory;
	CServerToLocalAutoCopy ServerToLocalAutoCopyExchange;
	CServerToLocalAutoCopy ServerToLocalAutoCopyContextMenu;
	CServerToLocalAutoCopy ServerToLocalAutoCopySkillPoints;
	CServerToLocalAutoCopy ServerToLocalAutoCopyDMGift;

	// move windows according to new screen size
	void moveAllWindowsToNewScreenSize(sint32 newScreenW, sint32 newScreenH, bool fixCurrentUI);
	void getNewWindowCoordToNewScreenSize(sint32 &x, sint32 &y, sint32 w, sint32 h, sint32 newW, sint32 newH) const;

	// Pop a new message box. If the message box was found, returns a pointer on it
	void messageBoxInternal(const std::string &msgBoxGroup, const ucstring &text, const std::string &masterGroup, TCaseMode caseMode);

	// Internal : adjust a tooltip with respect to its parent. Returns the number of coordinate that were clamped
	// against the screen border
	uint adjustTooltipPosition(CCtrlBase *newCtrl,
							   CInterfaceGroup *win,
							   THotSpot ttParentRef,
							   THotSpot ttPosRef,
							   sint32 xParent,
							   sint32 yParent,
							   sint32 wParent,
							   sint32 hParent
							  );

	// Update tooltip coordinate if they need to be (getInvalidCoords() returns a value != 0)
	void updateTooltipCoords(CCtrlBase *newCtrl);

	NL3D::UDriver *driver;
	NL3D::UTextContext *textcontext;
	CInterfaceLink::CInterfaceLinkUpdater *interfaceLinkUpdater;

};

#endif // NL_INTERFACE_MANAGER_H

/* End of interface_manager.h */
