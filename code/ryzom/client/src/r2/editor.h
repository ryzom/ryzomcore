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

#ifndef R2_EDITOR_H
#define R2_EDITOR_H

#include "nel/gui/lua_object.h"
#include "instance.h"
#include "tool.h"
#include "../decal.h"
#include "../decal_anim.h"
#include "entity_custom_select_box.h"
#include "island_collision.h"
#include "prim_render.h"
//
#include "nel/misc/singleton.h"
#include "nel/misc/class_registry.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/array_2d.h"
//
#include "dmc/dmc.h"
//
#include "game_share/object.h"
#include "game_share/scenario_entry_points.h"
#include "game_share/r2_types.h"




namespace NLGUI
{
	class CGroupTree;
}

class CEntityCL;

namespace NL3D
{
	class CPackedWorld;
}

namespace R2
{
class CDynamicMapService;
class CUserComponentsManager;
class CEntitySorter;

// texture for the default mouse cursor
extern const char *DEFAULT_CURSOR;


class CDisplayerVisualEntity;

/** Ryzom Ring in game editor
  *
  * ///////////////////////////////////////////////////////////
  * // TMP OVERVIEW OF R2 UI BEHAVIOR (WORK IN PROGRESS ...) //
  * ///////////////////////////////////////////////////////////
  *
  *
  * The ui can be in one of the following states :
  *
  * I CREATION MODE
  * ===============
  *
  * No tool is displayed as "highlighted"
  *
  * 1) Mouse in 3D view
  * --------------------
  *         The mouse cursor has a little star to indicate "creation mode".
  *         If creation is not possible at the mouse position, then there's is a little 'stop' icon shown
  *         No instance selection is done
  *
  *         EVENTS :
  *         --------
  *         LCLICK -> create
  *         RCLICK -> cancel & restore default tool
  *
  *         NB : we don't do creation on LDOWN or RUP in order to allow camera manipulation
  *
  *     2) Mouse over UI
  *     -----------------
  *         The mouse cursor still has a little star to indicate "creation mode".
  *         Ideally on some actions, the creation is canceled, and the default tool is backuped;
  *         NB nico : this is unclear to me atm, is it for button clicks only ?
  *                   (for example just moving a slider do not seem a good reason to cancel the tool...)
  *                   so no-op for now
  *         (TODO : exception for the minimap, if we want to be able to create by clicking on it ?)
  *
  * II CONTEXT MENU MODE
  * ====================
  *
  * The standard mouse cursor is displayed.
  *
  * EVENTS :
  * --------
  * LDOWN / RDOWN -> cancel the menu
  *
  * III CAPTURED BY UI MODE
  * =======================
  * ... same as 'context menu mode'
  *
  * IV TOOL MODE
  * =============
  * The current tool icon is highlighted in the toolbar
  *
  * std tool includes :
  * - Move
  * - Rotate
  * ...
  *
  *     1) Mouse in 3D view
  *         --------------------
  *         a ) Mouse over empty space in scene
  *         -----------------------------------
  *             The standard mouse cursor is displayed
  *
  *             EVENTS :
  *             --------
  *             nothing to do here, camera event are handled by the 'UserControls' global object
  *             If the right button is up without being preceded by a camera move, then the context
  *             menu is shown
  *
  *
  *         b ) Mouse over an instance in scene that is not the selection
  *         -------------------------------------------------------------
  *             The mouse with a little circle is displayed
  *
  *             EVENTS :
  *             --------
  *             RDOWN -> Test and see which one is best :
  *                      - a new slection is done and the context menu is shown ?
  *                      - nothing ? (e.g no selection and the context menu is trigerred by
  *                        the 'UserControls' global object as usual ?
  *             RDOWN while maintained action : cancel the current action and restore
  *                   state (same behaviour as most apps)
  *             LDOWN -> selection + tool dependent (instant action or maintained action)
  *                      a maintained action (such as moving an entity) will usually
  *                      capture the mouse.
  *             others -> tool dependent
  *
  *         c ) Mouse over an instance in scene that the selection
  *         -------------------------------------------------------------
  *             The mouse shows the action that can be performed
  *             RDOWN -> same as previous
  *             others -> tool dependent
  *             RDOWN while maintained action : cancel the current action and restore
  *                   state (same behaviour as most apps)
  *
  *          NB : a tool may want to capture the mouse, in this case it must call CTool::captureMouse
  *               and CTool::releaseMouse when it is done
  *
  *      REMARKS:
  *
  *        - Maybe it would be cool to make camera manipulation totally orthogonal to the system,
  *          and transparently layered on top of it.
  *          This would requires using something like the middle button or some shift / control combination
  *          to avoid conflict with the previous events ...
  *
  *     2) Mouse over UI
  *     -----------------
  *         The mouse is dipslayed as usual
  *         Events are taken in account by the UI, not by the tool
  *         This is transparent for CTool derivers, because mouse events won't reach them
  *         Clicking in the interface doesn't change the current tool
  *
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 5/2005
  */



class CEditor : public NLMISC::CSingleton<CEditor>
{
public:
	enum TMode { NotInitialized = 0, EditionMode, GoingToDMMode, GoingToEditionMode, TestMode, DMMode,
		AnimationModeLoading, AnimationModeWaitingForLoading, AnimationModeDm, AnimationModePlay,
		AnimationModeGoingToDm, AnimationModeGoingToPlay, ModeCount };

	enum TAccessMode { AccessEditor = 0, AccessDM, AccessOutlandOwner, AccessModeCount, AccessModeUnknown = AccessModeCount };

	////////////
	// OBJECT //
	////////////

	CEditor();
	~CEditor();

	// Init what's need to be initialized depending on the configuration
	void autoConfigInit(bool serverIsRingSession);

	// Release what's need to be released depending on the configuration
	void autoConfigRelease(bool serverIsRingSession);

	// initialisation of modules & gw
	void initModules(bool connectDMC);
	void releaseModules();

	// wait first scenario to be received (may be an empty one ...)
	void waitScenario();

	TAccessMode getAccessMode() const { return _AccessMode; }
	void setAccessMode(TAccessMode mode);

	// Initialisation of the editor
	void init(TMode initialMode, TAccessMode accessMode);

	// reload xml, lua & translation & reset. scenario is preserved
	void reset();
	// reload xml, lua & translation & reset. scenario is reseted
	void resetScenario();
	// reload xml, lua & translation & reset. Last save scenario is reloaded
	void reloadScenario();
	// load / reload the work language file
	void loadLanguageFile();
	//
	void release();
	// clear all content in the map, client side
	void clearContent();
	//
	bool isInitialized() const { return _Initialized; }
	// (re)load the ui if its date changed
	void reloadUI();

	// set current mode of the editor
	void	setMode(TMode mode);
	TMode	getMode() const { return _Mode; }

	// test if user is DMing (be it masterless or not)
	bool isDMing() const;

	/////////////////////
	// UPDATE & EVENTS //
	/////////////////////

	/** Events handling
	  * Handle an user input event
	  * \return true if the event was handled by the editor
	  */
	bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

	// Handle copy, reaches the editor if no edit box is currently active
	void copy();

	// Handle paste, reaches the editor if no edit box is currently active
	void paste();

	// An entity has been selected with the standard selection system (usually in test mode)
	void inGameSelection(const CLFECOMMON::TCLEntityId &slot);

	void onContinentChanged();


	/** update of editor just before precamera call for entities
	  */
	void updatePreCamera();
	/** update of editor (after update of entities just before render)
	  */
	void updateBeforeRender();
	/** Update of editor (just after render of the scene and entities update and before rendering of the interface)
	  * This is the place to display additionnal visual infos (slection bbox etc ...)
	  */
	void updateAfterRender();

	// called after main loop 'swap buffer'
	void updateBeforeSwapBuffer();

	/////////////////////////////////////////////
	// CLIENT OBJECTS (yet named 'instances' ) //
	/////////////////////////////////////////////

	// NB about this tmp mess
	// 'CInstance' are client counterparts of 'CObjectTable'

	// set current selected instance in the editor
	void           setSelectedInstance(CInstance *inst);
	void           forceSetSelectedInstance(CInstance *inst);
	// get current selected instance in the editor
	CInstance      *getSelectedInstance() const;
	// set current highlighted instance in the editor
	void           setFocusedInstance(CInstance *inst);
	// get current higlighted entity in the editor (with mouseoverit)
	CInstance      *getFocusedInstance() const;
	//
	bool           hasInstance(const CInstance *instance) const;
	// Check if there's an instance with the given id
	bool		   hasInstanceWithId(const TInstanceId &id) const;
	// Get an instance from its id
	CInstance      *getInstanceFromId(const TInstanceId &id) const;

	/** Create Visual Properties for an instance
	* Return false if can not retrieve VisualProperties
	*/
	bool getVisualPropertiesFromObject(CObject* instance, SPropVisualA& vA, SPropVisualB& vB, SPropVisualC& vC);

	/** Generate a new user readable name for an object in the scene
	  * If the name if already postfixed by a number, it will be stripped.
	  * Example : Npc 1, Npc 2, Group 1, Bandit camp 1 etc.
	  * Localised base name is given (for example "road" if using english)
	  * The function looks in all road instances and finds the first free number.
	  *
	  *
	  * \param baseClass Filter base class for the search : only class that derived from (or are of ) that class will
	  *                  be considered when looking for a new name
	  */
	ucstring		genInstanceName(const ucstring &baseName);

	// test if an instance name is post fixed by a number
	static bool		isPostFixedByNumber(const ucstring &baseName);

	/** test if a ucstring has the format of a generated name that is : baseName + " " + index
	  * Return index on success or -1 on failure
	  */
	static sint		getGeneratedNameIndex(const std::string &nameUtf8, const std::string &baseNameUtf8);

	/** Set a local user value (we name it  a "cookie") that should be attached to an instance at creation time
	  * (that is, when the object will be created on client -> when the server creation msg is received)
	  * The value will be stored in the 'User' lua table.
	  * The 'User' table is attached to each instances in the editor and is Read/Write
	  * (other properties in client and server and are read only, and should be modified by sending a requestxxx network message)
	  * This table is local only (that is, not seen by the server, nor by other clients editing the same map).
	  *
	  * Example of use :
	  *
	  * We want that when a road is created, a dialog pop to enter the name of the road.
	  *
	  * First we must be able to distinguish between roads created by ourselves and road created by
	  * other persons editing the map.
	  *
	  * Secondly we don't want that all roads to have this behaviour (for example, a road in a more complex system
	  * would be named automatically)
	  *
	  *
	  * In lua script it would look like :
	  *
	  * At creation request time :
	  * ==========================
	  *
	  * local road = r2.newComponent("Road")
	  * r2.setCookie(road.InstanceId, "AskName", true)
	  * r2.requestInsertNode(...     -> send newtork creation message for the road
	  *
	  *
	  * When creation message is received (may be placed in the 'onCreate' method of a displayer attached to that instance)
	  * ==================================================================================================================
	  *
	  * function roadDisplayer:onCreate(road)
	  *		if road.User.AskName == true then
	  *			-- road was created by this client
	  *         -- dialog to enter the name was asked
	  *         -- => popup the dialog
	  *         ....
	  *     end
	  * end
	  *
	  * Other example : when an act is created by the user it becomes the current act
	  *
	  */
	void setCookie(const TInstanceId &instanceId, const std::string &key, CLuaObject &value);
	// helpers to add cookie of predefined type (to be completed ..)
	void setCookie(const TInstanceId &instanceId, const std::string &key, bool value);


	struct IInstanceObserver : public NLMISC::CRefCount // refptr'ed observers
	{
		virtual ~IInstanceObserver() {}

		typedef NLMISC::CRefPtr<IInstanceObserver> TRefPtr;
		// called when the watched instance has been created
		virtual void onInstanceCreated(CInstance &/* instance */) {}
		virtual void onInstanceErased(CInstance &/* instance */) {}
		virtual void onInstancePreHrcMove(CInstance &/* instance */) {}
		virtual void onInstancePostHrcMove(CInstance &/* instance */) {}
		virtual void onPreHrcMove(CInstance &/* instance */) {}
		virtual void onPostHrcMove(CInstance &/* instance */) {}
		virtual void onInstanceEraseRequest(CInstance &/* instance */) {}
		virtual void onAttrModified(CInstance &/* instance */, const std::string &/* attrName */, sint32 /* attrIndex */) {}
	};

	typedef sint TInstanceObserverHandle;

	enum { BadInstanceObserverHandle = -1 };

	/** add a watch to know when an instance is created (the instance is identified by its instance id)
	  * returns a handle for removal, or 'BadInstanceObserverHandle' if creation failed
	  */
	TInstanceObserverHandle addInstanceObserver(const TInstanceId &instanceId, IInstanceObserver *observer);
	// Get a pointer to an observer from its handle (or NULL if not found)
	IInstanceObserver *getInstanceObserver(TInstanceObserverHandle handle);
	// Remove an instance observer from its handle (but do not delete it). Return the pointer to the observer
	IInstanceObserver *removeInstanceObserver(TInstanceObserverHandle handle);
	/** Test from a pointer if the object is currently observing an instance
	  * NB : slow because of linear search
	  */
	bool			  isInstanceObserver(IInstanceObserver *observer) const;



	////////////////////////
	// LUA R2 ENVIRONMENT //
	////////////////////////

	// get table for global variables in lua environment (equivallent to _G lua variable)
	CLuaObject       &getGlobals()        { return _Globals; }
	// get table for registry in lua environment
	CLuaObject       &getRegistry()       { return _Registry; }
	// get lua classes (the r2.Classes table)
	CLuaObject		 getClasses() throw(ELuaError);
	// get R2 environment (the 'r2' table into lua global environment)
	CLuaObject       &getEnv();
	// get the config table (that is the 'r2.Config' table)
	CLuaObject		 getConfig();
	// get a reference to the lua state object
	CLuaState		 &getLua();
	/** Project a CObjectTable into lua (accessor is pushed onto the lua stack)
	  * property of the table, which is a C++ object, will be accessible from lua (by using metatable)
	  */
	void			 projectInLua(const CObjectTable *table);

	// get the default feature for the current selected act
	CInstance		 *getDefaultFeature();
	// get the default feature for the given act
	CInstance		 *getDefaultFeature(CInstance *act);

	// set the current act (NULL being synonimous with the base act)
	void			 setCurrentAct(CInstance *act);
	void			 setCurrentActFromTitle(const std::string &title);
	CInstance		 *getCurrentAct() const { return _CurrentAct; }
	CInstance		 *getBaseAct() const { return _BaseAct; }

	/** helper :  calls a function in the lua r2ed environment
      * Arguments must have already been pushed on the stack
	  * If the call fails then an error msg is printed in the log, and no arguments are returned
	  *
	  * \param funcName name of the function to call (must resides in the r2ed table)
	  * \param numArgs, numbers of arguments that the functions will receive
	  * \param numRet : Number of parameters that the function returns
	  *                 As usual the stack will be adjusted to that size after the call
	  */
	bool              callEnvFunc(const char *funcName, int numArgs, int numRet = 0);
	// Behave like 'callEnvFunc', but call a method instead (r2 is passed as the 'self' parameter, that is)
	bool              callEnvMethod(const char *funcName, int numArgs, int numRet = 0);

	//////////////////////
	// SERVER COMMANDS  //
	//////////////////////

	// access to interface with server
	CDynamicMapClient &getDMC() const { nlassert(_DMC); return *_DMC; }

	/** Set a property of an object locally. No network msg is sent, but modification events are triggered to signal that
	  * the object has been modified (rooted to one of the CDisplayerBase derived object, attached to the object being modified. see displayer_base.h).
	  * Changes to local value must be commited or cancelled when edition is finish by calling 'requestCommitLocalNode' or
	  * 'requestRollbackLocalNode'
	  * Typical use is by the slider widget : real value is sent accross the network only when the user release the mouse, but
	  * local update of object property related to the slider is done each time the slider is modified before release.
	  * During the maintained action, no network message are desireable.
	  * NB : a copy of input parameter 'value' will be done. Caller is responsible for deleting 'value' after use.
	  */
	void requestSetLocalNode(const std::string& instanceId, const std::string& attrName, const CObject *value);
	void requestCommitLocalNode(const std::string& instanceId, const std::string& attrName);
	void requestRollbackLocalNode(const std::string& instanceId, const std::string& attrName);

	/////////////
	// DECALS  //
	/////////////
	void	showHighlightDecal(const NLMISC::CVector &pos, float scale);
	void	showSelectDecal(const NLMISC::CVector &pos, float scale);
	void	addSelectingDecal(const NLMISC::CVector &pos, float scale);
	void	showSelectBox(const NLMISC::CAABBox &localBox, const NLMISC::CMatrix &worldMat);
	void	showHighlightBox(const NLMISC::CAABBox &localBox, const NLMISC::CMatrix &worldMat);


	/////////////////
	// COLLISIONS  //
	/////////////////

	CIslandCollision &getIslandCollision() { return _IslandCollision; }


	///////////
	// MISC  //
	///////////

	// Shortcut to the GUI
	static CInterfaceManager &getUI();
	//
	/** Set current edition tool. NULL will reset to the default tool (selection tool)
	  */
	void   setCurrentTool(CTool *tool);
	// Get current tool for edition
	CTool *getCurrentTool() const { return _CurrentTool; }
	/** Helper to execute a lua script
	  * \param filename name of the lua script file
	  * \param fileDescText short description of the script function (for error messages)
	  * \return true on success
	  */
	bool doLuaScript(const char *filename, const char *fileDescText);
	// helper : create an entity in scene at the given slot & position, replacing any previous entity in that slot

	static CEntityCL *createEntity(uint slot, const NLMISC::CSheetId &sheetId, const NLMISC::CVector &pos, float heading, const std::string &	permanentStatutIcon=std::string(""));
	// helper : get an instance from a CEntityCL pointer
	CInstance *getInstanceFromEntity(CEntityCL *entity) const;
	// helper : clear content of the debug window
	void clearDebugWindow();
	// display the editor contextual menu
	void displayContextMenu();
	// tmp, for debug
	void dumpInstances();
	// helper : return the entity under the mouse
	CInstance *getInstanceUnderPos(float x, float y, float distSelection, bool &isPlayerUnderCursor);
	// helper : test intersection between an entity and a ray
	static float preciseEntityIntersectionTest(CEntityCL &entity, const NLMISC::CVector &worldRayStart, const NLMISC::CVector &worldRayDir);
	// Tmp show the connexion window and display a msg in it. An empty msg will close the window
	static void connexionMsg(const std::string &stringId);
	TEntityCustomSelectBoxMap &getEntityCustomSelectBoxMap() { return _EntityCustomSelectBoxMap; }
	// from a pointer on an entity, retrieve its local selection bbox (possibly redefined in r2_ui_custom_boxes_data.lua
	const NLMISC::CAABBox &getLocalSelectBox(CEntityCL &entity) const;
	// from a pointer on an entity, retrieve its local selection bbox (possibly redefined in r2_ui_custom_boxes_data.lua
	NLMISC::CAABBox getSelectBox(CEntityCL &entity) const;
	// check if there's room left to create new objects in the scene
	sint		   getLeftQuota();


	bool		   checkRoomLeft();
	// display an error msg to prompt the user to make room for new objects in its scenario
	void		   makeRoomMsg();
	// check if there is room in specific category if not display an error msg
	// verify ther is at least size object in category StaticObject or AiObject
	bool	verifyRoomLeft(uint aiCost, uint staticCost);

	// rename delete auto_save9, rename auto_save1.r2 to auto_save2.r2 and so until auto_save8.r2, copy auto_save.r2 to auto_save1.r2,
	void autoSave();

	/** Season driven from editor
      * This value is usually 'Unknwown' unless the mode is "edit'.
	  * In this case, the value depends on the act being edited
	  */
	enum TSeason { Automatic = 0, Spring, Summer, Autumn, Winter, UnknownSeason };
	TSeason getSeason() const;
	void	tpReceived();

	bool    getFixedLighting() const { return _FixedLighting; }
	void    setFixedLighting(bool enabled);

	/** Get current infos of a plot item plot display in the editor
	  * Plot items are items with SCROLL_R2 as family. Each sheet can be used only once in
	  * a scenario (meaning that each icon can only be seen for a single plot item).
	  * The name of a plot item can also be change by using the lua command r2:setPlotItemName(sheetId, ucName)
	  */
	void setPlotItemInfos(const TMissionItem &infos);
	const TMissionItem *getPlotItemInfos(uint32 sheetId) const;


	// convert name of a class in R2 to a unique index (-1 if not found)
	sint	classToIndex(const std::string &className) const;
	// this if one class id 'indexOf' another from their index
	bool	isKindOf(sint testedClassIndex, sint kindClassIndex) const;
	// from one class index, returns the index to the derived class (-1 if not found)
	sint	getBaseClass(sint derivedClass) const;


	///////////////////////////////
	// > 254 entities management //
	///////////////////////////////

	CEntitySorter *getEntitySorter() const;

private:

	CEntitySorter *_EntitySorter;


	// mapping from sheet id to plot item name
	std::map<uint32, TMissionItem>					_PlotItemInfos;
	CIslandCollision							_IslandCollision;

	//
	bool										_SerializeUIConfig;
	TMode										_Mode;
	TAccessMode									_AccessMode;

	typedef std::map<const CObjectTable *, CInstance::TSmartPtr> TInstanceMap;
private:

	CLuaObject					                _Globals; // match to the '_G' lua global variable
	CLuaObject					                _Registry;
	CLuaObject					                _Env;
	CLuaObject									_Config;
	CLuaObject									_ObjectProjectionMetatable;
	CLuaObject									_LuaUIMainLoop;
	//
	TInstanceMap                                _Instances;
	CInstance                                   *_SelectedInstance;
	CInstance                                   *_FocusedInstance;

	CHashMap<std::string, uint>		        	 _ClassNameToIndex; // Map each class name to an unique index (filled at init)
	NLMISC::CArray2D<uint8>						 _KindOfTable;      // Table to test if one class derives from another (filled at init)
	std::vector<sint>							 _BaseClassIndices; // for each class, give index of thebase class (vector ordered by classes indices)

	typedef CHashMultiMap<ucstring, CInstance *, NLMISC::CUCStringHashMapTraits> TInstanceByName;

	bool										_MaxVisibleEntityExceededFlag;

	// instance sorted by name
	class CSortedInstances
	{
	public:
		void insert(const ucstring &name, CInstance *inst);
		void remove(CInstance *inst);
		bool contains(CInstance *inst) const;
		TInstanceByName::iterator begin() { return _ByName.begin(); }
		TInstanceByName::iterator end() { return _ByName.end(); }
	private:

		typedef std::map<CInstance *, TInstanceByName::iterator> TInstanceToIter;
		//
		TInstanceByName _ByName;
		TInstanceToIter _InstanceToIter;
	};
//	typedef  TInstanceByDispName; // for usage by CInstance
	// list of instances for each classes (ordered by class index)
	std::vector<CSortedInstances>		     _InstancesByDispName; // instances sorted by their display name (private use)

	//priv for debug
	bool isRegisteredByDispName(CInstance *inst) const;

	// Cookies (local objects attached to instances at creation time, see setCookie)
	struct CCookie
	{
		std::string Key;
		CLuaObject  Value;
	};
	typedef std::list<CCookie>   TCookieList; // for a single instance, map each key to its value
	typedef std::map<TInstanceId, TCookieList>  TCookieMap;
	TCookieMap									_Cookies;
	//
	CInstance::TRefPtr							_CurrentAct;
	CInstance::TRefPtr							_BaseAct;
	CInstance::TRefPtr							_ScenarioInstance;
	CObjectTable								*_Scenario;
	std::string									_WantedActOnInit;
	//
	friend class CDynamicMapClientEventForwarder;
	friend class CAHR2QuitConnectingScreen;
	CDynamicMapClient							*_DMC; // replication of server map state on that client
	CDynamicMapService							*_DMS; // the server (hosted in local for now)
	CInstance::TRefPtr							_LastInstanceUnderPos;
	//
	CTool::TSmartPtr							_CurrentTool;
	static bool									_ReloadWanted;
	//
	CDecal										_HighlightDecal;
	CDecalAnim									_HighlightDecalAnim;
	CDecal										_SelectDecal;
	CDecalAnim									_SelectDecalAnim;
	CDecalAnim									_SelectingDecalAnim;
	CDecal										_PionneerDecal;
	// alternative selection for huge element like particle systems, display a box on ground rather than
	// the selection circle
	CPrimRender									_SelectBox;
	CPrimRender									_HighlightBox;
	//
	NLMISC::TTime								_LastAutoSaveTime;
	CDecalAnim									_PionneerDecalAnim;
	struct CSelectingDecal : public NLMISC::CRefCount
	{
		CDecal Decal;
		sint64 EndDate;
		NLMISC::CVector Pos;
		float Scale;
	};
	std::vector<NLMISC::CSmartPtr<CSelectingDecal> > _SelectingDecals;
	sint64											 _DecalRefTime; // reference time for "decals" animation
	//
	bool										_EnteredInSetSelectedInstance; // prevent recursive call from CEditor::setSelectedInstance
	bool										_Initialized;
	bool										_ForceDesktopReset[4];
	// cache to avoid to reparse the ui : last modification date of ui files
	std::map<std::string, uint32>				_LastUIModificationDate;
	TEntityCustomSelectBoxMap					_EntityCustomSelectBoxMap;
	/** system for local generation of name : for each kind of name, gives the locally allocated ids
	  * -> Used to generate name for instances, taking in account instance that have not been added to the scene yet.
	  * (that is, requestSetNode message has been sent, but server has not added object to the scenario yet)
	  */
	typedef std::set<uint> TNameSet;
	typedef std::map<std::string, TNameSet> TGeneratedNameMap;
	TGeneratedNameMap  _LocalGeneratedNames;

	// instance observers

	typedef std::multimap<TInstanceId, IInstanceObserver::TRefPtr> TInstanceObserverMap;
	TInstanceObserverMap										   _InstanceObservers;
	typedef std::map<TInstanceObserverHandle, TInstanceObserverMap::iterator> TInstanceObserverHandleMap;
	TInstanceObserverHandleMap										   _InstanceObserverHandles;			// map each observer handle into an entry in the map
	TInstanceObserverHandle											   _InstanceObserverHandleCounter;		// current handle to generate when adding a new observer


	TSeason										_Season;
	bool										_FixedLighting;
	bool										_IsWaitingTPForSeasonChange;
	bool										_UpdatingScenario;
	bool										_WillTP; // true if a teleport should be expected after the scenario has been updated
														 // in this case, first season change is ignored, because it is done during the teleport

	bool										_ScenarioReceivedFlag; // for the wait screen ...
	bool										_TPReceivedFlag; // for the wait screen ...
	bool										_WaitScenarioScreenWanted;	// lua requests that we display the 'wait scenario' screen
	bool										_WaitScenarioScreenActive;	// the 'wait scenario' screen is being displayed
	bool										_EditionModeDisconnectedFlag;
	CObject										*_NewScenario; // new scenario that will be updated just after the wait screen is over
	uint32										_NewScenarioInitialAct;// the start at which the user start an edition session (can be ~= from 1 after a test session)
	bool										_PostponeScenarioUpdated;

	// Contextual selection
	//
	// We keep track of the last selected 'logic entity' (npc most of the time),
	// and the last list of primitive through which its current activity sequence goes
	// This way we can handle the following scenario :
	// - create a npc
	// - make him wander in zone A then zone B
	// - select contextual visibility for primitive
	// - click on npc : both zone shows
	// - click on zone A : zone B dissapear -> strange
	// By keeping the last list of contextual primitive, we ensure that contextual selection
	// remains visible when one click on one element in the currently displayed sequence
	CInstance::TRefPtr				_LastContextualLogicEntity;
	std::vector<CInstance::TRefPtr> _LastContextualPrims;




private:

	/////////////////////////////
	// NETWORK EVENTS HANDLING //
	/////////////////////////////

	void nodeErased(const std::string& instanceId, const std::string& attrName, sint32 position);

	void nodeInserted(const std::string& instanceId, const std::string& attrName, sint32 position,
					  const std::string& key, CObject* value);

	virtual void nodeSet(const std::string& instanceId, const std::string& attrName, CObject* value);


	void nodeMoved(const std::string& instanceId, const std::string& attrName, sint32 position,
				   const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);

	void scenarioUpdated(CObject* highLevel, bool willTP, uint32 startingActIndex);

	// send the needed events to tell that the attr at 'attrName' inside 'parentInstance' (possibly at position (indexInArray') has been modified
	void onAttrModified(CInstance &parentInstance, const std::string &attrName, sint32 indexInArray = -1);

	// send the needed events to tell that an object has been modified (& propagate to parents)
	void onAttrModified(const CObject *value);


	void onResetEditionMode();
	void onEditionModeConnected( uint32 userSlotId, uint32 adventureId, CObject* highLevel, const std::string& versionName, bool willTP, uint32 initialActIndex);
	void onAnimationModeConnected(const CClientMessageAdventureUserConnection& connected);
	void onEditionModeDisconnected();
	void onTestModeConnected();
	// deconnect from test or play
	void onTestModeDisconnected(TSessionId sessionId, uint32 lasAct, TScenarioSessionType sessionType);


	///////////////////////////////////////////////////////
	// EDITOR OBJECTS (instances) / CObjectTable mapping //
	///////////////////////////////////////////////////////

public:
	// Get a CObjectTable from its id
	const CObjectTable  *getObjectTableFromId(const TInstanceId &id) const;
private:
	// Erase the current Scenario (and block outgoing message)
	void eraseScenario();
	void onErase(CObject *object);
	void onErase(CObject *object, bool &foundInBase, std::string &nameInParent);
	/** Create a new CInstance for the given CObject
	  * - Displayer are attached
	  * - Object is inserted in the object map
	  * - 'onCreate' msg is sent
	  * 'obj' must be a table or it fails
	  */
	void createNewInstanceForObjectTable(const CObject *obj);
	void createNewInstanceForObjectTableInternal(const CObject *obj);
	void onPostCreate(const CObject *obj);

	void waitScenarioScreen();

public:
	/** private : retrieve lua 'User' table attached to an object (a read / write table)
	  * The table is pushed on stack
	  */
	static void getLuaUserTableFromObject(CLuaState &ls, const CObjectTable &table);
private:

	//////////////////////////////////////
	// EDITOR FUNCTIONS EXPORTED TO LUA //
	//////////////////////////////////////
	static int luaGetVisualPropertiesFromInstanceId(CLuaState &ls);
	static int luaGetSelectedInstanceId(CLuaState &ls);
	static int luaGetSelectedInstance(CLuaState &ls);
	static int luaSetSelectedInstanceId(CLuaState &ls);
	static int luaSetCurrentTool(CLuaState &ls);
	static int luaGetCurrentTool(CLuaState &ls);
	static int luaGetInstanceFromId(CLuaState &ls);
	static int luaDisplayContextMenu(CLuaState &ls);
	static int luaConnectAsCreator(CLuaState &ls);
	static int luaDofile(CLuaState &ls); // equivalent of the 'dofile' lua function, but with more info output
	static int luaTryFile(CLuaState &ls); // same as try file, but do not throw an exception on error, just return an error message and return false
	static int luaSetEntityCustomSelectBox(CLuaState &ls);
	static int luaGetEntityCustomSelectBox(CLuaState &ls);
	static int luaChoosePos(CLuaState &ls);
	static int luaSnapPosToGround(CLuaState &ls); // takes x, y, z as parameters and return the snapped position
	static int luaGetUserEntityPosition(CLuaState &ls);
	static int luaGetUserEntityFront(CLuaState &ls);
	static int luaRequestSetLocalNode(CLuaState &ls);
	static int luaRequestCommitLocalNode(CLuaState &ls);
	static int luaRequestRollbackLocalNode(CLuaState &ls);
	static int luaSetCurrentActFromId(CLuaState &ls);
	static int luaGetCurrentAct(CLuaState &ls);
	static int luaAddInstanceObserver(CLuaState &ls); // param 1 = instance id of the instance to observe
													  // param 2 = table of an object that will receive notifications. The table should contain methods
													  // with the same names than those found in 'IInstanceObserver' (plus parameters  are the same)
													  // the method returns a handle for future deletion
	static int luaRemoveInstanceObserver(CLuaState &ls); // remove an observer that was previously added by a 'addInstanceObserver'
														 // param 1 = the handle returned by 'addInstanceObserver' at the registration time
														 // returns a reference to the observer that was registered
	static int luaGenInstanceName(CLuaState &ls);	// calls CEditor::genInstanceName, same parameters
													// NB : return has type 'ucstring', so may need to call :toUtf8() in the lua script
	static int luaIsPostFixedByNumber(CLuaState &ls);
	static int luaIsClearingContent(CLuaState &ls);
	static int luaSetCookie(CLuaState &ls);			// same than CEditor::setCookie
	static int luaIsCreature(CLuaState &ls);
	static int luaSetEditorSeason(CLuaState &ls);  // set the weather to display when editing
	static int luaSetFixedLighting(CLuaState &ls);
	static int luaGetFixedLighting(CLuaState &ls);
	static int luaSetPlotItemInfos(CLuaState &ls);
	static int luaIsCurrentSelectionPlayer(CLuaState &ls);
	static int luaFindEmptyPlace(CLuaState &ls);
	static int luaIsInIslandRect(CLuaState &ls); // test if pos is in the current island (not necessarily a valid pos, but inside the island rect)
												 // takes x, y as entry, returns true on success

	static int luaGetCurrentIslandName(CLuaState &ls);

	static int luaWaitScenarioScreen(CLuaState &ls); // display the wait screen after the scenario creation has been launched
	static int luaIsScenarioUpdating(CLuaState &ls);

	// undo / redo possible ?
	static int luaCanUndo(CLuaState &ls);
	static int luaCanRedo(CLuaState &ls);
	// return the name of the editer
	static sint luaGetUserEntityName(CLuaState &ls);

	static int luaGetStartingAnimationFilename(CLuaState &ls);

	static int luaKickCharacter(CLuaState &ls);
	static int luaUnkickCharacter(CLuaState &ls);
	static int luaTeleportToCharacter(CLuaState &ls);
	static int luaEnumInstances(CLuaState &ls);


	void connect();

	// remove all object from the entity manager (but the player)
	static void removeAllEntitySlots();


	////////////////
	// PLOT ITEMS //
	////////////////

public:
	static uint		   getMaxNumPlotItems();
	static NLMISC::CCDBNodeLeaf	   *getPlotItemSheetDBLeaf(uint index);
	static bool				getIsStartingScenario() { return _IsStartingScenario; }
	bool				isClearingContent() const { return _ClearingContent; }

private:
	void				initPlotItems();
	void			   initReferencePlotItems();
	static void		   initDummyPlotItems();
	void				resetPlotItems();
	static void		   setReferencePlotItemSheet(uint index, uint32 sheetId);
	static NLMISC::CCDBNodeLeaf	   *getRefPlotItemSheetDBLeaf(uint index);

	//////////
	// MISC //
	//////////
	void			   setMaxVisibleEntityExceededFlag(bool on);
	void			   backupRequestCommands();
	void			   restoreRequestCommands();
	void			   setForceDesktopReset(bool force);
	void			   setUIMode(uint8 mode);
	bool			   loadUIConfig(const std::string &prefix);
	void			   loadStandardUI();
	void			   saveUIConfig();
	void			   hideRingWindows();
	std::string		   getUIPrefix(TMode mode) const;
	void			   loadKeySet(const std::string &keySet);
	static std::string getKeySetPrefix(TMode mode);
	void			   saveCurrentKeySet();
	void			   reloadUI(const char *filename);
	void			   initHighlightDecal();
	void			   updateDecalBlendRegion(CDecal &decal, const NLMISC::CVector &pos);
	void               initPalette();
	void			   initObjectProjectionMetatable();
	void               registerDisplayers();
	void               registerTools();
	void			   registerLuaFunc();
	// add a C++ method in the environement
	void			   registerEnvMethod(const char *name, TLuaWrappedFunction func);
	void			   registerEnvFunction(const char *name, TLuaWrappedFunction func);
	// Initialisation of contextual cursor.
	void			   initDecals();
	void			   showDecal(const NLMISC::CVector2f &pos, float scale, CDecal &decal, const CDecalAnim &decalAnim);
	void			   updatePrimitiveContextualVisibility();
	void			   initClassInheritanceTable();
	// contextual mouse handling
	static void checkCursor();
	// Forward click from contextual cursor to current tool
	static void mouseClick(bool rightButton, bool dblClick);
	// callback to reload the editor when one of the config files changed
	static void reloadEditorCallback(const std::string &filename);
	// update the display of decals created when the player select an instance in the scene
	void updateSelectingDecals();
	// display of highlight or select box (for selection of huge objects)
	// the CDecalAnim is used to mimic the color cycle seen when standard selection circle is displayed
	// (no CPrimRenderAnim for now)
	void showPrimRender(CPrimRender &dest, const NLMISC::CAABBox &localBox, const NLMISC::CMatrix &worldMat, const CDecalAnim &refDecalAnim);

	CLuaObject _OldLuaRequestInsertNode;
	CLuaObject _OldLuaRequestInsertGhostNode;
	CLuaObject _OldLuaRequestSetNode;
	CLuaObject _OldLuaRequestEraseNode;
	CLuaObject _OldLuaRequestMoveNode;

	bool _ClearingContent;

	std::string _ConnexionMsg;
	static std::string _ScenarioToLoadWhenEntreringIntoAnimation;
	static bool _IsStartingScenario; // the scenario is an animation scenario launch from the ring access point

	//bool _ModeEnabled[ModeCount];

public:
	// private method
	CInstance *getInstanceFromObject(const CObject *obj) const; // nb : only table have associated instance
	void connectAsCreator();
	// notify obervers of an instance that it has been created
	void notifyInstanceObserversOfCreation(CInstance &inst);

	// TMP for debug : dump missing collisions in the log
	void checkMissingCollisions();

	// trigger an instance observers for the given instance id. A copy of the observer list is made, thus
	// allowing for safe removal of observer when they are triggered
	struct IObserverAction
	{
		virtual ~IObserverAction() { }
		virtual void doAction(IInstanceObserver &obs) = 0;
	};
	void				triggerInstanceObserver(const TInstanceId &id, IObserverAction &action);

	static void	setStartingAnimationFilename(const std::string& filename);

	// for CInstance usage : allows to keep a list of instances sorted by their display name
	void registerInstanceDispName(const ucstring &displayName, CInstance *inst);
	void unregisterInstanceDispName(CInstance *inst);

};

// shortcut function to get the editor
inline CEditor &getEditor() { return CEditor::getInstance(); }

// test whether editor is currently enabled and is in EDITION mode. (In this mode, selection is managed by the editor rather than by the entity manager)
bool isEditionCurrent();

/** helper to create a class from the registry
  * \TODO a true factory class!
  * \TODO or put this in a better place (NLMISC ? )
  */
template <class T>
T *createObjectFromClassName(const std::string &className)
{
	if (className.empty()) return NULL;
	try
	{
		NLMISC::IClassable *obj = NLMISC::CClassRegistry::create(className);
		if (!obj)
		{
			nlwarning("Couldn't create object of class %s", className.c_str());
			return NULL;
		}
		T *inst = dynamic_cast<T *>(obj);
		if (!inst)
		{
			nlwarning("<R2::createObjectFromClassName> class %s found in the registry, but does not match the expected class.",
						obj->getClassName().c_str());
			delete obj;
			return NULL;
		}
		return inst;
	}
	catch(const NLMISC::ERegistry &)
	{
		return NULL;
	}
}

extern bool ResetWanted;
extern bool ResetScenarioWanted;
extern bool ReloadScenarioWanted;
extern bool ConnectionWanted;
//



// helper : get a NLMISC::CVector from a DMC::CObject
NLMISC::CVector getVector(const CObject *obj);
NLMISC::CVectorD getVectorD(const CObject *obj);
//helper : build a CObject from a NLMISC::CVectorD
CObject *buildVector(const NLMISC::CVectorD &v, const std::string &instanceId = "");



// DMC helpers:
std::string getString(const CObject *obj, const std::string &attrName);
double		getNumber(const CObject *obj, const std::string &attrName);
const CObject		*getObject(const CObject *obj, const std::string &attrName);

/** Helper : read an enum from a lua string, printing necessary error if no match is found.
  * nil found -> no op
  */
template <class T>
void enumFromLua(const CLuaObject &value,
				 std::pair<std::string, T> *enumTable,
				 uint numEnum,
				 T &dest,
				 const std::string &errorMsgPrefix
				)
{
	if (value.isNil())
	{
		return;
	}
	if (!value.isString())
	{
		nlwarning("%s : String expected when reading '%s'", errorMsgPrefix.c_str(), value.getId().c_str());
		return;
	}
	for(uint k = 0; k < numEnum; ++k)
	{
		if (value.toString() == enumTable[k].first)
		{
			dest = enumTable[k].second;
			return;
		}
	}
	nlwarning("%s : Unknown enum %s read from object %s", errorMsgPrefix.c_str(),
			  value.toString().c_str(),
			  value.getId().c_str());
}


} // R2

bool IsInRingMode();


#endif
