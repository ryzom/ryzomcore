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
	// The sub-object selection the deforming modifiers act on, as the base Editable Mesh or an Edit Mesh left
	// it: the selection level (1 object, the whole mesh; 2 vertex; 4 face; 8 edge) and the selection of every
	// level, one flag per vertex, per face and per face side (3 per face). A selection whose size does not
	// match the mesh is not known.
	uint32 SelLevel;
	std::vector<bool> SelVerts, SelFaces, SelEdges;
	// What the modifiers above see is not known: the level (after a modifier the evaluator does not handle, or
	// over an Editable Poly base), or the selections (the same, or a selection chunk that was not readable).
	// An Edit Mesh makes the level known again, and the selections when it stores its own.
	bool SelLevelUnknown, SelBitsUnknown;
	// Soft selection is on: the modifiers above also move the vertices around the selection, by a falloff
	// the evaluator does not compute.
	bool SelSoft;

	SEvalMesh() : SelLevel(1), SelLevelUnknown(false), SelBitsUnknown(false), SelSoft(false) { }
	bool mapSupport(int channel) const { return Maps.find(channel) != Maps.end(); }
};

// Evaluate a node's object into a Mesh (modifier stack applied, object space).
// Unsupported modifiers warn to stderr and append their class id to warnings.
bool evalNodeMesh(INode &node, SEvalMesh &out, std::vector<std::string> *warnings = nullptr);

} /* namespace MESHEVAL */

#endif /* PIPELINE_MAX_EXPORT_SHAPE_MESH_EVAL_H */

/* end of file */
