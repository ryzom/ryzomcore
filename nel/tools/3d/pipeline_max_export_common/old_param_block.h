/**
 * \file old_param_block.h
 * \brief Reader for the pre-ParamBlock2 "old" Max ParamBlock (superclass 0x8) — the format Max's
 * own built-in parametric primitives (Box, Cylinder, Sphere, ...) still use for their dimension
 * parameters. The decode itself lives in the library's typed BUILTIN::CParamBlock (every
 * superclass-0x8 object parses through it; see pipeline_max/builtin/param_block.h for the chunk
 * layout); readOldParamBlock is a thin copy onto the legacy per-index map shape the exporters
 * consume. Previously the decode was duplicated verbatim here, in pipeline_max_export_ig/main.cpp
 * (readPBlockParams, used for lights and the parametric-mesh builder) and in
 * pipeline_max_export_shape/scene_lib.cpp.
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_OLD_PARAM_BLOCK_H
#define PIPELINE_MAX_EXPORT_COMMON_OLD_PARAM_BLOCK_H

#include <nel/misc/types_nl.h>

#include <map>

#include "../pipeline_max/scene_class.h"

namespace OLDPBLOCK {

/// One decoded old-ParamBlock parameter value (raw form; caller picks the right member per the
/// known type of the parameter it asked for).
struct SParam
{
	bool IsPoint3;
	bool IsInt;
	sint32 I;
	float V[3];
};

/// Decode every 0x0002 parameter entry off an old ParamBlock (superclass 0x8) scene object,
/// keyed by its 0x0003 declared index. Absent indices are simply missing from \a out.
void readOldParamBlock(PIPELINE::MAX::CSceneClass *pblock, std::map<sint32, SParam> &out);

/// Convenience float read (0.0f when the index is absent — matches PBF() in the ig exporter).
inline float paramFloat(const std::map<sint32, SParam> &params, sint32 index)
{
	std::map<sint32, SParam>::const_iterator it = params.find(index);
	return it != params.end() ? it->second.V[0] : 0.0f;
}

/// Convenience int read (0 when the index is absent — matches PBI() in the ig exporter).
inline sint paramInt(const std::map<sint32, SParam> &params, sint32 index)
{
	std::map<sint32, SParam>::const_iterator it = params.find(index);
	if (it == params.end()) return 0;
	return it->second.IsInt ? (sint)it->second.I : (sint)it->second.V[0];
}

} /* namespace OLDPBLOCK */

#endif /* PIPELINE_MAX_EXPORT_COMMON_OLD_PARAM_BLOCK_H */

/* end of file */
