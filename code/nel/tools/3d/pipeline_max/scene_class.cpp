/**
 * \file scene_class.cpp
 * \brief CSceneClass
 * \date 2012-08-20 09:07GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClass
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
#include "scene_class.h"

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

// Elevate warnings to errors in this file for stricter reading
#undef nlwarning
#define nlwarning nlerror

// Elevate debug to error in this file for debugging
// #undef nldebug
// #define nldebug nlerror

// Chunk identifiers
// #define ...

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CSceneClass::CSceneClass(CScene *scene) : m_Scene(scene)
{

}

CSceneClass::~CSceneClass()
{
	if (!m_ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = m_OrphanedChunks.begin(), end = m_OrphanedChunks.end(); it != end; ++it)
			delete it->second;
		m_OrphanedChunks.clear();
		for (std::vector<IStorageObject *>::iterator it = m_ArchivedChunks.begin(), end = m_ArchivedChunks.end(); it != end; ++it)
			delete (*it);
		m_ArchivedChunks.clear();
	}
}

std::string CSceneClass::className() const
{
	return classDesc()->internalName();
}

void CSceneClass::toString(std::ostream &ostream, const std::string &pad) const
{
	if (m_ChunksOwnsPointers)
	{
		CStorageContainer::toString(ostream, pad);
	}
	else
	{
		ostream << "<ptr=0x";
		{
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			ss << std::setw(16) << (uint64)(void *)this;
			ostream << ss.str();
		}
		ostream << "> ";
		ostream << "(" << className() << ": " << ucstring(classDesc()->displayName()).toUtf8() << ", " << classDesc()->classId().toString() << ", 0x";
		{
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			ss << std::setw(8) << classDesc()->superClassId();
			ostream << ss.str();
		}
		ostream << ", " << ucstring(classDesc()->dllPluginDesc()->internalName()).toUtf8() << ") [" << m_OrphanedChunks.size() << "] { ";
		toStringLocal(ostream, pad);
		// Append orphans
		std::string padpad = pad + "\t";
		sint i = 0;
		for (TStorageObjectContainer::const_iterator it = m_OrphanedChunks.begin(), end = m_OrphanedChunks.end(); it != end; ++it)
		{
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			ss << std::setw(4) << it->first;
			ostream << "\n" << pad << "Orphan[" << i << "] 0x" << ss.str() << ": ";
			it->second->toString(ostream, padpad);
			++i;
		}
		ostream << "} ";
	}
}

void CSceneClass::parse(uint16 version, uint filter)
{
	// Cannot be parsed yet
	if (!m_ChunksOwnsPointers) { nlerror("Already parsed"); return; } // Already parsed, illegal to call twice

	// Parse all child chunks
	CStorageContainer::parse(version);

	// Orphanize all child chunk
	m_OrphanedChunks.insert(m_OrphanedChunks.end(), m_Chunks.begin(), m_Chunks.end());

	// Take ownership
	m_ChunksOwnsPointers = false;

	// Inheriting classes take control from here on, they should check
	// m_ChunksOwnsPointers to be false before taking action, in case
	// a subclass called disown due to failure.
}

void CSceneClass::clean()
{
	if (m_ChunksOwnsPointers) { nldebug("Not parsed, or disowned"); return; } // Must have local ownership, parsing may have failed
	if (m_Chunks.size() == 0 && m_OrphanedChunks.size() != 0) { nlwarning("Already cleaned"); return; } // Already cleaned, should not call twice, not reliable because not all chunks have child chunks

	// Clear unneeded references from the parent
	m_Chunks.clear();

	// Clean owned child chunks
	for (TStorageObjectContainer::const_iterator it = m_OrphanedChunks.begin(), end = m_OrphanedChunks.end(); it != end; ++it)
	{
		if (it->second->isContainer())
		{
			static_cast<CStorageContainer *>(it->second)->clean();
		}
	}

	// Erase archived chunks, they must have been parsed perfectly
	for (std::vector<IStorageObject *>::iterator it = m_ArchivedChunks.begin(), end = m_ArchivedChunks.end(); it != end; ++it)
		delete (*it);
	m_ArchivedChunks.clear();
}

void CSceneClass::build(uint16 version, uint filter)
{
	// Must be clean first
	if (!m_ChunksOwnsPointers && m_Chunks.size() != 0) { nlerror("Not cleaned"); return; } // Cannot call twice, illegal call
	if (m_Chunks.size() != 0) { nldebug("Not parsed, or disowned"); return; } // Don't have local ownership, parsing may have failed, the built version is implicitly up to date

	// Store orphan chunks
	m_Chunks.insert(m_Chunks.end(), m_OrphanedChunks.begin(), m_OrphanedChunks.end());

	// Build the orphan chunks (this is a little trick to do it this
	// way here, don't do this from subclasses)
	CStorageContainer::build(version);

	// Set the insertion pointer before the orphans
	m_PutChunkInsert = m_Chunks.begin();

	// Inheriting classes take control from here on, so the build is
	// called to owned subclasses from the putChunk function.
}

void CSceneClass::disown()
{
	if (m_ChunksOwnsPointers) { nldebug("Not parsed"); }
	if (!m_ChunksOwnsPointers && (m_Chunks.size() < m_OrphanedChunks.size())) { nlerror("Not built"); return; } // If chunks is not the owner, built chunks must match the parsed data. This check is not fully reliable

	// Clear local references
	m_OrphanedChunks.clear();
	m_ArchivedChunks.clear();

	// Return ownership
	m_ChunksOwnsPointers = true;

	// Disown children
	CStorageContainer::disown();
}

void CSceneClass::init()
{
	// Nothing to do here!
}

IStorageObject *CSceneClass::createChunkById(uint16 id, bool container)
{
	return CStorageContainer::createChunkById(id, container);
}

const ucstring CSceneClass::DisplayName = ucstring("Invalid Scene Class");
const char *CSceneClass::InternalName = "SceneClass";
const NLMISC::CClassId CSceneClass::ClassId = NLMISC::CClassId::Null; // This class is invalid
const TSClassId CSceneClass::SuperClassId = 0x0000; // This class is invalid

namespace {
static const CSceneClassDesc<CSceneClass> SceneClassDesc(static_cast<const IDllPluginDescInternal *>(&DllPluginDescBuiltin));
} /* anonymous namespace */

bool CSceneClass::inherits(const NLMISC::CClassId classId) const
{
	return false;
}

const ISceneClassDesc *CSceneClass::classDesc() const
{
	return static_cast<const ISceneClassDesc *>(&SceneClassDesc);
}

void CSceneClass::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	// Nothing to do here...
}

IStorageObject *CSceneClass::getChunk(uint16 id)
{
	if (m_OrphanedChunks.begin()->first == id)
	{
		IStorageObject *result = m_OrphanedChunks.begin()->second;
		m_OrphanedChunks.pop_front();
		return result;
	}
	else
	{
		for (TStorageObjectContainer::iterator it = m_OrphanedChunks.begin(), end = m_OrphanedChunks.end(); it != end; ++it)
		{
			if (it->first == id)
			{
				nlwarning("Try to get chunk with 0x%x id, but found 0x%x instead. Found the correct chunk at a different position. Unknown chunks, or chunks out of order", (uint32)id, (uint32)m_OrphanedChunks.begin()->first);
				IStorageObject *result = it->second;
				m_OrphanedChunks.erase(it);
				return result;
			}
		}
	}
	// nldebug("Chunk 0x%x not found, this is allowed, returning NULL", (uint32)id);
	return NULL;
}

void CSceneClass::putChunk(uint16 id, IStorageObject *storageObject)
{
	if (storageObject->isContainer())
	{
		static_cast<CStorageContainer *>(storageObject)->build(VersionUnknown); // FIXME
	}
	m_OrphanedChunks.insert(m_PutChunkInsert, TStorageObjectWithId(id, storageObject));
}

uint16 CSceneClass::peekChunk()
{
	if (m_OrphanedChunks.size())
		return m_OrphanedChunks.begin()->first;
	return 0x0000;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
