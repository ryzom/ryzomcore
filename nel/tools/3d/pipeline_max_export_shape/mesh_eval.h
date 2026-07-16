/**
 * \file mesh_eval.h
 * \brief Evaluated Max Mesh extraction for the shape exporter: the full Mesh data (vertices,
 * faces with smoothing group + Max face flags incl. the matID high word, and the per-channel
 * map vertex/face arrays) of a node's object with the modifier stack applied — the headless
 * counterpart of EvalWorldState + ConvertToType(TRIOBJ) that CExportNel::buildMeshInterface
 * reads. Chunk-level decode established on the Max 9 shape corpus (see max_geometry_formats.md
 * Part F and pipeline_max_design.md §10i).
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_MESH_EVAL_H
#define PIPELINE_MAX_EXPORT_SHAPE_MESH_EVAL_H

#include <nel/misc/types_nl.h>

#include <map>
#include <string>
#include <vector>

#include "scene_lib.h"

namespace MESHEVAL {

using namespace SCENELIB;

// Max Face flags: edge visibility bits 0-2, hidden bit 4; matID in the high 16 bits.
#define MAX_FACE_MATID_SHIFT 16

struct SEvalFace
{
	uint32 V[3];
	uint32 SmGroup;
	uint32 Flags;
	uint16 matID() const { return (uint16)(Flags >> MAX_FACE_MATID_SHIFT); }
};

struct SMapFace
{
	uint32 T[3];
};

// One map channel (0 = vertex color, 1.. = UVW)
struct SMapChannel
{
	std::vector<MAXMATH::Point3M> Verts;
	std::vector<SMapFace> Faces; // same count as mesh faces
};

struct SEvalMesh
{
	std::vector<MAXMATH::Point3M> Verts; // object space
	std::vector<SEvalFace> Faces;
	std::map<int, SMapChannel> Maps;

	bool mapSupport(int channel) const { return Maps.find(channel) != Maps.end(); }
};

// Evaluate a node's object into a Mesh (modifier stack applied, object space).
// Unsupported modifiers warn to stderr and append their class id to warnings.
bool evalNodeMesh(INode &node, SEvalMesh &out, std::vector<std::string> *warnings = NULL);

} /* namespace MESHEVAL */

#endif /* PIPELINE_MAX_EXPORT_SHAPE_MESH_EVAL_H */

/* end of file */
