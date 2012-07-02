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

#include "stdpch.h"

#include "game_share/small_string_manager.h"

using namespace R2;

uint32 CSmallStringManager::registerString(const std::string& phrase,uint32 nb)
{
	std::map<std::string, uint32>::iterator found(_StringToId.find(phrase));
	//new text entered
	uint32 id;
	if (found == _StringToId.end())
	{
		id = getNewId();
		_StringToId.insert(std::pair<std::string, uint32>(phrase, id));
		_IdToString.insert( std::pair<uint32, std::pair<std::string, uint32> >(id, std::pair<std::string, uint32>(phrase, nb) ) );
	}
	else //already registered text, increment counter
	{
		id = found->second;
		std::map<uint32, std::pair<std::string, uint32> >::iterator  found2	(_IdToString.find(found->second));
		found2->second.second += nb;
	}
	//return the id of the text
	return id;
}


void CSmallStringManager::unregisterString(const std::string& phrase,uint32 nb)
{
	std::map<std::string, uint32>::iterator  found(_StringToId.find(phrase));
	nlassert(found != _StringToId.end());

	uint32 id = found->second;
	unregisterStringById(id, nb);

}

void CSmallStringManager::unregisterStringById(uint32 id, uint32 nb)
{
	std::map<uint32, std::pair<std::string, uint32> >::iterator  found2(_IdToString.find(id));
	if (found2 == _IdToString.end())
	{
		nlwarning("try to unload a unregistered table");
		return;
	}
	uint32& count = found2->second.second;
	if (count < nb) { count = 0; }
	count -= nb;
	if (count == 0)
	{
		std::map<std::string, uint32>::iterator found = _StringToId.find(found2->second.first);
		if (found != _StringToId.end()) { _StringToId.erase(found); }

		_FreeIds.insert(found2->first);
		_IdToString.erase(found2);
	}

}

uint32 CSmallStringManager::getNewId()
{
	//TMP
	if (!_FreeIds.empty())
	{
		std::set< uint32 >::iterator it = _FreeIds.begin();
		uint32 id = *it;
		_FreeIds.erase( it );
		return id;
	}
	_MaxId = _MaxId +1;
	return _MaxId-1;
}

const std::string& CSmallStringManager::getString(uint32 id) const
{
	static const std::string empty("Wrong id");
	std::map<uint32,std::pair<std::string,uint32> >::const_iterator  found(_IdToString.find(id));
	if (found == _IdToString.end()){ nlwarning("Try to access un accessible string"); return empty;}
	return found->second.first;
}

CSmallStringManager::CSmallStringManager()
{
	_MaxId=0;
}

CSmallStringManager::CSmallStringManager(CObject* textManager)
{
	if(textManager==NULL)
	{
		_MaxId = 0;
		return;
	}
	CObject* texts = textManager->getAttr("Texts");
	uint32 maxId = static_cast<uint32>(textManager->getAttr("MaxId")->toNumber());
	CObject* unused = textManager->getAttr("UnusedIds");
	uint32 max = unused->getSize();

	//_MaxId
	_MaxId = maxId;

	//unused ids
	for(uint32 i=0;i<max;i++)
	{
		uint32 id = static_cast<uint32>(unused->getValue(i)->toNumber());
		_FreeIds.insert(id);
	}

	//texts
	max = texts->getSize();
	for(uint32 i=0;i<max;i++)
	{
		CObject* entry = texts->getValue(i);
		std::string text = entry->getAttr("Text")->toString();
		uint32 textId = static_cast<uint32>(entry->getAttr("TextId")->toNumber());
		uint32 textCount = static_cast<uint32>(entry->getAttr("Count")->toNumber());
		_StringToId.insert(std::pair<std::string, uint32>(text, textId));
		_IdToString.insert( std::pair<uint32, std::pair<std::string, uint32> >(textId, std::pair<std::string, uint32>(text, textCount) ) );
	}
}

void CSmallStringManager::serial(NLMISC::IStream& stream)
{
	//serialize maxId
	stream.serial(_MaxId);
	//serialize the number of free ids
	uint32 tmp = (uint32)_FreeIds.size();
	stream.serial(tmp);
	//serialize the free ids
	std::set< uint32 >::const_iterator first(_FreeIds.begin()),last(_FreeIds.end());
	for(;first!=last;++first)
	{
		tmp = *first;
		stream.serial(tmp);
	}
	tmp = (uint32)_IdToString.size();
	nlassert(tmp==_StringToId.size());

	//serialize the number of entries
	stream.serial(tmp);

	//serialize the entries
	std::map<uint32, std::pair<std::string, uint32> >::const_iterator first2(_IdToString.begin()),last2(_IdToString.end());
	for(;first2!=last2;++first2)
	{
		//serialize the entry's id
		tmp = first2->first;
		stream.serial(tmp);
		//serialize the entry's text
		std::string tmp2 = first2->second.first;
		stream.serial(tmp2);
		//serialize the entry's count
		tmp = first2->second.second;
		stream.serial(tmp);
	}
}

void CSmallStringManager::release()
{

	_StringToId.clear();

	_IdToString.clear();

	_FreeIds.clear();;

	_MaxId = 0;

}

CSmallStringManager::~CSmallStringManager()
{
	release();

}

//////////////////////////////////////////////////////////////////////////

void CStringTableManager::createTable(uint32 tableId)
{
	TLocalTable *locTable = new TLocalTable();
	if ( !_LocalTables.insert( std::make_pair(tableId, locTable)).second )
	{
		nlwarning("Table already exist %u", tableId);
		delete locTable;
		return;
	}
}




//set the new value for an entry of a local table
void CStringTableManager::setValue(uint32 tableId,const std::string& localId, const std::string& value)
{
	TLocalTable * localTable = getLocalTable(tableId);
	if(localTable == NULL)
	{
		nlwarning("unknown scenario Id or no string table for this scenario! : %d",tableId);
		return;
	}

	TLocalTable::iterator found2 = localTable->find(localId);
	if(found2!=localTable->end())
	{
		std::string oldValue = _StringMgr.getString(found2->second);
		if (oldValue=="") nlwarning("error! no string %u in string manager! ",found2->second);
		_StringMgr.unregisterString(oldValue);
		found2->second = _StringMgr.registerString(value);
	}
	else
	{
		uint32 key = _StringMgr.registerString(value);
		localTable->insert(std::make_pair(localId, key));
	}

}


std::string CStringTableManager::getValue(uint32 tableId, const std::string& localId) const
{
	// Search the table
	TLocalTable* localTable = getLocalTable(tableId);
	if(localTable == NULL)
	{
		return localId;
	}

	// Searche the key in the table
	TLocalTable::const_iterator found2 = localTable->find(localId);
	if(found2 == localTable->end())
	{
		return localId;
	}

	//get the value
	return _StringMgr.getString(found2->second);

}


CStringTableManager::TLocalTable* CStringTableManager::getLocalTable(uint32 tableId) const
{
	TLocalTables::const_iterator found = _LocalTables.find(tableId);
	if(found != _LocalTables.end()) { return found->second; }
	return NULL;
}


CStringTableManager::~CStringTableManager()
{
	release();
}

void CStringTableManager::release()
{
	_StringMgr.release();

	TLocalTables::iterator first(_LocalTables.begin()), 	last(_LocalTables.end());

	for( ; first != last; ++first)
	{

		TLocalTable* table = first->second;
		delete table;
	}
	_LocalTables.clear();


}


void CStringTableManager::releaseTable(uint32 tableId)
{

	TLocalTables::iterator found( _LocalTables.find(tableId));
	if ( found != _LocalTables.end())
	{
		TLocalTable* table = found->second;
		if (table)
		{
			TLocalTable::const_iterator first(table->begin()), last(table->end());
			for (; first != last; ++first)
			{
				_StringMgr.unregisterStringById( first->second);
			}
			delete table;
		}

		_LocalTables.erase(found);
	}
}
