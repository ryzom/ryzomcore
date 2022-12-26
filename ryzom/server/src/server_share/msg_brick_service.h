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



#ifndef RY_MSG_BRICK_SERVICE_H
#define RY_MSG_BRICK_SERVICE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"

#include "game_share/synchronised_message.h"
#include "game_share/ryzom_entity_id.h"

#include "game_share/base_types.h"
#include "ai_share/ai_event_report.h"


/**
 * Class CEGSExecutePhraseMsg, message class used by npcs to execute a phrase
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CEGSExecutePhraseMsg : public CMirrorTransportClass
{
public:
	CEGSExecutePhraseMsg	() : CMirrorTransportClass()
	{}

	CEGSExecutePhraseMsg	(const TDataSetRow	&actorRowId, const TDataSetRow	&targetRowId, const NLMISC::CSheetId &phraseId ) :
	CMirrorTransportClass(), ActorRowId(actorRowId), TargetRowId(targetRowId), PhraseId(phraseId)
	{}

	TDataSetRow			ActorRowId;
	TDataSetRow			TargetRowId;
	NLMISC::CSheetId	PhraseId;

	virtual void description ()
	{
		className ("CEGSExecutePhraseMsg");
		property ("actorRowId", PropDataSetRow, TDataSetRow(), ActorRowId);
		property ("targetRowId", PropDataSetRow, TDataSetRow(), TargetRowId);
		property ("phrase", PropSheetId, NLMISC::CSheetId::Unknown, PhraseId);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/**
 * Class CEGSExecuteAiActionMsg, message class used by npcs to execute an ai action
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CEGSExecuteAiActionMsg : public CMirrorTransportClass
{
public:
	CEGSExecuteAiActionMsg	() : CMirrorTransportClass()
	{}

	CEGSExecuteAiActionMsg	(const TDataSetRow	&actorRowId, const TDataSetRow	&targetRowId, const NLMISC::CSheetId &actionId, float	damageCoef=1, float	damageSpeedCoef=1)
		:CMirrorTransportClass()
		,ActorRowId(actorRowId)
		,TargetRowId(targetRowId)
		,ActionId(actionId)
		,DamageCoef(damageCoef)
		,DamageSpeedCoef(damageSpeedCoef)
	{}

	TDataSetRow			ActorRowId;
	TDataSetRow			TargetRowId;
	NLMISC::CSheetId	ActionId;
	float				DamageCoef;
	float				DamageSpeedCoef;

	virtual void description ()
	{
		className ("CEGSExecuteAiActionMsg");
		property ("actorRowId", PropDataSetRow, TDataSetRow(), ActorRowId);
		property ("targetRowId", PropDataSetRow, TDataSetRow(), TargetRowId);
		property ("actionId", PropSheetId, NLMISC::CSheetId::Unknown, ActionId);
		property ("damageCoef", PropFloat, 1.f, DamageCoef);
		property ("damageSpeedCoef", PropFloat, 1.f, DamageSpeedCoef);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id) {}
};


/**
 * Class CEGSExecuteMsg, message class used to execute a phrase
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CEGSExecuteMsg : public CMirrorTransportClass
{
public:
	TDataSetRow						ActorRowId;
	TDataSetRow						TargetRowId;
	std::vector<NLMISC::CSheetId>	BrickIds;
	bool							Cyclic;

	uint8							Index; // index of the phrase in the player memorized phrase interface if != 0xff

	CEGSExecuteMsg() : Cyclic(false), Index(0xff)
	{}

	virtual void description ()
	{
		className ("CEGSExecuteMsg");
		property ("actorRowId", PropDataSetRow, TDataSetRow(), ActorRowId);
		property ("targetRowId", PropDataSetRow, TDataSetRow(), TargetRowId);
		propertyCont ("brickIds", PropSheetId, BrickIds);
		property ("cyclic", PropBool, false, Cyclic);
		property ("index", PropUInt8, (uint8)0xff, Index);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id) {}
};


/**
 * Class CBSAIEventReportMsg
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CBSAIEventReportMsg : public CMirrorTransportClass
{
public:
	std::vector<TDataSetRow>	Originator;
	std::vector<TDataSetRow>	Target;
	std::vector<uint8>			AffectedStat;
	std::vector<sint32>			DeltaValue;
	std::vector<float>			AggroAdd;
//	std::vector<float>			AggroMul;
	std::vector<uint8>			ActionType;

	CBSAIEventReportMsg()
	{}

	//void pushBack( const TDataSetRow& originator, const TDataSetRow& target, const CAiEventReport &report)
	void pushBack( const CAiEventReport &report)
	{
		const uint size = (uint)Originator.size();
		const uint nbElts = report.AffectedStats.empty() ? 1 : (uint)report.AffectedStats.size();

		Originator.resize(size+nbElts);
		Target.resize(size+nbElts);
		AffectedStat.resize(size+nbElts);
		DeltaValue.resize(size+nbElts,0);
		AggroAdd.resize(size+nbElts,0.0f);
//		AggroMul.resize(size+nbElts,1.0f);
		ActionType.resize(size+nbElts,0);

		for (uint i = 0 ; i < nbElts ; ++i)
		{
			Originator[size + i] = report.Originator;
			Target[size + i] = report.Target;
			ActionType[size + i] = (uint8)report.Type;

			if ( i < report.AffectedStats.size() )
			{
				AffectedStat[size + i] = (uint8) report.AffectedStats[i];
				DeltaValue[size + i] = report.DeltaValue[i];
			}
			
			if (i == 0)
			{
				AggroAdd[size + i] = report.AggroAdd;
//				AggroMul[size + i] = report.AggroMul;
			}
			else
			{
				AggroAdd[size + i] = 0.0f;
//				AggroMul[size + i] = 1.0f;
			}			
		}
	}

	void clear()
	{
		Originator.clear();
		Target.clear();
		AffectedStat.clear();
		DeltaValue.clear();
		AggroAdd.clear();
//		AggroMul.clear();
		ActionType.clear();		
	}

	virtual void description()
	{
		className		("CBSAIEventReportMsg");
		propertyCont	("Originator",		PropDataSetRow,	Originator		);
		propertyCont	("Target",			PropDataSetRow,	Target			);
		propertyCont	("AffectedStat",	PropUInt8,	AffectedStat	);
		propertyCont	("DeltaValue",		PropSInt32,	DeltaValue		);
		propertyCont	("AggroAdd",		PropFloat,	AggroAdd		);
//		propertyCont	("AggroMul",		PropFloat,	AggroMul		);
		propertyCont	("ActionType",		PropUInt8,	ActionType		);
	}

	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/**
 * Class CBSAIDeathReport
 * \author Stephane le Dorze
 * \author Nevrax France
 * \date 2003
 * \Class sent from EGS to AIS to set AIS bots to dead mode.
 */

class CBSAIDeathReport : public CMirrorTransportClass
{
public:
	std::vector<TDataSetRow>	Bots;		// Bots to mark as dead.
	std::vector<TDataSetRow>	Killers;	// killer of each bot
	std::vector<bool>			Zombies;	// if true indicate this bot is detected as zombie 
	
	CBSAIDeathReport	()
	{
	}

	virtual void description ()
	{
		className ("CBSAIDeathReport");
		propertyCont ("Bots", PropDataSetRow, Bots);
		propertyCont ("Killers", PropDataSetRow, Killers);
		propertyVector ("Zombies", PropBool, Zombies);
	}
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

#endif // RY_MSG_BRICK_SERVICE_H

/* End of msg_brick_service.h */
