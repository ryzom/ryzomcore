/**
 * \file control_transform.h
 * \brief CControlPRS, CControlLookAt
 * \date 2026-07-17
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * Typed classes for the two builtin node-transform controllers (superclass 0x9008
 * ControlTransform): PRS (0x2005) and LookAt (0x2006).
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

#ifndef PIPELINE_CONTROL_TRANSFORM_H
#define PIPELINE_CONTROL_TRANSFORM_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CControlTransformBase
 * \date 2026-07-17
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * Shared machinery for the typed node-transform controllers: claims the controller's known own
 * chunks head-first in file order (after CReferenceTarget's reference wiring), stopping at the
 * first unrecognized id so unknown chunks stay orphaned pass-through; all claimed chunks are
 * re-emitted verbatim in original order by build() — the CControlKeyFramerBase discipline. The
 * raw bytes stay authoritative (no authoring direction), so roundtrip is byte-exact by
 * construction.
 *
 * Corpus facts (2026-07-17, full-corpus inventory incl. Max 3 snowballs — design doc §10j-dix):
 * PRS carries 0x7230 (4 B) + 0x7231 (4 B) on every instance, 5 instances add a trailing 0x2535
 * (4 B); LookAt carries 0x7230 + 0x7231 then 0x0100 (4 B) + 0x0201 (1 B). Semantics of all of
 * these are unknown — they are claimed verbatim, not decoded.
 */
class CControlTransformBase : public CReferenceTarget
{
public:
	CControlTransformBase(CScene *scene);
	virtual ~CControlTransformBase();

	// inherited
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();
	virtual void init();
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const;

	//! \name Sub-controller value at t=0
	//! Resolve a reference slot to a typed keyframer and evaluate at tick 0
	//! (CControlKeyFramerBase::{pos,rot,scale,float}ValueAt0 — key table bracketed at tick 0,
	//! else the default-value chunk). Returns false when the slot is empty or its controller is
	//! not a typed keyframer — the corpus-wide inventory (§10j-dix) established that NO
	//! non-keyframer sub-controller carries a default-value chunk (0x2501/0x2503/0x2504/0x2505),
	//! so there is deliberately no raw-chunk fallback here.
	//@{
	bool slotPosValueAt0(uint slot, float out[3]) const;
	bool slotRotValueAt0(uint slot, float out[4]) const;
	bool slotScaleValueAt0(uint slot, float out[7]) const;
	bool slotFloatValueAt0(uint slot, float &out) const;
	//@}

protected:
	/// Own chunk ids this controller class claims (claim loop stops at the first id not in the
	/// set — anything after stays orphaned verbatim).
	virtual bool isKnownChunkId(uint16 id) const = 0;

	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	/// Chunks claimed off the orphan list head, in original file order; re-emitted verbatim.
	TStorageObjectContainer m_Claimed;

}; /* class CControlTransformBase */

/**
 * \brief CControlPRS
 * \date 2026-07-17
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * "Position/Rotation/Scale" (0x2005, superclass 0x9008) — the default node TM controller.
 * Reference slots (corpus-wide: always 3): 0 = position, 1 = rotation, 2 = scale
 * sub-controller. Rotation controller values are stored in the INVERSE convention relative to
 * the node-TM rotation (see max_scene.h / design doc §10 "PRS-path defects") — that convention
 * belongs to the consumer's math, not to this class.
 */
class CControlPRS : public CControlTransformBase
{
public:
	CControlPRS(CScene *scene);
	virtual ~CControlPRS();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual bool inherits(const NLMISC::CClassId classId) const;
	virtual const ISceneClassDesc *classDesc() const;

	//! \name Sub-controller slots (refs 0/1/2)
	//@{
	CReferenceMaker *positionController() const { return getReference(0); }
	CReferenceMaker *rotationController() const { return getReference(1); }
	CReferenceMaker *scaleController() const { return getReference(2); }
	//@}

	//! \name Values at t=0 (typed keyframer eval; false when a slot is not a typed keyframer)
	//@{
	bool posValueAt0(float out[3]) const { return slotPosValueAt0(0, out); }
	bool rotValueAt0(float out[4]) const { return slotRotValueAt0(1, out); }
	bool scaleValueAt0(float out[7]) const { return slotScaleValueAt0(2, out); }
	//@}

protected:
	virtual bool isKnownChunkId(uint16 id) const;

}; /* class CControlPRS */

typedef CSceneClassDesc<CControlPRS> CControlPRSClassDesc;
extern const CControlPRSClassDesc ControlPRSClassDesc;

/**
 * \brief CControlLookAt
 * \date 2026-07-17
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * "Look At" (0x2006, superclass 0x9008) — the target-following node TM controller (target
 * lights/cameras). Reference slots (corpus-wide: always 4): 0 = target NODE, 1 = position,
 * 2 = roll (float), 3 = scale sub-controller. The rotation is computed from the target at
 * evaluation time (the SDK's GetRotationController is NULL) — there is no rotation slot.
 */
class CControlLookAt : public CControlTransformBase
{
public:
	CControlLookAt(CScene *scene);
	virtual ~CControlLookAt();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual bool inherits(const NLMISC::CClassId classId) const;
	virtual const ISceneClassDesc *classDesc() const;

	//! \name Sub-controller slots (refs 0/1/2/3)
	//@{
	/// The target node (dynamic_cast to INode at the consumer; always a Node in the corpus).
	CReferenceMaker *targetNode() const { return getReference(0); }
	CReferenceMaker *positionController() const { return getReference(1); }
	CReferenceMaker *rollController() const { return getReference(2); }
	CReferenceMaker *scaleController() const { return getReference(3); }
	//@}

	//! \name Values at t=0 (typed keyframer eval; false when a slot is not a typed keyframer)
	//@{
	bool posValueAt0(float out[3]) const { return slotPosValueAt0(1, out); }
	bool rollValueAt0(float &out) const { return slotFloatValueAt0(2, out); }
	bool scaleValueAt0(float out[7]) const { return slotScaleValueAt0(3, out); }
	//@}

protected:
	virtual bool isKnownChunkId(uint16 id) const;

}; /* class CControlLookAt */

typedef CSceneClassDesc<CControlLookAt> CControlLookAtClassDesc;
extern const CControlLookAtClassDesc ControlLookAtClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_CONTROL_TRANSFORM_H */

/* end of file */
