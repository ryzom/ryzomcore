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



#ifndef RY_MISSION_EVENT_H
#define RY_MISSION_EVENT_H

#include "game_share/skills.h"
#include "mission_manager/ai_alias_translator.h"


/**
* This class is the base class For all events that affect missions
* \author Nicolas Brigand
* \author Nevrax France
* \date 2002
*/
class CMissionEvent
{
	NL_INSTANCE_COUNTER_DECL(CMissionEvent);
public:

	enum TResult
	{
		Nothing,
		Modified,
		StepEnds,
		MissionEnds,
		MissionFailed,
	};
	enum TMissionEventType
	{
		MissionDone,
		GiveItem,
		GiveMoney,
		EnterZone,
		Cast,
		Kill,
		KillPlayer,
		BuyItem,
		SellItem,
		Forage,
		Talk,
		SkillProgress,
		Target,
		Craft,
		Escort,
		AIMsg,
		LootRm,
		LootItem,
		KillGroup,
		EndDynChat,
		Debug,
		ChargePoints,
		OutpostGain,
		AddMission,
		QueueEntryOk,
		QueueExit,
		GroupSpawned,
		GroupDespawned,
		TaggedRingScenario,
		Unknown
	};
	enum TRestriction
	{
		NoRestriction,
		NoSolo,
		NoGroup,
	};
	inline CMissionEvent(TMissionEventType type , const TDataSetRow &targetEntity,TRestriction restriction = NoRestriction)
		:Type(type),TargetEntity(targetEntity),Restriction(restriction){}
	/// type of the event
	TMissionEventType	Type;
	/// entity targeted by the event
	TDataSetRow			TargetEntity;
	/// restriction on the way the event is processed
	TRestriction		Restriction;
	
	/// simulate a mission event described in a script
	static bool simMissionEvent(const std::vector< std::string > & script, NLMISC::CLog & log );
	
protected:
	CMissionEvent(){}
	/// build an event from a script
	virtual bool buildFromScript( const std::vector< std::string > & script,NLMISC::CLog& log ) = 0;
	
private:
	CMissionEvent(const CMissionEvent&);
	CMissionEvent& operator=(const CMissionEvent&);
};



/// "mission done" event
class CMissionEventMissionDone: public CMissionEvent
{
public:
	CMissionEventMissionDone(uint32 alias)
		:CMissionEvent(MissionDone,TDataSetRow()),Mission(alias){}
	uint32 Mission;
protected:
	friend class CMissionEvent;
	CMissionEventMissionDone(){}
	bool buildFromScript( const std::vector< std::string > & script  ,NLMISC::CLog& log);
};

/// "tagged ring scenario" event
class CMissionEventTaggedRingScenarioDone: public CMissionEvent
{
public:
	CMissionEventTaggedRingScenarioDone(const std::string &scenarioTag)
		:	CMissionEvent(TaggedRingScenario,TDataSetRow()),
			ScenarioTag(scenarioTag)
	{}

	std::string ScenarioTag;
protected:
	friend class CMissionEvent;
	CMissionEventTaggedRingScenarioDone(){}
	bool buildFromScript( const std::vector< std::string > & script  ,NLMISC::CLog& log);
};

/// "give item" event
class CMissionEventGiveItem: public CMissionEvent
{
public:
	CMissionEventGiveItem()
		:CMissionEvent(GiveItem,TDataSetRow()){}
	uint32 Quantity;
	uint32 StepIndex;
	
	friend class CMissionEvent;
protected:
	
	bool buildFromScript( const std::vector< std::string > & script  ,NLMISC::CLog& log);
};

/// "give money" event
class CMissionEventGiveMoney: public CMissionEvent
{
public:
	CMissionEventGiveMoney(uint32 amount)
		:CMissionEvent(GiveMoney,TDataSetRow()),Amount(amount){}
	uint32 Amount;
protected:
	friend class CMissionEvent;
	CMissionEventGiveMoney(){}
	bool buildFromScript( const std::vector< std::string > & script  ,NLMISC::CLog& log);
};

/// "enter zone" event (emitted when being in a zone and meeting the contraints, for instance hour constraints)
class CMissionEventVisitPlace: public CMissionEvent
{
public:
	CMissionEventVisitPlace(uint32 placeId)
		:CMissionEvent(EnterZone,TDataSetRow()),PlaceId(placeId){}
	uint32 PlaceId;
protected:
	friend class CMissionEvent;
	CMissionEventVisitPlace(){}
	bool buildFromScript( const std::vector< std::string > & script  ,NLMISC::CLog& log);
};

/// "cast" event
class CMissionEventCast: public CMissionEvent
{
public:
	CMissionEventCast(const std::vector<NLMISC::CSheetId> & bricks)
		:CMissionEvent(Cast,TDataSetRow()),Bricks(bricks){}
	std::vector<NLMISC::CSheetId> Bricks;
protected:
	friend class CMissionEvent;
	CMissionEventCast(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

/// "Kill" event
class CMissionEventKill : public CMissionEvent
{
public:
	CMissionEventKill( const TDataSetRow &targetEntity, CMissionEvent::TRestriction restriction )
		:CMissionEvent(Kill, targetEntity, restriction){}
protected:
	friend class CMissionEvent;
	CMissionEventKill(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
	
};

/// "Kill player" event
class CMissionEventKillPlayer : public CMissionEvent
{
public:
	CMissionEventKillPlayer(const TDataSetRow & victimId)
		:CMissionEvent(KillPlayer, victimId){}
protected:
	friend class CMissionEvent;
	CMissionEventKillPlayer(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

/// "buy" event
class CMissionEventBuyItem: public CMissionEvent
{
public:
	CMissionEventBuyItem(const TDataSetRow & entity,const NLMISC::CSheetId & item,uint32 quantity,uint16 quality)
		:CMissionEvent(BuyItem,entity),Sheet(item),Quantity(quantity),Quality(quality){}
	
	NLMISC::CSheetId	Sheet;
	uint32				Quantity;
	uint16				Quality;
protected:
	friend class CMissionEvent;
	CMissionEventBuyItem(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

/// "sell" event
class CMissionEventSellItem: public CMissionEvent
{
public:
	CMissionEventSellItem(const TDataSetRow & entity,const NLMISC::CSheetId & item, uint32 quantity,uint16 quality)
		:CMissionEvent(SellItem,entity),Sheet(item),Quantity(quantity),Quality(quality){}
	
	NLMISC::CSheetId	Sheet;
	uint32				Quantity;
	uint16				Quality;
protected:
	friend class CMissionEvent;
	CMissionEventSellItem(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

/// "forage" event
class CMissionEventForage: public CMissionEvent
{
public:
	CMissionEventForage(const NLMISC::CSheetId & item,uint32 quantity,uint16 quality)
		:CMissionEvent(Forage,TDataSetRow()),Sheet(item),Quantity(quantity),Quality(quality){}
	
	NLMISC::CSheetId	Sheet;
	uint32				Quantity;
	uint16				Quality;
protected:
	friend class CMissionEvent;
	CMissionEventForage(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

/// "talk" event
class CMissionEventTalk: public CMissionEvent
{
public:
	CMissionEventTalk()
		:CMissionEvent(Talk,TDataSetRow()){}
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log );
};

/// "Skill Progress" event
class CMissionEventSkillProgress : public CMissionEvent
{
public:
	CMissionEventSkillProgress(SKILLS::ESkills skill, uint level)
		:CMissionEvent(SkillProgress,TDataSetRow() ),Skill(skill),Level(level){}
	// true if event lead to success
	SKILLS::ESkills Skill;
	uint			Level;
protected:
	friend class CMissionEvent;
	CMissionEventSkillProgress(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

/// "target" event
class CMissionEventTarget : public CMissionEvent
{
public:
	CMissionEventTarget(const TDataSetRow &targetEntity)
		:CMissionEvent(Target,targetEntity ){}
protected:
	friend class CMissionEvent;
	CMissionEventTarget(){}
	bool buildFromScript( const std::vector< std::string > & script,NLMISC::CLog& log );
};

/// "craft" event
class CMissionEventCraft : public CMissionEvent
{
public:
	CMissionEventCraft(const NLMISC::CSheetId & item,uint32 quantity,uint16 quality)
		:CMissionEvent(Craft,TDataSetRow()),Sheet(item),Quantity(quantity),Quality(quality){}
	
	NLMISC::CSheetId	Sheet;
	uint32				Quantity;
	uint16				Quality;
protected:
	friend class CMissionEvent;
	CMissionEventCraft(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

class CMissionEventEscort : public CMissionEvent
{
public:
	CMissionEventEscort(TAIAlias mission)
		:CMissionEvent(Escort,TDataSetRow()),Mission(mission){}
	TAIAlias Mission;
	
protected:
	CMissionEventEscort(){}
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

class CMissionEventAIMsg : public CMissionEvent
{
public:
	CMissionEventAIMsg(const std::string & msg)
		:CMissionEvent(AIMsg,TDataSetRow()),Msg(msg){}
	std::string Msg;
	
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

class CMissionEventLootItem : public CMissionEvent
{
public:
	CMissionEventLootItem(const NLMISC::CSheetId & item,uint32 quantity,uint16 quality)
		:CMissionEvent(LootItem,TDataSetRow()),Sheet(item),Quantity(quantity),Quality(quality){}
	
	NLMISC::CSheetId	Sheet;
	uint32				Quantity;
	uint16				Quality;
protected:
	friend class CMissionEvent;
	CMissionEventLootItem(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

class CMissionEventLootRm : public CMissionEvent
{
public:
	CMissionEventLootRm(const NLMISC::CSheetId & item,uint32 quantity,uint16 quality)
		:CMissionEvent(LootRm,TDataSetRow()),Sheet(item),Quantity(quantity),Quality(quality){}
	
	NLMISC::CSheetId	Sheet;
	uint32				Quantity;
	uint16				Quality;
protected:
	friend class CMissionEvent;
	CMissionEventLootRm(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

class CMissionEventKillGroup : public CMissionEvent
{
public:
	CMissionEventKillGroup(TAIAlias alias, CMissionEvent::TRestriction restriction)
		:CMissionEvent(KillGroup,TDataSetRow(),restriction),Alias(alias){}
	
	TAIAlias Alias;
protected:
	friend class CMissionEvent;
	CMissionEventKillGroup(){}
	bool buildFromScript( const std::vector< std::string > & script ,NLMISC::CLog& log);
};

class CMissionEventEndDynChat : public CMissionEvent
{
public:
	CMissionEventEndDynChat()
		:CMissionEvent(EndDynChat,TDataSetRow()){}
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};

class CMissionEventDebug : public CMissionEvent
{
public:
	CMissionEventDebug()
		:CMissionEvent(Debug,TDataSetRow()){}
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};


class CMissionEventChargePoints : public CMissionEvent
{
public:
	CMissionEventChargePoints(uint32 points)
		:CMissionEvent(ChargePoints,TDataSetRow()),Points(points){}
	uint32 Points;
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};

class CMissionEventOutpostGain : public CMissionEvent
{
public:
	CMissionEventOutpostGain()
		:CMissionEvent(OutpostGain,TDataSetRow()){}
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};


class CMissionEventAddMission: public CMissionEvent
{
public:
	CMissionEventAddMission( TAIAlias giver, TAIAlias mission, TAIAlias mainMission, bool guild )
		:CMissionEvent(AddMission, TDataSetRow()) ,Giver(giver),Mission(mission),MainMission(mainMission), Guild(guild) {}
	TAIAlias Mission;
	TAIAlias Giver;
	TAIAlias MainMission;
	bool Guild;
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};

class CMissionEventQueueEntryOk: public CMissionEvent
{
public:
	CMissionEventQueueEntryOk()
		:CMissionEvent(QueueEntryOk, TDataSetRow()) {}
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};


class CMissionEventQueueExit: public CMissionEvent
{
public:
	CMissionEventQueueExit()
		:CMissionEvent(QueueExit, TDataSetRow()) {}
protected:
	friend class CMissionEvent;
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};

class CMissionEventGroupSpawned : public CMissionEvent
{
public:
	CMissionEventGroupSpawned(TAIAlias alias)
		:CMissionEvent(GroupSpawned,TDataSetRow()),Alias(alias){}
	
	TAIAlias Alias;
protected:
	friend class CMissionEvent;
	CMissionEventGroupSpawned(){}
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};

class CMissionEventGroupDespawned : public CMissionEvent
{
public:
	CMissionEventGroupDespawned(TAIAlias alias)
		:CMissionEvent(GroupDespawned,TDataSetRow()),Alias(alias){}
	
	TAIAlias Alias;
protected:
	friend class CMissionEvent;
	CMissionEventGroupDespawned(){}
	bool buildFromScript( const std::vector< std::string > & /* script */, NLMISC::CLog& /* log */){return false;}
};

#endif // RY_MISSION_EVENT_H

/* End of mission_event.h */













