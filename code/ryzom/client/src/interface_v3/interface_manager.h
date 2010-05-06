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
#include "nel/3d/u_texture.h"
#include "nel/3d/u_text_context.h"
#include "interface_group.h"
#include "interface_link.h"
#include "group_list.h"
#include "view_base.h"
#include "view_pointer.h"

#include "ctrl_base.h"
#include "ctrl_scroll.h"

#include "view_renderer.h"

// InterfaceV3
#include "interface_parser.h"
#include "ctrl_sheet_selection.h"
#include "interface_options.h"
#include "interface_config.h"
#include "interface_pointer.h"
#include "flying_text_manager.h"

// CLIENT
#include "../string_manager_client.h"
#include "yubo_chat.h"

static const float ROLLOVER_MIN_DELTA_PER_MS = 0.28f;
static const float ROLLOVER_MAX_DELTA_PER_MS = 0.12f;

//the NEL 3d textcontext
extern NL3D::UTextContext *TextContext;

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
class CInterfaceManager : public CInterfaceParser
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
		if (_Instance == NULL)
			_Instance = new CInterfaceManager();
		return _Instance;
	}

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


	/// Get the root of the database
	CCDBNodeBranch *getDB() const { return _DbRootNode; }
	// yoyo: should avoid to try creating DbPropr with this system... very dangerous
	CCDBNodeLeaf* getDbProp (const std::string & name, bool bCreate=true);
	// get a Db Branch by its name. NULL if don't exist or not a branch (never try to create it)
	CCDBNodeBranch *getDbBranch(const std::string &name);
	// return the DB as an int32. return 0 if the DB does not exist (never create)
	sint32			getDbValue32 (const std::string & name);

	/**
	 * get the window under a spot
	 * \param : X coord of the spot
	 * \param : Y coord of the spot
	 * \return : pointer to the window
	 */
	CInterfaceGroup* getWindowUnder (sint32 x, sint32 y);
	CInterfaceGroup* getCurrentWindowUnder() { return _WindowUnder; }
	CInterfaceGroup* getGroupUnder (sint32 x, sint32 y);
	void getViewsUnder (sint32 x, sint32 y, std::vector<CViewBase*> &vVB);
	void getCtrlsUnder (sint32 x, sint32 y, std::vector<CCtrlBase*> &vICL);
	void getGroupsUnder (sint32 x, sint32 y, std::vector<CInterfaceGroup*> &vIGL);
	/**
	 * get a window from its  Id of its group.
	 *	NB: "ctrl_launch_modal" is a special Id which return the last ctrl which has launch a modal. NULL if modal closed.
	 * \param groupId : the Id of the window group
	 */
	CInterfaceElement* getElementFromId (const std::string &sEltId);
	CInterfaceElement* getElementFromId (const std::string &sStart, const std::string &sEltId);
	void activateMasterGroup (const std::string &sMasterGroupName, bool bActive);
	/// get an element from a define ID. shortcut for getElementFromId(getDefine(define))
	CInterfaceElement* getElementFromDefine (const std::string &defineId);
	/// Get the window from an element (ui:interface:###)
	CInterfaceGroup* getWindow(CInterfaceElement*);
	/**
	 * set the top window
	 * \param win : pointer to the window to be set on top
	 */
	void setTopWindow (CInterfaceGroup *pWin);

	/**
	 * set the back window
	 * \param win : pointer to the window to be set on top
	 */
	void setBackWindow (CInterfaceGroup *pWin);

	/** get the top window in the first activated masterGroup
	 */
	CInterfaceGroup		*getTopWindow (uint8 nPriority = WIN_PRIORITY_NORMAL) const;

	/** get the back window in the first activated masterGroup
	 */
	CInterfaceGroup		*getBackWindow (uint8 nPriority = WIN_PRIORITY_NORMAL) const;

	/** get the last escapable top window in the first activated masterGroup
	 */
	CInterfaceGroup		*getLastEscapableTopWindow() const;

	void setWindowPriority (CInterfaceGroup *pWin, uint8 nPriority);

	/** return the priority of the Last Window setTopWindow()-ed.
	 */
	uint8				getLastTopWindowPriority() const;

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
	void enableModalWindow (CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG);
	void enableModalWindow (CCtrlBase *ctrlLaunchingModal, const std::string &groupName);
	// Disable all modals windows
	void disableModalWindow ();

	/** Push a modal window that becomes the current modal window
	  */
	void pushModalWindow(CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG);
	void pushModalWindow (CCtrlBase *ctrlLaunchingModal, const std::string &groupName);
	void popModalWindow();
	// pop all top modal windows with the given category (a string stored in the modal)
	void popModalWindowCategory(const std::string &category);

	CCtrlBase *getCtrlLaunchingModal ()
	{
		if (_ModalStack.empty()) return NULL;
		return _ModalStack.back().CtrlLaunchingModal;
	}
	/// get the currently active modal window, or NULL if none
	CInterfaceGroup *getModalWindow() const
	{
		if (_ModalStack.empty()) return NULL;
		return _ModalStack.back().ModalWindow;
	}


	/// Handle The Event. return true if the interfaceManager catch it and if must not send to the Game Action Manager
	bool handleEvent (const CEventDescriptor &eventDesc);
	void runActionHandler (const std::string &AHName, CCtrlBase *pCaller,
							const std::string &Params=std::string(""));
	void runActionHandler (IActionHandler *ah, CCtrlBase *pCaller,
							const std::string &Params=std::string(""));
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
	void checkCoords();
	void drawViews (NL3D::UCamera camera);
	void drawAutoAdd ();
	void drawContextHelp ();
	//void drawContextMenu ();

	CViewRenderer &getViewRenderer ()  { return _ViewRenderer; }
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


	/// Pointer
	CViewPointer *getPointer () { return _Pointer; }

	// Relative move of pointer
	void movePointer (sint32 dx, sint32 dy);
	// Set absolute coordinates of pointer
	void movePointerAbs(sint32 px, sint32 py);
	const std::vector<CViewBase*> &getViewsUnderPointer () { return _ViewsUnderPointer; }
	const std::vector<CInterfaceGroup *> &getGroupsUnderPointer () { return _GroupsUnderPointer; }
	const std::vector<CCtrlBase*> &getCtrlsUnderPointer () { return _CtrlsUnderPointer; }
	//
	void  clearGroupsUnders() { _GroupsUnderPointer.clear(); }
	void  clearViewUnders() { _ViewsUnderPointer.clear(); }
	void  clearCtrlsUnders() { _CtrlsUnderPointer.clear(); }

	// Remove all references on a view (called when the ctrl is destroyed)
	void	removeRefOnView (CViewBase *ctrlBase);

	// Remove all references on a ctrl (called when the ctrl is destroyed)
	void	removeRefOnCtrl (CCtrlBase *ctrlBase);

	// Remove all references on a group (called when the group is destroyed)
	void	removeRefOnGroup (CInterfaceGroup *group);

	/**
	 * Capture
	 */
	CCtrlBase *getCapturePointerLeft() { return _CapturePointerLeft; }
	CCtrlBase *getCapturePointerRight() { return _CapturePointerRight; }
	CCtrlBase *getCaptureKeyboard() { return _CaptureKeyboard; }
	CCtrlBase *getOldCaptureKeyboard() { return _OldCaptureKeyboard; }
	CCtrlBase *getDefaultCaptureKeyboard() { return _DefaultCaptureKeyboard; }

	void setCapturePointerLeft(CCtrlBase *c);
	void setCapturePointerRight(CCtrlBase *c);
	void setOldCaptureKeyboard(CCtrlBase *c) { _OldCaptureKeyboard = c; }
	// NB: setCaptureKeyboard(NULL) has not the same effect as resetCaptureKeyboard(). it allows the capture
	// to come back to the last captured window (resetCaptureKeyboard() not)
	void setCaptureKeyboard(CCtrlBase *c);
	void resetCaptureKeyboard();
	/**  Set the default box to use when no keyboard has been previously captured
	  *  The given dialog should be static
	  */
	void setDefaultCaptureKeyboard(CCtrlBase *c) { _DefaultCaptureKeyboard = c; }

	/// Update all the elements
	void updateAllLocalisedElements ();

	// display a debug info
	void		  displayDebugInfo(const ucstring &str, TSystemInfoMode mode = InfoMsg);
	// get the color associated with the given system info mode
	NLMISC::CRGBA getDebugInfoColor(TSystemInfoMode mode);

	// display a system info string
	void		  displaySystemInfo(const ucstring &str, const std::string &Category = "SYS");
	NLMISC::CRGBA getSystemInfoColor(const std::string &Category = "SYS");
	/**
	 * add an observer to a database entry
	 * \param observer : pointer on the observer
	 * \param id :  the thext id of the element to observe
	 * \return true if success
	 */
	bool addDBObserver (ICDBNode::IPropertyObserver* observer, ICDBNode::CTextId  id);

	/**
	 * add an observer to a database entry
	 * \param observer : pointer on the observer
	 * \param id :  the thext id of the element to observe
	 * \return true if success
	 */
	bool addDBObserver (ICDBNode::IPropertyObserver* observer, const std::string& id)
	{
		return addDBObserver(observer, ICDBNode::CTextId(id));
	}

	/** remove the observer from the dataBase
	 */
	bool removeDBObserver (ICDBNode::IPropertyObserver* observer, ICDBNode::CTextId  id);
	bool removeDBObserver (ICDBNode::IPropertyObserver* observer, const std::string& id)
	{
		return removeDBObserver(observer, ICDBNode::CTextId(id));
	}

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


	// Add a group into the windows list of its master goup
	void	makeWindow(CInterfaceGroup *group);

	// Remove a group from the windows list of its master group
	void    unMakeWindow(CInterfaceGroup *group, bool noWarning=false);


	// True if the keyboard is captured
	bool	isKeyboardCaptured() const {return _CaptureKeyboard!=NULL;}
	bool	isMouseOverWindow() const {return  _MouseOverWindow;}

	// Enable mouse Events to interface. if false, release Captures.
	void	enableMouseHandling(bool handle);
	bool    isMouseHandlingEnabled() const { return _MouseHandlingEnabled; }

	// register a view that wants to be notified at each frame (receive the msg 'clocktick')
	void	registerClockMsgTarget(CCtrlBase *vb);
	void	unregisterClockMsgTarget(CCtrlBase *vb);
	bool	isClockMsgTarget(CCtrlBase *vb) const;

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
	/// Get the lua state (NULL if not inited)
	class CLuaState		*getLuaState() const {return _LuaState;}
	/// Reload all LUA scripts inserted through <lua>
	void	reloadAllLuaFileScripts();
	/// for Debug: append some color TAG, to have a cool display in SystemInfo
	std::string	formatLuaErrorSysInfo(const std::string &error) {return std::string("@{FC8F}") + error;}
	/// for Debug: append/remove some color TAG, to have a cool display in SystemInfo/nlwarning
	void		formatLuaStackContext(std::string &stackContext);
	std::string formatLuaErrorNlWarn(const std::string &error);
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

	bool			localActionCounterSynchronizedWith(CCDBNodeLeaf *leaf)
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
			_AlphaRolloverSpeedDB = getDbProp("UI:SAVE:ALPHA_ROLLOVER_SPEED");
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
	const CEventDescriptorKey&	getLastEventKeyDesc() const { return _LastEventKeyDesc; }

	void	notifyMailAvailable();
	void	notifyForumUpdated();

	void updateTooltipCoords();

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
		void	onServerChange(ICDBNode *serverNode);
		// When something in the LOCAL DB changes
		void	onLocalChange(ICDBNode *localNode);

	private:
		class CLocalDBObserver : public ICDBNode::IPropertyObserver
		{
		public:
			CServerToLocalAutoCopy	&_Owner;
			CLocalDBObserver(CServerToLocalAutoCopy	&owner) : _Owner(owner) {}
			virtual void	update(ICDBNode *node)	{_Owner.onLocalChange(node);}
		};
		class CServerDBObserver : public ICDBNode::IPropertyObserver
		{
		public:
			CServerToLocalAutoCopy	&_Owner;
			CServerDBObserver(CServerToLocalAutoCopy	&owner) : _Owner(owner) {}
			virtual void	update(ICDBNode *node)	{_Owner.onServerChange(node);}
		};

		// A node here is a pair Server<->Local
		struct CNode
		{
			CCDBNodeLeaf	*ServerNode;
			CCDBNodeLeaf	*LocalNode;
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
		CCDBNodeLeaf			*_ServerCounter;
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

		void	buildRecursLocalLeaves(CCDBNodeBranch *branch, std::vector<CCDBNodeLeaf*> &leaves);
	};

	// Infos about a modal window.
	class CModalWndInfo
	{
	public:
		// Yoyo: store as CRefPtr in case they are deleted (can happen for instance if menu right click on a guild memeber, and guild members are udpated after)
		NLMISC::CRefPtr<CInterfaceGroup>		ModalWindow; // the current modal window
		NLMISC::CRefPtr<CCtrlBase>				CtrlLaunchingModal;
		bool				ModalClip;
		bool				ModalExitClickOut;
		bool				ModalExitClickL;
		bool				ModalExitClickR;
		bool				ModalExitKeyPushed;
		std::string			ModalHandlerClickOut;
		std::string			ModalClickOutParams;
	public:
		CModalWndInfo()
		{
			ModalWindow = NULL;
			CtrlLaunchingModal= NULL;
			ModalExitClickOut= false;
			ModalExitClickL= false;
			ModalExitClickR= false;
			ModalExitKeyPushed= false;
		}
	};


	// Database management stuff
	class CDBLandmarkObs : public ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(ICDBNode *node);
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
	CCDBNodeBranch *_DBB_UI_DUMMY;
	CCDBNodeLeaf *_DB_UI_DUMMY_QUANTITY;
	CCDBNodeLeaf *_DB_UI_DUMMY_QUALITY;
	CCDBNodeLeaf *_DB_UI_DUMMY_SHEET;
	CCDBNodeLeaf *_DB_UI_DUMMY_NAMEID;
	CCDBNodeLeaf *_DB_UI_DUMMY_ENCHANT;
	CCDBNodeLeaf *_DB_UI_DUMMY_SLOT_TYPE;
	CCDBNodeLeaf *_DB_UI_DUMMY_PHRASE;
	CCDBNodeLeaf *_DB_UI_DUMMY_WORNED;
	CCDBNodeLeaf *_DB_UI_DUMMY_PREREQUISIT_VALID;
	CCDBNodeLeaf *_DB_UI_DUMMY_FACTION_TYPE;

private:

	CCDBNodeLeaf *_CheckMailNode;
	CCDBNodeLeaf *_CheckForumNode;
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
	CInterfaceManager();

	///the singleton's instance
	static CInterfaceManager* _Instance;

	CCDBNodeLeaf	   *_DescTextTarget;

	// Capture
	NLMISC::CRefPtr<CCtrlBase>	_CaptureKeyboard;
	NLMISC::CRefPtr<CCtrlBase>	_OldCaptureKeyboard;
	NLMISC::CRefPtr<CCtrlBase>	_DefaultCaptureKeyboard;
	NLMISC::CRefPtr<CCtrlBase>	_CapturePointerLeft;
	NLMISC::CRefPtr<CCtrlBase>	_CapturePointerRight;
	bool		_MouseOverWindow;

	std::vector<CModalWndInfo> _ModalStack;
	static std::string	_CtrlLaunchingModalId;


	// view that should be notified from clock msg
	std::vector<CCtrlBase*> _ClockMsgTargets;

	// What is under pointer
	std::vector<CViewBase*>		_ViewsUnderPointer;
	std::vector<CCtrlBase*>		_CtrlsUnderPointer;
	std::vector<CInterfaceGroup *>	_GroupsUnderPointer;


	// Context Help
	bool					_ContextHelpActive;
	CCtrlBasePtr			_CurCtrlContextHelp;
	float					_DeltaTimeStopingContextHelp;
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

	/// Renderer
	CViewRenderer	_ViewRenderer;
	uint32			_ScreenW, _ScreenH; // Change res detection
	NLMISC::CRGBA	_GlobalColor;
	sint32			_LastInGameScreenW, _LastInGameScreenH; // Resolution used for last InGame interface

	// root node for interfaces properties in the databases
	CCDBNodeBranch *_DbRootNode;

	// List of active Anims
	std::vector<CInterfaceAnim*> _ActiveAnims;

	CInterfaceGroupPtr _WindowUnder;

	bool isControlInWindow (CCtrlBase *ctrl, CInterfaceGroup *pNewCurrentWnd);
	uint getDepth (CCtrlBase *ctrl, CInterfaceGroup *pNewCurrentWnd);

	void notifyElementCaptured(CCtrlBase *c);

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
	CCDBNodeLeaf *_NeutralColor;
	CCDBNodeLeaf *_WarningColor;
	CCDBNodeLeaf *_ErrorColor;
	CCDBNodeLeaf *_RProp;
	CCDBNodeLeaf *_GProp;
	CCDBNodeLeaf *_BProp;
	CCDBNodeLeaf *_AProp;
	CCDBNodeLeaf *_AlphaRolloverSpeedDB;

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
	CEventDescriptorKey	_LastEventKeyDesc;

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

};

#endif // NL_INTERFACE_MANAGER_H

/* End of interface_manager.h */
