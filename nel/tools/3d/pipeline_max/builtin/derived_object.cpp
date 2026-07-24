/**
 * \file derived_object.cpp
 * \brief CDerivedObject
 * \date 2026-07-16 16:30GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CDerivedObject
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
#include "derived_object.h"

// STL includes
#include <cstring>
#include <sstream>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

// OSM/WSM Derived wrapper chunk stream (corpus-established; see derived_object.h).
// Wrapper-level orphans: the contiguous 0x2500 ModApp run (one per modifier reference, in
// order), then one empty 0x2501 leaf.
#define PMB_DERIVED_MOD_APP_CHUNK_ID 0x2500
#define PMB_DERIVED_EMPTY_TAIL_CHUNK_ID 0x2501
// ModApp 0x2500 container children, canonical order, each at most once.
#define PMB_MOD_APP_CONTEXT_TM_CHUNK_ID 0x2510
#define PMB_MOD_APP_CONTEXT_BBOX_CHUNK_ID 0x2511
#define PMB_MOD_APP_LOCAL_DATA_CHUNK_ID 0x2512
#define PMB_MOD_APP_UNKNOWN2513_CHUNK_ID 0x2513
// The 0x2510 leaf: 4x3 float row-major TM (48 bytes) + 4 bytes flags.
#define PMB_MOD_APP_CONTEXT_TM_BYTES 52
#define PMB_MOD_APP_CONTEXT_TM_MATRIX_BYTES 48
#define PMB_MOD_APP_CONTEXT_BBOX_BYTES 24
#define PMB_MOD_APP_UNKNOWN2513_BYTES 4
// Modifier superclasses (the discriminator for the wrapper's modifier references).
#define PMB_SCLASS_OSMODIFIER 0x00000810
#define PMB_SCLASS_WSMODIFIER 0x00000820

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CDerivedObject::CDerivedObject(CScene *scene) : CReferenceTarget(scene),
	m_BaseReferenceIndex(0), m_HasBase(false), m_BaseIsLast(false),
	m_NonModifierRefs(0), m_NumModApps(0), m_ModAppsContiguous(true)
{

}

CDerivedObject::~CDerivedObject()
{

}

const ucstring CDerivedObject::DisplayName = ucstring("OSM Derived");
const char *CDerivedObject::InternalName = "DerivedObject";
const char *CDerivedObject::InternalNameUnknown = "DerivedObjectUnknown";
const NLMISC::CClassId CDerivedObject::ClassId = NLMISC::CClassId(0x29263a68, 0x405f22f5);
// The file stores NO superclass for the derived-object wrappers; they resolve through the
// hardcoded chunk ids 0x2032/0x2033, not through ClassDirectory3. 0x0 mirrors what every
// consumer has always observed through the unknown-class path.
const TSClassId CDerivedObject::SuperClassId = 0x00000000;
const CDerivedObjectClassDesc DerivedObjectClassDesc(&DllPluginDescBuiltin);

void CDerivedObject::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
	if (!m_ChunksOwnsPointers)
		decodeDerivedModel();
}

void CDerivedObject::clean()
{
	CReferenceTarget::clean();
}

void CDerivedObject::build(uint16 version, uint filter)
{
	// Raw chunks stay authoritative: the base re-emits every orphaned chunk verbatim, so
	// roundtrip is byte-exact by construction.
	CReferenceTarget::build(version);
}

void CDerivedObject::disown()
{
	m_ModifierSlots.clear();
	m_BaseReferenceIndex = 0;
	m_HasBase = false;
	m_BaseIsLast = false;
	m_NonModifierRefs = 0;
	m_NumModApps = 0;
	m_ModAppsContiguous = true;
	m_UnknownOrphanIds.clear();
	CReferenceTarget::disown();
}

void CDerivedObject::init()
{
	CReferenceTarget::init();
}

bool CDerivedObject::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceTarget::inherits(classId);
}

const ISceneClassDesc *CDerivedObject::classDesc() const
{
	return &DerivedObjectClassDesc;
}

// Decode the modifier-slot model over the orphaned chunks WITHOUT moving them (the raw chunks
// remain the serialization authority): classify the reference slots, then pair the contiguous
// 0x2500 ModApp run with the modifier slots in order.
void CDerivedObject::decodeDerivedModel()
{
	m_ModifierSlots.clear();
	m_BaseReferenceIndex = 0;
	m_HasBase = false;
	m_BaseIsLast = false;
	m_NonModifierRefs = 0;
	m_NumModApps = 0;
	m_ModAppsContiguous = true;
	m_UnknownOrphanIds.clear();

	// Reference slots: modifiers (superclass 0x810/0x820) in slot order; the base object is the
	// last non-modifier reference (the last slot on every corpus wrapper — the base slot's own
	// superclass is NOT restricted: GeomObject/Shape usually, nested OSM/WSM, XRef, but also
	// Helper/Camera).
	uint nb = nbReferences();
	for (uint i = 0; i < nb; ++i)
	{
		CReferenceMaker *r = getReference(i);
		if (!r) continue;
		TSClassId scid = r->classDesc()->superClassId();
		if (scid == PMB_SCLASS_OSMODIFIER || scid == PMB_SCLASS_WSMODIFIER)
		{
			SModifierSlot slot;
			slot.ReferenceIndex = i;
			m_ModifierSlots.push_back(slot);
		}
		else
		{
			++m_NonModifierRefs;
			m_BaseReferenceIndex = i;
			m_HasBase = true;
		}
	}
	m_BaseIsLast = m_HasBase && (m_BaseReferenceIndex + 1 == nb);

	// The orphaned 0x2500 run: i-th 0x2500 pairs with the i-th modifier reference (exact count
	// parity corpus-wide; extra apps are counted, missing ones leave the slot's ModApp NULL).
	uint slot = 0;
	bool inRun = false, runEnded = false;
	const TStorageObjectContainer &orphans = orphanedChunks();
	for (TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first == PMB_DERIVED_MOD_APP_CHUNK_ID)
		{
			if (runEnded) m_ModAppsContiguous = false;
			inRun = true;
			++m_NumModApps;
			if (slot < m_ModifierSlots.size())
				m_ModifierSlots[slot].ModApp = dynamic_cast<CStorageContainer *>(it->second);
			++slot;
		}
		else
		{
			if (inRun) runEnded = true;
			if (it->first != PMB_DERIVED_EMPTY_TAIL_CHUNK_ID)
			{
				bool known = false;
				for (std::vector<uint16>::const_iterator ut = m_UnknownOrphanIds.begin(); ut != m_UnknownOrphanIds.end(); ++ut)
					if (*ut == it->first) { known = true; break; }
				if (!known) m_UnknownOrphanIds.push_back(it->first);
			}
		}
	}
}

CSceneClass *CDerivedObject::modifier(uint i) const
{
	if (i >= m_ModifierSlots.size()) return NULL;
	return static_cast<CSceneClass *>(getReference(m_ModifierSlots[i].ReferenceIndex));
}

CStorageContainer *CDerivedObject::modApp(uint i) const
{
	if (i >= m_ModifierSlots.size()) return NULL;
	return m_ModifierSlots[i].ModApp;
}

bool CDerivedObject::modContextTM(uint i, float *tm12) const
{
	return modAppContextTM(modApp(i), tm12);
}

IStorageObject *CDerivedObject::localModData(uint i) const
{
	return modAppLocalModData(modApp(i));
}

CSceneClass *CDerivedObject::baseObject() const
{
	if (!m_HasBase) return NULL;
	return static_cast<CSceneClass *>(getReference(m_BaseReferenceIndex));
}

bool CDerivedObject::modAppContextTM(const CStorageContainer *modApp, float *tm12)
{
	if (!modApp) return false;
	const CStorageContainer::TStorageObjectContainer &chunks = modApp->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
	{
		if (it->first != PMB_MOD_APP_CONTEXT_TM_CHUNK_ID) continue;
		const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
		if (raw && raw->Value.size() >= PMB_MOD_APP_CONTEXT_TM_MATRIX_BYTES)
		{
			memcpy(tm12, nlVectorData(raw->Value), PMB_MOD_APP_CONTEXT_TM_MATRIX_BYTES);
			return true;
		}
		return false;
	}
	return false;
}

IStorageObject *CDerivedObject::modAppLocalModData(const CStorageContainer *modApp)
{
	if (!modApp) return NULL;
	const CStorageContainer::TStorageObjectContainer &chunks = modApp->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
	{
		if (it->first == PMB_MOD_APP_LOCAL_DATA_CHUNK_ID)
			return it->second;
	}
	return NULL;
}

bool CDerivedObject::selfTest(std::string &err) const
{
	if (modifierCount() != m_NumModApps)
	{
		std::stringstream ss;
		ss << "slot/mod-app parity: " << modifierCount() << " modifier refs, " << m_NumModApps << " 0x2500 containers";
		err = ss.str();
		return false;
	}
	if (!m_ModAppsContiguous)
	{
		err = "0x2500 run is not contiguous";
		return false;
	}
	if (m_NonModifierRefs > 1)
	{
		std::stringstream ss;
		ss << m_NonModifierRefs << " non-modifier references (expected the single base)";
		err = ss.str();
		return false;
	}
	if (nbReferences() > 0 && !m_HasBase)
	{
		err = "no base object reference";
		return false;
	}
	if (m_HasBase && !m_BaseIsLast)
	{
		err = "base object is not the last reference slot";
		return false;
	}
	if (!m_UnknownOrphanIds.empty())
	{
		std::stringstream ss;
		ss << "unknown orphan chunk id 0x" << std::hex << m_UnknownOrphanIds[0];
		err = ss.str();
		return false;
	}
	for (uint i = 0; i < m_ModifierSlots.size(); ++i)
	{
		const CStorageContainer *app = m_ModifierSlots[i].ModApp;
		if (!app)
		{
			std::stringstream ss;
			ss << "mod-app slot " << i << " is not a container";
			err = ss.str();
			return false;
		}
		// Canonical children: subset of 0x2510(52)/0x2511(24)/0x2512/0x2513(4), each at most
		// once, in order.
		static const uint16 canonicalIds[4] = {
			PMB_MOD_APP_CONTEXT_TM_CHUNK_ID, PMB_MOD_APP_CONTEXT_BBOX_CHUNK_ID,
			PMB_MOD_APP_LOCAL_DATA_CHUNK_ID, PMB_MOD_APP_UNKNOWN2513_CHUNK_ID
		};
		uint pos = 0;
		const CStorageContainer::TStorageObjectContainer &chunks = app->chunks();
		for (CStorageContainer::TStorageObjectConstIt it = chunks.begin(); it != chunks.end(); ++it)
		{
			while (pos < 4 && canonicalIds[pos] != it->first) ++pos;
			if (pos >= 4)
			{
				std::stringstream ss;
				ss << "mod-app slot " << i << " has unexpected/out-of-order child 0x" << std::hex << it->first;
				err = ss.str();
				return false;
			}
			++pos; // each id at most once
			const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
			size_t expect = 0;
			switch (it->first)
			{
			case PMB_MOD_APP_CONTEXT_TM_CHUNK_ID: expect = PMB_MOD_APP_CONTEXT_TM_BYTES; break;
			case PMB_MOD_APP_CONTEXT_BBOX_CHUNK_ID: expect = PMB_MOD_APP_CONTEXT_BBOX_BYTES; break;
			case PMB_MOD_APP_UNKNOWN2513_CHUNK_ID: expect = PMB_MOD_APP_UNKNOWN2513_BYTES; break;
			default: continue; // 0x2512: container or raw leaf, any size
			}
			if (!raw || raw->Value.size() != expect)
			{
				std::stringstream ss;
				ss << "mod-app slot " << i << " child 0x" << std::hex << it->first
				   << std::dec << " is not a " << (uint)expect << "-byte leaf";
				err = ss.str();
				return false;
			}
		}
	}
	return true;
}

void CDerivedObject::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
	if (!m_ModifierSlots.empty() || m_HasBase)
	{
		uint nApps = 0, nTM = 0, nData = 0;
		for (uint i = 0; i < m_ModifierSlots.size(); ++i)
		{
			if (m_ModifierSlots[i].ModApp) ++nApps;
			float tm[12];
			if (modContextTM(i, tm)) ++nTM;
			if (localModData(i)) ++nData;
		}
		ostream << "\n" << pad << "DerivedObject: " << m_ModifierSlots.size() << " modifiers, "
			<< nApps << " mod-apps (" << nTM << " ctx TM, " << nData << " local data), base "
			<< (m_HasBase ? "slot " : "none");
		if (m_HasBase) ostream << m_BaseReferenceIndex;
	}
}

IStorageObject *CDerivedObject::createChunkById(uint16 id, bool container)
{
	// The 0x2500 tree defaults to CStorageContainer for containers and CStorageRaw for every
	// leaf already; the raw bytes stay authoritative.
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
