/**
 * \file gltf_material.h
 * \brief NL3D::CMaterial <-> glTF `extras` codec (the `nel_*` material keys). The writer
 * (pipeline_max_export_gltf) emits the semantic field set of a built export material; the
 * reader (mesh_utils' nel-extras glTF path) reconstructs a CMaterial through the same public
 * setter sequence the exporter's material builder used, then verifies the reconstructed flag
 * dword against the emitted `nel_flags` — any gap in the field set fails loudly instead of
 * shipping a silently-different material. Schema documented in wiki drafts/nel_gltf_extras.md.
 *
 * Scope note: LightMaps are never populated by the 1_export-stage exporters (the standalone
 * lightmapper adds them downstream), so the codec rejects materials carrying them.
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

#ifndef NL_GLTF_MATERIAL_H
#define NL_GLTF_MATERIAL_H

#include <nel/misc/types_nl.h>
#include <nel/3d/material.h>

#include <string>

#include "json_value.h"

namespace NLGLTF {

/// Emit the nel_* material keys into `extras` (an object value). Returns false (with *err set)
/// when the material carries state outside the codec's scope (lightmaps).
bool materialToExtras(const NL3D::CMaterial &mat, CJsonValue &extras, std::string *err);

/// Reconstruct the material from nel_* extras. Returns false (with *err) on missing keys or
/// when the reconstructed flag dword mismatches the emitted `nel_flags` verification value.
bool materialFromExtras(const CJsonValue &extras, NL3D::CMaterial &mat, std::string *err);

} /* namespace NLGLTF */

#endif /* NL_GLTF_MATERIAL_H */

/* end of file */
