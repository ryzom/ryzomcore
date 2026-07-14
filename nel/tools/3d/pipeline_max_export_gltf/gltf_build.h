/**
 * \file gltf_build.h
 * \brief glTF 2.0 document builder for the max2gltf converter: accumulates nodes, meshes
 * (encoded losslessly from CMesh::CMeshBuild — see the encoding notes in gltf_build.cpp),
 * materials (PBR interop view + nel_* extras via the NLGLTF material codec), textures/images,
 * accessors and one external .bin buffer, then writes <out>.gltf + <out>.bin.
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

#ifndef PIPELINE_MAX_EXPORT_GLTF_BUILD_H
#define PIPELINE_MAX_EXPORT_GLTF_BUILD_H

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>
#include <nel/3d/mesh.h>

#include <map>
#include <string>
#include <vector>

#include "../nel_gltf/json_value.h"

namespace GLTFBUILD {

// glTF constants
const int COMP_UBYTE = 5121;
const int COMP_USHORT = 5123;
const int COMP_UINT = 5125;
const int COMP_FLOAT = 5126;
const int TARGET_ARRAY = 34962;
const int TARGET_ELEMENT = 34963;

class CGltfBuilder
{
public:
	CGltfBuilder();
	~CGltfBuilder();

	// --- nodes ---
	// Creates a node with standard TRS + the bit-exact nel_t*/r*/s* extras (NeL conventions,
	// same values as the standard fields — see nel_gltf_extras.md). Returns the node index.
	sint addNode(const std::string &name, const NLMISC::CVector &pos, const NLMISC::CQuat &rot,
	             const NLMISC::CVector &scale);
	void setNodeChildren(sint node, const std::vector<sint> &children);
	void setSceneRoots(const std::vector<sint> &roots);
	void attachMesh(sint node, sint mesh);
	// The node's extras object (for appdata keys); never NULL for a valid index.
	NLGLTF::CJsonValue *nodeExtras(sint node);

	// --- materials ---
	// Interop PBR view + nel_* extras. Deduplicated on (name, serialized bytes). Returns the
	// glTF material index, or -1 with *err set.
	sint addMaterial(const NL3D::CMaterial &mat, const std::string &name, std::string *err);

	// --- meshes ---
	// Lossless CMeshBuild encoding; materialIdx maps the build's local material ids to glTF
	// material indices (also emitted as the mesh's nel_materials list). SkinWeights/BonesNames
	// ride nel_skin_* extras when present (PaletteSkinFlag). `bsList` (optional) carries the MRM
	// morph-target builds as per-target corner streams in global face order — exactly the fields
	// CMRMBuilder::buildBlendShapes consumes. `skinInterop` (in/out, optional): when *in* is
	// true, standard JOINTS_0/WEIGHTS_0 attributes are also emitted per primitive (viewing tier;
	// values = the CSkinWeight MatrixId/Weights, so the caller's skin.joints must follow
	// BonesNames order) — cleared to false (never an error) when the weights don't fit the
	// interop form, so the caller knows not to attach a skin. Returns mesh index or -1 with *err.
	sint addMesh(const std::string &name, const NL3D::CMesh::CMeshBuild &mb,
	             const std::vector<sint> &materialIdx, std::string *err,
	             const std::vector<NL3D::CMesh::CMeshBuild *> *bsList = NULL,
	             bool *skinInterop = NULL);

	// --- skins (viewing tier) ---
	// glTF skin object: joints are node indices (BonesNames order), ibms is joints.size()*16
	// column-major floats (inverse bind world matrices). Returns the skin index.
	sint addSkin(const std::vector<sint> &joints, const float *ibms);
	void setNodeSkin(sint node, sint skin);

	// Plain viewing mesh (positions/normals/uv + triangle indices, no material, no nel_*
	// reconstruction data) — used for the tessellated nel_proxy meshes (zones).
	sint addProxyMesh(const std::string &name, const std::vector<float> &pos,
	                  const std::vector<float> &norm, const std::vector<float> &uv,
	                  const std::vector<uint32> &indices);

	// Top-level asset extras (nel_source etc.)
	NLGLTF::CJsonValue *assetExtras();

	// Write <path> (must end .gltf) and the sibling .bin. Returns false on IO failure.
	bool save(const std::string &gltfPath);

	// --- low-level (also used by the ig encoding later) ---
	sint addBufferView(const void *data, size_t bytes, int target);
	sint addAccessor(sint bufferView, int componentType, const char *type, size_t count,
	                 bool normalized, const float *minv, const float *maxv, int nComp);
	sint addAccessorFloat(const float *data, size_t count, int nComp, int target, bool withMinMax);
	sint addAccessorU32(const uint32 *data, size_t count, int target);
	sint addAccessorU8Vec4Norm(const uint8 *data, size_t count4);
	sint addAccessorU16Vec4(const uint16 *data, size_t count4);

private:
	NLGLTF::CJsonValue m_Root;
	std::vector<uint8> m_Bin;
	NLGLTF::CJsonValue *m_Scenes;
	NLGLTF::CJsonValue *m_Nodes;
	NLGLTF::CJsonValue *m_Meshes;
	NLGLTF::CJsonValue *m_Materials;
	NLGLTF::CJsonValue *m_Textures;
	NLGLTF::CJsonValue *m_Images;
	NLGLTF::CJsonValue *m_Accessors;
	NLGLTF::CJsonValue *m_BufferViews;
	NLGLTF::CJsonValue *m_Skins; // lazily created — most files have none
	std::vector<NLGLTF::CJsonValue *> m_NodeVals;
	std::map<std::string, sint> m_MaterialDedup;
	std::map<std::string, sint> m_ImageDedup;
	std::map<std::string, sint> m_TextureDedup;

	sint addImageTexture(const std::string &uri);
};

} /* namespace GLTFBUILD */

#endif /* PIPELINE_MAX_EXPORT_GLTF_BUILD_H */

/* end of file */
