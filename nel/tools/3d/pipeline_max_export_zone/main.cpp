/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
// Zone export: .max -> .zone / .ligozone, replicating the ExportRykolZone and NeLLigoExportZone
// paths of the 3ds Max plugins (build_gamedata processes/zone and processes/ligo) without
// 3ds Max.
//
// The ligo maxscript (processes/ligo/maxscript/nel_ligo_export.ms) classifies files by name:
//   zonematerial-<mat>-<cell>.max   -> one brick: select the single non-frozen RklPatch,
//                                      NeLLigoExportZone -> zones/<mat>-<cell>.zone +
//                                      zoneLigos/<mat>-<cell>.ligozone
//   zonetransition-<a>-<b>-<t>.max  -> nine bricks: each non-frozen RklPatch is classified on
//                                      the transition scheme grid by its cell position, then
//                                      exported through a mirrored/rotated/translated instance
//                                      transform with the symmetry/rotate appdata set
//   zonespecial-<name>.max          -> one brick, material "special"
// Frozen RklPatch nodes are boundary-repetition references, never exported (frozen marker =
// empty node chunk 0x0976, established corpus-wide). The direct zone process
// (processes/zone/maxscript/zone_export.ms, ExportRykolZone) exports the first RklPatch in
// scene order with the zone id derived from the node name (findID).
//
// The plugin side both paths share is RPatchMesh::exportZone (plugin_max/nel_patch_lib/
// rpo2nel.cpp): PatchMesh+RPatchMesh (+node object TM at t=0 in Max float math) ->
// CPatchInfo[] -> optional CPatchInfo::transform (symmetry/rotate with the tile bank) ->
// CZone::build -> serial. This tool replicates the conversion on the storage-level data (see
// pipeline_max/nelpatch/) and links real NL3D/NLLIGO for everything downstream. NeL Edit Patch
// modifier stacks are evaluated from the modifier's per-node local data (final patch 0x1140 +
// RPatchMesh 0x4001 + vertex-mapper deltas 0x1130 applied against the base patch, replicating
// EditPatchData::Apply + EPVertMapper::UpdateAndApplyDeltas).
//
// The .zone is written as serial version 4 (the reference era; current CZone::serial writes 5,
// which differs from 4 only in the version byte itself — verified by roundtripping reference
// zones through the current serializer).

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

#include <nel/misc/types_nl.h>

// MSVC 9.0 (VS2008) has no C99 snprintf; _snprintf is equivalent for our fixed-size formatting.
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/path.h>

#include <nel/3d/register_3d.h>
#include <nel/3d/tile_bank.h>
#include <nel/3d/zone.h>
#include <nel/3d/zone_symmetrisation.h>

#include <nel/ligo/ligo_config.h>
#include <nel/ligo/ligo_error.h>
#include <nel/ligo/zone_bank.h>
#include <nel/ligo/zone_template.h>

#include "../pipeline_max/storage_ole.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/storage_value.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/control_transform.h"

#include "../pipeline_max_export_common/max_math.h"
#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/appdata_util.h"
#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::NELPATCH;
using namespace MAXMATH;

// Patch-state eval + RPO->CPatchInfo conversion: shared with the standalone zone painter.
// Header-only static implementation unit included at the exact position the code occupied
// here (zone x87 tier is TU-sensitive; see the header doc).
#include "../pipeline_max_export_common/patch_eval.h"

static bool g_verbose = false;

// ---------------------------------------------------------------------------------------------
// AppData access (same shape as the other exporters).

// Shared script AppData readers (pipeline_max_export_common/appdata_util) — formerly
// file-local copies here (the local Int variant parsed via sscanf; fromString is equivalent
// over the decimal-string values this convention stores).
using APPDATA::getScriptAppData;
using APPDATA::getScriptAppDataInt;

static bool hasScriptAppData(CSceneClass *sc, uint32 subId)
{
	std::string s;
	return getScriptAppData(sc, subId, s);
}

// ---------------------------------------------------------------------------------------------
// Zone writing: current CZone::serial writes version 5; the references are version 4, and the
// two encodings differ only in the version byte (verified by reference roundtrip). Serialize
// to memory and write the version byte as 4.

static bool writeZoneV4(NL3D::CZone &zone, const std::string &path)
{
	NLMISC::CMemStream mem;
	nlassert(!mem.isReading());
	zone.serial(mem);
	if (mem.length() < 1) return false;
	std::vector<uint8> buf(mem.buffer(), mem.buffer() + mem.length());
	if (buf[0] != 5)
	{
		fprintf(stderr, "WARNING: unexpected CZone serial version %u, version byte left untouched\n", buf[0]);
	}
	else
	{
		buf[0] = 4;
	}
	NLMISC::COFile f;
	if (!f.open(path)) return false;
	f.serialBuffer(nlVectorData(buf), (uint)buf.size());
	return true;
}

// ---------------------------------------------------------------------------------------------
// Ligozone: zone template mask (CMaxToLigo::buildZoneTemplate replication) or square mask, and
// the CZoneBankElement categories protocol of NeLLigoExportZone.

static bool buildZoneMask(const SEvalPatch &ep, const Matrix3M &objectTM, bool symmetry,
                          const NLLIGO::CLigoConfig &config, std::vector<bool> &mask, uint &width, uint &height,
                          std::string &err)
{
	// Vertices in world space
	std::vector<NLMISC::CVector> vertices(ep.Pm.Verts.size());
	for (size_t i = 0; i < ep.Pm.Verts.size(); ++i)
	{
		Point3M v = { ep.Pm.Verts[i].Pos[0], ep.Pm.Verts[i].Pos[1], ep.Pm.Verts[i].Pos[2] };
		v = transformPoint(v, objectTM);
		vertices[i].x = v.x;
		vertices[i].y = v.y;
		vertices[i].z = v.z;
	}
	// Open edges
	std::vector<std::pair<uint, uint> > indexes;
	for (size_t e = 0; e < ep.Pm.Edges.size(); ++e)
	{
		const SPmEdge &edge = ep.Pm.Edges[e];
		if (edge.Patches.size() < 2)
		{
			if (symmetry)
				indexes.push_back(std::pair<uint, uint>((uint)edge.V2, (uint)edge.V1));
			else
				indexes.push_back(std::pair<uint, uint>((uint)edge.V1, (uint)edge.V2));
		}
	}
	NLLIGO::CZoneTemplate zoneTemplate;
	NLLIGO::CLigoError errors;
	if (!zoneTemplate.build(vertices, indexes, config, errors))
	{
		err = NLMISC::toString("zone template build failed (main error %d)", (int)errors.MainError);
		return false;
	}
	zoneTemplate.getMask(mask, width, height);
	return true;
}

// getSquareMask replication (ligo plugin script.cpp): bounding box over the built zone's patch
// vertices, all cells filled.
static void buildSquareMask(const std::vector<NL3D::CPatchInfo> &patchinfo, float cellSize,
                            std::vector<bool> &mask, uint &width, uint &height)
{
	sint maxX = 1;
	sint maxY = 1;
	for (size_t i = 0; i < patchinfo.size(); ++i)
	{
		for (uint v = 0; v < 4; ++v)
		{
			sint positionX = (sint)((patchinfo[i].Patch.Vertices[v].x + cellSize / 2) / cellSize);
			sint positionY = (sint)((patchinfo[i].Patch.Vertices[v].y + cellSize / 2) / cellSize);
			if (positionX > maxX) maxX = positionX;
			if (positionY > maxY) maxY = positionY;
		}
	}
	width = (uint)maxX;
	height = (uint)maxY;
	mask.clear();
	mask.resize(width * height, true);
}

static std::string toLowerAsciiStr(const std::string &s)
{
	std::string r = s;
	for (size_t i = 0; i < r.size(); ++i)
		if (r[i] >= 'A' && r[i] <= 'Z') r[i] = r[i] - 'A' + 'a';
	return r;
}

static bool writeLigozone(const std::vector<std::pair<std::string, std::string> > &categoriesIn,
                          const std::vector<bool> &mask, uint width, uint height,
                          const std::string &path)
{
	std::vector<std::pair<std::string, std::string> > categories = categoriesIn;

	// Is filled ?
	uint j;
	for (j = 0; j < mask.size(); ++j)
		if (!mask[j]) break;
	categories.push_back(std::pair<std::string, std::string>("filled", (j >= mask.size()) ? "yes" : "no"));
	categories.push_back(std::pair<std::string, std::string>("square", (width == height) ? "yes" : "no"));
	categories.push_back(std::pair<std::string, std::string>("size", NLMISC::toString("%dx%d", width, height)));

	NLLIGO::CZoneBankElement bankElm;
	bankElm.setMask(mask, (uint8)width, (uint8)height);
	for (j = 0; j < categories.size(); ++j)
		bankElm.addCategory(toLowerAsciiStr(categories[j].first), toLowerAsciiStr(categories[j].second));

	NLMISC::COFile outputLigoZone;
	if (!outputLigoZone.open(path)) return false;
	try
	{
		NLMISC::COXml outputXml;
		outputXml.init(&outputLigoZone);
		bankElm.serial(outputXml);
		outputXml.flush();
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: writing %s: %s\n", path.c_str(), e.what());
		return false;
	}
	return true;
}


// ---------------------------------------------------------------------------------------------
// Transition scheme tables (nel_ligo_export.ms).

static const bool TransitionScale[9] = { false, false, false, false, true, false, false, false, false };
static const int TransitionRot[9]    = { 2, 1, 3, 0, 1, 3, 0, 0, 0 };
static const float TransitionPos[9][3] = {
	{ 0, 0, 0 }, { -1, 0, 0 }, { -1, -1, 0 }, { -1, -2, 0 }, { 0, -2, 0 },
	{ 0, -3, 0 }, { -1, -3, 0 }, { -2, -3, 0 }, { -3, -3, 0 }
};
// TransitionIds[y][x], -1 = undefined; y rows of variable length (x < len)
static const int TransitionIdsRow0[] = { 1, 2 };
static const int TransitionIdsRow1[] = { -1, 3 };
static const int TransitionIdsRow2[] = { 5, 4 };
static const int TransitionIdsRow3[] = { 6, 7, 8, 9 };
static const int *TransitionIds[4] = { TransitionIdsRow0, TransitionIdsRow1, TransitionIdsRow2, TransitionIdsRow3 };
static const int TransitionIdsLen[4] = { 2, 2, 2, 4 };
static const char *TransitionType[9] = { "CornerA", "CornerA", "Flat", "CornerA", "CornerB", "CornerB", "Flat", "Flat", "CornerB" };
static const int TransitionNumBis[9] = { 5, 4, 2, 3, 7, 6, 0, 1, 8 };

// buildTransitionMatrix (MAXScript float ops): zero the translation, optional mirror
// (post-multiply scale [-1,1,1]), optional rotateZ (90*rot degrees), then translate to
// TransitionPos*cellSize + original position.
static Matrix3M buildTransitionMatrix(const Matrix3M &mt, int transitionZone, float cellSize)
{
	Matrix3M copyMt = mt;
	float backupPos[3] = { copyMt.m[3][0], copyMt.m[3][1], copyMt.m[3][2] };
	copyMt.m[3][0] = copyMt.m[3][1] = copyMt.m[3][2] = 0.0f;

	if (TransitionScale[transitionZone])
	{
		// scale copyMt [-1,1,1]: post-multiply by diag(-1,1,1) -> negate column 0
		for (int i = 0; i < 4; ++i)
			copyMt.m[i][0] = -copyMt.m[i][0];
	}

	if (TransitionRot[transitionZone] != 0)
	{
		// rotateZ copyMt (90*rot): post-multiply by RotateZMatrix(degrees)
		double rad = (double)(90 * TransitionRot[transitionZone]) * 3.14159265358979323846 / 180.0;
		float c = (float)cos(rad);
		float s = (float)sin(rad);
		Matrix3M rz = Matrix3M::identity();
		rz.m[0][0] = c; rz.m[0][1] = s;
		rz.m[1][0] = -s; rz.m[1][1] = c;
		copyMt = copyMt * rz;
	}

	// translate copyMt (TransitionPos*cellSize + backupPos)
	copyMt.m[3][0] = TransitionPos[transitionZone][0] * cellSize + backupPos[0];
	copyMt.m[3][1] = TransitionPos[transitionZone][1] * cellSize + backupPos[1];
	copyMt.m[3][2] = TransitionPos[transitionZone][2] * cellSize + backupPos[2];
	return copyMt;
}

// Node world bbox center over the evaluated patch's control points (MAXScript node.center
// equivalent for the 160m cell classification).
static bool nodeCenter(const SEvalPatch &ep, const Matrix3M &objectTM, float center[3])
{
	if (ep.Pm.Verts.empty()) return false;
	float bbMin[3] = { 0, 0, 0 }, bbMax[3] = { 0, 0, 0 };
	bool first = true;
	for (size_t i = 0; i < ep.Pm.Verts.size(); ++i)
	{
		Point3M v = { ep.Pm.Verts[i].Pos[0], ep.Pm.Verts[i].Pos[1], ep.Pm.Verts[i].Pos[2] };
		v = transformPoint(v, objectTM);
		float p[3] = { v.x, v.y, v.z };
		for (int a = 0; a < 3; ++a)
		{
			if (first || p[a] < bbMin[a]) bbMin[a] = p[a];
			if (first || p[a] > bbMax[a]) bbMax[a] = p[a];
		}
		first = false;
	}
	for (int a = 0; a < 3; ++a)
		center[a] = (bbMin[a] + bbMax[a]) * 0.5f;
	return true;
}

// ---------------------------------------------------------------------------------------------
// Ligo protocol driver.

struct SLigoOutputs
{
	std::string ZonesDir;     // <out>/zones
	std::string ZoneLigosDir; // <out>/zoneligos
};

static bool exportLigoBrick(const SEvalPatch &ep, const Matrix3M &objectTM,
                            bool symmetry, int rotate, bool useBoundingBox, bool passable,
                            const std::vector<std::pair<std::string, std::string> > &categories,
                            const NLLIGO::CLigoConfig &config, SExportContext &ctx,
                            const SLigoOutputs &out, const std::string &name, std::string &err)
{
	// The zone
	std::vector<NL3D::CPatchInfo> patchinfo;
	if (!exportZoneToPatchInfo(ep, objectTM, 0, symmetry, rotate, config.CellSize, config.Snap, ctx, patchinfo, err))
		return false;

	// The mask
	std::vector<bool> mask;
	uint width, height;
	if (useBoundingBox)
	{
		buildSquareMask(patchinfo, config.CellSize, mask, width, height);
	}
	else
	{
		if (!buildZoneMask(ep, objectTM, symmetry, config, mask, width, height, err))
			return false;
	}

	// The .zone
	NL3D::CZone zone;
	zone.build(0, patchinfo, std::vector<NL3D::CBorderVertex>());
	std::string zonePath = out.ZonesDir + "/" + name + ".zone";
	if (!writeZoneV4(zone, zonePath)) { err = "cannot write " + zonePath; return false; }

	// The .ligozone
	std::vector<std::pair<std::string, std::string> > cats = categories;
	cats.push_back(std::pair<std::string, std::string>("passable", passable ? "yes" : "no"));
	std::string ligozonePath = out.ZoneLigosDir + "/" + name + ".ligozone";
	if (!writeLigozone(cats, mask, width, height, ligozonePath)) { err = "cannot write " + ligozonePath; return false; }

	if (g_verbose) printf("OK %s (%s)\n", zonePath.c_str(), ligozonePath.c_str());
	return true;
}

// tokenize a filename base by '-'
static void tokenize(const std::string &s, std::vector<std::string> &tokens)
{
	tokens.clear();
	std::string::size_type pos = 0;
	while (pos <= s.size())
	{
		std::string::size_type dash = s.find('-', pos);
		if (dash == std::string::npos) { tokens.push_back(s.substr(pos)); break; }
		tokens.push_back(s.substr(pos, dash - pos));
		pos = dash + 1;
	}
}

static int exportLigoFile(const std::string &inputBase, CScene &scene, const NLLIGO::CLigoConfig &config,
                          SExportContext &ctx, const SLigoOutputs &out)
{
	std::vector<std::string> tokens;
	tokenize(inputBase, tokens);

	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	SNodeTMCache tmCache;

	if (tokens.size() == 3 && tokens[0] == "zonematerial")
	{
		// Single brick: the one non-frozen patch.
		std::vector<SZoneNode> sel;
		for (size_t i = 0; i < nodes.size(); ++i)
			if (!nodes[i].Frozen) sel.push_back(nodes[i]);
		if (sel.size() > 1)
		{
			fprintf(stderr, "ERROR: %s: multiple NelPatchMesh (%u), can't export\n", inputBase.c_str(), (uint)sel.size());
			return 1;
		}
		if (sel.empty())
		{
			fprintf(stderr, "WARNING: %s: no NelPatchMesh to export\n", inputBase.c_str());
			return 0;
		}
		CNodeImpl *node = sel[0].Node;
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
			return 1;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		bool symmetry = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
		int rotate = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_ROTATE, 0);
		bool useBB = getScriptAppDataInt(node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, 0) != 0;
		bool passable = hasScriptAppData(node, NEL3D_APPDATA_LIGO_PASSABLE);
		std::vector<std::pair<std::string, std::string> > cats;
		cats.push_back(std::pair<std::string, std::string>("zone", tokens[1] + "-" + tokens[2]));
		cats.push_back(std::pair<std::string, std::string>("material", tokens[1]));
		if (!exportLigoBrick(ep, objectTM, symmetry, rotate, useBB, passable, cats, config, ctx, out,
		                     tokens[1] + "-" + tokens[2], err))
		{
			fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
			return 1;
		}
		return 0;
	}
	else if (tokens.size() == 4 && tokens[0] == "zonetransition")
	{
		// Nine bricks from the transition scheme grid.
		int rc = 0;
		// Classify the non-frozen patches on the grid.
		CNodeImpl *transitionZone[9] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			if (nodes[i].Frozen) continue;
			CNodeImpl *node = nodes[i].Node;
			SEvalPatch ep;
			std::string err;
			if (!evalNodePatch(node, ep, err))
			{
				fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
				return 1;
			}
			Matrix3M objectTM = getObjectTM(node, tmCache);
			float center[3];
			if (!nodeCenter(ep, objectTM, center))
			{
				fprintf(stderr, "ERROR: %s: empty patch\n", inputBase.c_str());
				return 1;
			}
			int x = (int)(center[0] / config.CellSize);
			int y = (int)(center[1] / config.CellSize);
			if (y < 0 || y >= 4 || x < 0 || x >= TransitionIdsLen[y] || TransitionIds[y][x] < 0)
			{
				fprintf(stderr, "ERROR: %s: node '%s' is not at a transition scheme position (cell %d,%d)\n",
				        inputBase.c_str(), ucstring(node->userName()).toUtf8().c_str(), x, y);
				return 1;
			}
			transitionZone[TransitionIds[y][x] - 1] = node;
		}
		for (int zone = 0; zone < 9; ++zone)
		{
			std::string zoneBaseName = tokens[1] + "-" + tokens[2] + "-" + tokens[3] + "-" + NLMISC::toString(zone);
			CNodeImpl *node = transitionZone[zone];
			if (!node) continue;
			SEvalPatch ep;
			std::string err;
			if (!evalNodePatch(node, ep, err))
			{
				fprintf(stderr, "ERROR: %s: %s\n", zoneBaseName.c_str(), err.c_str());
				rc = 1;
				continue;
			}
			// The transformed instance: node TM through the transition matrix; the appdata
			// symmetry/rotate the maxscript sets on the instance.
			Matrix3M nodeTM = getNodeTM(node, tmCache);
			Matrix3M instTM = buildTransitionMatrix(nodeTM, zone, config.CellSize);
			// objectTM = offset * instance node TM
			Matrix3M objectTM;
			{
				// re-derive the offset from the node, then compose with the transformed node TM
				SNodeTMCache dummy;
				// offsetTM * nodeTM == getObjectTM; extract offset by objectTM * inverse(nodeTM)
				Matrix3M objTM = getObjectTM(node, tmCache);
				Matrix3M offset = objTM * inverseM3(nodeTM);
				objectTM = offset * instTM;
			}
			bool symmetry = TransitionScale[zone];
			int rotate = TransitionRot[zone];
			bool useBB = getScriptAppDataInt(node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, 0) != 0;
			bool passable = hasScriptAppData(node, NEL3D_APPDATA_LIGO_PASSABLE);
			std::vector<std::pair<std::string, std::string> > cats;
			cats.push_back(std::pair<std::string, std::string>("zone", zoneBaseName));
			cats.push_back(std::pair<std::string, std::string>("transname", tokens[1] + "-" + tokens[2]));
			cats.push_back(std::pair<std::string, std::string>("transtype", TransitionType[zone]));
			cats.push_back(std::pair<std::string, std::string>("transnum", NLMISC::toString(TransitionNumBis[zone])));
			if (!exportLigoBrick(ep, objectTM, symmetry, rotate, useBB, passable, cats, config, ctx, out,
			                     zoneBaseName, err))
			{
				fprintf(stderr, "ERROR: %s: %s\n", zoneBaseName.c_str(), err.c_str());
				rc = 1;
			}
		}
		return rc;
	}
	else if (tokens.size() == 2 && tokens[0] == "zonespecial")
	{
		std::vector<SZoneNode> sel;
		for (size_t i = 0; i < nodes.size(); ++i)
			if (!nodes[i].Frozen) sel.push_back(nodes[i]);
		if (sel.size() > 1)
		{
			fprintf(stderr, "ERROR: %s: multiple NelPatchMesh (%u), can't export\n", inputBase.c_str(), (uint)sel.size());
			return 1;
		}
		if (sel.empty())
		{
			fprintf(stderr, "WARNING: %s: no NelPatchMesh to export\n", inputBase.c_str());
			return 0;
		}
		CNodeImpl *node = sel[0].Node;
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
			return 1;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		bool symmetry = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
		int rotate = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_ROTATE, 0);
		bool useBB = getScriptAppDataInt(node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, 0) != 0;
		bool passable = hasScriptAppData(node, NEL3D_APPDATA_LIGO_PASSABLE);
		std::vector<std::pair<std::string, std::string> > cats;
		cats.push_back(std::pair<std::string, std::string>("zone", tokens[1]));
		cats.push_back(std::pair<std::string, std::string>("material", "special"));
		if (!exportLigoBrick(ep, objectTM, symmetry, rotate, useBB, passable, cats, config, ctx, out,
		                     tokens[1], err))
		{
			fprintf(stderr, "ERROR: %s: %s\n", inputBase.c_str(), err.c_str());
			return 1;
		}
		return 0;
	}

	fprintf(stderr, "WARNING: %s: not a zonematerial/zonetransition/zonespecial file, nothing to do\n", inputBase.c_str());
	return 0;
}

// ---------------------------------------------------------------------------------------------
// Direct zone export (processes/zone, ExportRykolZone with findID).

static bool findID(const std::string &name, int &zoneId)
{
	// Node name "NUM_AB..." split by '_': NameTab[1] = number, NameTab[2] = two letters.
	std::vector<std::string> parts;
	std::string::size_type pos = 0;
	while (pos <= name.size())
	{
		std::string::size_type us = name.find('_', pos);
		if (us == std::string::npos) { parts.push_back(name.substr(pos)); break; }
		parts.push_back(name.substr(pos, us - pos));
		pos = us + 1;
	}
	if (parts.size() < 2 || parts[1].size() < 2) return false;
	char l1 = parts[1][0], l2 = parts[1][1];
	if (l1 < 'A' || l1 > 'Z' || l2 < 'A' || l2 > 'Z') return false;
	int num = 0;
	if (sscanf(parts[0].c_str(), "%d", &num) != 1) return false;
	int alphaSub = ((l1 - 'A') * 26 + (l2 - 'A' + 1)) - 1;
	zoneId = (num - 1) * 256 + alphaSub;
	return true;
}

static int exportDirectZone(CScene &scene, const std::string &outPath, SExportContext &ctx, int zoneIdOverride)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	SNodeTMCache tmCache;

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		CNodeImpl *node = nodes[i].Node;
		// ExportRykolZone: don't-export appdata check
		if (getScriptAppDataInt(node, NEL3D_APPDATA_DONOTEXPORT, 0)) continue;

		int zoneId = zoneIdOverride;
		if (zoneId < 0)
		{
			if (!findID(ucstring(node->userName()).toUtf8(), zoneId))
			{
				fprintf(stderr, "ERROR: cannot derive zone id from node name '%s'\n", ucstring(node->userName()).toUtf8().c_str());
				continue;
			}
		}

		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			fprintf(stderr, "ERROR: node '%s': %s\n", ucstring(node->userName()).toUtf8().c_str(), err.c_str());
			continue;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		bool symmetry = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
		int rotate = getScriptAppDataInt(node, NEL3D_APPDATA_ZONE_ROTATE, 0);
		std::vector<NL3D::CPatchInfo> patchinfo;
		if (!exportZoneToPatchInfo(ep, objectTM, zoneId, symmetry, rotate, 160.0f, 1.0f, ctx, patchinfo, err))
		{
			fprintf(stderr, "ERROR: node '%s': %s\n", ucstring(node->userName()).toUtf8().c_str(), err.c_str());
			continue;
		}
		NL3D::CZone zone;
		zone.build(zoneId, patchinfo, std::vector<NL3D::CBorderVertex>());
		if (!writeZoneV4(zone, outPath))
		{
			fprintf(stderr, "ERROR: cannot write %s\n", outPath.c_str());
			return 1;
		}
		if (g_verbose) printf("OK %s (zone id %d)\n", outPath.c_str(), zoneId);
		return 0;
	}
	fprintf(stderr, "WARNING: no zone found in project\n");
	return 2;
}

// ---------------------------------------------------------------------------------------------
// Debug: dump the RPatchMesh tile data of the exported node's patch 0..N.

static int dumpRpoTiles(CScene &scene, uint maxPatches)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Frozen) continue;
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(nodes[i].Node, ep, err)) { fprintf(stderr, "FAIL %s\n", err.c_str()); return 1; }
		for (uint p = 0; p < ep.Rp.Patches.size() && p < maxPatches; ++p)
		{
			const SRpoPatch &up = ep.Rp.Patches[p];
			printf("patch %u: %dx%d tiles\n", p, 1 << up.NbTilesU, 1 << up.NbTilesV);
			for (uint t = 0; t < up.Tiles.size(); ++t)
			{
				const SRpoTile &tl = up.Tiles[t];
				printf("  tile %u: num=%u flags=0x%04x noise=%u layers", t, tl.Num, tl.Flags, tl.Noise);
				for (int l = 0; l < 3; ++l)
					printf(" (%d,r%d)", tl.Layer[l].Tile, tl.Layer[l].Rotate);
				printf("\n");
			}
		}
		break;
	}
	return 0;
}

// Debug: inspect one patch of the first non-frozen node: vec ids + binds referencing them.
static int inspectPatch(CScene &scene, int patchIdx)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Frozen) continue;
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(nodes[i].Node, ep, err)) { fprintf(stderr, "FAIL %s\n", err.c_str()); return 1; }
		if ((size_t)patchIdx >= ep.Pm.Patches.size()) { fprintf(stderr, "patch out of range\n"); return 1; }
		const SPmPatch &p = ep.Pm.Patches[patchIdx];
		printf("patch %d: v=%d,%d,%d,%d\n", patchIdx, p.V[0], p.V[1], p.V[2], p.V[3]);
		for (int j = 0; j < 8; ++j)
		{
			const float *vv = ep.Pm.Vecs[p.Vec[j]].Pos;
			printf("  tan%d = vec %d: %.6f %.6f %.6f (%a %a %a)\n", j, p.Vec[j], vv[0], vv[1], vv[2], vv[0], vv[1], vv[2]);
		}
		for (int j = 0; j < 4; ++j)
		{
			const SPmEdge &e = ep.Pm.Edges[p.Edge[j]];
			printf("  edge%d = %d: v1=%d vec12=%d vec21=%d v2=%d npatches=%u\n", j, p.Edge[j], e.V1, e.Vec12, e.Vec21, e.V2, (uint)e.Patches.size());
		}
		for (size_t v = 0; v < ep.Rp.Verts.size(); ++v)
		{
			const SRpoVertexBind &b = ep.Rp.Verts[v];
			if (!b.Binded) continue;
			printf("  bind: vert %u type %u target patch %u edge %u prim %u B2/B/A/A2/T = %u %u %u %u %u\n",
				(uint)v, b.Type, b.Patch, b.Edge, b.PrimVert, b.Before2, b.Before, b.After, b.After2, b.T);
			// is any cache one of this patch's vecs?
			for (int j = 0; j < 8; ++j)
			{
				uint32 vid = (uint32)p.Vec[j];
				if (b.Before2 == vid || b.Before == vid || b.After == vid || b.After2 == vid || b.T == vid)
					printf("    ^ cache touches patch %d tan%d (vec %u)\n", patchIdx, j, vid);
			}
		}
		// full data of this patch's four corner vertices
		for (int j = 0; j < 4; ++j)
		{
			const SPmVert &vt = ep.Pm.Verts[p.V[j]];
			printf("  corner%d = vert %d flags %d pos %a %a %a\n", j, p.V[j], vt.Flags, vt.Pos[0], vt.Pos[1], vt.Pos[2]);
			for (size_t k = 0; k < vt.Vectors.size(); ++k)
			{
				sint32 vc = vt.Vectors[k];
				const SPmVec &vv = ep.Pm.Vecs[vc];
				printf("    vec %d flags %d vert %d pos %a %a %a\n", vc, vv.Flags, vv.Vert, vv.Pos[0], vv.Pos[1], vv.Pos[2]);
			}
		}
		// verts of this patch that are bind TARGETS (edge of this patch is some bind's target)
		for (size_t v = 0; v < ep.Rp.Verts.size(); ++v)
		{
			const SRpoVertexBind &b = ep.Rp.Verts[v];
			if (!b.Binded) continue;
			if ((int)b.Patch == patchIdx)
				printf("  bind target on this patch: vert %u edge %u\n", (uint)v, b.Edge);
		}
		return 0;
	}
	return 1;
}

// Debug: trace patch 0 geometry indices + binds of the first non-frozen node.
static int debugPatch0(CScene &scene)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Frozen) continue;
		SEvalPatch ep;
		std::string err;
		CSceneClass *obj = dynamic_cast<CSceneClass *>(nodes[i].Node->getReference(1));
		// raw, without binding refresh:
		{
			// duplicate evalNodePatch without refresh
			SEvalPatch raw;
			CRklPatchObject *rpo = dynamic_cast<CRklPatchObject *>(obj);
			// go through modifier stack unwrap via evalNodePatch but disable refresh: quick hack —
			// call evalNodePatch then re-decode raw for comparison is complex; just print final.
		}
		if (!evalNodePatch(nodes[i].Node, ep, err)) { fprintf(stderr, "FAIL %s\n", err.c_str()); return 1; }
		const SPmPatch &p0 = ep.Pm.Patches[0];
		printf("patch0: v=%d,%d,%d,%d vec=%d,%d,%d,%d,%d,%d,%d,%d int=%d,%d,%d,%d edge=%d,%d,%d,%d flags=%d\n",
			p0.V[0], p0.V[1], p0.V[2], p0.V[3],
			p0.Vec[0], p0.Vec[1], p0.Vec[2], p0.Vec[3], p0.Vec[4], p0.Vec[5], p0.Vec[6], p0.Vec[7],
			p0.Interior[0], p0.Interior[1], p0.Interior[2], p0.Interior[3],
			p0.Edge[0], p0.Edge[1], p0.Edge[2], p0.Edge[3], p0.Flags);
		for (size_t v = 0; v < ep.Rp.Verts.size(); ++v)
		{
			const SRpoVertexBind &b = ep.Rp.Verts[v];
			if (!b.Binded) continue;
			printf("bind: vert %u type %u target patch %u edge %u prim %u before2/before/after/after2/T = %u %u %u %u %u\n",
				(uint)v, b.Type, b.Patch, b.Edge, b.PrimVert, b.Before2, b.Before, b.After, b.After2, b.T);
		}
		// which verts are on patch0's edges
		for (int e = 0; e < 4; ++e)
		{
			const SPmEdge &ed = ep.Pm.Edges[p0.Edge[e]];
			printf("patch0 edge%d (index %d): v1=%d vec12=%d vec21=%d v2=%d patches=%u\n", e, p0.Edge[e], ed.V1, ed.Vec12, ed.Vec21, ed.V2, (uint)ed.Patches.size());
		}
		return 0;
	}
	return 1;
}

// ---------------------------------------------------------------------------------------------
// Survey mode.

static int surveyFile(const std::string &inputBase, CScene &scene)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	SNodeTMCache tmCache;
	uint unfrozen = 0;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		CNodeImpl *node = nodes[i].Node;
		SEvalPatch ep;
		std::string err;
		bool ok = evalNodePatch(node, ep, err);
		if (!nodes[i].Frozen) ++unfrozen;
		float center[3] = { 0, 0, 0 };
		if (ok)
		{
			Matrix3M objectTM = getObjectTM(node, tmCache);
			nodeCenter(ep, objectTM, center);
		}
		printf("  node '%s'%s: %s", ucstring(node->userName()).toUtf8().c_str(),
		       nodes[i].Frozen ? " FROZEN" : "", ok ? "ok" : ("FAIL " + err).c_str());
		if (ok)
			printf(" (%u patches, %u verts, center %.1f %.1f %.1f)",
			       (uint)ep.Pm.Patches.size(), (uint)ep.Pm.Verts.size(), center[0], center[1], center[2]);
		printf("\n");
	}
	printf("%s: %u rklpatch nodes, %u unfrozen\n", inputBase.c_str(), (uint)nodes.size(), unfrozen);
	return 0;
}

// ---------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------
// Entry point for the max2gltf writer (this file compiled with PMB_ZONE_NO_MAIN — see
// pmb_zone_gltf.h): the standalone --ligo flow into a private temp dir, every produced file
// handed back as (relative name, bytes) for the glTF's nel_zones blob list, plus the authored
// patch sets (buildPatchInfo pre-transform, world space) for the tessellated nel_proxy viewing
// meshes. The blob is authoritative; the proxy is visualization only.

#include "pmb_zone_gltf.h"

#ifdef NL_OS_WINDOWS
#include <process.h>
#define PMB_ZONE_GETPID _getpid
#else
#include <unistd.h>
#define PMB_ZONE_GETPID getpid
#endif

static void pmbCollectDirFiles(const std::string &dir, const std::string &relPrefix,
                               std::vector<std::pair<std::string, std::vector<uint8> > > &filesOut)
{
	std::vector<std::string> content;
	NLMISC::CPath::getPathContent(dir, false, false, true, content);
	std::sort(content.begin(), content.end());
	for (size_t i = 0; i < content.size(); ++i)
	{
		NLMISC::CIFile f;
		if (!f.open(content[i]))
			continue;
		std::vector<uint8> bytes(f.getFileSize());
		if (!bytes.empty())
			f.serialBuffer(&bytes[0], (uint)bytes.size());
		f.close();
		filesOut.push_back(std::make_pair(relPrefix + NLMISC::CFile::getFilename(content[i]), bytes));
		NLMISC::CFile::deleteFile(content[i]);
	}
}

int pmbExportZonesForGltf(const std::string &maxPath, PMAXLOAD::SLoadedMax &lm,
                          const std::string &bankPath,
                          float cellSize, float snap,
                          std::vector<std::pair<std::string, std::vector<uint8> > > &filesOut,
                          std::vector<SPmbZoneProxy> *proxiesOut)
{
	// Ligo protocol names only (same filename dispatch as exportLigoFile)
	std::string inputBase = NLMISC::CFile::getFilenameWithoutExtension(maxPath);
	{
		std::vector<std::string> tokens;
		tokenize(inputBase, tokens);
		bool ligoName = (tokens.size() == 3 && tokens[0] == "zonematerial")
			|| (tokens.size() == 4 && tokens[0] == "zonetransition")
			|| (tokens.size() == 2 && tokens[0] == "zonespecial");
		if (!ligoName)
			return 0;
	}

	NL3D::registerSerial3d(); // internally guarded

	CScene &scene = *lm.Scene;

	SExportContext ctx;
	ctx.BankPath = bankPath;

	// Authored patch sets for the viewing proxies — every RklPatch node as the artist sees it
	// (frozen ones are the neighbor-reference bricks; flagged so viewers can dim them)
	if (proxiesOut)
	{
		std::vector<SZoneNode> nodes;
		collectZoneNodes(scene, nodes);
		SNodeTMCache tmCache;
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			SEvalPatch ep;
			std::string err;
			if (!evalNodePatch(nodes[i].Node, ep, err))
			{
				fprintf(stderr, "WARNING: zone proxy '%s': %s\n",
				        ucstring(nodes[i].Node->userName()).toUtf8().c_str(), err.c_str());
				continue;
			}
			Matrix3M objectTM = getObjectTM(nodes[i].Node, tmCache);
			SPmbZoneProxy proxy;
			proxy.NodeName = ucstring(nodes[i].Node->userName()).toUtf8();
			proxy.Frozen = nodes[i].Frozen;
			if (!buildPatchInfo(ep, objectTM, 0, proxy.Patches, err))
			{
				fprintf(stderr, "WARNING: zone proxy '%s': %s\n", proxy.NodeName.c_str(), err.c_str());
				continue;
			}
			proxiesOut->push_back(proxy);
		}
	}

	// The ligo flow into a private temp dir, collected back as blobs
	char tmpBuf[256];
	snprintf(tmpBuf, sizeof(tmpBuf), "/tmp/pipeline_max_export_gltf_zone.%d", (int)PMB_ZONE_GETPID());
	std::string tmpDir = tmpBuf;
	NLLIGO::CLigoConfig config;
	config.CellSize = cellSize;
	config.Snap = snap;
	SLigoOutputs out;
	out.ZonesDir = tmpDir + "/zones";
	out.ZoneLigosDir = tmpDir + "/zoneligos";
	NLMISC::CFile::createDirectoryTree(out.ZonesDir);
	NLMISC::CFile::createDirectoryTree(out.ZoneLigosDir);
	int rc = exportLigoFile(inputBase, scene, config, ctx, out);
	pmbCollectDirFiles(out.ZonesDir, "zones/", filesOut);
	pmbCollectDirFiles(out.ZoneLigosDir, "zoneligos/", filesOut);
	::remove(out.ZonesDir.c_str());
	::remove(out.ZoneLigosDir.c_str());
	::remove(tmpDir.c_str());
	if (rc != 0)
		return -1;
	return (int)filesOut.size();
}

#ifndef PMB_ZONE_NO_MAIN
int main(int argc, char **argv)
{
	// Parse args
	std::string input;
	std::string ligoOut;
	std::string zoneOut;
	std::string bankPath;
	float cellSize = 160.0f;
	float snap = 1.0f;
	int zoneIdOverride = -1;
	bool survey = false;
	bool dumpTiles = false;
	bool debugP0 = false;
	int inspectIdx = -1;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--verbose") g_verbose = true;
		else if (arg == "--survey") survey = true;
		else if (arg == "--dump-tiles") dumpTiles = true;
		else if (arg == "--debug-patch0") dumpTiles = false, survey = false, debugP0 = true;
		else if (arg == "--inspect" && i + 1 < argc) NLMISC::fromString(argv[++i], inspectIdx);
		else if (arg == "--ligo" && i + 1 < argc) ligoOut = argv[++i];
		else if (arg == "--zone" && i + 1 < argc) zoneOut = argv[++i];
		else if (arg == "--bank" && i + 1 < argc) bankPath = argv[++i];
		else if (arg == "--cellsize" && i + 1 < argc) NLMISC::fromString(argv[++i], cellSize);
		else if (arg == "--snap" && i + 1 < argc) NLMISC::fromString(argv[++i], snap);
		else if (arg == "--zoneid" && i + 1 < argc) NLMISC::fromString(argv[++i], zoneIdOverride);
		else if (arg[0] == '-')
		{
			fprintf(stderr, "unknown option %s\n", arg.c_str());
			return 1;
		}
		else input = arg;
	}
	if (input.empty() || (ligoOut.empty() && zoneOut.empty() && !survey && !dumpTiles && !debugP0 && inspectIdx < 0))
	{
		fprintf(stderr, "usage: pipeline_max_export_zone [--ligo <outdir> | --zone <out.zone> | --survey]\n"
		                "                                [--bank <bank.smallbank>] [--cellsize 160] [--snap 1]\n"
		                "                                [--zoneid N] [--verbose] <input.max>\n"
		                "--ligo: NeLLigoExportZone protocol by input filename (zonematerial/zonetransition/\n"
		                "        zonespecial), writing <outdir>/zones/*.zone + <outdir>/zoneligos/*.ligozone\n"
		                "--zone: ExportRykolZone protocol (zone id from the node name unless --zoneid)\n");
		return 1;
	}

	NL3D::registerSerial3d();

	// Load the max file
	CStorageOleIn in;
	if (!in.open(input)) { fprintf(stderr, "ERROR: not an OLE compound file: %s\n", input.c_str()); return 1; }

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);
	NELPATCH::CNelPatch::registerClasses(&reg);

	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(&reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { fprintf(stderr, "ERROR: no DllDirectory stream\n"); return 1; }
		{ CStorageStream st(b); dll.serial(st); }
		dll.parse(VersionUnknown);
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { fprintf(stderr, "ERROR: no ClassDirectory3 stream\n"); return 1; }
		{ CStorageStream st(b); cd.serial(st); }
		cd.parse(VersionUnknown);
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { fprintf(stderr, "ERROR: no Scene stream\n"); return 1; }
		{ CStorageStream st(b); scene.serial(st); }
		scene.parse(VersionUnknown);
	}

	SExportContext ctx;
	ctx.BankPath = bankPath;

	std::string inputBase = NLMISC::CFile::getFilenameWithoutExtension(input);

	int rc = 0;
	if (inspectIdx >= 0)
	{
		rc = inspectPatch(scene, inspectIdx);
	}
	else if (debugP0)
	{
		rc = debugPatch0(scene);
	}
	else if (dumpTiles)
	{
		rc = dumpRpoTiles(scene, 2);
	}
	else if (survey)
	{
		rc = surveyFile(inputBase, scene);
	}
	else if (!ligoOut.empty())
	{
		NLLIGO::CLigoConfig config;
		config.CellSize = cellSize;
		config.Snap = snap;
		SLigoOutputs out;
		out.ZonesDir = ligoOut + "/zones";
		out.ZoneLigosDir = ligoOut + "/zoneligos";
		NLMISC::CFile::createDirectoryTree(out.ZonesDir);
		NLMISC::CFile::createDirectoryTree(out.ZoneLigosDir);
		rc = exportLigoFile(inputBase, scene, config, ctx, out);
	}
	else
	{
		rc = exportDirectZone(scene, zoneOut, ctx, zoneIdOverride);
	}

	return rc;
}
#endif /* PMB_ZONE_NO_MAIN */

/* end of file */

