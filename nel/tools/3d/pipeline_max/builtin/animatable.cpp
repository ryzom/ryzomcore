/**
 * \file animatable.cpp
 * \brief CAnimatable
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * CAnimatable
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
#include "animatable.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "storage/app_data.h"

using namespace std;
// using namespace NLMISC;

#define PBM_ANIMATABLE_UNKNOWN2140_CHUNK_ID 0x2140

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CAnimatable::CAnimatable(CScene *scene) : CSceneClass(scene), m_AppData(NULL)
{

}

CAnimatable::~CAnimatable()
{
	if (!m_ChunksOwnsPointers)
	{
		delete m_Unknown2140;
		m_Unknown2140 = NULL;
		delete m_AppData;
		m_AppData = NULL;
	}
}

const ucstring CAnimatable::DisplayName = ucstring("Animatable");
const char *CAnimatable::InternalName = "Animatable";
const char *CAnimatable::InternalNameUnknown = "AnimatableUnknown";
const NLMISC::CClassId CAnimatable::ClassId = NLMISC::CClassId(0x3101497b, 0x24af711b); /* Not official, please correct */
const TSClassId CAnimatable::SuperClassId = 0x77a60fbd; /* Not official, please correct */
const CAnimatableClassDesc AnimatableClassDesc(&DllPluginDescBuiltin);
const CAnimatableSuperClassDesc AnimatableSuperClassDesc(&AnimatableClassDesc);

void CAnimatable::parse(uint16 version, uint filter)
{
	CSceneClass::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		m_Unknown2140 = getChunk(PBM_ANIMATABLE_UNKNOWN2140_CHUNK_ID);
		if (m_Unknown2140)
		{
			// nldebug("Found unknown 0x2140");
			// TODO: Put std::cout code here
		}
		m_AppData = static_cast<STORAGE::CAppData *>(getChunk(PMBS_APP_DATA_CHUNK_ID));
	}
}

void CAnimatable::clean()
{
	CSceneClass::clean();
	if (m_AppData) m_AppData->clean();
}

void CAnimatable::build(uint16 version, uint filter)
{
	CSceneClass::build(version);
	if (m_Unknown2140)
	{
		putChunk(PBM_ANIMATABLE_UNKNOWN2140_CHUNK_ID, m_Unknown2140);
	}
	if (m_AppData)
	{
		if (m_AppData->entries().size() == 0)
		{
			// Discard appdata if it has no entries
			delete m_AppData;
			m_AppData = NULL;
		}
		else
		{
			putChunk(PMBS_APP_DATA_CHUNK_ID, m_AppData);
		}
	}
}

void CAnimatable::disown()
{
	m_Unknown2140 = NULL;
	m_AppData = NULL;
	CSceneClass::disown();
}

void CAnimatable::init()
{
	CSceneClass::init();
}

bool CAnimatable::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CSceneClass::inherits(classId);
}

const ISceneClassDesc *CAnimatable::classDesc() const
{
	return &AnimatableClassDesc;
}

void CAnimatable::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CSceneClass::toStringLocal(ostream, pad);
	if (m_AppData && m_AppData->entries().size() != 0)
	{
		ostream << "\n" << pad << "AppData: ";
		m_AppData->toString(ostream, pad + "\t");
	}
}

STORAGE::CAppData *CAnimatable::appData()
{
	if (m_ChunksOwnsPointers) { nlerror("Not parsed"); return NULL; }
	if (!m_AppData)
	{
		m_AppData = new STORAGE::CAppData();
		m_AppData->init();
	}
	return m_AppData;
}

IStorageObject *CAnimatable::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMBS_APP_DATA_CHUNK_ID:
		return new STORAGE::CAppData;
	}
	return CSceneClass::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
