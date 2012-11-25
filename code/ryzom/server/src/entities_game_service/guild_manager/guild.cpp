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

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"

#include "guild.h"
#include "modules/guild_unifier.h"
#include "building_manager/building_manager.h"
#include "building_manager/building_physical.h"
#include "guild_manager.h"
#include "guild_member_module.h"
#include "guild_version_adapter.h"
#include "game_item_manager/game_item_manager.h"
#include "fame_manager.h"
#include "outpost_manager/outpost_manager.h"
#include "primitives_parser.h"
#include "modules/shard_unifier_client.h"
#include "mission_manager/mission_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"

/// todo guild remove entity id translator
#include "nel/misc/eid_translator.h"
#include "chat_groups_ids.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace EGSPD;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


NL_INSTANCE_COUNTER_IMPL(CGuild);

CVariable<uint32> GuildMaxOutpostCount("egs", "GuildMaxOutpostCount", "max number of outposts per guild", OUTPOSTENUMS::MAX_OUTPOST, 0, true);

// utility function to convert a guild id into a human readable string
extern std::string guildIdToString(uint32 guildId);
// utility to parse a human readable string into a guild id
extern uint32 parseGuildId(const std::string &str);

extern bool IOSIsUp;

//----------------------------------------------------------------------------
CGuild::CGuild()
{
	_Proxy = false;
	_DeclaredCult = PVP_CLAN::Neutral;
	_DeclaredCiv = PVP_CLAN::Neutral;
}

//----------------------------------------------------------------------------
CGuild::~CGuild()
{
	_GuildInventoryView->release();
	_GuildInventoryView = NULL;
}

//----------------------------------------------------------------------------
uint32 CGuild::getNameId()const
{
//	return uint32(getClientDBProp("GUILD:NAME"));
	return CBankAccessor_GUILD::getGUILD().getNAME_id(_DbGroup);
}

////----------------------------------------------------------------------------
uint32 CGuild::getDescriptionId()const
{
//	return uint32(getClientDBProp("GUILD:DESCRIPTION"));
	return CBankAccessor_GUILD::getGUILD().getDESCRIPTION_id(_DbGroup);
}
//----------------------------------------------------------------------------
const ucstring & CGuild::getName()const
{
	return _Name;
//	NLMISC::CEntityId stringEId( _EId );
//	stringEId.setType( RYZOMID::guildName );
//	const ucstring& str = EGSPD::PDSLib.getStringManager().getString( stringEId );
//	if ( str.empty() )
//		nlwarning("<GUILD> guild %u has no name",_Id );
//	return str;
}

//----------------------------------------------------------------------------
const ucstring & CGuild::getDescription()const
{
	return _Description;
//	NLMISC::CEntityId stringEId(_EId);
//	stringEId.setType( RYZOMID::guildDescription );
//	const ucstring& str = EGSPD::PDSLib.getStringManager().getString( stringEId );
//	return str;
}

//----------------------------------------------------------------------------
uint8 CGuild::getMembersSession()const
{
	return _MembersSession; 
} 

//----------------------------------------------------------------------------
//void CGuild::spendXP( uint32 xp )
//{
//	nlassert( xp <= _XP );
//	setXP( _XP - xp );
//	setClientDBProp( "GUILD:XP", _XP );
//}

//----------------------------------------------------------------------------
//void CGuild::addXP( uint32 xp )
//{
//	setXP( _XP + xp );
//	setClientDBProp( "GUILD:XP", _XP );
//}

//----------------------------------------------------------------------------
void CGuild::spendMoney(uint64 money)
{
	if ( money > _Money )
	{
		nlwarning( "spendMoney guild %u : money = %"NL_I64"u, max = %"NL_I64"u", _Id, money, _Money);
		return;
	}

	_GuildInventoryView->updateSessionForMoneyTransaction();
	CGuildPD::setMoney( _Money - money );
//	setClientDBProp( "GUILD:INVENTORY:MONEY", _Money );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setMONEY(_DbGroup, _Money);
}

//----------------------------------------------------------------------------
void CGuild::addMoney(uint64 money)
{
	_GuildInventoryView->updateSessionForMoneyTransaction();
	CGuildPD::setMoney( _Money + money );
//	setClientDBProp( "GUILD:INVENTORY:MONEY", _Money );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setMONEY(_DbGroup, _Money);
}

//----------------------------------------------------------------------------
void CGuild::setMoney(uint64 money)
{
	_GuildInventoryView->updateSessionForMoneyTransaction();
	CGuildPD::setMoney( money );
//	setClientDBProp( "GUILD:INVENTORY:MONEY", _Money );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setMONEY(_DbGroup, _Money);
}

//----------------------------------------------------------------------------
//void CGuild::clearChargePoints()
//{
//	setChargesPoints( 0 );
//	setClientDBProp( "GUILD:CHARGE_POINTS", _ChargesPoints );
//}

//----------------------------------------------------------------------------
//void CGuild::addChargePoints( uint32 points )
//{
//	setChargesPoints( _ChargesPoints + points );
//	setClientDBProp( "GUILD:CHARGE_POINTS", _ChargesPoints );
//}

//----------------------------------------------------------------------------
//bool CGuild::hasRoleMaster( EGSPD::CSPType::TSPType type )
//{
//	return getRolemastersValid(type);
//}

//----------------------------------------------------------------------------
//void CGuild::addRoleMaster( EGSPD::CSPType::TSPType type )
//{
//	setRolemastersValid(type,true);
//}

//----------------------------------------------------------------------------
void CGuild::setIcon( uint64 icon )
{
	CGuildPD::setIcon( icon );
//	setClientDBProp( "GUILD:ICON", icon );
	CBankAccessor_GUILD::getGUILD().setICON(_DbGroup, icon);
}

//----------------------------------------------------------------------------
void CGuild::setBuilding(TAIAlias buildingAlias)
{
	if (getBuilding() == buildingAlias)
		return;

	// check if the new guild building exists
	CBuildingPhysicalGuild * guildBuilding = dynamic_cast<CBuildingPhysicalGuild *>(CBuildingManager::getInstance()->getBuildingPhysicalsByAlias(buildingAlias));
	if (guildBuilding == NULL)
	{
		nlwarning("invalid guild building %s", CPrimitivesParser::aliasToString(buildingAlias).c_str());
		return;
	}

	// remove the previous guild building and clear guild inventory
	if (getBuilding() != CAIAliasTranslator::Invalid)
	{
		CBuildingManager::getInstance()->removeGuildBuilding( getId() );
		//_Inventory->clearInventory();
	}

	// set the new guild building
	CGuildPD::setBuilding(buildingAlias);
	guildBuilding->addGuild( getId() );
}

//----------------------------------------------------------------------------
void CGuild::setMOTD( const std::string& motd, const NLMISC::CEntityId& eId)
{
	if( ! isProxy() )
	{
		CGuildMember * member = getMemberFromEId(eId);
		CCharacter * user = PlayerManager.getChar( eId );
		if ( member == NULL || user == NULL )
		{
			nlwarning("<CGuildMemberModule::setMOTD>%s invalid member id %s",eId.toString().c_str());
			return;
		}

		if ( motd == "?" )
		{
			// Show the old MOTD
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal= _MessageOfTheDay;
			CCharacter::sendDynamicMessageToChatGroup(user->getEntityRowId(), "GMOTD", CChatGroup::guild, params);
			return;
		}

		EGSPD::CGuildGrade::TGuildGrade memberGrade = member->getGrade();
		if( memberGrade >= EGSPD::CGuildGrade::Member)
		{
			user->sendDynamicSystemMessage(eId,"GUILD_INSUFFICIENT_GRADE");
			return;
		}

		if( motd.empty() )
		{
			_MessageOfTheDay.clear();
		}
		else
		{
			_MessageOfTheDay.fromUtf8(motd);
		}

		if(!_MessageOfTheDay.empty())
		{
			// Show new MOTD to all members
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal= _MessageOfTheDay;
			sendMessageToGuildChat("GMOTD", params);
		}
	}
	else
	{
		CCharacter::sendDynamicSystemMessage(eId,"GUILD_IS_PROXY");
	}
}

//----------------------------------------------------------------------------
void CGuild::onGuildStringUpdated(ICDBStructNode *node)
{
	uint32 iosNameId = uint32(_DbGroup.Database.x_getProp(node));

	// update the mirror entry for all guild member with the new ios guild name id
	for ( std::map< EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::const_iterator it = getMembersBegin(); it != getMembersEnd(); ++it )
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember*>( (*it).second );
		EGS_PD_AST( member );
		
		// continue if the player is offline
		CGuildMemberModule * module = NULL;
		if ( member->getReferencingModule(module) )
		{
			CGuildCharProxy proxy;
			module->getProxy(proxy);
			if (!proxy.getEntityRowId().isValid())
			{
				// entity not in mirror now, skip it
				continue;
			}
			CMirrorPropValue<TYPE_GUILD_NAME_ID> mirrorName( TheDataset, proxy.getEntityRowId(), DSPropertyGUILD_NAME_ID );
			mirrorName = iosNameId;
		}
	}
}

//----------------------------------------------------------------------------
CGuildMember* CGuild::getMemberByIndex(uint16 index) const
{
	for ( std::map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::const_iterator it = getMembersBegin(); it != getMembersEnd(); ++it )
	{
		CGuildMember* member = EGS_PD_CAST<CGuildMember*> ( (*it).second );
		EGS_PD_AST(member);
		if ( member->getMemberIndex() == index )
			return member;
	}
	return NULL;
}

//----------------------------------------------------------------------------
uint16 CGuild::getMemberCount()const
{
	const uint size = _GradeCounts.size();
	uint16 count = 0;
	for (uint i = 0; i < size; ++i )
	{
		count += _GradeCounts[i];
	}
	return count;
}

//----------------------------------------------------------------------------
uint16 CGuild::getGradeCount(EGSPD::CGuildGrade::TGuildGrade grade)const
{
	nlassert( uint(grade) < _GradeCounts.size() );
	return _GradeCounts[grade];
}

//----------------------------------------------------------------------------
void CGuild::incGradeCount( EGSPD::CGuildGrade::TGuildGrade grade )
{
	nlassert( uint(grade) < _GradeCounts.size() );
	++_GradeCounts[grade];
}
//----------------------------------------------------------------------------
void CGuild::decGradeCount( EGSPD::CGuildGrade::TGuildGrade grade )
{
	nlassert( uint(grade) < _GradeCounts.size() );
	--_GradeCounts[grade];
}

//----------------------------------------------------------------------------
void CGuild::sendClientDBDeltas()
{
//	static ICDBStructNode *nodeCreationDate = _DbGroup.Database.getICDBStructNodeFromName( "GUILD:CREATION_DATE" );
//	_DbGroup.Database.setProp( nodeCreationDate, uint32(( CTickEventHandler::getGameCycle() - _CreationDate )  * CTickEventHandler::getGameTimeStep() / 86400 ) );
	CBankAccessor_GUILD::getGUILD().setCREATION_DATE(_DbGroup, uint32(( CTickEventHandler::getGameCycle() - _CreationDate )  * CTickEventHandler::getGameTimeStep() / 86400 ) );
	_DbGroup.sendDeltas( ~0, *_GuildInventoryView, CCDBGroup::SendDeltasToRecipients );
}


//----------------------------------------------------------------------------
void CGuild::incMemberSession()
{
	++_MembersSession;
//	setClientDBProp( "GUILD:COUNTER",++_MembersSession );
	CBankAccessor_GUILD::getGUILD().setCOUNTER(_DbGroup, _MembersSession);
}

//----------------------------------------------------------------------------
//void CGuild::setClientDBProp(const std::string & prop, sint64 value )
//{
//	_DbGroup.Database.x_setProp( prop, value );
//}
//
////----------------------------------------------------------------------------
//void CGuild::setClientDBPropString(const std::string & prop, const ucstring &value )
//{
//	_DbGroup.Database.setPropString( prop, value );
//}

//----------------------------------------------------------------------------
//sint64 CGuild::getClientDBProp(const std::string & prop) const
//{
//	return _DbGroup.Database.getProp( prop );
//}


void CGuild::setIsProxy(bool isProxy)
{
	_Proxy = isProxy;
}

//----------------------------------------------------------------------------
void CGuild::initNonPDMembers()
{
	H_AUTO(InitNonPDMembers);
	_MembersSession = 0;
	_NextMemberIndex = 0;

	// build entity id
	_EId = CEntityId( RYZOMID::guild, _Id );
	_EId.setCreatorId(0);
	
	{
		H_AUTO(resizeInitNonPDMembers);
		_GradeCounts.resize(EGSPD::CGuildGrade::EndGuildGrade);
		const uint size = _GradeCounts.size();
		for ( uint i = 0; i < size; i++ )
			_GradeCounts[i] = 0;
	}

	// Database (loading needs it ready for the inventory view callbacks)
	_DbGroup.init();

	// Inventory stuff
	_Inventory = new CGuildInventory;
	_GuildInventoryView = new CGuildInventoryView( this ); // unfortunately this MUST be a smartptr because of smartptrs to views in CInventoryBase
	_GuildInventoryView->init( _Inventory, &_DbGroup );
}

//----------------------------------------------------------------------------
void CGuild::postCreate()
{
	// called after a new guild is created
	// Set the new database values.
	resetFameDatabase();
}

//----------------------------------------------------------------------------
void CGuild::setName(const ucstring & str)
{
	_Name = str;
//	setClientDBPropString( "GUILD:NAME", _Name);
	CBankAccessor_GUILD::getGUILD().setNAME(_DbGroup, _Name);
//	H_AUTO( PDS_SAVE_STRINGS_NAME )
//	NLMISC::CEntityId stringEId( _EId );
//	stringEId.setType( RYZOMID::guildName );
//	EGSPD::PDSLib.getStringManager().unmap( stringEId );
//	EGSPD::PDSLib.getStringManager().addString( stringEId,str );
	//EGSPD::PDSLib.getStringManager().save();
}

//----------------------------------------------------------------------------
void CGuild::setDescription(const ucstring & str)
{
	_Description = str;
//	setClientDBPropString( "GUILD:DESCRIPTION", _Description);
	CBankAccessor_GUILD::getGUILD().setDESCRIPTION(_DbGroup, _Description);
//	H_AUTO( PDS_SAVE_STRINGS_DESC )
//	NLMISC::CEntityId stringEId(_EId);
//	stringEId.setType( RYZOMID::guildDescription );
//	EGSPD::PDSLib.getStringManager().addString( stringEId,str );
//	//EGSPD::PDSLib.getStringManager().save();
}

//----------------------------------------------------------------------------
void CGuild::dumpGuildInfos( NLMISC::CLog & log )
{
	log.displayNL("<GUILD_DUMP> Guild id: %s %s, name: '%s', eid: %s", 
		guildIdToString(getId()).c_str(), 
		getId()>>20 == IService::getInstance()->getShardId() ? "(Local)" : "(Foreign)",
		getName().toUtf8().c_str(), getEId().toString().c_str() );
	log.displayNL("\tDescription: '%s'", getDescription().toUtf8().c_str() );
	log.displayNL("\tMoney: %"NL_I64"u", getMoney() );
//	log.displayNL("\tVillage: %hu", getVillage() );
	log.displayNL("\tCreation date: %u", getCreationDate() );
//	log.displayNL("\tXP: %u", getXP() );
	log.displayNL("\tBulk: %d", _Inventory->getInventoryBulk() );
	log.displayNL("\tMax bulk: %d", _Inventory->getMaxBulk() );
//	log.displayNL("\tCharge points: %u", getChargesPoints() );
	log.displayNL("\tRace: %s", EGSPD::CPeople::toString(getRace()).c_str() );
	log.displayNL("\tIcon: 0x%016"NL_I64"x", getIcon() );
	log.displayNL("\tCiv Allegiance: %s", PVP_CLAN::toString(_DeclaredCiv).c_str());
	log.displayNL("\tCult Allegiance: %s", PVP_CLAN::toString(_DeclaredCult).c_str());

	string buildingName;
	TAIAlias buildingAlias = getBuilding();
	IBuildingPhysical * building = CBuildingManager::getInstance()->getBuildingPhysicalsByAlias( buildingAlias );
	if (building)
		buildingName = building->getName();
	log.displayNL("\tBuilding: %s", buildingName.c_str() );

	// TODO: display fame

	log.displayNL("\tNb of members: %hu", getMemberCount() );
	for (uint i = 0; i < _GradeCounts.size(); i++)
	{
		const EGSPD::CGuildGrade::TGuildGrade grade = EGSPD::CGuildGrade::TGuildGrade( i );
		const uint count = _GradeCounts[i];
		log.displayNL("\tNb of members with grade '%s': %u", EGSPD::CGuildGrade::toString( grade ).c_str(), count);
	}

	log.displayNL("\t-------- Members --------");
	for (map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD *>::iterator it = getMembersBegin(); it != getMembersEnd(); ++it)
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember *>( (*it).second );
		EGS_PD_AST(member);

		CEntityId eId = member->getIngameEId();
		string name = CEntityIdTranslator::getInstance()->getByEntity( eId ).toUtf8();
		log.displayNL("\tMember '%s' %s, index: %hu, grade: %s, enter time: %u",
			name.c_str(),
			eId.toString().c_str(),
			member->getMemberIndex(),
			EGSPD::CGuildGrade::toString( member->getGrade() ).c_str(),
			member->getEnterTime()
			);
	}

	log.displayNL("\t-------- Owned Outposts --------");
	for (vector<TAIAlias>::iterator it = _OwnedOutposts.begin(); it != _OwnedOutposts.end(); ++it)
	{
		CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(*it);
		if (outpost == NULL)
			continue;

		log.displayNL("\tOwned Outpost: alias %s, name '%s', sheet '%s'",
			CPrimitivesParser::aliasToString(outpost->getAlias()).c_str(),
			outpost->getName().c_str(),
			outpost->getSheet().toString().c_str()
			);
	}

	log.displayNL("\t-------- Challenged Outposts --------");
	for (vector<TAIAlias>::iterator it = _ChallengedOutposts.begin(); it != _ChallengedOutposts.end(); ++it)
	{
		CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(*it);
		if (outpost == NULL)
			continue;

		log.displayNL("\tChallenged Outpost: alias %s, name '%s', sheet '%s'",
			CPrimitivesParser::aliasToString(outpost->getAlias()).c_str(),
			outpost->getName().c_str(),
			outpost->getSheet().toString().c_str()
			);
	}
}

//----------------------------------------------------------------------------
void CGuild::registerGuild()
{
	H_AUTO(RegisterGuild);
	if ( getFameContainer() == NULL )
	{
		EGSPD::CGuildFameContainerPD* container = EGSPD::CGuildFameContainerPD::create(CEntityId(RYZOMID::guild,_Id));
		EGS_PD_AST( container );
		setFameContainer( container );
	}
	/// todo guild mission : update saved missions

	openGuildChatGroup();

	// register guild in fame manager
	CFameManager::getInstance().addGuild( _EId, *getFameContainer(), getRace() );

	// set database properties
//	setClientDBProp( "GUILD:PROXY",_Proxy);
	CBankAccessor_GUILD::getGUILD().setPROXY(_DbGroup, _Proxy);
//	setClientDBProp( "GUILD:ICON",_Icon );
	CBankAccessor_GUILD::getGUILD().setICON(_DbGroup, _Icon);
//	setClientDBProp( "GUILD:XP",_XP );
//	CBankAccessor_GUILD::getGUILD().setXP(_DbGroup, _XP);
//	setClientDBProp( "GUILD:VILLAGE",_Village );
//	CBankAccessor_GUILD::getGUILD().setVILLAGE(_DbGroup, _Village);
//	setClientDBProp( "GUILD:PEOPLE",_Race );
	CBankAccessor_GUILD::getGUILD().setPEOPLE(_DbGroup, _Race);
//	setClientDBProp( "GUILD:CREATION_DATE", uint32( ( CTickEventHandler::getGameCycle() - _CreationDate )  * CTickEventHandler::getGameTimeStep() / 86400 ) );
	CBankAccessor_GUILD::getGUILD().setCREATION_DATE(_DbGroup, uint32( ( CTickEventHandler::getGameCycle() - _CreationDate )  * CTickEventHandler::getGameTimeStep() / 86400 ));

//	setClientDBProp( "GUILD:INVENTORY:MONEY", _Money );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setMONEY(_DbGroup, _Money);

	// Set the database properties for fame.
	resetFameDatabase();

	// init member data and
	{
		H_AUTO(initMemberData_RegisterGuild);

		uint index = 0;
		for ( map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::iterator it = _Members.begin(); it != _Members.end(); ++it )
		{
			// init the grade count
			CGuildMember * member = EGS_PD_CAST<CGuildMember*>((*it).second);
			EGS_PD_AST( member );
			nlassert( uint(member->getGrade()) < _GradeCounts.size() );
			// add online members to the chat group and to the database
			++_GradeCounts[member->getGrade()];
			// init the member index
			member->setMemberIndex( index++ );
			setMemberClientDB(member);
			// if player is online, set is guild fame values
			if (PlayerManager.getChar((*it).first) != NULL)
			{
				CFameManager::getInstance().setPlayerGuild( (*it).first,getEId(), false );
			}
		}
		_NextMemberIndex = index;
	}

	/// todo guild : name
	//CEntityIdTranslator::getInstance()->registerEntity(_EId,guild->getGuildName(),0,0,"");
	
	if ( _Building != CAIAliasTranslator::Invalid )
		CBuildingManager::getInstance()->registerGuild( _Id, _Building );

	// set guild allegiance in database
//	setClientDBProp("GUILD:FAME:CULT_ALLEGIANCE", _DeclaredCult);
	CBankAccessor_GUILD::getGUILD().getFAME().setCULT_ALLEGIANCE(_DbGroup, _DeclaredCult);
//	setClientDBProp("GUILD:FAME:CIV_ALLEGIANCE", _DeclaredCiv);
	CBankAccessor_GUILD::getGUILD().getFAME().setCIV_ALLEGIANCE(_DbGroup, _DeclaredCiv);
}

//----------------------------------------------------------------------------

void CGuild::openGuildChatGroup()
{
	H_AUTO(openGuildChatGroup);
	// we need to wait for IOS up message before openning 
	if (!IOSIsUp)
		return;

	// register guild chat group in IOS
	TGroupId idGroup = CHAT_GROUPS_IDS::getGuildChatGroupId(_Id);
	CMessage msgout("ADD_GROUP");
	msgout.serial( idGroup );
	CChatGroup::TGroupType type = CChatGroup::guild;
	msgout.serialEnum( type );
	sendMessageViaMirror( "IOS", msgout );

	// add any online member in the chat group
	std::map<TCharacterId, CGuildMemberPD*>::const_iterator first(getMembers().begin()), last(getMembers().end());
	for (; first != last; ++first)
	{
		CGuildMember *member = dynamic_cast<CGuildMember *>(first->second);

		CCharacter *character = PlayerManager.getChar(member->getId());
		if (character != NULL)
		{
			addMemberToGuildChat(member);
		}
	}
}

//----------------------------------------------------------------------------
void CGuild::unregisterGuild()
{
	H_AUTO( PDS_SAVE_STRINGS_GUILD_UNREGISTER );

//	NLMISC::CEntityId nameEId(_EId);
//	nameEId.setType( RYZOMID::guildName );
//	EGSPD::PDSLib.getStringManager().unmap( nameEId );
//
//	NLMISC::CEntityId descEId(_EId);
//	descEId.setType( RYZOMID::guildDescription );
//	EGSPD::PDSLib.getStringManager().unmap( descEId );

	TGroupId idGroupe = CHAT_GROUPS_IDS::getGuildChatGroupId( _Id );
	NLNET::CMessage msgRemoveGroup("REMOVE_GROUP");
	msgRemoveGroup.serial( idGroupe );
	sendMessageViaMirror( "IOS", msgRemoveGroup );
	/// todo guild : remove fame

	// Set the database properties for fame.
	resetFameDatabase();

	CBuildingManager::getInstance()->removeGuildBuilding( _Id );
	CFameManager::getInstance().removeGuild(_EId);
	COutpostManager::getInstance().onRemoveGuild(this);
//	EGSPD::PDSLib.getStringManager().save();
}

//----------------------------------------------------------------------------
//void CGuild::rebuildCliendDB()
//{
//	// set database properties
//	setClientDBProp( "GUILD:ICON",_Icon );
//	setClientDBProp( "GUILD:XP",_XP );
//	setClientDBProp( "GUILD:VILLAGE",_Village );
//	setClientDBProp( "GUILD:PEOPLE",_Race );
//	setClientDBProp( "GUILD:CREATION_DATE", uint32( ( CTickEventHandler::getGameCycle() - _CreationDate )  * CTickEventHandler::getGameTimeStep() / 86400 ) );
//
//	setClientDBProp( "GUILD:INVENTORY:MONEY", _Money );
//
//	// init member data and
//	uint index = 0;
//	for ( map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::iterator it = _Members.begin(); it != _Members.end(); ++it )
//	{
//		// init the grade count
//		CGuildMember * member = EGS_PD_CAST<CGuildMember*>((*it).second);
//		setMemberClientDB(member);
//	}
//
//	// set guild allegiance in database
//	setClientDBProp("GUILD:FAME:CULT_ALLEGIANCE", _DeclaredCult);
//	setClientDBProp("GUILD:FAME:CIV_ALLEGIANCE", _DeclaredCiv);
//
//}

//----------------------------------------------------------------------------
void CGuild::removeMission( uint idx, TMissionResult result)
{
	if ( idx >= _Missions.size() )
		return;

	/// if the mission was finished, the result is success
	if ( _Missions[idx]->getFinished() )
	{
		if ( _Missions[idx]->getMissionSuccess() )
			result = mr_success;
		else
			result = mr_fail;
	}

	CMissionTemplate *tpl = CMissionManager::getInstance()->getTemplate(_Missions[idx]->getTemplateId());

	updateMissionHistories( _Missions[idx]->getTemplateId(), result);

	if ( tpl && !tpl->Tags.NoList )
	{
		_Missions[idx]->clearUsersJournalEntry();
	}

	CMissionManager::getInstance()->deInstanciateMission(_Missions[idx]);
	delete _Missions[idx];
	_Missions.erase(_Missions.begin() + idx) ;
}

//----------------------------------------------------------------------------
void CGuild::addSuccessfulMission(CMissionTemplate * templ)
{
	TMissionHistory &mh = _MissionHistories[templ->Alias];
	mh.Successfull = true;
}

//----------------------------------------------------------------------------
void CGuild::clearSuccessfulMissions()
{
	_MissionHistories.clear();
}

//----------------------------------------------------------------------------
void CGuild::updateMissionHistories(TAIAlias missionAlias, uint32 result)
{
	TMissionHistory &mh = _MissionHistories[missionAlias];

	switch(result)
	{
	case mr_success:
	case mr_forced:
		mh.Successfull = true;
		// validate last try date
		_MissionHistories[missionAlias].LastSuccessDate = CTickEventHandler::getGameCycle();
		break;
	}
}

//----------------------------------------------------------------------------
void CGuild::sendDynamicMessageToMembers(const string &msgName, const TVectorParamCheck &params, const set<CEntityId> &excluded) const
{
	for ( std::map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::const_iterator it = getMembersBegin();
			it != getMembersEnd();++it )
	{
		CCharacter * user = PlayerManager.getChar( it->first );

		if ( excluded.find(it->first) == excluded.end())
		{
			const uint32 stringId = STRING_MANAGER::sendStringToClient(TheDataset.getDataSetRow(it->first), msgName, params );
			PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(it->first), stringId);
		}
	}
}

//----------------------------------------------------------------------------
bool CGuild::processMissionEvent( CMissionEvent & event, TAIAlias alias)
{
	std::list<CMissionEvent*> listEvents;
	listEvents.push_back(&event);
	return processGuildMissionEvent(listEvents, alias);
}

//----------------------------------------------------------------------------
bool CGuild::processGuildMissionEvent(std::list< CMissionEvent *> & eventList, TAIAlias missionAlias)
{
	for (uint i = 0; i < _Missions.size(); i++ )
	{
		nlassert( _Missions[i] );
		if ( missionAlias == CAIAliasTranslator::Invalid	|| _Missions[i]->getTemplateId() == missionAlias )
		{
			if ( processGuildMissionStepEvent( eventList, _Missions[i]->getTemplateId() ,0xFFFFFFFF) )
				return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
bool CGuild::processGuildMissionStepEvent(std::list< CMissionEvent*> & eventList, TAIAlias missionAlias, uint32 stepIndex)
{
	CMissionGuild * mission = getMissionByAlias( missionAlias );
	if (!mission )
	{
		nlwarning("invalid missionAlias");
		return false;
	}
	// I don't know if i should pass _EId to this function
	CMissionEvent::TResult result = mission->processEvent(TheDataset.getDataSetRow(getHighestGradeOnlineUser()) /*TheDataset.getDataSetRow( _EId)*/ ,eventList,stepIndex );
	if ( result == CMissionEvent::Nothing )
		return false;
	else if ( result == CMissionEvent::MissionFailed )
		return true;

	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
	nlassert( templ );
	if ( result == CMissionEvent::MissionEnds )
	{
		CMissionEventMissionDone * event = new CMissionEventMissionDone(templ->Alias);
		eventList.push_back(event);

		addSuccessfulMission(templ);

		for ( std::map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator it = getMembersBegin();
			it != getMembersEnd();++it )
		{
			CCharacter * user = PlayerManager.getChar( it->first );
			if ( user )
			{
				if ( templ->Tags.NoList == false )
					CCharacter::sendDynamicSystemMessage( user->getEntityRowId(),"EGS_MISSION_SUCCESS");
			}
		}

		CMissionManager::getInstance()->missionDoneOnce(templ);
		mission->stopChildren();

		// only remove no list missions, other must be manually removed by user
		if ( templ->Tags.NoList || mission->isChained() || templ->Tags.AutoRemove )
		{
			mission->updateEncyclopedia();
			removeMission(mission, mr_success);
		}
		else
		{
			mission->setSuccessFlag();
			mission->updateUsersJournalEntry();
		}
		return true;
	}
	else if ( result == CMissionEvent::StepEnds )
	{
		if ( templ->Tags.NoList == false )
		{
			for ( std::map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator it = getMembersBegin();
				it != getMembersEnd();++it )
			{
				CCharacter * user = PlayerManager.getChar( it->first );
				if ( user )
				{
					if ( templ->Tags.NoList == false )
						CCharacter::sendDynamicSystemMessage( user->getEntityRowId(),"EGS_MISSION_STEP_SUCCESS");
				}
			}
		}
	}
	mission->updateUsersJournalEntry();
	return true;
}

//----------------------------------------------------------------------------
CMissionGuild* CGuild::getMissionByAlias( TAIAlias missionAlias )
{
	const uint size = (uint)_Missions.size();
	for ( uint i = 0; i < size; i++ )
	{
		if ( _Missions[i] && _Missions[i]->getTemplateId() == missionAlias )
			return _Missions[i];
	}
	return NULL;
}

//----------------------------------------------------------------------------
bool CGuild::isMissionSuccessfull(TAIAlias alias)
{
	std::map<TAIAlias, TMissionHistory>::iterator it(_MissionHistories.find(alias));
	if (it != _MissionHistories.end())
		return it->second.Successfull;
	return false;
}

//----------------------------------------------------------------------------
bool CGuild::isInGuildBuilding( const TDataSetRow & user )
{
	IBuildingPhysical * guildBuilding = CBuildingManager::getInstance()->getBuildingPhysicalsByAlias(CGuildPD::getBuilding());
	if (guildBuilding == NULL)
		return false;
	
	return guildBuilding->isUserInsideBuilding(user);
}

//----------------------------------------------------------------------------
bool CGuild::canAccessToGuildInventory( CCharacter * user )
{
	// check if user is in guild building
	if( isInGuildBuilding( user->getEntityRowId() ) )
		return true;

	// or in zone of the guild's outpost
	CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias(user->getCurrentOutpostZone());
	if( outpost != NULL )
		if( outpost->getOwnerGuild() == _Id )
			return true;
	return false;
}

//----------------------------------------------------------------------------
bool CGuild::putItem( CGameItemPtr item )
{
	CInventoryBase::TInventoryOpResult res = _Inventory->insertItem(item, INVENTORIES::INSERT_IN_FIRST_FREE_SLOT, true);
	if (res != CInventoryBase::ior_ok)
		item.deleteItem();
	return res == CInventoryBase::ior_ok;
}

//----------------------------------------------------------------------------
void CGuild::putItem( CCharacter * user, uint32 slot, uint32 quantity, uint16 session )
{
	// the session system works that way :
	// As player can share this inventory, we manage a per item session value
	// the user sends its session when he tries to manipulate the inventory. If it is higher than the targeted item session, it is okj
	// The item session is incremented and the highest session value is then sent to the clients
	// sessions are reseted when nobody uses the inventory
	nlassert( user );
	
	if( canAccessToGuildInventory( user ) == false )
	{
		CCharacter::sendDynamicSystemMessage(user->getEntityRowId(), "CANT_ACCESS_GUILD_INVENTORY");
		return;
	}

	// get the item
	CInventoryPtr srcItems = user->getInventory(INVENTORIES::bag);
	if ( slot >= srcItems->getSlotCount() )
	{
		nlwarning( "<GUILD> user %s Invalid bag slot %u, count = %u",user->getId().toString().c_str(), slot, srcItems->getSlotCount() );
		return;
	}
	CGameItemPtr srcItem = srcItems->getItem(slot);
	if ( srcItem == NULL )
	{
		nlwarning( "<GUILD> user %s Invalid bag slot %u, count = %u -> NULL item",user->getId().toString().c_str(), slot, srcItems->getSlotCount() );
		return;
	}

	// check if this type of item is legal in the guild inventory
	CGameItemPtr item = srcItem;
	if (	!item->getStaticForm()->DropOrSell
		||	item->getStaticForm()->Family == ITEMFAMILY::PET_ANIMAL_TICKET
		|| 	user->isAnActiveXpCatalyser(item)
		)
	{
		CCharacter::sendDynamicSystemMessage( user->getId(),"GUILD_ITEM_CANT_BE_PUT" );
		return;
	}

	// You cannot exchange genesis named items
	if (item->getPhraseId().find("genesis_") == 0)
	{
		nlwarning("Character %s tries to put in guild '%s'", user->getId().toString().c_str(), item->getPhraseId().c_str() );
		CCharacter::sendDynamicSystemMessage( user->getId(),"GUILD_ITEM_CANT_BE_PUT" );
		return;
	}

	// try to move the required quantity of the item
	if ( CInventoryBase::moveItem(
		user->getInventory(INVENTORIES::bag), slot,
		_Inventory,	INVENTORIES::INSERT_IN_FIRST_FREE_SLOT,
		quantity ) != CInventoryBase::ior_ok )
	{
		CCharacter::sendDynamicSystemMessage( user->getId(),"GUILD_ITEM_MAX_BULK" ); // "The guild warehouse is full"
		return;
	}
}

//----------------------------------------------------------------------------
void CGuild::takeItem( CCharacter * user, uint32 slot, uint32 quantity, uint16 session )
{
	// the session system works that way :
	// As player can share this inventory, we manage a per item session value
	// the user sends its session when he tries to manipulate the inventory. If it is higher than the targeted item session, it is okj
	// The item session is incremented and the highest session value is then sent to the clients
	// sessions are reseted when nobody uses the inventory

	nlassert( user );

	if( canAccessToGuildInventory( user ) == false )
	{
		CCharacter::sendDynamicSystemMessage(user->getEntityRowId(), "CANT_ACCESS_GUILD_INVENTORY");
		return;
	}

	// check if user is trial
	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( user->getId() ));
	BOMB_IF(p == NULL, "Failed to find player record for character: " << user->getId().toString(), return);
	if ( p->isTrialPlayer() )
	{
		user->sendDynamicSystemMessage( user->getId(), "EGS_CANT_USE_GUILD_INV_IS_TRIAL_PLAYER" );
		return;
	}


	CGuildMemberModule * module;
	if ( !user->getModuleParent().getModule(module) || !module->canTakeGuildItem() )
	{
		CCharacter::sendDynamicSystemMessage( user->getId(),"GUILD_ITEM_DONT_HAVE_RIGHTS" );
		return;
	}

	// get the source item
	CInventoryPtr srcItems = (CGuildInventory *)_Inventory;
	if ( slot >= srcItems->getSlotCount() )
	{
		nlwarning( "<swapItem> user %s Invalid guild slot %u, count = %u",user->getId().toString().c_str(), slot, srcItems->getSlotCount() );
		return;
	}
	CGameItemPtr srcItem = srcItems->getItem(slot);
	if ( srcItem == NULL )
	{
		nlwarning( "<swapItem> user %s Invalid guild slot %u, count = %u -> NULL item",user->getId().toString().c_str(), slot, srcItems->getSlotCount() );
		return;
	}

	// check session
	if ( ! _GuildInventoryView->checkSession( slot, session ) )
	{
		CCharacter::sendDynamicSystemMessage( user->getId(),"GUILD_ITEM_BAD_SESSION" );
		return;
	}

	// try to move the required quantity of the item
	if ( CInventoryBase::moveItem(
		_Inventory,	slot,
		user->getInventory(INVENTORIES::bag), INVENTORIES::INSERT_IN_FIRST_FREE_SLOT,
		quantity ) != CInventoryBase::ior_ok )
	{
		CCharacter::sendDynamicSystemMessage( user->getId(),"GUILD_PLAYER_BAG_FULL" );
		return;
	}
}

//----------------------------------------------------------------------------
uint CGuild::selectItems(NLMISC::CSheetId itemSheetId, uint32 quality, std::vector<CItemSlotId> *itemList)
{
	// For all items
	uint	quantitySelected= 0;
	for (uint32 i = 0; i < _Inventory->getSlotCount(); i++)
	{
		CGameItemPtr item = _Inventory->getItem(i);
		if (item == NULL)
			continue;

		// if match, append to the list
		if (item->getSheetId()==itemSheetId && item->quality()>=quality)
		{
			quantitySelected+= item->getStackSize();
			if(itemList)
			{
				CItemSlotId		entry;
				entry.Slot= i;
				entry.Quality= item->quality();
				itemList->push_back(entry);
			}
		}
	}

	return quantitySelected;
}

//----------------------------------------------------------------------------
uint CGuild::destroyItems(const std::vector<CItemSlotId> &itemSlotIns, uint32 maxQuantity)
{
	// none to destroy actually?
	if(maxQuantity==0 || itemSlotIns.empty())
		return 0;

	// If has to destroy only some of them, must sort to take first the ones of lowest quality
	const std::vector<CItemSlotId> *itemSlots= NULL;
	std::vector<CItemSlotId>	itemSlotSorted;
	if(maxQuantity!=uint32(-1))
	{
		itemSlotSorted= itemSlotIns;
		std::sort(itemSlotSorted.begin(), itemSlotSorted.end());
		itemSlots= &itemSlotSorted;
	}
	else
	{
		// just point to the original one
		itemSlots= &itemSlotIns;
	}

	// destroy items up to the maxquantity wanted
	uint	index= 0;
	uint	totalDestroyed= 0;
	while(maxQuantity>0 && index<itemSlotIns.size())
	{
		const CItemSlotId	&itemSlot= (*itemSlots)[index];
		// locate the item
		CGameItemPtr	pItem= getItem(itemSlot.Slot);
		if(pItem!=NULL)
		{
			// destroy
			uint32	quantityToDestroy= maxQuantity;
			quantityToDestroy= min(quantityToDestroy, pItem->getStackSize());

			CGameItemPtr item = _Inventory->removeItem(itemSlot.Slot, quantityToDestroy);
			item.deleteItem();

			// decrease if not infinity
			if(maxQuantity!=-1)
				maxQuantity-= quantityToDestroy;

			// increase count
			totalDestroyed+= quantityToDestroy;
		}

		// next slot to destroy
		index++;
	}

	return totalDestroyed;
}

//----------------------------------------------------------------------------
void	CGuild::takeMoney( CCharacter * user, uint64 money, uint16 session )
{
	nlassert(user);
	
	if( canAccessToGuildInventory( user ) == false )
	{
		CCharacter::sendDynamicSystemMessage(user->getEntityRowId(), "CANT_ACCESS_GUILD_INVENTORY");
		return;
	}
	
	if ( money > _Money )
	{
		nlwarning( "takeMoney guild %u user %s : money = %"NL_I64"u, max = %"NL_I64"u",_Id,user->getId().toString().c_str(),money,_Money);
		return;
	}
	if ( ! _GuildInventoryView->checkMoneySession( session ) )
	{
		CCharacter::sendDynamicSystemMessage( user->getId(),"GUILD_ITEM_BAD_SESSION" );
		return;
	}
	CGuildMemberModule * module;
	if ( !user->getModuleParent().getModule(module) || !module->canTakeGuildItem() )
	{
		CCharacter::sendDynamicSystemMessage( user->getId(),"GUILD_ITEM_DONT_HAVE_RIGHTS" );
		return;
	}

	_GuildInventoryView->updateSessionForMoneyTransaction();
	user->giveMoney( money );
	_Money -= money;
	
//	setClientDBProp( "GUILD:INVENTORY:MONEY",  _Money );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setMONEY(_DbGroup, _Money);

}

//----------------------------------------------------------------------------
void CGuild::putMoney( CCharacter * user, uint64 money, uint16 session )
{
	nlassert(user);
	
	if( canAccessToGuildInventory( user ) == false )
	{
		CCharacter::sendDynamicSystemMessage(user->getEntityRowId(), "CANT_ACCESS_GUILD_INVENTORY");
		return;
	}
	
	if ( money > user->getMoney() )
	{
		nlwarning( "putMoney guild %u user %s : money = %"NL_I64"u, max = %"NL_I64"u",_Id,user->getId().toString().c_str(),money,_Money);
		return;
	}

	_GuildInventoryView->updateSessionForMoneyTransaction();
	user->spendMoney( money );
	_Money += money;
//	setClientDBProp( "GUILD:INVENTORY:MONEY",  _Money );
	CBankAccessor_GUILD::getGUILD().getINVENTORY().setMONEY(_DbGroup, _Money);
}

//----------------------------------------------------------------------------
CGuildMember* CGuild::newMember( const EGSPD::TCharacterId & id, NLMISC::TGameCycle enterTime )
{
	incMemberSession();
	CGuildMember * member = EGS_PD_CAST<CGuildMember *>( EGSPD::CGuildMemberPD::create( id ) );
	EGS_PD_AST( member );
	setMembers(member);

	CGuildManager::getInstance()->storeCharToGuildAssoc(id, getId());

	member->setEnterTime( enterTime == 0 ? CTickEventHandler::getGameCycle() : enterTime );
	member->setGrade( EGSPD::CGuildGrade::Member );
	if ( !_FreeMemberIndexes.empty() )
	{
		member->setMemberIndex( *_FreeMemberIndexes.begin() );
		_FreeMemberIndexes.erase(_FreeMemberIndexes.begin());
	}
	else
		member->setMemberIndex( _NextMemberIndex++ );
	_GradeCounts[EGSPD::CGuildGrade::Member]++;
	CFameManager::getInstance().setPlayerGuild( id, _EId, true );
	setMemberClientDB( member );

	// if character online...
	CCharacter *character = PlayerManager.getChar(id);
	if (character != NULL)
	{
		CGuildCharProxy proxy(character);

		// create the guild member module
		CGuildMemberModule *module = new CGuildMemberModule( proxy, member );
		// build module and set the member as 'online'
		setMemberOnline( member, character->getId().getDynamicId() );
		character->updateTargetingChars();
	}
	return member;
}

//----------------------------------------------------------------------------
void CGuild::removeMember(const EGSPD::TCharacterId &id)
{
	CGuildMember *guildMember = static_cast<CGuildMember *>(getMembers(id));
	BOMB_IF(guildMember == NULL, "CGuild::removeMember : can't find guild member for character "<<id.toString(), return);

	ICharacter *character = ICharacter::getInterface(id, true);
	CGuildMemberModule * module = NULL;
	if (character != NULL && character->getEnterFlag() && character->getModuleParentWrap().getModule( module ))
	{
		nlinfo("CGuild::removeMember : remove online member %s from guild %s", 
			guildMember->getId().toString().c_str(), 
			guildIdToString(getId()).c_str());

		// online character, let the module do the job
		module->quitGuild();
		character->updateTargetingChars();
	}
	else
	{
		nlinfo("CGuild::removeMember : remove offline member %s from guild %s", 
			guildMember->getId().toString().c_str(), 
			guildIdToString(getId()).c_str());

		// offline character, do it the raw way
		deleteMember(guildMember);

		if (getMembers().empty())
		{
			// that was the last member, delete the guild
			CGuildManager::getInstance()->deleteGuild(getId());
		}
	}
}

//----------------------------------------------------------------------------
void CGuild::deleteMember( CGuildMember* member )
{
	nlassert(member);
	nlassert( uint(member->getGrade()) < _GradeCounts.size() );

	if (PlayerManager.getChar(member->getIngameEId()) != NULL)
		setMemberOffline( member );
	incMemberSession();
	uint16 idx = member->getMemberIndex();
	
	// update grade counts
	_GradeCounts[member->getGrade()]--;
	
	TDataSetRow row = TheDataset.getDataSetRow( member->getIngameEId() );
	CGuildManager::getInstance()->removeCharToGuildAssoc(member->getIngameEId(), getId());
	deleteFromMembers( member->getIngameEId() );
	
	_FreeMemberIndexes.insert( idx );
	
	// clear database entry
//	std::string dbBase = NLMISC::toString( "GUILD:MEMBERS:%u:",idx );
	CBankAccessor_GUILD::TGUILD::TMEMBERS::TArray &memberElem = CBankAccessor_GUILD::getGUILD().getMEMBERS().getArray(idx);
//	setClientDBProp( dbBase + "NAME", 0 );
	memberElem.setNAME(_DbGroup, 0 );
//	setClientDBProp( dbBase + "GRADE", 0 );
	memberElem.setGRADE(_DbGroup, 0 );
//	setClientDBProp( dbBase + "ENTER_DATE", 0 );
	memberElem.setENTER_DATE(_DbGroup, 0 );

	// set mirror properties
	if ( TheDataset.isAccessible( row ) )
	{
		CMirrorPropValue<TYPE_GUILD_SYMBOL> mirrorSymbol( TheDataset, row, DSPropertyGUILD_SYMBOL );
		CCharacter* character = PlayerManager.getChar(row);
		if (character)
			character->updateGuildFlag();
		else
			mirrorSymbol = 0;
		CMirrorPropValue<TYPE_GUILD_NAME_ID> mirrorName( TheDataset, row, DSPropertyGUILD_NAME_ID );
		mirrorName = 0;
	}
}

//----------------------------------------------------------------------------
uint16 CGuild::getMaxGradeCount(EGSPD::CGuildGrade::TGuildGrade grade)const
{
	const uint size = _GradeCounts.size();
	uint16 count = 0;
	for ( uint i = 0; i < size; ++i )
		count+=_GradeCounts[i];
	
	switch (grade)
	{
		case EGSPD::CGuildGrade::Leader:
			return 1;
			break;
		case EGSPD::CGuildGrade::HighOfficer:
		{
			return GuildMaxMemberCount;
			/*
			count *= 5;
			if ( count %100 == 0 )
				return count/100;
			else
				return count/100 + 1;
			*/
		}
		break;
		case EGSPD::CGuildGrade::Officer:
		{
			return GuildMaxMemberCount;
			/*
			count *= 10;
			if ( count %100 == 0 )
				return count/100;
			else
				return count/100 + 1;
			*/
		}
	}
	return 0xFFFF;
}

//----------------------------------------------------------------------------
void CGuild::setMemberOffline( CGuildMember * member )
{
	nlassert(member);
	TGroupId idGroup = CHAT_GROUPS_IDS::getGuildChatGroupId(_Id);
	NLNET::CMessage msgout("REMOVE_FROM_GROUP");
	msgout.serial( idGroup );
	EGSPD::TCharacterId id = member->getIngameEId();
	msgout.serial( id );
	sendMessageViaMirror( "IOS", msgout );
	_DbGroup.removeRecipient( id );
	setMemberClientDB(member);
}

//----------------------------------------------------------------------------
void CGuild::setMemberOnline( CGuildMember * member, uint8 dynamicId )
{
	nlassert(member);
	member->setDynamicId( dynamicId );
	addMemberToGuildChat(member);
	_DbGroup.addRecipient( member->getIngameEId() );
	setMemberClientDB(member);

	if( !_MessageOfTheDay.empty() )
	{
		CGuildMemberModule * module = NULL;
		if ( member->getReferencingModule(module) )
		{
			CGuildCharProxy proxy;
			module->getProxy(proxy);
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal= _MessageOfTheDay;
			proxy.sendDynamicMessageToChatGroup("GMOTD", CChatGroup::guild, params);			
		}
	}
	// Link guild inventory in CCharacter to the shared inventory
}

void CGuild::addMemberToGuildChat(CGuildMember *member)
{
	TGroupId idGroup = CHAT_GROUPS_IDS::getGuildChatGroupId(_Id);
	NLNET::CMessage msgout("ADD_TO_GROUP");
	msgout.serial( idGroup );
	EGSPD::TCharacterId id = member->getIngameEId();
	msgout.serial( id );
	sendMessageViaMirror( "IOS", msgout );
}

//----------------------------------------------------------------------------
void CGuild::sendMessageToGuildMembers( const std::string &  msg, const TVectorParamCheck & params )const
{
	for ( std::map< EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::const_iterator it = getMembersBegin(); it != getMembersEnd(); ++it )
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember*>( (*it).second );
		EGS_PD_AST( member );
		
		// continue if the player is offline
		CGuildMemberModule * module = NULL;
		if ( member->getReferencingModule(module) )
		{
			CGuildCharProxy proxy;
			module->getProxy(proxy);
			proxy.sendSystemMessage(msg,params);
		}
	}

	// send the message to peer guild unifiers
	IGuildUnifier::getInstance()->sendMessageToGuildMembers(this, msg, params);
}

//----------------------------------------------------------------------------
void CGuild::sendMessageToGuildChat( const std::string &  msg, const TVectorParamCheck & params )const
{
	for ( std::map< EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::const_iterator it = getMembersBegin(); it != getMembersEnd(); ++it )
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember*>( (*it).second );
		EGS_PD_AST( member );

		// continue if the player is offline
		CGuildMemberModule * module = NULL;
		if ( member->getReferencingModule(module) )
		{
			CGuildCharProxy proxy;
			module->getProxy(proxy);
			proxy.sendDynamicMessageToChatGroup(msg, CChatGroup::guild, params);
		}
	}
}

//----------------------------------------------------------------------------
void CGuild::setMemberClientDB( CGuildMember* member )
{
	// get a module pointing on the member
	nlassert( member );
	
	//const ucstring memberName = NLMISC::CEntityIdTranslator::getInstance()->getByEntity(member->getIngameEId() );
	const uint32 nameId = NLMISC::CEntityIdTranslator::getInstance()->getEntityNameStringId( member->getIngameEId() );
	
//	std::string dbBase = NLMISC::toString( "GUILD:MEMBERS:%u:",member->getMemberIndex() );
	CBankAccessor_GUILD::TGUILD::TMEMBERS::TArray &memberElem = CBankAccessor_GUILD::getGUILD().getMEMBERS().getArray(member->getMemberIndex());

	//setClientDBPropString( dbBase + "NAME", memberName.empty() ? ucstring("Unknown") : memberName);
//	setClientDBProp( dbBase + "NAME", nameId);
	memberElem.setNAME(_DbGroup, nameId );
//	setClientDBProp( dbBase + "GRADE", member->getGrade() );
	memberElem.setGRADE(_DbGroup, member->getGrade() );
//	setClientDBProp( dbBase + "ENTER_DATE", member->getEnterTime() );
	memberElem.setENTER_DATE(_DbGroup, member->getEnterTime() );

	CGuildMemberModule * module = NULL;
	if ( member->getReferencingModule( module ) )
	{
//		setClientDBProp( dbBase + "ONLINE", ccs_online );
		memberElem.setONLINE(_DbGroup, ccs_online );
	}
	else
	{
		// check if foreign online
		if (IShardUnifierEvent::getInstance() && IShardUnifierEvent::getInstance()->isCharacterOnlineAbroad(member->getIngameEId()))
		{
			// foreign online
//			setClientDBProp( dbBase + "ONLINE", ccs_online_abroad );
			memberElem.setONLINE(_DbGroup, ccs_online_abroad );
		}
		else
		{
//			setClientDBProp( dbBase + "ONLINE", ccs_offline );
			memberElem.setONLINE(_DbGroup, ccs_offline );
		}
		// DO NOT access to the CCharacter object if the player is offline or foreign online
		// because the object already is destroyed here
		// even if it still returned by PlayerManager.getChar()
		return;
	}
	
	// Set guild-related mirror and client database properties of the character
	TDataSetRow row = TheDataset.getDataSetRow( member->getIngameEId() );
	if ( TheDataset.isAccessible( row ) )
	{
		CMirrorPropValue<TYPE_GUILD_SYMBOL> mirrorSymbol( TheDataset, row, DSPropertyGUILD_SYMBOL );
		CCharacter* character = PlayerManager.getChar(row);
		if (character)
		{
			character->updateGuildFlag();
			character->updateOutpostAdminFlagInDB();
		}
		else
		{
			mirrorSymbol = getIcon();
		}
		CMirrorPropValue<TYPE_GUILD_NAME_ID> mirrorName( TheDataset, row, DSPropertyGUILD_NAME_ID );
		mirrorName = getNameId();
	}
}

//----------------------------------------------------------------------------
const EGSPD::TCharacterId CGuild::getHighestGradeOnlineUser() const
{
	// best successor is the member with best grade. If more than 1 user fits, take the older in the guild 
	const CGuildMember * best = NULL;
	for (std::map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::const_iterator it = getMembersBegin();
	it != getMembersEnd();
	++it  )
	{
		const CGuildMember * member = EGS_PD_CAST<CGuildMember*>( (*it).second );
		EGS_PD_AST( member );
		
		// check if the current member is the successor
		if ( best == NULL ||
			member->getGrade() < best->getGrade() ||
			( member->getGrade() == best->getGrade() && member->getEnterTime() < best->getEnterTime() ) )
		{
			best = member;
		}
	}
	if ( best )
		return best->getIngameEId();
	return NLMISC::CEntityId::Unknown;
}

//-----------------------------------------------------------------------------
void CGuild::updateMembersStringIds()
{
	// Optimized property access (part 1)
//	static ICDBStructNode *membersArray = _DbGroup.Database.getICDBStructNodeFromName("GUILD:MEMBERS");
	CBankAccessor_GUILD::TGUILD::TMEMBERS &memberDb = CBankAccessor_GUILD::getGUILD().getMEMBERS();
//	BOMB_IF(membersArray==NULL, "GUILD:MEMBERS not found in database.xml for guild "<< guildIdToString(getId())<<".", return);
//	static ICDBStructNode::CTextId nameTextId = ICDBStructNode::CTextId("NAME");
//	static ICDBStructNode *nodeOfNameOfMember0 = membersArray->getNode( 0 )->getNode(nameTextId, false);
//	BOMB_IF(nodeOfNameOfMember0==NULL, "NAME not found under GUILD:MEMBERS:0", return);
//	static uint indexOfNameInMember;
//	static bool result = membersArray->getNode( 0 )->getNodeIndex( nodeOfNameOfMember0, indexOfNameInMember );
//	BOMB_IF(!result, "Node index not found for guild "<< guildIdToString(getId())<<".", return);

	map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD *>::iterator it;
	for (it = getMembersBegin(); it != getMembersEnd(); ++it)
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember *>((*it).second);
		EGS_PD_AST( member );

		const uint32 nameId = CEntityIdTranslator::getInstance()->getEntityNameStringId( member->getIngameEId() );
//		ICDBStructNode *memberNode = membersArray->getNode( member->getMemberIndex() ); // asserts if out of bounds

//		// Optimized property access (part 2)
//		ICDBStructNode *nodeOfNameOfMember = memberNode->getNode( (uint16)indexOfNameInMember );
//		_DbGroup.Database.setProp( nodeOfNameOfMember, nameId );

		CBankAccessor_GUILD::getGUILD().getMEMBERS().getArray(member->getMemberIndex()).setNAME(_DbGroup, nameId);
	}
}

//-----------------------------------------------------------------------------
bool CGuild::setMemberGrade(CGuildMember * member, EGSPD::CGuildGrade::TGuildGrade grade, NLMISC::CLog * log, const NLMISC::CEntityId & csrEId)
{
	nlassert(member);

	const bool msgCSR = (csrEId != CEntityId::Unknown);
	const string memberName = CEntityIdTranslator::getInstance()->getByEntity( member->getIngameEId() ).toUtf8();

	if (grade == EGSPD::CGuildGrade::Unknown)
	{
		if (log)
			log->displayNL("Unknown grade");

		if (msgCSR)
			CCharacter::sendDynamicSystemMessage(csrEId, "CSR_GUILD_UNKNOWN_GRADE");

		return false;
	}

	const EGSPD::CGuildGrade::TGuildGrade oldGrade = member->getGrade();
	if (oldGrade == grade)
	{
		if (log)
			log->displayNL("Member '%s' already has grade '%s'", memberName.c_str(), EGSPD::CGuildGrade::toString(grade).c_str() );

		if (msgCSR)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal.fromUtf8( memberName );
			CCharacter::sendDynamicSystemMessage(csrEId, "CSR_GUILD_ALREADY_HAS_GRADE", params);
		}

		return true;
	}

	if ( !CGuildManager::getInstance()->isGMGuild( getId() ) && getGradeCount(grade) >= getMaxGradeCount(grade) )
	{
		if (log)
			log->displayNL("Cannot set '%s' as '%s' because max count for this grade (%hu) has been reached",
				memberName.c_str(),
				EGSPD::CGuildGrade::toString(grade).c_str(),
				getMaxGradeCount(grade)
				);

		if (msgCSR)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = getMaxGradeCount(grade);
			CCharacter::sendDynamicSystemMessage(csrEId, "CSR_GUILD_MAX_GRADE_COUNT", params);
		}

		return false;
	}

	member->setMemberGrade(grade);
	incGradeCount(grade);
	decGradeCount(oldGrade);

	// if member is online, module must be rebuilt for new grade
	CGuildMemberModule * module = NULL;
	if ( member->getReferencingModule(module) )
	{
		CGuildCharProxy proxy;
		module->getProxy( proxy );
		member->removeReferencingModule( module );
		module->onReferencedDestruction();

		MODULE_AST( CGuildMemberModule::createModule(proxy, member) );
		setMemberClientDB( member );
	}

	if (log)
		log->displayNL("Member '%s' now has grade '%s'", memberName.c_str(), EGSPD::CGuildGrade::toString(grade).c_str() );

	if (msgCSR)
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::literal, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8( memberName );
		params[1].Literal = EGSPD::CGuildGrade::toString(grade);
		CCharacter::sendDynamicSystemMessage(csrEId, "CSR_GUILD_NEW_GRADE", params);
	}

	return true;
}

//-----------------------------------------------------------------------------
CGuildMember * CGuild::getMemberFromEId(NLMISC::CEntityId eId)
{
	map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD *>::iterator it;
	for (it = getMembersBegin(); it != getMembersEnd(); ++it)
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember *>( (*it).second );
		EGS_PD_AST(member);

		if (eId == member->getIngameEId())
			return member;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
CGuildMember * CGuild::getLeader()
{
	map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD *>::iterator it;
	for (it = getMembersBegin(); it != getMembersEnd(); ++it)
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember *>( (*it).second );
		EGS_PD_AST(member);

		if (member->getGrade() == EGSPD::CGuildGrade::Leader)
			return member;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
uint8 CGuild::getAndSyncItemInfoVersion( uint32 slot, const NLMISC::CEntityId& characterId )
{
	uint8 infoVersion = _GuildInventoryView->getItemInfoVersion( slot );
	CGuildMember* member = EGS_PD_CAST<CGuildMember*>(getMembers( characterId ));
	CGuildMemberModule *onlineMember = NULL;
	if ( member->getReferencingModule( onlineMember ) ) // contains slow dynamic cast :(
		onlineMember->setLastSentInfoVersion( slot, infoVersion );
	
	return infoVersion;
}

//-----------------------------------------------------------------------------
bool CGuild::canAddOutpost() const
{
	uint32 outpostCount = getOutpostCount();
	if (outpostCount < GuildMaxOutpostCount.get() && outpostCount < OUTPOSTENUMS::MAX_OUTPOST)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
bool CGuild::canGiveUpOutpost() const
{
	return !canAddOutpost();
}

//-----------------------------------------------------------------------------
void CGuild::updateGUILD_OUTPOST_CANDEL()
{
	bool canDel = canGiveUpOutpost();
//	setClientDBProp("GUILD:OUTPOST:CANDEL", canDel);
	CBankAccessor_GUILD::getGUILD().getOUTPOST().setCANDEL(_DbGroup, canDel);
}

//-----------------------------------------------------------------------------
void CGuild::addOwnedOutpost(TAIAlias outpostAlias)
{
	H_AUTO(CGuild_addOwnedOutpost);

	for (uint i = 0; i < _OwnedOutposts.size(); ++i)
	{
		// just return if guild already owns this outpost
		if (_OwnedOutposts[i] == outpostAlias)
			return;
	}

	COutpost* outpost = COutpostManager::getInstance().getOutpostFromAlias(outpostAlias);
	if (outpost)
	{
		// only check the "hard" limit of outposts per guild (client database limit)
		// GuildMaxOutpostCount can be decreased and guilds with too many outposts can keep them
		nlassert(getOutpostCount() < OUTPOSTENUMS::MAX_OUTPOST);

		_OwnedOutposts.push_back(outpostAlias);

		// update all outposts following the new one (included)
		// it will shift right
		uint i = (uint)_OwnedOutposts.size() - 1;
		while (i < _OwnedOutposts.size() + _ChallengedOutposts.size())
		{
			if (!updateOutpostDB(i))
			{
				DEBUG_STOP;
			}
			++i;
		}
		// Open guild inventories
		std::map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator itMember, itMemberEnd;
		for (itMember=_Members.begin(), itMemberEnd=_Members.end(); itMember!=itMemberEnd; ++itMember)
		{
			CCharacter* user = PlayerManager.getChar(itMember->first);
			if (user && outpost->contains(user, false))
				PlayerManager.sendImpulseToClient(user->getId(), "GUILD:OPEN_INVENTORY");
		}

		updateGUILD_OUTPOST_CANDEL();
	}
}

//-----------------------------------------------------------------------------
void CGuild::removeOwnedOutpost(TAIAlias outpostAlias)
{
	H_AUTO(CGuild_removeOwnedOutpost);

	COutpost* outpost = COutpostManager::getInstance().getOutpostFromAlias(outpostAlias);
	if (outpost)
	{
		// Close guild inventories
		std::map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator itMember, itMemberEnd;
		for (itMember=_Members.begin(), itMemberEnd=_Members.end(); itMember!=itMemberEnd; ++itMember)
		{
			CCharacter* user = PlayerManager.getChar(itMember->first);
			if (user && outpost->contains(user, false))
				PlayerManager.sendImpulseToClient(user->getId(), "GUILD:CLOSE_INVENTORY");
		}
	}
	
	uint i = 0;
	while (i < _OwnedOutposts.size())
	{
		if (_OwnedOutposts[i] == outpostAlias)
			break;
		++i;
	}
	if (i == _OwnedOutposts.size())
		return;

	_OwnedOutposts.erase(_OwnedOutposts.begin() + i);

	// update all outposts following the removed one
	// it will shift left
	while (i < _OwnedOutposts.size() + _ChallengedOutposts.size())
	{
		if (!updateOutpostDB(i))
		{
			DEBUG_STOP;
		}
		++i;
	}

	// clear the last one (freed by the shift)
	COutpostGuildDBEraserPtr dbEraser = new COutpostGuildDBEraser(&_DbGroup, i);
	if (dbEraser != NULL)
		dbEraser->clearOutpostGuildDB();

	updateGUILD_OUTPOST_CANDEL();
}

//-----------------------------------------------------------------------------
void CGuild::addChallengedOutpost(TAIAlias outpostAlias)
{
	H_AUTO(CGuild_addChallengedOutpost);

	for (uint i = 0; i < _ChallengedOutposts.size(); ++i)
	{
		// just return if guild already owns this outpost
		if (_ChallengedOutposts[i] == outpostAlias)
			return;
	}

	COutpost* outpost = COutpostManager::getInstance().getOutpostFromAlias(outpostAlias);
	if (outpost)
	{
		// only check the "hard" limit of outposts per guild (client database limit)
		// GuildMaxOutpostCount can be decreased and guilds with too many outposts can keep them
		nlassert(getOutpostCount() < OUTPOSTENUMS::MAX_OUTPOST);

		_ChallengedOutposts.push_back(outpostAlias);
		updateOutpostDB( (uint32)(_OwnedOutposts.size()+_ChallengedOutposts.size()-1) );

		updateGUILD_OUTPOST_CANDEL();
	}
}

//-----------------------------------------------------------------------------
void CGuild::removeChallengedOutpost(TAIAlias outpostAlias)
{
	H_AUTO(CGuild_removeChallengedOutpost);

	uint i = 0;
	while (i < _ChallengedOutposts.size())
	{
		if (_ChallengedOutposts[i] == outpostAlias)
			break;
		++i;
	}
	if (i == _ChallengedOutposts.size())
		return;

	_ChallengedOutposts.erase(_ChallengedOutposts.begin() + i);

	// update all outposts following the removed one
	// it will shift left
	i += (uint)_OwnedOutposts.size();
	while (i < _OwnedOutposts.size() + _ChallengedOutposts.size())
	{
		if (!updateOutpostDB(i))
		{
			DEBUG_STOP;
		}
		++i;
	}

	// clear the last one (freed by the shift)
	COutpostGuildDBEraserPtr dbEraser = new COutpostGuildDBEraser(&_DbGroup, i);
	if (dbEraser != NULL)
		dbEraser->clearOutpostGuildDB();

	updateGUILD_OUTPOST_CANDEL();
}

//-----------------------------------------------------------------------------
uint32 CGuild::getOutpostCount() const
{
	return (uint32)(_OwnedOutposts.size() + _ChallengedOutposts.size());
}

//-----------------------------------------------------------------------------
void CGuild::getOwnedOutposts(std::vector<TAIAlias> & ownedOutposts) const
{
	ownedOutposts = _OwnedOutposts;
}

//-----------------------------------------------------------------------------
void CGuild::getChallengedOutposts(std::vector<TAIAlias> & challengedOutposts) const
{
	challengedOutposts = _ChallengedOutposts;
}

//-----------------------------------------------------------------------------
COutpostGuildDBUpdaterPtr CGuild::getOutpostGuildDBUpdater(COutpost * outpost)
{
	nlassert(outpost != NULL);

	uint32 outpostIndex;
	bool ownedOutpost;
	if (!getOutpostDBIndex(outpost->getAlias(), outpostIndex, ownedOutpost))
	{
		nlwarning("cannot find the outpost %s the owned/challenged outposts of the guild %u",
			CPrimitivesParser::aliasToString(outpost->getAlias()).c_str(),
			getId()
			);
		DEBUG_STOP;
		return NULL;
	}

	return getOutpostGuildDBUpdaterDetailed(outpost, outpostIndex, ownedOutpost);
}

//-----------------------------------------------------------------------------
COutpostGuildDBUpdaterPtr CGuild::getOutpostGuildDBUpdaterDetailed(COutpost * outpost, uint32 outpostIndex, bool ownedOutpost)
{
	nlassert(outpost != NULL);

	bool ownerGuild;
	if (getId() == outpost->getOwnerGuild())
	{
		ownerGuild = true;
	}
	else if (getId() == outpost->getAttackerGuild())
	{
		ownerGuild = false;
	}
	else
	{
		nlwarning("the outpost %s is neither owned nor challenged by the guild %u",
			CPrimitivesParser::aliasToString(outpost->getAlias()).c_str(),
			getId()
			);
		DEBUG_STOP;
		return NULL;
	}

	// check data consistency
	if (ownedOutpost != ownerGuild)
	{
		nlwarning("data of the outpost %s are not consistent with data of the guild %u : (ownedOutpost=%s, ownerGuild=%s)",
			CPrimitivesParser::aliasToString(outpost->getAlias()).c_str(),
			getId(),
			ownedOutpost ? "true":"false",
			ownerGuild ? "true":"false"
			);
		DEBUG_STOP;
	}

	OUTPOSTENUMS::TPVPSide side = ownerGuild ? OUTPOSTENUMS::OutpostOwner : OUTPOSTENUMS::OutpostAttacker;

	return new COutpostGuildDBUpdater(&_DbGroup, outpostIndex, outpost, side);
}

//-----------------------------------------------------------------------------
bool CGuild::getOutpostDBIndex(TAIAlias outpostAlias, uint32 & outpostIndex, bool & ownedOutpost) const
{
	for (uint i = 0; i < _OwnedOutposts.size(); ++i)
	{
		if (_OwnedOutposts[i] == outpostAlias)
		{
			outpostIndex = i;
			ownedOutpost = true;
			return true;
		}
	}

	for (uint i = 0; i < _ChallengedOutposts.size(); ++i)
	{
		if (_ChallengedOutposts[i] == outpostAlias)
		{
			outpostIndex = (uint32)_OwnedOutposts.size() + i;
			ownedOutpost = false;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool CGuild::updateOutpostDB(uint32 outpostIndex)
{
	TAIAlias outpostAlias;
	bool ownedOutpost;

	uint32 i = outpostIndex;
	if (i < _OwnedOutposts.size())
	{
		outpostAlias = _OwnedOutposts[i];
		ownedOutpost = true;
	}
	else
	{
		i -= (uint32)_OwnedOutposts.size();
		if (i < _ChallengedOutposts.size())
		{
			outpostAlias = _ChallengedOutposts[i];
			ownedOutpost = false;
		}
		else
			return false;
	}

	COutpost * outpost = COutpostManager::getInstance().getOutpostFromAlias(outpostAlias);
	if (outpost != NULL)
	{
		COutpostGuildDBUpdaterPtr dbUpdater = getOutpostGuildDBUpdaterDetailed(outpost, outpostIndex, ownedOutpost);
		if (dbUpdater != NULL)
		{
			dbUpdater->updateOutpostGuildDB();
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
CGuild::TAllegiances CGuild::getAllegiance() const
{ 
	return std::make_pair( _DeclaredCult, _DeclaredCiv ); 
}

//-----------------------------------------------------------------------------
bool CGuild::setDeclaredCult(PVP_CLAN::TPVPClan newClan, bool noCheck)
{
	if (newClan == PVP_CLAN::None || newClan == PVP_CLAN::Neutral
		|| ((newClan >= PVP_CLAN::BeginCults && newClan <= PVP_CLAN::EndCults)
		&& (noCheck 
			|| (CFameInterface::getInstance().getFameIndexed(getEId(),PVP_CLAN::getFactionIndex(newClan)) >= FameMinToDeclare) 
			|| _DeclaredCult == PVP_CLAN::None) ))
	{
		if( newClan != PVP_CLAN::None 
			&& newClan != PVP_CLAN::Neutral 
			&& CFameInterface::getInstance().getFameIndexed(getEId(),PVP_CLAN::getFactionIndex(newClan)) < FameMinToDeclare 
			&& _DeclaredCult == PVP_CLAN::None)
		{
			CFameInterface::getInstance().addFameIndexed(getEId(),PVP_CLAN::getFactionIndex(newClan), FameMinToDeclare - CFameInterface::getInstance().getFameIndexed(getEId(),PVP_CLAN::getFactionIndex(newClan)));

			// We don't inform the client right now, the timer will take care of this
			//character->sendEventForMissionAvailabilityCheck();
		}
		_DeclaredCult = newClan;
		CFameManager::getInstance().enforceFameCaps(getEId(), getAllegiance());
		// Go through membership list, change anyone who doesn't fit in guild to "None".
		verifyGuildmembersAllegiance();

		// write new allegiance in database
//		setClientDBProp("GUILD:FAME:CULT_ALLEGIANCE", newClan);
		CBankAccessor_GUILD::getGUILD().getFAME().setCULT_ALLEGIANCE(_DbGroup, newClan);

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool CGuild::setDeclaredCiv(PVP_CLAN::TPVPClan newClan, bool noCheck)
{
	if (newClan == PVP_CLAN::None || newClan == PVP_CLAN::Neutral
		|| ((newClan >= PVP_CLAN::BeginCivs && newClan <= PVP_CLAN::EndCivs)
		&& (noCheck 
			|| (CFameInterface::getInstance().getFameIndexed(getEId(),PVP_CLAN::getFactionIndex(newClan)) >= FameMinToDeclare) 
			|| _DeclaredCiv == PVP_CLAN::None) ))
	{
		if( newClan != PVP_CLAN::None 
			&& newClan != PVP_CLAN::Neutral 
			&& CFameInterface::getInstance().getFameIndexed(getEId(),PVP_CLAN::getFactionIndex(newClan)) < FameMinToDeclare 
			&& _DeclaredCiv == PVP_CLAN::None)
		{
			CFameInterface::getInstance().addFameIndexed(getEId(),PVP_CLAN::getFactionIndex(newClan), FameMinToDeclare - CFameInterface::getInstance().getFameIndexed(getEId(),PVP_CLAN::getFactionIndex(newClan)));

			// We don't inform the client right now, the timer will take care of this
			//character->sendEventForMissionAvailabilityCheck();
		}
		_DeclaredCiv = newClan;
		CFameManager::getInstance().enforceFameCaps(getEId(), getAllegiance());
		// Go through membership list, change anyone who doesn't fit in guild to "None".
		verifyGuildmembersAllegiance();

		// write new allegiance in database
//		setClientDBProp("GUILD:FAME:CIV_ALLEGIANCE", newClan);
		CBankAccessor_GUILD::getGUILD().getFAME().setCIV_ALLEGIANCE(_DbGroup, newClan);

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void CGuild::setAllegianceFromIndeterminedStatus(PVP_CLAN::TPVPClan allegiance)
{
	switch(allegiance)
	{
	case PVP_CLAN::Kami:
	case PVP_CLAN::Karavan:
		if(_DeclaredCult == PVP_CLAN::None)
		{
			setDeclaredCult(PVP_CLAN::Neutral);
			return;
		}
		nlwarning("Only guild with indefinined status in there cult allegiance can do that for become neutral, check the client code !");
		return;
	case PVP_CLAN::Fyros:
	case PVP_CLAN::Matis:
	case PVP_CLAN::Tryker:
	case PVP_CLAN::Zorai:
		if(_DeclaredCiv == PVP_CLAN::None)
		{
			setDeclaredCiv(PVP_CLAN::Neutral);
			return;
		}
		nlwarning("Only guild with indefinined status in there civ allegiance can do that for become neutral, check the client code !");
		return;
		
	default:
		nlwarning("Received wrong allegiance '%s'", PVP_CLAN::toString(allegiance).c_str());
		return;
	}
}

//-----------------------------------------------------------------------------
bool CGuild::verifyClanAllegiance(PVP_CLAN::TPVPClan theClan, sint32 newFameValue)
{
	// If the value is too low...
	if (newFameValue < FameMinToRemain)
	{
		// Check to see if cult or civilization matches.
		if (theClan == _DeclaredCult)
		{
			// Set declaration to Neutral.
			setDeclaredCult(PVP_CLAN::Neutral);

			return false;
		}
		else
		{
			if (theClan == _DeclaredCiv)
			{
				// Set declaration to Neutral.
				setDeclaredCiv(PVP_CLAN::Neutral);

				return false;
			}
		}
	}
	
	return true;
}

//-----------------------------------------------------------------------------
void CGuild::verifyGuildmembersAllegiance()
{
	std::map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::iterator it;
	for (it = this->getMembersBegin(); it != this->getMembersEnd(); ++it)
	{
		CCharacter *c = PlayerManager.getChar((*it).first);
		if (c)
		{
			// Player is logged on.
			c->canBelongToGuild((uint32)(getEId().getShortId()), true);
		}
	}
}

//-----------------------------------------------------------------------------
void CGuild::setFameValueGuild(uint32 factionIndex, sint32 guildFame, sint32 fameMax, uint8 fameTrend)
{
	H_AUTO(setFameValueGuild);
	uint32 fameIndexInDatabase = CStaticFames::getInstance().getDatabaseIndex(factionIndex);
	// guild fame in database are limited to civ and cult
	if( fameIndexInDatabase < PVP_CLAN::NbClans - PVP_CLAN::BeginClans )
	{
		if (guildFame != NO_FAME)
		{
//			setClientDBProp( toString("GUILD:FAME:%d:VALUE", fameIndexInDatabase), sint64(float(guildFame)/FameAbsoluteMax*100) );
			CBankAccessor_GUILD::getGUILD().getFAME().getArray(fameIndexInDatabase).setVALUE(_DbGroup, sint8(float(guildFame)/FameAbsoluteMax*100));
//			setClientDBProp( toString("GUILD:FAME:%d:TREND", fameIndexInDatabase), fameTrend );
			CBankAccessor_GUILD::getGUILD().getFAME().getArray(fameIndexInDatabase).setTREND(_DbGroup, fameTrend);
		}
		else
		{
//			setClientDBProp( toString("GUILD:FAME:%d:VALUE", fameIndexInDatabase), 0);
			CBankAccessor_GUILD::getGUILD().getFAME().getArray(fameIndexInDatabase).setVALUE(_DbGroup, 0);
//			setClientDBProp( toString("GUILD:FAME:%d:TREND", fameIndexInDatabase), 0);
			CBankAccessor_GUILD::getGUILD().getFAME().getArray(fameIndexInDatabase).setTREND(_DbGroup, 0);
		}
//		setClientDBProp( toString("GUILD:FAME:%d:THRESHOLD", fameIndexInDatabase), sint64(float(fameMax)/FameAbsoluteMax*100) );
		CBankAccessor_GUILD::getGUILD().getFAME().getArray(fameIndexInDatabase).setTHRESHOLD(_DbGroup, sint8(float(fameMax)/FameAbsoluteMax*100));
	}
}

//-----------------------------------------------------------------------------
void CGuild::resetFameDatabase()
{
	H_AUTO(resetFameDatabase);
	CFameInterface &fi = CFameInterface::getInstance();
	
	for (uint i=0; i< CStaticFames::getInstance().getNbFame(); ++i)
	{
		// update player fame info
		sint32 fame = fi.getFameIndexed(getEId(), i);
		sint32 maxFame = CFameManager::getInstance().getMaxFameByFactionIndex(getAllegiance(), i);
		setFameValueGuild(i, fame, maxFame, 0);
	}
}

//-----------------------------------------------------------------------------
void CGuild::setStartFameAndAllegiance( const CEntityId& guildCreator )
{
	// set guild allegiance fame and declaring guild allegiance
	CCharacter * c = PlayerManager.getChar( guildCreator );
	nlassert( c != 0 );
	TAllegiances allegiance = c->getAllegiance();
	if( allegiance.first != PVP_CLAN::Neutral && allegiance.first != PVP_CLAN::None )
	{
		uint32 factionIndex = PVP_CLAN::getFactionIndex(allegiance.first);
		CFameManager::getInstance().setEntityFame(getEId(), factionIndex, FameMinToDeclare);
		setDeclaredCult( allegiance.first );
	}
	if( allegiance.second != PVP_CLAN::Neutral && allegiance.second != PVP_CLAN::None )
	{
		uint32 factionIndex = PVP_CLAN::getFactionIndex(allegiance.second);
		CFameManager::getInstance().setEntityFame(getEId(), factionIndex, FameMinToDeclare);
		setDeclaredCiv( allegiance.second );
	}

	for (uint i=0; i < CStaticFames::getInstance().getNbFame(); ++i)
	{
		if( i != PVP_CLAN::getFactionIndex(allegiance.first) && i != PVP_CLAN::getFactionIndex(allegiance.second) )
		{
			// update guild fame
			CFameManager::getInstance().setEntityFame(getEId(), i, 0);
		}
	}
	CFameManager::getInstance().enforceFameCaps( getEId(), allegiance ); 
}


//-----------------------------------------------------------------------------
CCDBSynchronised &CGuild::getClientDB()
{
	return _DbGroup.Database;
}

//-----------------------------------------------------------------------------
IGuild *IGuild::getGuildInterface(CGuild *guild)
{
	return guild;
}

//-----------------------------------------------------------------------------
IGuild *IGuild::getGuildInterface(EGSPD::CGuildPD *guildPd)
{
	return EGS_PD_CAST<CGuild*>(guildPd);
}

//-----------------------------------------------------------------------------
void IGuild::setNameWrap(const ucstring &name)
{
	static_cast<CGuild*>(this)->setName(name);
}

//-----------------------------------------------------------------------------
const ucstring	&IGuild::getNameWrap()
{
	return static_cast<CGuild*>(this)->getName();
}


uint32	IGuild::getIdWrap()
{
	return static_cast<CGuild*>(this)->getId();
}

bool	IGuild::isProxyWrap()
{
	return static_cast<CGuild*>(this)->isProxy();
}


void	IGuild::updateMembersStringIds()
{
	static_cast<CGuild*>(this)->updateMembersStringIds();
}


//-----------------------------------------------------------------------------
/**
 * This class is used to load old guild inventory, DO NOT BREAK IT!
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class COldGuildInventoryLoader
{
public:
	DECLARE_PERSISTENCE_METHODS

	/// ctor
	COldGuildInventoryLoader(const CInventoryPtr & inv) : _Inventory(inv) {}

	/// load an item in the inventory from pdr
	void loadItem(CPersistentDataRecord & pdr)
	{
		CGameItemPtr itm;
		CGameItem::CPersistentApplyArg applyArgs;
		itm.newItem()->apply(pdr, applyArgs);
		BOMB_IF( itm->getStackSize() == 0, "COldGuildInventoryLoader::addItem load a empty stack from PDR", itm.deleteItem(); return );

		// ANTIBUG: remove items with unknown sheet
		if (itm->getSheetId() == CSheetId::Unknown)
		{
			nlwarning("found unknown item in inventory '%s' at slot %u",
				INVENTORIES::toString(_Inventory->getInventoryId()).c_str(),
				itm->getInventorySlot()
				);
			itm.deleteItem();
			return;
		}

		if (_Inventory->getFreeSlotCount() > 0)
		{
			_Inventory->forceLoadItem(itm, applyArgs.InventorySlot);
		}
		else
		{
			STOP(NLMISC::toString("Failed to fit item loaded from input into inventory! (sheet=%s)", itm->getSheetId().toString().c_str()));
			itm.deleteItem();
		}
	}

	/// clear inventory
	void clear()
	{
		_Inventory->clearInventory();
	}

private:
	CInventoryPtr _Inventory;
};

#define PERSISTENT_MACROS_AUTO_UNDEF

//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS COldGuildInventoryLoader

#define PERSISTENT_PRE_APPLY\
	H_AUTO(COldGuildInventoryLoaderApply);\

#define PERSISTENT_DATA\
	FLAG0(CLEAR, clear())\
	LSTRUCT_VECT(Child, if (0), ;/* do not store in old format anymore */, loadItem(pdr))

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"



//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CGuild

#define PERSISTENT_PRE_STORE\
	nlassert(!isProxy());\
	CGuildPD::store(pdr);\

#define PERSISTENT_PRE_APPLY\
	CGuildPD::apply(pdr);\
	pdr.rewind();\

#define PERSISTENT_POST_APPLY\
	CGuildVersionAdapter::getInstance()->adaptGuildFromVersion(*this);\

/*
	Token "_Inventory" was used to save old guild inventory, we still use it to load old guild saves (DO NOT suppress it).
	New token "GuildInventory" is now used for new inventory format.
*/
#define PERSISTENT_DATA\
	PROP2(_Name,string,getName().toUtf8(),ucstring s; s.fromUtf8(val); setName(s))\
	PROP2(_Description,string,getDescription().toUtf8(),ucstring s; s.fromUtf8(val); setDescription(s))\
	PROP2(_MessageOfTheDay,string,_MessageOfTheDay.toUtf8(),ucstring s; s.fromUtf8(val); _MessageOfTheDay=s)\
	LSTRUCT2(_Inventory, if (0), ;/* do not store in old format anymore */, COldGuildInventoryLoader((CGuildInventory *)_Inventory).apply(pdr))\
	STRUCT2(GuildInventory, _Inventory->store(pdr), _Inventory->apply(pdr, NULL))\
\
	PROP2(DeclaredCult,string,PVP_CLAN::toString(_DeclaredCult),_DeclaredCult=PVP_CLAN::fromString(val))\
	PROP2(DeclaredCiv,string,PVP_CLAN::toString(_DeclaredCiv),_DeclaredCiv=PVP_CLAN::fromString(val))\


//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

