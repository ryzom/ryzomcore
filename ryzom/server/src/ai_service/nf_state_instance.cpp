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

#include	"stdpch.h"

#include "script_compiler.h"

#include "ai_grp_npc.h"
#include "group_profile.h"
#include "ai_generic_fight.h"
#include "server_share/msg_brick_service.h"

#include "continent_inline.h"
#include "dyn_grp_inline.h"

#include "ai_script_data_manager.h"
#include "ai_logic_action.h"
using std::string;
using std::vector;
using namespace NLMISC;
using namespace AIVM;
using namespace AICOMP;
using namespace AITYPES;
using namespace RYAI_MAP_CRUNCH;


//----------------------------------------------------------------------------
/** @page code

@subsection setEvent_f_
Triggers a user event.

Arguments: f(EventId) ->
@param EventId is the user event id to be triggered

@code
()setEvent(0); // Triggers the event 0
@endcode

*/
// CStateInstance
void setEvent_f_(CStateInstance* entity, CScriptStack& stack)
{
	size_t const eventIndex = (size_t)(float)stack.top();
	stack.pop();
	
	if (!entity)
	{
		nlwarning("sendEvent failed");
		return;
	}
	
	nlassert(eventIndex<10);


	if (IsRingShard.get())
	{
		if ( entity->isUserEventBlocked( (uint32)eventIndex)	) { return; } // Do not allow uservent recursion on ring shard
		entity->blockUserEvent((uint32)eventIndex);
	}	
	
	entity->processStateEvent(entity->getPersistentStateInstance()->getEventContainer().EventUserEvent[eventIndex]);

	if (IsRingShard.get())
	{
		entity->unblockUserEvent((uint32)eventIndex);
	}

}

//----------------------------------------------------------------------------
/** @page code

@subsection setTimer_ff_
Sets a timer delay and start it.

Arguments: f(DeltaTime), f(TimerId) ->
@param[in] DeltaTime is the time (in ticks) before the timer event is triggered
@param[in] TimerId is the timer ID

@code
()setTimer(200, 0); // Sets the timer t0 to 200 ticks
@endcode

*/
// CStateInstance
void setTimer_ff_(CStateInstance* si, CScriptStack& stack)
{
	size_t const timerId = (int)(float)stack.top();
	stack.pop();
	size_t const deltaTime = (int)(float)stack.top();
	stack.pop();
	
	if (!si)
	{
		nlwarning("setTimer failed");
		return;
	}
	
	if (timerId<0 || timerId>3)
	{
		string errStr("critical, native function setTimer used with out of bound parameter (0-3) equals to ");
		errStr += toString(timerId);
		nlwarning(errStr.c_str());
		nlassert(false);
	}
	
	si->timerUser((uint)timerId).set((uint32)deltaTime);
}

//----------------------------------------------------------------------------
/** @page code

@subsection timerSetRyzomDaytime_fff_
Set a timer at a specified hour in Ryzom time and start it.

Arguments: f(TimerId), f(Hour), f(Minute) ->
@param[in] TimerId is the timer ID
@param[in] Hour is the hour we want the timer to trigger
@param[in] Minute is the minute we want the timer to trigger

@code
()timerSetRyzomDaytime(0, 13, 45); // create a timer at 13h45 (Ryzom time)
@endcode

*/
// CStateInstance
void timerSetRyzomDaytime_fff_(CStateInstance* si, CScriptStack& stack)
{

	float const minute = (float)stack.top();
	stack.pop();

	float const hour = (float)stack.top();
	stack.pop();

	size_t const timerId = (int)(float)stack.top();
	stack.pop();
	
	if ( !(0 <= minute && minute <=59 && 0 <= hour && hour <= 23) )
	{
		return;
	}

	float datime = hour + (minute * 100.f)/(60.f*100.f);
	

	float currentTime = CTimeInterface::getRyzomTime().getRyzomTime();
	if (datime < currentTime)
	{
		// advance to next day
		currentTime -= 24;
	}
	float deltaTime = datime - currentTime;
	// convert to ticks
	uint32 timeTicks = uint32(deltaTime * RYZOM_HOURS_IN_TICKS);


	if (!si)
	{
		nlwarning("timerSetRyzomDaytime failed");
		return;
	}
	
	if (timerId<0 || timerId>3)
	{
		string errStr("critical, native function timerSetRyzomDaytime used with out of bound parameter (0-3) equals to ");
		errStr += toString(timerId);
		nlwarning(errStr.c_str());
		return;
	}
	
	si->timerUser((uint)timerId).set((uint32)timeTicks);		
}

//----------------------------------------------------------------------------
/** @page code

@subsection timerIsEnabled_f_f
Test if a timer is enabled.

Arguments: f(TimerId) -> f(IsEnabled)
@param[in] TimerId is the timer ID
@param[out] IsEnabled 1 if the timer is enabled

@code
(isEnabled)timerIsEnabled(0); // test if the timer t0 is enabled and put the ret in isEnabled variable
@endcode

*/
// CStateInstance
void timerIsEnabled_f_f(CStateInstance* si, CScriptStack& stack)
{
	size_t const timerId = (int)(float)stack.top();
	stack.pop();
	
	
	if (!si)
	{
		nlwarning("timerIsEnabled failed");
		return;
	}
	
	if (timerId<0 || timerId>3)
	{
		string errStr("critical, native function timerIsEnabled used with out of bound parameter (0-3) equals to ");
		errStr += toString(timerId);
		nlwarning(errStr.c_str());
		return;
	}
	
	float isEnabled = si->timerUser((uint)timerId).isEnabled();
	stack.push(isEnabled);
}

//----------------------------------------------------------------------------
/** @page code

@subsection timerIsSuspended_f_f
Test if a timer is suspended.

Arguments: f(TimerId) -> f(IsSuspended)
@param[in] TimerId is the timer ID
@param[out] IsSuspended 1 if the timer is suspended

@code
(isSuspended)timerIsSuspended(0); // test if the timer t0 is suspended and put the ret in isSuspended variable
@endcode

*/
// CStateInstance
void timerIsSuspended_f_f(CStateInstance* si, CScriptStack& stack)
{
	size_t const timerId = (int)(float)stack.top();
	stack.pop();
	
	
	if (!si)
	{
		nlwarning("timerIsSuspended failed");
		return;
	}
	
	if (timerId<0 || timerId>3)
	{
		string errStr("critical, native function timerIsSuspended used with out of bound parameter (0-3) equals to ");
		errStr += toString(timerId);
		nlwarning(errStr.c_str());
		return;
	}
	
	float isSuspended = si->timerUser((uint)timerId).isSuspended();
	stack.push(isSuspended);
}

//----------------------------------------------------------------------------
/** @page code

@subsection timerSuspend_f_
Suspend a timer.

Arguments: f(TimerId) ->
@param[in] TimerId is the timer ID

@code
()timerSuspend(0); // suspend the timer t0
@endcode

*/
// CStateInstance
void timerSuspend_f_(CStateInstance* si, CScriptStack& stack)
{
	size_t const timerId = (int)(float)stack.top();
	stack.pop();
	
	
	if (!si)
	{
		nlwarning("timerSuspend failed");
		return;
	}
	
	if (timerId<0 || timerId>3)
	{
		string errStr("critical, native function timerSuspend used with out of bound parameter (0-3) equals to ");
		errStr += toString(timerId);
		nlwarning(errStr.c_str());
		return;
	}
	
	si->timerUser((uint)timerId).suspend();
}

//----------------------------------------------------------------------------
/** @page code

@subsection timerDisable_f_
Disable a timer.

Arguments: f(TimerId) ->
@param[in] TimerId is the timer ID

@code
()timerDisable(0); // disable the timer t0
@endcode

*/
// CStateInstance
void timerDisable_f_(CStateInstance* si, CScriptStack& stack)
{
	size_t const timerId = (int)(float)stack.top();
	stack.pop();
	
	
	if (!si)
	{
		nlwarning("timerDisable failed");
		return;
	}
	
	if (timerId<0 || timerId>3)
	{
		string errStr("critical, native function timerDisable used with out of bound parameter (0-3) equals to ");
		errStr += toString(timerId);
		nlwarning(errStr.c_str());
		return;
	}
	
	si->timerUser((uint)timerId).disable();
}

//----------------------------------------------------------------------------
/** @page code

@subsection timerResume_f_
Resume a timer.

Arguments: f(TimerId) ->
@param[in] TimerId is the timer ID

@code
()timerResume(0); // resume the timer t0
@endcode

*/
// CStateInstance
void timerResume_f_(CStateInstance* si, CScriptStack& stack)
{
	size_t const timerId = (int)(float)stack.top();
	stack.pop();
	
	
	if (!si)
	{
		nlwarning("timerResume failed");
		return;
	}
	
	if (timerId<0 || timerId>3)
	{
		string errStr("critical, native function timerResume used with out of bound parameter (0-3) equals to ");
		errStr += toString(timerId);
		nlwarning(errStr.c_str());
		return;
	}
	
	si->timerUser((uint)timerId).resume();	
}

//----------------------------------------------------------------------------
/** @page code

@subsection timerAdd_ff_
Add a delta time to a timer.

Arguments: f(TimerId), f(DeltaTime) ->
@param[in] TimerId is the timer ID
@param[in] DeltaTime is the delta time in ticks

@code
()timerAdd(0, 41); // add 41 ticks to the next trigger
()timerAdd(0, -10); // remove 10 ticks to the next trigger
@endcode

*/
// CStateInstance
void timerAdd_ff_(CStateInstance* si, CScriptStack& stack)
{

	float const dt = (float)stack.top();
	stack.pop();

	size_t const timerId = (int)(float)stack.top();
	stack.pop();
	
	if (!si)
	{
		nlwarning("timerAdd failed");
		return;
	}
	
	if (timerId<0 || timerId>3)
	{
		string errStr("critical, native function timerAdd used with out of bound parameter (0-3) equals to ");
		errStr += toString(timerId);
		nlwarning(errStr.c_str());
		return;
	}
	
	if (dt > 0)
	{
		uint32 dt2 = static_cast<uint32>(dt);
		si->timerUser((uint)timerId).add(dt2);
	}
	else
	{
		uint32 dt2 = static_cast<uint32>(-dt);
		si->timerUser((uint)timerId).sub(dt2);
	}
}

//----------------------------------------------------------------------------
/** @page code

@subsection dssStartAct_ff_
Request the start of a dss Act.

Arguments: f(SessionId), f(ActId)->
@param[in] SessionId is the id session
@param[in] ActId is the id of the act (0 = permanent content)

@code
()dssStartAct(42,2); // Start the 2ond act of the session 42

@endcode

*/
// CStateInstance
void dssStartAct_ff_(CStateInstance* si, CScriptStack& stack)
{

	float const actId = (float)stack.top();
	stack.pop();


	float const sessionId= (float)stack.top();
	stack.pop();

	
	if ( !(0 <= actId && 0 <= sessionId) )
	{
		return;
	}

	CAILogicActionDssStartActHelper::dssStartAct(TSessionId(static_cast<uint32>(sessionId)), static_cast<uint32>(actId));
}

//----------------------------------------------------------------------------
/** @page code

@subsection postNextState_s_
Triggers a state change.

Arguments: s(StateName) ->
@param[in] StateName is the name of the next state

@code
()postNextState("state_invasion_2"); // Post the next state named 'state_invasion_2' if found in this state machine
()postNextState($name_var); // Post the next state named by name_var if found in this state machine
@endcode

*/
// CStateInstance
void postNextState_s_(CStateInstance* si, CScriptStack& stack)
{
	string NextStateName = stack.top();
	stack.pop();
	if (!si)
	{
		nlwarning("postNextState failed!");
		return;
	}
	CAIState* const state = si->getPersistentStateInstance()->getEventContainer().cstStates().getChildByName(NextStateName);
	if (!state)
	{
		string errStr;
		if (si->getGroup())
			errStr="state "+NextStateName+" not found for "+si->getGroup()->getName();
		else
			errStr="state "+NextStateName+" not found for unnamed group";
		nlwarning(errStr.c_str());
		return;
	}

	si->setNextState(state);
}

//----------------------------------------------------------------------------
/** @page code

@subsection import_s_
Imports (executes in current context as a script function) a script defined in
a script rep.

Arguments: s(libName) ->
@param[in] libName is the library name

@code
()import("script_boss");
@endcode

*/
// CStateInstance
void import_s_(CStateInstance* entity, CScriptStack& stack)
{
	string LibName = stack.top();
	stack.pop();
	
	CSmartPtr<const CByteCode> const& codePtr = AIVM::CLibrary::getInstance().getLib(LibName);
	
	if (codePtr!=NULL)
		entity->interpretCode(NULL, codePtr);
	else
		nlwarning("unknown library %s", LibName.c_str());
}

//----------------------------------------------------------------------------
/** @page code

@subsection setNelVar_sf_
Sets the content of a NeL Variable. The variable is created if it doesn't
exist.

Arguments: s(varId),f(value) ->
@param[in] varId is a the name of the variable to set
@param[in] value is a the value to set

@code
()setNelVar("BotCount", 32);
@endcode

*/
// CStateInstance
void setNelVar_sf_(CStateInstance* entity, CScriptStack& stack)
{
	float value = (float)stack.top();
	stack.pop();
	std::string varId = (std::string)stack.top();
	stack.pop();
	
	entity->setNelVar(varId, value);
}

void setGlobalNelVar_sf_(CStateInstance* entity, CScriptStack& stack)
{
	float value = (float)stack.top();
	stack.pop();
	std::string varId = (std::string)stack.top();
	stack.pop();
	
	CStateInstance::setGlobalNelVar(varId, value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getNelVar_s_f
Returns the content of a NeL Variable. The variable is created if it doesn't
exist.

Arguments: s(varId) -> f(value)
@param[in] varId is a the name of the variable to set
@param[out] value is a the value of the variable

@code
(botCount)getNelVar("BotCount");
@endcode

*/
// CStateInstance
void getNelVar_s_f(CStateInstance* entity, CScriptStack& stack)
{
	std::string varId = (std::string)stack.top();
	
	stack.top() = entity->getNelVar(varId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection delNelVar_ss_
Detetes a NeL Variable. Passed value is used to determine the type of the
variable. Content of that value is ignored.

Arguments: s(varId) ->
@param[in] varId is a the name of the variable to delete
@param[in] value is a a value of the same type of the variable

@code
()delNelVar("BotCount", 0);
@endcode

*/
// CStateInstance
void delNelVar_sf_(CStateInstance* entity, CScriptStack& stack)
{
	stack.pop();
	std::string varId = (std::string)stack.top();
	stack.pop();
	entity->delNelVar(varId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection setNelVar_ss_
Sets the content of a NeL Variable. The variable is created if it doesn't
exist.

Arguments: s(varId),s(value) ->
@param[in] varId is a the name of the variable to set
@param[in] value is a the value to set

@code
()setNelVar("BotFamily", "the_killers");
@endcode

*/
// CStateInstance
void setNelVar_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string value = (std::string)stack.top();
	stack.pop();
	std::string varId = (std::string)stack.top();
	stack.pop();
	
	entity->setStrNelVar(varId, value);
}

void setGlobalNelVar_ss_(CStateInstance* entity, CScriptStack& stack)
{
	std::string value = (std::string)stack.top();
	stack.pop();
	std::string varId = (std::string)stack.top();
	stack.pop();
	
	CStateInstance::setGlobalNelVar(varId, value);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getNelVar_s_s
Returns the content of a NeL Variable. The variable is created if it doesn't
exist.

Arguments: s(varId) -> s(value)
@param[in] varId is a the name of the variable to set
@param[out] value is a the value of the variable

@code
(botFamily)getNelVar("BotFamily");
@endcode

*/
// CStateInstance
void getNelVar_s_s(CStateInstance* entity, CScriptStack& stack)
{
	std::string varId = (std::string)stack.top();
	
	stack.top() = entity->getStrNelVar(varId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection delNelVar_ss_
Detetes a NeL Variable. Passed value is used to determine the type of the
variable. Content of that value is ignored.

Arguments: s(varId) ->
@param[in] varId is a the name of the variable to delete
@param[in] value is a a value of the same type of the variable

@code
()delNelVar("BotFamily", "");
@endcode

*/
// CStateInstance
void delNelVar_ss_(CStateInstance* entity, CScriptStack& stack)
{
	stack.pop();
	std::string varId = (std::string)stack.top();
	stack.pop();
	entity->delStrNelVar(varId);
}

//----------------------------------------------------------------------------
/** @page code

@subsection getStateName__s
Returns the name of the current state.

Arguments: -> s(StateName)
@param[out] StateName is the name of the current state

@code
($name)getStateName();
@endcode

*/
// CStateInstance
void getStateName__s(CStateInstance* si, CScriptStack& stack)
{
	string str;
	
	breakable
	{
		if (!si)
			break;
		
		CAIState const* const curState = si->getState();
		if (!curState)
			break;
		
		str = curState->getName();
	}
	
	stack.push(str);
}

//////////////////////////////////////////////////////////////////////////////
// Undocumented methods                                                     //
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// CStateInstance
void loadFile_s_(CStateInstance* entity, CScriptStack& stack)
{
	string fileName = stack.top();
	stack.pop();
	
	try
	{
		NLMISC::CIFile file(NLMISC::CPath::lookup(fileName));
		
		vector<string> lines;
		while (!file.eof())
		{
			const size_t bufferSize = 4*1024;
			char buffer[bufferSize];
			file.getline(buffer, bufferSize);
			lines.push_back(buffer);
		}
		// Compile the buffer
		CSmartPtr<const CByteCode> codePtr = CCompiler::getInstance().compileCode(lines, fileName);
		
		// Interpret the code for the group
		entity->interpretCode(NULL, codePtr);
	}
	catch (const EPathNotFound &)
	{
		nlwarning("Path not found while loading AIS script %s", fileName.c_str());
	}
}

/****************************************************************************/

std::map<std::string, FScrptNativeFunc> nfGetStateInstanceNativeFunctions()
{
	std::map<std::string, FScrptNativeFunc> functions;
	
#define REGISTER_NATIVE_FUNC(cont, func) cont.insert(std::make_pair(std::string(#func), &func))
	
	REGISTER_NATIVE_FUNC(functions, setEvent_f_);
	REGISTER_NATIVE_FUNC(functions, setTimer_ff_);
	REGISTER_NATIVE_FUNC(functions, timerSetRyzomDaytime_fff_);
	REGISTER_NATIVE_FUNC(functions, timerDisable_f_);
	REGISTER_NATIVE_FUNC(functions, timerSuspend_f_);
	REGISTER_NATIVE_FUNC(functions, timerResume_f_);
	REGISTER_NATIVE_FUNC(functions, timerAdd_ff_);
	REGISTER_NATIVE_FUNC(functions, timerIsEnabled_f_f);
	REGISTER_NATIVE_FUNC(functions, timerIsSuspended_f_f);
	REGISTER_NATIVE_FUNC(functions, dssStartAct_ff_);
	REGISTER_NATIVE_FUNC(functions, postNextState_s_);
	REGISTER_NATIVE_FUNC(functions, import_s_);
	REGISTER_NATIVE_FUNC(functions, setNelVar_sf_);
	REGISTER_NATIVE_FUNC(functions, getNelVar_s_f);
	REGISTER_NATIVE_FUNC(functions, delNelVar_sf_);
	REGISTER_NATIVE_FUNC(functions, setNelVar_ss_);
	REGISTER_NATIVE_FUNC(functions, getNelVar_s_s);
	REGISTER_NATIVE_FUNC(functions, delNelVar_ss_);
	REGISTER_NATIVE_FUNC(functions, setGlobalNelVar_sf_);
	REGISTER_NATIVE_FUNC(functions, setGlobalNelVar_ss_);
	REGISTER_NATIVE_FUNC(functions, getStateName__s);
	REGISTER_NATIVE_FUNC(functions, loadFile_s_);
	
#undef REGISTER_NATIVE_FUNC
	
	return functions;
}
