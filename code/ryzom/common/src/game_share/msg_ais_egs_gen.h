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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef MSG_AIS_EGS_GEN
#define MSG_AIS_EGS_GEN
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "game_share/synchronised_message.h"

#include "nel/misc/entity_id.h"

#include "pvp_clan.h"

#include "base_types.h"

#include "sp_type.h"

#include "game_share/r2_share_itf.h"

namespace RYMSG
{

	class TPriceFactionFactor;

	class TPriceInfo;

	class TExplicitSale;

	class TContextOption;

	class TGenNpcDescMsg;



	struct TTradeCurrency
	{
		enum TValues
		{
			tc_dappers,
			tc_rrps,
			tc_item,
			tc_faction_points,
			tc_skill_points,
			/// the highest valid value in the enum
			last_enum_item = tc_skill_points,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 5
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(tc_dappers, 0));
				indexTable.insert(std::make_pair(tc_rrps, 1));
				indexTable.insert(std::make_pair(tc_item, 2));
				indexTable.insert(std::make_pair(tc_faction_points, 3));
				indexTable.insert(std::make_pair(tc_skill_points, 4));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(tc_dappers)
				NL_STRING_CONVERSION_TABLE_ENTRY(tc_rrps)
				NL_STRING_CONVERSION_TABLE_ENTRY(tc_item)
				NL_STRING_CONVERSION_TABLE_ENTRY(tc_faction_points)
				NL_STRING_CONVERSION_TABLE_ENTRY(tc_skill_points)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TTradeCurrency()
			: _Value(invalid_val)
		{
		}
		TTradeCurrency(TValues value)
			: _Value(value)
		{
		}

		TTradeCurrency(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TTradeCurrency &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TTradeCurrency &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TTradeCurrency &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TTradeCurrency &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TTradeCurrency &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TTradeCurrency &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TExplicitSaleType
	{
		enum TValues
		{
			est_item,
			est_named_item,
			est_brick,
			est_phrase,
			est_dappers,
			/// the highest valid value in the enum
			last_enum_item = est_dappers,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 5
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(est_item, 0));
				indexTable.insert(std::make_pair(est_named_item, 1));
				indexTable.insert(std::make_pair(est_brick, 2));
				indexTable.insert(std::make_pair(est_phrase, 3));
				indexTable.insert(std::make_pair(est_dappers, 4));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(est_item)
				NL_STRING_CONVERSION_TABLE_ENTRY(est_named_item)
				NL_STRING_CONVERSION_TABLE_ENTRY(est_brick)
				NL_STRING_CONVERSION_TABLE_ENTRY(est_phrase)
				NL_STRING_CONVERSION_TABLE_ENTRY(est_dappers)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TExplicitSaleType()
			: _Value(invalid_val)
		{
		}
		TExplicitSaleType(TValues value)
			: _Value(value)
		{
		}

		TExplicitSaleType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TExplicitSaleType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TExplicitSaleType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TExplicitSaleType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TExplicitSaleType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TExplicitSaleType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TExplicitSaleType &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}



		static TExplicitSaleType fromFileTag(const std::string &tag)
		{
			TExplicitSaleType ret;
			if (tag == "item")
				ret._Value = est_item;
			else if (tag == "named_item")
				ret._Value = est_named_item;
			else if (tag == "brick")
				ret._Value = est_brick;
			else if (tag == "phrase")
				ret._Value = est_phrase;
			else if (tag == "dappers")
				ret._Value = est_dappers;
			else
				ret._Value = invalid_val;

			return ret;
		}



	};
		// A price factor that depends on the buyer faction
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TPriceFactionFactor
	{
	protected:
		// Specify the faction
		PVP_CLAN::TPVPClan	_Faction;
		// the factor to apply to the price
		float	_Factor;
	public:
		// Specify the faction
		PVP_CLAN::TPVPClan getFaction() const
		{
			return _Faction;
		}

		void setFaction(PVP_CLAN::TPVPClan value)
		{

				_Faction = value;

		}
			// the factor to apply to the price
		float getFactor() const
		{
			return _Factor;
		}

		void setFactor(float value)
		{

				_Factor = value;

		}

		bool operator == (const TPriceFactionFactor &other) const
		{
			return _Faction == other._Faction
				&& _Factor == other._Factor;
		}


		// constructor
		TPriceFactionFactor()
		{
			// Default initialisation
			_Factor = 1;

		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Faction);
			s.serial(_Factor);

		}


	private:


	};


		// A price for an explicit sale
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TPriceInfo
	{
	protected:
		// Specify the type of currency
		TTradeCurrency	_Currency;
		// The amount of the specified currency. -1 for dappers mean 'use normal price'
		sint32	_Amount;
		// For RRPs, specify the level required
		R2::TSessionLevel	_RRPLevel;
		// SheetId for item currency (must be an item sheet)
		NLMISC::CSheetId	_SheetId;
		// The skill type for skill points currency
		EGSPD::CSPType::TSPType	_SkillType;
		// For price in faction points, specify the faction
		PVP_CLAN::TPVPClan	_Faction;
		// A list of price faction factor
		std::vector< TPriceFactionFactor >	_FactionFactors;
	public:
		// Specify the type of currency
		TTradeCurrency getCurrency() const
		{
			return _Currency;
		}

		void setCurrency(TTradeCurrency value)
		{

				_Currency = value;

		}
			// The amount of the specified currency. -1 for dappers mean 'use normal price'
		sint32 getAmount() const
		{
			return _Amount;
		}

		void setAmount(sint32 value)
		{

				_Amount = value;

		}
			// For RRPs, specify the level required
		R2::TSessionLevel getRRPLevel() const
		{
			return _RRPLevel;
		}

		void setRRPLevel(R2::TSessionLevel value)
		{

				_RRPLevel = value;

		}
			// SheetId for item currency (must be an item sheet)
		const NLMISC::CSheetId &getSheetId() const
		{
			return _SheetId;
		}

		NLMISC::CSheetId &getSheetId()
		{
			return _SheetId;
		}


		void setSheetId(const NLMISC::CSheetId &value)
		{


				_SheetId = value;


		}
			// The skill type for skill points currency
		EGSPD::CSPType::TSPType getSkillType() const
		{
			return _SkillType;
		}

		void setSkillType(EGSPD::CSPType::TSPType value)
		{

				_SkillType = value;

		}
			// For price in faction points, specify the faction
		PVP_CLAN::TPVPClan getFaction() const
		{
			return _Faction;
		}

		void setFaction(PVP_CLAN::TPVPClan value)
		{

				_Faction = value;

		}
			// A list of price faction factor
		std::vector< TPriceFactionFactor > getFactionFactors() const
		{
			return _FactionFactors;
		}

		void setFactionFactors(std::vector< TPriceFactionFactor > value)
		{

				_FactionFactors = value;

		}

		bool operator == (const TPriceInfo &other) const
		{
			return _Currency == other._Currency
				&& _Amount == other._Amount
				&& _RRPLevel == other._RRPLevel
				&& _SheetId == other._SheetId
				&& _SkillType == other._SkillType
				&& _Faction == other._Faction
				&& _FactionFactors == other._FactionFactors;
		}


		// constructor
		TPriceInfo()
		{
			// Default initialisation
			_Currency = TTradeCurrency::tc_dappers;
			_Amount = -1;
			_RRPLevel = R2::TSessionLevel::invalid_val;
			_SheetId = NLMISC::CSheetId::Unknown;
			_SkillType = EGSPD::CSPType::Unknown;
			_Faction = PVP_CLAN::None;

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_Currency);
			s.serial(_Amount);
			s.serial(_RRPLevel);
			s.serial(_SheetId);
			s.serialEnum(_SkillType);
			s.serialEnum(_Faction);
			s.serialCont(_FactionFactors);

		}


	private:


	};


		// Description of somethink explicitly sold by the NPC
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TExplicitSale
	{
	protected:
		//
		TExplicitSaleType	_SaleType;
		//
		NLMISC::CSheetId	_SheetId;
		// Name of item for named item
		std::string	_Named;
		//
		uint32	_Quantity;
		//
		uint32	_Quality;
		// The price of the item
		TPriceInfo	_Price;
	public:
		//
		TExplicitSaleType getSaleType() const
		{
			return _SaleType;
		}

		void setSaleType(TExplicitSaleType value)
		{

				_SaleType = value;

		}
			//
		const NLMISC::CSheetId &getSheetId() const
		{
			return _SheetId;
		}

		NLMISC::CSheetId &getSheetId()
		{
			return _SheetId;
		}


		void setSheetId(const NLMISC::CSheetId &value)
		{


				_SheetId = value;


		}
			// Name of item for named item
		const std::string &getNamed() const
		{
			return _Named;
		}

		std::string &getNamed()
		{
			return _Named;
		}


		void setNamed(const std::string &value)
		{


				_Named = value;


		}
			//
		uint32 getQuantity() const
		{
			return _Quantity;
		}

		void setQuantity(uint32 value)
		{

				_Quantity = value;

		}
			//
		uint32 getQuality() const
		{
			return _Quality;
		}

		void setQuality(uint32 value)
		{

				_Quality = value;

		}
			// The price of the item
		const TPriceInfo &getPrice() const
		{
			return _Price;
		}

		TPriceInfo &getPrice()
		{
			return _Price;
		}


		void setPrice(const TPriceInfo &value)
		{


				_Price = value;


		}

		bool operator == (const TExplicitSale &other) const
		{
			return _SaleType == other._SaleType
				&& _SheetId == other._SheetId
				&& _Named == other._Named
				&& _Quantity == other._Quantity
				&& _Quality == other._Quality
				&& _Price == other._Price;
		}


		// constructor
		TExplicitSale()
		{
			// Default initialisation
			_SheetId = NLMISC::CSheetId::Unknown;
			_Quantity = 1;
			_Quality = 0;

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_SaleType);
			s.serial(_SheetId);
			s.serial(_Named);
			s.serial(_Quantity);
			s.serial(_Quality);
			s.serial(_Price);

		}


	private:


	};


		//
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TContextOption
	{
	protected:
		//
		std::string	_Title;
		//
		std::string	_Detail;
	public:
		//
		const std::string &getTitle() const
		{
			return _Title;
		}

		std::string &getTitle()
		{
			return _Title;
		}


		void setTitle(const std::string &value)
		{


				_Title = value;


		}
			//
		const std::string &getDetail() const
		{
			return _Detail;
		}

		std::string &getDetail()
		{
			return _Detail;
		}


		void setDetail(const std::string &value)
		{


				_Detail = value;


		}

		bool operator == (const TContextOption &other) const
		{
			return _Title == other._Title
				&& _Detail == other._Detail;
		}


		// constructor
		TContextOption()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_Title);
			s.serial(_Detail);

		}


	private:


	};


		// Description of an NPC bot sent by AIS to EGS at spawn time
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TGenNpcDescMsg
	{
	protected:
		//
		uint32	_Alias;
		//
		uint32	_GrpAlias;
		//
		std::vector<uint32>	_MissionIds;
		//
		std::vector< uint32 >	_ShopCategories;
		//
		std::vector< TExplicitSale >	_ExplicitSales;
		//
		bool	_FilterExplicitActionTradeByPlayerRace;
		//
		EGSPD::CSPType::TSPType	_ExplicitActionSPType;
		//
		bool	_FilterExplicitActionTradeByBotRace;
		//
		std::string	_WelcomePhrase;
		//
		std::string	_WebPage;
		//
		std::string	_WebPageName;
		//
		NLMISC::CSheetId	_Sheet;
		//
		TDataSetRow	_EntityIndex;
		//
		bool	_PlayerAttackable;
		//
		bool	_BotAttackable;
		//
		NLMISC::CSheetId	_RightHandItem;
		//
		NLMISC::CSheetId	_LeftHandItem;
		//
		uint8	_RightHandItemQuality;
		//
		uint8	_LeftHandItemQuality;
		// True if this bot can give dynamic mission
		bool	_DynamicMissionGiver;
		//
		bool	_DontFollow;
		// if that bot is a building bot it will be caught by outpost system
		bool	_BuildingBot;
		// optional and rare properties
		std::vector< std::string >	_OptionalProperties;
		// List of news channel.
		std::vector< std::string >	_NewsChannels;
		// List of news channel.
		std::vector< TContextOption >	_ContextOptions;
		// list of loot
		std::vector< NLMISC::CSheetId >	_LootList;
		//
		NLMISC::CSheetId	_Outpost;
		uint32				_Organization;
		//
		float	_MaxHitRangeForPC;
		//
		std::string	_UserModelId;
		//
		std::string	_CustomLootTableId;
		//
		uint32	_PrimAlias;
	public:
		//
		uint32 getAlias() const
		{
			return _Alias;
		}

		void setAlias(uint32 value)
		{

				_Alias = value;

		}
			//
		uint32 getGrpAlias() const
		{
			return _GrpAlias;
		}

		void setGrpAlias(uint32 value)
		{

				_GrpAlias = value;

		}
			//
		const std::vector<uint32> &getMissionIds() const
		{
			return _MissionIds;
		}

		std::vector<uint32> &getMissionIds()
		{
			return _MissionIds;
		}


		void setMissionIds(const std::vector<uint32> &value)
		{


				_MissionIds = value;


		}
			//
		const std::vector< uint32 > &getShopCategories() const
		{
			return _ShopCategories;
		}

		std::vector< uint32 > &getShopCategories()
		{
			return _ShopCategories;
		}


		void setShopCategories(const std::vector< uint32 > &value)
		{


				_ShopCategories = value;


		}
			//
		const std::vector< TExplicitSale > &getExplicitSales() const
		{
			return _ExplicitSales;
		}

		std::vector< TExplicitSale > &getExplicitSales()
		{
			return _ExplicitSales;
		}


		void setExplicitSales(const std::vector< TExplicitSale > &value)
		{


				_ExplicitSales = value;


		}
			//
		bool getFilterExplicitActionTradeByPlayerRace() const
		{
			return _FilterExplicitActionTradeByPlayerRace;
		}

		void setFilterExplicitActionTradeByPlayerRace(bool value)
		{

				_FilterExplicitActionTradeByPlayerRace = value;

		}
			//
		EGSPD::CSPType::TSPType getExplicitActionSPType() const
		{
			return _ExplicitActionSPType;
		}

		void setExplicitActionSPType(EGSPD::CSPType::TSPType value)
		{

				_ExplicitActionSPType = value;

		}
			//
		bool getFilterExplicitActionTradeByBotRace() const
		{
			return _FilterExplicitActionTradeByBotRace;
		}

		void setFilterExplicitActionTradeByBotRace(bool value)
		{

				_FilterExplicitActionTradeByBotRace = value;

		}
			//
		const std::string &getWelcomePhrase() const
		{
			return _WelcomePhrase;
		}

		std::string &getWelcomePhrase()
		{
			return _WelcomePhrase;
		}


		void setWelcomePhrase(const std::string &value)
		{


				_WelcomePhrase = value;


		}
			//
		const std::string &getWebPage() const
		{
			return _WebPage;
		}

		std::string &getWebPage()
		{
			return _WebPage;
		}


		void setWebPage(const std::string &value)
		{


				_WebPage = value;


		}
			//
		const std::string &getWebPageName() const
		{
			return _WebPageName;
		}

		std::string &getWebPageName()
		{
			return _WebPageName;
		}


		void setWebPageName(const std::string &value)
		{


				_WebPageName = value;


		}
			//
		const NLMISC::CSheetId &getSheet() const
		{
			return _Sheet;
		}

		NLMISC::CSheetId &getSheet()
		{
			return _Sheet;
		}


		void setSheet(const NLMISC::CSheetId &value)
		{


				_Sheet = value;


		}
			//
		const TDataSetRow &getEntityIndex() const
		{
			return _EntityIndex;
		}

		TDataSetRow &getEntityIndex()
		{
			return _EntityIndex;
		}


		void setEntityIndex(const TDataSetRow &value)
		{


				_EntityIndex = value;


		}
			//
		bool getPlayerAttackable() const
		{
			return _PlayerAttackable;
		}

		void setPlayerAttackable(bool value)
		{

				_PlayerAttackable = value;

		}
			//
		bool getBotAttackable() const
		{
			return _BotAttackable;
		}

		void setBotAttackable(bool value)
		{

				_BotAttackable = value;

		}
			//
		const NLMISC::CSheetId &getRightHandItem() const
		{
			return _RightHandItem;
		}

		NLMISC::CSheetId &getRightHandItem()
		{
			return _RightHandItem;
		}


		void setRightHandItem(const NLMISC::CSheetId &value)
		{


				_RightHandItem = value;


		}
			//
		const NLMISC::CSheetId &getLeftHandItem() const
		{
			return _LeftHandItem;
		}

		NLMISC::CSheetId &getLeftHandItem()
		{
			return _LeftHandItem;
		}


		void setLeftHandItem(const NLMISC::CSheetId &value)
		{


				_LeftHandItem = value;


		}
			//
		uint8 getRightHandItemQuality() const
		{
			return _RightHandItemQuality;
		}

		void setRightHandItemQuality(uint8 value)
		{

				_RightHandItemQuality = value;

		}
			//
		uint8 getLeftHandItemQuality() const
		{
			return _LeftHandItemQuality;
		}

		void setLeftHandItemQuality(uint8 value)
		{

				_LeftHandItemQuality = value;

		}
			// True if this bot can give dynamic mission
		bool getDynamicMissionGiver() const
		{
			return _DynamicMissionGiver;
		}

		void setDynamicMissionGiver(bool value)
		{

				_DynamicMissionGiver = value;

		}
			//
		bool getDontFollow() const
		{
			return _DontFollow;
		}

		void setDontFollow(bool value)
		{

				_DontFollow = value;

		}
			// if that bot is a building bot it will be caught by outpost system
		bool getBuildingBot() const
		{
			return _BuildingBot;
		}

		void setBuildingBot(bool value)
		{

				_BuildingBot = value;

		}
			// optional and rare properties
		const std::vector< std::string > &getOptionalProperties() const
		{
			return _OptionalProperties;
		}

		std::vector< std::string > &getOptionalProperties()
		{
			return _OptionalProperties;
		}


		void setOptionalProperties(const std::vector< std::string > &value)
		{


				_OptionalProperties = value;


		}
			// List of news channel.
		const std::vector< std::string > &getNewsChannels() const
		{
			return _NewsChannels;
		}

		std::vector< std::string > &getNewsChannels()
		{
			return _NewsChannels;
		}


		void setNewsChannels(const std::vector< std::string > &value)
		{


				_NewsChannels = value;


		}
			// List of news channel.
		const std::vector< TContextOption > &getContextOptions() const
		{
			return _ContextOptions;
		}

		std::vector< TContextOption > &getContextOptions()
		{
			return _ContextOptions;
		}


		void setContextOptions(const std::vector< TContextOption > &value)
		{


				_ContextOptions = value;


		}
			// list of loot
		const std::vector< NLMISC::CSheetId > &getLootList() const
		{
			return _LootList;
		}

		std::vector< NLMISC::CSheetId > &getLootList()
		{
			return _LootList;
		}


		void setLootList(const std::vector< NLMISC::CSheetId > &value)
		{


				_LootList = value;


		}
			//
		const NLMISC::CSheetId &getOutpost() const
		{
			return _Outpost;
		}

		NLMISC::CSheetId &getOutpost()
		{
			return _Outpost;
		}


		void setOutpost(const NLMISC::CSheetId &value)
		{


				_Outpost = value;


		}

		//
		uint32 getOrganization() const 
		{
			return _Organization;
		}

		void setOrganization(uint32 value)
		{
			
			
				_Organization = value;
			
	
		}
			//
		float getMaxHitRangeForPC() const
		{
			return _MaxHitRangeForPC;
		}

		void setMaxHitRangeForPC(float value)
		{

				_MaxHitRangeForPC = value;

		}
			//
		const std::string &getUserModelId() const
		{
			return _UserModelId;
		}

		std::string &getUserModelId()
		{
			return _UserModelId;
		}


		void setUserModelId(const std::string &value)
		{


				_UserModelId = value;


		}
			//
		const std::string &getCustomLootTableId() const
		{
			return _CustomLootTableId;
		}

		std::string &getCustomLootTableId()
		{
			return _CustomLootTableId;
		}


		void setCustomLootTableId(const std::string &value)
		{


				_CustomLootTableId = value;


		}
			//
		uint32 getPrimAlias() const
		{
			return _PrimAlias;
		}

		void setPrimAlias(uint32 value)
		{

				_PrimAlias = value;

		}

		bool operator == (const TGenNpcDescMsg &other) const
		{
			return _Alias == other._Alias
				&& _GrpAlias == other._GrpAlias
				&& _MissionIds == other._MissionIds
				&& _ShopCategories == other._ShopCategories
				&& _ExplicitSales == other._ExplicitSales
				&& _FilterExplicitActionTradeByPlayerRace == other._FilterExplicitActionTradeByPlayerRace
				&& _ExplicitActionSPType == other._ExplicitActionSPType
				&& _FilterExplicitActionTradeByBotRace == other._FilterExplicitActionTradeByBotRace
				&& _WelcomePhrase == other._WelcomePhrase
				&& _WebPage == other._WebPage
				&& _WebPageName == other._WebPageName
				&& _Sheet == other._Sheet
				&& _EntityIndex == other._EntityIndex
				&& _PlayerAttackable == other._PlayerAttackable
				&& _BotAttackable == other._BotAttackable
				&& _RightHandItem == other._RightHandItem
				&& _LeftHandItem == other._LeftHandItem
				&& _RightHandItemQuality == other._RightHandItemQuality
				&& _LeftHandItemQuality == other._LeftHandItemQuality
				&& _DynamicMissionGiver == other._DynamicMissionGiver
				&& _DontFollow == other._DontFollow
				&& _BuildingBot == other._BuildingBot
				&& _OptionalProperties == other._OptionalProperties
				&& _NewsChannels == other._NewsChannels
				&& _ContextOptions == other._ContextOptions
				&& _LootList == other._LootList
				&& _Outpost == other._Outpost
				&& _Organization == other._Organization
				&& _MaxHitRangeForPC == other._MaxHitRangeForPC
				&& _UserModelId == other._UserModelId
				&& _CustomLootTableId == other._CustomLootTableId
				&& _PrimAlias == other._PrimAlias;
		}


		// constructor
		TGenNpcDescMsg()
		{
			// Default initialisation
			_MaxHitRangeForPC = -1.f;

		}

		void send(const std::string &serviceName)
		{
			NLNET::CMessage msg("TGenNpcDescMsg");
			serial(msg);
			sendMessageViaMirror( serviceName, msg );
		}

		void send(NLNET::TServiceId serviceId)
		{
			NLNET::CMessage msg("TGenNpcDescMsg");
			serial(msg);
			sendMessageViaMirror( serviceId, msg );
		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_Alias);
			s.serial(_GrpAlias);
			s.serialCont(_MissionIds);
			s.serialCont(_ShopCategories);
			s.serialCont(_ExplicitSales);
			s.serial(_FilterExplicitActionTradeByPlayerRace);
			s.serialEnum(_ExplicitActionSPType);
			s.serial(_FilterExplicitActionTradeByBotRace);
			s.serial(_WelcomePhrase);
			s.serial(_WebPage);
			s.serial(_WebPageName);
			s.serial(_Sheet);
			s.serial(_EntityIndex);
			s.serial(_PlayerAttackable);
			s.serial(_BotAttackable);
			s.serial(_RightHandItem);
			s.serial(_LeftHandItem);
			s.serial(_RightHandItemQuality);
			s.serial(_LeftHandItemQuality);
			s.serial(_DynamicMissionGiver);
			s.serial(_DontFollow);
			s.serial(_BuildingBot);
			s.serialCont(_OptionalProperties);
			s.serialCont(_NewsChannels);
			s.serialCont(_ContextOptions);
			s.serialCont(_LootList);
			s.serial(_Outpost);
			s.serial(_Organization);
			s.serial(_MaxHitRangeForPC);
			s.serial(_UserModelId);
			s.serial(_CustomLootTableId);
			s.serial(_PrimAlias);

		}


	private:


	};



}

#endif
