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

} /* anonymous namespace */

// ---------------------------------------------------------------------------------------------

/** One topology transform over the decoded target streams (delete, turn, ...). */
typedef bool (*TTopoXform)(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                           const std::set<uint> &sel, std::string &err);

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
			continue;
		}
		SPatchMesh pm;
		SRPatchMesh rp;
		SPmVertMapper mapper;
		bool haveMapper = false;
		if (!decodeTopoTarget(target, pm, rp, mapper, haveMapper, err))
		{
			g_PropStatusMsg = std::string(opName) + ": " + err;
			continue;
		}
		if (pm.Patches.size() != pz->Patches.size())
		{
			g_PropStatusMsg = std::string(opName) + ": stored stream does not match the displayed topology";
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
		if (!xf(pm, rp, haveMapper ? &mapper : NULL, ot->second, err))
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
			// The struct transform succeeded but the write-back did not; the stream may be
			// half-written. The rebuild below re-derives from storage either way, and the
			// file on disk is untouched until an explicit save.
			g_PropStatusMsg = std::string(opName) + ": " + err;
			delete snap;
			continue;
		}
		snap->PmNew = pm;
		snap->RpNew = rp;
		if (snap->HaveMapper)
			snap->MapperNew = target.MapperRaw->Value;
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
                          const std::set<uint> &sel, std::string &err)
{
	STopoRemap remap;
	return topoDeletePatches(pm, rp, mapper, sel, remap, err);
}

static bool zpXformTurnCcw(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper * /* mapper */,
                           const std::set<uint> &sel, std::string &err)
{
	return topoTurnPatches(pm, rp, sel, true, err);
}

static bool zpXformTurnCw(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper * /* mapper */,
                          const std::set<uint> &sel, std::string &err)
{
	return topoTurnPatches(pm, rp, sel, false, err);
}

static bool zpXformSubdivide(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper * /* mapper */,
                             const std::set<uint> &sel, std::string &err)
{
	// The mapper needs NO rewrite for a pure addition: input-indexed record slots keep,
	// surviving outputs keep their indices, added outputs stay unmapped.
	return topoSubdividePatches(pm, rp, sel, err);
}

// Weld threshold handed to the adapter through a file-static: the shared runner's
// transform signature is selection-only, and the threshold is op state like the brush
// size, not selection data.
static float s_WeldThreshold = 0.1f;
static bool zpXformWeld(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper *mapper,
                        const std::set<uint> &sel, std::string &err)
{
	STopoRemap remap;
	return topoWeldVerts(pm, rp, mapper, sel, s_WeldThreshold, remap, err);
}

static bool zpXformAddQuad(SPatchMesh &pm, SRPatchMesh &rp, SPmVertMapper * /* mapper */,
                           const std::set<uint> &sel, std::string &err)
{
	// Pure addition: the mapper needs no rewrite (input slots keep, new outputs unmapped).
	return topoAddQuads(pm, rp, sel, err);
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
 * vertex set through the same per-object grouping.
 */
uint zpWeldPatchSelection(float threshold)
{
	s_WeldThreshold = threshold > 0.f ? threshold : 0.1f;
	return zpRunTopoOpVerts("weld", NLMISC::toString("painter.weldPatchSelection(%.9g)",
	                                                 s_WeldThreshold), zpXformWeld);
}

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
