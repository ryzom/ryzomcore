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
	if (!parent) return nullptr;
	const CStorageContainer::TStorageObjectContainer &ch = parent->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = ch.begin(); it != ch.end(); ++it)
	{
		if (it->first == id)
		{
			CStorageContainer *c = dynamic_cast<CStorageContainer *>(it->second);
			if (c) return c;
		}
	}
	return nullptr;
}

static CStorageRaw *findChildRaw(CStorageContainer *parent, uint16 id)
{
	if (!parent) return nullptr;
	const CStorageContainer::TStorageObjectContainer &ch = parent->chunks();
	for (CStorageContainer::TStorageObjectConstIt it = ch.begin(); it != ch.end(); ++it)
	{
		if (it->first == id)
		{
			CStorageRaw *r = dynamic_cast<CStorageRaw *>(it->second);
			if (r) return r;
		}
	}
	return nullptr;
}

// Resolve boneRef → INode via the modifier's reference table.
// The stored value encodes a LINK index k: negative form is one's-complement (~stored = k,
// the rigid/primary encoding), positive form is k + 1 (the deformable cross-link encoding).
// Link k is the SEGMENT ENDING at the bone in reference slot k+1, and the vertex deforms by
// the segment's START bone — which is the INode PARENT of ref[k+1], NOT ref[k] itself.
// The two coincide for mid-chain slots (parent(ref[k+1]) == ref[k]), which is why a plain
// ref[k] read looked anatomically right everywhere except chain boundaries; at boundaries
// the parent rule is what matches the reference exporter (empirically solved 2026-07-10 by
// weight-multiset matching over every position-matched vertex of the five fy_hof_armor01
// nodes against their reference CMeshMRMSkinned shapes — see the design doc):
//   ref[k+1] = 'R Hand'        → 'R Forearm'  (the Hand slot's link starts at the forearm)
//   ref[k+1] = 'R Finger0'     → 'R Hand'     (k+1 right after a NULL chain-attach slot)
//   ref[k+1] = 'R Toe0' (nub)  → 'R Toe0'     (name-shadowed nub; parent = the real toe)
//   ref[k+1] = 'Bip01 Spine'   → 'Bip01 Pelvis'
// The link-tree ROOT bone: ref[0] when non-NULL (the root-attach end — 'Bip01 Pelvis' on
// every biped-attached corpus rig), else the INode PARENT of the first non-NULL ref (C04's
// table starts [NULL, 'Bip01 Spine', ...] with Pelvis absent — the reference still deforms
// its root-link verts by Pelvis = parent(Spine)). Corpus-pinned per-vertex against the clod
// references (diranak/estrasson/C04, all → 'Bip01 Pelvis', never the COM).
static INode *linkTreeRoot(CReferenceMaker *modRm)
{
	uint n = modRm->nbReferences();
	for (uint i = 0; i < n; ++i)
	{
		CNodeImpl *bn = dynamic_cast<CNodeImpl *>(modRm->getReference(i));
		if (!bn) continue;
		if (i == 0)
			return bn; // root-attach end is stored directly
		return bn->parent(); // ref[0] NULL: root = parent of the first chain bone
	}
	return nullptr;
}

// Stored value 0 is link k = -1 — the ROOT-ATTACH segment, which has no start bone above it
// in the Physique link tree: the deforming bone is the link-tree root (clod corpus: the
// references map these verts to 'Bip01 Pelvis', never the COM). A NULL ref[k+1] on a k >= 0
// link is a deleted/free chain-attach end and ALSO deforms by the link-tree root (diranak/
// estrasson's [-19] links end at the NULL slot ref[19]; the reference keeps them as real
// Pelvis influences inside blends at the stored relative weights). Out-of-range k+1 stays
// unresolved and falls through to the caller's root fallback.
static INode *resolveBoneRef(CReferenceMaker *modRm, sint32 boneRef)
{
	if (!modRm) return nullptr;
	uint n = modRm->nbReferences();
	sint32 k;
	if (boneRef < 0)
		k = ~boneRef; // one's complement (rigid link)
	else
		k = boneRef - 1; // 1-based (deformable cross-link); 0 → the root-attach link -1
	if (k < -1 || (uint)(k + 1) >= n) return nullptr;
	if (k == -1)
		return linkTreeRoot(modRm); // root-attach link
	CNodeImpl *endImpl = dynamic_cast<CNodeImpl *>(modRm->getReference((uint)(k + 1)));
	if (!endImpl)
		return linkTreeRoot(modRm); // NULL chain-attach end: deforms by the root
	INode *start = endImpl->parent();
	// A biped COM is not part of the DEFORMABLE spline tree: a cross-link (positive form)
	// whose segment start lands on a linked sub-rig's COM ('Bip02' via parent('Bip02 Spine'))
	// deforms by the bone the sub-rig is ATTACHED to — the COM's own parent ('Bip01 Spine2'
	// on the kitin queen, corpus-pinned per-vertex against the reference clod). RIGID links
	// (negative form) attach to the literal parent node — the kitihank/kitinagan references
	// keep 'Bip02' itself as the deform bone for their rigid links, so the skip must not
	// apply there.
	if (boneRef > 0)
	{
		while (start && PMAX_RIG::isBipedComNode(start))
		{
			CNodeImpl *ci = dynamic_cast<CNodeImpl *>(start);
			if (!ci) break;
			start = ci->parent();
		}
	}
	return start;
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
	std::map<sint32, uint> unresolvedVals;
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
			if (getenv("PMB_SKIN_DUMP_REFS"))
				fprintf(stderr, "SKINREF v%u ref=%d -> '%s'\n", v, (int)boneRef,
				        bone ? ucstring(bone->userName()).toUtf8().c_str() : "(null)");
			if (!bone)
			{
				++unresolvedBones;
				++unresolvedVals[boneRef];
				continue;
			}
			// A SINGLE-link record is a rigid vertex: it deforms 100% by its one bone and the
			// stored float is Physique's rigidity/blend factor, not a skinning weight — the
			// c03/monster family stores 0.0 there (620+ verts per mesh silently fell to the
			// root fallback, the tr_mo_c03_boss "12 vs 55 bones" class); the armor family
			// stores 1.0 (unaffected). Part M §M.2 documents the field's non-weight
			// semantics; forcing 1.0 on one-link records is the ConvertToRigid read.
			// Multi-link records keep the stored blend values, dropping non-positive entries.
			if (numBones == 1) weight = 1.f;
			else if (weight <= 0.f) continue;
			SBoneWeight bw;
			bw.Bone = bone;
			bw.Weight = weight;
			outVertWeights[v].push_back(bw);
		}
	}
	if (getenv("PMB_SKIN_STATS"))
	{
		fprintf(stderr, "PMB_SKIN_STATS verts=%u bone-entries=%u unresolved=%u",
		        (uint)vertRecs.size(), totalBones, unresolvedBones);
		for (std::map<sint32, uint>::iterator it = unresolvedVals.begin();
		     it != unresolvedVals.end(); ++it)
			fprintf(stderr, " [%d]x%u", (int)it->first, it->second);
		fprintf(stderr, "\n");
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// Skeleton bone map

INode *skeletonRootOf(INode *bone)
{
	if (!bone) return nullptr;
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
	CSceneClass *physMod = nullptr;
	CStorageContainer *physApp = nullptr;
	for (uint i = 0; i < mods.size(); ++i)
	{
		if (isPhysiqueModifier(mods[i]))
		{
			physMod = mods[i];
			physApp = (i < modApps.size()) ? modApps[i] : nullptr;
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
	INode *skelRoot = nullptr;
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
		// (Rigid one-link records already arrive with weight forced to 1.0 by the decode —
		// see decodePhysiqueWeights' single-link rule.)
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
	if (getenv("PMB_SKIN_DUMP_WEIGHTS"))
	{
		// Per INPUT vertex (pre-MRM): position + final normalized weights by bone name — the
		// position-matched oracle against the reference shape's finest-LOD VB.
		for (uint v = 0; v < vertWeights.size() && v < buildMesh.Vertices.size(); ++v)
		{
			const NLMISC::CVector &p = buildMesh.Vertices[v];
			const NL3D::CMesh::CSkinWeight &sw = buildMesh.SkinWeights[v];
			fprintf(stderr, "SKINW v%u pos(%.9g %.9g %.9g)", v, p.x, p.y, p.z);
			for (uint i = 0; i < NL3D_MESH_SKINNING_MAX_MATRIX; ++i)
				if (sw.Weights[i] > 0.f && sw.MatrixId[i] < bonesNames.size())
					fprintf(stderr, " '%s'=%.6g", bonesNames[sw.MatrixId[i]].c_str(), sw.Weights[i]);
			fprintf(stderr, "\n");
		}
	}
	(void)node; // reserved for future Skin-modifier path / diagnostics
	return true;
}

} /* namespace PHYSIQUESKIN */

/* end of file */
