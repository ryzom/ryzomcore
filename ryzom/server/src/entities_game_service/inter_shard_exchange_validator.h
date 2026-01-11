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

#ifndef INTER_SHARD_EXCHANGE_VALIDATOR_H
#define INTER_SHARD_EXCHANGE_VALIDATOR_H


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// NeL MISC
#include "nel/misc/singleton.h"


//-------------------------------------------------------------------------------------------------
// forward declarations
//-------------------------------------------------------------------------------------------------

class CGameItemPtr;


//-------------------------------------------------------------------------------------------------
// singleton class IInterShardExchangeValidator
//-------------------------------------------------------------------------------------------------

class IInterShardExchangeValidator
{
public:
	// public types
	typedef uint32 TShardId;
	typedef uint32 TLevelCap;

	/// validate an item for exchange
	virtual bool isExchangeAllowed(const CGameItemPtr& theItem, TShardId shardId0, TShardId shardId1) const =0;

	/// validate money for exchange
	virtual bool isMoneyExchangeAllowed(TShardId shardId0, TShardId shardId1) const =0;

	/// Singleton accessor
	static IInterShardExchangeValidator* getInstance();
};


#endif // INTER_SHARD_EXCHANGE_VALIDATOR_H
