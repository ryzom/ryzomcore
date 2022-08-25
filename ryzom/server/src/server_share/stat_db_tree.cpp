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
#include "stat_db_tree.h"
#include "game_share/utils.h"

using namespace std;
using namespace NLMISC;


// ****************************************************************************
// CStatDBTableLeaf
// ****************************************************************************

// ****************************************************************************
void CStatDBTableLeaf::playerAdd(NLMISC::CEntityId playerId, sint32 val)
{
	TPlayerValues::iterator it = _PlayerValues.find(playerId);
	if (it != _PlayerValues.end())
	{
		sint32 & playerValue = (*it).second;

		// avoid overflow
		if (val > 0 && (playerValue + val) < playerValue)
		{
			playerValue = 0x7FFFFFFF;
			return;
		}

		// add the value and remove the entry if the result is <= 0
		playerValue += val;
		if (playerValue <= 0)
		{
			_PlayerValues.erase(it);
		}
	}
	else
	{
		if (val > 0)
		{
			_PlayerValues[playerId] = val;
		}
	}
}

// ****************************************************************************
void CStatDBTableLeaf::playerSet(NLMISC::CEntityId playerId, sint32 val)
{
	TPlayerValues::iterator it = _PlayerValues.find(playerId);
	if (it != _PlayerValues.end())
	{
		sint32 & playerValue = (*it).second;

		// set the value and remove the entry if the result is <= 0
		if (val > 0)
		{
			playerValue = val;
		}
		else
		{
			_PlayerValues.erase(it);
		}
	}
	else
	{
		if (val > 0)
		{
			_PlayerValues[playerId] = val;
		}
	}
}

// ****************************************************************************
void CStatDBTableLeaf::guildAdd(EGSPD::TGuildId guildId, sint32 val)
{
	TGuildValues::iterator it = _GuildValues.find(guildId);
	if (it != _GuildValues.end())
	{
		sint32 & guildValue = (*it).second;

		// avoid overflow
		if (val > 0 && (guildValue + val) < guildValue)
		{
			guildValue = 0x7FFFFFFF;
			return;
		}

		// add the value and remove the entry if the result is <= 0
		guildValue += val;
		if (guildValue <= 0)
		{
			_GuildValues.erase(it);
		}
	}
	else
	{
		if (val > 0)
		{
			_GuildValues[guildId] = val;
		}
	}
}

// ****************************************************************************
void CStatDBTableLeaf::guildSet(EGSPD::TGuildId guildId, sint32 val)
{
	TGuildValues::iterator it = _GuildValues.find(guildId);
	if (it != _GuildValues.end())
	{
		sint32 & guildValue = (*it).second;

		// set the value and remove the entry if the result is <= 0
		if (val > 0)
		{
			guildValue = val;
		}
		else
		{
			_GuildValues.erase(it);
		}
	}
	else
	{
		if (val > 0)
		{
			_GuildValues[guildId] = val;
		}
	}
}

// ****************************************************************************
bool CStatDBTableLeaf::playerGet(NLMISC::CEntityId playerId, sint32 & val) const
{
	TPlayerValues::const_iterator it = _PlayerValues.find(playerId);
	if (it == _PlayerValues.end())
		return false;

	val = (*it).second;
	return true;
}

// ****************************************************************************
bool CStatDBTableLeaf::guildGet(EGSPD::TGuildId guildId, sint32 & val) const
{
	TGuildValues::const_iterator it = _GuildValues.find(guildId);
	if (it == _GuildValues.end())
		return false;

	val = (*it).second;
	return true;
}

// ****************************************************************************
void CStatDBTableLeaf::removePlayer(NLMISC::CEntityId playerId)
{
	TPlayerValues::iterator it = _PlayerValues.find(playerId);
	if (it != _PlayerValues.end())
		_PlayerValues.erase(it);
}

// ****************************************************************************
void CStatDBTableLeaf::removeGuild(EGSPD::TGuildId guildId)
{
	TGuildValues::iterator it = _GuildValues.find(guildId);
	if (it != _GuildValues.end())
		_GuildValues.erase(it);
}

// ****************************************************************************
// CStatDBBranch
// ****************************************************************************

// ****************************************************************************
bool CStatDBBranch::setNode(const std::string & path, IStatDBNodePtr node)
{
	BOMB_IF(path.empty(), "empty path!", return false);
	BOMB_IF(node == NULL, "NULL node!", return false);

	string token;
	string rest;
	splitPath(path, token, rest);

	if (!isValidToken(token))
		return false;

	if (rest.empty())
	{
		_Children.add(make_pair(token, node));
		return true;
	}

	IStatDBNodePtr nextNode;
	TChildren::iterator it = _Children.find(token);
	if (it == _Children.end())
	{
		nextNode = new CStatDBBranch;
		_Children.add(make_pair(token, nextNode));
	}
	else
	{
		nextNode = (*it).second;
	}

	return nextNode->setNode(rest, node);
}

// ****************************************************************************
IStatDBNodePtr CStatDBBranch::getNode(const std::string & path)
{
	BOMB_IF(path.empty(), "empty path!", return NULL);

	string token;
	string rest;
	splitPath(path, token, rest);

	TChildren::iterator it = _Children.find(token);
	if (it == _Children.end())
		return NULL;

	if (rest.empty())
		return (*it).second;

	return (*it).second->getNode(rest);
}

// ****************************************************************************
void CStatDBBranch::getNodes(const std::string & pathPattern, std::vector<CMatchingNode> & matchingNodes,
							 const std::string & currentPath)
{
	BOMB_IF(pathPattern.empty(), "empty path!", return);

	// DO NOT clear the vector 'matchingNodes' here because this method is recursive

	string token;
	string rest;
	splitPath(pathPattern, token, rest);

	string pathPrefix = currentPath;
	if (!pathPrefix.empty())
		pathPrefix += ".";

	// check wildcard
	if (token == "*")
	{
		if (rest.empty())
		{
			for (TChildren::iterator it = _Children.begin(); it != _Children.end(); ++it)
			{
				CMatchingNode matchingNode;
				matchingNode.Path = pathPrefix + (*it).first;
				matchingNode.Node = (*it).second;
				matchingNodes.push_back(matchingNode);
			}
		}
		else
		{
			for (TChildren::iterator it = _Children.begin(); it != _Children.end(); ++it)
			{
				(*it).second->getNodes(rest, matchingNodes, pathPrefix + (*it).first);
			}
		}
		return;
	}

	TChildren::iterator it = _Children.find(token);
	if (it == _Children.end())
		return;

	if (rest.empty())
	{
		CMatchingNode matchingNode;
		matchingNode.Path = pathPrefix + (*it).first;
		matchingNode.Node = (*it).second;
		matchingNodes.push_back(matchingNode);
	}
	else
	{
		(*it).second->getNodes(rest, matchingNodes, pathPrefix + (*it).first);
	}
}

// ****************************************************************************
IStatDBNodePtr CStatDBBranch::removeNode(const std::string & path)
{
	BOMB_IF(path.empty(), "empty path!", return NULL);

	string token;
	string rest;
	splitPath(path, token, rest);

	TChildren::iterator it = _Children.find(token);
	if (it == _Children.end())
		return NULL;

	if (rest.empty())
	{
		IStatDBNodePtr removedNode = (*it).second;
		_Children.del(it);
		return removedNode;
	}

	return (*it).second->removeNode(rest);
}

// ****************************************************************************
void CStatDBBranch::acceptVisitor(CStatDBNodeVisitor & visitor, const std::string & currentPath)
{
	string childPathPrefix;
	if (!currentPath.empty())
		childPathPrefix = currentPath + ".";

	visitor.visitBranch(this, currentPath);

	for (TChildren::iterator it = _Children.begin(); it != _Children.end(); ++it)
	{
		(*it).second->acceptVisitor(visitor, childPathPrefix + (*it).first);
	}
}

// ****************************************************************************
bool CStatDBBranch::isValidToken(const std::string & token) const
{
	if (token.empty())
		return false;

	for (string::const_iterator it = token.begin(); it != token.end(); ++it)
	{
		if (!isalnum(*it) && (*it) != '_')
			return false;
	}

	return true;
}

// ****************************************************************************
void CStatDBBranch::splitPath(const std::string & path, std::string & token, std::string & rest) const
{
	string::size_type i = path.find('.');
	if (i == string::npos)
	{
		token = path;
		rest.clear();
	}
	else
	{
		token = path.substr(0, i);
		rest = path.substr(i+1);
	}
}

// ****************************************************************************
// CStatDBEntitiesRemoval
// ****************************************************************************

// ****************************************************************************
void CStatDBEntitiesRemoval::addPlayerToRemove(NLMISC::CEntityId playerId)
{
	for (uint i = 0; i < _PlayersToRemove.size(); i++)
	{
		if (_PlayersToRemove[i] == playerId)
			return;
	}

	_PlayersToRemove.push_back(playerId);
}

// ****************************************************************************
void CStatDBEntitiesRemoval::addGuildToRemove(EGSPD::TGuildId guildId)
{
	for (uint i = 0; i < _GuildsToRemove.size(); i++)
	{
		if (_GuildsToRemove[i] == guildId)
			return;
	}

	_GuildsToRemove.push_back(guildId);
}

// ****************************************************************************
void CStatDBEntitiesRemoval::processRemoval(IStatDBNodePtr root)
{
	nlassert(root != NULL);

	if (_PlayersToRemove.empty() && _GuildsToRemove.empty())
		return;

	root->acceptVisitor(*this, "");

	_PlayersToRemove.clear();
	_GuildsToRemove.clear();
}

// ****************************************************************************
void CStatDBEntitiesRemoval::visitTableLeaf(CStatDBTableLeaf * tableLeaf, const std::string & path)
{
	for (uint i = 0; i < _PlayersToRemove.size(); i++)
	{
		tableLeaf->removePlayer(_PlayersToRemove[i]);
	}

	for (uint i = 0; i < _GuildsToRemove.size(); i++)
	{
		tableLeaf->removeGuild(_GuildsToRemove[i]);
	}
}

