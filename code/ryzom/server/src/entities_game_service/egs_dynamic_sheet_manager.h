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

#ifndef DYNAMIC_SHEET_MANAGER_H
#define DYNAMIC_SHEET_MANAGER_H

#include "egs_sheets/egs_static_game_sheet.h"
#include "server_share/npc_description_messages.h"

struct CUserModels
{
	NLNET::TServiceId					ServiceId;
	NLMISC::CSmartPtr<CStaticCreatures>	CustomSheet;
};

struct CCustomLootTables
{
	NLNET::TServiceId	ServiceId;
	CStaticLootTable	Table;
};


/** callback called when receiving USR_MDL msg sent by AIS */
void cbGetUserModels( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

/** callback called when receiving CUSTOMLT msg sent by AIS */
void cbGetCustomLootTables(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

void cbDeleteCustomDataByPrimAlias(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);

class CDynamicSheetManager
{
public:
	static CDynamicSheetManager *getInstance();
	static void release();

	void init();

	CDynamicSheetManager() { }
	~CDynamicSheetManager() { }

	/** when adding a user model, first check if it's already present in the manager's structures */
	bool isAlreadyStored(CCustomElementId id);

	/** Add user models into manager's structures upon reception of the AIS msg
	* @param msgin the AIS CMessage containing custom loot table info
	* @param serviceId id of the AIS that sent the msg
	*/
	void getUserModelsFromMsg(NLNET::CMessage &f, NLNET::TServiceId serviceId);

	/** Add custom loot tables into manager's structures upon reception of the AIS msg
	* @param msgin the AIS CMessage containing custom loot table info
	* @param serviceId id of the AIS that sent the msg
	*/
	void getCustomLootTablesFromMsg(NLNET::CMessage &msgin, NLNET::TServiceId serviceId);

	/** Gets a copy of the base sheet, applies script-defined modifications and stores the modified sheet
	* @param modelId id of the user model
	* @param scriptData all script lines
	* @param serviceId id of the AIS that sent the datas
	*/
	void instanciateDynamicSheet(CCustomElementId modelId, std::vector<std::string> scriptData, NLNET::TServiceId serviceId);

	/** Returns the modified CStaticCreature associated to a UserModel
	* @param userModelId the id of the user model
	*/
	CStaticCreatures *getDynamicSheet(uint32 primAlias, const std::string &userModelId);

	/** Indicates whether there have been parse errors while building a UserModel or not.
	* @param userModelId id of the user model
	* @return true if there are some errors in the user model
	*/
	bool scriptErrors(uint32 primAlias, const std::string &userModelId);

	/** deletes all data that were sent by a given AIS. Called when a AIS is down.
	* @param serviceId the AIS id
	*/
	void releaseCustomDataByServiceId(NLNET::TServiceId serviceId);

	void deleteCustomDataByPrimAlias(uint32 primAlias);

	void addCustomLootTable(CCustomElementId id, CCustomLootTable lootTable, NLNET::TServiceId serviceId);

	CStaticLootTable *getLootTable(uint32 primAlias, const std::string &tableId);

	typedef std::map<CCustomElementId, CUserModels> TModifiedCreaturesMap;
	typedef std::map<CCustomElementId, bool> TScriptErrors;
	typedef std::map<CCustomElementId, CCustomLootTables> TCustomTables;

private:

	static CDynamicSheetManager		*_Instance;

	/** contains complete custom loot tables. Key: CustomLootTableId, value: CStaticLootTable
	* @see CStaticLootTable
	*/
	TCustomTables					_CustomLootTables;

	/** contains all user models. key: user model id, value: struct containing the AIservice id that sent the model,
	* and a smart pointer towards the modified CStaticCreature
	* @see CUserModels
	*/
	TModifiedCreaturesMap			_CreaturesMap;

	/** just a map for script error identification key: user model id, value: if there were any errors or not */
	TScriptErrors					_UserModelLoadingErrors;
};

#endif
