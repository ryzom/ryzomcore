/**
 * \file viewer.cpp
 * \brief Interactive viewer main loop, mouse-listener impls, screenshot, bank load.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * `runViewer` (event-server loop, HUD, NLGUI draw, painter-script pump, screenshots);
 * `CPaintMouseListener` method bodies (paint-at-hover, mode/pick routing, prop-mode
 * selection); `zpViewerScreenshot`; `loadBankFile`.
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

// ---------------------------------------------------------------------------------------------
// Viewer / screenshot: the painting scene (paint.cpp myThread without the paint tools).

#include "viewer_listener.h"
// The light setup lives in g_Light* above (paint_ui.cpp defaults, vars-cfg overridable).

// kMainWidth / kMainHeight are defined in main.cpp (extern in zp_state.h).

// The tile bank (the plugin took it from the tile_utility choice; here it is --bank). Tile
// texture paths become CPath-resolvable relative names, seeded with the bank file's directory
// (recursive on request) plus any extra search paths.
bool loadBankFile(const std::string &bankPath, bool bankRecursive,
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
	// Landscape-side season resolution: the bank references unpostfixed names, the workspace
	// carries season-postfixed converted files (tiles/ + diplace/ siblings of the smallbank).
	{
		uint resolved = 0, missing = 0;
		ZPCTX::resolveBankTextures(bank, bankPath, resolved, missing);
		printf("bank textures: %u resolved, %u missing\n", resolved, missing);
	}
	return true;
}

// The viewer's paint mouse listener (plugin MouseListener port, tile mode only): left button
// paints through the shared op layer, right button picks the tile set under the cursor,
// Ctrl+Z / Ctrl+E undo/redo. The edit3d navigation stays on the middle mouse.
// ModeProp: left click selects editable zones instead of painting.
void zpUndo();
void zpRedo();

bool CPaintMouseListener::guiWantsMouse() const { return EditorUI && EditorUI->wantsMouse(); }

// One paint action at the current hover (shared by click and drag). Prop mode never
// paints. cont: continuation of an open stroke (recorded for stroke-aware replay;
// displace commits per-op and ignores it).
void CPaintMouseListener::paintAtHover(bool cont)
{
		if (Mode == ModeProp)
			return;
		std::string err;
		if (Mode == ModeColor)
		{
			NLMISC::CVector pos, dir, hit;
			Viewport.getRayWithPoint(MouseX, MouseY, pos, dir, Camera->getMatrix(), Camera->getFrustum());
			uint zone;
			sint32 tile;
			if (Core->pickTile(pos, dir, zone, tile, hit) && !Core->zoneFrozen(zone))
			{
				// %.9g round-trips float32 exactly (the old %.3f quantized the hit, and the
				// distance-based blend made replays only approximate); cont rides the
				// stroke-aware form: painter.endStroke() is recorded at mouse-up.
				if (Core->opColorBrush(zone, tile, hit, BrushRadius, BrushColor, BrushHardness, BrushOpacity, err))
					ZPSCRIPT::record(NLMISC::toString("painter.colorBrush(%u, %.9g, %.9g, %.9g, \"%02x%02x%02x\", %u, %u, %.9g, %s)",
						zone, hit.x, hit.y, BrushRadius, BrushColor.R, BrushColor.G, BrushColor.B,
						(uint)BrushHardness, (uint)BrushOpacity, hit.z, cont ? "true" : "false"));
			}
		}
		else if (Mode == ModeDisplace)
		{
			sint32 t = HoverTile;
			if (Core->opDisplace(HoverZone, (uint)(t / ZP_NUM_TILE_SEL), (uint)(t % ZP_NUM_TILE_SEL % ZP_MAX_TILE_IN_PATCH),
			                     (uint)(t % ZP_NUM_TILE_SEL / ZP_MAX_TILE_IN_PATCH), DisplaceIndex, err))
				ZPSCRIPT::record(NLMISC::toString("painter.paintDisplace(%u, %u, %u, %u, %u)",
					HoverZone, (uint)(t / ZP_NUM_TILE_SEL), (uint)(t % ZP_NUM_TILE_SEL % ZP_MAX_TILE_IN_PATCH),
					(uint)(t % ZP_NUM_TILE_SEL / ZP_MAX_TILE_IN_PATCH), (uint)DisplaceIndex));
		}
}

void CPaintMouseListener::updateHover()
{
	HaveHover = false;
	if (!Core || !Camera) return;
	NLMISC::CVector pos, dir, hit;
	Viewport.getRayWithPoint(MouseX, MouseY, pos, dir, Camera->getMatrix(), Camera->getFrustum());
	uint zone;
	sint32 tile;
	if (Core->pickTile(pos, dir, zone, tile, hit))
	{
		// Prop mode: only hover editable (unfrozen primary) zones; no outline on RO/instance.
		if (Mode == ModeProp && !zpZoneIsPropSelectable(zone))
			return;
		HaveHover = true;
		HoverZone = zone;
		HoverTile = tile;
	}
}

void CPaintMouseListener::operator()(const NLMISC::CEvent &event)
{
		if (!Core) return;
		if (g_ScriptUiLock) return; // painterscript pump: input locked (ESC handled by the pump)
		// A live gizmo drag CAPTURES the pointer. It can only start on the scene, so while it
		// runs the GUI does not steal the move - a release over a toolbar must still end the
		// drag, or it stays armed and the next stray move continues it with the button up, and
		// the cancelling right click must reach the drag wherever the pointer happens to be.
		const bool gizmoCapture = (zpPatchGizmoDragging() || zpWeldDragActive())
			&& (event == NLMISC::EventMouseMoveId || event == NLMISC::EventMouseUpId
			    || event == NLMISC::EventMouseDownId);
		if (!gizmoCapture && guiWantsMouse())
		{
			// Abort an in-progress stroke if the pointer enters a GUI window mid-drag
			if (Pressed && (event == NLMISC::EventMouseUpId || event == NLMISC::EventMouseMoveId))
			{
				Pressed = false;
				Core->endStroke();
				ZPSCRIPT::record("painter.endStroke()");
			}
			return;
		}
		if (event == NLMISC::EventMouseDownId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			MouseX = mouse->X;
			MouseY = mouse->Y;
			// Patch edit intercepts BEFORE the exact-equality test below: that test is
			// `Button == leftButton`, so Ctrl+left and Alt+left never reach it - which is
			// exactly the pair add/remove-from-selection needs. Patch mode also never
			// paints, so returning here is the whole of its mouse-down behaviour.
			if (Mode == ModePatch)
			{
				// Every level that MOVES something takes the click - the same three levels the
				// gizmo is drawn for. zpPatchVertexClick dispatches on the level itself, so the
				// edge and patch picks are its concern, not this test's.
				if ((mouse->Button & NLMISC::leftButton)
				    && (SubObj == SubVertex || SubObj == SubEdge || SubObj == SubPatch))
				{
					// The armed target-weld mode claims a press that lands ON a vertex;
					// anywhere else the press falls through to the normal paths, so arming
					// only changes what a vertex press means.
					NL3D::IDriver *drv = g_PaintCtx.UDriver
						? static_cast<NL3D::CDriverUser *>(g_PaintCtx.UDriver)->getDriver() : NULL;
					if (g_WeldTargetArmed && SubObj == SubVertex
					    && zpWeldDragBegin(Camera, drv, MouseX, MouseY))
						return;
					// A hot handle claims the click. Hover was resolved by the last frame's
					// draw, which is the frame the artist was looking at when they pressed.
					if (zpPatchGizmoBeginDrag(zpPatchGizmoHover(), Camera, Viewport, MouseX, MouseY))
						return;
					zpPatchVertexClick(Camera, drv, MouseX, MouseY, (uint)mouse->Button);
				}
				if (mouse->Button & NLMISC::rightButton)
				{
					// Cancel: a right click during a transform or weld drag abandons it and
					// nothing is written; armed-but-idle target weld disarms. Otherwise the
					// scene context menu - patch mode has no eyedropper to spend the right
					// button on, and this is where the user pivot gets placed. This must
					// live INSIDE the mode block: an earlier spelling sat below it, after
					// the unconditional return, and was dead. Live drags come BEFORE the
					// disarm: arming only claims presses that land ON a vertex, so a gizmo
					// drag can be live while armed - the cancel must reach the drag, not
					// spend itself disarming an idle mode under it.
					if (zpWeldDragActive())
						zpWeldDragCancel();
					else if (zpPatchGizmoDragging())
						zpPatchGizmoCancelDrag();
					else if (g_WeldTargetArmed)
						zpWeldTargetToggleClicked();
					else
						zpOpenSceneMenu();
				}
				return;
			}
			if (mouse->Button == NLMISC::leftButton)
			{
				// Prop mode: click selects (or reports read-only); no paint stroke.
				if (Mode == ModeProp)
				{
					NLMISC::CVector pos, dir, hit;
					Viewport.getRayWithPoint(MouseX, MouseY, pos, dir, Camera->getMatrix(), Camera->getFrustum());
					uint zone = 0;
					sint32 tile = -1;
					if (Core->pickTile(pos, dir, zone, tile, hit))
					{
						if (zpZoneIsPropSelectable(zone))
						{
							g_HavePropSelection = true;
							g_SelectedZoneId = zone;
							const SPaintZone *pz = zpFindPaintZone(zone);
							g_PropStatusMsg = pz ? ("selected " + pz->Name) : NLMISC::toString("selected zone %u", zone);
						}
						else
						{
							g_PropStatusMsg = "read-only";
						}
					}
					else
					{
						// Empty click clears selection (modern object-selection feel)
						zpClearPropSelection();
					}
					return;
				}
				updateHover();
				if (HaveHover && !Core->zoneFrozen(HoverZone))
				{
					if (Mode == ModeTile)
					{
						std::string err;
						if (Core->opTileStroke(HoverZone, HoverTile, CurTileSet, Mode256, true, err))
						{
							// Stroke-aware form (cont=false = stroke start): replay keeps the
							// live drag's rotation-following and one-undo-entry granularity;
							// painter.endStroke() is recorded at mouse-up.
							ZPSCRIPT::record(NLMISC::toString("painter.tileStroke(%u, %u, %u, %u, %d, %s, false)",
								HoverZone, (uint)(HoverTile / ZP_NUM_TILE_SEL),
								(uint)(HoverTile % ZP_NUM_TILE_SEL % ZP_MAX_TILE_IN_PATCH),
								(uint)(HoverTile % ZP_NUM_TILE_SEL / ZP_MAX_TILE_IN_PATCH), CurTileSet,
								Mode256 ? "true" : "false"));
							Pressed = true;
							StrokeZone = HoverZone;
							StrokeTile = HoverTile;
						}
					}
					else
					{
						paintAtHover(false);
						Pressed = true;
						StrokeZone = HoverZone;
						StrokeTile = HoverTile;
					}
				}
			}
			if (mouse->Button == NLMISC::rightButton)
			{
				// Pick under the cursor: tile mode = the base layer's set; color mode = the
				// vertex color; displace mode = the tile's displace index. Prop: no pick.
				// TODO (cursors): show curs_pick while the right button is held, and put the
				// mode's own shape back on release - the eyedropper is the one paint action
				// whose cursor art we already have. Manifest in script_and_ui.cpp above
				// zpSelectMode; the matching release is in the EventMouseUpId branch.
				if (Mode == ModeProp)
					return;
				updateHover();
				if (HaveHover)
				{
					ZPPAINT::CTileDescP desc;
					Core->getTile(HoverZone, HoverTile, desc);
					// Eyedropper picks mutate live paint state; record as abs setters so
					// replays end with the same state (the ops themselves embed their
					// values, but manual continuation after a replay would diverge).
					if (Mode == ModeTile)
					{
						if (!desc.isEmpty())
						{
							int ts = Core->tileSetOfTile(desc.getLayer(0).Tile);
							if (ts >= 0 && ts != CurTileSet)
							{
								CurTileSet = ts;
								ZPSCRIPT::record(NLMISC::toString("painter.setTileSet(%d)", ts));
							}
						}
					}
					else if (Mode == ModeColor)
					{
						sint32 t = HoverTile;
						uint32 raw;
						if (Core->getColor(HoverZone, (uint)(t / ZP_NUM_TILE_SEL),
						                   (sint32)(t % ZP_NUM_TILE_SEL % ZP_MAX_TILE_IN_PATCH),
						                   (sint32)(t % ZP_NUM_TILE_SEL / ZP_MAX_TILE_IN_PATCH), raw))
						{
							BrushColor = NLMISC::CRGBA((uint8)((raw >> 16) & 0xff), (uint8)((raw >> 8) & 0xff), (uint8)(raw & 0xff), 255);
							ZPSCRIPT::record(NLMISC::toString("painter.setBrushColor(%d, %d, %d)",
								(int)BrushColor.R, (int)BrushColor.G, (int)BrushColor.B));
						}
					}
					else if (Mode == ModeDisplace)
					{
						DisplaceIndex = desc.getDisplace();
						ZPSCRIPT::record(NLMISC::toString("painter.setDisplaceIndex(%d)", (int)DisplaceIndex));
					}
				}
			}
		}
		else if (event == NLMISC::EventMouseUpId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			if (zpWeldDragActive() && (mouse->Button & NLMISC::leftButton))
			{
				NL3D::IDriver *drv = g_PaintCtx.UDriver
					? static_cast<NL3D::CDriverUser *>(g_PaintCtx.UDriver)->getDriver() : NULL;
				zpWeldDragFinish(Camera, drv, mouse->X, mouse->Y);
				return;
			}
			if (zpPatchGizmoDragging() && (mouse->Button & NLMISC::leftButton))
			{
				zpPatchGizmoEndDrag();
				return;
			}
			// Mask test, not equality: NeL folds the modifier keys into Button, so a release
			// with Ctrl or Shift held would fail an exact compare and leave the stroke open -
			// the next press would then stack a second stroke onto it.
			if ((mouse->Button & NLMISC::leftButton) && Pressed)
			{
				Pressed = false;
				Core->endStroke();
				ZPSCRIPT::record("painter.endStroke()");
			}
		}
		else if (event == NLMISC::EventMouseMoveId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			if (zpWeldDragActive())
			{
				MouseX = mouse->X;
				MouseY = mouse->Y;
				zpWeldDragUpdate(MouseX, MouseY);
				return;
			}
			if (zpPatchGizmoDragging())
			{
				MouseX = mouse->X;
				MouseY = mouse->Y;
				zpPatchGizmoUpdateDrag(Camera, Viewport, MouseX, MouseY);
				return;
			}
			MouseX = mouse->X;
			MouseY = mouse->Y;
			if (Mode == ModeProp)
				return; // hover updated each frame in the main loop
			if (Pressed && (mouse->Button & NLMISC::leftButton))
			{
				updateHover();
				if (HaveHover && (HoverTile != StrokeTile || HoverZone != StrokeZone) && !Core->zoneFrozen(HoverZone))
				{
					if (Mode == ModeTile)
					{
						std::string err;
						if (Core->opTileStroke(HoverZone, HoverTile, CurTileSet, Mode256, false, err))
						{
							ZPSCRIPT::record(NLMISC::toString("painter.tileStroke(%u, %u, %u, %u, %d, %s, true)",
								HoverZone, (uint)(HoverTile / ZP_NUM_TILE_SEL),
								(uint)(HoverTile % ZP_NUM_TILE_SEL % ZP_MAX_TILE_IN_PATCH),
								(uint)(HoverTile % ZP_NUM_TILE_SEL / ZP_MAX_TILE_IN_PATCH), CurTileSet,
								Mode256 ? "true" : "false"));
							StrokeZone = HoverZone;
							StrokeTile = HoverTile;
						}
					}
					else
					{
						paintAtHover(true);
						StrokeZone = HoverZone;
						StrokeTile = HoverTile;
					}
				}
			}
		}
}

// F12 screenshot convenience (CNELU::screenshot port; uses the unwrapped IDriver).
void zpViewerScreenshot(NL3D::IDriver *driver, NLMISC::CEventListenerAsync &async)
{
	(void)async; // the binding is read through the key table (g_ViewerAsync)
	if (!zpKeyPushed(ZPK_Screenshot))
		return;
	NLMISC::CBitmap btm;
	driver->getBuffer(btm);
	std::string filename = NLMISC::CFile::findNewFile("screenshot.tga");
	NLMISC::COFile fs(filename);
	btm.writeTGA(fs, 24);
	nlinfo("Screenshot '%s' saved", filename.c_str());
}


// ---------------------------------------------------------------------------------------------
// Zoom Extents Selected

/** Grow `box` over one zone's patch hull; returns false when the zone has no geometry. */
static bool zpZoneBox(const SPaintZone &pz, NLMISC::CAABBox &box, bool &init)
{
	bool any = false;
	for (size_t p = 0; p < pz.Patches.size(); ++p)
	{
		const NL3D::CBezierPatch &bp = pz.Patches[p].Patch;
		for (uint v = 0; v < 4; ++v)
		{
			if (!init) { box.setCenter(bp.Vertices[v]); box.setHalfSize(NLMISC::CVector::Null); init = true; }
			else box.extend(bp.Vertices[v]);
			any = true;
		}
		for (uint v = 0; v < 8; ++v) box.extend(bp.Tangents[v]);
		for (uint v = 0; v < 4; ++v) box.extend(bp.Interiors[v]);
	}
	return any;
}

/**
 * What Z frames, in priority order:
 * 1. the last edit (paint modes: "centre on what I just worked on")
 * 2. the tile under the cursor - useful before anything has been painted
 * 3. the Prop-mode zone selection
 * 4. every editable zone (plain Zoom Extents)
 *
 * Note for feel-testing: in Prop mode a stale last-edit from before the mode switch still
 * wins over the zone selection. If that reads wrong in practice, move case 3 to the front
 * when the mode is ModeProp - it is a one-line reorder.
 */
static void zpFrameTarget(NL3D::CNavMouseListener &nav, const CPaintMouseListener &paint,
                          const std::vector<SPaintZone> &zones, ZPPAINT::CPaintCore *core)
{
	NLMISC::CAABBox box;

	// 1. last edit (world point + the extent that edit covered)
	if (core)
	{
		NLMISC::CVector pos;
		float radius = 0.f;
		if (core->lastEditPos(pos, radius))
		{
			box.setCenter(pos);
			// A bare tile is a small target; a little margin keeps its neighbours in frame.
			const float half = radius > 0.f ? radius * 2.f : 4.f;
			box.setHalfSize(NLMISC::CVector(half, half, half));
			nav.frameBox(box);
			printf("view: framed last edit (%.1f, %.1f, %.1f)\n", pos.x, pos.y, pos.z);
			return;
		}
	}

	// 2. hovered tile
	if (core && paint.HaveHover)
	{
		NLMISC::CVector c[4];
		if (core->tileCorners(paint.HoverZone, paint.HoverTile, c) == 0)
		{
			box.setCenter(c[0]);
			box.setHalfSize(NLMISC::CVector::Null);
			for (int i = 1; i < 4; ++i) box.extend(c[i]);
			nav.frameBox(box);
			printf("view: framed hovered tile (zone %u)\n", paint.HoverZone);
			return;
		}
	}

	// 3. Prop-mode zone selection
	if (g_HavePropSelection)
	{
		if (const SPaintZone *pz = zpFindPaintZone(g_SelectedZoneId))
		{
			bool init = false;
			if (zpZoneBox(*pz, box, init) && init)
			{
				nav.frameBox(box);
				printf("view: framed selected zone %u '%s'\n", pz->ZoneId, pz->Name.c_str());
				return;
			}
		}
	}

	// 4. every editable zone; frozen context only when nothing is editable (same two-pass
	// rule the session's initial camera placement uses).
	bool init = false;
	for (int pass = 0; pass < 2 && !init; ++pass)
	{
		for (size_t i = 0; i < zones.size(); ++i)
		{
			if (pass == 0 && zones[i].Frozen) continue;
			zpZoneBox(zones[i], box, init);
		}
	}
	if (!init)
		return;
	nav.frameBox(box);
	printf("view: framed all editable zones\n");
}

/**
 * Drop every global that points at runViewer's frame-local objects (the UI bridges, the
 * paint context, the script pump/host, the async listener, the session bank).
 *
 * Shared by the normal exit and BOTH catch blocks: the catch paths used to clear only the
 * paint bridge + paint ctx, leaving the session-board bridge, the script pump context and
 * g_ScriptHost.bridge pointing at destroyed stack objects while main() carried on into the
 * post-viewer save/teardown path.
 */
static void zpDropViewerFrameState()
{
	ZPUI::setSessionBoardBridge(NULL);
	ZPUI::setSessionBoardVisible(false);
	ZPUI::setPaintUIBridge(NULL);
	g_SessionBank = NULL;
	g_PaintCtx = SPaintCtx();
	g_SessionOpsAvailable = false;
	g_PumpCtx = SScriptPumpCtx();
	g_ScriptHost.bridge = NULL; // pointed at the frame's paintBridge
	g_ViewerAsync = NULL;
}

// Shared viewer host: when externalDriver is non-NULL, runViewer uses it and does not
// create/release the driver (startup flow owns one UDriver for screens + viewer). When
// externalEditorUI is non-NULL it is reused (already init'd); otherwise a local CEditorUI
// is constructed and shut down here. Headless modes never call runViewer.
int runViewer(std::vector<SPaintZone> &zones, NL3D::CTileBank &bank, ZPPAINT::CPaintCore *core,
                     PMAXLOAD::SLoadedMax &lm,
                     const std::string &screenshotPath, const std::string &fontPath,
                     const std::string &scriptPath, const std::string &savePath,
                     NL3D::UDriver *externalDriver,
                     ZPUI::CEditorUI *externalEditorUI)
{
	// Camera/hotspot bbox in world space. Frame the EDITABLE zones only (the plugin's
	// hotspot was the selection center) - frozen context stays visible around them but a
	// stray context file must not be able to displace the whole view. Fall back to all
	// zones when nothing is editable.
	NLMISC::CAABBox bbox;
	bool bboxInit = false;
	for (int pass = 0; pass < 2 && !bboxInit; ++pass)
	for (size_t i = 0; i < zones.size(); ++i)
	{
		if (pass == 0 && zones[i].Frozen) continue;
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
	}
	NLMISC::CVector center = bbox.getCenter();

	NL3D::UDriver *udriver = externalDriver;
	const bool ownsDriver = (externalDriver == NULL);
	NL3D::UScene *uscene = NULL;
	ZPUI::CEditorUI localEditorUI;
	ZPUI::CEditorUI *editorUI = externalEditorUI ? externalEditorUI : &localEditorUI;
	const bool ownsEditorUI = (externalEditorUI == NULL);
	try
	{
		// UDriver/UScene port of the previous CNELU init (needed for NLGUI's CViewRenderer,
		// which requires UDriver). Low-level landscape assembly unwraps to IDriver/CScene.
		NL3D::CScene::registerBasics();
		NL3D::CViewport viewport;
		if (ownsDriver)
		{
			udriver = NL3D::UDriver::createDriver(0, false, 0);
			if (!udriver)
			{
				fprintf(stderr, "ERROR: UDriver::createDriver failed (no 3D driver?)\n");
				return 1;
			}
			if (!udriver->setDisplay(NL3D::UDriver::CMode(kMainWidth, kMainHeight, 32, true)))
			{
				fprintf(stderr, "ERROR: UDriver::setDisplay failed\n");
				delete udriver;
				return 1;
			}
		}
		else if (!udriver)
		{
			fprintf(stderr, "ERROR: runViewer: external driver is null\n");
			return 1;
		}
		// Window title: zone_painter - <editable zone(s)> (+N neighbors / xN instances)
		{
			std::string title = "zone_painter";
			if (g_EditableFiles.size() > 1)
			{
				title += " - ";
				for (size_t i = 0; i < g_EditableFiles.size() && i < 4; ++i)
				{
					if (i) title += "+";
					title += g_EditableFiles[i].Basename;
				}
				if (g_EditableFiles.size() > 4)
					title += NLMISC::toString("+%u", (uint)(g_EditableFiles.size() - 4));
			}
			else if (!g_StartupZone.Basename.empty())
				title += " - " + g_StartupZone.Basename;
			else if (!g_InputPath.empty())
				title += " - " + NLMISC::CFile::getFilename(g_InputPath);
			if (!g_NeighborScenes.empty())
				title += NLMISC::toString(" (+%u neighbors)", (uint)g_NeighborScenes.size());
			if (g_InstanceCount > 1)
				title += NLMISC::toString(" (x%u instances)", g_InstanceCount);
			udriver->setWindowTitle(ucstring(title));
		}
		uscene = udriver->createScene(false);
		if (!uscene)
		{
			fprintf(stderr, "ERROR: UDriver::createScene failed\n");
			if (ownsDriver) { udriver->release(); delete udriver; }
			return 1;
		}
		uscene->setViewport(viewport);

		NL3D::IDriver *driver = static_cast<NL3D::CDriverUser *>(udriver)->getDriver();
		NL3D::CScene &scene = static_cast<NL3D::CSceneUser *>(uscene)->getScene();
		NL3D::CCamera *camera = uscene->getCam().getObjectPtr();
		NL3D::CShapeBank *shapeBank = scene.getShapeBank();
		g_ViewerAsync = &udriver->AsyncListener;

		// The painting landscape (paint.cpp myThread).
		NL3D::CLandscapeModel *theLand = (NL3D::CLandscapeModel *)scene.createModel(NL3D::LandscapeModelId);
		theLand->Landscape.setTileNear(10000.f);
		theLand->Landscape.TileBank = bank;
		theLand->Landscape.enableAutomaticLighting(false);
		theLand->Landscape.setupAutomaticLightDir(g_LightDirection);
		theLand->Landscape.setupStaticLight(g_LightDiffuse, g_LightAmbiant, g_LightMultiply);

		for (size_t i = 0; i < zones.size(); ++i)
		{
			NL3D::CZone zone;
			buildDisplayZone(zones[i], zone);
			if (!theLand->Landscape.addZone(zone))
				fprintf(stderr, "WARNING: addZone failed for zone %u '%s'\n", zones[i].ZoneId, zones[i].Name.c_str());
		}
		theLand->Landscape.setRefineMode(true);

		// Camera looking at the bbox center from a sensible distance (standalone starts from
		// a canonical three-quarter view).
		printf("view: framing bbox min=(%.1f,%.1f,%.1f) max=(%.1f,%.1f,%.1f) radius=%.1f\n",
		       bbox.getMin().x, bbox.getMin().y, bbox.getMin().z,
		       bbox.getMax().x, bbox.getMax().y, bbox.getMax().z, bbox.getRadius());
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
		camera->setTransformMode(NL3D::ITransformable::DirectMatrix);
		camera->setMatrix(camMat);
		camera->setPerspective(75.f * (float)NLMISC::Pi / 180.f, 1.33f, 0.1f, 10000.f);

		// In-engine NLGUI shell (optional: soft-fails if atlas/XML missing; keyboard+HUD remain).
		// Init before paint/nav listeners so GUI event routing is registered first.
		// When the startup flow already inited the shell, reuse it (do not re-init).
		// NLGUI always needs a TTF (panel labels). Default a system font when --font is
		// absent; the 3D HUD textContext below stays OFF unless --font was explicit .
		if (ownsEditorUI)
		{
			std::string uiFont = fontPath;
			if (uiFont.empty())
			{
				const char *sysFont = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
				if (NLMISC::CFile::fileExists(sysFont))
					uiFont = sysFont;
			}
			editorUI->init(udriver, uiFont);
		}
		// Ensure the Painter panel is active for the viewer session (startup screens hide it).
		editorUI->setVisible(true);
		ZPUI::startupHideAllScreens();
		ZPUI::startupShowPainter(true);

		// Navigation: middle-button set on the view-target model (nel/3d/nav_mouse_listener.h).
		// The left button is deliberately NOT bound here - it belongs to paint / select /
		// transform, and Ctrl+Left / Alt+Left are reserved for selection add / subtract.
		NL3D::CNavMouseListener mouseListener;
		mouseListener.setMatrix(camMat);
		mouseListener.setFrustrum(camera->getFrustum());
		mouseListener.setViewport(viewport);
		mouseListener.setTarget(center);
		mouseListener.addToServer(udriver->EventServer);

		CWindowCloseListener closeListener;
		udriver->EventServer.addListener(NLMISC::EventDestroyWindowId, &closeListener);
		udriver->EventServer.addListener(NLMISC::EventCloseWindowId, &closeListener);

		// Paint listener: live-landscape mirror + the mouse op path
		CPaintMouseListener paintListener;
		// Shared paint actions (keys + NLGUI); bridge lives for the viewer session.
		ZPUI::SPaintUIBridge paintBridge;
		g_PaintCtx.Active = (core != NULL);
		g_PaintCtx.Core = core;
		g_PaintCtx.Paint = &paintListener;
		g_PaintCtx.InputPath = g_InputPath;
		g_PaintCtx.SavePath = savePath;
		g_PaintCtx.Scene = lm.Scene;
		g_PaintCtx.InteractiveSave = g_InteractiveSave;
		g_PaintCtx.WantThumbnail = g_CliWantThumbnail; // CLI default; modal may toggle
		g_PaintCtx.UDriver = udriver;
		g_PaintCtx.UScene = uscene;
		g_PaintCtx.Land = theLand;
		g_PaintCtx.Camera = camera;
		g_PaintCtx.Zones = &zones;
		g_PaintCtx.Bank = &bank;
		g_PaintCtx.BankPath = g_BankPath;
		g_PaintCtx.AvailableSeasons = &g_AvailableSeasons;
		paintBridge.selectMode = zpSelectMode;
		paintBridge.selectSubObject = zpSelectSubObject;
		core->setGeomChangedCb(zpGeomVertChanged);
		core->setRpStateChangedCb(zpRpStateChanged);
		core->setTopoRestoreCb(zpTopoRestore);
		paintBridge.patchBind = zpPatchBindClicked;
		paintBridge.patchUnbind = zpPatchUnbindClicked;
		paintBridge.patchNoSmooth = zpPatchNoSmoothClicked;
		paintBridge.patchDelete = zpPatchDeleteClicked;
		paintBridge.patchTurnCcw = zpPatchTurnCcwClicked;
		paintBridge.patchTurnCw = zpPatchTurnCwClicked;
		paintBridge.patchSubdivide = zpPatchSubdivideClicked;
		paintBridge.patchWeld = zpPatchWeldClicked;
		paintBridge.patchAddQuad = zpPatchAddQuadClicked;
		paintBridge.patchDetach = zpPatchDetachClicked;
		paintBridge.patchElement = zpPatchElementClicked;
		paintBridge.moveToZoneDir = zpMoveToZoneDirClicked;
		paintBridge.weldTargetToggle = zpWeldTargetToggleClicked;
		paintBridge.patchWeldThreshold = zpPatchWeldThresholdClicked;
		paintBridge.patchFilterVertsToggle = zpPatchFilterVertsClicked;
		paintBridge.patchFilterVecsToggle = zpPatchFilterVecsClicked;
		paintBridge.patchLockHandlesToggle = zpPatchLockHandlesClicked;
		paintBridge.arrowsToggle = zpToggleShowArrows;
		paintBridge.patchSmGroup = zpPatchSmGroupClicked;
		paintBridge.patchSmGroupClear = zpPatchSmGroupClearClicked;
		paintBridge.patchTessDelta = zpPatchTessDeltaClicked;
		paintBridge.patchBalance = zpPatchBalanceClicked;
		paintBridge.selectTileSetDelta = zpSelectTileSetDelta;
		paintBridge.selectTileSetAbs = zpSelectTileSetAbs;
		paintBridge.toggleTileSize = zpToggleTileSize;
		paintBridge.brushSizeDelta = zpBrushSizeDelta;
		paintBridge.groupDelta = zpGroupDelta;
		paintBridge.toggleLockBorders = zpToggleLockBorders;
		paintBridge.undo = zpUndo;
		paintBridge.redo = zpRedo;
		paintBridge.fill = zpFill;
		paintBridge.save = zpSaveDirect;
		paintBridge.saveTo = zpSaveTo;
		paintBridge.saveOverwrite = zpSaveOverwrite;
		paintBridge.saveFileOverwrite = zpSaveFileOverwrite;
		paintBridge.saveFileCopy = zpSaveFileCopy;
		paintBridge.fileDir = zpFileDir;
		paintBridge.seasonNext = zpSeasonNext;
		paintBridge.seasonSelect = zpSeasonSelect;
		paintBridge.seasonMenuFill = zpSeasonMenuFill;
		paintBridge.selectPivotMode = zpSetPivotMode;
		paintBridge.userPivotToSelection = zpUserPivotToSelection;
		paintBridge.colorRadiusDelta = zpColorRadiusDelta;
		paintBridge.hardnessDelta = zpHardnessDelta;
		paintBridge.opacityDelta = zpOpacityDelta;
		paintBridge.cycleBrushMask = zpCycleBrushMask;
		paintBridge.toggleMaskMode = zpToggleMaskMode;
		paintBridge.displaceIndexDelta = zpDisplaceIndexDelta;
		paintBridge.displaceIndexAbs = zpDisplaceIndexAbs;
		paintBridge.togglePalette = zpTogglePalette;
		paintBridge.toggleBoard = zpToggleBoard;
		paintBridge.setBrushColor = zpSetBrushColor;
		paintBridge.propRotateDelta = zpPropRotateDelta;
		paintBridge.propToggleSymmetry = zpPropToggleSymmetry;
		paintBridge.propTogglePassable = zpPropTogglePassable;
		paintBridge.propToggleUseBBox = zpPropToggleUseBBox;
		paintBridge.setTileSize256 = zpSetTileSize256;
		paintBridge.setHardnessAbs = zpHardnessAbs;
		paintBridge.setOpacityAbs = zpOpacityAbs;
		paintBridge.setColorRadiusAbs = zpColorRadiusAbs;
		ZPUI::setPaintUIBridge(&paintBridge);

		// painterscript host : viewer capabilities + bridge-backed state
		g_PumpCtx.Driver = udriver;
		g_PumpCtx.Scene = uscene;
		g_PumpCtx.Ui = editorUI;
		g_PumpCtx.Nav = &mouseListener;
		g_PumpCtx.LastPump = 0;
		g_ScriptCancel = false;
		g_ScriptScreenshotFn = zpScriptScreenshotImpl;
		g_ScriptPumpFn = zpScriptPumpImpl;
		zpInstallScriptHost(&paintBridge);

		// Session board bridge (continent working set / ecosystem scratch)
		ZPUI::SSessionBoardBridge sessionBridge;
		if (!g_StartupWorld.MaxDir.empty()
		    && (g_StartupWorld.Kind == ZPWS::Continent || g_StartupWorld.Kind == ZPWS::Ecosystem))
		{
			sessionBridge.World = &g_StartupWorld;
			if (g_StartupWorld.Kind == ZPWS::Continent)
			{
				sessionBridge.getCellState = sessionGetCellState;
				g_SessionOpsAvailable = true;
				sessionBridge.openZone = sessionOpenZone;
				sessionBridge.closeZone = sessionCloseZone;
				sessionBridge.saveZone = sessionSaveZone;
				sessionBridge.toggleEditable = sessionToggleEditable;
				sessionBridge.isDirty = sessionIsDirty;
				sessionBridge.isOpen = sessionIsOpen;
				sessionBridge.isEditable = sessionIsEditable;
			}
			else
			{
				sessionBridge.getCellState = scratchGetCellState;
				sessionBridge.scratchPlace = scratchPlace;
				sessionBridge.scratchRotate = scratchRotate;
				sessionBridge.scratchMirror = scratchMirror;
				sessionBridge.scratchRemove = scratchRemove;
				sessionBridge.scratchGetInstance = scratchGetInstance;
				sessionBridge.scratchGetInstanceOrigin = scratchGetInstanceOrigin;
				sessionBridge.scratchPlaceContext = scratchPlaceContext;
				sessionBridge.scratchRemoveContext = scratchRemoveContext;
				sessionBridge.scratchGetContext = scratchGetContext;
				// Eco multi-editable: board placement of open files + the continent
				// per-file ops (close/save/toggle are basename-addressed and generic).
				sessionBridge.scratchOpenEditable = scratchOpenEditable;
				sessionBridge.scratchGetEditableAt = scratchGetEditableAt;
				sessionBridge.scratchContextToEditable = scratchContextToEditable;
				// instance any open brick
				sessionBridge.scratchPlaceInstanceOf = scratchPlaceInstanceOf;
				sessionBridge.scratchOpenFileCount = scratchOpenFileCount;
				sessionBridge.scratchGetInstanceSource = scratchGetInstanceSource;
				// per-cell saved-neighbor offers + picker priority
				sessionBridge.scratchGetHintAt = scratchGetHintAt;
				sessionBridge.scratchHintNames = scratchHintNames;
				// context brick rotate/mirror
				sessionBridge.scratchRotateContext = scratchRotateContext;
				sessionBridge.scratchMirrorContext = scratchMirrorContext;
				sessionBridge.scratchGetContextTransform = scratchGetContextTransform;
				// board drag move/copy
				sessionBridge.scratchDragDrop = scratchDragDrop;
				g_SessionOpsAvailable = true;
				sessionBridge.closeZone = sessionCloseZone;
				sessionBridge.saveZone = sessionSaveZone;
				sessionBridge.toggleEditable = sessionToggleEditable;
				sessionBridge.isDirty = sessionIsDirty;
				sessionBridge.isOpen = sessionIsOpen;
				sessionBridge.isEditable = sessionIsEditable;
				sessionBridge.FootprintCellsW = scratchFw();
				sessionBridge.FootprintCellsH = scratchFh();
				sessionBridge.FootprintMask = g_FootprintMask.empty() ? NULL : &g_FootprintMask;
			}
			ZPUI::setSessionBoardBridge(&sessionBridge);
		}
		else
			ZPUI::setSessionBoardBridge(NULL);
		// Session rebuild params
		g_SessionBank = &bank;
		// cellSize/snap/lockBorders captured later via globals set before runViewer

		if (core)
		{
			core->attachLandscape(&theLand->Landscape);
			paintListener.Core = core;
			paintListener.Camera = camera;
			paintListener.EditorUI = editorUI;
			paintListener.Viewport = viewport;
			paintListener.BrushColor = g_ViewerBrushColor;
			paintListener.BrushRadius = g_ViewerBrushRadius;
			paintListener.BrushHardness = g_ViewerBrushHardness;
			paintListener.BrushOpacity = g_ViewerBrushOpacity;
			paintListener.DisplaceIndex = g_ViewerDisplaceIndex;
			udriver->EventServer.addListener(NLMISC::EventMouseDownId, &paintListener);
			udriver->EventServer.addListener(NLMISC::EventMouseUpId, &paintListener);
			udriver->EventServer.addListener(NLMISC::EventMouseMoveId, &paintListener);
			// Preload flush (plugin preloadTiles): all tile sets into the driver
			if (g_PreloadTiles)
				core->preloadTiles(driver);
			// Tileset thumbnail palette - bank textures already resolved
			zpRebuildTilesetPalette();
		}

		// Scene point lights (CPaintLight parity - unconditional in the plugin's myThread)
		{
			uint nPaintLights = ZPCTX::setupPaintLights(lm, theLand->Landscape, scene);
			if (g_verbose || nPaintLights)
				printf("paint lights: %u point-light models\n", nPaintLights);
		}

		// Include-meshes context display (the plugin's includeMeshes branch: meshes + scene
		// ambient + driver lights)
		if (g_IncludeMeshes)
		{
			// Out-of-the-box texture resolution: authored material paths -> DBPATH -> CPath,
			// plus the ecosystem's converted map dir and the seasonal-name fallback
			uint texResolved = 0, texMissing = 0;
			uint texDirs = ZPCTX::registerContextTexturePaths(lm, g_InputPath, g_BankPath, texResolved, texMissing);
			printf("context textures: %u authored paths resolved, %u unresolved, %u directories registered\n",
			       texResolved, texMissing, texDirs);
			ZPCTX::SContextStats ctxStats;
			ZPCTX::addContextMeshes(lm, &scene, shapeBank, theLand, ctxStats);
			uint shapeTexResolved = 0, shapeTexMissing = 0;
			ZPCTX::resolveContextShapeTextures(ctxStats, shapeTexResolved, shapeTexMissing);
			printf("context shape textures: %u resolved, %u missing\n", shapeTexResolved, shapeTexMissing);
			NLMISC::CRGBA ambient(0, 0, 0);
			bool haveAmbient = ZPCTX::decodeSceneAmbient(lm, ambient);
			if (haveAmbient)
				driver->setAmbientColor(ambient);
			uint nDriverLights = ZPCTX::setupDriverLights(lm, driver);
			printf("context meshes: %u built, %u skipped, %u filtered (hidden %u, collision %u, accel %u, class %u); driver lights: %u; ambient: %s\n",
			       ctxStats.Built, ctxStats.Skipped, ctxStats.Filtered,
			       ctxStats.FilteredHidden, ctxStats.FilteredCollision, ctxStats.FilteredAccel, ctxStats.FilteredClass,
			       nDriverLights,
			       haveAmbient ? NLMISC::toString("%u,%u,%u", ambient.R, ambient.G, ambient.B).c_str() : "default (not decoded)");
		}

		// 3D HUD overlay text is opt-in via --font. Panel status/hints already cover
		// the same info; the HUD is a tiny dev aid only.
		NL3D::CFontManager fontManager;
		NL3D::CTextContext textContext;
		bool hudText = false;
		if (!fontPath.empty() && NLMISC::CFile::fileExists(fontPath))
		{
			textContext.init(driver, &fontManager);
			textContext.setFontGenerator(fontPath);
			textContext.setHotSpot(NL3D::CComputedString::TopLeft);
			textContext.setColor(NLMISC::CRGBA(255, 255, 255));
			textContext.setFontSize(16);
			hudText = true;
		}

		theLand->enableAdditive(true);

		// User startup script: --startup-lua path, else ./zone_painter_startup.lua
		// when present (same convention as keys/vars cfg). Runs once per viewer session,
		// before any CLI scripted pre-pass; errors are reported but never abort the viewer.
		if (core)
		{
			std::string startupLua = g_StartupLuaPath;
			bool explicitStartup = !startupLua.empty();
			if (!explicitStartup && NLMISC::CFile::fileExists("zone_painter_startup.lua"))
				startupLua = "zone_painter_startup.lua";
			if (!startupLua.empty())
			{
				if (explicitStartup && !NLMISC::CFile::fileExists(startupLua))
					fprintf(stderr, "WARNING: --startup-lua: no such file: %s\n", startupLua.c_str());
				else
				{
					printf("startup script: %s\n", startupLua.c_str());
					ZPSCRIPT::runFile(startupLua);
				}
			}
		}

		// Scripted pre-pass (the ops mirror straight into the attached live landscape)
		g_ViewerScriptRc = 0;
		if (core && !scriptPath.empty())
			g_ViewerScriptRc = runPaintScript(*core, scriptPath);
		if (core && g_ViewerScriptRc == 0 && !g_LuaScriptPath.empty())
			g_ViewerScriptRc = ZPSCRIPT::runFile(g_LuaScriptPath);

		if (!screenshotPath.empty())
		{
			// One refined frame -> .tga -> exit (the visual gate). GUI is drawn into this
			// frame so screenshots show the Painter panel over the terrain.
			udriver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
			uscene->render();
			theLand->Landscape.setRefineMode(false);
			theLand->Landscape.refineAll(pos);
			udriver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
			uscene->render();
			// Dev-only: ZONE_PAINTER_SHOT_MODE=color|displace selects mode for panel shots .
			// ZONE_PAINTER_ROLLOUTS=all force-opens every rollout (handled in syncPanelFromBridge).
			{
				const char *shotMode = getenv("ZONE_PAINTER_SHOT_MODE");
				if (shotMode && core)
				{
					if (strcmp(shotMode, "color") == 0 || strcmp(shotMode, "1") == 0)
						zpSelectMode(CPaintMouseListener::ModeColor);
					else if (strcmp(shotMode, "displace") == 0 || strcmp(shotMode, "2") == 0)
						zpSelectMode(CPaintMouseListener::ModeDisplace);
					else if (strcmp(shotMode, "tile") == 0 || strcmp(shotMode, "0") == 0)
						zpSelectMode(CPaintMouseListener::ModeTile);
					else if (strcmp(shotMode, "prop") == 0 || strcmp(shotMode, "3") == 0)
						zpSelectMode(CPaintMouseListener::ModeProp);
				}
				// Dev-only: ZONE_PAINTER_PROP_SELECT=zoneId forces Prop selection for shots .
				// ZONE_PAINTER_PROP_HOVER=zoneId forces hover outline; ZONE_PAINTER_PROP_RO=1
				// pretends a read-only click status (status line).
				{
					const char *psel = getenv("ZONE_PAINTER_PROP_SELECT");
					if (psel && psel[0] && core)
					{
						uint zid = 0;
						NLMISC::fromString(std::string(psel), zid);
						if (zpZoneIsPropSelectable(zid))
						{
							g_HavePropSelection = true;
							g_SelectedZoneId = zid;
							const SPaintZone *pz = zpFindPaintZone(zid);
							g_PropStatusMsg = pz ? ("selected " + pz->Name)
							                     : NLMISC::toString("selected zone %u", zid);
						}
					}
					const char *phov = getenv("ZONE_PAINTER_PROP_HOVER");
					if (phov && phov[0] && core)
					{
						uint zid = 0;
						NLMISC::fromString(std::string(phov), zid);
						if (zpZoneIsPropSelectable(zid))
						{
							paintListener.HaveHover = true;
							paintListener.HoverZone = zid;
							paintListener.HoverTile = 0;
						}
					}
					const char *pro = getenv("ZONE_PAINTER_PROP_RO");
					if (pro && pro[0] && pro[0] != '0')
						g_PropStatusMsg = "read-only";
				}
			}
			// Dev-only: ZONE_PAINTER_PANEL_ACTION_TEST drives a panel action-handler path once
			// (proves panel → same shared handler as keys). Values: hardness+|hardness-|opacity+|
			// opacity-|radius+|radius-|displace+|displace-|displace:N|color:rrggbb|
			// mask|maskmode|tileset:N|palette|colorpicker
			{
				const char *act = getenv("ZONE_PAINTER_PANEL_ACTION_TEST");
				if (act && act[0] && core)
				{
					ZPUI::SPaintUIBridge *b = ZPUI::getPaintUIBridge();
					const uint hardBefore = paintListener.BrushHardness;
					const uint opacBefore = paintListener.BrushOpacity;
					const float radBefore = paintListener.BrushRadius;
					const uint dispBefore = paintListener.DisplaceIndex;
					const int tsBefore = paintListener.CurTileSet;
					const bool maskModeBefore = core->brushMaskMode();
					const uint crBefore = paintListener.BrushColor.R;
					const uint cgBefore = paintListener.BrushColor.G;
					const uint cbBefore = paintListener.BrushColor.B;
					if (strcmp(act, "hardness+") == 0 && b && b->hardnessDelta) b->hardnessDelta(+51);
					else if (strcmp(act, "hardness-") == 0 && b && b->hardnessDelta) b->hardnessDelta(-51);
					else if (strcmp(act, "opacity+") == 0 && b && b->opacityDelta) b->opacityDelta(+51);
					else if (strcmp(act, "opacity-") == 0 && b && b->opacityDelta) b->opacityDelta(-51);
					else if (strcmp(act, "radius+") == 0 && b && b->colorRadiusDelta) b->colorRadiusDelta(+1);
					else if (strcmp(act, "radius-") == 0 && b && b->colorRadiusDelta) b->colorRadiusDelta(-1);
					else if (strcmp(act, "displace+") == 0 && b && b->displaceIndexDelta) b->displaceIndexDelta(+1);
					else if (strcmp(act, "displace-") == 0 && b && b->displaceIndexDelta) b->displaceIndexDelta(-1);
					else if (strncmp(act, "displace:", 9) == 0 && b && b->displaceIndexAbs)
					{
						int idx = 0;
						NLMISC::fromString(std::string(act + 9), idx);
						b->displaceIndexAbs(idx);
					}
					else if (strncmp(act, "color:", 6) == 0 && b && b->setBrushColor)
					{
						// color:rrggbb hex (6 digits)
						unsigned rgb = 0;
						sscanf(act + 6, "%x", &rgb);
						b->setBrushColor((int)((rgb >> 16) & 0xff),
						                 (int)((rgb >> 8) & 0xff),
						                 (int)(rgb & 0xff));
					}
					else if (strcmp(act, "mask") == 0 && b && b->cycleBrushMask) b->cycleBrushMask();
					else if (strcmp(act, "maskmode") == 0 && b && b->toggleMaskMode) b->toggleMaskMode();
					else if (strncmp(act, "tileset:", 8) == 0 && b && b->selectTileSetAbs)
					{
						int idx = 0;
						NLMISC::fromString(std::string(act + 8), idx);
						b->selectTileSetAbs(idx);
					}
					else if (strcmp(act, "palette") == 0 && b && b->togglePalette)
						b->togglePalette();
					else if (strcmp(act, "colorpicker") == 0)
						ZPUI::toggleColorPicker();
					printf("panel-action-test %s: hardness %u->%u opacity %u->%u radius %.1f->%.1f "
					       "displace %u->%u tileset %d->%d maskmode %d->%d mask='%s' "
					       "color %u,%u,%u->%u,%u,%u\n",
					       act, hardBefore, paintListener.BrushHardness,
					       opacBefore, paintListener.BrushOpacity,
					       radBefore, paintListener.BrushRadius,
					       dispBefore, paintListener.DisplaceIndex,
					       tsBefore, paintListener.CurTileSet,
					       (int)maskModeBefore, (int)core->brushMaskMode(),
					       core->brushMaskName().empty() ? "none" : core->brushMaskName().c_str(),
					       crBefore, cgBefore, cbBefore,
					       paintListener.BrushColor.R, paintListener.BrushColor.G, paintListener.BrushColor.B);
				}
			}
			// Sync panel labels before capture so the screenshot shows live state
			zpFillBridgeState(paintBridge);
			// Dev-only: ZONE_PAINTER_SAVE_MODAL_SHOT=1 opens the Save modal for one frame
			// so interactive save UI can be verified headlessly. Any other value =
			// a basename: opens the per-file "Save as…" form bound to that file .
			{
				const char *modalShot = getenv("ZONE_PAINTER_SAVE_MODAL_SHOT");
				if (modalShot && modalShot[0] && modalShot[0] != '0'
				    && g_PaintCtx.InteractiveSave)
				{
					if (std::string(modalShot) == "1")
						ZPUI::forceShowSaveDialogForShot();
					else
						ZPUI::openSaveDialogForFile(modalShot);
				}
			}
			// Dev-only: ZONE_PAINTER_PALETTE_SHOT=1 opens the Tiles palette for the frame.
			// ZONE_PAINTER_PALETTE_SCROLL=disp scrolls the body to the Displace section.
			{
				const char *palShot = getenv("ZONE_PAINTER_PALETTE_SHOT");
				if (palShot && palShot[0] && palShot[0] != '0')
					ZPUI::setTilesetPaletteVisible(true);
				// Dev-only: ZONE_PAINTER_SCRIPT_SHOT=1 opens the Script window
				const char *scShot = getenv("ZONE_PAINTER_SCRIPT_SHOT");
				if (scShot && scShot[0] && scShot[0] != '0')
					ZPUI::setScriptWindowVisible(true);
				const char *palScroll = getenv("ZONE_PAINTER_PALETTE_SCROLL");
				if (palScroll && (strcmp(palScroll, "disp") == 0 || strcmp(palScroll, "displace") == 0))
					ZPUI::scrollPaletteToDisplaceSection();
			}
			// Dev-only: ZONE_PAINTER_COLOR_PICKER_SHOT=1 opens the brush color picker .
			{
				const char *cpShot = getenv("ZONE_PAINTER_COLOR_PICKER_SHOT");
				if (cpShot && cpShot[0] && cpShot[0] != '0')
					ZPUI::forceShowColorPickerForShot();
			}
			// Dev-only: ZONE_PAINTER_SEASON_NEXT=1 cycles season once before capture (season-follow).
			{
				const char *sn = getenv("ZONE_PAINTER_SEASON_NEXT");
				if (sn && sn[0] && sn[0] != '0')
					zpSeasonNext();
			}
			// Session board env hooks. ZONE_PAINTER_SHOW_BOARD=1 opens the board.
			// Continent: BOARD_ACTION=open|close|save|toggle:BASE
			// Ecosystem: BOARD_ACTION=place|rotate|mirror|remove:cx,cy, plus the context ops
			// (place-context:cx,cy:base, remove-/rotate-/mirror-context:cx,cy, make-editable:cx,cy).
			// ';'-separated action lists run in order.
			// CLOSE_CONFIRM_SHOT / CELL_ACTION_SHOT / INST_ACTION_SHOT force modals for screenshots.
			{
				const char *showB = getenv("ZONE_PAINTER_SHOW_BOARD");
				if (showB && showB[0] && showB[0] != '0')
					ZPUI::setSessionBoardVisible(true);
				// BOARD_DRAG="fx,fy:tx,ty[:copy]" drives scratchDragDrop headlessly (E2E)
			{
				const char *bdrag = getenv("ZONE_PAINTER_BOARD_DRAG");
				if (bdrag && bdrag[0])
				{
					int fx = 0, fy = 0, tx = 0, ty = 0;
					char copyBuf[16] = { 0 };
					const int n = sscanf(bdrag, "%d,%d:%d,%d:%15s", &fx, &fy, &tx, &ty, copyBuf);
					if (n >= 4)
					{
						std::string derr;
						const bool copy = n >= 5 && copyBuf[0] && copyBuf[0] != '0';
						if (!scratchDragDrop(fx, fy, tx, ty, copy, derr))
							fprintf(stderr, "board-drag test (%d,%d)->(%d,%d)%s: %s\n",
							        fx, fy, tx, ty, copy ? " copy" : "", derr.c_str());
						else
						{
							printf("board-drag test (%d,%d)->(%d,%d)%s: OK\n",
							       fx, fy, tx, ty, copy ? " copy" : "");
							// Mirror the real drag handler so SHOW_BOARD screenshots
							// reflect the post-drag board
							ZPUI::refreshBoardAfterSessionOp();
						}
					}
					else
						fprintf(stderr, "board-drag test: expects fx,fy:tx,ty[:copy]\n");
				}
			}
			const char *bact = getenv("ZONE_PAINTER_BOARD_ACTION");
				if (bact && bact[0])
				{
				// ';'-separated action list - the recorder E2E drives several board
				// ops in one run. Single actions parse exactly as before.
				std::vector<std::string> actList;
				{
					std::string all(bact), cur;
					for (std::string::size_type ai = 0; ai <= all.size(); ++ai)
					{
						char c = ai < all.size() ? all[ai] : ';';
						if (c == ';') { if (!cur.empty()) actList.push_back(cur); cur.clear(); }
						else cur += c;
					}
				}
				for (size_t actIdx = 0; actIdx < actList.size(); ++actIdx)
				{
					const std::string &act = actList[actIdx];
					std::string::size_type colon = act.find(':');
					std::string op = colon == std::string::npos ? act : act.substr(0, colon);
					std::string base = colon == std::string::npos ? std::string() : act.substr(colon + 1);
					std::string err;
					bool ok = false;
					if (op == "open" && !base.empty())
						ok = sessionOpenZone(base, err);
					else if (op == "close" && !base.empty())
						ok = sessionCloseZone(base, /*saveFirst=*/false, /*forceDiscard=*/true, err);
					else if (op == "save" && !base.empty())
						ok = sessionSaveZone(base, err);
					else if (op == "toggle" && !base.empty())
						ok = sessionToggleEditable(base, false, true, err);
					else if (op == "place" || op == "rotate" || op == "mirror" || op == "remove"
					         || op == "place-context" || op == "remove-context"
					         || op == "rotate-context" || op == "mirror-context"
					         || op == "make-editable")
					{
						// place:cx,cy | place-context:cx,cy:basename | remove-context:cx,cy
						if (op == "place-context")
						{
							// base = cx,cy:basename
							std::string::size_type comma = base.find(',');
							std::string::size_type colon2 = base.find(':', comma == std::string::npos ? 0 : comma);
							int cx = 0, cy = 0;
							if (comma != std::string::npos && colon2 != std::string::npos
							    && NLMISC::fromString(base.substr(0, comma), cx)
							    && NLMISC::fromString(base.substr(comma + 1, colon2 - comma - 1), cy))
							{
								std::string bname = base.substr(colon2 + 1);
								ok = scratchPlaceContext(cx, cy, bname, err);
							}
							else
								err = "expected cx,cy:basename";
						}
						else
						{
							int cx = 0, cy = 0;
							std::string::size_type comma = base.find(',');
							if (comma != std::string::npos
							    && NLMISC::fromString(base.substr(0, comma), cx)
							    && NLMISC::fromString(base.substr(comma + 1), cy))
							{
								if (op == "place") ok = scratchPlace(cx, cy, err);
								else if (op == "rotate") ok = scratchRotate(cx, cy, +1, err);
								else if (op == "mirror") ok = scratchMirror(cx, cy, err);
								else if (op == "remove") ok = scratchRemove(cx, cy, err);
								else if (op == "remove-context") ok = scratchRemoveContext(cx, cy, err);
								else if (op == "rotate-context") ok = scratchRotateContext(cx, cy, +1, err);
								else if (op == "mirror-context") ok = scratchMirrorContext(cx, cy, err);
								else if (op == "make-editable") ok = scratchContextToEditable(cx, cy, err);
							}
							else
								err = "expected cx,cy";
						}
					}
					else
						err = "unknown BOARD_ACTION (open|close|save|toggle:base / place|rotate|mirror|remove:cx,cy / place-context:cx,cy:base / remove-context|rotate-context|mirror-context|make-editable:cx,cy; ';'-separated list runs in order)";
					printf("board-action %s: %s%s%s\n", act.c_str(), ok ? "OK" : "FAIL",
					       err.empty() ? "" : " ", err.c_str());
					if (ok)
						ZPUI::refreshSessionBoardStates();
				}
				}
				// Dev-only: ZONE_PAINTER_ZOOM_EXTENTS=1 runs the Z (Zoom Extents Selected)
				// fallback chain once before the capture, so the framing is gateable
				// headlessly (the key itself only exists in the interactive loop).
				{
					const char *ze = getenv("ZONE_PAINTER_ZOOM_EXTENTS");
					if (ze && ze[0] && ze[0] != '0')
						zpFrameTarget(mouseListener, paintListener, zones, core);
				}
				// Dev-only: ZONE_PAINTER_DUMP_RECORDER=1 prints the recorder buffer AFTER the
				// panel/board env hooks ran (board-op recording E2E extracts this dump
				// and replays it - dumping before the board hooks missed their lines).
				{
					const char *dumpRec = getenv("ZONE_PAINTER_DUMP_RECORDER");
					if (dumpRec && dumpRec[0] && dumpRec[0] != '0')
						printf("recorder dump:\n%s--- end recorder ---\n", ZPSCRIPT::recorderText().c_str());
				}
				const char *ccShot = getenv("ZONE_PAINTER_CLOSE_CONFIRM_SHOT");
				if (ccShot && ccShot[0] && ccShot[0] != '0')
				{
					ZPUI::setSessionBoardVisible(true);
					ZPUI::forceShowCloseConfirmForShot(std::string(ccShot) == "1" ? std::string() : std::string(ccShot));
				}
				const char *caShot = getenv("ZONE_PAINTER_CELL_ACTION_SHOT");
				if (caShot && caShot[0] && caShot[0] != '0')
				{
					ZPUI::setSessionBoardVisible(true);
					ZPUI::forceShowCellActionForShot(std::string(caShot) == "1" ? std::string() : std::string(caShot));
				}
				const char *iaShot = getenv("ZONE_PAINTER_INST_ACTION_SHOT");
				if (iaShot && iaShot[0] && iaShot[0] != '0')
				{
					ZPUI::setSessionBoardVisible(true);
					ZPUI::forceShowInstanceActionForShot(
						std::string(iaShot) == "1" ? std::string("I:1,0") : std::string(iaShot));
				}
				const char *ecShot = getenv("ZONE_PAINTER_EMPTY_CELL_SHOT");
				if (ecShot && ecShot[0] && ecShot[0] != '0')
				{
					ZPUI::setSessionBoardVisible(true);
					ZPUI::forceShowEmptyCellForShot(
						std::string(ecShot) == "1" ? std::string("E:1,0") : std::string(ecShot));
				}
				const char *ctxShot = getenv("ZONE_PAINTER_CONTEXT_ACTION_SHOT");
				if (ctxShot && ctxShot[0] && ctxShot[0] != '0')
				{
					ZPUI::setSessionBoardVisible(true);
					ZPUI::forceShowContextActionForShot(
						std::string(ctxShot) == "1" ? std::string() : std::string(ctxShot));
				}
				const char *pickShot = getenv("ZONE_PAINTER_CTX_PICKER_SHOT");
				if (pickShot && pickShot[0] && pickShot[0] != '0')
				{
					ZPUI::setSessionBoardVisible(true);
					int pmode = 0;
					NLMISC::fromString(pickShot, pmode);
					ZPUI::forceShowContextPickerForShot(pmode == 1 || pmode == 2 ? pmode : 0);
				}
				// Open season picker modal for one screenshot frame
				const char *smShot = getenv("ZONE_PAINTER_SEASON_MENU_SHOT");
				if (smShot && smShot[0] && smShot[0] != '0')
				{
					NLGUI::CInterfaceElement *btn = NLGUI::CWidgetManager::getInstance()->getElementFromId(
						"ui:zp:toolbar:header_closed:btn_season");
					NLGUI::CCtrlBase *caller = dynamic_cast<NLGUI::CCtrlBase *>(btn);
					zpSeasonMenuFill(NULL);
					NLGUI::CWidgetManager::getInstance()->enableModalWindow(caller, "ui:zp:season_menu");
				}
			}
			// Refresh bridge after season/palette/board env hooks
			zpFillBridgeState(paintBridge);
			// Capture sequence (restores refine discipline; keeps scene→UI→HUD draw order):
			// clear → scene → setRefineMode(false)+refineAll → clear → scene (refined)
			// → NLGUI → outlines → optional HUD (--font only)
			// → getBuffer (BACK, before swap) → swap.
			// The early seed refine above may leave the backbuffer refined, but a later
			// clear+re-render without a fresh refineAll produced shredded terrain (fresh
			// binary verified). Re-run the full refine pair immediately before overlays so the
			// captured frame is the refined second render. getBuffer before swap so glReadPixels
			// hits the just-drawn back buffer (default read target).
			{
				NLMISC::CMatrix camMat = mouseListener.getViewMatrix();
				camera->setMatrix(camMat);
				const NLMISC::CVector camPos = camMat.getPos();
				// Full refine pair (mode true seed → refineAll → second render). Mode may already
				// be false from the early seed; re-open progressive mode so refineAll rebuilds.
				theLand->Landscape.setRefineMode(true);
				udriver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
				uscene->render();
				theLand->Landscape.setRefineMode(false);
				theLand->Landscape.refineAll(camPos);
				udriver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
				uscene->render();
			}
			editorUI->update();
			editorUI->draw();
			{
				// zpDrawZoneOutline now culls segments that fall under an active GUI
				// window (see its comment) instead of painting over the just-drawn panel, so
				// this can stay after the GUI draw as it always was.
				if (core && paintListener.Mode == CPaintMouseListener::ModeProp)
				{
					if (g_HavePropSelection)
					{
						const SPaintZone *sel = zpFindPaintZone(g_SelectedZoneId);
						if (sel)
							zpDrawZoneOutline(driver, camera, viewport, *sel, NLMISC::CRGBA(255, 170, 40), true);
					}
					if (paintListener.HaveHover
					    && !(g_HavePropSelection && paintListener.HoverZone == g_SelectedZoneId))
					{
						const SPaintZone *hov = zpFindPaintZone(paintListener.HoverZone);
						if (hov)
							zpDrawZoneOutline(driver, camera, viewport, *hov, NLMISC::CRGBA(255, 255, 0), false);
					}
				}
				else if (paintListener.Mode == CPaintMouseListener::ModePatch)
				{
					zpDrawPatchLatticeAll(driver, camera, paintListener.SubObj);
					// Frame arrows under the cage: the display that makes Turn visible.
					zpDrawPatchArrows(driver, camera);
					// Gizmo after the cage so it is never overdrawn by it. Every level that
					// MOVES something gets one - edge and patch selections are projected onto
					// the same vertex set, so the gizmo is already correct for them.
					if (paintListener.SubObj == CPaintMouseListener::SubVertex
					    || paintListener.SubObj == CPaintMouseListener::SubEdge
					    || paintListener.SubObj == CPaintMouseListener::SubPatch)
						zpDrawPatchGizmo(driver, camera, paintListener.MouseX, paintListener.MouseY,
						                 mouseListener.isNavigating(), mouseListener.viewSerial());
					if (paintListener.SubObj == CPaintMouseListener::SubVertex)
						zpDrawWeldDrag(driver, camera);
				}
				if (core && hudText)
				{
					const int mi = paintListener.Mode;
					const char *mname = zpModeName(mi);
					textContext.setColor(NLMISC::CRGBA(255, 255, 255));
					if (paintListener.Mode == CPaintMouseListener::ModePatch)
					{
						textContext.printfAt(0.01f, 0.98f, "[PATCH:%s] sel %u undo %u",
						                     zpSubObjName(paintListener.SubObj),
						                     zpPatchVertSelCount(), core->undoDepth());
						// Same slot Prop mode uses for its status line. Without this the
						// "preview only" notice from a released drag would go nowhere.
						if (!g_PropStatusMsg.empty())
							textContext.printfAt(0.01f, 0.955f, "%s", g_PropStatusMsg.c_str());
					}
					else if (paintListener.Mode == CPaintMouseListener::ModeProp)
					{
						textContext.printfAt(0.01f, 0.98f, "[%s] click zone to select undo %u",
						                     mname, core->undoDepth());
						if (!g_PropStatusMsg.empty())
							textContext.printfAt(0.01f, 0.955f, "%s", g_PropStatusMsg.c_str());
						else if (g_HavePropSelection)
							textContext.printfAt(0.01f, 0.955f, "selected zone %u", g_SelectedZoneId);
						else
							textContext.printfAt(0.01f, 0.955f, "no selection");
					}
					else
						textContext.printfAt(0.01f, 0.98f, "[%s]", mname);
					textContext.printfAt(0.01f, 0.01f, "T/C/D/R mode O board Y season P tiles F10 UI ESC");
				}
			}
			// Read the just-drawn backbuffer BEFORE swap (glReadPixels default = GL_BACK).
			NLMISC::CBitmap btm;
			driver->getBuffer(btm);
			udriver->swapBuffers();
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
			// Pre-capture top-down thumb while the landscape is live (for --save --thumbnail).
			// First file's framing, matching the per-file save paths - NULL would fold
			// instance clones (--place) into the stashed thumbnail.
			if (g_CliWantThumbnail || g_PaintCtx.WantThumbnail)
			{
				NLMISC::CBitmap thumb;
				if (captureTopDownThumbnail(thumb,
				        g_EditableFiles.empty() ? NULL : &g_EditableFiles[0].ZoneIds))
				{
					g_CapturedThumb.swap(thumb);
					g_HaveCapturedThumb = true;
					printf("OK thumbnail pre-capture: %ux%u\n",
					       g_CapturedThumb.getWidth(), g_CapturedThumb.getHeight());
				}
			}
		}
		else
		{
			// MAIN LOOP (paint.cpp: pump, camera from the mouse listener, render; first frame
			// switches refine mode off and computes the full tessellation).
			NLMISC::TTime lastFrameTime = NLMISC::CTime::getLocalTime();
			bool viewerQuit = false;
			do
			{
				// Snapshot the orbit matrix so GUI mouse capture can discard nav deltas
				NLMISC::CMatrix navMatBefore = mouseListener.getViewMatrix();
				udriver->EventServer.pump();
				if (editorUI->wantsMouse())
					mouseListener.setMatrix(navMatBefore);
				// Script-window RUN queues its chunk from inside the pump; execute it here,
				// outside any event dispatch, so painter.pumpUI() can safely pump again.
				ZPSCRIPT::processPendingRun();
				// ESC, the universal cancel, innermost first: abandon a live gizmo drag,
				// else close the session board, else quit the viewer. Without the first
				// step ESC mid-drag would quit the tool out from under the drag.
				if (udriver->AsyncListener.isKeyPushed(NLMISC::KeyESCAPE))
				{
					// Live drags before the idle disarm, same order as the right-click chain:
					// a gizmo drag can be live while target weld is armed, and the cancel must
					// reach the drag first.
					if (zpWeldDragActive())
						zpWeldDragCancel();
					else if (zpPatchGizmoDragging())
						zpPatchGizmoCancelDrag();
					else if (g_WeldTargetArmed)
						zpWeldTargetToggleClicked();
					else if (ZPUI::isSessionBoardVisible())
						ZPUI::setSessionBoardVisible(false);
					else
						viewerQuit = true;
				}

				// Frame dt (the plugin's zoom timing)
				NLMISC::TTime nowTime = NLMISC::CTime::getLocalTime();
				float dt = (float)(nowTime - lastFrameTime) / 1000.f;
				lastFrameTime = nowTime;

				// F10 / ToggleUI: show/hide the NLGUI shell (keys still work either way)
				if (zpKeyPushed(ZPK_ToggleUI))
					editorUI->toggleVisible();
				// P / TogglePalette: show/hide the tileset thumbnail palette
				if (zpKeyPushed(ZPK_TogglePalette))
					zpTogglePalette();
				// O / ToggleBoard: continent working set / ecosystem scratch board
				if (zpKeyPushed(ZPK_ToggleBoard))
					zpToggleBoard();

				// Tile set / mode / brush keys → shared named handlers (same as NLGUI buttons).
				// Everything rides the key table now (see kPainterKeysName / --keys-cfg);
				// only ESC stays hardcoded, as the universal cancel.
				//
				// The table is polled off the driver's async listener, which does not know
				// about NLGUI focus - without this guard, typing "t" into the script window's
				// edit box also switched paint mode.
				const bool guiHasKeyboard =
					NLGUI::CWidgetManager::getInstance()->isKeyboardCaptured();
				if (core && !guiHasKeyboard)
				{
					uint count = core->tileSetCount();
					if (count)
					{
						if (zpKeyPushed(ZPK_TileSetPrev))
							zpSelectTileSetDelta(-1);
						if (zpKeyPushed(ZPK_TileSetNext))
							zpSelectTileSetDelta(+1);
						for (int k = 0; k <= 9; ++k)
							if (k < (int)count && zpKeyDigitPushed((uint)k))
								zpSelectTileSetAbs(k);
					}
					if (zpKeyPushed(ZPK_ToggleTileSize))
						zpToggleTileSize();
					if (zpKeyPushed(ZPK_MModeTile))
						zpSelectMode(CPaintMouseListener::ModeTile);
					if (zpKeyPushed(ZPK_MModeColor))
						zpSelectMode(CPaintMouseListener::ModeColor);
					if (zpKeyPushed(ZPK_MModeDisplace))
						zpSelectMode(CPaintMouseListener::ModeDisplace);
					if (zpKeyPushed(ZPK_MModeProp))
						zpSelectMode(CPaintMouseListener::ModeProp);
					if (zpKeyPushed(ZPK_MModePatch))
						zpSelectMode(CPaintMouseListener::ModePatch);
					// Same physical digit row as the tile sets above. Only one of the two
					// reads it, because the scope masks make the other binding dead in this
					// mode - no ordering trick, no "and not in patch mode" guard.
					for (uint lv = 0; lv < CPaintMouseListener::SubCount; ++lv)
						if (zpKeySubObjPushed(lv))
							zpSelectSubObject((int)lv);
					// W / E / R. R is ModeProp in the paint modes and SCALE here; the
					// scope masks keep exactly one of the two live, same as the digit row.
					if (zpKeyPushed(ZPK_XformMove))
						zpSetXformKind(ZPXF_Move);
					if (zpKeyPushed(ZPK_XformRotate))
						zpSetXformKind(ZPXF_Rotate);
					if (zpKeyPushed(ZPK_XformScale))
						zpSetXformKind(ZPXF_Scale);
					// C in patch scope: the legacy painter's orientation arrows.
					if (zpKeyPushed(ZPK_ToggleArrows))
						zpToggleShowArrows();
					if (zpKeyPushed(ZPK_SizeUp))
						zpBrushSizeDelta(+1);
					if (zpKeyPushed(ZPK_SizeDown))
						zpBrushSizeDelta(-1);
					if (zpKeyPushed(ZPK_GroupUp))
						zpGroupDelta(+1);
					if (zpKeyPushed(ZPK_GroupDown))
						zpGroupDelta(-1);
					// Displace / hardness / opacity / mask: shared handlers (keys + panel)
					if (zpKeyPushed(ZPK_DisplacePrev))
						zpDisplaceIndexDelta(-1);
					if (zpKeyPushed(ZPK_DisplaceNext))
						zpDisplaceIndexDelta(+1);
					if (zpKeyPushed(ZPK_HardnessUp))
						zpHardnessDelta(+51);
					if (zpKeyPushed(ZPK_HardnessDown))
						zpHardnessDelta(-51);
					if (zpKeyPushed(ZPK_OpacityUp))
						zpOpacityDelta(+51);
					if (zpKeyPushed(ZPK_OpacityDown))
						zpOpacityDelta(-51);
					if (zpKeyPushed(ZPK_SelectColorBrush))
						zpCycleBrushMask();
					if (zpKeyPushed(ZPK_ToggleColorBrushMode))
						zpToggleMaskMode();
					if (zpKeyPushed(ZPK_LockBorders))
						zpToggleLockBorders();
					if (zpKeyPushed(ZPK_SeasonNext))
						zpSeasonNext();
		// Z = Zoom Extents Selected; Shift+Z / Shift+Y = view history.
		// Plain Y still cycles the season: modifier matching is exact, so the
		// Shift bindings and the bare ones cannot fire on each other.
					if (zpKeyPushed(ZPK_ZoomExtentsSel))
						zpFrameTarget(mouseListener, paintListener, zones, core);
					if (zpKeyPushed(ZPK_ViewUndo) && !mouseListener.viewUndo())
						printf("view: nothing to step back to\n");
					if (zpKeyPushed(ZPK_ViewRedo) && !mouseListener.viewRedo())
						printf("view: nothing to step forward to\n");
					// Undo / redo. Commit an open mouse stroke first: opUndo/opRedo do not
					// guard m_CurStroke, and undoing beneath an open stroke corrupts the
					// pairing (paint+undo would no longer restore pristine). Goes through
					// the shared handlers, not Core, so the recorder sees it.
					if (zpKeyPushed(ZPK_Undo) || zpKeyPushed(ZPK_Redo) || zpKeyPushed(ZPK_Redo2))
					{
						if (paintListener.Pressed)
						{
							paintListener.Pressed = false;
							core->endStroke();
							ZPSCRIPT::record("painter.endStroke()");
						}
						if (zpKeyPushed(ZPK_Undo)) zpUndo();
						else zpRedo();
					}
					if (!paintListener.Pressed && !editorUI->wantsMouse())
						paintListener.updateHover();
					else if (editorUI->wantsMouse())
						paintListener.HaveHover = false;
					// Fill0-3 through the shared fill handler
					for (int fillRot = 0; fillRot < 4; ++fillRot)
					{
						if (zpKeyPushed((TPainterKey)(ZPK_Fill0 + fillRot)))
							zpFill(fillRot);
					}
					// Push live state into the NLGUI bridge for two-way panel sync
					zpFillBridgeState(paintBridge);
					// Window title dirty mark : append " *" when any editable is dirty
					{
						std::string title = "zone_painter";
						if (g_EditableFiles.size() > 1)
						{
							title += " - ";
							for (size_t i = 0; i < g_EditableFiles.size() && i < 4; ++i)
							{
								if (i) title += "+";
								title += g_EditableFiles[i].Basename;
							}
							if (g_EditableFiles.size() > 4)
								title += NLMISC::toString("+%u", (uint)(g_EditableFiles.size() - 4));
						}
						else if (!g_StartupZone.Basename.empty())
							title += " - " + g_StartupZone.Basename;
						else if (!g_InputPath.empty())
							title += " - " + NLMISC::CFile::getFilename(g_InputPath);
						if (!g_NeighborScenes.empty())
							title += NLMISC::toString(" (+%u neighbors)", (uint)g_NeighborScenes.size());
						if (g_InstanceCount > 1)
							title += NLMISC::toString(" (x%u instances)", g_InstanceCount);
						if (paintBridge.DirtyFileCount)
							title += " *";
						udriver->setWindowTitle(ucstring(title));
					}
				}

				// Zoom keys (plugin myThread zoom: ZoomSpeed * dt along the view direction;
				// unbound by default, cfg-bindable)
				if (zpKeyDown(ZPK_ZoomIn) || zpKeyDown(ZPK_ZoomOut))
				{
					float zoom = 0.f;
					if (zpKeyDown(ZPK_ZoomIn)) zoom += g_ZoomSpeed * dt;
					if (zpKeyDown(ZPK_ZoomOut)) zoom -= g_ZoomSpeed * dt;
					NLMISC::CMatrix zoomMat = mouseListener.getViewMatrix();
					zoomMat.setPos(zoomMat.getPos() + zoomMat.getJ() * zoom);
					mouseListener.setMatrix(zoomMat);
				}

				NLMISC::CMatrix camKey = mouseListener.getViewMatrix();
				camera->setMatrix(camKey);
				udriver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
				uscene->render();
				// First frame: full refine then re-render so the swapped buffer is not the
				// coarse seed tessellation (same refine-then-second-render as shot).
				if (theLand->Landscape.getRefineMode())
				{
					theLand->Landscape.setRefineMode(false);
					theLand->Landscape.refineAll(camKey.getPos());
					udriver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
					uscene->render();
				}

				// NLGUI after landscape, BEFORE outlines/HUD (solid panel backdrop).
				editorUI->update();
				editorUI->draw();

				// zpDrawZoneOutline now culls segments that fall under an active GUI window
				// (see its comment) instead of painting over the just-drawn panel, so this can stay
				// after the GUI draw as it always was.
				// Hovered tile outline (paint modes) OR zone boundary outline (Prop mode)
				if (core && paintListener.Mode == CPaintMouseListener::ModeProp)
				{
					// Timing note (printed once): boundary-edge outline cost on the working set.
					static bool s_TimedOutline = false;
					NLMISC::TTime t0 = 0;
					if (!s_TimedOutline)
						t0 = NLMISC::CTime::getLocalTime();
					// Selection thick (white/orange); hover thin yellow - only when not the selection.
					if (g_HavePropSelection)
					{
						const SPaintZone *sel = zpFindPaintZone(g_SelectedZoneId);
						if (sel)
							zpDrawZoneOutline(driver, camera, viewport, *sel, NLMISC::CRGBA(255, 170, 40), true);
					}
					if (paintListener.HaveHover
					    && !(g_HavePropSelection && paintListener.HoverZone == g_SelectedZoneId))
					{
						const SPaintZone *hov = zpFindPaintZone(paintListener.HoverZone);
						if (hov)
							zpDrawZoneOutline(driver, camera, viewport, *hov, NLMISC::CRGBA(255, 255, 0), false);
					}
					if (!s_TimedOutline)
					{
						const NLMISC::TTime dt = NLMISC::CTime::getLocalTime() - t0;
						printf("prop-outline: boundary-edge draw ~%u ms (hover+sel) on current working set\n",
						       (uint)dt);
						s_TimedOutline = true;
					}
				}
				else if (paintListener.Mode == CPaintMouseListener::ModePatch)
				{
					zpDrawPatchLatticeAll(driver, camera, paintListener.SubObj);
					// Frame arrows under the cage: the display that makes Turn visible.
					zpDrawPatchArrows(driver, camera);
					// Gizmo after the cage so it is never overdrawn by it. Every level that
					// MOVES something gets one - edge and patch selections are projected onto
					// the same vertex set, so the gizmo is already correct for them.
					if (paintListener.SubObj == CPaintMouseListener::SubVertex
					    || paintListener.SubObj == CPaintMouseListener::SubEdge
					    || paintListener.SubObj == CPaintMouseListener::SubPatch)
						zpDrawPatchGizmo(driver, camera, paintListener.MouseX, paintListener.MouseY,
						                 mouseListener.isNavigating(), mouseListener.viewSerial());
					if (paintListener.SubObj == CPaintMouseListener::SubVertex)
						zpDrawWeldDrag(driver, camera);
				}
				else if (core && paintListener.HaveHover)
				{
					NLMISC::CVector c[4];
					if (core->tileCorners(paintListener.HoverZone, paintListener.HoverTile, c) == 0)
					{
						NLMISC::CVector lift(0.f, 0.f, 0.15f);
						NLMISC::CRGBA col = core->zoneFrozen(paintListener.HoverZone) ? NLMISC::CRGBA(255, 64, 64) : NLMISC::CRGBA(255, 255, 0);
						driver->setupModelMatrix(NLMISC::CMatrix::Identity);
						for (int l = 0; l < 4; ++l)
							NL3D::CDRU::drawLine(c[l] + lift, c[(l + 1) & 3] + lift, col, *driver);
					}
				}

				// HUD text
				if (core && hudText)
				{
					textContext.setColor(NLMISC::CRGBA(255, 255, 255));
					// HUD matches the Painter panel: 1-based set index; name from bridge
					// (includes diffuse-stem fallback when smallbank set names are empty).
					{
						const char *tsName = paintBridge.TileSetName[0]
							? paintBridge.TileSetName : "(unnamed)";
						const uint tsCount = paintBridge.TileSetCount
							? paintBridge.TileSetCount : core->tileSetCount();
						const int tsOneBased = tsCount ? (paintListener.CurTileSet + 1) : 0;
						const int mi = paintListener.Mode;
						const char *mname = zpModeName(mi);
						if (paintListener.Mode == CPaintMouseListener::ModePatch)
						{
textContext.printfAt(0.01f, 0.98f, "[PATCH:%s] sel %u undo %u %s",
                     zpSubObjName(paintListener.SubObj),
                     zpPatchVertSelCount(), core->undoDepth(),
							                     paintBridge.SeasonLabel[0] ? paintBridge.SeasonLabel : "auto");
							if (!g_PropStatusMsg.empty())
								textContext.printfAt(0.01f, 0.955f, "%s", g_PropStatusMsg.c_str());
						}
						else if (paintListener.Mode == CPaintMouseListener::ModeProp)
						{
							textContext.printfAt(0.01f, 0.98f, "[%s] click zone to select undo %u %s",
							                     mname, core->undoDepth(),
							                     paintBridge.SeasonLabel[0] ? paintBridge.SeasonLabel : "auto");
						}
						else
						{
							textContext.printfAt(0.01f, 0.98f, "[%s] TileSet %d/%u %s %s brush %u group %u undo %u%s %s",
							                     mname,
							                     tsOneBased, tsCount, tsName,
							                     paintListener.Mode256 ? "256" : "128", core->brushSize(),
							                     core->tileGroup(), core->undoDepth(),
							                     core->lockBordersOn() ? " LOCK" : "",
							                     paintBridge.SeasonLabel[0] ? paintBridge.SeasonLabel : "auto");
						}
					}
					textContext.printfAt(0.01f, 0.01f, "T/C/D/R mode O board Y season P tiles F10 UI ESC");
					if (paintListener.Mode == CPaintMouseListener::ModeColor)
						textContext.printfAt(0.01f, 0.955f, "color %02x%02x%02x radius %.1fm hardness %u opacity %u mask %s",
						                     paintListener.BrushColor.R, paintListener.BrushColor.G, paintListener.BrushColor.B,
						                     paintListener.BrushRadius, paintListener.BrushHardness, paintListener.BrushOpacity,
						                     core->brushMaskMode() ? core->brushMaskName().c_str() : "off");
					else if (paintListener.Mode == CPaintMouseListener::ModeDisplace)
						textContext.printfAt(0.01f, 0.955f, "displace index %u", paintListener.DisplaceIndex);
					else if (paintListener.Mode == CPaintMouseListener::ModeProp)
					{
						if (!g_PropStatusMsg.empty())
							textContext.printfAt(0.01f, 0.955f, "%s", g_PropStatusMsg.c_str());
						else if (g_HavePropSelection)
							textContext.printfAt(0.01f, 0.955f, "selected zone %u", g_SelectedZoneId);
						else
							textContext.printfAt(0.01f, 0.955f, "no selection");
					}
					if (g_InstanceCount > 1)
						textContext.printfAt(0.01f, 0.905f, "INSTANCED x%u (%ux%u layout; shared paint backing)",
						                     g_InstanceCount, g_InstanceCols, g_InstanceRows);
					if (paintListener.HaveHover && paintListener.Mode != CPaintMouseListener::ModeProp)
					{
						sint32 t = paintListener.HoverTile;
						textContext.printfAt(0.01f, 0.93f, "zone %u patch %d tile (%d,%d)%s",
						                     paintListener.HoverZone, (int)(t / 256), (int)(t % 256 % 16), (int)(t % 256 / 16),
						                     core->zoneFrozen(paintListener.HoverZone) ? " FROZEN" : "");
					}
					else if (paintListener.HaveHover && paintListener.Mode == CPaintMouseListener::ModeProp)
					{
						const SPaintZone *hov = zpFindPaintZone(paintListener.HoverZone);
						textContext.printfAt(0.01f, 0.93f, "hover zone %u %s",
						                     paintListener.HoverZone,
						                     hov ? hov->Name.c_str() : "");
					}
					// Brush color swatch
					if (paintListener.Mode == CPaintMouseListener::ModeColor)
						NL3D::CDRU::drawQuad(0.30f, 0.955f, 0.32f, 0.975f, *driver,
						                     paintListener.BrushColor, viewport);
				}

				// F12 capture BEFORE swap: getBuffer reads GL_BACK, which after swap holds the
				// previous frame (or garbage on swap-exchange drivers).
				zpViewerScreenshot(driver, udriver->AsyncListener);
				udriver->swapBuffers();
			}
			while (!viewerQuit && closeListener.WindowActive && udriver->isActive());
		}

		if (ownsEditorUI)
			editorUI->shutdown();
		zpDropViewerFrameState();
		mouseListener.removeFromServer(udriver->EventServer);
		udriver->EventServer.removeListener(NLMISC::EventDestroyWindowId, &closeListener);
		udriver->EventServer.removeListener(NLMISC::EventCloseWindowId, &closeListener);
		if (core)
		{
			core->attachLandscape(NULL);
			udriver->EventServer.removeListener(NLMISC::EventMouseDownId, &paintListener);
			udriver->EventServer.removeListener(NLMISC::EventMouseUpId, &paintListener);
			udriver->EventServer.removeListener(NLMISC::EventMouseMoveId, &paintListener);
		}
		// Drop the painting UScene; the shared driver (startup flow) keeps its display.
		if (uscene)
		{
			udriver->deleteScene(uscene);
			uscene = NULL;
		}
		if (ownsDriver)
		{
			udriver->release();
			delete udriver;
			udriver = NULL;
		}
	}
	catch (const NL3D::EDru &e)
	{
		zpDropViewerFrameState();
		if (ownsDriver && udriver) { udriver->release(); delete udriver; }
		fprintf(stderr, "ERROR: 3D driver: %s\n", e.what());
		return 1;
	}
	catch (const NLMISC::Exception &e)
	{
		zpDropViewerFrameState();
		if (ownsDriver && udriver) { udriver->release(); delete udriver; }
		fprintf(stderr, "ERROR: %s\n", e.what());
		return 1;
	}
	return g_ViewerScriptRc;
}


