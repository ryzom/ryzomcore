// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2025-2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef NL3D_GPU_SKIN_VP_H
#define NL3D_GPU_SKIN_VP_H

#include "nel/misc/types_nl.h"
#include "nel/3d/vertex_program.h"

namespace NL3D {

class CUniformBuffer;
class CSkeletonModel;

/// Maximum bones supported by GPU skinning UBO.
/// 80 bones * 3 vec4 = 240 vec4 entries = 3840 bytes.
#define NL3D_GPU_SKIN_MAX_BONES 80

/// NlMorph UBO entry indices.
enum TGPUSkinMorphEntry
{
	GPUSkinMorphThreshold = 0,  // SInt: vertex index threshold for geomorph
	GPUSkinMorphAlpha = 1,      // Float: geomorph blend factor [0,1]
	GPUSkinUseSkeleton = 2,     // SInt: 1 = apply bone skinning, 0 = geomorph only
};

/// Get the singleton GPU skinning VP insert program (MRM geomorph + bone skinning).
/// Created lazily on first call. Shared by all MRM skinned meshes.
/// The returned program uses the glsl3vi profile.
CVertexProgram *getGPUSkinInsertVP();

/// Get the singleton simple GPU skinning VP insert program (bone skinning only).
/// Created lazily on first call. Shared by all simple skinned meshes (CMeshGeom).
/// The returned program uses the glsl3vi profile.
/// No morph UBO needed — only NlSkeleton UBO at UBBindingSkeleton.
CVertexProgram *getGPUSkinSimpleInsertVP();

/// Get the singleton bone UBO (lazy-created with NlSkeleton format).
CUniformBuffer *getGPUSkinBoneUBO();

/// Get the singleton morph UBO (lazy-created with NlMorph format).
CUniformBuffer *getGPUSkinMorphUBO();

/// Fill the bone UBO with matrices from the given skeleton.
void fillGPUSkinBoneUBO(CUniformBuffer *ub, CSkeletonModel *skeleton);

} // NL3D

#endif // NL3D_GPU_SKIN_VP_H

/* end of file */
