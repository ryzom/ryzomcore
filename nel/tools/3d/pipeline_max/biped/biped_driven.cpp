/**
 * \file biped_driven.cpp
 * \brief CBipedDriven
 * \date 2026-07-05
 * \author Jan Boon (Kaetemi)
 * CBipedDriven
 */

/*
 * Copyright (C) 2026  by authors
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
#include "biped_driven.h"

// STL includes

// NeL includes

// Project includes
#include "../storage_array.h"
#include "biped.h"

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace BIPED {

using BUILTIN::CReferenceTarget;

#define PMB_BIPED_DRIVEN_IDLINK_CHUNK_ID 0x0200

CBipedDriven::CBipedDriven(CScene *scene) : CReferenceTarget(scene), m_HasIdLink(false), m_BipedBoneId(0), m_BipedLinkIndex(0)
{

}

CBipedDriven::~CBipedDriven()
{

}

const ucstring CBipedDriven::DisplayName = ucstring("BipDriven Control");
const char *CBipedDriven::InternalName = "BipedDriven";
const NLMISC::CClassId CBipedDriven::ClassId = NLMISC::CClassId(0x00009154, 0x00000000);
const TSClassId CBipedDriven::SuperClassId = 0x00009008; // ControlTransform
const CBipedDrivenClassDesc BipedDrivenClassDesc(&DllPluginDescBiped);

void CBipedDriven::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		IStorageObject *chunk = getChunk(PMB_BIPED_DRIVEN_IDLINK_CHUNK_ID);
		if (chunk)
		{
			CStorageArray<uint32> *idLink = static_cast<CStorageArray<uint32> *>(chunk);
			nlassert(idLink->Value.size() == 2);
			m_BipedBoneId = idLink->Value[0];
			m_BipedLinkIndex = idLink->Value[1];
			m_HasIdLink = true;
			m_ArchivedChunks.push_back(idLink);
		}
	}
}

void CBipedDriven::clean()
{
	CReferenceTarget::clean();
}

void CBipedDriven::build(uint16 version, uint filter)
{
	CReferenceTarget::build(version);
	if (m_HasIdLink)
	{
		CStorageArray<uint32> *idLink = new CStorageArray<uint32>();
		idLink->Value.resize(2);
		idLink->Value[0] = m_BipedBoneId;
		idLink->Value[1] = m_BipedLinkIndex;
		m_ArchivedChunks.push_back(idLink);
		putChunk(PMB_BIPED_DRIVEN_IDLINK_CHUNK_ID, idLink);
	}
}

void CBipedDriven::disown()
{
	m_HasIdLink = false;
	m_BipedBoneId = 0;
	m_BipedLinkIndex = 0;
	CReferenceTarget::disown();
}

void CBipedDriven::init()
{
	CReferenceTarget::init();
}

bool CBipedDriven::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceTarget::inherits(classId);
}

const ISceneClassDesc *CBipedDriven::classDesc() const
{
	return &BipedDrivenClassDesc;
}

void CBipedDriven::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
	if (m_HasIdLink)
	{
		ostream << "\n" << pad << "BipedBoneId: " << m_BipedBoneId;
		ostream << "\n" << pad << "BipedLinkIndex: " << m_BipedLinkIndex;
	}
}

IStorageObject *CBipedDriven::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMB_BIPED_DRIVEN_IDLINK_CHUNK_ID:
		return new CStorageArray<uint32>();
	}
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BIPED */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
