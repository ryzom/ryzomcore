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

#ifndef RY_NPC_DESCRIPTION_MESSAGES_H
#define RY_NPC_DESCRIPTION_MESSAGES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"
#include "nel/net/transport_class.h"
#include "game_share/msg_ais_egs_gen.h"
#include "game_share/base_types.h"
#include "game_share/synchronised_message.h"
#include "game_share/sp_type.h"


//----------------------------------------------------------------
// Logging and debugging stuff...
//----------------------------------------------------------------

// The following serve for controlling verbose nature of logging - LOG is undefined at end of file
extern bool VerboseNpcDescriptionMsgLog;
#define LOG if (!(VerboseNpcDescriptionMsgLog && Mode==4)) {} else nlinfo

// build a man-readable description of a chat parameter set from component parts
std::string buildChatDebugString(const std::vector<uint32> &ShopCategories,
								 const std::vector<NLMISC::CSheetId> &ItemTypesForSale,
								 const std::vector<uint32> &MissionIds);

//----------------------------------------------------------------
// class used to describe a bot's chat possibilities
//----------------------------------------------------------------

class CNpcChatProfile
{
public:
	CNpcChatProfile() : /*_guildCreator(false),*/ /*_dynamicMissionGiver(false),*/ _FilterExplicitActionTradeByPlayerRace(false), 
		_ExplicitActionSPType(EGSPD::CSPType::Unknown), _FilterExplicitActionTradeByBotRace(true),
		_DynamicMissionGiver(false), _Organization(0) {}
	CNpcChatProfile(const CNpcChatProfile &other0,const CNpcChatProfile &other1);
	virtual ~CNpcChatProfile() {}

	// interface for interpreting chat info
	const std::vector<uint32>			&getShopTypes()	const		{ return _ShopTypes; }
	const std::vector< RYMSG::TExplicitSale >	getExplicitSales() const	{ return _ExplicitSales; }
//	const std::vector<NLMISC::CSheetId>	&getShopItemTypes()	{ return _shopItemTypes; }
//	const std::vector<uint32>			&getShopItemQualities()	{ return _shopItemQualities; }
//	const std::vector<sint32>			&getShopItemPrices() const { return _shopItemPrices; }
//	const std::vector<uint8>			&getShopItemFactionTypes()	const { return _shopItemFactionTypes; }
//	const std::vector<uint32>			&getShopItemFactionPointPrices() const	{ return _shopItemFactionPointPrices; }
	const std::vector<uint32>			&getMissions()	const	{ return _Missions; }
//	bool								getGuildCreator() const{ return _GuildCreator; } 
	bool								getDynamicMissionGiver() const { return _DynamicMissionGiver;}
	const std::vector<std::string>		&getNewsChannels() const { return _NewsChannels; }
	const std::vector< RYMSG::TContextOption >	&getContextOptions() const	{ return _ContextOptions; };
//	const std::vector<std::string>		&getContextOptionsTitles() const  { return _ContextOptionsTitles;}
//	const std::vector<std::string>		&getContextOptionsDetails() const  { return _ContextOptionsDetails;}
	const std::vector<std::string>		&getOptionalProperties() const  { return _OptionalProperties;}

//	const std::vector<std::string>		&getShopNamedItemNames() const { return _shopNamedItemNames; }
//	const std::vector<uint32>			&getShopNamedItemPrices() const { return _shopNamedItemPrices; }
//	const std::vector<uint8>			&getShopNamedItemFactionTypes() const { return _shopNamedItemFactionTypes; }
//	const std::vector<uint32>			&getShopNamedItemFactionPointPrices() const { return _shopNamedItemFactionPointPrices; }

//	const std::vector<NLMISC::CSheetId>	&getExplicitActionTradeList() const	{ return _ExplicitActionTradeList; }
	bool								getFilterExplicitActionTradeByPlayerRace() const {return _FilterExplicitActionTradeByPlayerRace;}
	EGSPD::CSPType::TSPType				getExplicitActionSPType() const {return _ExplicitActionSPType;}
	bool								getFilterExplicitActionTradeByBotRace() const {return _FilterExplicitActionTradeByBotRace;}
	
	const std::string					&getWelcomePhrase() const	{ return _WelcomePhrase; }
	const std::string					&getWebPage() const			{ return _WebPage; }
	const std::string					&getWebPageName() const		{ return _WebPageName; }

	const NLMISC::CSheetId				&getOutpost() const			{ return _Outpost; }
	uint32								getOrganization() const		{ return _Organization; }
	
protected:
	std::vector< RYMSG::TExplicitSale >	_ExplicitSales;
//	std::vector<NLMISC::CSheetId>	_shopItemTypes;			// list of specific items to be found in shop
//	std::vector<uint32>				_shopItemQualities;		// list of quality of specific item
//	std::vector<sint32>				_shopItemPrices;		// list of specific item prices. -1 indicate "use standard price"
//	std::vector<uint8>				_shopItemFactionTypes;
//	std::vector<uint32>				_shopItemFactionPointPrices;
//	std::vector<NLMISC::CSheetId>	_shopItemTypesNegators;	// field used when combining chat profiles
//	std::vector<uint32>				_shopItemQualitiesNegators;	// list of quality of specific item
	std::vector<uint32>				_ShopTypes;				// categories of shop inventory to include
//	std::vector<uint32>				_shopTypesNegators;		// field used when combining chat profiles 

//	std::vector<std::string>		_shopNamedItemNames;
//	std::vector<uint32>				_shopNamedItemPrices;
//	std::vector<uint8>				_shopNamedItemFactionTypes;
//	std::vector<uint32>				_shopNamedItemFactionPointPrices;

//	std::vector<NLMISC::CSheetId>	_ExplicitActionTradeList;	// list of specific actions to be found in shop
	bool							_FilterExplicitActionTradeByPlayerRace;
	EGSPD::CSPType::TSPType			_ExplicitActionSPType;
	bool							_FilterExplicitActionTradeByBotRace;
	
	std::vector<uint32>				_Missions;				// array of mission aliases
//	std::vector<uint32>				_missionsNegators;		// field used when combining chat profiles
	std::string						_WelcomePhrase;			// The welcome message
	std::string						_WebPage;				// The web page
	std::string						_WebPageName;			// The web page name (server phrase id)
//	bool							_GuildCreator;			// true if the bot proposes guild creation
	bool							_DynamicMissionGiver;	// true if this bot give dynamic mission
	std::vector<std::string>		_NewsChannels;			// List of news channel.
	std::vector< RYMSG::TContextOption >	_ContextOptions;
	/// titles of the bot context options ContextOptionsDetails.size() == ContextOptionsTitles.size()
//	std::vector< std::string >		_ContextOptionsTitles;
	/// detail text of the option WARNING: we assert that ContextOptionsDetails.size() == ContextOptionsTitles.size()
//	std::vector< std::string >		_ContextOptionsDetails;

	/// optional and rare properties
	std::vector<std::string>		_OptionalProperties;
	
	NLMISC::CSheetId				_Outpost;

	uint32							_Organization;
};

//----------------------------------------------------------------
// List of Character starting chat or ending chat with bot

struct CCharacterBotChatBeginEnd : public CMirrorTransportClass
{
	// Container contains pair of Player character / bot which start or end chating
	std::vector< TDataSetRow > BotChatStart;
	std::vector< TDataSetRow > BotChatEnd;

	virtual void description ()
	{
		className ("CCharacterBotChatBeginEnd");
		propertyCont ("botChatStart",	PropUInt32, BotChatStart);
		propertyCont ("botChatEnd",		PropUInt32, BotChatEnd);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) { }
};

struct CCharacterDynChatBeginEnd : public CMirrorTransportClass
{
	// Container contains only bot which start or end chating
	std::vector< TDataSetRow > DynChatStart;
	std::vector< TDataSetRow > DynChatEnd;

	virtual void description ()
	{
		className ("CCharacterDynChatBeginEnd");
		propertyCont ("dnChatStart",	PropUInt32, DynChatStart);
		propertyCont ("dnChatEnd",		PropUInt32, DynChatEnd);
	}

	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* id */) { }
};

struct CCustomElementId
{
	CCustomElementId(uint32 alias = 0, const std::string &id = "") : PrimAlias(alias), Id(id) { }
	uint32		PrimAlias;
	std::string	Id;
	void serial(NLMISC::IStream &f);
};

inline bool operator<(const CCustomElementId &leftHand, const CCustomElementId &rightHand)
{
	if (leftHand.PrimAlias < rightHand.PrimAlias)
		return true;
	if (leftHand.PrimAlias > rightHand.PrimAlias)
		return false;
	if (leftHand.Id < rightHand.Id)
		return true;
	return false;
}

//all script lines
typedef std::vector<std::string> TScriptContent;
//the key is a pair<primAlias, UserModelId>, the value is the content of the script
typedef std::map<CCustomElementId, TScriptContent> TScripts;

//class used for serialization when receiving AIS user_model msg
//also used to represent custom loot set (key : drop proba, value : set script) 
struct CScriptData
{
	TScripts Scripts;
	void serial(NLMISC::IStream &f);
};

//used for serialization when receiving AIS custom loot table msg
//contains all loot sets of the table, and money infos used for dappers drop
struct CCustomLootTable
{
	CScriptData			LootSets;
	float				MoneyFactor;
	float				MoneyProba;
	uint32				MoneyBase;
	void serial(NLMISC::IStream &f);
};

//key : pair<primAlias, customLootTableId>, value : all associated custom loot sets + money info
typedef std::map<CCustomElementId, CCustomLootTable> TCustomLootTable;

struct CCustomLootTableManager
{
	TCustomLootTable	Tables;
	void serial(NLMISC::IStream &f);
};

#undef LOG

//----------------------------------------------------------------
#endif
