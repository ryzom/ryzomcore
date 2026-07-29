/**
 * \file patch_edit_ops.cpp
 * \brief Patch-edit state and operations: weld rebuild, pivot, transforms, write, selection.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The half of patch-edit mode that changes state: the welded/unwelded landscape rebuild
 * (`zpSyncLandscapeWeld` / `zpLiveZoneInfo`), the pivot modes, the transform kinds and
 * `zpApplyPatchXform` (one routine for move, rotate and scale), the gizmo drag lifecycle
 * (begin / update / end), the live-surface push, the geometry-changed fan-out over every
 * node of an object, and the vertex / edge / patch / tangent selection sets with their
 * aliasing and projection rules. Drawing and picking live in patch_edit_ui.cpp; the two
 * share the drag state through patch_edit_internal.h.
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

// Per-zone counts from the last zpLiveZoneInfo, accumulated by the weld rebuild for its
// report. Pixels cannot assert the unweld: see the comment in zpLiveZoneInfo.
static uint s_WeldDroppedBinds = 0, s_WeldDroppedBorder = 0;

/**
 * Turn a LIVE zone's retrieved info into the zone info to rebuild it from.
 *
 * Rebuilding a zone from the display cage alone reverts the terrain: `pz.Patches` carries the
 * geometry, but its tile records are the ones assembly loaded, while every tile, colour and
 * lumel painted since lives in the landscape zone. So the live zone is retrieved whole and
 * only the parts that are actually changing are replaced - the Bezier from the cage (full
 * float, and the authority on where the vertices are) and the bind/border data from the paint
 * zone, filtered by the weld state.
 *
 * The bind data cannot come from the retrieve either: it reflects how the zone is built RIGHT
 * NOW, so going from unwelded back to welded would find the cross-zone binds already gone.
 * `pz` holds the welded truth throughout and is filtered on the way out.
 */
static void zpLiveZoneInfo(NL3D::CZone *lz, const SPaintZone &pz, NL3D::CZoneInfo &out)
{
	lz->retrieve(out);
	for (size_t p = 0; p < out.Patchs.size() && p < pz.Patches.size(); ++p)
	{
		out.Patchs[p].Patch = pz.Patches[p].Patch;
		for (uint e = 0; e < 4; ++e)
		{
			out.Patchs[p].BindEdges[e] = pz.Patches[p].BindEdges[e];
			if (!g_WeldedLandscape && out.Patchs[p].BindEdges[e].NPatchs != 0
			    && out.Patchs[p].BindEdges[e].ZoneId != (uint16)pz.ZoneId)
				out.Patchs[p].BindEdges[e].NPatchs = 0;
		}
	}
	out.BorderVertices = g_WeldedLandscape ? pz.BorderVertices
	                                       : std::vector<NL3D::CBorderVertex>();
	// Counted so a gate can assert the unweld actually removed something. Pixels cannot say
	// this: in the welded build the shared CTessVertex ends up holding whichever patch
	// refreshed last, so how visible the alias is depends on refresh order rather than on
	// whether the weld is there.
	s_WeldDroppedBinds = 0;
	s_WeldDroppedBorder = 0;
	if (!g_WeldedLandscape)
	{
		for (size_t p = 0; p < pz.Patches.size(); ++p)
			for (uint e = 0; e < 4; ++e)
				if (pz.Patches[p].BindEdges[e].NPatchs != 0
				    && pz.Patches[p].BindEdges[e].ZoneId != (uint16)pz.ZoneId)
					++s_WeldDroppedBinds;
		s_WeldDroppedBorder = (uint)pz.BorderVertices.size();
	}
}

/**
 * Rebuild every landscape zone so the weld state matches g_WeldedLandscape.
 *
 * Retrieve ALL first, then remove all, then add all. Retrieving after the removes would read
 * deleted zones, and adding before every remove is done would bind against a zone that is
 * about to go. The remove/add split is the same two-phase order rebuildWorkingSet uses.
 *
 * The paint core is NOT re-initialised. It holds no CZone pointers - every access goes through
 * m_Landscape->getZone(id) - and its pristine carriers, dirty flags and undo stack are
 * untouched by how the display is built. That is what makes flipping modes cheap enough to
 * hang off a keypress rather than something the artist has to think about.
 */
bool zpSyncLandscapeWeld()
{
	if (!g_PaintCtx.Zones || !g_PaintCtx.Land)
		return true;
	NL3D::CLandscape &land = g_PaintCtx.Land->Landscape;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	bool ok = true;

	std::vector<NL3D::CZoneInfo> infos(zones.size());
	std::vector<bool> have(zones.size(), false);
	uint dropBinds = 0, dropBorder = 0;
	for (size_t i = 0; i < zones.size(); ++i)
	{
		NL3D::CZone *lz = land.getZone((sint)zones[i].ZoneId);
		if (!lz)
			continue;
		zpLiveZoneInfo(lz, zones[i], infos[i]);
		dropBinds += s_WeldDroppedBinds;
		dropBorder += s_WeldDroppedBorder;
		have[i] = true;
	}
	for (size_t i = 0; i < zones.size(); ++i)
		if (have[i])
			land.removeZone((uint16)zones[i].ZoneId);
	for (size_t i = 0; i < zones.size(); ++i)
	{
		if (!have[i])
			continue;
		NL3D::CZone zone;
		zone.build(infos[i]);
		NL3D::CZoneCornerSmoother cornerSmoother;
		std::vector<NL3D::CZone *> emptyVector;
		cornerSmoother.computeAllCornerSmoothFlags(&zone, emptyVector);
		if (!land.addZone(zone))
		{
			fprintf(stderr, "ERROR: weld rebuild: addZone failed for zone %u '%s'\n",
			        zones[i].ZoneId, zones[i].Name.c_str());
			ok = false;
		}
	}
	land.setRefineMode(true);
	if (g_verbose)
		printf("landscape rebuilt %s (%u cross-zone binds dropped, %u border verts dropped)\n",
		       g_WeldedLandscape ? "welded" : "unwelded", dropBinds, dropBorder);
	return ok;
}

// Gizmo drag state; see the drag section below.
bool s_Dragging = false;
int s_DragHandle = ZPGIZ_NONE;
static NLMISC::CVector s_DragPlaneN, s_DragPlaneP, s_DragStartHit;
NLMISC::CVector s_DragDelta;
static NLMISC::CVector s_DragAxis;
// The transform the current drag represents. Move keeps using s_DragDelta - a translation is
// the same everywhere and needs no anchor - while rotate and scale fill this in, because their
// preview offset differs per element and cannot be a single vector.
SPatchXform s_DragXform;
static NLMISC::CVector s_DragStartVec; // rotate: the ring vector the drag started on

/**
 * The offset this drag gives a point at `p`. Null when nothing is being dragged.
 *
 * The reason the preview and the commit cannot disagree: both run the SAME transform, this one
 * per displayed point and zpApplyPatchXform per written element.
 */
NLMISC::CVector zpDragOffsetAt(const NLMISC::CVector &p)
{
	if (!s_Dragging)
		return kNoOffset;
	if (s_DragXform.Kind == ZPXF_Move)
		return s_DragDelta;
	return zpTransformPoint(s_DragXform, p) - p;
}

/**
 * The object this node shows. Several nodes may return the SAME pointer: that is what makes
 * them nodes of one object rather than independent zones, and it is the identity every
 * per-object rule keys on - selection aliasing, geometry fan-out, the shared paint carrier.
 */
const void *zpZoneNode(uint zoneId)
{
	if (!g_PaintCtx.Zones)
		return NULL;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
		if (zones[z].ZoneId == zoneId)
			return (const void *)zones[z].Node;
	return NULL;
}

/** Linear part only: the image of a delta, with the matrix's translation dropped. */
NLMISC::CVector zpXformDelta(const NLMISC::CVector &d, const MAXMATH::Matrix3M &m)
{
	MAXMATH::Point3M zero = { 0.f, 0.f, 0.f };
	MAXMATH::Point3M p = { d.x, d.y, d.z };
	const MAXMATH::Point3M w0 = MAXMATH::transformPoint(zero, m);
	const MAXMATH::Point3M w1 = MAXMATH::transformPoint(p, m);
	return NLMISC::CVector(w1.x - w0.x, w1.y - w0.y, w1.z - w0.z);
}

/**
 * Are handles the thing being transformed right now?
 *
 * Handles are only visible while their corner is selected, so the corner selection has to
 * SURVIVE picking a handle - drop it and the handle you just picked disappears. That leaves
 * both sets non-empty at once, so one of them has to be the target, and it is the handles:
 * clicking a handle is the artist saying "this one now". The corner selection stays as the
 * context that shows them, as when the vertex stays selected while you drag its handle.
 */
bool zpHandleMode() { return !g_PatchTanSel.empty(); }

// ---------------------------------------------------------------------------------------------
// Gizmo drag (preview only; see zp_state.h)


bool zpPatchGizmoDragging() { return s_Dragging; }

/** Ray/plane intersection. False when the ray runs parallel to the plane. */
static bool zpRayPlane(const NLMISC::CVector &pos, const NLMISC::CVector &dir,
                       const NLMISC::CVector &planeP, const NLMISC::CVector &planeN,
                       NLMISC::CVector &hit)
{
	const float denom = dir * planeN;
	if (fabsf(denom) < 1e-6f)
		return false;
	const float t = ((planeP - pos) * planeN) / denom;
	if (t <= 0.f)
		return false; // behind the eye
	hit = pos + dir * t;
	return true;
}

const NLMISC::CVector &zpPatchVertDragOffset(uint zoneId, uint16 vertIdx)
{
	if (!s_Dragging)
		return kNoOffset;
	if (!g_PatchVertSel.count(TPatchVertId(zoneId, vertIdx)))
		return kNoOffset;
	return s_DragDelta;
}

bool zpPatchSelCentroid(NLMISC::CVector &out)
{
	if ((g_PatchVertSel.empty() && g_PatchTanSel.empty()) || !g_PaintCtx.Zones)
		return false;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	NLMISC::CVector sum = NLMISC::CVector::Null;
	uint n = 0;
	std::set<TPatchVertId> hit, hitVec;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &pz = zones[z];
		for (uint p = 0; p < pz.Patches.size(); ++p)
		{
			const NL3D::CPatchInfo &pi = pz.Patches[p];
			for (uint c = 0; c < 4 && !zpHandleMode(); ++c)
			{
				const TPatchVertId id(pz.ZoneId, pi.BaseVertices[c]);
				if (!g_PatchVertSel.count(id) || !hit.insert(id).second)
					continue; // once per unique vertex, not once per patch touching it
				sum += pi.Patch.Vertices[c];
				++n;
			}
			if (g_PatchTanSel.empty() || p >= pz.Ep.Pm.Patches.size())
				continue;
			const PIPELINE::MAX::NELPATCH::SPmPatch &pp = pz.Ep.Pm.Patches[p];
			for (uint j = 0; j < 8; ++j)
			{
				if (pp.Vec[j] < 0)
					continue;
				const TPatchVertId id(pz.ZoneId, (uint16)pp.Vec[j]);
				if (!g_PatchTanSel.count(id) || !hitVec.insert(id).second)
					continue; // and once per unique handle, for the same reason
				sum += pi.Patch.Tangents[j];
				++n;
			}
		}
	}
	if (!n)
		return false;
	out = sum / (float)n;
	return true;
}

// ---------------------------------------------------------------------------------------------
// Pivot point (the pivot-point control). See TPivotMode in zp_state.h.

// The "all objects" centre, held between interactions rather than recomputed per frame. That
// is the whole point of the mode: a centre that moved WHILE you dragged would make a rotate
// chase its own tail.
static NLMISC::CVector s_AllObjCentre = NLMISC::CVector::Null;
static bool s_AllObjValid = false;

/** Centre of the display geometry of every editable node, or of just the ones in `only`. */
static bool zpNodeCentre(const std::set<uint> *only, NLMISC::CVector &out)
{
	if (!g_PaintCtx.Zones)
		return false;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	NLMISC::CAABBox bb;
	bool init = false;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &pz = zones[z];
		if (!pz.Editable)
			continue;
		if (only && !only->count(pz.ZoneId))
			continue;
		for (uint p = 0; p < pz.Patches.size(); ++p)
			for (uint c = 0; c < 4; ++c)
			{
				// Bounding-box centre, not a vertex average: an average is pulled towards
				// whichever part of the object happens to be finely tessellated, which is not
				// what "the centre of this object" means to an artist.
				if (!init) { bb.setCenter(pz.Patches[p].Patch.Vertices[c]); bb.setHalfSize(NLMISC::CVector::Null); init = true; }
				else bb.extend(pz.Patches[p].Patch.Vertices[c]);
			}
	}
	if (!init)
		return false;
	out = bb.getCenter();
	return true;
}

void zpPivotNoteInteractionEnd()
{
	s_AllObjValid = false;
}

bool zpTransformPivot(NLMISC::CVector &out)
{
	switch (g_PivotMode)
	{
	case ZPPIV_World:
		out = NLMISC::CVector::Null;
		return true;
	case ZPPIV_User:
		if (!g_HaveUserPivot)
			return false;
		out = g_UserPivot;
		return true;
	case ZPPIV_AllObjects:
		if (!s_AllObjValid)
			s_AllObjValid = zpNodeCentre(NULL, s_AllObjCentre);
		if (!s_AllObjValid)
			return false;
		out = s_AllObjCentre;
		return true;
	case ZPPIV_SelObjects:
	{
		// The nodes the selection lives in, not the selected sub-objects: rotating a corner
		// about its own zone's centre is a different and useful thing from rotating it about
		// the corner cloud.
		std::set<uint> owners;
		for (std::set<TPatchVertId>::const_iterator it = g_PatchVertSel.begin();
		     it != g_PatchVertSel.end(); ++it)
			owners.insert(it->first);
		for (std::set<TPatchVertId>::const_iterator it = g_PatchTanSel.begin();
		     it != g_PatchTanSel.end(); ++it)
			owners.insert(it->first);
		if (owners.empty())
			return false;
		return zpNodeCentre(&owners, out);
	}
	default:
		break;
	}
	return zpPatchSelCentroid(out);
}

void zpSetPivotMode(int mode)
{
	if (mode < 0) mode = 0;
	if (mode >= ZPPIV_Count) mode = ZPPIV_Count - 1;
	if (mode == g_PivotMode)
		return;
	g_PivotMode = mode;
	s_AllObjValid = false;
	zpPatchGizmoInvalidate(); // the gizmo sits ON the pivot, so its depth just changed
	ZPSCRIPT::record(NLMISC::toString("painter.setPivotMode(%d)", mode));
}

bool zpSetUserPivotToSelection()
{
	NLMISC::CVector c;
	if (!zpPatchSelCentroid(c))
	{
		g_PropStatusMsg = "pivot: nothing selected";
		return false;
	}
	g_UserPivot = c;
	g_HaveUserPivot = true;
	zpPatchGizmoInvalidate();
	g_PropStatusMsg = NLMISC::toString("user pivot at %.2f, %.2f, %.2f", c.x, c.y, c.z);
	ZPSCRIPT::record("painter.setUserPivotToSelection()");
	return true;
}

/** Bridge form: the bridge's function pointers are all void-returning. */
void zpUserPivotToSelection() { zpSetUserPivotToSelection(); }

/** Flat form for the script TU, which cannot see NLMISC::CVector through this header set. */
bool zpTransformPivotXYZ(float outPos[3])
{
	NLMISC::CVector p;
	if (!zpTransformPivot(p))
		return false;
	outPos[0] = p.x; outPos[1] = p.y; outPos[2] = p.z;
	return true;
}

const char *zpPivotModeName(int mode)
{
	static const char *kNames[ZPPIV_Count] = { "SEL", "WORLD", "ALL OBJ", "SEL OBJ", "USER" };
	if (mode < 0 || mode >= ZPPIV_Count)
		return "SEL";
	return kNames[mode];
}

NLMISC::CVector zpTransformPoint(const SPatchXform &xf, const NLMISC::CVector &p)
{
	const NLMISC::CVector r = p - xf.Pivot;
	if (xf.Kind == ZPXF_Scale)
		return xf.Pivot + NLMISC::CVector(r.x * xf.Scale.x, r.y * xf.Scale.y, r.z * xf.Scale.z);
	if (xf.Kind != ZPXF_Rotate)
		return p;
	// Rodrigues, so the axis needs no basis built around it and any axis works - the screen
	// ring is not one of the three world axes.
	NLMISC::CVector a = xf.Axis;
	const float n = a.norm();
	if (n < 1e-8f)
		return p;
	a /= n;
	const float c = cosf(xf.Angle), s2 = sinf(xf.Angle);
	const NLMISC::CVector rot = r * c + (a ^ r) * s2 + a * ((a * r) * (1.f - c));
	return xf.Pivot + rot;
}

void zpSetXformKind(int kind)
{
	if (kind < ZPXF_Move) kind = ZPXF_Move;
	if (kind > ZPXF_Scale) kind = ZPXF_Scale;
	if (kind == g_XformKind)
		return;
	g_XformKind = kind;
	zpPatchGizmoInvalidate();
	ZPSCRIPT::record(NLMISC::toString("painter.setXformKind(%d)", kind));
}

const char *zpXformKindName(int kind)
{
	static const char *kNames[3] = { "MOVE", "ROTATE", "SCALE" };
	if (kind < 0 || kind > ZPXF_Scale)
		return "MOVE";
	return kNames[kind];
}

bool zpPatchGizmoBeginDrag(int handle, NL3D::CCamera *camera, const NL3D::CViewport &vp,
                           float mouseX, float mouseY)
{
	s_Dragging = false;
	s_DragDelta = NLMISC::CVector::Null;
	if (handle == ZPGIZ_NONE || !camera)
		return false;
	NLMISC::CVector o;
	if (!zpTransformPivot(o))
		return false;

	const NLMISC::CMatrix camMat = camera->getMatrix();
	const NLMISC::CVector fwd = camMat.getJ();
	NLMISC::CVector axisVec[3];
	for (int a = 0; a < 3; ++a)
	{
		axisVec[a] = NLMISC::CVector::Null;
		if (a == 0) axisVec[a].x = 1.f; else if (a == 1) axisVec[a].y = 1.f; else axisVec[a].z = 1.f;
	}

	s_DragXform = SPatchXform();
	s_DragXform.Pivot = o;
	s_DragXform.Kind = (TXformKind)g_XformKind;
	if (g_XformKind == ZPXF_Rotate)
	{
		// Rotate resolves against the RING'S OWN plane, not against a camera-facing one: the
		// angle is only meaningful in the plane the ring lies in. The screen handle turns
		// about the view direction, which is what a screen-aligned ring means.
		s_DragAxis = (handle >= ZPGIZ_AXIS_X && handle <= ZPGIZ_AXIS_Z)
			? axisVec[handle - ZPGIZ_AXIS_X] : fwd;
		s_DragXform.Axis = s_DragAxis;
		s_DragPlaneN = s_DragAxis;
		s_DragPlaneP = o;
		NLMISC::CVector pos, dir, hit;
		vp.getRayWithPoint(mouseX, mouseY, pos, dir, camMat, camera->getFrustum());
		if (!zpRayPlane(pos, dir, s_DragPlaneP, s_DragPlaneN, hit))
			return false;
		s_DragStartHit = hit;
		s_DragStartVec = hit - o;
		if (s_DragStartVec.norm() < 1e-5f)
			return false; // grabbed the exact centre: no angle to measure from
		s_Dragging = true;
		s_DragHandle = handle;
		return true;
	}

	s_DragAxis = NLMISC::CVector::Null;
	if (handle >= ZPGIZ_AXIS_X && handle <= ZPGIZ_AXIS_Z)
	{
		// Resolve against the plane that CONTAINS the axis and faces the camera most
		// squarely, then project onto the axis. The standard construction, and the one that
		// stays stable when you look nearly down the axis.
		s_DragAxis = axisVec[handle - ZPGIZ_AXIS_X];
		NLMISC::CVector n = s_DragAxis ^ (fwd ^ s_DragAxis);
		if (n.norm() < 1e-6f)
			n = fwd; // axis points at the camera: fall back to the view plane
		n.normalize();
		s_DragPlaneN = n;
	}
	else if (handle >= ZPGIZ_PLANE_XY && handle <= ZPGIZ_PLANE_ZX)
	{
		// The plane's own normal is the axis it does not span.
		const int q = handle - ZPGIZ_PLANE_XY;
		s_DragPlaneN = axisVec[kGizmoPlaneAxes[q][0]] ^ axisVec[kGizmoPlaneAxes[q][1]];
		s_DragPlaneN.normalize();
	}
	else // ZPGIZ_SCREEN: move parallel to the view plane
	{
		s_DragPlaneN = fwd;
	}
	s_DragPlaneP = o;

	NLMISC::CVector pos, dir;
	vp.getRayWithPoint(mouseX, mouseY, pos, dir, camMat, camera->getFrustum());
	if (!zpRayPlane(pos, dir, s_DragPlaneP, s_DragPlaneN, s_DragStartHit))
		return false;

	s_Dragging = true;
	s_DragHandle = handle;
	return true;
}

void zpPatchGizmoUpdateDrag(NL3D::CCamera *camera, const NL3D::CViewport &vp,
                            float mouseX, float mouseY)
{
	if (!s_Dragging || !camera)
		return;
	NLMISC::CVector pos, dir, hit;
	vp.getRayWithPoint(mouseX, mouseY, pos, dir, camera->getMatrix(), camera->getFrustum());
	if (!zpRayPlane(pos, dir, s_DragPlaneP, s_DragPlaneN, hit))
		return; // grazing the plane: keep the last good delta rather than jumping
	if (s_DragXform.Kind == ZPXF_Rotate)
	{
		// Signed angle between the start and current ring vectors, about the ring axis. The
		// cross product supplies the sign, which atan2 of the two lengths alone would not.
		const NLMISC::CVector v = hit - s_DragPlaneP;
		if (v.norm() < 1e-5f)
			return;
		const NLMISC::CVector a = s_DragStartVec.normed(), b = v.normed();
		const float c = a * b;
		const float sgn = ((a ^ b) * s_DragAxis) < 0.f ? -1.f : 1.f;
		s_DragXform.Angle = sgn * acosf(c < -1.f ? -1.f : (c > 1.f ? 1.f : c));
		if (g_PatchLiveUpdate && !zpPatchPushLive(true))
			g_PropStatusMsg = "patch transform: the live surface could not be updated";
		return;
	}
	NLMISC::CVector delta = hit - s_DragStartHit;
	if (s_DragHandle >= ZPGIZ_AXIS_X && s_DragHandle <= ZPGIZ_AXIS_Z)
		delta = s_DragAxis * (delta * s_DragAxis); // constrain to the axis
	s_DragDelta = delta;
	if (s_DragXform.Kind == ZPXF_Scale)
	{
		// One gizmo length of drag doubles the size. Tying the factor to the gizmo rather than
		// to world units keeps the feel the same at every zoom, since the gizmo is itself
		// fitted to a pixel size.
		const float len = s_GizmoWorldLen > 1e-6f ? s_GizmoWorldLen : 1.f;
		float f[3] = { 1.f, 1.f, 1.f };
		if (s_DragHandle >= ZPGIZ_AXIS_X && s_DragHandle <= ZPGIZ_AXIS_Z)
		{
			const int a = s_DragHandle - ZPGIZ_AXIS_X;
			f[a] = 1.f + (delta * s_DragAxis) / len;
		}
		else if (s_DragHandle >= ZPGIZ_PLANE_XY && s_DragHandle <= ZPGIZ_PLANE_ZX)
		{
			const int q = s_DragHandle - ZPGIZ_PLANE_XY;
			const float k = 1.f + delta.norm() * (delta * NLMISC::CVector(1.f, 1.f, 1.f) < 0.f ? -1.f : 1.f) / len;
			f[kGizmoPlaneAxes[q][0]] = k;
			f[kGizmoPlaneAxes[q][1]] = k;
		}
		else
		{
			// Screen handle: uniform. Distance from the start, signed by which way it went.
			const float k = 1.f + delta.norm() * (delta * NLMISC::CVector(1.f, 1.f, 1.f) < 0.f ? -1.f : 1.f) / len;
			f[0] = f[1] = f[2] = k;
		}
		// A factor through zero mirrors the selection and then keeps going; clamp so a scale
		// can shrink to nothing but never turn the surface inside out mid-drag.
		for (int i = 0; i < 3; ++i)
			if (f[i] < 0.01f) f[i] = 0.01f;
		s_DragXform.Scale = NLMISC::CVector(f[0], f[1], f[2]);
	}
	if (g_PatchLiveUpdate && !zpPatchPushLive(true))
		g_PropStatusMsg = "patch move: the live surface could not be updated";
}

/**
 * Push edited geometry into the LIVE landscape, mirroring what the paint path does for tiles.
 *
 * The landscape is not rebuilt. CZone::refreshTesselationGeometry re-derives every
 * tessellation vertex from computeVertex, which evaluates the patch's Bezier - so writing the
 * patch's control points and refreshing is enough, and it costs no undo (a rebuild clears it).
 *
 * The control points are CVector3s: 16-bit fixed point relative to the zone's PatchBias and
 * PatchScale, where PatchScale = maxHalfSize / 32760. Two consequences the caller has to live
 * with. Quantization is one LSB per PatchScale - a few millimetres on a normal zone - so the
 * live surface sits on a slightly coarser lattice than the .max, which keeps full float
 * precision. And pack() CLAMPS: a control point dragged outside the bbox computed at build()
 * time would silently stop moving, so that case is detected and reported instead, because
 * only a rebuild recomputes the bbox.
 */
bool zpPatchPushLive(bool preview)
{
	if (!g_PaintCtx.Zones || !g_PaintCtx.Land)
		return true;
	NL3D::CLandscape &land = g_PaintCtx.Land->Landscape;
	std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	std::set<std::pair<uint, uint> > touched; // (zone id, patch) pushed this pass
	std::set<uint> outOfRange; // zones whose geometry no longer fits their packed range
	bool ok = true;

	// The selection, re-keyed from (node, vertex) onto (OBJECT, vertex) - the identity of the
	// thing that actually moved. Built once rather than per patch corner.
	std::set<std::pair<const void *, uint16> > objectSel, objectTanSel;
	for (std::set<TPatchVertId>::const_iterator it = g_PatchVertSel.begin();
	     it != g_PatchVertSel.end(); ++it)
	{
		const void *obj = zpZoneNode(it->first);
		if (obj)
			objectSel.insert(std::make_pair(obj, it->second));
	}
	for (std::set<TPatchVertId>::const_iterator it = g_PatchTanSel.begin();
	     it != g_PatchTanSel.end(); ++it)
	{
		const void *obj = zpZoneNode(it->first);
		if (obj)
			objectTanSel.insert(std::make_pair(obj, it->second));
	}

	for (uint z = 0; z < zones.size(); ++z)
	{
		SPaintZone &pz = zones[z];
		if (!pz.Editable)
			continue;
		NL3D::CZone *lz = land.getZone((sint)pz.ZoneId);
		if (!lz)
			continue;

		for (uint p = 0; p < pz.Patches.size() && p < (uint)lz->getNumPatchs(); ++p)
		{
			const NL3D::CPatchInfo &pi = pz.Patches[p];
			// Only patches with a selected corner: everything else is unchanged, and
			// refreshing them would be pure cost. Selected through ANY node of this node's
			// object, not just through this one - a sibling node's surface moved too.
			NLMISC::CVector off[4];
			bool any = false;
			for (uint c = 0; c < 4; ++c)
			{
				off[c] = preview ? zpVertOffset(pz, pi.BaseVertices[c]) : kNoOffset;
				if (objectSel.count(std::make_pair((const void *)pz.Node, pi.BaseVertices[c])))
					any = true;
			}
			// A handle can be selected without its corner, and moving it reshapes this patch,
			// so handles count towards "something here moved" as much as the corners do.
			if (!any && !objectTanSel.empty() && p < pz.Ep.Pm.Patches.size())
				for (uint j = 0; j < 8 && !any; ++j)
					if (pz.Ep.Pm.Patches[p].Vec[j] >= 0
					    && objectTanSel.count(std::make_pair((const void *)pz.Node,
					                                         (uint16)pz.Ep.Pm.Patches[p].Vec[j])))
						any = true;
			if (!any)
				continue;

			// Rebuild this patch's control cage with the corner offsets applied, tangents
			// riding their corner exactly as the preview draws them.
			NL3D::CBezierPatch bp = pi.Patch;
			for (uint c = 0; c < 4; ++c)
				bp.Vertices[c] += off[c];
			for (uint j = 0; j < 8; ++j)
			{
				// Each handle takes zpTanOffset, which is the corner's offset while it rides
				// and its own while it is dragged - so a handle drag reshapes the live surface
				// exactly as the cage shows it.
				if (preview && p < pz.Ep.Pm.Patches.size() && pz.Ep.Pm.Patches[p].Vec[j] >= 0)
					bp.Tangents[j] += zpTanOffset(pz, (uint16)pz.Ep.Pm.Patches[p].Vec[j]);
				else
					bp.Tangents[j] += off[(j & 1) ? (((j >> 1) + 1) & 3) : (j >> 1)];
			}

			// CZone owns the packing rules and refuses out-of-range writes wholesale, so
			// a partial write is impossible and the tool needs no copy of PatchBias/Scale.
			if (!lz->setPatchGeometry((sint)p, bp))
			{
				// Report which zone and which control point, so a real out-of-range move is
				// distinguishable from a frame mismatch without a rebuild of the tool.
				if (g_verbose)
					fprintf(stderr, "patch push: zone %u patch %u outside packed range "
					        "(zone bbox centre %.1f,%.1f,%.1f half %.1f,%.1f,%.1f)\n",
					        pz.ZoneId, p,
					        lz->getZoneBB().getCenter().x, lz->getZoneBB().getCenter().y,
					        lz->getZoneBB().getCenter().z,
					        lz->getZoneBB().getHalfSize().x, lz->getZoneBB().getHalfSize().y,
					        lz->getZoneBB().getHalfSize().z);
				outOfRange.insert(pz.ZoneId);
				continue;
			}
			touched.insert(std::make_pair(pz.ZoneId, p));
		}
	}

	// A zone whose geometry no longer fits the bbox chosen at build() time cannot be written
	// in place: PatchBias/PatchScale were derived from that bbox and pack() CLAMPS, so the
	// vertex would silently stop moving partway. Rebuilding the zone recomputes both from the
	// geometry as it now is, which is the only thing that makes the move representable.
	//
	// Commit only. During a drag the preview offsets live in a temporary cage while
	// pz.Patches still holds the un-moved geometry, so a rebuild would faithfully rebuild the
	// OLD shape - and pay for it every frame. The surface simply lags past the range boundary
	// until the drag is released, which is when the commit rebuilds once.
	std::set<std::pair<uint, uint> > refresh = touched;
	if (!preview)
	{
		for (std::set<uint>::const_iterator it = outOfRange.begin(); it != outOfRange.end(); ++it)
		{
			const SPaintZone *pz = zpFindPaintZone(*it);
			if (!pz)
			{
				ok = false;
				continue;
			}
			// Collect the seam neighbours BEFORE the remove: removeZone unbinds them, so
			// after the re-add their patches along the seam have to re-tessellate against
			// geometry that moved. Reading them afterwards would find the binds already gone.
			const NL3D::CZone *clz = land.getZone((sint)*it);
			for (uint p = 0; clz && p < (uint)clz->getNumPatchs(); ++p)
			{
				const NL3D::CPatch *lp = clz->getPatch((sint)p);
				if (!lp)
					continue;
				for (uint edge = 0; edge < 4; ++edge)
				{
					NL3D::CPatch::CBindInfo nb;
					lp->getBindNeighbor(edge, nb);
					if (!nb.Zone || (uint)nb.Zone->getZoneId() == *it)
						continue;
					for (uint i = 0; i < (uint)nb.NPatchs; ++i)
						if (nb.Next[i])
							refresh.insert(std::make_pair((uint)nb.Zone->getZoneId(),
							                              (uint)nb.Next[i]->getPatchId()));
				}
			}
			// The rebuilt zone's own patches are freshly compiled, so they are NOT added to
			// the refresh set - and must not be, since the pointers this pass collected for
			// that zone die with it.
			for (std::set<std::pair<uint, uint> >::iterator r = refresh.begin(); r != refresh.end();)
			{
				if (r->first == *it) refresh.erase(r++);
				else ++r;
			}
			for (std::set<std::pair<uint, uint> >::iterator t = touched.begin(); t != touched.end();)
			{
				if (t->first == *it) touched.erase(t++);
				else ++t;
			}
			// Same construction the weld rebuild uses: keep everything the live zone
			// accumulated, take the geometry from the cage. Building from the cage alone
			// would fix the geometry and revert every painted tile.
			NL3D::CZoneInfo zi;
			NL3D::CZone *mlz = land.getZone((sint)*it);
			if (!mlz)
			{
				ok = false;
				continue;
			}
			zpLiveZoneInfo(mlz, *pz, zi);
			NL3D::CZone zone;
			zone.build(zi);
			NL3D::CZoneCornerSmoother cornerSmoother;
			std::vector<NL3D::CZone *> emptyVector;
			cornerSmoother.computeAllCornerSmoothFlags(&zone, emptyVector);
			land.removeZone((uint16)*it);
			if (!land.addZone(zone))
			{
				fprintf(stderr, "ERROR: zone %u could not be re-added after an out-of-range "
				        "patch move; its surface is now missing\n", *it);
				ok = false;
				continue;
			}
			if (g_verbose)
				printf("patch push: zone %u rebuilt (move left the packed range)\n", *it);
		}
	}

	// Refresh the pushed patches and their bind neighbours, the same neighbour walk
	// applyChanges does - a moved corner is shared, so the patch on the other side of the
	// seam has to re-tessellate too or the surface cracks along it.
	for (std::set<std::pair<uint, uint> >::const_iterator it = touched.begin(); it != touched.end(); ++it)
	{
		// const, so the public const getPatch overload is the one chosen - this walk only
		// reads bind info.
		const NL3D::CZone *lz = land.getZone((sint)it->first);
		if (!lz)
			continue;
		const NL3D::CPatch *lp = lz->getPatch((sint)it->second);
		if (!lp)
			continue;
		for (uint edge = 0; edge < 4; ++edge)
		{
			NL3D::CPatch::CBindInfo nb;
			lp->getBindNeighbor(edge, nb);
			if (!nb.Zone)
				continue;
			for (uint i = 0; i < (uint)nb.NPatchs; ++i)
				if (nb.Next[i])
					refresh.insert(std::make_pair((uint)nb.Zone->getZoneId(), (uint)nb.Next[i]->getPatchId()));
		}
	}
	for (std::set<std::pair<uint, uint> >::const_iterator it = refresh.begin(); it != refresh.end(); ++it)
	{
		NL3D::CZone *lz = land.getZone((sint)it->first);
		if (lz)
			lz->refreshTesselationGeometry((sint)it->second);
	}
	return ok;
}

/**
 * Core -> display. Every position change comes through here, forward or by undo/redo, so the
 * cage cannot drift from what is actually stored. The object position is authoritative; the
 * world position is re-derived rather than accumulated, which is what keeps repeated undos
 * from creeping.
 */
/**
 * A vertex moved in the .max: shift the display cage to match.
 *
 * The payload is an object-space DELTA (see setGeomChangedCb) and is applied as a shift, never
 * as an absolute placement. That is what lets one routine serve all three write targets and
 * every node, however it is placed on the board.
 */
void zpGeomVertChanged(uint zoneId, uint16 elemIdx, int elem, const float *objDelta)
{
	if (!g_PaintCtx.Zones)
		return;
	std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	// The write landed in an OBJECT, so every node showing that object moved - not just the
	// one the edit was addressed to. Sibling nodes would otherwise draw a cage over geometry
	// that no longer matches, and the mismatch survives until the working set is rebuilt.
	const void *object = zpZoneNode(zoneId);
	if (!object)
		return;
	for (uint z = 0; z < zones.size(); ++z)
	{
		SPaintZone &pz = zones[z];
		if ((const void *)pz.Node != object)
			continue;
		if (elem == ZPPAINT::GeomVec)
		{
			if (elemIdx < pz.Ep.Pm.Vecs.size())
				for (int k = 0; k < 3; ++k)
					pz.Ep.Pm.Vecs[elemIdx].Pos[k] += objDelta[k];
		}
		else if (elemIdx < pz.Ep.Pm.Verts.size())
		{
			for (int k = 0; k < 3; ++k)
				pz.Ep.Pm.Verts[elemIdx].Pos[k] += objDelta[k];
		}
		// Object delta -> displayed-world delta through the node's FULL transform: the
		// difference of two transformed points, which drops the translation and so works for a
		// rotated or mirrored node without a special case. DisplayTM, not ObjectTM - a placed
		// file's cage is drawn a board cell away from where the file authored it.
		MAXMATH::Point3M zero = { 0.f, 0.f, 0.f };
		MAXMATH::Point3M od = { objDelta[0], objDelta[1], objDelta[2] };
		const MAXMATH::Point3M w0 = MAXMATH::transformPoint(zero, pz.DisplayTM);
		const MAXMATH::Point3M w1 = MAXMATH::transformPoint(od, pz.DisplayTM);
		const NLMISC::CVector shift(w1.x - w0.x, w1.y - w0.y, w1.z - w0.z);
		for (uint p = 0; p < pz.Patches.size(); ++p)
		{
			NL3D::CPatchInfo &pi = pz.Patches[p];
			if (elem == ZPPAINT::GeomVec)
			{
				// Vecs carry BOTH tangents and interiors, and one index can be reached from
				// several patch slots, so every slot that names it moves.
				if (p >= pz.Ep.Pm.Patches.size())
					continue;
				const PIPELINE::MAX::NELPATCH::SPmPatch &pp = pz.Ep.Pm.Patches[p];
				for (uint j = 0; j < 8; ++j)
					if (pp.Vec[j] >= 0 && (uint16)pp.Vec[j] == elemIdx)
						pi.Patch.Tangents[j] += shift;
				for (uint j = 0; j < 4; ++j)
					if (pp.Interior[j] >= 0 && (uint16)pp.Interior[j] == elemIdx)
						pi.Patch.Interiors[j] += shift;
				continue;
			}
			for (uint c = 0; c < 4; ++c)
			{
				if (pi.BaseVertices[c] != elemIdx)
					continue;
				// Tangents are stored as absolute points, so they take the same shift the
				// corner took rather than being recomputed.
				pi.Patch.Vertices[c] += shift;
				pi.Patch.Tangents[c * 2] += shift;
				pi.Patch.Tangents[((c + 3) & 3) * 2 + 1] += shift;
			}
		}
	}
}

uint zpApplyPatchMove(const NLMISC::CVector &worldDelta, std::string &msg)
{
	SPatchXform xf;
	return zpApplyPatchXform(xf, worldDelta, msg);
}

/** Rotate the selection about the current pivot. Axis 0/1/2 = X/Y/Z, angle in degrees. */
uint zpApplyPatchRotate(int axis, float degrees, std::string &msg)
{
	SPatchXform xf;
	xf.Kind = ZPXF_Rotate;
	if (!zpTransformPivot(xf.Pivot))
	{
		msg = "rotate: no pivot";
		return 0;
	}
	xf.Axis = NLMISC::CVector(axis == 0 ? 1.f : 0.f, axis == 1 ? 1.f : 0.f, axis == 2 ? 1.f : 0.f);
	xf.Angle = degrees * (float)NLMISC::Pi / 180.f;
	return zpApplyPatchXform(xf, NLMISC::CVector::Null, msg);
}

/** Rotate about an ARBITRARY axis - the form the gizmo records, since a screen ring is not
 * one of the three world axes. */
uint zpApplyPatchRotateAxis(float ax, float ay, float az, float degrees, std::string &msg)
{
	SPatchXform xf;
	xf.Kind = ZPXF_Rotate;
	if (!zpTransformPivot(xf.Pivot))
	{
		msg = "rotate: no pivot";
		return 0;
	}
	xf.Axis = NLMISC::CVector(ax, ay, az);
	xf.Angle = degrees * (float)NLMISC::Pi / 180.f;
	return zpApplyPatchXform(xf, NLMISC::CVector::Null, msg);
}

/** Scale the selection about the current pivot, per axis. */
uint zpApplyPatchScale(float sx, float sy, float sz, std::string &msg)
{
	SPatchXform xf;
	xf.Kind = ZPXF_Scale;
	if (!zpTransformPivot(xf.Pivot))
	{
		msg = "scale: no pivot";
		return 0;
	}
	xf.Scale = NLMISC::CVector(sx, sy, sz);
	return zpApplyPatchXform(xf, NLMISC::CVector::Null, msg);
}

/**
 * Apply a transform to the selection.
 *
 * One routine for move, rotate and scale because everything but the per-element world delta is
 * shared: which elements are eligible, the ride and bound rules, the node-space conversion,
 * the single undo stroke, the live push. A move hands every element the same delta; a rotate
 * or scale derives each element's from where it sits relative to the pivot, which is why the
 * core op takes a delta PER element.
 */
uint zpApplyPatchXform(const SPatchXform &xform, const NLMISC::CVector &worldDelta,
                       std::string &msg)
{
	msg.clear();
	if (!g_PaintCtx.Zones || g_PatchVertSel.empty())
		return 0;
	std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	uint written = 0, skippedBound = 0, skippedRiding = 0;
	uint nBase = 0, nModPm = 0, nDelta = 0; // which write target each vertex resolved to
	std::string firstErr;

	for (uint z = 0; z < zones.size(); ++z)
	{
		SPaintZone &pz = zones[z];
		if (!pz.Editable)
			continue;

		// Collect this zone's selected corners once; a corner is reached from up to four
		// patches and must be written exactly once.
		std::set<uint16> want;
		for (uint p = 0; p < pz.Patches.size() && !zpHandleMode(); ++p)
			for (uint c = 0; c < 4; ++c)
				if (g_PatchVertSel.count(TPatchVertId(pz.ZoneId, pz.Patches[p].BaseVertices[c])))
					want.insert(pz.Patches[p].BaseVertices[c]);
		// ... and its selected handles, keyed on the Vecs index the same way.
		std::set<uint16> wantVec;
		for (std::set<TPatchVertId>::const_iterator it = g_PatchTanSel.begin();
		     it != g_PatchTanSel.end(); ++it)
			if (it->first == pz.ZoneId)
				wantVec.insert(it->second);
		if (want.empty() && wantVec.empty())
			continue;

		// Displayed-world delta -> object delta, through the FULL node transform: the drag is
		// measured in the space the cage is drawn in, which is object space only for a node
		// sitting at the board origin untransformed. zpXformDelta takes the difference of two
		// transformed points, which drops the translation, so a rotated or mirrored node needs
		// no special case - its rotation is already in DisplayTM.
		const MAXMATH::Matrix3M inv = MAXMATH::inverseM3(pz.DisplayTM);

		// Bound vertices are derived from a target edge; they are recomputed on load, so a
		// written position could not survive the round trip. Filtered HERE rather than in the
		// core op, which trusts its caller to have applied the policy.
		std::vector<ZPPAINT::SGeomElemRef> move;
		move.reserve(want.size() + wantVec.size());
		for (std::set<uint16>::const_iterator it = want.begin(); it != want.end(); ++it)
		{
			if (*it < pz.Ep.Rp.Verts.size() && pz.Ep.Rp.Verts[*it].Binded)
				++skippedBound;
			else
				move.push_back(ZPPAINT::SGeomElemRef(*it, ZPPAINT::GeomVert));
		}
		// A handle whose corner is moving RIDES it - the corner's move already shifted it, so
		// writing it here too would move it twice. Same rule as a bound vertex, and the same
		// place: the core trusts its caller.
		for (std::set<uint16>::const_iterator it = wantVec.begin(); it != wantVec.end(); ++it)
		{
			const uint16 owner = zpTangentOwner(pz, *it);
			if (owner != (uint16)0xffff && want.count(owner))
			{
				++skippedRiding;
				continue;
			}
			// A handle of a BOUND corner is derived along with it, for the same reason.
			if (owner != (uint16)0xffff && owner < pz.Ep.Rp.Verts.size()
			    && pz.Ep.Rp.Verts[owner].Binded)
			{
				++skippedBound;
				continue;
			}
			move.push_back(ZPPAINT::SGeomElemRef(*it, ZPPAINT::GeomVec));
		}
		if (move.empty())
			continue;

		// Count the targets before the write, for the gate's log line - the core op does not
		// report them and does not need to.
		for (size_t i = 0; i < move.size(); ++i)
		{
			ZPPAINT::SGeomWriteTarget t;
			std::string e;
			if (!ZPPAINT::resolveGeomWriteTarget(pz.Node, move[i].Idx, move[i].Elem, t, e))
				continue;
			if (t.Kind == ZPPAINT::SGeomWriteTarget::BasePatchMesh) ++nBase;
			else if (t.Kind == ZPPAINT::SGeomWriteTarget::ModifierPatchMesh) ++nModPm;
			else ++nDelta;
		}

		// The core owns the write, the undo record and the dirty flag; the display follows
		// through the geom-changed callback, so undo and redo update the cage for free.
		std::string err;
		if (!g_PaintCtx.Core)
		{
			if (firstErr.empty()) firstErr = "no paint core";
			continue;
		}
		// One world delta per element. A move gives every element the same one; a rotate or a
		// scale gives each its own, derived from where it sits relative to the pivot.
		std::vector<NLMISC::CVector> deltas;
		deltas.reserve(move.size());
		for (size_t i = 0; i < move.size(); ++i)
		{
			NLMISC::CVector wd = worldDelta;
			if (xform.Kind != ZPXF_Move)
			{
				float wp[3];
				const bool have = move[i].Elem == ZPPAINT::GeomVec
					? zpPatchTangentWorld(pz.ZoneId, move[i].Idx, wp)
					: zpPatchVertWorld(pz.ZoneId, move[i].Idx, wp);
				if (!have)
				{
					deltas.push_back(NLMISC::CVector::Null);
					continue;
				}
				wd = zpTransformPoint(xform, NLMISC::CVector(wp[0], wp[1], wp[2]))
				     - NLMISC::CVector(wp[0], wp[1], wp[2]);
			}
			deltas.push_back(zpXformDelta(wd, inv));
		}
		const uint n = g_PaintCtx.Core->opMovePatchElems(pz.ZoneId, move, deltas, err);
		if (!n && firstErr.empty() && !err.empty())
			firstErr = err;
		written += n;
	}

	// Push the committed positions into the live surface HERE rather than at the drag's
	// release: a scripted move (painter.movePatchSelection) never goes through the drag path,
	// and would otherwise write the file correctly while leaving the landscape stale.
	// Unconditional, whatever PatchLiveUpdate says - that option governs per-FRAME mirroring
	// during a drag, not whether a committed edit reaches the surface at all.
	if (written && !zpPatchPushLive(false))
		msg = "patch move: the live surface could not be updated";

	if (!written)
	{
		if (skippedBound)
			msg = "patch move: bound vertices follow their edge, nothing to write";
		else if (!firstErr.empty())
			msg = "patch move: " + firstErr;
	}
	else if (skippedBound && msg.empty())
	{
		msg = NLMISC::toString("patch move: %u moved (base %u, modPM %u, delta %u), %u bound skipped",
		                       written, nBase, nModPm, nDelta, skippedBound);
	}
	else if (msg.empty())
	{
		msg = NLMISC::toString("patch move: %u vertices (base %u, modPM %u, delta %u)",
		                       written, nBase, nModPm, nDelta);
	}
	return written;
}

void zpPatchGizmoEndDrag()
{
	if (!s_Dragging)
		return;
	const NLMISC::CVector delta = s_DragDelta;
	const SPatchXform xf = s_DragXform;
	const bool moved = xf.Kind == ZPXF_Move
		? delta.norm() > 0.0001f
		: (xf.Kind == ZPXF_Rotate ? fabsf(xf.Angle) > 1e-4f
		                          : (xf.Scale - NLMISC::CVector(1.f, 1.f, 1.f)).norm() > 1e-4f);
	s_Dragging = false;
	s_DragHandle = ZPGIZ_NONE;
	s_DragDelta = NLMISC::CVector::Null;
	s_DragXform = SPatchXform();
	zpPatchGizmoInvalidate();
	// "Centre of all objects" is deliberately held DURING an interaction and re-fitted after
	// it, so a rotate does not chase a centre the rotate itself is moving.
	zpPivotNoteInteractionEnd();
	if (!moved)
		return;
	std::string msg;
	zpApplyPatchXform(xf, delta, msg);
	g_PropStatusMsg = msg;
	// Recorded as the op the artist performed, not as the drag that produced it - a replay
	// re-derives the pivot from the mode, which is what makes the script survive a different
	// pivot setting rather than baking one drag's anchor into it.
	if (xf.Kind == ZPXF_Rotate)
		ZPSCRIPT::record(NLMISC::toString("painter.rotatePatchSelectionAxis(%.9g, %.9g, %.9g, %.9g)",
		                                  xf.Axis.x, xf.Axis.y, xf.Axis.z,
		                                  xf.Angle * 180.f / (float)NLMISC::Pi));
	else if (xf.Kind == ZPXF_Scale)
		ZPSCRIPT::record(NLMISC::toString("painter.scalePatchSelection(%.9g, %.9g, %.9g)",
		                                  xf.Scale.x, xf.Scale.y, xf.Scale.z));
	else
		ZPSCRIPT::record(NLMISC::toString("painter.movePatchSelection(%.9g, %.9g, %.9g)",
		                                  delta.x, delta.y, delta.z));
}

/**
 * Abandon a live gizmo drag - right-click-while-dragging, and ESC.
 *
 * Nothing was committed: the drag is a preview, offsets drawn over an untouched cage, so
 * cancelling is dropping the drag state. The one thing that HAS already changed is the live
 * landscape - the per-frame live update pushed preview positions into the zone's control
 * points - so the untouched cage is pushed back over it and the surface returns to where the
 * file still says it is. No core op ran, so undo, dirty flags and the recorder see nothing.
 */
void zpPatchGizmoCancelDrag()
{
	if (!s_Dragging)
		return;
	s_Dragging = false;
	s_DragHandle = ZPGIZ_NONE;
	s_DragDelta = NLMISC::CVector::Null;
	s_DragXform = SPatchXform();
	zpPatchGizmoInvalidate();
	zpPivotNoteInteractionEnd();
	if (g_PatchLiveUpdate)
		zpPatchPushLive(false);
	g_PropStatusMsg = "transform cancelled";
}

/**
 * Is this vertex already selected through a DIFFERENT zone backed by the same object?
 *
 * cloneInstanceZone copies the SPaintZone wholesale, so an instance shares its source's Node
 * and therefore its storage: (node, vertex) is the identity of the thing being moved, while
 * (zone, vertex) is only how it was pointed at. Selecting one underlying vertex through two
 * instances would apply the drag to that single storage location twice - and because each
 * instance carries its own display transform, the two object-space deltas would not even
 * agree. There is no correct answer to give, so the selection is refused instead.
 *
 * DIFFERENT vertices of the same object through different instances share nothing and are
 * explicitly allowed: authoring an edge from whichever instance shows it best is the point.
 */
static bool zpVertAliased(uint zoneId, uint vertIdx, uint &otherZone)
{
	const void *node = zpZoneNode(zoneId);
	if (!node)
		return false;
	for (std::set<TPatchVertId>::const_iterator it = g_PatchVertSel.begin();
	     it != g_PatchVertSel.end(); ++it)
	{
		if (it->first == zoneId || it->second != (uint16)vertIdx)
			continue;
		if (zpZoneNode(it->first) == node)
		{
			otherZone = it->first;
			return true;
		}
	}
	return false;
}

/**
 * Drop selected handles whose corner is no longer selected.
 *
 * Handles are only drawn for selected corners, so one left behind by a corner deselect would
 * still move on the next drag while being invisible. Selections you cannot see are traps.
 */
static void zpDropOrphanedTangents()
{
	if (g_PatchTanSel.empty() || !g_PaintCtx.Zones)
		return;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	std::set<TPatchVertId> keep;
	for (std::set<TPatchVertId>::const_iterator it = g_PatchTanSel.begin();
	     it != g_PatchTanSel.end(); ++it)
	{
		for (uint z = 0; z < zones.size(); ++z)
		{
			if (zones[z].ZoneId != it->first)
				continue;
			const uint16 owner = zpTangentOwner(zones[z], it->second);
			if (owner != (uint16)0xffff && g_PatchVertSel.count(TPatchVertId(it->first, owner)))
				keep.insert(*it);
			break;
		}
	}
	g_PatchTanSel.swap(keep);
}

uint16 zpTangentOwner(const SPaintZone &pz, uint16 vecIdx)
{
	for (size_t p = 0; p < pz.Ep.Pm.Patches.size() && p < pz.Patches.size(); ++p)
	{
		const PIPELINE::MAX::NELPATCH::SPmPatch &pp = pz.Ep.Pm.Patches[p];
		for (uint j = 0; j < 8; ++j)
		{
			if (pp.Vec[j] < 0 || (uint16)pp.Vec[j] != vecIdx)
				continue;
			// Tangent 2e belongs to corner e, 2e+1 to corner (e+1)&3.
			const uint corner = (j & 1) ? (((j >> 1) + 1) & 3) : (j >> 1);
			return pz.Patches[p].BaseVertices[corner];
		}
	}
	return (uint16)0xffff;
}

void zpPatchVertSelect(uint zoneId, uint vertIdx, int op)
{
	const TPatchVertId id((uint)zoneId, (uint16)vertIdx);
	if (op != 2) // a removal can never create an alias
	{
		uint other = 0;
		if (zpVertAliased(zoneId, vertIdx, other))
		{
			// Replace still clears first, so the alias only blocks an ADD; a plain click
			// re-selecting through another instance is a legitimate change of viewpoint.
			if (op == 1)
			{
				g_PropStatusMsg = NLMISC::toString(
					"vertex %u already selected via zone %u (same object)", vertIdx, other);
				return;
			}
		}
	}
	// A click selects what was clicked, and nothing else. Welded partners are NOT dragged in:
	// in patch mode the landscape is built apart (see buildDisplayZone), so a seam is two
	// vertices in two files and the artist can see it. Propagating across a session's welds
	// would also have made the same edit mean different things depending on which files
	// happened to be open, which is not a property an authoring tool should have.
	if (op == 0)
	{
		g_PatchVertSel.clear();
		// Handles are shown for selected corners only, so a handle whose corner just left the
		// selection is no longer visible - and a selection you cannot see is a trap.
		g_PatchTanSel.clear();
	}
	if (op == 2)
	{
		g_PatchVertSel.erase(id);
		zpDropOrphanedTangents();
	}
	else
	{
		g_PatchVertSel.insert(id);
	}
	zpPatchGizmoInvalidate(); // the centroid moved, so its depth did, so the fit is stale
	ZPSCRIPT::record(NLMISC::toString("painter.selectPatchVertex(%u, %u, %d)", zoneId, vertIdx, op));
}

/**
 * Project the current level's selection onto the vertex set the move machinery consumes.
 *
 * Edge and patch levels move VERTICES - edge move is its two corners, patch move is
 * its four - so the whole gizmo/preview/write chain works unchanged and only the thing
 * being pointed at differs. The level's own set stays the authority and this is recomputed
 * from scratch every time, which is what makes removing one edge of a pair that shared a
 * corner leave that corner selected rather than dropping it.
 */
void zpRebuildVertSelFromSubObject()
{
	if (!g_PaintCtx.Paint)
		return;
	const int level = g_PaintCtx.Paint->SubObj;
	if (level != CPaintMouseListener::SubEdge && level != CPaintMouseListener::SubPatch)
		return;
	g_PatchVertSel.clear();
	std::set<TPatchVertId> want;
	if (level == CPaintMouseListener::SubEdge)
	{
		for (std::set<SPatchEdgeId>::const_iterator it = g_PatchEdgeSel.begin();
		     it != g_PatchEdgeSel.end(); ++it)
		{
			want.insert(TPatchVertId(it->Zone, it->A));
			want.insert(TPatchVertId(it->Zone, it->B));
		}
	}
	else
	{
		for (std::set<TPatchFaceId>::const_iterator it = g_PatchFaceSel.begin();
		     it != g_PatchFaceSel.end(); ++it)
		{
			const SPaintZone *pz = zpFindPaintZone(it->first);
			if (!pz || it->second >= pz->Patches.size())
				continue;
			for (uint c = 0; c < 4; ++c)
				want.insert(TPatchVertId(it->first, pz->Patches[it->second].BaseVertices[c]));
		}
	}
	// Same rule the vertex level applies: one object vertex reached through two nodes is one
	// storage location, so it goes in once.
	for (std::set<TPatchVertId>::const_iterator it = want.begin(); it != want.end(); ++it)
	{
		uint other = 0;
		if (zpVertAliased(it->first, it->second, other))
			continue;
		g_PatchVertSel.insert(*it);
	}
	zpPatchGizmoInvalidate();
}

void zpPatchEdgeSelect(uint zoneId, uint vertA, uint vertB, int op)
{
	const SPatchEdgeId id(zoneId, (uint16)vertA, (uint16)vertB);
	if (op == 0)
		g_PatchEdgeSel.clear();
	if (op == 2)
		g_PatchEdgeSel.erase(id);
	else
		g_PatchEdgeSel.insert(id);
	zpRebuildVertSelFromSubObject();
	ZPSCRIPT::record(NLMISC::toString("painter.selectPatchEdge(%u, %u, %u, %d)",
	                                  zoneId, vertA, vertB, op));
}

void zpPatchFaceSelect(uint zoneId, uint patchIdx, int op)
{
	const TPatchFaceId id(zoneId, patchIdx);
	if (op == 0)
		g_PatchFaceSel.clear();
	if (op == 2)
		g_PatchFaceSel.erase(id);
	else
		g_PatchFaceSel.insert(id);
	zpRebuildVertSelFromSubObject();
	ZPSCRIPT::record(NLMISC::toString("painter.selectPatchFace(%u, %u, %d)", zoneId, patchIdx, op));
}

uint zpPatchVertSelCount()
{
	return (uint)g_PatchVertSel.size();
}

uint zpPatchEdgeSelCount() { return (uint)g_PatchEdgeSel.size(); }
uint zpPatchFaceSelCount() { return (uint)g_PatchFaceSel.size(); }

bool zpPatchEdgeSelAt(uint index, uint &zoneOut, uint &aOut, uint &bOut)
{
	if (index >= g_PatchEdgeSel.size())
		return false;
	std::set<SPatchEdgeId>::const_iterator it = g_PatchEdgeSel.begin();
	std::advance(it, index);
	zoneOut = it->Zone;
	aOut = it->A;
	bOut = it->B;
	return true;
}

bool zpPatchFaceSelAt(uint index, uint &zoneOut, uint &patchOut)
{
	if (index >= g_PatchFaceSel.size())
		return false;
	std::set<TPatchFaceId>::const_iterator it = g_PatchFaceSel.begin();
	std::advance(it, index);
	zoneOut = it->first;
	patchOut = it->second;
	return true;
}

bool zpPatchVertSelAt(uint index, uint &zoneOut, uint &vertOut)
{
	if (index >= g_PatchVertSel.size())
		return false;
	std::set<TPatchVertId>::const_iterator it = g_PatchVertSel.begin();
	std::advance(it, index);
	zoneOut = it->first;
	vertOut = it->second;
	return true;
}

/** Displayed world position of a handle: the first patch slot that uses this Vecs index. */
bool zpPatchTangentWorld(uint zoneId, uint vecIdx, float outPos[3])
{
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz)
		return false;
	for (size_t p = 0; p < pz->Ep.Pm.Patches.size() && p < pz->Patches.size(); ++p)
		for (uint j = 0; j < 8; ++j)
		{
			if (pz->Ep.Pm.Patches[p].Vec[j] < 0
			    || (uint16)pz->Ep.Pm.Patches[p].Vec[j] != (uint16)vecIdx)
				continue;
			const NLMISC::CVector &v = pz->Patches[p].Patch.Tangents[j];
			outPos[0] = v.x; outPos[1] = v.y; outPos[2] = v.z;
			return true;
		}
	return false;
}

void zpPatchTangentSelect(uint zoneId, uint vecIdx, int op)
{
	const TPatchVertId id((uint)zoneId, (uint16)vecIdx);
	if (op == 0)
		g_PatchTanSel.clear();
	if (op == 2)
		g_PatchTanSel.erase(id);
	else
		g_PatchTanSel.insert(id);
	zpPatchGizmoInvalidate();
	ZPSCRIPT::record(NLMISC::toString("painter.selectPatchTangent(%u, %u, %d)", zoneId, vecIdx, op));
}

uint zpPatchTangentSelCount() { return (uint)g_PatchTanSel.size(); }

bool zpPatchTangentSelAt(uint index, uint &zoneOut, uint &vecOut)
{
	if (index >= g_PatchTanSel.size())
		return false;
	std::set<TPatchVertId>::const_iterator it = g_PatchTanSel.begin();
	std::advance(it, index);
	zoneOut = it->first;
	vecOut = it->second;
	return true;
}

bool zpPatchVertWorld(uint zoneId, uint vertIdx, float outPos[3])
{
	if (!g_PaintCtx.Zones)
		return false;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &pz = zones[z];
		if (pz.ZoneId != zoneId)
			continue;
		for (uint p = 0; p < pz.Patches.size(); ++p)
			for (uint c = 0; c < 4; ++c)
			{
				if (pz.Patches[p].BaseVertices[c] != vertIdx)
					continue;
				const NLMISC::CVector &v = pz.Patches[p].Patch.Vertices[c];
				outPos[0] = v.x; outPos[1] = v.y; outPos[2] = v.z;
				return true;
			}
		return false;
	}
	return false;
}

// ---------------------------------------------------------------------------------------------
// Bind and edge-flag ops (the first Tier B milestone: pristine-blob writes only, no stream
// encoder). The core owns the record writes and the undo stroke; this layer owns the policy -
// group semantics, the CheckBind configuration test, the geometry snap - and everything
// display: the eval mirror, BindEdges re-derivation, and the landscape rebuild.
//
// Bind semantics (BindingVertex / UnBindingVertex / UnbindRelatedVertex /
// AddHook / CheckBind) follow nel_patch_lib/nel_patch_mesh.cpp.

/** Is `vert` one of the four corners of pm patch `patchIdx`? */
static bool zpVertexInPmPatch(const PIPELINE::MAX::NELPATCH::SPatchMesh &pm,
                              sint32 vert, sint32 patchIdx)
{
	for (int i = 0; i < 4; ++i)
		if (pm.Patches[patchIdx].V[i] == vert)
			return true;
	return false;
}

/** Per-vertex incident-edge lists (the CVertexNeighborhood shape). */
static void zpBuildVertexNeighbors(const PIPELINE::MAX::NELPATCH::SPatchMesh &pm,
                                   std::vector<std::vector<uint> > &nb)
{
	nb.assign(pm.Verts.size(), std::vector<uint>());
	for (size_t e = 0; e < pm.Edges.size(); ++e)
	{
		if (pm.Edges[e].V1 >= 0 && (size_t)pm.Edges[e].V1 < pm.Verts.size())
			nb[pm.Edges[e].V1].push_back((uint)e);
		if (pm.Edges[e].V2 >= 0 && (size_t)pm.Edges[e].V2 < pm.Verts.size())
			nb[pm.Edges[e].V2].push_back((uint)e);
	}
}

/** The edge joining two vertices, or -1 (IsVerticesJoined). */
static sint32 zpJoinedEdge(const PIPELINE::MAX::NELPATCH::SPatchMesh &pm,
                           const std::vector<std::vector<uint> > &nb, sint32 a, sint32 b)
{
	if (a < 0 || (size_t)a >= nb.size())
		return -1;
	for (size_t n = 0; n < nb[a].size(); ++n)
	{
		const PIPELINE::MAX::NELPATCH::SPmEdge &e = pm.Edges[nb[a][n]];
		if (e.V1 == b || e.V2 == b)
			return (sint32)nb[a][n];
	}
	return -1;
}

/** Exactly one two-edge path a - mid - b, with the two edge indices (IsVerticesJoined2). */
static bool zpJoined2(const PIPELINE::MAX::NELPATCH::SPatchMesh &pm,
                      const std::vector<std::vector<uint> > &nb,
                      sint32 a, sint32 b, sint32 &mid, sint32 &e0, sint32 &e1)
{
	if (zpJoinedEdge(pm, nb, a, b) != -1)
		return false;
	if (a < 0 || (size_t)a >= nb.size())
		return false;
	int count = 0;
	for (size_t n = 0; n < nb[a].size(); ++n)
	{
		const PIPELINE::MAX::NELPATCH::SPmEdge &e = pm.Edges[nb[a][n]];
		const sint32 next = (e.V1 == a) ? e.V2 : e.V1;
		if (next == -1)
			continue;
		const sint32 second = zpJoinedEdge(pm, nb, next, b);
		if (second != -1)
		{
			++count;
			mid = next;
			e0 = (sint32)nb[a][n];
			e1 = second;
		}
	}
	return count == 1;
}

/**
 * CheckBind port (nel_patch_lib): can `vert` be bound onto pm edge `edgeIdx`?
 *
 * Returns 0 for the single configuration (vert joined to both ends of the target edge by
 * open edges), 1 for the triple (a v0 - v25 - vert - v75 - v1 chain of four distinct open
 * edges; v25/v75 returned), -1 when neither holds. The target edge must carry exactly one
 * patch - a closed edge has nothing to bind onto - and none of the vertices being bound may
 * belong to that patch.
 */
static int zpCheckBindConfig(const PIPELINE::MAX::NELPATCH::SPatchMesh &pm,
                             const std::vector<std::vector<uint> > &nb,
                             sint32 vert, sint32 edgeIdx, sint32 &v25, sint32 &v75)
{
	const PIPELINE::MAX::NELPATCH::SPmEdge &edge = pm.Edges[edgeIdx];
	const sint32 v0 = edge.V1, v1 = edge.V2;
	if (v0 == -1 || v1 == -1 || edge.Patches.size() != 1)
		return -1;
	const sint32 targetPatch = edge.Patches[0];
	if (targetPatch < 0 || (size_t)targetPatch >= pm.Patches.size())
		return -1;

	// Single: vert joined to both ends directly.
	const sint32 seg0 = zpJoinedEdge(pm, nb, v0, vert);
	const sint32 seg1 = zpJoinedEdge(pm, nb, v1, vert);
	if (seg0 != -1 && seg1 != -1)
	{
		if (pm.Edges[seg0].Patches.size() < 2 && pm.Edges[seg1].Patches.size() < 2
		    && !zpVertexInPmPatch(pm, vert, targetPatch))
			return 0;
	}

	// Triple: v0 - v25 - vert and v1 - v75 - vert, four distinct open edges.
	sint32 e0 = -1, e1 = -1, e2 = -1, e3 = -1;
	if (zpJoined2(pm, nb, v0, vert, v25, e0, e1) && zpJoined2(pm, nb, v1, vert, v75, e2, e3))
	{
		if (v25 != v75
		    && e0 != e1 && e0 != e2 && e0 != e3 && e1 != e2 && e1 != e3 && e2 != e3
		    && pm.Edges[e0].Patches.size() < 2 && pm.Edges[e1].Patches.size() < 2
		    && pm.Edges[e2].Patches.size() < 2 && pm.Edges[e3].Patches.size() < 2
		    && !zpVertexInPmPatch(pm, vert, targetPatch)
		    && !zpVertexInPmPatch(pm, v25, targetPatch)
		    && !zpVertexInPmPatch(pm, v75, targetPatch))
			return 1;
	}
	return -1;
}

/**
 * Re-derive one zone's BindEdges and smooth-flag bits from its (just-synced) eval mirror.
 *
 * The two passes are the ones buildPatchInfo runs (bind records first, then the one/one
 * edges), rewritten IN PLACE so the patch geometry - which the incremental display path
 * maintains and the display gates pin bit-for-bit - is never touched. Resetting first
 * matters: an unbind must make the derived entries disappear, and the passes only add.
 */
void zpRederiveBindEdges(SPaintZone &pz)
{
	const PIPELINE::MAX::NELPATCH::SPatchMesh &pm = pz.Ep.Pm;
	const PIPELINE::MAX::NELPATCH::SRPatchMesh &rpm = pz.Ep.Rp;
	if (pz.Patches.size() != pm.Patches.size() || rpm.Patches.size() != pm.Patches.size())
		return;

	for (size_t i = 0; i < pz.Patches.size(); ++i)
	{
		NL3D::CPatchInfo &pi = pz.Patches[i];
		for (int e = 0; e < 4; ++e)
		{
			pi.BindEdges[e] = NL3D::CPatchInfo::CBindInfo();
			pi.BindEdges[e].ZoneId = (uint16)pz.ZoneId;
		}
		// Smooth flags: UI edge flag bit 0 = no-smooth.
		pi.Flags &= ~0xf;
		for (int e = 0; e < 4; ++e)
			if (!(rpm.Patches[i].EdgeFlags[e] & 0x1))
				pi.Flags |= (1 << e);
	}

	// Pass 1: bindings from the vertex records.
	for (size_t isrcpatch = 0; isrcpatch < pm.Patches.size(); ++isrcpatch)
	{
		const PIPELINE::MAX::NELPATCH::SPmPatch &srcpatch = pm.Patches[isrcpatch];
		for (int nv = 0; nv < 4; ++nv)
		{
			if (srcpatch.V[nv] < 0 || (size_t)srcpatch.V[nv] >= rpm.Verts.size())
				continue;
			const PIPELINE::MAX::NELPATCH::SRpoVertexBind &uiv = rpm.Verts[srcpatch.V[nv]];
			if (!uiv.Binded)
				continue;
			const int idstpatch = (int)uiv.Patch;
			const int idstedge = (int)uiv.Edge;
			if (idstpatch < 0 || (size_t)idstpatch >= pm.Patches.size() || idstedge < 0 || idstedge >= 4)
				continue;
			int n = -1;
			int icv = -1;
			if (uiv.Type == BIND_SINGLE)
			{
				int orderdstvtx;
				icv = getCommonVertex(pm, idstpatch, (int)isrcpatch, &orderdstvtx);
				if (icv == -1)
					continue;
				n = (idstedge == orderdstvtx) ? 0 : 1;
			}
			else if (uiv.Type == BIND_25)
			{
				n = 1;
				icv = getOtherBindedVertex(rpm, pm, (int)isrcpatch, idstpatch, nv);
				if (icv == -1)
				{
					n = 0;
					icv = getCommonVertex(pm, idstpatch, (int)isrcpatch);
					if (icv == -1)
						continue;
				}
			}
			else if (uiv.Type == BIND_75)
			{
				n = 2;
				icv = getOtherBindedVertex(rpm, pm, (int)isrcpatch, idstpatch, nv);
				if (icv == -1)
				{
					n = 3;
					icv = getCommonVertex(pm, idstpatch, (int)isrcpatch);
					if (icv == -1)
						continue;
				}
			}
			if (n != -1)
			{
				const int isrcedge = getEdge(pm, srcpatch, srcpatch.V[nv], icv);
				if (isrcedge == -1)
					continue;
				pz.Patches[idstpatch].BindEdges[idstedge].NPatchs++;
				pz.Patches[idstpatch].BindEdges[idstedge].Edge[n] = (uint8)isrcedge;
				pz.Patches[idstpatch].BindEdges[idstedge].Next[n] = (uint16)isrcpatch;
				pz.Patches[isrcpatch].BindEdges[isrcedge].NPatchs = 5;
				pz.Patches[isrcpatch].BindEdges[isrcedge].Edge[0] = (uint8)idstedge;
				pz.Patches[isrcpatch].BindEdges[isrcedge].Next[0] = (uint16)idstpatch;
			}
		}
	}

	// Pass 2: one/one cases from the edge patch lists.
	for (size_t i = 0; i < pm.Patches.size(); ++i)
	{
		const PIPELINE::MAX::NELPATCH::SPmPatch &pPatch = pm.Patches[i];
		for (int e = 0; e < 4; ++e)
		{
			if (pPatch.Edge[e] < 0 || (size_t)pPatch.Edge[e] >= pm.Edges.size())
				continue;
			const PIPELINE::MAX::NELPATCH::SPmEdge &edge = pm.Edges[pPatch.Edge[e]];
			if (edge.Patches.size() > 1)
			{
				sint32 other = (edge.Patches[1] != (sint32)i) ? edge.Patches[1] : edge.Patches[0];
				if (other < 0 || (size_t)other >= pm.Patches.size())
					continue;
				const int ce = getCommonEdge(pm, pPatch.Edge[e], pm.Patches[other]);
				if (ce == -1)
					continue;
				pz.Patches[i].BindEdges[e].NPatchs = 1;
				pz.Patches[i].BindEdges[e].Next[0] = (uint16)other;
				pz.Patches[i].BindEdges[e].Edge[0] = (uint8)ce;
			}
		}
	}
}

/**
 * Rebuild the listed landscape zones from their cages (bind structure or smooth flags
 * changed - refreshTesselationGeometry cannot see either). Same construction as the
 * out-of-range rebuild in zpPatchPushLive: keep everything the live zone accumulated, take
 * geometry and binds from the paint zone, collect cross-zone bind neighbours BEFORE the
 * removes and refresh them after the re-adds.
 */
static void zpRebuildLiveZones(const std::set<uint> &zoneIds)
{
	if (!g_PaintCtx.Zones || !g_PaintCtx.Land || zoneIds.empty())
		return;
	NL3D::CLandscape &land = g_PaintCtx.Land->Landscape;
	std::set<std::pair<uint, uint> > refresh;
	// Neighbours first: removeZone unbinds them, so reading them afterwards would find the
	// binds already gone.
	for (std::set<uint>::const_iterator it = zoneIds.begin(); it != zoneIds.end(); ++it)
	{
		const NL3D::CZone *clz = land.getZone((sint)*it);
		for (uint p = 0; clz && p < (uint)clz->getNumPatchs(); ++p)
		{
			const NL3D::CPatch *lp = clz->getPatch((sint)p);
			if (!lp)
				continue;
			for (uint edge = 0; edge < 4; ++edge)
			{
				NL3D::CPatch::CBindInfo nbi;
				lp->getBindNeighbor(edge, nbi);
				if (!nbi.Zone || zoneIds.count((uint)nbi.Zone->getZoneId()))
					continue;
				for (uint i = 0; i < (uint)nbi.NPatchs; ++i)
					if (nbi.Next[i])
						refresh.insert(std::make_pair((uint)nbi.Zone->getZoneId(),
						                              (uint)nbi.Next[i]->getPatchId()));
			}
		}
	}
	for (std::set<uint>::const_iterator it = zoneIds.begin(); it != zoneIds.end(); ++it)
	{
		const SPaintZone *pz = zpFindPaintZone(*it);
		NL3D::CZone *lz = land.getZone((sint)*it);
		if (!pz || !lz)
			continue;
		NL3D::CZoneInfo zi;
		zpLiveZoneInfo(lz, *pz, zi);
		NL3D::CZone zone;
		zone.build(zi);
		NL3D::CZoneCornerSmoother cornerSmoother;
		std::vector<NL3D::CZone *> emptyVector;
		cornerSmoother.computeAllCornerSmoothFlags(&zone, emptyVector);
		land.removeZone((uint16)*it);
		if (!land.addZone(zone))
			fprintf(stderr, "ERROR: zone %u could not be re-added after a bind/flag change; "
			        "its surface is now missing\n", *it);
	}
	for (std::set<std::pair<uint, uint> >::const_iterator it = refresh.begin(); it != refresh.end(); ++it)
	{
		NL3D::CZone *lz = land.getZone((sint)it->first);
		if (lz)
			lz->refreshTesselationGeometry((sint)it->second);
	}
	land.setRefineMode(true);
}

/**
 * Core -> display for bind records and edge flags, forward or by undo/redo.
 *
 * The write landed in a CARRIER, so every node showing that object follows - the same
 * fan-out contract as zpGeomVertChanged. Per node: sync the eval mirror's authored bind
 * fields and edge flags from the pristine (the drawing and the move filter read Ep.Rp),
 * re-derive BindEdges + smooth flags into the display patchinfo, then rebuild the node's
 * landscape zone - a bind is structure, which refreshTesselationGeometry cannot express.
 */
void zpRpStateChanged(uint zoneId)
{
	if (!g_PaintCtx.Zones || !g_PaintCtx.Core)
		return;
	std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	const void *object = zpZoneNode(zoneId);
	if (!object)
		return;
	std::set<uint> rebuilt;
	for (uint z = 0; z < zones.size(); ++z)
	{
		SPaintZone &pz = zones[z];
		if ((const void *)pz.Node != object)
			continue;
		for (size_t v = 0; v < pz.Ep.Rp.Verts.size(); ++v)
		{
			bool binded;
			uint32 type, edge, patch, prim;
			if (!g_PaintCtx.Core->getVertBind(pz.ZoneId, (uint16)v, binded, type, edge, patch, prim))
				break;
			PIPELINE::MAX::NELPATCH::SRpoVertexBind &b = pz.Ep.Rp.Verts[v];
			b.Binded = binded ? 1 : 0;
			b.Type = type;
			b.Type2 = type;
			b.Edge = edge;
			b.Patch = patch;
			b.PrimVert = prim;
		}
		for (size_t p = 0; p < pz.Ep.Rp.Patches.size(); ++p)
		{
			uint32 flags[4];
			if (!g_PaintCtx.Core->getPatchEdgeFlags(pz.ZoneId, (uint16)p, flags))
				break;
			for (int e = 0; e < 4; ++e)
				pz.Ep.Rp.Patches[p].EdgeFlags[e] = flags[e];
		}
		zpRederiveBindEdges(pz);
		rebuilt.insert(pz.ZoneId);
	}
	zpRebuildLiveZones(rebuilt);
}

/**
 * Release the bound vertices of the current selection, whole groups at a time.
 *
 * Group semantics are the legacy UnbindRelatedVertex rule: every vertex bound to the same
 * (patch, edge) target releases together. A BIND_25/50/75 trio references its BIND_50
 * anchor through PrimVert, so releasing one of them alone would leave the siblings
 * evaluating against a vertex that is no longer their anchor - there is no half-released
 * state that means anything, which is why the group is the unit.
 */
uint zpUnbindPatchSelection()
{
	if (!g_PaintCtx.Core)
		return 0;
	// (zone -> vertex set), group-expanded from the selection.
	std::map<uint, std::set<uint16> > want;
	uint groups = 0;
	for (std::set<TPatchVertId>::const_iterator it = g_PatchVertSel.begin();
	     it != g_PatchVertSel.end(); ++it)
	{
		bool binded;
		uint32 type, edge, patch, prim;
		if (!g_PaintCtx.Core->getVertBind(it->first, it->second, binded, type, edge, patch, prim)
		    || !binded)
			continue;
		if (want[it->first].count(it->second))
			continue; // already collected via a sibling's group
		const SPaintZone *pz = zpFindPaintZone(it->first);
		if (!pz || !pz->Editable)
			continue;
		++groups;
		for (size_t v = 0; v < pz->Ep.Rp.Verts.size(); ++v)
		{
			bool b2;
			uint32 t2, e2, p2, pr2;
			if (g_PaintCtx.Core->getVertBind(it->first, (uint16)v, b2, t2, e2, p2, pr2)
			    && b2 && p2 == patch && e2 == edge)
				want[it->first].insert((uint16)v);
		}
	}
	uint written = 0;
	for (std::map<uint, std::set<uint16> >::const_iterator zit = want.begin();
	     zit != want.end(); ++zit)
	{
		std::vector<ZPPAINT::SBindEdit> edits;
		for (std::set<uint16>::const_iterator v = zit->second.begin(); v != zit->second.end(); ++v)
		{
			ZPPAINT::SBindEdit e;
			e.Vert = *v;
			e.Binded = false;
			edits.push_back(e);
		}
		std::string err;
		written += g_PaintCtx.Core->opEditBinds(zit->first,
			edits, std::vector<ZPPAINT::SGeomElemRef>(), std::vector<NLMISC::CVector>(), err);
		if (!err.empty() && g_PropStatusMsg.empty())
			g_PropStatusMsg = "unbind: " + err;
	}
	if (written)
		g_PropStatusMsg = NLMISC::toString("unbind: %u vertices released (%u group%s)",
		                                   written, groups, groups == 1 ? "" : "s");
	else if (g_PropStatusMsg.empty())
		g_PropStatusMsg = "unbind: no bound vertices in the selection";
	ZPSCRIPT::record("painter.unbindPatchSelection()");
	return written;
}

/**
 * Bind `vertIdx` onto edge `edgeSlot` of patch `patchIdx` (the AddHook shape, both
 * configurations). Validates with the CheckBind port, then snaps the bound vertices onto
 * their bindWhere points: the bind refresh is re-run on a COPY of the eval mesh and every
 * position it moved is written through the Tier A geometry path in the same undo stroke -
 * the post-refresh stored state, so a file saved here and one resaved after load agree.
 */
bool zpBindPatchVertexToEdge(uint zoneId, uint vertIdx, uint patchIdx, uint edgeSlot,
                             std::string &msg)
{
	if (!g_PaintCtx.Core)
	{
		msg = "bind: no session";
		return false;
	}
	SPaintZone *pz = zpFindPaintZoneMut(zoneId);
	if (!pz || !pz->Editable)
	{
		msg = "bind: zone is read-only";
		return false;
	}
	const PIPELINE::MAX::NELPATCH::SPatchMesh &pm = pz->Ep.Pm;
	if (patchIdx >= pm.Patches.size() || edgeSlot >= 4)
	{
		msg = "bind: target out of range";
		return false;
	}
	const sint32 edgeIdx = pm.Patches[patchIdx].Edge[edgeSlot];
	if (edgeIdx < 0 || (size_t)edgeIdx >= pm.Edges.size())
	{
		msg = "bind: target edge unresolved";
		return false;
	}
	if (vertIdx >= pz->Ep.Rp.Verts.size())
	{
		msg = "bind: vertex out of range";
		return false;
	}
	{
		bool binded;
		uint32 t, e, p, pr;
		if (g_PaintCtx.Core->getVertBind(zoneId, (uint16)vertIdx, binded, t, e, p, pr) && binded)
		{
			msg = "bind: vertex is already bound (unbind first)";
			return false;
		}
	}

	std::vector<std::vector<uint> > nb;
	zpBuildVertexNeighbors(pm, nb);
	sint32 v25 = -1, v75 = -1;
	const int config = zpCheckBindConfig(pm, nb, (sint32)vertIdx, edgeIdx, v25, v75);
	if (config == -1)
	{
		msg = "bind: no valid configuration onto that edge";
		return false;
	}

	std::vector<ZPPAINT::SBindEdit> edits;
	{
		ZPPAINT::SBindEdit e;
		e.Edge = edgeSlot;
		e.Patch = patchIdx;
		e.Binded = true;
		if (config == 0)
		{
			e.Vert = (uint16)vertIdx;
			e.Type = BIND_SINGLE;
			e.PrimVert = vertIdx;
			edits.push_back(e);
		}
		else
		{
			e.PrimVert = vertIdx;
			e.Vert = (uint16)v25;
			e.Type = BIND_25;
			edits.push_back(e);
			e.Vert = (uint16)vertIdx;
			e.Type = BIND_50;
			edits.push_back(e);
			e.Vert = (uint16)v75;
			e.Type = BIND_75;
			edits.push_back(e);
		}
	}

	// Snap: run the bind refresh on a copy carrying the new records, then harvest what it
	// moved as object-space deltas - RESTRICTED to the new bind's own closure. The refresh
	// sweeps every bound vertex and every auto interior of the zone, and on this corpus it
	// is NOT bit-for-bit idempotent (the stored data carries the reference exporter's exact
	// x87 bits, which our rebuild reproduces only to within an ulp) - harvesting everything
	// that differed would overwrite hundreds of Max-authored values per bind. The closure:
	// the bound vertices, their rebuilt tangent caches, and the interiors of AUTO patches
	// those corners and tangents feed. Everything else keeps its stored bytes.
	std::vector<ZPPAINT::SGeomElemRef> snapElems;
	std::vector<NLMISC::CVector> snapDeltas;
	{
		SEvalPatch copy = pz->Ep;
		for (size_t i = 0; i < edits.size(); ++i)
		{
			PIPELINE::MAX::NELPATCH::SRpoVertexBind &b = copy.Rp.Verts[edits[i].Vert];
			b.Binded = 1;
			b.Type = edits[i].Type;
			b.Type2 = edits[i].Type;
			b.Edge = edits[i].Edge;
			b.Patch = edits[i].Patch;
			b.PrimVert = edits[i].PrimVert;
			b.Before = b.Before2 = b.After = b.After2 = b.T = (uint32)-1;
		}
		std::string err;
		if (!updateBindingInfo(copy, err) || !updateBindingPos(copy, err))
		{
			// The refresh sweeps every bound vertex; a pre-existing record it cannot
			// classify aborts the snap but not the bind - the bound vertex was picked at
			// the junction, so its position is already substantially right.
			fprintf(stderr, "WARNING: bind snap skipped for zone %u: %s\n", zoneId, err.c_str());
		}
		else
		{
			std::set<uint16> movedVerts, movedVecs;
			for (size_t i = 0; i < edits.size(); ++i)
			{
				movedVerts.insert(edits[i].Vert);
				const PIPELINE::MAX::NELPATCH::SRpoVertexBind &b = copy.Rp.Verts[edits[i].Vert];
				const uint32 caches[5] = { b.Before, b.Before2, b.After, b.After2, b.T };
				for (int c = 0; c < 5; ++c)
					if (caches[c] < copy.Pm.Vecs.size())
						movedVecs.insert((uint16)caches[c]);
			}
			// Interiors fed by a moved corner or a moved tangent (computeInteriors rule:
			// interior j = corner j + its outgoing and incoming tangents).
			for (size_t p = 0; p < copy.Pm.Patches.size(); ++p)
			{
				const PIPELINE::MAX::NELPATCH::SPmPatch &pp = copy.Pm.Patches[p];
				if (!(pp.Flags & PM_PATCH_AUTO))
					continue;
				for (int j = 0; j < 4; ++j)
				{
					if ((size_t)pp.Interior[j] >= copy.Pm.Vecs.size())
						continue;
					const bool touched =
						(pp.V[j] >= 0 && movedVerts.count((uint16)pp.V[j]))
						|| (pp.Vec[j * 2] >= 0 && movedVecs.count((uint16)pp.Vec[j * 2]))
						|| (pp.Vec[(j * 2 + 7) & 7] >= 0 && movedVecs.count((uint16)pp.Vec[(j * 2 + 7) & 7]));
					if (touched)
						movedVecs.insert((uint16)pp.Interior[j]);
				}
			}
			for (std::set<uint16>::const_iterator it = movedVerts.begin(); it != movedVerts.end(); ++it)
			{
				if (*it >= copy.Pm.Verts.size() || *it >= pz->Ep.Pm.Verts.size())
					continue;
				const float *a = copy.Pm.Verts[*it].Pos, *o = pz->Ep.Pm.Verts[*it].Pos;
				if (a[0] != o[0] || a[1] != o[1] || a[2] != o[2])
				{
					snapElems.push_back(ZPPAINT::SGeomElemRef(*it, ZPPAINT::GeomVert));
					snapDeltas.push_back(NLMISC::CVector(a[0] - o[0], a[1] - o[1], a[2] - o[2]));
				}
			}
			for (std::set<uint16>::const_iterator it = movedVecs.begin(); it != movedVecs.end(); ++it)
			{
				if (*it >= copy.Pm.Vecs.size() || *it >= pz->Ep.Pm.Vecs.size())
					continue;
				const float *a = copy.Pm.Vecs[*it].Pos, *o = pz->Ep.Pm.Vecs[*it].Pos;
				if (a[0] != o[0] || a[1] != o[1] || a[2] != o[2])
				{
					snapElems.push_back(ZPPAINT::SGeomElemRef(*it, ZPPAINT::GeomVec));
					snapDeltas.push_back(NLMISC::CVector(a[0] - o[0], a[1] - o[1], a[2] - o[2]));
				}
			}
		}
	}

	std::string err;
	const uint n = g_PaintCtx.Core->opEditBinds(zoneId, edits, snapElems, snapDeltas, err);
	if (!n)
	{
		msg = err.empty() ? "bind: nothing written" : ("bind: " + err);
		return false;
	}
	msg = config == 0
		? NLMISC::toString("bind: vertex %u -> patch %u edge %u (single, %u snapped)",
		                   vertIdx, patchIdx, edgeSlot, (uint)snapElems.size())
		: NLMISC::toString("bind: vertices %d/%u/%d -> patch %u edge %u (25/50/75, %u snapped)",
		                   v25, vertIdx, v75, patchIdx, edgeSlot, (uint)snapElems.size());
	ZPSCRIPT::record(NLMISC::toString("painter.bindPatchVertex(%u, %u, %u, %u)",
	                                  zoneId, vertIdx, patchIdx, edgeSlot));
	return true;
}

/** Squared distance from a point to the segment ab (target-edge disambiguation). */
static float zpPointSegDist2(const NLMISC::CVector &p, const NLMISC::CVector &a,
                             const NLMISC::CVector &b)
{
	const NLMISC::CVector ab = b - a;
	const float len2 = ab * ab;
	float t = len2 > 1e-12f ? ((p - a) * ab) / len2 : 0.f;
	if (t < 0.f) t = 0.f;
	if (t > 1.f) t = 1.f;
	const NLMISC::CVector c = a + ab * t;
	return (p - c) * (p - c);
}

/**
 * Bind the selected free vertices, each onto the nearest open edge that accepts it.
 *
 * The selection names the vertex; the target edge is found by running the CheckBind test
 * over every open edge and taking the geometrically nearest valid one - a bindable vertex
 * sits ON its target edge (that is what a T-junction is), so the nearest-valid rule is the
 * drag gesture's answer without the drag. A triple binds through its middle vertex; the
 * companions are discovered by the configuration test, so selecting just the middle is
 * enough, and companions that were also selected are skipped once bound.
 */
uint zpBindPatchSelection()
{
	if (!g_PaintCtx.Core)
		return 0;
	uint bound = 0;
	std::string lastMsg;
	for (std::set<TPatchVertId>::const_iterator it = g_PatchVertSel.begin();
	     it != g_PatchVertSel.end(); ++it)
	{
		const SPaintZone *pz = zpFindPaintZone(it->first);
		if (!pz || !pz->Editable)
			continue;
		{
			bool binded;
			uint32 t, e, p, pr;
			if (g_PaintCtx.Core->getVertBind(it->first, it->second, binded, t, e, p, pr) && binded)
				continue; // bound this pass as a companion, or was never free
		}
		const PIPELINE::MAX::NELPATCH::SPatchMesh &pm = pz->Ep.Pm;
		if (it->second >= pm.Verts.size())
			continue;
		std::vector<std::vector<uint> > nb;
		zpBuildVertexNeighbors(pm, nb);
		const NLMISC::CVector vp(pm.Verts[it->second].Pos[0], pm.Verts[it->second].Pos[1],
		                         pm.Verts[it->second].Pos[2]);
		// Nearest open edge passing the configuration test.
		float bestD2 = 0.f;
		sint32 bestPatch = -1, bestSlot = -1;
		for (size_t e = 0; e < pm.Edges.size(); ++e)
		{
			const PIPELINE::MAX::NELPATCH::SPmEdge &edge = pm.Edges[e];
			if (edge.Patches.size() != 1 || edge.V1 < 0 || edge.V2 < 0)
				continue;
			sint32 v25, v75;
			if (zpCheckBindConfig(pm, nb, (sint32)it->second, (sint32)e, v25, v75) == -1)
				continue;
			const sint32 tp = edge.Patches[0];
			sint32 slot = -1;
			for (int s = 0; s < 4; ++s)
				if (pm.Patches[tp].Edge[s] == (sint32)e) { slot = s; break; }
			if (slot == -1)
				continue;
			const NLMISC::CVector a(pm.Verts[edge.V1].Pos[0], pm.Verts[edge.V1].Pos[1],
			                        pm.Verts[edge.V1].Pos[2]);
			const NLMISC::CVector b(pm.Verts[edge.V2].Pos[0], pm.Verts[edge.V2].Pos[1],
			                        pm.Verts[edge.V2].Pos[2]);
			const float d2 = zpPointSegDist2(vp, a, b);
			if (bestPatch == -1 || d2 < bestD2)
			{
				bestD2 = d2;
				bestPatch = tp;
				bestSlot = slot;
			}
		}
		if (bestPatch == -1)
		{
			if (lastMsg.empty())
				lastMsg = NLMISC::toString("bind: vertex %u has no valid target edge", it->second);
			continue;
		}
		std::string msg;
		if (zpBindPatchVertexToEdge(it->first, it->second, (uint)bestPatch, (uint)bestSlot, msg))
			++bound;
		lastMsg = msg;
	}
	if (!lastMsg.empty())
		g_PropStatusMsg = lastMsg;
	else if (!bound)
		g_PropStatusMsg = "bind: no free vertex selected";
	return bound;
}

/**
 * No-smooth over the selected edges: legacy setSmoothFlags writes the flag on EVERY patch
 * that carries the edge, so the seam has one state however it is reached.
 */
uint zpSetEdgeNoSmooth(bool noSmooth)
{
	if (!g_PaintCtx.Core)
		return 0;
	std::map<uint, std::vector<ZPPAINT::SEdgeFlagEdit> > writes;
	for (std::set<SPatchEdgeId>::const_iterator it = g_PatchEdgeSel.begin();
	     it != g_PatchEdgeSel.end(); ++it)
	{
		const SPaintZone *pz = zpFindPaintZone(it->Zone);
		if (!pz || !pz->Editable)
			continue;
		const PIPELINE::MAX::NELPATCH::SPatchMesh &pm = pz->Ep.Pm;
		// The selection keys edges on the corner pair; resolve to the pm edge.
		sint32 edgeIdx = -1;
		for (size_t e = 0; e < pm.Edges.size(); ++e)
			if ((pm.Edges[e].V1 == (sint32)it->A && pm.Edges[e].V2 == (sint32)it->B)
			    || (pm.Edges[e].V1 == (sint32)it->B && pm.Edges[e].V2 == (sint32)it->A))
			{
				edgeIdx = (sint32)e;
				break;
			}
		if (edgeIdx == -1)
			continue;
		for (size_t pi = 0; pi < pm.Edges[edgeIdx].Patches.size(); ++pi)
		{
			const sint32 p = pm.Edges[edgeIdx].Patches[pi];
			if (p < 0 || (size_t)p >= pm.Patches.size())
				continue;
			sint32 slot = -1;
			for (int s = 0; s < 4; ++s)
				if (pm.Patches[p].Edge[s] == edgeIdx) { slot = s; break; }
			if (slot == -1)
				continue;
			uint32 flags[4];
			if (!g_PaintCtx.Core->getPatchEdgeFlags(it->Zone, (uint16)p, flags))
				continue;
			ZPPAINT::SEdgeFlagEdit w;
			w.Patch = (uint16)p;
			w.EdgeSlot = (uint8)slot;
			w.NewFlags = (flags[slot] & ~0x1u) | (noSmooth ? 0x1u : 0u);
			writes[it->Zone].push_back(w);
		}
	}
	uint written = 0;
	for (std::map<uint, std::vector<ZPPAINT::SEdgeFlagEdit> >::const_iterator zit = writes.begin();
	     zit != writes.end(); ++zit)
	{
		std::string err;
		written += g_PaintCtx.Core->opSetEdgeFlags(zit->first, zit->second, err);
		if (!err.empty())
			g_PropStatusMsg = "no-smooth: " + err;
	}
	if (written)
		g_PropStatusMsg = NLMISC::toString("no smooth %s: %u edge flag%s written",
		                                   noSmooth ? "ON" : "OFF", written, written == 1 ? "" : "s");
	else if (g_PropStatusMsg.empty())
		g_PropStatusMsg = "no smooth: no selected edge changed";
	ZPSCRIPT::record(NLMISC::toString("painter.setEdgeNoSmooth(%s)", noSmooth ? "true" : "false"));
	return written;
}

/** Tri-state of one edge given as a corner pair: 0 none, 1 flagged, 2 out of range. An edge
 *  counts as flagged when ANY adjacent patch carries the bit (legacy getSmoothFlags). */
static int zpEdgeNoSmoothOne(uint zoneId, uint16 a, uint16 b)
{
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz || !g_PaintCtx.Core)
		return 2;
	const PIPELINE::MAX::NELPATCH::SPatchMesh &pm = pz->Ep.Pm;
	for (size_t e = 0; e < pm.Edges.size(); ++e)
	{
		if (!((pm.Edges[e].V1 == (sint32)a && pm.Edges[e].V2 == (sint32)b)
		      || (pm.Edges[e].V1 == (sint32)b && pm.Edges[e].V2 == (sint32)a)))
			continue;
		for (size_t pi = 0; pi < pm.Edges[e].Patches.size(); ++pi)
		{
			const sint32 p = pm.Edges[e].Patches[pi];
			if (p < 0 || (size_t)p >= pm.Patches.size())
				continue;
			sint32 slot = -1;
			for (int s = 0; s < 4; ++s)
				if (pm.Patches[p].Edge[s] == (sint32)e) { slot = s; break; }
			if (slot == -1)
				continue;
			uint32 flags[4];
			if (g_PaintCtx.Core->getPatchEdgeFlags(zoneId, (uint16)p, flags)
			    && (flags[slot] & 0x1))
				return 1;
		}
		return 0;
	}
	return 2;
}

/** Selection tri-state for the panel checkbox: 0 none, 1 all, 2 mixed / no selection. */
int zpEdgeNoSmoothTriState()
{
	bool any = false, flagged = false, clear = false;
	for (std::set<SPatchEdgeId>::const_iterator it = g_PatchEdgeSel.begin();
	     it != g_PatchEdgeSel.end(); ++it)
	{
		const int s = zpEdgeNoSmoothOne(it->Zone, it->A, it->B);
		if (s == 2)
			continue;
		any = true;
		if (s == 1) flagged = true;
		else clear = true;
	}
	if (!any)
		return 2;
	if (flagged && clear)
		return 2;
	return flagged ? 1 : 0;
}

/** Script/gate read access: the tri-state of ONE edge (0 clear, 1 flagged, -1 unknown). */
int zpEdgeNoSmoothQuery(uint zoneId, uint vertA, uint vertB)
{
	const int s = zpEdgeNoSmoothOne(zoneId, (uint16)vertA, (uint16)vertB);
	return s == 2 ? -1 : s;
}

/**
 * The panel checkbox click. Legacy behaviour: an indeterminate (mixed) state clears first,
 * otherwise the click inverts - so mixed -> all clear, clear -> all set, set -> all clear.
 */
void zpPatchNoSmoothClicked()
{
	const int state = zpEdgeNoSmoothTriState();
	zpSetEdgeNoSmooth(state == 0);
}

/** Bridge wrappers (void-returning function pointers). */
void zpPatchBindClicked() { zpBindPatchSelection(); }
void zpPatchUnbindClicked() { zpUnbindPatchSelection(); }

/** Script/gate read access to a pristine bind record (flat ints for the script TU). */
bool zpVertexBindQuery(uint zoneId, uint vertIdx, int &bindedOut, int &typeOut,
                       int &patchOut, int &edgeOut, int &primOut)
{
	if (!g_PaintCtx.Core)
		return false;
	bool binded;
	uint32 type, edge, patch, prim;
	if (!g_PaintCtx.Core->getVertBind(zoneId, (uint16)vertIdx, binded, type, edge, patch, prim))
		return false;
	bindedOut = binded ? 1 : 0;
	typeOut = (int)type;
	patchOut = (int)patch;
	edgeOut = (int)edge;
	primOut = (int)prim;
	return true;
}

/** The corner pair of edge `edgeSlot` of `patchIdx` (lets a script name an edge the way the
 *  edge selection does, from a bind record's target fields). */
bool zpPatchEdgeCornerPair(uint zoneId, uint patchIdx, uint edgeSlot, uint &aOut, uint &bOut)
{
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz || patchIdx >= pz->Ep.Pm.Patches.size() || edgeSlot >= 4)
		return false;
	const sint32 e = pz->Ep.Pm.Patches[patchIdx].Edge[edgeSlot];
	if (e < 0 || (size_t)e >= pz->Ep.Pm.Edges.size())
		return false;
	if (pz->Ep.Pm.Edges[e].V1 < 0 || pz->Ep.Pm.Edges[e].V2 < 0)
		return false;
	aOut = (uint)pz->Ep.Pm.Edges[e].V1;
	bOut = (uint)pz->Ep.Pm.Edges[e].V2;
	return true;
}

/** The vec index at ring slot 0..7 of a patch (lets a script name a tangent handle the
 *  way patchTangentPos wants it, without knowing allocation order). */
bool zpPatchVecIndex(uint zoneId, uint patchIdx, uint ringSlot, uint &vecOut)
{
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz || patchIdx >= pz->Ep.Pm.Patches.size() || ringSlot >= 8)
		return false;
	const sint32 v = pz->Ep.Pm.Patches[patchIdx].Vec[ringSlot];
	if (v < 0)
		return false;
	vecOut = (uint)v;
	return true;
}

void zpPatchVertClear()
{
	// Every level's set: at edge or patch level the vertex set is a projection of one of the
	// others, so clearing only the projection would leave the cage highlighted and the next
	// rebuild would put it straight back.
	if (g_PatchVertSel.empty() && g_PatchEdgeSel.empty() && g_PatchFaceSel.empty()
	    && g_PatchTanSel.empty())
		return;
	g_PatchVertSel.clear();
	g_PatchEdgeSel.clear();
	g_PatchFaceSel.clear();
	g_PatchTanSel.clear();
	zpPatchGizmoInvalidate();
	ZPSCRIPT::record("painter.clearPatchVertexSelection()");
}

/* end of file */
