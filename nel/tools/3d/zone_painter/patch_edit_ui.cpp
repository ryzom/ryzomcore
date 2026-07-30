/**
 * \file patch_edit_ui.cpp
 * \brief Patch-edit display and picking: control cage, gizmo, preview offsets, click dispatch.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The half of patch-edit mode that projects and draws: the control cage (`zpDrawPatchLattice`),
 * the move / rotate / scale gizmo with its fit-at-rest sizing and hover resolution, the
 * per-element preview offsets (`zpVertOffset` / `zpTanOffset`), the screen-space pick
 * functions for vertices, edges, faces and tangent handles, and the click dispatch
 * (`zpPatchVertexClick` / `zpPatchClickAt`). Everything here projects by hand and emits 2D
 * lines - overlays run after editorUI->draw(), which leaves the driver in NLGUI's 2D setup,
 * where world-space CDRU lines silently draw nothing. State changes live in
 * patch_edit_ops.cpp; the two share the drag state through patch_edit_internal.h.
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

#include <nel/misc/types_nl.h>

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif
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
#include <nel/3d/material.h>
#include <nel/misc/line.h>
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
#include <cctype>
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

#include <nel/gui/ctrl_base.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/ctrl_text_button.h>
#include <nel/gui/widget_manager.h>

#include "workspace_discovery.h"
#include "startup_ui.h"
#include "script_api.h"

#include <nel/3d/nav_mouse_listener.h>

#include "zp_state.h"
#include "viewer_listener.h"
#include "patch_edit_internal.h"

// Lift used by every patch overlay: the control points sit ON the landscape, and un-lifted
// geometry disappears into the tessellation it describes. Same value the zone outline uses.
static const float kPatchLift = 0.4f;
const NLMISC::CVector kNoOffset(0.f, 0.f, 0.f);

// Vertex marker half-size and pick radius, in viewport units of HEIGHT. The pick target is
// deliberately larger than the mark - the mark says where the vertex is, the radius says how
// close you have to be, and making the artist hit 6 pixels exactly is not a feature.
static const float kPatchVertHalf = 0.003f;
static const float kPatchVertPick = 0.012f;
// Edges are long targets, so a tighter radius than a vertex still leaves them easy to hit and
// stops an edge stealing a click meant for the corner sitting on it.
static const float kPatchEdgePick = 0.008f;
// Handles are picked before corners and sit right next to them, so the radius is tighter still
// - a generous one would swallow clicks meant for the corner.
static const float kPatchTanPick = 0.007f;

/** Project a world point (lifted) to screen. False when it is behind or across the near plane. */
static bool zpProjectLifted(const NLMISC::CMatrix &vm, const NL3D::CFrustum &f,
                            const NLMISC::CVector &world, float lift, NLMISC::CVector &out)
{
	NLMISC::CVector w = world;
	w.z += lift;
	const NLMISC::CVector eye = vm * w;
	if (eye.y <= f.Near * 0.5f)
		return false;
	out = f.project(eye);
	return true;
}

/**
 * Preview offset for a corner. A BOUND vertex never takes one: its position is derived from
 * the target edge, so when the edge moves it follows on the next evaluation, and when the
 * edge does not move it does not move either. Offsetting it here would draw a lie.
 *
 * A vertex selected through ANOTHER node of the same object still previews here, because the
 * commit will move it: the drag is measured in the dragged node's displayed space, so it
 * comes back to object space through that node and out again through this one. Dragging one
 * node of an object and watching its siblings follow is the whole point of the model.
 */
NLMISC::CVector zpVertOffset(const SPaintZone &pz, uint16 vi)
{
	if (vi < pz.Ep.Rp.Verts.size() && pz.Ep.Rp.Verts[vi].Binded)
		return kNoOffset;
	if (zpHandleMode())
		return kNoOffset; // handles are the target; their corners hold still
	// Selected through this very node: the drag is already in this node's displayed space, so
	// it applies directly.
	if (s_Dragging && g_PatchVertSel.count(TPatchVertId(pz.ZoneId, vi)))
	{
		float wp[3];
		if (zpPatchVertWorld(pz.ZoneId, vi, wp))
			return zpDragOffsetAt(NLMISC::CVector(wp[0], wp[1], wp[2]));
		return s_DragDelta;
	}
	if (!s_Dragging || !g_PaintCtx.Zones)
		return kNoOffset;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &other = zones[z];
		if (other.Node != pz.Node || other.ZoneId == pz.ZoneId)
			continue;
		if (!g_PatchVertSel.count(TPatchVertId(other.ZoneId, vi)))
			continue;
		float wp[3];
		if (!zpPatchVertWorld(other.ZoneId, vi, wp))
			continue;
		const NLMISC::CVector d = zpDragOffsetAt(NLMISC::CVector(wp[0], wp[1], wp[2]));
		const NLMISC::CVector od = zpXformDelta(d, MAXMATH::inverseM3(other.DisplayTM));
		return zpXformDelta(od, pz.DisplayTM);
	}
	return kNoOffset;
}

/** Is this corner selected through ANY node of the object `pz` shows? The ride rule keys on
 *  the OBJECT vertex: a corner dragged through a sibling node still carries its handles in
 *  every node's cage, exactly as the commit will move them. */
static bool zpCornerSelectedAnyNode(const SPaintZone &pz, uint16 vert)
{
	if (g_PatchVertSel.count(TPatchVertId(pz.ZoneId, vert)))
		return true;
	if (!g_PaintCtx.Zones)
		return false;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
		if (zones[z].Node == pz.Node && zones[z].ZoneId != pz.ZoneId
		    && g_PatchVertSel.count(TPatchVertId(zones[z].ZoneId, vert)))
			return true;
	return false;
}

/** The drag offset of an effectively-moving handle - selected through this node, or
 *  through a sibling node of the same object (the drag then lives in that node's
 *  displayed space and converts through object space). False when it is not moving. */
static bool zpTanMovingOffset(const SPaintZone &pz, uint16 vecIdx, NLMISC::CVector &off)
{
	if (zpTanSelectedEffective(pz, vecIdx))
	{
		float wp[3];
		if (zpPatchTangentWorld(pz.ZoneId, vecIdx, wp))
			off = zpDragOffsetAt(NLMISC::CVector(wp[0], wp[1], wp[2]));
		else
			off = s_DragDelta;
		return true;
	}
	if (!g_PaintCtx.Zones)
		return false;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &other = zones[z];
		if (other.Node != pz.Node || other.ZoneId == pz.ZoneId)
			continue;
		if (!zpTanSelectedEffective(other, vecIdx))
			continue;
		float wp[3];
		if (!zpPatchTangentWorld(other.ZoneId, vecIdx, wp))
			continue;
		const NLMISC::CVector d = zpDragOffsetAt(NLMISC::CVector(wp[0], wp[1], wp[2]));
		const NLMISC::CVector od = zpXformDelta(d, MAXMATH::inverseM3(other.DisplayTM));
		off = zpXformDelta(od, pz.DisplayTM);
		return true;
	}
	return false;
}

/**
 * Preview offset for a HANDLE.
 *
 * A handle whose corner is moving rides that corner - the same rule the write applies, so the
 * preview and the commit cannot disagree about which of the two moved it. Only a handle whose
 * corner is standing still takes its own drag.
 */
NLMISC::CVector zpTanOffset(const SPaintZone &pz, uint16 vecIdx)
{
	const uint16 owner = zpTangentOwner(pz, vecIdx);
	// A handle of a BOUND corner is derived along with it: the commit refuses to write it
	// (the same rule the move's skippedBound filter applies), so previewing a motion here
	// would draw a surface the release snaps back from.
	if (owner != (uint16)0xffff && owner < pz.Ep.Rp.Verts.size()
	    && pz.Ep.Rp.Verts[owner].Binded)
		return kNoOffset;
	if (!zpHandleMode() && owner != (uint16)0xffff && zpCornerSelectedAnyNode(pz, owner))
	{
		// The ride, at the HANDLE'S OWN position: the commit writes each ridden handle
		// through the transform's image of the handle itself (a rotate turns handles about
		// the pivot, not just their corner), so the preview must too. For a move the two
		// are the same offset. Selected through this node, or through a sibling - in the
		// sibling case the drag lives in that node's displayed space and converts through
		// object space, exactly as zpVertOffset does for the corner.
		if (!s_Dragging)
			return kNoOffset;
		if (g_PatchVertSel.count(TPatchVertId(pz.ZoneId, owner)))
		{
			float wp[3];
			if (zpPatchTangentWorld(pz.ZoneId, vecIdx, wp))
				return zpDragOffsetAt(NLMISC::CVector(wp[0], wp[1], wp[2]));
			return s_DragDelta;
		}
		if (!g_PaintCtx.Zones)
			return kNoOffset;
		const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
		for (uint z = 0; z < zones.size(); ++z)
		{
			const SPaintZone &other = zones[z];
			if (other.Node != pz.Node || other.ZoneId == pz.ZoneId)
				continue;
			if (!g_PatchVertSel.count(TPatchVertId(other.ZoneId, owner)))
				continue;
			float wp[3];
			if (!zpPatchTangentWorld(other.ZoneId, vecIdx, wp))
				continue;
			const NLMISC::CVector d = zpDragOffsetAt(NLMISC::CVector(wp[0], wp[1], wp[2]));
			const NLMISC::CVector od = zpXformDelta(d, MAXMATH::inverseM3(other.DisplayTM));
			return zpXformDelta(od, pz.DisplayTM);
		}
		return kNoOffset;
	}
	if (!s_Dragging || !g_PaintCtx.Zones)
		return kNoOffset;
	// EFFECTIVE membership: Lock Handles expands the selection over the owner group, and
	// the same predicate drives the commit and the live push - selected through this node
	// or through a sibling node of the same object.
	{
		NLMISC::CVector off;
		if (zpTanMovingOffset(pz, vecIdx, off))
			return off;
	}
	// Coplanar continuity: an UNMOVING handle of a coplanar vertex follows the plane its
	// moving siblings tilt - the commit writes exactly this through the same helper
	// (zpCoplanarSiblingReaim), the parity discipline.
	if (zpHandleMode() && owner != (uint16)0xffff && zpVertCoplanarConstrained(pz, owner))
	{
		std::vector<uint16> att;
		zpVertexTangents(pz, owner, att);
		std::vector<uint16> movedVecs;
		std::vector<NLMISC::CVector> movedOffs;
		for (size_t k = 0; k < att.size(); ++k)
		{
			if (att[k] == vecIdx)
				continue;
			NLMISC::CVector o;
			if (zpTanMovingOffset(pz, att[k], o))
			{
				movedVecs.push_back(att[k]);
				movedOffs.push_back(o);
			}
		}
		if (!movedVecs.empty())
		{
			NLMISC::CVector off;
			if (zpCoplanarSiblingReaim(pz, vecIdx, owner, movedVecs, movedOffs, off))
				return off;
		}
	}
	return kNoOffset;
}

/**
 * Patch control cage - the control lattice.
 *
 * Drawn from the display CPatchInfo rather than from the evaluated SPatchMesh, because that
 * data is already in world space and already carries an identity for the shared corners:
 * BaseVertices[] is NeL's own per-zone vertex index, so a corner touched by four patches is
 * one vertex here too. Nothing about the .max write target is needed to DRAW the cage, and
 * keeping the two apart is what lets the display land before the write path exists.
 *
 * SCREEN SPACE, like zpDrawZoneOutline and for the same reason: overlays run after
 * editorUI->draw(), which leaves the driver in NLGUI's 2D setup. World-space CDRU lines
 * silently draw nothing there. Project through the camera by hand and emit 2D lines.
 *
 * Each edge of a quad patch is the Bezier chain V -> T -> T -> V, so the cage is those four
 * chains; drawing them per patch double-draws shared edges, which costs a line and saves an
 * adjacency walk.
 */
// Cage line colors, shared by the 3D wireframe pass.
static const NLMISC::CRGBA kCageColor(90, 190, 255, 255);
static const NLMISC::CRGBA kCageSelColor(255, 40, 40, 255);

/**
 * Collect one zone's cage chains as WORLD segments (lifted like every overlay), split
 * into plain and selected batches - the split replaces the old two-pass 2D draw: a
 * shared edge is emitted by both its patches, and drawing the selected batch AFTER the
 * plain one is what keeps a neighbour from painting over the highlight.
 */
static void zpCollectCageLines(const SPaintZone &pz, int subObj,
                               std::vector<NLMISC::CLine> &plain,
                               std::vector<NLMISC::CLine> &selected)
{
	const NLMISC::CVector liftV(0.f, 0.f, kPatchLift);
	for (uint p = 0; p < pz.Patches.size(); ++p)
	{
		const NL3D::CBezierPatch &bp = pz.Patches[p].Patch;
		const NL3D::CPatchInfo &cpi = pz.Patches[p];
		// At patch level the whole cell lights up, so its four chains count as selected
		// whichever way each of them is reached.
		const bool faceSel = subObj == CPaintMouseListener::SubPatch
			&& g_PatchFaceSel.count(TPatchFaceId(pz.ZoneId, p)) != 0;
		for (uint e = 0; e < 4; ++e)
		{
			const bool edgeSel = subObj == CPaintMouseListener::SubEdge
				&& g_PatchEdgeSel.count(SPatchEdgeId(pz.ZoneId, cpi.BaseVertices[e],
				                                     cpi.BaseVertices[(e + 1) & 3])) != 0;
			// Tangent 2e is attached to vertex e, tangent 2e+1 to vertex (e+1)&3. The
			// handles take zpTanOffset, which returns the corner's offset when they are
			// riding it and their own when they are being dragged - so the cage bends
			// under a handle drag and deforms rather than tearing under a corner drag.
			const NLMISC::CVector offA = zpVertOffset(pz, cpi.BaseVertices[e]);
			const NLMISC::CVector offB = zpVertOffset(pz, cpi.BaseVertices[(e + 1) & 3]);
			NLMISC::CVector offTA = offA, offTB = offB;
			if (p < pz.Ep.Pm.Patches.size())
			{
				const PIPELINE::MAX::NELPATCH::SPmPatch &pp = pz.Ep.Pm.Patches[p];
				if (pp.Vec[e * 2] >= 0) offTA = zpTanOffset(pz, (uint16)pp.Vec[e * 2]);
				if (pp.Vec[e * 2 + 1] >= 0) offTB = zpTanOffset(pz, (uint16)pp.Vec[e * 2 + 1]);
			}
			std::vector<NLMISC::CLine> &dst = (faceSel || edgeSel) ? selected : plain;
			const NLMISC::CVector chain[4] = {
				bp.Vertices[e] + offA + liftV,
				bp.Tangents[e * 2] + offTA + liftV,
				bp.Tangents[e * 2 + 1] + offTB + liftV,
				bp.Vertices[(e + 1) & 3] + offB + liftV,
			};
			for (uint k = 0; k + 1 < 4; ++k)
				dst.push_back(NLMISC::CLine(chain[k], chain[k + 1]));
		}
	}
}

void zpDrawPatchLattice(NL3D::IDriver *driver, NL3D::CCamera *camera,
                        const SPaintZone &pz, int subObj)
{
	if (!driver || !camera || pz.Patches.empty())
		return;

	static const NLMISC::CRGBA kVertColor(255, 255, 255, 255);
	static const NLMISC::CRGBA kVertSelColor(255, 40, 40, 255); // selected red
	// Bound vertices: position DERIVED from a neighbouring patch's edge, not authored. All
	// four bind types qualify - RPatchMesh::UpdateBindingPos recomputes BIND_25/50/75 and
	// BIND_SINGLE alike by interpolating the target edge's Bezier - so the test is Binded,
	// not the bind type. (BIND_50 is the one that literally lands mid-edge, but singling it
	// out would leave the other three looking freely movable, which they are not.)
	static const NLMISC::CRGBA kVertBoundColor(0, 0, 0, 255);
	static const NLMISC::CRGBA kTanColor(80, 230, 120, 255); // handle green

	const NLMISC::CMatrix viewMat = camera->getMatrix().inverted();
	const NL3D::CFrustum &fr = camera->getFrustum();
	uint32 winW = 0, winH = 0;
	driver->getWindowSize(winW, winH);
	if (!winW || !winH)
		return;
	// Integer pixel half-size, so the marker cannot land between two pixel counts.
	const float halfPx = floorf(kPatchVertHalf * (float)winH + 0.5f) < 1.f
		? 1.f : floorf(kPatchVertHalf * (float)winH + 0.5f);
	// Handles read as subordinate to the corner they belong to, so they are drawn a pixel
	// smaller - and never smaller than one pixel.
	const float tanHalfPx = halfPx > 1.f ? halfPx - 1.f : 1.f;

	// The cage LINES live in the depth-tested pre-GUI pass now (zpDrawPatchWire3DAll):
	// the wireframe is occluded by terrain in front of it, with an x-ray faint pass so
	// hidden parts stay readable. Only the pixel-snapped markers and handles remain in
	// this screen-space pass - the artist must always see the selection.

	if (subObj != CPaintMouseListener::SubVertex)
		return;

	// Corner ticks, once per unique vertex. BaseVertices indexes the zone's own vertex table,
	// so a corner reached from four patches marks once - which is also the identity selection
	// will key on.
	std::set<uint16> seen;
	for (uint p = 0; p < pz.Patches.size(); ++p)
	{
		const NL3D::CPatchInfo &pi = pz.Patches[p];
		for (uint c = 0; c < 4; ++c)
		{
			if (!seen.insert(pi.BaseVertices[c]).second)
				continue;
			NLMISC::CVector v;
			if (!zpProjectLifted(viewMat, fr, pi.Patch.Vertices[c] + zpVertOffset(pz, pi.BaseVertices[c]),
			                     kPatchLift, v))
				continue;
			// x scaled by the aspect so the marker is square on screen rather than stretched
			// with the window - normalized coordinates are per-axis. drawQuad's centre+radius
			// overload cannot express that, hence the corner form.
			const uint16 vi = pi.BaseVertices[c];
			const bool sel = g_PatchVertSel.count(TPatchVertId(pz.ZoneId, vi)) != 0;
			// BaseVertices is the SPmPatch V[] index (patch_eval fills it straight from
			// pPatch.V), and the RPO bind table is indexed the same way - so the record for
			// this corner is simply Rp.Verts[vi]. Size-guarded because the two come from
			// different chunks and a malformed file could disagree.
			//
			// TODO (patch move): a bound vertex must not accept a move. it is recomputed
			// from the target edge on load, so the edit would be silently undone - the write
			// op has to refuse these rather than write a position that cannot survive.
			const bool bound = vi < pz.Ep.Rp.Verts.size() && pz.Ep.Rp.Verts[vi].Binded != 0;
			const NLMISC::CRGBA &vcol = sel ? kVertSelColor : (bound ? kVertBoundColor : kVertColor);

			// Snap to the PIXEL GRID. Both the half-size and the centre are rounded to whole
			// pixels, so the marker is the same size every frame and its edges land on pixel
			// boundaries. Without this the square breathes between N and N+1 pixels as the
			// camera moves, which reads as jitter across a field of hundreds of them.
			// Working in pixels also makes the aspect correction unnecessary - one half-size
			// in pixels is square by construction.
			const float cx = floorf(v.x * (float)winW + 0.5f);
			const float cy = floorf(v.y * (float)winH + 0.5f);
			NL3D::CDRU::drawQuad((cx - halfPx) / (float)winW, (cy - halfPx) / (float)winH,
			                     (cx + halfPx) / (float)winW, (cy + halfPx) / (float)winH,
			                     *driver, vcol, NL3D::CViewport());
		}
	}

	// Tangent handles, for SELECTED corners only. The rule, and the reason a dense cage does
	// not become a field of dots: a zone has four handles per patch corner, so showing them all
	// would bury the corners they belong to.
	//
	// A handle is drawn once per unique Vecs index. Two patches meeting along an edge name the
	// same vec in their own slot, so without that it would be drawn - and picked - twice.
	std::set<uint16> seenVec;
	for (uint p = 0; p < pz.Patches.size() && p < pz.Ep.Pm.Patches.size(); ++p)
	{
		const NL3D::CPatchInfo &pi = pz.Patches[p];
		const PIPELINE::MAX::NELPATCH::SPmPatch &pp = pz.Ep.Pm.Patches[p];
		for (uint j = 0; j < 8; ++j)
		{
			const uint corner = (j & 1) ? (((j >> 1) + 1) & 3) : (j >> 1);
			const uint16 owner = pi.BaseVertices[corner];
			if (!g_PatchVertSel.count(TPatchVertId(pz.ZoneId, owner)))
				continue;
			if (pp.Vec[j] < 0)
				continue;
			const uint16 vi = (uint16)pp.Vec[j];
			if (!seenVec.insert(vi).second)
				continue;
			NLMISC::CVector v;
			if (!zpProjectLifted(viewMat, fr, pi.Patch.Tangents[j] + zpTanOffset(pz, vi),
			                     kPatchLift, v))
				continue;
			const bool sel = g_PatchTanSel.count(TPatchVertId(pz.ZoneId, vi)) != 0;
			// Green is handle colour, and being a different hue from the corners is what
			// lets the two be told apart where a handle sits almost on top of its corner.
			const NLMISC::CRGBA &hcol = sel ? kVertSelColor : kTanColor;
			const float hx = floorf(v.x * (float)winW + 0.5f);
			const float hy = floorf(v.y * (float)winH + 0.5f);
			NL3D::CDRU::drawQuad((hx - tanHalfPx) / (float)winW, (hy - tanHalfPx) / (float)winH,
			                     (hx + tanHalfPx) / (float)winW, (hy + tanHalfPx) / (float)winH,
			                     *driver, hcol, NL3D::CViewport());
		}
	}
}

/**
 * Cage for every EDITABLE zone. In Max you are editing an object and its cage is simply up;
 * a cage that follows the mouse is not something you can work on. Cost is bounded by the
 * tool's one-editable-zone-per-max rule - frozen context zones are what make a working set
 * large, and they are skipped.
 *
 * Lives here rather than at the call sites because there are TWO of them: the interactive
 * loop and the --screenshot path each render their own overlay pass, and the prop outline is
 * already duplicated across both.
 */
void zpDrawPatchLatticeAll(NL3D::IDriver *driver, NL3D::CCamera *camera, int subObj)
{
	if (!driver || !camera || !g_PaintCtx.Zones)
		return;
	// (The GIZMO_DRAG dev pin moved to zpDrawPatchWire3DAll, which runs first.)
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
		if (zones[z].Editable)
			zpDrawPatchLattice(driver, camera, zones[z], subObj);
}

static void zpCollectPatchArrows(std::vector<NLMISC::CLine> &out); // defined below

/**
 * The depth-tested wireframe pass: cage lines and frame arrows, drawn in WORLD space
 * BEFORE editorUI->draw() - the driver is still in 3D setup there, which is what the
 * "overlays are 2D-only" invariant was actually about. Two passes per batch: an X-RAY
 * faint pass with the depth test OFF (hidden parts stay readable behind the terrain),
 * then the solid pass depth-tested with a Z-BIAS (the landscape's own decal convention,
 * negative = toward the viewer) so visible lines sit on the surface without fighting
 * it. Lines never write depth. The pixel-snapped markers, handles, gizmo and rubber
 * lines stay in the screen-space pass on top - the artist must always see those.
 */
void zpDrawPatchWire3DAll(NL3D::IDriver *driver, NL3D::CCamera *camera, int subObj)
{
	if (!driver || !camera || !g_PaintCtx.Zones)
		return;
	// The GIZMO_DRAG dev pin lives HERE now: this pass runs first each frame, and the
	// preview offsets both passes read must be pinned before either draws.
	{
		const char *dev = getenv("ZONE_PAINTER_GIZMO_DRAG");
		if (dev && *dev)
		{
			int h = 0;
			float dx = 0.f, dy = 0.f, dz = 0.f;
			if (sscanf(dev, "%d:%f,%f,%f", &h, &dx, &dy, &dz) == 4)
			{
				s_Dragging = true;
				s_DragHandle = h;
				s_DragDelta = NLMISC::CVector(dx, dy, dz);
			}
		}
	}
	static NL3D::CMaterial matFaint, matSolid;
	static bool inited = false;
	if (!inited)
	{
		inited = true;
		matFaint.initUnlit();
		matFaint.setBlend(true);
		matFaint.setBlendFunc(NL3D::CMaterial::srcalpha, NL3D::CMaterial::invsrcalpha);
		matFaint.setZFunc(NL3D::CMaterial::always);
		matFaint.setZWrite(false);
		matSolid.initUnlit();
		matSolid.setZWrite(false);
		matSolid.setZBias(-0.01f);
	}

	std::vector<NLMISC::CLine> plain, selected, arrows;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
		if (zones[z].Editable)
			zpCollectCageLines(zones[z], subObj, plain, selected);
	zpCollectPatchArrows(arrows);
	if (plain.empty() && selected.empty() && arrows.empty())
		return;

	const NL3D::CFrustum &fr = camera->getFrustum();
	driver->setFrustum(fr.Left, fr.Right, fr.Bottom, fr.Top, fr.Near, fr.Far, fr.Perspective);
	driver->setupViewMatrix(camera->getMatrix().inverted());
	driver->setupModelMatrix(NLMISC::CMatrix::Identity);

	static const NLMISC::CRGBA kArrowColor(255, 200, 60, 255);
	struct SBatch { const std::vector<NLMISC::CLine> *L; NLMISC::CRGBA C; };
	const SBatch batches[3] = {
		{ &plain, kCageColor }, { &arrows, kArrowColor }, { &selected, kCageSelColor },
	};
	for (int pass = 0; pass < 2; ++pass)
	{
		NL3D::CMaterial &mat = pass == 0 ? matFaint : matSolid;
		for (int b = 0; b < 3; ++b)
		{
			if (batches[b].L->empty())
				continue;
			NLMISC::CRGBA c = batches[b].C;
			if (pass == 0)
				c.A = 60;
			mat.setColor(c);
			NL3D::CDRU::drawLinesUnlit(*batches[b].L, mat, *driver);
		}
	}
}

/** Screen distance from a point to a segment, in units of viewport height. */
static float zpSegDist(float px, float py, const NLMISC::CVector &a, const NLMISC::CVector &b,
                       float aspect)
{
	const float ax = a.x * aspect, ay = a.y;
	const float bx = b.x * aspect, by = b.y;
	const float qx = px * aspect, qy = py;
	const float ex = bx - ax, ey = by - ay;
	const float len2 = ex * ex + ey * ey;
	float t = 0.f;
	if (len2 > 1e-12f)
	{
		t = ((qx - ax) * ex + (qy - ay) * ey) / len2;
		if (t < 0.f) t = 0.f;
		else if (t > 1.f) t = 1.f;
	}
	const float dx = qx - (ax + ex * t), dy = qy - (ay + ey * t);
	return sqrtf(dx * dx + dy * dy);
}

// Gizmo geometry, in pixels except the axis length which is world (see the fit below).
static const float kGizmoPixels = 110.f;
static const float kGizmoHeadPx = 11.f;
static const float kGizmoHeadWidePx = 4.5f;
static const float kGizmoPickPx = 7.f;
// Plane handles occupy the CORNER between their two axes, inner corner ON the origin. The
// middle of the gizmo is where all three meet, and that meeting point is what the screen
// handle means - leaving it empty would be wrong, the centre is "all axes at once".
static const float kGizmoPlaneFrac = 0.28f;
static const float kGizmoScreenPickPx = 9.f;
// Rotate rings. Enough segments that the circle does not read as a polygon at gizmo size, and
// a slightly larger screen ring so the four never sit on top of each other.
static const int kGizmoRingSegs = 48;
static const float kGizmoRingOuter = 1.18f;
const int kGizmoPlaneAxes[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };

/** Point in a convex quad, screen space. Same-sign cross products against each edge. */
static bool zpQuadContains(float px, float py, const float *qx, const float *qy)
{
	int sign = 0;
	for (int i = 0; i < 4; ++i)
	{
		const int j = (i + 1) & 3;
		const float cross = (qx[j] - qx[i]) * (py - qy[i]) - (qy[j] - qy[i]) * (px - qx[i]);
		if (cross > 0.f)
		{
			if (sign < 0) return false;
			sign = 1;
		}
		else if (cross < 0.f)
		{
			if (sign > 0) return false;
			sign = -1;
		}
	}
	return true;
}

// Fit-at-rest state: the gizmo holds a WORLD length, re-fitted only between interactions, so
// it never resizes under a drag or a view move. Same model as the navigation sample, and the
// reason it is not simply drawn at a constant pixel length.
float s_GizmoWorldLen = 1.f;
static bool s_GizmoFitDirty = true;
static uint32 s_GizmoFitSerial = 0xffffffffu;
static uint32 s_GizmoFitW = 0, s_GizmoFitH = 0;
static int s_GizmoHover = -1;

int zpPatchGizmoHover() { return s_GizmoHover; }
void zpPatchGizmoInvalidate() { s_GizmoFitDirty = true; }

void zpDrawPatchGizmo(NL3D::IDriver *driver, NL3D::CCamera *camera,
                      float mouseX, float mouseY, bool navigating, uint32 viewSerial)
{
	s_GizmoHover = ZPGIZ_NONE;
	if (!driver || !camera)
		return;
	NLMISC::CVector o;
	if (!zpTransformPivot(o))
		return;
	// A MOVE rides the drag, so the handle stays under the pointer. Rotate and scale leave the
	// gizmo where it is: the pivot is their anchor and does not move - s_DragDelta is set during
	// a scale drag too (it is the raw plane-hit delta the factor is derived from), and adding it
	// here slid the whole gizmo off the pivot while scaling.
	if (s_DragXform.Kind == ZPXF_Move)
		o += s_DragDelta;
	// Hidden while the view moves: the size it would be drawn at is stale by definition, and
	// a re-fit mid-navigation lands as a visible jump.
	if (navigating)
	{
		s_GizmoFitDirty = true;
		return;
	}

	uint32 winW = 0, winH = 0;
	driver->getWindowSize(winW, winH);
	if (!winW || !winH)
		return;
	if (viewSerial != s_GizmoFitSerial || winW != s_GizmoFitW || winH != s_GizmoFitH)
	{
		s_GizmoFitSerial = viewSerial;
		s_GizmoFitW = winW;
		s_GizmoFitH = winH;
		s_GizmoFitDirty = true;
	}

	const NLMISC::CMatrix camMat = camera->getMatrix();
	const NL3D::CFrustum &fr = camera->getFrustum();
	if (s_GizmoFitDirty)
	{
		// World length that projects to kGizmoPixels at the selection's depth.
		const float depth = (o - camMat.getPos()) * camMat.getJ();
		const float d = depth > fr.Near ? depth : fr.Near;
		const float worldPerPixelAtNear = (fr.Top - fr.Bottom) / (float)winH;
		s_GizmoWorldLen = kGizmoPixels * worldPerPixelAtNear * d / fr.Near;
		if (s_GizmoWorldLen <= 0.f)
			s_GizmoWorldLen = 1.f;
		s_GizmoFitDirty = false;
	}

	const NLMISC::CMatrix viewMat = camMat.inverted();
	NLMISC::CVector po;
	if (!zpProjectLifted(viewMat, fr, o, kPatchLift, po))
		return;
	// Everything below is in PIXELS: the axis directions come out of the projection, so
	// arrowheads and pick distances are the same size whatever the axis foreshortening.
	const float ox = po.x * (float)winW, oy = po.y * (float)winH;

	static const NLMISC::CRGBA kAxisCol[3] = {
		NLMISC::CRGBA(255, 70, 70, 255), NLMISC::CRGBA(70, 230, 70, 255),
		NLMISC::CRGBA(90, 140, 255, 255) };
	static const NLMISC::CRGBA kHotCol(255, 220, 40, 255);

	NLMISC::CVector axisVec[3];
	for (int a = 0; a < 3; ++a)
	{
		axisVec[a] = NLMISC::CVector::Null;
		if (a == 0) axisVec[a].x = 1.f; else if (a == 1) axisVec[a].y = 1.f; else axisVec[a].z = 1.f;
	}

	float tipX[3], tipY[3];
	bool tipOk[3];
	for (int a = 0; a < 3; ++a)
	{
		NLMISC::CVector pt;
		tipOk[a] = zpProjectLifted(viewMat, fr, o + axisVec[a] * s_GizmoWorldLen, kPatchLift, pt);
		tipX[a] = pt.x * (float)winW;
		tipY[a] = pt.y * (float)winH;
	}

	// Plane corners, projected. Inner corner is the origin itself, so index 0 of every quad
	// is the gizmo centre.
	const float planeLen = s_GizmoWorldLen * kGizmoPlaneFrac;
	float planeX[3][4], planeY[3][4];
	bool planeOk[3];
	for (int q = 0; q < 3; ++q)
	{
		const NLMISC::CVector &a = axisVec[kGizmoPlaneAxes[q][0]];
		const NLMISC::CVector &b = axisVec[kGizmoPlaneAxes[q][1]];
		const NLMISC::CVector corner[4] = { o, o + a * planeLen, o + (a + b) * planeLen, o + b * planeLen };
		planeOk[q] = true;
		for (int k = 0; k < 4; ++k)
		{
			NLMISC::CVector pt;
			if (!zpProjectLifted(viewMat, fr, corner[k], kPatchLift, pt))
			{
				planeOk[q] = false;
				break;
			}
			planeX[q][k] = pt.x * (float)winW;
			planeY[q][k] = pt.y * (float)winH;
		}
	}

	const float mpx = mouseX * (float)winW, mpy = mouseY * (float)winH;
	const char *forceHoverEnv = getenv("ZONE_PAINTER_GIZMO_HOVER");

	if (g_XformKind == ZPXF_Rotate)
	{
		// Rings, not arrows: an axis arrow says "along", a ring says "about", and a rotate
		// gizmo that looked like a move gizmo would be read as one.
		//
		// Four rings - the three world axes plus a screen-aligned one for turning about the
		// view direction, which is the only way to rotate about an axis pointing at you.
		const NLMISC::CVector fwd = camMat.getJ();
		NLMISC::CVector ringAxis[4] = { axisVec[0], axisVec[1], axisVec[2], fwd };
		const int handleOf[4] = { ZPGIZ_AXIS_X, ZPGIZ_AXIS_Y, ZPGIZ_AXIS_Z, ZPGIZ_SCREEN };
		float px[4][kGizmoRingSegs + 1], py[4][kGizmoRingSegs + 1];
		bool ringOk[4];
		for (int r = 0; r < 4; ++r)
		{
			// Any two vectors spanning the plane the ring lies in.
			NLMISC::CVector n = ringAxis[r];
			if (n.norm() < 1e-6f) { ringOk[r] = false; continue; }
			n.normalize();
			NLMISC::CVector u = n ^ NLMISC::CVector(0.f, 0.f, 1.f);
			if (u.norm() < 1e-3f)
				u = n ^ NLMISC::CVector(1.f, 0.f, 0.f);
			u.normalize();
			const NLMISC::CVector v = n ^ u;
			// The screen ring sits slightly outside the others so the four never coincide.
			const float rad = s_GizmoWorldLen * (r == 3 ? kGizmoRingOuter : 1.f);
			ringOk[r] = true;
			for (int k = 0; k <= kGizmoRingSegs; ++k)
			{
				const float t = (float)k * 2.f * (float)NLMISC::Pi / (float)kGizmoRingSegs;
				NLMISC::CVector pt;
				if (!zpProjectLifted(viewMat, fr, o + (u * cosf(t) + v * sinf(t)) * rad,
				                     kPatchLift, pt))
				{
					ringOk[r] = false;
					break;
				}
				px[r][k] = pt.x * (float)winW;
				py[r][k] = pt.y * (float)winH;
			}
		}
		// Nearest ring within the pick radius, measured against the drawn polyline.
		float bestR = kGizmoPickPx;
		for (int r = 0; r < 4; ++r)
		{
			if (!ringOk[r])
				continue;
			for (int k = 0; k < kGizmoRingSegs; ++k)
			{
				const NLMISC::CVector a(px[r][k], py[r][k], 0.f);
				const NLMISC::CVector b(px[r][k + 1], py[r][k + 1], 0.f);
				const float d = zpSegDist(mpx, mpy, a, b, 1.f);
				if (d < bestR)
				{
					bestR = d;
					s_GizmoHover = handleOf[r];
				}
			}
		}
		if (forceHoverEnv && *forceHoverEnv)
			s_GizmoHover = atoi(forceHoverEnv);
		if (s_Dragging)
			s_GizmoHover = s_DragHandle;
		static const NLMISC::CRGBA kScreenRing(200, 200, 200, 255);
		for (int r = 0; r < 4; ++r)
		{
			if (!ringOk[r])
				continue;
			const NLMISC::CRGBA col = (s_GizmoHover == handleOf[r])
				? kHotCol : (r == 3 ? kScreenRing : kAxisCol[r]);
			for (int k = 0; k < kGizmoRingSegs; ++k)
				NL3D::CDRU::drawLine(px[r][k] / winW, py[r][k] / winH,
				                     px[r][k + 1] / winW, py[r][k + 1] / winH, *driver, col);
		}
		return;
	}

	// Dev hook, same shape as ZONE_PAINTER_ZOOM_EXTENTS: force a handle hot so the hover
	// states are reachable from a --screenshot run, where there is no pointer to place.
	const char *forceHover = forceHoverEnv;

	// Centre first, then planes, then axes: smaller and more specific targets win, which is
	// also the order Max resolves gizmo overlaps in. The centre is invisible, so its target
	// is generous - and it is tested AHEAD of the plane corners it sits on top of, because
	// the very middle means all axes at once whichever corner is under it.
	{
		const float dx = mpx - ox, dy = mpy - oy;
		if (sqrtf(dx * dx + dy * dy) < kGizmoScreenPickPx)
			s_GizmoHover = ZPGIZ_SCREEN;
	}
	if (s_GizmoHover == ZPGIZ_NONE)
	{
		for (int q = 0; q < 3; ++q)
		{
			if (planeOk[q] && zpQuadContains(mpx, mpy, planeX[q], planeY[q]))
			{
				s_GizmoHover = ZPGIZ_PLANE_XY + q;
				break;
			}
		}
	}

	if (forceHover && *forceHover)
		s_GizmoHover = atoi(forceHover);
	if (s_Dragging)
		s_GizmoHover = s_DragHandle; // the handle you grabbed stays lit for the whole drag

	// Nearest axis within the pick radius, measured in pixels along the drawn segment.
	float best = kGizmoPickPx;
	for (int a = 0; s_GizmoHover == ZPGIZ_NONE && a < 3; ++a)
	{
		if (!tipOk[a])
			continue;
		const float vx = tipX[a] - ox, vy = tipY[a] - oy;
		const float len2 = vx * vx + vy * vy;
		float t = 0.f;
		if (len2 > 1e-6f)
		{
			t = ((mouseX * (float)winW - ox) * vx + (mouseY * (float)winH - oy) * vy) / len2;
			t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
		}
		const float dx = mouseX * (float)winW - (ox + vx * t);
		const float dy = mouseY * (float)winH - (oy + vy * t);
		const float d = sqrtf(dx * dx + dy * dy);
		if (d < best)
		{
			best = d;
			s_GizmoHover = ZPGIZ_AXIS_X + a;
		}
	}

	// Planes under the axes, so the axes stay legible where they cross.
	for (int q = 0; q < 3; ++q)
	{
		if (!planeOk[q])
			continue;
		const bool hot = (s_GizmoHover == ZPGIZ_PLANE_XY + q) || (s_GizmoHover == ZPGIZ_SCREEN);
		NLMISC::CRGBA c = hot ? kHotCol : kAxisCol[kGizmoPlaneAxes[q][0]];
		c.A = hot ? 190 : 130;
		NL3D::CDRU::drawTriangle(planeX[q][0] / winW, planeY[q][0] / winH,
		                         planeX[q][1] / winW, planeY[q][1] / winH,
		                         planeX[q][2] / winW, planeY[q][2] / winH,
		                         *driver, c, NL3D::CViewport());
		NL3D::CDRU::drawTriangle(planeX[q][0] / winW, planeY[q][0] / winH,
		                         planeX[q][2] / winW, planeY[q][2] / winH,
		                         planeX[q][3] / winW, planeY[q][3] / winH,
		                         *driver, c, NL3D::CViewport());
		// Outer edges solid. A translucent fill alone vanishes against bright terrain - the
		// landscape here is not the sample's dark background - and the two outer edges are
		// also how the handle is drawn: a corner bracket rather than a patch of colour.
		NLMISC::CRGBA edge = hot ? kHotCol : kAxisCol[kGizmoPlaneAxes[q][0]];
		NL3D::CDRU::drawLine(planeX[q][1] / winW, planeY[q][1] / winH,
		                     planeX[q][2] / winW, planeY[q][2] / winH, *driver, edge);
		NL3D::CDRU::drawLine(planeX[q][2] / winW, planeY[q][2] / winH,
		                     planeX[q][3] / winW, planeY[q][3] / winH, *driver, edge);
	}

	for (int a = 0; a < 3; ++a)
	{
		if (!tipOk[a])
			continue;
		const NLMISC::CRGBA col = (s_GizmoHover == ZPGIZ_AXIS_X + a) ? kHotCol : kAxisCol[a];
		float vx = tipX[a] - ox, vy = tipY[a] - oy;
		const float len = sqrtf(vx * vx + vy * vy);
		if (len < 1e-3f)
			continue; // axis points straight at the camera: nothing meaningful to draw
		vx /= len;
		vy /= len;
		// Shaft stops where the head begins, so the two do not overdraw each other.
		const float shaftEndX = tipX[a] - vx * kGizmoHeadPx;
		const float shaftEndY = tipY[a] - vy * kGizmoHeadPx;
		NL3D::CDRU::drawLine(ox / winW, oy / winH, shaftEndX / winW, shaftEndY / winH,
		                     *driver, col);
		const float nx = -vy, ny = vx;
		if (g_XformKind == ZPXF_Scale)
		{
			// A BOX, not an arrowhead. Scale has no direction the way a move does - it pushes
			// out and pulls in along the same axis - and a shape that reads as an arrow would
			// promise a translation.
			const float h = kGizmoHeadWidePx;
			const float c0x = shaftEndX + nx * h, c0y = shaftEndY + ny * h;
			const float c1x = shaftEndX - nx * h, c1y = shaftEndY - ny * h;
			const float c2x = tipX[a] - nx * h, c2y = tipY[a] - ny * h;
			const float c3x = tipX[a] + nx * h, c3y = tipY[a] + ny * h;
			NL3D::CDRU::drawTriangle(c0x / winW, c0y / winH, c1x / winW, c1y / winH,
			                         c2x / winW, c2y / winH, *driver, col, NL3D::CViewport());
			NL3D::CDRU::drawTriangle(c0x / winW, c0y / winH, c2x / winW, c2y / winH,
			                         c3x / winW, c3y / winH, *driver, col, NL3D::CViewport());
			continue;
		}
		// Filled head: base corners are the shaft end pushed along the screen normal.
		NL3D::CDRU::drawTriangle(tipX[a] / winW, tipY[a] / winH,
		                         (shaftEndX + nx * kGizmoHeadWidePx) / winW,
		                         (shaftEndY + ny * kGizmoHeadWidePx) / winH,
		                         (shaftEndX - nx * kGizmoHeadWidePx) / winW,
		                         (shaftEndY - ny * kGizmoHeadWidePx) / winH,
		                         *driver, col, NL3D::CViewport());
	}
}

bool zpPickPatchVertex(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                       uint &zoneOut, uint16 &vertOut)
{
	if (!camera || !driver || !g_PaintCtx.Zones)
		return false;
	const NLMISC::CMatrix viewMat = camera->getMatrix().inverted();
	const NL3D::CFrustum &fr = camera->getFrustum();
	uint32 winW = 0, winH = 0;
	driver->getWindowSize(winW, winH);
	// Distances measured in units of viewport HEIGHT, so x is scaled by the aspect first:
	// normalized coordinates are per-axis and an uncorrected radius is an ellipse on screen.
	const float aspect = (winW && winH) ? (float)winW / (float)winH : 1.f;

	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	float best = kPatchVertPick;
	bool found = false;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &pz = zones[z];
		if (!pz.Editable)
			continue; // frozen context is not editable, so it is not selectable either
		std::set<uint16> seen;
		for (uint p = 0; p < pz.Patches.size(); ++p)
		{
			const NL3D::CPatchInfo &pi = pz.Patches[p];
			for (uint c = 0; c < 4; ++c)
			{
				if (!seen.insert(pi.BaseVertices[c]).second)
					continue;
				NLMISC::CVector v;
				if (!zpProjectLifted(viewMat, fr, pi.Patch.Vertices[c], kPatchLift, v))
					continue;
				const float dx = (v.x - mx) * aspect, dy = v.y - my;
				const float d = sqrtf(dx * dx + dy * dy);
				if (d < best)
				{
					best = d;
					zoneOut = pz.ZoneId;
					vertOut = pi.BaseVertices[c];
					found = true;
				}
			}
		}
	}
	return found;
}

/**
 * Nearest cage edge to the pointer.
 *
 * Measured against the DRAWN chain (V -> T -> T -> V), not against the straight line between
 * the corners: a patch edge with real tangents bows visibly away from that line, and picking
 * what is drawn is the only version an artist can aim at.
 */
bool zpPickPatchEdge(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                     uint &zoneOut, uint16 &vertAOut, uint16 &vertBOut)
{
	if (!camera || !driver || !g_PaintCtx.Zones)
		return false;
	const NLMISC::CMatrix viewMat = camera->getMatrix().inverted();
	const NL3D::CFrustum &fr = camera->getFrustum();
	uint32 winW = 0, winH = 0;
	driver->getWindowSize(winW, winH);
	const float aspect = (winW && winH) ? (float)winW / (float)winH : 1.f;

	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	float best = kPatchEdgePick;
	bool found = false;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &pz = zones[z];
		if (!pz.Editable)
			continue;
		for (uint p = 0; p < pz.Patches.size(); ++p)
		{
			const NL3D::CPatchInfo &pi = pz.Patches[p];
			const NL3D::CBezierPatch &bp = pi.Patch;
			for (uint e = 0; e < 4; ++e)
			{
				NLMISC::CVector chain[4];
				bool ok = true;
				ok = ok && zpProjectLifted(viewMat, fr, bp.Vertices[e], kPatchLift, chain[0]);
				ok = ok && zpProjectLifted(viewMat, fr, bp.Tangents[e * 2], kPatchLift, chain[1]);
				ok = ok && zpProjectLifted(viewMat, fr, bp.Tangents[e * 2 + 1], kPatchLift, chain[2]);
				ok = ok && zpProjectLifted(viewMat, fr, bp.Vertices[(e + 1) & 3], kPatchLift, chain[3]);
				if (!ok)
					continue;
				for (uint k = 0; k + 1 < 4; ++k)
				{
					const float d = zpSegDist(mx, my, chain[k], chain[k + 1], aspect);
					if (d >= best)
						continue;
					best = d;
					zoneOut = pz.ZoneId;
					vertAOut = pi.BaseVertices[e];
					vertBOut = pi.BaseVertices[(e + 1) & 3];
					found = true;
				}
			}
		}
	}
	return found;
}

/**
 * Patch under the pointer.
 *
 * Point-in-quad on the projected CORNERS, which is the cage quad rather than the tessellated
 * surface - the artist is picking a cell of the lattice they can see. Ties go to the nearest,
 * so a patch behind another does not steal the click.
 */
bool zpPickPatchFace(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                     uint &zoneOut, uint &patchOut)
{
	if (!camera || !driver || !g_PaintCtx.Zones)
		return false;
	const NLMISC::CMatrix viewMat = camera->getMatrix().inverted();
	const NL3D::CFrustum &fr = camera->getFrustum();

	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	bool found = false;
	float bestDepth = 0.f;
	const NLMISC::CVector eye = camera->getMatrix().getPos();
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &pz = zones[z];
		if (!pz.Editable)
			continue;
		for (uint p = 0; p < pz.Patches.size(); ++p)
		{
			const NL3D::CBezierPatch &bp = pz.Patches[p].Patch;
			float qx[4], qy[4];
			bool ok = true;
			for (uint c = 0; c < 4 && ok; ++c)
			{
				NLMISC::CVector v;
				ok = zpProjectLifted(viewMat, fr, bp.Vertices[c], kPatchLift, v);
				qx[c] = v.x;
				qy[c] = v.y;
			}
			if (!ok || !zpQuadContains(mx, my, qx, qy))
				continue;
			NLMISC::CVector centre = bp.Vertices[0] + bp.Vertices[1] + bp.Vertices[2] + bp.Vertices[3];
			centre *= 0.25f;
			const float depth = (centre - eye).norm();
			if (found && depth >= bestDepth)
				continue;
			bestDepth = depth;
			zoneOut = pz.ZoneId;
			patchOut = p;
			found = true;
		}
	}
	return found;
}

/**
 * Selection modifiers: plain click replaces, Ctrl adds, Alt removes. A click that hits
 * nothing clears - the same object-selection feel Prop mode already has.
 */
/**
 * Scripted click at a normalized viewport position, with the session's own camera and driver.
 *
 * The picking is screen-space maths on projected geometry and is exactly the part a byte gate
 * and a selection-by-index gate both miss: a script that calls selectPatchEdge proves the
 * selection machinery, not that clicking on an edge finds that edge. `--screenshot` has no
 * pointer, so without this the pick functions cannot be exercised headlessly at all.
 */
bool zpPatchClickAt(float x, float y, uint buttons)
{
	if (!g_PaintCtx.Camera || !g_PaintCtx.UDriver)
		return false;
	NL3D::IDriver *driver = static_cast<NL3D::CDriverUser *>(g_PaintCtx.UDriver)->getDriver();
	if (!driver)
		return false;
	zpPatchVertexClick(g_PaintCtx.Camera, driver, x, y, buttons);
	return true;
}

/**
 * Screen position of a vertex (the same projection the pick uses), for scripts that need
 * to AIM the real pick paths - zpPatchClickAt and zpWeldDragAt land where this says.
 */
bool zpPatchVertScreen(uint zoneId, uint vertIdx, float &sxOut, float &syOut)
{
	if (!g_PaintCtx.Camera)
		return false;
	float w[3];
	if (!zpPatchVertWorld(zoneId, (uint16)vertIdx, w))
		return false;
	const NLMISC::CMatrix viewMat = g_PaintCtx.Camera->getMatrix().inverted();
	NLMISC::CVector p;
	if (!zpProjectLifted(viewMat, g_PaintCtx.Camera->getFrustum(),
	                     NLMISC::CVector(w[0], w[1], w[2]), kPatchLift, p))
		return false;
	sxOut = p.x;
	syOut = p.y;
	return true;
}

/**
 * Per-patch frame arrows (g_ShowArrows): a thin line arrow centred in each editable
 * patch, pointing along MINUS v - the direction a rot-0 tile's ADDITIVE arrow renders
 * (pinned empirically with a rot-0 fill legend against the ring axes). Collected as
 * WORLD segments for the depth-tested wireframe pass; the head is sized from the arrow
 * itself, so it scales with the patch rather than the screen.
 */
static void zpCollectPatchArrows(std::vector<NLMISC::CLine> &out)
{
	if (!g_ShowArrows || !g_PaintCtx.Zones)
		return;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	const NLMISC::CVector liftV(0.f, 0.f, kPatchLift);
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &pz = zones[z];
		if (!pz.Editable)
			continue;
		for (uint p = 0; p < pz.Patches.size(); ++p)
		{
			const NLMISC::CVector *V = pz.Patches[p].Patch.Vertices;
			// Bilinear frame point: u toward ring V3, v toward ring V1; base at v=0.70,
			// tip at v=0.30 - the MINUS v the additive arrows agree with.
			const float u = 0.5f;
			const NLMISC::CVector a = V[0] * ((1.f - u) * 0.30f) + V[1] * ((1.f - u) * 0.70f)
				+ V[2] * (u * 0.70f) + V[3] * (u * 0.30f) + liftV; // P(0.5, 0.70)
			const NLMISC::CVector b = V[0] * ((1.f - u) * 0.70f) + V[1] * ((1.f - u) * 0.30f)
				+ V[2] * (u * 0.30f) + V[3] * (u * 0.70f) + liftV; // P(0.5, 0.30) - the tip
			out.push_back(NLMISC::CLine(a, b));
			NLMISC::CVector dir = b - a;
			const float len = dir.norm();
			if (len < 1e-3f)
				continue;
			dir /= len;
			// A lateral in the patch plane: the u axis direction at the tip's altitude.
			NLMISC::CVector lat = (V[3] + V[2]) * 0.5f - (V[0] + V[1]) * 0.5f;
			const float ll = lat.norm();
			if (ll < 1e-3f)
				continue;
			lat /= ll;
			const NLMISC::CVector back = b - dir * (len * 0.18f);
			out.push_back(NLMISC::CLine(b, back + lat * (len * 0.08f)));
			out.push_back(NLMISC::CLine(b, back - lat * (len * 0.08f)));
		}
	}
}
/** Screen position of a tangent handle, the vertex form's sibling (gates aim picks at it). */
bool zpPatchTangentScreen(uint zoneId, uint vecIdx, float &sxOut, float &syOut)
{
	if (!g_PaintCtx.Camera)
		return false;
	float w[3];
	if (!zpPatchTangentWorld(zoneId, vecIdx, w))
		return false;
	const NLMISC::CMatrix viewMat = g_PaintCtx.Camera->getMatrix().inverted();
	NLMISC::CVector p;
	if (!zpProjectLifted(viewMat, g_PaintCtx.Camera->getFrustum(),
	                     NLMISC::CVector(w[0], w[1], w[2]), kPatchLift, p))
		return false;
	sxOut = p.x;
	syOut = p.y;
	return true;
}

/**
 * Scripted target-weld drag: press at (x0,y0), release at (x1,y1), through the REAL
 * begin/finish handlers (same caveat as zpPatchClickAt - this proves the drag machinery,
 * not the event dispatch that feeds it).
 */
/** Scripted SHIFT-drag extrude: press at (x0,y0), drag to (x1,y1), release - through the
 *  REAL begin/update/end handlers (the zpWeldDragAt caveat applies: this proves the drag
 *  machinery, not the event dispatch that feeds it). */
bool zpExtrudeDragAt(float x0, float y0, float x1, float y1)
{
	if (!g_PaintCtx.Camera)
		return false;
	const NL3D::CViewport vp;
	if (!zpPatchGizmoBeginExtrudeDrag(g_PaintCtx.Camera, vp, x0, y0))
		return false;
	zpPatchGizmoUpdateDrag(g_PaintCtx.Camera, vp, x1, y1);
	zpPatchGizmoEndDrag();
	return true;
}

bool zpWeldDragAt(float x0, float y0, float x1, float y1)
{
	if (!g_PaintCtx.Camera || !g_PaintCtx.UDriver)
		return false;
	NL3D::IDriver *driver = static_cast<NL3D::CDriverUser *>(g_PaintCtx.UDriver)->getDriver();
	if (!driver)
		return false;
	if (!zpWeldDragBegin(g_PaintCtx.Camera, driver, x0, y0))
		return false;
	zpWeldDragUpdate(x1, y1);
	zpWeldDragFinish(g_PaintCtx.Camera, driver, x1, y1);
	return true;
}

/**
 * Nearest visible HANDLE to the pointer.
 *
 * Only handles of selected corners are drawn, and only drawn things can be picked - otherwise
 * the artist would hit something invisible. Tighter radius than a corner, and tried FIRST by
 * the caller: a handle sits close to its corner and would otherwise never win.
 */
bool zpPickPatchTangent(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my,
                        uint &zoneOut, uint16 &vecOut)
{
	if (!camera || !driver || !g_PaintCtx.Zones || g_PatchVertSel.empty())
		return false;
	const NLMISC::CMatrix viewMat = camera->getMatrix().inverted();
	const NL3D::CFrustum &fr = camera->getFrustum();
	uint32 winW = 0, winH = 0;
	driver->getWindowSize(winW, winH);
	const float aspect = (winW && winH) ? (float)winW / (float)winH : 1.f;

	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	float best = kPatchTanPick;
	bool found = false;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &pz = zones[z];
		if (!pz.Editable)
			continue;
		std::set<uint16> seen;
		for (uint p = 0; p < pz.Patches.size() && p < pz.Ep.Pm.Patches.size(); ++p)
		{
			const NL3D::CPatchInfo &pi = pz.Patches[p];
			const PIPELINE::MAX::NELPATCH::SPmPatch &pp = pz.Ep.Pm.Patches[p];
			for (uint j = 0; j < 8; ++j)
			{
				const uint corner = (j & 1) ? (((j >> 1) + 1) & 3) : (j >> 1);
				if (!g_PatchVertSel.count(TPatchVertId(pz.ZoneId, pi.BaseVertices[corner])))
					continue;
				if (pp.Vec[j] < 0 || !seen.insert((uint16)pp.Vec[j]).second)
					continue;
				NLMISC::CVector v;
				if (!zpProjectLifted(viewMat, fr, pi.Patch.Tangents[j], kPatchLift, v))
					continue;
				const float dx = (v.x - mx) * aspect, dy = v.y - my;
				const float d = sqrtf(dx * dx + dy * dy);
				if (d >= best)
					continue;
				best = d;
				zoneOut = pz.ZoneId;
				vecOut = (uint16)pp.Vec[j];
				found = true;
			}
		}
	}
	return found;
}

void zpPatchVertexClick(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my, uint buttons)
{
	int op = 0;
	if (buttons & NLMISC::ctrlButton)
		op = 1;
	else if (buttons & NLMISC::altButton)
		op = 2;
	const int level = g_PaintCtx.Paint ? g_PaintCtx.Paint->SubObj : CPaintMouseListener::SubVertex;

	// Only a PLAIN click that hits nothing clears. A Ctrl or Alt click is an add or a remove -
	// missing the target and losing the whole selection over it would punish exactly the artist
	// who is building one up carefully, and Max keeps the selection on a modified miss too.
	if (level == CPaintMouseListener::SubEdge)
	{
		uint zone = 0;
		uint16 a = 0, b = 0;
		if (!zpPickPatchEdge(camera, driver, mx, my, zone, a, b))
		{
			if (op == 0)
				zpPatchVertClear();
			return;
		}
		zpPatchEdgeSelect(zone, a, b, op);
		return;
	}
	if (level == CPaintMouseListener::SubPatch)
	{
		uint zone = 0, patch = 0;
		if (!zpPickPatchFace(camera, driver, mx, my, zone, patch))
		{
			if (op == 0)
				zpPatchVertClear();
			return;
		}
		zpPatchFaceSelect(zone, patch, op);
		return;
	}

	// Handles first: they are drawn on top of the cage and sit near their corner, so a
	// corner-first order would make them unreachable wherever the two overlap. The
	// Filter Vertices / Vectors pair gates what a click can PICK, never what is drawn;
	// a filtered-out pick falls through, so a plain click still clears.
	uint zone = 0;
	uint16 vec = 0;
	if (g_PatchFilterVecs && zpPickPatchTangent(camera, driver, mx, my, zone, vec))
	{
		zpPatchTangentSelect(zone, vec, op);
		return;
	}
	uint16 vert = 0;
	if (!g_PatchFilterVerts || !zpPickPatchVertex(camera, driver, mx, my, zone, vert))
	{
		if (op == 0)
			zpPatchVertClear();
		return;
	}
	zpPatchVertSelect(zone, vert, op);
}

// ---------------------------------------------------------------------------------------------
// Target-weld drag: the armed command mode. Dragging a vertex ONTO another welds the
// dragged one INTO the target - the target keeps its position and identity, and NOTHING
// moves during the drag: the gesture is a selection mechanism (source, then target), the
// rubber line is its only visual. Arm with the panel Target toggle; right click or ESC
// cancels a live drag and disarms; the mode stays armed across welds (legacy Target Weld).

static bool s_WeldDragActive = false;
static uint s_WeldSrcZone = 0;
static uint16 s_WeldSrcVert = 0;
static float s_WeldMx = 0.f, s_WeldMy = 0.f;

bool zpWeldDragActive() { return s_WeldDragActive; }

/** Try to begin the drag at the pointer: only a press ON a vertex starts it. */
bool zpWeldDragBegin(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my)
{
	uint zone = 0;
	uint16 vert = 0;
	if (!zpPickPatchVertex(camera, driver, mx, my, zone, vert))
		return false;
	s_WeldDragActive = true;
	s_WeldSrcZone = zone;
	s_WeldSrcVert = vert;
	s_WeldMx = mx;
	s_WeldMy = my;
	g_PropStatusMsg = NLMISC::toString("target weld: drag vertex %u onto its target", (uint)vert);
	return true;
}

void zpWeldDragUpdate(float mx, float my)
{
	s_WeldMx = mx;
	s_WeldMy = my;
}

/** Would releasing over (zone, vert) weld? Shared by the release and the candidate diamond,
 *  so the overlay never marks a target the release would refuse. Same OBJECT only (a pick
 *  through a sibling node resolves to the same indices), and never the source vertex itself. */
static bool zpWeldDragTargetOk(uint zone, uint16 vert)
{
	if (vert == s_WeldSrcVert)
	{
		// Same index through the same object is the same vertex; through another object it
		// is a foreign zone and refused below anyway.
		const SPaintZone *a = zpFindPaintZone(zone);
		const SPaintZone *b = zpFindPaintZone(s_WeldSrcZone);
		if (a && b && a->Node == b->Node)
			return false;
	}
	if (zone == s_WeldSrcZone)
		return true;
	const SPaintZone *a = zpFindPaintZone(zone);
	const SPaintZone *b = zpFindPaintZone(s_WeldSrcZone);
	return a && b && a->Node == b->Node; // another viewpoint on the same object: indices agree
}

/** Release: weld the source INTO the vertex under the pointer (same object only). */
void zpWeldDragFinish(NL3D::CCamera *camera, NL3D::IDriver *driver, float mx, float my)
{
	if (!s_WeldDragActive)
		return;
	s_WeldDragActive = false;
	uint zone = 0;
	uint16 vert = 0;
	if (!zpPickPatchVertex(camera, driver, mx, my, zone, vert))
	{
		g_PropStatusMsg = "target weld: released over nothing";
		return;
	}
	if (!zpWeldDragTargetOk(zone, vert))
	{
		const SPaintZone *a = zpFindPaintZone(zone);
		const SPaintZone *b = zpFindPaintZone(s_WeldSrcZone);
		g_PropStatusMsg = (a && b && a->Node == b->Node)
			? "target weld: released on the same vertex"
			: "target weld: target is in another zone";
		return;
	}
	zpWeldVertexInto(s_WeldSrcZone, s_WeldSrcVert, vert);
}

void zpWeldDragCancel()
{
	if (!s_WeldDragActive)
		return;
	s_WeldDragActive = false;
	g_PropStatusMsg = "target weld: cancelled";
}

/** The rubber line (2D pass, drawn with the other overlays; project by hand). */
void zpDrawWeldDrag(NL3D::IDriver *driver, NL3D::CCamera *camera)
{
	// Dev hook for screenshots: ZONE_PAINTER_WELD_DRAG="zone,vert,mx,my" forces a live
	// drag so the overlay can be seen without a pointer.
	if (!s_WeldDragActive)
	{
		const char *dev = getenv("ZONE_PAINTER_WELD_DRAG");
		if (dev)
		{
			uint z = 0, v = 0;
			float mx = 0.f, my = 0.f;
			if (sscanf(dev, "%u,%u,%f,%f", &z, &v, &mx, &my) == 4)
			{
				s_WeldDragActive = true;
				s_WeldSrcZone = z;
				s_WeldSrcVert = (uint16)v;
				s_WeldMx = mx;
				s_WeldMy = my;
			}
		}
		if (!s_WeldDragActive)
			return;
	}
	if (!driver || !camera)
		return;
	float src[3];
	if (!zpPatchVertWorld(s_WeldSrcZone, s_WeldSrcVert, src))
		return;
	const NLMISC::CMatrix viewMat = camera->getMatrix().inverted();
	NLMISC::CVector p;
	if (!zpProjectLifted(viewMat, camera->getFrustum(),
	                     NLMISC::CVector(src[0], src[1], src[2]), kPatchLift, p))
		return;
	const NLMISC::CRGBA col(255, 160, 60, 255);
	NL3D::CDRU::drawLine(p.x, p.y, s_WeldMx, s_WeldMy, *driver, col, NL3D::CViewport());
	// A small diamond on the candidate target under the pointer - only where the release
	// would actually weld, so the overlay never promises what the drop refuses.
	uint zone = 0;
	uint16 vert = 0;
	if (zpPickPatchVertex(camera, driver, s_WeldMx, s_WeldMy, zone, vert)
	    && zpWeldDragTargetOk(zone, vert))
	{
		float dst[3];
		if (zpPatchVertWorld(zone, vert, dst))
		{
			NLMISC::CVector q;
			if (zpProjectLifted(viewMat, camera->getFrustum(),
			                    NLMISC::CVector(dst[0], dst[1], dst[2]), kPatchLift, q))
			{
				uint32 winW = 0, winH = 0;
				driver->getWindowSize(winW, winH);
				const float rx = winW ? 7.f / (float)winW : 0.01f;
				const float ry = winH ? 7.f / (float)winH : 0.01f;
				NL3D::CDRU::drawLine(q.x - rx, q.y, q.x, q.y + ry, *driver, col, NL3D::CViewport());
				NL3D::CDRU::drawLine(q.x, q.y + ry, q.x + rx, q.y, *driver, col, NL3D::CViewport());
				NL3D::CDRU::drawLine(q.x + rx, q.y, q.x, q.y - ry, *driver, col, NL3D::CViewport());
				NL3D::CDRU::drawLine(q.x, q.y - ry, q.x - rx, q.y, *driver, col, NL3D::CViewport());
			}
		}
	}
}

/* end of file */
