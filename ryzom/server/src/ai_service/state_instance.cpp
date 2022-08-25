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
#include "state_instance.h"

#include "nel/misc/smart_ptr.h"
#include "script_vm.h"
#include "script_compiler.h"
#include "event_reaction_container.h"
#include "ai_grp.h"
#include "ai_outpost.h"

using namespace std;
using namespace NLMISC;
using namespace AICOMP;
using namespace AIVM;

//////////////////////////////////////////////////////////////////////////////
// CStateInstance                                                           //
//////////////////////////////////////////////////////////////////////////////

CAIState const* CStateInstance::getActiveState() const
{
#if !FINAL_VERSION
	nlassert(this!=NULL);
#else
	if (!this)
		return NULL; // ugly! -> to remove  date = 19/05/2004.
#endif
	
	if (_PunctualState)	// the puntual state is active !
		return	_PunctualState;
	
	if (_state)	// the normal state is active !
		return	_state;
	
	// no state is active !
	return	NULL;
}

sint32 CStateInstance::getUserTimer(uint timerId)
{
	NLMISC::clamp(timerId, 0u, 3u);
	return _UserTimer[timerId].timeRemaining();
}

void CStateInstance::setUserTimer(uint timerId, sint32 time)
{
	NLMISC::clamp(timerId, 0u, 3u);
	if (_UserTimer[timerId].isEnabled())
		_UserTimer[timerId].set(time);
}

void CStateInstance::addStatePersistentObj(CAIState const* keyState, CSmartPtr<NLMISC::CVirtualRefCount> anyObj)
{
	_StatePersistentObjList.push_back(CStatePersistentObjEntry(keyState, anyObj));
}

void CStateInstance::setFirstBotSpawned()
{
	_FirstBotSpawned = true;
}

struct CDiffStateRemover
{
	CDiffStateRemover(CAIState const* const state) : _State(state) { }
	bool operator()(CStateInstance::CStatePersistentObjEntry const& entry) const
	{
		return entry._State!=_State;
	}
	CAIState const* const _State;
};

void CStateInstance::removeExceptForState(CAIState const* keyState)
{
	TStatePersistentObjList::iterator const itLast = remove_if(_StatePersistentObjList.begin(), _StatePersistentObjList.end(), CDiffStateRemover(keyState));
	_StatePersistentObjList.erase(itLast, _StatePersistentObjList.end());
}

void CStateInstance::setScriptCallBack(TStringId const& eventName, AIVM::CByteCodeEntry const& codeScriptEntry)
{
	_CallBacks[eventName] = codeScriptEntry; // effectively affects this callback to this eventName.
}

AIVM::CByteCodeEntry const* CStateInstance::getScriptCallBackPtr(TStringId const& eventName) const
{
	TCallBackList::const_iterator it = _CallBacks.find(eventName);
	if (it==_CallBacks.end())
		return NULL;
	return &(it->second);
}

void CStateInstance::callScriptCallBack(IScriptContext* caller, TStringId const& funcName, int mode, std::string const& inParamsSig, std::string const& outParamsSig, AIVM::CScriptStack* stack)
{
	// :FIXME: mode, signatures end stack are unused
	AIVM::CByteCodeEntry const* const codeScript = getScriptCallBackPtr(funcName);
	if (!codeScript || !codeScript->isValid())
		return;
	
	interpretCode(caller, *codeScript);	//	effectively calls the call back.
}

void CStateInstance::callNativeCallBack(IScriptContext* caller, std::string const& funcName, int mode, std::string const& inParamsSig, std::string const& outParamsSig, AIVM::CScriptStack* stack)
{
	// :FIXME: caller is unused
	CScriptNativeFuncParams* funcParam = CCompiler::getNativeFunc(funcName, inParamsSig, outParamsSig);
	if (funcParam!=NULL)
	{
		bool varArg = (mode & 1)!=0; // :KLUDGE: Hardcoded 1 :TODO: Replace with a names constant (see script_compiler.cpp)
		if (stack!=NULL)
		{
			if (varArg)
			{
				stack->push(outParamsSig);
				stack->push(inParamsSig);
			}
			funcParam->_func(this, *stack);
		}
		else
		{
			AIVM::CScriptStack dummyStack;
			if (varArg)
			{
				dummyStack.push(outParamsSig);
				dummyStack.push(inParamsSig);
			}
			funcParam->_func(this, dummyStack);
		}
	}
	else
	{
		nlwarning("Trying to call an unknown native function '%s_%s_%s'", funcName.c_str(), inParamsSig.c_str(), outParamsSig.c_str());
		// TODO:kxu: rebuild stack
	}
}

void CStateInstance::dumpVarsAndFunctions(CStringWriter& sw) const
{
	sw.append("float variables:");
	FOREACHC(varIt, TLogicVarList, _LogicVar)
		sw.append(" "+CStringMapper::unmap(varIt->first)+" = "+NLMISC::toString(varIt->second));
	
	sw.append("string variables:");
	FOREACHC(varIt, TStrLogicVarList, _StrLogicVar)
		sw.append(" "+CStringMapper::unmap(varIt->first)+" = "+varIt->second);
	
	sw.append("context variables:");
	FOREACHC(varIt, TCtxLogicVarList, _CtxLogicVar)
		sw.append(" "+CStringMapper::unmap(varIt->first)+" = "+NLMISC::toStringPtr(varIt->second));
	
	sw.append("callBacks:");
	FOREACHC(varIt, TCallBackList, _CallBacks)
		sw.append(" "+CStringMapper::unmap(varIt->first));
}

void CStateInstance::interpretCode(IScriptContext* caller, CByteCodeEntry const& codeScriptEntry)
{
	AIVM::IScriptContext* parent = getPersistentStateInstance()->getParentStateInstance();
	AIVM::CScriptVM::getInstance()->interpretCode(this, parent, caller, codeScriptEntry);
}

void CStateInstance::interpretCode(IScriptContext* caller, CSmartPtr<CByteCode const> const& codeScript)
{
	interpretCode(caller, CByteCodeEntry(codeScript, 0));
}

void CStateInstance::interpretCodeOnChildren(CByteCodeEntry const& codeScriptEntry)
{
	vector<CSmartPtr<CGroup> > tmpList;
	FOREACH(childIt, CPersistentStateInstance::TChildList, getPersistentStateInstance()->childs())
		tmpList.push_back((*childIt)->getGroup());
	
	FOREACH(childIt, vector<CSmartPtr<CGroup> >, tmpList)
		(*childIt)->getPersistentStateInstance()->interpretCode(this, codeScriptEntry);
}

IScriptContext* CStateInstance::findContext(NLMISC::TStringId const strId)
{
#if !FINAL_VERSION
	nlassert(this->getGroup());
#endif
	std::vector<CGroup*> grps;
	this->getGroup()->getAIInstance()->findGroup(grps, CStringMapper::unmap(strId));
	if (grps.size()==1)
		return grps.back()->getPersistentStateInstance();
	else
		return NULL;
}

std::string CStateInstance::getContextName()
{
	return getGroup()->getFullName();
}



void CStateInstance::init(CAIState* startState)
{
	_state=
	_PunctualState=
	_NextPunctualState=NULL;
	
	_CancelPunctualState = false;
	_NextState = startState;
	_LogicVarChanged = false;
	_FirstBotSpawned = false;
//	_VarIndex[CStringMapper::map("v0")]=0;
//	_VarIndex[CStringMapper::map("v1")]=1;
//	_VarIndex[CStringMapper::map("v2")]=2;
//	_VarIndex[CStringMapper::map("v3")]=3;
}

void CStateInstance::setGlobalNelVar(std::string const& varId, float value)
{
	if (NLMISC::ICommand::exists(varId))
		NLMISC::ICommand::execute(NLMISC::toString("%s %f", varId.c_str(), value), *NLMISC::InfoLog);
	else
		nlwarning("Nel variable \"%s\" do not exist", varId.c_str());
}

void CStateInstance::setGlobalNelVar(std::string const& varId, std::string value)
{
	std::vector<std::string> args; args.push_back(value);
	if (NLMISC::ICommand::exists(varId))
		NLMISC::ICommand::execute(NLMISC::toString("%s %s", varId.c_str(), value.c_str()), *NLMISC::InfoLog);
	else
		nlwarning("Nel variable \"%s\" do not exist", varId.c_str());
}


void CStateInstance::blockUserEvent(uint32 eventId)
{
	_UserEventBlocked |= (1 << eventId); 
}

void CStateInstance::unblockUserEvent(uint32 eventId)
{
	_UserEventBlocked &= ~(1 << eventId); 
}

bool CStateInstance::isUserEventBlocked(uint32 eventId) const
{
	return ((1 << eventId) & _UserEventBlocked) != 0;
}

//////////////////////////////////////////////////////////////////////////////
// CPersistentStateInstance                                                 //
//////////////////////////////////////////////////////////////////////////////

void CPersistentStateInstance::updateStateInstance()
{
	// deal with timer events -----------------------------------------
	
	// Event: State Timeout
	if (_StateTimeout.testOnce())
		processStateEvent(getEventContainer().EventPositionalStateTimeout);
	
	// Event: Punctual State Timeout
	if (_PunctualStateTimeout.testOnce())
		processStateEvent(getEventContainer().EventPunctualStateTimeout);
	
	// Event: User Timer n Triggered
	for (uint i=0;i<4;++i)
	{
		if	(!_UserTimer[i].testOnce())
			continue;
		processStateEvent(getEventContainer().EventUserTimer[i]);
	}
	


	// Event logic var changed
	if (_LogicVarChanged)
	{
		processStateEvent(getEventContainer().EventVariableChanged);
		for(uint i =0;i<4;++i)
		{
			if(_LogicVarChangedList[i]==true)
			{
				processStateEvent(getEventContainer().EventVariableChangedTab[i]);
				_LogicVarChangedList[i]=false;
			}
		}
		_LogicVarChanged = false;
	}
	
	// deal with positional state change ------------------------------
	
	// if state change requested (and not in punctual state) then setup new state
	if	(_NextState)
	{
		CAIState const* oldState = NULL;
		if (!_PunctualState && !_NextPunctualState && _state)
		{
			// close down current state
			oldState = _state;
			processStateEvent(getEventContainer().EventEndOfState);
		}
		
		// switch state	& initalise state status flags
		_state = _NextState;
		_NextState = NULL;
		_StateTimeout.disable();
		
		if (_PunctualState || _NextPunctualState)
			return;
		
		removeExceptForState(_state);
		processStateEvent(getEventContainer().EventStartOfState);
		stateChange(oldState, _state);
	}
	
	// deal with start of punctual state ------------------------------
	
	// if state change requested then setup new state
	if (_NextPunctualState)
	{
		CAIState const* oldState = NULL;
		
		breakable
		{
			if (_PunctualState)
			{
				oldState = _PunctualState;
				break;
			}
			if (_state)
			{
				oldState = _state;
				break;
			}
		}
		
		// switch state	& initalize state status flags
		_PunctualState = _NextPunctualState;
		_NextPunctualState = NULL;
		_CancelPunctualState = false;
		_PunctualStateTimeout.disable();
		
		// initalize new state
		CAIState const* const newState = _PunctualState;
		
		removeExceptForState(newState);
		processStateEvent(getEventContainer().EventStartOfState, newState);
		stateChange(oldState, newState);
	}
	
	// must be done after start of state
	if (_FirstBotSpawned)
	{
		_FirstBotSpawned = false;
		processStateEvent(getEventContainer().EventFirstBotSpawned);
	}
	// deal with end of punctual state --------------------------------
	
	if (!_CancelPunctualState)
		return;
	
	_CancelPunctualState = false;
	
	// if there was an active punctual state then cancel it
	if (!_PunctualState)
		return;
	
	// switch state	& initalise state status flags
	_PunctualStateTimeout.disable();
	_StateTimeout.resume();	// this line just in case timeout suspended during punctual state
	
	// close down current state
	CAIState const* const punctualState = _PunctualState;
	
	processStateEvent(getEventContainer().EventEndOfState, punctualState);
	_PunctualState = NULL;
	
	//	this removes obj that depends on state affectation existence.
	removeExceptForState(_state);
	
	// specialized virtual state change call.
	stateChange(punctualState, _state);
}




#include "event_reaction_include.h"
