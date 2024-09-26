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



#include "nel/misc/eid_translator.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "guild_unifier.h"
#include "guild_manager/guild_unifier_itf.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_member.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/fame_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/character_interface.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace EGSPD;
using namespace GU;

// utility function to convert a guild id into a human readable string
extern std::string guildIdToString(uint32 guildId);



class CGuildUnifier : 
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CGuildUnifierClientSkel,
//	public CGuildUnifierServerSkel,
	public IGuildUnifier
{
	
	/// flag about guild ready to broadcast or not
	bool		_GuildsReady;

	struct TPeerGuilds
	{
		set<TGuildId>	GuildIds;
	};

	typedef map<TModuleProxyPtr, TPeerGuilds>	TPeerModules;
	/// The list of guild unifier modules
	TPeerModules		_Peers;

	/// The list of guild unifier for broadcast purpose
	set<TModuleProxyPtr>			_Broadcast;

	/*************************************************************************/
	/* classical methods													 */
	/*************************************************************************/

public:
	CGuildUnifier()
		: _GuildsReady(false)
	{
		CGuildUnifierClientSkel::init(this);
	}

	// fill guild descriptor
	void buildGuildDesc(const CGuild *guild, CGuildDesc &guildDesc)
	{
		nlassert(guild != NULL);

		guildDesc.setGuildId(guild->getId() );
		guildDesc.setGuildName(guild->getName());
		guildDesc.setGuildDesc(guild->getDescription());
		guildDesc.setGuildMoney(guild->getMoney());
		guildDesc.setGuildCreationDate(guild->getCreationDate());
//		guildDesc.setGuildXP(guild->getXP());
//		guildDesc.setGuildChargePoints(guild->getChargesPoints());
		guildDesc.setGuildRace(guild->getRace());
		guildDesc.setGuildIcon(guild->getIcon());
		CGuild::TAllegiances allegiance = guild->getAllegiance();
		guildDesc.setDeclaredCult(allegiance.first);
		guildDesc.setDeclaredCiv(allegiance.second);

	}

	// fill a member desc
	void buildGuildMember(const CGuildMemberPD *guildMember, CGuildMemberDesc &memberDesc)
	{
		memberDesc.setMemberId(guildMember->getId());
//		memberDesc.setMemberName(CEntityIdTranslator::getInstance()->getByEntity(guildMember->getId()));
		memberDesc.setMemberEnterTime(guildMember->getEnterTime());
		memberDesc.setMemberGrade(guildMember->getGrade());
	}

	// fill a member list of a guild
	void buildGuildMemberList(const CGuild *guild, CGuildDesc &guildDesc)
	{
		nlassert(guild != NULL);

		// get the member list of the guild desc
		std::vector <CGuildMemberDesc> &members = guildDesc.getMembers();

		// iterator over the member
		map<TCharacterId, CGuildMemberPD*>::const_iterator first(guild->getMembersBegin());
		map<TCharacterId, CGuildMemberPD*>::const_iterator last(guild->getMembersEnd());

		for (; first != last; ++first)
		{
			CGuildMember * member = static_cast<CGuildMember *>( first->second );

			members.push_back(CGuildMemberDesc());
			buildGuildMember(member, members.back());
		}
	}

	// fill the fame of guild
	void buildGuildFames(const CGuild *guild, CGuildDesc &guildDesc)
	{
		nlassert(guild != NULL);
		
		// get the fame list of the guild desc
		vector<CFameEntryDesc> &fames = guildDesc.getFames();

		const CGuildFameContainerPD *famePD = guild->getFameContainer();
		BOMB_IF(famePD == NULL, "CGuildUnifier : buildGuildFames : failed to retreive fame for guild "<<guild->getEId().toString(), return);

		map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator first(famePD->getEntriesBegin());
		map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator last(famePD->getEntriesEnd());

		for (; first != last; ++first)
		{
			const CFameContainerEntryPD &fame = first->second;

			fames.push_back(CFameEntryDesc());

			CFameEntryDesc &fed = fames.back();

			fed.setFactionSheetId(first->first);
			fed.setFameValue(fame.getFame());
			fed.setFameMemory(fame.getFameMemory());
			fed.setFameTrend(fame.getLastFameChangeTrend());
		}
	}

	// fill a vector with descriptors for all local guilds
	void buildAllGuildDesc(vector<CGuildDesc> &guilds)
	{
		CGuildManager *gm = CGuildManager::getInstance();
		BOMB_IF(gm == NULL, "CGuildUnifier:buildAllGuildDesc couldn't access to guild manager", return);

		EGSPD::CGuildContainerPD *gc = gm->getGuildContainer();
		BOMB_IF(gm == NULL, "CGuildUnifier:buildAllGuildDesc couldn't access to guild container", return);

		map<TGuildId, CGuildPD*>::iterator first(gc->getGuildsBegin());
		map<TGuildId, CGuildPD*>::iterator last(gc->getGuildsEnd());

		for (; first != last; ++first)
		{
			CGuild * guild = EGS_PD_CAST<CGuild*>( first->second );

			if (guild->isProxy())
				// this is a proxy guild, don't send it
				continue;

			// ok, fill a guild info slot
			guilds.push_back(CGuildDesc());
			buildGuildDesc(guild, guilds.back());
			buildGuildMemberList(guild, guilds.back());
			buildGuildFames(guild, guilds.back());

		}
	}

	void setGuildFames(CGuild *guild, const CGuildDesc &guildDesc)
	{
		H_AUTO(setGuildFame);
		if ( guild->getFameContainer() == NULL )
		{
			EGSPD::CGuildFameContainerPD* container = EGSPD::CGuildFameContainerPD::create(CEntityId(RYZOMID::guild, guild->getId()));
			EGS_PD_AST( container );
			guild->setFameContainer( container );
		}

		EGSPD::CGuildFameContainerPD* container = guild->getFameContainer();

		// fill it with fame values
		// NB : we assume that fame entry are never removed. 
		// at worst, if an entry gets removed from the original guild, it will
		// stay in the proxy, this is not a major issue and it could be fixed
		// by the time we encounter it (witch a guess will not come)
		for (uint i=0; i<guildDesc.getFames().size(); ++i)
		{
			const CFameEntryDesc &fed = guildDesc.getFames()[i];

			CFameContainerEntryPD *entry = container->getEntries(fed.getFactionSheetId());
			if (entry == NULL)
			{
				// create a new fame entry in the table
				entry = container->addToEntries(fed.getFactionSheetId());
				nlassert(entry != NULL);
			}
			// update the values
			entry->setFame(fed.getFameValue());
			entry->setFameMemory(fed.getFameMemory());
			entry->setLastFameChangeTrend(fed.getFameTrend());
		}

		// update the fames mirror
		CFameManager::getInstance().updateGuildFame(guild->getEId(), *guild->getFameContainer());
		// update the fame database
		guild->resetFameDatabase();
	}

	void setGuildMemberList(CGuild *guild, const CGuildDesc &guildDesc)
	{
		H_AUTO(SetGuildMemberList);
		for (uint i=0; i<guildDesc.getMembers().size(); ++i)
		{
			const CGuildMemberDesc &gmd = guildDesc.getMembers()[i];

			// check that the eid translator already know the character name
//			if (!CEntityIdTranslator::getInstance()->isEntityRegistered(gmd.getMemberId()))
//			{
//				// register the name
//				CEntityIdTranslator::getInstance()->registerEntity(gmd.getMemberId(), 
//					gmd.getMemberName(), 
//					uint8(gmd.getMemberId().getShortId() & 0xf),
//					uint32(gmd.getMemberId().getShortId()>>4),
//					"");
//			}

			CGuildMember *member = guild->newMember(gmd.getMemberId(), gmd.getMemberEnterTime());
			member->setMemberGrade(gmd.getMemberGrade());

//			ICharacter *character = ICharacter::getInterface(PlayerManager.getChar(gmd.getMemberId()));
//			if (character)
//			{
//				// the character is online
//				guild->setMemberOnline(member, character->getId().getDynamicId());
//			}
//			else
//			{
//				// the character is offline
////				guild->setMemberOffline(member);
//			}
		}
	}

	void setGuildDesc(CGuild *guild, const CGuildDesc &guildDesc)
	{
		H_AUTO(setGuildDesc);
		guild->setName(guildDesc.getGuildName());
		guild->setDescription(guildDesc.getGuildDesc());
		guild->setMoney(guildDesc.getGuildMoney());
		guild->setCreationDate(guildDesc.getGuildCreationDate());
		guild->setRace(guildDesc.getGuildRace());
		guild->setIcon(guildDesc.getGuildIcon());
		guild->setDeclaredCult(guildDesc.getDeclaredCult(), true);
		guild->setDeclaredCiv(guildDesc.getDeclaredCiv(), true);
	}


	void deleteGuild(NLNET::IModuleProxy *proxy, TGuildId  guildId)
	{
		nldebug("CGuildUnifier::deleteGuild : deleting guild %u", guildId);

		CGuild *guild = CGuildManager::getInstance()->getGuildFromId(guildId);
		BOMB_IF(guild == NULL, "Can't find the guild "<<guildId<<" to delete", _Peers[proxy].GuildIds.erase(guildId); return);

		if (guild->getMembers().size() == 0)
		{
			nlinfo("deleteGuild : deleting guild %s that has 0 members !", guildIdToString(guildId).c_str());
			// no member in this guild, do a manual removal !
			CGuildManager::getInstance()->deleteGuild(guildId);
		}
		else
		{
			// remove all but the last members
			while (guild->getMembers().size() > 1)
			{
				guild->removeMember(guild->getMembers().begin()->first);
			}
			// remove the last member (this will delete the guild)
			guild->removeMember(guild->getMembers().begin()->first);
		}

		// a check
		STOP_IF(CGuildManager::getInstance()->getGuildFromId(guildId) != NULL, "The guild "<<guildId<<" had not been removed after deleting the last member");

		// remove the guild from the list of foreign guilds of this proxy
		_Peers[proxy].GuildIds.erase(guildId);
	}

	/*************************************************************************/
	/* IModule virtual overloads											 */
	/*************************************************************************/

	void onModuleUp(NLNET::IModuleProxy *module)
	{
		if (module->getModuleClassName() == "GuildUnifier")
		{
			// this is one of our friend, store it
			_Peers.insert(make_pair(module, TPeerGuilds()));

			if (_GuildsReady)
			{
				CGuildUnifierClientProxy guc(module);

				guc.guildReady(this);
			}
		}
	}

	void onModuleDown(NLNET::IModuleProxy *module)
	{
		if (module->getModuleClassName() == "GuildUnifier")
		{
			if (_Peers.find(module) != _Peers.end())
			{
				// we know it (we should in fact), remove proxyed guilds
				TPeerModules::iterator it(_Peers.find(module));
				TPeerGuilds &pg = it->second;

				while (!pg.GuildIds.empty())
				{
					TGuildId guildId = *(pg.GuildIds.begin());
					deleteGuild(module, guildId);
//					pg.GuildIds.erase(pg.GuildIds.begin());
				}

				// remove it
				_Peers.erase(module);
			}
			// remove if from broadcast the list
			_Broadcast.erase(module);
//			// loop in the broadcast vector, looking for this one
//			set<TModuleProxyPtr>::iterator first(_Broadcast.begin()), last(_Broadcast.end());
//			for (; first != last; ++first)
//			{
//				if (*first == module)
//				{
//					_Broadcast.erase(first);
//					return;
//				}
//			}
		}
	}

//	bool onProcessModuleMessage(NLNET::IModuleProxy *sender, const CMessage &message)
//	{
//		if (CGuildUnifierClientSkel::onDispatchMessage(sender, message))
//			return true;
////		if (CGuildUnifierServerSkel::onDispatchMessage(sender, message))
////			return;
//
//		BOMB("CGuildUnifier : received unknown message '"<<message.getName()<<"'", return false;);
//
//		return false;
//	}

	void onModuleUpdate()
	{
		H_AUTO(CGuildUnifier_onModuleUpdate);
		// each frame, check for guild member and member list modification to broadcast

		CGuildManager *gm = CGuildManager::getInstance();
		// first, check the member list
		{
			CGuildManager::TChangedMemberList::iterator first(gm->_ChangedMemberList.begin()), last(gm->_ChangedMemberList.end());
			for (; first != last; ++first)
			{
				TGuildId guildId = *first;
				CGuild *guild = gm->getGuildFromId(guildId);

				if (guild != NULL && !guild->isProxy())
				{
					CGuildDesc gd;

					// for known, we use 'brute force', i.e. we broadcast the whole list.
					buildGuildMemberList(guild, gd);
					
					CGuildUnifierClientProxy::broadcast_updateMemberList(_Broadcast.begin(), _Broadcast.end(), this, guildId, gd.getMembers());
				}
			}

			// cleanup the list
			gm->_ChangedMemberList.clear();
		}

		// second, check the members
		{
			CGuildManager::TChangedMembers::iterator first(gm->_ChangedMembers.begin()), last(gm->_ChangedMembers.end());
			for (; first != last; ++first)
			{
				TGuildId guildId = first->first;

				CGuild *guild = gm->getGuildFromId(guildId);
				if (guild && !guild->isProxy())
				{
					// we have the guild, now iterate over the changed members
					set<TCharacterId>::iterator f2(first->second.begin()), l2(first->second.end());
					for (; f2 != l2; ++f2)
					{
						TCharacterId cid = *f2;

						CGuildMemberPD *memberPd = guild->getMembers(cid);
						if (memberPd != NULL)
						{
							// ok this member still member of the guild
							CGuildMemberDesc memberDesc;
							buildGuildMember(memberPd, memberDesc);

							CGuildUnifierClientProxy::broadcast_updateMemberInfo(_Broadcast.begin(), _Broadcast.end(), this, guildId, memberDesc);
						}
					}
				}
			}

			// cleanup the list
			gm->_ChangedMembers.clear();
		}

	}

	/*************************************************************************/
	/* IGuildUnifier virtual implementation									 */
	/*************************************************************************/

	/// The guild manager ask to send all guild info a all known clients
	void broadcastAllGuilds()
	{
		nlassert(!_GuildsReady);

		_GuildsReady = true;
		// build a vector with all guild and broadcast to all clients
		vector<CGuildDesc>	guilds;

		buildAllGuildDesc(guilds);

		// send the 'ready' message to all peers
		TPeerModules::iterator first(_Peers.begin()), last(_Peers.end());
		for (; first != last; ++first)
		{
			CGuildUnifierClientProxy guc(first->first);

			guc.guildReady(this);
		}

		// send guild data to 'ready' peers (those in the broadcast list)
		CGuildUnifierClientProxy::broadcast_receiveForeignGuild(_Broadcast.begin(), _Broadcast.end(), this, guilds);
	}

	/// A guild has been added
	void guildCreated(const CGuild *guild)
	{
		vector<CGuildDesc> guilds(1);
		CGuildDesc &gd = guilds[0];

		// build the complete guild description
		buildGuildDesc(guild, gd);
		buildGuildMemberList(guild, gd);
		buildGuildFames(guild, gd);

		// send the guild to all peers
		CGuildUnifierClientProxy::broadcast_receiveForeignGuild(_Broadcast.begin(), _Broadcast.end(), this, guilds);
	}


	void guildDeleted(uint32 guildId)
	{
		CGuildUnifierClientProxy::broadcast_guildDeleted(_Broadcast.begin(), _Broadcast.end(), this, guildId);
	}

	/// Broadcast a guild update (guild base data and fames values)
	void broadcastGuildUpdate(IGuild *ig)
	{
		// ok, we need to rebuild a descriptor with base guild info and fames values
		CGuildDesc gd;

		CGuild *guild = static_cast<CGuild*>(ig);

		buildGuildDesc(guild, gd);
		buildGuildFames(guild, gd);

		CGuildUnifierClientProxy::broadcast_updateGuild(_Broadcast.begin(), _Broadcast.end(), this, gd);
	}


	/// Broadcast a guild message
	void sendMessageToGuildMembers( const CGuild *guild, const std::string &  msg, const TVectorParamCheck & params ) 
	{
		if (guild->isProxy())
			return;

		CGuildUnifierClientProxy::broadcast_messageToGuildMembers(_Broadcast.begin(), _Broadcast.end(), this, guild->getId(), msg, params);
	}

	void guildManagerReleased()
	{
		// clear all foreign guild list
		TPeerModules::iterator first(_Peers.begin()), last(_Peers.end());
		for (; first != last; ++first)
		{
			first->second.GuildIds.clear();
		}
	}

	

	/*************************************************************************/
	/* CGuimdUnifierClientSkel virtual implementation						 */
	/*************************************************************************/

	// A client says to others clients that it is ready to send/receive guild data
	virtual void guildReady(NLNET::IModuleProxy *sender)
	{
		// this peer is ready, insert him to the broadcast list
		_Broadcast.insert(sender);

		// if we are ready ourself, send it the guild data
		if (_GuildsReady)
		{
			vector<CGuildDesc>	guilds;

			buildAllGuildDesc(guilds);

			CGuildUnifierClientProxy guc(sender);

			guc.receiveForeignGuild(this, guilds);
		}
	}

	// The server send it local guilds to the client
	virtual void receiveForeignGuild(NLNET::IModuleProxy *sender, const std::vector < CGuildDesc > &guilds) 
	{
		H_AUTO(receiveForeignGuild);
		// iterate over the received guild and add them in the guild manager
		CGuildManager *gm = CGuildManager::getInstance();

		for (uint i=0; i<guilds.size(); ++i)
		{
			const CGuildDesc &gd = guilds[i];

			nldebug("receiveForeignGuild : Creating foreign guild proxy for guild %s from %s",
				guildIdToString(gd.getGuildId()).c_str(),
				sender->getModuleName().c_str());

			CGuild *guild = gm->createGuildProxy(gd.getGuildId(), 
				gd.getGuildName(), 
				gd.getGuildIcon(),
				gd.getGuildDesc(),
				gd.getGuildRace(),
				gd.getGuildCreationDate());

			// initialize the guild
			setGuildDesc(guild, gd);
			setGuildMemberList(guild, gd);
			setGuildFames(guild, gd);

			// store this guild as associated with the sender module.
			this->_Peers[sender].GuildIds.insert(guild->getId());
		}
	}

	// The member list have changed, each guild unifier receive a copy of the new list
	virtual void updateMemberList(NLNET::IModuleProxy *sender, uint32 guildId, const std::vector < CGuildMemberDesc > &members) 
	{
		CGuildManager *gm = CGuildManager::getInstance();
		// 1st, retrieve the guild
		CGuild *guild = gm->getGuildFromId(guildId);

		BOMB_IF(guild == NULL, "Can't find foreign guild "<<guildId, return);

		// build a temporary set to speed up removal
		set<CEntityId>	memberIds;
		for (uint i=0; i<members.size(); ++i)
			memberIds.insert(members[i].getMemberId());

		// 2nd, check all members for added members
		for (uint i=0; i<members.size(); ++i)
		{
			const CGuildMemberDesc &gmd = members[i];
			if (guild->getMembers(members[i].getMemberId()) == NULL)
			{
				// we need to add this member
				CGuildMember *guildMember = guild->newMember(gmd.getMemberId());
				BOMB_IF(guildMember == NULL,
					"Failed to add member "<<gmd.getMemberId().toString()<<" into guild "<<guildId, continue);
				guild->setMemberGrade(guildMember, gmd.getMemberGrade());
			}
		}
		// 3rd, remove any removed members
		vector<CEntityId>	memberToRemove;
		std::map<TCharacterId, CGuildMemberPD*>::iterator first(guild->getMembersBegin()), last(guild->getMembersEnd());
		for (; first != last; ++first)
		{
			if (memberIds.find(first->first) == memberIds.end())
			{
				// this member is no more part of the guild
				memberToRemove.push_back(first->first);
			}
		}
		// remove the unwanted members
		while (!memberToRemove.empty())
		{
			if (guild->getMembers().size() == 1)
			{
				// wow, this is the last member !
				// the guild will be deleted, so remove it from
				// the list of guild for the sender
				_Peers[sender].GuildIds.erase(guild->getId());
			}
			nldebug("CGuildUnifier::updateMemberList : removing member %s from foreign guild %u", memberToRemove.back().toString().c_str(), guildId);
			guild->removeMember(memberToRemove.back());
			memberToRemove.pop_back();
		}

	}
	// A member in the guild has changed, update it's info
	virtual void updateMemberInfo(NLNET::IModuleProxy *sender, uint32 guildId, const CGuildMemberDesc &memberInfo)
	{
		// 1st, retrieve the guild
		CGuild *guild = CGuildManager::getInstance()->getGuildFromId(guildId);
		BOMB_IF(guild == NULL, "Failed to retrieve foreign guild "<<guildId<<"to update", return);

		// get the member
		CGuildMember *member = guild->getMemberFromEId(memberInfo.getMemberId());
		BOMB_IF(member == NULL, "Failed to retrieve member "<<memberInfo.getMemberId()<<" to update un guild "<<guildId, return);

		// update it
		member->setMemberGrade(memberInfo.getMemberGrade());
	}
	// The guild has been saved, the guild host send an update of the guild status (with fames, but no members)
	virtual void updateGuild(NLNET::IModuleProxy *sender, const CGuildDesc &guildInfo)
	{
		// 1st, retrieve the guild
		CGuild *guild = CGuildManager::getInstance()->getGuildFromId(guildInfo.getGuildId());
		BOMB_IF(guild == NULL, "Failed to retrieve foreign guild "<<guildIdToString(guildInfo.getGuildId())<<" to update", return);

		// 2nd, update the guild properties
		setGuildDesc(guild, guildInfo);

		// 3rd, update fame value
		setGuildFames(guild, guildInfo);
	}

	virtual void guildDeleted(NLNET::IModuleProxy *sender, uint32 guildId)
	{
		deleteGuild(sender, guildId);
	}

	// Send a message to all the guild members
	virtual void messageToGuildMembers(NLNET::IModuleProxy *sender, uint32 guildId, const std::string &messageName, const TVectorParamCheck &params)
	{
		// retrieve the guild
		CGuild *guild = CGuildManager::getInstance()->getGuildFromId(guildId);
		if (guild == NULL)
			return;

		guild->sendMessageToGuildMembers(messageName, params);
	}


	/*************************************************************************/
	/* CGuimdUnifierServerSkel virtual implementation						 */
	/*************************************************************************/
	
	/*************************************************************************/
	/* Commands handler														 */
	/*************************************************************************/
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CGuildUnifier, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CGuildUnifier, dump, "Dump the module internal state", "no param");
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump)
	{
		NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

		log.displayNL("-----------------------------");
		log.displayNL("Dumping guild unifier state :");
		log.displayNL("-----------------------------");

		log.displayNL("  Guild manager have guilds %s", _GuildsReady ? "READY" : "NOT READY");


		log.displayNL("  There is %u known peer guild unifier :", _Peers.size());
		TPeerModules::iterator first(_Peers.begin()), last(_Peers.end());
		for (; first != last; ++first)
		{
			log.display("   + Peer module '%s' ", first->first->getModuleName().c_str());
			if (_Broadcast.find(first->first) != _Broadcast.end())
				log.displayNL("supporting %u guilds", first->second.GuildIds.size());
			else
				log.displayNL("NOT READY");
		}

		return true;

	}

};

NLNET_REGISTER_MODULE_FACTORY(CGuildUnifier, "GuildUnifier");

