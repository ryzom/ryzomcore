/**
 * \file anim_build.h
 * \brief Per-node material texture-matrix animation export for the shape process.
 *
 * The shape process (shape_export.ms) exports a <node>.anim per node with
 * NEL3D_APPDATA_AUTOMATIC_ANIMATION set, via NelExportAnimation. For the animated-material class
 * (waterfalls: a scrolling texture), that animation is the material texture-matrix tracks
 * (CExportNel::addMtlTracks -> addTexTracks): a material with bExportTextureMatrix has, per
 * enabled texture stage, its texmap's StdUVGen U/V Offset (etc.) driven by a Linear Float
 * controller, exported as a CTrackKeyFramerLinearFloat named <mtlName>.UTrans<stage> /
 * .VTrans<stage>. See pipeline_max_design.md §10k.
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

/// True when the node requests animation export (NEL3D_APPDATA_AUTOMATIC_ANIMATION != "0").
bool isAnimToBeExported(SCENELIB::INode &node);

/// Add the node's material texture-matrix animation tracks (gated by the node's
/// NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS). Returns the number of tracks added.
uint buildMaterialAnim(SCENELIB::INode &node, NL3D::CAnimation &animation);

} /* namespace SHAPEANIM */

#endif /* #ifndef PIPELINE_SHAPE_ANIM_BUILD_H */

/* end of file */
