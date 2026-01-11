/**
 * \file class_directory_3.cpp
 * \brief CClassDirectory3
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassDirectory3
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
#include "class_directory_3.h"

// STL includes
#include <iomanip>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "dll_directory.h"

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CClassDirectory3::CClassDirectory3(CDllDirectory *dllDirectory) : m_DllDirectory(dllDirectory)
{

}

// Parallel to CDllDirectory
CClassDirectory3::~CClassDirectory3()
{
	// Delete m_ChunkCache and m_Entries when !m_ChunksOwnsPointers
	if (!m_ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
		{
			delete it->second;
		}
		for (std::vector<CClassEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
		{
			delete (*subit);
		}
	}
	m_ChunkCache.clear();
	m_Entries.clear();
	m_ClassIdToIndex.clear();
}

std::string CClassDirectory3::className() const
{
	return "ClassDirectory3";
}

// Parallel to CDllDirectory
void CClassDirectory3::toString(std::ostream &ostream, const std::string &pad) const
{
	if (m_ChunksOwnsPointers)
	{
		CStorageContainer::toString(ostream, pad);
	}
	else
	{
		ostream << "(" << className() << ") [" << m_Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		sint i = 0;
		for (TStorageObjectContainer::const_iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
		{
			uint16 id = it->first;
			switch (id)
			{
			case 0x2040: // ClassEntry
				{
					uint subi = 0;
					for (std::vector<CClassEntry *>::const_iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
					{
						ostream << "\n" << pad << "Entries[" << subi << "]: ";
						(*subit)->toString(ostream, padpad);
						++subi;
					}
				}
				break;
			default:
				std::stringstream ss;
				ss << std::hex << std::setfill('0');
				ss << std::setw(4) << it->first;
				ostream << "\n" << pad << "0x" << ss.str() << ": ";
				it->second->toString(ostream, padpad);
				++i;
				break;
			}
		}
		ostream << "} ";
	}
}

// Parallel to CDllDirectory
void CClassDirectory3::parse(uint16 version, uint filter)
{
	// Ensure not yet parsed
	nlassert(m_ChunkCache.empty());
	nlassert(m_Entries.empty());

	// Parse entries first
	CStorageContainer::parse(version);

	// Initialize
	uint16 lastCached = 0xFFFF;
	bool parsedDllEntry = false;

	// Parse chunks
	for (TStorageObjectContainer::iterator it = m_Chunks.begin(), end = m_Chunks.end(); it != end; ++it)
	{
		uint16 id = it->first;
		switch (id)
		{
		case 0x2040: // ClassEntry
			{
				if (parsedDllEntry && (lastCached != id))
					throw EStorageParse(); // There were chunks inbetween
				if (!parsedDllEntry)
				{
					m_ChunkCache.push_back(TStorageObjectWithId(id, NULL)); // Dummy entry to know the location
					lastCached = id;
					parsedDllEntry = true;
				}
				CClassEntry *classEntry = static_cast<CClassEntry *>(it->second);
				m_ClassIdToIndex[classEntry->classId()] = m_Entries.size();
				m_Entries.push_back(classEntry);
				break;
			}
		default:
			m_ChunkCache.push_back(*it); // Dummy entry to know the location
			lastCached = id;
			break;
		}
	}

	// Now ownership of the pointers lies in m_ChunkCache and m_Entries
	m_ChunksOwnsPointers = false;
}

// Parallel to CDllDirectory
void CClassDirectory3::clean()
{
	// Ensure parsed
	nlassert(!m_ChunksOwnsPointers);

	// Clear m_Chunks
	m_Chunks.clear();

	// Clean chunks
	for (TStorageObjectContainer::iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
	{
		if (it->second != NULL && it->second->isContainer())
		{
			static_cast<CStorageContainer *>(it->second)->clean();
		}
	}
	for (std::vector<CClassEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
	{
		(*subit)->clean();
	}
}

// Parallel to CDllDirectory
void CClassDirectory3::build(uint16 version, uint filter)
{
	// Ensure parsed
	nlassert(!m_ChunksOwnsPointers);

	// Initialize
	nlassert(m_Chunks.empty());

	// Set up the m_Chunks list, when (CClassEntry::ID, NULL) is found write out all of the entries.
	for (TStorageObjectContainer::iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
	{
		uint16 id = it->first;
		switch (id)
		{
		case 0x2040: // ClassEntry
			for (std::vector<CClassEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
				m_Chunks.push_back(TStorageObjectWithId(id, (*subit)));
			break;
		default:
			m_Chunks.push_back(*it);
			break;
		}
	}

	// Build the entries last (after m_Chunks is built)
	CStorageContainer::build(version);

	// NOTE: Ownership remains with m_ChunkCache and m_Entries
}

// Parallel to CDllDirectory
void CClassDirectory3::disown()
{
	m_ChunkCache.clear();
	m_Entries.clear();
	m_ClassIdToIndex.clear();

	// Ownership goes back to m_Chunks
	m_ChunksOwnsPointers = true;

	// Disown child chunks
	CStorageContainer::disown();
}

// Parallel to CDllDirectory
const CClassEntry *CClassDirectory3::get(uint16 index) const
{
	nlassert(!m_ChunksOwnsPointers);
	if (index >= m_Entries.size()) { nlerror("Index 0x%x is above the number of entries %i", (uint32)index, (uint32)m_Entries.size()); return NULL; }
	return m_Entries[index];
}

// Parallel to CDllDirectory
void CClassDirectory3::reset()
{
	nlassert(!m_ChunksOwnsPointers);
	for (std::vector<CClassEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
	{
		delete (*subit);
	}
	m_Entries.clear();
	m_ClassIdToIndex.clear();
}

// Parallel to CDllDirectory
uint16 CClassDirectory3::getOrCreateIndex(const ISceneClassDesc *sceneClassDesc)
{
	nlassert(!m_ChunksOwnsPointers);
	std::map<NLMISC::CClassId, uint16>::iterator it = m_ClassIdToIndex.find(sceneClassDesc->classId());

	// Return existing index
	if (it != m_ClassIdToIndex.end())
		return it->second;

	// Create new entry
	CClassEntry *classEntry = new CClassEntry(m_DllDirectory, sceneClassDesc);
	uint16 index = m_Entries.size();
	m_ClassIdToIndex[classEntry->classId()] = index;
	m_Entries.push_back(classEntry);
	return index;
}

IStorageObject *CClassDirectory3::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x2040: // ClassEntry
			return new CClassEntry();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Entries[13]: (ClassEntry) [2] {
//	0 0x2060: (ClassEntryHeader) {
//		DllIndex: -2
//		ClassID: (0x222b9eb9, 0x64c75fec)
//		SuperClassID: 3072 }
//	1 0x2042: (CStorageValue) { NeL Material } }

CClassEntry::CClassEntry() : m_Header(NULL), m_Name(NULL)
{

}

CClassEntry::CClassEntry(CDllDirectory *dllDirectory, const ISceneClassDesc *sceneClassDesc) : m_Header(new CClassEntryHeader()), m_Name(new CStorageValue<ucstring>())
{
	m_Chunks.push_back(TStorageObjectWithId(0x2060, m_Header));
	m_Chunks.push_back(TStorageObjectWithId(0x2042, m_Name));
	m_Header->DllIndex = dllDirectory->getOrCreateIndex(sceneClassDesc->dllPluginDesc());
	m_Header->ClassId = sceneClassDesc->classId();
	m_Header->SuperClassId = sceneClassDesc->superClassId();
	m_Name->Value = sceneClassDesc->displayName();
}

CClassEntry::~CClassEntry()
{

}

std::string CClassEntry::className() const
{
	return "ClassEntry";
}

void CClassEntry::toString(std::ostream &ostream, const std::string &pad) const
{
	if (m_Header && m_Name)
	{
		ostream << "(" << className() << ") [" << m_Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		ostream << "\n" << pad << "Header: ";
		m_Header->toString(ostream, padpad);
		ostream << "\n" << pad << "Name: " << m_Name->Value.toUtf8();
		ostream << " } ";
	}
	else
	{
		CStorageContainer::toString(ostream, pad);
	}
}

void CClassEntry::parse(uint16 version, uint filter)
{
	// CStorageContainer::parse(version);
	nlassert(m_ChunksOwnsPointers);
	nlassert(m_Chunks.size() == 2);
	TStorageObjectContainer::iterator it = m_Chunks.begin();
	nlassert(it->first == 0x2060); // ClassEntryHeader
	m_Header = static_cast<CClassEntryHeader *>(it->second);
	++it;
	nlassert(it->first == 0x2042); // ClassEntryName
	m_Name = static_cast<CStorageValue<ucstring> *>(it->second);
	// ++it;
}

void CClassEntry::clean()
{
	// Nothing to do here! (m_Chunks retains ownership)
	// CStorageContainer::clean();
}

void CClassEntry::build(uint16 version, uint filter)
{
	// Nothing to do here!
	// CStorageContainer::build(version);
}

void CClassEntry::disown()
{
	// CStorageContainer::disown();
	m_Header = NULL;
	m_Name = NULL;
	nlassert(m_ChunksOwnsPointers);
}

IStorageObject *CClassEntry::createChunkById(uint16 id, bool container)
{
	if (!container)
	{
		switch (id)
		{
		case 0x2060: // ClassEntryHeader
			return new CClassEntryHeader();
		case 0x2042: // ClassEntryName
			return new CStorageValue<ucstring>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CClassEntryHeader::CClassEntryHeader()
{

}

CClassEntryHeader::~CClassEntryHeader()
{

}

std::string CClassEntryHeader::className() const
{
	return "ClassEntryHeader";
}

void CClassEntryHeader::serial(NLMISC::IStream &stream)
{
	stream.serial(DllIndex);
	stream.serial(ClassId);
	stream.serial(SuperClassId);
}

void CClassEntryHeader::toString(std::ostream &ostream, const std::string &pad) const
{
	ostream << "(" << className() << ") { ";
	ostream << "\n" << pad << "DllIndex: " << DllIndex;
	ostream << "\n" << pad << "ClassId: " << NLMISC::toString(ClassId);
	ostream << "\n" << pad << "SuperClassId: " << SuperClassId;
	ostream << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
