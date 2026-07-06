/**
 * \file rkl_patch_object.h
 * \brief CRklPatchObject
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * CRklPatchObject
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

#ifndef PIPELINE_RKL_PATCH_OBJECT_H
#define PIPELINE_RKL_PATCH_OBJECT_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../builtin/patch_object.h"
#include "rpo_data.h"

namespace PIPELINE {
namespace MAX {
namespace NELPATCH {

/**
 * \brief CRklPatchObject
 * \date 2026-07-06
 * \author Jan Boon (Kaetemi)
 * The Rykol Patch Object ("RklPatch", nelconvertpatch_r.dlm), ClassId (0x368c679f, 0x711c22ee),
 * superclass 0x10 (GeomObject) — the NeL landscape patch grid inside .max files. The original
 * RPO::Save writes chunk 0x08FD (uint32 rpoVersion + the RPatchMesh blob), then the Max
 * PatchMesh stream, then the cached Mesh stream, all as flat siblings (it deliberately skips
 * PatchObject::Save). This class claims those chunks head-first in file order and re-emits
 * them verbatim (raw bytes stay authoritative — no authoring direction yet), exposing typed
 * read access through the rpo_data.h decoders. See wiki drafts/max_geometry_formats.md Part A
 * and drafts/pipeline_max_design.md §10h.
 */
class CRklPatchObject : public BUILTIN::CPatchObject
{
public:
	CRklPatchObject(CScene *scene);
	virtual ~CRklPatchObject();

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

	// read access (valid between parse and clean/disown)
	/// The claimed chunk run (0x08FD + PatchMesh + Mesh chunks, in file order)
	inline const TStorageObjectContainer &claimedChunks() const { return m_Claimed; }
	/// The raw 0x08FD chunk (uint32 rpoVersion + RPatchMesh blob), NULL when absent
	const CStorageRaw *rpoChunk() const;
	/// Decode the RPatchMesh blob from the 0x08FD chunk
	bool decodeRPatch(SRPatchMesh &out, std::string &err) const;
	/// Decode the PatchMesh stream chunks
	bool decodePatch(SPatchMesh &out, std::string &err) const;

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	bool isKnownChunkId(uint16 id) const;

	/// Chunks claimed off the orphan list head, in original file order; re-emitted verbatim.
	TStorageObjectContainer m_Claimed;

}; /* class CRklPatchObject */

typedef CSceneClassDesc<CRklPatchObject> CRklPatchObjectClassDesc;
extern const CRklPatchObjectClassDesc RklPatchObjectClassDesc;

} /* namespace NELPATCH */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_RKL_PATCH_OBJECT_H */

/* end of file */
