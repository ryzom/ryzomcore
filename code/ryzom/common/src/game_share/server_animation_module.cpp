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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include "stdpch.h"

#include "server_animation_module.h"

#include "nel/net/unified_network.h"
#include "nel/net/module_message.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/string_conversion.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive_utils.h"


#include "game_share/utils.h"
#include "game_share/persistent_data.h"
#include "game_share/chat_group.h"
#include "game_share/misc_const.h"
#include "game_share/ring_session_manager_itf.h"

#include "game_share/object.h"
#include "game_share/scenario.h"
#include "game_share/r2_messages.h"
#include "game_share/r2_ligo_config.h"
#include "game_share/scenario_entry_points.h"
#include "game_share/ring_access.h"

// TEMP
//#include "r2_modules/server_edition_module.h"
//#include "r2_modules/server_admin_module.h"

#include "ai_wrapper.h"
#include "dms.h"





using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;
using namespace R2;

// Debug
CVariable<bool>	WriteScenarioDebugDataToFile( "DSS", "WriteScenarioDebugDataToFile", "Set to true to write scenario rtdata and xml primitive to file", false, 0, true );
CVariable<uint32> DelayBeforeStartAct ("DSS", "DelayBeforeStartAct", "The delay wait by player/dm between the moment a dm/ai has decided the change of act and the moment of the real change", 30, 0, true );
CVariable<uint32> DelayBeforeNewLocation ("DSS", "DelayBeforeNewLocation", "The min delay between 2 change of location", 25, 0, true );
CVariable<bool>	 UseSheetClientWithLevel( "DSS", "UseSheetClientWithLevel", "use sheet client like basic_fyros_male_f2.creature instead of basic_fyros_male_f2.creature", true, 0, true );
//std::vector<CPersistentDataRecordRyzomStore> Pdrs;
static CPersistentDataRecordRyzomStore		Pdr;

NLNET_REGISTER_MODULE_FACTORY(CServerAnimationModule, "ServerAnimationModule");

namespace R2
{

class CRtNpc :  public NLMISC::CRefCount
{
public:
	CRtNpc(TAIAlias alias, CObject*  objectData, TAIAlias grpAlias, uint32 dmProperty)
	{
		Alias = alias;
		ObjectData = objectData;
		GrpAlias = grpAlias;
		NpcAnimationProp = dmProperty;
		NameId = 0;
		Alived = true;
	}

	NLMISC::CEntityId EntityId;
	TAIAlias Alias;
	TAIAlias GrpAlias;
	TDataSetRow DataSetRow;
	CObject*  ObjectData;
	uint32 NpcAnimationProp;
	uint32 NameId;
	bool Alived;

};


class CRtGrp:  public NLMISC::CRefCount
{
public:
	CRtGrp(TAIAlias alias, CObject*  objectData, const std::string& fullName)
	{
		Alias = alias;
		ObjectData = objectData;
		FullName = fullName;
		//EntityId = NLMISC::CEntityId::Unknown; (default)

	}

	NLMISC::CEntityId EntityId;
	TAIAlias Alias;
	std::string FullName;
	CObject*  ObjectData;
};


class CRtUserTrigger
{
public:
	std::string FullName;
	std::string Grp;
	std::string Name;
};


class CRtAct : public NLMISC::CRefCount
{
public:
	typedef std::map<TAIAlias, NLMISC::CSmartPtr<R2::CRtNpc> > TRtNpcs;
	typedef std::map<TAIAlias, NLMISC::CSmartPtr<R2::CRtGrp> > TRtGrps;
	typedef std::vector<CRtUserTrigger> TRtUserTriggers;
	typedef uint32 TEggId;
	typedef std::string TFullGrpName;
	typedef std::map<TEggId, TFullGrpName> TActiveEasterEggs;

public:
	R2::CRtGrp* getRtGrpByName(const std::string& name) const;
	void addUserTrigger(const CRtUserTrigger& userTrigger);
	bool activateEasterEgg(uint32 easterEggId, const std::string & grpControler)
	{


		TRtGrps::const_iterator first(RtGrps.begin()), last(RtGrps.end());
		for ( ; first != last && first->second->FullName != grpControler ; ++first)	{ }

		if ( first == last)
		{
			nlwarning("SAM: Error while activating easter egg. The controler grp is not valid.");
			return false;
		}
		return ActiveEasterEggs.insert(std::make_pair(easterEggId, grpControler)).second;
	}
	bool deactivateEasterEgg(uint32 easterEggId);

public:
	TRtNpcs RtNpcs;
	TRtGrps RtGrps;
	uint16 WeatherValue; // 0 for auto weather
	//uint8  Season; // 0 for auto season
	uint32 LocationId;
	std::string Name;
	TRtUserTriggers UserTriggers;
	TActiveEasterEggs ActiveEasterEggs;
	std::string ActDescription;
	std::string PreActDescription;



};

bool CRtAct::deactivateEasterEgg(uint32 easterEggId)
{
	TActiveEasterEggs::iterator found(ActiveEasterEggs.find(easterEggId));
	if (found == ActiveEasterEggs.end()) return false;
	ActiveEasterEggs.erase(found);
	return true;
}


R2::CRtGrp* CRtAct::getRtGrpByName(const std::string& name) const
{
	TRtGrps::const_iterator first(RtGrps.begin()), last(RtGrps.end());
	for ( ; first != last ; ++first)
	{
		CObject* object = first->second->ObjectData;
		if ( object && object->isString("Id") )
		{
			std::string data = object->toString("Id");
			if (data == name) { return first->second; }
		}
	}
	return 0;
}

void CRtAct::addUserTrigger(const CRtUserTrigger& userTrigger)
{

	CRtGrp * rtGrp = getRtGrpByName(userTrigger.Grp);
	if (rtGrp)
	{
		UserTriggers.push_back(userTrigger);
		UserTriggers.back().FullName = rtGrp->FullName;
	}
	else
	{
		nlwarning("Try to create undef user trigger '%s'", userTrigger.Grp.c_str());
	}
}

class CRtLocation
{
public:
	uint8 Season;
	std::string Island;
	std::string EntryPoint;
};


class CBotControler
{
public:
	typedef NLMISC::CTwinMap< uint32, NLMISC::CEntityId >  TControlledEntities;
	typedef std::map <uint32, TControlledEntities > TControlledEntitiesList; //

public:
	CBotControler(){}

	bool add(const NLMISC::CEntityId& eid, const NLMISC::CEntityId& creatureId);

	bool remove(const NLMISC::CEntityId& eid, const NLMISC::CEntityId& creatureId);

	NLMISC::CEntityId getEntity(const NLMISC::CEntityId& eid, uint32 id) const;

	void getBots(const NLMISC::CEntityId& eid, std::map<uint32, NLMISC::CEntityId>&  bots);
	void getBots(uint32 playerId, std::map<uint32, NLMISC::CEntityId>&  bots);

	void removeChar(const NLMISC::CEntityId& eid);

	uint32 getBotsCount(uint32 charId) const;


private:
	TControlledEntitiesList _List;

};

void CBotControler::getBots(const NLMISC::CEntityId& eid, std::map<uint32, NLMISC::CEntityId>&  bots)
{
	TCharId charId = static_cast<TCharId>(eid.getShortId());
	getBots(charId, bots);

}


void CBotControler::getBots(uint32 charId, std::map<uint32, NLMISC::CEntityId>&  bots)
{
	TControlledEntitiesList::iterator found(_List.find(charId));
	if ( found == _List.end()) { return; }
	const std::map<uint32, NLMISC::CEntityId>&  entities =found->second.getAToBMap();
	bots = entities;


}

uint32 CBotControler::getBotsCount(uint32 charId) const
{
	TControlledEntitiesList::const_iterator found(_List.find(charId));
	if ( found == _List.end()) { return 0; }
	const std::map<uint32, NLMISC::CEntityId>&  entities =found->second.getAToBMap();
	return (uint32)entities.size();
}


void CBotControler::removeChar(const NLMISC::CEntityId& eid)
{
	TCharId charId = static_cast<TCharId>(eid.getShortId());
	TControlledEntitiesList::iterator found(_List.find(charId));
	if ( found != _List.end()) { _List.erase(found); }
}


bool CBotControler::add(const NLMISC::CEntityId& eid, const NLMISC::CEntityId& creatureId)
{

	TCharId charId = static_cast<TCharId>(eid.getShortId());
	TControlledEntitiesList::iterator found = _List.find(charId);
	if (found == _List.end())
	{
		found = _List.insert( std::make_pair(charId, CTwinMap<uint32,NLMISC::CEntityId >() ) ).first;
	}
	TControlledEntities & twin = found->second;
	const uint32* id = twin.getA(creatureId);
	if ( id ) { return false; }
	const std::map<uint32, NLMISC::CEntityId> & entities = twin.getAToBMap();
	uint32 last = 1;
	if (!entities.empty())
	{
		uint32 f = entities.rbegin()->first;
		last += f;
	}
	twin.add(last, creatureId);

	return true;

}

bool CBotControler::remove(const NLMISC::CEntityId& eid, const NLMISC::CEntityId& creatureId)
{
	TCharId charId = static_cast<TCharId>(eid.getShortId());
	TControlledEntitiesList::iterator found = _List.find(charId);
	if (found == _List.end())
	{
		return false;
	}
	TControlledEntities & twin = found->second;
	const uint32* id = twin.getA(creatureId);
	if ( !id ) { return false; }
	twin.removeWithB(creatureId);
	if (twin.getAToBMap().empty()){ _List.erase(found);	}
	return true;

}

NLMISC::CEntityId CBotControler::getEntity(const NLMISC::CEntityId& eid, uint32 id) const
{
	TCharId charId = static_cast<TCharId>(eid.getShortId());
	TControlledEntitiesList::const_iterator found = _List.find(charId);
	// As the player disconnected?
	if (found == _List.end()) { return NLMISC::CEntityId(); }
	const NLMISC::CTwinMap<uint32, NLMISC::CEntityId> & twin = found->second;
	const NLMISC::CEntityId * entity = twin.getB(id);
	if (!entity) { return NLMISC::CEntityId(); }
	return *entity;
}



struct TCharacterInfo
{
public:
	NLMISC::CEntityId TargetId;
};

class CAnimationSession
{
public:
	typedef std::vector< NLMISC::CSmartPtr<CRtAct> >  TActs;
	typedef std::vector<CRtLocation> TLocations;
	typedef uint32 TActId;
	typedef uint32 TEasterEggId;
	typedef  std::map<TEasterEggId, TActId>  TActiveEasterEggs;
	typedef uint32 TAiInstance;
	typedef std::map<uint32, TCharacterInfo> TCharacterInfos;

public:
	std::auto_ptr<CObject> RtData;
	TScenarioHeaderSerializer ScenarioHeader;
	TSessionId SessionId;
	//vector<userId>
	std::vector<uint32> ConnectedChars;
	std::vector<CPersistentDataRecordRyzomStore> Pdrs;
	TActs Acts;

	uint32 CurrentAct; //if 0 <=> no act
	uint16 WeatherValue; // current weather in the [1, 1023] range, or 0 for auto weather
	                   // weather value can come from the scenario description, or may have been changed by an animator
	uint8	CurrSeason; // current season or ~0 for no season set yet
	bool StartingAct; // 1 if a player/ia has schedule a startAct
	uint32 InitialAct; //act at which begin the scenario (1) for animation currentAct for test

	uint32			ScenarioScore;
	NLMISC::TTime	ScenarioTime;
	bool			TimingIsFinished;


	std::vector<TMissionItem> MissionItems;

	TLocations Locations;
	TActiveEasterEggs ActiveEasterEggs;

	CBotControler	IncarningBots;
	CBotControler	TalkingAsBots;
	TAiInstance		AiInstance;
	NLMISC::TTime  DateOfLastNewLocation;
	sint32 InitialX;
	sint32 InitialY;
	uint8 InitialSeason;
	std::set<uint32> CharacternValidePosition;
	bool	InitialTp;
	TCharacterInfos CharacterInfos;

public:
	CAnimationSession()
	{
		WeatherValue = 0; // auto weather
		CurrSeason = std::numeric_limits<uint8>::max();
		StartingAct = false;
		InitialAct = 1;
		AiInstance = std::numeric_limits<uint32>::max(); //wrong value
		DateOfLastNewLocation = 0;

		InitialX = 0;
		InitialY = 0;
		InitialSeason = 0;
		InitialTp = false;

		TimingIsFinished = false;
		ScenarioScore = 0;
	}

	void setStartParams(sint32 x, sint32 y, uint8 season) { InitialX = x; InitialY = y; InitialSeason = season; }

	void setWeatherValue(uint16 value);
	void setSeason(uint8 value);

	void sendWeatherValueToChar(uint32 charId, uint16 weatherValue) const;

	void sendSeasonToChar(uint32 charId, uint8 season) const;

	uint32 getActCount() const { return (uint32)Pdrs.size(); }

	void updateUserTriggerDescriptions(TUserTriggerDescriptions& userTriggerDescriptions);

	void updateActPositionDescriptions(TActPositionDescriptions& actPositionDescriptions);

	void easterEggLooted(uint32 easterEggId, TSessionId scenarioId);



};

void CAnimationSession::easterEggLooted(uint32 easterEggId, TSessionId /* scenarioId */)
{

	TActiveEasterEggs::const_iterator found = ActiveEasterEggs.find(easterEggId);
	DROP_IF(found == ActiveEasterEggs.end(), "Error try to loot an unactive easter egg" ,return);
	uint32 actId = found->second;
	DROP_IF(actId >=  Acts.size(), "Error try to loot an unactive easter egg", return);
	CRtAct* rtAct = Acts[actId];

	CRtAct::TActiveEasterEggs::const_iterator found2 = rtAct->ActiveEasterEggs.find(easterEggId);
	DROP_IF(found2 == rtAct->ActiveEasterEggs.end(), "Error try to loot an unactive easter egg", return);
	std::string fullname = found2->second;

	CAiWrapper::getInstance().triggerUserTrigger(fullname, 2);


}


void CAnimationSession::setWeatherValue(uint16 weatherValue)
{
	WeatherValue = weatherValue;
	// update weather on EGS for all players connected to that map
	for (uint k = 0; k < ConnectedChars.size(); ++k)
	{
		sendWeatherValueToChar(ConnectedChars[k], weatherValue);
	}
}

void CAnimationSession::setSeason(uint8 season)
{
	if (season == CurrSeason) return;
	CurrSeason = season;
	// update season on EGS for all players connected to that map
	for (uint k = 0; k < ConnectedChars.size(); ++k)
	{
		sendSeasonToChar(ConnectedChars[k], season);
	}
}

void CAnimationSession::sendWeatherValueToChar(uint32 charId, uint16 weatherValue) const
{

	CMessage msgout("SET_PLAYER_WEATHER");
	msgout.serial(charId);
	uint16 weatherValue16 = (uint16) weatherValue;
	msgout.serial(weatherValue16);
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}

void CAnimationSession::sendSeasonToChar(uint32 charId, uint8 season) const
{

	CMessage msgout("SET_PLAYER_SEASON");
	msgout.serial(charId);
	msgout.serial(season);
	CUnifiedNetwork::getInstance()->send("EGS", msgout);
}

void CAnimationSession::updateUserTriggerDescriptions(TUserTriggerDescriptions& userTriggerDescriptions)
{
	userTriggerDescriptions.clear();

	//Act0 trigger
	uint32 actIndex = 0;
	uint32 actCount = (uint32)Acts.size();
	for ( ; actIndex != actCount; ++actIndex)
	{
		CRtAct* act = Acts[actIndex];
		uint32 userTriggerCount = (uint32)act->UserTriggers.size();
		uint32 index = 0;
		for ( ; index != userTriggerCount; ++index)
		{
			TUserTriggerDescription userTriggerDescription;
			userTriggerDescription.Name = act->UserTriggers[index].Name;
			userTriggerDescription.Act = actIndex;
			userTriggerDescription.Id = index;
			userTriggerDescriptions.push_back(userTriggerDescription);
		}
	}
}



void CAnimationSession::updateActPositionDescriptions(TActPositionDescriptions& actPositionDescriptions)
{
	actPositionDescriptions.clear();

	uint32 actIndex = 0;
	uint32 actCount = (uint32)Acts.size();
	for ( ; actIndex != actCount; ++actIndex)
	{
		CRtAct* act = Acts[actIndex];
		TActPositionDescription actPositionDescription;
		if ( act->LocationId <= this->Locations.size())
		{
			actPositionDescription.Name = act->Name;
			actPositionDescription.Season = this->Locations[act->LocationId ].Season;
			actPositionDescription.Island = this->Locations[ act->LocationId].Island;
			actPositionDescription.LocationId = act->LocationId;
		}
		else
		{
			actPositionDescription.Name = act->Name;
			actPositionDescription.Season = 0;
			actPositionDescription.Island = "";
			actPositionDescription.LocationId = 0;
		}
		actPositionDescriptions.push_back(actPositionDescription);
	}
}


class CAttributeToProperty
{
public:
	CAttributeToProperty(CObject* object, IPrimitive* prim)
	{
		_Object = object;
		_Primitive = prim;
	}

	void setAttributeAsString(const std::string & attrName, const std::string& propName)
	{
		CObject* attr = _Object->getAttr(attrName);
		if (attr && attr->isString())
		{
			std::string str = attr->toString();
			_Primitive->addPropertyByName(propName.c_str(), new CPropertyString(str));
		}
	}

	void setAttributeAsStringWithPrefix(const std::string & attrName, const std::string& propName, const std::string& prefix)
	{
		CObject* attr = _Object->getAttr(attrName);
		if (attr && attr->isString())
		{
			std::string str = attr->toString();
			_Primitive->addPropertyByName(propName.c_str(), new CPropertyString(prefix+str));
		}
	}


	void setAiStateName( const std::string& prefix);

	void setAttributeAsStringArray(const std::string & attrName, const std::string& propName);

	void setAttributeAsStringArrayWithPrefix(const std::string & attrName, const std::string& propName, const std::string& prefix)
	{
		CObject* attr = _Object->getAttr(attrName);
		if (attr && attr->isString())
		{
			string str = attr->toString();
			vector<string> result;
			NLMISC::splitString(str, "\n", result);

			uint32 first = 0, last = (uint32)result.size();
			for ( ; first != last ; ++first)
			{
				result[first] = prefix + result[first] ;
			}

			_Primitive->addPropertyByName(propName.c_str(), new CPropertyStringArray(result));
		}
	}



	void setAttributeAsBool(const std::string & attrName, const std::string& propName)
	{
		CObject* attr = _Object->getAttr(attrName);
		if (attr && attr->isNumber())
		{
			sint value = static_cast<sint>(attr->toNumber());
			_Primitive->addPropertyByName(propName.c_str(), new CPropertyString(value?"true":"false"));
		}
	}

	void setPrimPoint()
	{
		CObject* attr = 0;
		CPrimPoint* point = dynamic_cast<CPrimPoint*>(_Primitive);

		nlassert(point);
		CObject* pt = _Object->getAttr("Pt");
		if (pt)
		{
			attr = pt->getAttr("x");
			if (attr && attr->isNumber()) {	point->Point.x = static_cast<float>(attr->toNumber()); 	}

			attr = pt->getAttr("y");
			if (attr && attr->isNumber()) {	point->Point.y = static_cast<float>(attr->toNumber()); 	}

			attr = pt->getAttr("z");
			if (attr && attr->isNumber()) {	point->Point.z = static_cast<float>(0.0); 	}
		}

		attr = _Object->getAttr("Angle");
		if (attr && attr->isNumber()) {	point->Angle =  static_cast<float>(attr->toNumber()); 	}
	}

	void setPrimPath();
	void setPrimZone();

private:
	CObject* _Object;
	IPrimitive* _Primitive;
};


void CAttributeToProperty::setAttributeAsStringArray(const std::string & attrName, const std::string& propName)
{
	CObject* attr = _Object->getAttr(attrName);
	if (attr && attr->isString())
	{
		string str = attr->toString();
		vector<string> result;
		NLMISC::splitString(str, "\n", result);
		_Primitive->addPropertyByName(propName.c_str(), new CPropertyStringArray(result));
	}
}


void CAttributeToProperty::setAiStateName(const std::string& prefix)
{
	std::string ai_movement, ai_activity;
	std::string name;

	CObject* tmp=_Object->getAttr("Id");
	if( !(tmp&&tmp->isString())&&((name = tmp->toString()).length()!=0))
	{
		nlwarning("R2Ani: invalide rtData");
		return;
	}

	_Primitive->addPropertyByName("name", new CPropertyString(prefix + name));

}


void CAttributeToProperty::setPrimPath()
{
	CObject* attr = 0;
	CPrimPath* points = dynamic_cast<CPrimPath*>(_Primitive);
	nlassert(points);
	CObject* pts = _Object->getAttr("Pts");
	if (pts)
	{

		uint32 first(0), last( pts->getSize() );
		points->VPoints.resize(last);
		for ( ; first != last ; ++first )
		{
			CObject* pt = pts->getValue(first);

			attr = pt->getAttr("x");
			if (attr && attr->isNumber()) {	points->VPoints[first].x = static_cast<float>(attr->toNumber()); 	}

			attr = pt->getAttr("y");
			if (attr && attr->isNumber()) {	points->VPoints[first].y = static_cast<float>(attr->toNumber()); 	}

			attr = pt->getAttr("z");
			if (attr && attr->isNumber()) {	points->VPoints[first].z = static_cast<float>(attr->toNumber()); 	}

		}

	}
}


void CAttributeToProperty::setPrimZone()
{
	CObject* attr = 0;
	CPrimZone* points = dynamic_cast<CPrimZone*>(_Primitive);
	nlassert(points);
	CObject* pts = _Object->getAttr("Pts");
	if (pts)
	{

		uint32 first(0), last( pts->getSize() );
		points->VPoints.resize(last);
		for ( ; first != last ; ++first )
		{
			CObject* pt = pts->getValue(first);

			attr = pt->getAttr("x");
			if (attr && attr->isNumber()) {	points->VPoints[first].x = static_cast<float>(attr->toNumber()); 	}

			attr = pt->getAttr("y");
			if (attr && attr->isNumber()) {	points->VPoints[first].y = static_cast<float>(attr->toNumber()); 	}

			attr = pt->getAttr("z");
			if (attr && attr->isNumber()) {	points->VPoints[first].z = static_cast<float>(attr->toNumber()); 	}

		}

	}
}

//--------------------------------------------------------------------------------------------

class CTaskStopTest : public CTask<NLMISC::TTime>
{
public:
	CTaskStopTest(CServerAnimationModule*  animationModule, TSessionId sessionId)
		:_AnimationModule(animationModule), _SessionId(sessionId) { }

	void doOperation()
	{
		uint32 unused;
		_AnimationModule->stopTestImpl(_SessionId, unused);
	}

private:
	CServerAnimationModule*  _AnimationModule;
	TSessionId _SessionId;

};

class CTaskBroadcast : public CTask<NLMISC::TTime>
{
public:
	// Take ownership of the message
	CTaskBroadcast(CServerAnimationModule*  animationModule, TSessionId sessionId, CMessage& msg)
		:_AnimationModule(animationModule), _SessionId(sessionId) { _Msg.swap(msg); }

	void doOperation()
	{
		_AnimationModule->broadcastMsg(_SessionId, _Msg);

	}

private:
	CServerAnimationModule*  _AnimationModule;
	TSessionId _SessionId;
	CMessage _Msg;

};


class CTaskStartAct : public CTask<NLMISC::TTime>
{
public:
	CTaskStartAct(NLMISC::TTime t, CServerAnimationModule*  animationModule, TSessionId sessionId, uint32 actId, bool mustTp)
		: CTask<NLMISC::TTime>(t),
		_AnimationModule(animationModule),
		_SessionId(sessionId),_ActId(actId), _MustTp(mustTp){}
	void doOperation()
	{
		CAnimationSession* session = _AnimationModule->getSession(_SessionId);
		if (session && session->StartingAct)
		{
			session->StartingAct = false;
			_AnimationModule->startAct(_SessionId, _ActId);
			if (_MustTp)
			{
				session->DateOfLastNewLocation = NLMISC::CTime::getLocalTime ();
				session->CharacternValidePosition.clear(); // we invalidate all player;
				std::vector<uint32>::const_iterator first(session->ConnectedChars.begin()), last(session->ConnectedChars.end());
				for ( ; first != last ; ++first)
				{
					session->CharacternValidePosition.insert(*first);
				}



			}

		}
	}
private:
	CServerAnimationModule* _AnimationModule;
	TSessionId _SessionId;
	uint32 _ActId;
	bool _MustTp;

};


class CTaskScheduleStartSession: public CTask<NLMISC::TTime>
{
public:
	CTaskScheduleStartSession( CServerAnimationModule*  animationModule, const CAnimationMessageAnimationStart& msg)
		:_AnimationModule(animationModule), _Msg(msg)
		{}

	void doOperation()
	{
		_AnimationModule->scheduleStartSessionImpl(_Msg);
	}
private:
	CServerAnimationModule* _AnimationModule;
	CAnimationMessageAnimationStart _Msg;

};

//--------------------------------------------------------------------------------------------


void CServerAnimationModule::broadcastMessage(CAnimationSession* session, const NLNET::CMessage& msg)
{
	if (!session->ConnectedChars.empty())
	{
		std::vector<uint32>::const_iterator first(session->ConnectedChars.begin()), last(session->ConnectedChars.end());
		for ( ; first != last ; ++first)
		{
			const NLNET::TModuleProxyPtr* ptr = getEditionModule()->getClientProxyPtr(*first);
			if (ptr)
			{
				(*ptr)->sendModuleMessage(this, msg);
			}
		}
	}
}


void CServerAnimationModule::scheduleStartAct(TSessionId sessionId, uint32 actId)
{
	CAnimationSession* session = getSession(sessionId);
	if (session)
	{
		CMessage msg;
		if (!session->StartingAct)
		{

			if (0<actId && actId < session->getActCount() )
			{
				session->StartingAct = true;

				uint32 currentAct = session->CurrentAct;
				uint32 nextLocationId = session->Acts[actId]->LocationId;
				uint32 currentLoctionId = session->Acts[currentAct]->LocationId;

				DROP_IF( nextLocationId >= session->Locations.size(), "Invalid Location", return);
				DROP_IF( currentLoctionId >= session->Locations.size(), "Invalid Location", return);



				bool mustTp = nextLocationId != currentLoctionId;







				NLMISC::TTime  delay = mustTp
						? 3000	// Tp with jump
						: 1000;	// Normal TP

				if (!session->Acts[actId]->PreActDescription.empty())
				{
					delay = 3000;
				}

				if (session->InitialTp )
				{
					session->InitialTp  = false;
					mustTp = true;
					delay = 0;
				}

				// if act change in the last 20 second add a wait effect.
				if (session->DateOfLastNewLocation != 0)
				{
					NLMISC::TTime now = CTime::getLocalTime();
					NLMISC::TTime wait = now - session->DateOfLastNewLocation;
					if ( wait < NLMISC::TTime(DelayBeforeNewLocation)*1000)
					{
						delay += NLMISC::TTime(DelayBeforeNewLocation)*1000 - wait;
					}

				}
				_Tasks.addTask( new CTaskStartAct( NLMISC::CTime::getLocalTime () + delay, this, sessionId, actId, mustTp));
				if (mustTp)
				{
					broadcastMessage(session, CShareClientEditionItfProxy::buildMessageFor_scheduleStartAct(msg, 0, actId, uint32(delay/1000)) );
				}

				if (!session->Acts[actId]->PreActDescription.empty())
				{
					NLNET::CMessage msg;
					CShareClientEditionItfProxy::buildMessageFor_systemMsg(msg, "BC_ML", "", session->Acts[actId]->PreActDescription);
					_Tasks.addTaskAt(NLMISC::CTime::getLocalTime () + delay  - 3000 , new CTaskBroadcast(this, sessionId, msg));
				}

			}
			else
			{
				broadcastMessage(session, CShareClientEditionItfProxy::buildMessageFor_scheduleStartAct(msg, 2, actId, DelayBeforeStartAct) );
			}

		}
		else
		{
			broadcastMessage(session, CShareClientEditionItfProxy::buildMessageFor_scheduleStartAct(msg, 1, actId, DelayBeforeStartAct) );
		}
	}
}

void CServerAnimationModule::init(NLNET::IModuleSocket* gateway, CDynamicMapService* server)
{
	_Server = server;

	CAiWrapper::getInstance().init( CPrimitiveContext::instance().CurrentLigoConfig);
	this->plugModule(gateway);
	_Emotes.get("");//force lazy initialization

	// check if the AI service is up
	const std::vector<TServiceId> &connectionList = CUnifiedNetwork::getInstance()->getConnectionList();
	for( uint i = (uint)connectionList.size(); i > 0; --i )
	{
		nldebug( "R2An: %s is already up", CUnifiedNetwork::getInstance()->getServiceName(connectionList[i-1]).c_str() );
		if( "AIS" == CUnifiedNetwork::getInstance()->getServiceName(connectionList[i-1]) )
		{
			_ReadyForNextSession = true;
		}
	}
}

CServerAnimationModule::CServerAnimationModule() :
	_ReadyForNextSession( false )
{
		CServerAnimationItfSkel::init(this);
		CShareServerAnimationItfSkel::init(this);
	_Server = 0;
}

CServerAnimationModule::~CServerAnimationModule()
{
	while( !_QueuedSessions.empty() )
	{
		CAnimationSession *animSession = _QueuedSessions.front();
		_QueuedSessions.pop_front();
		delete animSession;
	}
}


IPrimitive* CServerAnimationModule::getAction(CObject* action, const std::string& prefix,TSessionId scenarioId)
{
	if (!action)
	{
		nlwarning("Error while generating primitives in scenario '%u'",  scenarioId.asInt());
		return 0;
	}
	IPrimitive* pAction= dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimNode"));
	pAction->addPropertyByName("class", new CPropertyString("npc_event_handler_action"));
	pAction->addPropertyByName("ai_type", new CPropertyString("NPC_EVENT_ACTION"));	// AJM
	pAction->addPropertyByName("weight", new CPropertyString("1"));	// AJM
	CAttributeToProperty a2pAction(action, pAction);
	a2pAction.setAttributeAsString("Action","action");
	a2pAction.setAttributeAsString("Action","name");	// AJM: default value for name <- action
	CObject* tmp=action->getAttr("Action");
	if(tmp&&tmp->isString()&&tmp->toString()=="npc_say")
	{
		CObject* tmp2 = action->getAttr("Parameters");
		if(tmp2&&tmp2->isString())
		{
			NLMISC::CSString param (tmp2->toString());
			NLMISC::CVectorSString result;

			if(param.splitLines(result) )
			{
				if (result.size() == 0)
				{
					nlwarning("ERROR: npc_say but no parameters !!! %d in session ", scenarioId.asInt());
				}

				if(result.size()>=1)
				{
					NLMISC::CSString name(result[0]);
					if(name.find(":",0)!=string::npos)
					{
						result[0] = prefix+result[0];
					}
				}

				if(result.size()>=2)
					result[1] = "DSS_"+toString(scenarioId)+" "+result[1];
				if(result.size()>=3)
				{
					for(uint32 i=2;i<result.size();++i)
					{
						result[i] = prefix + result[i];
					}
				}
				pAction->addPropertyByName("parameters", new CPropertyStringArray( (std::vector<std::string> &)result ));
			}

		}
	}
	else
	{
		if (!action->isString("Action") )
		{
			nlwarning("Invalid rtData: no action found");
			return 0;
		}
		CSString caction(action->toString("Action"));
		if (caction == "begin_state" || caction.left(14)=="trigger_event_" || caction == "facing")
		{
			a2pAction.setAttributeAsStringArrayWithPrefix("Parameters","parameters", prefix);
		}
		else if (caction == "emot")
		{
			std::string attrName("Parameters"), propName("parameters");

			CObject* attr = action->getAttr(attrName);
			if (attr && attr->isString())
			{
				string str = attr->toString();
				vector<string> result;
				NLMISC::splitString(str, "\n", result);

				uint32 first = 0, last = (uint32)result.size();
				for ( ; first != last ; ++first)
				{
					if (first == 0)
					{
						result[first] = _Emotes.get(result[first]);
					}
					else
					{
						result[first] = prefix + result[first] ;
					}
				}

				pAction->addPropertyByName(propName.c_str(), new CPropertyStringArray(result));
			}

		}
		else if(caction == "modify_variable" || caction=="condition_if" || caction=="condition_if_else" || caction == "dynamic_if")
		{
			CSString str = action->toString("Parameters");
			if(str.find(":")!=string::npos)
			{
				a2pAction.setAttributeAsStringArrayWithPrefix("Parameters","parameters", prefix);
			}
			else
			{

				a2pAction.setAttributeAsStringArray("Parameters","parameters");
			}
		}
		else
		{
			a2pAction.setAttributeAsStringArray("Parameters","parameters");
		}
	}


	CObject* weight = action->getAttr("Weight");
	if(weight)
	{
		uint32 w = (int)weight->toNumber();
		std::string weightStr = toString(w);
		pAction->addPropertyByName("Weight", new CPropertyString(weightStr));
	}


	CObject* children= action->getAttr("Children");
	uint32 nb =children->getSize();
	for(uint32 i=0;i<nb;++i)
	{
		IPrimitive* tmp=getAction(children->getValue(i), prefix,scenarioId);
		if (!tmp)
		{
			nlwarning("Error in action %s nb: %u", action->toString("Name").c_str(), i);
			return 0;
		}
		pAction->insertChild(tmp);
	}

	return pAction;
}

bool CServerAnimationModule::queueSession(CAnimationSession* session, bool /* runTest */)
{
	// write scenario rtdata to a file for debugging
	try
	{
		if( WriteScenarioDebugDataToFile )
		{
			// binary
			COFile output;
			output.open("outpout.rt.bin");
			CObjectSerializerServer serializer( session->RtData.get());
			serializer.serial(output);
			output.flush();
			output.close();
		}

		if( WriteScenarioDebugDataToFile )
		{
			// text
			COFile output;
			//std::stringstream ss;
			std::string ss;
			output.open("outpout.rt");
			session->RtData->serialize(ss);
			output.serialBuffer((uint8*)ss.c_str(), (uint)ss.size());
			output.flush();
			output.close();
		}
	}
	catch (const std::exception& e )
	{
		 nlwarning("Exception while writing debug files %s", e.what() );
	}
	catch(...)
	{
		nlwarning("Undefined Exception while writing debug files");
	}


	{
		requestLoadTable(session);
	}
	//std::vector<CPersistentDataRecord> pdrs;
	session->Pdrs.clear();


	if (!makeAnimationSession(session, true))
	{

		nlwarning("R2An: Invalid Session");
		return false;
	}

	return true;
}


void CServerAnimationModule::startTest(TSessionId sessionId, CPersistentDataRecord& pdr)
{

	CAnimationSession* session = getSession(sessionId);
	if (!session) { return ; }
	uint32 aiInstance = session->AiInstance;

	nlinfo( "R2An: startTest for sessionId %u in ai instance %u", sessionId.asInt(), aiInstance );


	if (! _CharacterControlProxy.isNull() )
	{
		CCharacterControlItfProxy proxy(_CharacterControlProxy);
  		proxy.sendItemDescription(this,  sessionId, session->MissionItems);
	}
	if (_IOSRingProxy != NULL)
	{
		CIOSRingItfProxy proxy(_IOSRingProxy);
		vector<TCharMappedInfo>	itemInfos;
		for (uint i=0; i<session->MissionItems.size(); ++i)
		{
			R2::TCharMappedInfo itemInfo;
			itemInfo.setItemSheet(session->MissionItems[i].SheetId);
			itemInfo.setName(session->MissionItems[i].Name);
			itemInfos.push_back(itemInfo);
		}
		proxy.storeItemNamesForAIInstance(this, aiInstance, itemInfos);
	}
	//Use initial sessionId in case of linked session
	CAiWrapper::getInstance().startTest(session->SessionId, aiInstance, pdr);
}


void CServerAnimationModule::startAct(TSessionId sessionId, uint32 actId)
{
 	nlinfo( "R2An: startAct for sessionId %u", sessionId.asInt() );
	bool isLocal = !_Server->useNetwork();


	CAnimationSession* animSession = getSession(sessionId);
	DROP_IF(!animSession, "Invalid Session", return);
	uint32 aiInstance = animSession->AiInstance;
	DROP_IF(actId < 1 ||  animSession->Pdrs.size() <= actId, "Invalid Act", return);


	if ( !isLocal )
	{
		deactivateEasterEggsFromAct(sessionId, actId);
	}


	uint32 currentAct = animSession->CurrentAct;
	uint32 nextLocationId = animSession->Acts[actId]->LocationId;
	uint32 currentLoctionId = animSession->Acts[currentAct]->LocationId;
	bool mustTp = nextLocationId != currentLoctionId;
	if (animSession->InitialTp )
	{
		animSession->InitialTp  = false;
		mustTp = true;
	}
	animSession->CurrentAct = actId;
	// update entry point
	IServerEditionModule *svEditionModule = _Server->getEditionModule(); // note: in the future the modules could be in two distinct process, thus they would need an interface/proxy communication
	BOMB_IF(!svEditionModule, "Server edition module not found", return); // Can not happend

	CScenario *scenarioSession = svEditionModule->getScenarioById( sessionId );
	BOMB_IF(!scenarioSession, NLMISC::toString("Scenario Session not found for session %u", sessionId.asInt()) , return);
	CObject* scenario = scenarioSession->getHighLevel();

	if (!scenario)
	{
		nlwarning("ERROR that previously lead to crash");
		CAnimationSession* session = getSession(sessionId);
		if (session)
		{
			for(uint32 i = 0; i < session->ConnectedChars.size(); ++i)
			{
				nlwarning("Error: connected user %u", session->ConnectedChars[i]);
			}
		}


		if ( scenarioSession->getRtData())
		{
			nlwarning("ERROR: The corrupted scenario has RTDATA but no HL.");
		}

		nlwarning("ERROR: you must check the backup : session_%d_?.r2", sessionId.asInt());
		BOMB( NLMISC::toString("BIG ERROR: Scenario was not found for session %u, it previously lead to dss crash", sessionId.asInt()), return);
		return;
	}


	double x, y, orient;
	uint8 season;
	_Server->getAnimationModule()->getPosition( sessionId, x, y, orient, season );

	CFarPosition entryPoint;
	entryPoint.SessionId = sessionId;
	entryPoint.PosState.X = (sint32)(x*1000.0);
	entryPoint.PosState.Y = (sint32)(y*1000.0);
	entryPoint.PosState.Z = 0; // ??
	entryPoint.PosState.Heading = 0; // ??

	if (!isLocal)
	{
		if (animSession->WeatherValue != animSession->Acts[actId]->WeatherValue)
		{
			animSession->setWeatherValue(animSession->Acts[actId]->WeatherValue);
		}
		uint32 location = animSession->Acts[actId]->LocationId;
		season =  animSession->Locations[ location ].Season;
		animSession->setSeason(season);
	}

	{

		NLNET::CMessage msg;
		CShareClientEditionItfProxy::buildMessageFor_scheduleStartAct(msg, 0, actId, 0) ;
		_Tasks.addTaskAt(NLMISC::CTime::getLocalTime () + 3000 , new CTaskBroadcast(this, sessionId, msg));
	}


	if (!animSession->Acts[actId]->ActDescription.empty())
	{
		NLNET::CMessage msg;
		CShareClientEditionItfProxy::buildMessageFor_systemMsg(msg, "BC_ML", "", animSession->Acts[actId]->ActDescription);
		_Tasks.addTaskAt(NLMISC::CTime::getLocalTime () + 3000 , new CTaskBroadcast(this, sessionId, msg));
	}

	// send position to connected users
	CAnimationSession* session = getSession(sessionId);
	BOMB_IF(!session, NLMISC::toString( "Session not found for session %u", sessionId.asInt() ), return);
	for(uint32 i = 0; i < session->ConnectedChars.size(); ++i)
	{
		uint32 charId = session->ConnectedChars[i];

		const NLNET::TModuleProxyPtr * pUserModuleProxy = getEditionModule()->getClientProxyPtr(charId);
		if (pUserModuleProxy)
		{
			if ( !_CharacterControlProxy.isNull())
			{
				CCharacterControlItfProxy ccip( _CharacterControlProxy );
				ccip.setUserCharActPosition(this, charId, entryPoint, season);
			}
			addPioneer(sessionId, charId);
			if (mustTp)
			{
				getEditionModule()->tpToEntryPoint(*pUserModuleProxy, actId);
			}
		}
		else
		{
			nlinfo("The user %u has quit the animation session %u before its start", charId, sessionId.asInt());
		}


	}
	// Use Initial session in case of linked act
	CAiWrapper::getInstance().startAct(session->SessionId,  aiInstance, animSession->Pdrs[actId]);
}

void CServerAnimationModule::setWeatherValue(TSessionId sessionId, uint16 weatherValue)
{
	if (!_Server->useNetwork())
	{
		nlwarning("simulate setWeatherValue : %d", (int) weatherValue);
		return;
	}

	CAnimationSession *session = getSession(sessionId);
	if (!session)
	{
		nlwarning("R2An: Invalid session %d", sessionId.asInt());
		return;
	}
	if (weatherValue == session->WeatherValue) return;
	session->setWeatherValue(weatherValue);
}

void CServerAnimationModule::setSeasonValue(TSessionId sessionId, uint8 seasonValue)
{
	if (!_Server->useNetwork())
	{
		nlwarning("simulate setWeatherValue : %d", (int) seasonValue);
		return;
	}

	CAnimationSession *session = getSession(sessionId);
	if (!session)
	{
		nlwarning("R2An: Invalid session %d", sessionId.asInt());
		return;
	}
	if (seasonValue == session->CurrSeason) return;
	session->setSeason(seasonValue);
}



bool CServerAnimationModule::translateActToPrimitive(CInstanceMap& components, CAnimationSession* animSession,  CObject* act,
	uint32 actId, NLLIGO::CPrimitives& primDoc)
{
	H_AUTO( translateActToPrimitive );
	CRtAct* rtAct = new CRtAct();


	animSession->Acts[actId] = rtAct;

	if ( act->isNumber("LocationId")
		&& act->isString("Name")
		&& act->isString("ActDescription")
		&& act->isString("PreActDescription")
		)
	{
		rtAct->Name = act->toString("Name");
		rtAct->ActDescription = act->toString("ActDescription");
		rtAct->PreActDescription = act->toString("PreActDescription");
		rtAct->LocationId = static_cast<uint32>(act->toNumber("LocationId"));
	}
	else
	{
		nlwarning("Error in session '%u' data Corrupted", animSession->SessionId.asInt());
		return false;
	}



	nlassert(primDoc.RootNode != NULL);
	primDoc.RootNode->addPropertyByName("class", new CPropertyString("root"));	// AJM
	primDoc.RootNode->addPropertyByName("name", new CPropertyString(""));	// AJM
	primDoc.RootNode->addPropertyByName("path", new CPropertyString(""));	// AJM
	CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
	CLigoConfig* cfg = CPrimitiveContext::instance().CurrentLigoConfig;
	if (cfg)
	{
		CR2LigoConfig * r2Cfg = dynamic_cast<R2::CR2LigoConfig*>(cfg);
		if (r2Cfg)
		{
			CR2LigoConfig::TScenarioType type;
			if (actId == 0) { type = CR2LigoConfig::Base; }
			else { type = CR2LigoConfig::Act; }
			primDoc.setAliasStaticPart( r2Cfg->getStaticAliasMapping(animSession->AiInstance, type));
		}
	}

	CObject* aiStates = act->getAttr("AiStates");
	uint32 firstAiState = 0;
	uint32 lastAiState = aiStates->getSize();
	std::string managerPrefix = toString("r2.%04d.", animSession->SessionId.asInt());
	std::string prefix = toString("r2_%04d_", animSession->SessionId.asInt());




	// Content Manager
	IPrimitive *npcManager = 0;

	{
		IPrimitive *npc_manager = dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimZone"));
		npc_manager->addPropertyByName("class", new CPropertyString("npc_manager"));
		std::string managerName = managerPrefix + (actId==0? "base": "act");
		npc_manager->addPropertyByName("name", new CPropertyString(managerName));
		npc_manager->addPropertyByName("ai_type", new CPropertyString("MANAGER"));	// AJM
		npc_manager->addPropertyByName("ai_manager_type", new CPropertyString("NPC"));	// AJM
		npc_manager->addPropertyByName("trigger_type", new CPropertyString("npc_zone"));	// AJM
		primDoc.RootNode->insertChild(npc_manager);
		CPrimAlias *npc_manager_alias = dynamic_cast<CPrimAlias *> (CClassRegistry::create ("CPrimAlias"));
		npc_manager_alias->addPropertyByName("class", new CPropertyString("alias"));
		npc_manager_alias->addPropertyByName("name", new CPropertyString("alias"));
		npc_manager->insertChild(npc_manager_alias);
		npcManager = npc_manager;
	}

	IPrimitive *zoneTriggerManager = 0;
	// Trigger Manager
	{
		IPrimitive *npc_manager = dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimZone"));
		npc_manager->addPropertyByName("class", new CPropertyString("npc_manager"));
		std::string managerName = managerPrefix + (actId==0? "base.zone_trigger": "act.zone_trigger");
		npc_manager->addPropertyByName("name", new CPropertyString(managerName));
		npc_manager->addPropertyByName("ai_type", new CPropertyString("MANAGER"));	// AJM
		npc_manager->addPropertyByName("ai_manager_type", new CPropertyString("NPC"));	// AJM
		npc_manager->addPropertyByName("trigger_type", new CPropertyString("npc_zone"));	// AJM
		primDoc.RootNode->insertChild(npc_manager);
		CPrimAlias *npc_manager_alias = dynamic_cast<CPrimAlias *> (CClassRegistry::create ("CPrimAlias"));
		npc_manager_alias->addPropertyByName("class", new CPropertyString("alias"));
		npc_manager_alias->addPropertyByName("name", new CPropertyString("alias"));
		npc_manager->insertChild(npc_manager_alias);
		zoneTriggerManager = npc_manager;
	}

	for ( ; firstAiState != lastAiState ; ++firstAiState)
	{
		CObject*  aiState = aiStates->getValue(firstAiState);

		std::string aiMovement = aiState->toString("AiMovement");

		IPrimitive *state = 0;

		if (aiMovement == "follow_route")
		{
			state = dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimPath"));
			state->addPropertyByName("class", new CPropertyString("npc_route"));
			state->addPropertyByName("ai_type", new CPropertyString("NPC_STATE_ROUTE"));	// AJM
			state->addPropertyByName("ai_profile_params", new CPropertyStringArray());	// AJM
			state->addPropertyByName("vertical_pos", new CPropertyString("auto"));	// AJM
		}
		else
		{
			state = dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimZone"));
			state->addPropertyByName("class", new CPropertyString("npc_zone"));
			state->addPropertyByName("ai_type", new CPropertyString("NPC_STATE_ZONE"));	// AJM
			state->addPropertyByName("ai_profile_params", new CPropertyStringArray());	// AJM
			state->addPropertyByName("vertical_pos", new CPropertyString("auto"));	// AJM
			state->addPropertyByName("keywords", new CPropertyStringArray());	// AJM
		}

		CAttributeToProperty a2pAiState(aiState, state);

		if (aiMovement == "follow_route")
		{
			a2pAiState.setPrimPath();
		}
		else
		{
			a2pAiState.setPrimZone();
		}

		a2pAiState.setAttributeAsStringWithPrefix("Id", "name", prefix);
		a2pAiState.setAttributeAsString("AiActivity", "ai_activity");
		a2pAiState.setAttributeAsString("AiMovement", "ai_movement");
		a2pAiState.setAttributeAsStringArray("AiProfilesParams", "ai_profiles_params");
		a2pAiState.setAttributeAsStringArray("Keywords", "keywords");

		bool isTriggerZone = false;
		if ( aiState->isNumber("IsTriggerZone") )
		{
			isTriggerZone = static_cast<uint32>(aiState->toNumber("IsTriggerZone")) == 1;
		}
		if (isTriggerZone)
		{
			zoneTriggerManager->insertChild(state);
		}
		else
		{
			npcManager->insertChild(state);

		}


		CPrimAlias *state_alias = dynamic_cast<CPrimAlias *> (CClassRegistry::create ("CPrimAlias"));
		state_alias->addPropertyByName("class", new CPropertyString("alias"));
		state_alias->addPropertyByName("name", new CPropertyString("alias"));
		state->insertChild(state_alias);

		CObject* children = aiState->getAttr("Children");
		uint32 firstChild(0), lastChild(children->getSize());
		for ( ; firstChild != lastChild ; ++firstChild)
		{
			CObject* childName = children->getValue(firstChild);
			CObject* component = components.find(childName->toString());


			CObject* oName = component->getAttr("Id");
			if (!oName || !oName->isString())
			{
				nlwarning("Error data corrupt: Invalid group");
				return false;
			}

			std::string fullName = prefix+component->toString("Id");

			IPrimitive *npc_group = dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimNode"));
			npc_group->addPropertyByName("class", new CPropertyString("npc_group"));

			npc_group->addPropertyByName("name", new CPropertyString(prefix+component->toString("Id")));
			npc_group->addPropertyByName("ai_type", new CPropertyString("GROUP_NPC"));	// AJM
			if (component->isNumber("AutoSpawn") && component->toNumber("AutoSpawn")==0)
			{
				npc_group->addPropertyByName("autoSpawn", new CPropertyString("false"));
			}
			else
			{
				npc_group->addPropertyByName("autoSpawn", new CPropertyString("true"));	// AJM
			}

			npc_group->addPropertyByName("grp_keywords", new CPropertyStringArray());	// AJM
			npc_group->addPropertyByName("count", new CPropertyString("0"));	// AJM
			npc_group->addPropertyByName("bot_keywords", new CPropertyStringArray());	// AJM
			npc_group->addPropertyByName("bot_equipment", new CPropertyStringArray());	// AJM
			npc_group->addPropertyByName("bot_chat_parameters", new CPropertyStringArray());	// AJM
			npc_group->addPropertyByName("bot_sheet_client", new CPropertyString(""));	// AJM
			npc_group->addPropertyByName("bot_vertical_pos", new CPropertyString("auto"));	// AJM
			state->insertChild(npc_group);

			IPrimitive *npc_group_parameters = 0;
			{

				npc_group_parameters = dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimNode"));
				CAttributeToProperty a2p(component, npc_group_parameters);



				npc_group_parameters->addPropertyByName("class", new CPropertyString("npc_group_parameters"));
				npc_group_parameters->addPropertyByName("name", new CPropertyString("parameters"));
				npc_group_parameters->addPropertyByName("ai_type", new CPropertyString("GRP_PARAMETERS"));	// AJM
				a2p.setAttributeAsStringArray("AiProfilParams", "ai_profile_params");
				a2p.setAttributeAsStringArray("GrpParameters", "grp_parameters");
			}

			npc_group->insertChild(npc_group_parameters);

			CPrimAlias *npc_group_alias = dynamic_cast<CPrimAlias *> (CClassRegistry::create ("CPrimAlias"));
			npc_group_alias->addPropertyByName("class", new CPropertyString("alias"));
			npc_group_alias->addPropertyByName("name", new CPropertyString("alias"));
			npc_group->insertChild(npc_group_alias);
//			nlinfo("R2Anim: Group %u %s", npc_group_alias->getFullAlias(), std::string(prefix+component->toString("Id")).c_str());
			CRtGrp* rtGrp =  new CRtGrp(npc_group_alias->getFullAlias(), component, fullName);
			animSession->Acts[actId]->RtGrps[rtGrp->Alias] = rtGrp;

			CObject* npcChild = component->getAttr("Children");
			uint32 firstNpc(0), lastNpc(npcChild->getSize());
			for ( ; firstNpc != lastNpc ; ++firstNpc)
			{
				CObject* objectNpcId = npcChild->getValue(firstNpc);
				std::string npcId ( objectNpcId->toString() );
				CObject* objectNpc = components.find(npcId);


				if (!objectNpc
					|| !objectNpc->isString("Name")
					|| !objectNpc->isString("ChatParameters")
					|| !objectNpc->isString("Keywords")
					|| !objectNpc->isString("SheetClient")
					|| !objectNpc->isString("Sheet")
					)
				{
					nlwarning("Error in session '%u' data Corrupted", animSession->SessionId.asInt());
					return false;
				}

				CPrimPoint *npc_bot = dynamic_cast<CPrimPoint *> (CClassRegistry::create ("CPrimPoint"));

				CAttributeToProperty a2p(objectNpc, npc_bot);

				a2p.setPrimPoint();
				npc_bot->addPropertyByName("class", new CPropertyString("npc_bot"));
				npc_bot->addPropertyByName("ai_type", new CPropertyString("BOT_NPC"));	// AJM
				npc_bot->addPropertyByName("vertical_pos", new CPropertyString("auto"));	// AJM
				std::string botname = objectNpc->toString("Name");
				if (botname.length()==0)
				{
					//npc_bot->addPropertyByName("name",new CPropertyString(npcId));
					a2p.setAttributeAsStringWithPrefix("Id", "name", prefix);
				}
				else
				{
					a2p.setAttributeAsString("Name", "name");
				}
				a2p.setAttributeAsStringArray("ChatParameters", "chat_parameters");
				a2p.setAttributeAsStringArray("Keywords", "keywords");


				std::string sheet_client = objectNpc->toString("SheetClient");
				{
					uint32 len = (uint32)sheet_client.length();
					//9=".creature".length()
					if(len>9)
					{
						std::string right = sheet_client.substr(len-9, 9);
						if(right == ".creature")
							sheet_client = sheet_client.substr(0,len-9);
					}
				}

				std::string sheet = objectNpc->toString("Sheet");
				{

					uint32 len = (uint32)sheet.length();
					//9=".creature".length()
					if(len>9)
					{
						std::string right = sheet.substr(len-9, 9);
						if(right == ".creature")
							sheet = sheet.substr(0,len-9);
					}
				}

				if (UseSheetClientWithLevel)
				{
					static std::string basic="basic_";
					static uint32 basicSize = (uint32)basic.size();
					static std::string female = "_female";
					static uint32 femaleSize = (uint32)female.size();
					static std::string male = "_male";
					static uint32 maleSize = (uint32)male.size();

					uint32 sheetClientSize = (uint32)sheet_client.size();

					// Special case of basic_*_female or basic_*_female
					if ( (sheetClientSize > basicSize && sheet_client.substr(0, basicSize) == basic) && !sheet.empty() &&
						(
							(sheetClientSize > femaleSize && sheet_client.substr(sheetClientSize - femaleSize) == female) ||
							(sheetClientSize > maleSize && sheet_client.substr(sheetClientSize - maleSize) == male)
						)
					)
					{

						std::string::size_type pos = sheet.rfind('_');
						if (pos	!= std::string::npos)
						{

							std::string level = sheet.substr(pos);
							if (level.size() == 3
								&& 'a' <= level[1] &&  level[1] <= 'f'
								&& '1' <= level[2] &&  level[2] <= '4')
							{
								sheet_client += level;
							}

						}

					}
				}




				/*
					add an SHEET_CLIENT:SheetClient into the equipment if a alterantiv sheet_client is given
				*/

				if (sheet.empty())
				{

					a2p.setAttributeAsStringArray("Equipment", "equipment");
					npc_bot->addPropertyByName("sheet_client", new CPropertyString(sheet_client));
				}

				else
				{



					CObject* attr = objectNpc->getAttr("Equipment");
					if (attr && attr->isString())
					{
						string str = attr->toString();
						vector<string> result;
						NLMISC::splitString(str, "\n", result);
						result.push_back(NLMISC::toString("CLIENT_SHEET:%s", sheet_client.c_str()));
						npc_bot->addPropertyByName("equipment", new CPropertyStringArray(result));
					}

					npc_bot->addPropertyByName("sheet_client", new CPropertyString(sheet));

				}

				a2p.setAttributeAsBool("IsStuck", "is_stuck");
				npc_group->insertChild(npc_bot);

				CPrimAlias *npc_bot_alias = dynamic_cast<CPrimAlias *> (CClassRegistry::create ("CPrimAlias"));
				npc_bot_alias->addPropertyByName("class", new CPropertyString("alias"));
				npc_bot_alias->addPropertyByName("name", new CPropertyString("alias"));
				npc_bot->insertChild(npc_bot_alias);
//				nlinfo("R2Anim: Bot %u %s", npc_bot_alias->getFullAlias(), std::string(prefix+objectNpc->toString("Id")).c_str());
				uint32 dmProperty = 0;
				if (objectNpc->isNumber("DmProperty"))
				{
					dmProperty = static_cast< uint32 > (objectNpc->toNumber("DmProperty"));
				}
				CRtNpc* rtNpc =  new CRtNpc(npc_bot_alias->getFullAlias(), objectNpc, npc_group_alias->getFullAlias(), dmProperty);

				animSession->Acts[actId]->RtNpcs[rtNpc->Alias] = rtNpc;


			}
		}
	}
	CObject* events = act->getAttr("Events");
	uint32 firstEvent=0,lastEvent=0;
	if(events)lastEvent=events->getSize();

	//for each event
	for(;firstEvent!=lastEvent;++firstEvent)
	{
		//create the primitive event and its associated actions
		IPrimitive* pEvent = getEvent(events->getValue(firstEvent), components, prefix, animSession->SessionId);
		if (!pEvent)
		{
			nlwarning("Error while generating primitive");
			return false;
		}
		//insert the primitive event
		CObject* eventObject = events->getValue(firstEvent);

		bool isTriggerZone = false;
		if ( eventObject->isNumber("IsTriggerZone") )
		{
			isTriggerZone = static_cast<uint32>(eventObject->toNumber("IsTriggerZone")) == 1;
		}
		if (isTriggerZone)
		{
			zoneTriggerManager->insertChild(pEvent);
		}
		else
		{
			npcManager->insertChild(pEvent);
		}

	}
	animSession->Acts[actId]->WeatherValue = 0;
	CObject *weatherValue = act->getAttr("WeatherValue");
	if (weatherValue)
	{
		animSession->Acts[actId]->WeatherValue = (uint16) weatherValue->toNumber();
	}



	// nodeId
	{

		CObject* tree = act->getAttr("UserTriggers");
		if (!tree || !tree->isTable())
		{
			nlwarning("R2An: Data corrupted");
			return false;
		}
		uint32 lastnode = tree->getSize();
		uint32 firstnode = 0;
		for (; firstnode != lastnode; ++firstnode)
		{
			CObject* node = tree->getValue(firstnode);
			if (!node
				|| !node->isTable()
				|| !node->isString("Name")
				|| !node->isString("Grp")	)
			{
				nlwarning("R2An: Data corrupted");
				return false;
			}

   			CRtUserTrigger userTrigger;
			userTrigger.Grp = node->toString("Grp");
			userTrigger.Name = node->toString("Name");
			animSession->Acts[actId]->addUserTrigger(userTrigger);
		}

	}


	return true;
}

bool CServerAnimationModule::makeAnimationSession(CAnimationSession* animSession, bool /* runTest */)
{
	if( !animSession )
	{
		nlwarning("R2An: Null animation session received!");
		return false;
	}

	// add session to queue
	_QueuedSessions.push_back( animSession );
	if (!_Server->useNetwork())
	{
		_ReadyForNextSession = true;
	}

	nldebug( "R2An: animation session %u received, %u in queue", animSession->SessionId.asInt(), _QueuedSessions.size() );
	return true;
}

bool CServerAnimationModule::doMakeAnimationSession(CAnimationSession* animSession)
{
	H_AUTO( makeAnimationSession );
	if (!animSession)
	{
		nlwarning("R2An: No animation session to make");
		return false;
	}


	TSessionId sessionId = animSession->SessionId;


	bool sessionOk =_Sessions.insert(std::make_pair(sessionId, animSession)).second;
	if (!sessionOk)
	{
		nlwarning("R2An: Can't start test, previous session (%u) not closed and trying to start another one", sessionId.asInt());
		return false;
	}

	nlinfo("R2An: makeAnimationSession %u", sessionId.asInt());
	std::vector<CPrimitives> primDocs;


	CObject* rtScenario = animSession->RtData.get();

	uint32 aiInstance = animSession->AiInstance;

	if (!rtScenario)
	{
		nlwarning("R2An: Can't make animation session, no RtScenario");
		return false;
	}

	// build instance map
 	CInstanceMap components("Id");
	components.set(animSession->RtData.get());//default + act courant

	//Create Plot items

	CObject* plotItems = rtScenario->getAttr("PlotItems");
	if (!plotItems || !plotItems->isTable())
	{
		nlwarning("R2An: Data corrupted:session '%u'",sessionId.asInt());
		return false;
	}

	uint32 lastPlotItem = plotItems->getSize();
	uint32 firstPlotItem = 0;
	for (; firstPlotItem != lastPlotItem; ++firstPlotItem)
	{
		CObject* plotItem = plotItems->getValue(firstPlotItem);
		if (!plotItem
			|| !plotItem->isTable()
			|| !plotItem->isNumber("SheetId")
			|| !plotItem->isString("Name")
			|| !plotItem->isString("Description")
			|| !plotItem->isString("Comment")
			)
		{
			nlwarning("R2An: Data corrupted:session '%u'",sessionId.asInt());
			return false;
		}

		uint32 sheetIdAsInt = static_cast<uint32>(plotItem->toNumber("SheetId"));
		CSheetId plotItemSheetId( sheetIdAsInt );


		if ( !CRingAccess::getInstance().isPlotItemSheetId(plotItemSheetId) )
		{
			nlwarning("!!!!!!!!!!!!");
			nlwarning("!!!!!!!!!!!! Someone is trying to hack us?");
			nlwarning("!!!!!!!!!!!!");
			nlwarning("ERROR: a session %u has faked a Plot item sheetId, or new plot items have been added. SheetId='%s' sheetIdAsInt=%u", sessionId.asInt(), plotItemSheetId.toString().c_str(), sheetIdAsInt);
			std::vector<uint32>::const_iterator first(animSession->ConnectedChars.begin()), last(animSession->ConnectedChars.end());
			nlwarning("There is %u connected Chars:", animSession->ConnectedChars.size());
			for	( ; first != last ; ++first)
			{
				nlwarning("CharId = %u UserId(%u)", *first, *first >> 4);
			}

			nlwarning("!!!!!!!!!!!!");
			nlwarning("!!!!!!!!!!!!");
			return false;
		}
		TMissionItem missionItem;
		missionItem.SheetId = plotItemSheetId;
		missionItem.Name = ucstring::makeFromUtf8( plotItem->toString("Name") );
		missionItem.Description = ucstring::makeFromUtf8( plotItem->toString("Description") );
		missionItem.Comment = ucstring::makeFromUtf8( plotItem->toString("Comment") );

		animSession->MissionItems.push_back(missionItem);
	}


	// LocationId
	{

		CObject* locations = rtScenario->getAttr("Locations");
		if (!locations || !locations->isTable())
		{
			nlwarning("R2An: Data corrupted");
			return false;
		}
		uint32 lastLocation = locations->getSize();
		uint32 firstLocation = 0;
		for (; firstLocation != lastLocation; ++firstLocation)
		{
			CObject* location = locations->getValue(firstLocation);
			if (!location
				|| !location->isTable()
				|| !location->isNumber("Season")
				|| !location->isString("Island")
				|| !location->isString("EntryPoint")

				)
			{
				nlwarning("R2An: Data corrupted:session '%u'",sessionId.asInt());
				return false;
			}

			CRtLocation locationItem;
			locationItem.Season = static_cast<uint8>(location->toNumber("Season"));
			locationItem.Island = location->toString("Island");
			locationItem.EntryPoint = location->toString("EntryPoint");
			animSession->Locations.push_back(locationItem);
		}

	}




	//Translate Act to Primitive
	CObject* acts = rtScenario->getAttr("Acts");


	uint32 firstAct = 0;
	uint32 lastAct = acts->getSize();
	primDocs.resize(lastAct);
	animSession->Acts.resize(lastAct);
	for (; firstAct != lastAct ; ++firstAct)
	{
		//std::string key = acts->getKey(firstAct);
		CObject* act= acts->getValue(firstAct);
		if (!act)
		{
			nlwarning("R2An: Can't make animation session, invalid RtAct");
			return false;
		}

		bool ok = translateActToPrimitive(components, animSession, act, firstAct, primDocs[firstAct] ); //TODO
		if (!ok)
		{
			nlwarning("R2An: Data corrupted:session '%u'",sessionId.asInt());
			return false;
		}
	}

	uint32 first = 0, last = (uint32)primDocs.size();
	animSession->Pdrs.resize(last);


	for ( ; first != last; ++first)
	{
		H_AUTO( translatePrimitivesToPdr );
		// translatePrimitivesToPdr;
		//first <=> actId
		CPrimitives *primDoc = &primDocs[first];

		std::string streamFileName;
		if (first==0)
		{
			streamFileName= toString("r2.%04d.base.primitive", aiInstance);
		}
		else
		{
			streamFileName= toString("r2.%04d.act.primitive", aiInstance);
		}

		if (WriteScenarioDebugDataToFile) // Debug
		{
		 	nldebug("writing xml primitive file %s", toString("r2.%04u.act%u.primitive", sessionId.asInt(), first).c_str());
 			saveXmlPrimitiveFile(*primDoc, toString("r2.%04u.act%u.primitive", sessionId.asInt(), first)); // save for debug use
		}

		CAiWrapper::getInstance().primsToPdr(primDoc, streamFileName, animSession->Pdrs[first]);

		if (WriteScenarioDebugDataToFile) // Debug
		{
			string tmp = toString("r2.%04u.act%u.pdr.xml", sessionId.asInt(), first);
			nldebug( "writing xml pdr file %s", tmp.c_str() );
			animSession->Pdrs[first].writeToTxtFile( tmp.c_str() );
		}
	}

	// send start_test to the AI service
	startTest(sessionId, animSession->Pdrs.front());



	// Update animator session info
	{
		std::vector<uint32>::const_iterator first(animSession->ConnectedChars.begin()), last(animSession->ConnectedChars.end());
		for ( ; first != last ; ++first)
		{

			bool inserted = _CharSessions.insert(std::make_pair(*first, sessionId)).second;
			if (!inserted)
			{
				if (_CharSessions[*first] != sessionId)
				{
					TSessionId previousCharSessionId = _CharSessions[*first];
					CAnimationSession* previousSession = getSession( previousCharSessionId );
					BOMB_IF(!previousSession ,"BUG: Failed to get animation session object with id "+NLMISC::toString(previousCharSessionId.asInt()), return false);
					std::vector<uint32> & chars = previousSession->ConnectedChars;
					chars.erase(std::remove(chars.begin(), chars.end(), *first ), chars.end());
				}
				_CharSessions[*first] = sessionId;
			}
			nlinfo("R2An: Char %u is connected to session %u as animator / tester(edition)", *first, sessionId.asInt());
		}
	}

	// Not linked session. so we need to tp at first start (in order to avoid to be stuck at the
	// entry point of the scenario if the player has not enought ring point
	if (_Server->getEditionModule()->getLinkedSessionId(sessionId) == TSessionId(0)
		&& !_Server->getEditionModule()->isEditingSession(sessionId)	)
	{
			animSession->InitialTp = true;
	}

	animSession->StartingAct = true;
	// start the first act 0.5 second after the creation of AINSTANCE to be sure that setPionerRight is done after the creation of the instance
	_Tasks.addTask( new CTaskStartAct( NLMISC::CTime::getLocalTime () + 500, this, sessionId,  animSession->InitialAct, false));
	//startAct(sessionId, animSession->InitialAct);


	return true;
}


bool CServerAnimationModule::getConnectedChars(TSessionId sessionId, std::vector<TCharId>& chars) const
{
	CAnimationSession* previousSession = getSession( sessionId );
	if (!previousSession) { return false;}


	chars = previousSession->ConnectedChars;

	return true;
}



bool CServerAnimationModule::stopTestImpl(TSessionId sessionId, uint32 & lastAct)
{
	bool useNetwork = _Server->useNetwork();

	CAnimationSession* session = getSession( sessionId );
	if (!session)
	{
		nlwarning("R2An: Error can not stop a nonexistent animation %u", sessionId.asInt());
		return false;
	}
	uint32 aiInstance = session->AiInstance;
	requestUnloadTable(sessionId);
	requestReleaseChannels(sessionId);

	if ( !_CharacterControlProxy.isNull())
	{
		CCharacterControlItfProxy proxy(_CharacterControlProxy);
		proxy.scenarioEnded(this,  sessionId);
	}

	if (useNetwork)
	{
		session->setWeatherValue(0); // back to auto-weather
	}

	// Use initial session id in case of linked act
	CAiWrapper::getInstance().stopTest(session->SessionId, aiInstance);


	std::vector<uint32> chars = session->ConnectedChars;
	std::vector<uint32>::iterator first(chars.begin()), last(chars.end());
	for ( ; first != last ; ++first)
	{
		TCharSessions::iterator erased(_CharSessions.find(*first));
		if (erased != _CharSessions.end())
		{
			_CharSessions.erase(erased);
		}
	}
	chars.clear();


	TSessions::iterator found = _Sessions.find(sessionId);
	if (found == _Sessions.end())
	{
		TSessionId remappedSession = _Server->getEditionModule()->getLinkedSessionId(sessionId);
		found = _Sessions.find(remappedSession);
	}
	_Sessions.erase( found );

	lastAct = session->CurrentAct;
	delete session;
	return true;
}

bool CServerAnimationModule::stopTest(TSessionId sessionId, uint32 & lastAct)
{
	CAnimationSession* session = getSession( sessionId );
	if (!session)
	{
		nlinfo("R2An: trying to stop a nonexistent animation %u (will try later)", sessionId.asInt());
		_Tasks.addTaskAt( NLMISC::CTime::getLocalTime() + 1000, new CTaskStopTest(this, sessionId));
		return false;
	}
	return stopTestImpl(sessionId, lastAct);
}

void CServerAnimationModule::onModuleUpdate()
{
	H_AUTO(CServerAnimationModule_onModuleUpdate);

	bool queuedSession = _ReadyForNextSession && !_QueuedSessions.empty();

	if (queuedSession)
	{
		CAnimationSession *animSession = _QueuedSessions.front();
		_QueuedSessions.pop_front();
		doMakeAnimationSession( animSession );
		nldebug( "R2An: animation session %u processed, %u in queue", animSession->SessionId.asInt(), _QueuedSessions.size() );
	}

	NLMISC::TTime now = NLMISC::CTime::getLocalTime();

	_Tasks.execute(now);

}

void CServerAnimationModule::onModuleUp(NLNET::IModuleProxy *senderModuleProxy)
{
	std::string moduleName = senderModuleProxy->getModuleClassName();
	// send back a message to the client to open the firewall

	if ( moduleName == "StringManagerModule")
	{
		_StringManagerProxy = senderModuleProxy;
		nlinfo("StringManagerModule identified!!");
	}
	else if ( moduleName == "CharacterControl")
	{
		_CharacterControlProxy = senderModuleProxy;
	}
	else if ( moduleName == "IOSRingModule")
	{
		_IOSRingProxy= senderModuleProxy;
	}
}


void  CServerAnimationModule::onModuleDown(NLNET::IModuleProxy *senderModuleProxy)
{
	std::string moduleName = senderModuleProxy->getModuleClassName();

	if ( moduleName == "StringManagerModule")
	{
		_StringManagerProxy = NULL;
		nlinfo("StringManagerModule disconnected!");
	}
	else if ( moduleName == "CharacterControl")
	{
		_CharacterControlProxy = NULL;
	}
	else if ( moduleName == "ClientEditionModule") // a client has disconnected
	{

		uint32 charId;
		NLMISC::CEntityId eid;
		std::string userPriv;
		std::string extendedPriv;

		bool ok =  getCharInfo(senderModuleProxy, charId, eid, userPriv, extendedPriv);
		if (!ok) { return ; }

		TSessionId sessionId = getSessionIdByCharId(charId);

		{
			CAnimationSession* session = getSession(sessionId);
			if (session)
			{
				std::vector<uint32>& connectChars = session->ConnectedChars;
				std::vector<uint32>::iterator found ( std::find(connectChars.begin(), connectChars.end(), charId) );
				if (found != connectChars.end())
				{
					std::map<uint32, NLMISC::CEntityId> incarningBots;
					std::map<uint32, NLMISC::CEntityId> talkingAsBots;

					session->IncarningBots.getBots(eid, incarningBots);
					session->TalkingAsBots.getBots(eid, talkingAsBots);

					{
						std::map<uint32, NLMISC::CEntityId>::const_iterator first(talkingAsBots.begin()), last(talkingAsBots.end());
						for (; first != last; ++first)
						{
							removeTalkingAsPlayer(sessionId, first->second, eid);
						}
					}

					{
						std::map<uint32, NLMISC::CEntityId>::const_iterator first(incarningBots.begin()), last(incarningBots.end());
						for (; first != last; ++first)
						{
							removeIncarningPlayer(sessionId, first->second, eid);
						}
					}
					connectChars.erase(found);

					session->IncarningBots.removeChar(eid);
					session->TalkingAsBots.removeChar(eid);
				}

				{
					TCharSessions::iterator found( _CharSessions.find(charId) );
					if (found != _CharSessions.end()) { sessionId = found->second; _CharSessions.erase(found); }
				}
			}
		}
	}
	else if (senderModuleProxy == _IOSRingProxy)
	{
		_IOSRingProxy = NULL;
	}
}


void CServerAnimationModule::stopAct(TSessionId sessionId)
{
	nlinfo( "R2An: stopAct for sessionId %u", sessionId.asInt() );
	if (!_Server->useNetwork())
	{
		nlwarning("simulate stopAct");
		return;
	}

	CAnimationSession* session = getSession(sessionId);
	if (!session) return;

	//nlwarning CurrentAct == 0


	deactivateEasterEggsFromAct(sessionId, session->CurrentAct);

	session->CurrentAct = 0;

	uint32 aiInstance = session->AiInstance;
	// Use initial session in case of linked act
	CAiWrapper::getInstance().stopAct(session->SessionId, aiInstance);

}


bool CServerAnimationModule::getPosition(TSessionId sessionId, double& x, double& y, double& orient, uint8& season, uint32 actIndex)
{
	CAnimationSession *animSession = getSession( sessionId );

	if(! animSession) { return false; } // normal case when getStart position while creating scenario

	uint32 currentAct = animSession->CurrentAct;

	if (actIndex != 0)
	{
		currentAct = actIndex;
	}

	if (currentAct == 0) { currentAct = 1; }

	DROP_IF(currentAct >=  animSession->Acts.size(), "Error: invalide act", return false);

	uint32 locationId = animSession->Acts[currentAct]->LocationId;

	DROP_IF(locationId >=  animSession->Locations.size(), "Error: invalide location", return false);

	CRtLocation & location = animSession->Locations[locationId];

	CScenarioEntryPoints&  epManager = CScenarioEntryPoints::getInstance();

	CScenarioEntryPoints::CCompleteIsland * island = epManager.getIslandFromId(location.Island);
	DROP_IF(!island, "No Island.", return false);

	CScenarioEntryPoints::CShortEntryPoint *entryPoint = epManager.getEntryPointFromIds(location.Island, location.EntryPoint);
	if (!entryPoint)
	{
		entryPoint = epManager.getEntryPointFromIds(location.Island, island->EntryPoints[0].Location); //Evil Hack must be removed
	}
	DROP_IF(!entryPoint, "No EntryPoint.", return false);

	x = entryPoint->X;
	y = entryPoint->Y;
	orient = 0;
	season = location.Season;
	return true;

}


void CServerAnimationModule::getStartParams(NLNET::IModuleProxy * /* sender */, uint32 charId, TSessionId lastStoredSessionId)
{
	_Server->getEditionModule()->getStartParams(charId, lastStoredSessionId);
}


void CServerAnimationModule::askSetUserCharActPosition( NLNET::IModuleProxy * /* sender */, uint32 charId )
{
	// get entry point
	double x, y, orient;
	uint8 season;

	TSessionId sessionId = _Server->getAdminModule()->getSessionIdByCharId(charId );
	bool ok = _Server->getAdminModule()->getPosition( sessionId, x, y, orient, season );
	if (!ok) { return; }

	CFarPosition entryPoint;
	entryPoint.SessionId = sessionId;
	entryPoint.PosState.X = (sint32)(x*1000.0);
	entryPoint.PosState.Y = (sint32)(y*1000.0);
	entryPoint.PosState.Z = 0; // ??
	entryPoint.PosState.Heading = 0; // ??

	CCharacterControlItfProxy ccip( _CharacterControlProxy );
	ccip.setUserCharActPosition(this, charId, entryPoint, season);
}

CAnimationSession* CServerAnimationModule::getSession(TSessionId sessionId) const
{
	if (sessionId.asInt() == 0) { return 0; }

	TSessions::const_iterator session = _Sessions.find(sessionId);
	if (session == _Sessions.end())
	{
		TSessionId remapped = _Server->getEditionModule()->getLinkedSessionId(sessionId);
		if (remapped.asInt() == 0) { return 0; }
		session = _Sessions.find(remapped);
	}

	if (session == _Sessions.end()) { return 0; }

	return session->second;
}


void CServerAnimationModule::connectAnimationModePlay(NLNET::IModuleProxy* proxy)
{

	CShareClientEditionItfProxy client(proxy);
	//getEditionModule()->tpToEntryPoint(proxy, 0);
	client.onAnimationModePlayConnected(this);
}




TSessionId CServerAnimationModule::getSessionIdByCharId(TCharId charId) const
{

	TCharSessions::const_iterator charSessionFound( _CharSessions.find(charId) );

	if (charSessionFound == _CharSessions.end())
	{
		return (TSessionId)0;
	}
	return charSessionFound->second;
}

CAnimationSession* CServerAnimationModule::getSessionByCharId(TCharId charId) const
{
	if (charId == 0)
		return NULL;
	return getSession(getSessionIdByCharId(charId));
}

void CServerAnimationModule::addPioneer( TSessionId sessionId, TCharId charId)
{
	CAnimationSession* session = getSession(sessionId);
	if (!session) return;
	bool isLocal = !_Server->useNetwork();
	std::vector<uint32>::const_iterator found(std::find(session->ConnectedChars.begin(), session->ConnectedChars.end(), charId));
	bool added = false;
	if (found == session->ConnectedChars.end())
	{
		session->ConnectedChars.push_back(charId);
		added = true;
	}


	// Warning the player can receive an addPioneer Message before it client Module went down

	{
		bool inserted =_CharSessions.insert(std::make_pair(charId, sessionId)).second;
		if (!inserted)
		{
			// lookup the previous session id and make sure that we have a session for this id
			TSessionId previousCharSessionId = _CharSessions[charId];

			// disconnect from previous scenario
			if (previousCharSessionId != sessionId)
			{

				CAnimationSession* previousSession = getSession(previousCharSessionId);
				if (previousSession != session)
				{
					std::vector<uint32> & chars = previousSession->ConnectedChars;
					chars.erase(std::remove(chars.begin(), chars.end(), charId ), chars.end());
				}
			}
			_CharSessions[charId] = sessionId;
		}

	}
	nlinfo("R2An: Char %u is connected as animator", charId);
	// update weather for that char
	if ( !isLocal )
	{
		session->sendWeatherValueToChar(charId, session->WeatherValue);
	}


	const TModuleProxyPtr* clientProxyPtr = getEditionModule()->getClientProxyPtr(charId);
	if (!clientProxyPtr)
	{
		nlwarning("A pioneer has just entered in a session %u but has no proxy %u", sessionId.asInt(), charId);
		return;
	}
	getEditionModule()->updateCharPioneerRight(charId);
	// TODO send this information only if client
	askMissionItemsDescription(*clientProxyPtr);
	askActPositionDescriptions(*clientProxyPtr);
	askUserTriggerDescriptions(*clientProxyPtr);
	askTalkingAsListUpdate(*clientProxyPtr);
	askIncarnatingListUpdate(*clientProxyPtr);
	askUpdateScenarioHeader(*clientProxyPtr);

	CShareClientEditionItfProxy proxy(*clientProxyPtr);
	proxy.onCurrentActIndexUpdated(this, session->CurrentAct);
	if (session->CharacternValidePosition.find(charId) == session->CharacternValidePosition.end())
	{
		session->CharacternValidePosition.insert(charId);
	}


	if (added)
	{
		uint32 actId = session->CurrentAct;
		if (session->Acts.size() > actId && !session->Acts[actId]->ActDescription.empty())
		{
			CShareClientEditionItfProxy proxy(*clientProxyPtr);
			proxy.systemMsg(this, "BC_ML", "", session->Acts[actId]->ActDescription);
		}
	}

}



void CServerAnimationModule::onDssTarget( IModuleProxy *senderModuleProxy, const std::vector<std::string> & params)
{
	uint32 charId;
	NLMISC::CEntityId eid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(senderModuleProxy, charId, eid, userPriv, extendedPriv);
	if (!ok) { return ; }

	DROP_IF(!_CharacterControlProxy, "Try to send message to EGS must he is down", return);

	CCharacterControlItfProxy proxy(_CharacterControlProxy);
	proxy.sendCharTargetToDss(this, eid, params);

}

/***
** Called when the DM stops talking as a NPC or Incarning a NPC.
**/
void CServerAnimationModule::stopTalk(const NLMISC::CEntityId &eid, const NLMISC::CEntityId &/* creatureId */,
									  TDataSetRow entityRowId)
{
	TCharId charId = static_cast<TCharId>(eid.getShortId());
	const NLNET::TModuleProxyPtr * foundModule = getEditionModule()->getClientProxyPtr(charId);
	BOMB_IF(foundModule==NULL, "stopTalk failed because getClientProxyPtr() returned NULL for entity "+eid.toString(),return);

	CAnimationSession* session = getSessionByCharId(charId);
	if (!session) { return; }

	TModuleId moduleId = (*foundModule)->getModuleProxyId();

	CMessage msg("stopTalk");
	msg.serial(moduleId);
	msg.serial(entityRowId);
	_StringManagerProxy->sendModuleMessage(this,msg);


}


void CServerAnimationModule::stopIncarn(const NLMISC::CEntityId &eid, const NLMISC::CEntityId &creatureId)
{
	CAiWrapper::getInstance().stopControlNpc(eid, creatureId);
}



CRtNpc* CServerAnimationModule::getNpcByAlias(TSessionId sessionId, TAIAlias alias) const
{
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return 0; }
	if (session->Acts.empty()) { return 0; }



	{
		CRtAct::TRtNpcs::const_iterator found( session->Acts[0]->RtNpcs.find(alias) );
		if (found != session->Acts[0]->RtNpcs.end())
		{
			return found->second;
		}
	}

	uint32 currentAct = session->CurrentAct;
	if (currentAct != 0)
	{
		CRtAct::TRtNpcs::const_iterator  found( session->Acts[currentAct]->RtNpcs.find(alias) );
		if (found != session->Acts[currentAct]->RtNpcs.end())
		{
			return found->second;
		}
	}


	return 0;
}


void CServerAnimationModule::stopControlNpcs(TCharId charId)
{
	const NLNET::TModuleProxyPtr * foundModule = getEditionModule()->getClientProxyPtr(charId);
	if (!foundModule)
	{
		return;
	}

	NLNET::TModuleProxyPtr clientProxyPtr = *foundModule;

	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(clientProxyPtr, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return; }



	// Remove Incarn
	{

		std::map<uint32, NLMISC::CEntityId > entities;
		session->IncarningBots.getBots(charId, entities);


		{
			std::map<uint32, NLMISC::CEntityId > ::const_iterator botEid(entities.begin()), last(entities.end());
			for (; botEid != last; ++botEid)
			{

				if (isIncarnedByPlayer(botEid->second, clientEid))
				{
					removeIncarningPlayer(sessionId, botEid->second, clientEid);
					stopIncarn(clientEid, botEid->second);
				}

			}
		}
	}

	// Remove Talks as
	{

		std::map<uint32, NLMISC::CEntityId > entities;
		session->TalkingAsBots.getBots(charId, entities);


		{
			std::map<uint32, NLMISC::CEntityId > ::const_iterator botEid(entities.begin()), last(entities.end());
			for (; botEid != last; ++botEid)
			{

				if (isTalkingAs(botEid->second, clientEid))
				{
					TOwnedEntities::iterator iter = _TalkedAsEntities.find(botEid->second);
					if (iter != _TalkedAsEntities.end())
					{
						stopTalk(clientEid, botEid->second, iter->second.CreatureRowId);
						removeTalkingAsPlayer(sessionId, botEid->second, clientEid);
					}
				}
			}
		}
	}

}

void CServerAnimationModule::askUpdateScenarioHeader(NLNET::IModuleProxy *clientProxyPtr)
{

	uint32 charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(clientProxyPtr, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return; }


	CShareClientEditionItfProxy proxy(clientProxyPtr);
	proxy.updateScenarioHeader(this, session->ScenarioHeader);

}

void CServerAnimationModule::askIncarnatingListUpdate(NLNET::IModuleProxy *clientProxyPtr)
{

	uint32 charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(clientProxyPtr, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return; }

	std::map<uint32, NLMISC::CEntityId > entities;
	session->IncarningBots.getBots(charId, entities);
	std::map<uint32, NLMISC::CEntityId >::const_iterator first(entities.begin()), last(entities.end());
	std::vector<uint32> botsId;
	for (; first!= last ; ++first)
	{

		TOwnedEntities::iterator found = _IncarnedEntities.find(first->second);
		if (found != _IncarnedEntities.end())
		{
			TAIAlias alias = found->second.CreatatureAlias;

			CRtNpc* npc = getNpcByAlias(sessionId, alias);
			if (npc)
			{
				botsId.push_back(first->first);
				botsId.push_back(npc->NameId);
			}

		}
	}

	CShareClientEditionItfProxy proxy(clientProxyPtr);
	proxy.updateIncarningList(this, botsId);

}

void CServerAnimationModule::askTalkingAsListUpdate(NLNET::IModuleProxy  *clientProxyPtr)
{
	uint32 charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(clientProxyPtr, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CAnimationSession* session = getSession(sessionId);
	if (!session){ return; }

	std::map<uint32, NLMISC::CEntityId > entities;
	session->TalkingAsBots.getBots(charId, entities);
	std::map<uint32, NLMISC::CEntityId >::const_iterator first(entities.begin()), last(entities.end());
	std::vector<uint32> botsId;
	for (; first!= last ; ++first)
	{

		TOwnedEntities::iterator found = _TalkedAsEntities.find(first->second);
		if (found != _TalkedAsEntities.end())
		{
			TAIAlias alias = found->second.CreatatureAlias;

			CRtNpc* npc = getNpcByAlias(sessionId, alias);
			if (npc)
			{
				botsId.push_back(first->first);
				botsId.push_back(npc->NameId);
			}

		}
	}

	CShareClientEditionItfProxy proxy(clientProxyPtr);
	proxy.updateTalkingAsList(this, botsId);
}

void CServerAnimationModule::updateIncarningList(TSessionId /* sessionId */, uint32 charId)
{
	const NLNET::TModuleProxyPtr * foundModule = getEditionModule()->getClientProxyPtr(charId);
	if (foundModule)
	{
		askIncarnatingListUpdate(*foundModule);
	}
}


void CServerAnimationModule::updateTalkingAsList(TSessionId /* sessionId */, uint32 charId)
{

	const NLNET::TModuleProxyPtr * foundModule = getEditionModule()->getClientProxyPtr(charId);
	if (foundModule)
	{
		askTalkingAsListUpdate(*foundModule);
	}
}


bool CServerAnimationModule::setIncarningPlayer(TSessionId sessionId, const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid, TDataSetRow entityRowId, TAIAlias alias)
{

	bool ok = true;
	CAnimationSession* session = getSession(sessionId);
	if (!session) {  return  false;  }
	ok = session->IncarningBots.add( eid, creatureId);
	if (!ok) { return false; }


	TOwnedEntities::iterator iter = _IncarnedEntities.find(creatureId);
	if (iter == _IncarnedEntities.end())
	{
		COwnedCreatureInfo info;
		info.PlayerIds.push_back(eid);
		info.CreatureRowId = entityRowId;
		info.CreatatureAlias = alias;
		_IncarnedEntities[creatureId] = info;
	}
	else
	{
		_IncarnedEntities[creatureId].PlayerIds.push_back(eid);
	}

	updateIncarningList(sessionId, static_cast<uint32>(eid.getShortId()) );

	return true;

}

bool CServerAnimationModule::setTalkingAsPlayer(TSessionId sessionId,  const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid, TDataSetRow entityRowId, TAIAlias alias)
{
	bool ok = true;
	CAnimationSession* session = getSession(sessionId);
	if (!session) {  return  false;  }
	ok = session->TalkingAsBots.add( eid, creatureId);
	if (!ok) { return false; }

	TOwnedEntities::iterator iter = _TalkedAsEntities.find(creatureId);
	if (iter == _TalkedAsEntities.end())
	{
		COwnedCreatureInfo info;
		info.PlayerIds.push_back(eid);
		info.CreatureRowId = entityRowId;
		info.CreatatureAlias = alias;
		_TalkedAsEntities[creatureId] = info;
	}
	else
	{
		_TalkedAsEntities[creatureId].PlayerIds.push_back(eid);
	}


	updateTalkingAsList(sessionId, uint32(eid.getShortId())) ;

	return true;
}



 bool CServerAnimationModule::isIncarnedByPlayer(const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid) const
{
	TOwnedEntities::const_iterator iter = _IncarnedEntities.find(creatureId);
	if (iter == _IncarnedEntities.end())
	{
		return false;
	}
	COwnedCreatureInfo info = (*iter).second;
	std::vector<NLMISC::CEntityId>::const_iterator eidIt(info.PlayerIds.begin()), last(info.PlayerIds.end());
	for (; eidIt != last; ++eidIt)
	{
		if ((*eidIt) == eid)
		{
			return true;
		}
	}
	return false;
}

bool CServerAnimationModule::isTalkingAs(const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid) const
{
	TOwnedEntities::const_iterator iter = _TalkedAsEntities.find(creatureId);
	if (iter == _TalkedAsEntities.end())
	{
		return false;
	}
	COwnedCreatureInfo info = (*iter).second;
	std::vector<NLMISC::CEntityId>::const_iterator eidIt(info.PlayerIds.begin()), last(info.PlayerIds.end());
	for (; eidIt != last; ++eidIt)
	{
		if ((*eidIt) == eid)
		{
			return true;
		}
	}
	return false;

}


void CServerAnimationModule::removeTalkingAsPlayer(TSessionId sessionId, const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid)
{
	TOwnedEntities::iterator iter = _TalkedAsEntities.find(creatureId);
	COwnedCreatureInfo info = (*iter).second;
	std::vector<NLMISC::CEntityId>::iterator eidIt(info.PlayerIds.begin()), last(info.PlayerIds.end());
	for (; eidIt != last; ++eidIt)
	{
		if ((*eidIt) == eid)
		{
			info.PlayerIds.erase(eidIt);
			break;
		}
	}
	if (info.PlayerIds.empty())
	{
		_TalkedAsEntities.erase(iter);
	}

	CAnimationSession* session = getSession(sessionId);
	if (!session) {  return  ;  }
	session->TalkingAsBots.remove( eid, creatureId);

	updateTalkingAsList(sessionId, uint32( eid.getShortId()));


}

void CServerAnimationModule::removeIncarningPlayer(TSessionId sessionId, const NLMISC::CEntityId& creatureId, const NLMISC::CEntityId& eid)
{
	TOwnedEntities::iterator iter = _IncarnedEntities.find(creatureId);
	COwnedCreatureInfo info = (*iter).second;
	std::vector<NLMISC::CEntityId>::iterator eidIt(info.PlayerIds.begin()), last(info.PlayerIds.end());
	for (; eidIt != last; ++eidIt)
	{
		if ((*eidIt) == eid)
		{
			info.PlayerIds.erase(eidIt);
			break;
		}
	}
	if (info.PlayerIds.empty())
	{
		_IncarnedEntities.erase(iter);
	}

	CAnimationSession* session = getSession(sessionId);
	if (!session) {  return  ;  }
	session->IncarningBots.remove(eid, creatureId);

	updateIncarningList(sessionId, uint32(eid.getShortId()) );

}

void CServerAnimationModule::updateAnimationProperties(NLNET::IModuleProxy *senderModuleProxy, const NLMISC::CEntityId & eid, CRtNpc * rtNpc, CRtGrp* rtGrp)
{


	CMessage msg("NPC_APROP"); //Animation Properties

	if (eid == NLMISC::CEntityId::Unknown || rtNpc == 0)
	{
		uint32 zero = 0;
		msg.serial(zero);
		senderModuleProxy->sendModuleMessage(this, msg);
		return;
	}
	uint32 animationProp = rtNpc->NpcAnimationProp;
	const NLMISC::CEntityId & creatureId = rtNpc->EntityId;

	if (animationProp & CAnimationProp::Controlable && rtNpc->Alived)
	{
		if (isIncarnedByPlayer(creatureId, eid))
		{
			animationProp |=  CAnimationProp::Controled;
		}

	}

	if (animationProp & CAnimationProp::Speaking && rtNpc->Alived)
	{
		if (isTalkingAs(creatureId, eid))
		{
			animationProp |= CAnimationProp::SpeakedAs;
		}
	}

	if (!rtNpc->Alived)
	{
		animationProp &= ~CAnimationProp::Alive;
	}

	if (rtGrp)
	{
		CObjectTable* children = rtGrp->ObjectData->toTable("Children");
		if (children->getSize() > 1)
		{
			animationProp |= CAnimationProp::Grouped;
		}
	}

	msg.serial(animationProp);
	senderModuleProxy->sendModuleMessage(this, msg);
	return;
}

void CServerAnimationModule::onCharTargetReceived( NLNET::IModuleProxy *senderModuleProxy,
	const NLMISC::CEntityId& eid, const NLMISC::CEntityId&creatureId,
	TAIAlias alias, TDataSetRow entityRowId,
	const ucstring& /* ucName */, uint32 nameId,
	const std::vector<std::string> & param,
	bool alived)
{


	std::vector<std::string> args(param);
	TCharId charId = static_cast<TCharId>(eid.getShortId());

	const NLNET::TModuleProxyPtr * foundModule = getEditionModule()->getClientProxyPtr(charId);
	DROP_IF(!foundModule, NLMISC::toString("Invalid Char %u", charId), return);


	if (!alived || entityRowId == TDataSetRow() || alias == 0 ||  creatureId == CEntityId::Unknown )
	{
		updateAnimationProperties(* foundModule, CEntityId::Unknown, 0, 0);
		return;
	}



	TCharSessions::const_iterator charSessionFound( _CharSessions.find(charId) );
	DROP_IF(charSessionFound == _CharSessions.end(), NLMISC::toString("Invalid Session for Char %u", charId), return);


	TSessionId sessionId = charSessionFound->second;

	CAnimationSession* session = getSession(sessionId);
	DROP_IF(session->Acts.empty() , NLMISC::toString("Invalid Session %u for Char %u (No act available)",charSessionFound->second.asInt(),  charId), return);


	if ( session->Acts.empty() )
	{
		return;
	}

	NLMISC::CEntityId oldTargetId = NLMISC::CEntityId::Unknown;
	CAnimationSession::TCharacterInfos::iterator charInfos = session->CharacterInfos.find(charId);
	if (charInfos != session->CharacterInfos.end())
	{
		oldTargetId = charInfos->second.TargetId;
	}

	session->CharacterInfos[charId].TargetId = creatureId;

		// Add to targeted entities if necessary
	{

		COwnedCreatureInfo tmp;
		std::pair < TOwnedEntities::iterator, bool> ret = _TargetedEntities.insert( std::make_pair(creatureId, tmp));
		TOwnedEntities::iterator entity = ret.first;
		bool firstTime = ret.second;
		std::vector<NLMISC::CEntityId>& container = entity->second.PlayerIds;
		std::vector<NLMISC::CEntityId>::iterator it = std::find(container.begin(), container.end(), eid);
		if (it == container.end()){ container.push_back(eid); }
		if (firstTime)
		{
			CAiWrapper::getInstance().askBotDespawnNotification(creatureId, alias);
		}
	}

	if (oldTargetId !=  NLMISC::CEntityId::Unknown && oldTargetId != creatureId)
	{
		CAnimationSession::TCharacterInfos::iterator charInfos = session->CharacterInfos.find(charId);
		if (charInfos != session->CharacterInfos.end())
		{
			NLMISC::CEntityId oldTargetId = charInfos->second.TargetId;
			TOwnedEntities::iterator oldTargetIt = _TargetedEntities.find(oldTargetId);
			if (oldTargetIt != _TargetedEntities.end())
			{
				std::vector<NLMISC::CEntityId>& container = oldTargetIt->second.PlayerIds;
				std::vector<NLMISC::CEntityId>::iterator it = std::find(container.begin(), container.end(), eid);
				if (it != container.end())
				{
					container.erase(it);
				}
				if (container.empty())
				{
					_TargetedEntities.erase(oldTargetIt);
				}
			}

		}
	}


	uint32 SelectedNpcAct = 0;

	CRtAct::TRtNpcs::const_iterator npcFound ( session->Acts[0]->RtNpcs.find(alias) );
	CRtNpc * rtNpc = 0;
	std::string name;
	if (npcFound == session->Acts[0]->RtNpcs.end())
	{
		npcFound = session->Acts[ session->CurrentAct ]->RtNpcs.find(alias);
		if (npcFound != session->Acts[ session->CurrentAct ]->RtNpcs.end())
		{
			rtNpc = npcFound->second.getPtr();
			name = rtNpc->ObjectData->getAttr("Name")->toString();
			SelectedNpcAct= session->CurrentAct;
		}
	}
	else
	{
		rtNpc = npcFound->second.getPtr();
		name = rtNpc->ObjectData->getAttr("Name")->toString();
	}

	// Try to target an invalid
	if (!rtNpc)	{ return; }

	CRtAct::TRtGrps & rtGrps = session->Acts[ SelectedNpcAct ]->RtGrps;
	CRtAct::TRtGrps::const_iterator grpFound =	rtGrps.find(rtNpc->GrpAlias);
	CRtGrp *rtGrp = 0;
	if (grpFound != rtGrps.end())
	{
		rtGrp = grpFound->second.getPtr();
	}

	//update data
	rtNpc->EntityId = creatureId;
	rtNpc->DataSetRow = entityRowId;
	rtNpc->NameId = nameId;
	rtNpc->Alived = alived;

	uint32 animationProp = rtNpc->NpcAnimationProp;

	if (args.empty())
	{
		updateAnimationProperties(*foundModule, eid, rtNpc, rtGrp);
		return;
	}


	if ( (animationProp & CAnimationProp::Spawnable) )
	{
		if (args[0] == "DESPAWN_NPC")
		{

			if (rtNpc->EntityId != NLMISC::CEntityId::Unknown)
			{
				CAiWrapper::getInstance().despawnEntity(rtNpc->EntityId, alias);
			}
			return;
		}
	}

	if ( (animationProp & CAnimationProp::Alive) )
	{

		if (args[0] == "ADD_HP") {  CAiWrapper::getInstance().setHPLevel(rtNpc->EntityId, alias, 1); return; }

		if (args[0] == "KILL_NPC" && alived) { CAiWrapper::getInstance().setHPLevel(rtNpc->EntityId, alias, 0); return; }
		if (args[0] == "ADD_HP") {  CAiWrapper::getInstance().setHPLevel(rtNpc->EntityId, alias, 1); return; }

		if (args[0] == "GRP_KILL" && alived) { CAiWrapper::getInstance().setGrpHPLevel(rtNpc->EntityId, alias, 0); return; }
		if (args[0] == "GRP_HEAL") {  CAiWrapper::getInstance().setGrpHPLevel(rtNpc->EntityId, alias, 1); return; }



		if (args[0] == "AGGRO_RANGE_BIG"
			|| args[0] == "AGGRO_RANGE_NORMAL"
			|| args[0] == "AGGRO_RANGE_SMALL"
			|| args[0] == "AGGRO_RANGE_NONE")
		{
			if (args[0] == "AGGRO_RANGE_BIG") { CAiWrapper::getInstance().setAggroRange(rtNpc->EntityId, 100);  return;}
			if (args[0] == "AGGRO_RANGE_NORMAL") { CAiWrapper::getInstance().setAggroRange(rtNpc->EntityId, 30); return;}
			if (args[0] == "AGGRO_RANGE_SMALL") { CAiWrapper::getInstance().setAggroRange(rtNpc->EntityId, 15); return;}
			if (args[0] == "AGGRO_RANGE_NONE") { CAiWrapper::getInstance().setAggroRange(rtNpc->EntityId, 0); return;}

			return;
		}
	}



	if ( (animationProp & CAnimationProp::Controlable) )
	{
		if ( args[0] == "CONTROL"   && alived)
		{
			if (!isIncarnedByPlayer(creatureId, eid))
			{

				if (session->IncarningBots.getBotsCount(charId) == 0) // We can now only incarnate One Npc
				{
					if ( setIncarningPlayer(sessionId, creatureId, eid, entityRowId, alias) )
					{
						CAiWrapper::getInstance().controlNpc(eid, creatureId);
						/*
						if (_CharacterControlProxy)
						{
							CCharacterControlItfProxy proxy(_CharacterControlProxy);
							proxy.teleportCharacterToNpc(this, charId, creatureId, session->CurrSeason );
						}
						*/
						CAiWrapper::getInstance().askBotDespawnNotification(creatureId, alias);

						updateAnimationProperties(*foundModule, eid, rtNpc, rtGrp);
					}
				}
				else
				{
						CShareClientEditionItfProxy proxy( *foundModule );
						proxy.systemMsg(this, "BC", "", "uiR2EDAlreadyIncarningANpc");
				}

			}
			args[0] = "TALK_AS";
		}




		if ( args[0] == "STOP_CONTROL")
		{
			CEntityId bot = creatureId;

			if (args.size() == 2)
			{
				uint32 id;
				fromString(args[1], id);

				CAnimationSession* session = getSession(sessionId);
				if (session)
				{
					bot = session->IncarningBots.getEntity( eid, id);
					if ( bot == NLMISC::CEntityId() ) { return; }

				}
			}

			if (isIncarnedByPlayer(bot, eid))
			{
				stopIncarn(eid, bot);
				removeIncarningPlayer(sessionId, bot, eid);
			}


			if (isTalkingAs(bot, eid))
			{
				stopTalk(eid, bot, entityRowId);
				removeTalkingAsPlayer(sessionId, bot, eid);
			}
			updateAnimationProperties(*foundModule, eid, rtNpc, rtGrp);
			return;
		}

	}

	if ( (animationProp & CAnimationProp::Speaking) )
	{
		if (args[0] == "TALK_AS"  && alived)
		{
			if (isTalkingAs(creatureId, eid))
				return;



			if (session->TalkingAsBots.getBotsCount(charId) < 8)
			{
				if ( setTalkingAsPlayer(sessionId, creatureId, eid, entityRowId, alias) )
				{
					CMessage msg("TALK_AS");
					TModuleId tmp = (*foundModule)->getModuleProxyId();
					msg.serial(tmp);
					msg.serial(entityRowId);
					msg.serial(name);
					msg.serial(sessionId);
					_StringManagerProxy->sendModuleMessage(this,msg);

					CAiWrapper::getInstance().askBotDespawnNotification(creatureId, alias);
					updateAnimationProperties(*foundModule, eid, rtNpc, rtGrp);
				}
			}
			else
			{
				CShareClientEditionItfProxy proxy(*foundModule);
				proxy.systemMsg(this, "BC", "", "uiR2EDSpeakingAsTooManyEntities");
			}





		}

		if (args[0] == "STOP_TALK")
		{
			NLMISC::CEntityId bot = creatureId;
			TDataSetRow botDSR = entityRowId;
			if (args.size() == 2)
			{
				uint32 id;
				fromString(args[1], id);

				CAnimationSession* session = getSession(sessionId);
				if (session)
				{
					bot = session->TalkingAsBots.getEntity( eid, id);
					if ( bot == NLMISC::CEntityId() ) { return; }
					TOwnedEntities::const_iterator found(_TalkedAsEntities.find(bot));
					if (found == _TalkedAsEntities.end()) { return ;}
					botDSR = found->second.CreatureRowId;
				}
			}

			if (isTalkingAs(bot, eid))
			{
				stopTalk(eid, bot,botDSR);
				removeTalkingAsPlayer(sessionId, bot, eid);
				updateAnimationProperties(*foundModule, eid, rtNpc, rtGrp);
			}
			return;
		}
	}

}

// EGS message to indicates that a character is ready in mirror
void CServerAnimationModule::characterReady(NLNET::IModuleProxy * /* sender */, const NLMISC::CEntityId &charEid)
{
	uint32 charId = uint32(charEid.getShortId());

	nldebug("characterReady : received character ready for char %u", charId);
	// forward the event to the server edition module
	getEditionModule()->characterReady(charId);
}



bool CServerAnimationModule::onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &msgin)
{
	std::string operationName = msgin.getName();

//	if (CServerAnimationItfSkel::onDispatchMessage(senderModuleProxy, msgin))
//	{
//		return;
//	}
//
//	// From Client
//	if (CShareServerAnimationItfSkel::onDispatchMessage(senderModuleProxy, msgin))
//	{
//		return;
//	}



	// from service.cpp
	if (!senderModuleProxy)
	{
		if (operationName == "SESSION_ACK")
		{
			// AIS ack receiving anim session
			uint32 aiInstance;
			nlRead(msgin, serial, aiInstance);
			nlinfo( "R2An: ack received from AIS for anim session %u", aiInstance );
			_ReadyForNextSession = true;
			return true;
		}


		if (operationName == "translateAndForwardArg")
		{
			_StringManagerProxy->sendModuleMessage(this,msgin);
			return true;
		}
		nlassert(0);
		return false;

	}
	else if (senderModuleProxy->getModuleClassName() == "ClientEditionModule")
	{
		uint32 charId;
		NLMISC::CEntityId clientEid;
		std::string userPriv;
		std::string extendedPriv;
		bool ok =  checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
		if (!ok) { return true; }


		if (operationName == "requestStartAct")
		{
			// from users
			uint32 actId;
			nlRead(msgin,serial,actId);

			// check that char session is known
			TCharSessions::const_iterator charFound(_CharSessions.find(charId));
			if ( charFound == _CharSessions.end())
			{
				// if session is queued, then just info msg
				if( !_QueuedSessions.empty() )
				{
					bool bFoundSession = false;
					CAnimationSession *lastSession = _QueuedSessions.back();
					CAnimationSession *animSession = NULL;
					do
					{
						animSession = _QueuedSessions.front();
						_QueuedSessions.pop_front();
						_QueuedSessions.push_back( animSession );

						if( !bFoundSession )
						{
							// look for this char in the session
							std::vector<uint32>::const_iterator it = animSession->ConnectedChars.begin(),
							itEnd = animSession->ConnectedChars.end();
							while( it != itEnd )
							{
								if( charId == *it++ )
								{
									bFoundSession = true;
									break;
								}
							}
						}
					} while( lastSession != animSession );

					if( bFoundSession )
					{
						nlinfo("R2An: startAct received from char %u, anim session is in queue.", charId);
						return true;
					}
				}
				nlwarning("R2An: not connected Char(%u) try to start an act.", charId);
				return true;
			}

			scheduleStartAct(charFound->second, actId);
			return true;
		}

		if (operationName == "requestSetSeason")
		{
			uint8 seasonValue;
			nlRead(msgin, serial, seasonValue);

			TCharSessions::const_iterator charFound(_CharSessions.find(charId));
			if ( charFound == _CharSessions.end())
			{
				nlwarning("R2An: not connected char(%d) try to start an act.", charId);
				return true;
			}
			setSeasonValue(charFound->second, seasonValue);
			return true;
		}

		if (operationName == "requestSetWeather")
		{
			uint16 weatherValue;
			nlRead(msgin, serial, weatherValue);


			TCharSessions::const_iterator charFound(_CharSessions.find(charId));
			if ( charFound == _CharSessions.end())
			{
				nlwarning("R2An: not connected char(%d) try to set the weather.", charId);
				return true;
			}
			setWeatherValue(charFound->second, weatherValue);
			return true;
		}

		if (operationName == "requestStopAct")
		{
			//from chars
			TCharSessions::const_iterator charFound(_CharSessions.find(charId));
			if ( charFound == _CharSessions.end())
			{
				nlwarning("R2An: not connected char(%u) try to start an act.", charId);
				return true;
			}
			stopAct(charFound->second);
			return true;
		}


		if(operationName == "requestStringValue")
		{
			TSessionId scenarioId=(TSessionId)1;
			std::string stringId;
			TModuleId id = senderModuleProxy->getModuleProxyId();
			nlRead(msgin,serial,stringId);
			CMessage msg("requestStringValue");
			msg.serial(scenarioId);
			msg.serial(id);
			msg.serial(stringId);
			_StringManagerProxy->sendModuleMessage(this,msg);
			return true;
		}

		if(operationName == "requestIdList")
		{
			TSessionId scenarioId = (TSessionId)1;
			TModuleId id = senderModuleProxy->getModuleProxyId();
			CMessage msg("requestIdList");
			msg.serial(scenarioId);
			msg.serial(id);
			_StringManagerProxy->sendModuleMessage(this,msg);
			return true;
		}

		if(operationName =="talk_as")
		{
			std::string name ;
			nlRead(msgin,serial,name);
			CMessage msg("talk_as");
			TModuleId tmp = senderModuleProxy->getModuleProxyId();
			msg.serial(tmp);
			msg.serial(name);
			_StringManagerProxy->sendModuleMessage(this,msg);
			return true;
		}

		if(operationName == "stopTalk")
		{
			CMessage msg("stopTalk");
			TModuleId id = senderModuleProxy->getModuleProxyId();
			msg.serial(id);
			_StringManagerProxy->sendModuleMessage(this,msg);
			return true;
		}

		if(operationName == "requestStringTable")
		{
			CMessage msg("requestStringTable");
			TSessionId scenarioId = (TSessionId)1;
			TModuleId id = senderModuleProxy->getModuleProxyId();
			nlwarning("string table requested!!");
			msg.serial(scenarioId);
			msg.serial(id);
			_StringManagerProxy->sendModuleMessage(this,msg);
			return true;
		}

		if(operationName=="requestSetValue")
		{
			TSessionId scenarioId = (TSessionId)1;
			std::string stringId;
			std::string value;
			nlRead(msgin,serial,stringId);
			nlRead(msgin,serial,value);
			CMessage msg("requestSetValue");
			nlRead(msg,serial,scenarioId);
			nlRead(msg,serial,stringId);
			nlRead(msg,serial,value);
			_StringManagerProxy->sendModuleMessage(this,msg);
			return true;
		}

	}
	else if (senderModuleProxy->getModuleClassName() == "ServerEditionModule")
	{

		if (operationName == "DBG_CREATE_PRIMITIVES")
		{
			TSessionId sessionId;
			nlRead(msgin,serial,sessionId);
			TSessions::const_iterator found = _Sessions.find(sessionId);
			CObjectSerializerServer obj;
			nlRead(msgin,serial,obj);


			CAnimationSession* session = new CAnimationSession();
			session->CurrentAct = 0;
			session->RtData.reset( obj.getData() );
			session->SessionId = sessionId;
			queueSession(session, false);

			return true;
		}
	}
	return false;
}


TSessionId CServerAnimationModule::getScenarioId(uint32 charId)
{
	TCharSessions::const_iterator found = _CharSessions.find(charId);
	if(found!=_CharSessions.end())
	{
		return found->second;
	}
	return TSessionId(std::numeric_limits<uint16>::max());
}

void CServerAnimationModule::disconnectChar(TCharId charId)
{
	const NLNET::TModuleProxyPtr* client = getEditionModule()->getClientProxyPtr(charId);
	if (client)
	{
		onModuleDown(*client);
	}

}

void CServerAnimationModule::scheduleStartSession(const CAnimationMessageAnimationStart &msg)
{
	_Tasks.addTaskAt( NLMISC::CTime::getLocalTime() + 1000,  new CTaskScheduleStartSession( this, msg) );
}


void CServerAnimationModule::scheduleStartSessionImpl(const CAnimationMessageAnimationStart &msg)
{
	//create new Session
	nlinfo("R2An: creating new animSession %u", msg.SessionId.asInt());
	CAnimationSession* session = new CAnimationSession();
	session->CurrentAct = msg.StartingAct;

	std::vector<uint32>::const_iterator first(msg.AnimatorCharId.begin()), last(msg.AnimatorCharId.end());
	for ( ; first != last ; ++first)
	{
		session->ConnectedChars.push_back(*first);
	}

	session->RtData.reset( msg.RtData.getData() );
	session->SessionId = msg.SessionId;
	session->AiInstance = msg.AiInstance;
	session->InitialAct = msg.StartingAct;
	session->ScenarioHeader = msg.ScenarioHeader;

	// queue session
	bool ok = queueSession(session);
	if (!ok)
	{
		nlwarning("R2An: can't queue animation session");
		return;
	}


	// Update animator session info
	{
		std::vector<uint32>::const_iterator first(msg.AnimatorCharId.begin()), last(msg.AnimatorCharId.end());
		for ( ; first != last ; ++first)
		{

			bool inserted =_CharSessions.insert(std::make_pair(*first, msg.SessionId)).second;
			if (!inserted)
			{
				// lookup the previous sessin id and make sure that we have a session for thsi id
				TSessionId previousCharSessionId = _CharSessions[*first];
				nlinfo("R2An::scheduleStartSession Moving char (%u) from Session %u to Session %u", *first, previousCharSessionId.asInt(), msg.SessionId.asInt());
				BOMB_IF(_Sessions.find(previousCharSessionId)==_Sessions.end(),"scheduleStartSession giving up because failed to find _Sessions entry for character",return);
				// disconnect from previous scenario
				if (previousCharSessionId != msg.SessionId)
				{
					CAnimationSession* previousSession = getSession( previousCharSessionId );
					BOMB_IF(!previousSession, "BUG: Failed to get pointer to session object with Id: "+NLMISC::toString(previousCharSessionId.asInt()), return);
					std::vector<uint32> & chars = previousSession->ConnectedChars;
					chars.erase(std::remove(chars.begin(), chars.end(), *first ), chars.end());
				}
				_CharSessions[*first] = msg.SessionId;
			}
			nlinfo("R2An: Char %u is connected as animator", *first);
		}
	}
	session->CurrSeason = std::numeric_limits<uint8>::max();
}


IPrimitive* CServerAnimationModule::getEvent(CObject* event,CInstanceMap& components, const std::string& prefix,TSessionId scenarioId)
{
	//create the primitive event
	IPrimitive* pEvent = 0;
	pEvent = dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimNode"));
	pEvent->addPropertyByName("class", new CPropertyString("npc_event_handler"));
	pEvent->addPropertyByName("ai_type", new CPropertyString("NPC_EVENT"));	// AJM
	pEvent->addPropertyByName("state_keyword_filter", new CPropertyStringArray());	// AJM
	pEvent->addPropertyByName("group_keyword_filter", new CPropertyStringArray());	// AJM
	CAttributeToProperty a2pEvent(event, pEvent);

	a2pEvent.setAttributeAsStringWithPrefix("Id", "name", prefix); //
	a2pEvent.setAttributeAsString("Event", "event");
	a2pEvent.setAttributeAsStringArrayWithPrefix("StatesByName", "states_by_name", prefix);//TODO more verification
	a2pEvent.setAttributeAsStringArrayWithPrefix("GroupsByName", "groups_by_name",prefix); //TODO???

	CPrimAlias *event_alias = dynamic_cast<CPrimAlias *> (CClassRegistry::create ("CPrimAlias"));
	event_alias->addPropertyByName("class", new CPropertyString("alias"));
	event_alias->addPropertyByName("name", new CPropertyString("alias"));
	pEvent->insertChild(event_alias);

	CObject* actions_id = event->getAttr("ActionsId");
	uint32 firstAction=0,lastAction=0;
	if(actions_id)lastAction=actions_id->getSize();
	IPrimitive* father;
	if(lastAction>1)
	{
		father = dynamic_cast<IPrimitive *> (CClassRegistry::create ("CPrimNode"));
		father->addPropertyByName("class", new CPropertyString("npc_event_handler_action"));
		father->addPropertyByName("ai_type", new CPropertyString("NPC_EVENT_ACTION"));	// AJM
		father->addPropertyByName("weight", new CPropertyString("1"));	// AJM
		father->addPropertyByName("action",new CPropertyString("multi_actions"));
		father->addPropertyByName("name", new CPropertyString("multi_actions"));	// AJM
		pEvent->insertChild(father);
	}
	else
	{
		father = pEvent;
	}
	//for each action of this event
	for(;firstAction!=lastAction;++firstAction)
	{
		CObject * action_id = actions_id->getValue(firstAction);
		std::string id = action_id->toString();
		CObject* action=components.find(id); // can be null?
		if (!action)
		{
			nlwarning("Error while generating primitives in session '%u' in action '%s'", scenarioId.asInt(), id.c_str());
			return 0;
		}
		//create the primitive action
		IPrimitive* pAction=getAction(action, prefix,scenarioId);
		if (!pAction )
		{
			nlwarning("Error for '%u'th action '%s' in states '%s' with group '%s'", firstAction, event->toString("Name").c_str(), event->toString("StatesByName").c_str(), event->toString("GroupsByName").c_str());
			return 0;
		}
		//add the action to the event
		father->insertChild(pAction);
	}
	return pEvent;
}

void CServerAnimationModule::requestLoadTable(CAnimationSession* session)
{
	CObject* texts = session->RtData->getAttr("Texts");
	TSessionId scenarioId = session->SessionId;

	CMessage msg("registerTable");
	//serialize scenarioId
	msg.serial(scenarioId);

	{
		uint32 size = (uint32)session->ConnectedChars.size();
		std::vector<const TModuleProxyPtr*> connected;
		for(uint32 i=0; i<size; ++i)
		{
			const NLNET::TModuleProxyPtr* ptr = getEditionModule()->getClientProxyPtr(session->ConnectedChars[i]);
			if (ptr){ connected.push_back(ptr); }
		}
		uint32 connectedSize = (uint32)connected.size();

		msg.serial( connectedSize );
		for(uint32 i=0; i<connectedSize ;++i)
		{
			const TModuleProxyPtr* ptr = connected[i];
			TModuleId id= (*ptr)->getModuleProxyId();
			msg.serial(id);

		}

		if (connected.size() != session->ConnectedChars.size())
		{
			nlwarning("SAn: error pioneer deconnection not found.");
		}
	}

	//create the message to send the local string table to the
	//string manager module
	if((texts==NULL)||(texts->getAttr("Texts")->getSize()==0))
	{
		uint32 tmp=0;
		msg.serial(tmp);
	}
	else
	{
		CObject* textsTable = texts->getAttr("Texts");
		uint32 size;
		if(textsTable && textsTable->isTable() && (size=textsTable->getSize())!=0 )
		{

			//serialize entry count
			msg.serial(size);
			for(uint32 i=0;i<size;++i)
			{
				CObject* entry = textsTable->getValue(i);
				std::string tmp = entry->getAttr("Id")->toString();
				msg.serial(tmp);
				tmp = entry->getAttr("Text")->toString();
				msg.serial(tmp);
			}

		}
	}
	_StringManagerProxy->sendModuleMessage(this,msg );
}

void CServerAnimationModule::requestUnloadTable(TSessionId sessionId)
{
	CMessage msg("unregisterTable");
	msg.serial(sessionId);
	if (_StringManagerProxy != 0)
	{
		_StringManagerProxy->sendModuleMessage(this,msg);
	}
}

void CServerAnimationModule::requestReleaseChannels(TSessionId sessionId)
{
	CMessage msg("CLEAR_CHANNELS");
	msg.serial(sessionId);
	if (_StringManagerProxy != 0)
	{
		_StringManagerProxy->sendModuleMessage(this,msg);
	}
}

void CServerAnimationModule::onServiceUp(const std::string &serviceName, TServiceId /* serviceId */)
{
	nlinfo( "R2An: %s onServiceUp", serviceName.c_str() );
	if( serviceName == "AIS" )
	{
		_ReadyForNextSession = true;
	}
}

void CServerAnimationModule::onServiceDown(const std::string &serviceName, TServiceId /* serviceId */)
{
	nlinfo( "R2An: %s onServiceDown", serviceName.c_str() );
	if( serviceName == "AIS" )
	{
		_ReadyForNextSession = false;
	}
}


void CServerAnimationModule::askMissionItemsDescription(NLNET::IModuleProxy *senderModuleProxy)
{
	uint32 charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CAnimationSession* session = getSession(sessionId);
	if (session)
	{
		CShareClientEditionItfProxy clientproxy(senderModuleProxy);
		clientproxy.updateMissionItemsDescription(this, sessionId, session->MissionItems);
	}

}

void CServerAnimationModule::deactivateEasterEggsFromAct(TSessionId scenarioId, uint32 actId)
{
	DROP_IF(_CharacterControlProxy.isNull() , "No CharacterControlProxy", return);


	CAnimationSession* session = getSession(scenarioId);
	DROP_IF(!session, toString("No Session %d", scenarioId.asInt()), return);


	DROP_IF(actId >= session->Acts.size(), "Error in activateEasterEgg ", return );

	CRtAct* rtAct = session->Acts[actId];

	DROP_IF(!rtAct, "Error in activateEasterEgg ", return );

	if (rtAct->ActiveEasterEggs.empty())
	{
		return;
	}



	std::set<uint32> easterEggs;

	CRtAct::TActiveEasterEggs::const_iterator first(rtAct->ActiveEasterEggs.begin()), last(rtAct->ActiveEasterEggs.end());

	for (; first != last; ++first)
	{
		easterEggs.insert(first->first);
		CAnimationSession::TActiveEasterEggs::iterator toErase(session->ActiveEasterEggs.find(first->first));
		DROP_IF(toErase == session->ActiveEasterEggs.end(), "Error in activateEasterEgg ", return );
		session->ActiveEasterEggs.erase(toErase);
	}

	session->Acts[actId]->ActiveEasterEggs.clear();

	CCharacterControlItfProxy proxy(_CharacterControlProxy);
	proxy.deactivateEasterEggs(this, easterEggs , scenarioId);

}

void CServerAnimationModule::deactivateEasterEgg(class NLNET::IModuleProxy * /* aisControl */, uint32 easterEggId, TSessionId scenarioId, uint32 actId)
{
	DROP_IF(_CharacterControlProxy.isNull(), "No CharacterControlProxy", return);


	CAnimationSession* session = getSession(scenarioId);
	DROP_IF(!session, toString("No Session %d", scenarioId.asInt()), return);


	// TODO Move code to session?
	DROP_IF(actId >= session->Acts.size(), "Error in activateEasterEgg ", return );
	bool ok = session->Acts[actId]->deactivateEasterEgg(easterEggId);

	if (!ok)
	{
		// already removed
		return;
	}

	CAnimationSession::TActiveEasterEggs::iterator found(session->ActiveEasterEggs.find(easterEggId));
	if (found != session->ActiveEasterEggs.end())
	{
		session->ActiveEasterEggs.erase(found);
	}

	CCharacterControlItfProxy proxy(_CharacterControlProxy);
	proxy.deactivateEasterEgg(this,  easterEggId, scenarioId);
}

void CServerAnimationModule::activateEasterEgg(class NLNET::IModuleProxy * /* aisControl */, uint32 easterEggId, TSessionId scenarioId, uint32 actId ,const std::string & items, float x, float y, float z, float heading, const std::string& grpControler, const std::string& name, const std::string& look)
{
	DROP_IF(_CharacterControlProxy.isNull(), "No CharacterControlProxy", return);

	CAnimationSession* session = getSession(scenarioId);
	DROP_IF(!session, toString("No Session %d", scenarioId.asInt()), return);

	std::vector<std::string> itemNames;
	NLMISC::splitString(items, ";", itemNames);
	std::vector<R2::TItemAndQuantity> itemsAndQuantities;


	uint32 first = 0, last = (uint32)itemNames.size();
	for (; first != last ; ++first)
	{
		std::vector<std::string> itemAndQt;
		std::string itemQt = itemNames[first];
		NLMISC::splitString(itemQt, ":", itemAndQt);

		DROP_IF( itemAndQt.size() != 2, "Syntax error in activateEasterEgg", return );

		uint32 item;
		bool ok = NLMISC::fromString(itemAndQt[0], item);

		DROP_IF( !ok, "Error  activateEasterEgg", return);

		uint32 qt;
		ok = NLMISC::fromString(itemAndQt[1], qt);

		DROP_IF( !ok, "Error in activateEasterEgg", return);
		DROP_IF( qt > 255, "Error in activateEasterEgg", return);
		DROP_IF( item >= 	session->MissionItems.size(), "Error  activateEasterEgg", return);

		R2::TItemAndQuantity itemAndQuantity;
		itemAndQuantity.SheetId =  session->MissionItems[item].SheetId;
		itemAndQuantity.Quantity = qt;
		itemsAndQuantities.push_back(itemAndQuantity);
	}


	DROP_IF(actId >= session->Acts.size(), "Error in activateEasterEgg ", return );
	bool ok = session->Acts[actId]->activateEasterEgg(easterEggId, grpControler);

	if (!ok)
	{
		// component already activated
		return;
	}

	ok = session->ActiveEasterEggs.insert(make_pair(easterEggId, actId)).second;
	if (!ok)
	{
		//must never happend
		nlwarning("Error while activating easter egg");
	}

	CFarPosition pos;
	pos.SessionId = scenarioId;
	pos.PosState.X = static_cast<int>(x*1000);
	pos.PosState.Y = static_cast<int>(y*1000);
	pos.PosState.Z = static_cast<int>(z*1000);
	pos.PosState.Heading = static_cast<float>(heading*1000);

	CCharacterControlItfProxy proxy(_CharacterControlProxy);
	proxy.activateEasterEgg(this,  easterEggId, scenarioId, session->AiInstance, itemsAndQuantities, pos, name, look);

}

void CServerAnimationModule::onEasterEggLooted(class NLNET::IModuleProxy * /* egs */, uint32 easterEggId, TSessionId scenarioId)
{
	CAnimationSession* session = getSession(scenarioId);
	DROP_IF(!session, toString("No Session %d", scenarioId.asInt()), return);
	session->easterEggLooted(easterEggId, scenarioId);
}


void CServerAnimationModule::onUserTriggerTriggered(NLNET::IModuleProxy *senderModuleProxy, uint32 actId, uint32 triggerId)
{
	uint32 charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CAnimationSession* session = getSession(sessionId);
	if (session)
	{
		triggerUserTrigger(sessionId, actId, triggerId);
	}

}

void CServerAnimationModule::triggerUserTrigger( TSessionId sessionId, uint32 actId, uint32 triggerId)
{
	CAnimationSession* animationSession = getSession(sessionId);

	if (animationSession && actId < animationSession->Acts.size() && triggerId < animationSession->Acts[actId]->UserTriggers.size())
	{
		std::string groupName  = animationSession->Acts[actId]->UserTriggers[triggerId].FullName;
		CAiWrapper::getInstance().triggerUserTrigger(groupName, 1);
	}
	else
	{
		nlwarning("error in CServerAnimationModule::triggerUserTrigger(%d, %d, %d)", sessionId.asInt(), actId, triggerId);
	}

}





void CServerAnimationModule::askActPositionDescriptions(NLNET::IModuleProxy *senderModuleProxy)
{
	uint32 charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CAnimationSession* session = getSession(sessionId);
	if (session)
	{
		CShareClientEditionItfProxy clientproxy(senderModuleProxy);
		TActPositionDescriptions actPositionDescriptions;
		session->updateActPositionDescriptions(actPositionDescriptions);
		clientproxy.updateActPositionDescriptions(this, actPositionDescriptions);
	}

}

void CServerAnimationModule::askUserTriggerDescriptions(NLNET::IModuleProxy *senderModuleProxy)
{
	uint32 charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok =  checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CAnimationSession* session = getSession(sessionId);
	if (session)
	{
		CShareClientEditionItfProxy clientproxy(senderModuleProxy);
		TUserTriggerDescriptions userTriggerDescriptions;
		session->updateUserTriggerDescriptions(userTriggerDescriptions);
		clientproxy.updateUserTriggerDescriptions(this, userTriggerDescriptions);
	}


}

void CServerAnimationModule::onBotDeathNotification(NLMISC::CEntityId& creatureId)
{
	onBotDespawnNotification(creatureId);
}


void CServerAnimationModule::onBotDespawnNotification(NLMISC::CEntityId& creatureId)
{
	TOwnedEntities::iterator itControl = _IncarnedEntities.find(creatureId);
	TOwnedEntities::iterator itTalk = _TalkedAsEntities.find(creatureId);
	TOwnedEntities::iterator itTarget = _TargetedEntities.find(creatureId);

	if (itControl == _IncarnedEntities.end() && itTalk == _TalkedAsEntities.end() && itTarget == _TargetedEntities.end() )
	{
		return;
	}

	if (itTarget != _TargetedEntities.end() )
	{
		COwnedCreatureInfo& info = (*itTarget).second;
		std::vector<NLMISC::CEntityId>& playerId =  info.PlayerIds;
		std::vector<NLMISC::CEntityId>::iterator first(playerId.begin()), last(playerId.end());
		for (; first != last; ++first)
		{
			CEntityId eid = *first;
			uint32 charId = static_cast<uint32>(first->getShortId());
			const NLNET::TModuleProxyPtr* pClient = getEditionModule()->getClientProxyPtr(charId);
			if (pClient)
			{
					updateAnimationProperties(*pClient, CEntityId::Unknown, 0, 0);
			}
		}
		_TargetedEntities.erase(itTarget);

	}

	if (itControl != _IncarnedEntities.end())
	{
		COwnedCreatureInfo& info = (*itControl).second;

		std::vector<NLMISC::CEntityId> playerId =  info.PlayerIds;


		std::vector<NLMISC::CEntityId>::iterator first(playerId.begin()), last(playerId.end());
		for (; first != last; ++first)
		{
			CEntityId eid = *first;
			uint32 charId = static_cast<uint32>(first->getShortId());
			TSessionId sessionId = getSessionIdByCharId(charId)	;
			removeIncarningPlayer(sessionId, creatureId, eid);
			stopIncarn(eid, creatureId);
			const NLNET::TModuleProxyPtr* pClient = getEditionModule()->getClientProxyPtr(charId);
			if (pClient)
			{
					updateAnimationProperties(*pClient, CEntityId::Unknown, 0, 0);
			}
		}

	}

	if (itTalk != _TalkedAsEntities.end())
	{
		COwnedCreatureInfo& info = (*itTalk).second;

		std::vector<NLMISC::CEntityId> playerId =  info.PlayerIds;

		std::vector<NLMISC::CEntityId>::iterator first(playerId.begin()), last(playerId.end());
		for (; first != last; ++first)
		{
			CEntityId eid = *first;
			uint32 charId = static_cast<uint32>( first->getShortId() );
			TSessionId sessionId = getSessionIdByCharId(charId);
			removeTalkingAsPlayer(sessionId, creatureId, eid);
			stopTalk(eid, creatureId, info.CreatureRowId);
			const NLNET::TModuleProxyPtr* pClient = getEditionModule()->getClientProxyPtr(charId);
			if (pClient)
			{
				updateAnimationProperties(*pClient, CEntityId::Unknown, 0, 0);
			}
		}
	}


}

void CServerAnimationModule::onStopNpcControlNotification(NLMISC::CEntityId& creatureId)
{
	TOwnedEntities::iterator itControl = _IncarnedEntities.find(creatureId);


	if (itControl != _IncarnedEntities.end())
	{
		COwnedCreatureInfo& info = (*itControl).second;

		std::vector<NLMISC::CEntityId> playerId =  info.PlayerIds;


		std::vector<NLMISC::CEntityId>::iterator first(playerId.begin()), last(playerId.end());
		for (; first != last; ++first)
		{
			CEntityId eid = *first;
			uint32 charId = static_cast<uint32>(first->getShortId());
			TSessionId sessionId = getSessionIdByCharId(charId)	;
			removeIncarningPlayer(sessionId, creatureId, eid);
			stopIncarn(eid, creatureId);
			const NLNET::TModuleProxyPtr* pClient = getEditionModule()->getClientProxyPtr(charId);
			if (pClient)
			{
					updateAnimationProperties(*pClient, CEntityId::Unknown, 0, 0);
			}
		}
	}




}

NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, loadPdrFile)
{
	if (args.size() != 1)
		return false;
	Pdr.clear();
	CAiWrapper::getInstance().fileToPdr(args[0], Pdr);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, savePdrFile)
{

	if (args.size() != 1)
		return false;

	CAiWrapper::getInstance().pdrToFile(Pdr, args[0]);
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, displayPdr)
{

	if (args.size() != 0)
		return false;

	CAiWrapper::getInstance().displayPdr(Pdr);
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, clearPdr)
{
	if (args.size() != 0)
		return false;

	CAiWrapper::getInstance().clearPdr( Pdr);
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, loadPrimitiveFile)
{
	if (args.size() != 1)
		return false;
	CAiWrapper::getInstance().primitiveFileToPdr(args[0], Pdr);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, loadRtFile)
{
	if (args.size() != 1)
		return false;

	CIFile file;
	if (!file.open(args[0]))
	{
		nlwarning("can't open '%s'", args[0].c_str());
	}
	CObjectSerializerServer obj;
	obj.serial(file);
	Pdr.clear();
//	translateScenarioToPdr(obj.getData(), Pdr);

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, startTest)
{
	if (args.size() != 1)
		return false;

	uint32 id;
	fromString(args[0], id);
	startTest(TSessionId(id), Pdr);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, stopTest)
{
	if (args.size() != 1)
		return false;

	uint32 dummyLastAct = 0;
	uint32 id;
	fromString(args[0], id);
	stopTest(TSessionId(id), dummyLastAct );
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, displayMissionItems)
{
	if (args.size() != 1)
	{
		log.displayNL("ServerAnimationModule.displayMissionItems sessionId");
		return false;
	}
	uint32 id;
	fromString(args[0], id);
	TSessionId sessionId = TSessionId(id);
	CAnimationSession* animationSession = getSession(sessionId);
	if (animationSession)
	{
		uint32 first = 0, last = (uint32)animationSession->MissionItems.size();
		log.displayNL("%d Missions Item:", last);
		for ( ;first != last ; ++first)
		{
			log.displayNL("Item %d '%s' '%s' '%s' '%s'",
				first,
				animationSession->MissionItems[first].SheetId.toString().c_str(),
				animationSession->MissionItems[first].Name.toString().c_str(),
				animationSession->MissionItems[first].Description.toString().c_str(),
				animationSession->MissionItems[first].Comment.toString().c_str()
				);
		}

	}

	return true;
}




NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, displayUserTriggers)
{
	if (args.size() != 1)
	{
		log.displayNL("ServerAnimationModule.displayUserTriggers sessionId");
		return false;
	}

	uint32 id;
	fromString(args[0], id);
	TSessionId sessionId = TSessionId(id);
	CAnimationSession* animationSession = getSession(sessionId);
	if (animationSession)
	{
		TUserTriggerDescriptions userTriggerDescriptions;
		animationSession->updateUserTriggerDescriptions(userTriggerDescriptions);


		uint32 first = 0, last = (uint32)userTriggerDescriptions.size();
		log.displayNL("%d User Trigger:", last);
		for ( ;first != last ; ++first)
		{
			log.displayNL("Trigger %d: Name='%s' act='%d' id='%d'",
				first,
				userTriggerDescriptions[first].Name.c_str(),
				userTriggerDescriptions[first].Act,
				userTriggerDescriptions[first].Id
				);
		}

	}
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerAnimationModule, triggerUserTrigger)
{
	if (args.size() != 3)
	{
		log.displayNL("ServerAnimationModule.triggerUserTrigger sessionId actId triggerId");
		return false;
	}

	uint32 id;
	fromString(args[0], id);
	TSessionId sessionId = TSessionId(id);
	uint32 actId;
	fromString(args[1], actId);
	uint32 triggerId;
	fromString(args[2], triggerId);
	triggerUserTrigger(sessionId, actId, triggerId);
	return true;
}

} // namespace R2



IServerEditionModule* CServerAnimationModule::getEditionModule() const
{
	return _Server->getEditionModule();
}

NLNET::IModule* CServerAnimationModule::getModule() const { return const_cast<R2::CServerAnimationModule*>(this); }


bool CServerAnimationModule::isSessionRunning(TSessionId sessionId) const
{
	CAnimationSession* session = getSession(sessionId);
	return session != 0;
}

uint32 CServerAnimationModule::getCurrentAct(TSessionId sessionId) const
{
	CAnimationSession* session = getSession(sessionId);
	if (session)
	{
		return session->CurrentAct;
	}
	return 1;
}

void CServerAnimationModule::dssMessage(NLNET::IModuleProxy * /* ais */, TSessionId sessionId, const std::string & msgType, const std::string& who, const std::string& msg)
{
	CAnimationSession* session = getSession(sessionId);
	if (session)
	{
		CMessage message;
		std::string translatedMessage = _Server->getValue(sessionId, msg);
		CShareClientEditionItfProxy::buildMessageFor_systemMsg(message, msgType, who, translatedMessage);

		std::vector<uint32>::const_iterator first( session->ConnectedChars.begin()), last(session->ConnectedChars.end());
		for (; first != last; ++first)
		{
			const NLNET::TModuleProxyPtr* ptr = getEditionModule()->getClientProxyPtr(*first);
			if (ptr)
			{
				(*ptr)->sendModuleMessage(this, message);
			}
		}
	}
}



void CServerAnimationModule::setSessionStartParams(TSessionId sessionId, sint32 x, sint32 y, uint8 season)
{

	CAnimationSession* session = getSession(sessionId);
	if (session) { session->setStartParams(x, y, season);}
}

bool CServerAnimationModule::mustReloadPosition(TSessionId sessionId, TCharId charId) const
{
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return false; }
	bool ok = session->CharacternValidePosition.find(charId) != session->CharacternValidePosition.end();
	return ok;
}




bool CServerAnimationModule::getHeaderInfo(TSessionId sessionId, TScenarioHeaderSerializer::TValueType& values) const
{
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return false; }
	values = session->ScenarioHeader.Value;
	return true;
}

void CServerAnimationModule::teleportCharacter(NLNET::IModuleProxy * /* ais */, const NLMISC::CEntityId& eid, float x, float y, float z)
{
	CAnimationSession * session = getSessionByCharId(TCharId(eid.getShortId()));
	if (session)
	{
		const R2::TR2TpInfos tpInfos; //no tp Infos

		if ( !_CharacterControlProxy.isNull())
		{
			CCharacterControlItfProxy ccip( _CharacterControlProxy );
			ccip.onTpPositionAsked(this, eid, x, y, z, session->CurrSeason, tpInfos);
		}
	}

}

void CServerAnimationModule::broadcastMsg(TSessionId sessionId, const NLNET::CMessage& msg)
{
	CAnimationSession * session = getSession(sessionId);
	if (!session)
	{
		return;
	}

	broadcastMessage(session, msg);
}

void CServerAnimationModule::setScenarioPoints(NLNET::IModuleProxy * /* ais */, TSessionId sessionId, float scenarioPoints)
{
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return; }
	session->ScenarioScore = static_cast<uint32>(scenarioPoints);
	//nldebug("Current Scenario points= %f)", scenarioPoints);
	return;
}

void CServerAnimationModule::startScenarioTiming(NLNET::IModuleProxy * /* ais */, TSessionId sessionId)
{
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return; }

	if (!session->TimingIsFinished)
	{
		//NLMISC::TTime startTime = NLMISC::CTime::getLocalTime();
		session->ScenarioTime = NLMISC::CTime::getLocalTime();
	}

	//nlinfo("Scenario Start time= %u)", scenarioPoints);
	return;
}

void CServerAnimationModule::endScenarioTiming(NLNET::IModuleProxy * /* ais */, TSessionId sessionId)
{
	CAnimationSession* session = getSession(sessionId);
	if (!session) { return; }

	if (!session->TimingIsFinished)
	{
		//NLMISC::TTime startTime = NLMISC::CTime::getLocalTime();
		session->ScenarioTime = NLMISC::CTime::getLocalTime() - session->ScenarioTime;
		nldebug("Scenario has been completed in: %u ms", session->ScenarioTime);
		session->TimingIsFinished = true;
	}

	//nlinfo("Scenario Start time= %u)", scenarioPoints);
	return;
}

bool CServerAnimationModule::getScore(TSessionId sessionId, uint32 &score, NLMISC::TTime &timeTaken)
{

	CAnimationSession* session = getSession(sessionId);
	if (!session)
	{
		score = 0;
		timeTaken = 0;
		return false;
	}
	score = session->ScenarioScore;
	if (session->TimingIsFinished == true)
	{
		timeTaken = session->ScenarioTime;
	}
	else
	{
		timeTaken = 0;
	}
	return true;
}
