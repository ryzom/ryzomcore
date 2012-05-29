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
#include "game_share/utils.h"
#include "log_item_gen.h"


#include "logger_service_itf.h"
#include "logger_service_client.h"

// A function fo force linking of this code module
void forceLink_Item(){}




class CItemDesc
{
	friend class CLoggerClient;

	/// The list of log definition for this log class
	std::vector<LGS::TLogDefinition>	_LogDefs;

	/// Stack of context variable
	
	std::vector<NLMISC::CEntityId>	_charId;
	
	std::vector<NLMISC::CEntityId>	_validatorCharId;
	
	std::vector<NLMISC::CEntityId>	_otherEntityId;
	

	/// Counter of 'no context' object stacked.
	uint32	_NoContextCount;

public:
	/// constructor
	CItemDesc()
		:	_NoContextCount(0)
	{
		_LogDefs.resize(34);
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[0];
			
			logDef.setLogName("Item_BuyItem");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[1];
			
			logDef.setLogName("Item_CreateGuild");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[2];
			
			logDef.setLogName("Item_BuyGuildOption");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[3];
			
			logDef.setLogName("Item_Forage");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[4];
			
			logDef.setLogName("Item_QuarterOrLoot");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[5];
			
			logDef.setLogName("Item_OtherTempPickup");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[6];
			
			logDef.setLogName("Item_AutoMissionLoot");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[7];
			
			logDef.setLogName("Item_Craft");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[8];
			
			logDef.setLogName("Item_Mission");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[9];
			
			logDef.setLogName("Item_Sell");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[10];
			
			logDef.setLogName("Item_Destroy");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[11];
			
			logDef.setLogName("Item_DropTempInventory");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[12];
			
			logDef.setLogName("Item_DestroySaleStore");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[13];
			
			logDef.setLogName("Item_SaleStoreTimeout");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[14];
			
			logDef.setLogName("Item_ConsumeAmmo");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[15];
			
			logDef.setLogName("Item_ConsumeFaberMp");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[16];
			
			logDef.setLogName("Item_Consume");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[17];
			
			logDef.setLogName("Item_Exchange");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[18];
			
			logDef.setLogName("Item_BadSheet");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[19];
			
			logDef.setLogName("Item_Swap");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[20];
			
			logDef.setLogName("Item_Command");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[21];
			
			logDef.setLogName("Item_PetDespawn");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[22];
			
			logDef.setLogName("Item_NoRent");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[23];
			
			logDef.setLogName("Item_OutpostDriller");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[24];
			
			

			logDef.setLogName("Item_Create");
			logDef.setLogText("Character has a new item in an inventory");

			logDef.getParams().resize(5);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("itemId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_itemId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("sheetId");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_sheetId);
			logDef.getParams()[2].setList(false);
				
			logDef.getParams()[3].setName("quantity");
			logDef.getParams()[3].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[3].setList(false);
				
			logDef.getParams()[4].setName("quality");
			logDef.getParams()[4].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[4].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[25];
			
			

			logDef.setLogName("Item_UpdateQuantity");
			logDef.setLogText("The stack size has changed (ie the quantity)");

			logDef.getParams().resize(4);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("itemId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_itemId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("quantity");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[2].setList(false);
				
			logDef.getParams()[3].setName("oldQuantity");
			logDef.getParams()[3].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[3].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[26];
			
			

			logDef.setLogName("Item_Move");
			logDef.setLogText("The item is moved from one inventory to another");

			logDef.getParams().resize(4);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("itemId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_itemId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("srcInventoryId");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
			logDef.getParams()[3].setName("dstInventoryId");
			logDef.getParams()[3].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[3].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[27];
			
			

			logDef.setLogName("Item_PutInSaleStore");
			logDef.setLogText("The item have been put in sale store");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("itemId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_itemId);
			logDef.getParams()[1].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[28];
			
			

			logDef.setLogName("Item_RemoveFromSaleStore");
			logDef.setLogText("The item have retired from the sale store");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("itemId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_itemId);
			logDef.getParams()[1].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[29];
			
			

			logDef.setLogName("Item_Delete");
			logDef.setLogText("The item is destroyed or consumed");

			logDef.getParams().resize(5);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("itemId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_itemId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("sheetId");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_sheetId);
			logDef.getParams()[2].setList(false);
				
			logDef.getParams()[3].setName("quantity");
			logDef.getParams()[3].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[3].setList(false);
				
			logDef.getParams()[4].setName("quality");
			logDef.getParams()[4].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[4].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[30];
			
			

			logDef.setLogName("Item_FailedAddBoughtItem");
			logDef.setLogText("Cannot put the bought item in the character inventory");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("itemId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_itemId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("dstInventoryId");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[31];
			
			

			logDef.setLogName("Item_Money");
			logDef.setLogText("The money amount change");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("moneyBefore");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_uint64);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("moneyAfter");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_uint64);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[32];
			
			

			logDef.setLogName("Item_ExchangeWithChar");
			logDef.setLogText("A character exchange with another character");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("validatorCharId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("otherEntityId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_entityId);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[33];
			
			

			logDef.setLogName("Item_ExchangeWithNPC");
			logDef.setLogText("A character exchange with a NPC");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("validatorCharId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("otherEntityId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_entityId);
				
		}
		

		// Register the log definitions
		LGS::ILoggerServiceClient::addLogDefinitions(_LogDefs);
	}

	// Context var stack accessor
	
	bool getContextVar_charId (NLMISC::CEntityId &value)
	{
		if (_charId.empty())
			return false;

		value = _charId.back();
		return true;
	}

	void pushContextVar_charId (const NLMISC::CEntityId &value)
	{
		_charId.push_back(value);
	}
	void popContextVar_charId ()
	{
		_charId.pop_back();
	}
	
	bool getContextVar_validatorCharId (NLMISC::CEntityId &value)
	{
		if (_validatorCharId.empty())
			return false;

		value = _validatorCharId.back();
		return true;
	}

	void pushContextVar_validatorCharId (const NLMISC::CEntityId &value)
	{
		_validatorCharId.push_back(value);
	}
	void popContextVar_validatorCharId ()
	{
		_validatorCharId.pop_back();
	}
	
	bool getContextVar_otherEntityId (NLMISC::CEntityId &value)
	{
		if (_otherEntityId.empty())
			return false;

		value = _otherEntityId.back();
		return true;
	}

	void pushContextVar_otherEntityId (const NLMISC::CEntityId &value)
	{
		_otherEntityId.push_back(value);
	}
	void popContextVar_otherEntityId ()
	{
		_otherEntityId.pop_back();
	}
	

	void pushNoContext()
	{
		++_NoContextCount;
	}
	void popNoContext()
	{
		nlassert(_NoContextCount > 0);
		--_NoContextCount;
	}

	uint32 getNoContextCount()
	{
		return _NoContextCount;
	}

};
// Instantiate the descriptor class
CItemDesc	ItemDesc;



const std::string TLogContext_Item_BuyItem::_ContextName("Item_BuyItem");
/// The constructor push a log context in the logger system
TLogContext_Item_BuyItem::TLogContext_Item_BuyItem(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_BuyItem::~TLogContext_Item_BuyItem()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_CreateGuild::_ContextName("Item_CreateGuild");
/// The constructor push a log context in the logger system
TLogContext_Item_CreateGuild::TLogContext_Item_CreateGuild(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_CreateGuild::~TLogContext_Item_CreateGuild()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_BuyGuildOption::_ContextName("Item_BuyGuildOption");
/// The constructor push a log context in the logger system
TLogContext_Item_BuyGuildOption::TLogContext_Item_BuyGuildOption(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_BuyGuildOption::~TLogContext_Item_BuyGuildOption()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Forage::_ContextName("Item_Forage");
/// The constructor push a log context in the logger system
TLogContext_Item_Forage::TLogContext_Item_Forage(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Forage::~TLogContext_Item_Forage()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_QuarterOrLoot::_ContextName("Item_QuarterOrLoot");
/// The constructor push a log context in the logger system
TLogContext_Item_QuarterOrLoot::TLogContext_Item_QuarterOrLoot(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_QuarterOrLoot::~TLogContext_Item_QuarterOrLoot()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_OtherTempPickup::_ContextName("Item_OtherTempPickup");
/// The constructor push a log context in the logger system
TLogContext_Item_OtherTempPickup::TLogContext_Item_OtherTempPickup(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_OtherTempPickup::~TLogContext_Item_OtherTempPickup()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_AutoMissionLoot::_ContextName("Item_AutoMissionLoot");
/// The constructor push a log context in the logger system
TLogContext_Item_AutoMissionLoot::TLogContext_Item_AutoMissionLoot(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_AutoMissionLoot::~TLogContext_Item_AutoMissionLoot()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Craft::_ContextName("Item_Craft");
/// The constructor push a log context in the logger system
TLogContext_Item_Craft::TLogContext_Item_Craft(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Craft::~TLogContext_Item_Craft()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Mission::_ContextName("Item_Mission");
/// The constructor push a log context in the logger system
TLogContext_Item_Mission::TLogContext_Item_Mission(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Mission::~TLogContext_Item_Mission()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Sell::_ContextName("Item_Sell");
/// The constructor push a log context in the logger system
TLogContext_Item_Sell::TLogContext_Item_Sell(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Sell::~TLogContext_Item_Sell()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Destroy::_ContextName("Item_Destroy");
/// The constructor push a log context in the logger system
TLogContext_Item_Destroy::TLogContext_Item_Destroy(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Destroy::~TLogContext_Item_Destroy()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_DropTempInventory::_ContextName("Item_DropTempInventory");
/// The constructor push a log context in the logger system
TLogContext_Item_DropTempInventory::TLogContext_Item_DropTempInventory(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_DropTempInventory::~TLogContext_Item_DropTempInventory()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_DestroySaleStore::_ContextName("Item_DestroySaleStore");
/// The constructor push a log context in the logger system
TLogContext_Item_DestroySaleStore::TLogContext_Item_DestroySaleStore(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_DestroySaleStore::~TLogContext_Item_DestroySaleStore()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_SaleStoreTimeout::_ContextName("Item_SaleStoreTimeout");
/// The constructor push a log context in the logger system
TLogContext_Item_SaleStoreTimeout::TLogContext_Item_SaleStoreTimeout(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_SaleStoreTimeout::~TLogContext_Item_SaleStoreTimeout()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_ConsumeAmmo::_ContextName("Item_ConsumeAmmo");
/// The constructor push a log context in the logger system
TLogContext_Item_ConsumeAmmo::TLogContext_Item_ConsumeAmmo(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_ConsumeAmmo::~TLogContext_Item_ConsumeAmmo()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_ConsumeFaberMp::_ContextName("Item_ConsumeFaberMp");
/// The constructor push a log context in the logger system
TLogContext_Item_ConsumeFaberMp::TLogContext_Item_ConsumeFaberMp(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_ConsumeFaberMp::~TLogContext_Item_ConsumeFaberMp()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Consume::_ContextName("Item_Consume");
/// The constructor push a log context in the logger system
TLogContext_Item_Consume::TLogContext_Item_Consume(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Consume::~TLogContext_Item_Consume()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Exchange::_ContextName("Item_Exchange");
/// The constructor push a log context in the logger system
TLogContext_Item_Exchange::TLogContext_Item_Exchange(const NLMISC::CEntityId &validatorCharId, const NLMISC::CEntityId &otherEntityId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_validatorCharId(validatorCharId);
	ItemDesc.pushContextVar_otherEntityId(otherEntityId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Exchange::~TLogContext_Item_Exchange()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_validatorCharId();
	ItemDesc.popContextVar_otherEntityId();
	
}

const std::string TLogContext_Item_BadSheet::_ContextName("Item_BadSheet");
/// The constructor push a log context in the logger system
TLogContext_Item_BadSheet::TLogContext_Item_BadSheet(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_BadSheet::~TLogContext_Item_BadSheet()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Swap::_ContextName("Item_Swap");
/// The constructor push a log context in the logger system
TLogContext_Item_Swap::TLogContext_Item_Swap(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Swap::~TLogContext_Item_Swap()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_Command::_ContextName("Item_Command");
/// The constructor push a log context in the logger system
TLogContext_Item_Command::TLogContext_Item_Command(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_Command::~TLogContext_Item_Command()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_PetDespawn::_ContextName("Item_PetDespawn");
/// The constructor push a log context in the logger system
TLogContext_Item_PetDespawn::TLogContext_Item_PetDespawn(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_PetDespawn::~TLogContext_Item_PetDespawn()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_NoRent::_ContextName("Item_NoRent");
/// The constructor push a log context in the logger system
TLogContext_Item_NoRent::TLogContext_Item_NoRent(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_NoRent::~TLogContext_Item_NoRent()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}

const std::string TLogContext_Item_OutpostDriller::_ContextName("Item_OutpostDriller");
/// The constructor push a log context in the logger system
TLogContext_Item_OutpostDriller::TLogContext_Item_OutpostDriller(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	ItemDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Item_OutpostDriller::~TLogContext_Item_OutpostDriller()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	ItemDesc.popContextVar_charId();
	
}


/// No context context. Use this to disable any contextual log underneath
TLogNoContext_Item::TLogNoContext_Item()
{
	ItemDesc.pushNoContext();
}

TLogNoContext_Item::~TLogNoContext_Item()
{
	ItemDesc.popNoContext();
}



void _log_Item_Create(INVENTORIES::TItemId itemId, const NLMISC::CSheetId &sheetId, uint32 quantity, uint32 quality, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_Create");
		logInfo.getParams().resize(5);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!ItemDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(itemId);
		
	logInfo.getParams()[2] = LGS::TParamValue(sheetId);
		
	logInfo.getParams()[3] = LGS::TParamValue(quantity);
		
	logInfo.getParams()[4] = LGS::TParamValue(quality);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_UpdateQuantity(INVENTORIES::TItemId itemId, uint32 quantity, uint32 oldQuantity, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_UpdateQuantity");
		logInfo.getParams().resize(4);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!ItemDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(itemId);
		
	logInfo.getParams()[2] = LGS::TParamValue(quantity);
		
	logInfo.getParams()[3] = LGS::TParamValue(oldQuantity);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_Move(INVENTORIES::TItemId itemId, INVENTORIES::TInventory srcInventoryId, INVENTORIES::TInventory dstInventoryId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_Move");
		logInfo.getParams().resize(4);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!ItemDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(itemId);
		
	logInfo.getParams()[2] = LGS::TParamValue(INVENTORIES::toString(srcInventoryId));
		
	logInfo.getParams()[3] = LGS::TParamValue(INVENTORIES::toString(dstInventoryId));
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_PutInSaleStore(INVENTORIES::TItemId itemId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_PutInSaleStore");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!ItemDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(itemId);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_RemoveFromSaleStore(INVENTORIES::TItemId itemId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_RemoveFromSaleStore");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!ItemDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(itemId);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_Delete(INVENTORIES::TItemId itemId, const NLMISC::CSheetId &sheetId, uint32 quantity, uint32 quality, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_Delete");
		logInfo.getParams().resize(5);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!ItemDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(itemId);
		
	logInfo.getParams()[2] = LGS::TParamValue(sheetId);
		
	logInfo.getParams()[3] = LGS::TParamValue(quantity);
		
	logInfo.getParams()[4] = LGS::TParamValue(quality);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_FailedAddBoughtItem(INVENTORIES::TItemId itemId, INVENTORIES::TInventory dstInventoryId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_FailedAddBoughtItem");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!ItemDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(itemId);
		
	logInfo.getParams()[2] = LGS::TParamValue(INVENTORIES::toString(dstInventoryId));
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_Money(uint64 moneyBefore, uint64 moneyAfter, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_Money");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!ItemDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(moneyBefore);
		
	logInfo.getParams()[2] = LGS::TParamValue(moneyAfter);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_ExchangeWithChar(const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_ExchangeWithChar");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	validatorCharId;
	if (!ItemDesc.getContextVar_validatorCharId(validatorCharId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(validatorCharId);
			
	// Context parameter
		NLMISC::CEntityId	otherEntityId;
	if (!ItemDesc.getContextVar_otherEntityId(otherEntityId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[1] = LGS::TParamValue(otherEntityId);
			

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Item_ExchangeWithNPC(const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Item_ExchangeWithNPC");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	validatorCharId;
	if (!ItemDesc.getContextVar_validatorCharId(validatorCharId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(validatorCharId);
			
	// Context parameter
		NLMISC::CEntityId	otherEntityId;
	if (!ItemDesc.getContextVar_otherEntityId(otherEntityId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(ItemDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Item'");
		return;
	}

			
	logInfo.getParams()[1] = LGS::TParamValue(otherEntityId);
			

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}
