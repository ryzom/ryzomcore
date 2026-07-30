/**
 * \file patch_topo_ops.cpp
 * \brief Topological patch-edit operations: the Tier B ops that change element counts.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The zone-painter side of the topology transforms in nelpatch/patch_topo: resolve the
 * write-target stream (the TOPMOST edit-class modifier slot carrying an RFINALPATCH 0x4001,
 * else the base RklPatch - the same slot the tile-carrier and geometry policies pick, and
 * the slot whose stored state the stack eval replays wholesale), flush pending paint,
 * run the struct transform, re-encode the PatchMesh stream + RPatchMesh blob + vertex
 * mapper in place, and rebuild the working set so the display, the paint core and the
 * landscape re-derive from the mutated storage.
 *
 * Paint survival is the acceptance bar: the per-patch paint records travel with their
 * surviving patches through the transform's remap, so a painted zone keeps its paint
 * everywhere the surface survived - the thing the legacy editor destroyed.
 *
 * Topological ops are undoable through Kind 6 raw snapshots (patch_topo_snapshot.h): the
 * pre/post structs land as one stroke after a successful rebuild, the rebuild preserves
 * the stacks (keepUndo), and undo/redo re-encode the matching side then rebuild again -
 * byte-exact by the codec identity.
 */

/*
 * Copyright (C) 2026 by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>

#include <nel/misc/aabbox.h>
#include <nel/misc/app_context.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/cmd_args.h>
#include <nel/misc/common.h>
#include <nel/misc/config_file.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/event_server.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/camera.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/dru.h>
#include <nel/3d/event_mouse_listener.h>
#include <nel/3d/font_manager.h>
#include <nel/3d/landscape.h>
#include <nel/3d/landscape_model.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/scene.h>
#include <nel/3d/scene_user.h>
#include <nel/3d/text_context.h>
#include <nel/3d/texture_mem.h>
#include <nel/3d/tile_bank.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/viewport.h>
#include <nel/3d/zone.h>
#include <nel/3d/zone_corner_smoother.h>
#include <nel/3d/zone_symmetrisation.h>

#include <nel/ligo/ligo_config.h>
#include <nel/ligo/ligo_error.h>
#include <nel/ligo/zone_template.h>
#include <nel/ligo/zone_region.h>
#include <nel/ligo/zone_bank.h>

#include "../pipeline_max/storage_ole.h"
#include "max_thumbnail.h"

#include <algorithm>
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
#include "../pipeline_max/nelpatch/patch_topo.h"

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
#include "../pipeline_max_export_common/db_path.h"
#include "../pipeline_max_export_common/appdata_util.h"
#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::NELPATCH;
using namespace MAXMATH;

#include "../pipeline_max_export_common/patch_eval.h"

#include "paint_core.h"
#include "context_display.h"
#include "editor_ui.h"
#include "workspace_discovery.h"
#include "startup_ui.h"
#include "script_api.h"

#include <nel/3d/nav_mouse_listener.h>

#include "zp_state.h"
#include "patch_topo_snapshot.h"

// ---------------------------------------------------------------------------------------------

namespace {

/**
 * The stream a topological edit rewrites for one node: the topmost edit-class modifier
 * slot whose per-node local data carries an RFINALPATCH 0x4001 (its stored output is what
 * both our eval and host replay make authoritative), else the base RklPatch. Same slot
 * the tile-carrier policy picks.
 */
struct STopoTarget
{
	CRklPatchObject *Rpo; ///< base object (always set; the target when Local is NULL)
	CStorageContainer *Local; ///< topmost carrier slot's 0x1000 local data, or NULL
	CStorageContainer *PmChunk; ///< Local's 0x1140 (modifier target only)
	CStorageRaw *RpRaw; ///< Local's 0x4001 (modifier target only)
	CStorageRaw *MapperRaw; ///< Local's 0x1130 -> 0x1000 payload, or NULL
	STopoTarget() : Rpo(NULL), Local(NULL), PmChunk(NULL), RpRaw(NULL), MapperRaw(NULL) { }
};

bool resolveTopoTarget(CNodeImpl *node, STopoTarget &out, std::string &err)
{
	CSceneClass *obj = dynamic_cast<CSceneClass *>(node->getReference(1));
	if (!obj) { err = "node without object"; return false; }
	// Walk the derived chain top-first, mirroring the eval's modifier collection.
	int guard = 8;
	while (obj && guard-- > 0)
	{
		CDerivedObject *derived = dynamic_cast<CDerivedObject *>(obj);
		if (!derived) break;
		for (uint m = 0; m < derived->modifierCount() && !out.Local; ++m)
		{
			NLMISC::CClassId mcid = derived->modifier(m)->classDesc()->classId();
			if (mcid != CLASSID_NEL_EDIT_PATCH && mcid != CLASSID_NEL_PATCH_PAINT)
				continue;
			CStorageContainer *localData = editPatchLocalData(derived, m);
			if (!localData)
				continue;
			CStorageContainer *pmChunk = containerChild(localData, 0x1140);
			CStorageRaw *rpRaw = rawChildOf(localData, 0x4001);
			if (!pmChunk || !rpRaw)
				continue; // legacy record-format slot: not a carrier
			out.Local = localData;
			out.PmChunk = pmChunk;
			out.RpRaw = rpRaw;
			if (CStorageContainer *mapperC = containerChild(localData, 0x1130))
				out.MapperRaw = rawChildOf(mapperC, 0x1000);
		}
		CSceneClass *base = derived->baseObject();
		if (!base) { err = "derived object without base object"; return false; }
		obj = base;
	}
	out.Rpo = dynamic_cast<CRklPatchObject *>(obj);
	if (!out.Rpo) { err = "object is not an RklPatch"; return false; }
	return true;
}

/** Decode the target stream's pm/rp/mapper. */
bool decodeTopoTarget(const STopoTarget &t, SPatchMesh &pm, SRPatchMesh &rp,
                      SPmVertMapper &mapper, bool &haveMapper, std::string &err)
{
	haveMapper = false;
	if (t.Local)
	{
		if (!decodePatchMesh(t.PmChunk->chunks(), pm, err)) { err = "0x1140: " + err; return false; }
		if (!decodeRPatchMesh(nlVectorData(t.RpRaw->Value), t.RpRaw->Value.size(), rp, err))
		{ err = "0x4001: " + err; return false; }
		if (t.MapperRaw)
		{
			if (!decodeVertMapper(nlVectorData(t.MapperRaw->Value), t.MapperRaw->Value.size(), mapper, err))
			{ err = "0x1130: " + err; return false; }
			haveMapper = true;
		}
		return true;
	}
	if (!t.Rpo->decodePatch(pm, err)) { err = "base PatchMesh: " + err; return false; }
	if (!t.Rpo->decodeRPatch(rp, err)) { err = "base RPatchMesh: " + err; return false; }
	return true;
}

/** Encode the mutated pm/rp/mapper back into the target stream. */
bool encodeTopoTarget(const STopoTarget &t, const SPatchMesh &pm, const SRPatchMesh &rp,
                      const SPmVertMapper &mapper, bool haveMapper, std::string &err)
{
	if (t.Local)
	{
		if (!encodePatchMesh(pm, t.PmChunk->chunksMut(), err)) { err = "0x1140: " + err; return false; }
		encodeRPatchMesh(rp, t.RpRaw->Value);
		if (haveMapper && t.MapperRaw)
			encodeVertMapper(mapper, t.MapperRaw->Value);
		return true;
	}
	if (!t.Rpo->setPatchMesh(pm, err)) { err = "base PatchMesh: " + err; return false; }
	if (!t.Rpo->setRPatch(rp)) { err = "base RPatchMesh: no 0x08fd chunk"; return false; }
	return true;
}

/** The editable-file record owning a zone id, or NULL (synthetic/startup zones). */
SEditableFileInfo *zpZoneEditableFile(uint zoneId)
{
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
		for (size_t z = 0; z < g_EditableFiles[i].ZoneIds.size(); ++z)
			if (g_EditableFiles[i].ZoneIds[z] == zoneId)
				return &g_EditableFiles[i];
	return NULL;
}

/** The editable-file record of an OBJECT, through its in-file editable zone (a topo op can
 *  be addressed through a session-added instance, whose id no file record lists). Returns
 *  the file zone's id in `fileZoneOut` so cell replays resolve the same file later. */
SEditableFileInfo *zpObjectEditableFile(const void *node, uint &fileZoneOut)
{
	if (!g_PaintCtx.Zones)
		return NULL;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (size_t z = 0; z < zones.size(); ++z)
		if ((const void *)zones[z].Node == node && zones[z].InFile && zones[z].Editable)
		{
			fileZoneOut = zones[z].ZoneId;
			return zpZoneEditableFile(zones[z].ZoneId);
		}
	return NULL;
}

} /* anonymous namespace */

// ---------------------------------------------------------------------------------------------

/** One topology transform over the decoded target streams (delete, turn, ...). */
typedef bool (*TTopoXform)(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                           const std::set<uint> &sel, std::string &err,
                           const SPatchMesh *evalPm);

/**
 * Run one topological op over the patch selection (patch sub-object level).
 *
 * Per affected object: flush pending paint into the carrier, transform the target stream,
 * re-encode in place, then rebuild the working set once so display, core and landscape
 * re-derive from storage. Selections clear (indices may have died with the old topology)
 * and the pre/post snapshots land as a Kind 6 undo stroke after a successful rebuild.
 */
static uint zpRunTopoOpImpl(const char *opName, const std::string &recordLine, TTopoXform xf,
                            const std::map<const void *, std::set<uint> > &perObjectIn,
                            std::map<const void *, uint> &objectZone)
{
	const std::map<const void *, std::set<uint> > &perObject = perObjectIn;
	if (perObject.empty())
	{
		g_PropStatusMsg = std::string(opName) + ": selection has no editable elements";
		printf("%s\n", g_PropStatusMsg.c_str());
		fflush(stdout);
		return 0;
	}

	// Pending paint lives in the core's pristine copies; the transform reads and rewrites
	// the CARRIER BYTES, so everything unsaved flushes first. The rebuild afterwards skips
	// its own write-back for the same reason - it would re-encode the stale pre-delete
	// pristine over the transformed blob.
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_PropStatusMsg = std::string(opName) + ": paint write-back failed: " + err;
		return 0;
	}

	uint deleted = 0;
	std::vector<uint> touchedZones;
	std::vector<ZPPAINT::STopoSnapshot *> snaps;
	for (std::map<const void *, std::set<uint> >::const_iterator ot = perObject.begin();
	     ot != perObject.end(); ++ot)
	{
		const uint zoneId = objectZone[ot->first];
		SPaintZone *pz = zpFindPaintZoneMut(zoneId);
		if (!pz)
			continue;
		STopoTarget target;
		if (!resolveTopoTarget(pz->Node, target, err))
		{
			g_PropStatusMsg = std::string(opName) + ": " + err;
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			continue;
		}
		SPatchMesh pm;
		SRPatchMesh rp;
		SPmVertMapper mapper;
		bool haveMapper = false;
		if (!decodeTopoTarget(target, pm, rp, mapper, haveMapper, err))
		{
			g_PropStatusMsg = std::string(opName) + ": " + err;
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			continue;
		}
		// Full element-count match: the transforms read effective positions through the
		// eval mirror, so the mirror must be parallel to the stored stream, not merely
		// patch-count equal.
		if (pm.Patches.size() != pz->Patches.size()
		    || pm.Verts.size() != pz->Ep.Pm.Verts.size()
		    || pm.Vecs.size() != pz->Ep.Pm.Vecs.size())
		{
			g_PropStatusMsg = std::string(opName) + ": stored stream does not match the displayed topology";
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			continue;
		}
		// Undo payload: the decoded pre-op structs (post-writeBack, so the flushed paint is
		// part of the restored state) and, below, the post-op structs. Restoring either
		// side re-encodes it; decode -> encode identity makes the undo save byte-exact.
		ZPPAINT::STopoSnapshot *snap = new ZPPAINT::STopoSnapshot();
		snap->Zone = zoneId;
		snap->PmOld = pm;
		snap->RpOld = rp;
		snap->HaveMapper = haveMapper && target.MapperRaw;
		if (snap->HaveMapper)
			snap->MapperOld = target.MapperRaw->Value;
		// Anchor-cell fixup, the attach/move rule generalized: any topological op can move
		// the authored footprint ORIGIN (delete the west row, grow a quad past the corner,
		// weld a rim vertex away), and board placement re-derives from that origin on every
		// rebuild - without the fixup the survivors slide to keep the new origin at the
		// anchor cell. Captured before the encode, applied after it; the snapshot replays
		// the delta both ways, through the file's own zone id.
		uint fileZone = zoneId;
		SEditableFileInfo *efi = zpZoneEditableFile(zoneId);
		if (!efi)
			efi = zpObjectEditableFile(ot->first, fileZone); // addressed through an instance
		float oxPre = 0.f, oyPre = 0.f;
		bool haveOrigin = false;
		if (efi && g_SessionCellSize > 0.f)
		{
			SNodeTMCache tmCache;
			haveOrigin = zoneNodeAuthoredFootprintOrigin(pz->Node, tmCache, g_SessionCellSize,
			                                             oxPre, oyPre);
		}
		if (getenv("ZP_TOPO_DEBUG_GRID"))
		{
			const uint dp = *ot->second.begin();
			if (dp < rp.Patches.size())
			{
				const SRpoPatch &up = rp.Patches[dp];
				fprintf(stderr, "TOPO PRE p%u %dx%d:", dp, 1 << up.NbTilesU, 1 << up.NbTilesV);
				for (size_t t = 0; t < up.Tiles.size(); ++t)
					fprintf(stderr, " %d", up.Tiles[t].Layer[0].Tile);
				fprintf(stderr, "\n");
			}
		}
		if (!xf(pm, rp, haveMapper ? &mapper : NULL, ot->second, err, &pz->Ep.Pm))
		{
			g_PropStatusMsg = std::string(opName) + ": " + err;
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			delete snap;
			continue;
		}
		if (getenv("ZP_TOPO_DEBUG_GRID"))
		{
			const uint dp = *ot->second.begin();
			if (dp < rp.Patches.size())
			{
				const SRpoPatch &up = rp.Patches[dp];
				fprintf(stderr, "TOPO POST p%u %dx%d:", dp, 1 << up.NbTilesU, 1 << up.NbTilesV);
				for (size_t t = 0; t < up.Tiles.size(); ++t)
					fprintf(stderr, " %d", up.Tiles[t].Layer[0].Tile);
				fprintf(stderr, "\n");
			}
		}
		if (!encodeTopoTarget(target, pm, rp, mapper, haveMapper, err))
		{
			// The encoder mutates the chunk list stream by stream, so a failure can leave
			// the carrier half-written while display and core still hold the pre-op state.
			// Put the pre-op structs back through the same codec (the proven identity path)
			// so the carrier never keeps a partial state a later save would write out.
			std::string rerr;
			SPmVertMapper unusedMapper;
			if (!encodeTopoTarget(target, snap->PmOld, snap->RpOld, unusedMapper, false, rerr))
				fprintf(stderr, "ERROR: %s: pre-op restore failed: %s\n", opName, rerr.c_str());
			else if (snap->HaveMapper)
				target.MapperRaw->Value = snap->MapperOld;
			g_PropStatusMsg = std::string(opName) + ": " + err;
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			delete snap;
			continue;
		}
		snap->PmNew = pm;
		snap->RpNew = rp;
		if (snap->HaveMapper)
			snap->MapperNew = target.MapperRaw->Value;
		if (haveOrigin)
		{
			SNodeTMCache tmCache;
			float oxPost = 0.f, oyPost = 0.f;
			if (zoneNodeAuthoredFootprintOrigin(pz->Node, tmCache, g_SessionCellSize,
			                                    oxPost, oyPost))
			{
				const int cdx = (int)floor((oxPost - oxPre) / g_SessionCellSize + 0.5);
				const int cdy = (int)floor((oyPost - oyPre) / g_SessionCellSize + 0.5);
				if (cdx || cdy)
				{
					// The restore resolves the file from the snapshot's zone, so the record
					// carries the FILE zone (the op may have been addressed via an instance).
					snap->Zone = fileZone;
					snap->CellDX = cdx;
					snap->CellDY = cdy;
					efi->CellX += cdx;
					efi->CellY += cdy;
				}
			}
		}
		snaps.push_back(snap);
		deleted += (uint)ot->second.size();
		touchedZones.push_back(zoneId);
		// Unconditional: the target-stream choice is what the gates pin (m34 pins the
		// geometry write target the same way).
		printf("%s: zone %u, %u patches (%s target)\n", opName, zoneId,
		       (uint)ot->second.size(), target.Local ? "modifier" : "base");
		fflush(stdout);
	}
	if (!deleted)
	{
		for (size_t i = 0; i < snaps.size(); ++i)
			delete snaps[i];
		return 0;
	}

	ZPSCRIPT::record(recordLine);

	// Old indices may have died with the old topology; clear every sub-object selection directly
	// (the recorded op line replays the whole delete, no per-set records needed).
	g_PatchVertSel.clear();
	g_PatchEdgeSel.clear();
	g_PatchFaceSel.clear();
	g_PatchTanSel.clear();
	zpPatchGizmoInvalidate();

	uint welds = 0;
	if (!rebuildWorkingSet(err, welds, /* skipWriteBack= */ true, /* keepUndo= */ true))
	{
		g_PropStatusMsg = std::string(opName) + ": session rebuild failed: " + err;
		for (size_t i = 0; i < snaps.size(); ++i)
			delete snaps[i];
		return deleted;
	}
	// The undo stroke lands AFTER the rebuild (keepUndo preserved the stacks), so a
	// failed rebuild never leaves a restorable record for a session in an unknown state.
	g_PaintCtx.Core->opTopoStroke(snaps);
	// The PatchMesh stream mutation is invisible to the carrier blob compare (same reason
	// vertex moves are); the blob itself also changed, but mark the geometry flag so the
	// dirty signal never depends on that coincidence.
	for (size_t i = 0; i < touchedZones.size(); ++i)
		g_PaintCtx.Core->markGeomDirty(touchedZones[i]);
	g_PropStatusMsg = NLMISC::toString(
		"%s: %u element%s", opName, deleted, deleted == 1 ? "" : "s");
	return deleted;
}

/** Face-selection front-end (delete / turn / subdivide). */
static uint zpRunTopoOp(const char *opName, const std::string &recordLine, TTopoXform xf)
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Zones)
		return 0;
	if (g_PatchFaceSel.empty())
	{
		g_PropStatusMsg = std::string(opName) + ": no patches selected";
		printf("%s\n", g_PropStatusMsg.c_str());
		fflush(stdout);
		return 0;
	}
	// Group the selection per OBJECT: indices are object-level, and two zones of one
	// object address the same storage.
	std::map<const void *, std::set<uint> > perObject;
	std::map<const void *, uint> objectZone;
	for (std::set<TPatchFaceId>::const_iterator it = g_PatchFaceSel.begin();
	     it != g_PatchFaceSel.end(); ++it)
	{
		const SPaintZone *pz = zpFindPaintZone(it->first);
		if (!pz || !pz->Editable)
			continue;
		if (it->second >= pz->Patches.size())
			continue;
		perObject[(const void *)pz->Node].insert(it->second);
		objectZone[(const void *)pz->Node] = it->first;
	}
	return zpRunTopoOpImpl(opName, recordLine, xf, perObject, objectZone);
}

/** Edge-selection front-end (add quad): the selection's corner pairs resolve to pm edge
 *  indices through each zone's eval mirror. */
static uint zpRunTopoOpEdges(const char *opName, const std::string &recordLine, TTopoXform xf)
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Zones)
		return 0;
	if (g_PatchEdgeSel.empty())
	{
		g_PropStatusMsg = std::string(opName) + ": no edges selected";
		printf("%s\n", g_PropStatusMsg.c_str());
		fflush(stdout);
		return 0;
	}
	std::map<const void *, std::set<uint> > perObject;
	std::map<const void *, uint> objectZone;
	for (std::set<SPatchEdgeId>::const_iterator it = g_PatchEdgeSel.begin();
	     it != g_PatchEdgeSel.end(); ++it)
	{
		const SPaintZone *pz = zpFindPaintZone(it->Zone);
		if (!pz || !pz->Editable)
			continue;
		const PIPELINE::MAX::NELPATCH::SPatchMesh &pm = pz->Ep.Pm;
		for (size_t e = 0; e < pm.Edges.size(); ++e)
			if ((pm.Edges[e].V1 == (sint32)it->A && pm.Edges[e].V2 == (sint32)it->B)
			    || (pm.Edges[e].V1 == (sint32)it->B && pm.Edges[e].V2 == (sint32)it->A))
			{
				perObject[(const void *)pz->Node].insert((uint)e);
				objectZone[(const void *)pz->Node] = it->Zone;
				break;
			}
	}
	return zpRunTopoOpImpl(opName, recordLine, xf, perObject, objectZone);
}

/** Vertex-selection front-end (weld). */
static uint zpRunTopoOpVerts(const char *opName, const std::string &recordLine, TTopoXform xf)
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Zones)
		return 0;
	if (g_PatchVertSel.empty())
	{
		g_PropStatusMsg = std::string(opName) + ": no vertices selected";
		printf("%s\n", g_PropStatusMsg.c_str());
		fflush(stdout);
		return 0;
	}
	std::map<const void *, std::set<uint> > perObject;
	std::map<const void *, uint> objectZone;
	for (std::set<TPatchVertId>::const_iterator it = g_PatchVertSel.begin();
	     it != g_PatchVertSel.end(); ++it)
	{
		const SPaintZone *pz = zpFindPaintZone(it->first);
		if (!pz || !pz->Editable)
			continue;
		perObject[(const void *)pz->Node].insert((uint)it->second);
		objectZone[(const void *)pz->Node] = it->first;
	}
	return zpRunTopoOpImpl(opName, recordLine, xf, perObject, objectZone);
}

/** Adapter: delete (the remap is internal to the transform; callers observe via rebuild). */
static bool zpXformDelete(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                          const std::set<uint> &sel, std::string &err,
                          const SPatchMesh * /* evalPm */)
{
	STopoRemap remap;
	return topoDeletePatches(pm, rp, mapper, sel, remap, err);
}

static bool zpXformTurnCcw(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper * /* mapper */,
                           const std::set<uint> &sel, std::string &err,
                           const SPatchMesh * /* evalPm */)
{
	return topoTurnPatches(pm, rp, sel, true, err);
}

static bool zpXformTurnCw(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper * /* mapper */,
                          const std::set<uint> &sel, std::string &err,
                          const SPatchMesh * /* evalPm */)
{
	return topoTurnPatches(pm, rp, sel, false, err);
}

static bool zpXformSubdivide(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                             const std::set<uint> &sel, std::string &err,
                             const SPatchMesh *evalPm)
{
	// Structurally the mapper is a pure addition (input-indexed slots keep, added outputs
	// stay unmapped), but the REUSED tangent/interior slots may be mapped outputs - the
	// transform shifts their deltas and reads the split curves off the eval mirror.
	return topoSubdividePatches(pm, rp, sel, err, mapper, evalPm);
}

// Weld threshold handed to the adapter through a file-static: the shared runner's
// transform signature is selection-only, and the threshold is op state like the brush
// size, not selection data.
static float s_WeldThreshold = 0.1f;
static bool zpXformWeld(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                        const std::set<uint> &sel, std::string &err,
                        const SPatchMesh *evalPm)
{
	STopoRemap remap;
	return topoWeldVerts(pm, rp, mapper, sel, s_WeldThreshold, remap, err, evalPm);
}

// The directed weld's endpoints ride file-statics the same way (op state, not selection).
static uint s_WeldIntoSrc = 0, s_WeldIntoDst = 0;
static bool zpXformWeldInto(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                            const std::set<uint> & /* sel */, std::string &err,
                            const SPatchMesh * /* evalPm */)
{
	STopoRemap remap;
	return topoWeldVertInto(pm, rp, mapper, s_WeldIntoSrc, s_WeldIntoDst, remap, err);
}

static bool zpXformAddQuad(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper * /* mapper */,
                           const std::set<uint> &sel, std::string &err,
                           const SPatchMesh *evalPm)
{
	// Pure addition: the mapper needs no rewrite (input slots keep, new outputs unmapped);
	// the mirror seeds are computed off the eval mirror's positions.
	return topoAddQuads(pm, rp, sel, err, evalPm);
}

static bool zpXformDetachElement(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper * /* mapper */,
                                 const std::set<uint> &sel, std::string &err,
                                 const SPatchMesh *evalPm)
{
	// Pure addition again (duplicated boundary): input mapper slots keep, new outputs
	// stay unmapped, nothing dies; the duplicates copy EVAL positions so a mapper-driven
	// boundary stays an invisible seam.
	return topoDetachElements(pm, rp, sel, err, evalPm);
}

/**
 * Delete the selected patches. Paint and bind records travel with the survivors
 * (topoDeletePatches); everything else is the shared topo-op skeleton.
 */
uint zpDeletePatchSelection()
{
	return zpRunTopoOp("delete", "painter.deletePatchSelection()", zpXformDelete);
}

/**
 * Turn the selected quad patches a quarter turn (the legacy tile-frame rotation). The
 * painted grid transposes with its layers rotated, so the painted appearance follows the
 * frame; binds onto a turned patch follow the edge ring.
 */
uint zpTurnPatchSelection(bool ccw)
{
	return zpRunTopoOp(ccw ? "turn ccw" : "turn cw",
	                   ccw ? "painter.turnPatchSelection(true)"
	                       : "painter.turnPatchSelection(false)",
	                   ccw ? zpXformTurnCcw : zpXformTurnCw);
}

/**
 * Subdivide the selected quad patches into four children each. Exact bicubic split, the
 * painted quadrants inherit verbatim (halved tile orders), and midpoints on edges shared
 * with unselected neighbors become canonical T-junction binds.
 */
uint zpSubdividePatchSelection()
{
	return zpRunTopoOp("subdivide", "painter.subdividePatchSelection()", zpXformSubdivide);
}

/**
 * Weld the selected vertices (target-weld: clusters within the threshold merge onto their
 * lowest member, which keeps its position; coincident open edges fuse - the stitch).
 * Vertex-level selection; the runner's face-selection guard is bypassed by feeding the
 * vertex set through the same per-object grouping. The confirmed distance becomes the
 * next dialog's seed.
 */
uint zpWeldPatchSelection(float threshold)
{
	s_WeldThreshold = threshold > 0.f ? threshold : 0.1f;
	return zpRunTopoOpVerts("weld", NLMISC::toString("painter.weldPatchSelection(%.9g)",
	                                                 s_WeldThreshold), zpXformWeld);
}

/** The last-used weld distance (seeds the panel dialog). */
float zpLastWeldThreshold() { return s_WeldThreshold; }

/** Weld dialog OK. */
void zpPatchWeldThresholdClicked(float distance) { zpWeldPatchSelection(distance); }

/**
 * Grow one new quad from each selected OPEN edge (the legacy Add Quad; edge level). The
 * seed mirrors the owner across the edge; the artist moves the fresh corners into place
 * and welds.
 */
uint zpAddQuadPatchSelection()
{
	return zpRunTopoOpEdges("add quad", "painter.addQuadPatchSelection()", zpXformAddQuad);
}

/**
 * Directed weld: `srcVert` merges into `dstVert` of the same zone object, the target
 * keeping its position and identity - the drag-onto-a-vertex gesture's op. Rides the
 * shared runner with a single-vertex per-object set; the endpoints travel by op state.
 */
uint zpWeldVertexInto(uint zoneId, uint srcVert, uint dstVert)
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Zones)
		return 0;
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz || !pz->Editable)
	{
		g_PropStatusMsg = "weld: zone is read-only";
		return 0;
	}
	s_WeldIntoSrc = srcVert;
	s_WeldIntoDst = dstVert;
	std::map<const void *, std::set<uint> > perObject;
	std::map<const void *, uint> objectZone;
	perObject[(const void *)pz->Node].insert(srcVert);
	objectZone[(const void *)pz->Node] = zoneId;
	return zpRunTopoOpImpl("weld",
	                       NLMISC::toString("painter.weldVertexInto(%u, %u, %u)",
	                                        zoneId, srcVert, dstVert),
	                       zpXformWeldInto, perObject, objectZone);
}

// ---------------------------------------------------------------------------------------------
// Detach: split the selection off into a NEW BRICK FILE.
//
// In this tool one file carries one editable zone and the FILE is the brick identity, so
// the legacy "detach as a new object" maps to "detach as a new file": the new brick is the
// current file with the complement deleted, written through the atomic copy-save (source
// OLE streams verbatim, Scene re-encoded); the source then loses the selection through the
// normal delete flow. Both halves keep their paint by the delete transform's contract.
//
// The new file is NOT opened into the session - the zone set is unchanged, which is what
// keeps the source-side delete UNDOABLE (Kind 6). Undo restores the source zone; the new
// file stays on disk (it is a save, and saves are not undone). findWorldZone re-lists on
// a miss, so the fresh brick is openable from the board immediately.

namespace {

/** Owning file of a zone: path + scene (primary session file when not in the list). */
void zpZoneFilePathScene(uint zoneId, std::string &path, CScene *&scene)
{
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		const SEditableFileInfo &efi = g_EditableFiles[i];
		for (size_t z = 0; z < efi.ZoneIds.size(); ++z)
			if (efi.ZoneIds[z] == zoneId)
			{
				path = efi.Path;
				scene = editableScene(efi);
				return;
			}
	}
	path = g_PaintCtx.InputPath;
	scene = g_PaintCtx.Scene;
}

/** "name" sanitized to brick-filename characters, else empty. */
std::string zpSanitizeBrickName(const std::string &name)
{
	const std::string low = NLMISC::toLowerAscii(name);
	std::string out;
	for (size_t i = 0; i < low.size(); ++i)
	{
		const char c = low[i];
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')
			out += c;
	}
	return out;
}

} /* anonymous namespace */

/**
 * Detach the selection as its own ISLAND inside the same zone (detach-to-element; patch
 * level). The boundary splits - duplicated shared vertices and edges, copied curves - so
 * nothing moves and the zone still exports as one node; grab the island afterwards with
 * the Element expand. Kaetemi's reframing of legacy detach for the brick world.
 */
uint zpDetachPatchSelection()
{
	return zpRunTopoOp("detach", "painter.detachPatchSelection()", zpXformDetachElement);
}

/**
 * Detach the selected patches into a NEW BRICK FILE next to the source (patch level, one
 * zone at a time; SHELVED off the panel - script-only until the fresh-zone-file-creation
 * story is designed; splitting a multi-cell zone is its use). `nameIn` empty = auto
 * ("<source>-det", collision-bumped). Returns the detached patch count.
 */
uint zpDetachToFile(const std::string &nameIn)
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Zones)
		return 0;
	if (g_PatchFaceSel.empty())
	{
		g_PropStatusMsg = "detach: no patches selected";
		printf("%s\n", g_PropStatusMsg.c_str());
		fflush(stdout);
		return 0;
	}
	// One object only: the op writes one source file and one new file.
	const void *object = NULL;
	uint zoneId = 0;
	std::set<uint> sel;
	for (std::set<TPatchFaceId>::const_iterator it = g_PatchFaceSel.begin();
	     it != g_PatchFaceSel.end(); ++it)
	{
		const SPaintZone *pz = zpFindPaintZone(it->first);
		if (!pz || !pz->Editable || it->second >= pz->Patches.size())
			continue;
		if (object && (const void *)pz->Node != object)
		{
			g_PropStatusMsg = "detach: select patches in one zone only";
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			return 0;
		}
		object = (const void *)pz->Node;
		zoneId = it->first;
		sel.insert(it->second);
	}
	if (!object || sel.empty())
	{
		g_PropStatusMsg = "detach: selection has no editable elements";
		printf("%s\n", g_PropStatusMsg.c_str());
		fflush(stdout);
		return 0;
	}
	SPaintZone *pz = zpFindPaintZoneMut(zoneId);
	if (!pz)
		return 0;
	if (sel.size() >= pz->Patches.size())
	{
		g_PropStatusMsg = "detach: selection is the whole zone (save a copy instead)";
		printf("%s\n", g_PropStatusMsg.c_str());
		fflush(stdout);
		return 0;
	}

	// The owning file and its scene; the new brick lands in the same directory, which for
	// board sessions is the world dir (openable from the board immediately).
	std::string srcPath;
	CScene *scene = NULL;
	zpZoneFilePathScene(zoneId, srcPath, scene);
	if (srcPath.empty() || !scene || !NLMISC::CFile::fileExists(srcPath))
	{
		g_PropStatusMsg = "detach: source file unresolved";
		return 0;
	}
	const std::string dir = NLMISC::CFile::getPath(srcPath);
	const std::string srcBase = NLMISC::CFile::getFilenameWithoutExtension(srcPath);
	std::string name = zpSanitizeBrickName(nameIn);
	if (name.empty())
		name = srcBase + "-det";
	std::string target = dir + name + ".max";
	for (uint bump = 2; NLMISC::CFile::fileExists(target) || findEditableByPath(target); ++bump)
	{
		if (bump > 99)
		{
			g_PropStatusMsg = "detach: no free name near '" + name + "'";
			return 0;
		}
		target = dir + name + NLMISC::toString("%u", bump) + ".max";
	}

	// Flush pending paint: both passes below read and rewrite the CARRIER BYTES.
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_PropStatusMsg = "detach: paint write-back failed: " + err;
		return 0;
	}
	STopoTarget targetStream;
	if (!resolveTopoTarget(pz->Node, targetStream, err))
	{
		g_PropStatusMsg = "detach: " + err;
		return 0;
	}
	SPatchMesh pmPre;
	SRPatchMesh rpPre;
	SPmVertMapper mapperPre;
	bool haveMapper = false;
	if (!decodeTopoTarget(targetStream, pmPre, rpPre, mapperPre, haveMapper, err))
	{
		g_PropStatusMsg = "detach: " + err;
		return 0;
	}
	if (pmPre.Patches.size() != pz->Patches.size())
	{
		g_PropStatusMsg = "detach: stored stream does not match the displayed topology";
		return 0;
	}
	std::vector<uint8> mapperRawPre;
	if (haveMapper && targetStream.MapperRaw)
		mapperRawPre = targetStream.MapperRaw->Value;

	// --- Pass 1: the NEW file = complement deleted (the selection survives there).
	std::set<uint> complement;
	for (uint p = 0; p < (uint)pmPre.Patches.size(); ++p)
		if (!sel.count(p))
			complement.insert(p);
	{
		SPatchMesh pmNewFile = pmPre;
		SRPatchMesh rpNewFile = rpPre;
		SPmVertMapper mapperNewFile = mapperPre;
		STopoRemap remap;
		if (!topoDeletePatches(pmNewFile, rpNewFile, haveMapper ? &mapperNewFile : NULL,
		                       complement, remap, err))
		{
			g_PropStatusMsg = "detach: " + err;
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			return 0;
		}
		if (!encodeTopoTarget(targetStream, pmNewFile, rpNewFile, mapperNewFile, haveMapper, err))
		{
			g_PropStatusMsg = "detach: " + err;
			return 0;
		}
		const bool saved = saveCopyAtomic(srcPath, target, *scene, NULL);
		// Restore the pre-op streams before ANY error path: the carrier must never keep
		// the complement-deleted state meant for the new file.
		std::string rerr;
		if (!encodeTopoTarget(targetStream, pmPre, rpPre, mapperPre, haveMapper, rerr))
		{
			fprintf(stderr, "ERROR: detach: pre-op restore failed: %s\n", rerr.c_str());
			g_PropStatusMsg = "detach: pre-op restore failed: " + rerr;
			return 0;
		}
		if (haveMapper && targetStream.MapperRaw)
			targetStream.MapperRaw->Value = mapperRawPre;
		if (!saved)
		{
			g_PropStatusMsg = "detach: could not write " + target;
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			return 0;
		}
	}

	// --- Pass 2: the SOURCE loses the selection - the normal delete flow, Kind 6 undoable
	// (the zone set is unchanged; the new file is not opened).
	// Same anchor-cell fixup as the shared runner: detaching the west half of a multi-cell
	// zone moves the survivors' footprint origin, and without the cell shift the next
	// rebuild slides them to keep the new origin at the anchor. Captured here, after pass
	// 1's restore put the pre-op stream back.
	uint fileZone = zoneId;
	SEditableFileInfo *efi = zpZoneEditableFile(zoneId);
	if (!efi)
		efi = zpObjectEditableFile((const void *)pz->Node, fileZone);
	float oxPre = 0.f, oyPre = 0.f;
	bool haveOrigin = false;
	if (efi && g_SessionCellSize > 0.f)
	{
		SNodeTMCache tmCache;
		haveOrigin = zoneNodeAuthoredFootprintOrigin(pz->Node, tmCache, g_SessionCellSize,
		                                             oxPre, oyPre);
	}
	ZPPAINT::STopoSnapshot *snap = new ZPPAINT::STopoSnapshot();
	snap->Zone = zoneId;
	snap->PmOld = pmPre;
	snap->RpOld = rpPre;
	snap->HaveMapper = haveMapper && targetStream.MapperRaw;
	if (snap->HaveMapper)
		snap->MapperOld = mapperRawPre;
	{
		SPatchMesh pmSrc = pmPre;
		SRPatchMesh rpSrc = rpPre;
		SPmVertMapper mapperSrc = mapperPre;
		STopoRemap remap;
		if (!topoDeletePatches(pmSrc, rpSrc, haveMapper ? &mapperSrc : NULL, sel, remap, err)
		    || !encodeTopoTarget(targetStream, pmSrc, rpSrc, mapperSrc, haveMapper, err))
		{
			g_PropStatusMsg = "detach: " + err;
			printf("%s\n", g_PropStatusMsg.c_str());
			fflush(stdout);
			delete snap;
			return 0;
		}
		snap->PmNew = pmSrc;
		snap->RpNew = rpSrc;
		if (snap->HaveMapper)
			snap->MapperNew = targetStream.MapperRaw->Value;
	}
	if (haveOrigin)
	{
		SNodeTMCache tmCache;
		float oxPost = 0.f, oyPost = 0.f;
		if (zoneNodeAuthoredFootprintOrigin(pz->Node, tmCache, g_SessionCellSize,
		                                    oxPost, oyPost))
		{
			const int cdx = (int)floor((oxPost - oxPre) / g_SessionCellSize + 0.5);
			const int cdy = (int)floor((oyPost - oyPre) / g_SessionCellSize + 0.5);
			if (cdx || cdy)
			{
				snap->Zone = fileZone;
				snap->CellDX = cdx;
				snap->CellDY = cdy;
				efi->CellX += cdx;
				efi->CellY += cdy;
			}
		}
	}
	// The RESOLVED basename (collision bumps included) is what the recorder replays.
	name = NLMISC::CFile::getFilenameWithoutExtension(target);
	printf("detach: zone %u, %u patches -> %s (%s target)\n", zoneId, (uint)sel.size(),
	       target.c_str(), targetStream.Local ? "modifier" : "base");
	fflush(stdout);
	ZPSCRIPT::record("painter.detachToFile(\"" + name + "\")");
	g_PatchVertSel.clear();
	g_PatchEdgeSel.clear();
	g_PatchFaceSel.clear();
	g_PatchTanSel.clear();
	zpPatchGizmoInvalidate();
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds, /* skipWriteBack= */ true, /* keepUndo= */ true))
	{
		g_PropStatusMsg = "detach: session rebuild failed: " + err;
		delete snap;
		return (uint)sel.size();
	}
	std::vector<ZPPAINT::STopoSnapshot *> snaps(1, snap);
	g_PaintCtx.Core->opTopoStroke(snaps);
	g_PaintCtx.Core->markGeomDirty(zoneId);
	g_PropStatusMsg = NLMISC::toString("detach: %u patch%s -> %s.max",
	                                   (uint)sel.size(), sel.size() == 1 ? "" : "es", name.c_str());
	return (uint)sel.size();
}

/** Bridge wrapper: panel Detach button (auto name). */
void zpPatchDetachClicked() { zpDetachPatchSelection(); }

// ---------------------------------------------------------------------------------------------
// Attach: merge another open editable zone's patches into this one.
//
// The inverse of detach at the session level. The source zone's mesh is appended onto the
// target's write-target stream, reoriented through the two DISPLAY transforms so every
// patch keeps its place in the world; the source FILE is then saved and closed - its
// on-disk brick is untouched by the merge, so nothing is lost if the artist changes their
// mind. The close clears the undo history (a zone-set change, like any open/close); the
// attach itself then lands as the fresh stack's first Kind 6 stroke, so undo rolls the
// TARGET back to its pre-attach bytes while the closed source file simply stays closed.
//
// Positions come from the source's EVAL MIRROR (mapper-applied - a mapper-driven vertex's
// stored position is dead bytes); paint and binds come from the source's freshly-flushed
// STORED stream (the eval mirror's rp is a rebuild-time snapshot and would miss pending
// paint). The two agree on topology by construction, guarded below anyway.

namespace {

/** Row-major 3x4 (column-vector) form of src-object-space -> target-object-space. */
bool zpRelObjectTM(const SPaintZone &target, const SPaintZone &src, double out[12],
                   std::string &err)
{
	using namespace MAXMATH;
	if (getenv("ZP_ATTACH_DEBUG"))
	{
		for (int r = 0; r < 4; ++r)
			fprintf(stderr, "ATTACH tgt[%d]=(%g %g %g) src[%d]=(%g %g %g)\n", r,
			        target.DisplayTM.m[r][0], target.DisplayTM.m[r][1], target.DisplayTM.m[r][2],
			        r, src.DisplayTM.m[r][0], src.DisplayTM.m[r][1], src.DisplayTM.m[r][2]);
	}
	// Bitwise-equal display frames attach verbatim: no float noise on any position.
	if (memcmp(&target.DisplayTM, &src.DisplayTM, sizeof(Matrix3M)) == 0)
	{
		for (int i = 0; i < 12; ++i)
			out[i] = 0.0;
		out[0] = out[5] = out[10] = 1.0;
		return true;
	}
	// Row-vector convention: world = obj * DisplayTM, so rel = src * inverse(target).
	const Matrix3M rel = src.DisplayTM * inverseM3(target.DisplayTM);
	const double det =
		(double)rel.m[0][0] * ((double)rel.m[1][1] * rel.m[2][2] - (double)rel.m[1][2] * rel.m[2][1])
		- (double)rel.m[0][1] * ((double)rel.m[1][0] * rel.m[2][2] - (double)rel.m[1][2] * rel.m[2][0])
		+ (double)rel.m[0][2] * ((double)rel.m[1][0] * rel.m[2][1] - (double)rel.m[1][1] * rel.m[2][0]);
	if (det < 0.0)
	{
		err = "mirrored placement (patch winding would flip)";
		return false;
	}
	// Transpose into p' = M*p rows: out[4j+i] = rel.m[i][j], translation = rel.m[3][j].
	for (int j = 0; j < 3; ++j)
	{
		for (int i = 0; i < 3; ++i)
			out[4 * j + i] = (double)rel.m[i][j];
		out[4 * j + 3] = (double)rel.m[3][j];
	}
	return true;
}

} /* anonymous namespace */

/**
 * Merge zone `srcZone` into zone `targetZone` (both editable; the source must be an open
 * editable FILE, which is saved and closed on success). Returns the appended patch count.
 */
uint zpAttachZone(uint targetZone, uint srcZone, std::string &msg)
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Zones)
	{
		msg = "attach: no session";
		return 0;
	}
	SPaintZone *pzT = zpFindPaintZoneMut(targetZone);
	const SPaintZone *pzS = zpFindPaintZone(srcZone);
	if (!pzT || !pzS)
	{
		msg = "attach: no such zone";
		return 0;
	}
	if (!pzT->Editable || !pzS->Editable)
	{
		msg = "attach: both zones must be editable";
		return 0;
	}
	if (pzT->Node == pzS->Node)
	{
		msg = "attach: source and target are the same object";
		return 0;
	}
	SEditableFileInfo *srcFile = zpZoneEditableFile(srcZone);
	if (!srcFile)
	{
		msg = "attach: source is not an open editable file (attach the other way around)";
		return 0;
	}
	if (zpZoneEditableFile(targetZone) == srcFile)
	{
		msg = "attach: source and target live in the same file";
		return 0;
	}
	const std::string srcBase = srcFile->Basename;
	// The close below re-bases every zone id (ids are file-index-derived), so the target must
	// be re-found by its NODE - the one identity that survives the rebuild. The pre-close id
	// could land on a different file entirely once the indices shift.
	const void *tgtNode = (const void *)pzT->Node;
	const uint recTarget = targetZone, recSrc = srcZone; // the ids the user issued, for the record line

	double rel[12];
	if (!zpRelObjectTM(*pzT, *pzS, rel, msg))
	{
		msg = "attach: " + msg;
		return 0;
	}

	// Flush pending paint NOW: the source capture below reads its stored stream, and the
	// merged copy must carry everything unsaved.
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		msg = "attach: paint write-back failed: " + err;
		return 0;
	}

	// Source capture: stored stream for paint/binds/topology, eval-mirror positions.
	SPatchMesh srcPm;
	SRPatchMesh srcRp;
	{
		STopoTarget srcStream;
		SPmVertMapper srcMapper;
		bool srcHaveMapper = false;
		if (!resolveTopoTarget(pzS->Node, srcStream, err)
		    || !decodeTopoTarget(srcStream, srcPm, srcRp, srcMapper, srcHaveMapper, err))
		{
			msg = "attach: source: " + err;
			return 0;
		}
		const PIPELINE::MAX::NELPATCH::SPatchMesh &ev = pzS->Ep.Pm;
		if (srcPm.Verts.size() != ev.Verts.size() || srcPm.Vecs.size() != ev.Vecs.size()
		    || srcPm.Patches.size() != pzS->Patches.size())
		{
			msg = "attach: source stream does not match its displayed topology";
			return 0;
		}
		for (size_t i = 0; i < srcPm.Verts.size(); ++i)
			memcpy(srcPm.Verts[i].Pos, ev.Verts[i].Pos, 12);
		for (size_t i = 0; i < srcPm.Vecs.size(); ++i)
			memcpy(srcPm.Vecs[i].Pos, ev.Vecs[i].Pos, 12);
	}

	// Pre-validate the merge on copies BEFORE closing anything: a refusal (map channel,
	// hooks, Max 3 stream) must leave the session untouched.
	{
		STopoTarget tgtStream;
		SPatchMesh tgtPm;
		SRPatchMesh tgtRp;
		SPmVertMapper tgtMapper;
		bool tgtHaveMapper = false;
		if (!resolveTopoTarget(pzT->Node, tgtStream, err)
		    || !decodeTopoTarget(tgtStream, tgtPm, tgtRp, tgtMapper, tgtHaveMapper, err))
		{
			msg = "attach: " + err;
			return 0;
		}
		if (tgtPm.Patches.size() != pzT->Patches.size())
		{
			msg = "attach: stored stream does not match the displayed topology";
			return 0;
		}
		if (!topoAppendMesh(tgtPm, tgtRp, srcPm, srcRp, rel, err))
		{
			msg = "attach: " + err;
			printf("%s\n", msg.c_str());
			fflush(stdout);
			return 0;
		}
	}

	const uint appended = (uint)srcPm.Patches.size();

	// Save and close the source file (nested board ops do not record - this op is the
	// user action). The close's rebuild clears the undo stacks, the session open/close
	// rule; the attach stroke below starts the fresh stack.
	SBoardOpScope boardOp;
	if (!sessionCloseZone(srcBase, /* saveFirst= */ true, /* forceDiscard= */ false, err))
	{
		msg = "attach: closing '" + srcBase + "' failed: " + err;
		return 0;
	}
	pzS = NULL; // died with the close's rebuild

	// The real merge, on the post-close session state. Re-resolve by node, not by the stale
	// id: the file's own (InFile) editable zone is unique per file, and everything below -
	// snapshot, dirty flag, status - runs on the re-based id.
	pzT = NULL;
	{
		std::vector<SPaintZone> &zonesAll = *g_PaintCtx.Zones;
		for (size_t z = 0; z < zonesAll.size(); ++z)
			if ((const void *)zonesAll[z].Node == tgtNode && zonesAll[z].InFile
			    && zonesAll[z].Editable)
			{
				pzT = &zonesAll[z];
				break;
			}
	}
	if (!pzT)
	{
		msg = "attach: target zone lost across the source close";
		return 0;
	}
	targetZone = pzT->ZoneId;
	STopoTarget target;
	if (!resolveTopoTarget(pzT->Node, target, err))
	{
		msg = "attach: " + err;
		return 0;
	}
	SPatchMesh pm;
	SRPatchMesh rp;
	SPmVertMapper mapper;
	bool haveMapper = false;
	if (!decodeTopoTarget(target, pm, rp, mapper, haveMapper, err))
	{
		msg = "attach: " + err;
		return 0;
	}
	// The board placement re-derives from the file's authored footprint ORIGIN (snapped
	// geometry min) on every rebuild; the merge can extend that corner, which would slide
	// the whole zone to keep the new corner at the anchor cell. Capture the origin now and
	// move the anchor CELL by the same whole-cell delta after the merge, so the zone stays
	// put in the world and the footprint simply grows.
	float oxPre = 0.f, oyPre = 0.f;
	bool haveOrigin = false;
	SEditableFileInfo *tgtFile = zpZoneEditableFile(targetZone);
	if (tgtFile)
	{
		SNodeTMCache tmCache;
		haveOrigin = zoneNodeAuthoredFootprintOrigin(pzT->Node, tmCache, g_SessionCellSize,
		                                             oxPre, oyPre);
	}
	ZPPAINT::STopoSnapshot *snap = new ZPPAINT::STopoSnapshot();
	snap->Zone = targetZone;
	snap->PmOld = pm;
	snap->RpOld = rp;
	snap->HaveMapper = haveMapper && target.MapperRaw;
	if (snap->HaveMapper)
		snap->MapperOld = target.MapperRaw->Value;
	// The appended outputs are unmapped; input-indexed mapper slots keep - no rewrite.
	if (!topoAppendMesh(pm, rp, srcPm, srcRp, rel, err)
	    || !encodeTopoTarget(target, pm, rp, mapper, haveMapper, err))
	{
		msg = "attach: " + err;
		printf("%s\n", msg.c_str());
		fflush(stdout);
		delete snap;
		return 0;
	}
	snap->PmNew = pm;
	snap->RpNew = rp;
	if (snap->HaveMapper)
		snap->MapperNew = target.MapperRaw->Value;
	if (haveOrigin)
	{
		float oxPost = 0.f, oyPost = 0.f;
		SNodeTMCache tmCache;
		if (zoneNodeAuthoredFootprintOrigin(pzT->Node, tmCache, g_SessionCellSize,
		                                    oxPost, oyPost)
		    && g_SessionCellSize > 0.f)
		{
			snap->CellDX = (int)floor((oxPost - oxPre) / g_SessionCellSize + 0.5);
			snap->CellDY = (int)floor((oyPost - oyPre) / g_SessionCellSize + 0.5);
			tgtFile->CellX += snap->CellDX;
			tgtFile->CellY += snap->CellDY;
		}
	}
	printf("attach: zone %u += %u patches from '%s' (%s target)\n", targetZone, appended,
	       srcBase.c_str(), target.Local ? "modifier" : "base");
	fflush(stdout);
	// Recorded with the ids the user issued: a replay runs from the same pre-op state, where
	// those are the right names; the re-based id above is a post-op session detail.
	ZPSCRIPT::record(NLMISC::toString("painter.attachZone(%u, %u)", recTarget, recSrc));
	g_PatchVertSel.clear();
	g_PatchEdgeSel.clear();
	g_PatchFaceSel.clear();
	g_PatchTanSel.clear();
	zpPatchGizmoInvalidate();
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds, /* skipWriteBack= */ true, /* keepUndo= */ true))
	{
		msg = "attach: session rebuild failed: " + err;
		delete snap;
		return appended;
	}
	std::vector<ZPPAINT::STopoSnapshot *> snaps(1, snap);
	g_PaintCtx.Core->opTopoStroke(snaps);
	g_PaintCtx.Core->markGeomDirty(targetZone);
	msg = NLMISC::toString("attach: %u patch%s from '%s' (file saved and closed)",
	                       appended, appended == 1 ? "" : "es", srcBase.c_str());
	g_PropStatusMsg = msg;
	return appended;
}

// ---------------------------------------------------------------------------------------------
// Move selection to a neighbor zone: the cross-file patch transfer. No legacy equivalent -
// this op exists because the session shows several brick files at once, which the old
// plugin never did. The selected patches leave their file and join the neighbor's:
// sub-mesh extract (complement-delete on a copy), append into the destination reoriented
// through the display transforms (the patches keep their place in the world), delete from
// the source. All three passes are the proven transforms; BOTH files stay open, the zone
// set is unchanged, so the whole transfer is ONE Kind 6 stroke with TWO snapshots -
// fully undoable, paint travels, binds crossing the cut release.

/**
 * Move the selected patches (patch level, one source zone) into `dstZone`. Returns the
 * moved patch count.
 */
uint zpMovePatchSelectionToZone(uint dstZone, std::string &msg)
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Zones)
	{
		msg = "move: no session";
		return 0;
	}
	// The selection: one object, like detach.
	const void *object = NULL;
	uint srcZone = 0;
	std::set<uint> sel;
	for (std::set<TPatchFaceId>::const_iterator it = g_PatchFaceSel.begin();
	     it != g_PatchFaceSel.end(); ++it)
	{
		const SPaintZone *pz = zpFindPaintZone(it->first);
		if (!pz || !pz->Editable || it->second >= pz->Patches.size())
			continue;
		if (object && (const void *)pz->Node != object)
		{
			msg = "move: select patches in one zone only";
			return 0;
		}
		object = (const void *)pz->Node;
		srcZone = it->first;
		sel.insert(it->second);
	}
	if (!object || sel.empty())
	{
		msg = "move: no patches selected";
		printf("%s\n", msg.c_str());
		fflush(stdout);
		return 0;
	}
	SPaintZone *pzS = zpFindPaintZoneMut(srcZone);
	SPaintZone *pzT = zpFindPaintZoneMut(dstZone);
	if (!pzS || !pzT)
	{
		msg = "move: no such zone";
		return 0;
	}
	if (!pzT->Editable)
	{
		msg = "move: destination is read-only";
		return 0;
	}
	if (pzT->Node == pzS->Node)
	{
		msg = "move: source and destination are the same object";
		return 0;
	}
	if (sel.size() >= pzS->Patches.size())
	{
		msg = "move: selection is the whole zone (attach the file instead)";
		return 0;
	}
	double rel[12];
	if (!zpRelObjectTM(*pzT, *pzS, rel, msg))
	{
		msg = "move: " + msg;
		return 0;
	}

	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		msg = "move: paint write-back failed: " + err;
		return 0;
	}

	// Source streams (stored topology + paint, eval positions - the attach capture rule).
	STopoTarget srcStream;
	SPatchMesh srcPm;
	SRPatchMesh srcRp;
	SPmVertMapper srcMapper;
	bool srcHaveMapper = false;
	if (!resolveTopoTarget(pzS->Node, srcStream, err)
	    || !decodeTopoTarget(srcStream, srcPm, srcRp, srcMapper, srcHaveMapper, err))
	{
		msg = "move: source: " + err;
		return 0;
	}
	if (srcPm.Patches.size() != pzS->Patches.size()
	    || srcPm.Verts.size() != pzS->Ep.Pm.Verts.size()
	    || srcPm.Vecs.size() != pzS->Ep.Pm.Vecs.size())
	{
		msg = "move: source stream does not match the displayed topology";
		return 0;
	}
	// Destination streams.
	STopoTarget dstStream;
	SPatchMesh dstPm;
	SRPatchMesh dstRp;
	SPmVertMapper dstMapper;
	bool dstHaveMapper = false;
	if (!resolveTopoTarget(pzT->Node, dstStream, err)
	    || !decodeTopoTarget(dstStream, dstPm, dstRp, dstMapper, dstHaveMapper, err))
	{
		msg = "move: destination: " + err;
		return 0;
	}
	if (dstPm.Patches.size() != pzT->Patches.size())
	{
		msg = "move: destination stream does not match the displayed topology";
		return 0;
	}

	// Sub-mesh extract: eval positions onto a copy, then the complement deletes.
	SPatchMesh subPm = srcPm;
	SRPatchMesh subRp = srcRp;
	{
		for (size_t i = 0; i < subPm.Verts.size(); ++i)
			memcpy(subPm.Verts[i].Pos, pzS->Ep.Pm.Verts[i].Pos, 12);
		for (size_t i = 0; i < subPm.Vecs.size(); ++i)
			memcpy(subPm.Vecs[i].Pos, pzS->Ep.Pm.Vecs[i].Pos, 12);
		std::set<uint> complement;
		for (uint p = 0; p < (uint)srcPm.Patches.size(); ++p)
			if (!sel.count(p))
				complement.insert(p);
		STopoRemap remap;
		if (!topoDeletePatches(subPm, subRp, NULL, complement, remap, err))
		{
			msg = "move: extract: " + err;
			return 0;
		}
	}

	// The two real passes, validated on copies first (all or nothing).
	SPatchMesh srcPost = srcPm;
	SRPatchMesh srcRpPost = srcRp;
	SPmVertMapper srcMapperPost = srcMapper;
	{
		STopoRemap remap;
		if (!topoDeletePatches(srcPost, srcRpPost, srcHaveMapper ? &srcMapperPost : NULL,
		                       sel, remap, err))
		{
			msg = "move: " + err;
			printf("%s\n", msg.c_str());
			fflush(stdout);
			return 0;
		}
	}
	SPatchMesh dstPost = dstPm;
	SRPatchMesh dstRpPost = dstRp;
	if (!topoAppendMesh(dstPost, dstRpPost, subPm, subRp, rel, err))
	{
		msg = "move: " + err;
		printf("%s\n", msg.c_str());
		fflush(stdout);
		return 0;
	}

	// Snapshots (PRE captured post-writeBack), then encode both files.
	ZPPAINT::STopoSnapshot *snapSrc = new ZPPAINT::STopoSnapshot();
	snapSrc->Zone = srcZone;
	snapSrc->PmOld = srcPm;
	snapSrc->RpOld = srcRp;
	snapSrc->HaveMapper = srcHaveMapper && srcStream.MapperRaw;
	if (snapSrc->HaveMapper)
		snapSrc->MapperOld = srcStream.MapperRaw->Value;
	ZPPAINT::STopoSnapshot *snapDst = new ZPPAINT::STopoSnapshot();
	snapDst->Zone = dstZone;
	snapDst->PmOld = dstPm;
	snapDst->RpOld = dstRp;
	snapDst->HaveMapper = dstHaveMapper && dstStream.MapperRaw;
	if (snapDst->HaveMapper)
		snapDst->MapperOld = dstStream.MapperRaw->Value;

	// The anchor-cell rule, both sides: the transfer can move EITHER file's authored
	// footprint origin (the destination grows, the source shrinks).
	float srcOx = 0.f, srcOy = 0.f, dstOx = 0.f, dstOy = 0.f;
	bool srcHaveOrigin = false, dstHaveOrigin = false;
	SEditableFileInfo *srcFile = zpZoneEditableFile(srcZone);
	SEditableFileInfo *dstFile = zpZoneEditableFile(dstZone);
	{
		SNodeTMCache tmCache;
		if (srcFile)
			srcHaveOrigin = zoneNodeAuthoredFootprintOrigin(pzS->Node, tmCache,
			                                                g_SessionCellSize, srcOx, srcOy);
		if (dstFile)
			dstHaveOrigin = zoneNodeAuthoredFootprintOrigin(pzT->Node, tmCache,
			                                                g_SessionCellSize, dstOx, dstOy);
	}

	if (!encodeTopoTarget(srcStream, srcPost, srcRpPost, srcMapperPost, srcHaveMapper, err))
	{
		msg = "move: source encode: " + err;
		delete snapSrc;
		delete snapDst;
		return 0;
	}
	if (!encodeTopoTarget(dstStream, dstPost, dstRpPost, dstMapper, dstHaveMapper, err))
	{
		// Roll the source back so the transfer stays all-or-nothing.
		std::string rerr;
		if (!encodeTopoTarget(srcStream, srcPm, srcRp, srcMapper, srcHaveMapper, rerr))
			fprintf(stderr, "ERROR: move: source rollback failed: %s\n", rerr.c_str());
		else if (snapSrc->HaveMapper)
			srcStream.MapperRaw->Value = snapSrc->MapperOld;
		msg = "move: destination encode: " + err;
		delete snapSrc;
		delete snapDst;
		return 0;
	}
	snapSrc->PmNew = srcPost;
	snapSrc->RpNew = srcRpPost;
	if (snapSrc->HaveMapper)
		snapSrc->MapperNew = srcStream.MapperRaw->Value;
	snapDst->PmNew = dstPost;
	snapDst->RpNew = dstRpPost;
	if (snapDst->HaveMapper)
		snapDst->MapperNew = dstStream.MapperRaw->Value;
	{
		SNodeTMCache tmCache;
		float ox2 = 0.f, oy2 = 0.f;
		if (srcHaveOrigin && g_SessionCellSize > 0.f
		    && zoneNodeAuthoredFootprintOrigin(pzS->Node, tmCache, g_SessionCellSize, ox2, oy2))
		{
			snapSrc->CellDX = (int)floor((ox2 - srcOx) / g_SessionCellSize + 0.5);
			snapSrc->CellDY = (int)floor((oy2 - srcOy) / g_SessionCellSize + 0.5);
			srcFile->CellX += snapSrc->CellDX;
			srcFile->CellY += snapSrc->CellDY;
		}
		if (dstHaveOrigin && g_SessionCellSize > 0.f
		    && zoneNodeAuthoredFootprintOrigin(pzT->Node, tmCache, g_SessionCellSize, ox2, oy2))
		{
			snapDst->CellDX = (int)floor((ox2 - dstOx) / g_SessionCellSize + 0.5);
			snapDst->CellDY = (int)floor((oy2 - dstOy) / g_SessionCellSize + 0.5);
			dstFile->CellX += snapDst->CellDX;
			dstFile->CellY += snapDst->CellDY;
		}
	}

	const uint moved = (uint)sel.size();
	printf("move: %u patches, zone %u -> zone %u (%s -> %s target)\n", moved, srcZone,
	       dstZone, srcStream.Local ? "modifier" : "base", dstStream.Local ? "modifier" : "base");
	fflush(stdout);
	ZPSCRIPT::record(NLMISC::toString("painter.movePatchSelectionToZone(%u)", dstZone));
	g_PatchVertSel.clear();
	g_PatchEdgeSel.clear();
	g_PatchFaceSel.clear();
	g_PatchTanSel.clear();
	zpPatchGizmoInvalidate();
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds, /* skipWriteBack= */ true, /* keepUndo= */ true))
	{
		msg = "move: session rebuild failed: " + err;
		delete snapSrc;
		delete snapDst;
		return moved;
	}
	std::vector<ZPPAINT::STopoSnapshot *> snaps;
	snaps.push_back(snapSrc);
	snaps.push_back(snapDst);
	g_PaintCtx.Core->opTopoStroke(snaps);
	g_PaintCtx.Core->markGeomDirty(srcZone);
	g_PaintCtx.Core->markGeomDirty(dstZone);
	msg = NLMISC::toString("move: %u patch%s -> '%s'", moved, moved == 1 ? "" : "es",
	                       zpZoneFileBasename(dstZone).c_str());
	g_PropStatusMsg = msg;
	return moved;
}

/**
 * The editable file adjacent to the SELECTION's file in compass direction `dir`
 * (0=N 1=NE 2=E 3=SE 4=S 5=SW 6=W 7=NW; board cells, +Y = north), and its zone.
 * The candidates are the cells lining the source's footprint block on that side
 * (the corner cell for diagonals).
 */
bool zpMoveDirTarget(int dir, uint &dstZoneOut)
{
	static const int kDx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static const int kDy[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	if (dir < 0 || dir > 7 || g_PatchFaceSel.empty())
		return false;
	const SEditableFileInfo *src = zpZoneEditableFile(g_PatchFaceSel.begin()->first);
	if (!src)
		return false;
	std::vector<std::pair<int, int> > cand;
	const int x0 = src->CellX, y0 = src->CellY;
	const int w = src->CellsW > 0 ? src->CellsW : 1, h = src->CellsH > 0 ? src->CellsH : 1;
	const int dx = kDx[dir], dy = kDy[dir];
	if (dx != 0 && dy != 0)
		cand.push_back(std::make_pair(dx > 0 ? x0 + w : x0 - 1, dy > 0 ? y0 + h : y0 - 1));
	else if (dx != 0)
		for (int y = y0; y < y0 + h; ++y)
			cand.push_back(std::make_pair(dx > 0 ? x0 + w : x0 - 1, y));
	else
		for (int x = x0; x < x0 + w; ++x)
			cand.push_back(std::make_pair(x, dy > 0 ? y0 + h : y0 - 1));
	for (size_t f = 0; f < g_EditableFiles.size(); ++f)
	{
		const SEditableFileInfo &efi = g_EditableFiles[f];
		if (&efi == src || !efi.Editable)
			continue;
		const int fw = efi.CellsW > 0 ? efi.CellsW : 1, fh = efi.CellsH > 0 ? efi.CellsH : 1;
		for (size_t c = 0; c < cand.size(); ++c)
		{
			const int cx = cand[c].first, cy = cand[c].second;
			if (cx < efi.CellX || cx >= efi.CellX + fw || cy < efi.CellY || cy >= efi.CellY + fh)
				continue;
			if ((int)efi.Mask.size() == fw * fh
			    && !efi.Mask[(cy - efi.CellY) * fw + (cx - efi.CellX)])
				continue;
			// The file's editable zone.
			for (size_t z = 0; z < efi.ZoneIds.size(); ++z)
			{
				const SPaintZone *pz = zpFindPaintZone(efi.ZoneIds[z]);
				if (pz && pz->Editable)
				{
					dstZoneOut = efi.ZoneIds[z];
					return true;
				}
			}
		}
	}
	return false;
}

/** Bridge: the scene-menu compass click (dir 0..7). */
void zpMoveToZoneDirClicked(int dir)
{
	uint dst = 0;
	if (!zpMoveDirTarget(dir, dst))
	{
		g_PropStatusMsg = "move: no editable neighbor zone that way";
		return;
	}
	std::string msg;
	if (!zpMovePatchSelectionToZone(dst, msg))
		g_PropStatusMsg = msg; // success sets the status line itself
}

/**
 * Undo/redo sink for Kind 6 records: re-encode the snapshot's matching side into the
 * target stream. The working-set rebuild happens at the zpUndo/zpRedo level, once per
 * replay, after the core's stroke replay returns - see topoRestorePending.
 */
void zpTopoRestore(const ZPPAINT::STopoSnapshot &snap, bool useOld)
{
	SPaintZone *pz = zpFindPaintZoneMut(snap.Zone);
	if (!pz)
		return;
	STopoTarget target;
	std::string err;
	if (!resolveTopoTarget(pz->Node, target, err))
	{
		fprintf(stderr, "ERROR: topo restore: %s\n", err.c_str());
		return;
	}
	// Restoring re-encodes the decoded structs; identity of the codec makes an undo save
	// reproduce the pre-op bytes exactly. The mapper payload travels verbatim.
	SPmVertMapper unusedMapper;
	if (!encodeTopoTarget(target, useOld ? snap.PmOld : snap.PmNew,
	                      useOld ? snap.RpOld : snap.RpNew, unusedMapper, false, err))
	{
		fprintf(stderr, "ERROR: topo restore encode: %s\n", err.c_str());
		return;
	}
	if (snap.HaveMapper && target.MapperRaw)
		target.MapperRaw->Value = useOld ? snap.MapperOld : snap.MapperNew;
	// The op moved the owning file's anchor cell with the footprint origin (attach):
	// restore the matching side so the rebuild's placement math keeps the zone put.
	if (snap.CellDX || snap.CellDY)
	{
		if (SEditableFileInfo *f = zpZoneEditableFile(snap.Zone))
		{
			f->CellX += useOld ? -snap.CellDX : snap.CellDX;
			f->CellY += useOld ? -snap.CellDY : snap.CellDY;
		}
	}
}

/** Panel Target toggle: arm/disarm the target-weld command mode. */
void zpWeldTargetToggleClicked()
{
	g_WeldTargetArmed = !g_WeldTargetArmed;
	if (!g_WeldTargetArmed)
		zpWeldDragCancel();
	g_PropStatusMsg = g_WeldTargetArmed
		? "target weld armed: drag a vertex onto its target (right click leaves)"
		: "target weld off";
}

/** Bridge wrappers (void-returning function pointers). */
void zpPatchDeleteClicked() { zpDeletePatchSelection(); }
void zpPatchTurnCcwClicked() { zpTurnPatchSelection(true); }
void zpPatchTurnCwClicked() { zpTurnPatchSelection(false); }
void zpPatchSubdivideClicked() { zpSubdividePatchSelection(); }
void zpPatchWeldClicked() { zpWeldPatchSelection(0.1f); } // legacy default threshold
void zpPatchAddQuadClicked() { zpAddQuadPatchSelection(); }

/** Script/gate read access: the eval-mirror vertex count of a zone. */
bool zpZoneVertCount(uint zoneId, uint &countOut)
{
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz)
		return false;
	countOut = (uint)pz->Ep.Pm.Verts.size();
	return true;
}

/** Script/gate read access: the displayed patch count of a zone. */
bool zpZonePatchCount(uint zoneId, uint &countOut)
{
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz)
		return false;
	countOut = (uint)pz->Patches.size();
	return true;
}

/** Script/gate read access: layer-0 tile of one grid tile (from the pristine carrier,
 * display transform applied - the same read the paint modes use). */
bool zpTileQuery(uint zoneId, uint patchIdx, uint u, uint v, int &tileOut, int &rotOut,
                 int &numOut)
{
	if (!g_PaintCtx.Core)
		return false;
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz || patchIdx >= pz->Patches.size())
		return false;
	// Bound by the patch's own tile order, not the fixed 16x16 meta grid - the pristine
	// tile table is OrderS x OrderT and the raw read does not range-check.
	if (u >= pz->Patches[patchIdx].OrderS || v >= pz->Patches[patchIdx].OrderT)
		return false;
	ZPPAINT::CTileDescP desc;
	g_PaintCtx.Core->getTile(zoneId,
		(sint32)(patchIdx * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u), desc);
	numOut = desc.getNumLayer();
	tileOut = numOut ? (int)desc.getLayer(0).Tile : -1;
	rotOut = numOut ? (int)desc.getLayer(0).Rotate : 0;
	return true;
}

/* end of file */
