/**
 * \file mtl_base.cpp
 * \brief CMtlBase
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * CMtlBase
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
#include "mtl_base.h"

// STL includes
#include <cstring>

// NeL includes
#include <nel/misc/ucstring.h>

// Project includes

using namespace std;

#define PMB_MTL_BASE_CHUNK_ID 0x4000
#define PMB_MTL_NAME_CHUNK_ID 0x4001

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CMtlBase::CMtlBase(CScene *scene) : CReferenceTarget(scene), m_NameChunk(NULL)
{

}

CMtlBase::~CMtlBase()
{

}

const ucstring CMtlBase::DisplayName = ucstring("MtlBase");
const char *CMtlBase::InternalName = "MtlBase";
const char *CMtlBase::InternalNameUnknown = "MtlBaseUnknown";
const NLMISC::CClassId CMtlBase::ClassId = NLMISC::CClassId(0x0000c000, 0x00000000); /* Not official, please correct */
const TSClassId CMtlBase::SuperClassId = 0x00000c00;
const CMtlBaseClassDesc MtlBaseClassDesc(&DllPluginDescBuiltin);

void CMtlBase::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
	if (!m_ChunksOwnsPointers)
		decodeName();
}

void CMtlBase::clean()
{
	CReferenceTarget::clean();
}

void CMtlBase::build(uint16 version, uint filter)
{
	// Raw chunks stay authoritative — verbatim re-emit, byte-exact roundtrip.
	CReferenceTarget::build(version);
}

void CMtlBase::disown()
{
	m_NameChunk = NULL;
	CReferenceTarget::disown();
}

void CMtlBase::init()
{
	CReferenceTarget::init();
}

bool CMtlBase::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceTarget::inherits(classId);
}

const ISceneClassDesc *CMtlBase::classDesc() const
{
	return &MtlBaseClassDesc;
}

// Locate the name chunk without moving anything: a bare 0x4001 leaf on the object, or the 0x4001
// child of the 0x4000 material-base container.
void CMtlBase::decodeName()
{
	m_NameChunk = NULL;
	const TStorageObjectContainer &orphans = orphanedChunks();
	for (TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first == PMB_MTL_NAME_CHUNK_ID)
		{
			m_NameChunk = dynamic_cast<CStorageRaw *>(it->second);
			if (m_NameChunk) return;
		}
	}
	for (TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first != PMB_MTL_BASE_CHUNK_ID) continue;
		CStorageContainer *base = dynamic_cast<CStorageContainer *>(it->second);
		if (!base) continue;
		IStorageObject *so = base->findStorageObject(PMB_MTL_NAME_CHUNK_ID);
		m_NameChunk = dynamic_cast<CStorageRaw *>(so);
		if (m_NameChunk) return;
	}
}

std::string CMtlBase::name() const
{
	// The name chunk is raw UTF-16 sized by the chunk (no length prefix). Returned verbatim
	// (trailing bytes as stored) — a consumer that wants a display-clean name strips its own
	// trailing nulls; the exporter relies on the verbatim form for byte-exact animated-material
	// names.
	if (!m_NameChunk || m_NameChunk->Value.empty()) return std::string();
	ucstring us;
	us.resize(m_NameChunk->Value.size() / 2);
	memcpy(&us[0], nlVectorData(m_NameChunk->Value), us.size() * 2);
	return us.toUtf8();
}

void CMtlBase::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
	if (m_NameChunk)
		ostream << "\n" << pad << "MtlBase name: " << name();
}

IStorageObject *CMtlBase::createChunkById(uint16 id, bool container)
{
	// The 0x4000 base is a container, 0x4001 a raw leaf — both default correctly; raw stays
	// authoritative.
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
