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




//----------------------------------------------------------------------------

#include "stdpch.h"
#include "ais_actions.h"
#include "states.h"

#include "dyn_grp_inline.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace CAISActionEnums;
using namespace	AITYPES;


//---------------------------------------------------------------------------------------
// Stuff used for management of log messages

static bool VerboseLog=false;
#define LOG if (!VerboseLog) {} else nlinfo


//----------------------------------------------------------------------------
// Some handy utilities
//----------------------------------------------------------------------------

static CGroupNpc *lookupGrpInMgrNpc(uint32 alias)
{
	if (!CWorkPtr::mgrNpc())
		return NULL;
	return	static_cast<CGroupNpc*>(CWorkPtr::mgrNpc()->groups().getChildByAlias(alias));
}

static CAIState *lookupStateInEventReactionContainer(uint32 alias)
{
	if (!CWorkPtr::eventReactionContainer())
		return NULL;
	return	CWorkPtr::eventReactionContainer()->states().getChildByAlias(alias);
}

static CAIEventReaction *lookupEventInEventReactionContainer(uint32 alias)
{
	if (!CWorkPtr::eventReactionContainer())
		return NULL;
	return	NLMISC::safe_cast<CAIEventReaction*>(CWorkPtr::eventReactionContainer()->eventReactions().getChildByAlias(alias));
}

//static CAIStateProfile	*lookupProfileInState(uint32 alias)
//{
//	if (!CWorkPtr::stateState())
//		return NULL;
//
//
//
//
//	for (CCont<CAIState>::iterator it=.begin(), itEnd=.end();it!=itEnd;++it)
//	{
//		(*it)
//	}
//
//	for (uint i=0;i<CWorkPtr::stateState()->profileCount();++i)
//	{
//		CAIStateProfileNpc *profile=static_cast<CAIStateProfileNpc *>(CWorkPtr::stateState()->getProfile(i));
//		if	(	profile
//			&&	profile->getAlias()==alias)
//			return profile;
//	}
//	return NULL;
//}


/*
static CBotNpc *lookupBotInGrpNpc(uint32 alias)
{
	if (CWorkPtr::grpNpc()==NULL)
		return NULL;

	return	(	(CBotNpc*)	CWorkPtr::grpNpc()->getBotByAlias	(alias)	);
	
//	for (uint i=0;i<CWorkPtr::grpNpc()->botCount();++i)
//		if (CWorkPtr::grpNpc()->getBotNpc(i)->getAlias()==alias)
//			return CWorkPtr::grpNpc()->getBotNpc(i);
	return NULL;
}
*/
//----------------------------------------------------------------------------
// The NPC_MGR context
//----------------------------------------------------------------------------

DEFINE_ACTION(ContextNpcMgr,GRPNPC)
{
	CMgrNpc *mgr=CWorkPtr::mgrNpc();
	if (!mgr) 
		return;

	uint32 alias;
	if (!getArgs(args,name(),alias)) return;

 	LOG("NPC Manager: %s: NPC Group: %u",mgr->getAliasNode()->fullName().c_str(),alias);

	// set workptr::grp to this grp
	CWorkPtr::grp(lookupGrpInMgrNpc(alias));
	if (!CWorkPtr::grpNpc())
	{
		nlwarning("Failed to select grp %s as not found in manager: %s",
			LigoConfig.aliasToString(alias).c_str(),
			CWorkPtr::mgrNpc()->getName().c_str());
		return;
	}

	// if all went to plan setup the 'npc group' context
	CContextStack::setContext(ContextNpcGrp);
}

DEFINE_ACTION(ContextEventContainer,STATE)
{
	CStateMachine *container=CWorkPtr::eventReactionContainer();
	if (!container) 
		return;

	uint32 alias;
	if (!getArgs(args,name(),alias))
		return;

// 	LOG("EventContainer: %s: State (positional): %u",container->getAliasNode()->fullName().c_str(),alias);

	// set workptr::state to this state
	CWorkPtr::stateState(lookupStateInEventReactionContainer(alias));
	if (!CWorkPtr::stateState())
	{
		nlwarning("Failed to select state %s", LigoConfig.aliasToString(alias).c_str());
		return;
	}

	// set workptr state to this state
	CContextStack::setContext(ContextPositionalState);
}

DEFINE_ACTION(ContextEventContainer,EVENT)
{
	CStateMachine *container=CWorkPtr::eventReactionContainer();
	if	(!container)
		return;

	uint32 alias;
	CAIEventDescription::TSmartPtr eventDescription;
	if	(!getArgs(args,name(),alias,eventDescription))
		return;

	CAIEventReaction *event=lookupEventInEventReactionContainer(alias);

	if	(!event)
	{
		nlwarning("Failed to select event %s", LigoConfig.aliasToString(alias).c_str());
		return;
	}

	event->processEventDescription(eventDescription,container);

// 	LOG("EventContainer: %s: Event: %u (%s): %s",container->getAliasNode()->fullName().c_str(),alias,event->getName().c_str(),eventDescription->toString().c_str()); 
//	LOG("Event: %u (%s): %s",alias,event->getName().c_str(),eventDescription->toString().c_str()); 

	// need to extract the entire event from the argument list ...
}


DEFINE_ACTION(ContextEventContainer,SETACTN)
{
	

	uint32 alias;	
	if	(!getArgs(args,name(),alias))
		return;

	IAILogicAction *action = CWorkPtr::getLogicActionFromAlias(alias);

	if	(!action)
	{
		nlwarning("Failed to select logic actions", LigoConfig.aliasToString(alias).c_str());
		CWorkPtr::logicAction(NULL);
		return;
	}
	CWorkPtr::logicAction(action);
}

DEFINE_ACTION(ContextEventContainer,CLRACTN)
{	
	CWorkPtr::logicAction(NULL);
}

DEFINE_ACTION(ContextEventContainer, ENDEVENT)
{	
	CWorkPtr::clearLogicActionMap();
}





DEFINE_ACTION(ContextEventContainer,PUNCTUAL)
{
	CStateMachine *container=CWorkPtr::eventReactionContainer();
	if (!container) 
		return;

	uint32 alias;
	if (!getArgs(args,name(),alias))
		return;

// 	LOG("EventContainer: %s: Punctual state: %u",container->getAliasNode()->fullName().c_str(),alias);
	LOG("Punctual state: %u",alias);

	// set workptr::state to this state
	CWorkPtr::stateState(lookupStateInEventReactionContainer(alias));
	if (!CWorkPtr::stateState())
	{
		nlwarning("Failed to select state %s", LigoConfig.aliasToString(alias).c_str());
		return;
	}

	// set workptr state to this state
	CContextStack::setContext(ContextPunctualState);
}


//----------------------------------------------------------------------------
// The NPC_STATE context(s)
//----------------------------------------------------------------------------

DEFINE_ACTION(BaseContextState,KEYWORDS)
{
	if	(!CWorkPtr::stateState()) 
		return;

	CWorkPtr::stateState()->keywordsClear();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		CKeywordMask mask;
		if (!CAIKeywords::stateMask(s, mask))
		{
			nlwarning("There are some keyword error in '%s'", CWorkPtr::stateState()->getAliasNode()->fullName().c_str());
		}
		CWorkPtr::stateState()->keywordsAdd(mask);

		LOG("State: %s: keywords: %s",CWorkPtr::stateState()->getAliasNode()->fullName().c_str(),s.c_str());
	}
	LOG("State: %s: => %08x",CWorkPtr::stateState()->getAliasNode()->fullName().c_str(),CWorkPtr::stateState()->getKeywords().asUint());
}

DEFINE_ACTION(BaseContextState,PROFILE)
{
	if (!CWorkPtr::stateState()) 
		return;

	uint32 alias;
	if (!getArgs(args,name(),alias)) return;

 	LOG("State: %s: bot profile: %u",CWorkPtr::stateState()->getAliasNode()->fullName().c_str(),alias);

	// set workptr::state to this state
	CWorkPtr::stateProfile(CWorkPtr::stateState()->profiles().getChildByAlias(alias));
	
//	CWorkPtr::stateProfile(lookupProfileInState(alias));
	if (!CWorkPtr::stateProfile())
	{
		nlwarning("Failed to select state profile %s as not found in state: %s",
			LigoConfig.aliasToString(alias).c_str(),
			CWorkPtr::stateState()->getName().c_str());
		return;
	}

	// set workptr state to this state
	CContextStack::setContext(ContextStateProfile);
}

DEFINE_ACTION(BaseContextState,CHAT)
{
	if (!CWorkPtr::stateState())
		return;
	
	uint32 alias;
	if (!getArgs(args,name(),alias)) return;
	
	LOG("State: %s: bot chat: %u",CWorkPtr::stateState()->getAliasNode()->fullName().c_str(),alias);
	
	// set workptr::state to this state
	CWorkPtr::stateChat(CWorkPtr::stateState()->chats().getChildByAlias(alias));
	
	//	CWorkPtr::stateProfile(lookupProfileInState(alias));
	if (!CWorkPtr::stateChat())
	{
		nlwarning("Failed to select state chat %s as not found in state: %s",
			LigoConfig.aliasToString(alias).c_str(),
			CWorkPtr::stateState()->getName().c_str());
		return;
	}
	
	// set workptr state to this state
	CContextStack::setContext(ContextStateChat);
}

DEFINE_ACTION(BaseContextState,MOVEPROF)
{
	if (!CWorkPtr::stateState())
		return;
	
	string aiMovement;
	if (!getArgs(args,name(), aiMovement))
		return;
	
	if (aiMovement.empty())
	{
		// set the default ai_profile
		aiMovement = "no_change";
	}
	
	IAIProfileFactory* aiProfile = lookupAIGrpProfile(aiMovement.c_str());
	if	(!aiProfile)
	{
		nlwarning("Failed to set move profile to '%s' for state '%s' because no corresponding code module found",aiMovement.c_str(),CWorkPtr::stateState()->getName().c_str());
		// drop through and assign 'NULL' to pointer anyway
	}
	
 	LOG("State: %s: move profile: %s",CWorkPtr::stateState()->getAliasNode()->fullName().c_str(),aiMovement.c_str());
	CWorkPtr::stateState()->setMoveProfile(aiProfile);
}

DEFINE_ACTION(BaseContextState,ACTPROF)
{
	if	(!CWorkPtr::stateState()) 
		return;
	
	string aiMovement;
	if	(!getArgs(args,name(),aiMovement))
		return;
	
	if (aiMovement.empty())
	{
		// set the default ai_profile
		aiMovement = "no_change";
	}

	IAIProfileFactory* aiProfile = lookupAIGrpProfile(aiMovement.c_str());
	if	(!aiProfile)
	{
		nlwarning("Failed to set activity profile to '%s' for state '%s' because no corresponding code module found",aiMovement.c_str(),CWorkPtr::stateState()->getName().c_str());
		// drop through and assign 'NULL' to pointer anyway
	}
	
	LOG("State: %s: activity profile : %s",CWorkPtr::stateState()->getAliasNode()->fullName().c_str(),aiMovement.c_str());
	CWorkPtr::stateState()->setActivityProfile(aiProfile);
}

DEFINE_ACTION(BaseContextState,PROFPARM)
{
	if	(!CWorkPtr::stateState()) 
		return;

	vector<string>	params;

	for (uint i=0; i<args.size(); ++i)
	{
		string s;
		args[i].get(s);
		params.push_back(s);
	}

 	LOG("State: %s: profiles parameters : %s%s",CWorkPtr::stateState()->getAliasNode()->fullName().c_str(),
		!params.empty() ? params[0].c_str() : "no parameter",
		params.size() > 1 ? "..." : "");
	CWorkPtr::stateState()->setProfileParameters(params);
}


//----------------------------------------------------------------------------

DEFINE_ACTION(ContextPositionalState,VERTPOS)
{
	CAIStatePositional *state=dynamic_cast<CAIStatePositional *>(CWorkPtr::stateState());
	if (state==NULL || args.empty() ) 
		return;
	uint32	verticalPos;
	getArgs(args, name(), verticalPos);

	state->shape().setVerticalPos((TVerticalPos)verticalPos);
}

DEFINE_ACTION(ContextPositionalState,PATH)
{
	CAIStatePositional *state=dynamic_cast<CAIStatePositional *>(CWorkPtr::stateState());
	if (state==NULL || args.empty() ) 
		return;
	std::vector <CAIVector> points;
	
	LOG("State: %s: set path",state->getAliasNode()->fullName().c_str());

	for (uint i=1;i<args.size();i+=2)
	{
		double x,y;
		args[i-1].get(x);
		args[i].get(y);
		points.push_back(CAIVector(x,y));
		LOG("----  - (%.3f,%.3f)",x,y);
	}
	if (!state->shape().setPath(state->shape().getVerticalPos(), points))
	{
		nlwarning("CAIStatePositional: error while placing the points of '%s'", 
			state->getAliasFullName().c_str());

	}
}

DEFINE_ACTION(ContextPositionalState,PATAT)
{
	CAIStatePositional *state=dynamic_cast<CAIStatePositional *>(CWorkPtr::stateState());
	if (state==NULL || args.empty()) 
		return;

 	LOG("State: %s: set patat",state->getAliasNode()->fullName().c_str());
	std::vector <CAIVector> points;

	for (uint i=1;i<args.size();i+=2)
	{
		double x,y;
		args[i-1].get(x);
		args[i].get(y);
		points.push_back(CAIVector(x,y));
		LOG("----  - (%.3f,%.3f)",x,y);
	}
	if (!state->shape().setPatat(state->shape().getVerticalPos(), points))
	{
		nlwarning("CAIStatePositional: error while placing the points of '%s'", 
			state->getAliasFullName().c_str());
	}
}

//----------------------------------------------------------------------------
// The NPC_STATE_CHAT context
//----------------------------------------------------------------------------

DEFINE_ACTION(ContextStateChat,BOTKEYS)
{
	CAIStateChat	*sc=CWorkPtr::stateChat();
	if (!sc) 
		return;

	sc->botKeywordFilterClear();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		sc->botKeywordFilterAdd(s);
		LOG("State Profile: %s: bot keywords: %s",sc->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextStateChat,BOTNAMES)
{
	CAIStateChat	*sc=CWorkPtr::stateChat();
	if (!sc) 
		return;
	
	sc->botNameFilterClear();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		sc->botNameFilterAdd(s);
		LOG("State Profile: %s: bot name: %s",sc->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextStateChat,CHAT)
{
	CAIStateChat	*sc=CWorkPtr::stateChat();
	if (!sc) 
		return;
	
	sc->chatProfile().clear();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		sc->chatProfile().add(CWorkPtr::aiInstance(), s);
		LOG("State Profile: %s: chat: %s",sc->getAliasNode()->fullName().c_str(),s.c_str());
	}

}

//----------------------------------------------------------------------------
// The NPC_STATE_PROFILE context
//----------------------------------------------------------------------------

DEFINE_ACTION(ContextStateProfile,GRPKEYS)
{
	CAIStateProfile	*sp=CWorkPtr::stateProfile();
	if (!sp) 
		return;

	sp->grpKeywordFilterClear();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		sp->grpKeywordFilterAdd(s);
		LOG("State Profile: %s: grp keywords: %s",sp->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextStateProfile,GRPNAMES)
{
	CAIStateProfile	*sp=CWorkPtr::stateProfile();
	if (!sp) 
		return;

	sp->grpNameFilterClear();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		sp->grpNameFilterAdd(s);
		LOG("State Profile: %s: grp name: %s",sp->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextStateProfile,MOVEPROF)
{
	CAIStateProfile	*sp=CWorkPtr::stateProfile();
	if (!sp) 
		return;

	string type;
	if (!getArgs(args,name(),type)) return;

	IAIProfileFactory* profile = lookupAIGrpProfile(type.c_str());
	if	(!profile)
	{
		nlwarning("Failed to set move profile to '%s' for state '%s' because no corresponding code module found",type.c_str(),sp->getName().c_str());
		// drop through and assign 'NULL' to pointer anyway
	}

	sp->setMoveProfile(profile);
	
 	LOG("State Profile: %s: move profile : %s",sp->getAliasNode()->fullName().c_str(),type.c_str());
}

DEFINE_ACTION(ContextStateProfile,ACTPROF)
{
	CAIStateProfile	*const	sp=CWorkPtr::stateProfile();
	if (!sp) 
		return;
	
	string type;
	if (!getArgs(args,name(),type))
		return;
	
	IAIProfileFactory* profile = NULL;
	if (!type.empty())
	{
		profile = lookupAIGrpProfile(type.c_str());
		if	(!profile)
		{
			nlwarning("Failed to set activity profile to '%s' for state '%s' because no corresponding code module found",type.c_str(),sp->getName().c_str());
			// drop through and assign 'NULL' to pointer anyway
		}
	}
	
	sp->setActivityProfile(profile);
	
	LOG("State Profile: %s:	activity profile : %s",sp->getAliasNode()->fullName().c_str(),type.c_str());
}

DEFINE_ACTION(ContextStateProfile,PROFPARM)
{
	CAIStateProfile	*sp=CWorkPtr::stateProfile();
	if (!sp) 
		return;

	vector<string>	params;

	for (uint i=0; i<args.size(); ++i)
	{
		string s;
		args[i].get(s);
		params.push_back(s);
	}

 	LOG("State: %s: profiles parameters : %s%s",sp->getAliasNode()->fullName().c_str(),
		!params.empty() ? params[0].c_str() : "no parameter",
		params.size() > 1 ? "..." : "");
	sp->setProfileParameters(params);
}


//----------------------------------------------------------------------------
// The NPC_GRP context
//----------------------------------------------------------------------------

DEFINE_ACTION(ContextNpcGrp,AUTOSPWN)
{
	// set the feed and rest times
	// args: float time0, float time1

	if(!CWorkPtr::grpNpc())
		return;
	
	uint32 autoSpawn;
	if (!getArgs(args, name(), autoSpawn))
		return;
	CWorkPtr::grpNpc()->setAutoSpawn(autoSpawn != 0);
	LOG("AutoSpawn : %s", autoSpawn ? "true" : "false");
}


DEFINE_ACTION(ContextNpcGrp,KEYWORDS)
{
	CGroupNpc *grp=CWorkPtr::grpNpc();
	if (grp==NULL) 
		return;

	grp->keywordsClear();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		CKeywordMask mask;
		if (!CAIKeywords::groupMask(s, mask))
		{
			nlwarning("There are some keyword error in '%s'", grp->getAliasNode()->fullName().c_str());
		}
		grp->keywordsAdd(mask);

		LOG("NPC Group: %s: keywords: %s",grp->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextNpcGrp,PARAMETR)
{
	CGroupNpc *grp=CWorkPtr::grpNpc();
	if (grp==NULL) 
		return;

	grp->clearParameters();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		grp->addParameter(s);
		LOG("NPC Group: %s: parameter: %s",grp->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextNpcGrp,BOTCOUNT)
{
	// this is for the counter of automatically generated bots
	// if the counter is '0' then only real bots should be here

	CGroupNpc *grp=CWorkPtr::grpNpc();
	if (!grp) 
		return;

	uint32 count;
	if (!getArgs(args,name(),count))
		return;

	if (count==0)
	{
		grp->setBotsAreNamedFlag();
	}
	else
	{
		grp->clrBotsAreNamedFlag();
	 	LOG("NPC Group: %s: NPC auto generated bot count: %u",grp->getAliasNode()->fullName().c_str(),count);

		// clear any unused bots
		if (grp->bots().size() > count)
			grp->bots().setChildSize(count);

		// add any needed bots with a pseudo alias
		//	Todo remove getAlias()+i (wrong), there is other instances of this bug in the code.
		uint i;
		for (i=grp->bots().size();i<count;++i)
			grp->bots().addChild(new CBotNpc(grp, grp->getAlias()+i, grp->getName()));
	}
}

DEFINE_ACTION(ContextNpcGrp,BOTNPC)
{

	CGroupNpc *grp=CWorkPtr::grpNpc();
	if	(!grp) 
		return;

	if	(grp->botsAreNamed())
	{
		uint32 alias;
		if (!getArgs(args,name(),alias))
			return;

 		LOG("Named NPC Group: %s: NPC bot: %u",grp->getAliasNode()->fullName().c_str(),alias);

		// set workptr::bot to this bot
		
		CWorkPtr::bot(grp->bots().getChildByAlias(alias));	//lookupBotInGrpNpc(alias));
		if (!CWorkPtr::botNpc())
		{
			nlwarning("Failed to select bot %s as not found in group: %s",
				LigoConfig.aliasToString(alias).c_str(),
				CWorkPtr::grpNpc()->getName().c_str());
			return;
		}

	}
	else
	{
		uint32 index;
		if (!getArgs(args,name(),index))
			return;

 		LOG("NPC Group: %s: NPC bot number : %u",grp->getAliasNode()->fullName().c_str(), index);

		// set workptr::bot to this bot
		CWorkPtr::bot(grp->bots()[index]);
		if (!CWorkPtr::botNpc())
		{
			nlwarning("Failed to select bot (index:%u) as not found in group: %s", index, CWorkPtr::grpNpc()->getName().c_str());
			return;
		}

	}
	if	(!(CWorkPtr::botNpc()->getChat().isNull()))
	{
		CWorkPtr::botNpc()->getChat()->clearMissions();
	}
	// set workptr state to this state
	CContextStack::setContext(ContextNpcBot);
}

DEFINE_ACTION(ContextNpcGrp,PROFPARM)
{
	CGroupNpc *grp=CWorkPtr::grpNpc();
	if (grp==NULL) 
		return;

	vector<string>	params;

	for (uint i=0; i<args.size(); ++i)
	{
		string s;
		args[i].get(s);
		params.push_back(s);
	}

 	LOG("Group: %s: profiles parameters : %s%s", grp->getAliasNode()->fullName().c_str(),
		!params.empty() ? params[0].c_str() : "no parameter",
		params.size() > 1 ? "..." : "");
	grp->setProfileParameters(params);
}



//----------------------------------------------------------------------------
// The NPC_BOT context
//----------------------------------------------------------------------------

DEFINE_ACTION(ContextNpcBot,MISSIONS)
{
	CBotNpc *bot=CWorkPtr::botNpc();
	if (bot==NULL)
		return;

	if (args.size() != 2)
	{
		nlwarning("Action MISSIONS : invalid number of argument, execpted 2, received %u", args.size());
		return;
	}

	uint32 alias;
	std::string name;
	args[0].get(alias);
	args[1].get(name);
	if (bot->getChat().isNull())
	{
		bot->newChat();
	}
	bot->getChat()->addMission(alias);

	CWorkPtr::aiInstance()->addMissionInfo(name, alias);
	LOG("Bot: %s: mission: %u",bot->getAliasNode()->fullName().c_str(),alias);
}

DEFINE_ACTION(ContextNpcBot,ISSTUCK)
{
	CBotNpc *bot=CWorkPtr::botNpc();
	if (bot==NULL)
		return;

	uint32	isStuck;
	args[0].get(isStuck);
	
	bot->setStuck(isStuck!=0);
	LOG("Bot: %s: is %sstuck",bot->getAliasNode()->fullName().c_str(),(isStuck!=0)?"":"not ");
}

DEFINE_ACTION(ContextNpcBot,BLDNGBOT)
{
	CBotNpc *bot=CWorkPtr::botNpc();
	if (bot==NULL)
		return;
	
	uint32	isBuildingBot;
	args[0].get(isBuildingBot);
	
	bot->setBuildingBot(isBuildingBot!=0);
	LOG("Bot: %s: is %sbuildingbot",bot->getAliasNode()->fullName().c_str(),(isBuildingBot!=0)?"":"not ");
}

DEFINE_ACTION(ContextNpcBot,KEYWORDS)
{
	CBotNpc *bot=CWorkPtr::botNpc();
	if (bot==NULL) 
		return;

	bot->keywordsClear();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);		
		CKeywordMask mask;
		if (!CAIKeywords::botMask(s, mask))
		{
			nlwarning("There are some keyword error in '%s'", bot->getAliasNode()->fullName().c_str());
		}
		bot->keywordsAdd(mask);
		LOG("Bot: %s: keywords: %s",bot->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextNpcBot,EQUIP)
{
	CBotNpc *bot=CWorkPtr::botNpc();
	if (bot==NULL) 
		return;

	bot->equipmentInit();
	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		bot->equipmentAdd(s);
		LOG("Bot: %s: equipment: %s",bot->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextNpcBot,CHAT)
{
	CBotNpc *bot=CWorkPtr::botNpc();
	if (bot==NULL) 
		return;

	if (!(bot->getChat().isNull()))
	{
		bot->getChat()->clearShopInfo();
	}
	else
	{
		bot->newChat();
	}

	for (uint i=0;i<args.size();++i)
	{
		std::string s;
		args[i].get(s);
		bot->getChat()->add(bot->getAIInstance(), s);
		LOG("Bot: %s: chat: %s",bot->getAliasNode()->fullName().c_str(),s.c_str());
	}
}

DEFINE_ACTION(ContextNpcBot,LOOK)
{
	CBotNpc*bot=CWorkPtr::botNpc();
	if	(!bot)
		return;
	
	// set client look sheet
	// args: string sheet
	std::string sheetName;
	if (!getArgs(args,name(),sheetName))
		return;

//	LOG("Bot: %s: look sheet: %s.creature",bot->getAliasNode()->fullName().c_str(),sheet.c_str());
	NLMISC::CSheetId	sheetId(sheetName+".creature");
	if	(sheetId==CSheetId::Unknown)
	{
		nlwarning("Parsing npc look: Invalid sheet: '%s' in '%s'", 
			sheetName.c_str(), 
			bot->getAliasFullName().c_str());
		bot->grp().bots().removeChildByIndex(bot->getChildIndex());
		CWorkPtr::bot(NULL);
		return;
	}
	AISHEETS::ICreatureCPtr c = AISHEETS::CSheets::getInstance()->lookup(sheetId);
	if (c == NULL)
	{
		nlwarning("Parsing npc look: can't find creature info for sheetId '%s' in '%s'",
			sheetId.toString().c_str(),
			bot->getAliasFullName().c_str());
		bot->grp().bots().removeChildByIndex(bot->getChildIndex());
		CWorkPtr::bot(NULL);
		return;
	}
	bot->setSheet(c);
}

DEFINE_ACTION(ContextNpcBot,STATS)
{
	CBotNpc*bot=CWorkPtr::botNpc();
	if	(bot==NULL)
		return;

	bot->initEnergy	((*NLMISC::safe_cast<CGroupNpc*>(bot->getOwner())).getEnergyCoef());
}

DEFINE_ACTION(ContextNpcBot,STARTPOS)
{
	CBotNpc *bot=CWorkPtr::botNpc();
	if (bot==NULL)
		return;

	// set the bot's start position
	// args: int x, y
	sint32 x,y;
	float theta;
	uint32	i;
	if (!getArgs(args,name(), x, y, theta, i))
		return;

	TVerticalPos verticalPos = (TVerticalPos) i;

	LOG("Bot: %s: startpos: %.3f,%.3f:%s %d",
		bot->getAliasNode()->fullName().c_str(),
		double(x)/1000.0,
		double(y)/1000.0,
		verticalPosToString(verticalPos).c_str(),
		uint32(180.0*theta/3.14159265359+0.5)%360);

	bot->setStartPos(double(x)/1000.0, double(y)/1000.0, theta, TVerticalPos(verticalPos));
}


//---------------------------------------------------------------------------------------
// Control over verbose nature of logging
//---------------------------------------------------------------------------------------

NLMISC_COMMAND(verboseNPCParserLog,"Turn on or off or check the state of verbose .primitive parser logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
		StrToBool	(VerboseLog, args[0]);

	nlinfo("verbose Logging is %s",VerboseLog?"ON":"OFF");
	return true;
}



DEFINE_ACTION(ContextNpcMgr,GRPKAMI)
{
	if (!CWorkPtr::mgrNpc())
		return;
	
	// extract the arguments
	uint32 alias;
	if (!getArgs(args,"ContextNpcMgrKami:GRP <persistent id>",alias))
		return;
	
	CWorkPtr::grp(CWorkPtr::mgrNpc()->groups().getChildByAlias(alias));
	CWorkPtr::stateState(CWorkPtr::mgrNpc()->getStateMachine()->states().getChildByAlias(alias));
	
	// setup the KamiGrp context for adding kamis to the group
	CContextStack::push(ContextNpcGrp);
}


DEFINE_ACTION(ContextNpcMgr,DEPOSIT)
{
	if (!CWorkPtr::mgrNpc())
		return;
	
	// extract the arguments
	uint32 alias;
	if (!getArgs(args,"ContextNpcMgrKami:GRP <persistent id>",alias))
		return;
	
	CWorkPtr::grp(CWorkPtr::mgrNpc()->groups().getChildByAlias(alias));
	CWorkPtr::stateState(CWorkPtr::mgrNpc()->getStateMachine()->states().getChildByAlias(alias));
	
	// setup the KamiGrp context for adding kamis to the group
	CContextStack::push(ContextNpcGrp);
	CWorkPtr::aiInstance()->registerKamiDeposit(alias,CWorkPtr::grpNpc());
}

//---------------------------------------------------------------------------------------

