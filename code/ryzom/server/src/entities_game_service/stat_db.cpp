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
#include "stat_db.h"

#include "nel/net/service.h"

#include "server_share/mail_forum_validator.h"

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"

#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// ****************************************************************************

CStatDB *CStatDB::_Instance = NULL;

CVariable<uint32> StatDBSavePeriod("egs", "StatDBSavePeriod","stat database save period in ticks", 6, 0, true);

extern CVariable<bool> EGSLight;

// ****************************************************************************
// Helpers
// ****************************************************************************

// ****************************************************************************
static bool getPlayerName(CEntityId playerId, string & playerName)
{
	playerName = CEntityIdTranslator::getInstance()->getByEntity(playerId).toUtf8();
	return !playerName.empty();
}

// ****************************************************************************
static bool getGuildName(EGSPD::TGuildId guildId, string & guildName)
{
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(guildId);
	if (guild == NULL)
		return false;

	guildName = guild->getName().toUtf8();
	return true;
}

// ****************************************************************************
static void getNamesFromTable(const CStatDBTableLeafMsg & tableLeafMsg, CStatDBNamesMsg & namesMsg)
{
	for (	map<NLMISC::CEntityId,sint32>::const_iterator it = tableLeafMsg.PlayerValues.begin();
			it != tableLeafMsg.PlayerValues.end();
			++it)
	{
		const CEntityId & playerId = (*it).first;

		string playerName;
		if (!getPlayerName(playerId, playerName))
		{
#if FINAL_VERSION
			nlwarning("SDB: cannot find the name of the player %s", playerId.toString().c_str());
#endif // FINAL_VERSION
			continue;
		}

		namesMsg.PlayerNames[playerId] = playerName;
	}

	for (	map<EGSPD::TGuildId,sint32>::const_iterator it = tableLeafMsg.GuildValues.begin();
			it != tableLeafMsg.GuildValues.end();
			++it)
	{
		const EGSPD::TGuildId & guildId = (*it).first;

		string guildName;
		if (!getGuildName(guildId, guildName))
		{
#if FINAL_VERSION
			nlwarning("SDB: cannot find the name of the guild %u", guildId);
#endif // FINAL_VERSION
			continue;
		}

		namesMsg.GuildNames[guildId] = guildName;
	}
}

// ****************************************************************************
// CStatDBBackupLeafCollector
// ****************************************************************************

// ****************************************************************************
void CStatDBBackupLeafCollector::loadLeaves(IStatDBNodePtr root)
{
	nlassert(root != NULL);

	clearLeaves();
	_Root = root;
	_Root->acceptVisitor(*this, "");
}

// ****************************************************************************
bool CStatDBBackupLeafCollector::popTableLeafPD(CStatDBTableLeafPD & tableLeafPD)
{
	if (_Root == NULL)
		return false;

	while (!_TableLeafPaths.empty())
	{
		string path = _TableLeafPaths.back();
		_TableLeafPaths.pop_back();

		CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
		if (tableLeaf != NULL)
		{
			tableLeafPD.Path = path;
			tableLeafPD.PlayerValues = tableLeaf->getPlayerValues();
			tableLeafPD.GuildValues = tableLeaf->getGuildValues();
			return true;
		}
	}

	return false;
}

// ****************************************************************************
void CStatDBBackupLeafCollector::visitValueLeaf(CStatDBValueLeaf * valueLeaf, const std::string & path)
{
	CStatDBValueLeafPD valueLeafPD;
	valueLeafPD.Path = path;
	valueLeafPD.Value = valueLeaf->getValue();

	_ValueLeavesPD.ValueLeavesPD.push_back(valueLeafPD);
}

// ****************************************************************************
void CStatDBBackupLeafCollector::visitTableLeaf(CStatDBTableLeaf * tableLeaf, const std::string & path)
{
	_TableLeafPaths.push_back(path);
}

// ****************************************************************************
// CStatDBMFSInitLeafCollector
// ****************************************************************************

// ****************************************************************************
void CStatDBMFSInitLeafCollector::loadLeaves(IStatDBNodePtr root, CStatDBAllLeavesMsg & allLeavesMsg)
{
	nlassert(root != NULL);

	_AllLeavesMsg = &allLeavesMsg;
	_AllLeavesMsg->ValueLeavesMsg.clear();
	_AllLeavesMsg->TableLeavesMsg.clear();
	_AllLeavesMsg->NamesMsg.PlayerNames.clear();
	_AllLeavesMsg->NamesMsg.GuildNames.clear();

	root->acceptVisitor(*this, "");
}

// ****************************************************************************
void CStatDBMFSInitLeafCollector::visitValueLeaf(CStatDBValueLeaf * valueLeaf, const std::string & path)
{
	CStatDBValueLeafMsg valueLeafMsg;
	valueLeafMsg.Path = path;
	valueLeafMsg.Value = valueLeaf->getValue();

	_AllLeavesMsg->ValueLeavesMsg.push_back(valueLeafMsg);
}

// ****************************************************************************
void CStatDBMFSInitLeafCollector::visitTableLeaf(CStatDBTableLeaf * tableLeaf, const std::string & path)
{
	CStatDBTableLeafMsg tableLeafMsg;
	tableLeafMsg.Path = path;
	tableLeafMsg.PlayerValues = tableLeaf->getPlayerValues();
	tableLeafMsg.GuildValues = tableLeaf->getGuildValues();

	_AllLeavesMsg->TableLeavesMsg.push_back(tableLeafMsg);

	getNamesFromTable(tableLeafMsg, _AllLeavesMsg->NamesMsg);
}

// ****************************************************************************
// CStatDBNodeDisplayer
// ****************************************************************************

// ****************************************************************************
void CStatDBNodeDisplayer::displayNode(IStatDBNodePtr node, const std::string & currentPath, NLMISC::CLog & log)
{
	_Log = &log;

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
				if (!getPlayerName((*it).first, playerName))
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
				if (!getGuildName((*it).first, guildName))
				{
					guildName = toString("[not found] %u", (*it).first);
				}
				
				_Log->displayNL("\tguild '%s' = %d", guildName.c_str(), (*it).second);
			}
		}
	}
}

// ****************************************************************************
// CStatDB
// ****************************************************************************

// ****************************************************************************
CStatDB::CStatDB()
{
	_Root = new CStatDBBranch;
	_SDBIsLoaded = false;
	_GuildsAreLoaded = false;
	_MFSIsUp = false;
	_MFSIsInitialized = false;
}

// ****************************************************************************
bool CStatDB::createValue(const std::string & path, sint32 val)
{
	nlassert(_SDBIsLoaded);

	IStatDBNodePtr node = _Root->getNode(path);
	if (node != NULL)
		return false;

	bool res = _Root->setNode(path, new CStatDBValueLeaf(val));

	if (res && _MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();
		CStatDBValueLeafMsg valueLeafMsg;
		valueLeafMsg.Path = path;
		valueLeafMsg.Value = val;

		CMessage msgout("SDB:CREATE_VALUE");
		msgout.serial(shardId);
		msgout.serial(valueLeafMsg);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	return res;
}

// ****************************************************************************
bool CStatDB::valueSet(const std::string & path, sint32 val)
{
	nlassert(_SDBIsLoaded);

	CStatDBValueLeaf * valueLeaf = dynamic_cast<CStatDBValueLeaf *>(_Root->getNode(path).getPtr());
	if (valueLeaf == NULL)
		return false;

	valueLeaf->setValue(val);

	if (_MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();

		CMessage msgout("SDB:VALUE_SET");
		msgout.serial(shardId);
		msgout.serial(const_cast<string &>(path));
		msgout.serial(val);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	return true;
}

// ****************************************************************************
bool CStatDB::valueAdd(const std::string & path, sint32 val)
{
	nlassert(_SDBIsLoaded);

	CStatDBValueLeaf * valueLeaf = dynamic_cast<CStatDBValueLeaf *>(_Root->getNode(path).getPtr());
	if (valueLeaf == NULL)
		return false;

	valueLeaf->addValue(val);

	if (_MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();

		CMessage msgout("SDB:VALUE_ADD");
		msgout.serial(shardId);
		msgout.serial(const_cast<string &>(path));
		msgout.serial(val);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	return true;
}

// ****************************************************************************
bool CStatDB::valueGet(const std::string & path, sint32 & val)
{
	nlassert(_SDBIsLoaded);

	CStatDBValueLeaf * valueLeaf = dynamic_cast<CStatDBValueLeaf *>(_Root->getNode(path).getPtr());
	if (valueLeaf == NULL)
		return false;

	val = valueLeaf->getValue();
	return true;
}

// ****************************************************************************
bool CStatDB::createTable(const std::string & path)
{
	nlassert(_SDBIsLoaded);

	IStatDBNodePtr node = _Root->getNode(path);
	if (node != NULL)
		return false;

	return createTable(path, map<NLMISC::CEntityId,sint32>(), map<EGSPD::TGuildId,sint32>());
}

// ****************************************************************************
bool CStatDB::createTable(const std::string & path,
						  const std::map<NLMISC::CEntityId,sint32> & playerValues,
						  const std::map<EGSPD::TGuildId,sint32> & guildValues
						  )
{
	nlassert(_SDBIsLoaded);

	IStatDBNodePtr node = _Root->getNode(path);
	if (node != NULL)
		return false;

	bool res = _Root->setNode(path, new CStatDBTableLeaf(playerValues, guildValues));

	if (res && _MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();
		CStatDBTableLeafMsg tableLeafMsg;
		tableLeafMsg.Path = path;
		tableLeafMsg.PlayerValues = playerValues;
		tableLeafMsg.GuildValues = guildValues;

		CStatDBNamesMsg namesMsg;
		getNamesFromTable(tableLeafMsg, namesMsg);

		CMessage msgout("SDB:CREATE_TABLE");
		msgout.serial(shardId);
		msgout.serial(tableLeafMsg);
		msgout.serial(namesMsg);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	return res;
}

// ****************************************************************************
bool CStatDB::tablePlayerAdd(const std::string & path, NLMISC::CEntityId playerId, sint32 val)
{
	nlassert(_SDBIsLoaded);

	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	tableLeaf->playerAdd(playerId, val);

	if (_MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();
		string playerName;
		getPlayerName(playerId, playerName);

		CMessage msgout("SDB:TABLE_PLAYER_ADD");
		msgout.serial(shardId);
		msgout.serial(const_cast<string &>(path));
		msgout.serial(playerId);
		msgout.serial(playerName);
		msgout.serial(val);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	return true;
}

// ****************************************************************************
bool CStatDB::tablePlayerSet(const std::string & path, NLMISC::CEntityId playerId, sint32 val)
{
	nlassert(_SDBIsLoaded);

	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	tableLeaf->playerSet(playerId, val);

	if (_MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();
		string playerName;
		getPlayerName(playerId, playerName);

		CMessage msgout("SDB:TABLE_PLAYER_SET");
		msgout.serial(shardId);
		msgout.serial(const_cast<string &>(path));
		msgout.serial(playerId);
		msgout.serial(playerName);
		msgout.serial(val);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	return true;
}

// ****************************************************************************
bool CStatDB::tablePlayerGet(const std::string & path, NLMISC::CEntityId playerId, sint32& val)
{
	nlassert(_SDBIsLoaded);

	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	return tableLeaf->playerGet(playerId, val);
}

// ****************************************************************************
bool CStatDB::tableGuildAdd(const std::string & path, EGSPD::TGuildId guildId, sint32 val)
{
	nlassert(_SDBIsLoaded);

	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	tableLeaf->guildAdd(guildId, val);

	if (_MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();
		string guildName;
		getGuildName(guildId, guildName);

		CMessage msgout("SDB:TABLE_GUILD_ADD");
		msgout.serial(shardId);
		msgout.serial(const_cast<string &>(path));
		msgout.serial(guildId);
		msgout.serial(guildName);
		msgout.serial(val);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	return true;
}

// ****************************************************************************
bool CStatDB::tableGuildSet(const std::string & path, EGSPD::TGuildId guildId, sint32 val)
{
	nlassert(_SDBIsLoaded);

	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	tableLeaf->guildSet(guildId, val);

	if (_MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();
		string guildName;
		getGuildName(guildId, guildName);

		CMessage msgout("SDB:TABLE_GUILD_SET");
		msgout.serial(shardId);
		msgout.serial(const_cast<string &>(path));
		msgout.serial(guildId);
		msgout.serial(guildName);
		msgout.serial(val);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	return true;
}

// ****************************************************************************
bool CStatDB::tableGuildGet(const std::string & path, EGSPD::TGuildId guildId, sint32& val)
{
	nlassert(_SDBIsLoaded);

	CStatDBTableLeaf * tableLeaf = dynamic_cast<CStatDBTableLeaf *>(_Root->getNode(path).getPtr());
	if (tableLeaf == NULL)
		return false;

	return tableLeaf->guildGet(guildId, val);
}

// ****************************************************************************
class CStatDBBackupFileCleaner : private CStatDBNodeVisitor
{
public:
	void submitRemovedNode(IStatDBNodePtr removedNode, const std::string & removedNodePath, bool keepBackupOfFiles)
	{
		_KeepBackupOfFiles = keepBackupOfFiles;
		if (removedNode != NULL)
			removedNode->acceptVisitor(*this, removedNodePath);
	}

private:
	void visitTableLeaf(CStatDBTableLeaf * tableLeaf, const std::string & path)
	{
		string sFilePath = toString("sdb/table_leaf_%s_pdr.%s", path.c_str(), (XMLSave?"xml":"bin"));
		Bsi.deleteFile(sFilePath, _KeepBackupOfFiles);
	}

private:
	bool _KeepBackupOfFiles;
};

bool CStatDB::removeNode(const std::string & path, bool keepBackupOfFiles)
{
	nlassert(_SDBIsLoaded);

	IStatDBNodePtr removedNode = _Root->removeNode(path);
	bool res = (removedNode != NULL);

	if (res && _MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();

		CMessage msgout("SDB:REMOVE_NODE");
		msgout.serial(shardId);
		msgout.serial(const_cast<string &>(path));
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}

	if (res)
	{
		// remove save files associated to the removed node if any
		// only table leaves have their own save files
		CStatDBBackupFileCleaner backupFileCleaner;
		backupFileCleaner.submitRemovedNode(removedNode, path, keepBackupOfFiles);
	}

	return res;
}

// ****************************************************************************
void CStatDB::removePlayer(NLMISC::CEntityId playerId)
{
	_EntitiesRemoval.addPlayerToRemove(playerId);

	if (_MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();

		CMessage msgout("SDB:REMOVE_PLAYER");
		msgout.serial(shardId);
		msgout.serial(playerId);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}
}

// ****************************************************************************
void CStatDB::removeGuild(EGSPD::TGuildId guildId)
{
	_EntitiesRemoval.addGuildToRemove(guildId);

	if (_MFSIsInitialized)
	{
		uint32 shardId = IService::getInstance()->getShardId();

		CMessage msgout("SDB:REMOVE_GUILD");
		msgout.serial(shardId);
		msgout.serial(guildId);
		CUnifiedNetwork::getInstance()->send("MFS", msgout);
	}
}

// ****************************************************************************
bool CStatDB::displayNodes(const std::string & pathPattern, NLMISC::CLog & log, const CStatDBNodeDisplayer::CSettings & settings)
{
	vector<IStatDBNode::CMatchingNode> nodes;
	_Root->getNodes(pathPattern, nodes, "");

	if (nodes.empty())
		return false;

	CStatDBNodeDisplayer nodeDisplayer;
	nodeDisplayer.setSettings(settings);

	for (uint i = 0; i < nodes.size(); i++)
	{
		nodeDisplayer.displayNode(nodes[i].Node, nodes[i].Path, log);
	}

	return true;
}


uint32 nTotalLoaded = 0;

struct TValueLeaveFileCallback : public IBackupFileReceiveCallback
{
	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		CStatDB::getInstance()->valueLeaveFileCallback(fileDescription, dataStream);
	}
};

void  CStatDB::valueLeaveFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	if (!fileDescription.FileName.empty())
	{
		static CPersistentDataRecord	pdr;
		pdr.clear();
		CStatDBValueLeavesPD valueLeavesPD;

		pdr.fromBuffer(dataStream);
//		pdr.readFromFile(sFilePath.c_str());
		valueLeavesPD.apply(pdr);
		nTotalLoaded += fileDescription.FileSize;

		for (uint32 i = 0; i < valueLeavesPD.ValueLeavesPD.size(); ++i)
		{
			const CStatDBValueLeafPD & valueLeafPD = valueLeavesPD.ValueLeavesPD[i];

			IStatDBNodePtr node = _Root->getNode(valueLeafPD.Path);
			if (node == NULL)
			{
				bool res = _Root->setNode(valueLeafPD.Path, new CStatDBValueLeaf(valueLeafPD.Value));
				if (!res)
				{
					nlwarning("value leaf '%s' cannot be created!", valueLeafPD.Path.c_str());
					DEBUG_STOP;
				}
			}
			else
			{
				nlwarning("leaf '%s' already exists!", valueLeafPD.Path.c_str());
				DEBUG_STOP;
			}
		}
	}
}

vector<string>	fileNames;

struct TFileClassCallback : public IBackupFileClassReceiveCallback
{
	virtual void callback(const CFileDescriptionContainer& fileList)
	{
		for (uint i=0; i<fileList.size(); ++i)
		{
			fileNames.push_back(fileList[i].FileName);
		}
	}

};

struct TTableLeaveFileCallback : public IBackupFileReceiveCallback
{
	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		CStatDB::getInstance()->tableLeaveFileCallback(fileDescription, dataStream);
	}
};

void  CStatDB::tableLeaveFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	const string & fileName = CFile::getFilename(fileDescription.FileName);

	if (	CFile::getFilename(fileName).substr(0, 11) == "table_leaf_"
		&&	CFile::getExtension(fileName) == (XMLSave?"xml":"bin"))
	{
		H_AUTO(CStatDB_load_2);

		static CPersistentDataRecord	pdr;
		pdr.clear();
		CStatDBTableLeafPD tableLeafPD;

		pdr.fromBuffer(dataStream);
//		pdr.readFromFile(fileName.c_str());
		tableLeafPD.apply(pdr);
		nTotalLoaded += CFile::getFileSize(fileName);

		IStatDBNodePtr node = _Root->getNode(tableLeafPD.Path);
		if (node == NULL)
		{
			bool res = _Root->setNode(tableLeafPD.Path, new CStatDBTableLeaf(tableLeafPD.PlayerValues, tableLeafPD.GuildValues));
			if (!res)
			{
				nlwarning("table leaf '%s' cannot be created!", tableLeafPD.Path.c_str());
				DEBUG_STOP;
			}
		}
		else
		{
			nlwarning("leaf '%s' already exists!", tableLeafPD.Path.c_str());
			DEBUG_STOP;
		}
	}
}


// ****************************************************************************
void CStatDB::load()
{
	H_AUTO(CStatDB_load);

	if (EGSLight)
	{
		_SDBIsLoaded = true;
		return;
	}

	if (_SDBIsLoaded)
		return;

	nTotalLoaded = 0;

//	// create SDB path
//	{
//		string sPath = Bsi.getLocalPath() + "sdb";
//		if (!CFile::isExists(sPath))
//			CFile::createDirectory(sPath);
//	}
	
	// load value leaves
	{
		H_AUTO(CStatDB_load_1);

//		string sFilePath = Bsi.getLocalPath();
		string sFilePath = toString("sdb/value_leaves_pdr.%s", (XMLSave?"xml":"bin"));

		TValueLeaveFileCallback *cb = new TValueLeaveFileCallback;
		
		Bsi.syncLoadFile(sFilePath, cb);

//		if (CFile::isExists(sFilePath))
//		{
//			static CPersistentDataRecord	pdr;
//			pdr.clear();
//			CStatDBValueLeavesPD valueLeavesPD;
//
//			pdr.readFromFile(sFilePath.c_str());
//			valueLeavesPD.apply(pdr);
//			nTotalLoaded += CFile::getFileSize(sFilePath);
//
//			for (uint32 i = 0; i < valueLeavesPD.ValueLeavesPD.size(); ++i)
//			{
//				const CStatDBValueLeafPD & valueLeafPD = valueLeavesPD.ValueLeavesPD[i];
//
//				IStatDBNodePtr node = _Root->getNode(valueLeafPD.Path);
//				if (node == NULL)
//				{
//					bool res = _Root->setNode(valueLeafPD.Path, new CStatDBValueLeaf(valueLeafPD.Value));
//					if (!res)
//					{
//						nlwarning("value leaf '%s' cannot be created!", valueLeafPD.Path.c_str());
//						DEBUG_STOP;
//					}
//				}
//				else
//				{
//					nlwarning("leaf '%s' already exists!", valueLeafPD.Path.c_str());
//					DEBUG_STOP;
//				}
//			}
//		}
	}
	
	// load table leaves
//	string sdbSavePath = Bsi.getLocalPath() + "sdb";


	// get the file list
	vector<CBackupFileClass> fileClasses(1);
	fileClasses[0].Patterns.push_back(toString("table_leaf_*.%s", XMLSave ? "xml" : "bin"));
	TFileClassCallback *ccb = new TFileClassCallback;
	Bsi.syncLoadFileClass("sdb", fileClasses, ccb);

	// load the files
	TTableLeaveFileCallback *cb2 = new TTableLeaveFileCallback;
	Bsi.syncLoadFiles(fileNames, cb2);	

//	std::vector<std::string> files;
//	CPath::getPathContent(sdbSavePath, false, false, true, files);
//	for (uint i = 0; i < files.size(); i++)
//	{
//		const string & fileName = files[i];
//
//		if (	CFile::getFilename(fileName).substr(0, 11) == "table_leaf_"
//			&&	CFile::getExtension(fileName) == (XMLSave?"xml":"bin"))
//		{
//			H_AUTO(CStatDB_load_2);
//
//			static CPersistentDataRecord	pdr;
//			pdr.clear();
//			CStatDBTableLeafPD tableLeafPD;
//
//			pdr.readFromFile(fileName.c_str());
//			tableLeafPD.apply(pdr);
//			nTotalLoaded += CFile::getFileSize(fileName);
//
//			IStatDBNodePtr node = _Root->getNode(tableLeafPD.Path);
//			if (node == NULL)
//			{
//				bool res = _Root->setNode(tableLeafPD.Path, new CStatDBTableLeaf(tableLeafPD.PlayerValues, tableLeafPD.GuildValues));
//				if (!res)
//				{
//					nlwarning("table leaf '%s' cannot be created!", tableLeafPD.Path.c_str());
//					DEBUG_STOP;
//				}
//			}
//			else
//			{
//				nlwarning("leaf '%s' already exists!", tableLeafPD.Path.c_str());
//				DEBUG_STOP;
//			}
//		}
//	}

	nlinfo("SDB: loaded %u bytes", nTotalLoaded);

	_SDBIsLoaded = true;
	if (canInitMFS())
		initMFS();
}

// ****************************************************************************
bool CStatDB::canInitMFS() const
{
	return (_SDBIsLoaded && _GuildsAreLoaded && _MFSIsUp);
}

// ****************************************************************************
void CStatDB::initMFS()
{
	H_AUTO(CStatDB_initMFS);

	nlassert(canInitMFS());

	if (_MFSIsInitialized)
		return;

	uint32 shardId = IService::getInstance()->getShardId();
	if ( shardId == DEFAULT_SHARD_ID )
	{
#ifdef NL_OS_WINDOWS
		nlwarning
#else
		nlerror
#endif
		( "SDB: Sending default shard id (%u) to MFS", DEFAULT_SHARD_ID );
	}
	CStatDBAllLeavesMsg allLeavesMsg;
	CStatDBMFSInitLeafCollector().loadLeaves(_Root, allLeavesMsg);

	CMessage msgout("SDB:INIT");
	msgout.serial(shardId);
	msgout.serial(allLeavesMsg);

	nlinfo("SDB: initMFS: send %u bytes to MFS", msgout.length());

	CUnifiedNetwork::getInstance()->send("MFS", msgout);

	_MFSIsInitialized = true;
}

// ****************************************************************************
void CStatDB::cbMFServiceUp()
{
	_MFSIsUp = true;
	if (canInitMFS())
		initMFS();
}

// ****************************************************************************
void CStatDB::cbMFServiceDown()
{
	_MFSIsUp = false;
	_MFSIsInitialized = false;
}

// ****************************************************************************
void CStatDB::cbGuildsLoaded()
{
	_GuildsAreLoaded = true;
	if (canInitMFS())
		initMFS();
}

// ****************************************************************************
void CStatDB::tickUpdate()
{
	H_AUTO(CStatDB_tickUpdate);
	
	if (!_SDBIsLoaded)
		return;

	// process players and guilds removal at every ticks
	{
		H_AUTO(CStatDB_tickUpdate_1);
		_EntitiesRemoval.processRemoval(_Root);
	}

	// save SDB at every StatDBSavePeriod ticks
	if (CTickEventHandler::getGameCycle() % StatDBSavePeriod.get() != 0)
		return;

	if (_BackupLeafCollector.isEmpty())
	{
		H_AUTO(CStatDB_tickUpdate_2);
		_BackupLeafCollector.loadLeaves(_Root);

		// save value leaves file
		// even if there is no value leaf because a complete database erase must be saved
		saveValueLeaves(_BackupLeafCollector.getValueLeavesPD());
		_BackupLeafCollector.getValueLeavesPD().ValueLeavesPD.clear();
	}
	else
	{
		CStatDBTableLeafPD tableLeafPD;
		if (_BackupLeafCollector.popTableLeafPD(tableLeafPD))
		{
			H_AUTO(CStatDB_tickUpdate_4);
			saveTableLeaf(tableLeafPD);
		}
	}
}

// ****************************************************************************
void CStatDB::saveAll()
{
	_BackupLeafCollector.loadLeaves(_Root);

	saveValueLeaves(_BackupLeafCollector.getValueLeavesPD());
	_BackupLeafCollector.getValueLeavesPD().ValueLeavesPD.clear();

	CStatDBTableLeafPD tableLeafPD;
	while (_BackupLeafCollector.popTableLeafPD(tableLeafPD))
	{
		saveTableLeaf(tableLeafPD);
	}
}

// ****************************************************************************
void CStatDB::saveValueLeaves(const CStatDBValueLeavesPD & valueLeavesPD)
{
	string sFilePath = toString("sdb/value_leaves_pdr.%s", (XMLSave?"xml":"bin"));

	static CPersistentDataRecordRyzomStore	pdr;
	pdr.clear();
	valueLeavesPD.store(pdr);

	CBackupMsgSaveFile msg( sFilePath, CBackupMsgSaveFile::SaveFile, Bsi );
	if (XMLSave)
	{
		string s;
		pdr.toString(s);
		msg.DataMsg.serialBuffer((uint8*)&s[0], (uint)s.size());
	}
	else
	{
		uint size = pdr.totalDataSize();
		vector<char> buffer(size);
		pdr.toBuffer(&buffer[0], size);
		msg.DataMsg.serialBuffer((uint8*)&buffer[0], size);
	}

//	nlinfo("saveValueLeaves send %u bytes to BS", msgout.length());
	Bsi.sendFile( msg );
}

// ****************************************************************************
void CStatDB::saveTableLeaf(const CStatDBTableLeafPD & tableLeafPD)
{
	string sFilePath = toString("sdb/table_leaf_%s_pdr.%s", tableLeafPD.Path.c_str(), (XMLSave?"xml":"bin"));

	static CPersistentDataRecordRyzomStore	pdr;
	pdr.clear();
	tableLeafPD.store(pdr);

	CBackupMsgSaveFile msg( sFilePath, CBackupMsgSaveFile::SaveFile, Bsi );
	if (XMLSave)
	{
		string s;
		pdr.toString(s);
		msg.DataMsg.serialBuffer((uint8*)&s[0], (uint)s.size());
	}
	else
	{
		uint size = pdr.totalDataSize();
		vector<char> buffer(size);
		pdr.toBuffer(&buffer[0], size);
		msg.DataMsg.serialBuffer((uint8*)&buffer[0], size);
	}
	
//	nlinfo("saveTableLeaf(%s) send %u bytes to BS", tableLeafPD.Path.c_str(), msgout.length());
	Bsi.sendFile( msg );
}

// ****************************************************************************
// Commands
// ****************************************************************************

// ****************************************************************************
NLMISC_COMMAND (sdbCreateValue, "create a value leaf in SDB", "<path> [<value>]")
{
	if (args.size() < 1 || args.size() > 2)
		return false;

	const string & path = args[0];
	sint32 val;
	if (args.size() < 2)
		val = 0;
	else
		NLMISC::fromString(args[1], val);

	if (!CStatDB::getInstance()->createValue(path, val))
	{
		log.displayNL("cannot create a value leaf at the path '%s' (invalid path or already existing node)",
			path.c_str());
	}

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbCreateTable, "create a table leaf in SDB", "<path>")
{
	if (args.size() != 1)
		return false;

	const string & path = args[0];
	if (!CStatDB::getInstance()->createTable(path))
	{
		log.displayNL("cannot create a table leaf at the path '%s' (invalid path or already existing node)",
			path.c_str());
	}

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbRemoveNode, "remove a node from SDB", "<path>")
{
	if (args.size() != 1)
		return false;

	const string & path = args[0];
	if (!CStatDB::getInstance()->removeNode(path))
	{
		log.displayNL("path '%s' not found", path.c_str());
	}

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbValueSet, "set a value leaf in SDB", "<path> <value>")
{
	if (args.size() != 2)
		return false;

	const string & path = args[0];
	sint32 val;
	NLMISC::fromString(args[1], val);

	if (!CStatDB::getInstance()->valueSet(path, val))
	{
		log.displayNL("cannot find a value leaf at the path '%s'", path.c_str());
	}

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbValueAdd, "add a value to a value leaf in SDB", "<path> <value>")
{
	if (args.size() != 2)
		return false;

	const string & path = args[0];
	sint32 val;
	NLMISC::fromString(args[1], val);

	if (!CStatDB::getInstance()->valueAdd(path, val))
	{
		log.displayNL("cannot find a value leaf at the path '%s'", path.c_str());
	}

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbTableAdd, "add a value to a table leaf in SDB", "<path> <value> <target=player|guild> <player_id|guild_name>")
{
	if (args.size() != 4)
		return false;

	const string & path = args[0];
	sint32 val;
	NLMISC::fromString(args[1], val);
	const string & targetType = args[2];

	if (targetType == "guild")
	{
		CGuild * guild = CGuildManager::getInstance()->getGuildByName(args[3]);
		if (guild == NULL)
		{
			log.displayNL("unknown guild: '%s'", args[3].c_str());
			return true;
		}

		if (!CStatDB::getInstance()->tableGuildAdd(path, guild->getId(), val))
		{
			log.displayNL("cannot find a table leaf at the path '%s'", path.c_str());
		}
	}
	else
	{
		CEntityId playerId;
		playerId.fromString(args[3].c_str());

		if (playerId.getType() != RYZOMID::player)
		{
			log.displayNL("id %s is not a player id", playerId.toString().c_str());
			return true;
		}

		if (!CEntityIdTranslator::getInstance()->isEntityRegistered(playerId))
		{
			log.displayNL("player id %s is unknown", playerId.toString().c_str());
			return true;
		}

		if (!CStatDB::getInstance()->tablePlayerAdd(path, playerId, val))
		{
			log.displayNL("cannot find a table leaf at the path '%s'", path.c_str());
		}
	}

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbRemovePlayer, "remove a player from the whole SDB", "<player_id>")
{
	if (args.size() != 1)
		return false;

	CEntityId playerId;
	playerId.fromString(args[0].c_str());

	CStatDB::getInstance()->removePlayer(playerId);

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbRemoveGuild, "remove a guild from the whole SDB", "<guild_name>")
{
	if (args.size() != 1)
		return false;

	CGuild * guild = CGuildManager::getInstance()->getGuildByName(args[0]);
	if (guild == NULL)
	{
		log.displayNL("unknown guild: '%s'", args[0].c_str());
		return true;
	}

	CStatDB::getInstance()->removeGuild(guild->getId());

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbDisplayNodes, "display nodes of SDB", "<path> [<recursive>] [<display_values>] [<display_tables>]")
{
	if (args.size() < 1 || args.size() > 4)
		return false;

	const string & pathPattern = args[0];

	CStatDBNodeDisplayer::CSettings settings;

	if (args.size() >= 2)
		settings.Recursive = (args[1] == "1" || args[1] == "true");

	if (args.size() >= 3)
		settings.DisplayValueLeafContent = (args[2] == "1" || args[2] == "true");

	if (args.size() >= 4)
		settings.DisplayTableLeafContent = (args[3] == "1" || args[3] == "true");

	settings.DisplayBranch = !settings.Recursive;

	if (!CStatDB::getInstance()->displayNodes(pathPattern, log, settings))
	{
		log.displayNL("path '%s' not found", pathPattern.c_str());
	}

	return true;
}

// ****************************************************************************
NLMISC_COMMAND (sdbSaveNow, "save the whole SDB now (WARNING: it may stall the shard and flood the Backup Service)", "")
{
	if (args.size() != 0)
		return false;

	CStatDB::getInstance()->saveAll();

	return true;
}


#if !FINAL_VERSION

extern NLMISC::CRandom RandomGenerator;
// ****************************************************************************
NLMISC_COMMAND (sdbInitEpisode2, "(debug) init fake database for Episode2 tests", "")
{
	// Episode II init
	// Leaf simple pour le harvest
	const char *peuple[] = { "fyros", "matis", "tryker", "zorai" };
	uint peupleNB = sizeof(peuple)/sizeof(peuple[0]);
	const char *faction[] = { "kami", "karavan" };
	uint factionNB = sizeof(faction)/sizeof(faction[0]);
	const char *mp[] = { "carapace_a", "resine_a", "bois_a", "fibre_o","resine_o", "ecorce_o", 
					"carapace_i", "resine_i", "boucle_i", "bois_i" };
	uint mpNB = sizeof(mp)/sizeof(mp[0]);
	const char *mpByFaction[] = { "seve", "amber" };
	uint mpByFactionNB = sizeof(mpByFaction)/sizeof(mpByFaction[0]);
	
	for (uint i = 0; i < peupleNB; ++i)
	for (uint j = 0; j < factionNB; ++j)
	{
		string sPath = "storyline.episode2.";
		sPath += toString(peuple[i]) + ".";
		sPath += toString(faction[j]) + ".";
		for (uint k = 0; k < mpNB; ++k)
		{
			string sTmpPath = sPath + mp[k];
			CStatDB::getInstance()->createValue(sTmpPath+toString(".qtemin"), RandomGenerator.rand(500));
			CStatDB::getInstance()->createValue(sTmpPath+toString(".qtemax"), RandomGenerator.rand(500));
		}
		CStatDB::getInstance()->createValue(sPath+mpByFaction[j]+toString(".qtemin"), RandomGenerator.rand(500));
		CStatDB::getInstance()->createValue(sPath+mpByFaction[j]+toString(".qtemax"), RandomGenerator.rand(500));
	}

	// Leaf simple pour le craft
	const char *craft[] = { "socle", "colonne", "comble", "muraille", "revetement", "ornement", "statue",
		"colonne_justice", "racine", "tronc", "fibre", "ecorce", "feuille", "fleur", "symbole", "noyau" };
	uint craftNB = sizeof(craft)/sizeof(craft[0]);
	for (uint i = 0; i < peupleNB; ++i)
	{
		for (uint j = 0; j < craftNB; ++j)
		{
			string sPath = "storyline.episode2.";
			sPath += toString(peuple[i]) + ".";
			sPath += toString(craft[j]);
			CStatDB::getInstance()->createValue(sPath+toString(".qtemin"), RandomGenerator.rand(500));
			CStatDB::getInstance()->createValue(sPath+toString(".qtemax"), RandomGenerator.rand(500));
		}
	}

	// Leaf simple pour les maxima
	const char *max_values[] = { "socle_max", "colonne_max", "comble_max", "muraille_max", "revetement_max",
		"ornement_max", "statue_max", "colonne_justice_max", "racine_max", "tronc_max", "fibre_max",
		"ecorce_max", "feuille_max", "fleur_max", "symbole_max", "noyau_max" };
	uint max_valuesNB = sizeof(max_values)/sizeof(max_values[0]);
	for (uint i = 0; i < peupleNB; ++i)
	{
		for (uint j = 0; j < max_valuesNB; ++j)
		{
			string sPath = "storyline.episode2.";
			sPath += toString(peuple[i]) + ".";
			sPath += toString(max_values[j]);
			CStatDB::getInstance()->createValue(sPath, 500 + RandomGenerator.rand(500));
		}
	}

	// Tableaux de joueurs
	const char *action[] = { "craft", "harvest", "kill" };
	uint actionNB = sizeof(action)/sizeof(action[0]);
	const char *acte[] = { "acte1", "acte2", "acte3" };
	uint acteNB = sizeof(acte)/sizeof(acte[0]);
	for (uint i = 0; i < peupleNB; ++i)
	for (uint j = 0; j < factionNB; ++j)
	for (uint k = 0; k < actionNB; ++k)
	for (uint m = 0; m < acteNB; ++m)
	{
		string sPath = "storyline.episode2.";
		sPath += toString(peuple[i]) + ".";
		sPath += toString(faction[j]) + ".";
		sPath += toString(action[k]) + ".";
		sPath += toString(acte[m]);

		CStatDB::getInstance()->createTable(sPath);

		uint nbjoueur = 100+RandomGenerator.rand(400);
		for (uint n = 0; n < nbjoueur; ++n)
		{
			CEntityId playerId;
			playerId.setShortId(RandomGenerator.rand(5000) << 4);
			playerId.setType(RYZOMID::player);
			CStatDB::getInstance()->tablePlayerAdd(sPath, playerId, 10 * RandomGenerator.rand(5000));
		}

		uint nbguild = 250+RandomGenerator.rand(250);
		for (uint n = 0; n < nbguild; ++n)
		{

			EGSPD::TGuildId guildId = uint32(RandomGenerator.rand(20500));
			CStatDB::getInstance()->tableGuildAdd(sPath, guildId, 100 * RandomGenerator.rand(10000));
		}
	}

	return true;
}

#endif // !FINAL_VERSION

