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



#ifndef RY_BOT_CHAT_TYPES_H
#define RY_BOT_CHAT_TYPES_H

namespace BOTCHATTYPE
{
	// bot chat flags (bit indices) to knw
	enum TBotChatFlags
	{
		NewsFlag = 0,			//1       0
		TradeItemFlag,			//2       1
		TradePactFlag,			//4       2
		TradeBuildingOptions,	//8       3 // used for guild, appart
		GuildRoleMaster,		//16      4
		TradePhraseFlag,		//32      5
		CreateGuildFlag,		//64      6
		ChooseMissionFlag,		//128     7
		Attackable,				//256     8 ( entity is attackable == (Attackable || CProperties::attackable ) )

		CancelGuildDutyFlag,	//512     9
		AskGuildDutyFlag,		//1024    10

		TradeFactionFlag,		//2048	  11
		TradeCosmeticFlag,		//4096    12
		TradeTeleportFlag,		//8192    13
		GuildInviteFlag,		//16384   14
		DontFollow,				//32768   15
		WebPageFlag,			//65536   16
		Totem,					//131072  17
		OutpostFlag,			//262144  18
		TradeOutpostBuilding,	//524288  19
		UnknownFlag,
	};

	// meaning of TRADING:i:SELLER_TYPE
	enum TBotChatSellerType
	{
		// this item is created and sold by the NPC. appears only in the "NPC list".
		// => PRICE_RETIRE and RESALE_TIME_LEFT are not relevant.
		NPC= 0,
		// this item was created and sold by a third-party Player. appears only in the "Resale list".
		// => PRICE_RETIRE and RESALE_TIME_LEFT are not relevant.
		Resale,
		// this item was created and sold by the player who whatch the list. appears only in the "User list".
		// => PRICE_RETIRE and RESALE_TIME_LEFT are relevant.
		User,
		// Same than user and item can be retired
		UserRetirable,
		// this item was created and sold by the player who whatch the list. appears only in the "Resale list".
		// but useful to know that it belongs to the player and therefore PRICE_RETIRE and RESALE_TIME_LEFT are relevant
		ResaleAndUser,
		// same then ResaleAndUser and item can be retired
		ResaleAndUserRetirable,

		NumBotChatSellerType,
		UnknownSellerType= NumBotChatSellerType
	};

	// meaning of INVENTORY:BAG:i:RESALE_FLAG
	enum TBotChatResaleFlag
	{
		// this item can be resold
		ResaleOk= 0,
		// this item can't be sold because it is partially broken
		ResaleKOBroken,
		// this item can't be sold because its Resold time has expired
		ResaleKONoTimeLeft,
		// this item can't be sold because the owner has locked it (temporary hack to get around modifying database.xml)
		ResaleKOLockedByOwner,

		NumBotChatResaleFlag
	};

	/** Recommended Price Factor (Client-side).
	 *	For instance if an item cost 100 (minimum price), the price recommended for resell is 200
	 */
	const	float	RecommendedPriceFactor= 2.0f;

	//R2
	const	uint32	MaxR2MissionEntryDatabase = 4;

} // BOTCHATTYPE

#endif // RY_NEWS_TYPES_H

/* End of bot_chat_types.h */
