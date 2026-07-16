/**
 * \file anim_build.h
 * \brief Per-node animation export for the shape process (NelExportAnimation).
 *
 * The shape process (shape_export.ms) exports a <node>.anim per node with
 * NEL3D_APPDATA_AUTOMATIC_ANIMATION set, via NelExportAnimation #(node). That covers:
 *   - node transform tracks (pos/rotquat/scale) — e.g. conerotor
 *   - material texture-matrix tracks (bExportTextureMatrix) — waterfalls (§10k)
 *   - LightmapController.<animName> color tracks on lights with LM_ANIMATED — brazero/lanterne
 *   - morph factor tracks when a Morpher is present
 *
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * \author Grok 4.5
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

#ifndef PIPELINE_SHAPE_ANIM_BUILD_H
#define PIPELINE_SHAPE_ANIM_BUILD_H

#include <nel/misc/types_nl.h>
#include "scene_lib.h"

namespace NL3D { class CAnimation; }

namespace SHAPEANIM {

/// True when the node requests animation export (NEL3D_APPDATA_AUTOMATIC_ANIMATION != "0"),
/// and is not excluded by DONOTEXPORT / COLLISION flags (shape_export.ms isAnimToBeExported).
bool isAnimToBeExported(SCENELIB::INode &node);

/// Build the full NelExportAnimation track set for a single selected node (shape process:
/// one node, scene=false). Returns the number of tracks added. Caller serializes when > 0
/// (or always — empty animations exist as 29-byte NEL_ANIM headers for Sun.anim etc.;
/// the reference still writes those; we match by writing whenever isAnimToBeExported).
uint buildNodeAnim(SCENELIB::INode &node, NL3D::CAnimation &animation);

} /* namespace SHAPEANIM */

#endif /* #ifndef PIPELINE_SHAPE_ANIM_BUILD_H */

/* end of file */
