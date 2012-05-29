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



#ifndef RY_MISSION_ACTION_H
#define RY_MISSION_ACTION_H

#include "game_share/string_manager_sender.h"
#include "mission_item.h"
#include "mission_manager/mission_parser.h"
#include "mission_manager/mission_event.h"
#include "mission_types.h"

/**
 * interface for mission actions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class IMissionAction
{
	NL_INSTANCE_COUNTER_DECL(IMissionAction);
public:
	IMissionAction * getCopy()
	{
		IMissionAction * action = getNewPtr();
		*action = *this;
		return action;
	}
	virtual bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData) = 0;
	virtual bool solveTextsParams( CMissionSpecificParsingData & missionData ){return true;}
	virtual void launch(CMission* instance,std::list< CMissionEvent * > & eventList) = 0;
	virtual bool addReward(const std::vector<std::string>&  script,const std::vector< std::pair<std::string,CMissionItem> >& items,uint32 line)
	{
		nlerror("add reward is only implemented for reward blocs. SERIOUS BUG");
		return false;
	}
private:
	virtual IMissionAction * getNewPtr() = 0;
protected:
	uint32	_SourceLine;
};


class IMissionActionFactory
{
public:
	static IMissionAction* buildAction( uint32 line, const std::vector<std::string> & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData);	
protected:
	
	/// little init trick to avoid the problem of static data init order
	static void init();
	virtual IMissionAction * instanciate() = 0;
	static std::vector< std::pair< std::string, IMissionActionFactory* > > * Entries;
};

/// macro used to register steps
#define MISSION_REGISTER_ACTION(_class_,_name_) \
	class _class_##ActionFactory : public IMissionActionFactory \
	{\
	public:\
		_class_##ActionFactory()\
		{\
			init();\
			std::string str = std::string(_name_); \
			for (uint i = 0; i < (*Entries).size(); i++ ) \
			{\
				if ( (*Entries)[i].first == str || (*Entries)[i].second == this )nlstop;\
			}\
			(*Entries).push_back( std::make_pair( str, this ) );\
		}\
		IMissionAction * instanciate()\
		{ \
			return new _class_;\
		} \
	};\
	static _class_##ActionFactory* _class_##ActionFactoryInstance = new _class_##ActionFactory;


/// set the failure index
class CMissionActionSetFailureIndex : public IMissionAction
{
public:
	inline void setIndex(uint32 index){ _Index = index; }
protected:
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList);
	IMissionAction* getNewPtr()
	{ 
		CMissionActionSetFailureIndex * ptr = new CMissionActionSetFailureIndex;
		*ptr = *this; 
		return ptr; 
	}
	uint32	_Index;
};

/// set the crash handler index index
class CMissionActionSetPlayerReconnectHandlerIndex : public IMissionAction
{
public:
	inline void setIndex(uint32 index){ _Index = index; }
protected:
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList);
	IMissionAction* getNewPtr()
	{ 
		CMissionActionSetPlayerReconnectHandlerIndex * ptr = new CMissionActionSetPlayerReconnectHandlerIndex;
		*ptr = *this;
		return ptr; 
	}
	uint32	_Index;
};

/// set the crash handler index index
class CMissionActionSetCrashHandlerIndex : public IMissionAction
{
public:
	inline void setIndex(uint32 index){ _Index = index; }
protected:
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList);
	IMissionAction* getNewPtr()
	{ 
		CMissionActionSetCrashHandlerIndex * ptr = new CMissionActionSetCrashHandlerIndex;
		*ptr = *this;
		return ptr; 
	}
	uint32	_Index;
};


/// set_teleport event
class CMissionActionSetTeleport : public IMissionAction
{
public:
	uint32								SourceLine;
	uint16								DestinationIdx;
	std::string							PhraseId;
	bool								Once;
	TAIAlias							Bot;
	TVectorParamCheck					Params;
	uint32								Index;

private:
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData);
	void launch(CMission* instance,std::list< CMissionEvent * > & eventList);
	IMissionAction* getNewPtr()
	{ 
		CMissionActionSetTeleport * ptr = new CMissionActionSetTeleport;
		*ptr = *this;
		return ptr; 
	}
	bool solveTextsParams( CMissionSpecificParsingData & missionData );
};


/// jump
class CMissionActionJump : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData);
protected:
	void launch(CMission* instance,std::list< CMissionEvent * > & eventList);
	IMissionAction* getNewPtr();

	std::string Label;
};

#endif // RY_MISSION_ACTION_H

/* End of mission_action.h */


