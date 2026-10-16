/**
 * \file patch_topo_snapshot.h
 * \brief Raw pre/post snapshot of one topological edit (undo Kind 6 payload)
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * A topological op captures the decoded target-stream structs before and after the
 * transform; undo and redo re-encode the matching side back into storage and rebuild the
 * working set. decode -> encode is the corpus-proven byte identity, so undoing a
 * topological edit and saving reproduces the null-edit output byte for byte. The mapper
 * travels as raw payload bytes (verbatim restore, no re-encode needed).
 *
 * Kept out of paint_core.h on purpose: the core stores these by OPAQUE pointer so its
 * header stays free of pipeline_max types (editor TU hygiene). The op layer and
 * paint_ops.cpp include this header for the complete type.
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

#ifndef ZONE_PAINTER_PATCH_TOPO_SNAPSHOT_H
#define ZONE_PAINTER_PATCH_TOPO_SNAPSHOT_H

#include <nel/misc/types_nl.h>
#include <vector>

#include "../pipeline_max/nelpatch/rpo_data.h"

namespace ZPPAINT {

struct STopoSnapshot
{
	uint Zone; ///< zone id the op addressed (restore fans per object like the op did)
	PIPELINE::MAX::NELPATCH::SPatchMesh PmOld, PmNew;
	PIPELINE::MAX::NELPATCH::SRPatchMesh RpOld, RpNew;
	std::vector<uint8> MapperOld, MapperNew; ///< raw 0x1130 payloads, verbatim
	bool HaveMapper;
	/// Anchor-cell shift the op applied to the zone's owning file (attach: the merged
	/// geometry moved the authored footprint origin). Restore re-applies the matching
	/// direction so the placement math keeps the zone where the restored side had it.
	int CellDX, CellDY;
	STopoSnapshot() : Zone(0), HaveMapper(false), CellDX(0), CellDY(0) { }
};

} /* namespace ZPPAINT */

#endif /* ZONE_PAINTER_PATCH_TOPO_SNAPSHOT_H */

/* end of file */
