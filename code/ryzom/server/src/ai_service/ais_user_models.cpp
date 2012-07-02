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
#include "ais_user_models.h"
#include "ai_instance.h"
#include "ais_actions.h"
#include "nel/misc/sstring.h"
#include "nel/misc/algo.h"



using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace	AITYPES;
using namespace CAISActionEnums;

/**
* This manager is used by the dynamic creature system, allowing leveldesigners to modify a npc's basesheet attributes,
* or to define custom loot tables.
* First, the primitive is parsed in order to find usermodels and/or custom loot tables (see primitive_parser.cpp,
* methods: parsePrimCustomLootTable, parsePrimUserModelList). 
* The datas are stored in a specific class (CSCriptData for userModels, CCustomLootTableManager for clt). These classes 
* are defined in game_share so that they can be re-used egs side. 
* The datas are then sent to EGS, trough 2 AIActions "USR_MDL" for user models and "CUSTOMLT" for custom loot tables
* The real objects CDynamicSheet and CLootTables are built egs-side by the egs CDynamicSheetManager.
*/


CAIUserModelManager* CAIUserModelManager::_instance = 0;

/************************************************************************/
/*							UTILS                                       */
/************************************************************************/
void stripWhitespaces(std::string& str)
{
   if(str.empty()) return;

   string::size_type startIndex = str.find_first_not_of(" ");
   string::size_type endIndex = str.find_last_not_of(" ");
   std::string tmp = str;
   str.erase();

   str = tmp.substr(startIndex, (endIndex-startIndex+ 1) );
}


std::string removeComment(const std::string &str) 
{
	string::size_type newPos= str.find("//",0);
	
	if (newPos != string::npos)
	{
		if (newPos == 0)
			return "";
		else
			return str.substr(0, newPos);		
	}
	return str;
}


/************************************************************************/
/*							AI_ACTIONS                                  */
/************************************************************************/

/**
* used to send usermodels to EGS
*/
DEFINE_ACTION(ContextGlobal,USR_MDL)
{
	//not true anymore : primAlias also pushed in the vector
	/*if (args.size() % 3 != 0)
		return;*/
	
	TScripts scriptMap;
	std::vector<CAIActions::CArg>::const_iterator it = args.begin();
	
	uint32 primAlias;

	for (it = args.begin(); it != args.end();++it)
	{
		if (it == args.begin())
		{
			if (it->get(primAlias) == false)
			{
				return;
			}
			continue;
		}
		std::string modelId(it->toString());
		std::string sheetId((++it)->toString());
		std::string script((++it)->toString());
		
		//will contain all script info and the base sheet id
		TScriptContent scriptContent;
		//first push base sheet Id
		scriptContent.push_back(sheetId);
		
		//build a temporary vector. Each element of the vector is a script line
		std::vector<string> tmpVector;
		splitString(script, "\n", tmpVector);

		std::vector<string>::iterator tmpIt;
		for (tmpIt = tmpVector.begin(); tmpIt != tmpVector.end(); ++tmpIt)
		{
			//remove comment from the line and push back the line only if the line wasn't just a comment
			std::string noComment = removeComment(*tmpIt);
			stripWhitespaces(noComment);
			if (noComment != "")
				scriptContent.push_back(noComment);
		}
		
		//add the usermodel to the manager's map
		CAIUserModelManager::getInstance()->addToUserModels(primAlias, modelId, scriptContent);
	}
	
	CContextStack::setContext(ContextGlobal);

	//send msg to EGS if EGS up
	if (EGSHasMirrorReady)
	{
		CAIUserModelManager::getInstance()->sendUserModels();
	}
}

/**
* used to send custom loot tables
*/
DEFINE_ACTION(ContextGlobal,CUSTOMLT)
{
	std::vector<CAIActions::CArg>::const_iterator it = args.begin();;
	
	uint32 tablesNb; 
	if (it->get(tablesNb) == false)
	{
		nlwarning("<CAIUserModelManager::CUSTOMLT>Unable to retrieve number of parsed loot tables, abort.");
		return;
	}
	nldebug("<CAIUserModelManager::CUSTOMLT>Number of parsed loot tables: %u", tablesNb);
	++it;


	uint32 primAlias;
	if (it->get(primAlias) == false)
	{
		nlwarning("<CAIUserModelManager::CUSTOMLT>Unable to retrieve primitive alias, abort.");
		return;
	}
	
	TCustomLootTable customLootTables;

	//for each loot table
	for (uint i = 0; i < tablesNb; ++i)
	{	
		++it;

		//gets number of loot sets in a loot table
		uint32 lootSetsNb;
		if (it->get(lootSetsNb) == false)
		{
			return;
		}
		
		//then gets all info relative to the loot table (id, money values)
		std::string lootTableId((++it)->toString());
		float moneyProba;
		if ((++it)->get(moneyProba) == false)
		{
			nlwarning("<CAIUserModelManager::CUSTOMLT> unable to get moneyDropProbability");
			return;
		}
		if (moneyProba < 0.0 || moneyProba > 1.0)
		{
			nlwarning("<CAIUserModelManager::CUSTOMLT> invalid moneyDropProbability, don't send custom loot tables info to EGS");
			return;
		}
		
		float moneyFactor;
		if ((++it)->get(moneyFactor) == false)
		{
			nlwarning("<CAIUserModelManager::CUSTOMLT> unable to get moneyFactor");
			return;
		}
		if (moneyFactor < 0.0)
		{
			nlwarning("<CAIUserModelManager::CUSTOMLT> invalid moneyFactor, don't send custom loot tables info to EGS");
			return;
		}

		uint32 moneyBase;
		if ((++it)->get(moneyBase) == false)
		{
			nlwarning("<CAIUserModelManager::CUSTOMLT> unable to get moneyBase");
			return;
		}
		if (moneyBase < 0)
		{
			nlwarning("<CAIUserModelManager::CUSTOMLT> invalid moneyBase, don't send custom loot tables info to EGS");
			return;
		}


		TScripts lootSets;
		//for each loot set inside the current table
		for (uint j = 0; j < lootSetsNb; ++j)
		{
			std::string dropProba((++it)->toString());
			std::string script((++it)->toString());

			TScriptContent lootSetContent;
			
			std::vector<string> tmpVector;
			splitString(script, "\n", tmpVector);
			
			std::vector<string>::iterator tmpIt;
			for (tmpIt = tmpVector.begin(); tmpIt != tmpVector.end(); ++tmpIt)
			{
				std::string noComment = removeComment(*tmpIt);
				stripWhitespaces(noComment);
				if (noComment != "")
					lootSetContent.push_back(noComment);
			}		
			uint16 proba;
			NLMISC::fromString(dropProba, proba);
			// FIXME: test on proba value...
			if (proba == 0 || proba > 100)
			{
				nlwarning("<CUSTOMLT> invalid drop proba value for lootset %i in custom loot table '%s', skip the lootset",
					j + 1, lootTableId.c_str() );
				continue;
			}
			lootSets.insert(make_pair(CCustomElementId(0, dropProba), lootSetContent));
		}
		
		if (lootSets.empty())
		{
			nlwarning("<CUSTOMLT> Don't add empty lootsets to customloot table, skip loot table '%s'.", lootTableId.c_str());
			continue;
		}

		nldebug("<CAIUserModelManager> Adding table '%s' to manager with primAlias '%u'", lootTableId.c_str(), primAlias);
		CAIUserModelManager::getInstance()->addCustomLootTable(primAlias, lootTableId,lootSets,
				moneyProba, moneyFactor, moneyBase);
	}

	CContextStack::setContext(ContextGlobal);

	//send msg to EGS if EGS up
	if (EGSHasMirrorReady)
	{
		CAIUserModelManager::getInstance()->sendCustomLootTables();
	}
}


CAIUserModelManager *CAIUserModelManager::getInstance()
{
	if (_instance==0)
	{
		_instance = new CAIUserModelManager();
		_instance->init();
	}
	return _instance;
}

void CAIUserModelManager::destroyInstance()
{
	delete _instance;
	_instance = 0;
}

CAIUserModelManager::CAIUserModelManager()
{

}

CAIUserModelManager::~CAIUserModelManager()
{
	
}

void CAIUserModelManager::init()
{
	nldebug("<CAIUserModelManager> Manager initialized.");
}

void CAIUserModelManager::addToUserModels(uint32				primAlias,
										  const std::string		&userModelId, 
										  const TScriptContent	&userModel)
{
	bool inserted = _UserModels.Scripts.insert(make_pair(CCustomElementId(primAlias, userModelId), userModel)).second;
	if (!inserted)
	{
		nlwarning("<AIUserModelManager::addToUserModels> tried to register twice the same user model with id: '%s'", userModelId.c_str());
		return;
	}
	nldebug("<AIUserModelManager::addToUserModels> Added usermodel '%s' with alias '%u'", userModelId.c_str(), primAlias);
}

void CAIUserModelManager::sendUserModels()
{
	NLNET::CMessage msgout("USER_MODELS");
	const CScriptData &scriptData = CAIUserModelManager::getInstance()->getUserModels();
	msgout.serial(const_cast<CScriptData&>(scriptData));
	
	nldebug("<CAIUserModelManager> Sending %u user models to EGS", scriptData.Scripts.size());
	sendMessageViaMirror("EGS", msgout);
}

bool CAIUserModelManager::isUserModel(uint32 primAlias, const std::string &userModelId) const
{	
	CCustomElementId id(primAlias, userModelId);

	return _UserModels.Scripts.find(id) != _UserModels.Scripts.end();
}

bool CAIUserModelManager::isCustomLootTable(uint32 primAlias, const std::string &lootTableId) const
{
	CCustomElementId id(primAlias, lootTableId);
	
	return _CustomLootTables.Tables.find(id) != _CustomLootTables.Tables.end();
}


void CAIUserModelManager::sendCustomLootTables()
{
	nldebug("<CAIUserModelManager> Sending custom loot tables to EGS");
	NLNET::CMessage msgout("CUSTOM_LOOT_TABLES");
	CCustomLootTableManager customLootTable = CAIUserModelManager::getInstance()->getCustomLootTables();
	msgout.serial(customLootTable);
	sendMessageViaMirror("EGS", msgout);
}

void CAIUserModelManager::addCustomLootTable(uint32				primAlias,
											 const std::string	&tableId, 
											 TScripts			lootSets,
											 float				moneyProba,
											 float				moneyFactor,
											 uint32				moneyBase
											 )
{
	if (CAIUserModelManager::getInstance()->isCustomLootTable(primAlias, tableId) == true)
	{
		nlwarning("<CAIUserModelManager::addCustomLootTable> Table '%s' with primAlias '%u' is already present in manager. Skipping it.", tableId.c_str(), primAlias);
		return;
	}
	CScriptData tableContent;
	tableContent.Scripts = lootSets;
	CCustomLootTable customLootTable;

	customLootTable.LootSets = tableContent;
	customLootTable.MoneyProba = moneyProba;
	customLootTable.MoneyFactor = moneyFactor;
	customLootTable.MoneyBase = moneyBase;

	_CustomLootTables.Tables.insert(make_pair(CCustomElementId(primAlias, tableId), customLootTable));
}


void CAIUserModelManager::deleteCustomDataByPrimAlias(uint32 primAlias)
{
	CCustomElementId idLow(primAlias, "");
	CCustomElementId idHigh(primAlias + 1, "");
	TScripts::iterator upperBound = _UserModels.Scripts.upper_bound(idLow);
	TScripts::iterator lowerBound = _UserModels.Scripts.lower_bound(idHigh);
	if (upperBound != _UserModels.Scripts.end())
		_UserModels.Scripts.erase(upperBound, lowerBound);
	
	TCustomLootTable::iterator upperBoundLootTables = _CustomLootTables.Tables.upper_bound(idLow);
	TCustomLootTable::iterator lowerBoundLootTables = _CustomLootTables.Tables.lower_bound(idHigh);

	if (upperBoundLootTables != _CustomLootTables.Tables.end())
		_CustomLootTables.Tables.erase(upperBoundLootTables, lowerBoundLootTables);

	if (EGSHasMirrorReady)
	{
		nldebug("<CAIUserModelManager::deleteCustomData> Sending alias '%u' to EGS for data deletion", primAlias);
		NLNET::CMessage msgout("DELCUSTOM");
		msgout.serial(primAlias);

		sendMessageViaMirror("EGS", msgout);
	}
}



