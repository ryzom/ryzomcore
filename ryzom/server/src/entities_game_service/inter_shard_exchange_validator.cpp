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

#include "stdpch.h"

// NeL MISC
#include "nel/misc/variable.h"
#include "nel/misc/command.h"

// Local
#include "inter_shard_exchange_validator.h"
#include "egs_sheets/egs_static_game_item.h"
#include "game_item_manager/game_item.h"


//-------------------------------------------------------------------------------------------------
// NLMISC::CVariables
//-------------------------------------------------------------------------------------------------

// default level limit for shards if no limit is explicitly specified
NLMISC::CVariable<uint32>	DefaultInterShardExchangeLevelCap	("egs", "DefaultInterShardExchangeLevelCap", "Default level limit for shards if no limit is explicitly specified", 20,0,true);


//-------------------------------------------------------------------------------------------------
// class CInterShardExchangeValidator
//-------------------------------------------------------------------------------------------------

class CInterShardExchangeValidator
	:	public IInterShardExchangeValidator
{
public:
	// Singleton accessor
	static CInterShardExchangeValidator* getInstance();

public:
	// public instance methods

	// specialisations of methods from IInterShardExchangeValidator
	virtual bool isExchangeAllowed(const CGameItemPtr& theItem, TShardId shardId0, TShardId shardId1) const;
	virtual bool isMoneyExchangeAllowed(TShardId shardId0, TShardId shardId1) const;

	// accessors for the default level cap value
	void setDefaultLevelCap(TLevelCap defaultLevelCap);
	TLevelCap getDefaultLevelCap() const;

	// accessors for the default level cap value for a given shard
	void setLevelCap(TShardId shardId,TLevelCap levelCap);
	TLevelCap getLevelCap(TShardId shardId) const;
	void resetLevelCap(TShardId shardId);

	// method to dump the complete set of level caps to a given output log
	void displayAllLevelCaps(NLMISC::CLog& log) const;

private:
	// private instance data
	typedef std::map<TShardId,TLevelCap> TLevelCaps;
	TLevelCaps LevelCaps;

private:
	// prohibit illegal operations for singletons (ensure that noone outside the class can instantiate us)
	CInterShardExchangeValidator() {}	
	CInterShardExchangeValidator(const CInterShardExchangeValidator&);
	const CInterShardExchangeValidator& operator=(const CInterShardExchangeValidator&);
};


//-------------------------------------------------------------------------------------------------
// static methods CInterShardExchangeValidator
//-------------------------------------------------------------------------------------------------

CInterShardExchangeValidator* CInterShardExchangeValidator::getInstance()
{
	// a static pointer to hold the singleton instance
	static CInterShardExchangeValidator* instance=NULL;

	// allocate the instance on first access
	if (instance==NULL)
	{
		instance= new CInterShardExchangeValidator;
	}

	// return the instance
	return instance;
}


//-------------------------------------------------------------------------------------------------
// instance methods CInterShardExchangeValidator
//-------------------------------------------------------------------------------------------------

bool CInterShardExchangeValidator::isExchangeAllowed(const CGameItemPtr& theItem, TShardId shardId0, TShardId shardId1) const
{
	// allow all exchanges between characters from the same shard
	if (shardId0==shardId1) return true;

	// allow all exchanges of plot items
	if ( theItem->getStaticForm()->Family == ITEMFAMILY::SCROLL_R2 ) return true;

	// determine the maximum level for items exchanged between the 2 shards
	TLevelCap levelLimit= std::min(getLevelCap(shardId0),getLevelCap(shardId1));

	// if item is too high level then refuse
	if (theItem->quality()>levelLimit) return false;

	// if item is flagged as non-shardExchangeable then refuse
	if (theItem->getStaticForm()->ShardExchangeable==false) return false;

	// if item is named (and not a plot item) then refuse
	if (!theItem->getPhraseId().empty()) return false;

	// we've found no reason to refuse so return true
	return true;
}

bool CInterShardExchangeValidator::isMoneyExchangeAllowed(TShardId shardId0, TShardId shardId1) const
{
	// allow all exchanges between characters from the same shard
	if (shardId0==shardId1) return true;

	// don't allow money exchange between shards of different level caps
	if (getLevelCap(shardId0)!=getLevelCap(shardId1)) return false;

	// we've found no reason to refuse so return true
	return true;
}

void CInterShardExchangeValidator::setDefaultLevelCap(CInterShardExchangeValidator::TLevelCap defaultLevelCap)
{
	DefaultInterShardExchangeLevelCap= defaultLevelCap;
}

CInterShardExchangeValidator::TLevelCap CInterShardExchangeValidator::getDefaultLevelCap() const
{
	return DefaultInterShardExchangeLevelCap;
}

void CInterShardExchangeValidator::setLevelCap(CInterShardExchangeValidator::TShardId shardId,CInterShardExchangeValidator::TLevelCap levelCap)
{
	LevelCaps[shardId]= levelCap;
}

CInterShardExchangeValidator::TLevelCap CInterShardExchangeValidator::getLevelCap(CInterShardExchangeValidator::TShardId shardId) const
{
	// see if we have a specific level cap for this shard
	TLevelCaps::const_iterator it= LevelCaps.find(shardId);
	if (it!=LevelCaps.end())
	{
		// a specific level cap was found so return it
		return it->second;
	}
	// no specific level cap found so return the default value
	return getDefaultLevelCap();
}

void CInterShardExchangeValidator::resetLevelCap(CInterShardExchangeValidator::TShardId shardId)
{
	LevelCaps.erase(shardId);
}

void CInterShardExchangeValidator::displayAllLevelCaps(NLMISC::CLog& log) const
{
	log.displayNL("Level Caps:");

	// iterate over the set of level caps in our singleton
	TLevelCaps::const_iterator it= LevelCaps.begin();
	TLevelCaps::const_iterator itEnd= LevelCaps.end();
	for (;it!=itEnd;++it)
	{
		log.displayNL("- shard: %3u  levelCap: %3u",it->first,it->second);
	}

	if (CInterShardExchangeValidator::getInstance()->LevelCaps.empty())
	{
		log.displayNL("- none");
	}
}


//-------------------------------------------------------------------------------------------------
// methods IInterShardExchangeValidator
//-------------------------------------------------------------------------------------------------

IInterShardExchangeValidator* IInterShardExchangeValidator::getInstance()
{
	// delegate to our implementation class
	return CInterShardExchangeValidator::getInstance();
}


//-------------------------------------------------------------------------------------------------
// NLMISC_COMMANDs
//-------------------------------------------------------------------------------------------------

NLMISC_COMMAND(setShardExchangeLimit, "set the level limit for items exchanged between players from different shards","<shardId> <level>" )
{
	// make sure we have 2 arguments
	if (args.size()!=2)
		return false;

	// get numeric values for the 2 arguments
	IInterShardExchangeValidator::TShardId shardId;
	NLMISC::fromString(args[0], shardId);
	IInterShardExchangeValidator::TLevelCap levelCap;
	NLMISC::fromString(args[1], levelCap);

	// make sure the numeric conversion was successfule for the 2 arguments
	if (NLMISC::toString(shardId)!=args[0] || NLMISC::toString(levelCap)!=args[1])
		return false;

	log.displayNL("Setting level cap for shard %u to: %u",(uint32)shardId,(uint32)levelCap);
	CInterShardExchangeValidator::getInstance()->setLevelCap(shardId,levelCap);

	return true;
}


//-------------------------------------------------------------------------------------------------

NLMISC_COMMAND(resetShardExchangeLimit, "reset the level limit for items exchanged between players from different shards","<shardId>" )
{
	// make sure we have 2 arguments
	if (args.size()!=1)
		return false;

	// get numeric values for the 2 arguments
	IInterShardExchangeValidator::TShardId shardId;
	NLMISC::fromString(args[0], shardId);

	// make sure the numeric conversion was successfule for the 2 arguments
	if (NLMISC::toString(shardId)!=args[0])
		return false;

	log.displayNL("Setting level cap for shard %u to: %u",(uint32)shardId,(uint32)CInterShardExchangeValidator::getInstance()->getDefaultLevelCap());
	CInterShardExchangeValidator::getInstance()->resetLevelCap(shardId);

	return true;
}


//-------------------------------------------------------------------------------------------------

NLMISC_COMMAND(displayShardExchangeLimits, "change base value of a spire effect","<effectName> <value>" )
{
	// make sure we have 2 arguments
	if (!args.empty())
		return false;

	CInterShardExchangeValidator::getInstance()->displayAllLevelCaps(log);

	return true;
}
