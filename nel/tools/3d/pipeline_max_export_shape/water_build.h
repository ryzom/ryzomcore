/**
 * \file water_build.h
 * \brief CWaterShape construction from a Max node whose material carries the bWater flag —
 * the headless counterpart of CExportNel::buildWaterShape (plugin_max/nel_mesh_lib/
 * export_mesh.cpp:1675). Water shapes are the biggest remaining not-produced class after M2
 * (221 corpus files, ~10% of the reference shape output).
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_WATER_BUILD_H
#define PIPELINE_MAX_EXPORT_SHAPE_WATER_BUILD_H

#include <nel/misc/types_nl.h>
#include <nel/3d/shape.h>

#include "scene_lib.h"

namespace WATERBUILD {

using namespace SCENELIB;

/// Build a CWaterShape from a Max node whose material has bWater=1. Returns NULL on failure
/// (missing required maps, no valid geometry, etc.); the caller is expected to have gated on
/// hasWaterMaterial(node) before calling. Ownership of the returned shape passes to the caller
/// (delete after serialization).
///
/// Replicates CExportNel::buildWaterShape (plugin_max/nel_mesh_lib/export_mesh.cpp:1675):
///  - Evaluates the node's mesh (existing MESHEVAL::evalNodeMesh, respecting the modifier stack).
///  - Projects the mesh's node-local verts onto Z=0 and takes the convex hull → CPolygon2D
///    (setShape). Only convex shapes supported, matching the reference.
///  - Reads the water material's PB2 for required slots 1 (envMap above), 5 (bumpMap),
///    6 (displaceMap) — mandatory; missing → NULL like the reference. Optional slots 2 (envMap
///    below), 3/4 (underwater envmaps), 7 (diffuse/color map).
///  - Reads scroll/scale params (fBumpUScale, fBumpVScale, fBumpUSpeed, fBumpVSpeed;
///    fDisplaceMap*), pool id (iWaterPoolID), wave height factor (fWaterHeightFactor), splash
///    (bEnableWaterSplash), realtime reflection + fresnel (bWaterRealtimeReflection,
///    bWaterEnvMapCalcReflectivity, fWaterFresnelBias/Scale/Power), and scene-envmap toggles.
///  - Sets default pos/rot/scale from the node's local matrix decompose (same convention as
///    the mesh path — decompMatrix into CTrackDefault*).
///
/// NOT implemented in this pass (documented open items for the next iteration):
///  - Color map's texture-matrix (`setColorMapMat`) — a 2x3 affine derived from a chosen mesh
///    triangle's world positions vs its UVs, plus the diffuse map's UVGen crop. The math is
///    involved (see the reference at export_mesh.cpp:1957-2043) and only fires when the artist
///    enables slot 7 with a diffuse map; most corpus water shapes don't use it. Water shapes
///    without a color map are unaffected; ones with one will diff on the ColorMapMat.
///  - Texture blend (`CTextureBlend`) for env-map pairs (day/night blend, `_EnvMap[0/1]`). The
///    reference builds a CTextureBlend when both envmaps are present; we use the primary only.
NL3D::IShape *buildWaterShape(INode &node, SNodeTMCache &tmCache);

} /* namespace WATERBUILD */

#endif /* PIPELINE_MAX_EXPORT_SHAPE_WATER_BUILD_H */

/* end of file */
