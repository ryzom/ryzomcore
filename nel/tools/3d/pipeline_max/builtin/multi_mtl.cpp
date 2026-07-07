/**
 * \file multi_mtl.cpp
 * \brief CMultiMtl
 * \date 2012-08-22 08:55GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * CMultiMtl
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
#include "multi_mtl.h"

// STL includes
#include <cstring>

// NeL includes

// Project includes

using namespace std;

#define PMB_MULTI_SUBCOUNT_CHUNK_ID 0x4002

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CMultiMtl::CMultiMtl(CScene *scene) : CMtlBase(scene), m_NumSubMaterials(0)
{

}

CMultiMtl::~CMultiMtl()
{

}

const ucstring CMultiMtl::DisplayName = ucstring("MultiMtl");
const char *CMultiMtl::InternalName = "MultiMtl";
const NLMISC::CClassId CMultiMtl::ClassId = NLMISC::CClassId(0x00000200, 0x00000000);
const TSClassId CMultiMtl::SuperClassId = 0x00000c00;
const CMultiMtlClassDesc MultiMtlClassDesc(&DllPluginDescBuiltin);

void CMultiMtl::parse(uint16 version, uint filter)
{
	CMtlBase::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		m_NumSubMaterials = 0;
		const TStorageObjectContainer &orphans = orphanedChunks();
		for (TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		{
			if (it->first != PMB_MULTI_SUBCOUNT_CHUNK_ID) continue;
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
			if (raw && raw->Value.size() >= 4)
			{
				uint32 n;
				memcpy(&n, raw->Value.data(), 4);
				m_NumSubMaterials = n;
			}
			break;
		}
	}
}

void CMultiMtl::clean()
{
	CMtlBase::clean();
}

void CMultiMtl::build(uint16 version, uint filter)
{
	CMtlBase::build(version);
}

void CMultiMtl::disown()
{
	m_NumSubMaterials = 0;
	CMtlBase::disown();
}

void CMultiMtl::init()
{
	CMtlBase::init();
}

bool CMultiMtl::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CMtlBase::inherits(classId);
}

const ISceneClassDesc *CMultiMtl::classDesc() const
{
	return &MultiMtlClassDesc;
}

CMtlBase *CMultiMtl::subMaterial(uint i) const
{
	if (i >= m_NumSubMaterials) return NULL;
	// Sub-materials are references 1..N (reference 0 is the material's own ParamBlock2).
	CReferenceMaker *r = getReference(i + 1);
	return dynamic_cast<CMtlBase *>(r);
}

void CMultiMtl::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CMtlBase::toStringLocal(ostream, pad);
	ostream << "\n" << pad << "MultiMtl: " << m_NumSubMaterials << " sub-materials";
}

IStorageObject *CMultiMtl::createChunkById(uint16 id, bool container)
{
	return CMtlBase::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
