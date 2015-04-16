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

#include "dmc.h"



#include "nel/net/unified_network.h"
#include "nel/net/module_manager.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"

#include "game_share/object.h"
#include "game_share/scenario.h"
#include "game_share/r2_messages.h"
#include "game_share/ring_access.h"

#include "com_lua_module.h"
#include "property_accessor.h"
#include "palette.h"

#include "action_historic.h"

#include "client_edition_module.h"

#include "../editor.h"
#include "../instance.h"


#include <assert.h>



using namespace NLNET;
using namespace NLMISC;

namespace R2
{

//===================================================================================================================================================================================
// Utility class used by CFrameActionsRecorder
// Forward commands directly to the client
// Forward commands to the server for its record
class CClientInstantActionFeedBack : public IDynamicMapClient
{
public:
	CClientInstantActionFeedBack(CDynamicMapClient &dmc) : _DMC(dmc) {}
	virtual void doRequestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value);
	virtual void doRequestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
	virtual void doRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position);
	virtual void doRequestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);
	virtual CObject *find(const std::string& /* instanceId */, const std::string& /* attrName */ = "", sint32 /* position */ = -1, const std::string &/* key */ ="")
	{
		nlassert(0);
		return NULL;
	}
private:
	CDynamicMapClient &_DMC;
};

//========
void CClientInstantActionFeedBack::doRequestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value)
{
	//H_AUTO(R2_CClientInstantActionFeedBack_doRequestInsertNode)
	// server record (must be done before client feedback because 'unchanged value' optimisation would prevent the change, else)
	_DMC.doRequestInsertNode(instanceId, name, position, key, value);
	// client instant feed back
	_DMC.nodeInserted(instanceId, name, position, key, value->clone());
}

//========
void CClientInstantActionFeedBack::doRequestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CClientInstantActionFeedBack_doRequestSetNode)
	// server record (must be done before client feedback because 'unchanged value' optimisation would prevent the change, else)
	_DMC.doRequestSetNode(instanceId, attrName, value);
	// client instant feed back
	_DMC.nodeSet(instanceId, attrName, value->clone());
}

//========
void CClientInstantActionFeedBack::doRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CClientInstantActionFeedBack_doRequestEraseNode)
	// server record (must be done before client feedback because 'unchanged value' optimisation would prevent the change, else)
	_DMC.doRequestEraseNode(instanceId, attrName, position);
	// client instant feed back
	_DMC.nodeErased(instanceId, attrName, position);
}

//========
void CClientInstantActionFeedBack::doRequestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CClientInstantActionFeedBack_doRequestMoveNode)
	// server record (must be done before client feedback because 'unchanged value' optimisation would prevent the change, else)
	_DMC.doRequestMoveNode(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
	// client instant feed back
	_DMC.nodeMoved(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
}

//===================================================================================================================================================================================

// TMP Nico : instant feedback class
// This class record all the commands issued at current frame
// For this we reuse the CActionHistoric class, but without undo capability (it doesn't hold a local copy of the scenario)
// At the end of the frame, commands are redirected directly to client, and set to server too.
// For now the server feedback is ignored, however (server just repeats the command in mono edition case)
class CFrameActionsRecorder : public IDynamicMapClient
{
public:
	CFrameActionsRecorder(CDynamicMapClient &dmc) : _DMC(dmc), _Flushing(false) { _FrameActions.newSingleAction(ucstring()); }
	// from IDynamicMapClient
	virtual void doRequestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value);
	virtual void doRequestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
	virtual void doRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position);
	virtual void doRequestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition);
	virtual CObject *find(const std::string& instanceId, const std::string& attrName = "", sint32 position = -1, const std::string &key ="")
	{
		return _DMC.find(instanceId, attrName, position, key);
	}
	// flush all the commands recorded at this frame
	void flush();
private:
	CDynamicMapClient &_DMC;
	CActionHistoric _FrameActions;
	bool _Flushing;
};


//========
void CFrameActionsRecorder::doRequestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value)
{
	//H_AUTO(R2_CFrameActionsRecorder_doRequestInsertNode)
	_FrameActions.requestInsertNode(instanceId, name, position, key, value);
}

//========
void CFrameActionsRecorder::doRequestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CFrameActionsRecorder_doRequestSetNode)
	_FrameActions.requestSetNode(instanceId, attrName, value);
}

//========
void CFrameActionsRecorder::doRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CFrameActionsRecorder_doRequestEraseNode)
	_FrameActions.requestEraseNode(instanceId, attrName, position);
}

//========
void CFrameActionsRecorder::doRequestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CFrameActionsRecorder_doRequestMoveNode)
	_FrameActions.requestMoveNode(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
}

//========
void CFrameActionsRecorder::flush()
{
	//H_AUTO(R2_CFrameActionsRecorder_flush)
	if (_Flushing) return;
	if (!_FrameActions.isNewActionBeingRecorded()) return;
	_Flushing = true;
	CClientInstantActionFeedBack feedback(_DMC);
	_FrameActions.setDMC(&feedback);
	_FrameActions.endAction();
	_FrameActions.clear();
	_FrameActions.setDMC(NULL);
	_FrameActions.newSingleAction(ucstring());
	_Flushing = false;
}


CDynamicMapClient::CDynamicMapClient(const std::string &/* eid */, NLNET::IModuleSocket * clientGateway, lua_State *luaState)
{

	_ComLua = 0;
		//new CComLuaModule(this, luaState);

	_ActionHistoric.setDMC(this);

	IModuleManager &mm = IModuleManager::getInstance();


	//Edition
	IModule *clientEditionModuleInterface = mm.createModule("ClientEditionModule", "ClientEditionModule", "");

	nlassert(clientEditionModuleInterface != NULL);

	_EditionModule = safe_cast<CClientEditionModule* >(clientEditionModuleInterface);

	_EditionModule->init(clientGateway, this);


	_FrameActionsRecorder = new CFrameActionsRecorder(*this);

	if (!luaState)
	{
		loadFeatures(); // NB nico : when used by the client,  features are loaded externally for now ...
	}

	_InstantFeedBackFlag = false;

}

void CDynamicMapClient::release()
{
	//H_AUTO(R2_CDynamicMapClient_release)
	delete _ComLua;
	_ComLua = 0;
	_EditionModule->release();
	CRingAccess::releaseInstance();
		//new CComLuaModule(this, luaState);
}

void CDynamicMapClient::init(lua_State *luaState)
{
	//H_AUTO(R2_CDynamicMapClient_init)
	nlassert(luaState != 0);
	_ComLua = new CComLuaModule(this, luaState);
	_EditionModule->init();
	CRingAccess::getInstance().init();

}



CDynamicMapClient::~CDynamicMapClient()
{
	delete _ComLua;
	IModuleManager &mm = IModuleManager::getInstance();
	mm.deleteModule(_EditionModule);
	delete _FrameActionsRecorder;


}






void CDynamicMapClient::save(const std::string& /* filename */)
{
	//H_AUTO(R2_CDynamicMapClient_save)
	//	CScenarioValidator::TValues	emptyValues;
//	_EditionModule->saveWithParam(filename, emptyValues);

}

void CDynamicMapClient::saveRtData(const std::string& filename)
{
	//H_AUTO(R2_CDynamicMapClient_saveRtData)
	std::string name;
	name += filename;
	//std::ostringstream out2;
	std::string out2;
	NLMISC::COFile out(name.c_str());
	CObject* scenario =  _EditionModule->getCurrentScenario()->getRtData();
	if (scenario)
	{
		//out2 <<"scenario = "<< *scenario ;
		out2 += "scenario = ";
		scenario->serialize(out2);
		//std::string tmp=out2.str();
		out.serialBuffer((uint8*)out2.c_str(),(uint)out2.size());
		return;
	}
	nlwarning("Can't save: no scenario yet");

}

void CDynamicMapClient::requestTalkAs(const std::string& npcname)
{
	//H_AUTO(R2_CDynamicMapClient_requestTalkAs)
	_EditionModule->requestTalkAs(npcname);
}
bool CDynamicMapClient::load(const std::string& filename)
{
	//H_AUTO(R2_CDynamicMapClient_load)
	return _ComLua->load(filename);
}
extern bool ConnectionWanted;

bool CDynamicMapClient::loadAnimationFromBuffer(const std::string& content, const std::string& filename, std::string& errMsg, const CScenarioValidator::TValues& values)
{
	//H_AUTO(R2_CDynamicMapClient_loadAnimationFromBuffer)
	CObject* data = _ComLua->loadFromBuffer(content, filename, values);
	if (!data)
	{
		errMsg = "Invalid Data";
		R2::getEditor().setStartingAnimationFilename("");
		R2::getEditor().getLua().push(errMsg);
		R2::getEditor().getLua().pcallByName("printWarning", 1, 0, -1 );
		R2::getEditor().getLua().executeScriptNoThrow("UnitTest.testLoadAnimationScenarioUi()");
		return false;
	}

	R2::getEditor().getLua().executeScriptNoThrow("r2.RingAccess.LoadAnimation = true");
	_EditionModule->setMustStartScenario( true );
	this->scenarioUpdated(data, false, 1);
	R2::getEditor().getLua().executeScriptNoThrow("r2.RingAccess.LoadAnimation = false");



	if (ClientCfg.R2EDMustVerifyRingAccessWhileLoadingAnimation)
	{
		CLuaState &ls = R2::getEditor().getLua();
		CLuaStackChecker lsc(&ls);
		R2::getEditor().callEnvFunc( "verifyScenario", 0, 2);
		if (!ls.isBoolean(-2))
		{
			nlassert(0 && "verifyScenario return wrong type");
		}
		if (!ls.isString(-1))
		{
			nlassert(0 && "verifyScenario return wrong type");
		}
		bool result = ls.toBoolean(-2);
		std::string msg = ls.toString(-1);
		ls.pop(2);
		if (!result)
		{
			_EditionModule->setMustStartScenario( false );
			R2::getEditor().setStartingAnimationFilename("");
			R2::getEditor().clearContent();
			errMsg = msg;

			R2::getEditor().getLua().push(errMsg);
			R2::getEditor().getLua().pcallByName("printWarning", 1, 0, -1 );
			R2::getEditor().getLua().executeScriptNoThrow("UnitTest.testLoadAnimationScenarioUi()");
			return false;
			// Todo repop loading interface();
		}
	}
	return true;
}

void CDynamicMapClient::loadFeatures()
{
	//H_AUTO(R2_CDynamicMapClient_loadFeatures)
	_ComLua->loadFeatures();
}

void CDynamicMapClient::show() const
{
	//H_AUTO(R2_CDynamicMapClient_show)
//	std::stringstream ss;
//	std::string s;
	std::string ss;

	CObject* highLevel= getCurrentScenario()->getHighLevel();
	if (highLevel) { highLevel->serialize(ss); }
	else { ss += "No HL\n"; }

	std::vector<std::string> lines;
	NLMISC::splitString(ss, "\n", lines);
	uint first=0, last=(uint)lines.size();
	for (; first != last ; ++first) { nlinfo("%s", lines[first].c_str()); }

/*	while ( std::getline(ss, s))
	{
		nlinfo("%s", s.c_str());
	}*/

	//nlinfo("%s", ss.str().c_str());
}

void CDynamicMapClient::testConnectionAsCreator()
{
	//H_AUTO(R2_CDynamicMapClient_testConnectionAsCreator)
	nlassert(0);
	_EditionModule->requestMapConnection( 1, true );
}

void CDynamicMapClient::updateScenario(CObject* scenario, bool /* willTP */)
{
	//H_AUTO(R2_CDynamicMapClient_updateScenario)
	_EditionModule->updateScenario(scenario);
}

CObject *CDynamicMapClient::translateScenario(CObject* scenario)
{
	//H_AUTO(R2_CDynamicMapClient_translateScenario)
	std::string errorMsg;
	CObject *rtdata = _ComLua->translateFeatures(scenario, errorMsg);
	if( !rtdata )
	{
		nlwarning( errorMsg.c_str() );
	}
	return rtdata;
}

void CDynamicMapClient::doFile(const std::string& filename)
{
	//H_AUTO(R2_CDynamicMapClient_doFile)

	_ComLua->doFile(filename);
}

void CDynamicMapClient::runLuaScript(const std::string& filename)
{
	//H_AUTO(R2_CDynamicMapClient_runLuaScript)
	std::string erroMsg;
	_ComLua->runLuaScript(filename, erroMsg);
}

void CDynamicMapClient::addPaletteElement(const std::string& attrName, CObject* paletteElement)
{
	//H_AUTO(R2_CDynamicMapClient_addPaletteElement)
	_EditionModule->addPaletteElement(attrName, paletteElement);
}


CObject* CDynamicMapClient::getPropertyValue(CObject* component,  const std::string& attrName) const
{
	//H_AUTO(R2_CDynamicMapClient_getPropertyValue)
	return _EditionModule->getPropertyValue(component,  attrName);
}

CObject* CDynamicMapClient::getPropertyList(CObject* component) const
{
	//H_AUTO(R2_CDynamicMapClient_getPropertyList)
	return _EditionModule->getPropertyList(component);
}

CObject* CDynamicMapClient::getPaletteElement(const std::string& key)const
{
	//H_AUTO(R2_CDynamicMapClient_getPaletteElement)
	return _EditionModule->getPaletteElement(key);
}

bool CDynamicMapClient::isInPalette(const std::string& key)const
{
	//H_AUTO(R2_CDynamicMapClient_isInPalette)
	return _EditionModule->isInPalette(key);
}


CObject* CDynamicMapClient::newComponent(const std::string& type) const
{
	//H_AUTO(R2_CDynamicMapClient_newComponent)
	return _EditionModule->newComponent(type);
}

void CDynamicMapClient::registerGenerator(CObject* classObject)
{
	//H_AUTO(R2_CDynamicMapClient_registerGenerator)
	_EditionModule->registerGenerator(classObject);
}

void CDynamicMapClient::requestTranslateFeatures()
{
	//H_AUTO(R2_CDynamicMapClient_requestTranslateFeatures)
	_ComLua->callTranslateFeatures(_EditionModule->getCurrentScenario()->getHighLevel());

}

void CDynamicMapClient::requestUpdateRtScenario(CObject* scenario)
{
	//H_AUTO(R2_CDynamicMapClient_requestUpdateRtScenario)

	_EditionModule->requestUpdateRtScenario(scenario);

}

void CDynamicMapClient::requestCreateScenario(CObject* scenario)
{
	//H_AUTO(R2_CDynamicMapClient_requestCreateScenario)
	_EditionModule->requestCreateScenario(scenario);
}


void CDynamicMapClient::requestUploadCurrentScenario()
{
	//H_AUTO(R2_CDynamicMapClient_requestUploadCurrentScenario)

	CObject* highLevel = const_cast<R2::CObject*> ( getHighLevel() ); // the object is not change but the api is broken

	if (highLevel) { requestUploadScenario(highLevel); }
}


void CDynamicMapClient::doRequestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value)
{
	//H_AUTO(R2_CDynamicMapClient_doRequestInsertNode)

	nlwarning("DO REQUEST: insert node %s, %s, %d, %s", instanceId.c_str(), name.c_str(), (int) position, key.c_str());
	_EditionModule->requestInsertNode(instanceId, name, position ,key, value);
}


void CDynamicMapClient::doRequestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CDynamicMapClient_doRequestSetNode)
	nlwarning("DO REQUEST: set node %s, %s", instanceId.c_str(), attrName.c_str());
	_EditionModule->requestSetNode(instanceId, attrName, value);
}


void CDynamicMapClient::doRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CDynamicMapClient_doRequestEraseNode)
	nlwarning("DO REQUEST: erase node %s, %s, %d", instanceId.c_str(), attrName.c_str(), (int) position);
	_EditionModule->requestEraseNode(instanceId, attrName, position);
}


void CDynamicMapClient::doRequestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CDynamicMapClient_doRequestMoveNode)
	nlwarning("DO REQUEST: move node node from [%s, %s, %d] to [%s, %s, %d]",
				instanceId.c_str(), attrName.c_str(), (int) position,
			  destInstanceId.c_str(), destAttrName.c_str(), (int) destPosition
			 );
	_EditionModule->requestMoveNode(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
}

// undo / redo buffering

void CDynamicMapClient::requestInsertNode(const std::string& instanceId, const std::string& name, sint32 position, const std::string& key, CObject* value)
{
	//H_AUTO(R2_CDynamicMapClient_requestInsertNode)
	nlwarning("REQUEST: insert node %s, %s, %d, %s", instanceId.c_str(), name.c_str(), (int) position, key.c_str());
	_ActionHistoric.requestInsertNode(instanceId, name, position, key, value);
}

void CDynamicMapClient::requestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CDynamicMapClient_requestSetNode)
	if (attrName == "Versions")
	{
		nlwarning("SETTING VERSION");
	}
	_ActionHistoric.requestSetNode(instanceId, attrName, value);
}

// class to notify each erased instance's observers that the
// erase request is about to be sent
class CNotifySonDeletion : public CEditor::IObserverAction
{
public:
	virtual ~CNotifySonDeletion()
	{
	}

	CInstance &ErasedInstance;
	CNotifySonDeletion(CInstance &erasedInstance) : ErasedInstance(erasedInstance) {}
	virtual void doAction(CEditor::IInstanceObserver &obs)
	{
		obs.onInstanceEraseRequest(ErasedInstance);
	}
};
// traverse all sons of the instance that is about to be erased
// and warn their observers, using CNotifySonDeletion
class CTraverseEraseRequestedSons : public IInstanceVisitor
{
public:
	virtual ~CTraverseEraseRequestedSons()
	{
	}

	virtual void visit(CInstance &inst)
	{
		CNotifySonDeletion notifySonDeletion(inst);
		getEditor().triggerInstanceObserver(inst.getId(), notifySonDeletion);
	}
};


void CDynamicMapClient::requestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CDynamicMapClient_requestEraseNode)
	// warn observer of node being erased that these node are being erased
	// this allow observer of these node to remove themselves
	// for example, in the export window, the reference to a npc in the scene
	// will remove itself from the export if its target is removed
	CObject *target = find(instanceId, attrName, position);
	if (target)
	{
		CInstance *root = getEditor().getInstanceFromObject(target);
		if (root)
		{
			CTraverseEraseRequestedSons traverseEraseRequestedSons;
			root->visit(traverseEraseRequestedSons);
		}
	}
	nlwarning("REQUEST: erase node %s, %s, %d", instanceId.c_str(), attrName.c_str(), (int) position);
	_ActionHistoric.requestEraseNode(instanceId, attrName, position);
}

void CDynamicMapClient::requestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CDynamicMapClient_requestMoveNode)
	nlwarning("REQUEST: move node node from [%s, %s, %d] to [%s, %s, %d]",
			  instanceId.c_str(), attrName.c_str(), (int) position,
			  destInstanceId.c_str(), destAttrName.c_str(), (int) destPosition
			 );
	_ActionHistoric.requestMoveNode(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
}




void CDynamicMapClient::nodeSet(
		const std::string& instanceId, const std::string& attrName, CObject* value)
{
			//H_AUTO(R2_CDynamicMapClient_nodeSet)
	getCurrentScenario()->setNode(instanceId, attrName,value);
}

void CDynamicMapClient::nodeErased(
		const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CDynamicMapClient_nodeErased)
	getCurrentScenario()->eraseNode(instanceId, attrName, position);
}

void CDynamicMapClient::nodeInserted(
	const std::string& instanceId, const std::string& attrName, sint32 position,
	const std::string& key, CObject* value)
{
		//H_AUTO(R2_CDynamicMapClient_nodeInserted)
	getCurrentScenario()->insertNode(instanceId, attrName, position, key,value);
}

void CDynamicMapClient::nodeMoved(
		const std::string& instanceId, const std::string& attrName, sint32 position,
		const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CDynamicMapClient_nodeMoved)
	getCurrentScenario()->moveNode(instanceId, attrName, position,
			destInstanceId, destAttrName, destPosition);

}

void CDynamicMapClient::scenarioUpdated(CObject* highLevel, bool willTP, uint32 /* startingActIndex */)
{
	//H_AUTO(R2_CDynamicMapClient_scenarioUpdated)
	updateScenario(highLevel, willTP);
}


CObject *CDynamicMapClient::find(const std::string& instanceId, const std::string& attrName /*=""*/, sint32 position /*=-1*/, const std::string &key /*=""*/)
{
	//H_AUTO(R2_CDynamicMapClient_find)
	nlassert(_EditionModule->getCurrentScenario());
	return _EditionModule->getCurrentScenario()->find(instanceId, attrName, position, key);
}

const CObject *CDynamicMapClient::getHighLevel() const
{
	//H_AUTO(R2_CDynamicMapClient_getHighLevel)
	if (!getCurrentScenario()) { return 0;}
	return getCurrentScenario()->getHighLevel();
}

CPropertyAccessor &CDynamicMapClient::getPropertyAccessor() const
{
	//H_AUTO(R2_CDynamicMapClient_getPropertyAccessor)
	return _EditionModule->getPropertyAccessor();
}


void CDynamicMapClient::disconnect()
{
	//H_AUTO(R2_CDynamicMapClient_disconnect)
	//delete _Scenario;//??
	//_Scenario=new CScenario(0);
//	_ComModule->requestDisconnect(_Eid);
}



CScenario* CDynamicMapClient::getCurrentScenario() const
{
	//H_AUTO(R2_CDynamicMapClient_getCurrentScenario)
	return _EditionModule->getCurrentScenario();
}


CComLuaModule& CDynamicMapClient::getComLuaModule() const
{
	//H_AUTO(R2_CDynamicMapClient_getComLuaModule)
	nlassert(_ComLua);
	return *_ComLua;
}


CClientEditionModule& CDynamicMapClient::getEditionModule() const
{
	//H_AUTO(R2_CDynamicMapClient_getEditionModule)
	nlassert(_EditionModule);
	return *_EditionModule;
}


void CDynamicMapClient::requestUploadScenario(CObject* scenario)
{
	//H_AUTO(R2_CDynamicMapClient_requestUploadScenario)
	_EditionModule->requestUploadScenario(scenario);
}

void CDynamicMapClient::onEditionModeConnected( uint32 userSlotId, uint32 /* adventureId */, CObject* highLevel, const std::string& /* versionName */, bool willTP, uint32 initialActIndex)
{
	//H_AUTO(R2_CDynamicMapClient_onEditionModeConnected)
	_EditionModule->setEid(NLMISC::toString("Client%d", userSlotId));
	_EditionModule->getCurrentScenario()->setSessionType(st_edit);
	scenarioUpdated(highLevel, willTP, initialActIndex); // handle by CEditor.cpp that call _EditionModule->updateScenario(highLevel);

	CEditor::connexionMsg("");
	//_EditionModule->requestSetPioneerRight( TPioneerRight::DM);
}


void CDynamicMapClient::onAnimationModeConnected(const CClientMessageAdventureUserConnection & connected)
{
	//H_AUTO(R2_CDynamicMapClient_onAnimationModeConnected)
	_EditionModule->setEid(NLMISC::toString("Client%d", connected.EditSlotId));
	_EditionModule->getCurrentScenario()->setSessionType(connected.SessionType);
	_EditionModule->getCurrentScenario()->setMode(connected.Mode);

	if (connected.Mode == 0 || connected.Mode == 1) // DM Load game or Player wait game
	{
		scenarioUpdated(connected.HighLevel.getData(), true, 1); // handle by CEditor.cpp that call _EditionModule->updateScenario(highLevel);
	}

	if (connected.Mode == 2 || connected.Mode == 3) // DM Play
	{
		// TP done by server
	}

	/*

		  CShareServerEditionItfProxy proxy(_ServerEditionProxy);
			bool unused = true;
			proxy.advConnACK(this, unused);
	*/
}


void CDynamicMapClient::onAnimationModePlayConnected()
{
	//H_AUTO(R2_CDynamicMapClient_onAnimationModePlayConnected)



}

void CDynamicMapClient::onEditionModeDisconnected()
{
	//H_AUTO(R2_CDynamicMapClient_onEditionModeDisconnected)
}

void CDynamicMapClient::onResetEditionMode()
{
	//H_AUTO(R2_CDynamicMapClient_onResetEditionMode)
	//
}


void CDynamicMapClient::onTestModeConnected()
{
	//H_AUTO(R2_CDynamicMapClient_onTestModeConnected)
	CEditor::connexionMsg("");
}

void CDynamicMapClient::onTestModeDisconnected(TSessionId /* sessionId */, uint32 /* lasAct */, TScenarioSessionType /* sessionType */)
{
	//H_AUTO(R2_CDynamicMapClient_onTestModeDisconnected)
	CEditor::connexionMsg("uimR2EDGoToEditingMode");
}

CObject* CDynamicMapClient::getCurrentScenarioHighLevel()
{
	//H_AUTO(R2_CDynamicMapClient_getCurrentScenarioHighLevel)
	return _EditionModule->getCurrentScenario()->getHighLevel();
}


void CDynamicMapClient::onNpcAnimationTargeted(uint32 mode)
{
	//H_AUTO(R2_CDynamicMapClient_onNpcAnimationTargeted)
	nlinfo("R2Cl: DSS_TARGET");

	CLuaState &lua = getEditor().getLua();
	CLuaStackChecker lsc(&lua);

	int initialStackSize = lua.getTop();


	if (mode & CAnimationProp::Spawnable)
	{
		lua.push("r2ed_anim_despawn");
	}

	if (mode & CAnimationProp::Alive)
	{
		lua.push("r2ed_anim_kill");
		if (mode & CAnimationProp::Grouped)	{	lua.push("r2ed_anim_grp_kill"); }
		lua.push("r2ed_anim_add_hp");
		if (mode & CAnimationProp::Grouped)	{	lua.push("r2ed_anim_grp_heal"); }

	}

	if (mode & CAnimationProp::Controlable)
	{
		if (mode & CAnimationProp::Controled )
		{
			lua.push("r2ed_anim_stop_control");
		}
		else
		{
			lua.push("r2ed_anim_control");
		}
	}

	if (mode & CAnimationProp::Speaking)
	{
		if(mode & CAnimationProp::SpeakedAs)
		{
			lua.push("r2ed_anim_stop_speak");
		}
		else
		{
			lua.push("r2ed_anim_speak_as");
		}
	}

	//
	getEditor().callEnvMethod("updateAnimBarActions", lua.getTop() - initialStackSize);

}

TSessionId CDynamicMapClient::getSessionId() const
{
	//H_AUTO(R2_CDynamicMapClient_getSessionId)
	return getEditionModule().getSessionId();
}

void CDynamicMapClient::setInstantFeedBackFlag(bool instantFeedBackFlag)
{
	//H_AUTO(R2_CDynamicMapClient_setInstantFeedBackFlag)
	nlassert(_FrameActionsRecorder);
	if (instantFeedBackFlag == _InstantFeedBackFlag) return;
	if (_InstantFeedBackFlag)
	{
		_FrameActionsRecorder->flush();
	}
	if (instantFeedBackFlag)
	{
		_ActionHistoric.setDMC(_FrameActionsRecorder); // forward to action recording
	}
	else
	{
		_ActionHistoric.setDMC(this); // direct send to server
	}
	_EditionModule->setMute(instantFeedBackFlag);
	_InstantFeedBackFlag = instantFeedBackFlag;
}

void CDynamicMapClient::flushActions()
{
	//H_AUTO(R2_CDynamicMapClient_flushActions)
	if (!_InstantFeedBackFlag) return;
	_FrameActionsRecorder->flush();
}

void CDynamicMapClient::newAction(const ucstring &name)
{
	//H_AUTO(R2_CDynamicMapClient_newAction)
	flushActions();
	_ActionHistoric.newSingleAction(name);
}


} // R2
