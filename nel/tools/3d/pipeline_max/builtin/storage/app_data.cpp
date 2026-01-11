/**
 * \file app_data.cpp
 * \brief CAppData
 * \date 2012-08-21 11:47GMT
 * \author Jan Boon (Kaetemi)
 * CAppData
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "app_data.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#define PMBS_APP_DATA_PARSE 1

// Elevate warnings to errors in this file for stricter reading
#undef nlwarning
#define nlwarning nlerror

// Elevate debug to error in this file for debugging
// #undef nldebug
// #define nldebug nlerror

// Chunk identifiers
#define PMBS_APP_DATA_HEADER_CHUNK_ID 0x0100
#define PMBS_APP_DATA_ENTRY_CHUNK_ID 0x0110
#define PMBS_APP_DATA_ENTRY_KEY_CHUNK_ID 0x0120

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppData::TKey::TKey(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId) : ClassId(classId), SuperClassId(superClassId), SubId(subId)
{

}

bool CAppData::TKey::operator<(const CAppData::TKey &right) const
{
	if (ClassId < right.ClassId)
		return true;
	if (ClassId > right.ClassId)
		return false;
	if (SuperClassId < right.SuperClassId)
		return true;
	if (SuperClassId > right.SuperClassId)
		return false;
	if (SubId < right.SubId)
		return true;
	if (SubId > right.SubId)
		return false;
	return false;
}

bool CAppData::TKey::operator>(const CAppData::TKey &right) const
{
	if (ClassId > right.ClassId)
		return true;
	if (ClassId < right.ClassId)
		return false;
	if (SuperClassId > right.SuperClassId)
		return true;
	if (SuperClassId < right.SuperClassId)
		return false;
	if (SubId > right.SubId)
		return true;
	if (SubId < right.SubId)
		return false;
	return false;
}

bool CAppData::TKey::operator==(const CAppData::TKey &right) const
{
	return ClassId == right.ClassId && SuperClassId == right.SuperClassId && SubId == right.SubId;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppData::CAppData()
{

}

CAppData::~CAppData()
{
	if (!m_ChunksOwnsPointers)
	{
		for (TMap::const_iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
			delete subit->second;
		m_Entries.clear();
	}
}

std::string CAppData::className() const
{
	return "AppData";
}

void CAppData::toString(std::ostream &ostream, const std::string &pad) const
{
	if (m_ChunksOwnsPointers)
	{
		CStorageContainer::toString(ostream, pad);
	}
	else
	{
		ostream << "(" << className() << ") [" << m_Entries.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		uint subi = 0;
		for (TMap::const_iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
		{
			ostream << "\n" << pad << "Entries[" << subi << "]: ";
			subit->second->toString(ostream, padpad);
			++subi;
		}
		ostream << "} ";
	}
}

void CAppData::parse(uint16 version, uint filter)
{
#if PMBS_APP_DATA_PARSE
	// Cannot be parsed yet
	if (!m_ChunksOwnsPointers) { nlerror("Already parsed"); return; }

	// First parse all the child nodes
	CStorageContainer::parse(version);

	// Verify
	if (m_Chunks.size() < 2) { nlwarning("Bad container size %i", m_Chunks.size()); disown(); return; }

	// Header
	TStorageObjectContainer::iterator it = m_Chunks.begin();
	if (it->first != PMBS_APP_DATA_HEADER_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, PMBS_APP_DATA_HEADER_CHUNK_ID); disown();  return; }
	uint32 headerSize = static_cast<CStorageValue<uint32> *>(it->second)->Value;
	++it;

	// Entries
	for (TStorageObjectContainer::iterator end = m_Chunks.end(); it != end; ++it)
	{
		if (it->first != PMBS_APP_DATA_ENTRY_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, PMBS_APP_DATA_ENTRY_CHUNK_ID); disown(); return; }
		CAppDataEntry *entry = static_cast<CAppDataEntry *>(it->second);
		TKey key(entry->key()->ClassId, entry->key()->SuperClassId, entry->key()->SubId);
		if (m_Entries.find(key) != m_Entries.end()) { nlwarning("Duplicate entry"); disown(); return; }
		m_Entries[key] = entry;
	}

	// Verify or fail
	if (m_Entries.size() != headerSize) { nlwarning("Entry count %i does not match header %i", m_Entries.size(), headerSize); disown(); return; }

	// Take local ownership
	m_ChunksOwnsPointers = false;
#else
	CStorageContainer::parse(version);
#endif
}

void CAppData::clean()
{
#if PMBS_APP_DATA_PARSE
	if (m_ChunksOwnsPointers) { nldebug("Not parsed, or disowned"); return; } // Must have local ownership
	if (m_Chunks.size() == 0) { nlwarning("Already cleaned (or did not build due to coding error)"); return; } // Already cleaned
	if (m_Chunks.begin()->first != PMBS_APP_DATA_HEADER_CHUNK_ID) { nlerror("Bad id %x, expected %x", (uint32)m_Chunks.begin()->first, PMBS_APP_DATA_HEADER_CHUNK_ID); return; } // Cannot happen, because we won't have local ownership if parsing failed
	delete m_Chunks.begin()->second; // Delete the header chunk, since we own it
	m_Chunks.clear(); // Clear the remaining chunks

	// Clean raw storage
	for (TMap::const_iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
		subit->second->clean();

#else
	CStorageContainer::clean();
#endif
}

void CAppData::build(uint16 version, uint filter)
{
#if PMBS_APP_DATA_PARSE
	// Must be clean first
	if (!m_ChunksOwnsPointers && m_Chunks.size() != 0) { nlerror("Not cleaned"); return; }
	if (m_Chunks.size() != 0) { nldebug("Not parsed, or disowned"); return; }

	// Set up the header in the chunks container
	CStorageValue<uint32> *headerSize = new CStorageValue<uint32>(); // Owned locally, not by m_Chunks
	headerSize->Value = m_Entries.size();
	m_Chunks.push_back(TStorageObjectWithId(PMBS_APP_DATA_HEADER_CHUNK_ID, headerSize));

	// Set up the entries
	for (TMap::iterator it = m_Entries.begin(), end = m_Entries.end(); it != end; ++it)
		m_Chunks.push_back(TStorageObjectWithId(PMBS_APP_DATA_ENTRY_CHUNK_ID, it->second));

	// Rebuild raw storage
	CStorageContainer::build(version);
#else
	CStorageContainer::build(version);
#endif
}

void CAppData::disown()
{
#if PMBS_APP_DATA_PARSE
	if (m_ChunksOwnsPointers) { nldebug("Not parsed"); }
	if (!m_ChunksOwnsPointers && (m_Chunks.size() != (m_Entries.size() + 1))) { nlerror("Not built"); return; } // If chunks is not the owner, built chunks must match the parsed data
	// NOTE: m_Chunks must be valid at this point!

	// Disown locally
	m_Entries.clear();

	// Give ownership back
	m_ChunksOwnsPointers = true;

	// Disown all the child chunks
	CStorageContainer::disown();
#else
	CStorageContainer::disown();
#endif
}

void CAppData::init()
{
	// Cannot be init yet
	if (!m_ChunksOwnsPointers) { nlerror("Already parsed"); return; }
	if (m_Chunks.size() != 0) { nlerror("Already built or serialized"); return; }

	// We own this
	m_ChunksOwnsPointers = false;
}
/*
const uint8 *CAppData::read(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 &size) const
{
	if (m_ChunksOwnsPointers) { nlwarning("Not parsed"); return NULL; }
	TKey key(classId, superClassId, subId);
	TMap::const_iterator it = m_Entries.find(key);
	if (it == m_Entries.end()) { nldebug("Trying to read non-existant key, this is allowed, returning NULL"); return NULL; }
	size = it->second->value()->Value.size();
	return &it->second->value()->Value[0];
}

uint8 *CAppData::lock(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 capacity)
{
	if (m_ChunksOwnsPointers) { nlwarning("Not parsed"); return NULL; }
	TKey key(classId, superClassId, subId);
	TMap::const_iterator it = m_Entries.find(key);
	CAppDataEntry *appDataEntry;
	if (it == m_Entries.end())
	{
		appDataEntry = new CAppDataEntry();
		m_Entries[key] = appDataEntry;
		appDataEntry->key()->ClassId = classId;
		appDataEntry->key()->SuperClassId = superClassId;
		appDataEntry->key()->SubId = subId;
	}
	else
	{
		appDataEntry = it->second;
	}
	appDataEntry->key()->Size = capacity;
	appDataEntry->value()->Value.resize(capacity);
	return &appDataEntry->value()->Value[0];
}

void CAppData::unlock(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 size)
{
	if (m_ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	TKey key(classId, superClassId, subId);
	TMap::const_iterator it = m_Entries.find(key);
	if (it == m_Entries.end()) { nlerror("Unlocking non-existant key"); return; }
	CAppDataEntry *appDataEntry = it->second;
	appDataEntry->key()->Size = size;
	appDataEntry->value()->Value.resize(size);
}

void CAppData::fill(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint8 *buffer, uint32 size)
{
	uint8 *dest = lock(classId, superClassId, subId, size);
	memcpy(dest, buffer, size);

	// Internally not necessary, since we sent the correct size.
	// Outside classes should unlock in case the implementation changes.
	// unlock(classId, superClassId, subId, size);
}
*/
void CAppData::erase(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId)
{
	if (m_ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	TKey key(classId, superClassId, subId);
	TMap::const_iterator it = m_Entries.find(key);
	if (it == m_Entries.end()) { nldebug("Trying to erase non-existant key, this is allowed, doing nothing"); return; }
	m_Entries.erase(key);
}

IStorageObject *CAppData::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMBS_APP_DATA_HEADER_CHUNK_ID:
		nlassert(!container);
		return new CStorageValue<uint32>();
		break;
	case PMBS_APP_DATA_ENTRY_CHUNK_ID:
		nlassert(container);
		return new CAppDataEntry();
		break;
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppDataEntryKey::CAppDataEntryKey()
{

}

CAppDataEntryKey::~CAppDataEntryKey()
{

}

std::string CAppDataEntryKey::className() const
{
	return "AppDataEntryKey";
}

void CAppDataEntryKey::serial(NLMISC::IStream &stream)
{
	stream.serial(ClassId);
	stream.serial(SuperClassId);
	stream.serial(SubId);
	stream.serial(Size);
}

void CAppDataEntryKey::toString(std::ostream &ostream, const std::string &pad) const
{
	ostream << "(" << className() << ") { ";
	ostream << "\n" << pad << "ClassId: " << NLMISC::toString(ClassId);
	ostream << "\n" << pad << "SuperClassId: " << SuperClassId;
	ostream << "\n" << pad << "SubId: " << SubId;
	ostream << "\n" << pad << "Size: " << Size;
	ostream << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppDataEntry::CAppDataEntry() : m_Key(NULL), m_Raw(NULL), m_Value(NULL)
{

}

CAppDataEntry::~CAppDataEntry()
{
	delete m_Value;
	m_Value = NULL;
}

std::string CAppDataEntry::className() const
{
	return "AppDataEntry";
}

void CAppDataEntry::toString(std::ostream &ostream, const std::string &pad) const
{
	if (m_Key && m_Value)
	{
		ostream << "(" << className() << ") [" << m_Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		ostream << "\n" << pad << "Key: ";
		m_Key->toString(ostream, padpad);
		ostream << "\n" << pad << "Value: ";
		m_Value->toString(ostream, padpad);
		ostream << "} ";
	}
	else
	{
		CStorageContainer::toString(ostream, pad);
	}
}

void CAppDataEntry::parse(uint16 version, uint filter)
{
	// CStorageContainer::parse(version);
	// if (!m_ChunksOwnsPointers) { nlwarning("Already parsed"); return; }
	if (m_Chunks.size() != 2) { nlwarning("Bad container size"); disown(); return; }

	TStorageObjectContainer::iterator it = m_Chunks.begin();
	if (it->first != PMBS_APP_DATA_ENTRY_KEY_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, PMBS_APP_DATA_ENTRY_KEY_CHUNK_ID); disown();  return; }
	m_Key = static_cast<CAppDataEntryKey *>(it->second);

	++it;
	if (it->first != PMBS_APP_DATA_ENTRY_VALUE_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, PMBS_APP_DATA_ENTRY_VALUE_CHUNK_ID); disown(); return; }
	m_Raw = static_cast<CStorageRaw *>(it->second);

	// m_ChunksOwnsPointers = false;
	nlassert(m_ChunksOwnsPointers); // Never set false here
}

void CAppDataEntry::clean()
{
	// CStorageContainer::clean();
	if (m_Value)
	{
		nlassert(m_Raw);
		m_Raw->Value.resize(0);
		m_Key->Size = 0xFFFFFFFF;
	}
}

void CAppDataEntry::build(uint16 version, uint filter)
{
	// CStorageContainer::build(version);
	if (m_Value)
	{
		nlassert(m_Raw);
		NLMISC::CMemStream mem;
		m_Value->serial(mem);
		m_Raw->setSize(mem.getPos());
		mem.invert();
		m_Raw->serial(mem);
		m_Key->Size = m_Raw->Value.size();
	}
}

void CAppDataEntry::disown()
{
	// CStorageContainer::disown();
	if (m_Chunks.size() != 2) { nlerror("Not built"); return; } // Built chunks must match the parsed data
	m_Key = NULL;
	m_Raw = NULL;
	delete m_Value;
	m_Value = NULL;
}

void CAppDataEntry::init()
{
	nlassert(m_Chunks.size() == 0);
	m_Key = new CAppDataEntryKey();
	m_Chunks.push_back(TStorageObjectWithId(PMBS_APP_DATA_ENTRY_KEY_CHUNK_ID, m_Key));
	m_Raw = new CStorageRaw();
	m_Chunks.push_back(TStorageObjectWithId(PMBS_APP_DATA_ENTRY_VALUE_CHUNK_ID, m_Raw));
}

IStorageObject *CAppDataEntry::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMBS_APP_DATA_ENTRY_KEY_CHUNK_ID:
		nlassert(!container);
		return new CAppDataEntryKey();
	case PMBS_APP_DATA_ENTRY_VALUE_CHUNK_ID:
		nlassert(!container);
		return new CStorageRaw();
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
