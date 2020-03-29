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


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stat_char_info_extractor_factory.h"
#include "stat_character_scan_job.h"
#include "stat_character.h"
#include "game_share/pvp_clan.h"
#include "nel/net/service.h"
#include <time.h>


//-------------------------------------------------------------------------------------------------
// Handy utilities
//-------------------------------------------------------------------------------------------------

static NLMISC::CSString buildDateString(uint32 timeValue)
{
	const char* monthNames[]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dev"};
	if (timeValue==0)
		return "";
	time_t rawtime= timeValue;
	struct tm * timeinfo= gmtime(&rawtime);
	return NLMISC::toString("%u %s %u",timeinfo->tm_mday,monthNames[timeinfo->tm_mon],timeinfo->tm_year+1900);
}

static NLMISC::CSString buildDowTimeString(uint32 timeValue)
{
	const char* dowNames[]= {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
	if (timeValue==0)
		return "";
	time_t rawtime= timeValue;
	struct tm * timeinfo= gmtime(&rawtime);
	return NLMISC::toString("%s %u:%u",dowNames[timeinfo->tm_wday-1],timeinfo->tm_hour,timeinfo->tm_min);
}

static NLMISC::CSString buildDurationString(uint32 timeValue)
{
	NLMISC::CSString result;

	if (timeValue>= 24*60*60)
	{
		result= (timeValue>= 2*24*60*60)?
			NLMISC::toString("%u_days_",timeValue/(24*60*60)):
			"1_day_";
		timeValue%= (24*60*60);
	}
	result+= NLMISC::toString("%u:%02u:%02u",timeValue/3600,(timeValue/60)%60,timeValue%60);
	return result;
}



//-------------------------------------------------------------------------------------------------
// INFO_EXTRACTOR Implementations
//-------------------------------------------------------------------------------------------------

INFO_EXTRACTOR(Name,"The character name","name")
{
	job->charTblSetEntry("name",c->EntityBase._Name);
}

INFO_EXTRACTOR(PlayTime,"The time in hours that the character has spent on-line since the 'play time' stat was added","play_hours")
{
	float f= float(c->_PlayedTime)/(60.0f*60.0f);
	job->charTblSetEntry("play_hours",NLMISC::toString("%.2f",f));
}

INFO_EXTRACTOR(LastActivityDate,"The last time that the player connected (time stamp)","last_date")
{
	job->charTblSetEntry("last_date",buildDateString(c->_LastConnectedTime));
}

INFO_EXTRACTOR(FirstActivityDate,"The first time that the player connected","start_date")
{
	job->charTblSetEntry("start_date",buildDateString(c->_FirstConnectedTime));
}

INFO_EXTRACTOR(LastActivityTime,"The last time that the player connected (time stamp)","last_time")
{
	job->charTblSetEntry("last_time",buildDowTimeString(c->_LastConnectedTime));
}

INFO_EXTRACTOR(FirstActivityTime,"The first time that the player connected","start_time")
{
	job->charTblSetEntry("start_time",buildDowTimeString(c->_FirstConnectedTime));
}

INFO_EXTRACTOR(ActivityDuration,"The difference in days between first time connected and last time connected","active_days")
{
	uint32 firstDay= c->_FirstConnectedTime/ (60*60*24);
	uint32 lastDay= c->_LastConnectedTime/ (60*60*24);

	// if either the first activity or last activity values are not present then just return
	if (firstDay==0 || lastDay==0)
		return;

	uint32 days= lastDay- firstDay+1;
	job->charTblSetEntry("active_days",NLMISC::toString(days));
}

INFO_EXTRACTOR(RecentPlayHistory,"Recent play time stats","num_sessions,shortest_session,longest_session,average_session,ttl_session_time,session_list")
{
	uint32 numSessions=0;
	uint32 ttlPlayTime=0;
	uint32 longestSession=0;
	uint32 shortestSession=~0u;
	std::string sessionList;

	for (std::list<CStatsScanCharacterLogTime>::const_iterator it=c->_LastLogStats.begin();it!=c->_LastLogStats.end();++it)
	{
		sint32 sessionDuration= it->LogoffTime- it->LoginTime;
		if (sessionDuration<0) continue;
		numSessions++;
		ttlPlayTime+=sessionDuration;
		longestSession=std::max(longestSession,(uint32)sessionDuration);
		shortestSession=std::min(shortestSession,(uint32)sessionDuration);
		if (!sessionList.empty())
			sessionList+= ' ';
		sessionList+=buildDurationString(sessionDuration);
	}

	job->charTblSetEntry("num_sessions",NLMISC::toString(numSessions));
	job->charTblSetEntry("shortest_session",buildDurationString(shortestSession));
	job->charTblSetEntry("longest_session",buildDurationString(longestSession));
	job->charTblSetEntry("average_session",(numSessions==0)?"0":buildDurationString(ttlPlayTime/numSessions));
	job->charTblSetEntry("ttl_session_time",buildDurationString(ttlPlayTime));
	job->charTblSetEntry("session_list",sessionList);
}

INFO_EXTRACTOR(Race,"The character's race","race")
{
	job->charTblSetEntry("race",c->EntityBase._Race);
}

INFO_EXTRACTOR(Gender,"The character's gender","gender")
{
	job->charTblSetEntry("gender",c->EntityBase._Gender==0?"Male":"Female");
}

INFO_EXTRACTOR(GuildId,"The guild number","guild")
{
	job->charTblSetEntry("guild",NLMISC::toString(c->_GuildId));
}

INFO_EXTRACTOR(Money,"Cash in hand","money")
{
	job->charTblSetEntry("money",NLMISC::toString(c->_Money));
}

INFO_EXTRACTOR(AuraEnd,"end date of the prohibited reuse time for auras","aura_end")
{
	job->charTblSetEntry("aura_end",NLMISC::toString(c->_ForbidAuraUseEndDate));
}

INFO_EXTRACTOR(Position,"x y position","x,y")
{
	if (c->NormalPositions._Vec.empty())
	{
		job->charTblSetEntry("x",NLMISC::toString(c->EntityBase._EntityPosition.X/1000));
		job->charTblSetEntry("y",NLMISC::toString(c->EntityBase._EntityPosition.Y/1000));
	}
	else
	{
		job->charTblSetEntry("x",NLMISC::toString(c->NormalPositions._Vec.back().PosState.X/1000));
		job->charTblSetEntry("y",NLMISC::toString(c->NormalPositions._Vec.back().PosState.Y/1000));
	}
}

INFO_EXTRACTOR(Sessions,"home session id and current session id","home,session")
{
	// setup a static map of session ids to shard names
	typedef std::map<uint32,NLMISC::CSString> TShardNames;
	static TShardNames shardNames;
	static bool init= false;
	if (!init)
	{
		breakable
		{
			init= true;

			NLMISC::CConfigFile::CVar *sessionNames = NLNET::IService::getInstance()->ConfigFile.getVarPtr("HomeMainlandNames");
			DROP_IF(sessionNames == NULL,"'HomeMainlandNames' not found in cfg file",break);

			for (uint i=0; i<sessionNames->size()/3; ++i)
			{
				NLMISC::CSString sessionIdString= sessionNames->asString(i*3);
				uint32 sessionId = sessionIdString.atoui();
				NLMISC::CSString shardName = sessionNames->asString(i*3+1);
				DROP_IF(sessionId==0,"Invalid session id "+sessionIdString,continue);
				nldebug("Adding shard name mappiing: %u => %s",sessionId,shardName.c_str());
				shardNames[sessionId]= shardName;
			}
		}
	}

	// if this character doesn't have a position vector then the information on the home mainland is not available here
	// the character is probably old
	if (c->NormalPositions._Vec.empty())
	{
		return;
	}

	// lookup the player's home session in the shardNames map
	uint32 homeSessionId= c->NormalPositions._Vec[0].SessionId;
	TShardNames::iterator homeIt= shardNames.find(homeSessionId);

	// if the home shard is known then put in the shard name, otherwise put in a session number
	job->charTblSetEntry("home",
		(homeIt!=shardNames.end())?
			homeIt->second.c_str():
			NLMISC::toString("%u",homeSessionId));

	// if this character doesn't have a current session then just leave the field blank
	if (c->NormalPositions._Vec.size()<2)
	{
		return;
	}

	// lookup the player's current session in the shardNames map
	uint32 curSessionId= c->NormalPositions._Vec.back().SessionId;
	TShardNames::iterator curIt= shardNames.find(curSessionId);

	// if the shard is known then put in the shard name, otherwise put in a session number
	job->charTblSetEntry("session",
		(curIt!=shardNames.end())?
			curIt->second.c_str():
			NLMISC::toString("%s%u",shardNames.empty()?"":"Ring:",curSessionId));
}

INFO_EXTRACTOR(Characs,"constitution, strength, etc","Constitution,Metabolism,Intelligence,Wisdom,Strength,WellBalanced,Dexterity,Will")
{
	job->charTblSetEntry("Constitution",NLMISC::toString(c->EntityBase._PhysCharacs._PhysicalCharacteristics.find("Constitution")->second));
	job->charTblSetEntry("Metabolism",	NLMISC::toString(c->EntityBase._PhysCharacs._PhysicalCharacteristics.find("Metabolism")->second));
	job->charTblSetEntry("Intelligence",NLMISC::toString(c->EntityBase._PhysCharacs._PhysicalCharacteristics.find("Intelligence")->second));
	job->charTblSetEntry("Wisdom",		NLMISC::toString(c->EntityBase._PhysCharacs._PhysicalCharacteristics.find("Wisdom")->second));
	job->charTblSetEntry("Strength",	NLMISC::toString(c->EntityBase._PhysCharacs._PhysicalCharacteristics.find("Strength")->second));
	job->charTblSetEntry("WellBalanced",NLMISC::toString(c->EntityBase._PhysCharacs._PhysicalCharacteristics.find("WellBalanced")->second));
	job->charTblSetEntry("Dexterity",	NLMISC::toString(c->EntityBase._PhysCharacs._PhysicalCharacteristics.find("Dexterity")->second));
	job->charTblSetEntry("Will",		NLMISC::toString(c->EntityBase._PhysCharacs._PhysicalCharacteristics.find("Will")->second));
}

INFO_EXTRACTOR(Stats,"hp, sap, sta, focus","HP,Stamina,Sap,Focus")
{
	job->charTblSetEntry("HP",		NLMISC::toString(c->EntityBase._PhysScores.PhysicalScores.find("HitPoints")->second.Current));
	job->charTblSetEntry("Stamina",	NLMISC::toString(c->EntityBase._PhysScores.PhysicalScores.find("Stamina")->second.Current));
	job->charTblSetEntry("Sap",		NLMISC::toString(c->EntityBase._PhysScores.PhysicalScores.find("Sap")->second.Current));
	job->charTblSetEntry("Focus",	NLMISC::toString(c->EntityBase._PhysScores.PhysicalScores.find("Focus")->second.Current));
}

INFO_EXTRACTOR(Fames,"main fame scores","Fyros,Kami,Karavan,Matis,Tryker,Zorai")
{
	job->charTblSetEntry("Fyros",	NLMISC::toString(c->_Fames._Fame.find(NLMISC::CSheetId("fyros.faction"))->second.Fame));
	job->charTblSetEntry("Kami",	NLMISC::toString(c->_Fames._Fame.find(NLMISC::CSheetId("kami.faction"))->second.Fame));
	job->charTblSetEntry("Karavan",	NLMISC::toString(c->_Fames._Fame.find(NLMISC::CSheetId("karavan.faction"))->second.Fame));
	job->charTblSetEntry("Matis",	NLMISC::toString(c->_Fames._Fame.find(NLMISC::CSheetId("matis.faction"))->second.Fame));
	job->charTblSetEntry("Tryker",	NLMISC::toString(c->_Fames._Fame.find(NLMISC::CSheetId("tryker.faction"))->second.Fame));
	job->charTblSetEntry("Zorai",	NLMISC::toString(c->_Fames._Fame.find(NLMISC::CSheetId("zorai.faction"))->second.Fame));
}

INFO_EXTRACTOR(Civ,"declared civilization","civ")
{
	job->charTblSetEntry("civ",	c->DeclaredCiv);
}

INFO_EXTRACTOR(Cult,"declared cult","cult")
{
	job->charTblSetEntry("cult", c->DeclaredCult);
}

INFO_EXTRACTOR(DeathPenalty,"death penalty stats","Penalty,Nb_Deaths")
{
	job->charTblSetEntry("Penalty",	NLMISC::toString(c->_DeathPenalties._DeathXPToGain));
	job->charTblSetEntry("Nb_Deaths",	NLMISC::toString(c->_DeathPenalties._NbDeath));
}

INFO_EXTRACTOR(Rites,"frequency stats regarding rites","StartedRites,FinishedRites")
{
	uint32 startedRites= 0;
	uint32 finishedRites= 0;
	for (uint32 i=0;i<c->_EncycloChar._EncyCharAlbums.size();++i)
	{
		c->_EncycloChar._EncyCharAlbums[i].AlbumState;
		for (uint32 j=0;j<c->_EncycloChar._EncyCharAlbums[i].Themas.size();++j)
		{
			job->freqTblAddEntry("encyclopedia_themes",NLMISC::toString("Theme_%02d_%02d_State %d",i,j,c->_EncycloChar._EncyCharAlbums[i].Themas[j].ThemaState));
			uint32 bitCount=0;
			uint32 bitMask=c->_EncycloChar._EncyCharAlbums[i].Themas[j].RiteTaskStatePacked;
			for (uint32 k=bitMask;k!=0;k>>=2) if (k&3) ++bitCount;
			job->freqTblAddEntry("encyclopedia_tasks",NLMISC::toString("Theme_%02d_%02d_Tasks %d%s",i,j,bitCount,(bitMask&1)?" ALL":""));

			startedRites+= (bitCount>0)?1:0;
			finishedRites+= (bitMask&1)?1:0;
		}
	}
	job->charTblSetEntry("StartedRites",NLMISC::toString(startedRites));
	job->charTblSetEntry("FinishedRites",NLMISC::toString(finishedRites));
	job->freqTblAddEntry("started_rites",NLMISC::toString(startedRites));
	job->freqTblAddEntry("finished_rites",NLMISC::toString(finishedRites));
}

INFO_EXTRACTOR(SkillPoints,"skill points and related stats","spFight,ttlSpFight,minSpFight,spMagic,ttlSpMagic,minSpMagic,spCraft,ttlSpCraft,minSpCraft,spHarvest,ttlSpHarvest,minSpHarvest")
{
	job->charTblSetEntry("spFight",NLMISC::toString(c->SkillPoints.find("Fight")->second));
	job->charTblSetEntry("spMagic",NLMISC::toString(c->SkillPoints.find("Magic")->second));
	job->charTblSetEntry("spCraft",NLMISC::toString(c->SkillPoints.find("Craft")->second));
	job->charTblSetEntry("spHarvest",NLMISC::toString(c->SkillPoints.find("Harvest")->second));

	job->charTblSetEntry("ttlSpFight",NLMISC::toString(c->SkillPoints.find("Fight")->second+c->SpentSkillPoints.find("Fight")->second));
	job->charTblSetEntry("ttlSpMagic",NLMISC::toString(c->SkillPoints.find("Magic")->second+c->SpentSkillPoints.find("Magic")->second));
	job->charTblSetEntry("ttlSpCraft",NLMISC::toString(c->SkillPoints.find("Craft")->second+c->SpentSkillPoints.find("Craft")->second));
	job->charTblSetEntry("ttlSpHarvest",NLMISC::toString(c->SkillPoints.find("Harvest")->second+c->SpentSkillPoints.find("Harvest")->second));

	sint32 ttlF=0;
	sint32 ttlM=0;
	sint32 ttlC=0;
	sint32 ttlH=0;

	typedef std::map<NLMISC::CSString,CStatsScanSkillsEntry> TSkills;
	const TSkills& skills=c->EntityBase._Skills.Skills;
	for(TSkills::const_iterator it=skills.begin();it!=skills.end();++it)
	{
		BOMB_IF((*it).first.size()<2||(*it).first.left(1).toUpper()!="S","Invalid skill: '"+(*it).first+"'",continue);
		char skillType= (*it).first.toUpper()[1];

		uint32 skillValue= (*it).second.Current;
		uint32 result=0;
		switch((*it).second.Current)
		{
		case 21: case 22:	case 23: case 24: case 25: case 26: case 27: case 28: case 29: case 30: 
		case 31: case 32:	case 33: case 34: case 35: case 36: case 37: case 38: case 39: case 40: 
		case 41: case 42:	case 43: case 44: case 45: case 46: case 47: case 48: case 49: 
			result= skillValue-20;
			break;
		
		case 50: 
			result= ((*it).second.Xp>=(*it).second.XpNextLvl)? 30: 29;
			break;

		case 20: case 100: case 150: case 200: case 250:
			result= ((*it).second.Xp>=(*it).second.XpNextLvl)? ((skillValue-1)%50+1): ((skillValue-1)%50);
			break;

		default:
			result+= ((skillValue-1)%50);
			break;
		}

		switch (skillType)
		{
			case 'F': ttlF+= result; break;
			case 'M': ttlM+= result; break;
			case 'C': ttlC+= result; break;
			case 'H': ttlH+= result; break;
		}

		if (result!=0 && (*it).second.Xp>0)
			job->freqTblAddEntry("usedSkills",(*it).first);
	}

	job->charTblSetEntry("minSpFight",NLMISC::toString(ttlF*10));
	job->charTblSetEntry("minSpMagic",NLMISC::toString(ttlM*10));
	job->charTblSetEntry("minSpCraft",NLMISC::toString(ttlC*10));
	job->charTblSetEntry("minSpHarvest",NLMISC::toString(ttlH*10));
}

INFO_EXTRACTOR(HighestSkills,"Highest skills in each main branch","skill_best,skill_sf,skill_sm,skill_sc,skill_sh")
{
	sint32 best=0;
	sint32 bestF=0;
	sint32 bestM=0;
	sint32 bestC=0;
	sint32 bestH=0;
	NLMISC::CSString bestName;
	NLMISC::CSString bestFName;
	NLMISC::CSString bestMName;
	NLMISC::CSString bestCName;
	NLMISC::CSString bestHName;

	typedef std::map<NLMISC::CSString,CStatsScanSkillsEntry> TSkills;
	const TSkills& skills=c->EntityBase._Skills.Skills;
	for(TSkills::const_iterator it=skills.begin();it!=skills.end();++it)
	{
		BOMB_IF((*it).first.size()<2||(*it).first.left(1).toUpper()!="S","Invalid skill: '"+(*it).first+"'",continue);
		char skillType= (*it).first.toUpper()[1];
		switch (skillType)
		{
			case 'F': if (bestF<=(*it).second.Current) bestF=(*it).second.Current; bestFName=(*it).first; break;
			case 'M': if (bestM<=(*it).second.Current) bestM=(*it).second.Current; bestMName=(*it).first; break;
			case 'C': if (bestC<=(*it).second.Current) bestC=(*it).second.Current; bestCName=(*it).first; break;
			case 'H': if (bestH<=(*it).second.Current) bestH=(*it).second.Current; bestHName=(*it).first; break;
		}
		job->freqTblAddEntry("all_skills",NLMISC::toString("%s,%d",(*it).first.c_str(),(*it).second.Current));
	}
	if (bestF>= bestM && bestF>=bestH && bestF>=bestC)		{ best=bestF; bestName=bestFName; }
	else if (bestM>= bestF && bestM>=bestH && bestM>=bestC)	{ best=bestM; bestName=bestMName; }
	else if (bestH>= bestF && bestH>=bestM && bestH>=bestC)	{ best=bestH; bestName=bestHName; }
	else 												   	{ best=bestC; bestName=bestCName; }

	job->charTblSetEntry("skill_best",NLMISC::toString(best));	// job->charTblSetEntry("name_best",bestName);
	job->charTblSetEntry("skill_sf",NLMISC::toString(bestF));	// job->charTblSetEntry("name_sf",bestFName);
	job->charTblSetEntry("skill_sm",NLMISC::toString(bestM));	// job->charTblSetEntry("name_sm",bestMName);
	job->charTblSetEntry("skill_sc",NLMISC::toString(bestC));	// job->charTblSetEntry("name_sc",bestCName);
	job->charTblSetEntry("skill_sh",NLMISC::toString(bestH));	// job->charTblSetEntry("name_sh",bestHName);

	job->freqTblAddEntry("best_skill",NLMISC::toString(best));
	job->freqTblAddEntry("best_skill_sf",NLMISC::toString(bestF));
	job->freqTblAddEntry("best_skill_sm",NLMISC::toString(bestM));
	job->freqTblAddEntry("best_skill_sc",NLMISC::toString(bestC));
	job->freqTblAddEntry("best_skill_sh",NLMISC::toString(bestH));
}

INFO_EXTRACTOR(PlayerRoomAndPets,"The Id of the room and number of pets owned by the character","room,petSheet0,petPos0,petState0,petSheet1,petPos1,petState1,petSheet2,petPos2,petState2,petSheet3,petPos3,petState3")
{
	job->charTblSetEntry("room",NLMISC::toString(c->_PlayerRoom.Building));

	for (uint32 i=0;i<4;++i)
	{
		if (c->_PlayerPets.find(i)==c->_PlayerPets.end())
			continue;
		const CStatsScanPetAnimal& pet= c->_PlayerPets.find(i)->second;
		if (pet.PetSheetId!=NLMISC::CSheetId::Unknown)
		{
			job->charTblSetEntry("petSheet"+NLMISC::toString("%u",i),pet.PetSheetId.toString());
			job->charTblSetEntry("petPos"+NLMISC::toString("%u",i),NLMISC::toString("(%u %u %u) / %s",pet.Landscape_X,pet.Landscape_Y,pet.Landscape_Z,NLMISC::toString(pet.StableAlias).c_str()));
			job->charTblSetEntry("petState"+NLMISC::toString("%u",i),NLMISC::toString("%u / %u",pet.PetStatus,pet.DeathTick));

			job->freqTblAddEntry("stables",NLMISC::toString(pet.StableAlias));
		}
	}
}

INFO_EXTRACTOR(PlayerRoomStats,"The stats of number of players who have access to each room type","")
{
//	job->freqTblAddEntry("room",NLMISC::toString(c->_PlayerRoom.Building));
	job->freqTblAddEntry("room",NLMISC::CSheetId(c->_PlayerRoom.Building).toString());
}

INFO_EXTRACTOR(RespawnPointCount,"Number of respawn points","respawnPointCount")
{
	job->charTblSetEntry("respawnPointCount",NLMISC::toString(c->RespawnPoints.RespawnPoints.size()));
}

INFO_EXTRACTOR(RespawnPointStats,"Stats on number of players who have access to each of the respawn points","")
{
	for (uint32 i=0;i<c->RespawnPoints.RespawnPoints.size();++i)
	{
		job->freqTblAddEntry("respawn_points",c->RespawnPoints.RespawnPoints[i]);
	}
}

INFO_EXTRACTOR(StanzaStats,"Stats on number of players who know each stanza","")
{
	for (uint32 i=0;i<c->_KnownBricks.size();++i)
	{
		job->freqTblAddEntry("stanzas",c->_KnownBricks[i].toString());
	}
}

INFO_EXTRACTOR(VisPropStats,"Stats on race and visual props","")
{
	job->freqTblAddEntry("race_sex",c->EntityBase._Race+(c->EntityBase._Gender==0?"_Male":"_Female"));
	job->freqTblAddEntry("hair",c->EntityBase._Race+(c->EntityBase._Gender==0?"_Male_":"_Female_")+NLMISC::toString(c->HairType));
	job->freqTblAddEntry("tattoo",c->EntityBase._Race+(c->EntityBase._Gender==0?"_Male_":"_Female_")+NLMISC::toString(c->Tattoo));
}

INFO_EXTRACTOR(FactionPoints,"The character's faction points","factPts")
{
	std::string BestFaction;
	uint32 MaxFP = 0;

	std::map<NLMISC::CSString,uint32>::const_iterator it = c->FactionPoints.begin();
	while(it != c->FactionPoints.end())
	{
		if((*it).second > MaxFP)
		{
			MaxFP = (*it).second;
			BestFaction = (*it).first;
		}
		it++;
	}
/*
	for(uint32 i = 0; i < (PVP_CLAN::EndClans-PVP_CLAN::BeginClans+1); ++i)
	{
		std::string Faction = PVP_CLAN::toString((PVP_CLAN::TPVPClan)(i+PVP_CLAN::BeginClans));
		if(c->FactionPoints[Faction] > MaxFP)
		{
			MaxFP = c->FactionPoints[i];
			BestFaction = PVP_CLAN::toString((PVP_CLAN::TPVPClan)(i+PVP_CLAN::BeginClans));
		}
	}		
*/
	job->charTblSetEntry("factPts", NLMISC::toString(MaxFP));
	job->freqTblAddEntry("best_faction", BestFaction);
}
