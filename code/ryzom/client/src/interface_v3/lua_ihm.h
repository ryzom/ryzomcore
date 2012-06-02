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

#ifndef NL_LUA_IHM_H
#define NL_LUA_IHM_H

#include "nel/misc/types_nl.h"
#include "lua_helper.h"



namespace NLMISC
{
	class CPolygon2D;
	class CVector2f;
}

class CReflectable;
class CReflectedProperty;

// ***************************************************************************
/* 	Use this Exception for all LUA Error (eg: scripted passes bad number of paramters).
 *	Does not herit from Exception because avoid nlinfo,    because sent twice (catch then resent)
 *	This is special to lua and IHM since it works with CLuaStackChecker,    and also append to the error msg
 *	the FileName/LineNumber
 */
class ELuaIHMException : public ELuaWrappedFunctionException
{
private:
	static CLuaState *getLuaState();
public:
	ELuaIHMException() : ELuaWrappedFunctionException(getLuaState())
	{
	}
	ELuaIHMException(const std::string &reason) :   ELuaWrappedFunctionException(getLuaState(),    reason)
	{
	}
	ELuaIHMException(const char *format,    ...) : ELuaWrappedFunctionException(getLuaState())
	{
		std::string	reason;
		NLMISC_CONVERT_VARGS (reason,    format,    NLMISC::MaxCStringSize);
		init(getLuaState(),    reason);
	}
};


// ***************************************************************************
#define IHM_LUA_METATABLE		"__ui_metatable"
#define IHM_LUA_ENVTABLE		"__ui_envtable"


// ***************************************************************************
/**
 * Define Functions to export from C to LUA
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CLuaIHM
{
public:
	static void	registerAll(CLuaState &ls);

	// CInterfaceElement management on stack, stored by a CRefPtr.
	static void pushUIOnStack(CLuaState &ls, class CInterfaceElement *pIE);
	static bool	isUIOnStack(CLuaState &ls, sint index);
	static CInterfaceElement	*getUIOnStack(CLuaState &ls, sint index);

	/** CReflectableInterfaceElement management on stack, stored by a CRefPtr.
	  * May be called as well for ui element, because they derive from CReflectableRefPtrTarget
	  */
	static void pushReflectableOnStack(CLuaState &ls, class CReflectableRefPtrTarget *pRPT);
	static bool	isReflectableOnStack(CLuaState &ls, sint index);
	static CReflectableRefPtrTarget	*getReflectableOnStack(CLuaState &ls, sint index);


	// ucstring
	static bool pop(CLuaState &ls, ucstring &dest);
	static void push(CLuaState &ls, const ucstring &value);
	static bool	isUCStringOnStack(CLuaState &ls, sint index);
	static bool getUCStringOnStack(CLuaState &ls, sint index, ucstring &dest);


	// RGBA
	static bool pop(CLuaState &ls, NLMISC::CRGBA &dest);

	// CVector2f
	static bool pop(CLuaState &ls, NLMISC::CVector2f &dest);

	// helper : get a 2D poly (a table of cvector2f) from a lua table (throw on fail)
	static void getPoly2DOnStack(CLuaState &ls, sint index, NLMISC::CPolygon2D &dest);

	// Print a message in the log.
	// Lua messages must be enabled (with ClientCfg.DisplayLuaDebugInfo = 1)
	// Additionnally, if ClientCfg.LuaDebugInfoGotoButtonEnabled is set, then
	// a button will be created near the line to allow to goto the lua line that issued the message
	// by using an external editor
	static void		debugInfo(const std::string &dbg);
	// Print a message in the log
	// No 'goto file' button is created
	// Lua messages must be enabled (with ClientCfg.DisplayLuaDebugInfo = 1)
	static void		rawDebugInfo(const std::string &dbg);
	// Dump callstack in the console
	// Additionnally, if ClientCfg.LuaDebugInfoGotoButtonEnabled is set, then
	// buttons will be created in fonr of eahc line to allow to goto the lua line that issued the message
	// by using an external editor
	static void		dumpCallStack(int startStackLevel = 0);
	static void		getCallStackAsString(int startStackLevel, std::string &result);

	// Create a special tag that will add a 'goto' button for the given file and line
	// The tag should be appended in front of a string to use with 'rawDebugInfo'.
	// when the final string will be printed, a button will be created in front of it
	// Requires that 'ClientCfg.LuaDebugInfoGotoButtonEnabled' is set to 1, else
	// a, empty tag is returned
	static std::string	createGotoFileButtonTag(const char *fileName, uint line);

	// argument checkin helpers
	static void	checkArgCount(CLuaState &ls, const char* funcName, uint nArgs);		// check that number of argument is exactly the one required
	static void	checkArgMin(CLuaState &ls, const char* funcName, uint nArgs);		// check that number of argument is at least the one required
	static void	checkArgMax(CLuaState &ls, const char* funcName, uint nArgs);		// check that number of argument is at most the one required
	static void	check(CLuaState &ls, bool ok, const std::string &failReason);
	static void	checkArgType(CLuaState &ls, const char *funcName, uint index, int argType);
	static void	checkArgTypeRGBA(CLuaState &ls, const char *funcName, uint index);
	static void	checkArgTypeUIElement(CLuaState &ls, const char *funcName, uint index);
	static void	checkArgTypeUCString(CLuaState &ls, const char *funcName, uint index);
	/** throw a lua expection (inside a C function called from lua) with the given reason, and the current call stack
	  * The various check... function call this function when their test fails
	  */
	static void fails(CLuaState &ls, const char *format, ...);

	/** execute function that is currently on the stack, possibly outputing error messages to the log
	  * \return true if execution succeeded
	  */
	static bool	executeFunctionOnStack(CLuaState &ls, int numArgs, int numRet);


	// pop a sint32 from a lua stack, throw an exception on fail
	static bool popSINT32(CLuaState &ls, sint32 & dest);
	bool popString(CLuaState &ls, std::string & dest);

	/** read/write between values on a lua stack & a property exported from a 'CReflectable' derived object
	  * (throws on error)
	  */
	static void luaValueToReflectedProperty(CLuaState &ls, int stackIndex, CReflectable &target, const CReflectedProperty &property) throw(ELuaIHMException);

	// push a reflected property on the stack
	// NB : no check is done that 'property' is part of the class info of 'reflectedObject'
	static void luaValueFromReflectedProperty(CLuaState &ls, CReflectable &reflectedObject, const CReflectedProperty &property);



private:



	static void	registerBasics(CLuaState &ls);
	static void	registerIHM(CLuaState &ls);
	static void createLuaEnumTable(CLuaState &ls, const std::string &str);

	// Functions for the ui metatable
	static class CInterfaceElement *getUIRelative(class CInterfaceElement *pIE, const std::string &propName);
	static int luaUIIndex(CLuaState &ls);
	static int luaUINewIndex(CLuaState &ls);
	static int luaUIEq(CLuaState &ls);
	static int luaUINext(CLuaState &ls);
	static int luaUIDtor(CLuaState &ls);
	static int luaClientCfgIndex(CLuaState &ls);
	static int luaClientCfgNewIndex(CLuaState &ls);

	/// \name Exported Functions
	// @{

	// LUA exported Functions with luabind
	static sint32		getPlayerLevel();		// get max level among player skills (magi, combat, crafting ,foraging)
	static sint64		getPlayerVpa();
	static sint64		getPlayerVpb();
	static sint64		getPlayerVpc();
	static sint32		getTargetLevel();		// get current, precise level of the selected target, or -1 if there's no such selected target
	static sint32		getTargetForceRegion(); // get 'force region' for current target, or -1 if there's no selected target
	static sint32		getTargetLevelForce();	// get 'level force' for current target, or -1 if there's no selected target
	static ucstring		getTargetSheet();		// get the name of the target sheet (like 'zoha2old.creature')
	static sint64		getTargetVpa();
	static sint64		getTargetVpb();
	static sint64		getTargetVpc();
	static bool			isTargetNPC(); // return 'true' if the target is an npc
	static bool			isTargetPlayer(); // return 'true' if the target is a player
	static bool			isTargetUser(); // return 'true' if the target is the user
	static bool			isPlayerInPVPMode();
	static bool			isTargetInPVPMode();

	static void			pauseBGDownloader();
	static void			unpauseBGDownloader();
	static void			requestBGDownloaderPriority(uint priority);
	static sint			getBGDownloaderPriority();
	static ucstring		getPatchLastErrorMessage();
	static bool			isInGame();
	static uint32		getPlayerSelectedSlot();
	static bool			isPlayerSlotNewbieLand(uint32 slot);  // test if one of the player slot is a newbieland one, if not so, client must be patched in order to continue
	static uint32		getLocalTime();
	static double		getPreciseLocalTime();
	static sint32		getDbProp(const std::string &dbProp);					// return 0 if not found.
	static void			setDbProp(const std::string &dbProp, sint32 value);		// Nb: the db prop is not created if not present.
	static void			addDbProp(const std::string &dbProp, sint32 value);		// Nb: the db prop is created if not present.
	static void			delDbProp(const std::string &dbProp);
	static std::string	getDefine(const std::string &def);
	static void			messageBox(const ucstring &text);
	static void			messageBox(const ucstring &text, const std::string &masterGroup);
	static void			messageBox(const ucstring &text, const std::string &masterGroup, int caseMode);
	static void			messageBox(const std::string &text);
	static void			messageBoxWithHelp(const ucstring &text);
	static void			messageBoxWithHelp(const ucstring &text, const std::string &masterGroup);
	static void			messageBoxWithHelp(const ucstring &text, const std::string &masterGroup, int caseMode);
	static void			messageBoxWithHelp(const std::string &text);
	static std::string	findReplaceAll(const std::string &str, const std::string &search, const std::string &replace);
	static ucstring		findReplaceAll(const ucstring &str, const ucstring &search, const ucstring &replace);
	static bool			fileExists(const std::string &fileName);
	// just for ease of use
	static ucstring		findReplaceAll(const ucstring &str, const std::string &search, const std::string &replace);
	static ucstring		findReplaceAll(const ucstring &str, const std::string &search, const ucstring &replace);
	static ucstring		findReplaceAll(const ucstring &str, const ucstring &search, const std::string &replace);
	static void			setContextHelpText(const ucstring &text);
	// GameInfo
	static sint32	getSkillIdFromName(const std::string &def);
	static ucstring	getSkillLocalizedName(sint32 skillId);
	static sint32	getMaxSkillValue(sint32 skillId);
	static sint32	getBaseSkillValueMaxChildren(sint32 skillId);
	static sint32	getMagicResistChance(bool elementalSpell, sint32 casterSpellLvl, sint32 victimResistLvl);
	static sint32	getDodgeParryChance(sint32 attLvl, sint32 defLvl);
	static void		browseNpcWebPage(const std::string &htmlId, const std::string &url, bool addParameters, double timeout);
	static void		clearHtmlUndoRedo(const std::string &htmlId);
	static ucstring	getDynString(sint32 dynStringId);
	static bool		isDynStringAvailable(sint32 dynStringId);
	static bool		isFullyPatched();
	static std::string getSheetType(const std::string &sheet);
	static std::string getSheetName(uint32 sheetId);
	static sint32	getFameIndex(const std::string &factionName);
	static std::string getFameName(sint32 fameIndex);
	static sint32	getFameDBIndex(sint32 fameIndex); // convert from the fame index
	static sint32	getFirstTribeFameIndex(); // fame index of the 1st tribe
	static sint32	getNbTribeFameIndex(); // number of tribe fame index (which are contiguous)
	static std::string getClientCfg(const std::string &varName);
	static void		sendMsgToServer(const std::string &msgName);
	static void		sendMsgToServerPvpTag(bool pvpTag);
	static ucstring	replacePvpEffectParam(const ucstring &str, sint32 parameter);
	static std::string getRegionByAlias(uint32 alias);

	static void			tell(const ucstring &player, const ucstring &msg); // open the window to do a tell to 'player', if 'msg' is not empty, then the message will be sent immediatly
																		   // else, current command of the chat window will be replaced with tell 'player'

	static bool			isGuildQuitAvailable();
	static void			sortGuildMembers();
	static sint32		getNbGuildMembers();
	static std::string	getGuildMemberName(sint32 nMemberId);
	static std::string	getGuildMemberGrade(sint32 nMemberId);
	static sint32		secondsSince1970ToHour(sint32 seconds);

	// sheet access
	// TODO nico : using the reflection system on sheets would allow to export them to lua without these functions ...
	static std::string  getCharacterSheetSkel(const std::string &sheet, bool isMale);
	static sint32	getSheetId(const std::string &itemName);
	static bool		isR2Player(const std::string &sheet);
	static std::string	getR2PlayerRace(const std::string &sheet);
	static bool	isR2PlayerMale(const std::string &sheet);
	static sint			getCharacterSheetRegionForce(const std::string &sheet);
	static sint			getCharacterSheetRegionLevel(const std::string &sheet);

	static bool			isCtrlKeyDown(); // test if the ctrl key is down (NB nico : I didn't add other key,
							     // because it would be too easy to write a key recorder ...)

	static std::string encodeURLUnicodeParam(const ucstring &text);
	static bool isRingAccessPointInReach();
	
	static void updateTooltipCoords();
		
		
	// LUA exported Functions with standard lua (because use ui object, use variable param number, or return dynamic-typed object)
	static int  setCaptureKeyboard(CLuaState &ls);
	static int  resetCaptureKeyboard(CLuaState &ls);
	static int	setOnDraw(CLuaState &ls);		// params: CInterfaceGroup*, "script". return: none
	static int	addOnDbChange(CLuaState &ls);	// params: CInterfaceGroup*, "dblist", "script". return: none
	static int	removeOnDbChange(CLuaState &ls);// params: CInterfaceGroup*. return: none
	static int	getUICaller(CLuaState &ls);		// params: none. return: CInterfaceElement*  (nil if error)
	static int	getCurrentWindowUnder(CLuaState &ls);		// params: none. return: CInterfaceElement*  (nil if none)
	static int	getUI(CLuaState &ls);			// params: "ui:interface:...". return: CInterfaceElement*  (nil if error), an additionnal boolean parameter
												//		   can specify verbose display when the element is note found (default is true)
	static int  createGroupInstance(CLuaState &ls); // params : param 1 = template name,
													// param 2 = id of parent where the instance will be inserted
													// param 3 = table with ("template_param", "template_param_value") key/value pairs
													// such as { id="foo", x="10" } etc. -> returns a new instance of the template, or nil on fail
	static int  createRootGroupInstance(CLuaState &ls); // params : param 1 = template name,
														// param 2 = id of parent where the instance will be inserted
														// param 3 = table with ("template_param", "template_param_value") key/value pairs
														// such as { id="foo", x="10" } etc. -> returns a new instance of the template, or nil on fail
	static int  createUIElement(CLuaState &ls); // params : param 1 = template name,
												// param 2 = id of parent where the instance will be inserted
												// param 3 = table with ("template_param", "template_param_value") key/value pairs
											    // such as { id="foo", x="10" } etc. -> returns a new instance of the template, or nil on fail

	static int	displayBubble(CLuaState &ls);	// params : param 1 = bot id
												// param 2 = text
												// param 3 = table with all strings and urls
												// {"main text"="http:///", "text option 1"="http:///", "text option 2"="http:///") etc...
	static int	getIndexInDB(CLuaState &ls);	// params: CDBCtrlSheet*.... return: index, or 0 if error
	static int	getUIId(CLuaState &ls);			// params: CInterfaceElement*. return: ui id (empty if error)
	static int	runAH(CLuaState &ls);			// params: CInterfaceElement *, "ah", "params". return: none
	static int	runExpr(CLuaState &ls);			// params: "expr". return: any of: nil,bool,string,number, RGBA, UCString
	static int	runFct(CLuaState &ls);			// params: "expr", param1, param2.... return: any of: nil,bool,string,number, RGBA, UCString
	static int  runCommand(CLuaState &ls);      // params: "command name", param1, param2 ... return true or false
	static int	formatUI(CLuaState &ls);		// params: "expr", param1, param2.... return: string with # and % parsed
	static int	formatDB(CLuaState &ls);		// params: param1, param2.... return: string with @ and , added
	static int	launchContextMenuInGame(CLuaState &ls); // params : menu name
	static int  parseInterfaceFromString(CLuaState &ls); // params : intreface script
	static int  updateAllLocalisedElements(CLuaState &ls);
	static int  breakPoint(CLuaState &ls);
	static int  getWindowSize(CLuaState &ls);
	static int  i18n(CLuaState &ls);			// retrieve an unicode string from CI18N
	static int	setTextFormatTaged(CLuaState &ls);	// set a text that may contains Tag Format infos
	static int	validMessageBox(CLuaState &ls);	// ok/cancel type message box (can't get it to work through luabind)
	static int	concatUCString(CLuaState &ls); // workaround for + operator that don't work in luabind for ucstrings ...
	static int	concatString(CLuaState &ls); // speedup concatenation of several strings
	static int	tableToString(CLuaState &ls); // concat element of a table to build a string
	static int	setTopWindow(CLuaState &ls); // set the top window
	static int  initEmotesMenu(CLuaState &ls);
	static int  isUCString(CLuaState &ls);
	static int  hideAllWindows(CLuaState &ls);
	static int  hideAllNonSavableWindows(CLuaState &ls);
	static int  getDesktopIndex(CLuaState &ls);
	static int  setLuaBreakPoint(CLuaState &ls); // set a breakpoint in lua external debugger (file, line)
	static int	getMainPageURL(CLuaState &ls);
	static int	getCharSlot(CLuaState &ls);
	static int	displaySystemInfo(CLuaState &ls);
	static int	setWeatherValue(CLuaState &ls); // first value is a boolean to say automatic, second value ranges from of to 1 and gives the weather
	static int	getWeatherValue(CLuaState &ls); // get current real weather value (blend between server driven value & predicted value). Manual weather value is ignored
	static int	disableContextHelpForControl(CLuaState &ls);	// params: CCtrlBase*. return: none
	static int  disableContextHelp(CLuaState &ls);
	static int	getPathContent(CLuaState &ls);
	static int  getServerSeason(CLuaState &ls); // get the last season sent by the server
												// 0->auto, computed locally from the current day (or not received from server yet)
												// 1->server force spring
												// 2->' '		 ' summer
												// 3->' '		 ' autumn
												// 4->' '		 ' winter
	static int	computeCurrSeason(CLuaState &ls); // compute current displayed season (1->spring, etc .)
	static int	getAutoSeason(CLuaState &ls); // compute automatic season that would be at this time (1->spring, etc .)


	static int  getTextureSize(CLuaState &ls);
	static int	enableModalWindow(CLuaState &ls);
	static int	disableModalWindow(CLuaState &ls);
	static int	getPlayerPos(CLuaState &ls);
	static int	getPlayerFront(CLuaState &ls);
	static int	getPlayerDirection(CLuaState &ls);
	static int	getPlayerGender(CLuaState &ls);
	static int	getPlayerName(CLuaState &ls);
	static int	getPlayerTitleRaw(CLuaState &ls);
	static int	getPlayerTitle(CLuaState &ls);
	static int	getTargetPos(CLuaState &ls);
	static int	getTargetFront(CLuaState &ls);
	static int	getTargetDirection(CLuaState &ls);
	static int	getTargetGender(CLuaState &ls);
	static int	getTargetName(CLuaState &ls);
	static int	getTargetTitleRaw(CLuaState &ls);
	static int	getTargetTitle(CLuaState &ls);
	static int  addSearchPathUser(CLuaState &ls);
	static int  getClientCfgVar(CLuaState &ls);
	static int	isPlayerFreeTrial(CLuaState &ls);
	static int	isPlayerNewbie(CLuaState &ls);
	static int  isInRingMode(CLuaState &ls);
	static int  getUserRace(CLuaState &ls);
	static int  getSheet2idx(CLuaState &ls);
	static int	getTargetSlot(CLuaState &ls);
	static int  getSlotDataSetId(CLuaState &ls);


	// LUA functions exported for Dev only (debug)
	static int	deleteUI(CLuaState &ls);		// params: CInterfaceElement*.... return: none
	static int	deleteReflectable(CLuaState &ls);		// params: CInterfaceElement*.... return: none
	static int	dumpUI(CLuaState &ls);			// params: CInterfaceElement*.... return: none
	static int	setKeyboardContext(CLuaState &ls);





	// @}



	// Function export tools
	static int	runExprAndPushResult(CLuaState &ls, const std::string &expr);		// Used by runExpr and runFct

	// Function to forward lua call to C++ to a 'lua method' exported from a reflected object
	static int luaMethodCall(lua_State *ls);

	static int getCompleteIslands(CLuaState &ls);
	static int getIslandId(CLuaState &ls);//TEMP


};


#endif // NL_LUA_IHM_H

/* End of lua_ihm.h */
