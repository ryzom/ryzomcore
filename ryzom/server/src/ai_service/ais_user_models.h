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

#ifndef AIS_USER_MODELS
#define AIS_USER_MODELS


#include "server_share/npc_description_messages.h"

class CAIUserModelManager
{
public:
	static CAIUserModelManager *getInstance();
	static void destroyInstance();

	void init();

	CAIUserModelManager();
	~CAIUserModelManager();

	/** Add a user model in the map that will be sent to EGS 
	 *  @param primAlias the primitive alias
	 *	@param userModelId the id of the added user model, defined in the worldEdit user_model node 
	 *	@param userModel the script data of the user model, stored as a string vector containing all script lines
	 */
	void addToUserModels(uint32 primAlias, const std::string &userModelId, const TScriptContent &userModel);
	
	/** send user models to EGS for script parsing and modified CStaticCreatures instanciation
	 */
	void sendUserModels();
	
	/** send custom loot table script and info to EGS for script parsing and custom CStaticLootTable instanciation
	 */
	void sendCustomLootTables();
	
	const CScriptData &getUserModels() { return _UserModels;}

	const CCustomLootTableManager &getCustomLootTables() { return _CustomLootTables;}
	
	bool isUserModel(uint32 primAlias, const std::string &id) const;

	bool isCustomLootTable(uint32 primAlias, const std::string &id) const;
	
	/** Add a custom loot table in the struct that will be sent to EGS
	* @param tableId the id of the added table, defined in the worldEdit custom_loot_table node.
	* @param lootSets map of vector, containing all lootSets lines (key: lootSet id)
	* @param moneyProba the probability for the added custom loot table to contain money, defined in the worldEdit custom_loot_table node.
	* @param moneyFactor value used in the computation of the amount of money dropped, defined in the worldEdit custom_loot_table node.
	* @param moneyBase value representing the money base present in the table, defined in the worldEdit custom_loot_table node.
	*/
	void addCustomLootTable(uint32				primAlias,
							const std::string	&tableId, 
							TScripts			lootSets,
							float				moneyProba,
							float				moneyFactor,
							uint32				moneyBase);
	
	/** Delete all datas that were loaded from a given primitive (allow to dynamically load/reload custom datas)
	* The alias is the static part of the primitive alias basis. This alias is used as part of the identifier 
	* for all custom model and custom loot tables.
	*/
	void deleteCustomDataByPrimAlias(uint32 primAlias);

private:
	static CAIUserModelManager *_instance;
	/** Stores all parsed user models before sending them to EGS.
	* see @class CScriptData
	*/
	CScriptData _UserModels;

	/** Stores all parsed Custom Loot Tables before sending them to EGS
	* see @class CCustomLootTableManager
	*/
	CCustomLootTableManager _CustomLootTables;
};


#endif

