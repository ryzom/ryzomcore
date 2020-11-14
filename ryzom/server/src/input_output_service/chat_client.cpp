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
#include "chat_client.h"
#include "input_output_service.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

CVariable<bool>	VerboseChat("ios","VerboseChat", "Set verbosity for chat debugging", false, 0, true);
extern CVariable<bool>	VerboseChatManagement;

//-----------------------------------------------
//	CChatClient
//
//-----------------------------------------------
CChatClient::CChatClient( const TDataSetRow& index )
//:	_SayAudience(CChatGroup::say, ""),
//	_ShoutAudience(CChatGroup::shout, "")
{
	_DataSetIndex = index;
	_Id = TheDataset.getEntityId(index);

	// create fake groupe id for say and sout audience
	_SayAudienceId = CEntityId(RYZOMID::chatGroup, _Id.getShortId()|(uint64(1)<<(CEntityId::ID_SIZE-1)));
	_ShoutAudienceId = CEntityId(RYZOMID::chatGroup, _Id.getShortId()|(uint64(1)<<(CEntityId::ID_SIZE-2)));
	// register the client chat groups
	CChatManager &cm = IOS->getChatManager();
	cm.addGroup(_SayAudienceId, CChatGroup::say, "");
	cm.addGroup(_ShoutAudienceId, CChatGroup::shout, "");

	_Muted = false; 
	_ChatMode = CChatGroup::say;
	_SayLastAudienceUpdateTime = CTime::getLocalTime();
	_ShoutLastAudienceUpdateTime = _SayLastAudienceUpdateTime;
	_AudienceUpdatePeriod = 1000;

	_DynChatChan = 0;

} // CChatClient //

CChatClient::~CChatClient()
{
	CChatManager &cm = IOS->getChatManager();
	cm.removeGroup(_SayAudienceId);
	cm.removeGroup(_ShoutAudienceId);
}


void CChatClient::setChatMode( CChatGroup::TGroupType mode, TChanID dynChatChan) 
{ 
	_ChatMode = mode; 
	_DynChatChan = dynChatChan;
}

/*
// store chat group subscribe
void CChatClient::subscribeInChatGroup(CChatGroup::TGroupType type, TGroupId groupId)
{
	CChatGroup::TGroupType type = groupId.getType();
	_AllChatGroups.
}
// store chat group unsubscribe
void CChatClient::unsubscribeInChatGroup(CChatGroup::TGroupType type, TGroupId groupId);
{
}
*/


//-----------------------------------------------
//	mute
//
//-----------------------------------------------
void CChatClient::mute( sint32 delay )
{
	if( _Muted )
	{
		_Muted = false;
	}
	else
	{
		_Muted = true;
		_MuteStartTime = CTime::getLocalTime();
		_MuteDelay = delay;
	}

} // mute //



//-----------------------------------------------
//	isMuted
//
//-----------------------------------------------
bool CChatClient::isMuted()
{
	if( !_Muted )
	{
		return false;
	}
	else
	{
		if( _MuteDelay != -1 )
		{
			TTime time = CTime::getLocalTime();
			if( time > _MuteStartTime + _MuteDelay * 60 * 1000 )
			{
				_Muted = false;
				return false;
			}
		}
		return true;		
	}

} // isMuted //



//-----------------------------------------------
//	ignore :
//
//-----------------------------------------------
void CChatClient::setIgnoreStatus( const NLMISC::CEntityId &id, bool ignored)
{
	TIgnoreListCont::iterator itIgnore = _IgnoreList.find(id);
	if (ignored)
	{
		if( itIgnore == _IgnoreList.end() )
		{		
			_IgnoreList.insert( id );
		}
		
	}
	else
	{
		if( itIgnore != _IgnoreList.end() )
		{
			_IgnoreList.erase( itIgnore );
		}
	}
} // ignore //


//-----------------------------------------------
//	isInIgnoreList :
//
//-----------------------------------------------
bool CChatClient::isInIgnoreList( const NLMISC::CEntityId &id )
{
	TIgnoreListCont::const_iterator itIgnore = _IgnoreList.find(id);
	if( itIgnore != _IgnoreList.end() )
	{
		return true;
	}
	else
	{
		return false;
	}

} // isInIgnoreList //


//-----------------------------------------------
//	isInIgnoreList :
//
//-----------------------------------------------
bool CChatClient::isInIgnoreList( const TDataSetRow &id )
{
	const NLMISC::CEntityId &ei = IOS->DataSet->getEntityId(id);
	if (ei == NLMISC::CEntityId::Unknown) return false;
	return isInIgnoreList(ei);
}

//-----------------------------------------------
//	setIgnoreList
//
//-----------------------------------------------
void CChatClient::setIgnoreList(const std::vector<NLMISC::CEntityId> &ignoreList)
{
	TIgnoreListCont ignoreListCont(ignoreList.begin(), ignoreList.end());
	_IgnoreList.swap(ignoreListCont);
}

//-----------------------------------------------
//	filter :
//
//-----------------------------------------------
void CChatClient::filter( TFilter filterId )
{
	set<TFilter>::iterator itF = _Filters.find( filterId );
	if( itF != _Filters.end() )
	{
		_Filters.insert( filterId );
	}
	else
	{
		_Filters.erase( itF );
	}

} // addGameplayFilter //



//-----------------------------------------------
//	getAudience :
//
//-----------------------------------------------
CChatGroup& CChatClient::getAudience()
{ 
	CChatManager &cm = IOS->getChatManager();
	if(_ChatMode == CChatGroup::shout) 
		return cm.getGroup(_ShoutAudienceId); 
	else 
		return cm.getGroup(_SayAudienceId);

} // getAudience //



//-----------------------------------------------
//	getSayAudience :
//
//-----------------------------------------------
CChatGroup& CChatClient::getSayAudience( bool update )
{ 
	CChatManager &cm = IOS->getChatManager();
	if( update )
	{
		updateAudience( _SayAudienceId, MaxDistSay, _SayLastAudienceUpdateTime );
	}
	return cm.getGroup(_SayAudienceId);

} // getSayAudience //



//-----------------------------------------------
//	getShoutAudience :
//
//-----------------------------------------------
CChatGroup& CChatClient::getShoutAudience( bool update )
{ 
	CChatManager &cm = IOS->getChatManager();
	if( update )
	{
		updateAudience( _ShoutAudienceId, MaxDistShout, _ShoutLastAudienceUpdateTime );
	}
	return cm.getGroup(_ShoutAudienceId);

} // getShoutAudience //



//-----------------------------------------------
//	updateAudience :
//
//-----------------------------------------------
void CChatClient::updateAudience()
{
	switch( _ChatMode )
	{
	case CChatGroup::say :
		{
			updateAudience( _SayAudienceId, MaxDistSay, _SayLastAudienceUpdateTime );
		}
		break;
	case CChatGroup::shout :
		{
			updateAudience( _ShoutAudienceId, MaxDistShout, _ShoutLastAudienceUpdateTime );
		}
		break;
	default:
		{
			return;
		}
	}

} // updateAudience //



//-----------------------------------------------
//	updateAudience :
//
//-----------------------------------------------
void CChatClient::updateAudience( CEntityId &audienceId, sint maxDist, TTime& lastAudienceUpdateTime )
{
	// get the cell and coordinates of the player
	CMirrorPropValueRO<uint32> instanceId( TheDataset, _DataSetIndex, DSPropertyAI_INSTANCE );
	CMirrorPropValueRO<uint32> cell( TheDataset, _DataSetIndex, DSPropertyCELL );
	sint32 cellS = (sint32)(cell());

	CChatManager &cm = IOS->getChatManager();
	CChatGroup &audience = cm.getGroup(audienceId);

	// test if player is in an 'elevator' cell
	if( cellS == -1 )
	{
		while (!audience.Members.empty())
			cm.removeFromGroup(audienceId, *audience.Members.begin());
	}
	else
	{
		// if time has come to update player's audience
		TTime currentTime = CTime::getLocalTime();
		if( currentTime - lastAudienceUpdateTime > _AudienceUpdatePeriod )
		{
			// Browse all the entities!
			TEntityIdToEntityIndexMap::const_iterator itEntityIndex;
			for( itEntityIndex = TheDataset.entityBegin(); itEntityIndex != TheDataset.entityEnd(); ++itEntityIndex )
			{
				TDataSetRow entityIndex = TheDataset.getCurrentDataSetRow( GET_ENTITY_INDEX(itEntityIndex) );
				const CEntityId& entityId = (*itEntityIndex).first;
				if (entityIndex.isValid())
				{
					if ( entityId.getType() == RYZOMID::player )
					{
						bool isInAudience = false;

						CMirrorPropValueRO<uint32> charInstanceId( TheDataset, entityIndex, DSPropertyAI_INSTANCE);

						// first of all, check the instance id
						if (instanceId == charInstanceId)
						{
							CMirrorPropValueRO<sint32> charCell( TheDataset, entityIndex, DSPropertyCELL ); 
								
							// if the player is NOT in an appartment
							if( cellS > 0 )
							{
								uint16 senderCellX = (uint16)(cell>>16);
								uint16 senderCellY = (uint16)(cell&0xffff);

								uint16 charCellX = (uint16)(charCell>>16);
								uint16 charCellY = (uint16)(charCell&0xffff);
								
								sint32 distX = (sint32)(charCellX) - (sint32)(senderCellX);
								sint32 distY = (sint32)(charCellY) - (sint32)(senderCellY);

	//							nldebug("<CChatClient::updateAudience> dist : x=%d y=%d ",distX,distY);

								// if the player is close enough
								if( abs(distX)<=maxDist && abs(distY)<=maxDist )
								{
									isInAudience = true;
								}
							}
							// if the player is in an apartment
							else
							{
								// if both players are in the same apartment
								if( charCell == cellS )
								{
									isInAudience = true;
								}
							}
						}

						CChatClient &cc = cm.getClient(entityIndex);
						// add or remove char from player's audience
						if( isInAudience )
						{
							// we add him in the audience
							CChatGroup::TMemberCont::iterator itA = audience.Members.find( entityIndex );
							if( itA == audience.Members.end() )
							{
								cm.addToGroup(audienceId, entityIndex);
/*								cc.subscribeInChatGroup(groupId);
								audience.Members.insert( entityIndex );
								if (VerboseChat)
									nldebug("CChatClient::updateAudience : adding player %x in the audience of %x",
										entityIndex.getIndex(),
										_DataSetIndex.getIndex());
*/							}
						}
						else
						{
							// we remove him from the audience(if necessary)
							CChatGroup::TMemberCont::iterator itA = audience.Members.find( entityIndex );
							if( itA != audience.Members.end() )
							{
								cm.removeFromGroup(audienceId, entityIndex);
/*								audience.Members.erase( itA );
								if (VerboseChat)
									nldebug("CChatClient::updateAudience : removing player %x from the audience of %x",
									entityIndex.getIndex(),
									_DataSetIndex.getIndex());
*/							}
						}
					}
				}
			}
		}
		lastAudienceUpdateTime = currentTime;
	}
	
} // updateAudience //











//-----------------------------------------------
//	knowString :
//
//-----------------------------------------------
bool CChatClient::knowString( uint32 index )
{
	if( index >= _KnownStrings.size() )
	{
		_KnownStrings.resize( index+1, false );
	}

	if( _KnownStrings[index] == false )
	{
		_KnownStrings[index] = true;
		return false;
	}
	else
	{
		return true;
	}
} // knowString //



void CChatClient::subscribeInChatGroup(TGroupId groupId)
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCC: subscribeInChatGroup : client %s:%x subscribe in chat group %s",
			TheDataset.getEntityId(_DataSetIndex).toString().c_str(),
			_DataSetIndex.getIndex(),
			groupId.toString().c_str());
	}

	TSubscribedGroupCont::iterator it(_SubscribedGroups.find(groupId));
	if (it == _SubscribedGroups.end())
	{
		_SubscribedGroups.insert(groupId);
	}
	else
	{
		nlwarning("CChatClient::subscribeInChatGroup : error : client %s:%x has alrady subscribed to group %s",
			TheDataset.getEntityId(_DataSetIndex).toString().c_str(),
			_DataSetIndex.getIndex(),
			groupId.toString().c_str());
	}
}

void CChatClient::unsubscribeInChatGroup(TGroupId groupId)
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCC: unsubscribeInChatGroup : client %s:%x unsubscribe from chat group %s",
			TheDataset.getEntityId(_DataSetIndex).toString().c_str(),
			_DataSetIndex.getIndex(),
			groupId.toString().c_str());
	}

	TSubscribedGroupCont::iterator it(_SubscribedGroups.find(groupId));
	if (it != _SubscribedGroups.end())
	{
		_SubscribedGroups.erase(it);
	}
	else
	{
		nlwarning("CChatClient::unsubscribeInChatGroup : error : client %s:%x has not subscribed to group %s",
			TheDataset.getEntityId(_DataSetIndex).toString().c_str(),
			_DataSetIndex.getIndex(),
			groupId.toString().c_str());
	}
}

void CChatClient::unsubscribeAllChatGroup()
{
	CChatManager &cm = IOS->getChatManager();
	while (!_SubscribedGroups.empty())
	{
		TGroupId gid = *(_SubscribedGroups.begin());
		cm.removeFromGroup(gid, _DataSetIndex);

		if (_SubscribedGroups.find(gid) != _SubscribedGroups.end())
		{
			nlwarning("CChatClient::unsubscribeAllChatGroup : force unsubscrib from group %s", gid.toString().c_str());
			_SubscribedGroups.erase(gid);
		}
	}
}

