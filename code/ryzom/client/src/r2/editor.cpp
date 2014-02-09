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

#include "stdpch.h"
#include <sstream>

#include "nel/misc/path.h"
#include "nel/misc/i18n.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/polygon.h"
#include "nel/misc/time_nl.h"
//
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"
//
#include "nel/ligo/ligo_config.h"
#include "nel/net/unified_network.h"
#include "nel/net/module_manager.h"
#include "nel/pacs/u_global_retriever.h"
//
#include "r2_lua.h"
#include "game_share/r2_share_itf.h"
#include "game_share/r2_messages.h"
#include "game_share/scenario_entry_points.h"
//
#include "dmc/dmc.h" // client interface to the dynamic scenario service
#include "dmc/client_edition_module.h"
#include "dmc/property_accessor.h"
#include "dmc/com_lua_module.h"
#include "game_share/dms.h"
//
#include "../client_sheets/item_sheet.h"

#include "editor.h"
//
#include "nel/gui/lua_helper.h"
using namespace NLGUI;
#include "nel/gui/group_tree.h"
#include "../interface_v3/interface_manager.h"
#include "../contextual_cursor.h"
#include "../cursor_functions.h"
#include "../entities.h"
#include "../events_listener.h"
#include "nel/gui/group_list.h"
#include "nel/gui/event_descriptor.h"
#include "nel/gui/group_tree.h"
#include "../client_cfg.h"
#include "nel/gui/lua_ihm.h"
#include "../interface_v3/lua_ihm_ryzom.h"
#include "nel/gui/lua_object.h"
#include "../global.h"
#include "../connection.h"
#include "../main_loop.h"
#include "../interface_v3/people_interraction.h"
#include "../time_client.h"
#include "../pacs_client.h"
#include "nel/gui/lua_ihm.h"
#include "../actions.h"
#include "../actions_client.h"
#include "object_factory_client.h"
#include "../weather.h"
#include "../light_cycle_manager.h"
#include "../dummy_progress.h"
#include "../continent_manager.h"
#include "../world_database_manager.h"
#include "../init_main_loop.h"
#include "../net_manager.h"
#include "../interface_v3/input_handler_manager.h"
#include "../connection.h"
#include "../init_main_loop.h"
#include "nel/gui/group_editbox.h"
#include "../landscape_poly_drawer.h"
#include "../input.h"
#include "../motion/user_controls.h"
#include "../game_context_menu.h"
#include "../interface_v3/macrocmd_manager.h"
//
#include "../player_r2_cl.h"
#include "palette_node.h"
#include "tool_create_entity.h"
#include "tool_select_move.h"
#include "tool_new_vertex.h"
#include "displayer_visual_entity.h"
#include "displayer_visual_group.h"
#include "displayer_visual_shape.h"
#include "displayer_visual_activity_sequence.h"
#include "displayer_lua.h"
#include "verbose_clock.h"
#include "r2_config.h"
#include "entity_sorter.h"
//
#include "tool_draw_prim.h"
#include "tool_select_move.h"
#include "tool_select_rotate.h"
#include "tool_choose_pos_lua.h"
//


#include "../sheet_manager.h"

#include "../session_browser_impl.h"
#include "../far_tp.h"
#include "nel/gui/lua_manager.h"


using namespace NLMISC;
using namespace NLNET;
using namespace NL3D;


extern CEventsListener EventsListener;
extern CLog	g_log;
extern CGameContextMenu	GameContextMenu;
extern void badXMLParseMessageBox();

R2::TUserRole UserRoleInSession;

#define OPERATOR_EQUAL(X,Y) X==Y
//#define OPERATOR_EQUAL(X, Y) ::operator==(X,Y)

namespace R2
{

class CEditorCheck
{
public:
	CEditorCheck() { check(); }
	~CEditorCheck() { check(); }
	void check()
	{
		if (!EditorCreated) return; // avoid infinite loop
		if (getEditor().getMode() != CEditor::NotInitialized)
		{
			nlassert(getEditor().getEnv().isValid());
		}
	}
	static bool EditorCreated;
};


bool CEditorCheck::EditorCreated = false;

#ifdef NL_DEBUG
	#define	 CHECK_EDITOR CEditorCheck __ec;
#else
	#define	 CHECK_EDITOR (void) 0;
#endif



const char *DEFAULT_CURSOR = "curs_default.tga";

bool ResetWanted = false;
bool ReloadUIFlag = true; // by default, CEditor loads its own UI
bool ResetScenarioWanted = false;
bool ReloadScenarioWanted = false;
bool ConnectionWanted = false;
std::string CEditor::_ScenarioToLoadWhenEntreringIntoAnimation="";
bool CEditor::_IsStartingScenario=false;

// *********************************************************************************************************
/** this class forward modification events from the network so that the editor can
  * update its state to match the current state of the map
  */
class CDynamicMapClientEventForwarder : public CDynamicMapClient
{
public:
	CDynamicMapClientEventForwarder(const std::string &eid, NLNET::IModuleSocket * clientGateway, lua_State *luaState);
	virtual void nodeErased(const std::string& instanceId, const std::string& attrName, sint32 position);
	virtual void nodeSet(const std::string& instanceId, const std::string& attrName, CObject* value);
	virtual void nodeInserted(const std::string& instanceId, const std::string& attrName, sint32 position,
							  const std::string& key, CObject* value);
	virtual void nodeMoved(const std::string& instanceId, const std::string& attrName, sint32 position,
							const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);
	virtual void scenarioUpdated(CObject* highLevel, bool willTP, uint32 initialActIndex);

	virtual void onAnimationModeConnected(const CClientMessageAdventureUserConnection& connected);
	virtual void onEditionModeConnected( uint32 userSlotId, uint32 adventureId, CObject* highLevel, const std::string& versionName, bool willTP, uint32 initialActIndex);
	virtual void onEditionModeDisconnected();
	virtual void onResetEditionMode();
	virtual void onTestModeConnected();
	virtual void onTestModeDisconnected(TSessionId sessionId, uint32 lasAct, TScenarioSessionType sessionType);
};


CDynamicMapClientEventForwarder::CDynamicMapClientEventForwarder(const std::string &eid, NLNET::IModuleSocket * clientGateway, lua_State *luaState) :
CDynamicMapClient(eid, clientGateway, luaState)
{

}


void CDynamicMapClientEventForwarder::nodeErased(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_nodeErased)
	if (!getEditor().getMode() == CEditor::EditionMode) return;
	getEditor().nodeErased(instanceId, attrName, position);
}

void CDynamicMapClientEventForwarder::nodeSet(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_nodeSet)
	if (!getEditor().getMode() == CEditor::EditionMode) return;
	getEditor().nodeSet(instanceId, attrName, value);
}

void CDynamicMapClientEventForwarder::nodeInserted(const std::string& instanceId, const std::string& attrName, sint32 position,
							  const std::string& key, CObject* value)
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_nodeInserted)
	if (!getEditor().getMode() == CEditor::EditionMode) return;
	getEditor().nodeInserted(instanceId, attrName, position, key, value);
}

void CDynamicMapClientEventForwarder::nodeMoved(
				const std::string& instanceId, const std::string& attrName, sint32 position,
				const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_nodeMoved)
	if (!getEditor().getMode() == CEditor::EditionMode) return;
	getEditor().nodeMoved(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
}

void CDynamicMapClientEventForwarder::scenarioUpdated(CObject* highLevel, bool willTP, uint32 initialActIndex)
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_scenarioUpdated)
	if (!getEditor().getMode() == CEditor::EditionMode) return;
	getEditor().scenarioUpdated(highLevel, willTP, initialActIndex);
}


void CDynamicMapClientEventForwarder::onEditionModeConnected( uint32 userSlotId, uint32 adventureId, CObject* highLevel, const std::string& versionName, bool willTP, uint32 initialActIndex )
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_onEditionModeConnected)
	getEditor().onEditionModeConnected(userSlotId, adventureId, highLevel, versionName, willTP,  initialActIndex);
}

void CDynamicMapClientEventForwarder::onEditionModeDisconnected()
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_onEditionModeDisconnected)
	getEditor().onEditionModeDisconnected();
}

void CDynamicMapClientEventForwarder::onResetEditionMode()
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_onResetEditionMode)
	getEditor().onResetEditionMode();
}

void CDynamicMapClientEventForwarder::onTestModeConnected()
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_onTestModeConnected)
	getEditor().onTestModeConnected();
}

void CDynamicMapClientEventForwarder::onTestModeDisconnected(TSessionId sessionId, uint32 lastAct, TScenarioSessionType sessionType)
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_onTestModeDisconnected)
	getEditor().onTestModeDisconnected(sessionId, lastAct, sessionType);
}

void CDynamicMapClientEventForwarder::onAnimationModeConnected(const CClientMessageAdventureUserConnection& connected)
{
	//H_AUTO(R2_CDynamicMapClientEventForwarder_onAnimationModeConnected)
	getEditor().onAnimationModeConnected(connected);
}

// *********************************************************************************************************
CEditor::CEditor()
{
	//H_AUTO(R2_CEditor_CEditor)
	_SelectedInstance = NULL;
	_FocusedInstance = NULL;
	_Scenario = NULL;
	_EnteredInSetSelectedInstance = false;
	_DecalRefTime = 0;
	_Initialized = false;
	//vianney?? _LastUIModificationDate = 0;
	_InstanceObserverHandleCounter = 0;
	// module are initialized one time and never reconnected
	_DMC = NULL;
	_DMS = NULL;
	_Mode = NotInitialized;
	_AccessMode = AccessModeUnknown;
	_SerializeUIConfig = true;
	_LastAutoSaveTime = NLMISC::CTime::getLocalTime();// wait ClientCfg.R2EDAutoSaveWait
	setForceDesktopReset(false);
	CEditorCheck::EditorCreated = true;
	_ClearingContent = false;
	_Season = UnknownSeason;
	_IsWaitingTPForSeasonChange = true;
	_UpdatingScenario = false;
	_WillTP = false;
	_FixedLighting =  false;
	//
	_ScenarioReceivedFlag = false;
	_TPReceivedFlag = false;
	_WaitScenarioScreenWanted = false;
	_WaitScenarioScreenActive = false;
	_NewScenario = NULL;
	_NewScenarioInitialAct = 0;
	_EditionModeDisconnectedFlag = true;
	_PostponeScenarioUpdated = false;
	_EntitySorter = NULL;
	_MaxVisibleEntityExceededFlag = false;
}

// *********************************************************************************************************
void CEditor::setForceDesktopReset(bool force)
{
	//H_AUTO(R2_CEditor_setForceDesktopReset)
	CHECK_EDITOR
	std::fill(_ForceDesktopReset, _ForceDesktopReset + sizeofarray(_ForceDesktopReset), force);
}

// *********************************************************************************************************
void CEditor::autoConfigInit(bool serverIsRingSession)
{
	//H_AUTO(R2_CEditor_autoConfigInit)
	// R2ED enabled ?
	if (ClientCfg.R2EDEnabled)
	{
		// Editor and Animator (DM) modes
		initModules(true);
		TMode initialMode;
		TAccessMode initialAccessMode;
		switch (UserRoleInSession.getValue())
		{
		case R2::TUserRole::ur_editor:
			initialMode = R2::CEditor::EditionMode;
			initialAccessMode = R2::CEditor::AccessEditor;
			break;
		case R2::TUserRole::ur_animator:
			initialMode = R2::CEditor::AnimationModeDm;
			initialAccessMode = R2::CEditor::AccessDM;
			break;
		case R2::TUserRole::ur_outland_owner:
			initialMode = R2::CEditor::AnimationModeDm;
			initialAccessMode = R2::CEditor::AccessOutlandOwner;
			break;
		case R2::TUserRole::ur_player:
			initialMode = R2::CEditor::AnimationModePlay;
			// NOTE: I'm not sure if 'editor' is the right mode here - but I can't find anything better for now
			// If I try 'AccessModeUnknown' the code asserts later on.
			initialAccessMode = R2::CEditor::AccessEditor;
			break;
		default:
			nlwarning( "Unexpected user role %u while editor enabled", (uint)(UserRoleInSession.getValue()) );
			initialMode = R2::CEditor::EditionMode;
			initialAccessMode = R2::CEditor::AccessEditor;
		}
		init(initialMode, initialAccessMode);
	}
	else
	{
		// Normal player and Ring player modes
		ActionsContext.setContext("game");
		if (serverIsRingSession)
			initModules(false);
		if (UserRoleInSession.getValue() != R2::TUserRole::ur_player)
			nlwarning( "Unexpected user role %u while editor disabled", (uint)(UserRoleInSession.getValue()) );
	}
}


// *********************************************************************************************************
void CEditor::autoConfigRelease(bool serverIsRingSession)
{
	//H_AUTO(R2_CEditor_autoConfigRelease)
	if (ClientCfg.R2EDEnabled)
	{
		R2::getEditor().release();
		R2::getEditor().releaseModules();
	}
	else if (serverIsRingSession)
		R2::getEditor().releaseModules();
}

// *********************************************************************************************************
void CEditor::initModules(bool connectDMC)
{
	//H_AUTO(R2_CEditor_initModules)
	CHECK_EDITOR
	IModuleManager &mm = IModuleManager::getInstance();

	IModule *gateway = mm.createModule("StandardGateway", "clientGw", "");
	nlassert(gateway != NULL);
	NLNET::IModuleSocket *socketClientGw = mm.getModuleSocket("clientGw");
	nlassert(socketClientGw != NULL);
	// connect the client gateway to the server
	CCommandRegistry::getInstance().execute("clientGw.transportAdd FEClient fec", *InfoLog);
	if (!ClientCfg.Local)
	{
		CCommandRegistry::getInstance().execute("clientGw.transportCmd fec(open)", *InfoLog);
	}

	if (ClientCfg.Local)
	{
		_DMS = new CDynamicMapService(ClientCfg.ConfigFile,  socketClientGw);
		_DMS->init();
	}
	else
	{
		_DMS = 0;
	}
	if (connectDMC)
		_DMC = new CDynamicMapClientEventForwarder("Client0",socketClientGw,  getLua().getStatePointer());
	else
		_DMC = new CDynamicMapClient("Client0",socketClientGw,  getLua().getStatePointer());

	_DMC->setInstantFeedBackFlag(true);

}

// *********************************************************************************************************

void CEditor::releaseModules()
{
	//H_AUTO(R2_CEditor_releaseModules)
	CHECK_EDITOR
	delete _DMC; // must have a virtual destructor
	if (ClientCfg.Local)
		delete _DMS;
	_DMC = NULL;
	_DMS = NULL;

	IModuleManager &mm = IModuleManager::getInstance();
	NLNET::IModule*  clientGw = mm.getLocalModule("clientGw");
	if (clientGw)
	{
		mm.deleteModule(clientGw);
	}

	_Initialized = false;
}

// *********************************************************************************************************
void CEditor::inGameSelection(const CLFECOMMON::TCLEntityId &/* slot */)
{
	//H_AUTO(R2_CEditor_inGameSelection)
	switch(_Mode)
	{
		case DMMode:
		case AnimationModeDm:
			// reset current selection options
			getEditor().callEnvMethod("updateAnimBarActions", 0);

			getLua().push("");
			getEditor().callEnvMethod("dssTarget", 1);
		break;
		default:
			// no-op
		break;
	}
}

// *********************************************************************************************************
CEditor::~CEditor()
{
	release();
}

// ***************************************************************
void CEditor::requestSetLocalNode(const std::string& instanceId, const std::string& attrName, const CObject *value)
{
	//H_AUTO(R2_CEditor_requestSetLocalNode)
	CHECK_EDITOR

	CObject* obj = NULL;
	{

		CObject *src = _DMC->find(instanceId);
		if (!src)
		{
			nlwarning("Can't find object with id %s", instanceId.c_str());
			return;
		}
		if (!attrName.empty())
		{
			CObject *subObj = src->getAttr(attrName);
			if (!subObj)
			{
				CObject * valueBase = (CObject *) getObject(src, attrName); // from Base
				if (!valueBase)
				{
					nlwarning("Can't find attribute %s inside object (or is base) with InstanceId =  %s", attrName.c_str(), instanceId.c_str());
					return;
				}
				CObject *valueClone = valueBase->clone();
				valueClone->setParent(0);
				src->setObject(attrName, valueClone);
				subObj = src->getAttr(attrName);
			}
			if (!subObj)
			{
				nlwarning("Can't find attribute %s inside object with InstanceId =  %s", attrName.c_str(), instanceId.c_str());
				return;
			}

			obj = subObj;
		}
	}

	getDMC().getPropertyAccessor().shadowValue(obj, value->clone());
	// trigger handlers
	onAttrModified(obj);
}

// *****************************************

void CEditor::requestCommitLocalNode(const std::string& instanceId, const std::string& attrName)
{
	//H_AUTO(R2_CEditor_requestCommitLocalNode)
	CHECK_EDITOR
	CObject *obj = _DMC->find(instanceId, attrName);
	if (!obj)
	{
		nlwarning("(while calling requestSetLocalNode");
		return;
	}

	//nlwarning("**** current unshadowed value");
	//obj->dump();

	/*
	// when requestSetNode is called, the old value can't be retrieved because it will
	// be erased by 'commitValue', so backup old value
	CObject *oldValueBackup = obj->clone();

	// send net request
	getDMC().getPropertyAccessor().commitValue(obj);
	getDMC().requestSetNode(instanceId, attrName, obj);
	*/
	CObject *localValue = getDMC().getPropertyAccessor().getShadowingValue(obj);
	if (!localValue)
	{
		nlwarning("value is not shadowed : ");
		obj->dump();
		return;
	}
	CObject *localValueBackup = localValue->clone();
	getDMC().getPropertyAccessor().rollbackValue(obj);
	getDMC().requestSetNode(instanceId, attrName, localValueBackup);

}

// ***************************************************************
void CEditor::requestRollbackLocalNode(const std::string& instanceId, const std::string& attrName)
{
	//H_AUTO(R2_CEditor_requestRollbackLocalNode)
	CHECK_EDITOR
	CObject *obj = _DMC->find(instanceId, attrName);
	if (!obj)
	{
		nlwarning("(while calling requestSetLocalNode");
		return;
	}
	getDMC().getPropertyAccessor().rollbackValue(obj);
	// force the displayer to update their state from the shadowed value in place of the local value
	onAttrModified(obj);
}

// ***************************************************************
CInterfaceManager &CEditor::getUI()
{
	//H_AUTO(R2_CEditor_getUI)
	CHECK_EDITOR
	CInterfaceManager *im = CInterfaceManager::getInstance();
	nlassert(im);
	return *im;
}

// *********************************************************************************************************
CLuaState &CEditor::getLua()
{
	//H_AUTO(R2_CEditor_getLua)
	CHECK_EDITOR
	CLuaState *ls = CLuaManager::getInstance().getLuaState();
	nlassert(ls);
	return *ls;
}

// *********************************************************************************************************
CLuaObject &CEditor::getEnv()
{
	//H_AUTO(R2_CEditor_getEnv)
	nlassert(_Env.isValid());
	return _Env;
}

// *********************************************************************************************************
CLuaObject CEditor::getConfig()
{
	//H_AUTO(R2_CEditor_getConfig)
	CHECK_EDITOR
	return getEnv()["Config"];
}

// *********************************************************************************************************
void CEditor::clearDebugWindow()
{
	//H_AUTO(R2_CEditor_clearDebugWindow)
	CHECK_EDITOR
	getUI().flushDebugWindow();
	CGroupList *gl = dynamic_cast<CGroupList *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:debug_info:content:cb:text_list"));
	if (gl)
	{
		gl->deleteAllChildren();
	}
}

// *********************************************************************************************************
// callback to reload the editor when one of the config files changed
void CEditor::reloadEditorCallback(const std::string &filename)
{
	//H_AUTO(R2_CEditor_reloadEditorCallback)
	CHECK_EDITOR
	ResetWanted = true;
	if (nlstricmp(CFile::getExtension(filename), "xml") == 0 ||
		nlstricmp(CFile::getExtension(filename), "uxt") == 0)
	{
		ReloadUIFlag = true;
	}
}

// *********************************************************************************************************
void CEditor::setFixedLighting(bool enabled)
{
	//H_AUTO(R2_CEditor_setFixedLighting)
	if (_FixedLighting == enabled) return;
	_FixedLighting = enabled;
	LightCycleManager.touch();
}

// *********************************************************************************************************
int CEditor::luaGetSelectedInstanceId(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetSelectedInstanceId)
	CHECK_EDITOR
	if (!getEditor().getSelectedInstance())
	{
		ls.pushNil();
	}
	else
	{
		ls.push(toString(getEditor().getSelectedInstance()->getId()));
	}
	return 1;
}
void CEditor::setStartingAnimationFilename(const std::string& filename)
{
	//H_AUTO(R2_CEditor_setStartingAnimationFilename)
	_ScenarioToLoadWhenEntreringIntoAnimation = filename;
}
// *********************************************************************************************************

int CEditor::luaGetStartingAnimationFilename(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetStartingAnimationFilename)
	ls.push( _ScenarioToLoadWhenEntreringIntoAnimation);
	return 1;
}
// *********************************************************************************************************
int CEditor::luaSetSelectedInstanceId(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSetSelectedInstanceId)
	CHECK_EDITOR
	CLuaIHM::checkArgCount(ls, "setSelectedInstance (method)", 2);
	CLuaIHM::checkArgType(ls, "setSelectedInstance", 2, LUA_TSTRING);
	if (strcmp(ls.toString(2), "") == 0)
	{
		getEditor().setSelectedInstance(NULL);
		return 0;
	}
	CInstance *inst = getEditor().getInstanceFromId(ls.toString(2));
	if (!inst)
	{
		nlwarning("setSelectedInstance : Instance with Id %s not found", ls.toString(2));
		return 0;
	}
	if (!inst->getSelectableFromRoot())
	{
		nlwarning("Instance with id %s or one of its ancestor is not selectable", ls.toString(2));
		return 0;
	}
	getEditor().setSelectedInstance(inst);
	return 0;
}

// *********************************************************************************************************
int CEditor::luaSetCurrentTool(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSetCurrentTool)
	CHECK_EDITOR
	CLuaIHM::check(ls, ls.getTop() == 2 || ls.getTop() == 3, "luaSetCurrentTool (method)");
	CLuaIHM::checkArgType(ls, "setCurrentTool", 2, LUA_TSTRING);
	if (ls.strlen(2) ==  0)
	{
		getEditor().setCurrentTool(NULL);
	}
	else
	{
		CTool *tool = createObjectFromClassName<CTool>(ls.toString(2));
		if (tool)
		{
			if (ls.getTop() == 3)
			{
				CLuaObject initParams(ls);
				tool->init(initParams);
			}
			getEditor().setCurrentTool(tool);
		}
	}
	return 0;
}

// *********************************************************************************************************
int CEditor::luaGetCurrentTool(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetCurrentTool)
	CHECK_EDITOR
	CLuaIHM::checkArgCount(ls, "getCurrentTool", 1); // 1 because of the method call
	CLuaIHM::pushReflectableOnStack(ls, getEditor().getCurrentTool());
	return 1;
}

// *********************************************************************************************************
int CEditor::luaGetSelectedInstance(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetSelectedInstance)
	CHECK_EDITOR
	if (!getEditor().getSelectedInstance())
	{
		ls.pushNil();
	}
	else
	{
		getEditor().projectInLua(getEditor().getSelectedInstance()->getObjectTable());
	}
	return 1;
}

// *********************************************************************************************************
int CEditor::luaGetInstanceFromId(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetInstanceFromId)
	CHECK_EDITOR
	const char *funcName = "getInstanceFromId";
	CLuaIHM::checkArgCount(ls, funcName, 2);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CInstance *inst = getEditor().getInstanceFromId(ls.toString(2));
	if (inst == NULL)
	{
		ls.pushNil();
		return 1;
	}
	else
	{
		getEditor().projectInLua(inst->getObjectTable());
		return 1;
	}
	return 0;
}

// ********************************************************************************************************
int CEditor::luaGetVisualPropertiesFromInstanceId(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetVisualPropertiesFromInstanceId)
	const char *funcName = "getVisualPropertiesFromInstanceId";

	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING); // instance  sheetClient

	CInstance *inst = getEditor().getInstanceFromId(ls.toString(1));
	if (!inst) { return 0; }


	CObject *object = inst->getObjectTable();
	if (!object) { return 0; }

	SPropVisualA vA;
	SPropVisualB vB;
	SPropVisualC vC;

	bool ok = getEditor().getVisualPropertiesFromObject(object, vA, vB, vC);

	if (!ok)
	{
		return 0; //no param
	}

	uint64 uVPA = vA.get();
	uint64 uVPB = vB.get();
	uint64 uVPC = vC.get();

	std::string strVPABC = NLMISC::toString( "VPA:%016.16"NL_I64"x\nVPB:%016.16"NL_I64"x\nVPC:%016.16"NL_I64"x", uVPA, uVPB, uVPC );

	ls.push(strVPABC);

	return 1; //3 string
}

// *********************************************************************************************************
int CEditor::luaDisplayContextMenu(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaDisplayContextMenu)
	CHECK_EDITOR
	CLuaIHM::checkArgCount(ls, "displayContextMenu", 1); // this is a method
	getEditor().displayContextMenu();
	return 0;
}

// *********************************************************************************************************
int CEditor::luaConnectAsCreator(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaConnectAsCreator)
	CHECK_EDITOR
	CLuaIHM::checkArgCount(ls, "connectAsCreator", 1); // this is a method
	//getEditor().connectAsCreator();
	return 0;
}

// *********************************************************************************************************
int CEditor::luaDofile(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaDofile)
	CHECK_EDITOR
	CLuaIHM::checkArgCount(ls, "doFile", 1);
	CLuaIHM::checkArgType(ls, "doFile", 1, LUA_TSTRING);
	getEditor().doLuaScript(ls.toString(-1), ls.toString(-1));
	return 0;
}

// *********************************************************************************************************
int CEditor::luaTryFile(CLuaState &/* ls */)
{
	//H_AUTO(R2_CEditor_luaTryFile)
/*	CHECK_EDITOR
	CLuaIHM::checkArgCount(ls, "doFile", 1);
	CLuaIHM::checkArgType(ls, "doFile", 1, LUA_TSTRING);
	getEditor().doLuaScript(ls.toString(-1), ls.toString(-1));*/
	return 0;
}

// *********************************************************************************************************
int CEditor::luaSetEntityCustomSelectBox(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSetEntityCustomSelectBox)
	CHECK_EDITOR
	CLuaIHM::checkArgCount(ls, "setEntityCustomSelectBox", 3); // 3 because of method call
	CLuaIHM::checkArgType(ls, "setEntityCustomSelectBox", 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, "setEntityCustomSelectBox", 3, LUA_TTABLE);
	std::string sheetName = ls.toString(2);
	CEntityCustomSelectBox selectBox;
	CLuaObject obj(ls);
	selectBox.fromTable(obj);
	TEntityCustomSelectBoxMap &boxMap = getEditor().getEntityCustomSelectBoxMap();
	boxMap[sheetName] = selectBox;
	/*
	static volatile bool dumpMap = false;
	if (dumpMap)
	{
		for (TEntityCustomSelectBoxMap::const_iterator watchIt = boxMap.begin(); watchIt != boxMap.end(); ++ watchIt)
		{
			nlwarning(watchIt->first.c_str());
		}
	}
	*/
	return 0;
}

// *********************************************************************************************************
int CEditor::luaGetEntityCustomSelectBox(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetEntityCustomSelectBox)
	CHECK_EDITOR
	CLuaIHM::checkArgCount(ls, "getEntityCustomSelectBox", 2); // 3 because of method call
	CLuaIHM::checkArgType(ls, "getEntityCustomSelectBox", 2, LUA_TSTRING);
	std::string sheetName = ls.toString(2);
	TEntityCustomSelectBoxMap boxMap = getEditor().getEntityCustomSelectBoxMap();
	if (boxMap.count(sheetName))
	{
		ls.newTable();
		CLuaObject result(ls);
		boxMap[sheetName].toTable(result);
		result.push();
	}
	else
	{
		ls.pushNil();
	}
	return 1;
}

// *********************************************************************************************************
int CEditor::luaChoosePos(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaChoosePos)
	CHECK_EDITOR
	const char *funcName = "choosePos";
	CLuaIHM::checkArgMin(ls, funcName, 5);
	CLuaIHM::checkArgMax(ls, funcName, 10);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 5, LUA_TSTRING);
	std::string toolName = ls.toString(5);
	std::string cursValid = "curs_create.tga";
	std::string cursInvalid = "curs_stop.tga";
	if (ls.getTop() >= 6)
	{
		CLuaIHM::checkArgType(ls, funcName, 6, LUA_TSTRING);
		cursValid = ls.toString(6);
	}
	if (ls.getTop() >= 7)
	{
		CLuaIHM::checkArgType(ls, funcName, 7, LUA_TSTRING);
		cursInvalid = ls.toString(7);
	}
	// method param 1 -> sheet id
	// method param 2 -> ok function
	// method param 3 -> cancel function
	ls.pushValue(3);
	CLuaObject validFunc(ls); // pop valid function
	ls.pushValue(4);
	CLuaObject cancelFunc(ls); // pop cancel function
	//
	sint ghostSlot;
	if (ls.strlen(2) != 0)
	{
		CSheetId sheetId(ls.toString(2));
		if (sheetId == CSheetId::Unknown)
		{
			nlwarning("Can't get sheet %s", ls.toString(2));
			return 0;
		}
		ghostSlot = 1; // TMP TMP
		if (!createEntity(ghostSlot, sheetId, CVector::Null, 0.f))
		{
			nlwarning("Cannot create entity for sheet %s", ls.toString(2));
			return 0;
		}
	}
	else
	{
		ghostSlot = -1; // no representation in scene
	}
	// additionnal polys displayed by the choose pos
	std::vector<NLMISC::CPolygon2D> polys;
	CPrimLook validLook;
	CPrimLook invalidLook;
	if (ls.getTop() >= 8)
	{
		CLuaStackRestorer lsr(&ls, ls.getTop());
		CLuaIHM::checkArgType(ls, funcName, 8, LUA_TTABLE);
		ls.pushValue(8);
		CLuaObject poly;
		poly.pop(ls);
		ENUM_LUA_TABLE(poly, it)
		{
			NLMISC::CPolygon2D newPoly;
			it.nextValue().push();
			CLuaIHM::getPoly2DOnStack(ls, -1, newPoly);
			ls.pop();
			polys.push_back(newPoly);
		}
	}
	if (ls.getTop() >= 9)
	{
		ls.pushValue(9);
		CLuaObject look;
		look.pop(ls);
		validLook.init(look);
	}
	if (ls.getTop() >= 10)
	{
		ls.pushValue(10);
		CLuaObject look;
		look.pop(ls);
		invalidLook.init(look);
	}
	//
	getEditor().setCurrentTool(new CToolChoosePosLua(ghostSlot, validFunc, cancelFunc, toolName, cursValid, cursInvalid, polys, validLook, invalidLook));
	return 0;
}

// *********************************************************************************************************
int CEditor::luaSnapPosToGround(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSnapPosToGround)
	CHECK_EDITOR
	const char *funcName = "snapPosToGround";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TNUMBER);
	//
	/*NLPACS::UGlobalPosition gpos = GR->retrievePosition(CVector((float) ls.toNumber(2), (float) ls.toNumber(3), 2000.f));
	if (gpos.InstanceId != -1)
	{
		CVector snappedPos = GR->getGlobalPosition(gpos);
		ls.push((double) snappedPos.x);
		ls.push((double) snappedPos.y);
		ls.push((double) snappedPos.z);
		return 3;
	}
	else
	{
		// default z to 0
		ls.push(0.f);
		return 3;
	}*/

	// new algo : to avoid to snapping to a cliff, start from the approximate (valid) height found in the height map
	CVector inter;
	if (CTool::computeWorldMapIntersection((float) ls.toNumber(2), (float) ls.toNumber(3), inter) != CTool::NoIntersection)
	{
		ls.push((double) inter.x);
		ls.push((double) inter.y);
		ls.push((double) inter.z);
		return 3;
	}
	else
	{
		// default z to 0
		ls.push(0.f);
		return 3;
	}
}

// *********************************************************************************************************
int CEditor::luaGetUserEntityPosition(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetUserEntityPosition)
	CHECK_EDITOR
	const char *funcName = "getUserEntityPosition";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	ls.push((double) UserEntity->pos().x);
	ls.push((double) UserEntity->pos().y);
	ls.push((double) UserEntity->pos().z);
	return 3;
}

// *********************************************************************************************************
int CEditor::luaGetUserEntityFront(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetUserEntityFront)
	CHECK_EDITOR
	const char *funcName = "getUserEntityPosition";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	ls.push((double) UserEntity->front().x);
	ls.push((double) UserEntity->front().y);
	return 2;
}

// *********************************************************************************************************
int CEditor::luaRequestSetLocalNode(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaRequestSetLocalNode)
	CHECK_EDITOR
	const char *funcName = "requestSetLocalNode";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CObject *object = CComLuaModule::getObjectFromLua(ls.getStatePointer());
	if (!object)
	{
		CLuaIHM::fails(ls, "%s : can't read object from parameter 3", funcName);
	}
	getEditor().requestSetLocalNode(ls.toString(1), ls.toString(2), object);
	delete object;
	return 0;
}

// *********************************************************************************************************
int CEditor::luaRequestCommitLocalNode(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaRequestCommitLocalNode)
	CHECK_EDITOR
	const char *funcName = "requestCommitLocalNode";
	CLuaIHM::checkArgCount(ls, funcName, 2);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	getEditor().requestCommitLocalNode(ls.toString(1), ls.toString(2));
	return 0;
}

// *********************************************************************************************************
int CEditor::luaRequestRollbackLocalNode(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaRequestRollbackLocalNode)
	CHECK_EDITOR
	const char *funcName = "requestRollbackLocalNode";
	CLuaIHM::checkArgCount(ls, funcName, 2);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	getEditor().requestRollbackLocalNode(ls.toString(1), ls.toString(2));
	return 0;
}

// *********************************************************************************************************
int CEditor::luaSetCurrentActFromId(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSetCurrentActFromId)
	CHECK_EDITOR
	const char *funcName = "setCurrentActFromId";
	CLuaIHM::checkArgCount(ls, funcName, 2); // this is a method
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	CInstance *act = getEditor().getInstanceFromId(ls.toString(2));

	if (getEditor()._ClearingContent) { return 0; }
	if (!act)
	{
		nlwarning("%s : act with id %s not found", funcName, ls.toString(1));
		return 0;
	}
	if (!act->isKindOf("Act"))
	{
		nlwarning("%s : instance with id %s is not an act", funcName, ls.toString(1));
		return 0;
	}
	getEditor().setCurrentAct(act);
	return 0;
}

// *********************************************************************************************************
int CEditor::luaGetCurrentAct(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetCurrentAct)
	CHECK_EDITOR
	const char *funcName = "getCurrentAct";
	CLuaIHM::checkArgCount(ls, funcName, 1); // this is a method
	if (getEditor().getCurrentAct())
	{
		getEditor().projectInLua(getEditor().getCurrentAct()->getObjectTable());
	}
	else
	{
		ls.pushNil();
	}
	return 1;
}


// *********************************************************************************************************
int CEditor::luaGenInstanceName(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGenInstanceName)
	CHECK_EDITOR
	const char *funcName = "genInstanceName";
	CLuaIHM::checkArgCount(ls, funcName, 2); // this is a method
	CLuaIHM::checkArgTypeUCString(ls, funcName, 2); // name
	ucstring baseName;
	nlverify(CLuaIHM::getUCStringOnStack(ls, 2, baseName));
	CLuaIHM::push(ls, getEditor().genInstanceName(baseName));
	return 1;
}

// *********************************************************************************************************
int CEditor::luaIsPostFixedByNumber(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaIsPostFixedByNumber)
	CHECK_EDITOR
	const char *funcName = "isPostFixedByNumber";
	CLuaIHM::checkArgCount(ls, funcName, 2); // this is a method
	CLuaIHM::checkArgTypeUCString(ls, funcName, 2); // name
	ucstring baseName;
	nlverify(CLuaIHM::getUCStringOnStack(ls, 2, baseName));
	ls.push(getEditor().isPostFixedByNumber(baseName));
	return 1;
}

// *********************************************************************************************************
int CEditor::luaIsClearingContent(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaIsPostFixedByNumber)
	CHECK_EDITOR
	const char *funcName = "isClearingContent";
	CLuaIHM::checkArgCount(ls, funcName, 1); // this is a method
	ls.push(getEditor().isClearingContent());
	return 1;
}


// *********************************************************************************************************
int CEditor::luaSetCookie(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSetCookie)
	CHECK_EDITOR
	const char *funcName = "setCookie";
	CLuaIHM::checkArgCount(ls, funcName, 4); // this is a method (self + 3 params)
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING); // instance  id
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TSTRING); // key
	// last parameters may have any type
	CLuaObject value(ls); // pop it
	getEditor().setCookie(ls.toString(2), ls.toString(3), value);
	return 0;
}

// *********************************************************************************************************
int CEditor::luaIsCreature(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaIsCreature)
	CHECK_EDITOR
	const char *funcName = "isCreature";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING); // instance  sheetClient

	CInstance *inst = getEditor().getInstanceFromId(ls.toString(1));

	if (!inst) return 0;

	const CCharacterSheet *sheet = dynamic_cast<const CCharacterSheet *>(SheetMngr.get(NLMISC::CSheetId(inst->getSheet())));

	if (!sheet) return 0;

	bool isNPC = sheet->Race < EGSPD::CPeople::Creature;

	// neither NPC, not Kami, nor Karavan
	bool creature = !(isNPC || (!isNPC && (sheet->Race==EGSPD::CPeople::Kami || sheet->Race==EGSPD::CPeople::Karavan)));
	ls.push(creature);

	return 1;
}

// *********************************************************************************************************
int CEditor::luaSetEditorSeason(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSetEditorSeason)
	CHECK_EDITOR
	const char *funcName = "setEditorSeason";
	if (getEditor().getMode() != CEditor::EditionMode
		&& getEditor().getMode() != CEditor::AnimationModeLoading )
	{
		CLuaIHM::fails(ls, "%s : cannot set weather value outside of edit mode", funcName);
	}
	CLuaIHM::checkArgCount(ls, funcName, 2);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING); // this is a method (self + 1 param)
	if (strcmp(ls.toString(2), "Automatic") == 0) getEditor()._Season = Automatic;
	else if (strcmp(ls.toString(2), "Spring") == 0) getEditor()._Season = Spring;
	else if (strcmp(ls.toString(2), "Summer") == 0) getEditor()._Season = Summer;
	else if (strcmp(ls.toString(2), "Autumn") == 0) getEditor()._Season = Autumn;
	else if (strcmp(ls.toString(2), "Winter") == 0) getEditor()._Season = Winter;
	else CLuaIHM::fails(ls, "%s, invalid season name : %s", ls.toString(2));
	// If season is changed while scenario is being received, we do not have received the tp command yet,
	// so signal the weather system that the season should not be changed for now
	if ((getEditor()._UpdatingScenario && getEditor()._WillTP)
		|| getEditor().getMode() != CEditor::AnimationModeLoading )
	{
		getEditor()._IsWaitingTPForSeasonChange = true;
	}
	else
	{
		getEditor()._IsWaitingTPForSeasonChange = false;
	}
	return 0;
}

// *********************************************************************************************************
int CEditor::luaSetFixedLighting(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSetFixedLighting)
	const char *funcName = "setEditorSeason";
	CLuaIHM::checkArgCount(ls, funcName, 2);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TBOOLEAN); // this is a method (self + 1 param)
	getEditor().setFixedLighting(ls.toBoolean(2));
	return 0;
}

// *********************************************************************************************************
int CEditor::luaGetFixedLighting(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetFixedLighting)
	const char *funcName = "getEditorSeason";
	CLuaIHM::checkArgCount(ls, funcName, 1); // this is a method
	ls.push(getEditor().getFixedLighting());
	return 1;
}

// *********************************************************************************************************
int CEditor::luaSetPlotItemInfos(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaSetPlotItemInfos)
	const char *funcName = "setPlotItemInfos";
	CLuaIHM::checkArgCount(ls, funcName, 5); // a method with 4 args
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
	CLuaIHM::checkArgTypeUCString(ls, funcName, 3);
	CLuaIHM::checkArgTypeUCString(ls, funcName, 4);
	CLuaIHM::checkArgTypeUCString(ls, funcName, 5);
	CItemSheet *item = dynamic_cast<CItemSheet *>(SheetMngr.get(CSheetId((uint32) ls.toNumber(2))));
	if (!item || item->Family != ITEMFAMILY::SCROLL_R2)
	{
		CLuaIHM::fails(ls, "%s : bad sheet, r2 plot item required", funcName);
	}
	R2::TMissionItem mi;
	mi.SheetId = (uint32) ls.toNumber(2);
	CLuaIHM::getUCStringOnStack(ls, 3, mi.Name);
	CLuaIHM::getUCStringOnStack(ls, 4, mi.Description);
	CLuaIHM::getUCStringOnStack(ls, 5, mi.Comment);
	getEditor().setPlotItemInfos(mi);
	return 0;
}

// *********************************************************************************************************
int CEditor::luaIsCurrentSelectionPlayer(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaIsCurrentSelectionPlayer)
	const char *funcName = "isCurrentSelectionPlayer";
	CLuaIHM::checkArgCount(ls, funcName, 1); // method with 0 args
	CPlayerCL *player = dynamic_cast<CPlayerCL *>(EntitiesMngr.entity(UserEntity->getTargetSlotNoLag()));
	ls.push(player != NULL);
	return 1;
}

// *********************************************************************************************************
int CEditor::luaFindEmptyPlace(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaFindEmptyPlace)
	const char *funcName = "findEmptyPlace";
	CLuaIHM::checkArgCount(ls, funcName, 3); // method with 2 args
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TNUMBER);
	CVector result;
	if (getEditor().getIslandCollision().findEmptyPlace(CVector2f((float) ls.toNumber(2), (float) ls.toNumber(3)), result))
	{
		ls.push(result.x);
		ls.push(result.y);
		ls.push(result.z);
		return 3;
	}
	return 0;
}

// *********************************************************************************************************
int CEditor::luaIsInIslandRect(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaIsInIslandRect)
	const char *funcName = "isInIslandRect";
	CLuaIHM::checkArgCount(ls, funcName, 3); // method with 2 args
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TNUMBER);
	const R2::CScenarioEntryPoints::CCompleteIsland *island = getEditor().getIslandCollision().getCurrIslandDesc();
	if (!island)
	{
		ls.push(true);
		return 1;
	}
	float x = (float) ls.toNumber(2);
	float y = (float) ls.toNumber(3);
	ls.push(x >= island->XMin && y >= island->YMin &&
			x <= island->XMax && y <= island->YMax);
	return 1;
}

// *********************************************************************************************************
int CEditor::luaGetCurrentIslandName(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetCurrentIslandName)
	const char *funcName = "getCurrentIslandName";
	CLuaIHM::checkArgCount(ls, funcName, 1); // method with no args
	R2::CScenarioEntryPoints::CCompleteIsland *ci = getEditor().getIslandCollision().getCurrIslandDesc();
	ls.push(ci ? ci->Island.c_str() : "");
	return 1;
}

// *********************************************************************************************************
int CEditor::luaKickCharacter(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaKickCharacter)
	const char *funcName = "kickCharacter";
	CLuaIHM::checkArgCount(ls, funcName, 2); // this is a method (self + 1 params)
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);

	CSessionBrowserImpl	&sb = CSessionBrowserImpl::getInstance();
	sb.kickCharacter(sb.getCharId(), R2::getEditor().getDMC().getEditionModule().getCurrentAdventureId(),
		(uint32)ls.toNumber(2));

	if(!sb.waitOneMessage(sb.getMessageName("on_invokeResult")))
		nlwarning("kickCharacter callback return false");

	return 0;
}

// *********************************************************************************************************
int CEditor::luaUnkickCharacter(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaUnkickCharacter)
	const char *funcName = "unkickCharacter";
	CLuaIHM::checkArgCount(ls, funcName, 2); // this is a method (self + 1 params)
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);

	CSessionBrowserImpl	&sb = CSessionBrowserImpl::getInstance();
	sb.unkickCharacter(sb.getCharId(), R2::getEditor().getDMC().getEditionModule().getCurrentAdventureId(),
		(uint32)ls.toNumber(2));

	if(!sb.waitOneMessage(sb.getMessageName("on_invokeResult")))
		nlwarning("unkickCharacter callback return false");

	return 0;
}

// *********************************************************************************************************
int CEditor::luaTeleportToCharacter(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaTeleportToCharacter)
	const char *funcName = "teleportToCharacter";
	CLuaIHM::checkArgCount(ls, funcName, 2); // this is a method (self + 1 params)
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);

	CClientEditionModule & cem = R2::getEditor().getDMC().getEditionModule();
	cem.requestTeleportOneCharacterToAnother(cem.getCurrentAdventureId(), CSessionBrowserImpl::getInstance().getCharId(),
		(uint32)ls.toNumber(2));
	return 0;
}

// *********************************************************************************************************
int CEditor::luaWaitScenarioScreen(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaWaitScenarioScreen)
	const char *funcName = "waitScenarioScreen";
	CLuaIHM::checkArgCount(ls, funcName, 1); // method with no args
	if (!ClientCfg.Local)
	{
		getEditor()._WaitScenarioScreenWanted = true;
	}
	return 0;
}


// *********************************************************************************************************
int CEditor::luaIsScenarioUpdating(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaIsScenarioUpdating)
	const char *funcName = "isScenarioUpdating";
	CLuaIHM::checkArgCount(ls, funcName, 1); // method with no args
	ls.push( (double)getEditor()._UpdatingScenario  );
	return 1;
}


/*

// *********************************************************************************************************
int CEditor::luaIsValidPoly(CLuaState &ls)
{
//H_AUTO(R2_CEditor_luaIsValidPoly)
  const char *funcName = "isValidPoly";
	CLuaIHM::checkArgCount(ls, funcName, 2); // method with no args
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE); // method with no args
	ls.pushValue(3);
	CLuaObject poly;
	NLMISC::CPolygon2D poly2D;
	poly.pop(luaState);
	ENUM_LUA_TABLE(table, it)
	{
		NLMISC::CVector2f pos;
		if (!CLuaIHM::pop(ls, pos))
		{
			CLuaIHM::fails(ls, "%s expects CVector2f for poly coordinates);
		}
		poly2D.Vertices.push_back(pos);
	}
	ls.push(getEditor().getIslandCollision().isValidPoly(poly2D);
	return 1;
}

// *********************************************************************************************************
int CEditor::luaIsValidPosition(CLuaState &ls)
{
	CHECK_EDITOR
	const char *funcName = "snapPosToGround";
	CLuaIHM::checkArgCount(ls, funcName, 3);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TNUMBER);
	//
	// NLPACS::UGlobalPosition gpos = GR->retrievePosition(CVector((float) ls.toNumber(2), (float) ls.toNumber(3), 2000.f));
	if (gpos.InstanceId != -1)
	{
		CVector snappedPos = GR->getGlobalPosition(gpos);
		ls.push((double) snappedPos.x);
		ls.push((double) snappedPos.y);
		ls.push((double) snappedPos.z);
		return 3;
	}
	else
	{
		// default z to 0
		ls.push(0.f);
		return 3;
	}

	// new algo : to avoid to snapping to a cliff, start from the approximate (valid) height found in the height map
	CVector inter;
	if (CTool::computeWorldMapIntersection((float) ls.toNumber(2), (float) ls.toNumber(3), inter) != CTool::NoIntersection)
	{
		ls.push((double) inter.x);
		ls.push((double) inter.y);
		ls.push((double) inter.z);

}

*/

// *********************************************************************************************************
int CEditor::luaCanUndo(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaCanUndo)
	const char *funcName = "canUndo";
	CLuaIHM::checkArgCount(ls, funcName, 1); // method with no args
	ls.push(getEditor().getDMC().getActionHistoric().canUndo());
	return 1;
}

// *********************************************************************************************************
int CEditor::luaCanRedo(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaCanRedo)
	const char *funcName = "canRedo";
	CLuaIHM::checkArgCount(ls, funcName, 1); // method with no args
	ls.push(getEditor().getDMC().getActionHistoric().canRedo());
	return 1;
}

// *********************************************************************************************************
int CEditor::luaGetUserEntityName(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaGetUserEntityName)
	const char *funcName = "getUserEntityName";
	CLuaIHM::checkArgCount(ls, funcName, 1); // this is a method
	if (UserEntity)
	{
		ucstring name = UserEntity->getEntityName()+PlayerSelectedHomeShardNameWithParenthesis;
		ls.push( std::string( name.toUtf8() ) );
	}
	else
	{
		ls.push(std::string(""));
	}

	return 1;
}

static CLuaString lstr_Next("next");

// *********************************************************************************************************
int CEditor::luaEnumInstances(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaEnumInstances)
	CLuaStackChecker lsc(&ls, 1);
	// return an enumerator that allows to iterate over instance, by kind
	const char *funcName = "enumInstances";
	CLuaIHM::checkArgCount(ls, funcName, 2); // method with no args
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	struct CInstanceEnumerator
	{
		CSortedInstances *InstMap;
		TInstanceByName::iterator Current;
		static int next(CLuaState &ls)
		{
			const char *funcName = "InstanceEnumerator:next";
			CLuaIHM::checkArgCount(ls, funcName, 1);
			CLuaIHM::checkArgType(ls, funcName, 1, LUA_TUSERDATA);
			CInstanceEnumerator *ie = (CInstanceEnumerator *) ls.toUserData(1);
			CEditor &ed = getEditor();
			while(ie->Current != ie->InstMap->end())
			{
				CInstance *inst = ie->Current->second;
				if (!inst->getGhost())
				{
					// object must be in current act or base act
					CInstance *parentAct = inst->getParentAct();
					if (!parentAct || parentAct == ed.getCurrentAct() || parentAct == ed.getBaseAct())
					{
						inst->getLuaProjection().push();
						++ ie->Current;
						return 1;
					}
				}
				++ ie->Current;
			}
			return 0;
		}
		//
		static int gc(CLuaState &ls)
		{
			const char *funcName = "InstanceEnumerator:next";
			CLuaIHM::checkArgCount(ls, funcName, 1);
			CLuaIHM::checkArgType(ls, funcName, 1, LUA_TUSERDATA);
			CInstanceEnumerator *ie = (CInstanceEnumerator *) ls.toUserData(1);
			ie->~CInstanceEnumerator(); // no real effect for now...
			return 0;
		}
		static int index(CLuaState &ls)
		{
			const char *funcName = "InstanceEnumerator.__index";
			CLuaIHM::checkArgCount(ls, funcName, 2);
			CLuaIHM::checkArgType(ls, funcName, 1, LUA_TUSERDATA);
			CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
			if (ls.toString(2) == lstr_Next)
			{
				ls.push(CInstanceEnumerator::next);
				return 1;
			}
			return 0;
		}
	};
	sint classIndex = getEditor().classToIndex(ls.toString(2));
	if (classIndex < 0)
	{
		CLuaIHM::fails(ls, "Trying to iterate over unknown class : %s", ls.toString(2));
	}
	ls.newTable();
	CLuaObject mt;
	mt.pop(ls);
	ls.push(CInstanceEnumerator::index);
	mt.setValue("__index", CLuaObject(ls));
	ls.push(CInstanceEnumerator::gc);
	mt.setValue("__gc", CLuaObject(ls));
	//
	void *newIter = ls.newUserData(sizeof(CInstanceEnumerator));
	CInstanceEnumerator *ie = new (newIter) CInstanceEnumerator;

	ie->InstMap = &getEditor()._InstancesByDispName[classIndex];
	ie->Current = ie->InstMap->begin();
	mt.push();
	ls.setMetaTable(-2);
	//
	return 1;
}

// *********************************************************************************************************
void CEditor::waitScenarioScreen()
{
	//H_AUTO(R2_CEditor_waitScenarioScreen)
	if (ClientCfg.Local) return;
	if (_Mode==EditionMode)
	{
		setMode(GoingToEditionMode);
	}
	CWidgetManager::getInstance()->hideAllWindows();
	CInterfaceGroup *waitScreen = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_connecting"));
	if (waitScreen)
	{
		waitScreen->setActive(true);
		CWidgetManager::getInstance()->setTopWindow(waitScreen);
	}
	//
	enum TState { WaitingScenario, WaitingTP, DoExit };
	TState state = WaitingScenario;
	//
	getEditor()._ScenarioReceivedFlag = false;
	getEditor()._TPReceivedFlag = false;
	bool firewallTimeout = false;
	//
	ActionsContext.setContext("waiting_network");
	TGameCycle serverTick = NetMngr.getCurrentServerTick();
	CWidgetManager::getInstance()->setCaptureKeyboard(NULL);
	CWidgetManager::getInstance()->setDefaultCaptureKeyboard(NULL);
	loadBackgroundBitmap (StartBackground);

	// patch for the 'sys info that pop' prb (cause unknown for now ...)
	CInterfaceElement *sysInfo = CWidgetManager::getInstance()->getElementFromId("ui:interface:system_info");
	bool sysInfoActive = false;
	if (sysInfo) sysInfoActive = sysInfo->getActive();

	for (;;)
	{
		switch(state)
		{
			case WaitingScenario:
				if (getEditor()._ScenarioReceivedFlag)
				{
					if (getEditor()._WillTP)
					{
						state = WaitingTP;
					}
					else
					{
						state = DoExit;
					}
				}
			break;
			case WaitingTP:
				// no-op
			break;
			default:
				nlassert(0);
			break;
		}

		if (state == DoExit)
		{
			break;
		}

		// Update network.
		try
		{
			if ( ! firewallTimeout )
				NetMngr.update();
		}
		catch (const EBlockedByFirewall&)
		{
			if ( NetMngr.getConnectionState() == CNetManager::Disconnect )
			{
				firewallTimeout = true;
			}
			else
			{
				// Display the firewall alert string
				CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_connecting:title"));
				if (pVT != NULL)
					pVT->setText(CI18N::get("uiFirewallAlert")+ucstring("..."));

				// The mouse and fullscreen mode should be unlocked for the user to set the firewall permission
				nlSleep( 30 ); // 'nice' the client, and prevent to make too many send attempts
			}

		}
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();

		// check if we can send another dated block
		if (NetMngr.getCurrentServerTick() != serverTick)
		{
			//
			serverTick = NetMngr.getCurrentServerTick();
			NetMngr.send(serverTick);
		}
		else
		{
			// Send dummy info
			NetMngr.send();
		}

		// update module manager
		NLNET::IModuleManager::getInstance().updateModules();


		// Update the DT T0 and T1 global variables
		updateClientTime();
		CInputHandlerManager::getInstance()->pumpEvents();
		Driver->clearBuffers(CRGBA::Black);
		Driver->setMatrixMode2D11();
		drawLoadingBitmap (1);

		if ( state==WaitingTP && getEditor()._TPReceivedFlag)
		{
			break; // don't want an additionnal swap buffer here, so exit now (the loading bitmap is good,
													// has just been reloaded by the tp screen...
		}



		// Interface handling & displaying (processes clicks...)
		getUI().updateFrameEvents();

		if (waitScreen)
		{
			CWidgetManager::getInstance()->setTopWindow(waitScreen);
		}

		if (sysInfo) sysInfo->setActive(false);

		getUI().updateFrameViews(NULL);
		IngameDbMngr.flushObserverCalls();
		NLGUI::CDBManager::getInstance()->flushObserverCalls();

		// Movie shooter
		globalMenuMovieShooter();

		// Force the client to sleep a bit.
		if(ClientCfg.Sleep >= 0)
		{
			nlSleep(ClientCfg.Sleep);
		}

		//
		if (!waitScreen)
		{
			TextContext->setShaded(true);
			TextContext->setFontSize(40);
			TextContext->setColor(CRGBA::White);

			// TOP LEFT //
			//----------//
			TextContext->setHotSpot(NL3D::UTextContext::MiddleMiddle);
			TextContext->printfAt(0.5f, 0.5f, "(UI NOT FOUND) Waiting scenario from server.");
		}


		// Display
		Driver->swapBuffers();
		//
		if (NetMngr.getConnectionState() == CNetManager::Disconnect)
		{
			if ( firewallTimeout )
			{
				// Display the firewall error string instead of the normal failure string
				CViewText *pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_connecting:title"));
				if (pVT != NULL)
				{
					pVT->setMultiLine( true );
					pVT->setText(CI18N::get("uiFirewallFail")+ucstring(".\n")+
								  CI18N::get("uiFirewallAlert")+ucstring("."));
				}
			}
		}
	}
	if (sysInfo) sysInfo->setActive(sysInfoActive);
	destroyLoadingBitmap ();
	if (_Mode == GoingToEditionMode)
	{
		setMode(EditionMode);
	}
	return;
}

// *********************************************************************************************************
// lua instance observer
class CInstanceObserverLua : public CEditor::IInstanceObserver
{
public:
	CInstanceObserverLua(CLuaObject &receiver);
	~CInstanceObserverLua();
	CLuaObject &getReceiver() { return _Receiver; }
	// from IInstanceObserver
	virtual void onInstanceCreated(CInstance &instance);
	virtual void onInstanceErased(CInstance &instance);
	virtual void onInstancePreHrcMove(CInstance &instance);
	virtual void onInstancePostHrcMove(CInstance &instance);
	virtual void onPreHrcMove(CInstance &instance);
	virtual void onPostHrcMove(CInstance &instance);
	virtual void onAttrModified(CInstance &instance, const std::string &attrName, sint32 attrIndex);
private:
	CLuaObject _Receiver; // table that will receive the notifications
	static uint _Count;
};


uint CInstanceObserverLua::_Count = 0;


CInstanceObserverLua::CInstanceObserverLua(CLuaObject &receiver) : _Receiver(receiver)
{
	++ _Count;
}

CInstanceObserverLua::~CInstanceObserverLua()
{
	#ifdef NL_DEBUG
		nlassert(!getEditor().isInstanceObserver(this));
	#endif
	nlassert(_Count < 1000000000);
	-- _Count;
}


void CInstanceObserverLua::onInstanceCreated(CInstance &instance)
{
	//H_AUTO(R2_CInstanceObserverLua_onInstanceCreated)
	nlassert(_Receiver.isValid());
	if (_Receiver["onInstanceCreated"].isNil()) return; // no-op if not handled
	getEditor().projectInLua(instance.getObjectTable());
	_Receiver.callMethodByNameNoThrow("onInstanceCreated", 1, 0);
}

void CInstanceObserverLua::onInstanceErased(CInstance &instance)
{
	//H_AUTO(R2_CInstanceObserverLua_onInstanceErased)
	nlassert(_Receiver.isValid());
	if (_Receiver["onInstanceErased"].isNil()) return; // no-op if not handled
	getEditor().projectInLua(instance.getObjectTable());
	_Receiver.callMethodByNameNoThrow("onInstanceErased", 1, 0);
}

void CInstanceObserverLua::onInstancePreHrcMove(CInstance &instance)
{
	//H_AUTO(R2_CInstanceObserverLua_onInstancePreHrcMove)
	nlassert(_Receiver.isValid());
	if (_Receiver["onInstanceCreated"].isNil()) return; // no-op if not handled
	getEditor().projectInLua(instance.getObjectTable());
	_Receiver.callMethodByNameNoThrow("onInstancePreHrcMove", 1, 0);
}

void CInstanceObserverLua::onInstancePostHrcMove(CInstance &instance)
{
	//H_AUTO(R2_CInstanceObserverLua_onInstancePostHrcMove)
	nlassert(_Receiver.isValid());
	if (_Receiver["onInstanceCreated"].isNil()) return; // no-op if not handled
	getEditor().projectInLua(instance.getObjectTable());
	_Receiver.callMethodByNameNoThrow("onInstancePostHrcMove", 1, 0);
}

void CInstanceObserverLua::onPreHrcMove(CInstance &instance)
{
	//H_AUTO(R2_CInstanceObserverLua_onPreHrcMove)
	nlassert(_Receiver.isValid());
	if (_Receiver["onPreHrcMove"].isNil()) return; // no-op if not handled
	getEditor().projectInLua(instance.getObjectTable());
	_Receiver.callMethodByNameNoThrow("onPreHrcMove", 1, 0);
}

void CInstanceObserverLua::onPostHrcMove(CInstance &instance)
{
	//H_AUTO(R2_CInstanceObserverLua_onPostHrcMove)
	nlassert(_Receiver.isValid());
	if (_Receiver["onPostHrcMove"].isNil()) return; // no-op if not handled
	getEditor().projectInLua(instance.getObjectTable());
	_Receiver.callMethodByNameNoThrow("onPostHrcMove", 1, 0);
}

void CInstanceObserverLua::onAttrModified(CInstance &instance, const std::string &attrName, sint32 attrIndex)
{
	//H_AUTO(R2_CInstanceObserverLua_onAttrModified)
	nlassert(_Receiver.isValid());
	if (_Receiver["onAttrModified"].isNil()) return; // no-op if not handled
	getEditor().projectInLua(instance.getObjectTable());
	getEditor().getLua().push(attrName);
	getEditor().getLua().push((double) attrIndex);
	_Receiver.callMethodByNameNoThrow("onAttrModified", 3, 0);
}

// *********************************************************************************************************
int CEditor::luaAddInstanceObserver(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaAddInstanceObserver)
	CHECK_EDITOR
	const char *funcName = "addInstanceObserver";
	CLuaIHM::checkArgCount(ls, funcName, 3); // this is a method (self + 2 params)
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING); // instance  id
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE); // receiver
	CLuaObject receiver(ls); // pop the receiver
	ls.push((double) getEditor().addInstanceObserver(ls.toString(2), new CInstanceObserverLua(receiver)));
	return 1;
}

// *********************************************************************************************************
int CEditor::luaRemoveInstanceObserver(CLuaState &ls)
{
	//H_AUTO(R2_CEditor_luaRemoveInstanceObserver)
	CHECK_EDITOR
	const char *funcName = "removeInstanceObserver";
	CLuaIHM::checkArgCount(ls, funcName, 2); // this is a method (self + 1 params)
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER); // instance  id
	IInstanceObserver *observer = getEditor().getInstanceObserver((TInstanceObserverHandle) ls.toNumber(2));
	if (observer == NULL)
	{
		CLuaIHM::fails(ls, "Instance observer not found for handle = %d", (int) ls.toNumber(2));
	}
	CInstanceObserverLua *luaObserver = dynamic_cast<CInstanceObserverLua *>(observer);
	if (luaObserver == NULL)
	{
		CLuaIHM::fails(ls, "Instance observer found for handle %d, but has bad type, it wasn't registered from lua.", (int) ls.toNumber(2));
	}
	getEditor().removeInstanceObserver((TInstanceObserverHandle) ls.toNumber(2));
	CLuaObject receiver = luaObserver->getReceiver();
	delete luaObserver;
	receiver.push();
	return 1;
}

// *********************************************************************************************************
bool CEditor::isInstanceObserver(IInstanceObserver *observer) const
{
	//H_AUTO(R2_CEditor_isInstanceObserver)
	for(TInstanceObserverMap::const_iterator it = _InstanceObservers.begin(); it != _InstanceObservers.end(); ++it)
	{
		if (it->second == observer) return true;
	}
	return false;
}


// *********************************************************************************************************
const CObjectTable *CEditor::getObjectTableFromId(const TInstanceId &id) const
{
	//H_AUTO(R2_CEditor_getObjectTableFromId)
	CHECK_EDITOR
	nlassert(_DMC);
	const CObject *obj = _DMC->find(id);
	if (!obj) return NULL;
	nlassert(obj->isTable());
	return (CObjectTable *) obj;
}

// *********************************************************************************************************
CInstance *CEditor::getInstanceFromObject(const CObject *obj) const
{
	//H_AUTO(R2_CEditor_getInstanceFromObject)
	CHECK_EDITOR
	if (!obj->isTable()) return NULL;
	TInstanceMap::const_iterator it = _Instances.find((CObjectTable *) obj);
	if (it != _Instances.end())
	{
		return it->second;
	}
	return NULL;
}


// *********************************************************************************************************
void CEditor::registerEnvMethod(const char *name,TLuaWrappedFunction func)
{
	//H_AUTO(R2_CEditor_registerEnvMethod)
	CHECK_EDITOR
	nlassert(name);
	getEnv().setValue(name, func);
}

// *********************************************************************************************************
void CEditor::registerEnvFunction(const char *name, TLuaWrappedFunction func)
{
	//H_AUTO(R2_CEditor_registerEnvFunction)
	CHECK_EDITOR
	nlassert(name);
	getEnv().setValue(name, func);
}


// *********************************************************************************************************
void CEditor::registerLuaFunc()
{
	//H_AUTO(R2_CEditor_registerLuaFunc)
	CHECK_EDITOR
	registerEnvMethod("getSelectedInstanceId", luaGetSelectedInstanceId);
	registerEnvMethod("setSelectedInstanceId", luaSetSelectedInstanceId);
	registerEnvMethod("setCurrentTool", luaSetCurrentTool);
	registerEnvMethod("getCurrentTool", luaGetCurrentTool);
	registerEnvMethod("getVisualPropertiesFromInstanceId", luaGetVisualPropertiesFromInstanceId);
	registerEnvMethod("getInstanceFromId", luaGetInstanceFromId);
	registerEnvMethod("displayContextMenu", luaDisplayContextMenu);
	registerEnvMethod("getSelectedInstance", luaGetSelectedInstance);
	registerEnvMethod("connectAsCreator", luaConnectAsCreator);
	registerEnvMethod("doFile", luaDofile);
	registerEnvMethod("tryFile", luaTryFile);
	registerEnvMethod("setEntityCustomSelectBox", luaSetEntityCustomSelectBox);
	registerEnvMethod("getEntityCustomSelectBox", luaGetEntityCustomSelectBox);
	registerEnvMethod("choosePos", luaChoosePos);
	registerEnvMethod("snapPosToGround", luaSnapPosToGround);
	registerEnvMethod("getUserEntityPosition", luaGetUserEntityPosition);
	registerEnvMethod("getUserEntityFront", luaGetUserEntityFront);
	registerEnvMethod("setCurrentActFromId", luaSetCurrentActFromId);
	registerEnvMethod("getCurrentAct", luaGetCurrentAct);
	registerEnvMethod("genInstanceName", luaGenInstanceName);
	registerEnvMethod("isPostFixedByNumber", luaIsPostFixedByNumber);
	registerEnvMethod("setCookie", luaSetCookie);
	registerEnvMethod("addInstanceObserver", luaAddInstanceObserver);
	registerEnvMethod("removeInstanceObserver", luaRemoveInstanceObserver);
	registerEnvFunction("requestSetLocalNode", luaRequestSetLocalNode);
	registerEnvFunction("requestCommitLocalNode", luaRequestCommitLocalNode);
	registerEnvFunction("requestRollbackLocalNode", luaRequestRollbackLocalNode);
	registerEnvMethod("isCreature", luaIsCreature);
	registerEnvMethod("setEditorSeason", luaSetEditorSeason);
	registerEnvMethod("setFixedLighting", luaSetFixedLighting);
	registerEnvMethod("getFixedLighting", luaGetFixedLighting);
	registerEnvMethod("setPlotItemInfos", luaSetPlotItemInfos);
	registerEnvMethod("isCurrentSelectionPlayer", luaIsCurrentSelectionPlayer);
	registerEnvMethod("findEmptyPlace", luaFindEmptyPlace);
	registerEnvMethod("isInIslandRect", luaIsInIslandRect);
	registerEnvMethod("getCurrentIslandName", luaGetCurrentIslandName);
	registerEnvMethod("waitScenarioScreen", luaWaitScenarioScreen);
	registerEnvMethod("isScenarioUpdating", luaIsScenarioUpdating);
	registerEnvMethod("canUndo", luaCanUndo);
	registerEnvMethod("canRedo", luaCanRedo);
	registerEnvMethod("getUserEntityName", luaGetUserEntityName);
	registerEnvMethod("getStartingAnimationFilename", luaGetStartingAnimationFilename);
	registerEnvMethod("kickCharacter", luaKickCharacter);
	registerEnvMethod("unkickCharacter", luaUnkickCharacter);
	registerEnvMethod("teleportToCharacter", luaTeleportToCharacter);
	registerEnvMethod("enumInstances", luaEnumInstances);
	registerEnvMethod("isClearingContent", luaIsClearingContent);




}

/*
static void polyUnitTest()
{
	CPolygon2D poly;
	poly.Vertices.push_back(CVector2f(0.f, 0.f));
	poly.Vertices.push_back(CVector2f(1.f, 0.f));
	poly.Vertices.push_back(CVector2f(1.f, 1.f));
	poly.Vertices.push_back(CVector2f(0.f, 1.f));
	nlassert(poly.contains(CVector2f(0.f, 0.f), false));
	nlassert(poly.contains(CVector2f(1.f, 0.f), false));
	nlassert(poly.contains(CVector2f(1.f, 1.f), false));
	nlassert(poly.contains(CVector2f(0.f, 1.f), false));
	nlassert(poly.contains(CVector2f(0.5f, 0.f), false))
	nlassert(poly.contains(CVector2f(1.f, 0.5f), false));
	nlassert(poly.contains(CVector2f(0.5f, 1.f), false));
	nlassert(poly.contains(CVector2f(0.f, 0.5f), false));
	nlassert(poly.contains(CVector2f(0.5f, 0.5f), false));
	//
	nlassert(!poly.contains(CVector2f(-0.5f, 0.5f), false));
	nlassert(!poly.contains(CVector2f(1.5f, 0.5f), false));
	nlassert(!poly.contains(CVector2f(0.5f, -0.5f), false));
	nlassert(!poly.contains(CVector2f(0.5f, 1.5f), false));
}
*/

// *********************************************************************************************************
void CEditor::loadLanguageFile()
{
	//H_AUTO(R2_CEditor_loadLanguageFile)
	CHECK_EDITOR
	static bool reload = false;
	CI18N::ILoadProxy *oldLoadProxy = CI18N::getLoadProxy();
	CI18N::setLoadProxy(NULL);
	string lc = ClientCfg.LanguageCode;
	nlinfo("Reloading the r2_%s.uxt", lc.c_str());
	CI18N::loadFromFilename("r2_"+lc+".uxt", reload);
	CI18N::setLoadProxy(oldLoadProxy);
	reload = true;
}


// *********************************************************************************************************
void CEditor::loadKeySet(const std::string &keySet)
{
	//H_AUTO(R2_CEditor_loadKeySet)
	CHECK_EDITOR
	if (keySet.empty()) return;
	CVerboseClock clock("Parsing of keyset " + keySet);
	// remove all existing keys
	ActionsContext.removeAllCombos();
	CMacroCmdManager::getInstance()->removeAllMacros();

	// parse keys that are specific to r2ed
	std::vector<string> xmlFilesToParse;

	// Does the r2ed keys file exist ?
	std::string userKeyFileName = "save/" + keySet + "_"+PlayerSelectedFileName+".xml";
	if (CFile::fileExists(userKeyFileName) && CFile::getFileSize(userKeyFileName) > 0)
	{
		// Load the user key file
		xmlFilesToParse.push_back (userKeyFileName);
	}
	// Load the default key (but don't replace existings bounds, see keys.xml "key_def_no_replace")
	xmlFilesToParse.push_back (keySet + ".xml");

	if (!CInterfaceManager::getInstance()->parseInterface (xmlFilesToParse, true))
	{
		badXMLParseMessageBox();
	}

	CActionsManager *actionManager = ActionsContext.getActionsManager();
	if (actionManager)
	{
		// if some mode are unavailable, forbid the associated shortcuts
		/*if (!_ModeEnabled[EditionMode])
		{
			actionManager->removeBaseAction("r2ed_stop_test");
		}
		if (!_ModeEnabled[TestMode] || !_ModeEnabled[DMMode])
		{
			actionManager->removeBaseAction("r2ed_try_go_test");
		}*/
	}
}

// *********************************************************************************************************
void CEditor::saveCurrentKeySet()
{
	//H_AUTO(R2_CEditor_saveCurrentKeySet)
	CHECK_EDITOR
	std::string prefix = getKeySetPrefix(getMode());
	if (prefix.empty()) return;
	getUI().saveKeys ("save/" + prefix + "_" + PlayerSelectedFileName + ".xml");
}

// *********************************************************************************************************
std::string CEditor::getKeySetPrefix(TMode mode)
{
	//H_AUTO(R2_CEditor_getKeySetPrefix)
	CHECK_EDITOR
	switch(mode)
	{
		case EditionMode: return "keys_r2ed";
		case GoingToDMMode:
		case GoingToEditionMode:
		case AnimationModeLoading:
		case AnimationModeWaitingForLoading:
		case AnimationModeGoingToDm:
		case AnimationModeGoingToPlay:
			return "";
		case AnimationModePlay:
			return "keys";
		default:
			return "keys";
	}
	return std::string();
}

// *********************************************************************************************************
void CEditor::setUIMode(uint8 mode)
{
	//H_AUTO(R2_CEditor_setUIMode)
	CHECK_EDITOR
	getUI().setMode(mode);
	if (_ForceDesktopReset[mode])
	{
		// force to call reset when reloading the ui
		getLua().push((double) mode);
		callEnvMethod("resetDesktop", 1, 0);
		_ForceDesktopReset[mode] = false;
	}
	getLua().push((double) mode);
	callEnvMethod("onChangeDesktop", 1, 0);
}


// *********************************************************************************************************
void CEditor::loadStandardUI()
{
	//H_AUTO(R2_CEditor_loadStandardUI)
	getUI().setMode(0);
	// force to reload the standard interface
	ClientCfg.R2EDEnabled = false;
	loadUIConfig("");
	ClientCfg.R2EDEnabled = true;
	CWidgetManager::getInstance()->updateAllLocalisedElements();
}

// *********************************************************************************************************
bool CEditor::isDMing() const
{
	//H_AUTO(R2_CEditor_isDMing)
	if (getMode() == AnimationModePlay) return false; // masterless mode -> not a dm
	return getMode() == DMMode ||
		   getMode() == AnimationModeDm ||
		   (
				getMode() != NotInitialized &&
				getAccessMode() == AccessDM
		   );
}

// *********************************************************************************************************
void CEditor::setMode(TMode mode)
{
	switch(mode)
	{
		case NotInitialized:				 nlwarning("*R2* Setting mode to NotInitialized"); break;
		case EditionMode:					 nlwarning("*R2* Setting mode to EditionMode"); break;
		case GoingToDMMode:					 nlwarning("*R2* Setting mode to GoingToDMMode"); break;
		case GoingToEditionMode:			 nlwarning("*R2* Setting mode to GoingToEditionMode"); break;
		case TestMode:						 nlwarning("*R2* Setting mode to TestMode"); break;
		case DMMode:						 nlwarning("*R2* Setting mode to DMMode"); break;
		case AnimationModeLoading:			 nlwarning("*R2* Setting mode to AnimationModeLoading"); break;
		case AnimationModeWaitingForLoading: nlwarning("*R2* Setting mode to AnimationModeWaitingForLoading"); break;
		case AnimationModeDm:				 nlwarning("*R2* Setting mode to AnimationModeDm"); break;
		case AnimationModePlay:				 nlwarning("*R2* Setting mode to AnimationModePlay"); break;
		case AnimationModeGoingToDm:		 nlwarning("*R2* Setting mode to AnimationModeGoingToDm"); break;
		case AnimationModeGoingToPlay:		 nlwarning("*R2* Setting mode to AnimationModeGoingToPlay"); break;
		default:
			nlwarning("Setting mode to 'unknown'");
		break;
	}

	if (mode != EditionMode)
	{
		delete _EntitySorter;
		_EntitySorter = NULL;
	}
	//H_AUTO(R2_CEditor_setMode)
	CHECK_EDITOR
	//nlassert(_ModeEnabled[mode]);
	if (mode == _Mode && !_ForceDesktopReset[mode]) return;


	if (mode != _Mode)
	{
		if ((_Mode == AnimationModeDm || _Mode == AnimationModePlay)
			&& !((mode == AnimationModeDm || mode == AnimationModePlay)))
		{
			// remove the chat windows when leaving animation mode
			PeopleInterraction.removeAllFreeTellers();

		}


		CVerboseClock clock("Saving windows & key at a mode change ...");
		if (_Mode != NotInitialized)
		{
			saveCurrentKeySet();
		}
		//if (_Mode != NotInitialized) saveUIConfig();
		if (_Mode == EditionMode || _Mode == AnimationModeLoading)
		{
			getDMC().getActionHistoric().clear(); // no backup of undo/redo for now
			clearContent(); // when leaving edition, remove content from tree
		}
		else if (mode == EditionMode || mode == GoingToEditionMode)
		{
			removeAllEntitySlots(); // when leaving test or dm, remove the entities slots
		}

		if (mode == DMMode || mode == AnimationModeDm)
		{
			getEditor().getDMC().getEditionModule().askUpdateCharMode(TCharMode::Dm);
		}
		else if (mode == EditionMode)
		{
			getEditor().getDMC().getEditionModule().askUpdateCharMode(TCharMode::Editer);
		}
		else if (mode == AnimationModePlay)
		{
			if(_Mode != NotInitialized)
			{
				if (_SerializeUIConfig)
				{
					saveUIConfig();
				}
			}
			getEditor().getDMC().getEditionModule().askUpdateCharMode(TCharMode::Player);
			// if access mode is DM, then load the whole ui
			if (_AccessMode == AccessDM)
			{
				loadStandardUI();
			}
		}
		else if (mode == TestMode)
		{
			getEditor().getDMC().getEditionModule().askUpdateCharMode(TCharMode::Tester);
		}
		else // Other like going to dm ...
		{
		}


	}
	// reset any selection
	if ((!IngameDbMngr.initInProgress()) && UserEntity) // prevent from calling selection() if coming from a FarTP, otherwise the server will mess up the action counter because it can't know our datasetrow so early (initInProgress(): heuristic to ensure the setTarget() will work on the EGS)
	{
		UserEntity->selection(CLFECOMMON::INVALID_SLOT);
	}
	//
	ContextCur.release();
	_Mode = mode;
	loadKeySet(getKeySetPrefix(_Mode));
	CWidgetManager::getInstance()->disableModalWindow();
	CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
	CWidgetManager::getInstance()->setCapturePointerRight(NULL);
	CWidgetManager::getInstance()->setCaptureKeyboard(NULL);
	// Season is now unknown, until server force it (in test mode), or first set act set it (in edit mode)
	_Season = UnknownSeason;
	//
	switch(_Mode)

	{
		case EditionMode:
			if (!_EntitySorter)
			{
				_EntitySorter = new CEntitySorter;
			}
			ActionsContext.setContext("r2ed");
			// set new mode in lua
			_Env.setValue("Mode", "Edit");
			setUIMode(0);
			::IgnoreEntityDbUpdates = true;
			::SlotUnderCursor = CLFECOMMON::INVALID_SLOT;
			/**
			  * nb we manage mouse bitmap ourselves, so there's only one cursor context here, and it never changes:
			  * its sole purpose is to route messages to the current tool
			  */
			ContextCur.add(false,	"STAND BY",		DEFAULT_CURSOR,			0.0f,	CEditor::checkCursor,	CEditor::mouseClick);
			forceSetSelectedInstance(NULL);
			initReferencePlotItems();
			// force speed to "run" ("walk" not available in edition)
			if (!UserEntity->running())
			{
				UserEntity->switchVelocity();
			}
		break;
		case TestMode:
			ActionsContext.setContext("r2ed_anim_test");
			// set new mode in lua
			_Env.setValue("Mode", "Test");
			setUIMode(1);
			::IgnoreEntityDbUpdates = false;
			::initContextualCursor();
			nlassert(CDisplayerBase::ObjCount == 0);
			resetPlotItems();
		break;
		case DMMode:
			ActionsContext.setContext("r2ed_anim_dm");
			// set new mode in lua
			_Env.setValue("Mode", "DM");
			setUIMode(2);
			::IgnoreEntityDbUpdates = false;
			::initContextualCursor();
			nlassert(CDisplayerBase::ObjCount == 0);
			// when in local mode, init fake plot item to test the 'dm gift' interface
			initDummyPlotItems();
		break;
		case GoingToDMMode:
			ActionsContext.setContext("waiting_network");
			// set new mode in lua
			_Env.setValue("Mode", "GoingToDM");
			setUIMode(3);
			connexionMsg(_ConnexionMsg); // update connexion window
			::IgnoreEntityDbUpdates = false;
			::initContextualCursor();
			nlassert(CDisplayerBase::ObjCount == 0);
			resetPlotItems();
		break;
		case GoingToEditionMode:
			nlassert(CDisplayerBase::ObjCount == 0);
			ActionsContext.setContext("waiting_network");
			// set new mode in lua
			_Env.setValue("Mode", "BackToEditing");
			setUIMode(3);
			connexionMsg(_ConnexionMsg); // update connexion window
			::IgnoreEntityDbUpdates = true;
			::initContextualCursor();
			resetPlotItems();
		break;

		case AnimationModeLoading:
			nlassert(CDisplayerBase::ObjCount == 0);
			ActionsContext.setContext("waiting_network");
			// set new mode in lua
			_Env.setValue("Mode", "AnimationModeLoading");
			setUIMode(3);
			connexionMsg(_ConnexionMsg); // update connexion window
//			::IgnoreEntityDbUpdates = true;
			::initContextualCursor();
			resetPlotItems();
		break;

		case AnimationModeWaitingForLoading:
			nlassert(CDisplayerBase::ObjCount == 0);
			ActionsContext.setContext("waiting_network");
			// set new mode in lua
			_Env.setValue("Mode", "AnimationModeWaitingForLoading");
			setUIMode(3);
			connexionMsg(_ConnexionMsg); // update connexion window
//			::IgnoreEntityDbUpdates = true;
			::initContextualCursor();
			resetPlotItems();
		break;

		case AnimationModeDm:
			PeopleInterraction.updateAllFreeTellerHeaders(); // if invitation were received during loading, add 'invite' button to the new windows
			nlassert(CDisplayerBase::ObjCount == 0);
			ActionsContext.setContext("r2ed_anim_dm");
			// set new mode in lua
			_Env.setValue("Mode", "AnimationModeDm");
			setUIMode(2);
			::IgnoreEntityDbUpdates = false;
			::initContextualCursor();
			resetPlotItems();
		break;

		case AnimationModePlay:
		{
			// animation mode play is still visible in masterless mode
			nlassert(CDisplayerBase::ObjCount == 0);
			// no op here, just a player
			ActionsContext.setContext("game");
			//ActionsContext.setContext("r2ed_anim_test");
			// set new mode in lua
			_Env.setValue("Mode", "r2ed_anim_test");
			// hide all ring windows that may remain
			hideRingWindows();
			// setUIMode(1); // don't change desktop here, because virtual desktops are allowed in this mode
			getUI().setMode(0); // default to desktop 0 as with standard game
			::IgnoreEntityDbUpdates = false;
			::initContextualCursor();
			resetPlotItems();
			if (getDMC().getEditionModule().isSessionOwner())
			{
				// if we are not a player, pop the ui to pop control player window & to quit
				callEnvMethod("popDMToolbarWindow", 0, 0);
			}
			else
			{
				callEnvMethod("playerModeUIFix", 0, 0);
			}
		}
		break;

		case AnimationModeGoingToDm:
			nlassert(CDisplayerBase::ObjCount == 0);
			ActionsContext.setContext("waiting_network");
			//set new mode in lua
			_Env.setValue("Mode", "AnimationModeGoingToDM");
			setUIMode(3);
			connexionMsg(_ConnexionMsg); // update connexion window
			::IgnoreEntityDbUpdates = false;
			::initContextualCursor();
			resetPlotItems();
		break;



		default:
			nlassert(0);
		break;
	}
	ClientCfg.ManualWeatherSetup = false;
	ContextCur.context("STAND BY");
	_Mode = mode;

	// warn lua that mode has changed
	callEnvMethod("onModeChanged", 0, 0);
	//
	setCurrentTool(NULL);

	// display the bars over the player (not available during edition, but available at test time)
	UserEntity->buildInSceneInterface();

	if (_Mode == EditionMode)
	{
		// special context menu for edition
		GameContextMenu.init("game_context_menu_edition");
	}
	else
	{
		GameContextMenu.init("");
	}
}


// *********************************************************************************************************
void CEditor::hideRingWindows()
{
	//H_AUTO(R2_CEditor_hideRingWindows)
	static const char *ringWindows[] =
	{
		"r2ed_palette",
		"r2ed_connect",
		"r2ed_property_sheet_no_selection",
		"r2ed_property_sheet_no_properties",
		"r2ed_bbox_edit",
		"r2ed_toolbar",
		"r2ed_windows_dm_bar",
		"r2ed_toolbar_window",
		"r2ed_windows_bar",
		"r2ed_main_bl",
		"r2ed_animation_loading",
		"r2ed_animation_waiting",
		"dm_controlled_entities",
		"r2ed_current_session",
		"r2ed_toolbar_admin",
		"lua_inspector",
		"r2ed_npc",
		"r2ed_select_bar",
		"r2ed_main_menu_button",
		"r2ed_contextual_toolbar_new",
		"feature_help",
		"r2ed_logic_entities",
		"r2ed_events",
		"r2ed_activities",
		"r2ed_edit_activity_sequence",
		"r2ed_dialogs",
		"r2ed_edit_chat_sequence",
		"r2ed_mini_activity_view",
		"r2ed_acts",
		"r2ed_connecting",
		"r2ed_scenario",
		"r2ed_dm_gift",
		"r2ed_scenario_filter",
		"r2ed_testbar"
	};
	for (uint k = 0; k < sizeofarray(ringWindows); ++k)
	{
		std::string id = "ui:interface:" + std::string(ringWindows[k]);
		CInterfaceElement *grp = CWidgetManager::getInstance()->getElementFromId(id);
		if (grp)
		{
			grp->setActive(false);
		}
		else
		{
			nlwarning("Can't find group with id : %s", id.c_str());
		}
	}
}

// *********************************************************************************************************
void CEditor::init(TMode initialMode, TAccessMode accessMode)
{
	nlwarning("*R2* init");
	_EntitySorter = NULL;
	//H_AUTO(R2_CEditor_init)
	CNiceInputAuto niceInputs;
	nlassert((uint) initialMode < ModeCount);
	nlassert((uint) accessMode < AccessModeCount);
	//nlassert(_ModeEnabled[initialMode]; // mode not enabled ...
	CHECK_EDITOR
	CVerboseClock clock("Init of editor ");
	_Initialized = true;
	_AccessMode = accessMode;

	// add a list of files for which modifications will trigger R2ED reset
	for(uint k = 0; k < ClientCfg.R2EDReloadFiles.size(); ++k)
	{
		CFile::removeFileChangeCallback(ClientCfg.R2EDReloadFiles[k]); // avoid to add twice when a reset ...
		CFile::addFileChangeCallback(ClientCfg.R2EDReloadFiles[k], reloadEditorCallback);
	}
	//
	CLuaStackChecker lsc(&getLua());
	getLua().pushGlobalTable();
	_Globals.pop(getLua());
	getLua().pushValue(LUA_REGISTRYINDEX);
	_Registry.pop(getLua());
	// create R2 environment, and keep a reference on it
	_Env = _Globals.newTable(R2_LUA_PATH);
	_Config = _Env.newTable("Config");
	//
	_Env.setValue("InClient", true); // TMP : signal to the script that it is initialised by the client
	//
	switch (accessMode)
	{
		case AccessEditor:			_Env.setValue("AccessMode", "Editor"); break;
		case AccessDM:				_Env.setValue("AccessMode", "DM"); break;
		case AccessOutlandOwner:	_Env.setValue("AccessMode", "OutlandOwner"); break;
		default:
			nlassert(0);
		break;
	}

	// Load the UI files
	if (ReloadUIFlag)
	{
		loadLanguageFile();
		reloadUI();
		// setForceDesktopReset is done below
		ReloadUIFlag = false;
	}

	//
	registerDisplayers();
	registerTools();
	//
	std::vector<std::string> emptyArgsList;
	// Calling window init proc
	registerLuaFunc();
	//
	_DMC->init(getLua().getStatePointer());
	//
	initObjectProjectionMetatable(); // init system to access to scenary objects from lua
	// init client/server stuffs


	CConfigVarBase::getConfigFileTimeStamp() ++; // invalidate all var taken from the config file

	// load r2 features & components
	{
		CVerboseClock clock("Execution of r2_core.lua");
		doLuaScript("r2_core.lua", "r2ed common functions and definitions");
	}
	initClassInheritanceTable();
	//
	{
		CLuaStackChecker lsc4(&getLua());
		setCurrentTool(NULL); // force the default tool (select / move)
	}

	initDecals();

	// TMP nico : unit test for newpoly stuff
	//polyUnitTest();

	_Initialized = true;

	_LuaUIMainLoop = _Env["UIMainLoop"];

	if (_SerializeUIConfig)
	{
		// load virtual desktops configs
		bool configLoaded = false;
		{
			CVerboseClock clock("Load of ui config from disk ");
			// if access mode is DM, then load the whole ui (else it has been previuously loaded)
			if (initialMode == AnimationModePlay)
			{
				loadStandardUI();
			}
			else
			{
				configLoaded = loadUIConfig(getUIPrefix(initialMode));
			}
		}

		// if not found then reset all virtual desktops (one for each mode of the editor)
		if (!configLoaded)
		{
			nlwarning("No interface config found, resetting editor windows to their default");
			setForceDesktopReset(true);
		}
	}

	setMode(initialMode);

	{
		CVerboseClock clock("Update of localized elements");
		CWidgetManager::getInstance()->updateAllLocalisedElements();
	}

}

// *********************************************************************************************************
std::string CEditor::getUIPrefix(TMode mode) const
{
	//H_AUTO(R2_CEditor_getUIPrefix)
	switch(mode)
		{
			case AnimationModePlay:
				return "";
			break;
			default:
				return "r2ed_interface_";
			break;
		}
}

// *********************************************************************************************************
uint CEditor::getMaxNumPlotItems()
{
	//H_AUTO(R2_CEditor_getMaxNumPlotItems)
	uint ret;
	fromString( CWidgetManager::getInstance()->getParser()->getDefine("r2ed_max_num_plot_item_sheets"), ret);
	return ret;
}

// *********************************************************************************************************
CCDBNodeLeaf *CEditor::getRefPlotItemSheetDBLeaf(uint index)
{
	//H_AUTO(R2_CEditor_getRefPlotItemSheetDBLeaf)
	return NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:R2:REFERENCE_PLOT_ITEMS:%d:SHEET", (int) index), false);
}

// *********************************************************************************************************
CCDBNodeLeaf *CEditor::getPlotItemSheetDBLeaf(uint index)
{
	//H_AUTO(R2_CEditor_getPlotItemSheetDBLeaf)
	return NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:R2:PLOT_ITEMS:%d:SHEET", (int) index), false);
}


// *********************************************************************************************************
void CEditor::setReferencePlotItemSheet(uint index, uint32 sheetId)
{
	//H_AUTO(R2_CEditor_setReferencePlotItemSheet)
	CCDBNodeLeaf *leaf = getRefPlotItemSheetDBLeaf(index);
	if (leaf)
	{
		leaf->setValue32(sheetId);
	}
	leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:R2:AVAILABLE_PLOT_ITEMS:%d:SHEET", (int) index), false);
	if (leaf)
	{
		leaf->setValue32(sheetId);
	}
}

// *********************************************************************************************************
void CEditor::initDummyPlotItems()
{
	//H_AUTO(R2_CEditor_initDummyPlotItems)
	uint maxNumPlotItems = getMaxNumPlotItems();
	uint numItems = std::min(10u, maxNumPlotItems);
	uint k;
	for(k = 0; k < numItems; ++k)
	{
		CCDBNodeLeaf *destLeaf = getPlotItemSheetDBLeaf(k);
		if (!destLeaf) continue;
		CCDBNodeLeaf *srcLeaf = getRefPlotItemSheetDBLeaf(k);
		destLeaf->setValue32(srcLeaf ? srcLeaf->getValue32() : 0u);
	}
	for(; k < maxNumPlotItems; ++k)
	{
		CCDBNodeLeaf *destLeaf = getPlotItemSheetDBLeaf(k);
		if (!destLeaf) continue;
		destLeaf->setValue32(0);
	}
}

// *********************************************************************************************************
void CEditor::resetPlotItems()
{
	//H_AUTO(R2_CEditor_resetPlotItems)
	uint maxNumPlotItems = getMaxNumPlotItems();
//	uint numItems = std::min(10u, maxNumPlotItems);
	for(uint k = 0; k < maxNumPlotItems; ++k)
	{
		CCDBNodeLeaf *destLeaf = getPlotItemSheetDBLeaf(k);
		if (!destLeaf) continue;
		destLeaf->setValue32(0);
	}
	_PlotItemInfos.clear();
}

// *********************************************************************************************************
void CEditor::initReferencePlotItems()
{
	//H_AUTO(R2_CEditor_initReferencePlotItems)
	{
		CVerboseClock clock("InitReferencePlotItems");
		uint maxNumPlotItems = getMaxNumPlotItems();
		clamp(maxNumPlotItems, 0u, 1024u);
		uint currIndex = 0;
		// look for plot item sheets
		const CSheetManager::TEntitySheetMap &sheets = SheetMngr.getSheets();
		for(CSheetManager::TEntitySheetMap::const_iterator it = sheets.begin(); it != sheets.end(); ++it)
		{
			if (it->second.EntitySheet && it->second.EntitySheet->Type == CEntitySheet::ITEM)
			{
				std::string name = it->second.EntitySheet->Id.toString();
				if (strstr(name.c_str(), "r2_plot_item"))
				{
					uint32 sheetId = it->second.EntitySheet->Id.asInt();
					setReferencePlotItemSheet(currIndex, sheetId);
					++ currIndex;
				}
			}
		}
		for (; currIndex < maxNumPlotItems; ++currIndex)
		{
			setReferencePlotItemSheet(currIndex, 0);
		}
		resetPlotItems();
	}
}


// *********************************************************************************************************
bool CEditor::loadUIConfig(const std::string &prefix)
{
	//H_AUTO(R2_CEditor_loadUIConfig)
	CHECK_EDITOR
	return  getUI().loadConfig("save/" + prefix + PlayerSelectedFileName + ".icfg");
}

// *********************************************************************************************************
void CEditor::saveUIConfig()
{
	//H_AUTO(R2_CEditor_saveUIConfig)
	CHECK_EDITOR
	// if there are dirt desktop, update all of them
	if (_Mode != AnimationModePlay)
	{
		bool restoreMode = false;
		uint8 oldMode = getUI().getMode();
		for (uint k = 0; k < 4; ++k)
		{
			if (_ForceDesktopReset[k])
			{
				setUIMode(k);
				restoreMode = true;
				_ForceDesktopReset[k] = false;
			}
			if (restoreMode)
			{
				setUIMode(oldMode);
			}
		}
	}
	getUI().saveConfig("save/" + getUIPrefix(_Mode) + PlayerSelectedFileName + ".icfg");
}

// *********************************************************************************************************
void CEditor::initDecals()
{
	//H_AUTO(R2_CEditor_initDecals)
	CHECK_EDITOR
	// highlight
	CLuaObject config = getEnv()["Config"];
	CLuaObject objHighlight = config["HightlightDecalLook"];
	CLuaObject objSelect = config["SelectDecalLook"];
	CLuaObject objSelecting  = config["SelectingDecalLook"];
	CLuaObject objPioneer = config["PionneerDecalLook"];

	_HighlightDecalAnim.buildFromLuaTable(objHighlight);
	_SelectDecalAnim.buildFromLuaTable(objSelect);
	_SelectingDecalAnim.buildFromLuaTable(objSelecting);
	_PionneerDecalAnim.buildFromLuaTable(objPioneer);
	_HighlightDecal.setClipDownFacing(true);
	_SelectDecal.setClipDownFacing(true);
	_PionneerDecal.setClipDownFacing(true);
	//
	CPrimLook boxLook;
	boxLook.init(getEnv()["PrimRender"]["SelectBoxLook"]);
	_SelectBox.setLook(boxLook);
	boxLook.init(getEnv()["PrimRender"]["HighlightBoxLook"]);
	_HighlightBox.setLook(boxLook);

}


// *********************************************************************************************************
void CEditor::showPrimRender(CPrimRender &dest, const NLMISC::CAABBox &localBox, const NLMISC::CMatrix &worldMat, const CDecalAnim &refDecalAnim)
{
	//H_AUTO(R2_CEditor_showPrimRender)
	CHECK_EDITOR
	static NLMISC::CPolygon2D poly2D;
	poly2D.Vertices.resize(4);
	CVector pmin = localBox.getMin();
	CVector pmax = localBox.getMax();
	poly2D.Vertices[0] = worldMat * CVector(pmin.x, pmin.y, pmin.z);
	poly2D.Vertices[1] = worldMat * CVector(pmax.x, pmin.y, pmin.z);
	poly2D.Vertices[2] = worldMat * CVector(pmax.x, pmax.y, pmin.z);
	poly2D.Vertices[3] = worldMat * CVector(pmin.x, pmax.y, pmin.z);
	NLMISC::CAABBox bbox;
	CLandscapePolyDrawer::computeBBoxFromPolygon(poly2D, bbox);
	dest.setVertices(poly2D.Vertices);
	//dest.setWorldMapPolyColor(polyColor);
	float animRatio = refDecalAnim.DurationInMs == 0 ? 0.f : (T1 % refDecalAnim.DurationInMs) / (float) refDecalAnim.DurationInMs;
	animRatio = 0.5f * cosf(2.f * (float) Pi * animRatio) + 0.5f;
	CRGBA color = blend(refDecalAnim.StartDiffuse, refDecalAnim.EndDiffuse, animRatio);
	CPrimLook pl = dest.getLook();
	pl.EdgeLook.DecalColor = pl.VertexLook.DecalColor = pl.FirstVertexLook.DecalColor = color;
	dest.setLook(pl);
	dest.addDecalsToRenderList();
	static volatile bool showPoly = true;
	if (showPoly)
	{
		CLandscapePolyDrawer::getInstance().addPoly(poly2D,  dest.getLook().EdgeLook.WorldMapColor, bbox);
	}
}

// *********************************************************************************************************
void CEditor::showSelectBox(const NLMISC::CAABBox &localBox, const NLMISC::CMatrix &worldMat)
{
	//H_AUTO(R2_CEditor_showSelectBox)
	CHECK_EDITOR
	showPrimRender(_SelectBox, localBox, worldMat, _SelectDecalAnim);
}


// *********************************************************************************************************
void CEditor::showHighlightBox(const NLMISC::CAABBox &localBox, const NLMISC::CMatrix &worldMat)
{
	//H_AUTO(R2_CEditor_showHighlightBox)
	CHECK_EDITOR
	showPrimRender(_HighlightBox, localBox, worldMat, _HighlightDecalAnim);
}

// *********************************************************************************************************
void CEditor::showSelectDecal(const NLMISC::CVector &pos, float scale)
{
	//H_AUTO(R2_CEditor_showSelectDecal)
	CHECK_EDITOR
	updateDecalBlendRegion(_SelectDecal, pos);
	showDecal(pos, scale, _SelectDecal, _SelectDecalAnim);
}


// *********************************************************************************************************
void CEditor::showHighlightDecal(const NLMISC::CVector &pos, float scale)
{
	//H_AUTO(R2_CEditor_showHighlightDecal)
	CHECK_EDITOR
	updateDecalBlendRegion(_HighlightDecal, pos);
	showDecal(pos, scale, _HighlightDecal, _HighlightDecalAnim);
}

// *********************************************************************************************************
void CEditor::showDecal(const NLMISC::CVector2f &pos, float scale, CDecal &decal, const CDecalAnim &decalAnim)
{
	//H_AUTO(R2_CEditor_showDecal)
	CHECK_EDITOR
	float animRatio = decalAnim.DurationInMs == 0 ? 0.f : ((T1 - _DecalRefTime) % decalAnim.DurationInMs) / (float) decalAnim.DurationInMs;
	animRatio = 0.5f * cosf(2.f * (float) Pi * animRatio) + 0.5f;
	decalAnim.updateDecal(pos, animRatio, decal, scale);
	decal.addToRenderList();
}

// *********************************************************************************************************
void CEditor::addSelectingDecal(const NLMISC::CVector &pos, float scale)
{
	//H_AUTO(R2_CEditor_addSelectingDecal)
	CHECK_EDITOR
	CSelectingDecal *decal = NULL;
	// see if there's an unused decal in the list
	for(uint k = 0; k < _SelectingDecals.size(); ++k)
	{
		if (_SelectingDecals[k]->EndDate < T1)
		{
			decal = _SelectingDecals[k];
			break;
		}
	}
	if (decal == NULL)
	{
		_SelectingDecals.push_back(new CSelectingDecal);
		decal = _SelectingDecals.back();
	}
	decal->EndDate = T1 + _SelectingDecalAnim.DurationInMs;
	decal->Pos = pos;
	decal->Scale = scale;
	//
	updateDecalBlendRegion(decal->Decal, pos);
	_DecalRefTime = T1;
}

// *********************************************************************************************************
void CEditor::updateSelectingDecals()
{
	//H_AUTO(R2_CEditor_updateSelectingDecals)
	CHECK_EDITOR
	if (_SelectingDecalAnim.DurationInMs == 0) return;
	for(uint k = 0; k < _SelectingDecals.size(); ++k)
	{
		CSelectingDecal *decal = _SelectingDecals[k];
		if (decal)
		{
			if (decal->EndDate > T1)
			{
				float animRatio = (float) (T1 - (decal->EndDate - _SelectingDecalAnim.DurationInMs)) / (float) _SelectingDecalAnim.DurationInMs;
				_SelectingDecalAnim.updateDecal(decal->Pos, animRatio, decal->Decal, decal->Scale);
				decal->Decal.addToRenderList();
			}
		}
	}
}

// *******************************************************************************************************
void CEditor::reset()
{
	//H_AUTO(R2_CEditor_reset)
	if (ClientCfg.R2EDExtendedDebug)
		Driver->setWindowTitle(ucstring("Resetting R2ED editor ..."));

	CHECK_EDITOR
	_SerializeUIConfig = false; // prevent reloading of ui for speed
	_WantedActOnInit.clear();
	if (getCurrentAct())
	{
		R2::CObjectTable* act = getCurrentAct()->getObjectTable();
		if (act->isString("Name"))
		{
			_WantedActOnInit = act->toString("Name");
		}
		else
		{
			_WantedActOnInit = act->toString("Title"); //obsolete
		}
	}
	CVerboseClock clock("Reset of editor ");

	TMode oldMode = _Mode;
	TAccessMode oldAccessMode = _AccessMode;
	if (_Mode == EditionMode)
	{
		R2::getEditor().getLua().executeScriptNoThrow("r2.Version.save(\"save/r2_buffer.dat\")");
	}
	release();
	clearDebugWindow();
	if (ReloadUIFlag)
	{
		loadLanguageFile();
		reloadUI();
		setForceDesktopReset(true); // next changes of desktop should force a reset, since ui has been reloaded
		ReloadUIFlag = false;
	}
	init(oldMode, oldAccessMode);

	if (oldMode == EditionMode)
	{
		CVerboseClock clock("Request reconnection ");
		getLua().executeScriptNoThrow("r2.requestReconnection()");
	}
	// try to return to the act with the same title
	_SerializeUIConfig = true;

	if (ClientCfg.R2EDExtendedDebug)
	{
		Driver->setWindowTitle(CI18N::get("TheSagaOfRyzom"));
		// Show the window
		Driver->showWindow();
	}

	getUI().displaySystemInfo(CI18N::get("uiR2EDEditorReseted"), "BC");
}

// *********************************************************************************************************
void CEditor::reloadUI()
{
	//H_AUTO(R2_CEditor_reloadUI)
	CHECK_EDITOR
	CVerboseClock clock("Reload ui");
	// reload same list than at startup (given in client.cfg)
	for (uint k = 0; k < ClientCfg.XMLR2EDInterfaceFiles.size(); ++k)
	{
		reloadUI(ClientCfg.XMLR2EDInterfaceFiles[k].c_str());
	}
}

// *********************************************************************************************************
void CEditor::reloadUI(const char *filename)
{
	//H_AUTO(R2_CEditor_reloadUI)
	CHECK_EDITOR
	CVerboseClock clock("Loading ui " + std::string(filename));
	CLuaStackChecker ls(&getLua());

	std::string path = CPath::lookup(filename, false);
	if (path.empty())
	{
		nlwarning("File %s not found, r2 ui not (re)loaded.", filename);
		return;
	}
	uint32 fileDate = CFile::getFileModificationDate(path);
	if (_LastUIModificationDate.count(filename) == 0)
	{
		_LastUIModificationDate[filename] = 0;
	}
	uint32 &lastUIModificationDate = _LastUIModificationDate[filename];
	if (fileDate > lastUIModificationDate || !ClientCfg.R2EDDontReparseUnchangedUIFiles) // TMP (init not complete else ...)
	{
		// reload the ui files
		NLMISC::ICommand::execute(toString("loadui %s", filename).c_str(), g_log);
		lastUIModificationDate = fileDate;
	}
	else
	{
		if (!ClientCfg.R2EDDontReparseUnchangedUIFiles)
		{
			nlwarning("ui file %s date unchanged, not reloading.", filename);
		}
	}
}



// *********************************************************************************************************
void CEditor::connectAsCreator()
{
	//H_AUTO(R2_CEditor_connectAsCreator)
	CHECK_EDITOR
	nlassert(_DMC);
	{
		CLuaStackChecker lsc2(&getLua());
		_DMC->testConnectionAsCreator();
	}
}


// string cached into lua for fast comparison
static CLuaString lstr_isNil("isNil");
static CLuaString lstr_Parent("Parent");
static CLuaString lstr_ParentInstance("ParentInstance");
static CLuaString lstr_IndexInParent("IndexInParent");
static CLuaString lstr_User("User");
static CLuaString lstr_Size("Size");
static CLuaString lstr_DisplayerUI("DisplayerUI");
static CLuaString lstr_DisplayerVisual("DisplayerVisual");
static CLuaString lstr_DisplayerProperties("DisplayerProperties");




// *********************************************************************************************************
void CEditor::initObjectProjectionMetatable()
{
	//H_AUTO(R2_CEditor_initObjectProjectionMetatable)
	CHECK_EDITOR
	_ObjectProjectionMetatable = getEnv().newTable("_instance_projection_metatable");
	// projection functions

	struct CProjector
	{
		static bool checkTag(CLuaState &ls)
		{
			CLuaStackChecker lsc(&ls);
			ls.getMetaTable(1);
			ls.push("tag");
			ls.getTable(-2);
			bool ok = false;
			if (ls.isString(-1))
			{
				if (strcmp(ls.toString(-1), "CObjectTable") == 0)
				{
					ok = true;
				}
			}
			ls.pop(2);
			if (!ok)
			{
				nlwarning("metatable error : Editor object expected");
			}
			return ok;
		}

		static int index(CLuaState &ls)
		{
			nlassert(ls.getTop() == 2);
			#ifdef NL_DEBUG
				if (!checkTag(ls)) return false;
			#endif
			CObjectTable::TRefPtrConst &obj= *(CObjectTable::TRefPtrConst *) ls.toUserData(1);
			if (OPERATOR_EQUAL(obj, NULL))
				//NLMISC::operator==(obj, NULL))
			{
				std::string index = ls.toString(2);
				// special members
				if ( index == "isNil")
				//if (std::operator==(index "isNil")
				{
					ls.push(true);
					return 1;
				}
				CLuaIHMRyzom::dumpCallStack();
				// object has been deleted but the script maintains a reference on it
				throw ELuaWrappedFunctionException(&ls, "Attempt to access an erased object");
			}

			if (ls.isNumber(2))
			{
				// index is a number
				const CObject *other = obj->getValue((uint32) ls.toNumber(2));
				if (other)
				{
					pushValue(ls, other);
					return 1;
				}
				else
				{
					// 'bad index' message already printed by CObject::getValue
					CLuaIHMRyzom::dumpCallStack();
				}
			}

			if (!ls.isString(2))
			{
				nlwarning("String expected when accessing an object property, %s found instead", ls.getTypename(2));
				CLuaIHMRyzom::dumpCallStack(0);
				return 0;
			}

			const char *strId = ls.toString(2); // strings are shared in lua, so a pointer comparison is sufficient

			// special members
			// NB nico : this was added before the R2::CInstance class derived from CReflectable,
			// -> see if some of the following properties may better  fit into R2::CInstance exported properties

			if (OPERATOR_EQUAL( lstr_isNil, strId))
			{
				ls.push(false);
				return 1;
			}
			else
			if (OPERATOR_EQUAL(strId, lstr_Parent))
			{
				if (!obj->getParent())
				{
					ls.pushNil();
					return 1;
				}
				nlassert(obj->getParent()->isTable());
				getEditor().projectInLua((CObjectTable *) obj->getParent());
				return 1;
			}
			else if (OPERATOR_EQUAL(strId, lstr_ParentInstance))
			{
				// look for parent instance (that is, not the
				CObject *parent = obj->getParent();
				while (parent)
				{
					if (parent->findAttr("InstanceId"))
					{
						getEditor().projectInLua((CObjectTable *) parent);
						return 1;
					}
					parent = parent->getParent();
				}
				ls.pushNil();
				return 1;
			}
			if (OPERATOR_EQUAL(strId, lstr_IndexInParent))
			{
				if (!obj->getParent())
				{
					ls.pushNil();
					return 1;
				}
				sint32 index = obj->getParent()->findIndex(obj);
				if (obj->getParent()->getKey(index).empty())
				{
					ls.push((double) index);
				}
				else
				{
					ls.push((double) -1);
				}
				return 1;
			}
			else if (OPERATOR_EQUAL(strId, lstr_User))
			{
				// push user table on stack
				CEditor::getLuaUserTableFromObject(ls, *obj);
				return 1;
			}
			else if (OPERATOR_EQUAL(strId, lstr_Size))
			{
				if (obj->isTable())
				{
					ls.push((double) obj->getSize());
				}
				else
				{
					ls.pushNil();
				}
				return 1;
			}
			else
			if (OPERATOR_EQUAL(strId, lstr_DisplayerUI))
			{
				CInstance *instance = getEditor().getInstanceFromObject(obj);
				if (!instance)
				{
					ls.pushNil();
					return 1;
				}
				else
				{
					if (instance->getDisplayerUI())
					{
						instance->getDisplayerUI()->pushLuaAccess(ls);
					}
					else
					{
						ls.pushNil();
					}
					return 1;
				}
			}
			else
			if (OPERATOR_EQUAL(strId, lstr_DisplayerVisual))
			{
				CInstance *instance = getEditor().getInstanceFromObject(obj);
				if (!instance)
				{
					ls.pushNil();
					return 1;
				}
				else
				{
					if (instance->getDisplayerVisual())
					{
						instance->getDisplayerVisual()->pushLuaAccess(ls);
					}
					else
					{
						ls.pushNil();
					}
					return 1;
				}
			}
			else
			if (OPERATOR_EQUAL(strId, lstr_DisplayerProperties))
			{
				CInstance *instance = getEditor().getInstanceFromObject(obj);
				if (!instance)
				{
					ls.pushNil();
					return 1;
				}
				else
				{
					if (instance->getDisplayerProperties())
					{
						instance->getDisplayerProperties()->pushLuaAccess(ls);
					}
					else
					{
						ls.pushNil();
					}
					return 1;
				}
			}
			// else tries with a string
			std::string index = ls.toString(2);
			const CObject *other = getObject(obj, index);
			if (other)
			{
				pushValue(ls, other);
				return 1;
			}
			// this is not an attribute of the class
			// -> maybe this is a method ?
			// look in the parent class is there is a function with the wanted name
			// NOTE : calling a method on an editor object in the script can be done by doing
			// r2:getClass(instance).methodName(instance, parameters ...)
			// with this trick, one can do instance:methodName(parameters ...)
			CObject *className = obj->findAttr("Class");
			if (className)
			{
				CLuaObject method = getEditor().getClasses()[className->toString()][index];
				if (method.isFunction())
				{
					method.push();
					return 1;
				}
			}

			// Try with a property exported from CInstance
			// Is is ok to test thos properties after those defined in lua classes definition
			// because the init check that no "native" property is declared by the user

			CInstance *instance = getEditor().getInstanceFromObject(obj);
			if (instance)
			{
				const CReflectedProperty *prop = instance->getReflectedProperty(index, false);
				if (prop)
				{
					CLuaIHM::luaValueFromReflectedProperty(ls, *instance, *prop);
					return 1;
				}
			}

			// unknown attribute
			ls.pushNil();
			return 1;
		}
		static int newIndex(CLuaState &ls)
		{
			nlassert(ls.getTop() == 3);
			#ifdef NL_DEBUG
				if (!checkTag(ls)) return false;
			#endif
			CObjectTable::TRefPtrConst &obj= *(CObjectTable::TRefPtrConst *) ls.toUserData(1);
			if (obj == NULL)
			{
				throw ELuaWrappedFunctionException(&ls, "Trying to set a property in an erased object, returning nil");
			}
			#ifdef NL_DEBUG
				if (!checkTag(ls)) return false;
			#endif
			// try with a native (local ..) property exported from CInstance
			CInstance *instance = getEditor().getInstanceFromObject(obj);
			if (instance)
			{
				const CReflectedProperty *prop = instance->getReflectedProperty(ls.toString(2));
				if (prop)
				{
					CLuaIHM::luaValueToReflectedProperty(ls, 3, *instance, *prop);
					return 1;
				}
			}
			// other instances properties are read only !!
			throw ELuaWrappedFunctionException(&ls, "Property %s of editor object is read-only. You must use 'r2.requestSetNode' function to change it", ls.toString(2));
		}
		static int gc(CLuaState &ls)
		{
			#ifdef NL_DEBUG
				if (!checkTag(ls)) return false;
			#endif
			// TODO : maybe not useful ...
			CObjectTable::TRefPtrConst &obj= *(CObjectTable::TRefPtrConst *) ls.toUserData(1);
			typedef CObjectTable::TRefPtrConst TRefPtrConst;
			obj.~TRefPtrConst();
			return 0;
		}
		// tool function used by 'next'
		// TODO : put this code in a better place ...
		static void pushValue(CLuaState &ls, const CObject *obj)
		{
			if (!obj)
			{
				ls.pushNil();
				return;
			}
			if (obj->isString())
			{
				ls.push(obj->toString());
			}
			else if (obj->isNumber())
			{
				ls.push(obj->toNumber());
			}
			else if (obj->isTable())
			{
				getEditor().projectInLua((CObjectTable *) obj);
			}
			else
			{
				nlassert(0); // type not supported ...
			}
		}

		// helper function used by 'next' : push a key
		static void pushKey(CLuaState &ls, const CObject *obj, sint32 index)
		{
			if (!obj)
			{
				ls.pushNil();
				return;
			}
			if (obj->getKey(index).empty())
			{
				ls.push((double) index);
			}
			else
			{
				ls.push(obj->getKey(index));
			}
		}


		// special : we redefine the 'next' global function found in the standard
		// lua library in order to allow fields traversal for the projected object.
		// This is done in lua in the 'r2_core.lua' script
		// The new 'next' function will look into the object metatable
		// If a __next function is found, then it will be called for traversal
		// otherwise, the standard 'next' function is called instead.
		// This allow to have a traversal for objects that are not tables
		static int next(CLuaState &ls)
		{
			if (ls.getTop() != 2)
			{
				CLuaIHM::fails(ls, "__next metamethod require 2 arguments (table & key)");
			}
			#ifdef NL_DEBUG
				if (!checkTag(ls)) return false;
			#endif
			CObjectTable::TRefPtrConst &obj= *(CObjectTable::TRefPtrConst *) ls.toUserData(1);
			if (obj == NULL)
			{
				throw ELuaWrappedFunctionException(&ls, "editor object '__next' metatmethod : Attempt to access an erased object");
			}
			if (ls.isNil(2))
			{
				// key is nil -> start of traversal
				if(obj->getSize() == 0)
				{
					ls.remove(-2);
					ls.pushNil();
					return 2; // let (nil, nil) on the stack
				}
				else
				{
					// look for duplicated keys (ignoring empty keys, because in this case th index is the key)
					std::set<std::string> keys;
					for(uint k = 0; k < obj->getSize(); ++k)
					{
						std::string key = obj->getKey((sint32) k);
						if (!key.empty())
						{
							if (keys.count(key))
							{
								nlwarning("Duplicated key of type string found while attempting to enumerate an instance content.");
								nlwarning("key is %s", key.c_str());
								CLuaIHMRyzom::dumpCallStack(1);
								CLuaIHM::fails(ls, "Aborting to avoid infinite loop.");
							}
							keys.insert(key);
						}
					}
					ls.pop(2);
					pushKey(ls, obj, 0);
					pushValue(ls, obj->getValue(0));
					return 2;
				}
			}
			else
			{
				// continuation of traversal
				// -> retrieve index from the key
				sint32 index;
				if (ls.isNumber(2))
				{
					index = (uint32) ls.toNumber(2);
				}
				else
				{
					if (!ls.isString(2))
					{
						nlwarning("__next metamethod : string expected");
						ls.pop();
						ls.pushNil();
						return 2;
					}
					else
					{
						index = obj->findIndex(ls.toString(2));
					}
				}
				if (index == -1)
				{
					nlwarning("__next metamethod : key not found");
					ls.pop(2);
					ls.pushNil();
					ls.pushNil();
					return 2;
				}
				// retrieve next key
				sint32 newIndex = (uint32) (index + 1);
				if (newIndex == (sint32) obj->getSize())
				{
					// this was the last element, returns nil
					ls.pop(2);
					ls.pushNil();
					ls.pushNil();
					return 2;
				}
				else
				{
					ls.pop(2);
					pushKey(ls, obj, newIndex);
					pushValue(ls, obj->getValue(newIndex));
					return 2;
				}
			}
			return 0;
		}
		static int equal(CLuaState &ls)
		{
			// TMP TMP TMP
			static volatile bool from = false;
			if (from)
			{
				CLuaIHMRyzom::dumpCallStack(0);
			}
			nlassert(ls.getTop() == 2);
			if (!checkTag(ls)) return false;
			CObjectTable::TRefPtrConst &lhs= *(CObjectTable::TRefPtrConst *) ls.toUserData(1);
			CObjectTable::TRefPtrConst &rhs= *(CObjectTable::TRefPtrConst *) ls.toUserData(2);
			ls.push(OPERATOR_EQUAL(lhs, rhs));
			return 1;
		}
	};
	// affect functions to the metatable
	getLua().push(CProjector::index);
	_ObjectProjectionMetatable.setValue("__index", CLuaObject(getLua()));
	getLua().push(CProjector::newIndex);
	_ObjectProjectionMetatable.setValue("__newindex", CLuaObject(getLua()));
	getLua().push(CProjector::gc);
	_ObjectProjectionMetatable.setValue("__gc", CLuaObject(getLua()));
	getLua().push(CProjector::next);
	_ObjectProjectionMetatable.setValue("__next", CLuaObject(getLua()));
	getLua().push(CProjector::equal);
	_ObjectProjectionMetatable.setValue("__eq", CLuaObject(getLua()));
	// tag to mark that the user data is a CObjectTable ref ptr
	_ObjectProjectionMetatable.setValue("tag", std::string("CObjectTable"));
}

// *********************************************************************************************************
void CEditor::getLuaUserTableFromObject(CLuaState &ls, const CObjectTable &table)
{
	//H_AUTO(R2_CEditor_getLuaUserTableFromObject)
	CHECK_EDITOR
	CLuaStackChecker lsc(&ls, 1);
	// read write environement accessible from lua
	ls.pushLightUserData((void *) &table);
	ls.getTable(LUA_REGISTRYINDEX);
	if (ls.isNil())
	{
		ls.pop();
		// no user table created yet ?... create it
		ls.pushLightUserData((void *) &table);
		ls.newTable();
		ls.pushValue(-1); // saves the table
		ls.insert(-3);
		ls.setTable(LUA_REGISTRYINDEX);
		// table remains on the stack
	}
}

// *********************************************************************************************************
void CEditor::setCurrentAct(CInstance *act)
{
	//H_AUTO(R2_CEditor_setCurrentAct)
	CHECK_EDITOR
	if (!_ScenarioInstance) return;
	if (_ClearingContent) return;
	bool currentActSelected = _SelectedInstance && (_SelectedInstance == _CurrentAct);
	CInstance* previousAct = _CurrentAct;
	_CurrentAct = act ? act : getBaseAct();
	//
	struct CPreActChangedVisitor : public IInstanceVisitor
	{
		virtual void visit(CInstance &inst)
		{
			inst.onPreActChanged();
		}
	};
	CPreActChangedVisitor preActChangedVisitor;
	_ScenarioInstance->visit(preActChangedVisitor);
	struct CActChangedVisitor : public IInstanceVisitor
	{
		virtual void visit(CInstance &inst)
		{
			inst.onActChanged();
		}
	};
	CActChangedVisitor actChangedVisitor;
	_ScenarioInstance->visit(actChangedVisitor);
	//TP if necesary


	// warn lua that current act has changed
	if (!previousAct)
	{
		getLua().pushNil();
	}
	else
	{
		previousAct->getLuaProjection().push();
	}

	if (!_CurrentAct)
	{
		getLua().pushNil();
	}
	else
	{
		_CurrentAct->getLuaProjection().push();
	}

	callEnvMethod("onActChanged", 2, 0);

	setSelectedInstance(currentActSelected ? _CurrentAct : NULL);


	setCurrentTool(NULL);
}

// *********************************************************************************************************
void CEditor::setCurrentActFromTitle(const std::string &wantedTitle)
{
	//H_AUTO(R2_CEditor_setCurrentActFromTitle)
	CHECK_EDITOR
	if (!_Scenario)
	{
		nlwarning("Scenario initialisation failed, can't set current act");
		return;
	}
	CObject *actTable = _Scenario->getAttr("Acts");
	if (actTable->isTable())
	{
		for(uint k = 0; k < actTable->getSize(); ++k)
		{
			R2::CObject *act = actTable->getValue(k);
			nlassert(act);
			std::string actTitle;
			if (act->isString("Name"))
			{
				actTitle = act->toString("Name");
			}
			else
			{
				actTitle = act->toString("Title"); //obsolete
			}

			if (actTitle == wantedTitle)
			{
				setCurrentAct(getInstanceFromObject(act));
				break;
			}
		}
	}
	else
	{
		nlwarning("'Acts' field in scenario should be a table");
	}
}

// *********************************************************************************************************
void CEditor::projectInLua(const CObjectTable *table)
{
	//H_AUTO(R2_CEditor_projectInLua)
	CHECK_EDITOR
	nlassert(table);
	const CObjectTableClient *otc = NLMISC::safe_cast<const CObjectTableClient *>(table);
	otc->pushOnLuaStack(getLua(), _ObjectProjectionMetatable);
}


// *********************************************************************************************************
void CEditor::backupRequestCommands()
{
	//H_AUTO(R2_CEditor_backupRequestCommands)
	nlassert(!_OldLuaRequestInsertNode.isValid()); // restoreRequestCommands not called ?
	nlassert(!_OldLuaRequestInsertGhostNode.isValid());
	nlassert(!_OldLuaRequestSetNode.isValid());
	nlassert(!_OldLuaRequestEraseNode.isValid());
	nlassert(!_OldLuaRequestMoveNode.isValid());

	_OldLuaRequestInsertNode = getEnv()["requestInsertNode"];
	_OldLuaRequestInsertGhostNode = getEnv()["requestInsertGhostNode"];
	_OldLuaRequestSetNode    = getEnv()["requestSetNode"];
	_OldLuaRequestEraseNode  = getEnv()["requestEraseNode"];
	_OldLuaRequestMoveNode  = getEnv()["requestMoveNode"];

	struct CIgnoreRequestCall
	{
		static int requestSomething(CLuaState &)
		{
			nlwarning("PAS BIEN C MAL !!!");
			/*
			CLuaIHM::debugInfo("Can't call a 'r2.request' command while object are erased in displayers, or are just being created (in this case, use onPostCreate instead) !");
			CLuaIHM::debugInfo("Callstack is :");
			CLuaIHM::dumpCallStack(1);*/
			return 0;
		}
	};


	// While we delete the object tree, displayers will be triggered
	// Make sure that they don't call any r2.requestSomething commands, end if so,
	// print an error message
	getEnv().setValue("requestInsertNode", CIgnoreRequestCall::requestSomething);
	getEnv().setValue("requestInsertGhostNode", CIgnoreRequestCall::requestSomething);
	getEnv().setValue("requestSetNode", CIgnoreRequestCall::requestSomething);
	getEnv().setValue("requestEraseNode", CIgnoreRequestCall::requestSomething);
	getEnv().setValue("requestMoveNode", CIgnoreRequestCall::requestSomething);
}

// *********************************************************************************************************
void CEditor::restoreRequestCommands()
{
	//H_AUTO(R2_CEditor_restoreRequestCommands)
	nlassert(_OldLuaRequestInsertNode.isValid()); // backupRequestCommands not called ?
	nlassert(_OldLuaRequestInsertGhostNode.isValid());
	nlassert(_OldLuaRequestSetNode.isValid());
	nlassert(_OldLuaRequestEraseNode.isValid());
	nlassert(_OldLuaRequestMoveNode.isValid());
	// restore old functions
	getEnv().setValue("requestInsertNode", _OldLuaRequestInsertNode);
	getEnv().setValue("requestInsertGhostNode", _OldLuaRequestInsertGhostNode);
	getEnv().setValue("requestSetNode",	   _OldLuaRequestSetNode);
	getEnv().setValue("requestEraseNode",  _OldLuaRequestEraseNode);
	getEnv().setValue("requestMoveNode",   _OldLuaRequestMoveNode);
	//
	_OldLuaRequestInsertNode.release();
	_OldLuaRequestInsertGhostNode.release();
	_OldLuaRequestSetNode.release();
	_OldLuaRequestEraseNode.release();
	_OldLuaRequestMoveNode.release();
}

// *********************************************************************************************************
void CEditor::clearContent()
{
	nlwarning("*R2* clear content");
	setMaxVisibleEntityExceededFlag(false);

	//H_AUTO(R2_CEditor_clearContent)
	if (_Mode != EditionMode && _Mode != AnimationModeLoading) return;
	_ClearingContent = true;

	CHECK_EDITOR
	setSelectedInstance(NULL);
	setFocusedInstance(NULL);

	// backup all "requestCommand"


	if (_CurrentTool) _CurrentTool->cancel();
	CTool::releaseMouse();

	delete _NewScenario; // will be not NULL only if quit is called during the scenario connection screen
	_NewScenario = NULL;

	eraseScenario();

	_InstancesByDispName.clear();
	_InstancesByDispName.resize(_ClassNameToIndex.size());
	_InstanceObservers.clear();
	_InstanceObserverHandles.clear();
	_Cookies.clear();
	_LocalGeneratedNames.clear();
	_Instances.clear();
	_BaseAct = NULL;
	_CurrentAct = NULL;
	_SelectedInstance = NULL;
	_FocusedInstance = NULL;
	_ScenarioInstance = NULL;
	_Scenario = NULL;
	_SelectingDecals.clear();
	_InstanceObservers.clear();
	_InstanceObserverHandles.clear();
	_LastInstanceUnderPos = NULL;
	_CurrentTool = NULL;


	removeAllEntitySlots(); // for safety...
	_DMC->CDynamicMapClient::scenarioUpdated(NULL, false, 1); // clear scenario for real
	_ClearingContent = false;

	_PlotItemInfos.clear();

	PeopleInterraction.removeAllFreeTellers();

}


// *********************************************************************************************************
void CEditor::setPlotItemInfos(const TMissionItem &infos)
{
	//H_AUTO(R2_CEditor_setPlotItemInfos)
	_PlotItemInfos[infos.SheetId.asInt()] = infos;
}

// *********************************************************************************************************
const TMissionItem *CEditor::getPlotItemInfos(uint32 sheetId) const
{
	//H_AUTO(R2_CEditor_getPlotItemInfos)
	std::map<uint32, TMissionItem>::const_iterator it = _PlotItemInfos.find(sheetId);
	if (it == _PlotItemInfos.end()) return NULL;
	return &it->second;
}


// *********************************************************************************************************
void CEditor::release()
{
	nlwarning("*R2* release");
	//H_AUTO(R2_CEditor_release)
	GameContextMenu.init("");
	setFixedLighting(false);
	CHECK_EDITOR

	if (_Mode == NotInitialized)
	{
		// nothing more to do there
		return;
	}

	// warn lua of final release
	callEnvMethod("onFinalRelease", 0, 0);

	if (_SerializeUIConfig)
	{
		// serialization disabled when resetting the editor (for speed)
		saveCurrentKeySet();
		saveUIConfig();
	}
	CWidgetManager::getInstance()->hideAllWindows(); // make sure all action handlers are called while the r2 lua environment is still active
	clearContent();
	_EntityCustomSelectBoxMap.clear();

	// must do this after clearContent, which sets the default tool
	if (_CurrentTool)
	{
		_CurrentTool->cancel();
		_CurrentTool = NULL;
	}

	// clear the environment
	if (CLuaManager::getInstance().getLuaState())
	{
		getLua().pushGlobalTable();
		getLua().push(R2_LUA_PATH);
		getLua().pushNil();
		getLua().setTable(-3); // pop pop
		getLua().pop();
		_Globals.release();
		_Registry.release();
		_ObjectProjectionMetatable.release();	// AJM
		_Env.release();
		_Config.release();
		CEditorCheck::EditorCreated = false;
		// force a garbage collection to free the mem for real
		getLua().setGCThreshold(0);
	}


	// stop at login
	if (_DMC)
	{
		_DMC->release();
	}
	//
	nlassert(CDisplayerBase::ObjCount == 0);
	_Initialized = false;
	CEditorCheck::EditorCreated = false;	// AJM
	_Mode = NotInitialized;
	_AccessMode = AccessModeUnknown;

	::IgnoreEntityDbUpdates = false;

	//
	_IslandCollision.release();
	//
	_LastUIModificationDate.clear();
	_InstancesByDispName.clear();
	//
	delete _EntitySorter;
	_EntitySorter = NULL;
}



// *********************************************************************************************************
void CEditor::removeAllEntitySlots()
{
	//H_AUTO(R2_CEditor_removeAllEntitySlots)
	CHECK_EDITOR
	for (uint k = 1; k < 255; ++k)
	{
		if (EntitiesMngr.entity(k) != NULL)
		{
			EntitiesMngr.remove(k, false);
		}
	}
}

// *********************************************************************************************************
void CEditor::registerDisplayers()
{
	//H_AUTO(R2_CEditor_registerDisplayers)
	CHECK_EDITOR
	static bool registered = false;
	if (registered) return;
	NLMISC_REGISTER_CLASS(R2::CDisplayerVisualActivitySequence);
	NLMISC_REGISTER_CLASS(R2::CDisplayerVisualEntity);
	NLMISC_REGISTER_CLASS(R2::CDisplayerVisualGroup)
	NLMISC_REGISTER_CLASS(R2::CDisplayerVisualShape)
	NLMISC_REGISTER_CLASS(R2::CDisplayerLua)
	registered = true;
}

// *********************************************************************************************************
void CEditor::registerTools()
{
	//H_AUTO(R2_CEditor_registerTools)
	CHECK_EDITOR
	static bool registered = false;
	if (registered) return;
	NLMISC_REGISTER_CLASS(R2::CToolDrawPrim);
	NLMISC_REGISTER_CLASS(R2::CToolSelectMove);
	NLMISC_REGISTER_CLASS(R2::CToolSelectRotate);
	NLMISC_REGISTER_CLASS(R2::CToolNewVertex);
	registered = true;
}



// *********************************************************************************************************
void CEditor::setSelectedInstance(CInstance *inst)
{
	//H_AUTO(R2_CEditor_setSelectedInstance)
	CHECK_EDITOR
	if (inst == _SelectedInstance) return;
	forceSetSelectedInstance(inst);
}

// *********************************************************************************************************
void CEditor::forceSetSelectedInstance(CInstance *inst)
{
	//H_AUTO(R2_CEditor_forceSetSelectedInstance)
	CHECK_EDITOR
	if (_EnteredInSetSelectedInstance) return; // prevent recursive call (may happen because selection highlight an item in a menu, which trigger selection in turn)

	if (inst)
	{
		nlassert(inst->getSelectableFromRoot());
	}

	_EnteredInSetSelectedInstance = true;
	if (inst)
	{
		nlassert(hasInstance(inst)); // must have been added in the editor !
	}
	if (_SelectedInstance)
	{
		_SelectedInstance->onSelect(false);
	}
	_SelectedInstance = inst;
	if (_SelectedInstance)
	{
		_SelectedInstance->onSelect(true);
	}

	// call r2 method 'onSelectInstance'
	if (!_SelectedInstance)
	{
		getLua().pushNil();
	}
	else
	{
		_SelectedInstance->getLuaProjection().push();
	}
	callEnvMethod("onSelectInstance", 1, 0);
	_EnteredInSetSelectedInstance = false;

}

// *********************************************************************************************************
CInstance *CEditor::getSelectedInstance() const
{
	//H_AUTO(R2_CEditor_getSelectedInstance)
	CHECK_EDITOR
	return _SelectedInstance;
}

// *********************************************************************************************************
void CEditor::setFocusedInstance(CInstance *inst)
{
	//H_AUTO(R2_CEditor_setFocusedInstance)
	CHECK_EDITOR
	if (inst == _FocusedInstance) return;
	if (inst)
	{
		nlassert(hasInstance(inst)); // must have been added in the editor !
	}
	if (_FocusedInstance)
	{
		_FocusedInstance->onFocus(false);
	}
	_FocusedInstance = inst;
	if (_FocusedInstance)
	{
		_FocusedInstance->onFocus(true);
	}
}

// *********************************************************************************************************
CInstance *CEditor::getFocusedInstance() const
{
	//H_AUTO(R2_CEditor_getFocusedInstance)
	CHECK_EDITOR
	return _FocusedInstance;
}


// *********************************************************************************************************
bool CEditor::hasInstance(const CInstance *instance) const
{
	//H_AUTO(R2_CEditor_hasInstance)
	CHECK_EDITOR
	// TMP : slow test for debug
	for(TInstanceMap::const_iterator it = _Instances.begin(); it != _Instances.end(); ++it)
	{
		if (it->second == instance) return true;
	}
	return false;
}

// *********************************************************************************************************
bool CEditor::hasInstanceWithId(const TInstanceId &id) const
{
	//H_AUTO(R2_CEditor_hasInstanceWithId)
	CHECK_EDITOR
	return getInstanceFromId(id) != NULL;
}

// *********************************************************************************************************
CInstance *CEditor::getInstanceFromId(const TInstanceId &id) const
{
	//H_AUTO(R2_CEditor_getInstanceFromId)
	CHECK_EDITOR
	const CObjectTable   *objTable = getObjectTableFromId(id);
	if (!objTable) return NULL;
	return getInstanceFromObject(objTable);
}

// *********************************************************************************************************
sint CEditor::getGeneratedNameIndex(const std::string &nameUtf8, const std::string &baseNameUtf8)
{
	//H_AUTO(R2_CEditor_getGeneratedNameIndex)
	CHECK_EDITOR
	if (nameUtf8.size() >= baseNameUtf8.size() + 2)
	{
		if (nameUtf8.substr(0, baseNameUtf8.size()) == baseNameUtf8)
		{
			std::string::const_iterator strIt = nameUtf8.begin() + baseNameUtf8.size();
			std::string::const_iterator endStrIt = nameUtf8.end();
			if (*strIt == ' ')
			{
				++ strIt;
				const char *numberStart = &*strIt;
				for (; strIt != endStrIt && isdigit(*strIt); ++strIt) {}
				if (strIt == endStrIt)
				{
					sint ret;
					fromString(numberStart, ret);
					return ret;
				}
			}
		}
	}
	return -1;
}

// *********************************************************************************************************
bool CEditor::isPostFixedByNumber(const ucstring &baseName)
{
	//H_AUTO(R2_CEditor_isPostFixedByNumber)
	// strip number & spaces at the end of the name
	sint lastIndex = (sint)baseName.length() - 1;
	while (lastIndex > 0)
	{
		int currChar = (int) baseName[lastIndex];
		if (!isdigit(currChar) &&
			currChar != ' ' &&
			currChar != '\t')
		{
			break;
		}
		-- lastIndex;
	}
	return lastIndex != (sint) baseName.length() - 1;
}

// *********************************************************************************************************
ucstring CEditor::genInstanceName(const ucstring &baseName)
{
	//H_AUTO(R2_CEditor_genInstanceName)
	CHECK_EDITOR
	uint maxIndex = 0;
	// strip number & spaces at the end of the name
	ucstring strippedName = baseName;
	sint lastIndex = (sint)strippedName.length() - 1;
	while (lastIndex > 0)
	{
		int currChar = (int) strippedName[lastIndex];
		if (!isdigit(currChar) &&
			currChar != ' ' &&
			currChar != '\t')
		{
			break;
		}
		-- lastIndex;
	}
	strippedName = strippedName.substr(0, lastIndex + 1);
	std::string baseNameUtf8 = strippedName.toUtf8();
	//
	for(TInstanceMap::const_iterator it = _Instances.begin(); it != _Instances.end(); ++it)
	{
		maxIndex = (uint) std::max((sint) maxIndex, getGeneratedNameIndex(it->second->getName(), baseNameUtf8));
	}

	nlinfo("CEditor::genInstanceName newVersion");

	// Old part : used when there was a delay between net command and effect
	// did not work in some parts if genInstance is not followed by creation of an instance with that name
	// cf RT 11560
	/*
	// now, look in local generated names (instance that have not been already added to the scene)
	TGeneratedNameMap::iterator nameSetIt = _LocalGeneratedNames.find(baseNameUtf8);
	if (nameSetIt != _LocalGeneratedNames.end())
	{
		if (!nameSetIt->second.empty())
		{
			maxIndex = std::max(maxIndex, *(nameSetIt->second.rbegin())); // take bigger element of the set (the last element)
		}
	}
	// add max index in the map (will be removed when object is truly created)
	_LocalGeneratedNames[baseNameUtf8].insert(maxIndex + 1);
	*/
	return strippedName + " " + toString(maxIndex + 1);
}

// *********************************************************************************************************
void CEditor::setCookie(const TInstanceId &instanceId, const std::string &key, CLuaObject &value)
{
	//H_AUTO(R2_CEditor_setCookie)
	CHECK_EDITOR
	TCookieList &cl = _Cookies[instanceId];
	cl.push_front(CCookie());
	CCookie &cookie =  cl.front();  // don't build struct and push to avoid costly copy of a CLuaObject
	cookie.Key    = key;
	cookie.Value  = value;
}

// *********************************************************************************************************
void CEditor::setCookie(const TInstanceId &instanceId, const std::string &key, bool value)
{
	//H_AUTO(R2_CEditor_setCookie)
	CHECK_EDITOR
	getLua().push(value);
	CLuaObject obj(getLua());
	setCookie(instanceId, key, obj);
}

// *********************************************************************************************************
void CEditor::setCurrentTool(CTool *tool)
{
	//H_AUTO(R2_CEditor_setCurrentTool)
	CHECK_EDITOR
	if (_CurrentTool)
	{
		_CurrentTool->cancel();
	}
	else
	{
		if (tool)
		{
			// setting a tool in non R2 mode force to change the contextual cursor
			ContextCur.release();
			ContextCur.add(false,	"STAND BY",	DEFAULT_CURSOR,	0.0f,	CEditor::checkCursor,	CEditor::mouseClick);
			ContextCur.context("STAND BY");
		}
	}
	CTool::setContextHelp(ucstring(""));
	if (tool == NULL)
	{
		if (_Mode == EditionMode)
		{
			_CurrentTool = new CToolSelectMove;
		}
		else
		{
			_CurrentTool = NULL;
			ContextCur.release();
			::initContextualCursor();
			ContextCur.context("STAND BY");
			CTool::setMouseCursor(DEFAULT_CURSOR);
		}
	}
	else
	{
		_CurrentTool = tool;
	}
	if (_CurrentTool)
	{
		_CurrentTool->onActivate();
	}
	{
		CLuaStackChecker lsc(&getLua());
		// activate tool in the ui
		getLua().push(_CurrentTool ? _CurrentTool->getToolUIName() : "");
		getEnv()["ToolUI"].callMethodByNameNoThrow("setActiveToolUIByName", 1, 0);
	}
}

// *********************************************************************************************************
CLuaObject CEditor::getClasses() throw(ELuaError)
{
	//H_AUTO(R2_getClasses_throw)
	CHECK_EDITOR
	return getEnv().at("Classes");
}


// *********************************************************************************************************
bool CEditor::callEnvFunc(const char *funcName, int numArgs, int numRet /*=0*/)
{
	//H_AUTO(R2_CEditor_callEnvFunc)
	CHECK_EDITOR
	static volatile bool dumpStackWanted = false;
	if (dumpStackWanted) getLua().dumpStack();
	nlassert(funcName);
	nlassert(getLua().getTop() >= numArgs);
	int initialStackSize = getLua().getTop();
	getEnv().push();
	if (dumpStackWanted) getLua().dumpStack();
	getLua().insert(-1 - numArgs); // put the table before the args
	if (dumpStackWanted) getLua().dumpStack();
	int result = getLua().pcallByName(funcName, numArgs, numRet, -1 - numArgs);
	if (dumpStackWanted) getLua().dumpStack();
	if (result != 0)
	{
		nlwarning("Error while calling function %s : %s", funcName, getLua().toString());
		getLua().setTop(initialStackSize - numArgs + numRet); // clean the stack
		return false;
	}
	// remove the R2 table from the stack
	int newSize = getLua().getTop();
	if (dumpStackWanted) getLua().dumpStack();
	nlassert(newSize == initialStackSize + 1 - numArgs + numRet); // -1 is because of the r2 table
	if (dumpStackWanted) getLua().dumpStack();
	getLua().remove(- numRet - 1); // remove the 'R2' table
	if (dumpStackWanted) getLua().dumpStack();
	return true; // results remains on the stack
}

// *********************************************************************************************************
bool CEditor::callEnvMethod(const char *funcName, int numArgs, int numRet /*= 0*/)
{
	//H_AUTO(R2_CEditor_callEnvMethod)
	CHECK_EDITOR
	nlassert(funcName);
	nlassert(getLua().getTop() >= numArgs);
	getEnv().push();
	getLua().insert(-1 - numArgs); // put the table before the args (self parameter)
	return callEnvFunc(funcName, numArgs + 1, numRet);
}

// *********************************************************************************************************
bool CEditor::doLuaScript(const char *filename, const char *fileDescText)
{
	//H_AUTO(R2_CEditor_doLuaScript)
	CHECK_EDITOR
	CVerboseClock clock("parsing of " + std::string(filename));
	// load the classes definition file
	std::string filePath = NLMISC::CPath::lookup(filename, false, true);
	if (filePath.empty())
	{
		nlwarning("Can't find %s : %s", fileDescText, filename);
		return false;
	}

	if( 0 && FINAL_VERSION == 1) // disabled for the moment because there are lua file that must be loaded from example
	{
		const static std::string path = "data_common.bnp@";
		const static std::string::size_type len= path.size();

		if (filePath.size() < len || filePath.substr(0, len) != path)
		{
			nlwarning("Can't find %s : %s in ('%s')", fileDescText, filename, path.c_str());
			return false;
		}
	}


	try
	{
		if (!getLua().executeFile(filePath))
		{
			nlwarning("Couldn't open file %s : %s for R2 is not loaded", filename, fileDescText);
		}
		CLuaStackChecker ls(&getLua());
		return true;
	}
	catch(const NLMISC::EStream &e)
	{
		nlwarning("Error while loading R2 %s (file = %s) : %s", fileDescText, filename, e.what());
	}
	catch(const ELuaError &e)
	{
		//char filename[MAX_PATH];
		std::string msg = e.what();
		if (testWildCard(msg, "*.lua:*:*")) // TODO nico : more accurate testing for filename format
		{
			std::string::size_type extPos = msg.find(".lua");
			nlassert(extPos != std::string::npos);
			std::string filename = msg.substr(0, extPos + 4); // extract filename including extension
			int line;
			fromString(&*(msg.begin() + extPos + 5), line); // line number follows
			nlwarning((CLuaIHMRyzom::createGotoFileButtonTag(filename.c_str(), line) + e.what()).c_str());
		}
		else
		{
			// no filename / line in the result, so maybe the parse error was in an 'inline' script
			nlwarning("Error in R2 %s (file = %s) : %s", fileDescText, filename, e.what());
		}
	}
	return false;
}




///////////////////////
// CONTEXTUAL CURSOR //
///////////////////////



// *********************************************************************************************************
void CEditor::checkCursor()
{
	//H_AUTO(R2_CEditor_checkCursor)
	CHECK_EDITOR
	// no-op there : we delegate management of mouse to the CTool derived classes
}


// *********************************************************************************************************
void CEditor::mouseClick(bool rightButton, bool /* dblClick */)
{
	//H_AUTO(R2_CEditor_mouseClick)
	CHECK_EDITOR
	CTool *currentTool = getEditor().getCurrentTool();
	if (!currentTool) return;
	if (!rightButton)
	{
		// nb : result not handled there (no defaut action for left click)
		currentTool->onMouseLeftButtonClicked();
	}
	else
	{
		bool handled = currentTool->onMouseRightButtonClicked();
		if (!handled)
		{
			getEditor().displayContextMenu();
		}
	}
}



// *********************************************************************************************************
void CEditor::updatePreCamera()
{
	//H_AUTO(R2_CEditor_updatePreCamera)

	if (_Mode == EditionMode)
	{
		static uint32 loop = 0;
		++loop;
		if (loop % 200 == 0) // minimal wait between to save = 20 seconds
		{
			if ( (CTime::getLocalTime() -_LastAutoSaveTime)/1000 > ClientCfg.R2EDAutoSaveWait) // 5 minutes if not  change in Confile
			{
				autoSave();
			}
			loop = 0;
		}
		// if there's no pending action, then start a new one at each frame
		if (!_DMC->getActionHistoric().isPendingActionInProgress())
		{
			_DMC->newAction(CI18N::get("uiR2EDUnamedAction"));
		}
		else
		{
			_DMC->getActionHistoric().flushPendingAction();
		}
	}
}

// *********************************************************************************************************
void CEditor::updatePrimitiveContextualVisibility()
{
	//H_AUTO(R2_CEditor_updatePrimitiveContextualVisibility)
	if (!_SelectedInstance) return;
	// if selected instance remains in last contextual prim list, then they may remain visible
	bool ok = false;
	for(uint k = 0; k < _LastContextualPrims.size(); ++k)
	{
		if (_LastContextualPrims[k] == _SelectedInstance ||
			_SelectedInstance->isSonOf(_LastContextualPrims[k])
		   )
		{
			ok = true;
			break;
		}
	}
	_LastContextualPrims.clear();
	bool isLogicEntity = _SelectedInstance->isKindOf("LogicEntity");
	if (!ok && !isLogicEntity) return;
	if (isLogicEntity)
	{
		_LastContextualLogicEntity = _SelectedInstance;
	}
	if (!_LastContextualLogicEntity) return;
	//
	CObject *seq = _LastContextualLogicEntity->getGroupSelectedSequence();
	if (!seq) return;
	if (seq)
	{
		CObjectTable *activities = seq->toTable("Components");
		if (activities)
		{
			// get first world object parent to get start position
			for(uint k = 0; k < activities->getSize(); ++k)
			{
				// search next zone of activity
				CObjectTable *activity = activities->getValue(k)->toTable();
				if (!activity) continue;
				std::string zoneId = getString(activity, "ActivityZoneId");
				CInstance *primitive = getInstanceFromId(zoneId);
				if (primitive)
				{
					CDisplayerVisualGroup *primDisp = dynamic_cast<CDisplayerVisualGroup *>(primitive->getDisplayerVisual());
					if (primDisp)
					{
						_LastContextualPrims.push_back(primitive);
						primDisp->setContextualVisibilityDate(T1);
					}
				}
			}
		}
	}
}

// *********************************************************************************************************
CEntitySorter *CEditor::getEntitySorter() const
{
	return _EntitySorter;
}


// *********************************************************************************************************
void CEditor::updateBeforeRender()
{
	//H_AUTO(R2_CEditor_updateBeforeRender)
	CHECK_EDITOR
	//
	_IslandCollision.updateCurrPackedIsland();
	// update contextual visibility of primitive from current selection
	updatePrimitiveContextualVisibility();
	//
	if (ConnectionWanted) return; // TMP special case for connection
	if (_CurrentTool)
	{
		_CurrentTool->updateBeforeRender();
	}
	for(TInstanceMap::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
	{
		if (it->second->getDisplayerVisual())
		{
			it->second->getDisplayerVisual()->onPreRender();
		}
	}
	updateSelectingDecals();
	// hide or show user depending on the mode
	if (UserEntity)
	{
		if (_Mode == EditionMode || isDMing())
		{
			updateDecalBlendRegion(_PionneerDecal, UserEntity->pos());
			showDecal(CVector2f((float) UserEntity->pos().x, (float) UserEntity->pos().y), 1.f, _PionneerDecal, _PionneerDecalAnim);
		}
	}
	if ((_SelectedInstance && _SelectedInstance->maxVisibleEntityExceeded()) ||
		(_FocusedInstance && _FocusedInstance->maxVisibleEntityExceeded()))
	{
		setMaxVisibleEntityExceededFlag(true);
	}
	else
	{
		setMaxVisibleEntityExceededFlag(false);
	}
}

// *********************************************************************************************************
void CEditor::updateDecalBlendRegion(CDecal &decal, const NLMISC::CVector &pos)
{
	//H_AUTO(R2_CEditor_updateDecalBlendRegion)
	float topBlendDist = CV_DecalTopBlendStartDist.get();
	float bottomBlendDist = CV_DecalBottomBlendStartDist.get();
	float blendLength = CV_DecalBlendLength.get();
	decal.setBottomBlend(pos.z - bottomBlendDist - blendLength,
								  pos.z - bottomBlendDist);
	decal.setTopBlend(pos.z + topBlendDist,
								  pos.z + topBlendDist + blendLength);
}

// *********************************************************************************************************
void CEditor::updateBeforeSwapBuffer()
{
	//H_AUTO(R2_CEditor_updateBeforeSwapBuffer)
	CHECK_EDITOR
	if (_WaitScenarioScreenWanted)
	{
		waitScenario();
		_WaitScenarioScreenWanted= false;
	}
}

// *********************************************************************************************************
void CEditor::waitScenario()
{
	//H_AUTO(R2_CEditor_waitScenario)
   	_EditionModeDisconnectedFlag = false;
	_WaitScenarioScreenActive = true;
	waitScenarioScreen();
	_WaitScenarioScreenActive = false;


	if (_PostponeScenarioUpdated)
	{
		scenarioUpdated(_NewScenario, false, _NewScenarioInitialAct);
		_PostponeScenarioUpdated = false;
		_NewScenario = NULL;
	}


}

// *********************************************************************************************************
void CEditor::updateAfterRender()
{
	if (_EntitySorter) _EntitySorter->clipEntitiesByDist();
	//H_AUTO(R2_CEditor_updateAfterRender)
	_IslandCollision.updateCurrPackedIsland();
	CHECK_EDITOR
	if (ConnectionWanted)
	{
		connect();
		return;
	}
	//
	if (_CurrentTool)
	{
		_CurrentTool->updateAfterRender();
	}
	if (!_LuaUIMainLoop.isNil())
	{
		_LuaUIMainLoop.callMethodByNameNoThrow("onPostSceneRender", 0, 0);
	}
	//
	for(TInstanceMap::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
	{
		if (it->second->getDisplayerVisual())
		{
			it->second->getDisplayerVisual()->onPostRender();
		}
	}
}





// *********************************************************************************************************
void CEditor::autoSave()
{
	//H_AUTO(R2_CEditor_autoSave)



	_LastAutoSaveTime = NLMISC::CTime::getLocalTime();
	uint32 maxAutoSave = ClientCfg.R2EDAutoSaveSlot;

	std::string lastFile = toString("autosave_%u", maxAutoSave);


	uint32 i = maxAutoSave -1;

	for ( ; i != 0  ; --i )
	{
		std::string current = NLMISC::toString("autosave_%02u.r2", i);
		std::string next = NLMISC::toString("autosave_%02u.r2", i+1);

		if (CFile::fileExists(current))
		{
			if (CFile::fileExists(next)) // true only for i = maxAutoSave -1
			{
				CFile::deleteFile(next);
			}
			CFile::moveFile(next.c_str(), current.c_str());
		}
	}

	if (CFile::fileExists("save/r2_buffer.dat"))
	{
		CFile::copyFile("autosave_01.r2", "save/r2_buffer.dat");
	}

	R2::getEditor().getLua().executeScriptNoThrow("r2.Version.save(\"save/r2_buffer.dat\")");


}

// *********************************************************************************************************
bool CEditor::handleEvent (const NLGUI::CEventDescriptor &eventDesc)
{
	//H_AUTO(R2_CEditor_handleEvent )
	CHECK_EDITOR
	if (ConnectionWanted || !_CurrentTool) return false; // TMP special case for connection
	if (eventDesc.getType() == NLGUI::CEventDescriptor::system)
	{
		const NLGUI::CEventDescriptorSystem &eds = (const NLGUI::CEventDescriptorSystem &) eventDesc;
		if (eds.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::setfocus)
		{
			const NLGUI::CEventDescriptorSetFocus &edsf = (const NLGUI::CEventDescriptorSetFocus &) eds;
			if (edsf.hasFocus() == false)
			{
				// cancel current tool
				setCurrentTool(NULL);
				return true;
			}
		}
	}
	if (_CurrentTool)
	{
		return _CurrentTool->handleEvent(eventDesc);
	}
	return false;
}

// *********************************************************************************************************
void CEditor::copy()
{
	//H_AUTO(R2_CEditor_copy)
	// forward to lua
	callEnvMethod("copy", 0);
}

// *********************************************************************************************************
void CEditor::paste()
{
	//H_AUTO(R2_CEditor_paste)
	// forward to lua
	callEnvMethod("paste", 0);
}

// *********************************************************************************************************
/*
void CEditor::updateEvents()
{
	if (!_CurrentTool) return;
	//
	if(EventsListener.isMouseButtonPushed(leftButton))
	{
		nlwarning("onMouseLeftButtonDown");
		_CurrentTool->onMouseLeftButtonDown();
	}
	if(EventsListener.isMouseButtonReleased(leftButton))
	{
		nlwarning("isMouseButtonReleased");
		_CurrentTool->onMouseLeftButtonUp();
	}
	if(EventsListener.isMouseButtonPushed(rightButton))
	{
		nlwarning("isMouseButtonPushed");
		_CurrentTool->onMouseRightButtonDown();
	}
	if(EventsListener.isMouseButtonReleased(rightButton))
	{
		nlwarning("onMouseRightButtonUp");
		_CurrentTool->onMouseRightButtonUp();
	}
}*/

// *********************************************************************************************************
CEntityCL *CEditor::createEntity(uint slot, const NLMISC::CSheetId &sheetId, const NLMISC::CVector &pos, float heading, const std::string & permanentStatutIcon)
{
	//H_AUTO(R2_CEditor_createEntity)
	CHECK_EDITOR
	if (sheetId == NLMISC::CSheetId::Unknown) return NULL;
	CInterfaceManager *im = CInterfaceManager::getInstance();

	if (EntitiesMngr.entity(slot))
	{
		EntitiesMngr.remove(slot, false);
	}
	// Create the temporary entity in the entity manager
	TNewEntityInfo emptyEntityInfo;
	emptyEntityInfo.reset();
	CEntityCL *entity = EntitiesMngr.create(slot, sheetId.asInt(), emptyEntityInfo);
	if (!entity)
	{
		nlwarning("Can't create entity");
		return NULL;
	}

	// Set the permanent statut icon
	entity->setPermanentStatutIcon(permanentStatutIcon);

	// TMP TMP : code taken from /entity command
	sint64       *prop = 0;
	CCDBNodeLeaf *node = 0;
	// Set The property 'CLFECOMMON::PROPERTY_POSITION'.
	node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E" + NLMISC::toString("%d", slot)+":P" + NLMISC::toString("%d", CLFECOMMON::PROPERTY_POSX), false);
	if(node)
	{
		sint64 x = (sint64)(pos.x*1000.0);
		sint64 y = (sint64)(pos.y*1000.0);
		sint64 z = (sint64)(pos.z*1000.0);
		node->setValue64(x);
		node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_POSY), false);
		if(node)
		{
			node->setValue64(y);
			node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_POSZ), false);
			if(node)
				node->setValue64(z);
		}
	}
	// Set The property 'PROPERTY_ORIENTATION'.
	node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_ORIENTATION), false);
	if(node)
	{
		union
		{
			uint64 heading64;
			float  headingFloat;
		};
		headingFloat = heading;
		node->setValue64(heading64);
	}
	// Set Mode
	node = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_MODE), false);
	if(node)
	{
		MBEHAV::EMode m = MBEHAV::NORMAL;
		prop = (sint64 *)&m;
		node->setValue64(*prop);
		EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_MODE);
	}
	// Set Visual Properties
	SPropVisualA visualA;
	//visualA.PropertySubData.LTrail = 1;

	// Set alternate look
	SAltLookProp altLookProp;

	// fill infos for look
	prop = (sint64 *)&visualA;

	if(dynamic_cast<CPlayerCL *>(entity))
	{
		// visual property A depends on the type of the entity
		visualA.PropertySubData.Sex = ClientCfg.Sex;
	}
	else if(dynamic_cast<CPlayerR2CL *>(entity) == NULL)
	{
		// Get the database entry.
		// Get the old value (not useful since we change the whole property).
		altLookProp.Summary = 0;
		altLookProp.Element.ColorTop		= 0;
		altLookProp.Element.ColorBot		= 2;
		altLookProp.Element.WeaponRightHand	= 0;
		altLookProp.Element.WeaponLeftHand	= 0;
		altLookProp.Element.Seed			= 100;
		altLookProp.Element.ColorHair		= 4;
		altLookProp.Element.Hat				= 0;
		// old colors
		altLookProp.Element.ColorGlove		= altLookProp.Element.ColorTop;
		altLookProp.Element.ColorArm		= altLookProp.Element.ColorTop;
		altLookProp.Element.ColorBoot		= altLookProp.Element.ColorBot;

		// fill alt infos for look
		prop = (sint64 *)&altLookProp.Summary;
	}

	// Set the database.
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->setValue64(*prop);

	// Set Visual Properties
	SPropVisualB visualB;
	visualB.PropertySubData.LTrail = 1;
	// fill infos for look
	sint64       *propB = 0;
	propB = (sint64 *)&visualB;
	// Set the database.
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", slot)+":P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->setValue64(*propB);

	// Apply Changes.
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPA);
	EntitiesMngr.updateVisualProperty(0, slot, CLFECOMMON::PROPERTY_VPB);

	return entity;
}

// *********************************************************************************************************
CInstance *CEditor::getInstanceFromEntity(CEntityCL *entity) const
{
	//H_AUTO(R2_CEditor_getInstanceFromEntity)
	CHECK_EDITOR
	for(TInstanceMap::const_iterator it = _Instances.begin(); it != _Instances.end(); ++it)
	{
		if (it->second->getEntity() == entity) return it->second;
	}
	return NULL;
}

// *********************************************************************************************************
void CEditor::displayContextMenu()
{
	//H_AUTO(R2_CEditor_displayContextMenu)
	CHECK_EDITOR
	if (!_SelectedInstance)
	{
		// launch standard context menu (for free look & move options)
		getUI().launchContextMenuInGame("ui:interface:game_context_menu_edition");
		return;
	}
	if (getCurrentTool() && getCurrentTool()->isCreationTool())
	{
		setCurrentTool(NULL);
	}
	// retrieve the context menu depending on the type of the entity
	CLuaObject classDesc = _SelectedInstance->getClass();
	if (classDesc.isNil())
	{
		nlwarning("Can't retrieve object class");
		return;
	}
	std::string menu = classDesc["Menu"];
	if (menu.empty()) return; // no menu for that instance ?
	//
	CLuaObject menuSetupFunction = classDesc["onSetupMenu"];
	if (menuSetupFunction.isFunction())
	{
		_SelectedInstance->getLuaProjection().push();
		menuSetupFunction.callNoThrow(1, 0);
	}
	//
	getUI().launchContextMenuInGame(menu);
}



// *********************************************************************************************************
void CEditor::triggerInstanceObserver(const TInstanceId &id, IObserverAction &action)
{
	//H_AUTO(R2_CEditor_triggerInstanceObserver)
	TInstanceObserverMap::iterator lb = _InstanceObservers.lower_bound(id);
	TInstanceObserverMap::iterator ub = _InstanceObservers.upper_bound(id);
	if (lb == ub) return;
	// must do a copy, because an observer may erase himself from the list when it is triggered
	static std::vector<IInstanceObserver::TRefPtr> dest;
	dest.clear();
	for (TInstanceObserverMap::iterator it = lb; it != ub; ++it)
	{
		dest.push_back(it->second);
	}
	for (std::vector<IInstanceObserver::TRefPtr>::iterator it = dest.begin(); it != dest.end(); ++it)
	{
		if (*it) action.doAction(**it);
	}
}

// *********************************************************************************************************
void CEditor::onErase(CObject *object)
{
	//H_AUTO(R2_CEditor_onErase)
	bool dummyFoundInBase;
	std::string dummyNameInParent;
	onErase(object, dummyFoundInBase, dummyNameInParent);
}

// *********************************************************************************************************
void CEditor::onErase(CObject *root, bool &foundInBase, std::string &nameInParent)
{
	//H_AUTO(R2_CEditor_onErase)
	foundInBase = false;
	CHECK_EDITOR
	if (!root) return;
	CInstance *inst = getInstanceFromObject(root);
	// sons
	if (root->isTable())
	{
		for(uint k = 0; k < root->getSize(); ++k)
		{
			CObject *obj = root->getValue(k);
			if (obj->isTable())
			{
				onErase(obj);
			}
		}
	}
	//
	CObject *parent = NULL;
	//
	if (inst)
	{
		// Add an 'Erased' flag to the object so that
		// any pending property in the property sheet
		// won't send a 'requestSetNode' when it is closed
		try
		{
			(*inst).getLuaProjection()["User"].setValue("Erased", true);
		}
		catch (const ELuaNotATable &e)
		{
			nlwarning(e.what());
		}

		// if object is selected or focused, then clear these flags
		if (inst == getSelectedInstance())
		{
			setSelectedInstance(NULL);
		}
		if (inst == getFocusedInstance())
		{
			setFocusedInstance(NULL);
		}
	}

	// if object can be found in its base, then not really a deletion, but
	// rather a change of attribute to the "default value' (may happen in an undo operation)
	parent = root->getParent();
	if (parent)
	{
		sint32 sonIndex = parent->findIndex(root);
		if (sonIndex != -1)
		{
			nameInParent =  parent->getKey(sonIndex);
			if (!nameInParent.empty() && getDMC().getPropertyAccessor().hasValueInBase(parent, nameInParent))
			{
				foundInBase = true;
			}
		}
	}

	if (inst && !foundInBase)
	{
		// send event to instance & displayers
		inst->onErase();
		// trigger observers
		class CEraseNotification : public IObserverAction
		{
		public:
			CEraseNotification(CInstance &instance) : Instance(instance) {}
			virtual void doAction(IInstanceObserver &obs)
			{
				obs.onInstanceErased(Instance);
			}
			CInstance &Instance;
		};
		CEraseNotification eraseNotification(*inst);
		triggerInstanceObserver(inst->getId(), eraseNotification);
	}

	// if object has a user environment attached to it, remove it from lua
	if (inst)
	{
		CLuaStackChecker lsc(&getLua());
		getLua().pushLightUserData((void *) root);
		getLua().getTable(LUA_REGISTRYINDEX);
		if (!getLua().isNil())
		{
			getLua().pushLightUserData((void *) root); // key
			getLua().pushNil(); // value
			getLua().setTable(LUA_REGISTRYINDEX); // erase
		}
		getLua().pop();
	}

	if (root->isTable())
	{
		// special patch: ref ids under this object should not be triggered any more for that object
		CObjectTable *rootTable = root->toTable();
		for (uint32 k = 0; k < rootTable->getSize(); ++k)
		{
			CObject *obj = rootTable->getValue(k);
			CObjectRefIdClient *objRefId = dynamic_cast<CObjectRefIdClient *>(obj);
			if (objRefId)
			{
				objRefId->enable(false); // don't observe anything
			}
		}
	}

	if (!_ClearingContent)
	{
		if (inst)
		{
			nlassert(_Instances.count((const CObjectTable *) root) == 1);
			//nlwarning("Instance with id %s deleted, but not inserted", inst->getId().c_str());
		}
	}
	// really remove object
	//nlwarning("Removing instance with id %s (table = 0x%s)", inst->getId().c_str(), (int) root);
	if (inst)
	{
		_Instances.erase((const CObjectTable *) root);
	}

}

// *********************************************************************************************************
void CEditor::nodeErased(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CEditor_nodeErased)
	CHECK_EDITOR
	CObject *obj = _DMC->find(instanceId, attrName, position);
	if (!obj) return;
	bool foundInBase;
	std::string nameInParent;
	onErase(obj, foundInBase, nameInParent);
	CObject *parent = obj->getParent();
	_DMC->CDynamicMapClient::nodeErased(instanceId, attrName, position);
	if (foundInBase)
	{
		nlassert(parent);
		// erased, but reading in the base will give a new value, so the real
		// action from observers standpoint is 'modified'
		CInstance *parentInstance = getInstanceFromObject(parent);
		if (parentInstance)
		{
			onAttrModified(*parentInstance, nameInParent);
		}
		else
		{
			nlwarning("Can't found instance in which %s was modified", nameInParent.c_str());
		}
	}
	// warn the parent that it has been modified
	onAttrModified(parent);
	// NB : msg for deleted attribute does not exist yet, so the parent is warned that it is modified, but
	// 'onTableModified' is not called (because key doesn't exist any more)
}


void CEditor::onResetEditionMode()
{
	//H_AUTO(R2_CEditor_onResetEditionMode)
	CHECK_EDITOR
	// called when a scenario just before a scenario is created
	// no-op, to clean
}

void CEditor::onEditionModeConnected( uint32 userSlotId, uint32 adventureId, CObject* highLevel, const std::string& versionName, bool willTP, uint32 initialActIndex)
{
	//H_AUTO(R2_CEditor_onEditionModeConnected)
	CHECK_EDITOR
	if (!_WaitScenarioScreenActive)
	{
		setMode(EditionMode);
	}
	CInterfaceGroup *currentSessionGroup = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_current_session"));
	if (currentSessionGroup)
	{
		CViewText *text = dynamic_cast<CViewText *>(currentSessionGroup->getView("current_session"));
		if (text)
		{
			text->setText(toString("Edition Session = %d (%s)", adventureId, versionName.c_str()));
		}
	}
	setDefaultChatWindow(PeopleInterraction.ChatGroup.Window);
	_DMC->CDynamicMapClient::onEditionModeConnected(userSlotId, adventureId, highLevel, versionName, willTP, initialActIndex);
}

void CEditor::setAccessMode(TAccessMode mode)
{
	//H_AUTO(R2_CEditor_setAccessMode)
	_AccessMode = mode;
	switch ( mode)
	{
		case AccessEditor:			_Env.setValue("AccessMode", "Editor"); break;
		case AccessDM:
			_Env.setValue("AccessMode", "DM");
			if (_Mode == AnimationModePlay)
			{
				loadStandardUI();
			}
		break;
		case AccessOutlandOwner:	_Env.setValue("AccessMode", "OutlandOwner"); break;
		default:
			nlassert(0);
		break;
	}
}

void CEditor::onAnimationModeConnected(const CClientMessageAdventureUserConnection& connected)
{
	//H_AUTO(R2_CEditor_onAnimationModeConnected)
	_ScenarioReceivedFlag = true; // end wait screen
	// TMP PATCH
	// _WillTP = true;
	CHECK_EDITOR
	switch(connected.Mode)
	{
		case 0: setMode(AnimationModeLoading); break;
		case 1: setMode(AnimationModeWaitingForLoading); break;
		case 2: setMode(AnimationModeDm); break;
		case 3: setMode(AnimationModePlay); break;
		default: nlwarning("Unhandled %u in Animation Session", connected.Mode);

	}

	CInterfaceGroup *currentSessionGroup = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_current_session"));
	if (currentSessionGroup)
	{
		CViewText *text = dynamic_cast<CViewText *>(currentSessionGroup->getView("current_session"));
		if (text)
		{
			text->setText(toString("Animation Session = %d (%s)", connected.SessionId.asInt(), connected.VersionName.c_str()));
		}
	}
	setDefaultChatWindow(PeopleInterraction.ChatGroup.Window);
	_DMC->CDynamicMapClient::onAnimationModeConnected(connected);
}

void  CEditor::onEditionModeDisconnected()
{
	//H_AUTO(R2_CEditor_onEditionModeDisconnected)
	_EditionModeDisconnectedFlag = true;
	delete _NewScenario;
	_NewScenario = NULL;
	CHECK_EDITOR
	// Useful only for the pionner that does not do requestTranslateFeatures()
	// Because avec using the button the currentScenario = 0
	try
	{
		R2::getEditor().getLua().executeScript("r2.Version.save(\"save/r2_buffer.dat\")");
	}
	catch (const std::exception& e)
	{
		nlwarning("Can't start Edition Mode", e.what());
	}
	_DMC->CDynamicMapClient::onEditionModeDisconnected();
}

void  CEditor::onTestModeConnected()
{
	//H_AUTO(R2_CEditor_onTestModeConnected)
	CHECK_EDITOR
	// TODO nico : change the name of the function : should rather be 'onAnimationModeConnected'
	// start as a GM
	CAHManager::getInstance()->runActionHandler("r2ed_anim_dm_mode", NULL, "");
	_DMC->CDynamicMapClient::onTestModeConnected();
}

void  CEditor::onTestModeDisconnected(TSessionId sessionId, uint32 lastAct, TScenarioSessionType sessionType)
{
	//H_AUTO(R2_CEditor_onTestModeDisconnected)
	CHECK_EDITOR
	_DMC->CDynamicMapClient::onTestModeDisconnected(sessionId, lastAct, sessionType);

}


// *********************************************************************************************************
void CEditor::nodeInserted(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& key, CObject* value)
{
	//H_AUTO(R2_CEditor_nodeInserted)
	CHECK_EDITOR
	_DMC->CDynamicMapClient::nodeInserted(instanceId, attrName, position, key, value);
	if (value->isTable())
	{
		std::string id = getString(value, "InstanceId");
 		if (!id.empty())
		{
			const CObject *clonedValue = _DMC->find(id);
			// bad insert
			if (clonedValue)
			{
				createNewInstanceForObjectTable(clonedValue);

				onAttrModified(clonedValue->getParent()); // parent table is modified
														  // so -> only parent of parent table will be notified of change
														  // maybe we need another msg "onInsert" that is sent
														  // to the parent table.
			}
		}
	}
}

// *********************************************************************************************************
void CEditor::createNewInstanceForObjectTable(const CObject *obj)
{
	//H_AUTO(R2_CEditor_createNewInstanceForObjectTable)
	CHECK_EDITOR
	createNewInstanceForObjectTableInternal(obj);
	onPostCreate(obj);
}

// *********************************************************************************************************
void CEditor::notifyInstanceObserversOfCreation(CInstance &inst)
{
	//H_AUTO(R2_CEditor_notifyInstanceObserversOfCreation)
	CHECK_EDITOR
	// if there are observers watching this instance, warn them
	const std::string &id = inst.getId();
	// trigger observers
	class CCreationNotification : public IObserverAction
	{
	public:
		CCreationNotification(CInstance &instance) : Instance(instance) {}
		virtual void doAction(IInstanceObserver &obs)
		{
			obs.onInstanceCreated(Instance);
		}
		CInstance &Instance;
	};
	CCreationNotification creationNotification(inst);
	triggerInstanceObserver(id, creationNotification);
}

// *********************************************************************************************************
void CEditor::onPostCreate(const CObject *obj)
{
	//H_AUTO(R2_CEditor_onPostCreate)
	CHECK_EDITOR
	struct CPostCreateVisitor : public IObjectVisitor
	{
		virtual void visit(CObjectTable &obj)
		{
			// if table has a "InstanceId" field there is a matching editor object
			CInstance *inst = getEditor().getInstanceFromObject(&obj);
			if (inst)
			{
				inst->onPostCreate();
				getEditor().notifyInstanceObserversOfCreation(*inst);
			}
		}
	};
	if (obj)
	{
		CPostCreateVisitor postCreateVisitor;
		const_cast<CObject *>(obj)->visit(postCreateVisitor);
	}
}

// *********************************************************************************************************
sint CEditor::getLeftQuota()
{
	//H_AUTO(R2_CEditor_getLeftQuota)
	CLuaState &ls = getLua();
	CLuaStackChecker lsc(&ls);
	callEnvMethod("getLeftQuota", 0, 1);
	if (!ls.isNumber(-1))
	{
		ls.pop(1);
		return 0;
	}
	sint result = (sint) ls.toNumber(-1);
	ls.pop(1);
	return result;
}

// *********************************************************************************************************
bool CEditor::checkRoomLeft()
{
	//H_AUTO(R2_CEditor_checkRoomLeft)
	return getLeftQuota() > 0;
}

// *********************************************************************************************************
void CEditor::makeRoomMsg()
{
	//H_AUTO(R2_CEditor_makeRoomMsg)
	// delegate ui display to lua
	callEnvMethod("makeRoomMsg", 0, 0);
}


// *********************************************************************************************************
bool CEditor::verifyRoomLeft(uint aiCost, uint staticCost)
{
	//H_AUTO(R2_CEditor_verifyRoomLeft)

	CLuaState &ls = getLua();
	if (aiCost)
	{
		CLuaStackChecker lsc(&ls);
		getEditor().getLua().push((lua_Number)aiCost);
		callEnvMethod("checkAiQuota", 1, 1);
		if (!ls.isBoolean(-1))
		{
			ls.pop(1);
			return false;
		}
		sint result = (sint) ls.toBoolean(-1);
		ls.pop(1);
		return result != 0;
	}
	if (staticCost)
	{
		CLuaStackChecker lsc(&ls);
		getEditor().getLua().push((lua_Number)staticCost);
		callEnvMethod("checkStaticQuota", 1, 1);
		if (!ls.isBoolean(-1))
		{
			ls.pop(1);
			return false;
		}
		sint result = (sint) ls.toBoolean(-1);
		ls.pop(1);
		return result != 0;
	}
	return true;

}



// *********************************************************************************************************
void CEditor::createNewInstanceForObjectTableInternal(const CObject *obj)
{
	//H_AUTO(R2_CEditor_createNewInstanceForObjectTableInternal)
	CHECK_EDITOR
	if (!obj) return;
	if (!obj->isTable()) return; // not a table ...
	const CObjectTable *table = (const CObjectTable *) obj;
	std::string id = getString(obj, "InstanceId");
	if (!id.empty())
	{
		// CInstance is created only for objects with an instance id
		CInstance *inst = new CInstance(table, getLua());
		nlassert(_Instances.count(table) == 0);
		_Instances[table] = inst;
		// if a cookie was created, add in in the instance lua 'User' table
		TCookieMap::iterator cookieList = _Cookies.find(id);
		if (cookieList != _Cookies.end())
		{
			if (obj->isTable())
			{
				CLuaState &ls = getLua();
				getLuaUserTableFromObject(ls, * (CObjectTable *) obj);
				CLuaObject userTable(ls); // pop the table into a CLuaObject for convenience
				for (TCookieList::iterator it = cookieList->second.begin(); it != cookieList->second.end(); ++it)
				{
					userTable.setValue(it->Key, it->Value);
				}
			}
			_Cookies.erase(cookieList);
		}
		// create displayers
		CLuaObject classDesc = inst->getClass();
		if (!classDesc.isNil())
		{
			CLuaStackChecker lsc(&getLua());
			CDisplayerVisual *dispViz= createObjectFromClassName<CDisplayerVisual>(classDesc["DisplayerVisual"].toString());
			if (dispViz)
			{
				bool ok = dispViz->init(classDesc["DisplayerVisualParams"]);
				if (!ok)
				{
					nlwarning("Error when calling init on visual displayer of class %s", classDesc["Name"].toString().c_str());
				}
			}
			inst->setDisplayerVisual(dispViz);
			//
			CDisplayerBase *dispUI = createObjectFromClassName<CDisplayerBase>(classDesc["DisplayerUI"].toString());
			if (dispUI)
			{
				bool ok = dispUI->init(classDesc["DisplayerUIParams"]);
				if (!ok)
				{
					nlwarning("Error when calling init on  ui displayer of class %s", classDesc["Name"].toString().c_str());
				}
			}
			inst->setDisplayerUI(dispUI);
			//
			CDisplayerBase *dispProp = createObjectFromClassName<CDisplayerBase>(classDesc["DisplayerProperties"].toString());
			if (dispProp)
			{
				bool ok = dispProp->init(classDesc["DisplayerPropertiesParams"]);
				if (!ok)
				{
					nlwarning("Error when calling init on property displayer of class %s", classDesc["Name"].toString().c_str());
				}
			}
			inst->setDisplayerProperties(dispProp);
		}
		// prevent completion of the tree while an instance is being created
		//backupRequestCommands();
		inst->onCreate();
		//restoreRequestCommands();
		//
		// if a name was generated locally, erase from the local name map.
		std::string className = getString(obj, "Class");
		std::string name = getString(obj, "Name");
		if (!className.empty() && !name.empty())
		{
			for (TGeneratedNameMap::iterator it = _LocalGeneratedNames.begin(); it != _LocalGeneratedNames.end(); ++it)
			{
				sint index = getGeneratedNameIndex(name, it->first);
				if (index != -1)
				{
					it->second.erase(index);
					break;
				}
			}
		}
	}
	// do the same on sons
	for(uint k = 0; k < table->getSize(); ++k)
	{
		createNewInstanceForObjectTableInternal(table->getValue(k));
	}
}


// *********************************************************************************************************
void CEditor::onAttrModified(CInstance &parentInstance, const std::string &attrName, sint32 indexInArray)
{
	//H_AUTO(R2_CEditor_onAttrModified)
	parentInstance.onAttrModified(attrName, indexInArray);
	class CAttrModifiedNotification : public IObserverAction
	{
	public:
		CAttrModifiedNotification(CInstance &instance, const std::string &key, sint32 indexInArray)
			: Instance(instance), Key(key), IndexInArray(indexInArray)  {}
		virtual void doAction(IInstanceObserver &obs)
		{
			obs.onAttrModified(Instance, Key, IndexInArray);
		}
		CInstance		  &Instance;
		const std::string &Key;
		sint32			  IndexInArray;
	};
	CAttrModifiedNotification attrModifiedNotification(parentInstance, attrName, indexInArray);
	triggerInstanceObserver(parentInstance.getId(), attrModifiedNotification);
}

// *********************************************************************************************************
void CEditor::onAttrModified(const CObject *value)
{
	//H_AUTO(R2_CEditor_onAttrModified)
	CHECK_EDITOR
	if (!value) return;
	const CObject *son = value;
	const CObject *parent = value->getParent();
	sint32 indexInArray  = -1;
	while (parent)
	{
		CInstance *parentInstance = getInstanceFromObject(parent);
		sint32 indexInParent = parent->findIndex(son);
		nlassert(indexInParent != -1);
		if (parentInstance)
		{
			// we are in an instance (a CObjectTable with an instance id)
			// TODO nico : a cache for the 'name' in the parent like with CObjectRefId ...
			std::string key = parent->getKey(indexInParent);
			onAttrModified(*parentInstance, key, indexInArray);
			indexInArray = -1;
		}
		else
		{
			// we are in an array in an instance -> memorize index in that array for next call to "onAttrModified"...
			indexInArray = indexInParent;
		}
		son = parent;
		parent = parent->getParent();
	}
}


// *********************************************************************************************************
void CEditor::nodeSet(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CEditor_nodeSet)
	CHECK_EDITOR
	CObject *obj = _DMC->find(instanceId);
	if (!obj) return;
	// erase previous object
	nlassert(obj->isTable());
	//
	if (!attrName.empty())
	{
		obj = obj->findAttr(attrName);
	}
	if (obj)
	{
		onErase(obj);
	}
	// change the actual value
	_DMC->CDynamicMapClient::nodeSet(instanceId, attrName, value);
	//
	nlassert(!getInstanceFromObject(value)); // this must be a new object...
	//

	if (value->isTable())
	{
		std::string id = getString(value, "InstanceId");
		if (!id.empty())
		{
			const CObject *clonedValue = _DMC->find(id);
			if (clonedValue)
			{
				createNewInstanceForObjectTable(clonedValue);	// if the created object is a table, warn him that he has been created
				onAttrModified(clonedValue);
				return;
			}
		}
	}
	// no instance id for object, retrieve object pointer from parent
	const CObject *parent = _DMC->find(instanceId);
	if (!parent) return;
	onAttrModified(parent->getAttr(attrName));
}

// *********************************************************************************************************
void CEditor::eraseScenario()
{
	//H_AUTO(R2_CEditor_eraseScenario)
	if (_Scenario)
	{

		backupRequestCommands();
		onErase(_Scenario);
		restoreRequestCommands();
		_Scenario = NULL;
	}

}

// *********************************************************************************************************
void CEditor::scenarioUpdated(CObject* highLevel, bool willTP, uint32 initialActIndex)
{
	//H_AUTO(R2_CEditor_scenarioUpdated)
	CHECK_EDITOR
	_ScenarioReceivedFlag = true;

	if (_WaitScenarioScreenActive)
	{
		// defer scenario update to the end of the wait screen
		_NewScenario = highLevel ? highLevel->clone() : NULL;
		_NewScenarioInitialAct = initialActIndex;
		_PostponeScenarioUpdated = true;
		return;
	}

 	if (!highLevel)
	{
		_IsStartingScenario = false;
		_WillTP = false;
		callEnvFunc("onEmptyScenarioUpdated", 0);
		return;
	}

	// _WillTP = willTP;
	_WillTP = false; // TMP TMP
	_UpdatingScenario = true;


	_BaseAct = NULL;
	_CurrentAct = NULL;
	CLuaStackRestorer lsc(&getLua(), getLua().getTop());
	//nlwarning("Scenario updated, start highlevel = ");
	//highLevel->dump();

	eraseScenario();

	_IsStartingScenario = false;
	if (_DMC->getEditionModule().getMustStartScenario())
	{
		_IsStartingScenario = true;
	}


	nlassert(_Instances.empty());
	nlassert(CDisplayerBase::ObjCount == 0);
	//
	_DMC->CDynamicMapClient::scenarioUpdated(highLevel, willTP, initialActIndex);
	//nlwarning("Scenario updated, content is  = ");
	/*
	if (_DMC->getHighLevel())
	{
		_DMC->getHighLevel()->dump();
	}
	else
	{
		nlwarning("NULL");
	}
	*/
	createNewInstanceForObjectTableInternal(_DMC->getHighLevel());
	nlassert(highLevel->isTable());
	_Scenario = (CObjectTable *) _DMC->getHighLevel();
	if (_Scenario->getAttr("InstanceId"))
	{
		_ScenarioInstance = getInstanceFromId(_Scenario->getAttr("InstanceId")->toString());
	}
	else
	{
		_ScenarioInstance = NULL;
		nlwarning("Can't retrieve scenario (no instance id)");
		_Scenario->dump();
	}

	static volatile bool forceDump = false;
	if (forceDump)
	{
		_Scenario->dump();
	}
	//
	projectInLua(_Scenario); // push on the lua stack
	getLua().push(float(initialActIndex)); // example reconnect after test in act4
	// update value in the framework
	callEnvFunc("onScenarioUpdated", 2);
	//nlwarning("Instance list now is :");
	//dumpInstances();
	CObject *acts = _Scenario->getAttr("Acts");
	if (acts)
	{
		CObject *baseAct = acts->getValue(0);
		if (baseAct)
		{
			_BaseAct = getInstanceFromId(baseAct->toString("InstanceId"));
		}
	}
	if (!_BaseAct)
	{
		nlwarning("Base act not found at scenario update");
	}
	if (_WantedActOnInit.empty())
	{
		if (!_CurrentAct) // if act not currently setted at scenario creation ...
		{
			setCurrentAct(_BaseAct); // ...then default to the base act
		}
	}
	else
	{
		setCurrentActFromTitle(_WantedActOnInit);
		_WantedActOnInit.clear();
	}

	_DMC->getActionHistoric().clear(highLevel); // reinit the undo / redo stack
	onPostCreate(_Scenario); // post creation require that current act has been set

	//TP is done via the onTpPositionSimulated message
	/*
	// teleport in good island
	if (ClientCfg.Local)
	{
		sint locationId = (uint) _ScenarioInstance->getLuaProjection()["Description"]["LocationId"].toNumber();

		CScenarioEntryPoints &sep = CScenarioEntryPoints::getInstance();
		_IslandCollision.loadEntryPoints();
		if (sep.getCompleteIslands().empty())
		{
			nlwarning("Entry points not loaded, teleport not done (local mode)");
		}
		else if (locationId >= (sint) sep.getCompleteIslands().size())
		{
			nlwarning("Bad location id %d", locationId);
		}
		else
		{
			// check if already in this entry point (no tp if so)
			const CScenarioEntryPoints::CCompleteIsland &ci = sep.getCompleteIslands()[locationId];
			CVectorD playerPos = UserEntity->pos();
			//
			if (playerPos.x <= ci.XMin ||
				playerPos.x >= ci.XMax ||
				playerPos.y <= ci.YMin ||
				playerPos.y >= ci.YMax)
			{
				if(ci.EntryPoints.size()>0)
				{
					const CScenarioEntryPoints::CShortEntryPoint & shortEntryPoint = ci.EntryPoints[0];
					CVector dest((float) shortEntryPoint.X, (float) shortEntryPoint.Y, 0.f);

					UserEntity->pos(dest); // change position in pacs
					// Select the closest continent from the new position.
					beginLoading (LoadingBackground);
					#define BAR_STEP_TP 2 // fixme : this define is duplicated....
					ProgressBar.reset (BAR_STEP_TP);
					ucstring nmsg("Loading...");
					ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
					ProgressBar.progress(0);
					ContinentMngr.select(dest, ProgressBar);
					endLoading();
					// Teleport the User.
					UserEntity->tp(dest);
				}
			}
		}
	}
*/

	_UpdatingScenario = false;


	if (_DMC->getEditionModule().getMustStartScenario()
		&& _DMC->getEditionModule().getScenarioUpToDate())
	{
		_DMC->getEditionModule().setMustStartScenario( false);
		if ( _DMC->getCurrentScenario()->getHighLevel())
		{

			bool result = true;
			if (ClientCfg.R2EDMustVerifyRingAccessWhileLoadingAnimation)
			{
				CLuaState &ls = getLua();
				CLuaStackChecker lsc(&ls);
				callEnvFunc( "verifyScenario", 0, 1);
				if (!ls.isBoolean(-1))
				{
					nlassert(0 && "verifyScenario return wrong type");
				}
				result = ls.toBoolean(-1);
				ls.pop();
			}

			if (result) //Start scenario only if allowed
			{
				ConnectionWanted = true; // ugly
			}
		}
	}
}

// *********************************************************************************************************
CInstance *CEditor::getDefaultFeature(CInstance *act)
{
	//H_AUTO(R2_CEditor_getDefaultFeature)
	CHECK_EDITOR
	if (!act) return NULL;
	CObject *defaultFeature = act->getObjectTable()->getAttr("Features");
	if (!defaultFeature) return NULL;
	defaultFeature = defaultFeature->getValue(0);
	if (!defaultFeature) return NULL; // 0 should be the default feature
	CInstance *result = getInstanceFromId(defaultFeature->toString("InstanceId"));
	if (!result) return NULL;
	if (!result->isKindOf("DefaultFeature"))
	{
		nlwarning("Can't retrieve default feature.");
	}
	return result;
}

// *********************************************************************************************************
CInstance *CEditor::getDefaultFeature()
{
	//H_AUTO(R2_CEditor_getDefaultFeature)
	CHECK_EDITOR
	return getDefaultFeature(getCurrentAct());
}

// *********************************************************************************************************
void CEditor::nodeMoved(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CEditor_nodeMoved)
	CHECK_EDITOR
	const CObject *src = _DMC->find(instanceId, attrName, position);
	if (!src) return;
	//
	CObject *oldParent = src->getParent();
	//
	CInstance *inst = getInstanceFromObject(src);
	// tells object that he is about to move
	if (inst)
	{
		inst->onPreHrcMove();
		// notify possible observers that this instance will move
		class CPreHrcMoveNotification : public IObserverAction
		{
		public:
			CPreHrcMoveNotification(CInstance &instance) : Instance(instance) {}
			virtual void doAction(IInstanceObserver &obs)
			{
				obs.onPreHrcMove(Instance);
			}
			CInstance		  &Instance;
		};
		CPreHrcMoveNotification preHrcMoveNotification(*inst);
		triggerInstanceObserver(inst->getId(), preHrcMoveNotification);
	}
	// do the move
	_DMC->CDynamicMapClient::nodeMoved(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
	// warn the previous parent that it has been modified
	if (src->getParent() != oldParent)
	{
		onAttrModified(oldParent); // the old parent is modified so send appropriate msg
	}
	// else ... if new parent is the same then send message only at the end (else object believe it has been modified twice in a row)
	// tells object that he has moved
	if (inst)
	{
		inst->onPostHrcMove();
		// notify possible observers that this instance has moved
		class CPostHrcMoveNotification : public IObserverAction
		{
		public:
			CPostHrcMoveNotification(CInstance &instance) : Instance(instance) {}
			virtual void doAction(IInstanceObserver &obs)
			{
				obs.onPostHrcMove(Instance);
			}
			CInstance		  &Instance;
		};
		CPostHrcMoveNotification postHrcMoveNotification(*inst);
		triggerInstanceObserver(inst->getId(), postHrcMoveNotification);
	}
	onAttrModified(src); // the new parent is modified so send appropriate msg
}

// *********************************************************************************************************
void CEditor::dumpInstances()
{
	//H_AUTO(R2_CEditor_dumpInstances)
	CHECK_EDITOR
	for(TInstanceMap::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
	{
		nlwarning("Obj = %p, id = %s, instance = %p", it->first, it->second->getId().c_str(), &(*it->second));
	}
}

// ***************************************************************
struct CSortSelectableObject
{
	ISelectableObject	*Object;
	float				Depth;
	CVector				RayStart, RayStartClipped, RayEndClipped;
	NLMISC::CAABBox		SelectBox;
	bool				SelectBoxInWorld;
	bool	operator<(const CSortSelectableObject &o) const
	{
		return Depth<o.Depth;
	}
};
// special selectable object to detect that mouse is over user
class CSelectableUser : public ISelectableObject
{
public:
	virtual bool			isSelectable() const { return true; }
	virtual bool			getLastClip() const { return UserEntity->getLastClip(); }
	virtual NLMISC::CAABBox getSelectBox() const
	{
		return UserEntity->localSelectBox();
	}
	virtual const NLMISC::CMatrix &getInvertedMatrix() const
	{
		static CMatrix invertedMatrix;
		invertedMatrix = UserEntity->dirMatrix();
		invertedMatrix.setPos(UserEntity->pos());
		invertedMatrix.invert();
		return invertedMatrix;
	}
	virtual float			preciseIntersectionTest(const NLMISC::CVector &worldRayStart, const NLMISC::CVector &worldRayDir) const
	{
		if (!UserEntity) return FLT_MAX;
		return CEditor::preciseEntityIntersectionTest(*UserEntity, worldRayStart, worldRayDir);
	}
	virtual CInstance		*getInstanceInEditor() const { return NULL; }
};

// ***************************************************************
static CSelectableUser SelectableUser;
//
CInstance *CEditor::getInstanceUnderPos(float x, float y, float distSelection, bool &isPlayerUnderCursor)
{
	static volatile bool ignore = false;
	if (ignore)
	{
		return NULL;
	}
	//H_AUTO(R2_CEditor_getInstanceUnderPos)
	CHECK_EDITOR
	// TODO nico: this code was copied from CEntityManager::getEntityUnderPos
	// then modified, so some factoring could be made ...



	uint	i;

	// valid only if bbox still intersect
	CInstance	*precInstanceUnderPos= _LastInstanceUnderPos;
	bool		precInstanceUnderPosValid= false;


	// reset result
	isPlayerUnderCursor= false;
	_LastInstanceUnderPos = NULL;


	// build the ray
	CMatrix camMatrix = MainCam.getMatrix();
	NL3D::CFrustum camFrust = MainCam.getFrustum();
	NL3D::CViewport viewport = Driver->getViewport();

	// Get the Ray made by the mouse.
	CTool::CWorldViewRay worldViewRay;
	worldViewRay.OnMiniMap = false;
	worldViewRay.Valid = true;
	//
	viewport.getRayWithPoint(x, y, worldViewRay.Origin, worldViewRay.Dir, camMatrix, camFrust);
	worldViewRay.Dir.normalize();
	worldViewRay.Right = camMatrix.getI().normed();
	worldViewRay.Up = camMatrix.getK().normed();


	// **** Get entities with box intersecting the ray.
	static	std::vector<ISelectableObject *>		validObjects;
	validObjects.clear();
	static	std::vector<CSortSelectableObject>		intersectedObjects;
	intersectedObjects.clear();


	ISelectableObject *precSelectableObject = NULL;

	validObjects.push_back(&SelectableUser); // add fake object for test with user entity
	for(TInstanceMap::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
	{
		CInstance *instance = it->second;
		CDisplayerVisual *vd = instance->getDisplayerVisual();
		if (!vd) continue;
		if (!vd->isSelectable()) continue;
		if (instance == getEditor().getSelectedInstance())
		{
			precSelectableObject = vd;
		}
		validObjects.push_back(vd);
	}

	// compute intersection of mouse with landscape / batiments for test with projected object like region & decals
	CVector sceneInter;
	CTool::TRayIntersectionType rayInterType = CTool::computeLandscapeRayIntersection(worldViewRay, sceneInter);

	CSortSelectableObject	selectObj;
	ISelectableObject *borderSelected = NULL;
	for(uint k = 0; k < validObjects.size(); ++k)
	{
		ISelectableObject *object = validObjects[k];
		// if entity not visible, skip
		if(object->getLastClip())
			continue;
		float depth = FLT_MAX;
		ISelectableObject::TSelectionType selectionType = object->getSelectionType();
		bool borderSelection = false;
		switch(selectionType)
		{
			case ISelectableObject::GroundProjected:
				if (rayInterType != CTool::NoIntersection)
				{
					CVector2f testPos(sceneInter.x, sceneInter.y);
					borderSelection = object->isInProjectionBorder(testPos);
					if (borderSelection || object->isInProjection(testPos))
					{
						depth = (worldViewRay.Origin - sceneInter).norm();
					}
				}
			break;
			case ISelectableObject::LocalSelectBox:
			case ISelectableObject::WorldSelectBox:
			{
				const CMatrix &rayMatrix = (selectionType == ISelectableObject::LocalSelectBox) ? object->getInvertedMatrix()
					                                                                               : CMatrix::Identity;
				selectObj.RayStart = rayMatrix * worldViewRay.Origin;
				selectObj.RayStartClipped = selectObj.RayStart;
				selectObj.RayEndClipped = rayMatrix * (worldViewRay.Origin + worldViewRay.Dir * distSelection);
				// if intersect the bbox
				selectObj.SelectBox = object->getSelectBox();
				selectObj.SelectBoxInWorld = false;
				if (selectObj.SelectBox.clipSegment(selectObj.RayStartClipped, selectObj.RayEndClipped))
				{
					depth = (selectObj.RayStartClipped - selectObj.RayStart).norm();
					// is it the last entity under pos?
					if(object->getInstanceInEditor() ==precInstanceUnderPos)
						precInstanceUnderPosValid= true;
				}
			}
			break;
			default:
				nlassert(0);
			break;
		}



		if (depth != FLT_MAX)
		{
			selectObj.Object		= object;
			selectObj.Depth			= depth;
			if (borderSelection)
			{
				borderSelected = object;
			}
			// add this entity to the list of possible entities
			intersectedObjects.push_back(selectObj);
		}
	}

	// if no intersected entities, quit
	if(intersectedObjects.empty())
		return NULL;

	// Compute startDistBox: nearest entity distance, but the user
	float	startDistBox;

	if(intersectedObjects[0].Object == &SelectableUser)
	{
		// if the nearest entity is the user, set res
		isPlayerUnderCursor= true;
		// if only player intersected, return NULL!
		if(intersectedObjects.size()==1)
			return NULL;
		// so take the second for startDistBox
		startDistBox= intersectedObjects[1].Depth;
	}
	else
	{
		// ok, take it.
		startDistBox= intersectedObjects[0].Depth;
	}



	/*static std::vector<ISelectableObject *> projectedObjects;
	projectedObjects.clear();*/

	// **** get best entity according to distance face-camera or box-ray if no face intersection
	ISelectableObject	*objectSelected= NULL;
	float		bestDistBox= FLT_MAX;
	float		bestDistZ= FLT_MAX;
	for(i=0;i<intersectedObjects.size();i++)
	{
		ISelectableObject	*object = intersectedObjects[i].Object;

		// If this entity is the UserEntity, skip!!
		if(object == &SelectableUser)
			continue;

		float	distZ = 0.f;
		bool	preciseInterFound = false;
		ISelectableObject::TSelectionType selectionType = object->getSelectionType();
		switch(selectionType)
		{
			case ISelectableObject::GroundProjected:
			{
				// if current selection remains selected, keep it
				bool keepSelection = objectSelected &&
									 objectSelected->getSelectionType() == ISelectableObject::GroundProjected &&
									 objectSelected == precSelectableObject;
				if (!keepSelection)
				{
					preciseInterFound = true;
					distZ = intersectedObjects[i].Depth;
				}
			}
			break;
			case ISelectableObject::LocalSelectBox:
			case ISelectableObject::WorldSelectBox:
				// if (!object->intersectionTest(bbox, pos, dir, dist2D, distZ, distSelection)) continue;
				distZ = object->preciseIntersectionTest(worldViewRay.Origin, worldViewRay.Dir);
				if (distZ != FLT_MAX)
				{
					preciseInterFound = true;
				}
			break;
			default:
				nlassert(0);
			break;
		}


		// *** if intersect face, then take the best face-intersection, else use box-ray cost
		// true face-col found?
		if(preciseInterFound)
		{
			// yes, get the nearest
			if(distZ<=bestDistZ)
			{
				bestDistBox= 0;
				bestDistZ= distZ;
				objectSelected= object;
				/*if (selectionType == ISelectableObject::GroundProjected)
				{
					projectedObjects.push_back(object);
				}*/
			}
		}
		// else
		else
		{
			// if a true face-intersection has not been found for others entities
			if(bestDistZ==FLT_MAX)
			{
				// get the "distance to camera" contribution.
				// NB: ray & select box are in the same space (local or world)
				CVector			c= selectObj.SelectBox.getCenter();
				float			distCamCost= intersectedObjects[i].Depth;
				// get relative to the nearest intersected entity
				distCamCost-= startDistBox;
				// take the middle of the clipped segment. suppose that this middle is the "nearest ray point to center"
				// This is false, but gives better results.
				CVector	m= (selectObj.RayStartClipped + selectObj.RayEndClipped) / 2;
				// get the distance to center. NB: small entities are preferred since smaller mean lower cost
				float			outBBoxCost= (m - c).norm();

				// the final cost is a weighted sum of the both. NB: distCamCost is in meter,
				// and outBBBoxCost is meters. Hence ClientCfg.SelectionOutBBoxWeight is a factor
				float	boxCost= distCamCost + outBBoxCost * ClientCfg.SelectionOutBBoxWeight;

				// take the lowest cost
				if(boxCost<bestDistBox)
				{
					objectSelected= object;
					bestDistBox= boxCost;
				}
			}
		}
	}

	// If precise intersection not found
	if(bestDistZ==FLT_MAX)
	{
		// if the last entity under pos is valid, prefer it among all other approximate ones
		if(precInstanceUnderPos && precInstanceUnderPosValid)
			objectSelected= precInstanceUnderPos->getDisplayerVisual();
	}

	if (objectSelected->getSelectionType() == ISelectableObject::GroundProjected)
	{
		if (borderSelected && borderSelected != objectSelected)
		{
			if (!objectSelected || !objectSelected->isInProjectionBorder(CVector2f(sceneInter.x, sceneInter.y)))
			{
				// when mouse over zone border, preffered over other zones
				objectSelected = borderSelected;
			}
		}
		// TODO nico: list not really needed here (comes from old code)
		/*uint k;
		for(k = 0; k < projectedObjects.size(); ++k)
		{
			if (projectedObjects[k] == objectSelected) break;
		}
		objectSelected = projectedObjects[(k + 1) % projectedObjects.size()];
		*/
	}


	// return the best entity
	_LastInstanceUnderPos = objectSelected ? objectSelected->getInstanceInEditor() : NULL;
	return	_LastInstanceUnderPos;
}// getEntityUnderPos //


// *********************************************************************************************************
float CEditor::preciseEntityIntersectionTest(CEntityCL &entity, const NLMISC::CVector &worldRayStart, const NLMISC::CVector &worldRayDir)
{
	//H_AUTO(R2_CEditor_preciseEntityIntersectionTest)
	CHECK_EDITOR
		// if entity skeleton model was clipped, skip
	NL3D::USkeleton	*skeleton = entity.skeleton();
	if(!ClientCfg.Light && skeleton && !skeleton->getLastClippedState())
		return FLT_MAX;

	H_AUTO(RZ_Client_GEUP_face_intersect)


	// *** Try get face-intersection, result in distZ
	// if the entity support fast and precise intersection (and if it succeeds)
//	bool	trueIntersectComputed= false;
	float dist2D, distZ;
	if(!ClientCfg.Light)
	{
		if(skeleton)
		{
			if(skeleton->supportFastIntersect() && skeleton->fastIntersect(worldRayStart, worldRayDir, dist2D, distZ, false))
			{
				if (dist2D == 0.f)
				{
					return distZ;
				}
			}
		}
		// get the intersection with the instance (bot object)
		else if(!entity.instances().empty() && !entity.instances()[0].Current.empty())
		{
			NL3D::UInstance	inst= entity.instances()[0].Current;
			if(inst.supportFastIntersect())
			{
				if (inst.fastIntersect(worldRayStart, worldRayDir, dist2D, distZ, false))
				{
					if (dist2D == 0.f)
					{
						return distZ;
					}
				}
			}
			else
			{
				// TMP
				// precise test was asked, but there's no better test than bbox, so return the dist for bbox middle
				return (entity.pos().asVector() + entity.localSelectBox().getCenter() - worldRayStart).norm();
			}
		}
	}
	return FLT_MAX;
}

// *********************************************************************************************************
const NLMISC::CAABBox &CEditor::getLocalSelectBox(CEntityCL &entity) const
{
	//H_AUTO(R2_CEditor_getLocalSelectBox)
	CHECK_EDITOR
	const TEntityCustomSelectBoxMap &boxMap = getEditor().getEntityCustomSelectBoxMap();
	TEntityCustomSelectBoxMap::const_iterator it = boxMap.find(CSheetId(entity.sheetId()).toString());
	if (it != boxMap.end())
	{
		if (it->second.Enabled)
		{
			return it->second.Box;
		}
	}
	return entity.localSelectBox();
}

// *********************************************************************************************************
NLMISC::CAABBox CEditor::getSelectBox(CEntityCL &entity) const
{
	//H_AUTO(R2_CEditor_getSelectBox)
	CHECK_EDITOR
	const TEntityCustomSelectBoxMap &boxMap = getEditor().getEntityCustomSelectBoxMap();
	TEntityCustomSelectBoxMap::const_iterator it = boxMap.find(CSheetId(entity.sheetId()).toString());
	if (it != boxMap.end())
	{
		if (it->second.Enabled)
		{
			// box is local, transform in world
			CMatrix modelMatrix;
			modelMatrix = entity.dirMatrix();
			modelMatrix.setPos(entity.pos().asVector());
			return CAABBox::transformAABBox(modelMatrix, it->second.Box);
		}
	}
	return entity.selectBox();
}

// *********************************************************************************************************
void CEditor::connexionMsg(const std::string &stringId)
{
	//H_AUTO(R2_CEditor_connexionMsg)
	CHECK_EDITOR
	getEditor()._ConnexionMsg = stringId;
	// ignore if current ui desktop is not the third
	if (getUI().getMode() != 3) return;
	// show the connection window
	CInterfaceGroup *r2ConnectWindow = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_connect"));
	if (!r2ConnectWindow) return;
	if (stringId.empty())
	{
		r2ConnectWindow->setActive(false);
		return;
	}
	else
	{
		if (!r2ConnectWindow->getActive())
		{
			// center for the first display
			r2ConnectWindow->setActive(true);
			r2ConnectWindow->center();
		}
		CViewText *vt = dynamic_cast<CViewText *>(r2ConnectWindow->getView("connexionMsg"));
		if (vt)
		{
			vt->setText(CI18N::get(stringId));
		}
	}
}


// *********************************************************************************************************
void CEditor::connect()
{
	//H_AUTO(R2_CEditor_connect)
	CHECK_EDITOR
	if (_Mode == EditionMode || _Mode ==  AnimationModeLoading)
	{
		bool ok = false;
		if (_ScenarioInstance)
		{
			try
			{

				if (_ScenarioInstance->getLuaProjection().callMethodByNameNoThrow("validateForTesting", 0, 1))
				{
					if (getLua().toBoolean(-1) == true)
					{


						R2::getEditor().getDMC().getEditionModule().requestStartScenario();

						ok = true;
					}
				}
			}
			catch (const std::exception& )
			{
				// 'ok' still == false ...
			}
		}
		if (!ok)
		{
			nlwarning("Can't go live");
		}
	}
	else if (_Mode == TestMode || _Mode == DMMode)
	{
		try
		{
			R2::getEditor().getLua().executeScript("r2.requestStopLive()");
			if (ClientCfg.Local)
			{
				R2::getEditor().setMode(CEditor::EditionMode);
				getLua().executeScriptNoThrow("r2.requestReconnection()");
			}
			else
			{
				R2::getEditor().setMode(CEditor::GoingToEditionMode);
			}
			CEditor::connexionMsg("uimR2EDGoToEditingMode");
		}
		catch (const std::exception& e)
		{
			nlwarning("Can't go live: %s", e.what());
		}
	}

	// TODO Nico : reset the good capture keyboard
	setDefaultChatWindow(PeopleInterraction.ChatGroup.Window);
	ConnectionWanted = false;
	return;
}

// *********************************************************************************************************
CEditor::TInstanceObserverHandle CEditor::addInstanceObserver(const TInstanceId &instanceId, IInstanceObserver *observer)
{
	//H_AUTO(R2_CEditor_addInstanceObserver)
	CHECK_EDITOR
	//nlwarning("#adding instance observer 0x%x", (int) observer);
	nlassert(_InstanceObservers.size() == _InstanceObserverHandles.size());
	const TInstanceObserverMap::const_iterator lb = _InstanceObservers.lower_bound(instanceId);
	const TInstanceObserverMap::const_iterator ub = _InstanceObservers.upper_bound(instanceId);
	// NB nico : removed the inserted twice stuff below because observer pointers are not used as keys,
	// so sharing is possible
	// see if not inserted twice
	/*
	for (TInstanceObserverMap::const_iterator it = lb; it != ub; ++it)
	{
		if (it->second == observer)
		{
			nlwarning("addInstanceObserver : Instance observer inserted twice");
			return BadInstanceObserverHandle;
		}
	}
	*/
	// insert the handle
	TInstanceObserverHandle handle = _InstanceObserverHandleCounter++;
	if (_InstanceObserverHandleCounter == BadInstanceObserverHandle)
	{
		++ _InstanceObserverHandleCounter; // avoid bad handle
	}
	_InstanceObserverHandles[handle] = _InstanceObservers.insert(TInstanceObserverMap::value_type(instanceId, observer));
	nlassert(_InstanceObservers.size() == _InstanceObserverHandles.size());
	return handle;
}

// *********************************************************************************************************
CEditor::IInstanceObserver *CEditor::removeInstanceObserver(TInstanceObserverHandle handle)
{
	//H_AUTO(R2_CEditor_removeInstanceObserver)
	CHECK_EDITOR
	nlassert(_InstanceObservers.size() == _InstanceObserverHandles.size());
	TInstanceObserverHandleMap::iterator it = _InstanceObserverHandles.find(handle);
	if (it == _InstanceObserverHandles.end())
	{
		nlwarning("removeInstanceObserver : Instance observer handle not found : %d", (int) handle);
		return NULL;
	}
	IInstanceObserver *observer = it->second->second;
	//nlwarning("#removing instance observer 0x%x", (int) observer);
	_InstanceObservers.erase(it->second); // remove from observer map
	_InstanceObserverHandles.erase(it);
	nlassert(_InstanceObservers.size() == _InstanceObserverHandles.size());
	return observer;
}

// *********************************************************************************************************
CEditor::IInstanceObserver *CEditor::getInstanceObserver(TInstanceObserverHandle handle)
{
	//H_AUTO(R2_CEditor_getInstanceObserver)
	CHECK_EDITOR
	nlassert(_InstanceObservers.size() == _InstanceObserverHandles.size());
	TInstanceObserverHandleMap::iterator it = _InstanceObserverHandles.find(handle);
	if (it == _InstanceObserverHandles.end())
	{
		return NULL;
	}
	return it->second->second;
}

// *********************************************************************************************************
CEditor::TSeason CEditor::getSeason() const
{
	//H_AUTO(R2_CEditor_getSeason)
	if (_IsWaitingTPForSeasonChange) return UnknownSeason; // as long at the teleport message hasn't been received, don't change
														   // the season for nothing -> pretend that we don't know the season so that it remains unchnged
	return _Season;
}

// *********************************************************************************************************
void CEditor::tpReceived()
{
	//H_AUTO(R2_CEditor_tpReceived)
	_IsWaitingTPForSeasonChange = false; // season can be changed now
	_TPReceivedFlag = true;
}

// *********************************************************************************************************
void CEditor::checkMissingCollisions()
{
	//H_AUTO(R2_CEditor_checkMissingCollisions)
	CScenarioEntryPoints &sep = CScenarioEntryPoints::getInstance();
	sep.loadCompleteIslands();
	const CScenarioEntryPoints::TCompleteIslands &islands = sep.getCompleteIslands();
	for(uint k = 0; k < islands.size(); ++k)
	{
		bool found = !(CPath::lookup(islands[k].Island + ".packed_island", false, false).empty());
		if (!found)
		{
			nlwarning("ISLAND COLLISION MISSING FOR : %s", islands[k].Island.c_str());
		}
		found = !(CPath::lookup(islands[k].Island + ".island_hm", false, false).empty());
		if (!found)
		{
			nlwarning("ISLAND HEIGHTMAP MISSING FOR : %s", islands[k].Island.c_str());
		}
	}
}

// *********************************************************************************************************
// move
class CAHEdContextMenu : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		getEditor().displayContextMenu();
	}
};
REGISTER_ACTION_HANDLER(CAHEdContextMenu, "r2ed_context_menu");


// *********************************************************************************************************
NLMISC::CVector getVector(const CObject *obj)
{
	return getVectorD(obj).asVector();
}

// *********************************************************************************************************
NLMISC::CVectorD getVectorD(const CObject *obj)
{
	return CVectorD(getNumber(obj, "x"), getNumber(obj, "y"), getNumber(obj, "z"));
}


// *********************************************************************************************************
CObject *buildVector(const NLMISC::CVectorD &vector, const std::string &instanceId /*= ""*/)
{
	CObject *table;
	if (instanceId.empty() )
	{
		table = getEditor().getDMC().newComponent("Position");
		table->set("x", vector.x);
		table->set("y", vector.y);
		table->set("z", vector.z);
	}
	else
	{
		table = new CObjectTableClient;
		table->insert("InstanceId", new CObjectString(instanceId));
		table->insert("Class", new CObjectString("Position"));
		table->insert("x", new CObjectNumber(vector.x));
		table->insert("y", new CObjectNumber(vector.y));
		table->insert("z", new CObjectNumber(vector.z));
	}
	return table;
}

// *********************************************************************************************************
const CObject *getObject(const CObject *obj,const std::string &attrName)
{
	if (!obj) return NULL;
	return getEditor().getDMC().getPropertyAccessor().getPropertyValue(obj, attrName);
}

// *********************************************************************************************************
std::string getString(const CObject *obj, const std::string &attrName)
{
	obj = getObject(obj, attrName);
	if (!obj) return "";
	return obj->isString() ? obj->toString() : "";
}

// *********************************************************************************************************
double getNumber(const CObject *obj, const std::string &attrName)
{
	obj = getObject(obj, attrName);
	if (!obj) return 0;
	return obj->isNumber() ? obj->toNumber() : 0;
}


bool isEditionCurrent()
{
	CEditor &ed = getEditor();
	return ClientCfg.R2EDEnabled && ed.getMode() == CEditor::EditionMode;
}


// *********************************************************************************************************
void CEditor::onContinentChanged()
{
	//H_AUTO(R2_CEditor_onContinentChanged)
	if (_Mode != EditionMode) return;
	// refresh all collisions
	if (_ScenarioInstance)
	{
		struct CContinentChangedVisitor : public IInstanceVisitor
		{
			virtual void visit(CInstance &inst)
			{
				inst.onContinentChanged();
			}
		};
		CContinentChangedVisitor continentChangedVisitor;
		_ScenarioInstance->visit(continentChangedVisitor);
	}
}

//-----------------------------
bool CEditor::getVisualPropertiesFromObject(CObject* object, SPropVisualA& vA, SPropVisualB& vB, SPropVisualC& vC)
{
	//H_AUTO(R2_CEditor_getVisualPropertiesFromObject)

	std::string sheetClient = getString(object, "SheetClient");

	const CEntitySheet *entitySheet = SheetMngr.get(CSheetId(sheetClient));
	if (!entitySheet)
	{
		nlwarning("Can't find client sheet %s", sheetClient.c_str());
		return false;
	}

	CSheetId sheetId(sheetClient);
	if (sheetId == CSheetId::Unknown)
	{
		nlwarning("Can't get sheet");
		return false;
	}


	//-------------------------random init npc visual properties

	std::map< std::string, double > visualProps;


	static const char* keys[] = { "GabaritHeight", "GabaritTorsoWidth", "GabaritArmsWidth", "GabaritLegsWidth", "GabaritBreastSize"
		, "HairType", "HairColor", "Tattoo", "EyesColor"
		, "MorphTarget1", "MorphTarget2", "MorphTarget3", "MorphTarget4"
		, "MorphTarget5", "MorphTarget6", "MorphTarget7", "MorphTarget8"
		, "JacketModel", "TrouserModel", "FeetModel", "HandsModel"
		, "ArmModel", "WeaponRightHand", "WeaponLeftHand"
		, "JacketColor", "ArmColor", "HandsColor"
		, "TrouserColor", "FeetColor"};

	unsigned int first = 0;
	unsigned int last = sizeof(keys) / sizeof(keys[0]);
	for (; first != last; ++first)
	{
		visualProps[keys[first]] = getNumber(object, keys[first]);
	}

	//vA.PropertySubData.Sex = (uint) visualProps["Sex"];

	if(entitySheet)
	{
		 const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
		 if(chSheet)
		 {
			vA.PropertySubData.Sex = (chSheet->Gender == GSGENDER::female);
		 }
	}

	vC.PropertySubData.CharacterHeight	= (uint) visualProps["GabaritHeight"];
	vC.PropertySubData.ArmsWidth		= (uint) visualProps["GabaritArmsWidth"];
	vC.PropertySubData.TorsoWidth		= (uint) visualProps["GabaritTorsoWidth"];
	vC.PropertySubData.LegsWidth		= (uint) visualProps["GabaritLegsWidth"];
	vC.PropertySubData.BreastSize		= (uint) visualProps["GabaritBreastSize"];

	int itemNb = (int) visualProps["HairType"];
	std::string itemFileName;
	if(itemNb>0)
	{
		itemFileName = CSheetId(itemNb).toString();
		vA.PropertySubData.HatModel		= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::HEAD_SLOT);
	}
	else
	{
		vA.PropertySubData.HatModel = 0;
	}

	vA.PropertySubData.HatColor		= (uint) visualProps["HairColor"];
	vC.PropertySubData.Tattoo		= (uint) visualProps["Tattoo"];
	vC.PropertySubData.EyesColor	= (uint) visualProps["EyesColor"];

	vC.PropertySubData.MorphTarget1 = (uint) visualProps["MorphTarget1"];
	vC.PropertySubData.MorphTarget2 = (uint) visualProps["MorphTarget2"];
	vC.PropertySubData.MorphTarget3 = (uint) visualProps["MorphTarget3"];
	vC.PropertySubData.MorphTarget4 = (uint) visualProps["MorphTarget4"];
	vC.PropertySubData.MorphTarget5 = (uint) visualProps["MorphTarget5"];
	vC.PropertySubData.MorphTarget6 = (uint) visualProps["MorphTarget6"];
	vC.PropertySubData.MorphTarget7 = (uint) visualProps["MorphTarget7"];
	vC.PropertySubData.MorphTarget8 = (uint) visualProps["MorphTarget8"];

	itemNb = (int) visualProps["JacketModel"];
	if(itemNb>0)
	{
		itemFileName = CSheetId(itemNb).toString();
		vA.PropertySubData.JacketModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::CHEST_SLOT);
	}
	else
	{
		vA.PropertySubData.JacketModel = 0;
	}

	itemNb = (int) visualProps["TrouserModel"];
	if(itemNb>0)
	{
		itemFileName = CSheetId(itemNb).toString();
		vA.PropertySubData.TrouserModel	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::LEGS_SLOT);
	}
	else
	{
		vA.PropertySubData.TrouserModel = 0;
	}

	itemNb = (int) visualProps["FeetModel"];
	if(itemNb>0)
	{
		itemFileName = CSheetId(itemNb).toString();
		vB.PropertySubData.FeetModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::FEET_SLOT);
	}
	else
	{
		vB.PropertySubData.FeetModel = 0;
	}

	itemNb = (int) visualProps["HandsModel"];
	if(itemNb>0)
	{
		itemFileName = CSheetId(itemNb).toString();
		vB.PropertySubData.HandsModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::HANDS_SLOT);
	}
	else
	{
		vB.PropertySubData.HandsModel = 0;
	}

	itemNb = (int) visualProps["ArmModel"];
	if(itemNb>0)
	{
		itemFileName = CSheetId(itemNb).toString();
		vA.PropertySubData.ArmModel	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::ARMS_SLOT);
	}
	else
	{
		vA.PropertySubData.ArmModel = 0;
	}

	vA.PropertySubData.JacketColor		= (uint) visualProps["JacketColor"];
	vA.PropertySubData.TrouserColor		= (uint) visualProps["TrouserColor"];
	vB.PropertySubData.FeetColor		= (uint) visualProps["FeetColor"];
	vB.PropertySubData.HandsColor		= (uint) visualProps["HandsColor"];
	vA.PropertySubData.ArmColor			= (uint) visualProps["ArmColor"];

	itemNb = (int) visualProps["WeaponRightHand"];
	if(itemNb>0)
	{
		itemFileName = CSheetId(itemNb).toString();
		vA.PropertySubData.WeaponRightHand	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::RIGHT_HAND_SLOT);
	}
	else
	{
		vA.PropertySubData.WeaponRightHand = 0;
	}

	itemNb = (int) visualProps["WeaponLeftHand"];
	if(itemNb>0)
	{
		itemFileName = CSheetId(itemNb).toString();
		vA.PropertySubData.WeaponLeftHand	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::LEFT_HAND_SLOT);
	}
	else
	{
		vA.PropertySubData.WeaponLeftHand = 0;
	}
	return true;


}


// *********************************************************************************************************
bool CEditor::CSortedInstances::contains(CInstance *inst) const
{
	TInstanceToIter::const_iterator it = _InstanceToIter.find(inst);
	#ifdef NL_DEBUG
	if (it == _InstanceToIter.end())
	{
		for(TInstanceByName::const_iterator it = _ByName.begin(); it != _ByName.end(); ++it)
		{
			nlassert(it->second != inst); // should be empty
		}
	}
	#endif
	return it != _InstanceToIter.end();
}

// *********************************************************************************************************
void CEditor::CSortedInstances::insert(const ucstring &name, CInstance *inst)
{
	nlassert(inst);
	#ifdef NL_DEBUG
		static volatile bool doTest = true;
		if (doTest)
		{
			nlassert(!contains(inst)); // inserted twice !!
		}
	#endif
	_InstanceToIter[inst] = _ByName.insert(std::make_pair(name, inst));
}

// *********************************************************************************************************
void CEditor::CSortedInstances::remove(CInstance *inst)
{
	nlassert(inst);
	TInstanceToIter::iterator it = _InstanceToIter.find(inst);
	nlassert(it != _InstanceToIter.end());
	_ByName.erase(it->second);
	_InstanceToIter.erase(it);
	#ifdef NL_DEBUG
		nlassert(!contains(inst));
	#endif
}


// *********************************************************************************************************
bool CEditor::isRegisteredByDispName(CInstance *inst) const
{
	nlassert(inst);
	for (uint k = 0; k < _InstancesByDispName.size(); ++k)
	{
		if (_InstancesByDispName[k].contains(inst)) return true;	// registered twice !!
	}
	return false;
}

// *********************************************************************************************************
void CEditor::registerInstanceDispName(const ucstring &displayName, CInstance *inst)
{
	nlassert(inst);
	sint currClass = inst->getClassIndex();
	if (currClass < 0)
	{
		nlwarning("Classindex not found for class %s", inst->getClassName().c_str());
		return;
	}
	#ifdef NL_DEBUG
		nlassert(!isRegisteredByDispName(inst));
	#endif
	// for each class & subclass of the object, insert in the matching list
	while (currClass >= 0)
	{
		nlassert(currClass < (sint) _InstancesByDispName.size());
		_InstancesByDispName[currClass].insert(displayName, inst);
		currClass = getBaseClass(currClass);
	}
	//
	#ifdef NL_DEBUG
		nlassert(isRegisteredByDispName(inst));
	#endif
}

// *********************************************************************************************************
void CEditor::unregisterInstanceDispName(CInstance *inst)
{
	nlassert(inst);
	sint currClass = inst->getClassIndex();
	if (currClass < 0)
	{
		nlwarning("Classindex not found for class %s", inst->getClassName().c_str());
		return;
	}
	#ifdef NL_DEBUG
		nlassert(isRegisteredByDispName(inst));
	#endif
	while (currClass >= 0)
	{
		nlassert(currClass < (sint) _InstancesByDispName.size());
		_InstancesByDispName[currClass].remove(inst);
		currClass = getBaseClass(currClass);
	}
	#ifdef NL_DEBUG
		nlassert(!isRegisteredByDispName(inst));
	#endif
}


// *********************************************************************************************************
//
// Creation of a new entity in scene
//
class CAHCreateEntity : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CHECK_EDITOR
		if (getEditor().getMode() != CEditor::EditionMode)
		{
			nlwarning("Can't modify scenario while testing");
			return;
		}

		CInterfaceManager *im = CInterfaceManager::getInstance();
		// Retrieve sheet for entity that is to be created.
		std::string paletteId = getParam(sParams, "PaletteId");
		CObject *paletteNode = getEditor().getDMC().getPaletteElement(paletteId);
		if (!paletteNode)
		{
			nlwarning("Can't retrieve palette node for id %s", paletteId.c_str());
			return;
		}
		if (!paletteNode->isTable())
		{
			nlwarning("Bad type for palette node %s (should be a table)", paletteId.c_str());
			return;
		}

		std::string sheetClient = getString(paletteNode, "SheetClient");

		const CEntitySheet *entitySheet = SheetMngr.get(CSheetId(sheetClient));
		if (!entitySheet)
		{
			nlwarning("Can't find client sheet %s", sheetClient.c_str());
			return;
		}
		const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
		if(chSheet->R2Npc)
		{
			getEditor().getLua().push(sheetClient);
			if (getEditor().getEnv().callMethodByNameNoThrow("randomNPCSex", 1, 1))
			{
				CLuaObject result(getEditor().getLua());
				sheetClient = result.toString();
			}
		}
		CSheetId sheetId(sheetClient);
		if (sheetId == CSheetId::Unknown)
		{
			nlwarning("Can't get sheet");
			return;
		}
		getEditor().setCurrentTool(NULL); // remove current to avoid to have ghost removed by that tool if it was a "CToolCreateEntity" too.
		uint ghostSlot = 1; // TMP TMP
		CEntityCL * entity = NULL;
		if (!(entity=CEditor::createEntity(ghostSlot, sheetId, CVector::Null, 0.f)))
		{
			return;
		}

		//-------------------------random init npc visual properties
		if(dynamic_cast<CPlayerR2CL *>(entity))
		{
			// push equipment id
			getEditor().getLua().push(getString(paletteNode, "Equipment"));

			// push race
			std::string race;
			switch(entity->people())
			{
			case EGSPD::CPeople::Fyros:
				race = "Fyros";
				break;

			case EGSPD::CPeople::Matis:
				race = "Matis";
				break;

			case EGSPD::CPeople::Tryker:
				race = "Tryker";
				break;

			case EGSPD::CPeople::Zorai:
				race = "Zorai";
				break;

			default:
				nlwarning("CAHCreateEntity::execute unknown people");
			}
			getEditor().getLua().push(race);

			if (getEditor().getEnv().callMethodByNameNoThrow("randomNPCProperties", 2, 1))
			{
				CLuaObject result(getEditor().getLua());
				std::map< std::string, double > visualProps;
				ENUM_LUA_TABLE(result, it)
				{
					visualProps[it.nextKey().toString()] = it.nextValue().toNumber();
				}

				// visual property A depends on the type of the entity
				SPropVisualA vA;
				SPropVisualB vB;
				SPropVisualC vC;
				sint64     *prop = 0;

				//vA.PropertySubData.Sex = (uint) visualProps["Sex"];
				const CEntitySheet *entitySheet = SheetMngr.get((CSheetId)sheetId.asInt());
				if(entitySheet)
				{
					 const CCharacterSheet *chSheet = dynamic_cast<const CCharacterSheet *>(entitySheet);
					 if(chSheet)
					 {
						vA.PropertySubData.Sex = (chSheet->Gender == GSGENDER::female);
					 }
				}

				vC.PropertySubData.CharacterHeight	= (uint) visualProps["GabaritHeight"];
				vC.PropertySubData.ArmsWidth		= (uint) visualProps["GabaritArmsWidth"];
				vC.PropertySubData.TorsoWidth		= (uint) visualProps["GabaritTorsoWidth"];
				vC.PropertySubData.LegsWidth		= (uint) visualProps["GabaritLegsWidth"];
				vC.PropertySubData.BreastSize		= (uint) visualProps["GabaritBreastSize"];

				int itemNb = (int) visualProps["HairType"];
				std::string itemFileName;
				if(itemNb>0)
				{
					itemFileName = CSheetId(itemNb).toString();
					vA.PropertySubData.HatModel		= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::HEAD_SLOT);
				}
				else
				{
					vA.PropertySubData.HatModel = 0;
				}

				vA.PropertySubData.HatColor		= (uint) visualProps["HairColor"];
				vC.PropertySubData.Tattoo		= (uint) visualProps["Tattoo"];
				vC.PropertySubData.EyesColor	= (uint) visualProps["EyesColor"];

				vC.PropertySubData.MorphTarget1 = (uint) visualProps["MorphTarget1"];
				vC.PropertySubData.MorphTarget2 = (uint) visualProps["MorphTarget2"];
				vC.PropertySubData.MorphTarget3 = (uint) visualProps["MorphTarget3"];
				vC.PropertySubData.MorphTarget4 = (uint) visualProps["MorphTarget4"];
				vC.PropertySubData.MorphTarget5 = (uint) visualProps["MorphTarget5"];
				vC.PropertySubData.MorphTarget6 = (uint) visualProps["MorphTarget6"];
				vC.PropertySubData.MorphTarget7 = (uint) visualProps["MorphTarget7"];
				vC.PropertySubData.MorphTarget8 = (uint) visualProps["MorphTarget8"];

				itemNb = (int) visualProps["JacketModel"];
				if(itemNb>0)
				{
					itemFileName = CSheetId(itemNb).toString();
					vA.PropertySubData.JacketModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::CHEST_SLOT);
				}
				else
				{
					vA.PropertySubData.JacketModel = 0;
				}

				itemNb = (int) visualProps["TrouserModel"];
				if(itemNb>0)
				{
					itemFileName = CSheetId(itemNb).toString();
					vA.PropertySubData.TrouserModel	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::LEGS_SLOT);
				}
				else
				{
					vA.PropertySubData.TrouserModel = 0;
				}

				itemNb = (int) visualProps["FeetModel"];
				if(itemNb>0)
				{
					itemFileName = CSheetId(itemNb).toString();
					vB.PropertySubData.FeetModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::FEET_SLOT);
				}
				else
				{
					vB.PropertySubData.FeetModel = 0;
				}

				itemNb = (int) visualProps["HandsModel"];
				if(itemNb>0)
				{
					itemFileName = CSheetId(itemNb).toString();
					vB.PropertySubData.HandsModel = (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::HANDS_SLOT);
				}
				else
				{
					vB.PropertySubData.HandsModel = 0;
				}

				itemNb = (int) visualProps["ArmModel"];
				if(itemNb>0)
				{
					itemFileName = CSheetId(itemNb).toString();
					vA.PropertySubData.ArmModel	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::ARMS_SLOT);
				}
				else
				{
					vA.PropertySubData.ArmModel = 0;
				}

				vA.PropertySubData.JacketColor		= (uint) visualProps["JacketColor"];
				vA.PropertySubData.TrouserColor		= (uint) visualProps["TrouserColor"];
				vB.PropertySubData.FeetColor		= (uint) visualProps["FeetColor"];
				vB.PropertySubData.HandsColor		= (uint) visualProps["HandsColor"];
				vA.PropertySubData.ArmColor			= (uint) visualProps["ArmColor"];

				itemNb = (int) visualProps["WeaponRightHand"];
				if(itemNb>0)
				{
					itemFileName = CSheetId(itemNb).toString();
					vA.PropertySubData.WeaponRightHand	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::RIGHT_HAND_SLOT);
				}
				else
				{
					vA.PropertySubData.WeaponRightHand = 0;
				}

				itemNb = (int) visualProps["WeaponLeftHand"];
				if(itemNb>0)
				{
					itemFileName = CSheetId(itemNb).toString();
					vA.PropertySubData.WeaponLeftHand	= (uint) SheetMngr.getVSIndex(itemFileName, SLOTTYPE::LEFT_HAND_SLOT);
				}
				else
				{
					vA.PropertySubData.WeaponLeftHand = 0;
				}

				prop = (sint64 *)&vA;
				NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", entity->slot())+":P"+toString("%d", CLFECOMMON::PROPERTY_VPA))->setValue64(*prop);

				prop = (sint64 *)&vB;
				NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", entity->slot())+":P"+toString("%d", CLFECOMMON::PROPERTY_VPB))->setValue64(*prop);

				prop = (sint64 *)&vC;
				NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString("%d", entity->slot())+":P"+toString("%d", CLFECOMMON::PROPERTY_VPC))->setValue64(*prop);

				EntitiesMngr.updateVisualProperty(0, entity->slot(), CLFECOMMON::PROPERTY_VPA);
				EntitiesMngr.updateVisualProperty(0, entity->slot(), CLFECOMMON::PROPERTY_VPB);
				EntitiesMngr.updateVisualProperty(0, entity->slot(), CLFECOMMON::PROPERTY_VPC);
			}
		}

		getEditor().setCurrentTool(new CToolCreateEntity(ghostSlot, paletteId, NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:R2_DRAW_ARRAY")->getValueBool()));
	}
};
REGISTER_ACTION_HANDLER(CAHCreateEntity, "r2ed_create_entity");


// *********************************************************************************************************
sint CEditor::classToIndex(const std::string &className) const
{
	CHashMap<std::string, uint>::const_iterator it = _ClassNameToIndex.find(className);
	if (it == _ClassNameToIndex.end()) return -1;
	return it->second;
}

// *********************************************************************************************************
bool CEditor::isKindOf(sint testedClassIndex, sint kindClassIndex) const
{
	if (testedClassIndex < 0 || kindClassIndex < 0) return false;
	return _KindOfTable(testedClassIndex, kindClassIndex) != 0;
}

// *********************************************************************************************************
void CEditor::initClassInheritanceTable()
{
	std::vector<std::string> classes;
	std::map<std::string, std::string> baseClasses;

	ENUM_LUA_TABLE(getClasses(), it)
	{
		CLuaObject name = it.nextValue()["Name"];
		CLuaObject base = it.nextValue()["BaseClass"];
		std::string baseName;
		if (base.isString())
		{
			baseName = base.toString();
		}
		if (name.isString())
		{
			_ClassNameToIndex[name.toString()] = (uint)classes.size();
			classes.push_back(name.toString());
			baseClasses[name.toString()] = baseName;
		}
	}

	_KindOfTable.init((uint)classes.size(), (uint)classes.size(), 0);
	_BaseClassIndices.resize(classes.size());
	for (uint k = 0; k < classes.size(); ++k)
	{
		//nlwarning("Class %d = %s", (int) k, classes[k].c_str());
		_BaseClassIndices[k] = classToIndex(baseClasses[classes[k]]);
		for (uint l = 0; l < classes.size(); ++l)
		{
			std::string currClass = classes[k];
			while(!currClass.empty())
			{
				if (currClass == classes[l])
				{
					_KindOfTable(k, l) = 1;
					break;
				}
				currClass = baseClasses[currClass];
			}
		}
	}
	_InstancesByDispName.clear();
	_InstancesByDispName.resize(classes.size());
}


// *********************************************************************************************************
sint CEditor::getBaseClass(sint derivedClass) const
{
	if (derivedClass < 0 || derivedClass >= (sint) _BaseClassIndices.size()) return -1;
	return _BaseClassIndices[derivedClass];
}


// *********************************************************************************************************
void CEditor::setMaxVisibleEntityExceededFlag(bool on)
{
	if (on == _MaxVisibleEntityExceededFlag) return;
	_MaxVisibleEntityExceededFlag = on;
	// lua ui update
	CLuaStackChecker lsc(&getLua(), 0);
	getLua().push(on);
	callEnvMethod("setMaxVisibleEntityExceededFlag", 1, 0);
}

// *********************************************************************************************************
class CAHGoTest : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CHECK_EDITOR
		ConnectionWanted = true;
	}
};
REGISTER_ACTION_HANDLER(CAHGoTest, "r2ed_go_test");

// *********************************************************************************************************
class CAHStopTest : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CHECK_EDITOR
		if (getEditor().getAccessMode() != CEditor::AccessEditor) return;
		ConnectionWanted = true;
	}
};
REGISTER_ACTION_HANDLER(CAHStopTest, "r2ed_stop_test");

// *********************************************************************************************************
class CAHOpenScenarioControl : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CHECK_EDITOR

		bool openUI = true;
		bool showHide = (getParam(sParams, "showHide")=="1");

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup* wnd = NULL;

		if(!R2::getEditor().isInitialized())
			wnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:ring_scenario_loading_window"));
		else
			wnd = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_scenario_control"));

		if(wnd)
		{
			if(showHide)
				openUI = !wnd->getActive();

			wnd->setActive(openUI);
		}
		//}
	}
};
REGISTER_ACTION_HANDLER(CAHOpenScenarioControl, "open_scenario_control");

// *********************************************************************************************************
class CAHR2StopLive : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CHECK_EDITOR

		bool confirmStopLive = getParam(sParams, "confirmStopLive")=="1";

		if(!confirmStopLive)
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiR2EDconfirmStopLive"), "r2_stop_live", "confirmStopLive=1", "", "", "ui:interface");
		}
		else
		{
			CSessionBrowserImpl	&sb = CSessionBrowserImpl::getInstance();
			sb.closeSession(sb.getCharId(), R2::getEditor().getDMC().getEditionModule().getCurrentAdventureId());

			if(sb.waitOneMessage(sb.getMessageName("on_invokeResult")))
			{
				if(R2::getEditor().getDMC().getEditionModule().getEditSessionLink()!=0)
				{
					// Now we expect to receive an impulsion FAR_TP to mainland, triggered
					// by the DSS. We'll disobey it by FarTPing to the Edition session instead!
					nldebug( "Will return to editing session %u", R2::getEditor().getDMC().getEditionModule().getEditSessionLink().asInt() );
					FarTP.hookNextFarTPForEditor();
				}
				// otherwise, let accomplish the FAR_TP to mainland (returnToPreviousSession)
				// triggered by the DSS.
			}
			else
			{
				nlwarning("closeSession callback return false");
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CAHR2StopLive, "r2_stop_live");

// *********************************************************************************************************
class CAHInviteCharacter : public IActionHandler
{
	virtual void execute(CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CHECK_EDITOR

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler("leave_modal", pCaller, "");

		if(pCaller)
		{
			CInterfaceGroup *fatherGC = pCaller->getParent();
			if (fatherGC)
			{
				// Look for the root parent
				for(;;)
				{
					CInterfaceGroup *parent = fatherGC->getParent();
					if (!parent || (parent->getId()=="ui:interface"))
						break;
					fatherGC = parent;
				}

				// Get the modal edit box
				CGroupEditBox *geb = dynamic_cast<CGroupEditBox *>(fatherGC->getGroup("add_contact_eb:eb"));
				if (geb && !geb->getInputString().empty())
				{
					string charName = geb->getInputString().toString();
					CSessionBrowserImpl & sessionBrowser = CSessionBrowserImpl::getInstance();
					sessionBrowser.inviteCharacterByName(sessionBrowser.getCharId(), charName);

					if(!sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_invokeResult")))
					{
						nlwarning("inviteCharacterByName callback return false");
					}

					if(sessionBrowser._LastInvokeResult == 14)
					{
						CViewText* pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:warning_free_trial:text"));
						if (pVT != NULL)
							pVT->setText(CI18N::get("uiRingWarningInviteFreeTrial"));

						CAHManager::getInstance()->runActionHandler("enter_modal", pCaller, "group=ui:interface:warning_free_trial");
					}
					else if(sessionBrowser._LastInvokeResult == 12)
					{
						CAHManager::getInstance()->runActionHandler("enter_modal", pCaller, "group=ui:interface:warning_newcomer");
					}

					geb->setInputString(ucstring(""));
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CAHInviteCharacter, "r2ed_invite_character");

// *********************************************************************************************************
class CAHTryGoTest : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CHECK_EDITOR
		getEditor().callEnvMethod("tryGoTest", 0, 0);
	}
};
REGISTER_ACTION_HANDLER(CAHTryGoTest, "r2ed_try_go_test");

// *********************************************************************************************************
class CAHCancelTool : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if (!ClientCfg.R2EDEnabled || !getEditor().isInitialized()) return;
		CHECK_EDITOR
		getEditor().setCurrentTool(NULL);
	}
};
REGISTER_ACTION_HANDLER(CAHCancelTool, "r2ed_cancel_tool");


//////////////////////////////////////////
// SWITCH DM / TESTER AT ANIMATION TIME //
//////////////////////////////////////////

// *********************************************************************************************************
class CAHAnimTestMode : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CHECK_EDITOR
		if (getEditor().getAccessMode() == CEditor::AccessDM) return;
		getEditor().setMode(CEditor::TestMode);
	}
};
REGISTER_ACTION_HANDLER(CAHAnimTestMode, "r2ed_anim_test_mode");

// *********************************************************************************************************
class CAHAnimDMMode : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CHECK_EDITOR
		getEditor().setMode(CEditor::DMMode);
	}
};
REGISTER_ACTION_HANDLER(CAHAnimDMMode, "r2ed_anim_dm_mode");


// *********************************************************************************************************
class CAHR2ContextCommand : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CHECK_EDITOR
		// forward the call to lua
		// first param is the id of the command
		std::string commandId = getParam(sParams, "commandId");
		// if there's a current active tool, then see if it wants to handle it first
		if (commandId == "delete")
		{
			if (getEditor().getCurrentTool() && getEditor().getCurrentTool()->onDeleteCmd())
				return;
		}
		getEditor().getLua().push(commandId);
		getEditor().callEnvMethod("execContextCommand", 1, 0);
	}
};
REGISTER_ACTION_HANDLER(CAHR2ContextCommand, "r2ed_context_command");


// *********************************************************************************************************
class CAHR2Teleport : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if (!ClientCfg.R2EDEnabled) return;
		if (R2::getEditor().getMode() != CEditor::EditionMode &&
			R2::getEditor().getMode() != CEditor::DMMode &&
			R2::getEditor().getMode() != CEditor::AnimationModeDm) return;

		CHECK_EDITOR
		// just forward to lua
		getEditor().callEnvMethod("activeTeleportTool", 0, 0);
	}
};
REGISTER_ACTION_HANDLER(CAHR2Teleport, "r2ed_teleport");

// *********************************************************************************************************
class CAHR2Undo : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// if an edit box currently has focus, then try undo on it first
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>( CWidgetManager::getInstance()->getCaptureKeyboard());
		if (eb && eb->undo())
		{
			return;
		}
		CActionHistoric &historic = getEditor().getDMC().getActionHistoric();
		if (historic.canUndo())
		{
			getEditor().setCurrentTool(NULL);
			const ucstring *actionName = historic.getPreviousActionName();
			nlassert(actionName);
			CLuaIHM::push(getEditor().getLua(), *actionName);
			historic.undo();
			getEditor().callEnvMethod("onUndo", 1, 0);
		}
		else
		{
			getEditor().callEnvMethod("onCantUndo", 0, 0);
		}
	}
};
REGISTER_ACTION_HANDLER(CAHR2Undo, "r2ed_undo");

// *********************************************************************************************************
class CAHR2Redo : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// if an edit box currently has focus, then try redo on it first
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(CWidgetManager::getInstance()->getCaptureKeyboard());
		if (eb && eb->redo())
		{
			return;
		}
		CActionHistoric &historic = getEditor().getDMC().getActionHistoric();
		if (historic.canRedo())
		{
			getEditor().setCurrentTool(NULL);
			const ucstring *actionName = historic.getNextActionName();
			nlassert(actionName);
			CLuaIHM::push(getEditor().getLua(), *actionName);
			historic.redo();
			getEditor().callEnvMethod("onRedo", 1, 0);
		}
		else
		{
			getEditor().callEnvMethod("onCantRedo", 0, 0);
		}
	}
};
REGISTER_ACTION_HANDLER(CAHR2Redo, "r2ed_redo");

// *********************************************************************************************************
// signal the server that a dm gift has begun, so that the server can take note of the target entity (the current target)
class CAHR2DMGiftBegin : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("DM_GIFT:BEGIN", out))
		{
			NetMngr.push(out);
		}
		else
			nlwarning("<CHandlerInvalidateExchange::execute> unknown message name 'DM_GIFT:BEGIN");
	}
};
REGISTER_ACTION_HANDLER(CAHR2DMGiftBegin, "r2ed_dm_gift_begin");

// *********************************************************************************************************
// signal the server that a dm gift has begun, so that the server can take note of the target entity (the current target)
class CAHR2DMGiftValidate : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("DM_GIFT:VALIDATE", out))
		{
			for(uint k = 0; k < 8; ++k)
			{
				uint32 sheetId = 0;
				uint8  quantity = 0;
				CCDBNodeLeaf *sheetLeaf = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:R2:DM_GIFT:%d:SHEET", (int) k));
				if (sheetLeaf) sheetId = (uint32) sheetLeaf->getValue32();
				CCDBNodeLeaf *quantityLeaf = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:R2:DM_GIFT:%d:QUANTITY", (int) k));
				if (quantityLeaf) quantity = (uint8) quantityLeaf->getValue8();
				out.serial(sheetId);
				out.serial(quantity);
			}
			NetMngr.push(out);
		}
		else
			nlwarning("<CHandlerInvalidateExchange::execute> unknown message name 'DM_GIFT:VALIDATE");
	}
};
REGISTER_ACTION_HANDLER(CAHR2DMGiftValidate, "r2ed_dm_gift_validate");


// *********************************************************************************************************
// freeze / unfreeze bot objects
class CAHR2FreezeUnfreezeBotObjects : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CLuaManager::getInstance().executeLuaScript("r2:freezeUnfreezeBotObjects()");
	}
};
REGISTER_ACTION_HANDLER(CAHR2FreezeUnfreezeBotObjects, "r2ed_freeze_unfreeze_botobjects");




} // R2



// *********************************************************************************************************
// COMMAND TO REINITIALIZE THE EDITOR : useful to check scripts error
NLMISC_COMMAND(resetEditor, "reset R2 editor, reload all scripts (except ui xmls)", "")
{
	if (!args.empty()) return false;
	R2::ResetWanted = true;
	R2::ReloadUIFlag = false;
	return true;
}

// *********************************************************************************************************
// COMMAND TO REINITIALIZE THE EDITOR : useful to check scripts error
NLMISC_COMMAND(resetEditorAndReloadUI, "reset R2 editor, reload all scripts & xml (possibly preserving scenario)", "")
{
	if (!args.empty()) return false;
	R2::ResetWanted = true;
	R2::ReloadUIFlag = true;
	return true;
}
// *********************************************************************************************************
// write scenario rtdata to a file for debugging or stress testing by simulator_service
NLMISC_COMMAND(saveScenarioRtData, "save scenario RtData to file", "<filename>")
{
	const R2::CObject *scenario = R2::getEditor().getDMC().getHighLevel();
	if( !scenario )
	{
		nlinfo("No current scenario -- can't save rtdata");
		return false;
	}

	std::string fileName = "outpout";
	if( args.empty() )
	{
		// try to name file from title, then area+instance
		R2::CObject *description = scenario->findAttr("Description");
		if( description )
		{
			R2::CObject *name = description->findAttr("Name");
			R2::CObject *title = 0;
			if (name)
			{
				title = name;
			}
			else
			{
				title =  description->findAttr("Title"); //obsolete
			}
			R2::CObject *locationId = description->findAttr("LocationId");
			R2::CObject *instanceId = description->findAttr("InstanceId");
			if( title )
			{
				string sTitle = title->toString();
				if( !sTitle.empty() )
					fileName = sTitle;
			}
			else if( locationId && instanceId )
				fileName = "area" + locationId->toString() + "_" + instanceId->toString();
		}
	}
	else	// filename given
	{
		fileName = args[0];
	}

	nlinfo("translating current scenario");
	R2::CObject *pHighLevel = scenario->clone();
	nlassert( pHighLevel );

	R2::CObject *pRtData = R2::getEditor().getDMC().translateScenario( pHighLevel );
	delete pHighLevel;
	if( !pRtData )
	{
		nlwarning("Failed to translate high-level scenario into rtdata");
		return false;
	}

	// binary
	string fullFileName = fileName;
	fullFileName += ".rt.bin";
	nlinfo("writing rtdata to %s", fullFileName.c_str());

	COFile output;
	output.open(fullFileName);
	R2::CObjectSerializerClient serializer( pRtData );
	serializer.serial(output);
	output.flush();
	output.close();

	if( true )	// debug
	{
		// text
		fullFileName = fileName;
		fullFileName += ".rt.txt";

		COFile output;
		//std::stringstream ss;
		std::string ss;
		output.open(fullFileName);
		pRtData->serialize(ss);
		//std::string str = ss.str();
		output.serial(ss);
		output.flush();
		output.close();
	}
	return true;
}


NLMISC_COMMAND(dumpValidPositions, "dump valid position to create objects from the current camera viewpoint", "<filename>")
{
	if (args.size() != 1) return false;
	try
	{
		COFile output(args[0]);
		CBitmap result;
		Driver->getBuffer(result);
		if (result.getWidth() == 0 || result.getHeight() == 0) return false;
		CMatrix camMatrix = MainCam.getMatrix();
		NL3D::CFrustum fru = MainCam.getFrustum();
		R2::CTool::CWorldViewRay wvr;
		wvr.Right = camMatrix.getI().normed();
		wvr.Up = camMatrix.getK().normed();
		wvr.OnMiniMap = false;
		wvr.Origin = camMatrix.getPos();
		wvr.Valid = true;
		for (uint x = 0; x < result.getWidth(); ++x)
		{
			for (uint y = 0; y < result.getHeight(); ++y)
			{
				CVector ray(fru.Left + (fru.Right - fru.Left) * (x / (float) result.getWidth()), fru.Near,
							fru.Top + (fru.Bottom - fru.Top) * (y / (float) result.getHeight()));
				ray.normalize();
				ray = camMatrix.mulVector(ray);
				CVector inter;
				wvr.Dir = ray;
				R2::CTool::TRayIntersectionType rit = R2::CTool::computeLandscapeRayIntersection(wvr, inter);
				CRGBA resultCol;
				switch(rit)
				{
					case  R2::CTool::NoIntersection: resultCol = CRGBA(0, 0, 0, 0); break;
					case  R2::CTool::ValidPacsPos: resultCol = CRGBA(0, 0, 255, 100); break;
					default: resultCol = CRGBA(255, 0, 0, 100); break;
				}
				CRGBA *dest = ((CRGBA *) &result.getPixels(0)[0]) + x + y * result.getWidth();
				dest->blendFromui(*dest, resultCol, resultCol.A);
			}





		}
		result.writeTGA(output, 32, false);
	}
	catch(...)
	{
	}
	return true;
}

/*
// *********************************************************************************************************
NLMISC_COMMAND(resetScenario, "reset R2 editor, reload all scripts & xml", "")
{
	if (!args.empty()) return false;
	R2::ResetScenarioWanted = true;
	return true;
}

// *********************************************************************************************************
NLMISC_COMMAND(reloadScenario, "reset R2 editor, reload all scripts & xml (possibly preserving scenario)", "")
{
	if (!args.empty()) return false;
	//
	R2::ReloadScenarioWanted = true;
	return true;
}
*/

bool IsInRingMode()
{
	return R2::getEditor().getMode() != R2::CEditor::NotInitialized;
}


