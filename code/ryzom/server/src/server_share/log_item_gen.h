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



#ifndef _LOG_GEN_ITEM_H
#define _LOG_GEN_ITEM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"


struct TLogContext_Item_BuyItem
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_BuyItem(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_BuyItem();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_CreateGuild
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_CreateGuild(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_CreateGuild();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_BuyGuildOption
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_BuyGuildOption(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_BuyGuildOption();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Forage
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Forage(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Forage();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_QuarterOrLoot
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_QuarterOrLoot(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_QuarterOrLoot();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_OtherTempPickup
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_OtherTempPickup(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_OtherTempPickup();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_AutoMissionLoot
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_AutoMissionLoot(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_AutoMissionLoot();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Craft
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Craft(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Craft();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Mission
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Mission(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Mission();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Sell
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Sell(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Sell();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Destroy
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Destroy(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Destroy();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_DropTempInventory
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_DropTempInventory(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_DropTempInventory();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_DestroySaleStore
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_DestroySaleStore(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_DestroySaleStore();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_SaleStoreTimeout
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_SaleStoreTimeout(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_SaleStoreTimeout();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_ConsumeAmmo
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_ConsumeAmmo(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_ConsumeAmmo();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_ConsumeFaberMp
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_ConsumeFaberMp(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_ConsumeFaberMp();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Consume
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Consume(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Consume();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Exchange
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Exchange(const NLMISC::CEntityId &validatorCharId, const NLMISC::CEntityId &otherEntityId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Exchange();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_BadSheet
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_BadSheet(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_BadSheet();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Swap
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Swap(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Swap();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_Command
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_Command(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_Command();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_PetDespawn
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_PetDespawn(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_PetDespawn();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_NoRent
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_NoRent(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_NoRent();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Item_OutpostDriller
{
	/// The constructor push a log context in the logger system
	TLogContext_Item_OutpostDriller(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Item_OutpostDriller();

private:
	/// The name of the context
	static const std::string _ContextName;


};


/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Item
{
	TLogNoContext_Item();
	~TLogNoContext_Item();
};



void _log_Item_Create(INVENTORIES::TItemId itemId, const NLMISC::CSheetId &sheetId, uint32 quantity, uint32 quality, const char *_filename_, uint _lineNo_);
#define log_Item_Create(itemId, sheetId, quantity, quality) \
	_log_Item_Create(itemId, sheetId, quantity, quality, __FILE__, __LINE__)

void _log_Item_UpdateQuantity(INVENTORIES::TItemId itemId, uint32 quantity, uint32 oldQuantity, const char *_filename_, uint _lineNo_);
#define log_Item_UpdateQuantity(itemId, quantity, oldQuantity) \
	_log_Item_UpdateQuantity(itemId, quantity, oldQuantity, __FILE__, __LINE__)

void _log_Item_Move(INVENTORIES::TItemId itemId, INVENTORIES::TInventory srcInventoryId, INVENTORIES::TInventory dstInventoryId, const char *_filename_, uint _lineNo_);
#define log_Item_Move(itemId, srcInventoryId, dstInventoryId) \
	_log_Item_Move(itemId, srcInventoryId, dstInventoryId, __FILE__, __LINE__)

void _log_Item_PutInSaleStore(INVENTORIES::TItemId itemId, const char *_filename_, uint _lineNo_);
#define log_Item_PutInSaleStore(itemId) \
	_log_Item_PutInSaleStore(itemId, __FILE__, __LINE__)

void _log_Item_RemoveFromSaleStore(INVENTORIES::TItemId itemId, const char *_filename_, uint _lineNo_);
#define log_Item_RemoveFromSaleStore(itemId) \
	_log_Item_RemoveFromSaleStore(itemId, __FILE__, __LINE__)

void _log_Item_Delete(INVENTORIES::TItemId itemId, const NLMISC::CSheetId &sheetId, uint32 quantity, uint32 quality, const char *_filename_, uint _lineNo_);
#define log_Item_Delete(itemId, sheetId, quantity, quality) \
	_log_Item_Delete(itemId, sheetId, quantity, quality, __FILE__, __LINE__)

void _log_Item_FailedAddBoughtItem(INVENTORIES::TItemId itemId, INVENTORIES::TInventory dstInventoryId, const char *_filename_, uint _lineNo_);
#define log_Item_FailedAddBoughtItem(itemId, dstInventoryId) \
	_log_Item_FailedAddBoughtItem(itemId, dstInventoryId, __FILE__, __LINE__)

void _log_Item_Money(uint64 moneyBefore, uint64 moneyAfter, const char *_filename_, uint _lineNo_);
#define log_Item_Money(moneyBefore, moneyAfter) \
	_log_Item_Money(moneyBefore, moneyAfter, __FILE__, __LINE__)

void _log_Item_ExchangeWithChar(const char *_filename_, uint _lineNo_);
#define log_Item_ExchangeWithChar() \
	_log_Item_ExchangeWithChar(__FILE__, __LINE__)

void _log_Item_ExchangeWithNPC(const char *_filename_, uint _lineNo_);
#define log_Item_ExchangeWithNPC() \
	_log_Item_ExchangeWithNPC(__FILE__, __LINE__)


#endif

