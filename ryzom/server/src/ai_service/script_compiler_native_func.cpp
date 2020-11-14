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

using std::string;
using std::vector;
using namespace NLMISC;
using namespace AIVM;
using namespace AICOMP;
using namespace AITYPES;
using namespace RYAI_MAP_CRUNCH;


/**

@page code Le script d'AI

@section general General information
Its a C like code.

Comments are defined after two slashes like this:
@code
// my comment
@endcode

Code is used to specify decisions for state machine on group. Each group can
have different variables and functions, they can only be affected via code
only.

Variables don't need to be defined before they are used. They are
automatically created with value equals to 0 or "".

Variables are either floating point numbers (no prefix), character strings
('$' prefix) or script contexts ('@' prefix).

There are two special object that are hard-coded and that can be used to call
function:
- parent: represent the parent group of the current group
- caller: represent the group that called a function on the current group

Additionnaly one can reference any static group defined in the same AI
instance by its name. The usage is similar to that of 'parent' and 'caller'.
In fact 'parent', 'caller' and group names are a kind of pseudo-contexts and
can be used as such (without the '@' prefix).

@section debugAndLog Debug and log
There are two special function for debug and logging:
- print: used for debug purpose, removed in release mode
- log: used to display any information that should be read in release mode

Syntax:
@code
print("foo");
log("bar");
@endcode

NB: you can put any number of const string (e.g. "toto") and var (@c float or
@c $string) separated by coma. All the string representation are concatenated
to form the final string.

E.g:
@code
print("Ma position est ", x, ":", y);
@endcode

@section aboutFunctions About functions
You can dynamically define function on the group.

This function are called <em>script function</em> or <em>user function</em>
(as opposed to the hard coded "native function").

User functions don't have parameters.

Functions can be defined this way:
@code
my_function()
{
  print("message"); // print a message
  mavar = 5*mavar; // the variable mavar is now equals to mavar*5
  caller.mavar = mavar; // the variable mavar of the caller's is now equal to mavar (see bellow for more explanation)
}
@endcode
You can call a function like this (note that if a function is not defined for a group, nothing is called):
@code
my_function(); // call my_function from this group
group1_tryker.my_function(); // call my_function for group1_tryker (inside this function the current group is accessible via caller keyword) 
@endcode

@section variableManipulation Variable manipulation
Some simple mathematical operators can be used: +, -, /, * with parenthesis or not.

E.g.:
@code
mavar =(5*mavar+1)/3;
@endcode
Note that the negative operator should be used with caution, YOU MUST add a
space after the "-" symbol.

You can do comparison with variables and or numbers (can be used for if and
while statements). Note that you can't do mathematical computation inside the
if parameter (you CANNOT write if (x+10 < 20)).
@code
mavar = 10;
if(mavar < 10)
{
  ...
}
@endcode
In the above code we could replace @e caller by @e parent which is another
keyword that indicate the parent of a machine state.

The parent/children relation is defined for dynamic spawned groups (see below
native functions). 

@section codeFlowStructure Code flow structure
@subsection codeFlowStructureClassic Classical code flow constructs
@e while, @e if, @e else can be used as in C.

@subsection codeFlowStructureOnChildren onchildren
@e onchildren special construct allow you to execute a script block on all the
child of the current group.
@code
onchildren()
{
  ...
}
@endcode

@subsection codeFlowStructureOnChildren random
@e random special construct allow you to execute randomly a block of code among
several. One instruction in the block following rand keyword is chosen and
executed. This instruction can be a block, in which case the whole block is
executed.
@code
random()
{
  block1();
  { // Block2 begin
    print("foo");
  } // Block2 end
  (bar)block3(baf);
}
@endcode

@section nativeFunctionCalls Native function calls
Native calls are available too. Native calls offer hard coded services to the
scripter.

Native calls have input and output parameters (AKA parameters and return
values).
@code
(destVar1,destVar2)nativeMethod(ParamVar1);
@endcode
This line calls the function nativeMethod for the current group. The left
parenthesis are used to define output arguments, the right for input
arguments.

@section nativeFunctionList Quick reference list of native functions

@subsection nativeFunctionsSpawn Spawn/despawn

- @ref spawn__
- @ref despawn_f_

@subsection nativeFunctionsTriggers Event handler creation

- @ref addHpUpTrigger_ff_
- @ref addHpDownTrigger_ff_
- @ref delHpUpTrigger_ff_
- @ref delHpDownTrigger_ff_
- @ref addHpUpTrigger_fs_
- @ref addHpDownTrigger_fs_
- @ref delHpUpTrigger_fs_
- @ref delHpDownTrigger_fs_
- @ref addNamedEntityListener_ssf_
- @ref delNamedEntityListener_ssf_
- @ref addNamedEntityListener_sss_
- @ref delNamedEntityListener_sss_

@subsection nativeFunctionsActions Actions

- @ref setEvent_f_
- @ref getEventParam_f_f
- @ref getEventParam_f_s
- @ref setTimer_ff_
- @ref timerSetRyzomDaytime_fff_
- @ref timerDisable_f_
- @ref timerSuspend_f_
- @ref timerResume_f_
- @ref timerAdd_ff_
- @ref timerIsEnabled_f_f
- @ref timerIsSuspended_f_f
- @ref dssStartAct_ff_
- @ref postNextState_s_
- @ref moveToZone_ss_
- @ref waitInZone_s_
- @ref stopMoving__
- @ref followPlayer_sf_
- @ref wander__
- @ref downScaleHP_f_
- @ref upScaleHP_f_
- @ref scaleHP_f_
- @ref setMaxHP_ff_
- @ref setHPLevel_f_
- @ref addHP_f_
- @ref aiAction_s_
- @ref aiActionSelf_s_
- @ref setPlayerController_ss_
- @ref clearPlayerController_s_
- @ref activateEasterEgg_fffsfff_
- @ref deactivateEasterEgg_fff_
- @ref receiveMissionItems_ssc_
- @ref giveMissionItems_ssc_
- @ref talkTo_sc_
- @ref giveRewardTo_sc_

@subsection nativeFunctionsGroupCreation Group creation

- @ref newNpcChildGroup_sss_
- @ref newNpcChildGroup_sssf_
- @ref newNpcChildGroupPos_ssff_
- @ref newNpcChildGroupPos_ssfff_
- @ref newNpcChildGroupMl_sssf_
- @ref newNpcChildGroupMl_sssff_
- @ref newNpcChildGroupPosMl_ssfff_
- @ref newNpcChildGroupPosMl_ssffff_

@subsection nativeFunctionsPrimitiveLookup Primitive lookup

- @ref getGroupTemplateWithFlags_ss_s
- @ref getGroupTemplateWithFlags_sss_s
- @ref getZoneWithFlags_sss_s
- @ref getZoneWithFlags_ssss_s
- @ref getNeighbourZoneWithFlags_sss_s
- @ref getNeighbourZoneWithFlags_ssss_s
- @ref getNearestZoneWithFlags_ffss_s
- @ref getNearestZoneWithFlags_ffsss_s
	
@subsection nativeFunctionsGroupAccessors Group parameters

- @ref getMidPos__ff
- @ref getStateName__s
- @ref setActivity_s_
- @ref setDynEnergy_sff_
- @ref copyDynEnergy_sff_
- @ref setAggro_ff_
- @ref setCanAggro_f_
- @ref clearAggroList_f_
- @ref clearAggroList__
- @ref setAttackable_f_
- @ref setPlayerAttackable_f_
- @ref setBotAttackable_f_
- @ref setFactionAttackableAbove_sff_
- @ref setFactionAttackableBelow_sff_
- @ref setMode_s_
- @ref setDespawnTime_f_
- @ref setRespawnTime_f_
- @ref setAutoSpawn_f_
- @ref setFactionProp_ss_
- @ref addProfileParameter_s_
- @ref addProfileParameter_ss_
- @ref addProfileParameter_sf_
- @ref removeProfileParameter_s_
- @ref addBotChat_s_
- @ref clearBotChat__

@subsection nativeFunctionsPhraseManagement Phrase management

- @ref setSimplePhrase_ss_

@subsection nativeFunctionsPersistency Persistency functions

- @ref dataGetVar_s_s
- @ref dataGetVar_s_f
- @ref dataSetVar_ss_
- @ref dataSetVar_sf_
- @ref dataSave__

@subsection  nativeFunctionBoss Boss Functions

@subsubsection nativeFunctionsBossText Sending Text/Emote/System with parameters
- @ref  phraseBegin__
- @ref  phrasePushValue_sf_
- @ref  phrasePushString_ss_
- @ref  phraseEndNpcMsg_fss_
- @ref  phraseEndSystemMsg_fss_
- @ref  phraseEndEmoteMsg_fs_

@subsubsection nativeFunctionsBossTime Knowing Date of the server / Time in game
- @ref  getServerTimeStr__s
- @ref  getServerTime__s
- @ref  getRyzomDateStr__s
- @ref  getRyzomDate__s

@subsubsection nativeFunctionBossPlayer Knowing infos on the player
- @ref isPlayerAlived_s_f
- @ref getPlayerStat_ss_f
- @ref getPlayerDistance_fs_f
- @ref getCurrentPlayerEid__s
- @ref queryEgs_sscfs_
- @ref queryEgs_ssscfs_

@subsubsection nativeFunctionBossBotInfo Infos on bot
- @ref  getBotIndex_s_f
- @ref  getBotEid_f_s
- @ref  getCurrentSpeakerEid__s
- @ref  getBotIndexByName_s_f
- @ref  isGroupAlived__f
- @ref  isBotAlived_f_f

@subsubsection nativeFunctionBossAggro Aggro functions
- @ref getCurrentPlayerAggroListTarget_f_s
- @ref getRandomPlayerAggroListTarget_f_s
- @ref getAggroListElement_ff_s
- @ref getAggroListSize_f_f
- @ref setAggroListTarget_fs_
- @ref setGroupAggroListTarget_s_
- @ref setManagerAggroListTarget_ss_
- @ref setAggro_ff_
- @ref setCanAggro_f_
- @ref clearAggroList__


@subsubsection nativeFunctionTeleport Teleport Functions
- @ref teleportPlayer_sffff_
- @ref summonPlayer_fs_

@subsection nativeFunctionsMisc Misc function (not related to current group)

@subsubsection nativeFunctionsMiscMath Math functions

- @ref clamp_fff_f
- @ref min_ff_f
- @ref max_ff_f
- @ref rndm_ff_f
- @ref floor_f_f
- @ref ceil_f_f
- @ref round_f_f
- @ref abs_f_f
- @ref sin_f_f
- @ref asin_f_f
- @ref sinh_f_f
- @ref cos_f_f
- @ref acos_f_f
- @ref cosh_f_f
- @ref tan_f_f
- @ref atan_f_f
- @ref tanh_f_f
- @ref sqrt_f_f
- @ref exp_f_f
- @ref pow_ff_f

@subsubsection nativeFunctionsMiscString String functions

- @ref strlen_s_f
- @ref substr_sff_s
- @ref strtof_s_f
- @ref strtof_s_ff
- @ref strtof_s_fff
- @ref import_s_

@subsubsection nativeFunctionsMiscNelVar NeL variables management

- @ref setNelVar_sf_
- @ref getNelVar_s_f
- @ref delNelVar_sf_
- @ref setNelVar_ss_
- @ref getNelVar_s_s
- @ref delNelVar_ss_

@subsubsection nativeFunctionsMiscNamedEntities Named entities management

- @ref createNamedEntity_s_
- @ref setNamedEntityProp_sss_
- @ref setNamedEntityPropCb_sss_
- @ref getNamedEntityProp_ss_s
- @ref destroyNamedEntity_s_

@subsubsection nativeFunctionsMiscManagers Foreign managers access

- @ref spawnManager_s_
- @ref despawnManager_s_

@section nativeFunctions Detailed documentation of native functions

*/

typedef std::map<std::string, FScrptNativeFunc> TNativeFuncContainer;
extern TNativeFuncContainer nfGetStaticNativeFunctions();
extern TNativeFuncContainer nfGetStateInstanceNativeFunctions();
extern TNativeFuncContainer nfGetGroupNativeFunctions();
extern TNativeFuncContainer nfGetNpcGroupNativeFunctions();

//////////////////////////////////////////////////////////////////////////////
//	Register List

void AICOMP::CCompiler::registerNativeFunc()
{
	TNativeFuncContainer functions;

	functions = nfGetStaticNativeFunctions();
	FOREACHC(itFunction, TNativeFuncContainer, functions)
		addNativeFunc(itFunction->first, itFunction->second);
	
	functions = nfGetStateInstanceNativeFunctions();
	FOREACHC(itFunction, TNativeFuncContainer, functions)
		addNativeFunc(itFunction->first, itFunction->second);
	
	functions = nfGetGroupNativeFunctions();
	FOREACHC(itFunction, TNativeFuncContainer, functions)
		addNativeFunc(itFunction->first, itFunction->second);
	
	functions = nfGetNpcGroupNativeFunctions();
	FOREACHC(itFunction, TNativeFuncContainer, functions)
		addNativeFunc(itFunction->first, itFunction->second);
	
	// Deprecated functions
	addDeprecatedNativeFunc("spawnSpecialGroup_ffsfs_"); // x, y, sheet, count, stateMachine.
}

/*****************************************************************************
** Old function list (for security purpose)                                 **

spawn__
despawn_f_
	
setEvent_f_
	
newNpcChildGroup_sss_
newNpcChildGroup_sssf_
	
newNpcChildGroupPos_ssff_
newNpcChildGroupPos_ssfff_
	
newNpcChildGroupMl_sssf_
newNpcChildGroupMl_sssff_
	
newNpcChildGroupPosMl_ssfff_
newNpcChildGroupPosMl_ssffff_
	
getMidPos__ff
	
getStateName__s
setTimer_ff_
postNextState_s_
	
spawnManager_s_
despawnManager_s_
	
getGroupTemplateWithFlags_ss_s
getGroupTemplateWithFlags_sss_s
	
getZoneWithFlags_sss_s
getZoneWithFlags_ssss_s
getNeighbourZoneWithFlags_sss_s
getNeighbourZoneWithFlags_ssss_s
getNearestZoneWithFlags_ffss_s
getNearestZoneWithFlags_ffsss_s
	
moveToZone_ss_
waitInZone_s_
	
setActivity_s_
	
setDynEnergy_sff_
copyDynEnergy_sff_
	
setAgro_ff_
setAggro_ff_
setAttackable_f_
setPlayerAttackable_f_
setBotAttackable_f_
setFactionAttackableAbove_sff_
setFactionAttackableBelow_sff_
	
setMode_s_
	
setDespawnTime_f_
setRespawnTime_f_
setAutoSpawn_f_
	
setFactionProp_ss_
	
downScaleHP_f_
upScaleHP_f_
scaleHP_f_
setHPLevel_f_
addHP_f_
	
clamp_fff_f
min_ff_f
max_ff_f
rndm_ff_f
	
floor_f_f
ceil_f_f
round_f_f
abs_f_f
sin_f_f
asin_f_f
sinh_f_f
cos_f_f
acos_f_f
cosh_f_f
tan_f_f
atan_f_f
tanh_f_f
sqrt_f_f
exp_f_f
pow_ff_f
	
strlen_s_f
substr_sff_s
strtof_s_f
strtof_s_ff
strtof_s_fff
	
import_s_
loadFile_s_
break__
groupName_c_s
	
addHpUpTrigger_ff_
addHpDownTrigger_ff_
delHpUpTrigger_ff_
delHpDownTrigger_ff_
addHpUpTrigger_fs_
addHpDownTrigger_fs_
delHpUpTrigger_fs_
delHpDownTrigger_fs_
	
aiAction_s_
aiActionSelf_s_
	
setNelVar_sf_
getNelVar_s_f
delNelVar_sf_
setNelVar_ss_
getNelVar_s_s
delNelVar_ss_
	
createNamedEntity_s_
setNamedEntityProp_sss_
setNamedEntityPropCb_sss_
getNamedEntityProp_ss_s
destroyNamedEntity_s_
addNamedEntityListener_ssf_
delNamedEntityListener_ssf_
addNamedEntityListener_sss_
delNamedEntityListener_sss_
	
addBotChat_s_
clearBotChat__
setSimplePhrase_ss_
	
addProfileParameter_s_
addProfileParameter_ss_
addProfileParameter_sf_
removeProfileParameter_s_
	
ignoreOffensiveActions_f_
	
getOutpostStateName__s
isOutpostTribeOwner__f
isOutpostGuildOwner__f
	
dataGetVar_s_s
dataGetVar_s_f
dataSetVar_ss_
dataSetVar_sf_
dataSave__
	
setHealer_f_
setZoneState_sf_
	
getEventParam_f_s
getEventParam_f_f
	
setSheet_s_
*****************************************************************************/


//----------------------------------------------------------------------------
// setActivityVa
// Arguments: v(Activity,...) ->
// (null), bandit, escorted, guard, guard_escorted, normal, no_change
/*
void	setActivityVa	(CStateInstance	*entity, CScriptStack	&stack)
{
	string inSig = stack.top();
	stack.pop();
	string outSig = stack.top();
	stack.pop();
	
	// Pop input args
	std::deque<size_t> params;
	for (string::size_type i=0; i<inSig.length(); ++i)
	{
		params.push_front(stack.top());
		stack.pop();
	}
	vector<size_t> inParams(params.begin(), params.end());
	params.clear();
	vector<size_t> outParams(pOutSig->length());
	
//////////////////////////////////////////////////////////////////////////////
	
	// Content
	
//////////////////////////////////////////////////////////////////////////////
	
	// Push output args
	params.assign(outParams.begin(), outParams.end());
	for (string::size_type i=0; i<outSig.length(); ++i)
	{
		stack.push(params.front());
		params.pop_front();
	}
}
*/

#include "event_reaction_include.h"
