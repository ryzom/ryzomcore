/**
 * \file material_sidecar.h
 * \brief Materials sidecar loader for the assimp import route: a materials-only .gltf (valid
 * glTF, no nodes needed) carrying the same nel_* material extras the whole-file route uses,
 * resolved by material NAME against whatever model file the artist provided. Named scene
 * materials with a sidecar entry get the exact reconstructed CMaterial (flag-dword verified by
 * the codec); names without an entry warn and fall back to assimp_material conversion — never
 * positional binding. Multiple sidecars stack (shared library first, per-asset second; later
 * files override earlier ones by name). See wiki drafts/max2gltf_plan.md (decision 2026-07-14).
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
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

#ifndef NL_MESH_UTILS_MATERIAL_SIDECAR_H
#define NL_MESH_UTILS_MATERIAL_SIDECAR_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

#include "scene_meta.h" // TMaterialMap

namespace NLPIPELINE {
	class CToolLogger;
}

/// Load material sidecar .gltf files into `out` (name -> reconstructed CMaterial). Later files
/// override earlier ones (info-logged). Entries without a name or without nel_* extras are
/// warned and skipped; a codec reconstruction failure (missing keys, flag verification) is a
/// hard error — the sidecar is corrupt or outdated. Returns false on hard error.
bool loadMaterialSidecars(const std::vector<std::string> &paths, TMaterialMap &out,
                          NLPIPELINE::CToolLogger &logger, const std::string &sourceFilePath);

#endif /* NL_MESH_UTILS_MATERIAL_SIDECAR_H */

/* end of file */
