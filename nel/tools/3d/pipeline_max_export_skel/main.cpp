// Skel export: .max -> .skel, replicating the NelExportSkeleton path of the 3ds Max plugin
// (build_gamedata processes/skel) without 3ds Max.
// Reads Bip01, walks children in scene order, and emits the .skel binary in the same format
// NeL's CShapeStream + CSkeletonShape + CBoneBase produce (including skeleton LODs built from
// the NEL3D_APPDATA_BONE_LOD_DISTANCE AppData, same algorithm as CSkeletonShape::build).
// Non-biped bones take their local transforms from the PRS controller's Bezier Position /
// TCB Rotation / Bezier Scale sub-controllers (default values at chunks 0x2503/0x2504/0x2505);
// biped bones are reconstructed from the figure-mode records on their Biped (0x9155) system
// object (see the reconstruction section below and pipeline_max_design.md).

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>
#include <nel/misc/matrix.h>

#include <algorithm>
#include <cmath>
#include <cstring>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/biped/biped_driven.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::BIPED;

#include "../pipeline_max_rig/biped_rig.h"

using namespace PMAX_RIG;

// Emit the MAXScript fragment (one parenthesized block per file) that regenerates this file's
// biped in Max 9. Uses helper functions (S2 / regenFinalize) provided once by the driver's header
// (gen_biped_regen.py). Returns false (with a SKIP comment written) when regeneration isn't
// supported for this file (no biped, or multiple biped systems).
static bool writeMaxscriptFragment(FILE *fp, const std::string &baseName)
{
	// exactly one biped system supported
	if (g_bipedRigs.size() != 1)
	{
		fprintf(fp, "-- SKIP %s: %zu biped systems (only single-biped regeneration supported)\n\n",
		        baseName.c_str(), g_bipedRigs.size());
		return false;
	}
	SBipedRig &rig = g_bipedRigs.begin()->second;
	g_rig = &rig;

	// --- structure counts from the captured id/link table (left side; right mirrors) ---
	int spineLinks = 0, neckLinks = 0, tailLinks = 0, pony1Links = 0, pony2Links = 0;
	int maxLegLinkSeen = -1;
	bool arms = false, prop1 = false, prop2 = false, prop3 = false;
	sint32 comIdx = -1, clavIdx = -1, thighIdx = -1;
	std::string rootName = "Bip01";
	for (size_t i = 0; i < g_msBones.size(); ++i)
	{
		const SMsBone &mb = g_msBones[i];
		if (mb.IsCom) { comIdx = (sint32)i; rootName = mb.Name; }
		if (!mb.IsBiped) continue;
		switch (mb.Id)
		{
		case BID_SPINE: ++spineLinks; break;
		case BID_NECK: ++neckLinks; break;
		case BID_TAIL: ++tailLinks; break;
		case BID_PONY1: ++pony1Links; break;
		case BID_PONY2: ++pony2Links; break;
		case BID_LARM: arms = true; if (mb.Link == 0) clavIdx = (sint32)i; break;
		case BID_LLEG: maxLegLinkSeen = std::max(maxLegLinkSeen, (int)mb.Link); if (mb.Link == 0) thighIdx = (sint32)i; break;
		case BID_PROP1: prop1 = true; break;
		case BID_PROP2: prop2 = true; break;
		case BID_PROP3: prop3 = true; break;
		default: break;
		}
	}
	int legLinks = (maxLegLinkSeen >= 0) ? (maxLegLinkSeen + 1) : 3;
	int fingers = (int)rig.Fingers[0].size();
	int fingerLinks = 0;
	for (size_t i = 0; i < rig.Fingers[0].size(); ++i) fingerLinks = std::max(fingerLinks, rig.Fingers[0][i].NLinks);
	int toes = (int)rig.Toes[0].size();
	int toeLinks = 0;
	for (size_t i = 0; i < rig.Toes[0].size(); ++i) toeLinks = std::max(toeLinks, rig.Toes[0][i].NLinks);
	if (!fingerLinks) fingerLinks = 1;
	if (!toeLinks) toeLinks = 1;

	// height: 0x000c stores height * 0.11325325 (dataset: 1.83 -> 0.207253, 1.2 -> 0.135904,
	// 2.4 -> 0.271807). ankleAttach: 0x000f ([8], [9]) split the foot length as a/(a+b).
	float height = 1.8f;
	const float *hc = bipedChunkFloats(0x000c, 1);
	if (hc) height = hc[0] / 0.11325325f;
	float ankleAttach = 0.0f;
	const float *lf = bipedChunkFloats(0x000f, 10);
	if (lf && (lf[8] + lf[9]) > 1e-9f) ankleAttach = lf[8] / (lf[8] + lf[9]);

	// triangle flags from node parenting: thigh under Pelvis => trianglePelvis; clavicle NOT
	// under a neck link => triangleNeck.
	bool trianglePelvis = true, triangleNeck = false;
	if (thighIdx >= 0 && g_msBones[thighIdx].FatherIdx >= 0)
	{
		const SMsBone &fa = g_msBones[g_msBones[thighIdx].FatherIdx];
		trianglePelvis = (fa.IsBiped && fa.Id == BID_PELVIS);
	}
	if (clavIdx >= 0 && g_msBones[clavIdx].FatherIdx >= 0)
	{
		const SMsBone &fa = g_msBones[g_msBones[clavIdx].FatherIdx];
		triangleNeck = !(fa.IsBiped && fa.Id == BID_NECK);
	}

	NLMISC::CVector wpos = (comIdx >= 0) ? g_msBones[comIdx].WorldPos : NLMISC::CVector::Null;

	fprintf(fp, "-- FILE %s\n(\n", baseName.c_str());
	fprintf(fp, "  resetMaxFile #noPrompt\n");
	// MAXScript parses "f 1.8 -90.0" as a subtraction — every positional numeric literal that can
	// be negative gets its own parentheses.
	fprintf(fp, "  local com = biped.createNew %.9g (-90.0) [%.9g,%.9g,%.9g] \\\n", height, wpos.x, wpos.y, wpos.z);
	fprintf(fp, "      arms:%s neckLinks:%d spineLinks:%d legLinks:%d \\\n",
	        arms ? "true" : "false", std::max(neckLinks, 1), std::max(spineLinks, 1), legLinks);
	fprintf(fp, "      tailLinks:%d ponytail1Links:%d ponytail2Links:%d \\\n", tailLinks, pony1Links, pony2Links);
	fprintf(fp, "      fingers:%d fingerLinks:%d toes:%d toeLinks:%d \\\n", fingers, fingerLinks, toes, toeLinks);
	fprintf(fp, "      ankleAttach:%.9g trianglePelvis:%s triangleNeck:%s \\\n",
	        ankleAttach, trianglePelvis ? "true" : "false", triangleNeck ? "true" : "false");
	fprintf(fp, "      prop1Exists:%s prop2Exists:%s prop3Exists:%s forearmTwistLinks:0\n",
	        prop1 ? "true" : "false", prop2 ? "true" : "false", prop3 ? "true" : "false");
	fprintf(fp, "  com.transform.controller.rootName = \"%s\"\n", rootName.c_str());
	fprintf(fp, "  com.transform.controller.figureMode = true\n");
	// Leg-chain sizes and pelvis width are NOT rubber-banded by biped.setTransform #pos (unlike
	// the arm/spine chains), so v1/v2 regen rigs came out with grossly mis-sized thighs/calves
	// and too-narrow pelves. Force them via figure-mode scale first: pelvis width from the thigh
	// side offset, per-bone lengths from the decoded child distances (SW/SL helpers in the
	// driver header measure the current value in-scene and apply the ratio).
	{
		// per-side leg link node indices (0=thigh .. maxLegLink=foot; 4-link mounts included)
		// + first toe base per side (foot length forcing: ankle -> toe attach)
		sint32 leg[2][8];
		sint32 toe0[2] = { -1, -1 };
		for (int s2 = 0; s2 < 2; ++s2)
			for (int l = 0; l < 8; ++l)
				leg[s2][l] = -1;
		for (size_t i = 0; i < g_msBones.size(); ++i)
		{
			const SMsBone &mb = g_msBones[i];
			if (!mb.IsBiped || mb.Link >= 8) continue;
			if (mb.Id == BID_LLEG) leg[0][mb.Link] = (sint32)i;
			else if (mb.Id == BID_RLEG) leg[1][mb.Link] = (sint32)i;
			else if (mb.Id == BID_LTOES && mb.Link == 0) toe0[0] = (sint32)i;
			else if (mb.Id == BID_RTOES && mb.Link == 0) toe0[1] = (sint32)i;
		}
		// Two passes: the pelvis rescale shifts the leg chain, so lengths re-measure and
		// re-apply (SL/SW measure the current in-scene value each call).
		fprintf(fp, "  for spass = 1 to 2 do (\n");
		// pelvis width = distance from COM to a thigh (decoded thigh side offset)
		if (comIdx >= 0 && leg[0][0] >= 0)
			fprintf(fp, "    SW com \"%s\" \"%s\" (%.9g)\n", rootName.c_str(), g_msBones[leg[0][0]].Name.c_str(),
			        (g_msBones[leg[0][0]].WorldPos - g_msBones[comIdx].WorldPos).norm());
		// per-link lengths from the decoded child distances, both sides, all leg links
		for (int s2 = 0; s2 < 2; ++s2)
		{
			int lastLeg = -1;
			for (int l = 0; l + 1 < 8; ++l)
			{
				if (leg[s2][l] < 0 || leg[s2][l + 1] < 0) continue;
				lastLeg = l + 1;
				fprintf(fp, "    SL com \"%s\" \"%s\" (%.9g)\n",
				        g_msBones[leg[s2][l]].Name.c_str(), g_msBones[leg[s2][l + 1]].Name.c_str(),
				        (g_msBones[leg[s2][l + 1]].WorldPos - g_msBones[leg[s2][l]].WorldPos).norm());
			}
			// foot length: ankle (last leg link) -> first toe base
			if (lastLeg >= 0 && leg[s2][lastLeg] >= 0 && toe0[s2] >= 0)
				fprintf(fp, "    SL com \"%s\" \"%s\" (%.9g)\n",
				        g_msBones[leg[s2][lastLeg]].Name.c_str(), g_msBones[toe0[s2]].Name.c_str(),
				        (g_msBones[toe0[s2]].WorldPos - g_msBones[leg[s2][lastLeg]].WorldPos).norm());
		}
		fprintf(fp, "  )\n");
	}
	// Two passes: rubber-banding a child position rescales its parent, which shifts already-set
	// grandchildren; the second pass converges the chain.
	fprintf(fp, "  for pass = 1 to 2 do (\n");
	for (size_t i = 0; i < g_msBones.size(); ++i)
	{
		const SMsBone &mb = g_msBones[i];
		if (!mb.IsCom && !mb.IsBiped) continue;              // PRS markers aren't biped-creatable
		if (mb.IsBiped && mb.Id >= BID_RFINGERNUB) continue; // end-effector dummies follow parents
		if (mb.IsBiped && (mb.Id == BID_VERTICAL || mb.Id == BID_HORIZONTAL || mb.Id == BID_TURN || mb.Id == BID_FOOTPRINTS)) continue;
		// MAXScript quats are the conjugate of the NeL convention.
		fprintf(fp, "    S2 com \"%s\" [%.9g,%.9g,%.9g] (quat (%.9g) (%.9g) (%.9g) (%.9g))\n",
		        mb.Name.c_str(), mb.WorldPos.x, mb.WorldPos.y, mb.WorldPos.z,
		        -mb.WorldRot.x, -mb.WorldRot.y, -mb.WorldRot.z, mb.WorldRot.w);
	}
	fprintf(fp, "  )\n");
	fprintf(fp, "  regenFinalize com \"%s\"\n", baseName.c_str());
	fprintf(fp, ")\n\n");
	return true;
}

// Dump the walked bones' figure-mode WORLD transforms in the biped_regen manifest.txt line format
// (biped.getTransform ground truth from Max 9): one BONE line per walked node, positions in world
// Z-up meters, rotations as MAXScript quats (the conjugate of the NeL convention). id/link are the
// MaxScript-facing 1-based pair for biped bones, 0/0 otherwise (the harness matches by name).
// This is the decode side of the encode-direction cross-validation: run against a regenerated
// fresh-format .max and diff against the manifest entry Max wrote for the same rig.
static void writeManifestDump(FILE *fp, const std::string &baseName)
{
	fprintf(fp, "STATE\t%s\n", baseName.c_str());
	for (size_t i = 0; i < g_msBones.size(); ++i)
	{
		const SMsBone &mb = g_msBones[i];
		uint32 id1 = mb.IsBiped ? (mb.Id + 1) : 0;
		uint32 link1 = mb.IsBiped ? (mb.Link + 1) : 0;
		fprintf(fp, "  BONE\t%s\tid\t%u\tlink\t%u\tpos\t%.9g,%.9g,%.9g\trot\t%.9g,%.9g,%.9g,%.9g\tbiped\t%d\tcom\t%d\tfather\t%s\n",
		        mb.Name.c_str(), id1, link1,
		        mb.WorldPos.x, mb.WorldPos.y, mb.WorldPos.z,
		        -mb.WorldRot.x, -mb.WorldRot.y, -mb.WorldRot.z, mb.WorldRot.w,
		        mb.IsBiped ? 1 : 0, mb.IsCom ? 1 : 0,
		        (mb.FatherIdx >= 0 && (size_t)mb.FatherIdx < g_msBones.size()) ? g_msBones[(size_t)mb.FatherIdx].Name.c_str() : "-");
	}
}

// Debug dump of every raw chunk on each Biped (0x9155) system object encountered during the walk:
// chunk id, byte size, and the payload as floats (with the uint32 bit pattern alongside where the
// float looks like a bit-stored int). Triage aid for record decode work (fresh-format analysis).
static void writeRigDump(FILE *fp)
{
	int rigIdx = 0;
	for (std::map<CSceneClass *, SBipedRig>::iterator it = g_bipedRigs.begin(); it != g_bipedRigs.end(); ++it, ++rigIdx)
	{
		fprintf(fp, "RIG %d figver %d\n", rigIdx, it->second.FigureVersion);
		CSceneClass *sys = it->first;
		// after parse, everything unclaimed sits in orphanedChunks; m_Chunks may hold the rest
		std::vector<std::pair<uint16, IStorageObject *> > all;
		for (auto c = sys->orphanedChunks().begin(); c != sys->orphanedChunks().end(); ++c) all.push_back(*c);
		for (auto c = sys->chunks().begin(); c != sys->chunks().end(); ++c) all.push_back(*c);
		for (size_t k = 0; k < all.size(); ++k)
		{
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(all[k].second);
			if (!raw) { fprintf(fp, "CHUNK 0x%04x (non-raw)\n", all[k].first); continue; }
			size_t nb = raw->Value.size();
			fprintf(fp, "CHUNK 0x%04x bytes %zu\n", all[k].first, nb);
			const uint8 *d = raw->Value.data();
			for (size_t i = 0; i + 4 <= nb; i += 4)
			{
				float f; uint32 u;
				memcpy(&f, d + i, 4);
				memcpy(&u, d + i, 4);
				if (u != 0 && u < 0x10000) // small bit-stored int (counts, flags)
					fprintf(fp, "  [%zu] %.9g (int %u)\n", i / 4, (double)f, u);
				else
					fprintf(fp, "  [%zu] %.9g\n", i / 4, (double)f);
			}
		}
	}
}

// Serialize a CSkeletonShape file (SHAP magic + CShapeStream + CSkeletonShape v1 + CBoneBase v2 + CLod v0).
static void writeSkel(const std::string &path, const std::vector<Bone> &bonesIn)
{
	// CSkeletonShape::build semantics, reproduced exactly (nel/src/3d/skeleton_shape.cpp):
	// 1. A bone must be LOD-disabled no later than its father: inherit/clamp LodDisableDistance.
	// 2. One lod per distinct non-zero distance (ascending) + the base lod; a lod disables every
	//    bone whose distance is non-zero and <= the lod's distance.
	std::vector<Bone> bones = bonesIn;
	for (size_t i = 0; i < bones.size(); ++i)
	{
		sint32 fa = bones[i].FatherId;
		if (fa >= 0 && bones[(size_t)fa].LodDisableDistance != 0.0f)
		{
			float fatherDist = bones[(size_t)fa].LodDisableDistance;
			if (bones[i].LodDisableDistance == 0.0f) bones[i].LodDisableDistance = fatherDist;
			else bones[i].LodDisableDistance = std::min(bones[i].LodDisableDistance, fatherDist);
		}
	}
	std::set<float> distSet;
	for (size_t i = 0; i < bones.size(); ++i)
		if (bones[i].LodDisableDistance > 0.0f)
			distSet.insert(bones[i].LodDisableDistance);

	NLMISC::COFile of(path);
	// SHAP magic — serialCheck writes NELID("PAHS") which is "SHAP" big-endian
	of.serialCheck(NELID("PAHS"));

	// serialPolyPtr for CSkeletonShape: write u64 id=1 (first ptr encountered), then class name.
	uint64 nodeId = 1;
	of.serial(nodeId);
	std::string className = "CSkeletonShape";
	of.serial(className);

	// CSkeletonShape::serial: serialVersion(1) + serialCont(_Bones) + serialCont(_BoneMap) + serialCont(_Lods)
	uint8 skelVersion = 1;
	of.serial(skelVersion);

	// _Bones: sint32 count + each CBoneBase
	sint32 boneCount = (sint32)bones.size();
	of.serial(boneCount);
	for (const Bone &b : bones)
	{
		uint8 boneVer = 2;
		of.serial(boneVer);
		std::string name = b.Name;
		of.serial(name);
		NLMISC::CMatrix invBind = b.InvBindPos;
		invBind.serial(of);
		sint32 father = b.FatherId;
		of.serial(father);
		bool unherit = b.UnheritScale;
		of.serial(unherit);
		float lodDist = b.LodDisableDistance;
		of.serial(lodDist);
		// CTrackDefaultVector DefaultPos: version(0) + CVector
		uint8 tVer = 0;
		of.serial(tVer); NLMISC::CVector v = b.DefaultPos; of.serial(v);
		of.serial(tVer); NLMISC::CVector euler = NLMISC::CVector::Null; of.serial(euler); // DefaultRotEuler
		of.serial(tVer); NLMISC::CQuat q = b.DefaultRotQuat; of.serial(q);
		of.serial(tVer); NLMISC::CVector s = b.DefaultScale; of.serial(s);
		of.serial(tVer); NLMISC::CVector pivot = NLMISC::CVector::Null; of.serial(pivot);
		// SkinScale (ver>=2)
		NLMISC::CVector skinScale(1, 1, 1);
		of.serial(skinScale);
	}

	// _BoneMap: std::map<string, uint32>, iterated in std::map's sorted key order (not bone
	// insertion order — that was a bug that manifested as a mismatch in the _BoneMap byte
	// range against mesh_export's output, which uses CSkeletonShape::build's actual std::map).
	sint32 mapCount = (sint32)bones.size();
	of.serial(mapCount);
	std::map<std::string, uint32> boneMap;
	for (uint32 i = 0; i < bones.size(); ++i) boneMap[bones[i].Name] = i;
	for (auto it = boneMap.begin(); it != boneMap.end(); ++it)
	{
		std::string n = it->first;
		of.serial(n);
		uint32 idx = it->second;
		of.serial(idx);
	}

	// _Lods: base lod (all bones active) + one lod per distinct LodDisableDistance
	sint32 lodCount = (sint32)(1 + distSet.size());
	of.serial(lodCount);
	{
		uint8 lodVer = 0;
		of.serial(lodVer);
		float lodDistance = 0.0f;
		of.serial(lodDistance);
		sint32 activeCount = (sint32)bones.size();
		of.serial(activeCount);
		for (sint32 i = 0; i < activeCount; ++i)
		{
			uint8 active = 0xFF;
			of.serial(active);
		}
	}
	for (std::set<float>::iterator it = distSet.begin(); it != distSet.end(); ++it)
	{
		uint8 lodVer = 0;
		of.serial(lodVer);
		float lodDistance = *it;
		of.serial(lodDistance);
		sint32 activeCount = (sint32)bones.size();
		of.serial(activeCount);
		for (sint32 i = 0; i < activeCount; ++i)
		{
			float dist = bones[(size_t)i].LodDisableDistance;
			uint8 active = (lodDistance >= dist && dist != 0.0f) ? 0x00 : 0xFF;
			of.serial(active);
		}
	}
}

// Format a float for JSON such that assimp's parser sees it as a floating-point number and
// not an integer. Assimp's glTF loader picks the smallest integer type first when the JSON
// token has no '.' or 'e', so we force decimal notation on integer-valued floats.
// %.9g round-trips float32 exactly (24-bit mantissa needs 9 decimal digits).
static std::string formatFloat(float v)
{
	char buf[32];
	if (std::isnan(v) || std::isinf(v))
	{
		// glTF spec forbids these in TRS values; use 0 to keep the file valid rather than
		// emitting an invalid token. Should not occur for real skeleton data.
		return "0.0";
	}
	snprintf(buf, sizeof(buf), "%.9g", (double)v);
	// Append ".0" if the printed form has no decimal marker so the reader picks a float type.
	if (!strchr(buf, '.') && !strchr(buf, 'e') && !strchr(buf, 'E'))
	{
		size_t n = strlen(buf);
		if (n + 2 < sizeof(buf)) { buf[n] = '.'; buf[n+1] = '0'; buf[n+2] = 0; }
	}
	return buf;
}

// Escape a name for embedding in JSON. Skeleton node names are typically simple identifiers
// (no quotes, backslashes, newlines), but escape defensively.
static std::string jsonEscape(const std::string &s)
{
	std::string out;
	out.reserve(s.size() + 2);
	for (char c : s)
	{
		switch (c)
		{
		case '"':  out += "\\\""; break;
		case '\\': out += "\\\\"; break;
		case '\n': out += "\\n";  break;
		case '\r': out += "\\r";  break;
		case '\t': out += "\\t";  break;
		default:
			if ((unsigned char)c < 0x20)
			{
				char buf[8]; snprintf(buf, sizeof(buf), "\\u%04x", (int)(unsigned char)c);
				out += buf;
			}
			else out += c;
		}
	}
	return out;
}

// Emit the skeleton as glTF 2.0. Skeleton-only — no meshes, no skins, no materials. Each bone
// becomes a node with its local translation/rotation/scale. Children lists are built from the
// bones' FatherId. mesh_export can walk this and reconstruct the same bone data using the
// same NeL CMatrix operations, producing a byte-identical .skel modulo glTF float rasterization.
//
// float precision: JSON stores numbers as doubles; we write with %.9g which round-trips a
// float32 exactly (float32 has ~7 decimal digits, 9 is enough).
//
// The root bone's DefaultPos/DefaultRotQuat are already zeroed by walkNode's "if fatherId<0"
// branch (NeL buildSkeleton convention). If the downstream reader needs the REAL root
// transform, it would have to be encoded separately — we don't need it for skel round-trip
// since our .skel writer applies the same reset.
static void writeGltf(const std::string &path, const std::vector<Bone> &bones)
{
	// Build per-parent children lists
	std::vector<std::vector<size_t> > children(bones.size());
	for (size_t i = 0; i < bones.size(); ++i)
		if (bones[i].FatherId >= 0)
			children[(size_t)bones[i].FatherId].push_back(i);

	FILE *fp = fopen(path.c_str(), "w");
	if (!fp) { std::cerr << "cannot open " << path << " for writing\n"; return; }

	fprintf(fp,
		"{\n"
		"  \"asset\": {\"version\": \"2.0\", \"generator\": \"pipeline_max_export_skel\"},\n"
		"  \"scene\": 0,\n"
		"  \"scenes\": [{\"nodes\": [0]}],\n"
		"  \"nodes\": [\n");

	for (size_t i = 0; i < bones.size(); ++i)
	{
		const Bone &b = bones[i];
		fprintf(fp, "    {");
		fprintf(fp, "\"name\": \"%s\"", jsonEscape(b.Name).c_str());
		// Use the REAL local transform (OrigPos/OrigRot), NOT the reset DefaultPos/DefaultRotQuat.
		// The root reset is applied by the .skel writer, and mesh_export re-applies it on its
		// side; feeding reset values through glTF would zero out the source's InvBindPos noise
		// pattern and give a false byte-diff between the two paths.
		fprintf(fp, ", \"translation\": [%s, %s, %s]",
			formatFloat(b.OrigPos.x).c_str(), formatFloat(b.OrigPos.y).c_str(), formatFloat(b.OrigPos.z).c_str());
		fprintf(fp, ", \"rotation\": [%s, %s, %s, %s]",
			formatFloat(b.OrigRot.x).c_str(), formatFloat(b.OrigRot.y).c_str(),
			formatFloat(b.OrigRot.z).c_str(), formatFloat(b.OrigRot.w).c_str());
		fprintf(fp, ", \"scale\": [%s, %s, %s]",
			formatFloat(b.DefaultScale.x).c_str(), formatFloat(b.DefaultScale.y).c_str(), formatFloat(b.DefaultScale.z).c_str());
		if (!children[i].empty())
		{
			fprintf(fp, ", \"children\": [");
			for (size_t k = 0; k < children[i].size(); ++k)
				fprintf(fp, "%s%zu", k ? ", " : "", children[i][k]);
			fprintf(fp, "]");
		}
		// Extras: NeL-specific per-node data that either isn't representable in stock glTF (the
		// two convertMatrix flags) or has to be preserved bit-exact for our .skel roundtrip
		// (the T/R/S floats). See wiki: nel_gltf_extras.md for the full catalogue.
		//
		// Rationale for nel_tx/ty/... instead of nel_translation: assimp maps JSON arrays into
		// aiMetadata differently across versions, and mapping scalar JSON numbers to typed
		// metadata entries is the most portable path. Cost is a few extra keys per node.
		fprintf(fp, ", \"extras\": {");
		fprintf(fp, "\"nel_tx\": %s, \"nel_ty\": %s, \"nel_tz\": %s",
			formatFloat(b.OrigPos.x).c_str(), formatFloat(b.OrigPos.y).c_str(), formatFloat(b.OrigPos.z).c_str());
		fprintf(fp, ", \"nel_rx\": %s, \"nel_ry\": %s, \"nel_rz\": %s, \"nel_rw\": %s",
			formatFloat(b.OrigRot.x).c_str(), formatFloat(b.OrigRot.y).c_str(),
			formatFloat(b.OrigRot.z).c_str(), formatFloat(b.OrigRot.w).c_str());
		fprintf(fp, ", \"nel_sx\": %s, \"nel_sy\": %s, \"nel_sz\": %s",
			formatFloat(b.DefaultScale.x).c_str(), formatFloat(b.DefaultScale.y).c_str(), formatFloat(b.DefaultScale.z).c_str());
		fprintf(fp, ", \"nel_unheritScale\": %s", b.UnheritScale ? "true" : "false");
		fprintf(fp, ", \"nel_lodDisableDistance\": %s", formatFloat(b.LodDisableDistance).c_str());
		fprintf(fp, "}");
		fprintf(fp, "}%s\n", (i + 1 < bones.size()) ? "," : "");
	}

	fprintf(fp,
		"  ]\n"
		"}\n");
	fclose(fp);
}

int main(int argc, char **argv)
{
	// Args: [--double] [--gltf <path>] [--allow-biped-degraded] <input.max> <output.skel>
	//   --double                 world-matrix accumulation in double instead of float
	//   --gltf <path>            also write a glTF 2.0 skeleton-only file next to the .skel; used for
	//                            the mesh_export roundtrip validator (Blender-importable,
	//                            assimp-readable)
	//   --allow-biped-degraded   proceed on biped files with identity local transforms + a warning
	//                            (bind pose is stored inside proprietary Biped controller chunks and
	//                            we don't yet decode them, so the output has correct names + hierarchy
	//                            but wrong per-bone InvBindPos); default is to refuse with an error so
	//                            silent-broken outputs don't slip into the corpus.
	int argi = 1;
	const char *gltfOut = NULL;
	const char *maxscriptOut = NULL;
	const char *manifestOut = NULL;
	const char *rigDumpOut = NULL;
	bool allowBipedDegraded = false;
	while (argi < argc && argv[argi][0] == '-' && argv[argi][1] == '-')
	{
		if (std::string(argv[argi]) == "--double") { g_useDouble = true; ++argi; }
		else if (std::string(argv[argi]) == "--gltf" && argi + 1 < argc) { gltfOut = argv[argi + 1]; argi += 2; }
		else if (std::string(argv[argi]) == "--maxscript" && argi + 1 < argc) { maxscriptOut = argv[argi + 1]; argi += 2; }
		else if (std::string(argv[argi]) == "--manifest" && argi + 1 < argc) { manifestOut = argv[argi + 1]; argi += 2; }
		else if (std::string(argv[argi]) == "--dump-rig" && argi + 1 < argc) { rigDumpOut = argv[argi + 1]; argi += 2; }
		else if (std::string(argv[argi]) == "--allow-biped-degraded") { allowBipedDegraded = true; ++argi; }
		else break;
	}
	if (argc - argi < 2) { std::cerr << "usage: export_skel [--double] [--gltf <path>] [--allow-biped-degraded] <input.max> <output.skel>\n"; return 1; }
	const char *maxFile = argv[argi];
	const char *skelOut = argv[argi + 1];

	g_set_prgname(argv[0]);
	gsf_init();

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);

	GsfInput *src = gsf_input_stdio_new(maxFile, NULL);
	if (!src) { std::cerr << "cannot open " << maxFile << "\n"; return 1; }
	GsfInfile *in = gsf_infile_msole_new(src, NULL);
	if (!in) { std::cerr << "not an OLE file\n"; return 1; }

	CDllDirectory dll;
	{ GsfInput *s = gsf_infile_child_by_name(in, "DllDirectory"); CStorageStream st(s); dll.serial(st); g_object_unref(s); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ GsfInput *s = gsf_infile_child_by_name(in, "ClassDirectory3"); CStorageStream st(s); cd.serial(st); g_object_unref(s); }
	cd.parse(VersionUnknown);

	bool isBiped = looksLikeBipedFile(cd);
	(void)allowBipedDegraded; // legacy flag, now a no-op — biped bind pose is reconstructed
	CScene scene(&reg, &dll, &cd);
	{ GsfInput *s = gsf_infile_child_by_name(in, "Scene"); CStorageStream st(s); scene.serial(st); g_object_unref(s); }
	scene.parse(VersionUnknown);

	INode *root = scene.container()->scene()->rootNode();
	INode *bip01 = root->find(ucstring("Bip01"));
	if (!bip01) { std::cerr << "Bip01 not found in " << maxFile << "\n"; return 2; }

	// Per-system rig state (COM, per-limb records) is parsed lazily during the walk: each biped
	// bone's TM controller references its own Biped (0x9155) system object as getReference(0),
	// which handles files with multiple bipeds (e.g. tr_mo_kitin_queen's Bip01 + Bip02) correctly.
	g_bipedRigs.clear();
	g_rig = NULL;
	g_msBones.clear();

	std::vector<Bone> bones;
	std::set<std::string> nameSet;
	if (g_useDouble && !isBiped)
	{
		Mat4D rootMatD = Mat4D::identity();
		walkNodeD(bip01, -1, rootMatD, scene.container(), bones, nameSet);
	}
	else
	{
		NLMISC::CMatrix rootMat; rootMat.identity();
		walkNode(bip01, -1, rootMat, scene.container(), bones, nameSet);
		patchFootstepsGround(bones);
	}

	std::cout << "Extracted " << bones.size() << " bones from " << maxFile << "\n";
	for (size_t i = 0; i < bones.size() && i < 10; ++i)
	{
		std::cout << "  [" << i << "] " << bones[i].Name << " father=" << bones[i].FatherId
		          << " pos=" << bones[i].DefaultPos.toString()
		          << " rotQ=(" << bones[i].DefaultRotQuat.x << "," << bones[i].DefaultRotQuat.y << "," << bones[i].DefaultRotQuat.z << "," << bones[i].DefaultRotQuat.w << ")"
		          << " scale=" << bones[i].DefaultScale.toString()
		          << "\n";
	}

	writeSkel(skelOut, bones);
	std::cout << "Wrote " << skelOut << "\n";

	if (gltfOut)
	{
		writeGltf(gltfOut, bones);
		std::cout << "Wrote " << gltfOut << "\n";
	}

	if (maxscriptOut || manifestOut)
	{
		std::string base = maxFile;
		std::string::size_type slash = base.find_last_of("/\\");
		if (slash != std::string::npos) base = base.substr(slash + 1);
		std::string::size_type dot = base.rfind('.');
		if (dot != std::string::npos) base = base.substr(0, dot);
		if (maxscriptOut)
		{
			FILE *fp = fopen(maxscriptOut, "w");
			if (!fp) { std::cerr << "cannot open " << maxscriptOut << " for writing\n"; return 1; }
			bool ok = writeMaxscriptFragment(fp, base);
			fclose(fp);
			std::cout << (ok ? "Wrote " : "Skipped (see comment) ") << maxscriptOut << "\n";
		}
		if (manifestOut)
		{
			FILE *fp = fopen(manifestOut, "w");
			if (!fp) { std::cerr << "cannot open " << manifestOut << " for writing\n"; return 1; }
			writeManifestDump(fp, base);
			fclose(fp);
			std::cout << "Wrote " << manifestOut << "\n";
		}
	}

	if (rigDumpOut)
	{
		FILE *fp = fopen(rigDumpOut, "w");
		if (!fp) { std::cerr << "cannot open " << rigDumpOut << " for writing\n"; return 1; }
		writeRigDump(fp);
		fclose(fp);
		std::cout << "Wrote " << rigDumpOut << "\n";
	}

	g_object_unref(in);
	g_object_unref(src);
	gsf_shutdown();
	return 0;
}
