/**
 * \file gltf_nel_scene.h
 * \brief Exact-tier glTF import for NeL: when a .gltf carries the nel_* extras written by
 * pipeline_max_export_gltf (max2gltf), reconstruct the pre-build CMeshBuild/CMeshBaseBuild/
 * CMaterial state directly from the glTF JSON + buffer (no assimp — the extras and the NeL
 * reconstruction accessors are read from the document itself, per the max2gltf plan), then
 * replay the same NL3D build calls the direct .max exporter runs. Output .shape files are
 * byte-comparable with the direct route's.
 *
 * Artist-authored glTF without nel_* extras (or with --no-nel-extras) keeps going through the
 * assimp route in mesh_utils.cpp.
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

#ifndef NL_MESH_UTILS_GLTF_NEL_SCENE_H
#define NL_MESH_UTILS_GLTF_NEL_SCENE_H

#include <nel/misc/types_nl.h>

#include <string>

struct CMeshUtilsSettings;

/// Is this file a nel-extras glTF (extension .gltf and asset.extras.nel_source present)?
bool isNelGltfFile(const std::string &filePath);

/// Export the nel-extras glTF to .shape files. Returns EXIT_SUCCESS / EXIT_FAILURE.
int exportNelGltfScene(const CMeshUtilsSettings &settings);

#endif /* NL_MESH_UTILS_GLTF_NEL_SCENE_H */

/* end of file */
