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

#ifndef RYAI_PROFILE_PET_H
#define RYAI_PROFILE_PET_H

#include "profile.h"	// for CAIBaseProfile
#include "ai_share/ai_types.h"	// for TProfiles
#include "ai_pos.h"				// for CAIPos
#include "ai_share/world_map.h"	// for TAStarFlag
#include "path_behaviors.h"		// for CPathCont

class CSpawnBotPet;
/*
#include "ai_mgr.h"
#include "ai_bot_pet.h"
#include "server_share/pet_interface_msg.h"
*/

//////////////////////////////////////////////////////////////////////////////
// CAIPetProfileStand                                                       //
//////////////////////////////////////////////////////////////////////////////

class CAIPetProfileStand
: public CAIBaseProfile
{
public:
	CAIPetProfileStand(CSpawnBotPet* bot);
	
	virtual void beginProfile() { }
	virtual	void updateProfile(uint ticksSinceLastUpdate) { }
	virtual void endProfile() { }
	
	virtual std::string getOneLineInfoString() const { return std::string("stand pet profile"); }
	
	AITYPES::TProfiles getAIProfileType() const { return AITYPES::PET_STAND; }
	
private:
	NLMISC::CDbgPtr<CSpawnBotPet> _Bot;
};

//////////////////////////////////////////////////////////////////////////////
// CAIPetProfileFollowPlayer                                                //
//////////////////////////////////////////////////////////////////////////////

class CAIPetProfileFollowPlayer
: public CAIBaseProfile
{
public:
	CAIPetProfileFollowPlayer(CSpawnBotPet* bot, TDataSetRow const& playerRow);
	
	virtual void beginProfile() { }
	virtual	void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile() { }
	
	virtual std::string getOneLineInfoString() const { return std::string("follow_player pet profile"); }
	
	AITYPES::TProfiles getAIProfileType() const { return AITYPES::PET_FOLLOW; }
	
protected:
private:
	NLMISC::CDbgPtr<CSpawnBotPet>	_Bot;
	TDataSetRow	_PlayerRow;
};

//////////////////////////////////////////////////////////////////////////////
// CAIPetProfileGotoPoint                                                   //
//////////////////////////////////////////////////////////////////////////////

class CAIPetProfileGotoPoint
: public CAIBaseProfile
{
public:
	CAIPetProfileGotoPoint(CSpawnBotPet* bot, CAIPos const& position, RYAI_MAP_CRUNCH::TAStarFlag denyFlags, bool despawn = false);
	
	bool isValid() const { return _Valid; }
	
	virtual void beginProfile() { }
	virtual void updateProfile(uint ticksSinceLastUpdate);
	virtual void endProfile() { }
	
	virtual std::string getOneLineInfoString() const { return std::string("goto_point pet profile"); }
	
	AITYPES::TProfiles getAIProfileType() const;
	
private:
	NLMISC::CDbgPtr<CSpawnBotPet> _Bot;
	CPathCont	_PathCont;
	CAIVector	_Pos;
	bool const	_Despawn;
	bool		_Valid;
};

#endif
