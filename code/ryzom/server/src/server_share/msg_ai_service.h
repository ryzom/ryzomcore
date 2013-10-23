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



#ifndef RY_AIS_INTERFACE_MESSAGES_H
#define RY_AIS_INTERFACE_MESSAGES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"

#include "game_share/base_types.h"
#include "game_share/synchronised_message.h"
#include "game_share/ryzom_mirror_properties.h"

#include "game_share/misc_const.h"
#include "game_share/outpost.h"

#include <vector>

// Transport class messages interface for AIS / EGS communication

//----------------------------------------------------------------------------
// EGS ask information about creatures / npcs against characters
//----------------------------------------------------------------------------
class CCreatureAskInformationMsg : public CMirrorTransportClass
{
public:
	std::vector< TDataSetRow > Character;
	std::vector< TDataSetRow > Creature;

	virtual void description ()
	{
		className ("CCreatureAskInformationMsg");

		propertyCont ("Character", PropDataSetRow, Character);
		propertyCont ("Creature", PropDataSetRow, Creature);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// Generic AIS state action message (AIS=>EGS)
//----------------------------------------------------------------------------
class CCAisActionMsg : public CMirrorTransportClass
{
public:
	uint32						Alias;
	std::vector<std::string>	Content;

	virtual void description ()
	{
		className ("CCAisActionMsg");
		property ("Alias", PropUInt32, uint32(0xffffffff), Alias);
		propertyCont ("Content", PropString, Content);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// User event to control AIS state machine (EGS=>AIS)
//----------------------------------------------------------------------------
class CUserEventMsg : public CMirrorTransportClass
{
public:
	uint32						InstanceNumber;
	/// The group to with the event will be send
	uint32						GrpAlias;
	/// The event number, must be 0 to 9.
	uint8						EventId;
	/// Parameters for the user event
	std::vector<std::string>	Params;

	virtual void description ()
	{
		className ("CUserEventMsg");
		property ("InstanceNumber", PropUInt32, uint32(~0), InstanceNumber);
		property ("GrpAlias", PropUInt32, uint32(0xffffffff), GrpAlias);
		property ("EventId", PropUInt8, uint8(0xff), EventId);
		propertyCont ("Params", PropString, Params);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// Fauna bot description (AIS->EGS) send each frame with info about all spawned fauna bots
//----------------------------------------------------------------------------
class CFaunaBotDescription : public CMirrorTransportClass
{
public:
	/// The list of bots spawned this frame.
	std::vector<TDataSetRow>	Bots;
	/// The group alias of each bots in Bots vector.
	std::vector<uint32>			GrpAlias;

	virtual void description ()
	{
		className ("CFaunaBotDescription");
		propertyCont ("Bots", PropDataSetRow, Bots);
		propertyCont ("GrpAlias", PropUInt32, GrpAlias);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// Message to set escort team id for a group EGS->AIS
//----------------------------------------------------------------------------
class CSetEscortTeamId : public CMirrorTransportClass
{
public:
	uint32						InstanceNumber;
	/// The list of group alias to escort by the team
	std::vector<uint32>			Groups;
	/// The team Id of the escorter
	uint16						TeamId;

	virtual void description ()
	{
		className ("CSetEscortTeamId");
		property ("InstanceNumber", PropUInt32, uint32(~0), InstanceNumber);
		propertyCont ("Groups", PropUInt32, Groups);
		property ("TeamId", PropUInt16, CTEAM::InvalidTeamId, TeamId);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS Message used to inform EGS of the existing *
// Sent every time an AIS is launched, for the continents it manages *
//----------------------------------------------------------------------------
//class COutpostList : public CMirrorTransportClass
//{
//public:
//	/// Continent name.
//	std::string					Continent;
//	/// outpost names
//	std::vector<std::string>	OutpostNames;
//	
//	virtual void description ()
//	{
//		className ("COutpostList");
//		property ("Continent", PropString, std::string() ,Continent);
//		propertyCont ("OutpostNames", PropString, OutpostNames);
//	}
//	
//	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
//};

//----------------------------------------------------------------------------
// AIS -> EGS Message used to inform EGS of the description of an outpost
//----------------------------------------------------------------------------
//class COutpostDescription : public CMirrorTransportClass
//{
//public:
//	
//	/// outpost names
//	std::string					OutpostName;
//
//	/// continent name ( where the outpost is )
//	std::string					Continent;
//
//	/// duties linked to this outpost
//	std::vector<std::string>	DutyNames;
//
//	/// X coord of the center of the outpost
//	sint32						CenterX;
//	/// Y coord of the center of the outpost
//	sint32						CenterY;
//
//	/// fames of the duties
////	std::vector<std::string>	Fames;
//	
//	virtual void description ()
//	{
//		className ("COutpostDescription");
//		property ("OutpostName", PropString,std::string(), OutpostName);
//		property ("Continent", PropString, std::string() ,Continent);
//		propertyCont ("DutyNames", PropString, DutyNames);
//		property ("CenterX", PropSInt32, (sint32) 0, CenterX);
//		property ("CenterY", PropSInt32, (sint32) 0, CenterY);
////		propertyCont ("Fames", PropString, DutyNames);
//	}
//	
//	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
//};

//----------------------------------------------------------------------------
// AIS -> EGS Message used to inform EGS of the description of a duty
//----------------------------------------------------------------------------
class CDutyDescription : public CMirrorTransportClass
{
public:
	
	/// Duty names
	std::string					DutyName;
	/// Civilisation 
	std::string					Civilisation;

	/// Duty parameters
	std::vector<std::string>	Parameters;

	/// fames of the duties
//	std::vector<std::string>	Fames;
	
	virtual void description ()
	{
		className ("CDutyDescription");
		property ("DutyName", PropString,std::string(), DutyName);
		property ("Civilisation", PropString, std::string() ,Civilisation);
		propertyCont ("Parameters", PropString, Parameters);
	}
	
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to inform AIS of the state of an outpost
//----------------------------------------------------------------------------
//class COutpostState : public CMirrorTransportClass
//{
//public:
//	//ZC Id
//	std::string			OutpostName;
//	//ZC state ( see ZCSTATE::TZcState enum in "zc_shard_common.h"
//	uint8				State;
//	// Id of the involved guild
//	TDataSetRow			GuildIndex;
//
//
//	
//	virtual void description ()
//	{
//		className ("COutpostState");
//		property ("OutpostName", PropString,std::string(), OutpostName);
//		property ("State", PropUInt8,(uint8)0,State);
//		property ("GuildIndex", PropDataSetRow, TDataSetRow() ,GuildIndex);
//	}
//	
//	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
//};


//----------------------------------------------------------------------------
// AIS -> EGS Message used to inform EGS of the result of a war
//----------------------------------------------------------------------------
//class COutpostWarEnd : public CMirrorTransportClass
//{
//public:
//	//ZC Id
//	std::string			OutpostName;
//
//	// true if the guild is victorious
//	bool				GuildVictory;
//
//	// Id of the involved guild
//	TDataSetRow			GuildId;	
//	
//	virtual void description ()
//	{
//		className ("COutpostWarEnd");
//		property ("OutpostName", PropString,std::string(), OutpostName);
//		property ("GuildVictory", PropBool,false,GuildVictory);
//		property ("GuildId", PropDataSetRow, TDataSetRow() ,GuildId);
//	}
//	
//	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
//};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to set a bot heading
//----------------------------------------------------------------------------
class CSetBotHeadingMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		BotRowId;
	float			Heading;
	
	
	virtual void description ()
	{
		className ("CSetBotHeadingMsg");
		property ("BotRowId", PropDataSetRow, TDataSetRow() , BotRowId);
		property ("Heading", PropFloat,0.0f,Heading);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to indicate a player "taunt" an AI entity
//----------------------------------------------------------------------------
class CAITauntMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		PlayerRowId;
	TDataSetRow		TargetRowId;
	
	virtual void description ()
	{
		className ("CAITauntMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
		property ("TargetRowId", PropDataSetRow, TDataSetRow(), TargetRowId);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to indicate a player "calm" an AI entity
// and go at the end of this creature aggro list
//----------------------------------------------------------------------------
class CAICalmCreatureMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		PlayerRowId;
	TDataSetRow		CreatureRowId;
	
	virtual void description ()
	{
		className ("CAICalmCreatureMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
		property ("CreatureRowId", PropDataSetRow, TDataSetRow(), CreatureRowId);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS Message used to indicate an aggro lost
//----------------------------------------------------------------------------
class CAILostAggroMsg : public CMirrorTransportClass
{
public:
	CAILostAggroMsg	()
	{	}

	CAILostAggroMsg	(const	TDataSetRow	&targetRowId,	const	TDataSetRow	&playerRowId) : PlayerRowId(playerRowId), TargetRowId(targetRowId)
	{
#ifdef NL_DEBUG
		nlassert(playerRowId.isValid());
		nlassert(targetRowId.isValid());
#endif
	}

	TDataSetRow		PlayerRowId;
	TDataSetRow		TargetRowId;
	
	virtual void description ()
	{
		className ("CAILostAggroMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
		property ("TargetRowId", PropDataSetRow, TDataSetRow(), TargetRowId);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS Message used to indicate an aggro gain
//----------------------------------------------------------------------------
class CAIGainAggroMsg : public CMirrorTransportClass
{
public:
	CAIGainAggroMsg	()
	{	}

	CAIGainAggroMsg	(const TDataSetRow &targetRowId, const TDataSetRow &playerRowId) : PlayerRowId(playerRowId), TargetRowId(targetRowId)
	{
#ifdef NL_DEBUG
		nlassert(playerRowId.isValid());
		nlassert(targetRowId.isValid());
#endif
	}

	TDataSetRow		PlayerRowId;
	TDataSetRow		TargetRowId;
	
	virtual void description ()
	{
		className ("CAIGainAggroMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
		property ("TargetRowId", PropDataSetRow, TDataSetRow(), TargetRowId);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to indicate a player has respawned
//----------------------------------------------------------------------------
class CAIPlayerRespawnMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		PlayerRowId;
	
	virtual void description ()
	{
		className ("CAIPlayerRespawnMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS ask infos on entity
//----------------------------------------------------------------------------
class CAIAskForInfosOnEntityMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		EntityRowId;
	TDataSetRow		AskerRowID;
	
	virtual void description ()
	{
		className ("CAIAskForInfosOnEntityMsg");
		property ("EntityRowId", PropDataSetRow, TDataSetRow(), EntityRowId);
		property ("AskerRowID", PropDataSetRow, TDataSetRow(), AskerRowID);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS returns infos on entity as a vector of strings
//----------------------------------------------------------------------------
class CAIInfosOnEntityMsg : public CMirrorTransportClass
{
public:
	TDataSetRow					EntityRowId;
	TDataSetRow					AskerRowID;
	std::vector<std::string>	Infos;
	
	virtual void description ()
	{
		className ("CAIInfosOnEntityMsg");
		property ("EntityRowId", PropDataSetRow, TDataSetRow(), EntityRowId);
		property ("AskerRowID", PropDataSetRow, TDataSetRow(), AskerRowID);
		propertyCont ("Infos", PropString, Infos);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS enable / disable agro on a specific entity
//----------------------------------------------------------------------------
class CEnableAggroOnPlayerMsg : public CMirrorTransportClass
{
public:
	TDataSetRow					EntityRowId;
	bool						EnableAggro;
	
	virtual void description ()
	{
		className ("CEnableAggroOnPlayerMsg");
		property ("EntityRowId", PropDataSetRow, TDataSetRow(), EntityRowId);
		property ("EnableAggro", PropBool, true, EnableAggro);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS send the available collision data
//----------------------------------------------------------------------------
class CReportAICollisionAvailableMsg : public CMirrorTransportClass
{
public:
	std::vector<std::string>	ContinentsCollision;
	
	virtual void description ()
	{
		className ("CReportAICollisionAvailableMsg");
		propertyCont ("ContinentsCollision", PropString, ContinentsCollision);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS send static continent instance in the ais
//----------------------------------------------------------------------------
class CReportStaticAIInstanceMsg : public CMirrorTransportClass
{
public:
	uint32			InstanceNumber;
	std::string		InstanceContinent;
	
	virtual void description ()
	{
		className ("CReportStaticAIInstanceMsg");
		property ("InstanceNumber", PropUInt32, uint32(0), InstanceNumber);
		property ("InstanceContinent", PropString, std::string(), InstanceContinent);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS send bot url information
//----------------------------------------------------------------------------
class CCreatureSetUrlMsg : public CMirrorTransportClass
{
public:
	std::vector<TDataSetRow>	Entities;
	std::string		ActionName;
	std::string		Url;
	
	virtual void description ()
	{
		className ("CCreatureSetUrlMsg");
		propertyCont ("Entities", PropDataSetRow, Entities);
		property ("ActionName", PropString, std::string(), ActionName);
		property ("Url", PropString, std::string(), Url);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS send despawn of aiinstance
//----------------------------------------------------------------------------
class CReportAIInstanceDespawnMsg : public CMirrorTransportClass
{
public:
	std::vector<uint32>			InstanceNumbers;
	
	virtual void description ()
	{
		className ("CReportAIInstanceDespawnMsg");
		propertyCont ("InstanceNumbers", PropUInt32, InstanceNumbers);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};


//----------------------------------------------------------------------------
// EGS -> AIS informe an AIS that one of it's instance spoof an already used instance number
//				or associate a static instance number with a bad continent.
//				The AIS must react to this by despawning the AIInstance.
//----------------------------------------------------------------------------
class CWarnBadInstanceMsg : public CMirrorTransportClass
{
public:
	uint32			InstanceNumber;
	
	virtual void description ()
	{
		className ("CWarnBadInstanceMsg");
		property ("InstanceNumber", PropUInt32, uint32(0), InstanceNumber);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS set/reset Action Flag
//----------------------------------------------------------------------------
class CChangeActionFlagMsg : public CMirrorTransportClass
{
public:
	std::vector<TDataSetRow>	Entities;
	std::vector<uint8>			ActionFlags;
	std::vector<bool>			Values;

	virtual void description ()
	{
		className ("CChangeActionFlagMsg");
		propertyCont ("Entities", PropDataSetRow, Entities);
		propertyCont ("ActionFlags", PropUInt8, ActionFlags);
		propertyVector ("Values", PropBool, Values);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}

	inline void push(TDataSetRow entity, uint8 flag, bool value)
	{
		Entities.push_back(entity);
		ActionFlags.push_back(flag);
		Values.push_back(value);
	}
};

//----------------------------------------------------------------------------
// AIS -> EGS must completely heal given creatures
//----------------------------------------------------------------------------
class CCreatureCompleteHealMsg : public CMirrorTransportClass
{
public:
	std::vector<TDataSetRow>	Entities;
	
	virtual void description ()
	{
		className ("CCreatureCompleteHealMsg");
		propertyCont ("Entities", PropDataSetRow, Entities);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS ais change creature Max HP
//----------------------------------------------------------------------------
class CChangeCreatureMaxHPMsg : public CMirrorTransportClass
{
public:
	std::vector<TDataSetRow>    Entities;
	std::vector<uint32>         MaxHp;
	std::vector<uint8>          SetFull;
	
	virtual void description ()
	{
		className ("CChangeCreatureMaxHPMsg");
		propertyCont ("Entities", PropDataSetRow, Entities);
		propertyCont ("MaxHp", PropUInt32, MaxHp);
		propertyCont ("SetFull", PropUInt8, SetFull);
	}
	
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS ais change creature HP
//----------------------------------------------------------------------------
class CChangeCreatureHPMsg : public CMirrorTransportClass
{
public:
	std::vector<TDataSetRow>	Entities;
	std::vector<sint32>			DeltaHp;
	
	virtual void description ()
	{
		className ("CChangeCreatureHPMsg");
		propertyCont ("Entities", PropDataSetRow, Entities);
		propertyCont ("DeltaHp", PropSInt32, DeltaHp);
	}
	
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS egs want to change creature mode (pet animals...)
//----------------------------------------------------------------------------
class CChangeCreatureModeMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		CreatureId;
	uint8			NewMode;
	
	virtual void description ()
	{
		className ("CChangeCreatureModeMsg");
		property ("CreatureId", PropDataSetRow, TDataSetRow(), CreatureId);
		property ("NewMode", PropUInt8, (uint8)0, NewMode);	}
	
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to despawn creatures
//----------------------------------------------------------------------------
class CCreatureDespawnMsg : public CMirrorTransportClass
{
public:
	std::vector<TDataSetRow>	Entities;
	
	virtual void description ()
	{
		className ("CCreatureDespawnMsg");
		propertyCont ("Entities", PropDataSetRow, Entities);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

/////////////////////////////////////////////////////////////////////////////
// Outposts                                                                //
/////////////////////////////////////////////////////////////////////////////

// EGS -> AIS Message used to create a squad
struct COutpostCreateSquadMsg
{
	TAIAlias	Outpost;
	TAIAlias	Group;
	TAIAlias	Zone;
	uint32		CreateOrder;
	uint32		RespawnTimeS;
	OUTPOSTENUMS::TPVPSide	Side;
	void serial(NLMISC::IStream& s)
	{
		static uint32 const s_version = 1;
		uint32 version = s_version;
		s.serial(version);
		nlassert(version==s_version);
		s.serial(Outpost);
		s.serial(Group);
		s.serial(Zone);
		s.serial(CreateOrder);
		s.serial(RespawnTimeS);
		std::string str = OUTPOSTENUMS::toString(Side);
		s.serial(str);
		Side = OUTPOSTENUMS::toPVPSide(str);
	}
};

// AIS -> EGS Message used to notify a squad spawned
struct COutpostSquadCreatedMsg
{
	TAIAlias	Outpost;
	TAIAlias	CreateOrder;
	uint32		GroupId;
	void serial(NLMISC::IStream& s)
	{
		s.serial(Outpost);
		s.serial(CreateOrder);
		s.serial(GroupId);
	}
};

// EGS -> AIS Message used to spawn a squad all squads
struct COutpostSpawnSquadMsg
{
	TAIAlias	Outpost;
	uint32		GroupId;
	void serial(NLMISC::IStream& s)
	{
		s.serial(Outpost);
		s.serial(GroupId);
	}
};

// AIS -> EGS Message used to notify a squad spawned
struct COutpostSquadSpawnedMsg
{
	TAIAlias	Outpost;
	uint32		GroupId;
	void serial(NLMISC::IStream& s)
	{
		s.serial(Outpost);
		s.serial(GroupId);
	}
};

// EGS -> AIS Message used to despawn a squad all squads
struct COutpostDespawnSquadMsg
{
	TAIAlias	Outpost;
	uint32		GroupId;
	void serial(NLMISC::IStream& s)
	{
		s.serial(Outpost);
		s.serial(GroupId);
	}
};

// AIS -> EGS Message used to notify a squad despawned
struct COutpostSquadDespawnedMsg
{
	TAIAlias	Outpost;
	uint32		GroupId;
	void serial(NLMISC::IStream& s)
	{
		s.serial(Outpost);
		s.serial(GroupId);
	}
};

// EGS -> AIS Message used to delete a squad all squads
struct COutpostDeleteSquadMsg
{
	TAIAlias	Outpost;
	uint32		GroupId;
	void serial(NLMISC::IStream& s)
	{
		s.serial(Outpost);
		s.serial(GroupId);
	}
};

// AIS -> EGS Message used to notify a squad died
struct COutpostSquadDiedMsg
{
	TAIAlias	Outpost;
	uint32		GroupId;
	void serial(NLMISC::IStream& s)
	{
		s.serial(Outpost);
		s.serial(GroupId);
	}
};

// AIS -> EGS Message used to notify a squad leader died
struct COutpostSquadLeaderDiedMsg
{
	TAIAlias	Outpost;
	uint32		GroupId;
	void serial(NLMISC::IStream& s)
	{
		s.serial(Outpost);
		s.serial(GroupId);
	}
};

// EGS -> AIS Message used to despawn all squads
struct COutpostDespawnAllSquadsMsg
{
	TAIAlias	Outpost;
	
	void	serial( NLMISC::IStream& s )
	{
		s.serial( Outpost );
	}
};

// NOTE: for the moment an alliance is only a guild in the EGS.
// We keep the high level concept of "alliance" in AIS
// because what is really an alliance does not matter for the AIS.
typedef uint32 TAllianceId;
const TAllianceId InvalidAllianceId = 0;

//----------------------------------------------------------------------------
// EGS -> AIS Message used to set the owner of an outpost
//----------------------------------------------------------------------------
struct CSetOutpostOwner
{
	TAIAlias		Outpost;
	TAllianceId		Owner;		// InvalidAllianceId means "the tribe"

	void	serial( NLMISC::IStream& s )
	{
		s.serial( Outpost );
		s.serial( Owner );
	}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to set the attacker of an outpost.
//----------------------------------------------------------------------------
struct CSetOutpostAttacker
{
	TAIAlias		Outpost;
	TAllianceId		Attacker;	// InvalidAllianceId means "no attacker"
	
	void	serial( NLMISC::IStream& s )
	{
		s.serial( Outpost );
		s.serial( Attacker );
	}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to notify ais of outpost state changes
//----------------------------------------------------------------------------
struct COutpostSetStateMsg
{
	TAIAlias	Outpost;
	OUTPOSTENUMS::TOutpostState	State;
	void serial(NLMISC::IStream& s)
	{
		uint32 const localVersion = 1;
		uint32 version = localVersion;
		s.serial(version);
		nlassert(version==localVersion);
		s.serial(Outpost);
		std::string str = OUTPOSTENUMS::toString(State);
		s.serial(str);
		State = OUTPOSTENUMS::toOutpostState(str);
	}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to notify ais of outpost state changes
//----------------------------------------------------------------------------
struct COutpostEventMsg
{
	TAIAlias	Outpost;
	OUTPOSTENUMS::TSpecialOutpostEvent	Event;
	void serial(NLMISC::IStream& s)
	{
		uint32 const localVersion = 1;
		uint32 version = localVersion;
		s.serial(version);
		nlassert(version==localVersion);
		s.serial(Outpost);
		std::string str = OUTPOSTENUMS::toString(Event);
		s.serial(str);
		Event = OUTPOSTENUMS::toSpecialOutpostEvent(str);
	}
};

//----------------------------------------------------------------------------
// EGS -> AIS Message used to set the sheet of an outpost building bot
//----------------------------------------------------------------------------
struct COutpostSetBuildingBotSheetMsg
{
	TAIAlias			Outpost;
	TAIAlias			Building;
	NLMISC::CSheetId	SheetId;
	bool				AutoSpawnDespawn;
	std::string			CustomName;
	void serial(NLMISC::IStream& s)
	{
		uint32 const localVersion = 1;
		uint32 version = localVersion;
		s.serial(version);
		nlassert(version==localVersion);
		s.serial(Outpost);
		s.serial(Building);
		s.serial(SheetId);
		s.serial(AutoSpawnDespawn);
		s.serial(CustomName);
	}
};

//----------------------------------------------------------------------------
// EGS -> AIS Ask to add a handle on a group (spawn it if necessary) (called handledAIGroup)
//----------------------------------------------------------------------------
class CAddHandledAIGroupMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		PlayerRowId;
	uint32			GroupAlias;
	uint32			MissionAlias;
	uint32			DespawnTimeInTick;

	virtual void description ()
	{
		className ("CAddHandledAIGroupMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
		property ("GroupAlias", PropUInt32, uint32(0xffffffff), GroupAlias);
		property ("MissionAlias", PropUInt32, uint32(0xffffffff), MissionAlias);
		property ("DespawnTimeInTick", PropUInt32, uint32(0), DespawnTimeInTick);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// EGS -> AIS Ask to del a handle on a group (despawn it if necessary) (called handledAIGroup)
//----------------------------------------------------------------------------
class CDelHandledAIGroupMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		PlayerRowId;
	uint32			GroupAlias;
	uint32			MissionAlias;
	
	virtual void description ()
	{
		className ("CDelHandledAIGroupMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
		property ("GroupAlias", PropUInt32, uint32(0xffffffff), GroupAlias);
		property ("MissionAlias", PropUInt32, uint32(0xffffffff), MissionAlias);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS ok the group is spawned
//----------------------------------------------------------------------------
class CHandledAIGroupSpawnedMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		PlayerRowId;
	uint32			GroupAlias;
	uint32			MissionAlias;
	
	virtual void description ()
	{
		className ("CHandledAIGroupSpawnedMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
		property ("GroupAlias", PropUInt32, uint32(0xffffffff), GroupAlias);
		property ("MissionAlias", PropUInt32, uint32(0xffffffff), MissionAlias);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS ok the group is despawned
//----------------------------------------------------------------------------
class CHandledAIGroupDespawnedMsg : public CMirrorTransportClass
{
public:
	TDataSetRow		PlayerRowId;
	uint32			GroupAlias;
	uint32			MissionAlias;
	
	virtual void description ()
	{
		className ("CHandledAIGroupDespawnedMsg");
		property ("PlayerRowId", PropDataSetRow, TDataSetRow(), PlayerRowId);
		property ("GroupAlias", PropUInt32, uint32(0xffffffff), GroupAlias);
		property ("MissionAlias", PropUInt32, uint32(0xffffffff), MissionAlias);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};


//----------------------------------------------------------------------------
// AIS -> EGS AIS send a item request, message interface for manage in same way give and receive msg
//----------------------------------------------------------------------------
class CItemRequestMsgItf : public CMirrorTransportClass
{
public:
	virtual uint32									getInstanceId() const =0;
	virtual uint32									getGroupAlias() const =0;
	virtual TDataSetRow								getCharacterRowId() const =0;
	virtual TDataSetRow								getCreatureRowId() const =0;
	virtual const std::vector<NLMISC::CSheetId>&	getItems() const =0;
	virtual const std::vector<uint32>&				getQuantities() const =0;
	virtual const std::string&						getMissionText() const =0;	// utf8 string
};

//----------------------------------------------------------------------------
// AIS -> EGS AIS send a give item request
//----------------------------------------------------------------------------
class CGiveItemRequestMsg : public CItemRequestMsgItf
{
public:
	uint32							InstanceId;
	uint32							GroupAlias;
	TDataSetRow						CharacterRowId;
	TDataSetRow						CreatureRowId;
	std::vector<NLMISC::CSheetId>	Items;
	std::vector<uint32>				Quantities;
	std::string						MissionText;	// utf8 string

	uint32									getInstanceId() const { return InstanceId; }
	uint32									getGroupAlias() const { return GroupAlias; }
	TDataSetRow								getCharacterRowId() const { return CharacterRowId; }
	TDataSetRow								getCreatureRowId() const { return CreatureRowId; }
	const std::vector<NLMISC::CSheetId>&	getItems() const { return Items; }
	const std::vector<uint32>&				getQuantities() const { return Quantities; }
	const std::string&						getMissionText() const { return MissionText; }

	virtual void description ()
	{
		className ("CGiveItemRequestMsg");
		property ("InstanceId", PropUInt32, uint32(0xffffffff), InstanceId);
		property ("GroupAlias", PropUInt32, uint32(0xffffffff), GroupAlias);
		property ("CharacterRowId", PropDataSetRow, TDataSetRow(), CharacterRowId);
		property ("CreatureRowId", PropDataSetRow, TDataSetRow(), CreatureRowId);
		propertyCont ("Items", PropSheetId, Items);
		propertyCont ("Quantities", PropUInt32, Quantities);
		property ("MissionText", PropString, std::string(), MissionText);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};

//----------------------------------------------------------------------------
// AIS -> EGS AIS send a receive item request
//----------------------------------------------------------------------------
class CReceiveItemRequestMsg : public CItemRequestMsgItf
{
public:
	uint32							InstanceId;
	uint32							GroupAlias;
	TDataSetRow						CharacterRowId;
	TDataSetRow						CreatureRowId;
	std::vector<NLMISC::CSheetId>	Items;
	std::vector<uint32>				Quantities;
	std::string						MissionText;	// utf8 string

	uint32									getInstanceId() const { return InstanceId; }
	uint32									getGroupAlias() const { return GroupAlias; }
	TDataSetRow								getCharacterRowId() const { return CharacterRowId; }
	TDataSetRow								getCreatureRowId() const { return CreatureRowId; }
	const std::vector<NLMISC::CSheetId>&	getItems() const { return Items; }
	const std::vector<uint32>&				getQuantities() const { return Quantities; }
	const std::string&						getMissionText() const { return MissionText; }

	virtual void description ()
	{
		className ("CReceiveItemRequestMsg");
		property ("InstanceId", PropUInt32, uint32(0xffffffff), InstanceId);
		property ("GroupAlias", PropUInt32, uint32(0xffffffff), GroupAlias);
		property ("CharacterRowId", PropDataSetRow, TDataSetRow(), CharacterRowId);
		property ("CreatureRowId", PropDataSetRow, TDataSetRow(), CreatureRowId);
		propertyCont ("Items", PropSheetId, Items);
		propertyCont ("Quantities", PropUInt32, Quantities);
		property ("MissionText", PropString, std::string(), MissionText);
	}
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
};


//----------------------------------------------------------------------------
// AIS -> EGS Ais script send a query to EGS that will be answered via script (user event)
//----------------------------------------------------------------------------
//see CUserEventMsg

//----------------------------------------------------------------------------
// User event to control AIS state machine (EGS=>AIS)
//----------------------------------------------------------------------------
class CQueryEgs : public CMirrorTransportClass
{
public:
	enum  TFunEnum
	{
		Name,
		Hp, MaxHp, RatioHp,
		Sap, MaxSap, RatioSap,
		Stamina, MaxStamina, RatioStamina,
		Focus, MaxFocus, RatioFocus,
		BestSkillLevel, 
		Target,
		IsInInventory, KnowBrick,
		Undef
	};

	typedef std::map<std::string, TFunEnum>  TFuns;

public:
	uint32						InstanceId;
	/// The group to with the event will be send
	uint32						GroupAlias;
	/// The event number, must be 0 to 9.
	uint8						EventId;
	/// Parameters for the user event
	std::vector<std::string>	Params;

	virtual void description ()
	{
		className ("CQueryEgs");
		property ("InstanceId", PropUInt32, uint32(0xffffffff), InstanceId);
		property ("GroupAlias", PropUInt32, uint32(0xffffffff), GroupAlias);
		property ("EventId", PropUInt8, uint8(0xff), EventId);
		propertyCont ("Params", PropString, Params);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) {}
	
	TFunEnum getFunEnum(const std::string& funName) const;
	
private:
	// lazy Initialisation
	void init();
private:
	static TFuns _Funs;
};


#endif //RY_AIS_INTERFACE_MESSAGES_H
