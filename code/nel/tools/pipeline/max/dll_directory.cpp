/**
 * \file dll_directory.cpp
 * \brief CDllDirectory
 * \date 2012-08-18 09:01GMT
 * \author Jan Boon (Kaetemi)
 * CDllDirectory
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
#include "dll_directory.h"

// STL includes
#include <iomanip>

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

CDllDirectory::CDllDirectory() : m_DllEntryBuiltin(&DllPluginDescBuiltin), m_DllEntryScript(&DllPluginDescScript)
{

}

// Parallel to CClassDirectory3
CDllDirectory::~CDllDirectory()
{
	// Delete m_ChunkCache and m_Entries when !ChunksOwnsPointers
	if (!ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
		{
			delete it->second;
		}
		for (std::vector<CDllEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
		{
			delete (*subit);
		}
	}
	m_ChunkCache.clear();
	m_Entries.clear();
}

std::string CDllDirectory::getClassName()
{
	return "DllDirectory";
}

// Parallel to CClassDirectory3
void CDllDirectory::toString(std::ostream &ostream, const std::string &pad)
{
	if (ChunksOwnsPointers)
	{
		CStorageContainer::toString(ostream, pad);
	}
	else
	{
		ostream << "(" << getClassName() << ") [" << Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		sint i = 0;
		for (TStorageObjectContainer::const_iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
		{
			uint16 id = it->first;
			switch (id)
			{
			case 0x2038: // DllEntry
				{
					uint subi = 0;
					for (std::vector<CDllEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
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

// Parallel to CClassDirectory3
void CDllDirectory::parse(uint16 version, TParseLevel level)
{
	if (level & PARSE_INTERNAL)
	{
		// Ensure not yet parsed
		nlassert(m_ChunkCache.empty());
		nlassert(m_Entries.empty());

		// Parse entries first
		CStorageContainer::parse(version, level);

		// Initialize
		addInternalIndices();
		uint16 lastCached = 0xFFFF;
		bool parsedDllEntry = false;

		// Parse chunks
		for (TStorageObjectContainer::iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
		{
			uint16 id = it->first;
			switch (id)
			{
			case 0x2038: // DllEntry
				{
					if (parsedDllEntry && (lastCached != id))
						throw EStorageParse(); // There were chunks inbetween
					if (!parsedDllEntry)
					{
						m_ChunkCache.push_back(TStorageObjectWithId(id, NULL)); // Dummy entry to know the location
						lastCached = id;
						parsedDllEntry = true;
					}
					CDllEntry *dllEntry = static_cast<CDllEntry *>(it->second);
					m_InternalNameToIndex[NLMISC::toLower(dllEntry->dllFilename())] = m_Entries.size();
					m_Entries.push_back(dllEntry);
				}
				break;
			default:
				m_ChunkCache.push_back(*it); // Dummy entry to know the location
				lastCached = id;
				break;
			}
		}

		// Now ownership of the pointers lies in m_ChunkCache and m_Entries
		ChunksOwnsPointers = false;
	}
}

// Parallel to CClassDirectory3
void CDllDirectory::clean()
{
	// Ensure parsed
	nlassert(!ChunksOwnsPointers);

	// Clear Chunks
	Chunks.clear();

	// Clean chunks
	for (TStorageObjectContainer::iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
	{
		if (it->second != NULL && it->second->isContainer())
		{
			static_cast<CStorageContainer *>(it->second)->clean();
		}
	}
	for (std::vector<CDllEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
	{
		(*subit)->clean();
	}
}

// Parallel to CClassDirectory3
void CDllDirectory::build(uint16 version)
{
	// Ensure parsed
	nlassert(!ChunksOwnsPointers);

	// Initialize
	nlassert(Chunks.empty());

	// Set up the Chunks list, when (CDllEntry::ID, NULL) is found write out all of the entries.
	for (TStorageObjectContainer::iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
	{
		uint16 id = it->first;
		switch (id)
		{
		case 0x2038: // DllEntry
			for (std::vector<CDllEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
				Chunks.push_back(TStorageObjectWithId(id, (*subit)));
			break;
		default:
			Chunks.push_back(*it);
			break;
		}
	}

	// Build the entries last (after Chunks is built)
	CStorageContainer::build(version);

	// NOTE: Ownership remains with m_ChunkCache and m_Entries
}

// Parallel to CClassDirectory3
void CDllDirectory::disown()
{
	CStorageContainer::disown();
	m_ChunkCache.clear();
	m_Entries.clear();
	m_InternalNameToIndex.clear();

	// Ownership goes back to Chunks
	ChunksOwnsPointers = true;
}

// Parallel to CClassDirectory3
const CDllEntry *CDllDirectory::get(sint32 index) const
{
	nlassert(!ChunksOwnsPointers);
	if (index < 0)
	{
		// Handle internal dummy values
		switch (index)
		{
		case -1:
			return &m_DllEntryBuiltin;
		case -2:
			return &m_DllEntryScript;
		default:
			nlerror("Bad dll entry index");
		}
	}
	else
	{
		nlassert(index < (sint32)m_Entries.size());
		return m_Entries[index];
	}
	nlassert(false);
	return NULL;
}

// Parallel to CClassDirectory3
void CDllDirectory::reset()
{
	nlassert(!ChunksOwnsPointers);
	for (std::vector<CDllEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
	{
		delete (*subit);
	}
	m_Entries.clear();
	m_InternalNameToIndex.clear();
	addInternalIndices();
}

// Parallel to CClassDirectory3
sint32 CDllDirectory::getOrCreateIndex(const IDllPluginDescInternal *dllPluginDesc)
{
	nlassert(!ChunksOwnsPointers);
	std::map<ucstring, sint32>::iterator it = m_InternalNameToIndex.find(NLMISC::toLower(ucstring(dllPluginDesc->internalName())));

	// Return existing index
	if (it != m_InternalNameToIndex.end())
		return it->second;

	// Create new entry
	CDllEntry *dllEntry = new CDllEntry(dllPluginDesc);
	sint32 index = m_Entries.size();
	m_InternalNameToIndex[NLMISC::toLower(dllEntry->dllFilename())] = index;
	m_Entries.push_back(dllEntry);
	return index;
}

void CDllDirectory::addInternalIndices()
{
	m_InternalNameToIndex[NLMISC::toLower(ucstring(DllPluginDescBuiltin.internalName()))] = -1;
	m_InternalNameToIndex[NLMISC::toLower(ucstring(DllPluginDescScript.internalName()))] = -2;
}

IStorageObject *CDllDirectory::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x2038: // DllEntry
			return new CDllEntry();
		}
	}
	else
	{
		switch (id)
		{
		case 0x21C0: // DllDirectoryHeader
			return new CStorageValue<uint32>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Entries[5]: (DllEntry) [2] PARSED {
// 	DllDescription: ...
// 	DllFilename: ... }

CDllEntry::CDllEntry() : m_DllDescription(NULL), m_DllFilename(NULL)
{

}

CDllEntry::CDllEntry(const IDllPluginDescInternal *dllPluginDesc) : m_DllDescription(new CStorageValue<ucstring>()), m_DllFilename(new CStorageValue<ucstring>())
{
	Chunks.push_back(TStorageObjectWithId(0x2039, m_DllDescription));
	Chunks.push_back(TStorageObjectWithId(0x2037, m_DllFilename));
	m_DllDescription->Value = dllPluginDesc->displayName();
	m_DllFilename->Value = dllPluginDesc->internalName();
}

CDllEntry::~CDllEntry()
{

}

std::string CDllEntry::getClassName()
{
	return "DllEntry";
}

void CDllEntry::toString(std::ostream &ostream, const std::string &pad)
{
	if (m_DllDescription && m_DllFilename)
	{
		ostream << "(" << getClassName() << ") [" << Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		ostream << "\n" << pad << "DllDescription: " << m_DllDescription->Value.toUtf8();
		ostream << "\n" << pad << "DllFilename: " << m_DllFilename->Value.toUtf8();
		ostream << " } ";
	}
	else
	{
		CStorageContainer::toString(ostream, pad);
	}
}

void CDllEntry::parse(uint16 version, TParseLevel level)
{
	// CStorageContainer::parse(version, level);
	nlassert(ChunksOwnsPointers);
	nlassert(Chunks.size() == 2);
	TStorageObjectContainer::iterator it = Chunks.begin();
	nlassert(it->first == 0x2039); // DllDescription
	m_DllDescription = static_cast<CStorageValue<ucstring> *>(it->second);
	++it;
	nlassert(it->first == 0x2037); // DllFilename
	m_DllFilename = static_cast<CStorageValue<ucstring> *>(it->second);
	// ++it;
}

void CDllEntry::clean()
{
	// Nothing to do here! (Chunks retains ownership)
	// CStorageContainer::clean();
}

void CDllEntry::build(uint16 version)
{
	// Nothing to do here!
	// CStorageContainer::build(version);
}

void CDllEntry::disown()
{
	// CStorageContainer::disown();
	m_DllDescription = NULL;
	m_DllFilename = NULL;
	nlassert(ChunksOwnsPointers);
}

IStorageObject *CDllEntry::createChunkById(uint16 id, bool container)
{
	if (!container)
	{
		switch (id)
		{
		case 0x2039: // DllDescription
		case 0x2037: // DllFilename
			return new CStorageValue<ucstring>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
