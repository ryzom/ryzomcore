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



#ifndef RY_SHARD_STAT_DB_MANAGER_H
#define RY_SHARD_STAT_DB_MANAGER_H


class CShardStatDB;
typedef NLMISC::CSmartPtr<CShardStatDB> CShardStatDBPtr;

/**
 * This class manages player and guild names for SDB
 * 
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005 July
 */
class CStatDBNameManager
{
public:
	void setPlayerName(NLMISC::CEntityId playerId, const std::string & playerName);
	void setGuildName(EGSPD::TGuildId guildId, const std::string & guildName);

	bool getPlayerName(NLMISC::CEntityId playerId, std::string & playerName) const;
	bool getGuildName(EGSPD::TGuildId guildId, std::string & guildName) const;

	void removePlayerName(NLMISC::CEntityId playerId);
	void removeGuildName(EGSPD::TGuildId guildId);

	void loadNames(const CStatDBNamesMsg & namesMsg);

private:
	std::map<NLMISC::CEntityId,std::string>	_PlayerNames;
	std::map<EGSPD::TGuildId,std::string>	_GuildNames;
};

/**
 * This class displays a node of SDB
 * 
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005 July
 */
class CStatDBNodeDisplayer : private CStatDBNodeVisitor
{
public:
	struct CSettings
	{
		CSettings() { setDefaults(); }

		void setDefaults()
		{
			Recursive				= true;
			DisplayBranch			= true;
			DisplayValueLeaf		= true;
			DisplayValueLeafContent	= true;
			DisplayTableLeaf		= true;
			DisplayTableLeafContent	= true;
		}

		bool Recursive					: 1;
		bool DisplayBranch				: 1;
		bool DisplayValueLeaf			: 1;
		bool DisplayValueLeafContent	: 1;
		bool DisplayTableLeaf			: 1;
		bool DisplayTableLeafContent	: 1;
	};

public:
	CStatDBNodeDisplayer() : _Log(NULL) {}

	void setSettings(const CSettings & settings) { _Settings = settings; }
	const CSettings & getSettings() const { return _Settings; }

	void displayNode(IStatDBNodePtr node, const std::string & currentPath, NLMISC::CLog & log, const CStatDBNameManager & nameManager);

private:
	void visitBranch(CStatDBBranch * branch, const std::string & path);
	void visitValueLeaf(CStatDBValueLeaf * valueLeaf, const std::string & path);
	void visitTableLeaf(CStatDBTableLeaf * tableLeaf, const std::string & path);

	void displayOneNode(IStatDBNodePtr node, const std::string & currentPath);

private:
	NLMISC::CLog *				_Log;
	const CStatDBNameManager *	_NameManager;
	CSettings					_Settings;
};

/**
 * This class provides a read-only access to a shard SDB copy
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005 July
 */
class CShardStatDBReader
{
public:
	typedef std::map<NLMISC::CEntityId,sint32>	TPlayerValues;
	typedef std::map<EGSPD::TGuildId,sint32>	TGuildValues;

public:
	CShardStatDBReader(CShardStatDBPtr statDB);

	uint32 getShardId() const;

	bool getValue(const std::string & path, sint32 & val) const;
	bool getValues(const std::string & pathPattern, std::vector<sint32> & values) const;

	/// NOTE: use this as an accessor to table (DO NOT keep pointers)
	bool getTable(const std::string & path,
		const TPlayerValues * & playerValues,
		const TGuildValues * & guildValues) const;

	/// NOTE: use this as an accessor to tables (DO NOT keep pointers)
	bool getTables(const std::string & pathPattern,
		std::vector<const TPlayerValues *> & playerValuesVec,
		std::vector<const TGuildValues *> & guildValuesVec) const;

	bool getPlayerName(NLMISC::CEntityId playerId, std::string & playerName) const;
	bool getGuildName(EGSPD::TGuildId guildId, std::string & guildName) const;

private:
	CShardStatDBPtr _StatDB;
};

/**
 * A shard SDB copy
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005 July
 */
class CShardStatDB : public NLMISC::CRefCount, public CStatDBNameManager
{
	friend class CShardStatDBReader;

public:
	CShardStatDB(uint32 shardId);

	uint32 getShardId() const { return _ShardId; }

	CShardStatDBReader getReader() const { return CShardStatDBReader(const_cast<CShardStatDB *>(this)); }

	void resetStatDB();

	bool createValue(const std::string & path, sint32 val);
	bool valueSet(const std::string & path, sint32 val);
	bool valueAdd(const std::string & path, sint32 val);

	bool createTable(const std::string & path,
		const std::map<NLMISC::CEntityId,sint32> & playerValues,
		const std::map<EGSPD::TGuildId,sint32> & guildValues
		);
	bool tablePlayerAdd(const std::string & path, NLMISC::CEntityId playerId, sint32 val);
	bool tablePlayerSet(const std::string & path, NLMISC::CEntityId playerId, sint32 val);
	bool tableGuildAdd(const std::string & path, EGSPD::TGuildId guildId, sint32 val);
	bool tableGuildSet(const std::string & path, EGSPD::TGuildId guildId, sint32 val);

	bool removeNode(const std::string & path);

	void removePlayer(NLMISC::CEntityId playerId);
	void removeGuild(EGSPD::TGuildId guildId);

	bool displayNodes(const std::string & pathPattern, NLMISC::CLog & log, const CStatDBNodeDisplayer::CSettings & settings);

private:
	const uint32	_ShardId;
	IStatDBNodePtr	_Root;
};

/**
 * This manager keeps copies of shard statistical databases up-to-date
 * and furnishes a read-only access to the shard SDB copies
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005 July
 */
class CShardStatDBManager
{
	NLMISC_COMMAND_FRIEND(sdbDisplayNodes);

public:
	static void init();

	/// get the singleton instance
	static CShardStatDBManager * getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CShardStatDBManager;
		return _Instance;
	}

	/// get readers on all available shard SDB
	void getShardStatDBReaders(std::vector<CShardStatDBReader> & statDBReaders) const;

private:
	CShardStatDBManager();

	void initShardStatDB(uint32 shardId, const CStatDBAllLeavesMsg & allLeavesMsg);
	CShardStatDBPtr getShardStatDB(uint32 shardId);

	///\name Callbacks of messages sent by shards to keep shard SDB up-to-date
	//@{
	static void cbInit(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbCreateValue(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbCreateTable(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbValueSet(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbValueAdd(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbTablePlayerAdd(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbTablePlayerSet(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbTableGuildAdd(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbTableGuildSet(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbRemoveNode(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbRemovePlayer(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	static void cbRemoveGuild(NLNET::CMessage & msgin, const std::string & serviceName, NLNET::TServiceId serviceId);
	//@}

private:
	/// singleton instance
	static CShardStatDBManager * _Instance;

	/// copies of shard SDB
	std::vector<CShardStatDBPtr> _ShardStatDB;
};


#endif // RY_SHARD_STAT_DB_MANAGER_H
