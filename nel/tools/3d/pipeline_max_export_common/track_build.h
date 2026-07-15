/**
 * \file track_build.h
 * \brief Shared Max controller → NL3D ITrack builders (buildATrack).
 *
 * Replicates CExportNel::buildATrack / createKeyFramer key conversions so both
 * pipeline_max_export_anim and pipeline_max_export_shape produce identical track
 * bytes. Supports typePos / typeRotation / typeScale / typeFloat / typeColor.
 *
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_TRACK_BUILD_H
#define PIPELINE_MAX_EXPORT_COMMON_TRACK_BUILD_H

#include <nel/misc/types_nl.h>

namespace NL3D { class ITrack; class CAnimation; }
namespace PIPELINE { namespace MAX { namespace BUILTIN { class CReferenceMaker; } } }

namespace TRACKBUILD {

/// Max ticks per second (GetTicksPerFrame * GetFrameRate = 4800).
static const float TICKS_PER_SECOND = 4800.0f;

enum TNelValueType
{
	typePos,
	typeRotation,
	typeScale,
	typeFloat,
	typeColor
};

/// Build a NeL track from a typed Max keyframer controller, or NULL when unsupported / no keys.
/// Mirrors plugin_max/nel_mesh_lib/export_anim.cpp CExportNel::buildATrack.
NL3D::ITrack *buildATrack(PIPELINE::MAX::BUILTIN::CReferenceMaker *ctrl, TNelValueType type);

/// First-wins add (delete track if the name is already present).
void addTrackChecked(NL3D::CAnimation &animation, const std::string &name, NL3D::ITrack *track);

} /* namespace TRACKBUILD */

#endif /* PIPELINE_MAX_EXPORT_COMMON_TRACK_BUILD_H */

/* end of file */
