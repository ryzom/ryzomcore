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
#include "action_historic.h"
#include "property_accessor.h"
#include "../object_factory_client.h"
#include "../editor.h"
#include "nel/gui/lua_ihm.h"

namespace R2
{

/////////////////////
// CActionHistoric //
/////////////////////


//====================================================================================
CActionHistoric::CActionHistoric() : _Scenario(NULL, st_edit)
{
	_NewActionIsPending = false;
	_CurrActionIndex = -1;
	setMaxNumActions(50);
	_SubActionCount = 0;
}

//====================================================================================
CActionHistoric::~CActionHistoric()
{
}

//====================================================================================
void CActionHistoric::newSingleAction(const ucstring &name)
{
	//H_AUTO(R2_CActionHistoric_newSingleAction)
	endAction();
	if (_SubActionCount == 0)
	{
		_SubActionCount = 1;
		_NewActionIsPending = false;
	}
	if (!_NewActionIsPending)
	{
		_NewActionName = name;
	}
}

//====================================================================================
void CActionHistoric::cancelAction()
{
	//H_AUTO(R2_CActionHistoric_cancelAction)
	if (_NewAction && _NewActionIsPending)
	{
		_NewAction->rollback(_DMC, _Scenario);
	}
	_NewAction = NULL;
	_NewActionIsPending = false;
	_SubActionCount = 1;
	_NewActionName = "";
	getEditor().callEnvMethod("onCancelActionInHistoric", 0, 0);
}

//====================================================================================
void CActionHistoric::newMultiAction(const ucstring &name, uint actionCount)
{
	//H_AUTO(R2_CActionHistoric_newMultiAction)
	_SubActionCount = 1; // force previous multi action to finish
	endAction();
	_NewActionIsPending = false;
	_SubActionCount = actionCount;
	_NewActionName = name;
}

//====================================================================================
void CActionHistoric::flushPendingAction()
{
	//H_AUTO(R2_CActionHistoric_flushPendingAction)
	if (_NewAction && _NewActionIsPending)
	{
		_NewAction->flush(_DMC, _Scenario);
	}
}

//====================================================================================
void CActionHistoric::newPendingMultiAction(const ucstring &name, uint actionCount)
{
	//H_AUTO(R2_CActionHistoric_newPendingMultiAction)
	_SubActionCount = 1; // force previous multi action to finish
	endAction();
	_NewActionIsPending = true;
	_SubActionCount = actionCount;
	_NewActionName = name;
	getEditor().callEnvMethod("onPendingActionBegin", 0, 0);
}

//====================================================================================
void CActionHistoric::newPendingAction(const ucstring &name)
{
	//H_AUTO(R2_CActionHistoric_newPendingAction)
	endAction();
	_NewActionIsPending = true;
	_NewActionName = name;
	getEditor().callEnvMethod("onPendingActionBegin", 0, 0);
}

//====================================================================================
void CActionHistoric::endAction()
{
	//H_AUTO(R2_CActionHistoric_endAction)
	// if current action is void, no-op
	if (!_NewAction)
	{
		return;
	};

	if (_SubActionCount == 0)
	{
		return;
	}
	-- _SubActionCount;
	if (_SubActionCount > 0)
	{
		return; // several actions expected before the merge
	}
	++_CurrActionIndex;
	// Push the new action -> discard all actions beyond _CurrActionIndex
	uint newSize = (uint) std::max((sint) _Actions.getSize() + _CurrActionIndex, 0);
	while (_Actions.getSize() > newSize)
	{
		_Actions.pop();
	}
	_CurrActionIndex = -1;
	_Actions.push(_NewAction);
	if (!_NewActionIsPending)
	{
		_NewAction->flush(_DMC, _Scenario); // redo may trigger other observers that complete the action
	}
	_NewAction->setCompleted();
	_CurrActionIndex = -1;
	_NewAction = NULL;
	_NewActionIsPending = false;
	// warn lua that a new action has been added
	if (isUndoSupported())
	{
		getEditor().callEnvMethod("onNewActionAddedInHistoric", 0, 0);
	}
}

//====================================================================================
void CActionHistoric::forceEndMultiAction()
{
	//H_AUTO(R2_CActionHistoric_forceEndMultiAction)
	if (_NewActionIsPending && _NewAction)
	{
		_NewAction->flush(_DMC, _Scenario);
	}
	if (_SubActionCount > 1) _SubActionCount = 1;
	endAction();
}

//====================================================================================
void CActionHistoric::setMaxNumActions(uint count)
{
	//H_AUTO(R2_CActionHistoric_setMaxNumActions)
	nlassert(count >= 1);
	_Actions.setMaxSize(count);
}

//====================================================================================
void CActionHistoric::clear(CObject *newScenario)
{
	//H_AUTO(R2_CActionHistoric_clear)
	if (_NewActionIsPending)
	{
		nlwarning("Historic was cleared while a pending action!!");
	}
	_Actions.clear();
	_CurrActionIndex = -1;
	_NewAction = NULL;
	_NewActionIsPending = false;
	_Scenario.setHighLevel(newScenario ? CRequestBase::cloneObject(newScenario) : NULL);
	// warn lua that all actions have been cleared
	getEditor().callEnvMethod("onClearActionHistoric", 0, 0);
}


//====================================================================================
sint CActionHistoric::getPreviousActionIndex() const
{
	//H_AUTO(R2_CActionHistoric_getPreviousActionIndex)
	sint index = (sint) _Actions.getSize() + _CurrActionIndex;
	return index >= 0 ? index : -1;
}

//====================================================================================
sint CActionHistoric::getNextActionIndex() const
{
	//H_AUTO(R2_CActionHistoric_getNextActionIndex)
	if (_Actions.empty()) return -1;
	sint index = (sint) _Actions.getSize() + (_CurrActionIndex + 1);
	return index >= 0 && index < (sint) _Actions.getSize() ? index : -1;
}

//====================================================================================
bool CActionHistoric::canUndo() const
{
	//H_AUTO(R2_CActionHistoric_canUndo)
	if (_NewActionIsPending) return false;
	return getPreviousActionIndex() != -1;
}

//====================================================================================
bool CActionHistoric::canRedo() const
{
	//H_AUTO(R2_CActionHistoric_canRedo)
	if (_NewActionIsPending) return false;
	return getNextActionIndex() != -1;
}

//====================================================================================
bool CActionHistoric::undo()
{
	//H_AUTO(R2_CActionHistoric_undo)
	nlassert(!_NewActionIsPending);
	sint index = getPreviousActionIndex();
	if (index == -1) return false;
	_Actions[index]->undo(_DMC, _Scenario);
	-- _CurrActionIndex;
	return true;
}

//====================================================================================
bool CActionHistoric::redo()
{
	//H_AUTO(R2_CActionHistoric_redo)
	nlassert(!_NewActionIsPending);
	sint index = getNextActionIndex();
	if (index == -1) return false;
	_Actions[index]->redo(_DMC, _Scenario);
	++ _CurrActionIndex;
	return true;
}

//====================================================================================
const ucstring *CActionHistoric::getPreviousActionName() const
{
	//H_AUTO(R2_CActionHistoric_getPreviousActionName)
	sint index = getPreviousActionIndex();
	return index != -1 ? &_Actions[index]->getName() : NULL;
}

//====================================================================================
const ucstring *CActionHistoric::getNextActionName() const
{
	//H_AUTO(R2_CActionHistoric_getNextActionName)
	sint index = getNextActionIndex();
	return index != -1 ? &_Actions[index]->getName() : NULL;
}

//====================================================================================
void CActionHistoric::requestInsertNode(const std::string& instanceId, const std::string& name,sint32 position, const std::string& key, CObject* value)
{
	//H_AUTO(R2_CActionHistoric_requestInsertNode)
	if (_Scenario.getHighLevel())
	{
		CObject *target = _DMC->find(instanceId, name);
		if (value->getGhost() || (target && target->getGhost()))
		{
			// direct effect, assumed to be local display only
			getEditor().getDMC().nodeInserted(instanceId, name, position, key, value->clone());
			return;
		}
	}
	if (!_NewAction) _NewAction = new CAction(_NewActionName);
	CRequestBase::TSmartPtr req = new CRequestInsertNode(instanceId, name, position, key, value);
	//if (_NewActionIsPending) req->redo(_DMC, _Scenario);
	_NewAction->pushRequest(req);
}

//====================================================================================
void CActionHistoric::requestSetNode(const std::string& instanceId,const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CActionHistoric_requestSetNode)
	if (_Scenario.getHighLevel())
	{
		CObject *clObj = _DMC->find(instanceId, attrName);
		if (clObj && clObj->getGhost())
		{
			value->setGhost(true);
		}
		if (value->getGhost())
		{
			// direct effect, assumed to be local display only
			getEditor().getDMC().nodeSet(instanceId, attrName, value->clone());
			return;
		}
	}
	if (!_NewAction) _NewAction = new CAction(_NewActionName);
	CRequestBase::TSmartPtr req = new CRequestSetNode(instanceId, attrName, value);
	//if (_NewActionIsPending) req->redo(_DMC, _Scenario);
	_NewAction->pushRequest(req);
}

//====================================================================================
void CActionHistoric::requestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CActionHistoric_requestEraseNode)
	if (_Scenario.getHighLevel())
	{
		CObject *obj = _DMC->find(instanceId, attrName, position);
		if (obj && obj->getGhost())
		{
			// direct effect, assumed to be local display only
			getEditor().getDMC().nodeErased(instanceId, attrName, position);
			return;
		}
	}
	if (!_NewAction) _NewAction = new CAction(_NewActionName);
	CRequestBase::TSmartPtr req = new CRequestEraseNode(instanceId, attrName, position);
	//if (_NewActionIsPending) req->redo(_DMC, _Scenario);
	_NewAction->pushRequest(req);
}

//====================================================================================
void CActionHistoric::requestMoveNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CActionHistoric_requestMoveNode)
	if (_Scenario.getHighLevel())
	{
		CObject *src = _DMC->find(instanceId, attrName, position);
		CObject *dest = _DMC->find(destInstanceId, destAttrName);
		if (src && dest)
		{
			nlassert(src->getGhost() == dest->getGhost());
			if (src->getGhost())
			{
				// direct effect, assumed to be local display only
				getEditor().getDMC().nodeMoved(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
				return;
			}
		}
	}
	if (!_NewAction) _NewAction = new CAction(_NewActionName);
	CRequestBase::TSmartPtr req = new CRequestMoveNode(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
	//if (_NewActionIsPending) req->redo(_DMC, _Scenario);
	_NewAction->pushRequest(req);
}

/////////////
// CAction //
/////////////


//====================================================================================
CActionHistoric::CAction::CAction(const ucstring &name)  : _Name(name), _FlushedCount(0), _Completed(false), _Flushing(false)
{
}

//====================================================================================
void CActionHistoric::CAction::pushRequest(CRequestBase *req)
{
	//H_AUTO(R2_CAction_pushRequest)
	nlassert(req);
	nlassert(!_Completed);
	_Requests.push_back(req);
}


//====================================================================================
CActionHistoric::CAction::~CAction()
{
}

//====================================================================================
void CActionHistoric::CAction::flush(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CAction_flush)
	nlwarning("Flushing action at 0x%p", this);
	if (_Flushing) return;
	_Flushing = true;
	nlassert(_FlushedCount <= _Requests.size());
	nlassert(!_Completed);
	while (_FlushedCount != _Requests.size())
	{
		_Requests[_FlushedCount]->redo(dmc, scenario);
		++ _FlushedCount;
		nlassert(_FlushedCount <= _Requests.size());
	}
	nlassert(_FlushedCount == _Requests.size());
	_Flushing = false;
}

//====================================================================================
void CActionHistoric::CAction::rollback(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CAction_rollback)
	nlassert(_FlushedCount <= _Requests.size());
	for (sint k = _FlushedCount - 1; k >= 0; --k)
	{
		_Requests[k]->undo(dmc, scenario);
	}
	_FlushedCount = 0;
}

//====================================================================================
void CActionHistoric::CAction::redo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CAction_redo)
	nlassert(_FlushedCount <= _Requests.size());
	nlassert(_Completed);
	nlassert(dmc);
	for (uint k = 0; k < _Requests.size(); ++k)
	{
		_Requests[k]->redo(dmc, scenario);
	}
}

//====================================================================================
void CActionHistoric::CAction::undo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CAction_undo)
	nlassert(_FlushedCount <= _Requests.size());
	nlassert(_Completed);
	nlassert(dmc);
	for (sint k = (sint)_Requests.size() - 1; k >= 0; --k)
	{
		_Requests[k]->undo(dmc, scenario);
	}
}

//////////////
// REQUESTS //
//////////////

//====================================================================================
CObject *CActionHistoric::CRequestBase::cloneObject(const CObject *src)
{
	//H_AUTO(R2_CRequestBase_cloneObject)
	CObject *result = src->clone();
	struct CDisableRefIDs : public IObjectVisitor
	{
		virtual void visit(CObjectRefId &obj)
		{
			CObjectRefIdClient *refId = NLMISC::safe_cast<CObjectRefIdClient *>(&obj);
			refId->enable(false); // disable events
		}
	};
	CDisableRefIDs disableRefIDs;
	result->visit(disableRefIDs);
	return result;
}

//====================================================================================
CActionHistoric::CRequestSetNode::CRequestSetNode(const std::string &instanceId,
								 const std::string& attrName,
								 CObject* value)
{
	nlassert(value);
	_InstanceId = instanceId;
	_AttrName = attrName;
	_NewValue = cloneObject(value);
}
//
void CActionHistoric::CRequestSetNode::redo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CRequestSetNode_redo)
	_OldValue = NULL;
	if (scenario.getHighLevel()) // undo allowed ?
	{
		CObject *old = scenario.find(_InstanceId, _AttrName);
		if (old) // may be not found if defined in the base and not redefined yet
		{
			// maybe a shadowed property ?
			/*CObject *shadow = dmc->getPropertyAccessor().getShadowingValue(old);
			if (shadow) old = shadow; // use the shadow instead*/
			_OldValue = cloneObject(old);
			nlwarning("**** backupped value for undo request set node");
			_OldValue->dump();
		}
		// modify local version
		scenario.setNode(_InstanceId, _AttrName, _NewValue);
	}
	// send to network
	dmc->doRequestSetNode(_InstanceId, _AttrName, _NewValue);
}
//
void CActionHistoric::CRequestSetNode::undo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CRequestSetNode_undo)
	nlassert(scenario.getHighLevel()); // if this assert fires, then 'clear' was called with a NULL pointer ! The action historic need a start scenario to have undo capability
	if (!_OldValue)
	{
		// was a value from the base ? just erase ...

		// modify local version
		scenario.eraseNode(_InstanceId, _AttrName, -1);
		// send to network
		dmc->doRequestEraseNode(_InstanceId, _AttrName, -1);
	}
	else
	{
		// modify local version
		scenario.setNode(_InstanceId, _AttrName, _OldValue);
		// send to network
		// TODO nico : handle case where same value than the base is restored here
		dmc->doRequestSetNode(_InstanceId, _AttrName, _OldValue);
	}

	_OldValue = NULL;
}
//====================================================================================
CActionHistoric::CRequestEraseNode::CRequestEraseNode(const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CRequestEraseNode_CRequestEraseNode)
	_InstanceId = instanceId;
	_AttrName = attrName;
	_Position = position;
}
//
void CActionHistoric::CRequestEraseNode::redo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CRequestEraseNode_redo)
	nlassert(dmc);
	_OldValue = NULL;
	if (scenario.getHighLevel()) // undo allowed ?
	{
		CObject *old = scenario.find(_InstanceId, _AttrName, _Position);
		if (!old)
		{
			nlwarning("Can't redo erase request!!");
			nlassert(0); // TMP TMP
			return;
		}
		if (!old->getNameInParent(_ParentInstanceId, _AttrNameInParent, _PositionInParent))
		{
			nlwarning("Can't retrieve name in parent, requestEraseNode undo will fail!");
		}
		_OldValue = cloneObject(old);

		// modify local version
		scenario.eraseNode(_InstanceId, _AttrName, _Position);
	}
	// send to network
	dmc->doRequestEraseNode(_InstanceId, _AttrName, _Position);
}
//
void CActionHistoric::CRequestEraseNode::undo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CRequestEraseNode_undo)
	nlassert(scenario.getHighLevel()); // if this assert fires, then 'clear' was called with a NULL pointer ! The action historic need a start scenario to have undo capability
	nlassert(dmc);
	if (!_OldValue)
	{
		nlwarning("Can't undo erase node, previous node not saved");
		return;
	}
	if (_ParentInstanceId.empty())
	{
		nlwarning("Can't undo erase node, parent name not known");
		return;
	}
	// modify local version
	scenario.insertNode(_ParentInstanceId, _AttrNameInParent, _PositionInParent, "", cloneObject(_OldValue));
	// send to network
	dmc->doRequestInsertNode(_ParentInstanceId, _AttrNameInParent, _PositionInParent, "", _OldValue);
	_OldValue = NULL;

	CLuaIHM::push(getEditor().getLua(), _InstanceId);
	getEditor().callEnvMethod("setUndoRedoInstances", 1, 0);
}
//====================================================================================
CActionHistoric::CRequestInsertNode::CRequestInsertNode(const std::string& instanceId,
									   const std::string &attrName,
									   sint32 position,
									   const std::string& key,
									   CObject* value)
{
	_InstanceId = instanceId;
	_AttrName = attrName;
	_Position = position;
	_Key = key;
	_Value = cloneObject(value);
}
//
void CActionHistoric::CRequestInsertNode::redo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CRequestInsertNode_redo)
	nlassert(dmc);
	if (scenario.getHighLevel())
	{
		// modify local version
		scenario.insertNode(_InstanceId, _AttrName, _Position, _Key, cloneObject(_Value));
	}
	// send to network
	dmc->doRequestInsertNode(_InstanceId, _AttrName, _Position, _Key, _Value);

	CObject* nodeId = _Value->findAttr("InstanceId");
	CLuaIHM::push(getEditor().getLua(), nodeId->toString());
	getEditor().callEnvMethod("setUndoRedoInstances", 1, 0);
}
//
void CActionHistoric::CRequestInsertNode::undo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CRequestInsertNode_undo)
	nlassert(scenario.getHighLevel()); // if this assert fires, then 'clear' was called with a NULL pointer ! The action historic need a start scenario to have undo capability
	nlassert(dmc);
	// if there's a key here, not good for us :(
	// so try to find another shorter name for this object...
	if (!_Key.empty())
	{
		std::string instanceId;
		std::string attrName;
		sint32      position;
		CObject *currObj = scenario.find(_InstanceId, _AttrName, _Position, _Key);
		if (!currObj) return;
		if (currObj->getShortestName(instanceId, attrName, position))
		{
			// modify local version
			scenario.eraseNode(instanceId, attrName, position);
			// send to network
			dmc->doRequestEraseNode(instanceId, attrName, position);

			CLuaIHM::push(getEditor().getLua(), instanceId);
			getEditor().callEnvMethod("setUndoRedoInstances", 1, 0);
		}
		else
		{
			nlassert(0); // TMP : can this really happen in practice ?
			nlwarning("Can't build request insert node reciprocal");
		}
	}
	else
	{
		// special here : if position is -1, requestEraseNode will erase the
		// table, not the last element!
		if (!_AttrName.empty() && _Position == -1)
		{
			CObject *parentTable = scenario.find(_InstanceId, _AttrName);
			if (parentTable && parentTable->isTable())
			{
				uint index = parentTable->getSize() - 1;
				// modify local version
				scenario.eraseNode(_InstanceId, _AttrName, index);
				// send to network
				dmc->doRequestEraseNode(_InstanceId, _AttrName, index);
			}
		}
		else
		{
			// modify local version
			scenario.eraseNode(_InstanceId, _AttrName, _Position);
			// send to network
			dmc->doRequestEraseNode(_InstanceId, _AttrName, _Position);
		}
	}
}
//====================================================================================
CActionHistoric::CRequestMoveNode::CRequestMoveNode(const std::string &srcInstanceId,
								   const std::string &srcAttrName,
								   sint32			  srcPosition,
								   const std::string &destInstanceId,
								   const std::string &destAttrName,
								   sint32 destPosition)
{
	_SrcInstanceId = srcInstanceId;
	_SrcAttrName = srcAttrName;
	_SrcPosition = srcPosition;
	_DestInstanceId = destInstanceId;
	_DestAttrName = destAttrName;
	_DestPosition = destPosition;
}
//
void CActionHistoric::CRequestMoveNode::redo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CRequestMoveNode_redo)
	nlassert(dmc);
	if (scenario.getHighLevel())
	{
		CObject *src = scenario.find(_SrcInstanceId, _SrcAttrName, _SrcPosition);
		if (!src)
		{
			nlwarning("Can't find source when moving node");
			return;
		}
		if (!src->getNameInParent(_SrcInstanceIdInParent, _SrcAttrNameInParent, _SrcPositionInParent))
		{
			nlwarning("Can't find name of source object in its parent");
			return;
		}
		// modify local version
		scenario.moveNode(_SrcInstanceId,
						  _SrcAttrName,
						  _SrcPosition,
						  _DestInstanceId,
						  _DestAttrName,
						  _DestPosition);
		if (!src->getShortestName(_DestInstanceIdAfterMove, _DestAttrNameAfterMove, _DestPositionAfterMove))
		{
			nlwarning("Can't retrieve name of instance after move, undo will fail");
		}
	}
	// send to network
	dmc->doRequestMoveNode(_SrcInstanceId,
						 _SrcAttrName,
						 _SrcPosition,
						 _DestInstanceId,
						 _DestAttrName,
						 _DestPosition);
}
//
void CActionHistoric::CRequestMoveNode::undo(IDynamicMapClient *dmc, CScenario &scenario)
{
	//H_AUTO(R2_CRequestMoveNode_undo)
	nlassert(scenario.getHighLevel()); // if this assert fires, then 'clear' was called with a NULL pointer ! The action historic need a start scenario to have undo capability
	nlassert(dmc);
	// modify local version
	scenario.moveNode(_DestInstanceIdAfterMove,
					  _DestAttrNameAfterMove,
					  _DestPositionAfterMove,
					  _SrcInstanceIdInParent,
					  _SrcAttrNameInParent,
					  _SrcPositionInParent);
	// send to network
	dmc->doRequestMoveNode(_DestInstanceIdAfterMove,
						   _DestAttrNameAfterMove,
						   _DestPositionAfterMove,
						   _SrcInstanceIdInParent,
						   _SrcAttrNameInParent,
						   _SrcPositionInParent);
}












} // R2
