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

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CClassDirectory3::CClassDirectory3()
{

}

// Parallel to CDllDirectory
CClassDirectory3::~CClassDirectory3()
{
	// Delete m_ChunkCache and m_Entries when !ChunksOwnsPointers
	if (!ChunksOwnsPointers)
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
}

std::string CClassDirectory3::getClassName()
{
	return "ClassDirectory3";
}

// Parallel to CDllDirectory
void CClassDirectory3::toString(std::ostream &ostream, const std::string &pad)
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
			case 0x2040: // ClassEntry
				{
					uint subi = 0;
					for (std::vector<CClassEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
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
void CClassDirectory3::parse(uint16 version, TParseLevel level)
{
	if (level & PARSE_INTERNAL)
	{
		// Ensure not yet parsed
		nlassert(m_ChunkCache.empty());
		nlassert(m_Entries.empty());

		// Parse entries first
		CStorageContainer::parse(version, level);

		// Initialize
		uint16 lastCached = 0xFFFF;
		bool parsedDllEntry = false;

		// Parse chunks
		for (TStorageObjectContainer::iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
		{
			uint16 id = it->first;
			switch (id)
			{
			case 0x2040: // ClassEntry
				if (parsedDllEntry && (lastCached != id))
					throw EStorageParse(); // There were chunks inbetween
				if (!parsedDllEntry)
				{
					m_ChunkCache.push_back(TStorageObjectWithId(id, NULL)); // Dummy entry to know the location
					lastCached = id;
					parsedDllEntry = true;
				}
				m_Entries.push_back(static_cast<CClassEntry *>(it->second));
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

// Parallel to CDllDirectory
void CClassDirectory3::clean()
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
	for (std::vector<CClassEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
	{
		(*subit)->clean();
	}
}

// Parallel to CDllDirectory
void CClassDirectory3::build(uint16 version)
{
	// Ensure parsed
	nlassert(!ChunksOwnsPointers);

	// Initialize
	nlassert(Chunks.empty());

	// Set up the Chunks list, when (CClassEntry::ID, NULL) is found write out all of the entries.
	for (TStorageObjectContainer::iterator it = m_ChunkCache.begin(), end = m_ChunkCache.end(); it != end; ++it)
	{
		uint16 id = it->first;
		switch (id)
		{
		case 0x2040: // ClassEntry
			for (std::vector<CClassEntry *>::iterator subit = m_Entries.begin(), subend = m_Entries.end(); subit != subend; ++subit)
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

// Parallel to CDllDirectory
void CClassDirectory3::disown()
{
	CStorageContainer::disown();
	m_ChunkCache.clear();
	m_Entries.clear();

	// Ownership goes back to Chunks
	ChunksOwnsPointers = true;
}

// Parallel to CDllDirectory
const CClassEntry *CClassDirectory3::get(std::vector<CClassEntry *>::size_type idx) const
{
	return m_Entries[idx];
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

CClassEntry::~CClassEntry()
{

}

std::string CClassEntry::getClassName()
{
	return "ClassEntry";
}

void CClassEntry::toString(std::ostream &ostream, const std::string &pad)
{
	if (m_Header && m_Name)
	{
		ostream << "(" << getClassName() << ") [" << Chunks.size() << "] PARSED { ";
		std::string padpad = pad + "\t";
		ostream << "\n" << pad << "Header: ";
		m_Header->toString(ostream, padpad);
		ostream << "\n" << pad << "Name: " << m_Name->Value.toUtf8();
		ostream << "} ";
	}
	else
	{
		CStorageContainer::toString(ostream, pad);
	}
}

void CClassEntry::parse(uint16 version, TParseLevel level)
{
	// CStorageContainer::parse(version, level);
	nlassert(ChunksOwnsPointers);
	nlassert(Chunks.size() == 2);
	TStorageObjectContainer::iterator it = Chunks.begin();
	nlassert(it->first == 0x2060); // ClassEntryHeader
	m_Header = static_cast<CClassEntryHeader *>(it->second);
	++it;
	nlassert(it->first == 0x2042); // ClassEntryName
	m_Name = static_cast<CStorageValue<ucstring> *>(it->second);
	// ++it;
}

void CClassEntry::clean()
{
	// Nothing to do here! (Chunks retains ownership)
	// CStorageContainer::clean();
}

void CClassEntry::build(uint16 version)
{
	// Nothing to do here!
	// CStorageContainer::build(version);
}

void CClassEntry::disown()
{
	// CStorageContainer::disown();
	m_Header = NULL;
	m_Name = NULL;
	nlassert(ChunksOwnsPointers);
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

std::string CClassEntryHeader::getClassName()
{
	return "ClassEntryHeader";
}

void CClassEntryHeader::serial(NLMISC::IStream &stream)
{
	stream.serial(DllIndex);
	stream.serial(ClassID);
	stream.serial(SuperClassID);
}

void CClassEntryHeader::toString(std::ostream &ostream, const std::string &pad)
{
	ostream << "(" << getClassName() << ") { ";
	ostream << "\n" << pad << "DllIndex: " << DllIndex;
	ostream << "\n" << pad << "ClassID: " << NLMISC::toString(ClassID);
	ostream << "\n" << pad << "SuperClassID: " << SuperClassID;
	ostream << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
