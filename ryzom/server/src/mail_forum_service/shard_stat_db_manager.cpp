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
#include "shard_stat_db_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


CShardStatDBManager * CShardStatDBManager::_Instance = NULL;


// ****************************************************************************
// CStatDBNameManager
// ****************************************************************************

// ****************************************************************************
void CStatDBNameManager::setPlayerName(NLMISC::CEntityId playerId, const std::string & playerName)
{
	if (playerName.empty())
		return;

	string & oldPlayerName = _PlayerNames[playerId];
	if (playerName != oldPlayerName)
	{
		oldPlayerName = playerName;
	}
}

// ****************************************************************************
void CStatDBNameManager::setGuildName(EGSPD::TGuildId guildId, const std::string & guildName)
{
	if (guildName.empty())
		return;

	string & oldGuildName = _GuildNames[guildId];
	if (guildName != oldGuildName)
	{
		oldGuildName = guildName;
	}
}

// ****************************************************************************
bool CStatDBNameManager::getPlayerName(NLMISC::CEntityId playerId, std::string & playerName) const
{
	map<NLMISC::CEntityId,std::string>::const_iterator it = _PlayerNames.find(playerId);
	if (it != _PlayerNames.end())
	{
		playerName = (*it).second;
		return true;
	}

	return false;
}

// ****************************************************************************
bool CStatDBNameManager::getGuildName(EGSPD::TGuildId guildId, std::string & guildName) const
{
	map<EGSPD::TGuildId,std::string>::const_iterator it = _GuildNames.find(guildId);
	if (it != _GuildNames.end())
	{
		guildName = (*it).second;
		return true;
	}

	return false;
}

// ****************************************************************************
void CStatDBNameManager::removePlayerName(NLMISC::CEntityId playerId)
{
	_PlayerNames.erase(playerId);
}

// ****************************************************************************
void CStatDBNameManager::removeGuildName(EGSPD::TGuildId guildId)
{
	_GuildNames.erase(guildId);
}

// ****************************************************************************
void CStatDBNameManager::loadNames(const CStatDBNamesMsg & namesMsg)
{
	for (	map<NLMISC::CEntityId,std::string>::const_iterator it = namesMsg.PlayerNames.begin();
			it != namesMsg.PlayerNames.end();
			++it)
	{
		setPlayerName((*it).first, (*it).second);
	}

	for (	map<EGSPD::TGuildId,std::string>::const_iterator it = namesMsg.GuildNames.begin();
			it != namesMsg.GuildNames.end();
			++it)
	{
		setGuildName((*it).first, (*it).second);
	}
}

// ****************************************************************************
// CStatDBNodeDisplayer
// ****************************************************************************

// ****************************************************************************
void CStatDBNodeDisplayer::displayNode(IStatDBNodePtr node, const std::string & currentPath, NLMISC::CLog & log, const CStatDBNameManager & nameManager)
{
	_Log = &log;
	_NameManager = &nameManager;

	if (_Settings.Recursive)
	{
		node->acceptVisitor(*this, currentPath);
	}
	else
	{
		displayOneNode(node, currentPath);

		// if the node is a branch 
		CStatDBBranch * branch = dynamic_cast<CStatDBBranch *>(node.getPtr());
		if (branch != NULL)
		{
			vector<IStatDBNode::CMatchingNode> children;
			branch->getNodes("*", children, currentPath);
			for (uint i = 0; i < children.size(); i++)
			{
				displayOneNode(children[i].Node, children[i].Path);
			}
			return;
		}
	}
}

// ****************************************************************************
void CStatDBNodeDisplayer::displayOneNode(IStatDBNodePtr node, const std::string & currentPath)
{
	BOMB_IF(node == NULL, "node is NULL!", return);

	CStatDBBranch * branch = dynamic_cast<CStatDBBranch *>(node.getPtr());
	if (branch != NULL)
	{
		visitBranch(branch, currentPath);
		return;
	}

	CStatDBValueLeaf * valueLeaf = dynamic_cast<CStatDBValueLeaf *>(node.getPtr());
	if (valueLeaf != NULL)
	{
		visitValueLeaf(valueLeaf, currentPath);
		return;
	}

	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(node.getPtr());
	if (tableLeaf != NULL)
	{
		visitTableLeaf(tableLeaf, currentPath);
		return;
	}
}

// ****************************************************************************
void CStatDBNodeDisplayer::visitBranch(CStatDBBranch * branch, const std::string & path)
{
	if (_Settings.DisplayBranch)
	{
		_Log->displayNL("(B) %s", path.c_str());
	}
}

// ****************************************************************************
void CStatDBNodeDisplayer::visitValueLeaf(CStatDBValueLeaf * valueLeaf, const std::string & path)
{
	if (_Settings.DisplayValueLeaf)
	{
		if (_Settings.DisplayValueLeafContent)
			_Log->displayNL("(V) %s = %d", path.c_str(), valueLeaf->getValue());
		else
			_Log->displayNL("(V) %s", path.c_str());
	}
}

// ****************************************************************************
void CStatDBNodeDisplayer::visitTableLeaf(CStatDBTableLeaf * tableLeaf, const std::string & path)
{
	if (_Settings.DisplayTableLeaf)
	{
		_Log->displayNL("(T) %s", path.c_str());
		if (_Settings.DisplayTableLeafContent)
		{
			for (	map<NLMISC::CEntityId,sint32>::const_iterator it = tableLeaf->getPlayerValues().begin();
					it != tableLeaf->getPlayerValues().end();
					++it)
			{
				string playerName;
				if (!_NameManager->getPlayerName((*it).first, playerName))
				{
					playerName = "[not found] " + (*it).first.toString();
				}
				
				_Log->displayNL("\tplayer '%s' = %d", playerName.c_str(), (*it).second);
			}

			for (	map<EGSPD::TGuildId,sint32>::const_iterator it = tableLeaf->getGuildValues().begin();
					it != tableLeaf->getGuildValues().end();
					++it)
			{
				string guildName;
				if (!_NameManager->getGuildName((*it).first, guildName))
				{
					guildName = toString("[not found] %u", (*it).first);
				}
				
				_Log->displayNL("\tguild '%s' = %d", guildName.c_str(), (*it).second);
			}
		}
	}
}

// ****************************************************************************
// CShardStatDBReader
// ****************************************************************************

// ****************************************************************************
CShardStatDBReader::CShardStatDBReader(CShardStatDBPtr statDB)
: _StatDB(statDB)
{
	nlassert(_StatDB != NULL);
}

// ****************************************************************************
uint32 CShardStatDBReader::getShardId() const
{
	return _StatDB->_ShardId;
}

// ****************************************************************************
bool CShardStatDBReader::getValue(const std::string & path, sint32 & val) const
{
	CStatDBValueLeaf * valueLeaf = dynamic_cast<CStatDBValueLeaf *>(_StatDB->_Root->getNode(path).getPtr());
	if (valueLeaf == NULL)
		return false;

	val = valueLeaf->getValue();
	return true;
}

// ****************************************************************************
bool CShardStatDBReader::getValues(const std::string & pathPattern, std::vector<sint32> & values) const
{
	vector<IStatDBNode::CMatchingNode> nodes;
	_StatDB->_Root->getNodes(pathPattern, nodes, "");

	if (nodes.empty())
		return false;

	values.clear();

	for (uint i = 0; i < nodes.size(); i++)
	{
		CStatDBValueLeaf * valueLeaf = dynamic_cast<CStatDBValueLeaf *>(nodes[i].Node.getPtr());
		if (valueLeaf == NULL)
		{
			nlwarning("a node found at '%s' is not a value leaf", pathPattern.c_str());
			return false;
		}

		values.push_back(valueLeaf->getValue());
	}

	return true;
}

// ****************************************************************************
bool CShardStatDBReader::getTable(const std::string & path,
								  const TPlayerValues * & playerValues,
								  const TGuildValues * & guildValues) const
{
	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_StatDB->_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	playerValues = &tableLeaf->getPlayerValues();
	guildValues = &tableLeaf->getGuildValues();
	return true;
}

// ****************************************************************************
bool CShardStatDBReader::getTables(const std::string & pathPattern,
								   std::vector<const TPlayerValues *> & playerValuesVec,
								   std::vector<const TGuildValues *> & guildValuesVec) const
{
	vector<IStatDBNode::CMatchingNode> nodes;
	_StatDB->_Root->getNodes(pathPattern, nodes, "");

	if (nodes.empty())
		return false;

	playerValuesVec.clear();
	guildValuesVec.clear();

	for (uint i = 0; i < nodes.size(); i++)
	{
		CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(nodes[i].Node.getPtr());
		if (tableLeaf == NULL)
		{
			nlwarning("a node found at '%s' is not a table leaf", pathPattern.c_str());
			return false;
		}

		playerValuesVec.push_back(&tableLeaf->getPlayerValues());
		guildValuesVec.push_back(&tableLeaf->getGuildValues());
	}

	return true;
}

// ****************************************************************************
bool CShardStatDBReader::getPlayerName(NLMISC::CEntityId playerId, std::string & playerName) const
{
	return _StatDB->getPlayerName(playerId, playerName);
}

// ****************************************************************************
bool CShardStatDBReader::getGuildName(EGSPD::TGuildId guildId, std::string & guildName) const
{
	return _StatDB->getGuildName(guildId, guildName);
}

// ****************************************************************************
// CShardStatDB
// ****************************************************************************

// ****************************************************************************
CShardStatDB::CShardStatDB(uint32 shardId)
: _ShardId(shardId)
{
	_Root = new CStatDBBranch;
}

// ****************************************************************************
void CShardStatDB::resetStatDB()
{
	_Root = new CStatDBBranch;
}

// ****************************************************************************
bool CShardStatDB::createValue(const std::string & path, sint32 val)
{
	IStatDBNodePtr node = _Root->getNode(path);
	if (node != NULL)
		return false;

	return _Root->setNode(path, new CStatDBValueLeaf(val));
}

// ****************************************************************************
bool CShardStatDB::valueSet(const std::string & path, sint32 val)
{
	CStatDBValueLeaf * valueLeaf = dynamic_cast<CStatDBValueLeaf *>(_Root->getNode(path).getPtr());
	if (valueLeaf == NULL)
		return false;

	valueLeaf->setValue(val);
	return true;
}

// ****************************************************************************
bool CShardStatDB::valueAdd(const std::string & path, sint32 val)
{
	CStatDBValueLeaf * valueLeaf = dynamic_cast<CStatDBValueLeaf *>(_Root->getNode(path).getPtr());
	if (valueLeaf == NULL)
		return false;

	valueLeaf->addValue(val);
	return true;
}

// ****************************************************************************
bool CShardStatDB::createTable(const std::string & path,
							   const std::map<NLMISC::CEntityId,sint32> & playerValues,
							   const std::map<EGSPD::TGuildId,sint32> & guildValues
							   )
{
	IStatDBNodePtr node = _Root->getNode(path);
	if (node != NULL)
		return false;

	return _Root->setNode(path, new CStatDBTableLeaf(playerValues, guildValues));
}

// ****************************************************************************
bool CShardStatDB::tablePlayerAdd(const std::string & path, NLMISC::CEntityId playerId, sint32 val)
{
	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	tableLeaf->playerAdd(playerId, val);
	return true;
}

// ****************************************************************************
bool CShardStatDB::tablePlayerSet(const std::string & path, NLMISC::CEntityId playerId, sint32 val)
{
	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	tableLeaf->playerSet(playerId, val);
	return true;
}

// ****************************************************************************
bool CShardStatDB::tableGuildAdd(const std::string & path, EGSPD::TGuildId guildId, sint32 val)
{
	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	tableLeaf->guildAdd(guildId, val);
	return true;
}

// ****************************************************************************
bool CShardStatDB::tableGuildSet(const std::string & path, EGSPD::TGuildId guildId, sint32 val)
{
	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	tableLeaf->guildSet(guildId, val);
	return true;
}

// ****************************************************************************
bool CShardStatDB::removeNode(const std::string & path)
{
	IStatDBNodePtr removedNode = _Root->removeNode(path);
	return (removedNode != NULL);
}

// ****************************************************************************
void CShardStatDB::removePlayer(NLMISC::CEntityId playerId)
{
	CStatDBEntitiesRemoval entitiesRemoval;
	entitiesRemoval.addPlayerToRemove(playerId);
	entitiesRemoval.processRemoval(_Root);

	removePlayerName(playerId);
}

// ****************************************************************************
void CShardStatDB::removeGuild(EGSPD::TGuildId guildId)
{
	CStatDBEntitiesRemoval entitiesRemoval;
	entitiesRemoval.addGuildToRemove(guildId);
	entitiesRemoval.processRemoval(_Root);

	removeGuildName(guildId);
}

// ****************************************************************************
bool CShardStatDB::displayNodes(const std::string & pathPattern, NLMISC::CLog & log, const CStatDBNodeDisplayer::CSettings & settings)
{
	vector<IStatDBNode::CMatchingNode> nodes;
	_Root->getNodes(pathPattern, nodes, "");

	if (nodes.empty())
		return false;

	CStatDBNodeDisplayer nodeDisplayer;
	nodeDisplayer.setSettings(settings);

	for (uint i = 0; i < nodes.size(); i++)
	{
		nodeDisplayer.displayNode(nodes[i].Node, nodes[i].Path, log, *this);
	}

	return true;
}

// ****************************************************************************
// CShardStatDBManager
// ****************************************************************************

// ****************************************************************************
CShardStatDBManager::CShardStatDBManager()
{
}

// ****************************************************************************
void CShardStatDBManager::init()
{
	TUnifiedCallbackItem cbClientArray[] =
	{
		"SDB:INIT",					cbInit,
		"SDB:CREATE_VALUE",			cbCreateValue,
		"SDB:CREATE_TABLE",			cbCreateTable,
		"SDB:VALUE_SET",			cbValueSet,
		"SDB:VALUE_ADD",			cbValueAdd,
		"SDB:TABLE_PLAYER_ADD",		cbTablePlayerAdd,
		"SDB:TABLE_PLAYER_SET",		cbTablePlayerSet,
		"SDB:TABLE_GUILD_ADD",		cbTableGuildAdd,
		"SDB:TABLE_GUILD_SET",		cbTableGuildSet,
		"SDB:REMOVE_NODE",			cbRemoveNode,
		"SDB:REMOVE_PLAYER",		cbRemovePlayer,
		"SDB:REMOVE_GUILD",			cbRemoveGuild,
	};

	CUnifiedNetwork::getInstance()->addCallbackArray(cbClientArray, sizeof(cbClientArray)/sizeof(cbClientArray[0]));
}

// ****************************************************************************
void CShardStatDBManager::getShardStatDBReaders(std::vector<CShardStatDBReader> & statDBReaders) const
{
	statDBReaders.clear();
	for (uint i = 0; i < _ShardStatDB.size(); i++)
	{
		statDBReaders.push_back(_ShardStatDB[i]->getReader());
	}
}

// ****************************************************************************
void CShardStatDBManager::initShardStatDB(uint32 shardId, const CStatDBAllLeavesMsg & allLeavesMsg)
{
	CShardStatDBPtr statDB;

	for (uint i = 0; i < _ShardStatDB.size(); i++)
	{
		if (_ShardStatDB[i]->getShardId() == shardId)
		{
			statDB = _ShardStatDB[i];
			nlassert(statDB != NULL);
			statDB->resetStatDB();
			break;
		}
	}

	if (statDB == NULL)
	{
		statDB = new CShardStatDB(shardId);
		_ShardStatDB.push_back(statDB);
	}

	for (uint i = 0; i < allLeavesMsg.ValueLeavesMsg.size(); i++)
	{
		const CStatDBValueLeafMsg & valueLeafMsg = allLeavesMsg.ValueLeavesMsg[i];
		if (!statDB->createValue(valueLeafMsg.Path, valueLeafMsg.Value))
		{
			STOP(toString("cannot create a value leaf at '%s'", valueLeafMsg.Path.c_str()));
		}
	}

	for (uint i = 0; i < allLeavesMsg.TableLeavesMsg.size(); i++)
	{
		const CStatDBTableLeafMsg & tableLeafMsg = allLeavesMsg.TableLeavesMsg[i];
		statDB->createTable(tableLeafMsg.Path, tableLeafMsg.PlayerValues, tableLeafMsg.GuildValues);
	}

	for (	map<NLMISC::CEntityId,std::string>::const_iterator it = allLeavesMsg.NamesMsg.PlayerNames.begin();
			it != allLeavesMsg.NamesMsg.PlayerNames.end();
			++it)
	{
		statDB->setPlayerName((*it).first, (*it).second);
	}

	for (	map<EGSPD::TGuildId,std::string>::const_iterator it = allLeavesMsg.NamesMsg.GuildNames.begin();
			it != allLeavesMsg.NamesMsg.GuildNames.end();
			++it)
	{
		statDB->setGuildName((*it).first, (*it).second);
	}
}

// ****************************************************************************
CShardStatDBPtr CShardStatDBManager::getShardStatDB(uint32 shardId)
{
	for (uint i = 0; i < _ShardStatDB.size(); i++)
	{
		if (_ShardStatDB[i]->getShardId() == shardId)
		{
			return _ShardStatDB[i];
		}
	}

	return NULL;
}

// ****************************************************************************
void CShardStatDBManager::cbInit(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	CStatDBAllLeavesMsg allLeavesMsg;

	msgin.serial(shardId);
	msgin.serial(allLeavesMsg);

	getInstance()->initShardStatDB(shardId, allLeavesMsg);
}

// ****************************************************************************
void CShardStatDBManager::cbCreateValue(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	CStatDBValueLeafMsg valueLeafMsg;

	msgin.serial(shardId);
	msgin.serial(valueLeafMsg);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->createValue(valueLeafMsg.Path, valueLeafMsg.Value);
}

// ****************************************************************************
void CShardStatDBManager::cbCreateTable(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	CStatDBTableLeafMsg tableLeafMsg;
	CStatDBNamesMsg namesMsg;

	msgin.serial(shardId);
	msgin.serial(tableLeafMsg);
	msgin.serial(namesMsg);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->createTable(tableLeafMsg.Path, tableLeafMsg.PlayerValues, tableLeafMsg.GuildValues);
	statDB->loadNames(namesMsg);
}

// ****************************************************************************
void CShardStatDBManager::cbValueSet(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	string path;
	sint32 val;

	msgin.serial(shardId);
	msgin.serial(path);
	msgin.serial(val);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->valueSet(path, val);
}

// ****************************************************************************
void CShardStatDBManager::cbValueAdd(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	string path;
	sint32 val;

	msgin.serial(shardId);
	msgin.serial(path);
	msgin.serial(val);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->valueAdd(path, val);
}

// ****************************************************************************
void CShardStatDBManager::cbTablePlayerAdd(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	string path;
	CEntityId playerId;
	string playerName;
	sint32 val;

	msgin.serial(shardId);
	msgin.serial(path);
	msgin.serial(playerId);
	msgin.serial(playerName);
	msgin.serial(val);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->tablePlayerAdd(path, playerId, val);
	statDB->setPlayerName(playerId, playerName);
}

// ****************************************************************************
void CShardStatDBManager::cbTablePlayerSet(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	string path;
	CEntityId playerId;
	string playerName;
	sint32 val;

	msgin.serial(shardId);
	msgin.serial(path);
	msgin.serial(playerId);
	msgin.serial(playerName);
	msgin.serial(val);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->tablePlayerSet(path, playerId, val);
	statDB->setPlayerName(playerId, playerName);
}

// ****************************************************************************
void CShardStatDBManager::cbTableGuildAdd(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	string path;
	EGSPD::TGuildId guildId;
	string guildName;
	sint32 val;

	msgin.serial(shardId);
	msgin.serial(path);
	msgin.serial(guildId);
	msgin.serial(guildName);
	msgin.serial(val);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->tableGuildAdd(path, guildId, val);
	statDB->setGuildName(guildId, guildName);
}

// ****************************************************************************
void CShardStatDBManager::cbTableGuildSet(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	string path;
	EGSPD::TGuildId guildId;
	string guildName;
	sint32 val;

	msgin.serial(shardId);
	msgin.serial(path);
	msgin.serial(guildId);
	msgin.serial(guildName);
	msgin.serial(val);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->tableGuildSet(path, guildId, val);
	statDB->setGuildName(guildId, guildName);
}

// ****************************************************************************
void CShardStatDBManager::cbRemoveNode(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	string path;

	msgin.serial(shardId);
	msgin.serial(path);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->removeNode(path);
}

// ****************************************************************************
void CShardStatDBManager::cbRemovePlayer(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	CEntityId playerId;

	msgin.serial(shardId);
	msgin.serial(playerId);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->removePlayer(playerId);
}

// ****************************************************************************
void CShardStatDBManager::cbRemoveGuild(CMessage & msgin, const std::string & serviceName, TServiceId serviceId)
{
	uint32 shardId;
	EGSPD::TGuildId guildId;

	msgin.serial(shardId);
	msgin.serial(guildId);

	CShardStatDB * statDB = getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		STOP(toString("stat db of shard %u is not initialized yet!", shardId));
		return;
	}
	
	statDB->removeGuild(guildId);
}

// ****************************************************************************
// Commands
// ****************************************************************************

// ****************************************************************************
NLMISC_COMMAND (sdbDisplayNodes, "display nodes of SDB", "<shard_id> <path> [<recursive>] [<display_values>] [<display_tables>]")
{
	if (args.size() < 2 || args.size() > 5)
		return false;

	uint32 shardId;
	NLMISC::fromString(args[0], shardId);

	CShardStatDB * statDB = CShardStatDBManager::getInstance()->getShardStatDB(shardId);
	if (statDB == NULL)
	{
		log.displayNL("shard %u has no stat db", shardId);
		return true;
	}

	const string & pathPattern = args[1];

	CStatDBNodeDisplayer::CSettings settings;

	if (args.size() >= 3)
		settings.Recursive = (args[2] == "1" || args[2] == "true");

	if (args.size() >= 4)
		settings.DisplayValueLeafContent = (args[3] == "1" || args[3] == "true");

	if (args.size() >= 5)
		settings.DisplayTableLeafContent = (args[4] == "1" || args[4] == "true");

	settings.DisplayBranch = !settings.Recursive;

	if (!statDB->displayNodes(pathPattern, log, settings))
	{
		log.displayNL("path '%s' not found", pathPattern.c_str());
	}

	return true;
}

