#ifndef LUA_IHM_RYZOM_H
#define LUA_IHM_RYZOM_H

#include "nel/gui/lua_ihm.h"

using namespace NLGUI;

class CLuaIHMRyzom
{
public:
	static void RegisterRyzomFunctions( CLuaState &ls );

private:
	static void createLuaEnumTable(CLuaState &ls, const std::string &str);

	static int luaClientCfgIndex(CLuaState &ls);
	static int luaClientCfgNewIndex(CLuaState &ls);
	
	// CInterfaceElement management on stack, stored by a CRefPtr.
public:

private:

	static int getUI(CLuaState &ls); // params: "ui:interface:...". return: CInterfaceElement*  (nil if error), an additionnal boolean parameter
	// LUA exported Functions with standard lua (because use ui object, use variable param number, or return dynamic-typed object)
	static int	getUICaller(CLuaState &ls);		// params: none. return: CInterfaceElement*  (nil if error)
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
	static int	formatUI(CLuaState &ls);		// params: "expr", param1, param2.... return: string with # and % parsed
	static int	formatDB(CLuaState &ls);		// params: param1, param2.... return: string with @ and , added
	static int	launchContextMenuInGame(CLuaState &ls); // params : menu name
	static int  parseInterfaceFromString(CLuaState &ls); // params : intreface script
	static int  updateAllLocalisedElements(CLuaState &ls);
	static int  breakPoint(CLuaState &ls);
	static int  i18n(CLuaState &ls);			// retrieve an unicode string from CI18N
	static int	setTextFormatTaged(CLuaState &ls);	// set a text that may contains Tag Format infos
	static int	validMessageBox(CLuaState &ls);	// ok/cancel type message box (can't get it to work through luabind)
	static int  initEmotesMenu(CLuaState &ls);
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
	static int  getServerSeason(CLuaState &ls); // get the last season sent by the server
												// 0->auto, computed locally from the current day (or not received from server yet)
												// 1->server force spring
												// 2->' '		 ' summer
												// 3->' '		 ' autumn
												// 4->' '		 ' winter
	static int	computeCurrSeason(CLuaState &ls); // compute current displayed season (1->spring, etc .)
	static int	getAutoSeason(CLuaState &ls); // compute automatic season that would be at this time (1->spring, etc .)


	static int	enableModalWindow(CLuaState &ls);
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
	static int	dumpUI(CLuaState &ls);			// params: CInterfaceElement*.... return: none
	static int	setKeyboardContext(CLuaState &ls);

	static int getCompleteIslands(CLuaState &ls);
	static int getIslandId(CLuaState &ls);//TEMP


	///////////////////////////// Standard Lua stuff ends here //////////////////////////////////////////////

	static sint32 getDbProp(const std::string &dbProp); // return 0 if not found.
	static void	setDbProp(const std::string &dbProp, sint32 value);		// Nb: the db prop is not created if not present.
	static void	addDbProp(const std::string &dbProp, sint32 value);		// Nb: the db prop is created if not present.
	static void	delDbProp(const std::string &dbProp);

public:
	// Print a message in the log.
	// Lua messages must be enabled (with ClientCfg.DisplayLuaDebugInfo = 1)
	// Additionnally, if ClientCfg.LuaDebugInfoGotoButtonEnabled is set, then
	// a button will be created near the line to allow to goto the lua line that issued the message
	// by using an external editor
	static void	debugInfo(const std::string &dbg);
	// Print a message in the log
	// No 'goto file' button is created
	// Lua messages must be enabled (with ClientCfg.DisplayLuaDebugInfo = 1)

	static void	dumpCallStack(int startStackLevel = 0);

	/** execute function that is currently on the stack, possibly outputing error messages to the log
	  * \return true if execution succeeded
	  */
	static bool	executeFunctionOnStack(CLuaState &ls, int numArgs, int numRet);

private:
	static void	rawDebugInfo(const std::string &dbg);
	// Dump callstack in the console
	// Additionnally, if ClientCfg.LuaDebugInfoGotoButtonEnabled is set, then
	// buttons will be created in fonr of eahc line to allow to goto the lua line that issued the message
	// by using an external editor

	static void	getCallStackAsString(int startStackLevel, std::string &result);
	static std::string	getDefine(const std::string &def);
	static void			setContextHelpText(const ucstring &text);


	static void	messageBox(const ucstring &text);
	static void	messageBox(const ucstring &text, const std::string &masterGroup);
	static void	messageBox(const ucstring &text, const std::string &masterGroup, int caseMode);
	static void	messageBox(const std::string &text);
	static void	messageBoxWithHelp(const ucstring &text);
	static void	messageBoxWithHelp(const ucstring &text, const std::string &masterGroup);
	static void	messageBoxWithHelp(const ucstring &text, const std::string &masterGroup, int caseMode);
	static void	messageBoxWithHelp(const std::string &text);

	static ucstring	replacePvpEffectParam(const ucstring &str, sint32 parameter);
	static sint32 secondsSince1970ToHour(sint32 seconds);
	static void	pauseBGDownloader();
	static void	unpauseBGDownloader();
	static void	requestBGDownloaderPriority(uint priority);
	static sint	getBGDownloaderPriority();
	static ucstring	getPatchLastErrorMessage();
	static bool	isInGame();
	static uint32 getPlayerSelectedSlot();
	static bool	isPlayerSlotNewbieLand(uint32 slot);  // test if one of the player slot is a newbieland one, if not so, client must be patched in order to continue

	// GameInfo
	static sint32 getSkillIdFromName(const std::string &def);
	static ucstring	getSkillLocalizedName(sint32 skillId);
	static sint32 getMaxSkillValue(sint32 skillId);
	static sint32 getBaseSkillValueMaxChildren(sint32 skillId);
	static sint32 getMagicResistChance(bool elementalSpell, sint32 casterSpellLvl, sint32 victimResistLvl);
	static sint32 getDodgeParryChance(sint32 attLvl, sint32 defLvl);
	static void	browseNpcWebPage(const std::string &htmlId, const std::string &url, bool addParameters, double timeout);
	static void	clearHtmlUndoRedo(const std::string &htmlId);
	static ucstring	getDynString(sint32 dynStringId);
	static bool	isDynStringAvailable(sint32 dynStringId);
	static bool	isFullyPatched();
	static std::string getSheetType(const std::string &sheet);
	static std::string getSheetName(uint32 sheetId);
	static sint32 getFameIndex(const std::string &factionName);
	static std::string getFameName(sint32 fameIndex);
	static sint32 getFameDBIndex(sint32 fameIndex); // convert from the fame index
	static sint32 getFirstTribeFameIndex(); // fame index of the 1st tribe
	static sint32 getNbTribeFameIndex(); // number of tribe fame index (which are contiguous)
	static std::string getClientCfg(const std::string &varName);
	static void	sendMsgToServer(const std::string &msgName);
	static void	sendMsgToServerPvpTag(bool pvpTag);
	static bool	isGuildQuitAvailable();
	static void	sortGuildMembers();
	static sint32 getNbGuildMembers();
	static std::string getGuildMemberName(sint32 nMemberId);
	static std::string getGuildMemberGrade(sint32 nMemberId);
	static bool isR2Player(const std::string &sheet);
	static std::string getR2PlayerRace(const std::string &sheet);
	static bool	isR2PlayerMale(const std::string &sheet);
	// sheet access
	// TODO nico : using the reflection system on sheets would allow to export them to lua without these functions ...
	static std::string getCharacterSheetSkel(const std::string &sheet, bool isMale);
	static sint32 getSheetId(const std::string &itemName);
	static sint getCharacterSheetRegionForce(const std::string &sheet);
	static sint	getCharacterSheetRegionLevel(const std::string &sheet);
	static std::string getRegionByAlias(uint32 alias);
	// open the window to do a tell to 'player', if 'msg' is not empty, then the message will be sent immediatly
    // else, current command of the chat window will be replaced with tell 'player'
	static void	tell(const ucstring &player, const ucstring &msg);
	static bool isRingAccessPointInReach();
	static void updateTooltipCoords();
	// test if the ctrl key is down (NB nico : I didn't add other key,
	// because it would be too easy to write a key recorder ...)
	static bool	isCtrlKeyDown(); 							     
	static std::string encodeURLUnicodeParam(const ucstring &text);

	static sint32 getPlayerLevel();		// get max level among player skills (magi, combat, crafting ,foraging)
	static sint64 getPlayerVpa();
	static sint64 getPlayerVpb();
	static sint64 getPlayerVpc();
	static sint32 getTargetLevel();		// get current, precise level of the selected target, or -1 if there's no such selected target
	static sint32 getTargetForceRegion(); // get 'force region' for current target, or -1 if there's no selected target
	static sint32 getTargetLevelForce();	// get 'level force' for current target, or -1 if there's no selected target
	static ucstring getTargetSheet();		// get the name of the target sheet (like 'zoha2old.creature')
	static sint64 getTargetVpa();
	static sint64 getTargetVpb();
	static sint64 getTargetVpc();
	static bool	isTargetNPC(); // return 'true' if the target is an npc
	static bool	isTargetPlayer(); // return 'true' if the target is a player
	static bool	isTargetUser(); // return 'true' if the target is the user
	static bool	isPlayerInPVPMode();
	static bool	isTargetInPVPMode();


public:

	// Create a special tag that will add a 'goto' button for the given file and line
	// The tag should be appended in front of a string to use with 'rawDebugInfo'.
	// when the final string will be printed, a button will be created in front of it
	// Requires that 'ClientCfg.LuaDebugInfoGotoButtonEnabled' is set to 1, else
	// a, empty tag is returned
	static std::string	createGotoFileButtonTag(const char *fileName, uint line);
};

#endif

