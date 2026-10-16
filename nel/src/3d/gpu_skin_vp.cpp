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

#include "std3d.h"

#include "nel/3d/gpu_skin_vp.h"
#include "nel/3d/program.h"
#include "nel/3d/uniform_buffer_format.h"
#include "nel/3d/uniform_buffer.h"
#include "nel/3d/skeleton_model.h"

namespace NL3D {

// GLSL snippet for the VP insert.
// The UBO block declarations are handled separately via UniformBufferFormats;
// this source only contains the function body.
// morphThreshold and morphAlpha come from the NlMorph UBO (VP binding).
// bones[] comes from the NlSkeleton UBO (skeleton binding).
// Bones are stored as 3 row-vectors (vec4) per bone in the UBO:
//   bones[b+0] = row0 = (Xx, Xy, Xz, Tx)
//   bones[b+1] = row1 = (Yx, Yy, Yz, Ty)
//   bones[b+2] = row2 = (Zx, Zy, Zz, Tz)
// Skinning: skinnedPos = bone * vec4(pos, 1) via 3 dot products per row.
// Normal/tangent: use .xyz of each row (rotation only, no translation).
static const char *s_GPUSkinInsertGLSL =
	"void nlPreTransform(inout vec4 pos, inout vec3 norm, inout vec4 tc0, inout vec4 tangent)\n"
	"{\n"
	"    ivec4 boneIdx = ivec4(vpaletteSkin);\n"
	"    vec4 boneWgt = vweight;\n"
	"\n"
	"    // MRM geomorph (vertices above threshold are morphing out)\n"
	"    if (gl_VertexID >= morphThreshold) {\n"
	"        pos.xyz = mix(vtexCoord4.xyz, pos.xyz, morphAlpha);\n"
	"        norm = mix(vtexCoord5.xyz, norm, morphAlpha);\n"
	"        tc0.xy = mix(vtexCoord6.xy, tc0.xy, morphAlpha);\n"
	"        tangent.xyz = mix(vtexCoord7.xyz, tangent.xyz, morphAlpha);\n"
	"    }\n"
	"\n"
	"    if (useSkeleton != 0) {\n"
	"        // Weighted bone skinning (3 row-vectors per bone)\n"
	"        vec3 skinnedPos = vec3(0.0);\n"
	"        vec3 skinnedNorm = vec3(0.0);\n"
	"        vec3 skinnedTan = vec3(0.0);\n"
	"        vec4 p = vec4(pos.xyz, 1.0);\n"
	"        for (int i = 0; i < 4; i++) {\n"
	"            float w = boneWgt[i];\n"
	"            if (w <= 0.0) continue;\n"
	"            int b = boneIdx[i] * 3;\n"
	"            vec4 row0 = bones[b];\n"
	"            vec4 row1 = bones[b+1];\n"
	"            vec4 row2 = bones[b+2];\n"
	"            skinnedPos += w * vec3(dot(row0, p), dot(row1, p), dot(row2, p));\n"
	"            skinnedNorm += w * vec3(dot(row0.xyz, norm), dot(row1.xyz, norm), dot(row2.xyz, norm));\n"
	"            skinnedTan += w * vec3(dot(row0.xyz, tangent.xyz), dot(row1.xyz, tangent.xyz), dot(row2.xyz, tangent.xyz));\n"
	"        }\n"
	"        pos = vec4(skinnedPos, 1.0);\n"
	"        norm = normalize(skinnedNorm);\n"
	"        tangent = vec4(normalize(skinnedTan), tangent.w);\n"
	"    } else {\n"
	"        // Geomorph only -- normalize after interpolation\n"
	"        norm = normalize(norm);\n"
	"        tangent = vec4(normalize(tangent.xyz), tangent.w);\n"
	"    }\n"
	"}\n";

static CVertexProgram *s_GPUSkinInsertVP = NULL;

CVertexProgram *getGPUSkinInsertVP()
{
	if (s_GPUSkinInsertVP)
		return s_GPUSkinInsertVP;

	CVertexProgram *vp = new CVertexProgram();
	IProgram::CSource *src = new IProgram::CSource();
	src->Profile = IProgram::glsl3vi;
	src->DisplayName = "GPU Skinning Insert";
	src->setSource(s_GPUSkinInsertGLSL);

	// NlMorph UBO at VP binding: morphThreshold + morphAlpha + useSkeleton
	CUniformBufferFormat *morphFmt = new CUniformBufferFormat();
	morphFmt->Name = "NlMorph";
	morphFmt->push("morphThreshold", CUniformBufferFormat::SInt, 1);
	morphFmt->push("morphAlpha", CUniformBufferFormat::Float, 1);
	morphFmt->push("useSkeleton", CUniformBufferFormat::SInt, 1);
	src->UniformBufferFormats[UBBindingVertexProgram] = morphFmt;

	// NlSkeleton UBO at skeleton binding: bones
	CUniformBufferFormat *skelFmt = new CUniformBufferFormat();
	skelFmt->Name = "NlSkeleton";
	skelFmt->push("bones", CUniformBufferFormat::FloatVec4, NL3D_GPU_SKIN_MAX_BONES * 3);
	src->UniformBufferFormats[UBBindingSkeleton] = skelFmt;

	vp->addSource(src);

	s_GPUSkinInsertVP = vp;
	return vp;
}

// Simple insert VP: bone skinning only (no MRM geomorph, no morph UBO).
// Used by CMeshGeom which has no MRM/LOD.
static const char *s_GPUSkinSimpleInsertGLSL =
	"void nlPreTransform(inout vec4 pos, inout vec3 norm, inout vec4 tc0, inout vec4 tangent)\n"
	"{\n"
	"    ivec4 boneIdx = ivec4(vpaletteSkin);\n"
	"    vec4 boneWgt = vweight;\n"
	"\n"
	"    // Weighted bone skinning (3 row-vectors per bone)\n"
	"    vec3 skinnedPos = vec3(0.0);\n"
	"    vec3 skinnedNorm = vec3(0.0);\n"
	"    vec3 skinnedTan = vec3(0.0);\n"
	"    vec4 p = vec4(pos.xyz, 1.0);\n"
	"    for (int i = 0; i < 4; i++) {\n"
	"        float w = boneWgt[i];\n"
	"        if (w <= 0.0) continue;\n"
	"        int b = boneIdx[i] * 3;\n"
	"        vec4 row0 = bones[b];\n"
	"        vec4 row1 = bones[b+1];\n"
	"        vec4 row2 = bones[b+2];\n"
	"        skinnedPos += w * vec3(dot(row0, p), dot(row1, p), dot(row2, p));\n"
	"        skinnedNorm += w * vec3(dot(row0.xyz, norm), dot(row1.xyz, norm), dot(row2.xyz, norm));\n"
	"        skinnedTan += w * vec3(dot(row0.xyz, tangent.xyz), dot(row1.xyz, tangent.xyz), dot(row2.xyz, tangent.xyz));\n"
	"    }\n"
	"    pos = vec4(skinnedPos, 1.0);\n"
	"    norm = normalize(skinnedNorm);\n"
	"    tangent = vec4(normalize(skinnedTan), tangent.w);\n"
	"}\n";

static CVertexProgram *s_GPUSkinSimpleInsertVP = NULL;

CVertexProgram *getGPUSkinSimpleInsertVP()
{
	if (s_GPUSkinSimpleInsertVP)
		return s_GPUSkinSimpleInsertVP;

	CVertexProgram *vp = new CVertexProgram();
	IProgram::CSource *src = new IProgram::CSource();
	src->Profile = IProgram::glsl3vi;
	src->DisplayName = "GPU Skinning Simple Insert";
	src->setSource(s_GPUSkinSimpleInsertGLSL);

	// Only NlSkeleton UBO at skeleton binding: bones
	CUniformBufferFormat *skelFmt = new CUniformBufferFormat();
	skelFmt->Name = "NlSkeleton";
	skelFmt->push("bones", CUniformBufferFormat::FloatVec4, NL3D_GPU_SKIN_MAX_BONES * 3);
	src->UniformBufferFormats[UBBindingSkeleton] = skelFmt;

	vp->addSource(src);

	s_GPUSkinSimpleInsertVP = vp;
	return vp;
}

// Singleton bone UBO
static NLMISC::CSmartPtr<CUniformBuffer> s_BoneUB;

CUniformBuffer *getGPUSkinBoneUBO()
{
	if (!s_BoneUB)
	{
		s_BoneUB = new CUniformBuffer();
		s_BoneUB->Format.Name = "NlSkeleton";
		s_BoneUB->Format.push("bones", CUniformBufferFormat::FloatVec4, NL3D_GPU_SKIN_MAX_BONES * 3);
		s_BoneUB->UsageHint = CUniformBuffer::StreamDraw;
	}
	return s_BoneUB;
}

void fillGPUSkinBoneUBO(CUniformBuffer *ub, CSkeletonModel *skeleton)
{
	enum { EntryBones = 0 };

	uint numBones = (uint)skeleton->Bones.size();
	if (numBones > NL3D_GPU_SKIN_MAX_BONES)
		numBones = NL3D_GPU_SKIN_MAX_BONES;

	ub->lock();
	for (uint boneId = 0; boneId < numBones; boneId++)
	{
		const CMatrix &boneMat = skeleton->getActiveBoneSkinMatrix(boneId);
		const float *m = boneMat.get(); // column-major float[16]

		// Row 0: (m[0], m[4], m[8], m[12]) = (Xx, Xy, Xz, Tx)
		sint rowBase = ub->Format.offset(EntryBones, boneId * 3);
		ub->set(rowBase, m[0], m[4], m[8], m[12]);

		// Row 1: (m[1], m[5], m[9], m[13]) = (Yx, Yy, Yz, Ty)
		rowBase = ub->Format.offset(EntryBones, boneId * 3 + 1);
		ub->set(rowBase, m[1], m[5], m[9], m[13]);

		// Row 2: (m[2], m[6], m[10], m[14]) = (Zx, Zy, Zz, Tz)
		rowBase = ub->Format.offset(EntryBones, boneId * 3 + 2);
		ub->set(rowBase, m[2], m[6], m[10], m[14]);
	}
	ub->unlock();
}

// Singleton morph UBO
static NLMISC::CSmartPtr<CUniformBuffer> s_MorphUB;

CUniformBuffer *getGPUSkinMorphUBO()
{
	if (!s_MorphUB)
	{
		s_MorphUB = new CUniformBuffer();
		s_MorphUB->Format.Name = "NlMorph";
		s_MorphUB->Format.push("morphThreshold", CUniformBufferFormat::SInt, 1);
		s_MorphUB->Format.push("morphAlpha", CUniformBufferFormat::Float, 1);
		s_MorphUB->Format.push("useSkeleton", CUniformBufferFormat::SInt, 1);
		s_MorphUB->UsageHint = CUniformBuffer::StreamDraw;
	}
	return s_MorphUB;
}

} // NL3D

/* end of file */
