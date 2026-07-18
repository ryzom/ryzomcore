/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
// Standalone zone painter (design doc §14-paint): load a .max directly through pipeline_max,
// assemble the painting landscape the way the in-Max painter's NeL thread did
// (plugin_max/nel_patch_paint paint.cpp myThread), paint tiles against a user-selected tile
// bank (P3b: the tile brush with automatic transitions, rotation, 128/256, undo — see
// paint_core.h), and save the .max back through the proven P1/P2 write path.
//
// Modes:
//   (default)      viewer: CNELU + CLandscapeModel painting scene, tile bank from --bank,
//                  CEvent3dMouseListener edit3d orbiting the landscape bbox center. LEFT MOUSE
//                  paints the selected tile set, right mouse picks the set under the cursor,
//                  PgUp/PgDn (and 0-9) select the tile set, B toggles 128/256, Ctrl+Z / Ctrl+E
//                  undo/redo, ESC or window close exits (--save writes the result). HUD text
//                  via --font (any .ttf; defaults to a system font when present).
//   --screenshot   same scene setup, render one refined frame, dump the framebuffer to .tga
//                  and exit (the visual gate; combine with --paint-script for before/after).
//   --paint-script headless (or viewer pre-pass) scripted painting: one op per line,
//                  '#' comments — `tile <zone> <patch> <u> <v> <tileSet> [rot]`,
//                  `tile256 ...` (same args), `clear <zone> <patch> <u> <v>`, `clear256 ...`,
//                  `undo`, `redo`, `seed <n>`. Ops go through the SAME implementations as the
//                  mouse path (single op layer in paint_core).
//   --save         write-back + whole-file save after ops: each (possibly tile-mutated)
//                  pristine carrier blob is encoded into its P2 write-target (topmost 0x4001
//                  snapshot, else base 0x08FD via setRPatch), the Scene stream is rebuilt and
//                  every other stream kept verbatim (OLE class id preserved).
//   --null-edit    the same write-back path with no ops at all: evaluate, resolve carriers,
//                  write back untouched pristine blobs, save to --out. With --verify-identical
//                  every stream must byte-compare against the input (the §14-paint null-edit
//                  property, now THROUGH the paint save path).
//   --dump-zones   headless proof of the eval->weld->build path: write every built CZone
//                  (serial version 4, the reference era) and report patch/bind/border counts.
//   --dump-rpo     dump every carrier's pristine tile records (the mechanical verification
//                  surface for the paint round-trip); runs after --paint-script when given.
//   --dump-bank-xref / --dump-carrier-blob   bank xref table / raw carrier blob bytes, for
//                  the transition-witness and surgical-diff checks.
//
// Scene assembly replicates the painter plugin: per RklPatch node evalNodePatch + object TM
// at t=0 -> buildPatchInfo in authored space (NO symmetry/rotate — the painting scene shows
// what the artist authored; zoneId = node collection index like the plugin's vectMesh index)
// -> cross-zone open-edge weld (the paint.cpp WELD_THRESOLD port, session-only, never
// persisted) -> CZone::build -> CZoneCornerSmoother -> Landscape.addZone. Frozen nodes
// (empty node chunk 0x0976) are boundary-reference display like the exporter's boundary
// bricks: they participate in the landscape, the weld and the metaTile graph but are never
// paint targets and their carrier blobs are never rewritten.

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
#include <nel/misc/aabbox.h>
#include <nel/misc/app_context.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/cmd_args.h>
#include <nel/misc/common.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/event_server.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/path.h>

#include <nel/3d/camera.h>
#include <nel/3d/event_mouse_listener.h>
#include <nel/3d/font_manager.h>
#include <nel/3d/landscape.h>
#include <nel/3d/landscape_model.h>
#include <nel/3d/nelu.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/text_context.h>
#include <nel/3d/tile_bank.h>
#include <nel/3d/viewport.h>
#include <nel/3d/zone.h>
#include <nel/3d/zone_corner_smoother.h>
#include <nel/3d/zone_symmetrisation.h>

#include "../pipeline_max/storage_ole.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef NL_OS_WINDOWS
#include <process.h>
#define ZP_GETPID _getpid
#else
#include <unistd.h>
#define ZP_GETPID getpid
#endif

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
#include "../pipeline_max_export_common/max_load.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::NELPATCH;
using namespace MAXMATH;

// Patch-state eval + RPO->CPatchInfo conversion: the shared unit of the zone exporter and this
// painter. Header-only static implementation unit (zone x87 tier is TU-sensitive; see the
// header doc for the include contract this file follows).
#include "../pipeline_max_export_common/patch_eval.h"

// The tile painting core (P3b): metaTile graph, transition solver, pristine carrier state,
// live-landscape mirror, undo, write-back.
#include "paint_core.h"

static bool g_verbose = false;
// Result of the viewer script pre-pass (propagated as the viewer exit code for scripted gates)
static int g_ViewerScriptRc = 0;

// ---------------------------------------------------------------------------------------------
// Zone writing (--dump-zones): current CZone::serial writes version 5; the references are
// version 4, and the two encodings differ only in the version byte (verified by reference
// roundtrip in the zone exporter). Serialize to memory and write the version byte as 4.

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
// The painting scene zones: every RklPatch node, evaluated in authored space, zoneId = node
// collection index (the plugin used its vectMesh index the same way).

struct SPaintZone
{
	CNodeImpl *Node;
	bool Frozen;
	std::string Name;
	uint ZoneId;
	std::vector<NL3D::CPatchInfo> Patches;
	std::vector<NL3D::CBorderVertex> BorderVertices; // session-only, filled by the weld pass
	SEvalPatch Ep; // evaluated topology, kept for the paint core's metaTile graph (P3b)
};

static bool buildPaintZones(CScene &scene, std::vector<SPaintZone> &zones)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	SNodeTMCache tmCache;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		CNodeImpl *node = nodes[i].Node;
		std::string name = ucstring(node->userName()).toUtf8();
		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			// The plugin showed a message box and kept going; keep the surviving zones displayed.
			fprintf(stderr, "WARNING: node '%s': %s (zone skipped)\n", name.c_str(), err.c_str());
			continue;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		SPaintZone pz;
		pz.Node = node;
		pz.Frozen = nodes[i].Frozen;
		pz.Name = name;
		pz.ZoneId = (uint)i;
		if (!buildPatchInfo(ep, objectTM, (int)i, pz.Patches, err))
		{
			fprintf(stderr, "WARNING: node '%s': %s (zone skipped)\n", name.c_str(), err.c_str());
			continue;
		}
		pz.Ep = ep;
		zones.push_back(pz);
		if (g_verbose)
			printf("zone %u '%s'%s: %u patches\n", pz.ZoneId, pz.Name.c_str(),
			       pz.Frozen ? " FROZEN" : "", (uint)pz.Patches.size());
	}
	return !zones.empty();
}

// ---------------------------------------------------------------------------------------------
// Cross-zone open-edge weld (port of the paint.cpp cross-mesh branch, WELD_THRESOLD=1): two
// open edges of different zones whose world-space endpoints coincide reversed within the
// threshold become a one/one bind across the zones, with CBorderVertex records on both sides
// so the landscape shares the corner tess vertices (and rebinds borders when zones are added
// in sequence). Session-only: this only touches the display CPatchInfo/CZone copies, the .max
// data is never modified by it.

#define WELD_THRESOLD 1.0f

// Both-direction border-vertex record for one matched corner pair, deduplicated (a corner is
// shared by the two consecutive welded edges).
static void addBorderVertexPair(std::vector<SPaintZone> &zones, uint zi, uint16 va, uint zj, uint16 vb,
                                std::set<std::pair<std::pair<uint, uint>, std::pair<uint, uint> > > &seen)
{
	std::pair<std::pair<uint, uint>, std::pair<uint, uint> > key(
		std::pair<uint, uint>(zi, va), std::pair<uint, uint>(zj, vb));
	if (key.second < key.first) std::swap(key.first, key.second);
	if (!seen.insert(key).second) return;
	NL3D::CBorderVertex bv;
	bv.CurrentVertex = va;
	bv.NeighborZoneId = (uint16)zones[zj].ZoneId;
	bv.NeighborVertex = vb;
	zones[zi].BorderVertices.push_back(bv);
	bv.CurrentVertex = vb;
	bv.NeighborZoneId = (uint16)zones[zi].ZoneId;
	bv.NeighborVertex = va;
	zones[zj].BorderVertices.push_back(bv);
}

static uint weldPaintZones(std::vector<SPaintZone> &zones)
{
	uint welds = 0;
	std::set<std::pair<std::pair<uint, uint>, std::pair<uint, uint> > > seenVerts;
	for (uint zi = 0; zi < zones.size(); ++zi)
	{
		std::vector<NL3D::CPatchInfo> &pa = zones[zi].Patches;
		for (uint p = 0; p < pa.size(); ++p)
		for (uint e = 0; e < 4; ++e)
		{
			if (pa[p].BindEdges[e].NPatchs != 0) continue; // interior or already welded
			// Edge e runs from corner e to corner (e+1)&3 (Max Patch edge convention, same as
			// the paint.cpp scan).
			const NLMISC::CVector &vA1 = pa[p].Patch.Vertices[e];
			const NLMISC::CVector &vB1 = pa[p].Patch.Vertices[(e + 1) & 3];
			bool found = false;
			for (uint zj = 0; zj < zones.size() && !found; ++zj)
			{
				if (zj == zi) continue;
				std::vector<NL3D::CPatchInfo> &pb = zones[zj].Patches;
				for (uint pp = 0; pp < pb.size() && !found; ++pp)
				for (uint ee = 0; ee < 4; ++ee)
				{
					if (pb[pp].BindEdges[ee].NPatchs != 0) continue;
					const NLMISC::CVector &vA2 = pb[pp].Patch.Vertices[ee];
					const NLMISC::CVector &vB2 = pb[pp].Patch.Vertices[(ee + 1) & 3];
					// The same edge, reversed orientation (paint.cpp: vA1~vB2 && vA2~vB1).
					if ((vA1 - vB2).norm() < WELD_THRESOLD && (vA2 - vB1).norm() < WELD_THRESOLD)
					{
						pa[p].BindEdges[e].NPatchs = 1;
						pa[p].BindEdges[e].ZoneId = (uint16)zones[zj].ZoneId;
						pa[p].BindEdges[e].Next[0] = (uint16)pp;
						pa[p].BindEdges[e].Edge[0] = (uint8)ee;
						pb[pp].BindEdges[ee].NPatchs = 1;
						pb[pp].BindEdges[ee].ZoneId = (uint16)zones[zi].ZoneId;
						pb[pp].BindEdges[ee].Next[0] = (uint16)p;
						pb[pp].BindEdges[ee].Edge[0] = (uint8)e;
						// Shared corners: (zi corner e <-> zj corner ee+1), (zi corner e+1 <-> zj
						// corner ee).
						addBorderVertexPair(zones, zi, pa[p].BaseVertices[e],
						                    zj, pb[pp].BaseVertices[(ee + 1) & 3], seenVerts);
						addBorderVertexPair(zones, zi, pa[p].BaseVertices[(e + 1) & 3],
						                    zj, pb[pp].BaseVertices[ee], seenVerts);
						++welds;
						found = true;
						break;
					}
				}
			}
		}
	}
	return welds;
}

// Build one display CZone from a paint zone: CZone::build + the corner smoother, exactly the
// plugin's per-zone sequence (smoother runs on the built, uncompiled zone with no neighbor
// list, like the plugin did).
static void buildDisplayZone(const SPaintZone &pz, NL3D::CZone &zone)
{
	zone.build((uint16)pz.ZoneId, pz.Patches, pz.BorderVertices);
	NL3D::CZoneCornerSmoother cornerSmoother;
	std::vector<NL3D::CZone *> emptyVector;
	cornerSmoother.computeAllCornerSmoothFlags(&zone, emptyVector);
}

// ---------------------------------------------------------------------------------------------
// --dump-zones: the headless eval->weld->build proof. Writes each built zone and reports the
// counts that the weld and bind passes produced.

static std::string sanitizeName(const std::string &s)
{
	std::string r = s;
	for (size_t i = 0; i < r.size(); ++i)
	{
		char c = r[i];
		bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
		if (!ok) r[i] = '_';
	}
	return r;
}

static int dumpZones(std::vector<SPaintZone> &zones, uint welds, const std::string &outDir)
{
	NLMISC::CFile::createDirectoryTree(outDir);
	uint totalPatches = 0, totalBound = 0, totalCross = 0, totalBorderVerts = 0;
	for (size_t i = 0; i < zones.size(); ++i)
	{
		const SPaintZone &pz = zones[i];
		uint bound = 0, cross = 0;
		for (size_t p = 0; p < pz.Patches.size(); ++p)
		for (uint e = 0; e < 4; ++e)
		{
			const NL3D::CPatchInfo::CBindInfo &b = pz.Patches[p].BindEdges[e];
			if (b.NPatchs == 0) continue;
			++bound;
			if (b.ZoneId != pz.ZoneId) ++cross;
		}
		NL3D::CZone zone;
		buildDisplayZone(pz, zone);
		std::string path = outDir + "/" + NLMISC::toString("zone_%u_", pz.ZoneId) + sanitizeName(pz.Name) + ".zone";
		if (!writeZoneV4(zone, path))
		{
			fprintf(stderr, "ERROR: cannot write %s\n", path.c_str());
			return 1;
		}
		printf("zone %u '%s'%s: %u patches, %u bound edges (%u cross-zone), %u border verts -> %s\n",
		       pz.ZoneId, pz.Name.c_str(), pz.Frozen ? " FROZEN" : "",
		       (uint)pz.Patches.size(), bound, cross, (uint)pz.BorderVertices.size(), path.c_str());
		totalPatches += (uint)pz.Patches.size();
		totalBound += bound;
		totalCross += cross;
		totalBorderVerts += (uint)pz.BorderVertices.size();
	}
	printf("OK dump-zones: %u zones, %u patches, %u bound edges (%u cross-zone), %u welds, %u border verts\n",
	       (uint)zones.size(), totalPatches, totalBound, totalCross, welds, totalBorderVerts);
	return 0;
}

// ---------------------------------------------------------------------------------------------
// Whole-file save: rebuilt Scene stream + every other stream verbatim + OLE class id (the P2
// flow, modeled on the corpus harness' rpoModifySaveTest). The caller mutates the parsed scene
// (paint write-back) BEFORE calling; a null edit through this same path is byte-identical.

// Serialize a container to a temp file and read the file bytes back. CMemStream's write-mode
// seek-back fails during leaveChunk; COFile handles seeks freely, so temp-file roundtrip is
// the working pattern (same as the corpus harness).
static std::vector<uint8> writeContainerToTemp(CStorageContainer &ctr, const std::string &tempPath)
{
	{
		NLMISC::COFile of(tempPath);
		ctr.serial(of, 0); // explicit-size overload avoids the outer 0x4352 wrapper
	}
	std::vector<uint8> out;
	std::ifstream ifs(tempPath.c_str(), std::ios::binary);
	if (ifs)
	{
		ifs.seekg(0, std::ios::end);
		std::streampos end = ifs.tellg();
		ifs.seekg(0);
		out.resize((size_t)end);
		if ((size_t)end) ifs.read((char *)nlVectorData(out), (std::streamsize)end);
	}
	return out;
}

static int saveWholeFile(const std::string &input, const std::string &output, CScene &scene, bool verifyIdentical)
{
	// The known .max stream set (same list as the corpus harness save tests).
	static const char *kStreams[] = {
		"VideoPostQueue", "Config", "ClassData", "DllDirectory", "ClassDirectory3", "Scene",
		"\05SummaryInformation", "\05DocumentSummaryInformation", NULL
	};
	std::vector<std::string> present;
	std::vector<std::vector<uint8> > rawOrig;
	uint8 classId[16];
	bool haveClassId;
	{
		CStorageOleIn in;
		if (!in.open(input)) { fprintf(stderr, "ERROR: not an OLE compound file: %s\n", input.c_str()); return 1; }
		for (const char **n = kStreams; *n; ++n)
		{
			std::vector<uint8> b;
			if (in.readStream(*n, b)) { present.push_back(*n); rawOrig.push_back(b); }
		}
		haveClassId = in.getClassId(classId);
	}

	// Rebuild the Scene stream from the typed graph (§5 lifecycle) and write the whole file.
	std::string tempPath = NLMISC::toString("/tmp/zone_painter.%d.tmp", (int)ZP_GETPID());
	std::vector<uint8> newScene;
	try
	{
		scene.clean();
		scene.build(VersionUnknown);
		scene.disown();
		newScene = writeContainerToTemp(scene, tempPath);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "ERROR: scene rebuild: %s\n", e.what());
		remove(tempPath.c_str());
		return 1;
	}
	remove(tempPath.c_str());

	{
		CStorageOleOut out;
		for (size_t i = 0; i < present.size(); ++i)
		{
			if (present[i] == "Scene") out.addStream("Scene", newScene);
			else out.addStream(present[i], rawOrig[i]);
		}
		if (haveClassId) out.setClassId(classId);
		if (!out.write(output)) { fprintf(stderr, "ERROR: cannot create %s\n", output.c_str()); return 1; }
	}

	uint diffs = 0;
	if (verifyIdentical)
	{
		CStorageOleIn in2;
		if (!in2.open(output)) { fprintf(stderr, "ERROR: cannot reopen %s\n", output.c_str()); return 1; }
		for (size_t i = 0; i < present.size(); ++i)
		{
			std::vector<uint8> b2;
			in2.readStream(present[i], b2);
			if (b2 != rawOrig[i])
			{
				fprintf(stderr, "ERROR: stream %s NOT byte-identical (%u -> %u bytes)\n",
				        (present[i][0] == '\05' ? present[i].substr(1) : present[i]).c_str(),
				        (uint)rawOrig[i].size(), (uint)b2.size());
				++diffs;
			}
		}
	}
	if (verifyIdentical)
		printf("%s null-edit: %u stream diffs -> %s\n", diffs ? "FAIL" : "OK", diffs, output.c_str());
	else
		printf("OK save -> %s\n", output.c_str());
	return diffs ? 1 : 0;
}

// Build the paint core inputs from the assembled zones (pointers into the final zones vector).
static void buildPaintInputs(std::vector<SPaintZone> &zones, std::vector<ZPPAINT::SPaintZoneInput> &inputs)
{
	inputs.clear();
	for (size_t i = 0; i < zones.size(); ++i)
	{
		ZPPAINT::SPaintZoneInput in;
		in.Node = zones[i].Node;
		in.Frozen = zones[i].Frozen;
		in.ZoneId = zones[i].ZoneId;
		in.Name = zones[i].Name;
		in.Patches = &zones[i].Patches;
		in.Pm = &zones[i].Ep.Pm;
		in.EvalRp = &zones[i].Ep.Rp;
		inputs.push_back(in);
	}
}

// ---------------------------------------------------------------------------------------------
// Scripted paint mode: one op per line, same op layer as the mouse path (see the file header
// for the command list). Any FAILed op fails the run (scripts are curated test inputs).

static int runPaintScript(ZPPAINT::CPaintCore &core, const std::string &path)
{
	std::ifstream ifs(path.c_str());
	if (!ifs) { fprintf(stderr, "ERROR: cannot open script %s\n", path.c_str()); return 1; }
	std::string line;
	int lineNo = 0;
	int fails = 0;
	while (std::getline(ifs, line))
	{
		++lineNo;
		std::string::size_type hash = line.find('#');
		if (hash != std::string::npos) line.erase(hash);
		std::vector<std::string> tok;
		{
			std::string cur;
			for (size_t i = 0; i <= line.size(); ++i)
			{
				char c = (i < line.size()) ? line[i] : ' ';
				if (c == ' ' || c == '\t' || c == '\r') { if (!cur.empty()) { tok.push_back(cur); cur.clear(); } }
				else cur += c;
			}
		}
		if (tok.empty()) continue;
		std::string err;
		bool ok = true;
		if ((tok[0] == "tile" || tok[0] == "tile256") && tok.size() >= 6)
		{
			uint zone, patch, u, v;
			int ts, rot = 0;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], u);
			NLMISC::fromString(tok[4], v);
			NLMISC::fromString(tok[5], ts);
			if (tok.size() >= 7) NLMISC::fromString(tok[6], rot);
			ok = core.opTile(zone, patch, u, v, ts, rot, tok[0] == "tile256", err);
		}
		else if (tok[0] == "rot" && tok.size() >= 6)
		{
			// Re-put the tile's own base tile set at the requested rotation (goes through the
			// same put/transition machinery as a paint).
			uint zone, patch, u, v;
			int rot;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], u);
			NLMISC::fromString(tok[4], v);
			NLMISC::fromString(tok[5], rot);
			ZPPAINT::CTileDescP desc;
			core.getTile(zone, (sint32)(patch * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u), desc);
			if (desc.isEmpty()) { ok = false; err = "rot on an empty tile"; }
			else
			{
				int ts = core.tileSetOfTile(desc.getLayer(0).Tile);
				if (ts < 0) { ok = false; err = "rot: tile without bank xref"; }
				else ok = core.opTile(zone, patch, u, v, ts, rot, desc.getCase() != 0, err);
			}
		}
		else if ((tok[0] == "clear" || tok[0] == "clear256") && tok.size() >= 5)
		{
			uint zone, patch, u, v;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], u);
			NLMISC::fromString(tok[4], v);
			ok = core.opClear(zone, patch, u, v, tok[0] == "clear256", err);
		}
		else if (tok[0] == "undo") { ok = core.opUndo(); if (!ok) err = "undo stack empty"; }
		else if (tok[0] == "redo") { ok = core.opRedo(); if (!ok) err = "redo stack empty"; }
		else if (tok[0] == "seed" && tok.size() >= 2)
		{
			uint s;
			NLMISC::fromString(tok[1], s);
			srand(s);
		}
		else
		{
			fprintf(stderr, "ERROR: script line %d: bad command '%s'\n", lineNo, tok[0].c_str());
			return 1;
		}
		if (ok) printf("OK line %d: %s (%u tile writes)\n", lineNo, tok[0].c_str(), core.strokeSetCount());
		else
		{
			printf("FAIL line %d: %s: %s\n", lineNo, tok[0].c_str(), err.c_str());
			++fails;
		}
	}
	return fails ? 1 : 0;
}

// ---------------------------------------------------------------------------------------------
// Viewer / screenshot: the painting scene (paint.cpp myThread without the paint tools).

// Window close tracking (the plugin's MouseListener WindowActive flag).
class CWindowCloseListener : public NLMISC::IEventListener
{
public:
	bool WindowActive;
	CWindowCloseListener() : WindowActive(true) { }
	virtual void operator()(const NLMISC::CEvent &event)
	{
		if (event == NLMISC::EventDestroyWindowId || event == NLMISC::EventCloseWindowId)
			WindowActive = false;
	}
};

// paint_ui.cpp default light setup (the cfg overrides are P3b UI territory).
static const NLMISC::CVector kLightDirection(1.f, 1.f, -1.f);
static const NLMISC::CRGBA kLightDiffuse(255, 255, 255);
static const NLMISC::CRGBA kLightAmbiant(0, 0, 0);
static const float kLightMultiply = 1.f;

static const uint kMainWidth = 800;
static const uint kMainHeight = 600;

// The tile bank (the plugin took it from the tile_utility choice; here it is --bank). Tile
// texture paths become CPath-resolvable relative names, seeded with the bank file's directory
// (recursive on request) plus any extra search paths.
static bool loadBankFile(const std::string &bankPath, bool bankRecursive,
                         const std::vector<std::string> &searchPaths, NL3D::CTileBank &bank)
{
	try
	{
		NLMISC::CIFile file;
		if (!file.open(bankPath)) { fprintf(stderr, "ERROR: cannot open bank %s\n", bankPath.c_str()); return false; }
		bank.serial(file);
		bank.computeXRef();
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: bank: %s\n", e.what());
		return false;
	}
	bank.makeAllPathRelative();
	bank.setAbsPath("");
	NLMISC::CPath::addSearchPath(NLMISC::CFile::getPath(bankPath), bankRecursive, false);
	for (size_t i = 0; i < searchPaths.size(); ++i)
		NLMISC::CPath::addSearchPath(searchPaths[i], true, false);
	return true;
}

// The viewer's paint mouse listener (plugin MouseListener port, tile mode only): left button
// paints through the shared op layer, right button picks the tile set under the cursor,
// Ctrl+Z / Ctrl+E undo/redo. The edit3d navigation stays on the middle mouse.
class CPaintMouseListener : public NLMISC::IEventListener
{
public:
	ZPPAINT::CPaintCore *Core;
	NL3D::CEvent3dMouseListener *Nav;
	NL3D::CViewport Viewport;
	int CurTileSet;
	bool Mode256;
	bool Pressed;
	float MouseX, MouseY;
	bool HaveHover;
	uint HoverZone;
	sint32 HoverTile;
	uint StrokeZone;
	sint32 StrokeTile;

	CPaintMouseListener() : Core(NULL), Nav(NULL), CurTileSet(0), Mode256(false), Pressed(false),
		MouseX(0.5f), MouseY(0.5f), HaveHover(false), HoverZone(0), HoverTile(-1), StrokeZone(0), StrokeTile(-1) { }

	void updateHover()
	{
		HaveHover = false;
		if (!Core) return;
		NLMISC::CVector pos, dir, hit;
		Viewport.getRayWithPoint(MouseX, MouseY, pos, dir, NL3D::CNELU::Camera->getMatrix(), NL3D::CNELU::Camera->getFrustum());
		uint zone;
		sint32 tile;
		if (Core->pickTile(pos, dir, zone, tile, hit))
		{
			HaveHover = true;
			HoverZone = zone;
			HoverTile = tile;
		}
	}

	virtual void operator()(const NLMISC::CEvent &event)
	{
		if (!Core) return;
		if (event == NLMISC::EventMouseDownId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			MouseX = mouse->X;
			MouseY = mouse->Y;
			if (mouse->Button == NLMISC::leftButton)
			{
				updateHover();
				if (HaveHover && !Core->zoneFrozen(HoverZone))
				{
					std::string err;
					if (Core->opTileStroke(HoverZone, HoverTile, CurTileSet, Mode256, true, err))
					{
						Pressed = true;
						StrokeZone = HoverZone;
						StrokeTile = HoverTile;
					}
				}
			}
			if (mouse->Button == NLMISC::rightButton)
			{
				// Pick: current tile set = the base layer's set under the cursor
				updateHover();
				if (HaveHover)
				{
					ZPPAINT::CTileDescP desc;
					Core->getTile(HoverZone, HoverTile, desc);
					if (!desc.isEmpty())
					{
						int tileSet, number;
						NL3D::CTileBank::TTileType type;
						// the bank the core paints against resolves the xref
						CurTileSet = -1;
						(void)number;
						(void)type;
						(void)tileSet;
						CurTileSet = Core->tileSetOfTile(desc.getLayer(0).Tile);
					}
				}
			}
		}
		else if (event == NLMISC::EventMouseUpId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			if (mouse->Button == NLMISC::leftButton && Pressed)
			{
				Pressed = false;
				Core->endStroke();
			}
		}
		else if (event == NLMISC::EventMouseMoveId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			MouseX = mouse->X;
			MouseY = mouse->Y;
			if (Pressed && (mouse->Button & NLMISC::leftButton))
			{
				updateHover();
				if (HaveHover && (HoverTile != StrokeTile || HoverZone != StrokeZone) && !Core->zoneFrozen(HoverZone))
				{
					std::string err;
					if (Core->opTileStroke(HoverZone, HoverTile, CurTileSet, Mode256, false, err))
					{
						StrokeZone = HoverZone;
						StrokeTile = HoverTile;
					}
				}
			}
		}
		else if (event == NLMISC::EventKeyDownId)
		{
			NLMISC::CEventKeyDown *keyDown = (NLMISC::CEventKeyDown *)&event;
			if (keyDown->FirstTime && (keyDown->Button & NLMISC::ctrlKeyButton))
			{
				if (keyDown->Key == NLMISC::KeyZ) Core->opUndo();
				if (keyDown->Key == NLMISC::KeyE) Core->opRedo();
			}
		}
	}
};

static int runViewer(std::vector<SPaintZone> &zones, NL3D::CTileBank &bank, ZPPAINT::CPaintCore *core,
                     const std::string &screenshotPath, const std::string &fontPath,
                     const std::string &scriptPath)
{
	// Landscape bbox in world space (camera + mouse hotspot).
	NLMISC::CAABBox bbox;
	bool bboxInit = false;
	for (size_t i = 0; i < zones.size(); ++i)
	for (size_t p = 0; p < zones[i].Patches.size(); ++p)
	{
		const NL3D::CBezierPatch &bp = zones[i].Patches[p].Patch;
		for (uint v = 0; v < 4; ++v)
		{
			if (!bboxInit) { bbox.setCenter(bp.Vertices[v]); bbox.setHalfSize(NLMISC::CVector::Null); bboxInit = true; }
			else bbox.extend(bp.Vertices[v]);
		}
		for (uint v = 0; v < 8; ++v) bbox.extend(bp.Tangents[v]);
		for (uint v = 0; v < 4; ++v) bbox.extend(bp.Interiors[v]);
	}
	NLMISC::CVector center = bbox.getCenter();

	try
	{
		NL3D::CViewport viewport;
		if (!NL3D::CNELU::init(kMainWidth, kMainHeight, viewport))
		{
			fprintf(stderr, "ERROR: CNELU::init failed (no 3D driver?)\n");
			return 1;
		}

		// The painting landscape (paint.cpp myThread).
		NL3D::CLandscapeModel *theLand = (NL3D::CLandscapeModel *)NL3D::CNELU::Scene->createModel(NL3D::LandscapeModelId);
		theLand->Landscape.setTileNear(10000.f);
		theLand->Landscape.TileBank = bank;
		theLand->Landscape.enableAutomaticLighting(false);
		theLand->Landscape.setupAutomaticLightDir(kLightDirection);
		theLand->Landscape.setupStaticLight(kLightDiffuse, kLightAmbiant, kLightMultiply);

		for (size_t i = 0; i < zones.size(); ++i)
		{
			NL3D::CZone zone;
			buildDisplayZone(zones[i], zone);
			if (!theLand->Landscape.addZone(zone))
				fprintf(stderr, "WARNING: addZone failed for zone %u '%s'\n", zones[i].ZoneId, zones[i].Name.c_str());
		}
		theLand->Landscape.setRefineMode(true);

		// Camera looking at the bbox center from a sensible distance (the plugin inherited the
		// Max viewport matrix; standalone starts from a canonical three-quarter view).
		float dist = std::max(bbox.getRadius() * 2.0f, 10.f);
		NLMISC::CVector dir = NLMISC::CVector(-0.55f, -0.65f, 0.55f).normed();
		NLMISC::CVector pos = center + dir * dist;
		NLMISC::CVector J = (center - pos).normed();
		NLMISC::CVector I = (J ^ NLMISC::CVector::K).normed();
		NLMISC::CVector K = I ^ J;
		NLMISC::CMatrix camMat;
		camMat.identity();
		camMat.setRot(I, J, K, true);
		camMat.setPos(pos);
		NL3D::CNELU::Camera->setTransformMode(NL3D::ITransformable::DirectMatrix);
		NL3D::CNELU::Camera->setMatrix(camMat);
		NL3D::CNELU::Camera->setPerspective(75.f * (float)NLMISC::Pi / 180.f, 1.33f, 0.1f, 10000.f);

		// Mouse listener: edit3d orbiting the landscape center (the plugin's hotspot was the
		// selection center).
		NL3D::CEvent3dMouseListener mouseListener;
		mouseListener.setMatrix(camMat);
		mouseListener.setFrustrum(NL3D::CNELU::Camera->getFrustum());
		mouseListener.setViewport(viewport);
		mouseListener.setHotSpot(center);
		mouseListener.setMouseMode(NL3D::CEvent3dMouseListener::edit3d);
		mouseListener.addToServer(NL3D::CNELU::EventServer);

		CWindowCloseListener closeListener;
		NL3D::CNELU::EventServer.addListener(NLMISC::EventDestroyWindowId, &closeListener);
		NL3D::CNELU::EventServer.addListener(NLMISC::EventCloseWindowId, &closeListener);

		// Paint listener: live-landscape mirror + the mouse op path
		CPaintMouseListener paintListener;
		if (core)
		{
			core->attachLandscape(&theLand->Landscape);
			paintListener.Core = core;
			paintListener.Nav = &mouseListener;
			paintListener.Viewport = viewport;
			NL3D::CNELU::EventServer.addListener(NLMISC::EventMouseDownId, &paintListener);
			NL3D::CNELU::EventServer.addListener(NLMISC::EventMouseUpId, &paintListener);
			NL3D::CNELU::EventServer.addListener(NLMISC::EventMouseMoveId, &paintListener);
			NL3D::CNELU::EventServer.addListener(NLMISC::EventKeyDownId, &paintListener);
		}

		// HUD text (any TrueType through the font manager; silently disabled without a font)
		NL3D::CFontManager fontManager;
		NL3D::CTextContext textContext;
		bool hudText = false;
		if (!fontPath.empty() && NLMISC::CFile::fileExists(fontPath))
		{
			textContext.init(NL3D::CNELU::Driver, &fontManager);
			textContext.setFontGenerator(fontPath);
			textContext.setHotSpot(NL3D::CComputedString::TopLeft);
			textContext.setColor(NLMISC::CRGBA(255, 255, 255));
			textContext.setFontSize(16);
			hudText = true;
		}

		theLand->enableAdditive(true);

		// Scripted pre-pass (the ops mirror straight into the attached live landscape)
		g_ViewerScriptRc = 0;
		if (core && !scriptPath.empty())
			g_ViewerScriptRc = runPaintScript(*core, scriptPath);

		if (!screenshotPath.empty())
		{
			// One refined frame -> .tga -> exit (the visual gate).
			NL3D::CNELU::clearBuffers(NLMISC::CRGBA(90, 90, 90));
			NL3D::CNELU::Scene->render();
			theLand->Landscape.setRefineMode(false);
			theLand->Landscape.refineAll(pos);
			NL3D::CNELU::clearBuffers(NLMISC::CRGBA(90, 90, 90));
			NL3D::CNELU::Scene->render();
			NL3D::CNELU::swapBuffers();
			NLMISC::CBitmap btm;
			NL3D::CNELU::Driver->getBuffer(btm);
			NLMISC::COFile fs;
			if (!fs.open(screenshotPath))
			{
				fprintf(stderr, "ERROR: cannot write %s\n", screenshotPath.c_str());
			}
			else
			{
				btm.writeTGA(fs, 24);
				printf("OK screenshot: %ux%u -> %s\n", btm.getWidth(), btm.getHeight(), screenshotPath.c_str());
			}
		}
		else
		{
			// MAIN LOOP (paint.cpp: pump, camera from the mouse listener, render; first frame
			// switches refine mode off and computes the full tessellation).
			do
			{
				NL3D::CNELU::EventServer.pump();

				// Tile set selection keys (plugin: the texture panel; here PgUp/PgDn + 0-9 + B)
				if (core)
				{
					uint count = core->tileSetCount();
					if (count)
					{
						if (NL3D::CNELU::AsyncListener.isKeyPushed(NLMISC::KeyPRIOR))
							paintListener.CurTileSet = (paintListener.CurTileSet + (int)count - 1) % (int)count;
						if (NL3D::CNELU::AsyncListener.isKeyPushed(NLMISC::KeyNEXT))
							paintListener.CurTileSet = (paintListener.CurTileSet + 1) % (int)count;
						for (int k = 0; k <= 9; ++k)
							if (NL3D::CNELU::AsyncListener.isKeyPushed((NLMISC::TKey)(NLMISC::Key0 + k)) && k < (int)count)
								paintListener.CurTileSet = k;
					}
					if (NL3D::CNELU::AsyncListener.isKeyPushed(NLMISC::KeyB))
						paintListener.Mode256 = !paintListener.Mode256;
					if (!paintListener.Pressed)
						paintListener.updateHover();
				}

				NLMISC::CMatrix camKey = mouseListener.getViewMatrix();
				NL3D::CNELU::Camera->setMatrix(camKey);
				NL3D::CNELU::clearBuffers(NLMISC::CRGBA(90, 90, 90));
				NL3D::CNELU::Scene->render();
				if (theLand->Landscape.getRefineMode())
				{
					theLand->Landscape.setRefineMode(false);
					theLand->Landscape.refineAll(camKey.getPos());
				}

				// Hovered tile outline (world-space lines after the scene render)
				if (core && paintListener.HaveHover)
				{
					NLMISC::CVector c[4];
					if (core->tileCorners(paintListener.HoverZone, paintListener.HoverTile, c) == 0)
					{
						NLMISC::CVector lift(0.f, 0.f, 0.15f);
						NLMISC::CRGBA col = core->zoneFrozen(paintListener.HoverZone) ? NLMISC::CRGBA(255, 64, 64) : NLMISC::CRGBA(255, 255, 0);
						NL3D::CNELU::Driver->setupModelMatrix(NLMISC::CMatrix::Identity);
						for (int l = 0; l < 4; ++l)
							NL3D::CDRU::drawLine(c[l] + lift, c[(l + 1) & 3] + lift, col, *NL3D::CNELU::Driver);
					}
				}

				// HUD text
				if (core && hudText)
				{
					textContext.setColor(NLMISC::CRGBA(255, 255, 255));
					textContext.printfAt(0.01f, 0.98f, "TileSet %d/%u '%s'  %s  undo %u",
					                     paintListener.CurTileSet, core->tileSetCount(),
					                     core->tileSetName(paintListener.CurTileSet).c_str(),
					                     paintListener.Mode256 ? "256" : "128", core->undoDepth());
					if (paintListener.HaveHover)
					{
						sint32 t = paintListener.HoverTile;
						textContext.printfAt(0.01f, 0.95f, "zone %u patch %d tile (%d,%d)%s",
						                     paintListener.HoverZone, (int)(t / 256), (int)(t % 256 % 16), (int)(t % 256 / 16),
						                     core->zoneFrozen(paintListener.HoverZone) ? " FROZEN" : "");
					}
				}

				NL3D::CNELU::swapBuffers();
				NL3D::CNELU::screenshot(); // F12, same convenience as the other NeL viewers
			}
			while (!NL3D::CNELU::AsyncListener.isKeyPushed(NLMISC::KeyESCAPE) && closeListener.WindowActive);
		}

		mouseListener.removeFromServer(NL3D::CNELU::EventServer);
		NL3D::CNELU::EventServer.removeListener(NLMISC::EventDestroyWindowId, &closeListener);
		NL3D::CNELU::EventServer.removeListener(NLMISC::EventCloseWindowId, &closeListener);
		if (core)
		{
			core->attachLandscape(NULL);
			NL3D::CNELU::EventServer.removeListener(NLMISC::EventMouseDownId, &paintListener);
			NL3D::CNELU::EventServer.removeListener(NLMISC::EventMouseUpId, &paintListener);
			NL3D::CNELU::EventServer.removeListener(NLMISC::EventMouseMoveId, &paintListener);
			NL3D::CNELU::EventServer.removeListener(NLMISC::EventKeyDownId, &paintListener);
		}
		NL3D::CNELU::release();
	}
	catch (const NL3D::EDru &e)
	{
		fprintf(stderr, "ERROR: 3D driver: %s\n", e.what());
		return 1;
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: %s\n", e.what());
		return 1;
	}
	return g_ViewerScriptRc;
}

// ---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext applicationContext;

	NLMISC::CCmdArgs args;
	args.setDescription("Standalone zone painter (design doc \xc2\xa7" "14-paint). Default mode opens the "
	                    "painting landscape viewer; the headless modes need no 3D driver.");
	args.addAdditionalArg("input.max", "Input .max scene");
	args.addArg("", "bank", "bank", "Tile bank (.smallbank/.bank); required for painting and the viewer/screenshot modes");
	args.addArg("", "bank-recursive", "", "Add the bank directory to the texture search path recursively");
	args.addArg("", "search-path", "dir", "Extra recursive texture search path (repeatable)", false);
	args.addArg("", "out", "output.max", "Output .max for --null-edit (in-place save is refused)");
	args.addArg("", "save", "output.max", "Write-back + whole-file save after ops (in-place save is refused)");
	args.addArg("", "cellsize", "meters", "Ligo cell size for the zone-symmetry state (default 100)");
	args.addArg("", "snap", "meters", "Ligo snap for the zone-symmetry state (default 1)");
	args.addArg("", "paint-script", "file", "Scripted paint ops (headless without a display mode)");
	args.addArg("", "seed", "n", "Random seed for the paint ops (default 1; ops use a cycle counter for base tiles)");
	args.addArg("", "lock-borders", "", "Lock tiles bordering frozen zones or open edges (plugin lockBorders)");
	args.addArg("", "null-edit", "", "Headless: resolve carriers, write back untouched pristine blobs, save to --out");
	args.addArg("", "verify-identical", "", "With --null-edit: byte-compare the output against the input");
	args.addArg("", "dump-zones", "dir", "Headless: write every built display CZone and report counts");
	args.addArg("", "dump-rpo", "", "Dump every carrier's pristine tile records to stdout");
	args.addArg("", "dump-bank-xref", "", "Dump the bank's tile -> (set, number, type) xref table to stdout");
	args.addArg("", "dump-carrier-blob", "dir", "Write each zone's original carrier blob bytes to <dir>/zone<id>.blob");
	args.addArg("", "screenshot", "out.tga", "Render one frame to a .tga and exit");
	args.addArg("", "font", "file.ttf", "HUD font for the viewer (default: a system font when present)");
	args.addArg("", "verbose", "", "Verbose output");
	if (!args.parse(argc, argv))
		return 1;

	std::string input = args.getAdditionalArg("input.max")[0];
	g_verbose = args.haveLongArg("verbose");
	std::string bankPath = args.haveLongArg("bank") ? args.getLongArg("bank")[0] : std::string();
	bool bankRecursive = args.haveLongArg("bank-recursive");
	float cellSize = 100.f;
	float snap = 1.f;
	if (args.haveLongArg("cellsize")) NLMISC::fromString(args.getLongArg("cellsize")[0], cellSize);
	if (args.haveLongArg("snap")) NLMISC::fromString(args.getLongArg("snap")[0], snap);
	uint seed = 1;
	if (args.haveLongArg("seed")) NLMISC::fromString(args.getLongArg("seed")[0], seed);
	srand(seed);
	std::string scriptPath = args.haveLongArg("paint-script") ? args.getLongArg("paint-script")[0] : std::string();
	std::string savePath = args.haveLongArg("save") ? args.getLongArg("save")[0] : std::string();
	std::string fontPath = args.haveLongArg("font") ? args.getLongArg("font")[0] : std::string();
	if (fontPath.empty())
	{
		// Default HUD font: a common system TrueType (HUD text silently off when absent)
		const char *sysFont = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
		if (NLMISC::CFile::fileExists(sysFont)) fontPath = sysFont;
	}
	bool nullEdit = args.haveLongArg("null-edit");
	bool doDumpRpo = args.haveLongArg("dump-rpo");
	bool doDumpXRef = args.haveLongArg("dump-bank-xref");
	std::string dumpBlobDir = args.haveLongArg("dump-carrier-blob") ? args.getLongArg("dump-carrier-blob")[0] : std::string();
	// Viewer runs for --screenshot and for the plain interactive invocation (a paint script
	// without a display mode is headless; the dump-only modes are headless too).
	bool viewerMode = !nullEdit && !args.haveLongArg("dump-zones")
		&& (args.haveLongArg("screenshot") || (scriptPath.empty() && !doDumpRpo && !doDumpXRef && dumpBlobDir.empty()));

	if (nullEdit && !args.haveLongArg("out"))
	{
		fprintf(stderr, "ERROR: --null-edit refuses to save in place; give --out <output.max>\n");
		return 1;
	}
	if (!savePath.empty() && savePath == input)
	{
		fprintf(stderr, "ERROR: --save refuses to save in place\n");
		return 1;
	}

	// Load + assemble the painting zones (all modes; the null-edit path exercises exactly the
	// paint save path with zero ops).
	NL3D::registerSerial3d();
	PMAXLOAD::SLoadedMax lm;
	if (!PMAXLOAD::loadMaxFile(input, lm)) { fprintf(stderr, "ERROR: cannot load %s\n", input.c_str()); return 1; }

	std::vector<SPaintZone> zones;
	bool haveZones = buildPaintZones(*lm.Scene, zones);
	if (!haveZones && !nullEdit)
	{
		fprintf(stderr, "ERROR: no displayable RklPatch zone in %s\n", input.c_str());
		return 1;
	}
	uint welds = weldPaintZones(zones);
	if (g_verbose) printf("weld: %u cross-zone edges\n", welds);

	if (args.haveLongArg("dump-zones"))
		return dumpZones(zones, welds, args.getLongArg("dump-zones")[0]);

	// The tile bank: required for paint ops and display; the null-edit/dump paths run without.
	NL3D::CTileBank bank;
	bool haveBank = false;
	if (!bankPath.empty())
	{
		std::vector<std::string> searchPaths;
		if (args.haveLongArg("search-path")) searchPaths = args.getLongArg("search-path");
		if (!loadBankFile(bankPath, bankRecursive, searchPaths, bank)) return 1;
		haveBank = true;
	}
	if ((viewerMode || !scriptPath.empty()) && !haveBank)
	{
		fprintf(stderr, "ERROR: this mode needs --bank <bank.smallbank>\n");
		return 1;
	}

	// The painting core over the assembled zones
	ZPPAINT::CPaintCore core;
	std::vector<ZPPAINT::SPaintZoneInput> inputs;
	buildPaintInputs(zones, inputs);
	{
		std::string err;
		if (!core.init(inputs, haveBank ? &bank : NULL, cellSize, snap, args.haveLongArg("lock-borders"), err))
		{
			fprintf(stderr, "ERROR: paint core: %s\n", err.c_str());
			return 1;
		}
	}

	if (doDumpXRef)
	{
		if (!haveBank) { fprintf(stderr, "ERROR: --dump-bank-xref needs --bank\n"); return 1; }
		core.dumpBankXRef(stdout);
	}
	if (!dumpBlobDir.empty())
	{
		NLMISC::CFile::createDirectoryTree(dumpBlobDir);
		for (size_t i = 0; i < zones.size(); ++i)
		{
			std::vector<uint8> blob;
			if (!core.dumpCarrierBlob(zones[i].ZoneId, blob)) continue;
			std::string path = dumpBlobDir + NLMISC::toString("/zone%u.blob", zones[i].ZoneId);
			NLMISC::COFile f;
			if (f.open(path) && !blob.empty()) f.serialBuffer(nlVectorData(blob), (uint)blob.size());
		}
	}

	int rc = 0;
	if (viewerMode)
	{
		std::string screenshotPath = args.haveLongArg("screenshot") ? args.getLongArg("screenshot")[0] : std::string();
		rc = runViewer(zones, bank, &core, screenshotPath, fontPath, scriptPath);
	}
	else if (!scriptPath.empty())
	{
		rc = runPaintScript(core, scriptPath);
	}

	if (doDumpRpo)
		core.dumpRpo(stdout);

	// Save flows: --null-edit (untouched write-back, optional byte-compare) or --save (after ops)
	if (nullEdit)
	{
		std::string err;
		if (!core.writeBack(err)) { fprintf(stderr, "ERROR: write-back: %s\n", err.c_str()); return 1; }
		int saveRc = saveWholeFile(input, args.getLongArg("out")[0], *lm.Scene, args.haveLongArg("verify-identical"));
		return saveRc ? saveRc : rc;
	}
	if (!savePath.empty())
	{
		std::string err;
		if (!core.writeBack(err)) { fprintf(stderr, "ERROR: write-back: %s\n", err.c_str()); return 1; }
		int saveRc = saveWholeFile(input, savePath, *lm.Scene, false);
		if (saveRc) return saveRc;
	}

	return rc;
}

/* end of file */
