/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
// Standalone zone painter, milestone P3a (design doc §14-paint): load a .max directly through
// pipeline_max, assemble the painting landscape the way the in-Max painter's NeL thread did
// (plugin_max/nel_patch_paint paint.cpp myThread), and save the .max back through the proven
// P1/P2 write path. No painting operations yet (P3b) — this milestone is load -> landscape
// viewer -> null-edit save, plus headless batch modes that need no 3D driver.
//
// Modes:
//   (default)      viewer: CNELU + CLandscapeModel painting scene, tile bank from --bank,
//                  CEvent3dMouseListener edit3d orbiting the landscape bbox center, ESC or
//                  window close to exit. The GL driver is loaded dynamically at runtime.
//   --screenshot   same scene setup, render one refined frame, dump the framebuffer to .tga
//                  and exit (the future visual gate).
//   --dump-zones   headless proof of the eval->weld->build path: write every built CZone
//                  (serial version 4, the reference era) and report patch/bind/border counts.
//   --null-edit    headless save proof: resolve every RPatchMesh blob in the file (base
//                  0x08FD via CRklPatchObject::setRPatch, modifier snapshot 0x4001 via
//                  NELPATCH::encodeRPatchMesh into the raw leaf — decode then re-encode, no
//                  changes), rebuild the Scene stream, write the whole .max to --out (other
//                  streams verbatim, OLE class id preserved). With --verify-identical the
//                  output must be byte-identical to the input, stream by stream (the
//                  §14-paint P2 null-edit property).
//
// Scene assembly replicates the painter plugin: per RklPatch node evalNodePatch + object TM
// at t=0 -> buildPatchInfo in authored space (NO symmetry/rotate — the painting scene shows
// what the artist authored; zoneId = node collection index like the plugin's vectMesh index)
// -> cross-zone open-edge weld (the paint.cpp WELD_THRESOLD port, session-only, never
// persisted) -> CZone::build -> CZoneCornerSmoother -> Landscape.addZone. Frozen nodes
// (empty node chunk 0x0976) are boundary-reference display like the exporter's boundary
// bricks: they participate in the landscape and the weld but are flagged — they will not be
// paint targets in P3b.

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
#include <nel/3d/landscape.h>
#include <nel/3d/landscape_model.h>
#include <nel/3d/nelu.h>
#include <nel/3d/register_3d.h>
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

static bool g_verbose = false;

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
// Null-edit save: the P2 whole-file save flow (modeled on the corpus harness'
// rpoModifySaveTest). Every RPatchMesh blob decode->encode in place, Scene stream rebuilt from
// the typed graph, every other stream verbatim, OLE class id preserved.

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

static int nullEditSave(const std::string &input, const std::string &output, bool verifyIdentical)
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

	PMAXLOAD::SLoadedMax lm;
	if (!PMAXLOAD::loadMaxFile(input, lm)) { fprintf(stderr, "ERROR: cannot load %s\n", input.c_str()); return 1; }
	CScene &scene = *lm.Scene;

	// Push every blob through the write path in place: base RPO 0x08FD via setRPatch, modifier
	// snapshot 0x4001 (per-node local data 0x2512 -> 0x1000) via encodeRPatchMesh into the raw
	// leaf. Decode->encode is corpus-proven byte-identity (P1), so a null edit is a no-op.
	uint nRpo = 0, nSnap = 0, fails = 0;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		if (CRklPatchObject *rpo = dynamic_cast<CRklPatchObject *>(it->second))
		{
			const CStorageRaw *raw = rpo->rpoChunk();
			if (!raw) continue;
			SRPatchMesh rp;
			std::string err;
			if (!decodeRpoChunk(nlVectorData(raw->Value), raw->Value.size(), rp, err))
			{ fprintf(stderr, "ERROR: 0x08fd decode: %s\n", err.c_str()); ++fails; continue; }
			if (!rpo->setRPatch(rp))
			{ fprintf(stderr, "ERROR: setRPatch failed\n"); ++fails; continue; }
			++nRpo;
			continue;
		}
		CDerivedObject *d = dynamic_cast<CDerivedObject *>(it->second);
		if (!d) continue;
		for (uint i = 0; i < d->modifierCount(); ++i)
		{
			CStorageContainer *data = dynamic_cast<CStorageContainer *>(d->localModData(i));
			if (!data) continue;
			CStorageContainer *wrap = containerChild(data, 0x1000);
			if (!wrap) continue;
			CStorageRaw *rfp = rawChildOf(wrap, 0x4001);
			if (!rfp) continue;
			SRPatchMesh rp;
			std::string err;
			if (!decodeRPatchMesh(nlVectorData(rfp->Value), rfp->Value.size(), rp, err))
			{ fprintf(stderr, "ERROR: 0x4001 decode: %s\n", err.c_str()); ++fails; continue; }
			encodeRPatchMesh(rp, rfp->Value);
			++nSnap;
		}
	}
	if (fails) return 1;

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

	printf("%s null-edit: %u rpo, %u snapshots, %u stream diffs -> %s\n",
	       diffs ? "FAIL" : "OK", nRpo, nSnap, diffs, output.c_str());

	delete lm.Scene;
	delete lm.Cd;
	delete lm.Dll;
	return diffs ? 1 : 0;
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

static int runViewer(std::vector<SPaintZone> &zones, const std::string &bankPath, bool bankRecursive,
                     const std::vector<std::string> &searchPaths, const std::string &screenshotPath)
{
	// The tile bank (the plugin took it from the tile_utility choice; here it is --bank).
	NL3D::CTileBank bank;
	try
	{
		NLMISC::CIFile file;
		if (!file.open(bankPath)) { fprintf(stderr, "ERROR: cannot open bank %s\n", bankPath.c_str()); return 1; }
		bank.serial(file);
		bank.computeXRef();
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: bank: %s\n", e.what());
		return 1;
	}
	// Make the tile texture paths resolvable: relative names looked up through CPath, seeded
	// with the bank file's directory (recursive on request).
	bank.makeAllPathRelative();
	bank.setAbsPath("");
	NLMISC::CPath::addSearchPath(NLMISC::CFile::getPath(bankPath), bankRecursive, false);
	for (size_t i = 0; i < searchPaths.size(); ++i)
		NLMISC::CPath::addSearchPath(searchPaths[i], true, false);

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

		theLand->enableAdditive(true);

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
				NLMISC::CMatrix camKey = mouseListener.getViewMatrix();
				NL3D::CNELU::Camera->setMatrix(camKey);
				NL3D::CNELU::clearBuffers(NLMISC::CRGBA(90, 90, 90));
				NL3D::CNELU::Scene->render();
				if (theLand->Landscape.getRefineMode())
				{
					theLand->Landscape.setRefineMode(false);
					theLand->Landscape.refineAll(camKey.getPos());
				}
				NL3D::CNELU::swapBuffers();
				NL3D::CNELU::screenshot(); // F12, same convenience as the other NeL viewers
			}
			while (!NL3D::CNELU::AsyncListener.isKeyPushed(NLMISC::KeyESCAPE) && closeListener.WindowActive);
		}

		mouseListener.removeFromServer(NL3D::CNELU::EventServer);
		NL3D::CNELU::EventServer.removeListener(NLMISC::EventDestroyWindowId, &closeListener);
		NL3D::CNELU::EventServer.removeListener(NLMISC::EventCloseWindowId, &closeListener);
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
	return 0;
}

// ---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext applicationContext;

	NLMISC::CCmdArgs args;
	args.setDescription("Standalone zone painter (design doc \xc2\xa7" "14-paint). Default mode opens the "
	                    "painting landscape viewer; the headless modes need no 3D driver.");
	args.addAdditionalArg("input.max", "Input .max scene");
	args.addArg("", "bank", "bank", "Tile bank (.smallbank/.bank); required for the viewer/screenshot modes");
	args.addArg("", "bank-recursive", "", "Add the bank directory to the texture search path recursively");
	args.addArg("", "search-path", "dir", "Extra recursive texture search path (repeatable)", false);
	args.addArg("", "out", "output.max", "Output .max for --null-edit (in-place save is refused)");
	args.addArg("", "cellsize", "meters", "Ligo cell size (default 100; reserved for the painting core)");
	args.addArg("", "snap", "meters", "Ligo snap (default 1; reserved for the painting core)");
	args.addArg("", "null-edit", "", "Headless: decode+re-encode every RPatchMesh blob, rebuild, save to --out");
	args.addArg("", "verify-identical", "", "With --null-edit: byte-compare the output against the input");
	args.addArg("", "dump-zones", "dir", "Headless: write every built display CZone and report counts");
	args.addArg("", "screenshot", "out.tga", "Render one frame to a .tga and exit");
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
	(void)cellSize; // the painting core (P3b) consumes these for the zone-symmetry state
	(void)snap;

	if (args.haveLongArg("null-edit"))
	{
		if (!args.haveLongArg("out"))
		{
			fprintf(stderr, "ERROR: --null-edit refuses to save in place; give --out <output.max>\n");
			return 1;
		}
		return nullEditSave(input, args.getLongArg("out")[0], args.haveLongArg("verify-identical"));
	}

	// Landscape modes.
	NL3D::registerSerial3d();
	PMAXLOAD::SLoadedMax lm;
	if (!PMAXLOAD::loadMaxFile(input, lm)) { fprintf(stderr, "ERROR: cannot load %s\n", input.c_str()); return 1; }

	std::vector<SPaintZone> zones;
	if (!buildPaintZones(*lm.Scene, zones))
	{
		fprintf(stderr, "ERROR: no displayable RklPatch zone in %s\n", input.c_str());
		return 1;
	}
	uint welds = weldPaintZones(zones);
	if (g_verbose) printf("weld: %u cross-zone edges\n", welds);

	if (args.haveLongArg("dump-zones"))
		return dumpZones(zones, welds, args.getLongArg("dump-zones")[0]);

	if (bankPath.empty())
	{
		fprintf(stderr, "ERROR: the viewer/screenshot modes need --bank <bank.smallbank>\n");
		return 1;
	}
	std::vector<std::string> searchPaths;
	if (args.haveLongArg("search-path")) searchPaths = args.getLongArg("search-path");
	std::string screenshotPath = args.haveLongArg("screenshot") ? args.getLongArg("screenshot")[0] : std::string();
	return runViewer(zones, bankPath, bankRecursive, searchPaths, screenshotPath);
}

/* end of file */
