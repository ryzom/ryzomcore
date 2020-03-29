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



#ifndef RYAI_AIS_ACTIONS_H
#define RYAI_AIS_ACTIONS_H

#include "ai_share/ai_actions.h"
#include "ai_place.h"

#include "ai_mgr.h"
#include "ai_mgr_fauna.h"
#include "ai_mgr_npc.h"

#include "ai_grp.h"
#include "ai_grp_fauna.h"
#include "ai_grp_npc.h"

#include "ai_bot.h"
#include "ai_bot_fauna.h"
#include "ai_bot_npc.h"
#include "profile.h"

#include "state_chat.h"

#include "event_reaction_container.h"
#include "continent.h"

namespace CAISActionEnums
{
	//----------------------------------------------------------------------------
	// contexts for the command set
	enum	TContext
	{
		ContextGlobal,
		BaseContextMgr,

		ContextFaunaMgr,
		ContextFaunaGrp,

		ContextNpcMgr,
		ContextNpcMgrKaravan,
		ContextNpcMgrTribe,

		ContextNpcGrp,
		ContextNpcGrpKaravan,

		ContextNpcBot,

		ContextEventContainer,		
		
		BaseContextState,
		ContextPositionalState,
		ContextPunctualState,

		ContextStateProfile,
		ContextStateChat,

		ContextContinent,
		ContextRegion,
		ContextGroupFamily,
		ContextCellZone,
		ContextCell,
		ContextFaunaZone,
		ContextNpcZone,
		ContextRoad,
		ContextRoadTrigger,
		ContextGroupDesc,
		ContextBotDesc,

		ContextSquadTemplate,
		ContextSquadTemplateVariant,
		
		ContextOutpost,
		ContextOutpostCharge,
		ContextOutpostSquadFamily,
		ContextOutpostGroupDesc,
		ContextOutpostBotDesc,

		ContextNumContexts
	};

	inline	TContext	parentContext(TContext context)
	{
		#define	NO_PARENT return ContextNumContexts;
		switch(context)
		{
			case ContextGlobal:				NO_PARENT
			case ContextEventContainer:		NO_PARENT
			case BaseContextMgr:			return ContextEventContainer;
			case ContextFaunaMgr:			return BaseContextMgr;
			case ContextNpcMgr:				return BaseContextMgr;
			case ContextNpcMgrKaravan:		return ContextNpcMgr;
			case ContextNpcMgrTribe:		return ContextNpcMgr;
			case ContextFaunaGrp:			NO_PARENT
			case ContextNpcGrp:				NO_PARENT
			case ContextNpcGrpKaravan:		return ContextNpcGrp;
			case ContextNpcBot:				NO_PARENT
			case BaseContextState:			NO_PARENT
			case ContextPositionalState:	return BaseContextState;
			case ContextPunctualState:		return BaseContextState;
			case ContextStateProfile:		NO_PARENT
			case ContextStateChat:			NO_PARENT
			case ContextContinent:			NO_PARENT
			case ContextRegion:				return ContextContinent;
			case ContextCellZone:			return ContextRegion;
			case ContextCell:				return ContextCellZone;
			case ContextFaunaZone:			return ContextCell;
			case ContextNpcZone:			return ContextCell;
			case ContextRoad:				return ContextNpcZone;
			case ContextRoadTrigger:		return ContextRoad;
			case ContextGroupDesc:			return ContextGroupFamily;
			case ContextBotDesc:			return ContextGroupDesc;
			case ContextOutpost:			NO_PARENT
			case ContextOutpostCharge:		return ContextOutpost;
			case ContextOutpostSquadFamily:	return ContextOutpost;
			case ContextOutpostGroupDesc:	return ContextOutpostSquadFamily;
			case ContextOutpostBotDesc:		return ContextOutpostGroupDesc;
			case ContextGroupFamily:		return ContextRegion;
					
			default:
				nlwarning("parentContext(context): Unknown context");
				NO_PARENT
		}
		#undef NO_PARENT
	}

}


class CAISActions: public CAIActions::IExecutor
{
public:
	//----------------------------------------------------------------------------
	// init & release
	
	static void init() 
	{
		if (CAISActions::Instance==NULL)
			CAISActions::Instance=new CAISActions;
		CAIActions::init(Instance); 
	}
	static void release()
	{
		CAIActions::release(); 
	}

	
	//----------------------------------------------------------------------------
	// inheritted virtual interface

	virtual void openFile(const std::string &fileName);
	virtual void closeFile(const std::string &fileName);

	virtual void execute(uint64 action,const std::vector <CAIActions::CArg> &args);

	virtual void begin(uint32 contextAlias);
	virtual void end(uint32 contextAlias);



	//----------------------------------------------------------------------------
	// instance data

	// the 'actionMap' (map of action ids to handlers)
	class IActionHandler;
	typedef std::map <uint64,IActionHandler*> TActionMap;
	TActionMap ActionMap[CAISActionEnums::ContextNumContexts];

	//----------------------------------------------------------------------------
	// the actoin handler strucure and a macro for defining handlers easily
	class	IActionHandler
	{
	public:
		IActionHandler(CAISActionEnums::TContext context, const char *name)
		{
			if (CAISActions::Instance==NULL)
				CAISActions::Instance=new CAISActions;

			uint64 id=0;
			uint i;
			for (i=0;i<8 && name[i]!=0;++i)
				((char *)&id)[i]=name[i];

			// the following assert ensures that we never try to cram >8 letters into an int64
			nlassert(name[i]==0);
			CAISActions::Instance->ActionMap[context][id]=this;
		};

		virtual const char *name()=0;
		virtual void operator()(const std::vector <CAIActions::CArg> &args)=0;
	};


	#define DEFINE_ACTION(context,cmdName)													\
		struct CActionHandler_##context##_##cmdName : public CAISActions::IActionHandler	\
		{																					\
			CActionHandler_##context##_##cmdName()											\
			: CAISActions::IActionHandler(CAISActionEnums::context, #cmdName) { }			\
			virtual char const* name() { return #cmdName; }									\
			virtual void operator()(std::vector<CAIActions::CArg> const& args);				\
		};																					\
		static CActionHandler_##context##_##cmdName ActionHandler_##context##_##cmdName;	\
		void CActionHandler_##context##_##cmdName::operator()(std::vector<CAIActions::CArg> const& args)
	
	#define DEFINE_ACTION_TEMPLATE1(context,cmdName,templateTypeName1)						\
		template <typename templateTypeName1>												\
		struct CActionHandler_##context##_##cmdName : public CAISActions::IActionHandler	\
		{																					\
			char const* _Name;																\
			CActionHandler_##context##_##cmdName(char const* name)							\
			: CAISActions::IActionHandler(CAISActionEnums::context, name), _Name(name) { }	\
			virtual char const* name() { return _Name; }									\
			virtual void operator()(std::vector<CAIActions::CArg> const& args);				\
		};																					\
		template <typename templateTypeName1>												\
		void CActionHandler_##context##_##cmdName<templateTypeName1>::operator()(std::vector<CAIActions::CArg> const& args)
	// cmdName + typeId should not exceed 8 characters long
	#define DEFINE_ACTION_TEMPLATE1_INSTANCE(context,cmdName,typeId,typeName1)						\
		static CActionHandler_##context##_##cmdName<typeName1> ActionHandler_##context##_##cmdName(#cmdName#typeId)
	
	//----------------------------------------------------------------------------
	// the singleton class instance
	static CAISActions	*Instance;
};


//----------------------------------------------------------------------------
// The context stack
//----------------------------------------------------------------------------

class CContextStack
{
public:
	static void init()
	{
		if (!initialised())
			Stack.push_back(CAISActionEnums::ContextGlobal);
	}

	static void push(CAISActionEnums::TContext context)
	{
		if (!initialised()) init();
		Stack.push_back(context);
	}

	static CAISActionEnums::TContext topContext()
	{
		if (!initialised()) init();
		return context(size()-1);
	}

	static CAISActionEnums::TContext context(uint i)
	{
		if (!initialised()) init();
		nlassert(i<size());
		return Stack[i];
	}

	static void setContext(CAISActionEnums::TContext context)
	{
		nlassert(initialised());
		nlassert(!Stack.empty());

		Stack.back() = context;
	}

	static uint size()
	{
		if (!initialised()) init();
		return (uint)Stack.size();
	}

	static void pop()
	{
		nlassert(Stack.size()>1);
		Stack.pop_back();
	}

private:
	static bool	initialised()
	{
		return !Stack.empty();
	}

	static std::vector<CAISActionEnums::TContext> Stack;
};

//----------------------------------------------------------------------------
// Pointers to managers, groups etc	currently being worked on
//----------------------------------------------------------------------------

class CWorkPtr
{
public:
	static void					aiInstance(CAIInstance *instance)	{ Instance=instance;	}
	static CAIInstance			*aiInstance()						{ nlassert(Instance); return Instance;	}
	
	static void					mgr(CManager *m)					{ Mgr=m; }
	static CManager				*mgr()								{ return Mgr; }

	static CMgrFauna			*mgrFauna()							{ return dynamic_cast<CMgrFauna			*>(Mgr); }
	static CMgrNpc				*mgrNpc()							{ return dynamic_cast<CMgrNpc			*>(Mgr); }
	static COutpostManager		*mgrOutpost()						{ return dynamic_cast<COutpostManager	*>(Mgr); }

	static void					grp(CGroup*	g)						{ Grp=g; }
	static CGroup				*grp()								{ return Grp; }
	static CGrpFauna			*grpFauna()							{ return dynamic_cast<CGrpFauna	*>(Grp); }
	static CGroupNpc			*grpNpc()							{ return dynamic_cast<CGroupNpc	*>(Grp); }
	
	static void					bot(CBot *b)						{ Bot=b; }
	static CBot					*bot()								{ return Bot; }							    
	static CBotFauna			*botFauna()							{ return dynamic_cast<CBotFauna	*>(Bot); }
	static CBotNpc				*botNpc()							{ return dynamic_cast<CBotNpc	*>(Bot); }

	static void					stateState(CAIState	*s)				{ State=s; }
	static CAIState				*stateState()						{ return State; }
	
	static	void				eventReactionContainer(CStateMachine	*eventContainer)	{	EventContainer=eventContainer;	}
	static	CStateMachine		*eventReactionContainer()		{	return	EventContainer;	}	
	

	static void					stateProfile(CAIStateProfile *p)	{ Profile=p; }
	static CAIStateProfile		*stateProfile()						{ return Profile; }

	static void					stateChat(CAIStateChat *p)			{ Chat=p; }
	static CAIStateChat			*stateChat()						{ return Chat; }

	static void					continent(CContinent *c)			{ Continent = c;}
	static CContinent			*continent()						{ return Continent;}
	static void					region(CRegion *r)					{ Region = r;}
	static CRegion				*region()							{ return Region;}

	static void					groupFamily(CGroupFamily *gf)		{ GroupFamily = gf;}
	static CGroupFamily			*groupFamily()						{ return GroupFamily;}
	

	static void					cellZone(CCellZone *c)				{ CellZone = c;}
	static CCellZone			*cellZone()							{ return CellZone;}
	static void					cell(CCell *c)						{ Cell = c;}
	static CCell				*cell()								{ return Cell;}
	static void					faunaZone(CFaunaZone *z)			{ FaunaZone = z;}
	static CFaunaZone			*faunaZone()						{ return FaunaZone;}
	static void					npcZone(CNpcZone *z)				{ NpcZone = z;}
	static CNpcZone				*npcZone()							{ return NpcZone;}
	static void					road(CRoad *r)						{ Road = r;}
	static CRoad				*road()								{ return Road;}
	static void					roadTrigger(CRoadTrigger *t)		{ RoadTrigger = t;}
	static CRoadTrigger			*roadTrigger()						{ return RoadTrigger;}
	/*
	static void						groupDesc(CGroupDesc<CGroupFamily> *d)	{ GroupDesc = d;}
	static CGroupDesc<CGroupFamily>	*groupDesc()							{ return GroupDesc;}
	static void						botDesc(CBotDesc<CGroupFamily> *d)		{ BotDesc = d;}
	static CBotDesc<CGroupFamily>	*botDesc()								{ return BotDesc;}
	*/

	static void					outpost(COutpost *o)				{ Outpost = o;}
	static COutpost				*outpost()							{ return Outpost;}
//	static void					outpostCharge(COutpostCharge *oc)	{ OutpostCharge = oc;}
//	static COutpostCharge		*outpostCharge()					{ return OutpostCharge;}
	static void					squadVariantName(const std::string& n)	{ SquadVariantName = n;}
	static const std::string&	squadVariantName()					{ return SquadVariantName;}
	/*
	static void								outpostGroupDesc(CGroupDesc<COutpostSquadFamily> *d)	{ OutpostGroupDesc = d;}
	static CGroupDesc<COutpostSquadFamily>	*outpostGroupDesc()										{ return OutpostGroupDesc;}
	static void								outpostBotDesc(CBotDesc<COutpostSquadFamily> *d)		{ OutpostBotDesc = d;}
	static CBotDesc<COutpostSquadFamily>	*outpostBotDesc()										{ return OutpostBotDesc;}
	*/
	static void					groupDesc(IGroupDesc *d)	{ GroupDesc = d;}
	static IGroupDesc			*groupDesc()				{ return GroupDesc;}
	static void					botDesc(IBotDesc *d)		{ BotDesc = d;}
	static IBotDesc				*botDesc()					{ return BotDesc;}
	
	static void					addLogicAction(IAILogicAction *action, uint32 alias) { _LogicActionMap[alias] = action; }
	// retrive logic action from alias
	static IAILogicAction		*getLogicActionFromAlias(uint32 alias) { return _LogicActionMap.count(alias) ? _LogicActionMap[alias] : NULL; }
	// clear logic action map
	static void					clearLogicActionMap() { _LogicActionMap.clear(); }
	// current logic action
	static void                 logicAction(IAILogicAction *action) { _LogicAction = action; }
	static IAILogicAction		*logicAction() { return _LogicAction; }
public:
	static IAILogicAction					  *_LogicAction;
	static std::map<uint32, IAILogicAction *> _LogicActionMap;
	static CAIInstance*	Instance;
	static CManager*	Mgr;
	static CGroup*		Grp;
	static CBot*		Bot;

	static CAIState*	State;	
	static CAIStateProfile*	 Profile;
	static CAIStateChat*	 Chat;

	static CStateMachine*	EventContainer;

	static CContinent*		Continent;
	static CRegion*			Region;
	static CGroupFamily*	GroupFamily;
	static CCellZone*		CellZone;
	static CCell*			Cell;
	static CFaunaZone*		FaunaZone;
	static CNpcZone*		NpcZone;
	static CRoad*			Road;
	static CRoadTrigger*	RoadTrigger;
//	static CGroupDesc<CGroupFamily>*	GroupDesc;
//	static CBotDesc<CGroupFamily>*		BotDesc;

	static COutpost*		Outpost;
//	static COutpostCharge*	OutpostCharge;
	static std::string		SquadVariantName;
//	static CGroupDesc<COutpostSquadFamily>*	OutpostGroupDesc;
//	static CBotDesc<COutpostSquadFamily>*	OutpostBotDesc;
	static IGroupDesc*		GroupDesc;
	static IBotDesc*		BotDesc;
	
	CWorkPtr()
	{
		_Instance	=	Instance;
		_Mgr	=Mgr;
		_Grp	=Grp;
		_Bot	=Bot;
		_State	=State;
		_Profile=Profile;
		_Chat	=Chat;
		_EventContainer=EventContainer;
		_EventReaction = NULL;
	}

	virtual ~CWorkPtr()
	{
		Instance	=	_Instance;
		Mgr		=_Mgr;
		Grp		=_Grp;
		Bot		=_Bot;
		State	=_State;		
		Profile	=_Profile;
		Chat	=_Chat;
		EventContainer=_EventContainer;
	}

private:
	// the structure is only ever instantiated  for context save purposes
	CAIInstance*	_Instance;
	CManager*		_Mgr;
	CGroup*			_Grp;
	CBot*			_Bot;

	CAIState*						_State;
	CAIEventReaction*				_EventReaction;
	CAIStateProfile*				_Profile;
	CAIStateChat*					_Chat;

	CStateMachine*	_EventContainer;
};


//----------------------------------------------------------------------------
// Local routines for the action handlers to use
//----------------------------------------------------------------------------

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
inline bool getArgs(const std::vector <CAIActions::CArg> &args,
					const char *actionName,T0 &v0,T1 &v1,T2 &v2,T3 &v3,T4 &v4,T5 &v5,T6 &v6)
{
	if (args.size()!=7)	 { nlwarning("Action %s FAILED due to argument count",actionName); return false; }
	if (!args[0].get(v0)){ nlwarning("Action %s FAILED due to type of argument 0",actionName); return false; } 
	if (!args[1].get(v1)){ nlwarning("Action %s FAILED due to type of argument 1",actionName); return false; } 
	if (!args[2].get(v2)){ nlwarning("Action %s FAILED due to type of argument 2",actionName); return false; } 
	if (!args[3].get(v3)){ nlwarning("Action %s FAILED due to type of argument 3",actionName); return false; } 
	if (!args[4].get(v4)){ nlwarning("Action %s FAILED due to type of argument 4",actionName); return false; } 
	if (!args[5].get(v5)){ nlwarning("Action %s FAILED due to type of argument 5",actionName); return false; } 
	if (!args[6].get(v6)){ nlwarning("Action %s FAILED due to type of argument 6",actionName); return false; } 
	return true;
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
inline bool getArgs(const std::vector <CAIActions::CArg> &args,
					const char *actionName,T0 &v0,T1 &v1,T2 &v2,T3 &v3,T4 &v4,T5 &v5)
{
	if (args.size()!=6)	 { nlwarning("Action %s FAILED due to argument count",actionName); return false; }
	if (!args[0].get(v0)){ nlwarning("Action %s FAILED due to type of argument 0",actionName); return false; } 
	if (!args[1].get(v1)){ nlwarning("Action %s FAILED due to type of argument 1",actionName); return false; } 
	if (!args[2].get(v2)){ nlwarning("Action %s FAILED due to type of argument 2",actionName); return false; } 
	if (!args[3].get(v3)){ nlwarning("Action %s FAILED due to type of argument 3",actionName); return false; } 
	if (!args[4].get(v4)){ nlwarning("Action %s FAILED due to type of argument 4",actionName); return false; } 
	if (!args[5].get(v5)){ nlwarning("Action %s FAILED due to type of argument 5",actionName); return false; } 
	return true;
}

template <class T0, class T1, class T2, class T3, class T4>
inline bool getArgs(const std::vector <CAIActions::CArg> &args,
					const char *actionName,T0 &v0,T1 &v1,T2 &v2,T3 &v3,T4 &v4)
{
	if (args.size()!=5)	 { nlwarning("Action %s FAILED due to argument count",actionName); return false; }
	if (!args[0].get(v0)){ nlwarning("Action %s FAILED due to type of argument 0",actionName); return false; } 
	if (!args[1].get(v1)){ nlwarning("Action %s FAILED due to type of argument 1",actionName); return false; } 
	if (!args[2].get(v2)){ nlwarning("Action %s FAILED due to type of argument 2",actionName); return false; } 
	if (!args[3].get(v3)){ nlwarning("Action %s FAILED due to type of argument 3",actionName); return false; } 
	if (!args[4].get(v4)){ nlwarning("Action %s FAILED due to type of argument 4",actionName); return false; } 
	return true;
}

template <class T0, class T1, class T2, class T3>
inline bool getArgs(const std::vector <CAIActions::CArg> &args,
					const char *actionName,T0 &v0,T1 &v1,T2 &v2,T3 &v3)
{
	if (args.size()!=4)	 { nlwarning("Action %s FAILED due to argument count",actionName); return false; }
	if (!args[0].get(v0)){ nlwarning("Action %s FAILED due to type of argument 0",actionName); return false; } 
	if (!args[1].get(v1)){ nlwarning("Action %s FAILED due to type of argument 1",actionName); return false; } 
	if (!args[2].get(v2)){ nlwarning("Action %s FAILED due to type of argument 2",actionName); return false; } 
	if (!args[3].get(v3)){ nlwarning("Action %s FAILED due to type of argument 3",actionName); return false; } 
	return true;
}

template <class T0, class T1, class T2>
inline bool getArgs(const std::vector <CAIActions::CArg> &args,
					const char *actionName,T0 &v0,T1 &v1,T2 &v2)
{
	if (args.size()!=3)	 { nlwarning("Action %s FAILED due to argument count",actionName); return false; }
	if (!args[0].get(v0)){ nlwarning("Action %s FAILED due to type of argument 0",actionName); return false; } 
	if (!args[1].get(v1)){ nlwarning("Action %s FAILED due to type of argument 1",actionName); return false; } 
	if (!args[2].get(v2)){ nlwarning("Action %s FAILED due to type of argument 2",actionName); return false; } 
	return true;
}

template <class T0, class T1>
inline bool getArgs(const std::vector <CAIActions::CArg> &args,
					const char *actionName,T0 &v0,T1 &v1)
{
	if (args.size()!=2)	 { nlwarning("Action %s FAILED due to argument count",actionName); return false; }
	if (!args[0].get(v0)){ nlwarning("Action %s FAILED due to type of argument 0",actionName); return false; } 
	if (!args[1].get(v1)){ nlwarning("Action %s FAILED due to type of argument 1",actionName); return false; } 
	return true;
}

template <class T0>
inline bool getArgs(const std::vector <CAIActions::CArg> &args,
					const char *actionName,T0 &v0)
{
	if (args.size()!=1)	 { nlwarning("Action %s FAILED due to argument count",actionName); return false; }
	if (!args[0].get(v0)){ nlwarning("Action %s FAILED due to type of argument 0",actionName); return false; } 
	return true;
}

#endif

