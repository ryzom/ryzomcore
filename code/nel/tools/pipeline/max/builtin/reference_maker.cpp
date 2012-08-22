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

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CReferenceMaker::CReferenceMaker()
{

}

CReferenceMaker::~CReferenceMaker()
{
	if (!m_ChunksOwnsPointers)
	{
		delete m_References2034;
		m_References2034 = NULL;
		delete m_References2035;
		m_References2035 = NULL;
		delete m_204B_Equals_2E;
		m_204B_Equals_2E = NULL;
	}
}

const ucchar *CReferenceMaker::DisplayName = ucstring("ReferenceMaker").c_str();
const char *CReferenceMaker::InternalName = "ReferenceMaker";
const char *CReferenceMaker::InternalNameUnknown = "ReferenceMakerUnknown";
const NLMISC::CClassId CReferenceMaker::ClassId = NLMISC::CClassId(0x2ec43d15, 0x10a270ad); /* Not official, please correct */
const TSClassId CReferenceMaker::SuperClassId = 0x00000100;
const CReferenceMakerClassDesc ReferenceMakerClassDesc(&DllPluginDescBuiltin);
const CReferenceMakerSuperClassDesc ReferenceMakerSuperClassDesc(&ReferenceMakerClassDesc);

void CReferenceMaker::parse(uint16 version, TParseLevel level)
{
	CAnimatable::parse(version, level);
	if (!m_ChunksOwnsPointers)
	{
		m_References2034 = static_cast<CStorageArray<sint32> *>(getChunk(PMB_REFERENCES_2034_CHUNK_ID));
		m_References2035 = static_cast<CStorageArray<sint32> *>(getChunk(PMB_REFERENCES_2035_CHUNK_ID));
		if (m_References2034) nlassert(m_References2035 == NULL); // Apparently, there can be only one.
		if (m_References2035) nlassert(m_References2034 == NULL);
		m_204B_Equals_2E = static_cast<CStorageValue<uint8> *>(getChunk(PMB_204B_EQUALS_2E_CHUNK_ID));
		if (m_204B_Equals_2E) nlassert(m_204B_Equals_2E->Value == 0x2e); // Really, let me know when it has another value.
		// TODO: Parse contents
	}
}

void CReferenceMaker::clean()
{
	CAnimatable::clean();
	// TODO: Delete unnecessary stuff
}

void CReferenceMaker::build(uint16 version)
{
	CAnimatable::build(version);
	// TODO: Build contents
	if (m_References2034) putChunk(PMB_REFERENCES_2034_CHUNK_ID, m_References2034);
	if (m_References2035) putChunk(PMB_REFERENCES_2035_CHUNK_ID, m_References2035);
	if (m_204B_Equals_2E) putChunk(PMB_204B_EQUALS_2E_CHUNK_ID, m_204B_Equals_2E);
}

void CReferenceMaker::disown()
{
	m_References2034 = NULL;
	m_References2035 = NULL;
	m_204B_Equals_2E = NULL;
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

void CReferenceMaker::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	CAnimatable::toStringLocal(ostream, pad);
	if (m_References2034)
	{
		ostream << "\n" << pad << "References 0x2034: ";
		m_References2034->toString(ostream, pad + "\t");
	}
	if (m_References2035)
	{
		ostream << "\n" << pad << "References 0x2035: ";
		m_References2035->toString(ostream, pad + "\t");
	}
	if (m_204B_Equals_2E)
	{
		ostream << "\n" << pad << "0x204B Equals 0x2E (46): ";
		m_204B_Equals_2E->toString(ostream, pad + "\t");
	}
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
