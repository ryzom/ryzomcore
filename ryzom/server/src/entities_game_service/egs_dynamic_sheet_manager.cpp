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
#include "egs_dynamic_sheet_manager.h"
#include "egs_sheets/egs_sheets.h"
#include "nel/misc/algo.h"

#include "nel/misc/command.h"
#include "nel/net/unified_network.h"
#include "nel/misc/variable.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

CDynamicSheetManager *CDynamicSheetManager::_Instance = 0;

CDynamicSheetManager *CDynamicSheetManager::getInstance()
{
	if (_Instance == 0)
	{
		_Instance = new CDynamicSheetManager();
		_Instance->init();
		nldebug("Initializing DynamicSheetManager");
	}
	return _Instance;
}

void CDynamicSheetManager::release()
{
	delete _Instance;
	_Instance = 0;
}

/**
* callbacks applied when receiving ai messages (user models & custom loot table)
*/
void cbGetUserModels( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	CDynamicSheetManager::getInstance()->getUserModelsFromMsg(msgin, serviceId);
}

void cbGetCustomLootTables(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	CDynamicSheetManager::getInstance()->getCustomLootTablesFromMsg(msgin, serviceId);
}

void cbDeleteCustomDataByPrimAlias(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	uint32 primAlias;
	msgin.serial(primAlias);
	CDynamicSheetManager::getInstance()->deleteCustomDataByPrimAlias(primAlias);
}

bool CDynamicSheetManager::isAlreadyStored(CCustomElementId id)
{
	TModifiedCreaturesMap::const_iterator it = _CreaturesMap.find(id);
	if (it != _CreaturesMap.end())
		return true;
	return false;
}

/**
* for each user model, instanciate the matching dynamic sheet
*/
void CDynamicSheetManager::getUserModelsFromMsg(NLNET::CMessage &msgin, NLNET::TServiceId serviceId)
{
	nldebug("Receiving UserModels from AIS");
	CScriptData scriptData;
	msgin.serial(scriptData);

	for (TScripts::iterator it = scriptData.Scripts.begin(); it != scriptData.Scripts.end(); ++it)
	{
		if (isAlreadyStored(it->first))
		{
			nlwarning("User model '%s' already defined, skipping it.", it->first.Id.c_str());
		}
		else
		{
			instanciateDynamicSheet(it->first, it->second, serviceId);
		}
	}
}

/**
* create the custom loot tables objects
*/
void CDynamicSheetManager::addCustomLootTable(CCustomElementId id, CCustomLootTable lootTable, NLNET::TServiceId serviceId)
{
	if (_CustomLootTables.find(id) != _CustomLootTables.end())
	{
		nlwarning("Custom loot table '%s' already present in manager", id.Id.c_str());
		return;
	}

	vector<string> tmpVector;

	CCustomLootTables table;

	TScripts::iterator it;
	for (it = lootTable.LootSets.Scripts.begin(); it != lootTable.LootSets.Scripts.end(); ++it)
	{
		CStaticLootSet tmplootSet;
		string probaStr = it->first.Id;
		uint16 proba;
		NLMISC::fromString(probaStr, proba);
		TScriptContent script = it->second;

		uint32 lineNb = 0;
		for (TScriptContent::iterator it2 = script.begin(); it2 != script.end(); ++it2)
		{
			// for each script line..
			tmpVector.clear();	
			splitString((*it2), " ", tmpVector);
			++lineNb;

			if (tmpVector.size() != 3)
			{
				nlwarning("CustomLootTable '%s' : Too many tokens or missing tokens in uncommented line %d, skipping it", id.Id.c_str(), lineNb);
				continue;
			}

			CStaticLootSet::SItemLoot item;
			item.Item = tmpVector[0];
			NLMISC::fromString(tmpVector[1], item.Level);
			NLMISC::fromString(tmpVector[2], item.Quantity);
			nldebug("New Item: name='%s' quality=%d quantity=%d", item.Item.c_str(), item.Level, item.Quantity);

			tmplootSet.ItemLoot.push_back(item);
		}
		table.Table.CustomLootSets.insert(make_pair(proba, tmplootSet));
	}

	table.Table.MoneyLvlFactor = lootTable.MoneyFactor;
	table.Table.MoneyBase = lootTable.MoneyBase;
	table.Table.MoneyDropProbability = lootTable.MoneyProba;
	
	table.ServiceId = serviceId;

	_CustomLootTables.insert(make_pair(id, table));
	nldebug("done with table: id='%s'", id.Id.c_str());

	return;
}

void CDynamicSheetManager::getCustomLootTablesFromMsg(NLNET::CMessage &msgin, NLNET::TServiceId serviceId)
{
	nldebug("Receiving CustomLootTables from AIS");

	CCustomLootTableManager receivedCustomLootTables;
	msgin.serial(receivedCustomLootTables);

	for (TCustomLootTable::const_iterator it = receivedCustomLootTables.Tables.begin(); it != receivedCustomLootTables.Tables.end(); ++it)
	{
		addCustomLootTable(it->first, it->second, serviceId);
	}
}

void CDynamicSheetManager::init()
{	
	// array of callback
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "USER_MODELS", cbGetUserModels },
		{ "CUSTOM_LOOT_TABLES", cbGetCustomLootTables },
		{ "DELCUSTOM", cbDeleteCustomDataByPrimAlias }
	};
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
}

/**
* create the dynamic sheet that will then be stored into the manager
*/
void CDynamicSheetManager::instanciateDynamicSheet(CCustomElementId			modelId, 
												   std::vector<std::string>	scriptData,
												   NLNET::TServiceId		serviceId)
{
	if (scriptData.size() == 0 || scriptData[0].empty())
	{
		nlwarning("Cannot instanciate empty model : '%s'.", modelId.Id.c_str());
		return;
	}

	if (_CreaturesMap.find(modelId) != _CreaturesMap.end())
	{
		nlwarning("Sheet '%s' already present in manager", modelId.Id.c_str());
		return;
	}

	nldebug("Instanciating sheet associated to model '%s' and alias '%u'", modelId.Id.c_str(), modelId.PrimAlias);
	//scriptData[0] is the base sheetId for userModel
	std::string sheetIdFilename = scriptData[0] + ".creature";

	//first create a temp sheet containing all normal attributes
	NLMISC::CSheetId sheetId = NLMISC::CSheetId(sheetIdFilename);
	const CStaticCreatures *form = CSheets::getCreaturesForm(sheetId);

	//copy the normal sheet (unmodified attributes will have correct values in comparison with the base sheet)
	CStaticCreatures *sheetCopy = new CStaticCreatures(*form);

	//and apply user model defined modifications on the copy
	bool errors = sheetCopy->applyUserModel(modelId, scriptData);

	CUserModels userModel;

	//XXXXX!!!
	//keep the id of the ais that sent the user models in order to free the resources if the services goes down
	userModel.ServiceId = serviceId;
	userModel.CustomSheet = CSmartPtr<CStaticCreatures>(sheetCopy);

	//store the smartptr into the manager
	_CreaturesMap.insert(make_pair(modelId, userModel));

	//if errors happened during the modifications, the npc that possesses a user model as a sheet will have its name
	//changed from "botName" to "<ERROR>botName"
	_UserModelLoadingErrors.insert(make_pair(modelId, errors));
}

CStaticCreatures *CDynamicSheetManager::getDynamicSheet(uint32 primAlias, const std::string &userModelId)
{
	CCustomElementId id(primAlias, userModelId);

	TModifiedCreaturesMap::const_iterator it = _CreaturesMap.find(id);
	if (it != _CreaturesMap.end())
	{
		return it->second.CustomSheet;
	}
	return 0;
}

bool CDynamicSheetManager::scriptErrors(uint32 primAlias, const std::string &userModelId)
{
	CCustomElementId id(primAlias, userModelId);
	TScriptErrors::const_iterator it = _UserModelLoadingErrors.find(id);
	if (it != _UserModelLoadingErrors.end())
	{
		return it->second;
	}
	nlwarning("Can't find UserModel script '%s' associated to primAlias '%u'", userModelId.c_str(), primAlias);
	return true;
}

void CDynamicSheetManager::releaseCustomDataByServiceId(NLNET::TServiceId serviceId)
{	
	// release usermodels
	TModifiedCreaturesMap::iterator it = _CreaturesMap.begin();
	while(it != _CreaturesMap.end())
	{
		if (it->second.ServiceId == serviceId)
		{
			TModifiedCreaturesMap::iterator itToErase = it;
			++it;
			nldebug("'%s' model erased from manager because associated service '%u' is down", itToErase->first.Id.c_str(), serviceId.get());
			
			// also remove error information about that user model
			TScriptErrors::iterator error = _UserModelLoadingErrors.find(it->first);
			if (error != _UserModelLoadingErrors.end())
			{
				_UserModelLoadingErrors.erase(error);
			}
			_CreaturesMap.erase(itToErase);

			continue;
		}
		++it;
	}

	TCustomTables::iterator itTables = _CustomLootTables.begin();
	while (itTables != _CustomLootTables.end())
	{
		if (itTables->second.ServiceId == serviceId)
		{
			TCustomTables::iterator itToErase = itTables;
			nldebug("'%s' custom loot table erased from manager because associated service '%u' is down", itToErase->first.Id.c_str(), serviceId.get());
			++itTables;
			_CustomLootTables.erase(itToErase);

			continue;
		}
		++itTables;
	}
}

void CDynamicSheetManager::deleteCustomDataByPrimAlias(uint32 primAlias)
{
	CCustomElementId idLow(primAlias, "");
	CCustomElementId idHigh(primAlias + 1, "");

	nldebug("Deleting custom data for alias '%u'", primAlias); 

	TModifiedCreaturesMap::iterator upperBound = _CreaturesMap.upper_bound(idLow); // works because (primAlias, "") can't be part of the map (otherwise would not be included in the deletion)
	TModifiedCreaturesMap::iterator lowerBound = _CreaturesMap.lower_bound(idHigh);
	if (upperBound != _CreaturesMap.end())
		_CreaturesMap.erase(upperBound, lowerBound);

	TCustomTables::iterator tableUpperBound = _CustomLootTables.upper_bound(idLow);
	TCustomTables::iterator tableLowerBound = _CustomLootTables.lower_bound(idHigh);
	if (tableUpperBound != _CustomLootTables.end())
		_CustomLootTables.erase(tableUpperBound, tableLowerBound);
}

CStaticLootTable *CDynamicSheetManager::getLootTable(uint32 primAlias, const std::string &tableId)
{	
	CCustomElementId id(primAlias, tableId);
	TCustomTables::iterator it = _CustomLootTables.find(id);
	if (it != _CustomLootTables.end())
	{
		return &(it->second.Table);
	}
	nlwarning("Unable to find custom table '%s' associated to primAlias '%u' returning NULL", tableId.c_str(), primAlias);
	return 0;
}
