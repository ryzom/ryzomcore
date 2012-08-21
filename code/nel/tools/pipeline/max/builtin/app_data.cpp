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

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#define NLMAXFILE_APP_DATA_HEADER_CHUNK_ID 0x0100
#define NLMAXFILE_APP_DATA_ENTRY_CHUNK_ID 0x0110
#define NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID 0x0120
#define NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID 0x0130

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

}

std::string CAppData::getClassName()
{
	return "AppData";
}

void CAppData::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

void CAppData::parse(uint16 version, TParseLevel level)
{
	CStorageContainer::parse(version, level);
}

void CAppData::clean()
{
	CStorageContainer::clean();
}

void CAppData::build(uint16 version)
{
	CStorageContainer::build(version);
}

void CAppData::disown()
{
	CStorageContainer::disown();
}

const uint8 *CAppData::read(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 &size) const
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return NULL; }
	TKey key(classId, superClassId, subId);
	TMap::const_iterator it = m_Entries.find(key);
	if (it == m_Entries.end()) return NULL;
	size = it->second->value()->Value.size();
	return &it->second->value()->Value[0];
}

uint8 *CAppData::lock(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 capacity)
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return NULL; }
	TKey key(classId, superClassId, subId);
}

void CAppData::unlock(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 size)
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	TKey key(classId, superClassId, subId);
}

void CAppData::fill(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint8 *buffer, uint32 size)
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	TKey key(classId, superClassId, subId);
}

void CAppData::erase(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId)
{
	if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	TKey key(classId, superClassId, subId);
}

IStorageObject *CAppData::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case NLMAXFILE_APP_DATA_HEADER_CHUNK_ID:
		nlassert(!container);
		return new CStorageValue<uint32>();
		break;
	case NLMAXFILE_APP_DATA_ENTRY_CHUNK_ID:
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

std::string CAppDataEntryKey::getClassName()
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

void CAppDataEntryKey::toString(std::ostream &ostream, const std::string &pad)
{
	ostream << "(" << getClassName() << ") { ";
	ostream << "\n" << pad << "ClassId: " << NLMISC::toString(ClassId);
	ostream << "\n" << pad << "SuperClassId: " << SuperClassId;
	ostream << "\n" << pad << "SubId: " << SubId;
	ostream << "\n" << pad << "Size: " << Size;
	ostream << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CAppDataEntry::CAppDataEntry() : m_Key(NULL), m_Value(NULL)
{

}

CAppDataEntry::~CAppDataEntry()
{

}

std::string CAppDataEntry::getClassName()
{
	return "AppDataEntry";
}

void CAppDataEntry::toString(std::ostream &ostream, const std::string &pad)
{
	if (m_Key && m_Value)
	{
		ostream << "(" << getClassName() << ") [" << Chunks.size() << "] PARSED { ";
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

void CAppDataEntry::parse(uint16 version, TParseLevel level)
{
	if (level & PARSE_BUILTIN)
	{
		// CStorageContainer::parse(version, level);
		// if (!ChunksOwnsPointers) { nlwarning("Already parsed"); return; }
		if (Chunks.size() != 2) { nlwarning("Bad container size"); disown(); return; }

		TStorageObjectContainer::iterator it = Chunks.begin();
		if (it->first != NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID); disown();  return; }
		m_Key = static_cast<CAppDataEntryKey *>(it->second);

		++it;
		if (it->first != NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID) { nlwarning("Bad id %x, expected %x", (uint32)it->first, NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID); disown(); return; }
		m_Value = static_cast<CStorageRaw *>(it->second);

		// ChunksOwnsPointers = false;
	}

}

void CAppDataEntry::clean()
{
	// CStorageContainer::clean();
	// if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	// Nothing to do here!
}

void CAppDataEntry::build(uint16 version)
{
	// if (ChunksOwnsPointers) { nlwarning("Not parsed"); return; }
	// Nothing to do here!
	// CStorageContainer::build(version);
}

void CAppDataEntry::disown()
{
	// CStorageContainer::disown();
	m_Key = NULL;
	m_Value = NULL;
}

void CAppDataEntry::init()
{
	nlassert(Chunks.size() == 0);
	m_Key = new CAppDataEntryKey();
	Chunks.push_back(TStorageObjectWithId(NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID, m_Key));
	m_Value = new CStorageRaw();
	Chunks.push_back(TStorageObjectWithId(NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID, m_Value));
}

CStorageRaw *CAppDataEntry::value()
{
	return m_Value;
}

IStorageObject *CAppDataEntry::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case NLMAXFILE_APP_DATA_ENTRY_KEY_CHUNK_ID:
		nlassert(!container);
		return new CAppDataEntryKey();
	case NLMAXFILE_APP_DATA_ENTRY_VALUE_CHUNK_ID:
		nlassert(!container);
		return new CStorageRaw();
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
