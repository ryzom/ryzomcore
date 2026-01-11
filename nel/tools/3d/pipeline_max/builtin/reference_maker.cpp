/**
 * \file reference_maker.cpp
 * \brief CReferenceMaker
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * CReferenceMaker
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
#include "reference_maker.h"

// STL includes
#include <iomanip>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

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
#define PMB_REFERENCES_2034_CHUNK_ID 0x2034
#define PMB_REFERENCES_2035_CHUNK_ID 0x2035
#define PMB_204B_EQUALS_2E_CHUNK_ID 0x204B
#define PMB_UNKNOWN2045_CHUNK_ID 0x2045
#define PMB_UNKNOWN2047_CHUNK_ID 0x2047
#define PMB_UNKNOWN21B0_CHUNK_ID 0x21B0

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CReferenceMaker::CReferenceMaker(CScene *scene) : CAnimatable(scene), m_ReferenceMap(false), m_204B_Equals_2E(NULL), m_References2035Value0(0), m_Unknown2045(NULL), m_Unknown2047(NULL), m_Unknown21B0(NULL)
{

}

CReferenceMaker::~CReferenceMaker()
{
	if (!m_ChunksOwnsPointers)
	{
		delete m_204B_Equals_2E;
		m_204B_Equals_2E = NULL;
		delete m_Unknown2045;
		m_Unknown2045 = NULL;
		delete m_Unknown2047;
		m_Unknown2047 = NULL;
		delete m_Unknown21B0;
		m_Unknown21B0 = NULL;
	}
}

const ucstring CReferenceMaker::DisplayName = ucstring("ReferenceMaker");
const char *CReferenceMaker::InternalName = "ReferenceMaker";
const char *CReferenceMaker::InternalNameUnknown = "ReferenceMakerUnknown";
const NLMISC::CClassId CReferenceMaker::ClassId = NLMISC::CClassId(0x2ec43d15, 0x10a270ad); /* Not official, please correct */
const TSClassId CReferenceMaker::SuperClassId = 0x00000100;
const CReferenceMakerClassDesc ReferenceMakerClassDesc(&DllPluginDescBuiltin);
const CReferenceMakerSuperClassDesc ReferenceMakerSuperClassDesc(&ReferenceMakerClassDesc);

void CReferenceMaker::parse(uint16 version, uint filter)
{
	CAnimatable::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		CStorageArray<sint32> *references2034 = static_cast<CStorageArray<sint32> *>(getChunk(PMB_REFERENCES_2034_CHUNK_ID));
		CStorageArray<sint32> *references2035 = static_cast<CStorageArray<sint32> *>(getChunk(PMB_REFERENCES_2035_CHUNK_ID));
		if (references2034) nlassert(references2035 == NULL); // Apparently, there can be only one.
		if (references2035) nlassert(references2034 == NULL);
		m_204B_Equals_2E = static_cast<CStorageValue<uint8> *>(getChunk(PMB_204B_EQUALS_2E_CHUNK_ID));
		if (m_204B_Equals_2E) nlassert(m_204B_Equals_2E->Value == 0x2e); // Really, let me know when it has another value.

		// Parse contents
		if (references2034)
		{
			m_ReferenceMap = false; // NOTE: Plugins may check after parse if they parsed with the correct type, to find a pattern
			m_ArchivedChunks.push_back(references2034);
			for (std::vector<sint32>::size_type i = 0; i < references2034->Value.size(); ++i)
			{
				if (references2034->Value[i] > 0)
				{
					CReferenceMaker *referenceMaker = dynamic_cast<CReferenceMaker *>(container()->getByStorageIndex(references2034->Value[i]));
					if (!referenceMaker) nlerror("Reference maker %s %s, 0x%x is not a reference maker", ucstring(container()->getByStorageIndex(references2034->Value[i])->classDesc()->displayName()).toUtf8().c_str(), container()->getByStorageIndex(references2034->Value[i])->classDesc()->classId().toString().c_str(), container()->getByStorageIndex(references2034->Value[i])->classDesc()->superClassId());
					setReference(i, referenceMaker);
				}
			}
		}
		if (references2035)
		{
			m_ReferenceMap = true;
			m_ArchivedChunks.push_back(references2035);
			std::vector<sint32>::iterator it = references2035->Value.begin();
			m_References2035Value0 = (*it);
			++it;
			std::vector<sint32>::iterator end = references2035->Value.end();
			while (it != end)
			{
				sint32 index = (*it);
				++it;
				sint32 referenceindex = (*it);
				++it;
				CReferenceMaker *referenceMaker = dynamic_cast<CReferenceMaker *>(container()->getByStorageIndex(referenceindex));
				if (!referenceMaker) nlerror("Reference maker %s, 0x%x is not a reference maker", container()->getByStorageIndex(referenceindex)->classDesc()->classId().toString().c_str(), container()->getByStorageIndex(referenceindex)->classDesc()->superClassId());
				setReference(index, referenceMaker);
			}
		}

		m_Unknown2045 = static_cast<CStorageRaw *>(getChunk(PMB_UNKNOWN2045_CHUNK_ID)); // not sure if this is part of maker or target
		m_Unknown2047 = static_cast<CStorageRaw *>(getChunk(PMB_UNKNOWN2047_CHUNK_ID)); // not sure if this is part of maker or target
		m_Unknown21B0 = static_cast<CStorageRaw *>(getChunk(PMB_UNKNOWN21B0_CHUNK_ID)); // not sure if this is part of maker or target
	}
}

void CReferenceMaker::clean()
{
	CAnimatable::clean(); // Nothing to do here, m_ArchivedChunks is cleaned (deleted) for us!
}

void CReferenceMaker::build(uint16 version, uint filter)
{
	CAnimatable::build(version);
	// TODO: Build contents
	//if (m_References2034) putChunk(PMB_REFERENCES_2034_CHUNK_ID, m_References2034);
	//if (m_References2035) putChunk(PMB_REFERENCES_2035_CHUNK_ID, m_References2035);
	if (!m_ReferenceMap)
	{
		CStorageArray<sint32> *references2034 = new CStorageArray<sint32>();
		uint nb = nbReferences();
		references2034->Value.resize(nb);
		for (uint i = 0; i < nb; ++i)
		{
			CReferenceMaker *referenceMaker = getReference(i);
			if (referenceMaker) references2034->Value[i] = container()->getOrCreateStorageIndex(referenceMaker);
			else references2034->Value[i] = -1;
		}
		putChunk(PMB_REFERENCES_2034_CHUNK_ID, references2034);
		m_ArchivedChunks.push_back(references2034);
	}
	else
	{
		CStorageArray<sint32> *references2035 = new CStorageArray<sint32>();
		uint nb = nbReferences();
		references2035->Value.push_back(m_References2035Value0);
		for (uint i = 0; i < nb; ++i)
		{
			CReferenceMaker *referenceMaker = getReference(i);
			if (referenceMaker)
			{
				references2035->Value.push_back(i);
				references2035->Value.push_back(container()->getOrCreateStorageIndex(referenceMaker));
			}
		}
		putChunk(PMB_REFERENCES_2035_CHUNK_ID, references2035);
		m_ArchivedChunks.push_back(references2035);
	}
	if (m_204B_Equals_2E) putChunk(PMB_204B_EQUALS_2E_CHUNK_ID, m_204B_Equals_2E);
	if (m_Unknown2045) putChunk(PMB_UNKNOWN2045_CHUNK_ID, m_Unknown2045);
	if (m_Unknown2047) putChunk(PMB_UNKNOWN2047_CHUNK_ID, m_Unknown2047);
	if (m_Unknown21B0) putChunk(PMB_UNKNOWN21B0_CHUNK_ID, m_Unknown21B0);
}

void CReferenceMaker::disown()
{
	m_References.clear();
	m_ReferenceMap = false;
	m_References2035Value0 = 0;
	m_204B_Equals_2E = NULL;
	m_Unknown2045 = NULL;
	m_Unknown2047 = NULL;
	m_Unknown21B0 = NULL;
	CAnimatable::disown();
}

void CReferenceMaker::init()
{
	CAnimatable::init();
}

bool CReferenceMaker::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CAnimatable::inherits(classId);
}

const ISceneClassDesc *CReferenceMaker::classDesc() const
{
	return &ReferenceMakerClassDesc;
}

void CReferenceMaker::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CAnimatable::toStringLocal(ostream, pad);
	uint nb = nbReferences();
	if (nb)
	{
		if (!m_ReferenceMap) ostream << "\n" << pad << "References 0x2034: ";
		else ostream << "\n" << pad << "References 0x2035: ";
		std::string padpad = pad + "\t";
		ostream << "PARSED ";
		if (!m_References.size()) ostream << "VIRTUAL ";
		ostream << "{ ";
		for (uint i = 0; i < nb; ++i)
		{
			CReferenceMaker *referenceMaker = getReference(i);
			if (referenceMaker)
			{
				ostream << "\n" << padpad << i << ": <ptr=0x";
				{
					std::stringstream ss;
					ss << std::hex << std::setfill('0');
					ss << std::setw(16) << (uint64)(void *)referenceMaker;
					ostream << ss.str();
				}
				ostream << "> ";
				ostream << "(" << ucstring(referenceMaker->classDesc()->displayName()).toUtf8() << ", " << referenceMaker->classDesc()->classId().toString() << ") ";
			}
		}
		ostream << "} ";
	}
	if (m_204B_Equals_2E)
	{
		ostream << "\n" << pad << "0x204B Equals 0x2E (46): ";
		m_204B_Equals_2E->toString(ostream, pad + "\t");
	}
	if (m_Unknown2045)
	{
		ostream << "\n" << pad << "Unknown 0x2045: ";
		m_Unknown2045->toString(ostream, pad + "\t");
	}
	if (m_Unknown2047)
	{
		ostream << "\n" << pad << "Unknown 0x2047: ";
		m_Unknown2047->toString(ostream, pad + "\t");
	}
	if (m_Unknown21B0)
	{
		ostream << "\n" << pad << "Unknown 0x21B0: ";
		m_Unknown21B0->toString(ostream, pad + "\t");
	}
}

CReferenceMaker *CReferenceMaker::getReference(uint index) const
{
	if (m_References.size() <= index) return NULL;
	return m_References[index];
}

void CReferenceMaker::setReference(uint index, CReferenceMaker *reference)
{
	if (m_References.size() <= index) m_References.resize(index + 1);
	m_References[index] = reference;
}

uint CReferenceMaker::nbReferences() const
{
	return m_References.size();
}

IStorageObject *CReferenceMaker::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMB_REFERENCES_2034_CHUNK_ID:
		return new CStorageArray<sint32>();
	case PMB_REFERENCES_2035_CHUNK_ID:
		return new CStorageArray<sint32>();
	case PMB_204B_EQUALS_2E_CHUNK_ID:
		return new CStorageValue<uint8>();
	case PMB_UNKNOWN21B0_CHUNK_ID:
		return new CStorageRaw();
	}
	return CAnimatable::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
