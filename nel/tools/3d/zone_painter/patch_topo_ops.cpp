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
 * The working-set rebuild clears the undo stack (session semantics); a topological op is
 * currently NOT undoable and says so in its status line. Raw-snapshot undo is the recorded
 * next step in the plan.
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

/**
 * Delete the selected patches (patch sub-object level).
 *
 * Per affected object: flush pending paint into the carrier, transform the target stream
 * (topoDeletePatches - the paint and bind records travel with their surviving elements),
 * re-encode in place, then rebuild the working set once so display, core and landscape
 * re-derive from storage. Selections clear (their indices died with the old topology) and
 * the undo history clears with the rebuild - stated in the status line.
 */
uint zpDeletePatchSelection()
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Zones)
		return 0;
	if (g_PatchFaceSel.empty())
	{
		g_PropStatusMsg = "delete: no patches selected";
		return 0;
	}
	// Group the selection per OBJECT: indices are object-level, and two zones of one object
	// address the same storage.
	std::map<const void *, std::set<uint> > perObject; // object -> patch indices
	std::map<const void *, uint> objectZone; // object -> one zone id to report on
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
	if (perObject.empty())
	{
		g_PropStatusMsg = "delete: selection has no editable patches";
		return 0;
	}

	// Pending paint lives in the core's pristine copies; the transform reads and rewrites
	// the CARRIER BYTES, so everything unsaved flushes first. The rebuild afterwards skips
	// its own write-back for the same reason - it would re-encode the stale pre-delete
	// pristine over the transformed blob.
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_PropStatusMsg = "delete: paint write-back failed: " + err;
		return 0;
	}

	uint deleted = 0;
	std::vector<uint> touchedZones;
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
			g_PropStatusMsg = "delete: " + err;
			continue;
		}
		SPatchMesh pm;
		SRPatchMesh rp;
		SPmVertMapper mapper;
		bool haveMapper = false;
		if (!decodeTopoTarget(target, pm, rp, mapper, haveMapper, err))
		{
			g_PropStatusMsg = "delete: " + err;
			continue;
		}
		if (pm.Patches.size() != pz->Patches.size())
		{
			g_PropStatusMsg = "delete: stored stream does not match the displayed topology";
			continue;
		}
		STopoRemap remap;
		if (!topoDeletePatches(pm, rp, haveMapper ? &mapper : NULL, ot->second, remap, err))
		{
			g_PropStatusMsg = "delete: " + err;
			continue;
		}
		if (!encodeTopoTarget(target, pm, rp, mapper, haveMapper, err))
		{
			// The struct transform succeeded but the write-back did not; the stream may be
			// half-written. The rebuild below re-derives from storage either way, and the
			// file on disk is untouched until an explicit save.
			g_PropStatusMsg = "delete: " + err;
			continue;
		}
		deleted += (uint)ot->second.size();
		touchedZones.push_back(zoneId);
		// Unconditional: the target-stream choice is what the gates pin (m34 pins the
		// geometry write target the same way).
		printf("delete: zone %u, %u patches removed (%s target)\n", zoneId,
		       (uint)ot->second.size(), target.Local ? "modifier" : "base");
		fflush(stdout);
	}
	if (!deleted)
		return 0;

	ZPSCRIPT::record("painter.deletePatchSelection()");

	// Old indices died with the old topology; clear every sub-object selection directly
	// (the recorded op line replays the whole delete, no per-set records needed).
	g_PatchVertSel.clear();
	g_PatchEdgeSel.clear();
	g_PatchFaceSel.clear();
	g_PatchTanSel.clear();
	zpPatchGizmoInvalidate();

	uint welds = 0;
	if (!rebuildWorkingSet(err, welds, /* skipWriteBack= */ true))
	{
		g_PropStatusMsg = "delete: session rebuild failed: " + err;
		return deleted;
	}
	// The PatchMesh stream mutation is invisible to the carrier blob compare (same reason
	// vertex moves are); the blob itself also changed, but mark the geometry flag so the
	// dirty signal never depends on that coincidence.
	for (size_t i = 0; i < touchedZones.size(); ++i)
		g_PaintCtx.Core->markGeomDirty(touchedZones[i]);
	g_PropStatusMsg = NLMISC::toString(
		"delete: %u patch%s removed (undo history cleared)", deleted, deleted == 1 ? "" : "es");
	return deleted;
}

/** Bridge wrapper (void-returning function pointer). */
void zpPatchDeleteClicked() { zpDeletePatchSelection(); }

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
