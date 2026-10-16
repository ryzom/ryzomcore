/**
 * \file flare_build.h
 * \brief CFlareShape construction from a NeL flare node (nel_flare scripted plugin, ClassId
 * (0x4e913532, 0x3c2f2307)) — replicates CExportNel::buildFlare (plugin_max/nel_mesh_lib/
 * export_flare.cpp). See pipeline_max_design.md §10z handoff (flare = 13 files, close class).
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_FLARE_BUILD_H
#define PIPELINE_MAX_EXPORT_SHAPE_FLARE_BUILD_H

#include <nel/misc/types_nl.h>

#include "scene_lib.h"

namespace NL3D { class IShape; }

namespace FLAREBUILD {

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

// Build a CFlareShape from a nel_flare-instance node. Returns NULL on failure (missing spacing,
// no PB2 found). Caller owns the returned pointer. tmCache is used only for the default-position
// derivation (via getLocalMatrix), same convention as buildWaterShape.
NL3D::IShape *buildFlareShape(INode &node, SCENELIB::SNodeTMCache &tmCache);

} // namespace FLAREBUILD

#endif /* PIPELINE_MAX_EXPORT_SHAPE_FLARE_BUILD_H */

/* end of file */
