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

// Documentation:
// https://wiki.ryzom.dev/world_editor/ai_script
// https://wiki.ryzom.dev/world_editor/ai_script/functions

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
