/**
 * \file physique_skin.cpp
 * \brief See physique_skin.h.
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

#include "physique_skin.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <set>

#include "biped_rig.h"

#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/storage_object.h"

#include <nel/misc/ucstring.h>

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

namespace PHYSIQUESKIN {

const NLMISC::CClassId CLASSID_PHYSIQUE(0x00000100, 0x00000000);
const NLMISC::CClassId CLASSID_SKIN(0x0095c6a3, 0x00015666);

bool isPhysiqueModifier(CSceneClass *mod)
{
	if (!mod || !mod->classDesc()) return false;
	return mod->classDesc()->classId() == CLASSID_PHYSIQUE
	       && mod->classDesc()->superClassId() == SCLASS_OSMODIFIER;
}

bool isSkinModifier(CSceneClass *mod)
{
	if (!mod || !mod->classDesc()) return false;
	return mod->classDesc()->classId() == CLASSID_SKIN;
}

// ---------------------------------------------------------------------------------------------
// Mod-app payload walk

static CStorageContainer *findChildContainer(CStorageContainer *parent, uint16 id)
{
	if (!parent) return NULL;
	const CStorageContainer::TStorageObjectContainer &ch = parent->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = ch.begin(); it != ch.end(); ++it)
	{
		if (it->first == id)
		{
			CStorageContainer *c = dynamic_cast<CStorageContainer *>(it->second);
			if (c) return c;
		}
	}
	return NULL;
}

static CStorageRaw *findChildRaw(CStorageContainer *parent, uint16 id)
{
	if (!parent) return NULL;
	const CStorageContainer::TStorageObjectContainer &ch = parent->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = ch.begin(); it != ch.end(); ++it)
	{
		if (it->first == id)
		{
			CStorageRaw *r = dynamic_cast<CStorageRaw *>(it->second);
			if (r) return r;
		}
	}
	return NULL;
}

// Resolve boneRef → INode via the modifier's reference table.
// Primary/rigid links store ~index (one's-complement); deformable cross-links sometimes store a
// plain non-negative index. Both are tried; NULL slots in the ref table are counted in the index
// (Part M §M.3 — Bip01 Head at index 6 because index 5 is NULL).
static INode *resolveBoneRef(CReferenceMaker *modRm, sint32 boneRef)
{
	if (!modRm) return NULL;
	uint n = modRm->nbReferences();
	sint32 idx = -1;
	// ~index form: high 24 bits all 1s for indices < 256 (the corpus span); general form is any
	// negative value from one's-complement of a valid index.
	if (boneRef < 0)
	{
		idx = ~boneRef; // one's complement
	}
	else
	{
		// Deformable cross-link candidate: raw non-negative index.
		idx = boneRef;
	}
	if (idx < 0 || (uint)idx >= n) return NULL;
	return dynamic_cast<INode *>(modRm->getReference((uint)idx));
}

bool decodePhysiqueWeights(CSceneClass *mod,
                           CStorageContainer *modApp,
                           std::vector<std::vector<SBoneWeight> > &outVertWeights,
                           std::string *err)
{
	outVertWeights.clear();
	if (!mod || !modApp)
	{
		if (err) *err = "null mod or modApp";
		return false;
	}
	CReferenceMaker *modRm = dynamic_cast<CReferenceMaker *>(mod);
	if (!modRm)
	{
		if (err) *err = "Physique modifier is not a ReferenceMaker";
		return false;
	}

	// 0x2500 → 0x2512 → 0x2504 (Part M §M.1)
	CStorageContainer *c2512 = findChildContainer(modApp, 0x2512);
	if (!c2512)
	{
		// Some stacks nest the payload one level deeper, or the app IS the 0x2500 leaf's parent
		// and we were handed the 0x2500 container itself already. Try treating modApp as 0x2500.
		c2512 = findChildContainer(modApp, 0x2500);
		if (c2512)
		{
			// modApp was the OSM wrapper; 0x2500 is the real app.
			modApp = c2512;
			c2512 = findChildContainer(modApp, 0x2512);
		}
	}
	if (!c2512)
	{
		if (err) *err = "Physique mod-app missing 0x2512 container";
		return false;
	}
	CStorageContainer *c2504 = findChildContainer(c2512, 0x2504);
	if (!c2504)
	{
		if (err) *err = "Physique mod-app missing 0x2504 payload";
		return false;
	}

	// Per-vertex records are the 0x2506 children after the fixed 4-chunk header.
	const CStorageContainer::TStorageObjectContainer &kids = c2504->chunks();
	std::vector<CStorageContainer *> vertRecs;
	vertRecs.reserve(kids.size());
	for (CStorageContainer::TStorageObjectConstIt it = kids.begin(); it != kids.end(); ++it)
	{
		if (it->first != 0x2506) continue;
		CStorageContainer *c = dynamic_cast<CStorageContainer *>(it->second);
		if (c) vertRecs.push_back(c);
	}
	if (vertRecs.empty())
	{
		if (err) *err = "Physique 0x2504 has no 0x2506 per-vertex records";
		return false;
	}

	outVertWeights.resize(vertRecs.size());
	uint unresolvedBones = 0;
	uint totalBones = 0;
	for (uint v = 0; v < vertRecs.size(); ++v)
	{
		CStorageRaw *rec = findChildRaw(vertRecs[v], 0x0989);
		if (!rec || rec->Value.size() < 4)
		{
			// Empty record → no influences (caller may root-fallback).
			continue;
		}
		const uint8 *p = (const uint8 *)nlVectorData(rec->Value);
		const uint8 *end = p + rec->Value.size();
		uint32 numBones = 0;
		memcpy(&numBones, p, 4);
		p += 4;
		// Each bone is 20 bytes: u32 boneRef + Point3(12) + float weight.
		if (numBones > 64 || p + (size_t)numBones * 20 > end)
		{
			// Malformed — try to consume what fits, or skip.
			uint32 maxFit = (uint32)((end - p) / 20);
			if (numBones > maxFit) numBones = maxFit;
		}
		outVertWeights[v].reserve(numBones);
		for (uint32 b = 0; b < numBones; ++b)
		{
			sint32 boneRef = 0;
			float offset[3];
			float weight = 0.f;
			memcpy(&boneRef, p, 4); p += 4;
			memcpy(offset, p, 12); p += 12;
			memcpy(&weight, p, 4); p += 4;
			(void)offset; // bind-pose only; not used for SkinWeights (Part M §M.3)
			++totalBones;
			INode *bone = resolveBoneRef(modRm, boneRef);
			if (!bone)
			{
				++unresolvedBones;
				continue;
			}
			// Skip non-positive weights (Physique stores rigidity factors that can be 0).
			if (weight <= 0.f) continue;
			SBoneWeight bw;
			bw.Bone = bone;
			bw.Weight = weight;
			outVertWeights[v].push_back(bw);
		}
	}
	if (getenv("PMB_SKIN_STATS"))
	{
		fprintf(stderr, "PMB_SKIN_STATS verts=%u bone-entries=%u unresolved=%u\n",
		        (uint)vertRecs.size(), totalBones, unresolvedBones);
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Skeleton bone map

INode *skeletonRootOf(INode *bone)
{
	if (!bone) return NULL;
	// CRootNode inherits INode but does NOT override parent() — INode::parent() nlerrors
	// (biped_rig.cpp:1302 note). Only CNodeImpl has a real parent().
	INode *n = bone;
	for (;;)
	{
		CNodeImpl *cn = dynamic_cast<CNodeImpl *>(n);
		if (!cn) break;
		INode *p = cn->parent();
		if (!p) break;
		// Stop when parent is the scene root (not a CNodeImpl, or itself has no parent).
		CNodeImpl *cp = dynamic_cast<CNodeImpl *>(p);
		if (!cp || !cp->parent())
			return n; // n is the top-level bone under the scene root
		n = p;
	}
	return n;
}

void buildSkeletonBoneMap(INode *root,
                          CSceneClassContainer *ssc,
                          std::map<INode *, sint32> &mapId,
                          std::vector<std::string> &bonesNames)
{
	mapId.clear();
	bonesNames.clear();
	if (!root) return;
	std::set<std::string> nameSet;

	// Depth-first, orderedChildrenOf for scene-order children — matches buildSkeletonShape.
	struct Walker
	{
		CSceneClassContainer *Ssc;
		std::map<INode *, sint32> *MapId;
		std::vector<std::string> *Names;
		std::set<std::string> *NameSet;
		void walk(INode *node)
		{
			sint32 id = (sint32)Names->size();
			std::string name = ucstring(node->userName()).toUtf8();
			if (!NameSet->insert(name).second)
				name += "_Second";
			Names->push_back(name);
			(*MapId)[node] = id;
			std::vector<INode *> kids = PMAX_RIG::orderedChildrenOf(node, Ssc);
			for (uint i = 0; i < kids.size(); ++i)
				walk(kids[i]);
		}
	} w;
	w.Ssc = ssc;
	w.MapId = &mapId;
	w.Names = &bonesNames;
	w.NameSet = &nameSet;
	w.walk(root);
}

// ---------------------------------------------------------------------------------------------
// Apply into CMeshBuild

bool applyPhysiqueSkinning(NL3D::CMesh::CMeshBuild &buildMesh,
                           INode &node,
                           const std::vector<CSceneClass *> &mods,
                           const std::vector<CStorageContainer *> &modApps,
                           CSceneClassContainer *ssc,
                           std::string *err)
{
	// Locate the Physique modifier + its mod-app slot.
	CSceneClass *physMod = NULL;
	CStorageContainer *physApp = NULL;
	for (uint i = 0; i < mods.size(); ++i)
	{
		if (isPhysiqueModifier(mods[i]))
		{
			physMod = mods[i];
			physApp = (i < modApps.size()) ? modApps[i] : NULL;
			break;
		}
	}
	if (!physMod)
	{
		if (err) *err = "no Physique modifier on node";
		return false;
	}
	if (!physApp)
	{
		if (err) *err = "Physique modifier has no mod-app slot";
		return false;
	}

	std::vector<std::vector<SBoneWeight> > vertWeights;
	if (!decodePhysiqueWeights(physMod, physApp, vertWeights, err))
		return false;

	if (vertWeights.size() != buildMesh.Vertices.size())
	{
		// Vertex count mismatch usually means the Physique was authored against a different
		// stack state than the evaluated mesh (Edit Mesh after Physique, etc.). Still try if
		// close, but refuse large skews.
		if (err)
		{
			char buf[160];
			sprintf(buf, "Physique vertex count %u != mesh vertex count %u",
			        (uint)vertWeights.size(), (uint)buildMesh.Vertices.size());
			*err = buf;
		}
		return false;
	}

	// Skeleton root from the first resolvable bone influence (same idea as getSkeletonRootBone).
	INode *skelRoot = NULL;
	for (uint v = 0; v < vertWeights.size() && !skelRoot; ++v)
	{
		for (uint b = 0; b < vertWeights[v].size(); ++b)
		{
			if (vertWeights[v][b].Bone)
			{
				skelRoot = skeletonRootOf(vertWeights[v][b].Bone);
				break;
			}
		}
	}
	// Fallback: first non-NULL ref on the Physique modifier.
	if (!skelRoot)
	{
		CReferenceMaker *modRm = dynamic_cast<CReferenceMaker *>(physMod);
		for (uint r = 0; modRm && r < modRm->nbReferences(); ++r)
		{
			INode *bn = dynamic_cast<INode *>(modRm->getReference(r));
			if (bn) { skelRoot = skeletonRootOf(bn); break; }
		}
	}
	if (!skelRoot)
	{
		if (err) *err = "could not locate skeleton root from Physique bones";
		return false;
	}

	std::map<INode *, sint32> mapId;
	std::vector<std::string> bonesNames;
	buildSkeletonBoneMap(skelRoot, ssc, mapId, bonesNames);
	if (bonesNames.empty())
	{
		if (err) *err = "skeleton bone map is empty";
		return false;
	}

	// Root bone id (always 0 by construction of the walk) — fallback for unresolved vertices
	// (the ConvertToRigid → Bip01 promotion class, Part M §M.3 item 2).
	const sint32 rootBoneId = 0;

	buildMesh.BonesNames = bonesNames;
	buildMesh.SkinWeights.resize(vertWeights.size());

	uint vertsNoWeight = 0;
	uint vertsRootFallback = 0;
	for (uint v = 0; v < vertWeights.size(); ++v)
	{
		// Collect weight → boneId, keeping only bones that resolve in the skeleton map.
		// multimap sorts ascending by weight so we can drop the lowest when over the max.
		std::multimap<float, sint32> weightMap;
		for (uint b = 0; b < vertWeights[v].size(); ++b)
		{
			INode *bn = vertWeights[v][b].Bone;
			float w = vertWeights[v][b].Weight;
			if (!bn || w <= 0.f) continue;
			std::map<INode *, sint32>::const_iterator it = mapId.find(bn);
			if (it == mapId.end()) continue;
			weightMap.insert(std::make_pair(w, it->second));
		}

		// Top-NL3D_MESH_SKINNING_MAX_MATRIX by weight (drop lowest).
		while (weightMap.size() > NL3D_MESH_SKINNING_MAX_MATRIX)
			weightMap.erase(weightMap.begin());

		NL3D::CMesh::CSkinWeight &sw = buildMesh.SkinWeights[v];
		for (uint i = 0; i < NL3D_MESH_SKINNING_MAX_MATRIX; ++i)
		{
			sw.MatrixId[i] = 0;
			sw.Weights[i] = 0.f;
		}

		if (weightMap.empty())
		{
			// No resolvable influence — root fallback (coverage path for ConvertToRigid residues).
			sw.MatrixId[0] = (uint32)rootBoneId;
			sw.Weights[0] = 1.f;
			++vertsNoWeight;
			++vertsRootFallback;
			continue;
		}

		float sum = 0.f;
		for (std::multimap<float, sint32>::const_iterator it = weightMap.begin();
		     it != weightMap.end(); ++it)
			sum += it->first;
		if (sum <= 0.f)
		{
			sw.MatrixId[0] = (uint32)rootBoneId;
			sw.Weights[0] = 1.f;
			++vertsRootFallback;
			continue;
		}

		// Emit highest-weight first (reference iterates the multimap from end).
		uint id = 0;
		for (std::multimap<float, sint32>::const_reverse_iterator it = weightMap.rbegin();
		     it != weightMap.rend() && id < NL3D_MESH_SKINNING_MAX_MATRIX; ++it, ++id)
		{
			sw.MatrixId[id] = (uint32)it->second;
			sw.Weights[id] = it->first / sum;
		}
	}

	if (getenv("PMB_SKIN_STATS"))
	{
		fprintf(stderr, "PMB_SKIN_STATS node skinned: verts=%u bones=%u noWeight=%u rootFallback=%u root='%s'\n",
		        (uint)vertWeights.size(), (uint)bonesNames.size(), vertsNoWeight, vertsRootFallback,
		        bonesNames.empty() ? "?" : bonesNames[0].c_str());
	}
	(void)node; // reserved for future Skin-modifier path / diagnostics
	return true;
}

} /* namespace PHYSIQUESKIN */

/* end of file */
