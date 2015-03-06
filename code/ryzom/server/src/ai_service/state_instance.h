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

#ifndef RYAI_STATE_INSTANCE_H
#define RYAI_STATE_INSTANCE_H

#include "event_manager.h"
#include "timer.h"
#include "debug_history.h"
#include "keyword_owner.h"
#include "event_manager.h"
#include "script_vm.h"
#include "ai_grp.h"

class CStateMachine;
class CGroup;
class CAIState;

class CPersistentStateInstance;
class CAliasTreeOwner;

extern NLLIGO::CLigoConfig LigoConfig;

//////////////////////////////////////////////////////////////////////////////
// CStateInstance                                                           //
//////////////////////////////////////////////////////////////////////////////

class CStateInstance
: public AIVM::IScriptContext
{
public:
	inline
	CStateInstance(CAIState* startState);
	
	void init(CAIState* startState);
	
	virtual CPersistentStateInstance* getPersistentStateInstance();
	
	//////////////////////////////////////////////////////////////////////////
	//	State Persistent.
	
	struct CStatePersistentObjEntry
	{
		CStatePersistentObjEntry();
		CStatePersistentObjEntry(CAIState const* state, NLMISC::CSmartPtr<NLMISC::CVirtualRefCount> obj);
		virtual ~CStatePersistentObjEntry();
		CAIState const* _State;
		NLMISC::CSmartPtr<NLMISC::CVirtualRefCount> _Obj;
	};
	typedef std::vector<CStatePersistentObjEntry> TStatePersistentObjList;
	
	TStatePersistentObjList _StatePersistentObjList;
	
	// Made to allow obj with life time less or equal to state affectation life time.
	void addStatePersistentObj(CAIState const* keyState, NLMISC::CSmartPtr<NLMISC::CVirtualRefCount> anyObj);
	void removeExceptForState(CAIState const* keyState);
	
	/** Try to obtain a group interface from the CStateInstance. Can return NULL if the
	 *	CStateInstance if not implemeted by a group related objet.
	 */
	// Bad, Bad, Bad ..
	virtual CGroup* getGroup() = 0;
	
	//////////////////////////////////////////////////////////////////////////
	
	CAITimerExtended& timerStateTimeout() { return _StateTimeout; }	
	CAITimerExtended& timerPunctTimeout() { return _PunctualStateTimeout; }	
	CAITimerExtended& timerUser(uint idx);
	
	CAIState* getCAIState();
	
	virtual	CAliasTreeOwner* aliasTreeOwner() = 0;
	
	virtual void stateChange(CAIState const* oldState, CAIState const* newState) = 0;
	
	CAIState* getState() const { return _state; }
	void setNextState(CAIState*);
	
	CAIState* getPunctualState() const { return _PunctualState; }
	CAIState* getNextPunctualState() const { return _NextPunctualState; }
	void setNextPunctualState(CAIState* state);
	void cancelPunctualState() { _CancelPunctualState = true; }
	
	std::string buidStateInstanceDebugString() const;
	
	void dumpVarsAndFunctions(CStringWriter& sw) const;
		
	virtual CDebugHistory* getDebugHistory () = 0;
	
	CAIState const* getActiveState() const;
	
	sint32 getUserTimer(uint timerId);
	void setUserTimer(uint timerId, sint32 time);
	
	void logicVarsToString(std::string& str) const;
	
	float getNelVar(std::string const& varId);
	void setNelVar(std::string const& varId, float value);
	void delNelVar(std::string const& varId);
	
	std::string getStrNelVar(std::string const& varId);
	void setStrNelVar(std::string const& varId, std::string const& value);
	void delStrNelVar(std::string const& varId);

	static void setGlobalNelVar(std::string const& varId, float value);
	static void setGlobalNelVar(std::string const& varId, std::string value);
	
	CAITimerExtended const& userTimer (uint32 index) const;
	
	bool advanceUserTimer(uint32 nbTicks);
	
	void processStateEvent(CAIEvent const& stateEvent, CAIState const* state = NULL);
	
	//	callerStateInstance could be NULL;
	void interpretCode(AIVM::IScriptContext* callerStateInstance, AIVM::CByteCodeEntry const& codeScriptEntry);
	void interpretCode(AIVM::IScriptContext* callerStateInstance, NLMISC::CSmartPtr<AIVM::CByteCode const> const& codeScript);
	
	/// @name IScriptContext implementation
	//@{
	virtual std::string getContextName();
	virtual void interpretCodeOnChildren(AIVM::CByteCodeEntry const& codeScriptEntry);
	
	float getLogicVar(NLMISC::TStringId	varId);
	void setLogicVar(NLMISC::TStringId varId, float value);
	std::string getStrLogicVar(NLMISC::TStringId varId);
	void setStrLogicVar(NLMISC::TStringId varId, std::string const& value);
	AIVM::IScriptContext* getCtxLogicVar(NLMISC::TStringId varId);
	void setCtxLogicVar(NLMISC::TStringId varId, AIVM::IScriptContext* value);
	void setFirstBotSpawned();
	
	virtual AIVM::IScriptContext* findContext(NLMISC::TStringId const strId);
	
	virtual void setScriptCallBack(NLMISC::TStringId const& eventName, AIVM::CByteCodeEntry const& codeScriptEntry);
	virtual AIVM::CByteCodeEntry const* getScriptCallBackPtr(NLMISC::TStringId const& eventName) const;
	virtual void callScriptCallBack(AIVM::IScriptContext* caller, NLMISC::TStringId const& funcName, int mode = 0, std::string const& inParamsSig = "", std::string const& outParamsSig = "", AIVM::CScriptStack* stack = NULL);
	virtual void callNativeCallBack(AIVM::IScriptContext* caller, std::string const&       funcName, int mode = 0, std::string const& inParamsSig = "", std::string const& outParamsSig = "", AIVM::CScriptStack* stack = NULL);
	
	void blockUserEvent(uint32 eventId);
	void unblockUserEvent(uint32 eventId);
	bool isUserEventBlocked(uint32 eventId) const;
	//@}
	
protected:
	/// Logic variables
	typedef	std::map<NLMISC::TStringId, float>           TLogicVarList;
	typedef	std::map<NLMISC::TStringId, std::string>     TStrLogicVarList;
	typedef std::map<NLMISC::TStringId, uint32>		 TLogicVarIndex;
	typedef	std::map<NLMISC::TStringId, AIVM::IScriptContext*> TCtxLogicVarList;
	TLogicVarList    _LogicVar;
	TStrLogicVarList _StrLogicVar;
	TCtxLogicVarList _CtxLogicVar;
	
	// Nel variables
	typedef	std::map<std::string, NLMISC::CVariable<float>*>       TNelVarList;
	typedef	std::map<std::string, NLMISC::CVariable<std::string>*> TStrNelVarList;
	TNelVarList    _NelVar;
	TStrNelVarList _StrNelVar;
	
	// Callbacks (?)
	typedef	std::map<NLMISC::TStringId, AIVM::CByteCodeEntry> TCallBackList;
	TCallBackList _CallBacks;
	
	/// Flag for variable modification
	bool _LogicVarChanged;
	bool _LogicVarChangedList[4];
	//TLogicVarIndex _VarIndex;
	// update logic	timers ---------------------------------------------
	/// 4 timers available for user logic
	CAITimerExtended _UserTimer[4];			
	
	/// timer for timing positional states
	CAITimerExtended _StateTimeout;			
	/// current state (index into manager's state vector)		
	CAIState* _state;
	/// variable set to request a state change (std::numeric_limits<uint32>::max() otherwise)
	CAIState* _NextState;
	
	/// timer for timing	punctual states
	CAITimerExtended	_PunctualStateTimeout;
	
	CAIState* _PunctualState;
	CAIState* _NextPunctualState;
	
	/// Flag for leaving the punctual state, returning to normal behavior
	bool _CancelPunctualState;
	bool _FirstBotSpawned;

	uint32 _UserEventBlocked;
};

//////////////////////////////////////////////////////////////////////////////
// CPersistentStateInstance                                                 //
//////////////////////////////////////////////////////////////////////////////

class CPersistentStateInstance
: public NLMISC::CDbgRefCount<CPersistentStateInstance>
, public CKeyWordOwner
, public CStateInstance
{
public:
	CPersistentStateInstance(CStateMachine& reactionContainer);
	virtual	~CPersistentStateInstance();
	
	typedef	std::vector<NLMISC::CDbgPtr<CPersistentStateInstance> >	TChildList;
	
	void setParentStateInstance(CPersistentStateInstance* parentStateInstance);
	
	CPersistentStateInstance* getParentStateInstance() const { return _ParentStateInstance; }
	
	void addChildStateInstance(CPersistentStateInstance* parentStateInstance);
	
	void removeChildStateInstance(CPersistentStateInstance* parentStateInstance);
	
	TChildList& childs() { return _PSIChilds; }
	
	TChildList _PSIChilds;
	
	// Interface to state status variables -----------------------------
	CAIState* getStartState() { return _StartState; }
	void setStartState(CAIState* state);
	
	CStateMachine& getEventContainer() { return _Container; }
	
	void updateStateInstance();
	
	//////////////////////////////////////////////////////////////////////////
	//	CStateInstance
	CPersistentStateInstance* getPersistentStateInstance() { return this; }
	
	//////////////////////////////////////////////////////////////////////////
	
private:
	/// id of the state to use at startup
	NLMISC::CDbgPtr<CAIState> _StartState;
	CStateMachine& _Container;
	NLMISC::CDbgPtr<CPersistentStateInstance> _ParentStateInstance;
};

/****************************************************************************/
/* Inlined methods                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CStateInstance                                                           //
//////////////////////////////////////////////////////////////////////////////

inline
CStateInstance::CStateInstance(CAIState* startState)
{
	_UserEventBlocked = 0;
	_CtxLogicVar[NLMISC::CStringMapper::map("@this")] = this;
	init(startState);
}

inline
CPersistentStateInstance* CStateInstance::getPersistentStateInstance()
{
//	nlassert(false);
	return (CPersistentStateInstance*)NULL;
}

inline
CStateInstance::CStatePersistentObjEntry::CStatePersistentObjEntry()
{
}

inline
CStateInstance::CStatePersistentObjEntry::CStatePersistentObjEntry(CAIState const* state, NLMISC::CSmartPtr<NLMISC::CVirtualRefCount> obj)
: _State(state)
, _Obj(obj)
{
}

inline
CStateInstance::CStatePersistentObjEntry::~CStatePersistentObjEntry()
{
	_Obj = NULL;
	_State = NULL;
}

inline
CAITimerExtended& CStateInstance::timerUser(uint idx)
{
	nlassert(idx<4); return _UserTimer[idx];
}

inline
void CStateInstance::logicVarsToString(std::string& str) const
{
	for	(TLogicVarList::const_iterator it=_LogicVar.begin(), itEnd=_LogicVar.end();it!=itEnd;++it)
		str+=*(it->first)+"="+NLMISC::toString(it->second)+" ";
}

inline
float CStateInstance::getLogicVar(NLMISC::TStringId	varId)
{
	TLogicVarList::iterator		it=_LogicVar.find(varId);
	if (it==_LogicVar.end())
	{
		_LogicVar[varId]=0.f;
		return	0.f;
	}
	return	it->second;	//_LogicVar[varId];
}

inline
void CStateInstance::setLogicVar(NLMISC::TStringId varId, float value)
{
	_LogicVar[varId] = value;
	_LogicVarChanged = true;

	if (varId && varId->size() == 2 && (*varId)[0] == 'v')
	{
		sint32 index = (*varId)[1] - '0';
		if (0 <= index && index < 4)
		{
			//_VarIndex[varId]
			_LogicVarChangedList[static_cast<uint32>(index)] = true;
		}
		
	}
	
}

inline
std::string CStateInstance::getStrLogicVar(NLMISC::TStringId varId)
{
	return _StrLogicVar[varId];
}

inline
void CStateInstance::setStrLogicVar(NLMISC::TStringId varId, std::string const& value)
{
	_StrLogicVar[varId] = value;
	_LogicVarChanged = true;
	if (varId && varId->size() == 2 && (*varId)[0] == 'v')
	{
		sint32 index = (*varId)[1] - '0';
		if (0 <= index && index < 4)
		{
			//_VarIndex[varId]
			_LogicVarChangedList[static_cast<uint32>(index)] = true;
		}
		
	}
	//_LogicVarChangedList[_VarIndex[varId]] = true;
}

inline
AIVM::IScriptContext* CStateInstance::getCtxLogicVar(NLMISC::TStringId varId)
{
	return _CtxLogicVar[varId];
}

inline
void CStateInstance::setCtxLogicVar(NLMISC::TStringId varId, AIVM::IScriptContext* value)
{
	_CtxLogicVar[varId] = value;
	_LogicVarChanged = true;
}

inline
float CStateInstance::getNelVar(std::string const& varId)
{
	TNelVarList::iterator it = _NelVar.find(varId);
	if (it==_NelVar.end())
	{
		if (NLMISC::CVariable<float>::exists(varId))
		{
			nlwarning("Nel variable \"%s\" exists outside of this state instance", varId.c_str());
			return 0.f;
		}
		_NelVar[varId] = new NLMISC::CVariable<float>("StateInstance", varId.c_str(), "", 0.f);
		_NelVar[varId]->get();
	}
	return it->second->get();
}

inline
void CStateInstance::setNelVar(std::string const& varId, float value)
{
	TNelVarList::iterator it = _NelVar.find(varId);
	if (it==_NelVar.end())
	{
		if (NLMISC::CVariable<float>::exists(varId))
		{
			nlwarning("Nel variable \"%s\" exists outside of this state instance", varId.c_str());
			return;
		}
		_NelVar[varId] = new NLMISC::CVariable<float>("StateInstance", varId.c_str(), "", 0.f);
	}
	_NelVar[varId]->set(value);
}

inline
void CStateInstance::delNelVar(std::string const& varId)
{
	TNelVarList::iterator it = _NelVar.find(varId);
	if (it!=_NelVar.end())
	{
		delete it->second;
		_NelVar.erase(it);
	}
	else
	{
		nldebug("Trying to delete Nel variable that doesn't exist: \"%s\"", varId.c_str());
	}
}

inline
std::string CStateInstance::getStrNelVar(std::string const& varId)
{
	TStrNelVarList::iterator it = _StrNelVar.find(varId);
	if (it==_StrNelVar.end())
	{
		if (NLMISC::CVariable<float>::exists(varId))
		{
			nlwarning("Nel variable \"%s\" exists outside of this state instance", varId.c_str());
			return "";
		}
		_StrNelVar[varId] = new NLMISC::CVariable<std::string>("StateInstanceVar", varId.c_str(), "", std::string());
		_NelVar[varId]->get();
	}
	return it->second->get();
}

inline
void CStateInstance::setStrNelVar(std::string const& varId, std::string const& value)
{
	TStrNelVarList::iterator it = _StrNelVar.find(varId);
	if (it==_StrNelVar.end())
	{
		if (NLMISC::CVariable<float>::exists(varId))
		{
			nlwarning("Nel variable \"%s\" exists outside of this state instance", varId.c_str());
			return;
		}
		_StrNelVar[varId] = new NLMISC::CVariable<std::string>("StateInstanceStrVar", varId.c_str(), "", std::string());
	}
	_StrNelVar[varId]->set(value);
}

inline
void CStateInstance::delStrNelVar(std::string const& varId)
{
	TStrNelVarList::iterator it = _StrNelVar.find(varId);
	if (it!=_StrNelVar.end())
	{
		delete it->second;
		_StrNelVar.erase(it);
	}
	else
	{
		nldebug("Trying to delete Nel variable that doesn't exist: \"%s\"", varId.c_str());
	}
}

inline
CAITimerExtended const& CStateInstance::userTimer (uint32 index) const
{
	return	_UserTimer[index];
}

inline
bool CStateInstance::advanceUserTimer(uint32 nbTicks)
{
	for (uint k=0; k<4; ++k)
	{
		const	uint32 t = getUserTimer(k);
		setUserTimer(k, (t>nbTicks)?(t-nbTicks):0);
	}
	return	true;
}				

inline
void CStateInstance::processStateEvent(CAIEvent const& stateEvent, CAIState const* state)
{
	//	NOTE: This is a quick and inefficient implementation of event treatment - needs to be re-worked
	//	note that it is OK for state to be 'NULL'
	if	(!state)
	{
		state=getActiveState();
		if	(!state)
			return;
	}
	
	bool foundReaction=false;
	//	nlassert(_mgr);
	for (uint i=0;i<stateEvent.reactionList().size();++i)
	{
		const CAIEventReaction	&reaction=*stateEvent.reactionList()[i];
		if	(!reaction.testCompatibility(this,state))
			continue;
					
		getDebugHistory()->addHistory("STATE: '%s' EVENT: '%s' REACTION: '%s'",	state->getAliasNode()->fullName().c_str(),
			stateEvent.getName().c_str(),	reaction.getAliasNode()->fullName().c_str());
		
		foundReaction=true;
		
		if (!reaction.getAction())
		{
			nlwarning("Failed to find action for event: %s",reaction.getAliasNode()->fullName().c_str());
			continue;
		}
		if (!reaction.getAction()->executeAction(this, NULL))
		{
			nlwarning("Failed to execute action for event '%s': for stateInstance:'%s' in state:'%s'",	stateEvent.getName().c_str(),
				aliasTreeOwner()->getAliasNode()->fullName().c_str(),	state->getAliasNode()->fullName().c_str());
			continue;
		}
		
	}
	if (!foundReaction)
	{
		getDebugHistory()->addHistory("STATE: '%s' EVENT: '%s' NO REACTION",	state->getAliasNode()->fullName().c_str(),
			stateEvent.getName().c_str());
	}
	
}

inline
void CStateInstance::setNextState(CAIState* state)
{
	// we're allowed to set state to 'no state'
	if (!state)
	{
		_NextState = state;
		return;
	}
	
	// make sure the state is positional (not punctual)
	if (!state->isPositional()) 
	{ 
		nlwarning("setNextState(): State should not be punctual '%s'%s - setting state to ~0",
			state->getAliasNode()->fullName().c_str(),
			state->getAliasString().c_str());
		_NextState = NULL;
		return;
	}
	
	// set the next state
	_NextState = state;
}

inline
void CStateInstance::setNextPunctualState(CAIState* state)
{ 
	// we're allowed to set state to 'no state'
	if (!state)
		return;
	
	// make sure the state is not positional (ie punctual)
	if (state->isPositional()) 
	{ 
		nlwarning("CStateInstance::setNextPunctualState(): State should be punctual '%s'%s - setting state to ~0",
			state->getAliasNode()->fullName().c_str(),
			state->getAliasString().c_str());
		state = NULL;
		return;
	} 
	
	// set the next state
	_NextPunctualState = state;
}

//////////////////////////////////////////////////////////////////////////////
// CPersistentStateInstance                                                 //
//////////////////////////////////////////////////////////////////////////////

inline
CPersistentStateInstance::CPersistentStateInstance(CStateMachine& reactionContainer)
: CKeyWordOwner()
, CStateInstance(NULL)
, _StartState()
, _Container(reactionContainer)
{
}

inline
CPersistentStateInstance::~CPersistentStateInstance()
{
#if !FINAL_VERSION
	if (_PSIChilds.size()!=0)
		nlwarning("a Npc group is dying and still have childs, which is not possible !!");
	nlassert(_PSIChilds.size()==0);
#endif
	if (!_ParentStateInstance.isNULL())
		_ParentStateInstance->removeChildStateInstance(this);
}

inline
void CPersistentStateInstance::setParentStateInstance(CPersistentStateInstance* parentStateInstance)
{
	if (!_ParentStateInstance.isNULL())
		_ParentStateInstance->removeChildStateInstance(this);
	_ParentStateInstance=parentStateInstance;
	if (parentStateInstance!=NULL)
		parentStateInstance->addChildStateInstance(this);
}

inline
void CPersistentStateInstance::addChildStateInstance(CPersistentStateInstance* parentStateInstance)
{
#if !FINAL_VERSION
	nlassert(std::find(_PSIChilds.begin(), _PSIChilds.end(), NLMISC::CDbgPtr<CPersistentStateInstance>(parentStateInstance))==_PSIChilds.end());
#endif
	_PSIChilds.push_back(parentStateInstance);
}

inline
void CPersistentStateInstance::removeChildStateInstance(CPersistentStateInstance* parentStateInstance)
{
	TChildList::iterator it = std::find(_PSIChilds.begin(), _PSIChilds.end(), NLMISC::CDbgPtr<CPersistentStateInstance>(parentStateInstance));
#if !FINAL_VERSION
	nlassert(it!=_PSIChilds.end());
#endif		
	if (it!=_PSIChilds.end())
		_PSIChilds.erase(it);
}

inline
void CPersistentStateInstance::setStartState(CAIState* state)
{
	_StartState = state;
	CStateInstance::init(_StartState);
}

#endif
