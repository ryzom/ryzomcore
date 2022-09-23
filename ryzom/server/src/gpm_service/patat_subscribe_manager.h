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



#ifndef NL_PATAT_SUBSCRIBE_MANAGER_H
#define NL_PATAT_SUBSCRIBE_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/vector.h"
#include "nel/misc/stream.h"

#include "patat_grid.h"
#include "nel/net/unified_network.h"

#include <map>
#include <set>
#include <vector>
#include <string>

namespace NLPACS
{
	class UMoveContainer;
};

/**
 * Patat event subscription management.
 * Allows to be notified when an entity enters or leaves a patat.
 *
 * To use it, proceed this way :
 * 1. \c init() with world bounds (in meters)
 * 2. Load all needed patats with \c usePrim(<file>) where <file> points to a .prim file.
 * 3. \c build() the manager.
 *
 * Once this is done, you can't add more patats (won't be inserted anyway.)
 * Subscription is done by calling subscribe(), and removed by unsubscribe().
 * Call getNewEntryIndex() to update the patats events, and call emitChanges() at each update.
 * 
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CPatatSubscribeManager
{
public:
	typedef uint16	TPatatId;

	typedef std::pair<std::string, TPatatId>	TPatatSubscription;

protected:

	class CPatat;
	class CSubscriber;

	/// A Map of subscribed patats
	typedef std::map<sint32, CPatat>		TPatatMap;

	/// A Map of subscribers
	typedef std::map<NLNET::TServiceId, CSubscriber>	TSubscriberMap;

	/// A Map of id
	typedef std::map<std::string, sint32>	TTriggerIdMap;

	/// A patat subscriber
	class CPatatSubscriber
	{
	public:
		NLNET::TServiceId				Service;
		TSubscriberMap::iterator		SubscriberIterator;
		TPatatId						PatatId;
	};

	/// A patat
	class CPatat
	{
	public:
		CPatat() : Modified(false) {}
		std::string						Name;
		sint32							InternalPatatId;
		std::vector<CPatatSubscriber>	Subscribers;
		std::vector<NLMISC::CEntityId>	Ins;
		std::vector<NLMISC::CEntityId>	Outs;
		std::set<NLMISC::CEntityId>		StillIns;
		bool							Modified;

		void	serial(NLMISC::IStream &f)
		{
			f.serial(Name);
			f.serial(InternalPatatId);
		}
	};

	/// A list of modified patats
	typedef std::vector<TPatatMap::iterator>	TModifiedPatats;

	/// A subscribed patat
	class CSubscribedPatat
	{
	public:
		sint32							InternalPatatId;
		TPatatId						PatatId;
		TPatatMap::iterator				PatatIterator;
	};

	/// A subscriber
	class CSubscriber
	{
	public:
		CSubscriber() : OutsMessage("",false), InsMessage("",false) {} // always output messages

		NLNET::TServiceId				Service;
		NLNET::CMessage					OutsMessage;
		uint32							OutsMsgSize;
		NLNET::CMessage					InsMessage;
		uint32							InsMsgSize;
		std::vector<CSubscribedPatat>	Patats;
	};

	/// Patat grid
	CPatatGrid		_PatatGrid;

	/// Subscribed patats
	TPatatMap		_PatatMap;

	/// Subscribers
	TSubscriberMap	_SubscriberMap;

	/// Triggers map
	TTriggerIdMap	_TriggerMap;

	/// Modified patats
	TModifiedPatats	_ModifiedPatats;

public:

	/// Constructor
	CPatatSubscribeManager();

	/// Destructor
	~CPatatSubscribeManager();

	/// Init
	void	init();

	/// Serial
	void	serial(NLMISC::IStream &f);

	/// Use a prim file, patats in file will be processed at build() time.
	void	usePrim(const std::string &primFile);

	/// Register a pacs trigger id
	void	usePacsTrigger(sint32 id, const std::string &name);

	/// Checks if patat exists
	bool	exist(const std::string &name) const { return _PatatGrid.exist(name); }

	/// Subscribe to a patat
	void	subscribe(NLNET::TServiceId service, const TPatatSubscription &patat);

	/// Unsubscribe
	void	unsubscribe(NLNET::TServiceId service, TPatatId patat);

	/// Unsubscribe for a whole service
	void	unsubscribe(NLNET::TServiceId service);


	/// Get entry index at pos
	uint32	getEntryIndex(const NLMISC::CVector &pos)	{ return _PatatGrid.getEntryIndex(pos); }

	/// Move entity and get its new patats entry index
	uint32	getNewEntryIndex(const NLMISC::CEntityId &id, const NLMISC::CVector &pos, uint32 previousEntryIndex);

	/// Set entry index
	void	setNewEntryIndex(const NLMISC::CEntityId &id, uint32 newEntryIndex, uint32 previousEntryIndex);

	/// Process pacs trigger collisions
	void	processPacsTriggers(NLPACS::UMoveContainer *moveContainer);

	/// Emit changes
	void	emitChanges();


	/// Display info for trigger
	void	displayTriggers(NLMISC::CLog *log = NLMISC::InfoLog);

	/// Display info for trigger
	void	displayTriggerInfo(const std::string &name, NLMISC::CLog *log = NLMISC::InfoLog);

	/// Display info for trigger
	void	displaySubscribers(NLMISC::CLog *log = NLMISC::InfoLog);

	/// Display info for trigger
	void	displaySubscriberInfo(NLNET::TServiceId service, NLMISC::CLog *log = NLMISC::InfoLog);

	/// Display patat grid info
	void	displayPatatGridInfo(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		_PatatGrid.displayInfo(log);
	}


	/// Add CPrimZone class filter
	void	addPrimZoneFilter(const std::string &filter)	{ _PatatGrid.addPrimZoneFilter(filter); }

	/// Remove CPrimZone class filter
	void	removePrimZoneFilter(const std::string &filter)	{ _PatatGrid.removePrimZoneFilter(filter); }

	/// Reset CPrimZone class filter
	void	resetPrimZoneFilter()							{ _PatatGrid.resetPrimZoneFilter(); }
};


#endif // NL_PATAT_SUBSCRIBE_MANAGER_H

/* End of patat_subscribe_manager.h */
