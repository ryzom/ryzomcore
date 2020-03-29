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



#ifndef RY_STAT_DB_TREE_H
#define RY_STAT_DB_TREE_H

#include "nel/misc/smart_ptr.h"
#include "nel/misc/static_map.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/log.h"

#include "stat_db_common.h"
#include "stat_db_tree_visitor.h"


class IStatDBNode;
typedef NLMISC::CSmartPtr<IStatDBNode> IStatDBNodePtr;


/**
 * Node structure of the statistical database,
 * this is the base for branches and leaves.
 *
 * A path is a string composed of keys separated with a '.'
 * A key is an alphanumeric string: [0-9A-Za-z_]*
 * Example of path: "branch1_name.branch2_name.leaf1"
 *
 * A path pattern can contain wildcard keys:
 * Wildcard key '*' means any key
 *
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date 2005 July
 */
class IStatDBNode : public NLMISC::CRefCount
{
public:
	struct CMatchingNode
	{
		std::string		Path;
		IStatDBNodePtr	Node;
	};

public:
	/// virtual dtor
	virtual ~IStatDBNode() {}

	/// add a node at the given path, it creates the path if necessary
	/// NOTE: if another node is already at the given path it is replaced
	virtual bool setNode(const std::string & path, IStatDBNodePtr node) = 0;

	/// get the node at the given path or NULL if the path does not exist
	virtual IStatDBNodePtr getNode(const std::string & path) = 0;

	/// get all nodes whose path matches the given pattern
	/// \param pathPattern : a path pattern
	/// \param matchingNodes : return the matching nodes
	/// \param currentPath : the current path
	/// WARNING: the vector 'matchingNodes' will not be cleared by this method before matching nodes are added
	virtual void getNodes(const std::string & pathPattern, std::vector<CMatchingNode> & matchingNodes,
		const std::string & currentPath) = 0;

	/// remove and return the node at the given path
	virtual IStatDBNodePtr removeNode(const std::string & path) = 0;

	/// accept a visitor (visitor design pattern)
	/// \param currentPath : the path of this node
	virtual void acceptVisitor(CStatDBNodeVisitor & visitor, const std::string & currentPath) = 0;
};

/**
 * Leaf structure of the statistical database.
 * 
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date 2005 July
 */
class CStatDBLeaf : public IStatDBNode
{
public:
	virtual ~CStatDBLeaf() {}

	bool setNode(const std::string & /* path */, IStatDBNodePtr  /* node */) { return false; }
	IStatDBNodePtr getNode(const std::string & /* path */) { return NULL; }
	void getNodes(const std::string & /* pathPattern */, std::vector<CMatchingNode> & /* matchingNodes */,
		const std::string & /* currentPath */) {}
	IStatDBNodePtr removeNode(const std::string & /* path */) { return NULL; }
	
	virtual void acceptVisitor(CStatDBNodeVisitor & /* visitor */, const std::string & /* currentPath */) {}
};

/**
 * Simple value leaf structure of the statistical database
 * 
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date 2005 July
 */
class CStatDBValueLeaf : public CStatDBLeaf
{
public:
	/// ctor
	CStatDBValueLeaf(sint32 val = 0) : _Value(val) {}

	/// set value
	void setValue(sint32 val) { _Value = val; }
	/// get value
	sint32 getValue() { return _Value; }

	/// add value
	void addValue(sint32 val) { _Value += val; }

	void acceptVisitor(CStatDBNodeVisitor & visitor, const std::string & currentPath)
	{
		visitor.visitValueLeaf(this, currentPath);
	}

private:
	sint32	_Value;
};

/**
 * Leaf structure of the statistical database retaining info for players and guilds.
 * For the moment a table leaf removes entries with a value <= 0 (cf playerAdd() and guildAdd() methods).
 * It is typically made to store some positive scores,
 * but it may support both signed and unsigned scores in the future.
 * 
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date 2005 July
 */
class CStatDBTableLeaf : public CStatDBLeaf
{
public:
	typedef std::map<NLMISC::CEntityId,sint32>	TPlayerValues;
	typedef std::map<EGSPD::TGuildId,sint32>	TGuildValues;

public:
	/// ctor
	CStatDBTableLeaf() {}
	CStatDBTableLeaf(const TPlayerValues & playerValues, const TGuildValues & guildValues)
		: _PlayerValues(playerValues), _GuildValues(guildValues)
	{
	}

	/// add a value to a player
	/// NOTE: if the new value of the player is <= 0 the player entry is removed
	void playerAdd(NLMISC::CEntityId playerId, sint32 val);
	/// add a value to a guild
	/// NOTE: if the new value of the guild is <= 0 the guild entry is removed
	void guildAdd(EGSPD::TGuildId guildId, sint32 val);

	/// set a player value
	/// NOTE: if the new value of the player is <= 0 the player entry is removed
	void playerSet(NLMISC::CEntityId playerId, sint32 val);
	/// set a guild value
	/// NOTE: if the new value of the guild is <= 0 the guild entry is removed
	void guildSet(EGSPD::TGuildId guildId, sint32 val);

	/// get a value of a player
	bool playerGet(NLMISC::CEntityId playerId, sint32 & val) const;
	/// get a value of a guild
	bool guildGet(EGSPD::TGuildId guildId, sint32 & val) const;

	/// get player values
	const TPlayerValues & getPlayerValues() const { return _PlayerValues; }
	/// get guild values
	const TGuildValues & getGuildValues() const { return _GuildValues; }

	/// remove a player from the table
	void removePlayer(NLMISC::CEntityId playerId);
	/// remove a guild from the table
	void removeGuild(EGSPD::TGuildId guildId);

	void acceptVisitor(CStatDBNodeVisitor & visitor, const std::string & currentPath)
	{
		visitor.visitTableLeaf(this, currentPath);
	}

private:
	TPlayerValues	_PlayerValues;
	TGuildValues	_GuildValues;
};

/**
 * Branch structure of the statistical database.
 * We use a static map for branches because we will not add branches all days its a
 * 'created once read/write multiple' type of structure.
 *
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date 2005 July
 */
class CStatDBBranch : public IStatDBNode
{
public:
	bool setNode(const std::string & path, IStatDBNodePtr node);
	IStatDBNodePtr getNode(const std::string & path);
	void getNodes(const std::string & pathPattern, std::vector<CMatchingNode> & matchingNodes,
		const std::string & currentPath);
	IStatDBNodePtr removeNode(const std::string & path);

	void acceptVisitor(CStatDBNodeVisitor & visitor, const std::string & currentPath);

private:
	bool isValidToken(const std::string & token) const;
	void splitPath(const std::string & path, std::string & token, std::string & rest) const;

private:
	typedef NLMISC::CStaticMap<std::string, IStatDBNodePtr> TChildren;
	TChildren _Children;
};

/**
 * This class removes entities (players and guilds) through a SDB node
 * 
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005 July
 */
class CStatDBEntitiesRemoval : private CStatDBNodeVisitor
{
public:
	/// queue a player to remove
	void addPlayerToRemove(NLMISC::CEntityId playerId);
	/// queue a guild to remove
	void addGuildToRemove(EGSPD::TGuildId guildId);

	/// process removal of players and guilds actually queued and flush them
	void processRemoval(IStatDBNodePtr root);

private:
	void visitTableLeaf(CStatDBTableLeaf * tableLeaf, const std::string & path);

private:
	std::vector<NLMISC::CEntityId>	_PlayersToRemove;
	std::vector<EGSPD::TGuildId>	_GuildsToRemove;
};


#endif // RY_STAT_DB_TREE_H
