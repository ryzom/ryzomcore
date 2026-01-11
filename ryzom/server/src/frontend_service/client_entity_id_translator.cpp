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

#include <nel/misc/debug.h>

#include "client_entity_id_translator.h"

using namespace std;
using namespace CLFECOMMON;

//

/*inline TCLEntityId	CClientEntityIdTranslator::CCEIdBackMap::allocCEId(const TEntityIndex& id)
{
	TCLEntityId	newId;

	if (_Free.empty())
	{
		// if no more free item, create one in the table
		newId = _Table.size();

		// if no more item in the table, return fail
		if (newId > LastUsedCEId)
			return InvalidCEId;

		_Table.push_back();
	}
	else
	{
		// else pop an element of the free queue
		// use queue tail so the newly allocated item is the oldest in the free list
		// to avoid confusion on the client
		newId = _Free.back();
		_Free.pop_back();
	}

	// add it to the used queue
	_Used.push_front(newId);
	_Table[newId].clear(id);
	_Table[newId].Used = true;
	return newId;
}*/

/*inline void	CClientEntityIdTranslator::CCEIdBackMap::freeCEId(TCLEntityId id)
{
	// remove the id of the used queue
	nlassertex(_Table[id].Used, ("id=%d", id));
	TCEIdQueue::iterator	it = find(_Used.begin(), _Used.end(), id);
	nlassertex(it != _Used.end(), ("id=%d", id));
	_Used.erase(it);
	// and add it to the free queue
	_Free.push_front(id);
	// reset the feid
	_Table[id].clear(INVALID_ENTITY_INDEX);
}*/



/*
 * Constructor
 */
CClientEntityIdTranslator::CClientEntityIdTranslator()
{
}


//

/*TCLEntityId	CClientEntityIdTranslator::getNewCEId(const TEntityIndex& id)
{
	// check we have enough space to allocate an id
	nlassertex(_ClientEntityIdBackMap.usedId() <= LastUsedCEId-FirstUsedCEId+1, ("_ClientEntityIdBackMap.usedId()=%d LastUsedCEId-FirstUsedCEId+1=%d _Id=%d EntityIndex=%d", _ClientEntityIdBackMap.usedId(), LastUsedCEId-FirstUsedCEId+1, _Id, id));
	// check the feid doesn't exist yet
	nlassertex(_ClientEntityIdMap.find(id) == _ClientEntityIdMap.end(), ("_Id=%d id=%d", _Id, id));

	// alloc a client entity id
	TCLEntityId id = _ClientEntityIdBackMap.allocCEId(id);
	if ( id != InvalidCEId )
	{
		// and create association to the feid
		_ClientEntityIdMap.insert(make_pair(id, id));
//		nldebug("FEIDT: Client[%d] Set Id association %s->%d", _Id, id.getString().c_str(), id);
	}
	return id;
}*/

/*void	CClientEntityIdTranslator::releaseId(TCLEntityId id)
{
	TEntityIndex	id = getEntityId(id);

	if (id == INVALID_ENTITY_INDEX)
	{
		nlwarning("FEIDT: Client[%d] Remove not existing association (ceId=%d). Associations left unchanged", _Id, id);
		return;
	}

	nldebug("IdT[%d] Remove Id association %d->%d", _Id, id, id);

	_ClientEntityIdBackMap.freeCEId(id);
	_ClientEntityIdMap.erase(id);
}*/

void	CClientEntityIdTranslator::releaseId(const TEntityIndex& id)
{
	TCEIdMap::iterator	it = _ClientEntityIdMap.find(id);
	
	if (it == _ClientEntityIdMap.end())
	{
		nlwarning("FEIDT: Client[%d] Remove not existing association (entityIndex=%u). Associations left unchanged", _Id, id.getIndex());
		return;
	}

	//nldebug("FEIDT: Client[%d] Remove Id association %d->%d", _Id, id, (*it).second);

	_ClientEntityIdBackMap.freeCEId((*it).second);
	_ClientEntityIdMap.erase(it);
}


/*
// INLINED
bool	CClientEntityIdTranslator::isUsed(const TEntityIndex& id) const
{
	return _ClientEntityIdMap.find(id) != _ClientEntityIdMap.end();
}

bool	CClientEntityIdTranslator::isUsed(TCLEntityId id) const
{
	return _ClientEntityIdBackMap.isUsed(id);
}
*/


/*TCLEntityId	CClientEntityIdTranslator::operator [] (TEntityIndex id)
{
	TCEIdMap::iterator	it = _ClientEntityIdMap.find(id);
	return (it != _ClientEntityIdMap.end()) ? (*it).second : getNewCEId(id);
}*/
