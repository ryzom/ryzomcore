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
#define PMB_REFERENCES_2035_CHUNK_ID 0x2034
#define PMB_204B_EQUALS_2E_CHUNK_ID 0x204B

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CReferenceMaker::CReferenceMaker()
{

}

CReferenceMaker::~CReferenceMaker()
{

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
}

void CReferenceMaker::clean()
{
	CAnimatable::clean();
}

void CReferenceMaker::build(uint16 version)
{
	CAnimatable::build(version);
}

void CReferenceMaker::disown()
{
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
}

IStorageObject *CReferenceMaker::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
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
