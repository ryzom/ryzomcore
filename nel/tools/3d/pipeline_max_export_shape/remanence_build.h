/**
 * \file remanence_build.h
 * \brief CSegRemanenceShape construction from a USE_REMANENCE-flagged Shape node — replicates
 * CExportNel::buildRemanence (plugin_max/nel_mesh_lib/export_remanence.cpp). Closes the
 * 82-file remanence skip class (§10z-sept handoff). Shared spline decode lives in
 * pipeline_max_export_common/spline_shape.
 * \author Jan Boon (Kaetemi)
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_REMANENCE_BUILD_H
#define PIPELINE_MAX_EXPORT_SHAPE_REMANENCE_BUILD_H

#include <nel/misc/types_nl.h>

#include "scene_lib.h"

namespace NL3D { class IShape; }

namespace REMANENCEBUILD {

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

/// Build a CSegRemanenceShape from a node with NEL3D_APPDATA_USE_REMANENCE. Returns NULL on
/// failure (not a shape object, multi-curve, empty curve, material count ≠ 1). Caller owns.
NL3D::IShape *buildRemanenceShape(INode &node, SCENELIB::SNodeTMCache &tmCache,
                                  bool exportLighting);

} // namespace REMANENCEBUILD

#endif /* PIPELINE_MAX_EXPORT_SHAPE_REMANENCE_BUILD_H */

/* end of file */
