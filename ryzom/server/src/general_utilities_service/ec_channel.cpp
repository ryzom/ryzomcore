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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
#include "game_share/utils.h"

// local
#include "ec_channel.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace GUS;


//-----------------------------------------------------------------------------
// EC namespace
//-----------------------------------------------------------------------------

namespace EC
{
	//-----------------------------------------------------------------------------
	// methods CUserGroup
	//-----------------------------------------------------------------------------

	CUserGroup::CUserGroup(IChannel* parent)
	{
		_Parent= parent;
	}

	NLMISC::CSString CUserGroup::processCommand(const NLMISC::CSString& command)
	{
		CSString result;
		CVectorSString tokens;
		command.splitWords(tokens);

		// if command is empty then just display the user group
		if (tokens.empty())
		{
			return CSString().join(_CharacterIds,' ');
		}

		// there must be an even number of tokens
		if (tokens.size()&1)
			return "Bad command syntax (odd token count found): "+command;

		// perform a syntax check and giveup if hte syntax is bad
		for (uint32 i=0;i<tokens.size();i+=2)
		{
			if (tokens[i]!="+" && tokens[i]!="-")
				return "Bad command syntax (expected '+' or '-' but found '"+tokens[i]+"'): "+command;
			if (!tokens[i+1].isValidKeyword())
				return "Bad command syntax (expected character naem but found '"+tokens[i]+"'): "+command;
		}

		// perform adds and removals
		for (uint32 i=0;i+1<tokens.size();i+=2)
		{
			CSString& sign= tokens[i];
			CSString& name= tokens[i+1];
			nldebug("EC: Treating channel operation: '%s' %s",sign.c_str(),name.c_str());
			if (sign=="+")
			{
				// look to see whether name already exists in this user group
				TCharacterIds::iterator it= _CharacterIds.begin();
				while (it!=_CharacterIds.end() && (*it)!=name) ++it;
				// if name found then we're done so skip forwards
				if (it!=_CharacterIds.end())
					continue;

				// add to this user group
				_CharacterIds.push_back(name);

				// tell container to add named character to chat list
				nldebug("EC: Telling chat channel to add user: %s",name.c_str());
				_Parent->_chatAdd(name);

				// add a segment to the result string
				result+=(result.empty()?"+":" +")+name;
			}
			else if (sign=="-")
			{
				// look to see whether name exists in this user group
				TCharacterIds::iterator it= _CharacterIds.begin();
				while (it!=_CharacterIds.end() && (*it)!=name) ++it;
				// if name not found then we're done so skip forwards
				if (it==_CharacterIds.end())
					continue;

				// remove from this user group
				(*it)=_CharacterIds.back();
				_CharacterIds.pop_back();

				// tell container to remove named character from chat list
				nldebug("EC: Telling chat channel to remove user: %s",name.c_str());
				_Parent->_chatRemove(name);

				// add a segment to the result string
				result+=(result.empty()?"-":" -")+name;
			}
			else
			{
				nlerror("This can never happen because we performed a previous test to ensure that all 'signs' are valid before doing this");
			}
		}
		return result;
	}

	bool CUserGroup::contains(const TCharacterId& id) const
	{
		for (uint32 i=_CharacterIds.size();i--;)
			if (_CharacterIds[i]==id)
				return true;
		return false;
	}


	//-----------------------------------------------------------------------------
	// methods IChannel
	//-----------------------------------------------------------------------------

	IChannel::IChannel(const NLMISC::CSString& name): _Members(this), _Officers(this), _Archs(this)
	{
		_Chat= CChatManager::getInstance()->createChatChannel(name);
		_Chat->setChatCallback(this);
	}

	IChannel::~IChannel()
	{
		_Chat->closeChannel();
	}

	const NLMISC::CSString& IChannel::getChannelName() const
	{
		return _Chat->getChannelName();
	}

	const NLMISC::CSString& IChannel::getChannelTitle() const
	{
		return _Chat->getChannelTitle();
	}

	void IChannel::setChannelTitle(const NLMISC::CSString& title)
	{
		_Chat->setChannelTitle(title);
	}

	void IChannel::addMember(const TCharacterId& id)
	{
		setRank(id,MEMBER);
	}

	void IChannel::addOfficer(const TCharacterId& id)
	{
		setRank(id,OFFICER);
	}

	void IChannel::addArch(const TCharacterId& id)
	{
		setRank(id,ARCH);
	}

	CUserGroup& IChannel::getMembers()
	{
		return _Members;
	}

	CUserGroup& IChannel::getOfficers()
	{
		return _Officers;
	}

	CUserGroup& IChannel::getArchs()
	{
		return _Archs;
	}

	void IChannel::removeUser(const TCharacterId& id)
	{
		getMembers().processCommand("-"+id);
		getOfficers().processCommand("-"+id);
		getArchs().processCommand("-"+id);
	}

	const TChannelRank IChannel::getRank(const TCharacterId& id)
	{
		if (_Archs.contains(id))	return ARCH;
		if (_Officers.contains(id))	return OFFICER;
		if (_Members.contains(id))	return MEMBER;

		return NO_RANK;
	}

	void IChannel::setRank(const TCharacterId& id,const TChannelRank& rank)
	{
		const TChannelRank& oldRank= getRank(id);

		// if there's nothing to do then there's nothing to do!
		if (oldRank==rank)
			return;

		// the old rank and new rank differ so start by removing from the old ranking list
		switch(oldRank)
		{
			case MEMBER:	getMembers().processCommand("-"+id);	break;
			case OFFICER:	getOfficers().processCommand("-"+id);	break;
			case ARCH:		getArchs().processCommand("-"+id);		break;
			default:		break;
		};

		// add the client to one of the user groups or ignore him if the new rank
		switch(rank)
		{
			case MEMBER:	getMembers().processCommand("+"+id);	break;
			case OFFICER:	getOfficers().processCommand("+"+id);	break;
			case ARCH:		getArchs().processCommand("+"+id);		break;
			default:		break;
		};

		// perform additions or removals to/from chat provoked by the rank change
		_chatUpdate();
	}

	void IChannel::displayMembers()
	{
		InfoLog->displayNL("Members: %s",getMembers().processCommand("").c_str());
	}

	void IChannel::displayOfficers()
	{
		InfoLog->displayNL("Officers: %s",getOfficers().processCommand("").c_str());
	}

	void IChannel::displayArchs()
	{
		InfoLog->displayNL("Archs: %s",getArchs().processCommand("").c_str());
	}

	void IChannel::displayAllUsers()
	{
		displayMembers();
		displayOfficers();
		displayArchs();
	}

	void IChannel::sendMessage(GUS::TClientId clientId,const NLMISC::CSString& speaker,const NLMISC::CSString& txt)
	{
		getChannel().sendMessage(clientId,speaker,txt);
	}

	void IChannel::broadcastMessage(const NLMISC::CSString& speaker,const NLMISC::CSString& txt)
	{
		getChannel().broadcastMessage(speaker,txt);
	}

	CChatChannel& IChannel::getChannel()
	{
		nlassert(_Chat!=NULL);
		return *_Chat;
	}

	void IChannel::receiveMessage(GUS::TClientId clientId,const ucstring& txt)
	{
		// call the cbChatText callback
		TCharacterId id= CClientManager::getInstance()->getCharacterName(clientId);
		cbChatText(getRank(id),id,clientId,txt.toUtf8());

		// perform additions or removals to/from chat provoked by processing of the received message
		_chatUpdate();
	}

	void IChannel::clientReadyInChannel(CChatChannel *chatChannel, GUS::TClientId clientId)
	{
		BOMB_IF(_Chat!=chatChannel,"Unexpected receipt of clientReadyInChannel callback for wrong chat channel",return);

		// call the cbAddUser callback
		TCharacterId id= CClientManager::getInstance()->getCharacterName(clientId);
		cbAddUser(getRank(id),id,clientId);
	}

	bool IChannel::isClientAllowedInChatChannel(GUS::TClientId clientId, CChatChannel *chatChannel)
	{
		BOMB_IF(_Chat!=chatChannel,"Unexpected receipt of isClientAllowedInChatChannel callback for wrong chat channel",return false);

		TCharacterId id= CClientManager::getInstance()->getCharacterName(clientId);
		return _Members.contains(id) || _Officers.contains(id) || _Archs.contains(id);
	}

	void IChannel::_chatAdd(const TCharacterId& name)
	{
		// either add to the container's adds list or remove from its removes list
		if (_ChatRemoves.find(name)==_ChatRemoves.end())
		{
			// add name to the container channel's 'adds' list
			_ChatAdds.insert(name);
			nldebug("EC: Adding user to 'add' list: %s",name.c_str());
		}
		else
		{
			// remove name from the container channel's 'removes' list
			_ChatRemoves.erase(name);
			nldebug("EC: Removing user from 'remove' list: %s",name.c_str());
		}
	}

	void IChannel::_chatRemove(const TCharacterId& name)
	{
		// either add to the container's removes list or remove from its adds list
		if (_ChatAdds.find(name)==_ChatAdds.end())
		{
			// add name to the container channel's 'removes' list
			_ChatRemoves.insert(name);
			nldebug("EC: Adding user to 'remove' list: %s",name.c_str());
		}
		else
		{
			// remove name from the container channel's 'adds' list
			_ChatAdds.erase(name);
			nldebug("EC: Removing user from 'add' list: %s",name.c_str());
		}
	}

	void IChannel::_chatUpdate()
	{
		// if nothing to do then give up
		if (_ChatRemoves.empty() && _ChatAdds.empty())
			return;

		// remove players in the ChatRemoves set from the chat channel
		if (!_ChatRemoves.empty())
		{
			nldebug("EC: Treating chat removes for channel: %s (%d removes)",getChannelTitle().c_str(),_ChatRemoves.size());
			CSString removes;
			for (TUntreatedUserSet::iterator it= _ChatRemoves.begin();it!=_ChatRemoves.end();++it)
			{
				// try to get the client id from the id, treating the id as a character name
				TClientId clientId= CClientManager::getInstance()->getClientId(*it);
				nldebug("EC: - Adding Client: %s(%d)",(*it).c_str(),clientId.getIndex());

				// if we don't have a handle on the player then we're all done as they can't be logged in
				if (clientId== BadTClientId)
					continue;

				// remove the player from the current chat group
				_Chat->removeClient(clientId);

				// call the cbRemoveUser callback
				TCharacterId id= CClientManager::getInstance()->getCharacterName(clientId);
				cbRemoveUser(getRank(id),id,clientId);

				// add a block to the log string
				removes+= (removes.empty()?"":" ")+ (*it);
			}
			nlinfo("EC: update chat: remove %d: online: %s",_ChatRemoves.size(),removes.c_str());
			_ChatRemoves.clear();
		}

		// add players in the ChatAdds set to the chat channel
		if (!_ChatAdds.empty())
		{
			nldebug("EC: Treating chat adds for channel: %s (%d adds)",getChannelTitle().c_str(),_ChatAdds.size());
			CSString adds;
			for (TUntreatedUserSet::iterator it= _ChatAdds.begin();it!=_ChatAdds.end();++it)
			{
				// try to get the client id from the id, treating the id as a character name
				TClientId clientId= CClientManager::getInstance()->getClientId(*it);
				nldebug("EC: - Removing Client: %s(%d)",(*it).c_str(),clientId.getIndex());

				// if we don't have a handle on the player then we're all done as they can't be logged in
				if (clientId== BadTClientId)
					continue;

				// add the client to the chat channel
				_Chat->addClient(clientId);

				// add a block to the log string
				adds+= (adds.empty()?"":" ")+ (*it);
			}
			nlinfo("EC: update chat: add %d: online: %s",_ChatAdds.size(),adds.c_str());
			_ChatAdds.clear();
		}
	}
}
