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



/*
#ifndef RYAI_ENTITY_ID_H
#define RYAI_ENTITY_ID_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/entity_id.h"
#include "game_share/ryzom_entity_id.h"

#include <string>


//--------------------------------------------------------------------------
// Advanced declaration of some classes	with circular refferences
//--------------------------------------------------------------------------
/*
class AI;
class CAIBot;
class CAIGrp;
class CAIMgr;
class CAIEntity;
class CAIPlayer;
*/

//--------------------------------------------------------------------------
// The class itself
//--------------------------------------------------------------------------

/*
class CAIEntityId
{
public:
	// default ctor
	inline CAIEntityId();

	///////////////////
	// Statics Start //	

	// returns the highest numbers allowed for bot, grp and mgr id components
	inline static uint32 maxBotId();
	inline static uint32 maxGrpId();
	inline static uint32 maxMgrId();
	
	// building an id from component parts
	inline	static CAIEntityId botId(uint mgr, uint grp, uint bot);
	inline	static CAIEntityId botId(CAIEntityId grp, uint bot);
	
	inline	static CAIEntityId grpId(uint mgr, uint grp);
	inline	static CAIEntityId grpId(CAIEntityId mgr, uint grp);
	
	inline	static CAIEntityId mgrId(uint mgr);
	
	inline	static CAIEntityId plrId(const TDataSetRow& entityIndex);
	
	// these methods are not intended to be fast but they should be thorough,
	// trying every sensible way of interpretting the strings to identify bots
	// including: CAIEntitId format, CEntityId format, mgr_name.grp_name.bot_name
	// format, stand alone name and mgr_name:grp_name:bot_name format
	// Not inlined coz they thake a large amount of code.
			static CAIEntityId botId(const std::string &name);
			static CAIEntityId grpId(const std::string &name);
			static CAIEntityId mgrId(const std::string &name);
			static CAIEntityId entityId(const std::string &name);

	//	get the ptr of the CAIEntity generated with the parameter String ..
	inline static	CAIBot	*botPtr(const std::string &name);		// get ptr to Bot (NULL if !isBot() or !getGrp() or bot>=getGrp()->size)
	inline static	CAIGrp	*grpPtr(const std::string &name);		// get ptr to Grp (NULL if (!isBot()&&!isGrp()) or !getMgr() or bot>=getMgr()->size)
	inline static	CAIMgr	*mgrPtr(const std::string &name);		// get ptr to Mgr (NULL if isInvalid() or !getMgr())
	inline static	CAIEntity *entityPtr(const std::string &name);	// get ptr to Plr, Mgr, Grp or Bot			
	
	// Statics End //	
	/////////////////


	// What kind of id is this - is it a bot, grp, mgr, plr or invalid

	inline bool isBot() const;		// syntactically, is this a bot id
	inline bool isGrp() const;		// syntactically, is this a	grp id
	inline bool isMgr() const;		// syntactically, is this a mgr id
	inline bool isPlr() const;		// syntactically, is this a player id
	inline bool isValid() const;	// syntactically, is this a valid id  ( not invalid, may it be ? ).
	inline bool isInvalid() const;	// syntactically, is this an invalid id  
	
	// Getting ptr to object of this id ----------------------------------------
	inline bool exists() const;			// traverse entity hierachy to see whether this entity exists
	inline CAIBot *botPtr() const;		// get ptr to Bot (NULL if !isBot() or !getGrp() or bot>=getGrp()->size)
	inline CAIGrp *grpPtr() const;		// get ptr to Grp (NULL if (!isBot()&&!isGrp()) or !getMgr() or bot>=getMgr()->size)
	inline CAIMgr *mgrPtr() const;		// get ptr to Mgr (NULL if isInvalid() or !getMgr())
	inline CAIPlayer *plrPtr() const;	// get ptr to Plr (NULL if isInvalid() or !getMgr())

	inline CAIEntity *entityPtr() const;	// get ptr to Plr, Mgr, Grp or Bot

	// Routines for iterating through ids
	inline static CAIEntityId firstMgr();	// get first mgr in the ai singleton
	inline CAIEntityId firstGrp() const;		// get first grp for the mgr part of this CAIEntityId
	inline CAIEntityId firstBot() const;		// get first bot for the grp part of this CAIEntityId

	inline CAIEntityId nextMgr() const;
	inline CAIEntityId nextGrp(bool sameMgr=false) const;
	inline CAIEntityId nextBot(bool sameGrp=false,bool sameMgr=false) const;

	// Basic accessors
	inline bool setBotId(uint32 id);
	inline bool setGrpId(uint32 id);
	inline bool setMgrId(uint32 id);


	inline uint32 getBotId() const; 
	inline uint32 getGrpId() const;
	inline uint32 getMgrId() const;
	inline uint32 getPlrId() const;


	// Convertions to and from string, int and CEntityId
	inline CAIEntityId(const NLMISC::CEntityId &id);
	inline CAIEntityId(const std::string &str);
	inline CAIEntityId(uint32 val);
	inline uint32 toInt32() const;
	
	inline NLMISC::CEntityId toEntityId() const;
	inline std::string toString() const;

	// Handy basic operators
	inline bool operator==(const CAIEntityId &other) const;
	inline bool operator!=(const CAIEntityId &other) const;
	inline bool operator>=(const CAIEntityId &other) const;
	inline bool operator<=(const CAIEntityId &other) const;
	inline bool operator>(const CAIEntityId &other) const;
	inline bool operator<(const CAIEntityId &other) const;

protected:
private:
	uint32 _id;
};

#endif
*/

