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



#include "stdpch.h"
#include "npc_description_msg.h"
#include "ai.h"
#include "ai_instance.h"
#include "ai_mgr.h"

#include "game_share/pvp_clan.h"

using namespace std;
using namespace NLMISC;


CNpcChatProfileImp::CNpcChatProfileImp(const CNpcChatProfileImp &other0,const CNpcChatProfileImp &other1)
: CNpcChatProfile(static_cast<const CNpcChatProfile&>(other0), static_cast<const CNpcChatProfile&>(other1))
{}

CNpcChatProfileImp CNpcChatProfileImp::combineChatProfile(const CNpcChatProfileImp &referenceChat, const CNpcChatProfileImp &currentChat)
{
	CNpcChatProfileImp newProfile;
	{
		newProfile._ExplicitSales = referenceChat._ExplicitSales;
//		vector< RYMSG::TExplicitSale >::const_iterator first(referenceChat._ExplicitSales.begin()), last(referenceChat._ExplicitSales.end());
//		for (; first != last; ++first)
//		{
////			if (find(currentChat._shopItemTypesNegators.begin(), currentChat._shopItemTypesNegators.end(), *first) == currentChat._shopItemTypesNegators.end())
////			{
//			// also check quality
////			uint index = first - referenceChat._shopItemTypes.begin();
////			uint32 quality = referenceChat._shopItemQualities[index];
////			if (find(currentChat._shopItemQualitiesNegators.begin(), currentChat._shopItemQualitiesNegators.end(), quality) == currentChat._shopItemQualitiesNegators.end())
////			{
//			newProfile._ExplicitSales.push_back(first);
//			newProfile._shopItemTypes.push_back(*first);
//			newProfile._shopItemQualities.push_back(currentChat._shopItemQualities[index]);
//			newProfile._shopItemPrices.push_back(currentChat._shopItemPrices[index]);
//			newProfile._shopItemFactionTypes.push_back(currentChat._shopItemFactionTypes[index]);
//			newProfile._shopItemFactionPointPrices.push_back(currentChat._shopItemFactionPointPrices[index]);
////			}
////			}
//		}
	}
	{
		newProfile._ShopTypes = referenceChat._ShopTypes;
//		std::vector<uint32>::const_iterator first(referenceChat._shopTypes.begin()), last(referenceChat._shopTypes.end());
//		for (; first != last; ++first)
//		{
////			if (find(currentChat._shopTypesNegators.begin(), currentChat._shopTypesNegators.end(), *first) == currentChat._shopTypesNegators.end())
////			{
//			newProfile._shopTypes.push_back(*first);
////			}
//		}
	}
	{
		newProfile._Missions = referenceChat._Missions;
//		std::vector<uint32>::const_iterator first(referenceChat._missions.begin()), last(referenceChat._missions.end());
//		for (; first != last; ++first)
//		{
////			if (find(currentChat._missionsNegators.begin(), currentChat._missionsNegators.end(), *first) == currentChat._missionsNegators.end())
////			{
//			newProfile._missions.push_back(*first);
////			}
//		}
	}
	// this properties are read from current chat profile...
	newProfile._CellZones = currentChat._CellZones;
	newProfile._NewsChannels = currentChat._NewsChannels;
//	newProfile._GuildCreator = currentChat._GuildCreator;
	newProfile._WelcomePhrase = currentChat._WelcomePhrase;
	newProfile._WebPage = currentChat._WebPage;
	newProfile._WebPageName = currentChat._WebPageName;
//	newProfile._ExplicitActionTradeList = currentChat._ExplicitActionTradeList;
	newProfile._FilterExplicitActionTradeByPlayerRace = currentChat._FilterExplicitActionTradeByPlayerRace;
	newProfile._ExplicitActionSPType = currentChat._ExplicitActionSPType;
	newProfile._FilterExplicitActionTradeByBotRace = currentChat._FilterExplicitActionTradeByBotRace;
	
	// this properties are directly merged
	newProfile._OptionalProperties = referenceChat._OptionalProperties;
	newProfile._OptionalProperties.insert(newProfile._OptionalProperties.end(), currentChat._OptionalProperties.begin(), currentChat._OptionalProperties.end());

	return newProfile;
}

//---------------------------------------------------------------------------------------
// Management of shop type names
//---------------------------------------------------------------------------------------

static std::vector<std::string> ShopTypeNames;
static std::map<std::string,std::string> ShopNameAliases;

// try to add an entry corresponding to shop name 'name' to vector 'shopList'
static bool LookupShopType(std::string name,std::vector<uint32> &shopList)
{
	static bool init=false;

	// if this is the first call to the routine go and read the config file that contains the shop list
	if (!init)
	{
		init=true;

		try
		{
			NLMISC::CConfigFile ShopConfigFile;
			const string path = NLMISC::CPath::lookup( "shop_category.cfg", false, true, true );
			if (path.empty())
				return false;
			ShopConfigFile.load( path );

			// read the vector of shop names (in their long hand format)
			CConfigFile::CVar& cvShopType = ShopConfigFile.getVar("ShopCategory");
			uint	i;
			for ( i=0; i < cvShopType.size(); ++i )
			{
				if ( !cvShopType.asString(i).empty() )
				{
					// make sure the string doesn't turn up more than once in input data
					for (uint j=0;j<ShopTypeNames.size();++j)
						if (NLMISC::nlstricmp(ShopTypeNames[j],cvShopType.asString(i))==0)
						{
							nlwarning("Shop name '%s' comes up more than once in config file: shop_category.cfg",cvShopType.asString(i).c_str());
							continue;
						}
					// add the shop name
					ShopTypeNames.push_back( cvShopType.asString( i ) );
				}
			}

			// read the map of shorthand aliases for shop names
			CConfigFile::CVar& cvNameAliases = ShopConfigFile.getVar("ShopNameAliases");
			for ( i=0; i < cvNameAliases.size(); ++i )
			{
				std::string alias, tail;
				if (AI_SHARE::stringToKeywordAndTail(cvNameAliases.asString( i ),alias,tail))
				{
					if (ShopNameAliases.find(alias)!=ShopNameAliases.end())
					{
						nlwarning("Shop name alias '%s' declared more than once in file: shop_category.cfg",alias.c_str());
						continue;
					}
					// add the alias
					ShopNameAliases[alias]=tail;
				}
			}
		}
		catch(const EUnknownVar &) 
		{
			nlwarning("<CShopTypeManager::initShopBase> shop_category.cfg or ShopCategory not found, no shop base type intialized...");
		}
	}

	// deal with multi-part strings
	std::string clause,tail;
	AI_SHARE::stringToKeywordAndTail(name,clause,tail);
	if (!tail.empty())
	{
		bool result=true;
		do
		{
			result = LookupShopType(clause,shopList) && result;
		}
		while (result && AI_SHARE::stringToKeywordAndTail(tail,clause,tail));
		return result;
	}

	// lookup the name in the alias map - if found use the alias (may be multi-part)
	if (ShopNameAliases.find(name)!=ShopNameAliases.end())
		return LookupShopType(ShopNameAliases[name],shopList);

	// look through the names vector for a name match
	for (uint i=0;i<ShopTypeNames.size();++i)
	{
		if (NLMISC::nlstricmp(name,ShopTypeNames[i])==0)
		{
			// make sure we don't already have this entry in the vector
			for (uint j=0;j<shopList.size();++j)
				if (shopList[j]==i)
					return false;

			// add the new entry to the vector
			shopList.push_back(i);

			return true;
		}
	}

	// failed to find a match so return the 'not found' index
	return false;
}



//---------------------------------------------------------------------------------------
// Debug string construction
//---------------------------------------------------------------------------------------

std::string buildChatDebugString(const std::vector<uint32> &ShopCategories,
								 const std::vector<NLMISC::CSheetId> &ItemTypesForSale,
								 const std::vector<uint32> &MissionIds)
{
	std::string s;
	if (!ShopCategories.empty())
	{
		if (ShopCategories[0]>=ShopTypeNames.size())
			s+="shopTypes(ERROR";
		else
			s+=NLMISC::toString(" shopTypes(%s",ShopTypeNames[ShopCategories[0]].c_str());
		for (uint i=1;i<ShopCategories.size();++i)
		{
			if (ShopCategories[i]>=ShopTypeNames.size())
				s+=",ERROR";
			else
				s+=NLMISC::toString(",%s",ShopTypeNames[ShopCategories[i]].c_str());
		}
		s+=")";
	}

	if (!ItemTypesForSale.empty())
	{
		s+=NLMISC::toString(" shopTypes(%u",ItemTypesForSale[0].toString().c_str());
		for (uint i=1;i<ItemTypesForSale.size();++i)
			s+=NLMISC::toString(",%s",ItemTypesForSale[i].toString().c_str());
		s+=")";
	}

	if (!MissionIds.empty())
	{
		s+=NLMISC::toString(" missions(%u",MissionIds[0]);
		for (uint i=1;i<MissionIds.size();++i)
			s+=NLMISC::toString(",%u",MissionIds[i]);
		s+=")";
	}

	if (s.empty())
		s=" No chat parameters";

	return s;
}

//---------------------------------------------------------------------------------------
// Management of chat arguments
//---------------------------------------------------------------------------------------

bool	CNpcChatProfileImp::parseChatArgs(CAIInstance	*aiInstance, const std::string &args)
{
	CSString keyword, tail;

	// check that the argument string isn't empty (if empty just smile and continue)
	if	(!AI_SHARE::stringToWordAndTail(args,keyword,tail))
		return	true;

	// see whether this is a single keyword that we recognise
	if	(	tail.empty()
		&&	(	NLMISC::nlstricmp(keyword,"enable")==0
			||	NLMISC::nlstricmp(keyword,"disable")==0
			))
	{
		nlwarning("*** chat enable/ disable NOT IMPLEMENTED YET ***");
		return	true;
	}
	
	// split the input string into a keyword and tail separated by a colon (if none found throw an error)
	if	(!AI_SHARE::stringToKeywordAndTail(args,keyword,tail))
		return	false;

	if (NLMISC::nlstricmp(keyword, "OP")==0)
	{
		_OptionalProperties.push_back(tail);
		return	true;
	}

	if (NLMISC::nlstricmp(keyword,"shop")==0)
	{
		return LookupShopType(tail,_ShopTypes);
	}

//	if (NLMISC::nlstricmp(keyword,"-shop")==0)
//	{
//		return LookupShopType(tail,_shopTypesNegators);
//	}

	// deal with 'item' and '-item' keywords
	if (NLMISC::nlstricmp(keyword,"item")==0)
	{
		string sheet, sTmp;
		uint32 quality;
		sint32 price= -1;		// init with -1 by default because means "use standard price compute"
		PVP_CLAN::TPVPClan factionType = PVP_CLAN::None;
		uint32 factionPointPrice = 0;
		
		AI_SHARE::stringToWordAndTail(tail, sheet, tail);
		AI_SHARE::stringToWordAndTail(tail, sTmp, tail);

		quality = NLMISC::atoui(sTmp.c_str());

		// try to convert our text to a sheet id
		NLMISC::CSheetId	sheetId;

		if	(CFile::getExtension(sheet)=="sitem")
			sheetId=NLMISC::CSheetId(sheet);
		else
			sheetId=NLMISC::CSheetId(sheet+".sitem");
		
		if (sheetId==NLMISC::CSheetId::Unknown)
			return false;

		// make sure we don't already have this entry in the vector
		for (uint i=0;i<_ExplicitSales.size();++i)
			if (_ExplicitSales[i].getSheetId() ==sheetId && _ExplicitSales[i].getQuality() == quality)
				return false;

		// parse optional item price
		if(AI_SHARE::stringToWordAndTail(tail, sTmp, tail))
		{
			NLMISC::fromString(sTmp, price);
		}
				
		// parse optional Faction type and point
		if(AI_SHARE::stringToWordAndTail(tail, sTmp, tail))
		{
			factionType = PVP_CLAN::fromString(sTmp);
			if(AI_SHARE::stringToWordAndTail(tail, sTmp, tail))
			{
				factionPointPrice = (PVP_CLAN::TPVPClan)NLMISC::atoui(sTmp.c_str());
			}
			else
				factionType = PVP_CLAN::None;
		}
		
		// add the new entry to the vector
		RYMSG::TExplicitSale itemToSale;
		itemToSale.setSaleType(RYMSG::TExplicitSaleType::est_item);
		itemToSale.setSheetId(sheetId);
		itemToSale.setQuality(quality);
		itemToSale.setQuantity(1);
		if (factionType != PVP_CLAN::None)
		{
			// set the price in faction points
			itemToSale.getPrice().setCurrency(RYMSG::TTradeCurrency::tc_faction_points);
			itemToSale.getPrice().setFaction(factionType);
			itemToSale.getPrice().setAmount(factionPointPrice);
		}
		else
		{
			// set the price in dappers
			itemToSale.getPrice().setCurrency(RYMSG::TTradeCurrency::tc_dappers);
			itemToSale.getPrice().setAmount(price);
		}

		_ExplicitSales.push_back(itemToSale);

		return	true;
	}
	
//	if (NLMISC::nlstricmp(keyword,"-item")==0)
//	{
//		string sheet, sTmp;
//		uint32 quality;
//		
//		AI_SHARE::stringToWordAndTail(tail, sheet, tail);
//		AI_SHARE::stringToWordAndTail(tail, sTmp, tail);
//
//		quality = NLMISC::atoui(sTmp.c_str());
//		// try to convert our text to a sheet id
//		NLMISC::CSheetId sheetId(sheet);
//		if (sheetId==NLMISC::CSheetId::Unknown)
//			sheetId=NLMISC::CSheetId(sheet+".sitem");
//		if (sheetId!=NLMISC::CSheetId::Unknown)
//			return false;
//		// make sure we don't already have this entry in the vector
//		for (uint i=0;i<_shopItemTypesNegators.size();++i)
//			if (_shopItemTypesNegators[i]==sheetId && _shopItemQualitiesNegators[i] == quality)
//				return false;
//
//		// add the new entry to the vector
//		_shopItemTypesNegators.push_back(sheetId);
//		_shopItemQualitiesNegators.push_back(quality);
//		return	true;
//	}

	// deal with 'nitem' keywords
	if (NLMISC::nlstricmp(keyword,"nitem") == 0)
	{
		string namedItemName, sTmp;
		uint32 price = 0;
		PVP_CLAN::TPVPClan factionType = PVP_CLAN::None;
		uint32 factionPointPrice = 0;

		AI_SHARE::stringToWordAndTail(tail, namedItemName, tail);

		AI_SHARE::stringToWordAndTail(tail, sTmp, tail);
		price = NLMISC::atoui(sTmp.c_str());

		AI_SHARE::stringToWordAndTail(tail, sTmp, tail);
		factionType = PVP_CLAN::fromString(sTmp);

		AI_SHARE::stringToWordAndTail(tail, sTmp, tail);
		factionPointPrice = NLMISC::atoui(sTmp.c_str());

		// add the new entry to the vector
		RYMSG::TExplicitSale namedItem;
		namedItem.setSaleType(RYMSG::TExplicitSaleType::est_named_item);

		namedItem.setNamed(namedItemName);
		namedItem.setQuantity(1);
		if (factionType != PVP_CLAN::None)
		{
			// set the price in faction points
			namedItem.getPrice().setCurrency(RYMSG::TTradeCurrency::tc_faction_points);
			namedItem.getPrice().setFaction(factionType);
			namedItem.getPrice().setAmount(factionPointPrice);
		}
		else
		{
			// set the price in dappers
			namedItem.getPrice().setCurrency(RYMSG::TTradeCurrency::tc_dappers);
			namedItem.getPrice().setAmount(price);
		}

		_ExplicitSales.push_back(namedItem);
		return	true;
	}

	// deal with 'phrase' keywords
	if (NLMISC::nlstricmp(keyword, "phrase") == 0 )
	{
		// try to convert our text to a sheet id
		NLMISC::CSheetId	sheetId;
		
		if	(CFile::getExtension(tail)=="sphrase")
			sheetId=NLMISC::CSheetId(tail);
		else
			sheetId=NLMISC::CSheetId(tail+".sphrase");
		
		if (sheetId==NLMISC::CSheetId::Unknown)
			return false;

		// make sure we don't already have this entry in the vector
		for (uint i=0;i<_ExplicitSales.size();++i)
			if(_ExplicitSales[i].getSheetId() == sheetId)
				return false;
			
		// then add to list 
		RYMSG::TExplicitSale phrase;
		phrase.setSaleType(RYMSG::TExplicitSaleType::est_phrase);
		phrase.setSheetId(sheetId);
		_ExplicitSales.push_back(phrase);
		return true;
	}

	// deal with 'sale' keyword
	if (NLMISC::nlstricmp(keyword, "sale") == 0 )
	{
		// the list of prices info (will generate one item for each price)
		vector<RYMSG::TPriceInfo> priceInfos;
		CVectorSString parts;
		tail.splitBySeparator(';', parts);

		RYMSG::TExplicitSale sale;
		// read the parts
		for (uint i=0; i<parts.size(); ++i)
		{
			switch (i)
			{
			case 0:
				// read the type of sale
				sale.setSaleType(RYMSG::TExplicitSaleType::fromFileTag(parts[0]));
				if (sale.getSaleType() == RYMSG::TExplicitSaleType::invalid_val)
					return false;
				break;
			case 1:
				// read the faction price factor
				if (!parts[i].empty())
				{
					CVectorSString factors;
					parts[i].splitBySeparator(',', factors);

					for (uint j=0; j<factors.size(); ++j)
					{
						CSString factionName = factors[j].firstWord(true);
						CSString factor = factors[j].firstWord(true);

						PVP_CLAN::TPVPClan faction = PVP_CLAN::fromString(factionName);

						if (factor != 0 || faction == PVP_CLAN::Unknown)
							return false;

						RYMSG::TPriceFactionFactor pff;
						pff.setFaction(faction);
						pff.setFactor(float(atof(factor.c_str())));

						sale.getPrice().getFactionFactors().push_back(pff);
					}
				}
				break;
			case 2:
				// read the sheet id, item name or quantity
				switch (sale.getSaleType().getValue())
				{
				case RYMSG::TExplicitSaleType::est_item:
					sale.setSheetId(CSheetId(parts[i]+".sitem"));
					if (sale.getSheetId() == CSheetId::Unknown)
						return false;
					break;
				case RYMSG::TExplicitSaleType::est_named_item:
					sale.setNamed(parts[i]);
					if (sale.getNamed().empty())
						return false;
					break;
				case RYMSG::TExplicitSaleType::est_brick:
					sale.setSheetId(CSheetId(parts[i]+".sbrick"));
					if (sale.getSheetId() == CSheetId::Unknown)
						return false;
					break;
				case RYMSG::TExplicitSaleType::est_phrase:
					sale.setSheetId(CSheetId(parts[i]+".sphrase"));
					if (sale.getSheetId() == CSheetId::Unknown)
						return false;
					break;
				case RYMSG::TExplicitSaleType::est_dappers:
				{
					// read the amount of dapper
					uint32 quantity;
					NLMISC::fromString(parts[i], quantity),
					sale.setQuantity(quantity);
					break;
				}
				default:
					STOP("Unsupported sale type "<<sale.getSaleType().toString());
				}
				break;
			case 3:
			{
				// read the quality
				uint32 quality;
				NLMISC::fromString(parts[i], quality);
				sale.setQuality(quality);
				break;
			}
			case 4:
				// read the money (may be more than one)
				if (!parts[i].empty())
				{
					CVectorSString prices;
					parts[i].splitBySeparator(',', prices);

					for (uint j=0; j<prices.size(); ++j)
					{
						RYMSG::TPriceInfo	priceInfo;
						CSString money = prices[j].firstWord(true).strip();
						CSString param2 = prices[j].firstWord().strip();

						// check the money type
						if (money.size() == 4 && 
							(	money == "RRP1"
								|| money == "RRP2"
								|| money == "RRP3"
								|| money == "RRP4"
								|| money == "RRP5"
								|| money == "RRP6"))
						{
							// money is RRP
							priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_rrps);
							priceInfo.setRRPLevel((R2::TSessionLevel::TValues)(money[3]-'0'));
							sint32 amount;
							NLMISC::fromString(param2, amount);
							priceInfo.setAmount(amount);
						}
						else if (money == "dappers")
						{
							// money is dappers
							priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_dappers);
							sint32 amount;
							NLMISC::fromString(param2, amount);
							priceInfo.setAmount(amount);
						}
						else if (money == "item")
						{
							// money is an item sheet
							priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_item);
							priceInfo.setAmount(1);
							
							if (param2.empty())
							{
								nlwarning("No sheet name supplied following 'item' keyword in sale value: %s",args.c_str());
								priceInfo.setSheetId(CSheetId::Unknown);
							}
							else
							{
								priceInfo.setSheetId(CSheetId(param2));
							}
							if (priceInfo.getSheetId() == CSheetId::Unknown)
								return false;
						}
						else if (money == "faction_points")
						{
							CSString param3 = prices[j].firstWord();
							// money is an item sheet
							priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_faction_points);
							priceInfo.setFaction(PVP_CLAN::fromString(param2));
							sint32 amount;
							NLMISC::fromString(param3, amount);
							priceInfo.setAmount(amount);
							if (priceInfo.getFaction() == PVP_CLAN::Unknown)
								return false;
						}
						else if (money == "skill")
						{
							CSString param3 = prices[j].firstWord();
							// money is an item sheet
							priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_skill_points);
							priceInfo.setSkillType(EGSPD::CSPType::fromString(param2));
							sint32 amount;
							NLMISC::fromString(param3, amount);
							priceInfo.setAmount(amount);
							if (priceInfo.getSkillType() == EGSPD::CSPType::Unknown)
								return false;
						}
						else
						{
							STOP("Invalid price type '"<<money<<"'");
							return false;
						}

						// store the price
						priceInfos.push_back(priceInfo);
					}
				}
				break;
			case 5:
				// read the frequency
				// TODO

				break;

			default:
				// hum ?
				STOP("Invalid number of part in the sale record");
			}
		}

		// build the list of item to sell according to the list of prices
		for (uint i=0; i<priceInfos.size(); ++i)
		{
			_ExplicitSales.push_back(sale);
			_ExplicitSales.back().setPrice(priceInfos[i]);
		}
	}

	// deal with 'phrase_race_filter' keywords
	if (NLMISC::nlstricmp(keyword, "phrase_race_filter") == 0 )
	{
		NLMISC::fromString(tail, _FilterExplicitActionTradeByPlayerRace);
	}

	// deal with 'phrase_type' keywords
	if (NLMISC::nlstricmp(keyword, "phrase_type") == 0 )
	{
		_ExplicitActionSPType= EGSPD::CSPType::fromString(tail);
	}

	// deal with 'phrase_bot_race_filter' keywords
	if (NLMISC::nlstricmp(keyword, "phrase_bot_race_filter") == 0 )
	{
		NLMISC::fromString(tail, _FilterExplicitActionTradeByBotRace);
	}
	
	// deal with '+mission' and '-mission' keywords
	if (NLMISC::nlstricmp(keyword,"+mission")==0)
	{
		nlwarning("*** missions listed in chat NOT IMPLEMENTED YET ***");
		return	true;
	}

//	if (NLMISC::nlstricmp(keyword,"-mission")==0)
//	{
//		if	(tail.empty())
//			return	true;
//
//		aiInstance->findMissionAlias(_missionsNegators, tail);
//
//		if	(_missionsNegators.empty())
//			nlwarning("Can't found alias for mission '%s' !", tail.c_str());
//
//		return	true;
//	}

	// welcome message
	if (NLMISC::nlstricmp(keyword, "welcome")==0)
	{
		if (!_WelcomePhrase.empty())
			nlwarning("Welcome phrase ('%s') already set in chat ! Overwriting with '%s'", _WelcomePhrase.c_str(), tail.c_str());

		_WelcomePhrase = tail;
	}

	// web page
	if (NLMISC::nlstricmp(keyword, "web")==0)
	{
		if (!_WebPage.empty())
			nlwarning("Web Page ('%s') already set in chat ! Overwriting with '%s'", _WebPage.c_str(), tail.c_str());
		
		// split to webpage name and the true webpage
		_WebPageName.clear();
		AI_SHARE::stringToWordAndTail(tail, _WebPageName, tail);
		_WebPage = tail;
	}
	
	// guild creator
	if (NLMISC::nlstricmp(keyword, "guild")==0)
	{
		BOMB("No more supported option", return false);
//		_GuildCreator = tail == "true" || tail == "1" || tail =="on";
//		return	true;
	}

	// dynamic mission cell zones names
	if (NLMISC::nlstricmp(keyword, "dyn_mis") == 0)
	{
		string	word;
		while (AI_SHARE::stringToWordAndTail(tail, word, tail))
		{
			_CellZones.push_back(word);
		}

		_DynamicMissionGiver = true;
		return	true;
	}
	// news channel names
	if (NLMISC::nlstricmp(keyword, "news") == 0)
	{
		string	word;
		while (AI_SHARE::stringToWordAndTail(tail, word, tail))
		{
			_NewsChannels.push_back(word);
		}
		if (_NewsChannels.empty())
		{
			// no channels name, push an empty string to tag that we must use the local info channel (ie outpost or sector)
			_NewsChannels.push_back(string());
		}
		return	true;
	}
	// context menu entry
	if (NLMISC::nlstricmp(keyword, "menu") == 0)
	{
		string	title;
		while (AI_SHARE::stringToWordAndTail(tail, title, tail))
		{
			string detail;
			if (!AI_SHARE::stringToWordAndTail(tail, detail, tail))
			{
				nlwarning("Mission detail string ID in 'menu' entry for title '%s'", title.c_str());
				return false;
			}
			RYMSG::TContextOption co;
			co.setTitle(title);
			co.setDetail(detail);
			_ContextOptions.push_back(co);
		}
		return	true;
	}

	// outpost entry
	if (NLMISC::nlstricmp(keyword, "outpost") == 0)
	{
		string	word;
		while (AI_SHARE::stringToWordAndTail(tail, word, tail))
		{
			_Outpost = NLMISC::CSheetId(word+".outpost");
		}
		return	true;
	}
	
	// organization entry
	if (NLMISC::nlstricmp(keyword, "organization") == 0)
	{
		NLMISC::fromString(tail, _Organization);
		return	true;
	}

	// if no match found throw an error
	return false;
}

void	TGenNpcDescMsgImp::setChat(const CNpcChatProfileImp& chatProfile)
{
	_MissionIds				=	chatProfile.getMissions();
	_ExplicitSales			=	chatProfile.getExplicitSales();
//	ItemTypesForSale		=	chatProfile.getShopItemTypes();
//	ItemQualitiesForSale	=	chatProfile.getShopItemQualities();
//	ItemPrices				=	chatProfile.getShopItemPrices();
//	ItemFactionType			=	chatProfile.getShopItemFactionTypes();
//	ItemFactionPointPrice	=	chatProfile.getShopItemFactionPointPrices();
	_ShopCategories			=	chatProfile.getShopTypes();
	
//	NamedItemName				= chatProfile.getShopNamedItemNames();
//	NamedItemPrice				= chatProfile.getShopNamedItemPrices();
//	NamedItemFactionType		= chatProfile.getShopNamedItemFactionTypes();
//	NamedItemFactionPointPrice	= chatProfile.getShopNamedItemFactionPointPrices();

//	ExplicitActionTradeList =	chatProfile.getExplicitActionTradeList();
	_FilterExplicitActionTradeByPlayerRace =	chatProfile.getFilterExplicitActionTradeByPlayerRace();
	_ExplicitActionSPType =	chatProfile.getExplicitActionSPType();

	_FilterExplicitActionTradeByBotRace = chatProfile.getFilterExplicitActionTradeByBotRace();
	
	_WelcomePhrase			=	chatProfile.getWelcomePhrase();
	_WebPage					=	chatProfile.getWebPage();
	_WebPageName				=	chatProfile.getWebPageName();
//	_GuildCreator			=	chatProfile.getGuildCreator()	();
	_DynamicMissionGiver		=	chatProfile.getDynamicMissionGiver(); //!chatProfile.getCellZones().empty();		
	_NewsChannels			=	chatProfile.getNewsChannels();
	
	_ContextOptions			=	chatProfile.getContextOptions();
//	ContextOptionsTitles	=	chatProfile.getContextOptionsTitles();
//	ContextOptionsDetails	=	chatProfile.getContextOptionsDetails();

	vector<string> const& chatOptionalProperties = chatProfile.getOptionalProperties();
	_OptionalProperties.insert(_OptionalProperties.end(), chatOptionalProperties.begin(), chatOptionalProperties.end());

	_Outpost					=	chatProfile.getOutpost();
	_Organization					=	chatProfile.getOrganization();
}
