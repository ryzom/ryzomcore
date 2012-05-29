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

#include "nel/net/service.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/algo.h"

#include "game_share/bot_chat_types.h"
#include "game_share/backup_service_interface.h"
#include "game_share/ryzom_mirror_properties.h"
#include "server_share/mail_forum_validator.h"
#include "game_share/persistent_data_tree.h"
#include "server_share/log_item_gen.h"

#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"

#include "outpost_manager/outpost_manager.h"
#include "guild_manager.h"
#include "guild_leader_module.h"
#include "guild_invitation.h"
#include "fame_manager.h"
#include "guild_version_adapter.h"
#include "creature_manager/creature.h"
#include "outpost_manager/outpost_manager.h"
#include "primitives_parser.h"
#include "stat_db.h"
#include "modules/shard_unifier_client.h"
#include "modules/guild_unifier.h"
#include "guild_char_proxy.h"
#include "egs_pd.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace EGSPD;

extern bool						PDSIsUp;
extern bool						IOSIsUp;
extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern CVariable<uint32> AllowCharsFromAllSessions;
extern CVariable<bool> DontUseSU;

CGuildManager* CGuildManager::_Instance = NULL;

class CGuildFileClassCb : public IBackupFileClassReceiveCallback
{
public:
	/// Callback from BSI for file class list
	virtual void callback(const CFileDescriptionContainer& fileList)
	{
		CGuildManager::getInstance()->callback(fileList);
	}
};

class CGuildFileCb : public IBackupFileReceiveCallback
{
public:
	/// Callback from BSI for one guild file
	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		CGuildManager::getInstance()->callback(fileDescription, dataStream);
	}
};

// utility function to convert a guild id into a human readable string
std::string guildIdToString(uint32 guildId)
{
	static char buffer[256];

	sprintf(buffer, "%u:%u", guildId>>20, guildId & 0xfffff);

	return string(buffer);
}

// utility to parse a human readable string into a guild id
uint32 parseGuildId(const std::string &str)
{
	uint32 guildId;
	uint32 shardId;
	uint32 localId;

	if (sscanf(str.c_str(), "%u:%u", &shardId, &localId) != 2)
		return 0;

	guildId = (shardId << 20) | (localId & 0xfffff);

	return guildId;
}


// PDLIB factory for guild members
RY_PDS::IPDBaseData* CGuildManager::guildMemberFactoryPD()
{
	return new CGuildMember();
}

// PDLIB factory for guilds
RY_PDS::IPDBaseData* CGuildManager::guildFactoryPD()
{
	return new CGuild();
}

IGuildManager &IGuildManager::getInstance()
{
	return *(CGuildManager::getInstance());
}

const std::string &CGuildManager::getCommandHandlerName() const
{
	static string name("guildManager");

	return name;
}

//----------------------------------------------------------------------------
void CGuildManager::init()
{
	nlassert(_Instance == NULL );
	// allocate the instance
	_Instance = new CGuildManager;
}

CGuildManager::CGuildManager()
{
	_Container = NULL;
	_UpdateGuildMembersStringIds = false;
	_GuildLoaded = false;
	registerCommandsHandler();

	// register the callback for the 'GUILD:NAME' property in the client database
	CCDBStructBanks::instance()->getICDBStructNodeFromName(CDBGuild, "GUILD:NAME")->attachCallback(&CGuildManager::cb_guildNameIdAvailable);

//	// load the guilds
//	loadGuilds();
//
//	// call this after guild data are loaded
//	if (_UpdateGuildMembersStringIds)
//	{
//		updateGuildMembersStringIds();
//	}

}

//----------------------------------------------------------------------------
void CGuildManager::release()
{
	if(_Instance)
	{
		//EGSPD::PDSLib.getStringManager().save();
#ifndef USE_PDS
		if(_Instance->_Container)
		{
			for ( map<EGSPD::TGuildId, EGSPD::CGuildPD*>::iterator it = _Instance->_Container->getGuildsBegin(); it != _Instance->_Container->getGuildsEnd();++it )
			{
				CGuild * guild = EGS_PD_CAST<CGuild*>( (*it).second );
				nlassert(guild);
				_Instance->saveGuild(guild);
			}
		}
#endif
		delete _Instance;
		_Instance = NULL;
	}

	IGuildUnifier::getInstance()->guildManagerReleased();
}

//----------------------------------------------------------------------------
void CGuildManager::onIOSConnection()
{

	// create guild chat groups
	if (_Container != NULL)
	{
		std::map<TGuildId, CGuildPD*>::const_iterator first(_Container->getGuilds().begin()), last(_Container->getGuilds().end());
		for (; first != last; ++first)
		{
			CGuild *guild = dynamic_cast<CGuild*>(first->second);
			if (guild)
			{
				guild->openGuildChatGroup();
			}
		}
	}

//	loadGuilds();
//
//	// call this after guild data are loaded
//	if (_UpdateGuildMembersStringIds)
//	{
//		updateGuildMembersStringIds();
//	}

	/// todo guild : change that when PDS manage IOS connection
//#ifndef USE_PDS
//	onPDSConnection();
//#endif
//	if ( PDSIsUp )
//	{
//		// Prevent guild reloading in case of IOS crash
//		static bool firstLoad = true;
//		if (!firstLoad)
//			return;
//		firstLoad = false;
//
//		egs_gminfo("IOS connects. AS PDS is up I can load the guilds");
//
//		// load guilds
//		EGSPD::CGuildContainerPD::load(1);
//	}
}


//----------------------------------------------------------------------------
//void CGuildManager::onPDSConnection()
//{
//	// Prevent guild reloading in case of IOS crash
//	static bool firstLoad = true;
//	if (!firstLoad)
//		return;
//	firstLoad = false;
//
//	if ( IOSIsUp )
//	{
//		egs_gminfo("PDS connects. AS IOS is up I can load the guilds");
//
//		// load guilds
//		EGSPD::CGuildContainerPD::load(1);
//
//		// call this after guild data are loaded
//		if (_UpdateGuildMembersStringIds)
//		{
//			updateGuildMembersStringIds();
//		}
//	}
//}

//----------------------------------------------------------------------------
void CGuildManager::clientDBUpdate()
{
	H_AUTO( CGuildManagerUpdate );
	if ( !_Container )
		return;
	{
		H_AUTO( CGuildManagerUpdateGUILD_DB )
		for ( map<EGSPD::TGuildId,EGSPD::CGuildPD*>::iterator it = _Container->getGuildsBegin(); it != _Container->getGuildsEnd(); ++it )
		{
			CGuild* guild = EGS_PD_CAST< CGuild* >((*it).second);
			EGS_PD_AST( guild );
			{
				H_AUTO( CGuildManagerUpdateGUILD_DB_SEND_DELTAS )
				guild->sendClientDBDeltas();
			}
		}
	}

	if ( ( CTickEventHandler::getGameCycle() % GuildSavingPeriod ) == 0 )
	{
		// It's time to save a guild

		static TGuildId guildIdToSave = 0;
		std::map<TGuildId, CGuildPD*>::const_iterator it;

		// get the next guild to save
		it = _Container->getGuilds().upper_bound(guildIdToSave);
		for (;;)
		{
			if (it == _Container->getGuilds().end())
			{
				// we have finished to iterate over the container.
				guildIdToSave = 0;
				break;
			}
			else
			{
				guildIdToSave = it->first;
				CGuild * guild = EGS_PD_CAST<CGuild*>( _Container->getGuilds( guildIdToSave ) );
				if ( guild && !guild->isProxy())
				{
					// this is a local guild, save it and break the loop
					saveGuild( guild );
					break;
				}
				++it;
			}
		}
	}
}

//----------------------------------------------------------------------------
void CGuildManager::saveGuild( CGuild* guild )
{
	EGS_PD_AST( guild );
	if (guild->isProxy())
		return;

	uint32 id = guild->getId();
	{
		H_AUTO( CGuildManagerSaveGUILD_FAME )
			CFameManager::getInstance().saveGuildFame(guild->getEId(), *guild->getFameContainer());
	}

	static CPersistentDataRecordRyzomStore	pdr;
	pdr.clear();
	{
		H_AUTO( CGuildManagerUpdateGUILD_SAVE)
			guild->setVersion(CGuildVersionAdapter::getInstance()->currentVersionNumber());
			guild->store(pdr);
	}
	if ( DataPersistsAsText )
	{
		H_AUTO( CGuildManagerUpdate_PDR_WRITE_TEXT )
			string fileName = Bsi.getLocalPath()+NLMISC::toString("guilds/guild_%05u.txt", id & 0x000fffff);
		pdr.writeToTxtFile(fileName.c_str());
	}
	else
	{
		string fileName = NLMISC::toString("guilds/guild_%05u.bin", id & 0x000fffff);
		if( UseBS )
		{
			try
			{
				H_AUTO( CGuildManagerUpdatePDR_WRITE_BIN_PDS )
				// build a buffer from the record
				vector<char> buffer;
				uint32 bufSize= pdr.totalDataSize();
				buffer.resize(bufSize);
				{
					H_AUTO( CGuildManagerUpdateTO_BUFFER)
						pdr.toBuffer(&buffer[0], bufSize);
				}
				// serial it to file
				CBackupMsgSaveFile msg( fileName, CBackupMsgSaveFile::SaveFile, Bsi );
				msg.DataMsg.serialBuffer((uint8*)&buffer.front(), (uint)buffer.size());
				Bsi.sendFile( msg );
			}
			catch(const Exception &)
			{
				nlwarning("<guilds>  :  Can't serial file %s",fileName.c_str());
				return;
			}
		}
		else
		{
			H_AUTO( CGuildManagerUpdatePDR_WRITE_BIN_NO_PDS )
				pdr.writeToBinFile((Bsi.getLocalPath() + fileName).c_str());
		}
	}

	// broadcast new guild state to guild unifier peers
	IGuildUnifier::getInstance()->broadcastGuildUpdate(guild);
}

//----------------------------------------------------------------------------
bool CGuildManager::loadedGuilds() const
{
	return (_Container != NULL);
}


//----------------------------------------------------------------------------
/// A member have been changed in some way
void CGuildManager::guildMemberChanged(EGSPD::CGuildMemberPD *guildMemberPd)
{
	// store the member id in the change list for next guild unifier broadcast
	_ChangedMembers[static_cast<CGuild*>(guildMemberPd->getGuild())->getId()].insert(guildMemberPd->getId());
}

//----------------------------------------------------------------------------
/// the member list has been changed (ether add/replace or delete)
void CGuildManager::guildMemberListChanged(EGSPD::CGuildPD *guildPd)
{
	// store the guild id in the change list for next guild unifier broadcast
	_ChangedMemberList.insert(static_cast<CGuild*>(guildPd)->getId());
}

//-----------------------------------------------------------------------------
void CGuildManager::cb_guildNameIdAvailable(CCDBSynchronised *syncDb, ICDBStructNode *node)
{
	if (_Instance->_Container == NULL)
		return;
	// retrieve the corresponding guild with the syncDB
	std::map<TGuildId, CGuildPD*>::const_iterator first(_Instance->_Container->getGuilds().begin()), last(_Instance->_Container->getGuilds().end());
	for (; first  != last; ++first)
	{
		CGuild *guild = dynamic_cast<CGuild*>(first->second);

		if (&guild->getClientDB() == syncDb)
		{
			// we found it !
			guild->onGuildStringUpdated(node);
		}
	}

}


//----------------------------------------------------------------------------
CGuild *CGuildManager::createGuildProxy(uint32 guildId, const ucstring & guildName,const uint64 &icon,const ucstring & description, EGSPD::CPeople::TPeople race, NLMISC::TGameCycle creationDate)
{
	H_AUTO(CreateGuildProxy);
	CGuild * guild = EGS_PD_CAST<CGuild*>( EGSPD::CGuildPD::create(guildId) );
	EGS_PD_AST( guild );
	guild->initNonPDMembers();
//	guild->postCreate();
	_Container->setGuilds( guild );

	// tag this guild has 'proxy' guild
	guild->setIsProxy(true);

	EGSPD::CGuildFameContainerPD * fameCont = EGSPD::CGuildFameContainerPD::create(CEntityId(RYZOMID::guild,guildId));
	EGS_PD_AST(fameCont);
	guild->setFameContainer( fameCont );

	/// register guild in other system
	guild->registerGuild();

	// set initial guild fames
///	guild->setStartFameAndAllegiance( proxy.getId() );

//	proxy.setGuildId(guildId);

	// set the guild initial attributes
	guild->setIcon(icon);
//	guild->setVillage(place);
	guild->setRace(race);
	guild->setCreationDate( creationDate);

//	for ( uint i = 0; i < EGSPD::CSPType::EndSPType; i++ )
//	{
//		guild->setRolemastersValid( (EGSPD::CSPType::TSPType)i, false );
//	}

	// init the guild strings
	guild->setName(guildName);
	guild->setDescription(description);
//	_GuildsAwaitingString.insert( std::make_pair( guildName, guild->getId() ) );
//	_GuildsAwaitingString.insert( std::make_pair( description, guild->getId() ) );
//	NLMISC::CEntityId stringEId = guild->getEId();
//	stringEId.setType( RYZOMID::guildName );
//	EGSPD::PDSLib.getStringManager().addString( stringEId, guildName );
//	stringEId.setType( RYZOMID::guildDescription );
//	EGSPD::PDSLib.getStringManager().addString( stringEId, description );

	return guild;
}

//----------------------------------------------------------------------------
void CGuildManager::dumpGuilds( bool onlyLocal, NLMISC::CLog & log )const
{
	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (1)");
		return;
	}
	uint count = 0;
	log.displayNL("Dumping from %u guilds total:", _Container->getGuilds().size());
	for ( map<uint32,EGSPD::CGuildPD*>::iterator it = _Container->getGuildsBegin(); it != _Container->getGuildsEnd(); ++it )
	{
		CGuild * guild = EGS_PD_CAST<CGuild*>( (*it).second );

		if (onlyLocal && guild->isProxy())
			continue;

		EGS_PD_AST(guild);
		count++;
		const ucstring & name = guild->getName();
		log.displayNL("  id = %s %s %s, name = '%s', %u members",
			guildIdToString(it->first).c_str(),
			(it->first)>>20 == IService::getInstance()->getShardId() ? "(Local)" : "(Foreign)",
			guild->getEId().toString().c_str(),
			name.toString().c_str(),
			guild->getMembers().size());
	}
	log.displayNL("\n%u valid guilds dumped",count);
}

//----------------------------------------------------------------------------
uint32 CGuildManager::getFreeGuildId()
{
	nlassert(_Container != NULL);

	uint32 guildId = 1 + (IService::getInstance()->getShardId()<<20);

	std::map<TGuildId, CGuildPD*>::const_iterator it = _Container->getGuildsBegin();

	while (it != _Container->getGuildsEnd())
	{
		if (it->first > guildId)
		{
			// ok, this number is free
			break;
		}

		// increment the counter and the iterator
		++guildId;
		++it;
	}

	return guildId;
}


//----------------------------------------------------------------------------
CGuild * CGuildManager::getGuildByName( const ucstring & name )
{
	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (2)");
		return NULL;
	}
	for ( std::map<EGSPD::TGuildId,EGSPD::CGuildPD*>::iterator it = _Container->getGuildsBegin();  it != _Container->getGuildsEnd(); ++it )
	{
		CGuild * guild = EGS_PD_CAST<CGuild*>( (*it).second );
		EGS_PD_AST(guild);
		string guildName, namesAsked;
		guildName = guild->getName().toString();
		namesAsked = name.toString();
		if ( strlwr(guildName) == strlwr(namesAsked) )
			return guild;
	}
	return NULL;
}

//----------------------------------------------------------------------------
CGuild * CGuildManager::getGuildFromId( EGSPD::TGuildId id )
{
	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (3)");
		return NULL;
	}
	EGSPD::CGuildPD* guildPD = _Container->getGuilds( id );
	if( guildPD == NULL)
		return NULL;
	CGuild * guild = EGS_PD_CAST<CGuild*>(guildPD);
	EGS_PD_AST( guild );
	return guild;
}

//----------------------------------------------------------------------------
//bool CGuildManager::updateGuildStringIds( const ucstring & str )
//{
//	if ( _Container == NULL )
//	{
//		nlwarning("<GUILD> container not initialized (4)");
//		return false;
//	}
//	std::pair< std::map<ucstring,EGSPD::TGuildId>::iterator, std::map<ucstring,EGSPD::TGuildId>::iterator > thePair = _GuildsAwaitingString.equal_range( str );
//	if ( thePair.first == thePair.second )
//		return false;
//
//	for ( std::map<ucstring,EGSPD::TGuildId>::iterator it = thePair.first; it != thePair.second;++it )
//	{
//		TGuildId guildId = it->second;
//		CGuild * guild = EGS_PD_CAST<CGuild*>( _Container->getGuilds( guildId ) );
//		EGS_PD_AST(guild);
//		guild->onGuildStringUpdated();
//	}
//	_GuildsAwaitingString.erase( str );
//	return true;
//}

//----------------------------------------------------------------------------
bool CGuildManager::checkGuildMemberShip( const EGSPD::TCharacterId & userId, const EGSPD::TGuildId & guildId )const
{
	// if we have no container, it means that the shard is not initialized yet and that players are loaded for check purpose
	if ( !_Container )
		return true;

	EGSPD::CGuildPD * guild = _Container->getGuilds( guildId );
	if ( ! guild )
		return false;
	return ( guild->getMembers( userId ) != NULL );
}

//----------------------------------------------------------------------------
void CGuildManager::playerConnection( CGuildCharProxy & proxy )
{
	// simply create a guild module
	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (5)");
		return;
	}
	if ( proxy.getGuildId() == 0 )
		return;
	CGuild * guild = EGS_PD_CAST<CGuild*>(_Container->getGuilds( proxy.getGuildId() ) );
	if ( !guild )
	{
		nlwarning("<GUILD>%s has an invalid guild %d. SERIOUS BUG : checks must be done before");
		return;
	}
	EGSPD::CGuildMemberPD * member = guild->getMembers( proxy.getId() );
	if ( !member )
	{
		nlwarning("<GUILD>%s guild %d : user is not in its guild. SERIOUS BUG : checks must be done before");
		return;
	}
	CGuildMember * realMember = EGS_PD_CAST<CGuildMember*>( member );
	EGS_PD_AST( realMember );
	CGuildMemberModule * module = CGuildMemberModule::createModule( proxy, realMember );
	if ( module == NULL )
	{
		nlwarning("<GUILD>%s guild %d : invalid member grade, deleteing the member");
		guild->deleteFromMembers( proxy.getId() );

		CGuildManager::getInstance()->removeCharToGuildAssoc(proxy.getId(), guild->getId());

		proxy.setGuildId(0);
	}
	else
		guild->setMemberOnline( realMember,proxy.getId().getDynamicId() );
}

//----------------------------------------------------------------------------
void CGuildManager::removeInvitation(CGuildInvitation* invitation)
{
	nlassert(invitation);
	const uint size = (uint)_Invitations.size();
	for ( uint i = 0; i < size; ++i )
	{
		if ( _Invitations[i] == invitation )
		{
			delete invitation;
			_Invitations[i] = _Invitations.back();
			_Invitations.pop_back();
			return;
		}
	}
	nlerror("<GUILD> could not find an existing invitation : SERIOUS BUG");
}

//----------------------------------------------------------------------------
bool CGuildManager::setGMGuild( uint32 guildId )
{
	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (6)");
		return false;
	}
	if ( guildId == 0 )
	{
		nlwarning("<GUILD> invalid GM guild 0 valid indexes start at 1");
		return false;
	}
	CGuild* guild = EGS_PD_CAST<CGuild*>( _Container->getGuilds( guildId ) );
	if ( !guild )
	{
		nlwarning("<GUILD> invalid GM guild %s : not found in the manager", guildIdToString(guildId).c_str());
		return false;
	}
	_Container->setGMGuild( guildId );
	return true;
}

//----------------------------------------------------------------------------
bool CGuildManager::isGMGuild( const EGSPD::TGuildId & guildId )
{
	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (7)");
		return false;
	}
	return ( guildId == _Container->getGMGuild() );
}

//----------------------------------------------------------------------------
void CGuildManager::createGuild(CGuildCharProxy & proxy,const ucstring & guildName,const uint64 &icon,const ucstring & description)
{
	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (8)");
		return;
	}

	// cancel the afk state
	proxy.cancelAFK();
	// check if interlocutor is ok
	CCreature * bot = proxy.getInterlocutor();
	if ( !bot )
	{
		nlwarning("<GUILD>%s Invalid interlocutor",proxy.getId().toString().c_str() );
		return;
	}
	if ( ( bot->getBotChatProgram() & ( (uint32)1 << (uint32)BOTCHATTYPE::CreateGuildFlag ) ) == 0)
	{
		nlwarning("<GUILD>%s bot %s is not a guild creator",proxy.getId().toString().c_str(),bot->getId().toString().c_str());
		return;
	}

	// player must not have a guild
	CGuildMemberModule * module = NULL;
	if ( proxy.getModule( module ) )
	{
		proxy.sendSystemMessage("GUILD_ALREADY_IN_A_GUILD");
		return;
	}

	// check guild name and description
	if ( !checkGuildStrings(proxy,guildName,description) )
		return;

	// check user money
	if ( proxy.getMoney() < GuildCreationCost.get() )
	{
		SM_STATIC_PARAMS_1(params,STRING_MANAGER::integer);
		params[0].Int = GuildCreationCost;
		proxy.sendSystemMessage( "GUILD_NOT_ENOUGH_MONEY",params );
		return;
	}

	// get a valid guild id
	uint32 guildId = getFreeGuildId();

	// boris 2006 : validate the name with the Shard unifier and store guild creation info when SU returns
	// store the creation data
	TPendingGuildCreate pgc;
	pgc.CreatorChar = proxy.getId();
	pgc.GuildName = guildName;
	pgc.Description = description;
	pgc.Icon = icon;

	_PendingGuildCreates.insert(make_pair(guildId, pgc));

	IShardUnifierEvent::getInstance()->validateGuildName(guildId, guildName);

}


void CGuildManager::createGuildStep2(uint32 guildId, const ucstring &guildName, CHARSYNC::TCharacterNameResult result)
{
	// remove pending guild info
	TPendingGuildCreateInfos::iterator it(_PendingGuildCreates.find(guildId));
	BOMB_IF(it == _PendingGuildCreates.end(), "CGuildManager::createGuildStep2 : can't find guild "<<guildId<<" in the pending guild creation", return);
	TPendingGuildCreate pgc = it->second;
	_PendingGuildCreates.erase(it);

	if (result != CHARSYNC::TCharacterNameResult::cnr_ok)
	{
		// no valid name
		return;
	}
	TLogContext_Item_CreateGuild itemContext(pgc.CreatorChar);

	// Rebuild a proxy
	ICharacter *character = ICharacter::getInterface(pgc.CreatorChar, true);
	if (character == NULL)
	{
		nlinfo("CGuildManager::createGuildStep2 failed to retrieve creator character %s", pgc.CreatorChar.toString().c_str());
		return;
	}

	CGuildCharProxy proxy(character->getCharacter());

	// recheck if interlocutor is ok
	CCreature * bot = proxy.getInterlocutor();
	if ( !bot )
	{
		nlwarning("<GUILD>%s Invalid interlocutor",proxy.getId().toString().c_str() );
		return;
	}

	// check user place
	uint16 place = proxy.getMainPlace();
	if (  place == 0xFFFF )
	{
		nlwarning( "<GUILD>%s has no valid place",proxy.getId().toString().c_str() );
		return;
	}

	proxy.sendSystemMessage( "GUILD_CREATED" );
	proxy.spendMoney( GuildCreationCost.get() );

	// create the guild through PDS
	CGuild * guild = EGS_PD_CAST<CGuild*>( EGSPD::CGuildPD::create(guildId) );
	EGS_PD_AST( guild );
	guild->initNonPDMembers();
//	guild->postCreate();
	_Container->setGuilds( guild );

	EGSPD::CGuildFameContainerPD * fameCont = EGSPD::CGuildFameContainerPD::create(CEntityId(RYZOMID::guild,guildId));
	EGS_PD_AST(fameCont);
	guild->setFameContainer( fameCont );

	/// register guild in other system
	guild->registerGuild();

	// inform the SU that a new guild now exist (to update the ring database).
	if (IShardUnifierEvent::getInstance())
		IShardUnifierEvent::getInstance()->addGuild(guildId, guildName);

	// set initial guild fames
	guild->setStartFameAndAllegiance( proxy.getId() );

	proxy.setGuildId(guildId);

	// set the guild initial attributes
	guild->setIcon(pgc.Icon);
//	guild->setVillage(place);
	guild->setRace(bot->getRace());
	guild->setCreationDate( CTickEventHandler::getGameCycle() );

//	for ( uint i = 0; i < EGSPD::CSPType::EndSPType; i++ )
//	{
//		guild->setRolemastersValid( (EGSPD::CSPType::TSPType)i, false );
//	}

	// init the guild strings
	guild->setName(guildName);
	guild->setDescription(pgc.Description);
//	_GuildsAwaitingString.insert( std::make_pair( guildName, guild->getId() ) );
//	_GuildsAwaitingString.insert( std::make_pair( pgc.Description, guild->getId() ) );
//	NLMISC::CEntityId stringEId = guild->getEId();
//	stringEId.setType( RYZOMID::guildName );
//	EGSPD::PDSLib.getStringManager().addString( stringEId, guildName );
//	stringEId.setType( RYZOMID::guildDescription );
//	EGSPD::PDSLib.getStringManager().addString( stringEId, pgc.Description );

	// create a new guild member
	CGuildMember * memberCore = guild->newMember( proxy.getId() );
//	guild->decGradeCount( EGSPD::CGuildGrade::Member );
//	guild->incGradeCount( EGSPD::CGuildGrade::Leader );
	nlassert( memberCore );
	memberCore->setEnterTime( CTickEventHandler::getGameCycle() );
	STOP_IF(!guild->setMemberGrade(memberCore, EGSPD::CGuildGrade::Leader), "Failed to set grade to leader for new guild"<<guildId<<" creator "<<character->getId().toString());
//	memberCore->setMemberGrade( EGSPD::CGuildGrade::Leader );

	// create the leader module to init properties
//	CGuildLeaderModule * leader = new CGuildLeaderModule( proxy, memberCore );
//	guild->setMemberOnline( memberCore, proxy.getId().getDynamicId()  );

	// broadcast the new guild info
	IGuildUnifier::getInstance()->guildCreated(guild);

	// close guild creation interface
	PlayerManager.sendImpulseToClient( proxy.getId(),"GUILD:ABORT_CREATION" );

	/// end current bot chat
	proxy.endBotChat();
	proxy.updateTarget();
	_ExistingGuildNames.insert( NLMISC::toLower( guild->getName().toString() ) );

	// ask the client to open it's guild interface
	PlayerManager.sendImpulseToClient( proxy.getId(),"GUILD:OPEN_GUILD_WINDOW" );
}



//----------------------------------------------------------------------------
void CGuildManager::deleteGuild(uint32 id)
{
	nldebug("CGuildManager::deleteGuild : deleting guild %s", guildIdToString(id).c_str());

	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (9)");
		return;
	}

	CGuild * guild = EGS_PD_CAST<CGuild*>( _Container->getGuilds( id ) );
	if ( !guild )
	{
		nlwarning("<GUILD> invalid guild %s", guildIdToString(id).c_str());
		return;
	}
	// delete and remove invitations ( special loop : increment i only if we dont erase )
	for ( uint i = 0; i < _Invitations.size();  )
	{
		nlassert( _Invitations[i] );
		if ( _Invitations[i]->getGuild() == guild )
		{
			delete _Invitations[i];
			_Invitations[i] = _Invitations.back();
			_Invitations.pop_back();
		}
		else
			++i;
	}

	string name = guild->getName().toUtf8();
	if (!guild->isProxy())
	{
		CMailForumValidator::removeGuild(name);

		CStatDB::getInstance()->removeGuild(id);
	}

	_ExistingGuildNames.erase( NLMISC::toLower( guild->getName().toString() ) );
	guild->unregisterGuild();

	if (!guild->isProxy())
	{
		// not a proxy guild, delete the guild file in backup
		string fileName = NLMISC::toString("guilds/guild_%05u.bin", id & 0x000fffff);

		Bsi.deleteFile( fileName );

		IGuildUnifier::getInstance()->guildDeleted(id);

		// inform the SU that the guild is deleted
		if (IShardUnifierEvent::getInstance())
			IShardUnifierEvent::getInstance()->removeGuild(id);
	}

	_Container->deleteFromGuilds(id);

//	if ( _HighestGuildId == id )
//	{
//		--_HighestGuildId;
//		while( _HighestGuildId != 0 )
//		{
//			if ( _Container->getGuilds( _HighestGuildId ) )
//				break;
//			--_HighestGuildId;
//		}
//	}
//	else
//		_FreeGuildIds.insert(id);
}

//----------------------------------------------------------------------------
void CGuildManager::characterDeleted( CCharacter & user )
{
	if ( !_Container )
	{
		nlwarning("<GUILD> container not initialized (10)");
		return;
	}
	if ( user.getGuildId() == 0 )
		return;

	CGuild * guild = EGS_PD_CAST<CGuild*>( _Container->getGuilds( user.getGuildId() ) );
	if ( guild == NULL )
	{
		nlwarning("<GUILD>invalid guild id %s", guildIdToString(user.getGuildId()).c_str() );
		return;
	}

	if (guild->isProxy())
	{
		// for proxyfied guild, we wait the action of the origin shard
		return;
	}

	CGuildMember* userMember = EGS_PD_CAST<CGuildMember*>( guild->getMembers( user.getId() ) );
	if ( !userMember )
	{
		nlwarning("<GUILD>'%s' is not in guild %s", user.getId().toString().c_str(), guildIdToString(user.getGuildId()).c_str() );
		return;
	}

	if ( userMember->getGrade() == EGSPD::CGuildGrade::Leader )
	{
		// the leader quits : find a successor
		CGuildMember * successor = NULL;
		// best successor is the member with best grade. If more than 1 user fits, take the older in the guild
		for (map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator it = guild->getMembersBegin();
			it != guild->getMembersEnd();
			++it  )
		{
			CGuildMember * member = EGS_PD_CAST<CGuildMember*>( (*it).second );
			EGS_PD_AST( member );
			// ignore current leader
			if ( member->getGrade() == EGSPD::CGuildGrade::Leader )
				continue;
			// check if the current member is the successor
			if ( successor == NULL ||
				member->getGrade() < successor->getGrade() ||
				( member->getGrade() == successor->getGrade() && member->getEnterTime() < successor->getEnterTime() ) )
			{
				successor = member;
			}
		}

		// if the guild is still valid, set the successor as leader
		if ( successor )
		{
			guild->decGradeCount( successor->getGrade() );
			guild->incGradeCount( EGSPD::CGuildGrade::Leader );
			successor->setMemberGrade( EGSPD::CGuildGrade::Leader );

			// tell guild
			SM_STATIC_PARAMS_1( params,STRING_MANAGER::player );
			params[0].setEIdAIAlias( successor->getIngameEId(), CAIAliasTranslator::getInstance()->getAIAlias(successor->getIngameEId()) );

			guild->sendMessageToGuildMembers("GUILD_NEW_LEADER",params );

			// If the player is online, the module must be recreated. Do as the reference was destroyed
			CGuildMemberModule * successorModule = NULL;
			if ( successor->getReferencingModule(successorModule) )
			{
				CGuildCharProxy successorProxy;
				successorModule->getProxy(successorProxy);
				successor->removeReferencingModule(successorModule);
				successorModule->onReferencedDestruction();
				// set the member offline, to reset database
				::IModule * moduleTarget = CGuildMemberModule::createModule(successorProxy,successor);
				guild->setMemberClientDB( successor );
				MODULE_AST(moduleTarget);
			}
		}
		userMember->setMemberGrade(EGSPD::CGuildGrade::Member);
		guild->incGradeCount( EGSPD::CGuildGrade::Member );
		guild->decGradeCount( EGSPD::CGuildGrade::Leader );
	}

	guild->deleteMember( userMember );
	if ( guild->getMembersBegin() == guild->getMembersEnd() )
	{
		CGuildManager::getInstance()->deleteGuild(guild->getId());
		return;
	}
	SM_STATIC_PARAMS_1( params,STRING_MANAGER::player);
	params[0].setEIdAIAlias( user.getId(), CAIAliasTranslator::getInstance()->getAIAlias(user.getId()) );
	guild->sendMessageToGuildMembers("GUILD_USER_DELETED",params);
}


//----------------------------------------------------------------------------
void CGuildManager::callback(const CFileDescriptionContainer& fileList)
{
//	if (fileList.empty())
//	{
//		// no guild to load, set guild as ready
//		IShardUnifierEvent::getInstance()->registerLoadedGuildNames(vector<CHARSYNC::CGuildInfo>());
//
//		// notify statistical DB that guilds are loaded
//		CStatDB::getInstance()->cbGuildsLoaded();
//
//		// warn the guild unifier that guilds are ready to dispatch
//		IGuildUnifier::getInstance()->broadcastAllGuilds();
//
//		_GuildLoaded = true;
//
//		IService::getInstance()->clearCurrentStatus("Loading Guilds");
//
//		return;
//	}

	for (uint i=0; i<fileList.size(); ++i)
	{
		const CFileDescription &fd = fileList[i];

		// extract the local guild id.
		string::size_type pos = fd.FileName.find("guild_");
		BOMB_IF(pos == string::npos, "Invalid guild file "<<fd.FileName<<" returned by BS : can't find 'guild_' inside", continue);
		BOMB_IF(pos+6+5 >= fd.FileName.size(), "Invalid guild file "<<fd.FileName<<" returned by BS : not enough character after 'guild_'", continue);

		TGuildId guildId;
		NLMISC::fromString(fd.FileName.substr(pos+6, 5), guildId);

		if (_GuildToLoad.find(guildId) != _GuildToLoad.end())
		{
			// check most recent timestamp
			if (_GuildToLoad[guildId].Timestamp < fd.FileTimeStamp)
			{
				// replace with this file
				_GuildToLoad[guildId].FileName = fd.FileName;
			}
		}
		else
		{
			_GuildToLoad[guildId].FileName = fd.FileName;
			_GuildToLoad[guildId].Timestamp = fd.FileTimeStamp;
		}
	}

//	// ask all the guild file to the backup service
//	TGuildToLoad::iterator first(_GuildToLoad.begin()), last(_GuildToLoad.end());
//	for (; first != last; ++first)
//	{
//
//		loadGuild(first->second.FileName);
//	}
}

CGuild *lastLoadedGuild = NULL;

//----------------------------------------------------------------------------
void CGuildManager::loadGuild(const std::string &fileName)
{
	// create the container if it was not loaded
	if ( _Container == NULL )
		_Container = EGSPD::CGuildContainerPD::create( 1 );

	nldebug("Loading guild file '%s' from BS", fileName.c_str());
	Bsi.syncLoadFile(fileName, new CGuildFileCb());

	// only register the new guild
	registerGuildAfterLoading(lastLoadedGuild);

	// update unified guild info
	std::vector<CHARSYNC::CGuildInfo> guildInfos;
	CHARSYNC::CGuildInfo gi;
	gi.setGuildId(lastLoadedGuild->getId());
	gi.setGuildName(lastLoadedGuild->getName());

	guildInfos.push_back(gi);

	IShardUnifierEvent::getInstance()->registerLoadedGuildNames(guildInfos);

}


//----------------------------------------------------------------------------
void CGuildManager::callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	const uint len = (uint)strlen("guild_XXXXX.ext");

	string file = CFile::getFilename(fileDescription.FileName);

	string::size_type pos = fileDescription.FileName.find("guild_");
	BOMB_IF(pos == string::npos, "Invalid guild file "<<fileDescription.FileName<<" send by BS : can't find 'guild_' inside", return);
	BOMB_IF(pos+6+5 >= fileDescription.FileName.size(), "Invalid guild file "<<fileDescription.FileName<<" send by BS : not enough character after 'guild_'", return);

	TGuildId guildId;
	NLMISC::fromString(fileDescription.FileName.substr(pos+6, 5), guildId);

	BOMB_IF(guildId == 0, "Invalid guild file name '"<<fileDescription.FileName<<"' : can't found a valid guild id", return);

	// make the id 'shard unique'
	guildId = guildId | IService::getInstance()->getShardId() << 20;

	BOMB_IF(getGuildFromId(guildId) != NULL, "The guild "<<guildIdToString(guildId)<<") is already loaded", _GuildToLoad.erase(guildId & 0xfffff); return);

	CGuild *guild = NULL;
	// check if we have a guild file
	if ( file.size() == len &&
		(file.find( "bin" ) == len - 3 )
		 || file.find( "txt" ) == len - 3 )
	{
		// we have a certainly a guild file

		nldebug("Reading guild from file '%s' received from BS as guild %s", fileDescription.FileName.c_str(), guildIdToString(guildId).c_str());

		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.fromBuffer(dataStream);

		guild = EGS_PD_CAST<CGuild*>( EGSPD::CGuildPD::create(guildId) );
		EGS_PD_AST(guild);
		// store it as loast loaded guild
		lastLoadedGuild = guild;

		// hack the pdr to update the guild id
		CPersistentDataTree tree;
		BOMB_IF(!tree.readFromPdr(pdr), "Failed to convert pdr into tree for guild file '"<<fileDescription.FileName<<"'", _GuildToLoad.erase(guildId & 0xfffff); return);
		CPersistentDataTreeNode *node = tree.getNode("Id");
		BOMB_IF(node == NULL, "Invalid content in guild file '"<<fileDescription.FileName<<"', could not find token 'Id'", _GuildToLoad.erase(guildId & 0xfffff); return);
		node->setValue(toString("%u", guildId));
		pdr.clear();
		BOMB_IF(!tree.writeToPdr(pdr), "Failed to convert pdr tree back into pdr for guild file '"<<fileDescription.FileName<<"'", _GuildToLoad.erase(guildId & 0xfffff); return);

		guild->initNonPDMembers();
		guild->apply( pdr );

		// check the fame values (because I have found some bad values in a guild file
		const CGuildFameContainerPD *fameCont = guild->getFameContainer();
		const std::map<NLMISC::CSheetId, CFameContainerEntryPD> &fames = fameCont->getEntries();
		std::map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator first(fames.begin()), last(fames.end());
		for (; first != last; ++first)
		{
			const CFameContainerEntryPD &fame = first->second;
			if (fame.getLastFameChangeTrend() >= CFameTrend::Unknown)
			{
				nlwarning("In guild file '%s', fame trend value for faction '%s' is invalid, setting it to steady",
					file.c_str(),
					first->first.toString().c_str());

				const_cast<CFameContainerEntryPD &>(fame).setLastFameChangeTrend(CFameTrend::FameSteady);
			}

		}


		// add the guild to the container
		_Container->setGuilds( guild );
	}
	else
	{
		BOMB("Invalid guild file '"<<fileDescription.FileName<<"'", return);
	}

	// remove the guild from the guild to load
	_GuildToLoad.erase(guildId & 0xfffff);

//	if (_GuildToLoad.empty())
//	{
//		// we have finished to receive the guild, do the final work
//		if (!_GuildLoaded)
//		{
//			nldebug("All guilds finished to read.");
//
//		}
//
//		// check members consistency
//		std::map<EGSPD::TGuildId, EGSPD::CGuildPD*>::iterator first(_Container->getGuildsBegin()), last(_Container->getGuildsEnd());
//		if (first != last)
//		{
//			do
//			{
//				--last;
//				EGSPD::CGuildPD *guild = last->second;
//
//				checkMemberConsistency(EGS_PD_CAST<CGuild*>(guild));
//			} while (first != last);
//		}
//
//
//		if (!_GuildLoaded)
//		{
//			// register all guild in other system
//			map<uint32,EGSPD::CGuildPD*>::iterator lastIt = _Container->getGuildsBegin();
//			for (map<uint32,EGSPD::CGuildPD*>::iterator it = lastIt;  it != _Container->getGuildsEnd(); ++it )
//			{
//				// 0 is an invalid id
//				nlassert( (*it).first != 0 );
//				CGuild * guild = EGS_PD_CAST<CGuild*>( (*it).second );
//				EGS_PD_AST(guild);
//
//				registerGuildAfterLoading(guild);
//			}
//
//			// load outposts save files here (all guilds are loaded)
//			nlinfo("Loading outposts from save files...");
//			COutpostManager::getInstance().loadOutpostSaveFiles();
//
//			// Send the guild list to SU to synchronize the ring database and name manager
//			// (and eventualy rename some conflicting guilds)
//			{
//				std::vector<CHARSYNC::CGuildInfo> guildInfos;
//
//				fillGuildInfos(guildInfos);
//
//				IShardUnifierEvent::getInstance()->registerLoadedGuildNames(guildInfos);
//			}
//
//			// notify statistical DB that guilds are loaded
//			CStatDB::getInstance()->cbGuildsLoaded();
//
//			// warn the guild unifier that guilds are ready to dispatch
//			IGuildUnifier::getInstance()->broadcastAllGuilds();
//
//
//			IService::getInstance()->clearCurrentStatus("Loading Guilds");
//
//			_GuildLoaded = true;
//
//			// check guild member list against the eid translator
//			if (IShardUnifierEvent::getInstance()->isEidTranslatorInitilazed())
//			{
//				checkGuildMemberLists();
//			}
//
//			// check outpost attribution against guild list
//			COutpostManager::getInstance().validateOutpostOwners();
//		}
//		else
//		{
//			// only register the new guild
//			registerGuildAfterLoading(guild);
//
//			// update unified guild info
//			std::vector<CHARSYNC::CGuildInfo> guildInfos;
//			CHARSYNC::CGuildInfo gi;
//			gi.setGuildId(guild->getId());
//			gi.setGuildName(guild->getName());
//
//			guildInfos.push_back(gi);
//
//			IShardUnifierEvent::getInstance()->registerLoadedGuildNames(guildInfos);
//		}
//
//	}
//	else if (!_GuildLoaded)
//	{
		nldebug("Waiting for %u guild to load before guilds ready", _GuildToLoad.size());
//	}
}

//----------------------------------------------------------------------------
void CGuildManager::loadGuilds()
{
	if (_GuildLoaded)
		return;

	IService::getInstance()->setCurrentStatus("Loading Guilds");

	// create the container if it was not loaded
	if ( _Container == NULL )
		_Container = EGSPD::CGuildContainerPD::create( 1 );

	// request the files list to the BS
	std::vector<CBackupFileClass> classes;

	CBackupFileClass bfc;
//	bfc.Patterns.push_back("guild_*.txt");
	bfc.Patterns.push_back("guild_*.bin");

	classes.push_back(bfc);

	nldebug("CGuildManager : sending guild files request to the BS...");
	Bsi.syncLoadFileClass("guilds", classes, new CGuildFileClassCb());
	nldebug("CGuildManager : Loading %u guild...", _GuildToLoad.size());
	vector<string> fileNames;
	for(TGuildToLoad::iterator it(_GuildToLoad.begin()); it != _GuildToLoad.end(); ++it)
		fileNames.push_back(it->second.FileName);
	Bsi.syncLoadFiles(fileNames, new CGuildFileCb());

	///////////////////////////////////////////////////////////////////////////
	// do post load treatment
	// we have finished to receive the guild, do the final work
	if (!_GuildLoaded)
	{
		nldebug("All guilds finished to read.");

	}

	// check members consistency
	std::map<EGSPD::TGuildId, EGSPD::CGuildPD*>::iterator first(_Container->getGuildsBegin()), last(_Container->getGuildsEnd());
	if (first != last)
	{
		do
		{
			--last;
			EGSPD::CGuildPD *guild = last->second;

			checkMemberConsistency(EGS_PD_CAST<CGuild*>(guild));
		} while (first != last);
	}


	// register all guild in other system
	map<uint32,EGSPD::CGuildPD*>::iterator lastIt = _Container->getGuildsBegin();
	for (map<uint32,EGSPD::CGuildPD*>::iterator it = lastIt;  it != _Container->getGuildsEnd(); ++it )
	{
		// 0 is an invalid id
		nlassert( (*it).first != 0 );
		CGuild * guild = EGS_PD_CAST<CGuild*>( (*it).second );
		EGS_PD_AST(guild);

		registerGuildAfterLoading(guild);
	}

	// load outposts save files here (all guilds are loaded)
	nlinfo("Loading outposts from save files...");
	COutpostManager::getInstance().loadOutpostSaveFiles();

	// Send the guild list to SU to synchronize the ring database and name manager
	// (and eventualy rename some conflicting guilds)
	if (DontUseSU.get() == 0)
	{
		std::vector<CHARSYNC::CGuildInfo> guildInfos;

		fillGuildInfos(guildInfos);

		IShardUnifierEvent::getInstance()->registerLoadedGuildNames(guildInfos);
	}

	// notify statistical DB that guilds are loaded
	CStatDB::getInstance()->cbGuildsLoaded();

	// warn the guild unifier that guilds are ready to dispatch
	IGuildUnifier::getInstance()->broadcastAllGuilds();


	IService::getInstance()->clearCurrentStatus("Loading Guilds");

	_GuildLoaded = true;

	// check guild member list against the eid translator
	if ((DontUseSU.get() == 0) && IShardUnifierEvent::getInstance()->isEidTranslatorInitilazed())
	{
		checkGuildMemberLists();
	}

	// check outpost attribution against guild list
	COutpostManager::getInstance().validateOutpostOwners();

	nldebug("CGuildManager : finished loading guilds");
}

//----------------------------------------------------------------------------
void CGuildManager::registerGuildAfterLoading(CGuild *guildToRegister)
{
	if (guildToRegister != NULL)
	{
		// register the guild in subsystem
		guildToRegister->registerGuild();

//		// here we may already have the guild string ID. If so, update the guild names, otherwise flag the guild as 'waiting for a string"
//		if ( guildToRegister->getNameId() != RY_PDS::PD_INVALID_STRING_PID && guildToRegister->getNameId() != 0 )
//			guildToRegister->onGuildStringUpdated();
//		else
//		{
//			const ucstring & str = guildToRegister->getName();
//			_GuildsAwaitingString.insert( make_pair( str, guildToRegister->getId() ) );
//		}
//		// do the same for descriptions
//		if ( guildToRegister->getDescriptionId() != RY_PDS::PD_INVALID_STRING_PID  && guildToRegister->getDescriptionId() != 0 )
//			guildToRegister->onGuildStringUpdated();
//		else
//		{
//			const ucstring & str = guildToRegister->getDescription();
//			_GuildsAwaitingString.insert( make_pair( str, guildToRegister->getId() ) );
//		}

		_ExistingGuildNames.insert( NLMISC::toLower( guildToRegister->getName().toString() ) );
	}
}

//----------------------------------------------------------------------------
void CGuildManager::checkMemberConsistency(CGuild *guildToCheck)
{
	if (guildToCheck != NULL)
	{
		set<EGSPD::TCharacterId>	memberToRemove;
		std::map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::const_iterator first(guildToCheck->getMembersBegin()), last(guildToCheck->getMembersEnd());
		for (; first != last; ++first)
		{
			// check that no other previously loaded guild pretend having this char as member
			EGSPD::TGuildId gid = getCharGuildAssoc(first->first);
			if (gid != 0)
			{
				nlwarning("Guild %s says it contains member %s, but he is in guild %u, removing from guild %u",
					guildIdToString(guildToCheck->getId()).c_str(),
					first->first.toString().c_str(),
					gid,
					guildToCheck->getId());

				memberToRemove.insert(first->first);
			}
			else
			{
				// store the char to guild assoc
				storeCharToGuildAssoc(first->first, guildToCheck->getId());
			}
		}

		// remove member that are in other guild
		while (!memberToRemove.empty())
		{
			guildToCheck->deleteFromMembers(*(memberToRemove.begin()));
			memberToRemove.erase(memberToRemove.begin());
		}
	}
}

//----------------------------------------------------------------------------
bool CGuildManager::checkGuildStrings(CGuildCharProxy & proxy,const ucstring & name, const ucstring & description)
{
	if( name.empty() )
		return false;

	// check if name already exists in the player list
	if ( NLMISC::CEntityIdTranslator::getInstance()->entityNameExists( name ) )
	{
		proxy.sendSystemMessage("GUILD_NAME_ALREADY_EXISTS");
		return false;
	}
	// check if name already exists in the guild list
	if ( _ExistingGuildNames.find( NLMISC::toLower( name.toString() ) ) != _ExistingGuildNames.end() )
	{
		proxy.sendSystemMessage("GUILD_NAME_ALREADY_EXISTS");
		return false;
	}


	// check name and desc syntax
	if (name.size() < 3 || name.size() > 50 )
	{
		proxy.sendSystemMessage("GUILD_NAME_BAD_SIZE");
		return false;
	}

	if ( !isalpha(name[0]) || !isalpha(name[name.size()-1]) )
	{
		proxy.sendSystemMessage("GUILD_NAME_BAD_CHAR");
		return false;
	}

	bool prevBlank = false;
	for (uint i = 0; i < name.size(); i++)
	{
		if ( name[i] == (uint16)' ' )
		{
			if ( prevBlank )
			{
				proxy.sendSystemMessage("GUILD_NAME_BAD_CHAR");
				return false;
			}
			prevBlank = true;
		}
		else
		{
			prevBlank = false;
			if (!isalpha (name[i]))
			{
				proxy.sendSystemMessage("GUILD_NAME_BAD_CHAR");
				return false;
			}
		}
	}

	if ( description.size() > 200 )
	{
		proxy.sendSystemMessage("GUILD_DESC_BAD_SIZE");
		return false;
	}

	/// check if name and description are ethically	acceptable
	const uint size = (uint)CEntityIdTranslator::getInstance()->getInvalidNames().size();
	string nameStr = CEntityIdTranslator::getInstance()->getRegisterableString( name );
	string descriptionStr = CEntityIdTranslator::getInstance()->getRegisterableString( description );
	for (uint i = 0; i < CEntityIdTranslator::getInstance()->getInvalidNames().size(); i++)
	{
		if(testWildCard(nameStr, CEntityIdTranslator::getInstance()->getInvalidNames()[i]))
		{
			proxy.sendSystemMessage("GUILD_NAME_BAD_WORD");
			return false;
		}
		if(testWildCard(descriptionStr, CEntityIdTranslator::getInstance()->getInvalidNames()[i]))
		{
			proxy.sendSystemMessage("GUILD_DESCRIPTION_BAD_WORD");
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------------
void CGuildManager::updateGuildMembersStringIds()
{
	H_AUTO( CGuildManagerUpdate );

	// if guild data are not loaded yet
	if ( !IOSIsUp )
	{
		_UpdateGuildMembersStringIds = true;
		return;
	}

	if ( !_Container )
		return;

	map<uint32,EGSPD::CGuildPD*>::iterator it;
	for (it = _Container->getGuildsBegin(); it != _Container->getGuildsEnd(); ++it)
	{
		// 0 is an invalid id
		nlassert( (*it).first != 0 );
		CGuild * guild = EGS_PD_CAST<CGuild*>( (*it).second );
		EGS_PD_AST(guild);

		guild->updateMembersStringIds();
	}

	_UpdateGuildMembersStringIds = false;
}


// fill guild info descriptor with all local guilds
void CGuildManager::fillGuildInfos(std::vector<CHARSYNC::CGuildInfo> &guildInfos)
{
	map<uint32,EGSPD::CGuildPD*>::const_iterator first( getGuildContainer()->getGuildsBegin()), last(getGuildContainer()->getGuildsEnd());
	for (; first != last; ++first)
	{
		CGuild *guild = static_cast<CGuild*>(first->second);

		if (guild->isProxy())
			continue;

		CHARSYNC::CGuildInfo gi;
		gi.setGuildId(guild->getIdWrap());
		gi.setGuildName(guild->getNameWrap().toUtf8());

		guildInfos.push_back(gi);
	}
}

/// Check all guild member lists against the entity translator
void CGuildManager::checkGuildMemberLists()
{
	if (_Container == NULL)
	{
		/// guild are not initialized yet, can't check now
		return;
	}


	TGuildId	lastCheckedGuild = 0;

restartGuildLoop:
	CEntityIdTranslator *eidt = CEntityIdTranslator::getInstance();
	map<uint32,EGSPD::CGuildPD*>::const_iterator first( getGuildContainer()->getGuilds().lower_bound(lastCheckedGuild)), last(getGuildContainer()->getGuilds().end());
	for (; first != last; ++first)
	{
		lastCheckedGuild = first->first;
		CGuild * guild = static_cast<CGuild *>(first->second);

		if (guild->isProxy())
			continue;

		TCharacterId lastCheckedMember(uint64(0));
restartMemberLoop:
		std::map<TCharacterId, CGuildMemberPD*>::const_iterator f2(guild->getMembers().lower_bound(lastCheckedMember)), l2(guild->getMembers().end());
		for (; f2 != l2; ++f2)
		{
			lastCheckedMember = f2->first;

			const CEntityId &eid = f2->first;
			bool removeMember = false;

			if (!eidt->isEntityRegistered(eid))
			{
				// the entity is not registered
				removeMember = true;
				nldebug("checkGuildMemberList : removing char %s from guild %u because entity not registered", eid.toString().c_str(), guild->getId());
			}
			else if (eidt->getEntityShardId(eid) != 0)
			{
				if (!AllowCharsFromAllSessions && eidt->getEntityShardId(eid) != IService::getInstance()->getShardId())
				{
					// the entity do not belong to this shard
					removeMember = true;
					nldebug("checkGuildMemberList : removing char %s from guild %u because char is on shard %u and guild on shard %u", eid.toString().c_str(), guild->getId(), eidt->getEntityShardId(eid), IService::getInstance()->getShardId());
				}
			}
			if (removeMember)
			{
				// this entity no more exist or is not hosted on this shard ! remove it from the guild
				if (guild->getMembers().size() == 1)
				{
					// this is the last member, we need to restart the guild loop
					guild->removeMember(eid);
					goto restartGuildLoop;
				}
				else
				{
					// only a member, restart the member loop
					guild->removeMember(eid);
					goto restartMemberLoop;
				}
			}
		}
	}
}

// A character connect/disconnect on another shard, update the online tags
void	CGuildManager::characterConnectionEvent(const NLMISC::CEntityId &eid, bool online)
{
	// iterate over all guild, for each look the member list and update online state if it is the concerned character

	if (_Container == NULL)
	{
		/// guild are not initialized yet, can't check now
		return;
	}

	CEntityIdTranslator *eidt = CEntityIdTranslator::getInstance();
	map<uint32,EGSPD::CGuildPD*>::const_iterator first( getGuildContainer()->getGuilds().begin()), last(getGuildContainer()->getGuilds().end());
	for (; first != last; ++first)
	{
		CGuild * guild = static_cast<CGuild *>(first->second);

		std::map<TCharacterId, CGuildMemberPD*>::const_iterator it(guild->getMembers().find(eid));
		if (it != guild->getMembers().end())
		{
			// we found it !
			CGuildMember *member = EGS_PD_CAST<CGuildMember*>(it->second);
			BOMB_IF(member == NULL, "Can't convert CGuildMemberPD into CGuildMember, guild "<<guildIdToString(guild->getId())<<", member "<<it->first.toString(), return);

			// force an update of the clientdb content, this will update the 'online' state
			guild->setMemberClientDB(member);

			// no need to continue
			return;
		}
	}
}


//----------------------------------------------------------------------------
//void CGuildManager::rebuildCliendDB()
//{
//	map<uint32,EGSPD::CGuildPD*>::iterator it;
//	for (it = _Container->getGuildsBegin(); it != _Container->getGuildsEnd(); ++it)
//	{
//		// 0 is an invalid id
//		nlassert( (*it).first != 0 );
//		CGuild * guild = EGS_PD_CAST<CGuild*>( (*it).second );
//		EGS_PD_AST(guild);
//
//		guild->rebuildCliendDB();
//	}
//}

//----------------------------------------------------------------------------
const EGSPD::CGuildContainerPD *CGuildManager::getGuildContainer() const
{
	return _Container;
}

//----------------------------------------------------------------------------
/// A player entity have been removed from eid translator, check all guild member list
void CGuildManager::playerEntityRemoved(const NLMISC::CEntityId &eid)
{
	if(_Container == 0)
		return;

	// loop on all guilds
	std::map<TGuildId, CGuildPD*>::const_iterator first(_Container->getGuilds().begin()), last(_Container->getGuilds().end());
	for (; first != last; ++first)
	{
		CGuild *guild = static_cast<CGuild*>(first->second);

		// do not manage proxified guilds
		if (guild->isProxy())
			continue;

		// loop on all members
		std::map<TCharacterId, CGuildMemberPD*>::const_iterator f2(guild->getMembers().begin()), l2(guild->getMembers().end());
		for (; f2 != l2; ++f2)
		{
			if (f2->first == eid)
			{
				// the removed entity is member of this guild, remove it
				nldebug("Guild %u, removing deleted member %s", first->first, f2->first.toString().c_str());
				guild->removeMember(eid);
				// a character can't be in more than one guild
				return;
			}
		}
	}
}


//----------------------------------------------------------------------------
void CGuildManager::storeCharToGuildAssoc(const NLMISC::CEntityId &charId, EGSPD::TGuildId guildId)
{
	TCharToGuildCont::iterator it(_CharToGuildAssoc.find(charId));

	BOMB_IF(it != _CharToGuildAssoc.end(), "Storing char assoc with guild, but char already assoc to a guild", ;);
	if (it != _CharToGuildAssoc.end())
	{
		nlwarning("Storing char %s assoc with guild %s, but already assoc with guild %s, replacing !",
			charId.toString().c_str(),
			guildIdToString(guildId).c_str(),
			guildIdToString(it->second).c_str());
		it->second = guildId;
	}
	else
	{
		_CharToGuildAssoc.insert(make_pair(charId, guildId));
	}
}

//----------------------------------------------------------------------------
void CGuildManager::removeCharToGuildAssoc(const NLMISC::CEntityId &charId, EGSPD::TGuildId guildId)
{
	TCharToGuildCont::iterator it(_CharToGuildAssoc.find(charId));

	BOMB_IF(it == _CharToGuildAssoc.end(), "removing char assoc with guild, but char not assoc to a guild", return);
	BOMB_IF(it->second != guildId, "removing char assoc with guild, but char is assoc with another guild", return);

	_CharToGuildAssoc.erase(it);
}

//----------------------------------------------------------------------------
EGSPD::TGuildId CGuildManager::getCharGuildAssoc(const NLMISC::CEntityId &charId) const
{
	TCharToGuildCont::const_iterator it(_CharToGuildAssoc.find(charId));

	if (it != _CharToGuildAssoc.end())
	{
		return it->second;
	}
	else
	{
		return 0;
	}
}


NLMISC_CLASS_COMMAND_IMPL(CGuildManager, unloadGuild)
{
	if (args.size() != 1)
		return false;

	TGuildId guildId = parseGuildId(args[0]);
	if (guildId == 0)
	{
		log.displayNL("Invalid guild id '%s'", args[0].c_str());

		return true;
	}

	CGuild * guild = getGuildFromId(guildId);
	if (guild == NULL)
	{
		log.displayNL("Can't find guild with id %s", guildIdToString(guildId).c_str());
		return true;
	}

	if (guild->isProxy())
	{
		log.displayNL("Can't unload a foreign guild %s", guildIdToString(guildId).c_str());
		return true;
	}

	// force a save of the guild
	saveGuild(guild);
	// a little fake : we make the guild 'foreign' to skip deletion of the file
	guild->setProxy(true);

	// remove all but the last members
	while (guild->getMembers().size() > 1)
	{
		guild->removeMember(guild->getMembers().begin()->first);
	}
	// remove the last member (this will delete the guild)
	guild->removeMember(guild->getMembers().begin()->first);

	STOP_IF(CGuildManager::getInstance()->getGuildFromId(guildId) != NULL, "The guild "<<guildId<<" has not been removed after deleting the last member");

	log.displayNL("The guild %s have been saved and unloaded", guildIdToString(guildId).c_str());

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CGuildManager, loadGuild)
{
	if (args.size() != 1)
		return false;

	string::size_type pos = args[0].find("guild_");
	if (pos == string::npos)
	{
		log.displayNL("Invalid guild file name '%s', can't find 'guild_' inside", args[0].c_str());
		return true;
	}

	TGuildId guildId;
	NLMISC::fromString(args[0].substr(pos+6, 5), guildId);
	guildId = guildId & 0xfffff;
	if (guildId == 0)
	{
		log.displayNL("Invalid guild ID in '%s'", args[0].c_str());

		return true;
	}

	CGuild * guild = getGuildFromId(guildId + (IService::getInstance()->getShardId() << 20));
	if (guild != NULL)
	{
		// we must unload it first
		if (!NLMISC::CCommandRegistry::getInstance().execute(getCommandHandlerName()+".unloadGuild "+NLMISC::toString(guildId), log, quiet, human))
			return false;
	}

	// now, we can load the guild
	CGuildManager::loadGuild("guilds/"+args[0]);

	log.displayNL("Loading of guild %s as file '%s' requested to BS, check later for result", guildIdToString(guildId).c_str(), (string("guilds/")+args[0]).c_str());

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CGuildManager, renameGuild)
{
	if (args.size() != 2)
		return false;

	TGuildId guildId = parseGuildId(args[0]);
	if (guildId == 0)
	{
		log.displayNL("Invalid guild id '%s'", args[0].c_str());

		return true;
	}

	CGuild * guild = getGuildFromId(guildId);
	if (guild == NULL)
	{
		log.displayNL("Can't find guild with id %s", guildIdToString(guildId).c_str());
		return true;
	}

	if (guild->isProxy())
	{
		log.displayNL("Can't rename a foreign guild %s", guildIdToString(guildId).c_str());
		return true;
	}

	// ok, rename it
	ucstring newName;
	newName.fromUtf8(args[1]);
	guild->setName(newName);

	log.displayNL("Guild %s renamed as '%s'", guildIdToString(guildId).c_str(), newName.toUtf8().c_str());

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CGuildManager, addGuildMember)
{
	log.displayNL("Not implemented yet, sorry :(");
	return false;
//	if (args.size() < 2)
//		return false;
//	if (args.size() > 3)
//		return false;
//
//	TGuildId guildId;
//	NLMISC::fromString(args[0], guildId);
//
//	CGuild *guild = getGuildFromId(guildId);
//	if (guild == NULL)
//	{
//		log.displayNL("Failed to find guild %u", guildId);
//		return true;
//	}
//
//	CEntityId charEid(args[1]);
//	if (charEid == CEntityId::Unknown)
//	{
//		log.displayNL("Failed to parse character id '%s'", args[1].c_str());
//		return true;
//	}
//
//	EGSPD::CGuildGrade::TGuildGrade grade = EGSPD::CGuildGrade::Leader;
//
//	if (args.size() > 2)
//	{
//		grade = EGSPD::CGuildGrade::fromString(args[2]);
//		if (grade == EGSPD::CGuildGrade::Unknown)
//		{
//			log.displayNL("Failed to parse grade '%s'", args[2].c_str());
//			return true;
//		}
//	}
//
//	// check that the character is not already member
//	if (guild->getMemberFromEId(charEid) != NULL)
//	{
//		log.displayNL("The character %s is already member of the guild", charEid.toString().c_str());
//		return true;
//	}
//	// check that we not violate the grade count limit
//	if (guild->getGradeCount(grade) >= guild->getMaxGradeCount(grade))
//	{
//		log.displayNL("There is already the maximum member with the grade '%s'", EGSPD::CGuildGrade::toString(grade).c_str());
//		return true;
//	}
//
//	CGuildCharProxy
//	// ok, now do the job
//	CGuildMember * member = guild->newMember( charEid );
//	BOMB_IF( !member, "created null guild member!", return true );
//
//	module = new CGuildMemberModule( proxy, member );
//	BOMB_IF( !module, "created null guild member module!", return true );
//
//	member->setMemberGrade( EGSPD::CGuildGrade::Member );
//	guild->setMemberOnline( member, proxy.getId().getDynamicId() );
//
//	log.displayNL("The character %s have been added to guild %u with grade '%s'",
//		charEid.toString().c_str(),
//		guildId,
//		EGSPD::CGuildGrade::toString(grade).c_str());
//
//	return true;
}
