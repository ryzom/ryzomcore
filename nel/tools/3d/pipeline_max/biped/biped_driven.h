/**
 * \file biped_driven.h
 * \brief CBipedDriven
 * \date 2026-07-05
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
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

#ifndef PIPELINE_BIPED_DRIVEN_H
#define PIPELINE_BIPED_DRIVEN_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../builtin/reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BIPED {

/**
 * \brief CBipedDriven
 * \date 2026-07-05
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 * "BipSlave Control" (pre-2022) / "BipDriven Control" (2022+), ClassId {0x9154, 0}, superclass
 * 0x9008 (ControlTransform). Every non-COM biped bone has one of these as its TM controller
 * (getReference(0)). Chunk 0x0200 (8 bytes = 2 uint32) identifies which biped bone this drives:
 * the plugin's internal (bone_id, link_index) pair, 0-based internally (see pipeline_max_design.md
 * for the confirmed id table). This is currently the only sub-chunk typed here; everything else
 * on a BipDriven Control (e.g. chunk 0x0201) stays orphaned pass-through.
 */
class CBipedDriven : public BUILTIN::CReferenceTarget
{
public:
	CBipedDriven(CScene *scene);
	virtual ~CBipedDriven();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();
	virtual void init();
	virtual bool inherits(const NLMISC::CClassId classId) const;
	virtual const ISceneClassDesc *classDesc() const;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const;

	// read access
	/// True if this instance carries the (bone_id, link_index) chunk (always true in the observed
	/// corpus; guarded defensively in case a future file lacks it).
	inline bool hasBipedIdLink() const { return m_HasIdLink; }
	/// Internal bone-id enum value (0-based; see EBipedBoneId in pipeline_max_export_skel).
	inline uint32 bipedBoneId() const { return m_BipedBoneId; }
	/// Segment index within the bone's chain (e.g. which finger/spine/toe link).
	inline uint32 bipedLinkIndex() const { return m_BipedLinkIndex; }

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	bool m_HasIdLink;
	uint32 m_BipedBoneId;
	uint32 m_BipedLinkIndex;

}; /* class CBipedDriven */

typedef CSceneClassDesc<CBipedDriven> CBipedDrivenClassDesc;
extern const CBipedDrivenClassDesc BipedDrivenClassDesc;

} /* namespace BIPED */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_BIPED_DRIVEN_H */

/* end of file */
