/**
 * \file derived_object.h
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

#ifndef PIPELINE_DERIVED_OBJECT_H
#define PIPELINE_DERIVED_OBJECT_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <vector>

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CDerivedObject
 * \date 2026-07-16 16:30GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The OSM Derived object, ClassId (0x29263a68, 0x405f22f5): the wrapper that holds a node's
 * modifier stack. Together with its WSM sibling (CWSMDerivedObject, (0x4ec13906, 0x5578130e))
 * it is one of the two scene classes that are NOT resolved through ClassDirectory3: their
 * container chunk ids are the fixed 0x2032/0x2033, hardcoded in
 * CSceneClassContainer::createChunkById, which is also why their classDesc()->superClassId()
 * is 0x0: the file stores no superclass for them.
 *
 * Format (corpus-established, identical from Max 3 through Max 2010; no era fork):
 *
 * - The reference array (0x2034 flat form on every corpus wrapper; no 0x2035, no empty slots)
 *   holds the MODIFIER references first (superclass 0x810 object-space modifier or 0x820
 *   world-space modifier) in stack order bottom-up, then the BASE OBJECT as the LAST reference:
 *   the object being wrapped. The base is usually a GeomObject/Shape, a nested OSM/WSM wrapper
 *   (deep chains exist), or an XRefObject, but some corpus wrappers wrap a Helper (old Bone)
 *   or a Camera: the base slot is NOT restricted by superclass. Wrappers can also carry a base
 *   and ZERO modifiers (ligo zone sources).
 * - The wrapper's own chunk stream, canonical order: 0x2034 refs, [0x2045, 0x2047 (or 0x204b)]
 *   (claimed by CReferenceMaker), then one 0x2500 ModApp container PER MODIFIER REFERENCE
 *   (count parity is exact corpus-wide, the run is contiguous, i-th 0x2500 = i-th modifier
 *   reference; the SDK's per-node ModApp/LocalModData pairing), then one EMPTY 0x2501 leaf
 *   (always present, always last, semantics unknown).
 * - ModApp 0x2500 container children, canonical order, each at most once: 0x2510 (optional,
 *   52 bytes: the mod-context TM, 4x3 float row-major + 4 bytes flags), 0x2511 (always,
 *   24 bytes: the mod-context bounding box, 2 Point3), 0x2512 (optional: the modifier-specific
 *   LocalModData payload; a container on most, a raw LEAF on some; STAYS RAW at this level,
 *   the payload formats are per-modifier: Edit Mesh MeshDelta 0x4000, Physique 0x2504, Map
 *   Extender cache, NeL Edit Patch 0x1000, ...), 0x2513 (always, 4 bytes, unknown).
 *
 * This class keeps the raw chunks authoritative (the CParamBlock/CShapeObject overlay-codec
 * discipline): parse decodes a typed slot model over the orphaned chunks WITHOUT moving them,
 * build re-emits them verbatim, so roundtrip is byte-exact by construction. On top
 * of that it exposes the modifier slots (resolved modifier reference + paired 0x2500 ModApp),
 * the 0x2510 mod-context TM, the 0x2512 LocalModData payload object, and the base object.
 */
class CDerivedObject : public CReferenceTarget
{
public:
	/// One modifier slot: the reference-array index of the modifier, and the paired orphaned
	/// 0x2500 ModApp container (not owned; NULL when absent or not a container).
	struct SModifierSlot
	{
		uint ReferenceIndex;
		CStorageContainer *ModApp;

		SModifierSlot() : ReferenceIndex(0), ModApp(nullptr) { }
	};

	CDerivedObject(CScene *scene);
	virtual ~CDerivedObject() NL_OVERRIDE;

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const char *InternalNameUnknown;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void clean() NL_OVERRIDE;
	virtual void build(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void disown() NL_OVERRIDE;
	virtual void init() NL_OVERRIDE;
	virtual bool inherits(const NLMISC::CClassId classId) const NL_OVERRIDE;
	virtual const ISceneClassDesc *classDesc() const NL_OVERRIDE;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const NL_OVERRIDE;

	//! \name Typed read access (valid between parse and clean/disown)
	//@{
	/// Number of modifier references (superclass 0x810/0x820), in reference (stack bottom-up) order.
	inline uint modifierCount() const { return (uint)m_ModifierSlots.size(); }
	/// The modifier scene object of slot \a i (resolved reference). NULL out of range.
	CSceneClass *modifier(uint i) const;
	/// The 0x2500 ModApp container paired with modifier slot \a i. NULL when absent.
	CStorageContainer *modApp(uint i) const;
	/// The 0x2510 mod-context TM of slot \a i: copies the leading 48 bytes (4x3 float row-major)
	/// into \a tm12. Returns false (leaving \a tm12 untouched) when the slot has no 0x2510 —
	/// callers keep their identity default.
	bool modContextTM(uint i, float *tm12) const;
	/// The 0x2512 LocalModData payload of slot \a i — a CStorageContainer on most corpus
	/// instances, a raw CStorageRaw LEAF on some (the payload stays raw at this level; the
	/// per-modifier formats are decoded by their consumers). NULL when absent.
	IStorageObject *localModData(uint i) const;
	/// The base object — the last non-modifier reference (always the LAST reference slot
	/// corpus-wide): the object this wrapper applies its modifier stack to. NULL when none.
	CSceneClass *baseObject() const;
	inline bool hasBase() const { return m_HasBase; }
	//@}

	//! \name Static mod-app helpers, for consumers holding only a 0x2500 container
	//@{
	/// Read the leading 48 bytes of the container's 0x2510 leaf into \a tm12 (see modContextTM).
	static bool modAppContextTM(const CStorageContainer *modApp, float *tm12);
	/// The container's 0x2512 child (container or raw leaf), NULL when absent.
	static IStorageObject *modAppLocalModData(const CStorageContainer *modApp);
	//@}

	//! \name Structural diagnostics (for the corpus selftest)
	//@{
	/// Count of orphaned 0x2500 containers (equals modifierCount() corpus-wide).
	inline uint numModApps() const { return m_NumModApps; }
	/// False when a non-0x2500 chunk interrupts the 0x2500 run (never observed).
	inline bool modAppsContiguous() const { return m_ModAppsContiguous; }
	/// Count of non-null non-modifier references (1 corpus-wide: the base).
	inline uint nonModifierRefs() const { return m_NonModifierRefs; }
	/// True when the base is the last reference slot (corpus-invariant).
	inline bool baseIsLastReference() const { return m_BaseIsLast; }
	/// Orphan chunk ids other than the 0x2500 run and the empty 0x2501 tail (empty corpus-wide).
	inline const std::vector<uint16> &unknownOrphanIds() const { return m_UnknownOrphanIds; }
	/// Verify every corpus-established structural invariant on this wrapper: slot/mod-app count
	/// parity, contiguous 0x2500 run, single base at the last slot, no unknown orphan ids, and
	/// per-app canonical children (0x2510[52]/0x2511[24]/0x2512/0x2513[4], each at most once, in
	/// order). Returns false and fills \a err on the first violation.
	bool selfTest(std::string &err) const;
	//@}

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container) NL_OVERRIDE;

private:
	void decodeDerivedModel();

	std::vector<SModifierSlot> m_ModifierSlots;
	uint m_BaseReferenceIndex;
	bool m_HasBase;
	bool m_BaseIsLast;
	uint m_NonModifierRefs;
	uint m_NumModApps;
	bool m_ModAppsContiguous;
	std::vector<uint16> m_UnknownOrphanIds;

}; /* class CDerivedObject */

typedef CSceneClassDesc<CDerivedObject> CDerivedObjectClassDesc;
extern const CDerivedObjectClassDesc DerivedObjectClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_DERIVED_OBJECT_H */

/* end of file */
