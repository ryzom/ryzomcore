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

#include "patat_subscribe_manager.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/synchronised_message.h"

#include "nel/misc/command.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_collision_desc.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLLIGO;
using namespace NLPACS;

/*
 * Constructor
 */
CPatatSubscribeManager::CPatatSubscribeManager()
{
}


/*
 * Destructor
 */
CPatatSubscribeManager::~CPatatSubscribeManager()
{
}


/*
 * Init the subscriber
 */
void	CPatatSubscribeManager::init()
{
	_PatatGrid.init();
	_PatatMap.clear();
	_SubscriberMap.clear();
	_TriggerMap.clear();
	_ModifiedPatats.clear();
}

/*
 * Serialize a manager file (no subscription saved, only triggers/patats saved)
 */
void	CPatatSubscribeManager::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);

	if (f.isReading())
		init();

	f.serial(_PatatGrid);
	f.serialCont(_PatatMap);
	f.serialCont(_TriggerMap);
}


/*
 * Use a prim file
 */
void	CPatatSubscribeManager::usePrim(const string &primFile)
{
	vector<uint32>	prims;
	_PatatGrid.usePrim(primFile, prims);

	uint	i;
	for (i=0; i<prims.size(); ++i)
	{
		// checks if patat exists in subscribed patats (creates and inits if not)
		TPatatMap::iterator	itp = _PatatMap.find(prims[i]);
		if (itp == _PatatMap.end())
		{
			pair<TPatatMap::iterator, bool>	res = _PatatMap.insert(TPatatMap::value_type(prims[i], CPatat()));
			itp = res.first;
			(*itp).second.Name = _PatatGrid.getZoneName(prims[i]);
			(*itp).second.InternalPatatId = prims[i];

			_TriggerMap.insert(TTriggerIdMap::value_type((*itp).second.Name, (*itp).second.InternalPatatId));
		}
	}
}

/*
 * Register a pacs trigger id
 */
void	CPatatSubscribeManager::usePacsTrigger(sint32 id, const std::string &name)
{
	// checks if patat exists in subscribed patats (creates and inits if not)
	TPatatMap::iterator	itp = _PatatMap.find(id);
	if (itp == _PatatMap.end())
	{
		pair<TPatatMap::iterator, bool>	res = _PatatMap.insert(TPatatMap::value_type(id, CPatat()));
		itp = res.first;
		(*itp).second.Name = name;
		(*itp).second.InternalPatatId = id;

		_TriggerMap.insert(TTriggerIdMap::value_type((*itp).second.Name, (*itp).second.InternalPatatId));
	}
}



//

/*
 * Subscribe to a patat
 */
void	CPatatSubscribeManager::subscribe(NLNET::TServiceId service, const TPatatSubscription &patat)
{
	nldebug("Subscribe service %d to patat %s (#id %d)", service.get(), patat.first.c_str(), patat.second);

	// checks the patat is referenced in patatgrid/pacs triggers
	TTriggerIdMap::iterator	it = _TriggerMap.find(patat.first);

	if (it == _TriggerMap.end())
	{
		nlwarning("Can't subscribe service %d to patat %s, not referenced in PatatGrid", service.get(), patat.first.c_str());
		return;
	}

	sint32	patatId = (*it).second;

	// checks if patat exists in subscribed patats (creates and inits if not)
	TPatatMap::iterator	itp = _PatatMap.find(patatId);
	if (itp == _PatatMap.end())
	{
		pair<TPatatMap::iterator, bool>	res = _PatatMap.insert(TPatatMap::value_type(patatId, CPatat()));
		itp = res.first;

		(*itp).second.Name = patat.first;
		(*itp).second.InternalPatatId = patatId;
	}

	// checks if subscriber exists in subscribers (creates and inits if not)
	TSubscriberMap::iterator	its = _SubscriberMap.find(service);
	if (its == _SubscriberMap.end())
	{
		pair<TSubscriberMap::iterator, bool>	res = _SubscriberMap.insert(TSubscriberMap::value_type(service, CSubscriber()));
		its = res.first;

		(*its).second.Service = service;
	}



	// checks if service not yet in patat's subscribers
	uint	i;
	for (i=0; i<(*itp).second.Subscribers.size(); ++i)
		if ((*itp).second.Subscribers[i].Service == service)
			break;

	if (i == (*itp).second.Subscribers.size())
		(*itp).second.Subscribers.resize(i+1);

	(*itp).second.Subscribers[i].Service = service;
	(*itp).second.Subscribers[i].SubscriberIterator = its;
	(*itp).second.Subscribers[i].PatatId = patat.second;

	// checks if patat not yet in service's subscribed patats
	for (i=0; i<(*its).second.Patats.size(); ++i)
		if ((*its).second.Patats[i].InternalPatatId == patatId)
			break;

	if (i == (*its).second.Patats.size())
		(*its).second.Patats.resize(i+1);

	(*its).second.Patats[i].InternalPatatId = patatId;
	(*its).second.Patats[i].PatatId = patat.second;
	(*its).second.Patats[i].PatatIterator = itp;

	//
	if (!(*itp).second.StillIns.empty())
	{
		CMessage	msg("TRIGGER_IN");
		msg.serial(const_cast<uint16&>(patat.second));
		msg.serialCont((*itp).second.StillIns);
	}
}

/*
 * Unsubscribe to a patat
 */
void	CPatatSubscribeManager::unsubscribe(NLNET::TServiceId service, TPatatId patat)
{
	nldebug("Unsubscribe service %d to patat #id %d", service.get(), patat);
	// checks the patat is referenced in patatgrid

	sint32	patatId = 0;

	// find patat and remove it
	TSubscriberMap::iterator	its = _SubscriberMap.find(service);
	TPatatMap::iterator			itp = _PatatMap.end();
	if (its != _SubscriberMap.end())
	{
		vector<CSubscribedPatat>::iterator	itsp;
		for (itsp=(*its).second.Patats.begin(); itsp!=(*its).second.Patats.end(); ++itp)
		{
			if ((*itsp).PatatId == patat)
			{
				patatId = (*itsp).InternalPatatId;
				itp = (*itsp).PatatIterator;
				(*its).second.Patats.erase(itsp);
				break;
			}
		}
	}

	// find subscriber and remove it
	if (itp != _PatatMap.end())
	{
		vector<CPatatSubscriber>::iterator	its;
		for (its=(*itp).second.Subscribers.begin(); its!=(*itp).second.Subscribers.end(); ++its)
		{
			if ((*its).Service == service)
			{
				(*itp).second.Subscribers.erase(its);
				break;
			}
		}
	}
}

/*
 * Unsubscribe a whole service
 */
void	CPatatSubscribeManager::unsubscribe(NLNET::TServiceId service)
{
	nldebug("Unsubscribe service %d to all subscribed patats", service.get());

	// find patat and remove it
	TSubscriberMap::iterator	its = _SubscriberMap.find(service);
	if (its != _SubscriberMap.end())
	{
		// for all subscribed patats
		vector<CSubscribedPatat>::iterator	itp;
		for (itp=(*its).second.Patats.begin(); itp!=(*its).second.Patats.end(); ++itp)
		{
			// remove in patats subscriber list all reference to subcriber
			vector<CPatatSubscriber>			&subscribers = (*((*itp).PatatIterator)).second.Subscribers;
			vector<CPatatSubscriber>::iterator	it;
			for (it=subscribers.begin(); it!=subscribers.end(); )
				if ((*it).Service == service)
					it = subscribers.erase(it);
				else
					++it;
		}
	}
}



/*
 * Get the new entry index for an entity, given its position
 */
uint32	CPatatSubscribeManager::getNewEntryIndex(const CEntityId &id, const CVector &pos, uint32 previousEntryIndex)
{
	// get the entry index for the position
	sint32	newEntryIndex = _PatatGrid.getEntryIndex(pos);

	setNewEntryIndex(id, newEntryIndex, previousEntryIndex);

	return newEntryIndex;
}

/*
 * set the new entry index for an entity
 */
void	CPatatSubscribeManager::setNewEntryIndex(const CEntityId &id, uint32 newEntryIndex, uint32 previousEntryIndex)
{
	vector<uint32>	in, out;

	// compute the patatId differences between previous and new entry indexes
	if (!_PatatGrid.diff((CPatatGrid::TEntryIndex)previousEntryIndex, (CPatatGrid::TEntryIndex)newEntryIndex, in, out))
		return;

	uint	i;
	// for each patat left, notify patat the entity left it
	for (i=0; i<out.size(); ++i)
	{
		nldebug("Notfied %s left patat %s", id.toString().c_str(), _PatatGrid.getZoneName(out[i]).c_str());

		// find the left patat
		TPatatMap::iterator	it = _PatatMap.find((sint32)out[i]);
		if (it == _PatatMap.end())
			continue;	// not referenced

		// adds the entity to the outs list
		(*it).second.Outs.push_back(id);
		(*it).second.StillIns.erase(id);

		// notify it as modified
		if (!(*it).second.Modified)
		{
			(*it).second.Modified = true;
			_ModifiedPatats.push_back(it);
		}
	}

	// same job, but for entered patats
	for (i=0; i<in.size(); ++i)
	{
		nldebug("Notfied %s entered patat %s", id.toString().c_str(), _PatatGrid.getZoneName(in[i]).c_str());

		TPatatMap::iterator	it = _PatatMap.find((sint32)in[i]);
		if (it == _PatatMap.end())
			continue;

		(*it).second.Ins.push_back(id);
		(*it).second.StillIns.insert(id);

		if (!(*it).second.Modified)
		{
			(*it).second.Modified = true;
			_ModifiedPatats.push_back(it);
		}
	}
}

/*
 * Process pacs trigger collisions
 */
void	CPatatSubscribeManager::processPacsTriggers(UMoveContainer *moveContainer)
{
	uint	num = moveContainer->getNumTriggerInfo();
	uint	i;

	for (i=0; i<num; ++i)
	{
		const UTriggerInfo	&info = moveContainer->getTriggerInfo(i);

		CEntityId		trigger = CEntityId(info.Object0);
		CEntityId		entity = CEntityId(info.Object1);

		if (trigger.getType() != RYZOMID::trigger)
			swap(trigger, entity);

		// don't warn of non trigger collision
		if (trigger.getType() != RYZOMID::trigger)
			continue;

		sint32			triggerId = (sint32)trigger.getShortId();

		TPatatMap::iterator	it = _PatatMap.find(triggerId);
		if (it == _PatatMap.end())
			continue;

		switch (info.CollisionType)
		{
		case UTriggerInfo::In:
			{
				nldebug("Notified %s entered trigger %s", entity.toString().c_str(), (*it).second.Name.c_str());
				// adds the entity to the ins list
				(*it).second.Ins.push_back(entity);
				// don't insert in still list, cause when primitive is deleted, no trigger out event occurs
				// and that might fill still list quickly !
				//(*it).second.StillIns.insert(entity);

				// adds to the modified list
				if (!(*it).second.Modified)
				{
					(*it).second.Modified = true;
					_ModifiedPatats.push_back(it);
				}
			}
			break;
		case UTriggerInfo::Out:
			{
				nldebug("Notified %s entered trigger %s", entity.toString().c_str(), (*it).second.Name.c_str());
				// adds the entity to the outs list
				(*it).second.Outs.push_back(entity);
				//(*it).second.StillIns.erase(entity);

				// notify it as modified
				if (!(*it).second.Modified)
				{
					(*it).second.Modified = true;
					_ModifiedPatats.push_back(it);
				}
			}
			break;
		case UTriggerInfo::Inside:
			break;
		}
	}
}


/*
 * Emit changes
 */
void	CPatatSubscribeManager::emitChanges()
{
	// initialize subscribers messages
	// for each subscriber, reset in and out messages
	TSubscriberMap::iterator	its;
	for (its=_SubscriberMap.begin(); its!=_SubscriberMap.end(); ++its)
	{
		// reset out message
		CMessage	&msgouts = (*its).second.OutsMessage;
		msgouts.clear();
		/*if (msgouts.isReading())
			msgouts.invert();*/
		msgouts.setType("TRIGGER_OUT");
		(*its).second.OutsMsgSize = msgouts.length();

		// reset in message
		CMessage	&msgins = (*its).second.InsMessage;
		msgins.clear();
		/*if (msgins.isReading())
			msgins.invert();*/
		msgins.setType("TRIGGER_IN");
		(*its).second.InsMsgSize = msgins.length();
	}

	// for each modified patat, add the patatId and the list of entities that entered/left the patat
	TModifiedPatats::iterator	itm;
	for (itm=_ModifiedPatats.begin(); itm!=_ModifiedPatats.end(); ++itm)
	{
		CPatat	&patat = (*(*itm)).second;
		uint	i;

		// if entities left the patat
		// notify all subscribers -- serializes in OutsMessage patatId and vector of entity ids
		if (!patat.Outs.empty())
		{
			for (i=0; i<patat.Subscribers.size(); ++i)
			{
				its = patat.Subscribers[i].SubscriberIterator;
				// serialize patatId for this subscriber (different from internal patatid)
				(*its).second.OutsMessage.serial(patat.Subscribers[i].PatatId);
				// serialize vector of CEntityId
				(*its).second.OutsMessage.serialCont(patat.Outs);
			}
		}

		// if entities entered the patat
		// notify all subscribers -- serializes in InsMessage patatId and vector of entity ids
		if (!patat.Ins.empty())
		{
			for (i=0; i<patat.Subscribers.size(); ++i)
			{
				its = patat.Subscribers[i].SubscriberIterator;
				// serialize patatId for this subscriber (different from internal patatid)
				(*its).second.InsMessage.serial(patat.Subscribers[i].PatatId);
				// serialize vector of CEntityId
				(*its).second.InsMessage.serialCont(patat.Ins);
			}
		}

		// reset modify state
		patat.Modified = false;
		// and clear temp in/out vectors
		patat.Ins.clear();
		patat.Outs.clear();
	}

	// reset the list of modified patat
	_ModifiedPatats.clear();

	// and send messages to subscribers (if needed)
	for (its=_SubscriberMap.begin(); its!=_SubscriberMap.end(); ++its)
	{
		if ((*its).second.InsMessage.length() > (*its).second.InsMsgSize)
			sendMessageViaMirror((*its).second.Service, (*its).second.InsMessage);

		if ((*its).second.OutsMessage.length() > (*its).second.OutsMsgSize)
			sendMessageViaMirror((*its).second.Service, (*its).second.OutsMessage);
	}
}


/*
 * Display triggers
 */
void	CPatatSubscribeManager::displayTriggers(NLMISC::CLog *log)
{
	log->displayNL("Registered triggers:");

	TPatatMap::iterator	it;
	for (it=_PatatMap.begin(); it!=_PatatMap.end(); ++it)
	{
		CPatat	&patat = (*it).second;
		log->displayNL(" - '%s', internal #%d, %d subscribers, %d entities in patat [0 for PACS triggers]", patat.Name.c_str(), patat.InternalPatatId, patat.Subscribers.size(), patat.StillIns.size());
	}

	log->displayNL("End of registered triggers");
}

/*
 * Display info for trigger
 */
void	CPatatSubscribeManager::displayTriggerInfo(const string &name, NLMISC::CLog *log)
{
	TTriggerIdMap::iterator	it = _TriggerMap.find(name);
	if (it == _TriggerMap.end())
	{
		log->displayNL("No trigger '%s' registered", name.c_str());
		return;
	}

	sint32	id = (*it).second;

	log->displayNL("Trigger info: '%s', internal #%d", name.c_str(), id);

	TPatatMap::iterator		itp = _PatatMap.find(id);
	if (itp == _PatatMap.end())
	{
		log->displayNL("Trigger not referenced in patat map, data not consistent !");
		log->displayNL("End of trigger info");
		return;
	}

	CPatat	&patat = (*itp).second;

	uint	i;

	log->displayNL("Subscribers:");
	for (i=0; i<patat.Subscribers.size(); ++i)
		log->displayNL(" - %d, userId #d", patat.Subscribers[i].Service.get(), patat.Subscribers[i].PatatId);

	log->displayNL("Ins:");
	for (i=0; i<patat.Ins.size(); ++i)
		log->displayNL(" - %s", patat.Ins[i].toString().c_str());

	log->displayNL("Outs:");
	for (i=0; i<patat.Ins.size(); ++i)
		log->displayNL(" - %s", patat.Outs[i].toString().c_str());

	log->displayNL("Outs:");
	for (i=0; i<patat.Ins.size(); ++i)
		log->displayNL(" - %s", patat.Outs[i].toString().c_str());

	log->displayNL("StillIns:");
	set<NLMISC::CEntityId>::iterator	its;
	for (its=patat.StillIns.begin(); its!=patat.StillIns.end(); ++its)
		log->displayNL(" - %s", (*its).toString().c_str());

	log->displayNL("End of trigger info");
}

/*
 * Display subscribers
 */
void	CPatatSubscribeManager::displaySubscribers(NLMISC::CLog *log)
{
	log->displayNL("Registered subscribers:");

	TSubscriberMap::iterator	it;
	for (it=_SubscriberMap.begin(); it!=_SubscriberMap.end(); ++it)
	{
		CSubscriber	&subscriber = (*it).second;
		log->displayNL(" - %d, %d subscribed triggers", subscriber.Service.get(), subscriber.Patats.size());
	}

	log->displayNL("End of registered subscribers");
}

/*
 * Display info for subscriber
 */
void	CPatatSubscribeManager::displaySubscriberInfo(NLNET::TServiceId service, NLMISC::CLog *log)
{
	TSubscriberMap::iterator	it = _SubscriberMap.find(service);
	if (it == _SubscriberMap.end())
	{
		log->displayNL("Service %d no registered", service.get());
		return;
	}

	log->displayNL("Subscriber info: %d", service.get());

	CSubscriber	&subscriber = (*it).second;

	uint	i;

	log->displayNL("Subscribed triggers:");
	for (i=0; i<subscriber.Patats.size(); ++i)
		log->displayNL(" - internal #%d, subscribed as userId #d", subscriber.Patats[i].InternalPatatId, subscriber.Patats[i].PatatId);

	log->displayNL("End of subscriber info");
}



