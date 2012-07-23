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
	THE FOLLOWING 32 BIT CODING IS ASSUMED TO BE THE WAY FORWARDS - AS SUCH IT IS 
	RESPECTED BY THE LOW 32 BITS OF THE ENTITY IDS CREATED HERE

	notes:
	there are many more items than anything else in the game so the high bit switches 'item/other'
	there are a lot more ais than anything else (once items removed) so bit 30 switches 'ai/other'
	there are a lot of players, a lot of sheets and not a huge lot of anything else so the coding follows:
	bit 29 'player/other'
	bit 28 'sheet/other'
	bits 20..27 'type' for remaining types (player groups/ guilds/ buildings/ etc

	this means that:
	items:	 1xxx <31 bit id>
	ais:	 01xx <30 bit id>
	players: 001x <29 bit id> (32 slots for 16M players)
	sheets:  0001 <28 bit id>
	others:  0000 <8 bit type> <20 bit id>

	for ais the structure of the id is type dependent (all of them start with 10 bit manager id)
	fauna:	<10 bit manager id> <12 bit group id><8 bit bot id>
	others:	<10 bit manager id> ... the remaining 20 bits TBD ...

    for ais as a rule ~0 is used as a reserved value
	managers are:		01 <10 bit manager id>  1111 1111 1111 1111 1111
	fauna groups are:	01 <10 bit manager id> <12 bit grp id> 1111 1111
*/

/*
#ifndef RYAI_ENTITY_ID_INLINE_H
#define RYAI_ENTITY_ID_INLINE_H


//--------------------------------------------------------------------------
// Some local defines that'll be undefined before the end of the file
//--------------------------------------------------------------------------

#define PLR_HDR_BITS 3
#define PLR_ID_BITS ( 32 - PLR_HDR_BITS )
#define PLR_ID_BIT_MASK ( ( 1<< PLR_ID_BITS ) -1 )
#define PLR_HDR_BIT_SHIFT PLR_ID_BITS
#define PLR_HDR_BIT_MASK (~0u<<PLR_HDR_BIT_SHIFT)
#define PLR_HDR  0x20000000	//(1<<PLR_HDR_BIT_SHIFT)

#define BOT_BITS 4 //8
#define BOT_BIT_SHIFT 0
#define BOT_BIT_MASK (((1<<BOT_BITS)-1)<<BOT_BIT_SHIFT)

#define GRP_BITS 16 //12
#define GRP_BIT_SHIFT BOT_BITS
#define GRP_BIT_MASK (((1<<GRP_BITS)-1)<<GRP_BIT_SHIFT)

#define MGR_BITS 10
#define MGR_BIT_SHIFT (BOT_BITS+GRP_BITS)
#define MGR_BIT_MASK (((1<<MGR_BITS)-1)<<MGR_BIT_SHIFT)

#define HDR_BIT_SHIFT (BOT_BITS+GRP_BITS+MGR_BITS)
#define HDR_BIT_MASK (~0u<<HDR_BIT_SHIFT)
#define HDR (1<<HDR_BIT_SHIFT)

  
//--------------------------------------------------------------------------
// Inline methods
//--------------------------------------------------------------------------

// default ctor
inline CAIEntityId::CAIEntityId()
{
	// this is the 'invalid id' value
	_id=(HDR|MGR_BIT_MASK|GRP_BIT_MASK|BOT_BIT_MASK);
}


//	get the ptr of the CAIEntity generated with the parameter String ..
inline CAIBot	*CAIEntityId::botPtr(const std::string &name)
{
	return	(	CAIEntityId::botId(name).botPtr()	);
}

inline CAIGrp	*CAIEntityId::grpPtr(const std::string &name)
{
	return	(	CAIEntityId::grpId(name).grpPtr()	);
}

inline CAIMgr	*CAIEntityId::mgrPtr(const std::string &name)
{
	return	(	CAIEntityId::mgrId(name).mgrPtr()	);
}

inline CAIEntity *CAIEntityId::entityPtr(const std::string &name)
{
	return	(	CAIEntityId::entityId(name).entityPtr()	);
}


// building an id from component parts
inline CAIEntityId CAIEntityId::botId(uint mgr, uint grp, uint bot)
{
	CAIEntityId id;
	if (!id.setMgrId(mgr)) return CAIEntityId();
	if (!id.setGrpId(grp)) return CAIEntityId();
	if (!id.setBotId(bot)) return CAIEntityId();
	return id;
}
inline CAIEntityId CAIEntityId::botId(CAIEntityId grp, uint bot)
{
	CAIEntityId id(grp);
	if (!id.setBotId(bot)) return CAIEntityId();
	return id;
}


inline CAIEntityId CAIEntityId::grpId(uint mgr, uint grp)
{
	CAIEntityId id;
	if (!id.setMgrId(mgr)) return CAIEntityId();
	if (!id.setGrpId(grp)) return CAIEntityId();
	return id;
}
inline CAIEntityId CAIEntityId::grpId(CAIEntityId mgr, uint grp)
{
	CAIEntityId id(mgr);
	if (!id.setBotId(grp)) return CAIEntityId();
	return id;
}


inline CAIEntityId CAIEntityId::mgrId(uint mgr)
{
	CAIEntityId id;
	if (!id.setMgrId(mgr)) return CAIEntityId();
	return id;
}


inline CAIEntityId CAIEntityId::plrId(const TDataSetRow& entityIndex)
{
	uint32 idx= entityIndex& (CAIS::maxPlayers()-1);
	#ifdef NL_DEBUG
		nlassert((idx<<GRP_BIT_SHIFT)<GRP_BIT_MASK)
	#endif
	return CAIEntityId(PLR_HDR|(idx<<GRP_BIT_SHIFT));
}

// What kind of id is this - is it a bot, grp or mgr

inline bool CAIEntityId::isBot() const
{
	return (_id&BOT_BIT_MASK)!=BOT_BIT_MASK && ((_id&(HDR_BIT_MASK))==(HDR));
}
inline bool CAIEntityId::isGrp() const
{
	return (_id&GRP_BIT_MASK)!=GRP_BIT_MASK && ((_id&(HDR_BIT_MASK|BOT_BIT_MASK))==(HDR|BOT_BIT_MASK));
}
inline bool CAIEntityId::isMgr() const
{
	return (_id&MGR_BIT_MASK)!=MGR_BIT_MASK && ((_id&(HDR_BIT_MASK|BOT_BIT_MASK|GRP_BIT_MASK))==(HDR|BOT_BIT_MASK|GRP_BIT_MASK));
}

inline bool CAIEntityId::isPlr() const
{
	// the following is equivqlent to (~0<<29) == (1<<29) .. ie top 3 bits read 001 (top nibble is 001x .. ie 2 or 3)
	return ( _id & PLR_HDR_BIT_MASK ) == PLR_HDR;	//	0xe0000000 PLR_HDR_BIT_MASK
}

inline bool CAIEntityId::isValid() const
{
	return  (	!isInvalid()	);	
}

inline bool CAIEntityId::isInvalid() const
{
	return  ( (_id&HDR_BIT_MASK)!=HDR || (_id&MGR_BIT_MASK)==MGR_BIT_MASK );
}

// Getting ptr to object of this id ----------------------------------------
inline bool CAIEntityId::exists() const
{
	return entityPtr()!=NULL;
}		

// get ptr to Bot (NULL if !isBot() or !getGrp() or bot>=getGrp()->size
inline CAIBot *CAIEntityId::botPtr() const
{
	#ifdef NL_DEBUG
	nlassert(isBot());
	#endif
	CAIGrp *grp=grpPtr();
	if (grp==NULL) return NULL;
	return grp->getBot(getBotId());
}	
// get ptr to Grp (NULL if (!isBot()&&!isGrp()) or !getMgr() or bot>=getMgr()->size
inline CAIGrp *CAIEntityId::grpPtr() const
{
	// the following line is the optimised version of:	if (!isGrp() && !isBot()) return NULL;
	if ((_id&HDR_BIT_MASK)!=HDR ||(_id&(BOT_BIT_MASK|GRP_BIT_MASK))==(BOT_BIT_MASK|GRP_BIT_MASK)) return NULL;
	CAIMgr *mgr=CAIS::getMgr(getMgrId());
	if (mgr==NULL) return NULL;
	return mgr->getGrp(getGrpId());
}	
// get ptr to Mgr (NULL if isInvalid() or !getMgr()
inline CAIMgr *CAIEntityId::mgrPtr() const
{
//	if (isInvalid()) return NULL;
	return CAIS::getMgr(getMgrId());
}	
// get ptr to Player (NULL if isInvalid() or !getMgr()
inline CAIPlayer *CAIEntityId::plrPtr() const
{
//	if ( isInvalid() )  
//		return NULL;

	return CAIS::getPlayer( *this );
}

// get ptr to Mgr, Grp or Bot
inline CAIEntity *CAIEntityId::entityPtr() const
{
	switch (_id>>29)
	{
	case 1:	return plrPtr();	// 0x2...
	case 2:	break;				// 0x4...
	case 3:	break;				// 0x6...
	default: return NULL;
	}
	if ( (_id&BOT_BIT_MASK) != BOT_BIT_MASK) return botPtr();
	if ( (_id&GRP_BIT_MASK) != GRP_BIT_MASK) return grpPtr();
	return mgrPtr();
}		

// Routines for iterating through ids
inline CAIEntityId CAIEntityId::firstMgr()
{
	CAIEntityId result=mgrId(0);

	while (!result.exists())
		if (!result.setMgrId(result.getMgrId()+1))
			return CAIEntityId();

	return result;
}
inline CAIEntityId CAIEntityId::firstGrp() const
{
	CAIEntityId result(_id);
	if (result.isBot())
		result.setBotId(-1);
	if (!isInvalid())
		result.setGrpId(0);
	CAIMgr *mgr=mgrPtr();
	if (mgr==NULL)
		return CAIEntityId();
	return result;
}
inline CAIEntityId CAIEntityId::firstBot() const
{
	CAIEntityId result(_id);
	if (result.isMgr())
		result.setGrpId(0);
	if (!isInvalid())
		result.setBotId(0);
	CAIGrp *grp=grpPtr();
	if (grp==NULL)
		return CAIEntityId();
	return result;
}

inline CAIEntityId CAIEntityId::nextMgr() const
{
	CAIEntityId result(_id);
	result.setBotId(-1);		// turn the id into a mgr id
	result.setGrpId(-1);		// turn the id into a mgr id
	do
	{
		if (!result.setMgrId(result.getMgrId()+1))
			return CAIEntityId();
	}
	while (!result.exists());
	return result;
}

inline CAIEntityId CAIEntityId::nextGrp(bool sameMgr) const
{
	uint32 mgrId=getMgrId();
	uint32 grpId=getGrpId()+1;
	do
	{
		CAIMgr *mgr=CAIS::getMgr(mgrId);
		while (mgr!=NULL && grpId<mgr->grpCount())
		{
			if (mgr->getGrp(grpId)!=NULL)
				return grpId(mgrId,grpId);
			++grpId;
		}
		if (sameMgr)
			return CAIEntityId();
		grpId=0;
	}
	while (++mgrId<maxMgrId());
	return CAIEntityId();
}

inline CAIEntityId CAIEntityId::nextBot(bool sameGrp,bool sameMgr) const
{
	CAIEntityId result(_id);
	// if this is not last bot in its group return next bot
	if (result.setBotId(getBotId()+1) && result.exists())
		return result;
	// we didn't find the bot in current group so see if we're allowed to change group
	if (sameGrp) 
		return CAIEntityId();
	do
	{
		// get the id of the next grp - it'll have a bot id of ~0 so it needs resetting if it exists...
		result=result.nextGrp(sameMgr);
		// if the group exists convert group id to bot id of its first bot
		if (!result.exists())
			return CAIEntityId();
		result.setBotId(0);
	}
	while(!result.exists());
	return result;
}


// Basic accessors
inline uint32 CAIEntityId::getBotId() const
{
	return _id&( (1<<BOT_BITS)-1 );
}
inline uint32 CAIEntityId::getGrpId() const
{
	return (_id>>BOT_BITS)&( (1<<GRP_BITS)-1 );
}
inline uint32 CAIEntityId::getMgrId() const
{
	return (_id>>(BOT_BITS+GRP_BITS))&( (1<<MGR_BITS)-1 );
}
inline uint32 CAIEntityId::getPlrId() const
{
	return getGrpId();
}
inline bool CAIEntityId::setBotId(uint32 id)
{
	uint32 mask=((1<<BOT_BITS)-1);
	if (id+1>mask && id!=~0u) return false;	// Test for bit count overflow
	if (id==~0u) id=mask;					//	Special case ~0u
	_id=(_id&~mask)|id;
	return true;
}
inline bool CAIEntityId::setGrpId(uint32 id)
{
	uint32 mask=((1<<GRP_BITS)-1);
	if (id>=mask && id!=~0u) return false;	// Test for bit count overflow
	if (id==~0u) id=mask;					//	Special case ~0u
	mask<<=BOT_BITS;
	id<<=BOT_BITS;
	_id=(_id&~mask)|id;
	return true;
}
inline bool CAIEntityId::setMgrId(uint32 id)
{
	uint32 mask=((1<<MGR_BITS)-1);
	if (id>=mask && id!=~0u) return false;	// Test for bit count overflow
	//if (id==~0u) id=mask;
	id&=mask;		//	Special case ~0u
	mask<<=BOT_BITS+GRP_BITS;
	id<<=BOT_BITS+GRP_BITS;
	_id=(_id&~mask)|id;
	return true;
}

// Convertions to and from string, int and CEntityId
inline CAIEntityId::CAIEntityId(const NLMISC::CEntityId &eid)
{
	if (eid==NLMISC::CEntityId::Unknown)
		*this=CAIEntityId();
	else
	{
		if (eid.getType()==RYZOMID::player)
			*this = CAIS::getPlayer(eid)->id();	// buildPlrId(id);
		else if (eid.getType()&0x80)
			_id=(uint32)eid.getShortId();
		else
			*this=CAIEntityId();
	}
}
inline CAIEntityId::CAIEntityId(const std::string &str)
{
	_id=CAIEntityId()._id;
	sscanf(str.c_str(),"AI:%x",&_id);
}
inline CAIEntityId::CAIEntityId(uint32 val)
{
	_id=val;
}

inline NLMISC::CEntityId CAIEntityId::toEntityId() const
{
	if (isPlr())
	{
		return plrPtr()->eid();
	}
	if (isBot() && exists() && botPtr()->isNPC())
		return NLMISC::CEntityId(RYZOMID::npc,(uint64)_id);
	else
		return NLMISC::CEntityId(RYZOMID::creature,(uint64)_id);
}
inline std::string CAIEntityId::toString() const
{
	char result[30];
	if (isInvalid())	sprintf(result,"AI:%04x:INVALID_ID",_id);
	if (isBot()) sprintf(result,"AI:%04x:BOT:%04x:%04x:%04x",_id,getMgrId(),getGrpId(),getBotId());
	if (isGrp()) sprintf(result,"AI:%04x:GRP:%04x:%04x",_id,getMgrId(),getGrpId());
	if (isMgr()) sprintf(result,"AI:%04x:MGR:%04x",_id,getMgrId());
	if (isPlr()) sprintf(result,"AI:%04x:PLR:%s",_id,toEntityId().toString().c_str());
	return std::string(result);
}


inline uint32 CAIEntityId::toInt32() const
{
	return _id;
}

	// Statics that return the highest numbers allowed for bot, grp and mgr id components
inline uint32 CAIEntityId::maxBotId() { return (1<<BOT_BITS)-2; }
inline uint32 CAIEntityId::maxGrpId() { return (1<<GRP_BITS)-2; }
inline uint32 CAIEntityId::maxMgrId() { return (1<<MGR_BITS)-2; }

// Handy basic operators
inline bool CAIEntityId::operator==(const CAIEntityId &other) const { return _id==other._id; }
inline bool CAIEntityId::operator!=(const CAIEntityId &other) const { return _id!=other._id; }
inline bool CAIEntityId::operator>=(const CAIEntityId &other) const { return _id>=other._id; }
inline bool CAIEntityId::operator<=(const CAIEntityId &other) const { return _id<=other._id; }
inline bool CAIEntityId::operator>(const CAIEntityId &other)  const { return _id>other._id; }
inline bool CAIEntityId::operator<(const CAIEntityId &other)  const { return _id<other._id; }


//--------------------------------------------------------------------------
// Undefining local constants
//--------------------------------------------------------------------------

#undef BOT_BITS 
#undef BOT_BIT_SHIFT 
#undef BOT_BIT_MASK 

#undef GRP_BITS 
#undef GRP_BIT_SHIFT 
#undef GRP_BIT_MASK 

#undef MGR_BITS 
#undef MGR_BIT_SHIFT 
#undef MGR_BIT_MASK 

#undef HDR_BIT_SHIFT 
#undef HDR_BIT_MASK 
#undef HDR 


#endif

*/
