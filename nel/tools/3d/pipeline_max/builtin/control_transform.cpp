/**
 * \file control_transform.cpp
 * \brief CControlPRS, CControlLookAt
 * \date 2026-07-17
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * Typed classes for the two builtin node-transform controllers.
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
#include "control_transform.h"

// STL includes

// NeL includes

// Project includes
#include "control_keyframer.h"

using namespace std;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

// Own chunk ids on the transform controllers (corpus-inventoried 2026-07-17, §10j-dix; all
// claimed verbatim, semantics undecoded):
// - 0x7230 (4 B) + 0x7231 (4 B): on every PRS and LookAt instance, in that order.
// - 0x2535 (4 B): trailing, 5 PRS instances corpus-wide.
// - 0x0100 (4 B) + 0x0201 (1 B): LookAt only, after 0x7230/0x7231.
#define PMB_CTRLTM_UNKNOWN7230_CHUNK_ID 0x7230
#define PMB_CTRLTM_UNKNOWN7231_CHUNK_ID 0x7231
#define PMB_CTRLTM_PRS_UNKNOWN2535_CHUNK_ID 0x2535
#define PMB_CTRLTM_LOOKAT_UNKNOWN0100_CHUNK_ID 0x0100
#define PMB_CTRLTM_LOOKAT_UNKNOWN0201_CHUNK_ID 0x0201

////////////////////////////////////////////////////////////////////////
// CControlTransformBase
////////////////////////////////////////////////////////////////////////

CControlTransformBase::CControlTransformBase(CScene *scene) : CReferenceTarget(scene)
{

}

CControlTransformBase::~CControlTransformBase()
{
	if (!m_ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
			delete it->second;
		m_Claimed.clear();
	}
}

void CControlTransformBase::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		// Claim known chunks off the head of the orphan list, in file order, stopping at the
		// first unrecognized id (the CControlKeyFramerBase discipline) — anything after simply
		// stays orphaned, verbatim pass-through.
		for (;;)
		{
			uint16 id = peekChunk();
			if (id == 0x0000) break;
			if (!isKnownChunkId(id)) break;
			IStorageObject *so = getChunk(id);
			if (!so) break;
			m_Claimed.push_back(TStorageObjectWithId(id, so));
		}
	}
}

void CControlTransformBase::clean()
{
	CReferenceTarget::clean();
}

void CControlTransformBase::build(uint16 version, uint filter)
{
	CReferenceTarget::build(version);
	for (TStorageObjectContainer::iterator it = m_Claimed.begin(), end = m_Claimed.end(); it != end; ++it)
		putChunk(it->first, it->second);
}

void CControlTransformBase::disown()
{
	m_Claimed.clear();
	CReferenceTarget::disown();
}

void CControlTransformBase::init()
{
	CReferenceTarget::init();
}

void CControlTransformBase::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
}

bool CControlTransformBase::slotPosValueAt0(uint slot, float out[3]) const
{
	const CControlKeyFramerBase *kf = dynamic_cast<const CControlKeyFramerBase *>(getReference(slot));
	return kf && kf->posValueAt0(out);
}

bool CControlTransformBase::slotRotValueAt0(uint slot, float out[4]) const
{
	const CControlKeyFramerBase *kf = dynamic_cast<const CControlKeyFramerBase *>(getReference(slot));
	return kf && kf->rotValueAt0(out);
}

bool CControlTransformBase::slotScaleValueAt0(uint slot, float out[7]) const
{
	const CControlKeyFramerBase *kf = dynamic_cast<const CControlKeyFramerBase *>(getReference(slot));
	return kf && kf->scaleValueAt0(out);
}

bool CControlTransformBase::slotFloatValueAt0(uint slot, float &out) const
{
	const CControlKeyFramerBase *kf = dynamic_cast<const CControlKeyFramerBase *>(getReference(slot));
	return kf && kf->floatValueAt0(out);
}

IStorageObject *CControlTransformBase::createChunkById(uint16 id, bool container)
{
	// All own chunks stay raw (claimed verbatim, semantics undecoded).
	return CReferenceTarget::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
// CControlPRS
////////////////////////////////////////////////////////////////////////

CControlPRS::CControlPRS(CScene *scene) : CControlTransformBase(scene)
{

}

CControlPRS::~CControlPRS()
{

}

const ucstring CControlPRS::DisplayName = ucstring("Position/Rotation/Scale");
const char *CControlPRS::InternalName = "ControlPRS";
const NLMISC::CClassId CControlPRS::ClassId = NLMISC::CClassId(0x00002005, 0x00000000);
const TSClassId CControlPRS::SuperClassId = 0x00009008; // ControlTransform
const CControlPRSClassDesc ControlPRSClassDesc(&DllPluginDescBuiltin);

bool CControlPRS::isKnownChunkId(uint16 id) const
{
	switch (id)
	{
	case PMB_CTRLTM_UNKNOWN7230_CHUNK_ID:
	case PMB_CTRLTM_UNKNOWN7231_CHUNK_ID:
	case PMB_CTRLTM_PRS_UNKNOWN2535_CHUNK_ID:
		return true;
	}
	return false;
}

bool CControlPRS::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CControlTransformBase::inherits(classId);
}

const ISceneClassDesc *CControlPRS::classDesc() const
{
	return &ControlPRSClassDesc;
}

////////////////////////////////////////////////////////////////////////
// CControlLookAt
////////////////////////////////////////////////////////////////////////

CControlLookAt::CControlLookAt(CScene *scene) : CControlTransformBase(scene)
{

}

CControlLookAt::~CControlLookAt()
{

}

const ucstring CControlLookAt::DisplayName = ucstring("Look At");
const char *CControlLookAt::InternalName = "ControlLookAt";
const NLMISC::CClassId CControlLookAt::ClassId = NLMISC::CClassId(0x00002006, 0x00000000);
const TSClassId CControlLookAt::SuperClassId = 0x00009008; // ControlTransform
const CControlLookAtClassDesc ControlLookAtClassDesc(&DllPluginDescBuiltin);

bool CControlLookAt::isKnownChunkId(uint16 id) const
{
	switch (id)
	{
	case PMB_CTRLTM_UNKNOWN7230_CHUNK_ID:
	case PMB_CTRLTM_UNKNOWN7231_CHUNK_ID:
	case PMB_CTRLTM_LOOKAT_UNKNOWN0100_CHUNK_ID:
	case PMB_CTRLTM_LOOKAT_UNKNOWN0201_CHUNK_ID:
		return true;
	}
	return false;
}

bool CControlLookAt::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CControlTransformBase::inherits(classId);
}

const ISceneClassDesc *CControlLookAt::classDesc() const
{
	return &ControlLookAtClassDesc;
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
