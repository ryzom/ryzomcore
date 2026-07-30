/**
 * \file script_and_ui.cpp
 * \brief Paint-script op executor + viewer UI actions + host bridge glue.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * `zpExecScriptOp` / `runPaintScript` (the shared op layer — same implementations the
 * mouse and painterscript reach); brush/tile/color/displace mode handlers (zpSelectMode
 * / zpSelectTileSetAbs / zpColorRadiusAbs / zpFill …); prop-mode edit + zone-outline
 * draw; season toggle + palette rebuild; SScriptHost bridge and zpFillBridgeState so
 * NLGUI panels stay in sync with paint state.
 *
 * Patch-edit mode lives in its own pair of TUs: patch_edit_ops.cpp (state and operations)
 * and patch_edit_ui.cpp (drawing and picking), sharing drag state via patch_edit_internal.h.
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

// ---------------------------------------------------------------------------------------------
// Scripted paint mode: one op per line, same op layer as the mouse path (see the file header
// for the command list). Any FAILed op fails the run (scripts are curated test inputs).

// Forward: prop write (defined with Prop helpers below).
bool zpWriteZoneProp(uint zoneId, const std::string &which, int value, std::string &err);
bool zpZoneIsFootprintSource(uint zoneId);

// One canonical script op: the --paint-script vocabulary; painterscript (script_api)
// executes through the SAME function so Lua ops are byte-equivalent to file ops.
// Blank/comment-only lines return true with *opName empty. Unknown commands set
// *unknownCmd (runPaintScript aborts the file on those).
bool zpExecScriptOp(ZPPAINT::CPaintCore &core, const std::string &rawLine,
                           std::string &err, std::string *opName, bool *unknownCmd)
{
	if (opName) opName->clear();
	if (unknownCmd) *unknownCmd = false;
	std::string line = rawLine;
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
	if (tok.empty()) return true;
	if (opName) *opName = tok[0];
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
		// Undo/redo may replay a TOPOLOGY record, which restores storage and needs the
		// working-set rebuild the core cannot run itself - same handling as the U/I keys.
		else if (tok[0] == "undo") { ok = core.opUndo(); if (!ok) err = "undo stack empty"; else zpHandleTopoRestorePending(); }
		else if (tok[0] == "redo") { ok = core.opRedo(); if (!ok) err = "redo stack empty"; else zpHandleTopoRestorePending(); }
		else if (tok[0] == "prop" && tok.size() >= 4)
		{
			// prop <zone> <rotate|symmetry|passable|usebbox> <value>
			// Write path (same handlers as the Prop panel); also adds undo records.
			uint zone = 0;
			int val = 0;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[3], val);
			ok = zpWriteZoneProp(zone, tok[2], val, err);
		}
		else if (tok[0] == "seed" && tok.size() >= 2)
		{
			uint s;
			NLMISC::fromString(tok[1], s);
			srand(s);
		}
		else if (tok[0] == "color" && tok.size() >= 6)
		{
			// color <zone> <patch> <s> <t> <rrggbb> [blend 0-256]
			uint zone, patch;
			sint32 s, t;
			uint blend = 256;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], s);
			NLMISC::fromString(tok[4], t);
			uint32 rgb = (uint32)strtoul(tok[5].c_str(), NULL, 16);
			if (tok.size() >= 7) NLMISC::fromString(tok[6], blend);
			NLMISC::CRGBA col((uint8)((rgb >> 16) & 0xff), (uint8)((rgb >> 8) & 0xff), (uint8)(rgb & 0xff), 255);
			ok = core.opColorVertex(zone, patch, s, t, col, blend, err);
		}
		else if (tok[0] == "cbrush" && tok.size() >= 8)
		{
			// cbrush <zone> <worldX> <worldY> <radius> <rrggbb> <hardness 0-255> <opacity 0-255> [worldZ]
			// Without Z the hit comes from a vertical pick at (x, y) (the mouse-ray stand-in);
			// with Z the hit is explicit (steep terrain, exact-vertex validation).
			uint zone;
			float x, y, radius;
			uint hardness, opacity;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], x);
			NLMISC::fromString(tok[3], y);
			NLMISC::fromString(tok[4], radius);
			uint32 rgb = (uint32)strtoul(tok[5].c_str(), NULL, 16);
			NLMISC::fromString(tok[6], hardness);
			NLMISC::fromString(tok[7], opacity);
			NLMISC::CRGBA col((uint8)((rgb >> 16) & 0xff), (uint8)((rgb >> 8) & 0xff), (uint8)(rgb & 0xff), 255);
			uint hitZone = zone;
			sint32 hitTile = -1;
			NLMISC::CVector hit;
			if (tok.size() >= 9)
			{
				float zc;
				NLMISC::fromString(tok[8], zc);
				hit.set(x, y, zc);
				if (!core.nearestTile(zone, hit, hitTile)) { ok = false; err = "no tile in zone"; }
			}
			else if (!core.pickTile(NLMISC::CVector(x, y, 20000.f), NLMISC::CVector(0.f, 0.f, -1.f), hitZone, hitTile, hit))
			{ ok = false; err = "no tile under the brush position"; }
			else if (hitZone != zone) { ok = false; err = NLMISC::toString("pick landed in zone %u", hitZone); }
			if (ok)
			{
				ok = core.opColorBrush(hitZone, hitTile, hit, radius, col, hardness, opacity, err);
				// cbrush ... <z> [cont]: with the cont token (stroke-aware recordings) the
				// commit comes from a recorded `endstroke` line, preserving the live drag's
				// one-undo-entry-per-drag granularity; legacy form commits per line.
				if (tok.size() < 10)
					core.endStroke();
			}
		}
		else if ((tok[0] == "fill" || tok[0] == "fill256") && tok.size() >= 4)
		{
			// fill <zone> <patch> <tileSet> [rot]
			uint zone, patch;
			int ts, rot = 0;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], ts);
			if (tok.size() >= 5) NLMISC::fromString(tok[4], rot);
			ok = core.opFillTile(zone, patch, ts, rot, tok[0] == "fill256", err);
		}
		else if (tok[0] == "cfill" && tok.size() >= 4)
		{
			// cfill <zone> <patch> <rrggbb> [blend 0-256]
			uint zone, patch;
			uint blend = 256;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			uint32 rgb = (uint32)strtoul(tok[3].c_str(), NULL, 16);
			if (tok.size() >= 5) NLMISC::fromString(tok[4], blend);
			NLMISC::CRGBA col((uint8)((rgb >> 16) & 0xff), (uint8)((rgb >> 8) & 0xff), (uint8)(rgb & 0xff), 255);
			ok = core.opFillColor(zone, patch, col, blend, err);
		}
		else if (tok[0] == "displace" && tok.size() >= 6)
		{
			// displace <zone> <patch> <u> <v> <0-15>
			uint zone, patch, u, v, d;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], u);
			NLMISC::fromString(tok[4], v);
			NLMISC::fromString(tok[5], d);
			ok = core.opDisplace(zone, patch, u, v, d, err);
		}
		else if (tok[0] == "dfill" && tok.size() >= 4)
		{
			// dfill <zone> <patch> <0-15>
			uint zone, patch, d;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], d);
			ok = core.opFillDisplace(zone, patch, d, err);
		}
		else if (tok[0] == "brush" && tok.size() >= 2)
		{
			uint b;
			NLMISC::fromString(tok[1], b);
			core.setBrushSize(b);
		}
		else if (tok[0] == "group" && tok.size() >= 2)
		{
			uint g;
			NLMISC::fromString(tok[1], g);
			core.setTileGroup(g);
		}
		else if (tok[0] == "mask" && tok.size() >= 2)
		{
			// mask <file.tga|none>: color-brush bitmap mask (CPath-resolved)
			if (tok[1] == "none")
				core.clearBrushMask();
			else
				ok = core.loadBrushMask(tok[1], err);
		}
		else if (tok[0] == "dumpclosure" && tok.size() >= 5)
		{
			// dumpclosure <zone> <patch> <s> <t>: print the vertex's co-location closure
			uint zone, patch;
			sint32 s, t;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], s);
			NLMISC::fromString(tok[4], t);
			ok = core.dumpClosure(zone, patch, s, t, stdout);
			if (!ok) err = "bad vertex";
		}
		else if (tok[0] == "rawtile" && tok.size() >= 6)
		{
			// DEBUG: rawtile <zone> <patch> <u> <v> <tile> [rot]: raw record, no solver
			uint zone, patch, u, v;
			int tile, rot = 0;
			NLMISC::fromString(tok[1], zone);
			NLMISC::fromString(tok[2], patch);
			NLMISC::fromString(tok[3], u);
			NLMISC::fromString(tok[4], v);
			NLMISC::fromString(tok[5], tile);
			if (tok.size() >= 7) NLMISC::fromString(tok[6], rot);
			ok = core.opRawTile(zone, patch, u, v, tile, rot, err);
		}
		else if (tok[0] == "tstroke" && tok.size() >= 6)
	{
		// tstroke <zone> <patch> <u> <v> <set> [big 0|1] [cont 0|1]: the MOUSE tile-stroke
		// path (brush-size recursion via opTileStroke), for recorder replay fidelity
		// (big = 256 flag). WITHOUT the cont token (legacy form) each line is a
		// self-contained stroke (first + commit). WITH it, the line joins an open stroke:
		// first when cont==0, continuation when cont==1, and the commit comes from a
		// recorded `endstroke` line. This preserves the live drag's calcRotPath rotation-
		// following (first=false derives rotation from the previous tile) and its undo
		// granularity (one entry per drag, not per tile), which the legacy per-line form
		// could not replay byte-identically.
		uint zone, patch, u, v; int ts, big = 0, cont = -1;
		NLMISC::fromString(tok[1], zone);
		NLMISC::fromString(tok[2], patch);
		NLMISC::fromString(tok[3], u);
		NLMISC::fromString(tok[4], v);
		NLMISC::fromString(tok[5], ts);
		if (tok.size() >= 7) NLMISC::fromString(tok[6], big);
		if (tok.size() >= 8) NLMISC::fromString(tok[7], cont);
		sint32 tileId = (sint32)(patch * ZP_NUM_TILE_SEL + v * ZP_MAX_TILE_IN_PATCH + u);
		if (cont < 0)
		{
			ok = core.opTileStroke(zone, tileId, ts, big != 0, true, err);
			core.endStroke();
		}
		else
			ok = core.opTileStroke(zone, tileId, ts, big != 0, cont == 0, err);
	}
	else if (tok[0] == "endstroke")
	{
		// endstroke: commit the open stroke (mouse-up / stroke-aware recordings)
		core.endStroke();
	}
	else if (tok[0] == "lockborders" && tok.size() >= 2)
	{
		// lockborders <0|1>: plugin lockBorders state (recorded scripts replay headless;
		// CLI --lock-borders sets the same core flag).
		uint on = 0;
		NLMISC::fromString(tok[1], on);
		core.setLockBorders(on != 0);
	}
	else if (tok[0] == "maskmode" && tok.size() >= 2)
	{
		// maskmode <0|1>: color-brush mask-mode gate (Q key equivalent).
		uint on = 0;
		NLMISC::fromString(tok[1], on);
		core.setBrushMaskMode(on != 0);
	}
	else if (tok[0] == "checkseams" && tok.size() >= 2)
		{
			uint zone;
			NLMISC::fromString(tok[1], zone);
			uint illegal = core.checkSeams(zone, stdout);
			ok = illegal == 0;
			if (!ok) err = NLMISC::toString("%u illegal seams", illegal);
		}
	else
	{
		if (unknownCmd) *unknownCmd = true;
		err = NLMISC::toString("bad command '%s'", tok[0].c_str());
		ok = false;
	}
	return ok;
}

int runPaintScript(ZPPAINT::CPaintCore &core, const std::string &path)
{
	std::ifstream ifs(path.c_str());
	if (!ifs) { fprintf(stderr, "ERROR: cannot open script %s\n", path.c_str()); return 1; }
	std::string line;
	int lineNo = 0;
	int fails = 0;
	while (std::getline(ifs, line))
	{
		++lineNo;
		std::string err, op;
		bool unknown = false;
		bool ok = zpExecScriptOp(core, line, err, &op, &unknown);
		if (op.empty()) continue;
		if (unknown)
		{
			fprintf(stderr, "ERROR: script line %d: %s\n", lineNo, err.c_str());
			return 1;
		}
		if (ok) printf("OK line %d: %s (%u tile writes)\n", lineNo, op.c_str(), core.strokeSetCount());
		else
		{
			printf("FAIL line %d: %s: %s\n", lineNo, op.c_str(), err.c_str());
			++fails;
		}
	}
	return fails ? 1 : 0;
}

// ---------------------------------------------------------------------------------------------
// Shared paint actions: keyboard AND NLGUI call these (no second op implementation).
// Live for the duration of runViewer only (g_PaintCtx.Active). Also used by the headless
// --panel-save-test hook (same zpSaveTo / zpSaveOverwrite functions).


/** Quote a string as a Lua literal for recorder lines. */
std::string luaQuote(const std::string &s)
{
	std::string r = "\"";
	for (size_t i = 0; i < s.size(); ++i)
	{
		char c = s[i];
		if (c == '"' || c == '\\') { r += '\\'; r += c; }
		else if (c == '\n') r += "\\n";
		else r += c;
	}
	r += "\"";
	return r;
}

// Board-op recorder. Board ops NEST (openEditable→placeInstanceOf on dup opens,
// contextToEditable→openEditable, dragDrop→place paths, toggle→open/save); record at
// the OUTERMOST op only, and only on success, so a recording replays each user action
// exactly once. ZPSCRIPT::record() itself already no-ops during script execution
// (replays must not re-record) and while REC is off.

void recordBoardOp(const std::string &line)
{
	if (g_BoardOpDepth == 1)
		ZPSCRIPT::record(line);
}

/** Current paint mode for the key table's mode scoping (see ZPKS_* in zp_state.h). */
int zpCurrentPaintMode()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint)
		return -1;
	return g_PaintCtx.Paint->Mode;
}

/*
 * TODO (cursors): THE cursor manifest. Mode-dependent pointer shapes are not wired yet -
 * every mode currently shows the plain arrow, and mode is signalled only by the toolbar's
 * pushed button and the --font HUD. That is tolerable while the left button does exactly
 * one thing per mode; it stops being tolerable once the patch-edit sub-object levels and
 * the Max transform modes land, because then the same click means select / move / rotate /
 * scale / bind depending on state the artist cannot otherwise see.
 *
 * MECHANISM: custom shapes do NOT ride the pointer's XML tx_* slots (CViewPointer only
 * defines a fixed set: default / move_window / resize_* / rotate / scale / colpick / pan /
 * can_pan). They go through CViewPointer::setCursor("name.tga"), which overrides the
 * DEFAULT shape - so a mode switch sets it and leaving the mode sets it back to
 * "curs_default.tga". Anything the GUI itself claims (resizers, colour picker) still wins
 * over it, which is the behaviour we want: chrome beats tool.
 *
 * WHERE THE ART GOES: shared shapes belong in ryzomcore_graphics/interfaces/v3 (packed into
 * every tool's atlas); painter-only shapes go in ZONE_PAINTER_EXTRA_TEXTURES in the tool's
 * CMakeLists, which stages them into the extras dir build_interface packs. Either way the
 * name must land in the atlas .txt before setCursor can resolve it.
 *
 * HAVE (already packed from interfaces/v3, usable today):
 *   curs_default   select / no-op            curs_pick     eyedropper pick
 *   curs_rotate    rotate, and orbit drag    curs_scale    scale
 *   curs_pan       panning                   curs_can_pan  pan available
 *   curs_resize_*  container resizers (wired in NLGUI already, not a tool concern)
 *
 * NEED BITMAPS - paint modes (this switch):
 *   curs_zp_tile      Tile mode brush
 *   curs_zp_color     Colour mode brush
 *   curs_zp_displace  Displace mode brush
 *   (Prop mode uses curs_default: it is a selection mode.)
 *
 * NEED BITMAPS - patch-edit modes (same switch, once the sub-object levels exist):
 *   curs_zp_move      4-way move
 *   curs_zp_region    rubber-band region select
 *   curs_zp_bind      bind vertex to edge
 *   curs_zp_weld      weld
 *   curs_zp_attach    attach
 *   curs_zp_subdiv    subdivide / add patch
 *   (Rotate and Scale reuse curs_rotate / curs_scale. Turn-edge can reuse curs_rotate.)
 *
 * NEED BITMAPS - navigation (see nel/src/3d/nav_mouse_listener.cpp):
 *   curs_zp_zoom      dolly / zoom drag
 *   (Pan and orbit reuse curs_pan / curs_rotate.)
 *
 * The full art list with motif descriptions is the wiki's zone_painter_icon_manifest.md.
 * nel_patch_paint cursor art may be converted; patch-edit cursors are redraw-only from
 * the manifest's motif descriptions.
 */
/**
 * Orientation arrows - the legacy painter's ToggleArrows (paint.cpp), verbatim recipe:
 * every bank tile gains an ADDITIVE arrow layer so each painted tile renders its frame
 * orientation, rotation included; off restores the pristine bank. Every patch of every
 * zone is then invalidated (changePatchTextureAndColor with NULLs) so the tiles reload.
 * Patch mode additionally draws a thin per-patch frame arrow on the overlay
 * (zpDrawPatchArrows) - the display that makes Turn CW/CCW visible at all.
 */
void zpSetShowArrows(bool on)
{
	if (on == g_ShowArrows)
		return;
	g_ShowArrows = on;
	if (g_PaintCtx.Land && g_PaintCtx.Bank)
	{
		NL3D::CLandscape &land = g_PaintCtx.Land->Landscape;
		if (on)
		{
			// The arrow tile lives at the _texture_tiles ROOT, which the bank resolve does
			// not index - and a missing additive texture whitewashes the whole terrain
			// (NeL's missing-texture fallback). Register it once, probing up from the bank.
			static bool s_ArrowRegistered = false;
			if (!s_ArrowRegistered)
			{
				std::string dir = NLMISC::CFile::getPath(g_PaintCtx.BankPath);
				for (int up = 0; up < 4 && !s_ArrowRegistered; ++up)
				{
					const std::string cand = dir + "arrow.png";
					if (NLMISC::CFile::fileExists(cand))
					{
						NLMISC::CPath::addSearchFile(cand);
						s_ArrowRegistered = true;
					}
					dir += "../";
				}
				if (!s_ArrowRegistered)
					fprintf(stderr, "WARNING: arrow.png not found near the bank; "
					        "tile arrows will render white\n");
			}
			for (sint i = 0; i < land.TileBank.getTileCount(); ++i)
			{
				land.TileBank.getTile(i)->setFileName(NL3D::CTile::additive, "arrow.png");
				land.releaseTiles((uint)i, 1);
			}
		}
		else
		{
			land.TileBank = *g_PaintCtx.Bank;
			for (sint i = 0; i < land.TileBank.getTileCount(); ++i)
				land.releaseTiles((uint)i, 1);
		}
		if (g_PaintCtx.Zones)
		{
			const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
			for (uint z = 0; z < zones.size(); ++z)
			{
				NL3D::CZone *lz = land.getZone((sint)zones[z].ZoneId);
				for (uint p = 0; lz && p < (uint)lz->getNumPatchs(); ++p)
					lz->changePatchTextureAndColor((sint)p, NULL, NULL);
			}
		}
	}
	g_PropStatusMsg = on ? "orientation arrows ON" : "orientation arrows off";
	ZPSCRIPT::record(NLMISC::toString("painter.setShowArrows(%s)", on ? "true" : "false"));
}

void zpToggleShowArrows() { zpSetShowArrows(!g_ShowArrows); }

void zpSelectMode(int mode)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (mode < 0) mode = 0;
	if (mode > CPaintMouseListener::ModePatch) mode = CPaintMouseListener::ModePatch;
	// A live drag cannot survive its mode: the selection it targets is cleared below, yet the
	// release would still commit (and record) the transform. The key table stays live during a
	// drag, so this is reachable from the keyboard mid-drag. Both cancels are no-ops when idle.
	zpPatchGizmoCancelDrag();
	zpWeldDragCancel();
	g_PaintCtx.Paint->Mode = mode;
	// Welds serve painting and actively hide patch edits (see buildDisplayZone), so the weld
	// state follows the mode. Only on a CHANGE - the rebuild is cheap but not free, and
	// re-selecting the mode you are already in should do nothing.
	{
		bool want = mode != CPaintMouseListener::ModePatch;
		// Dev hook, alongside ZONE_PAINTER_GIZMO_DRAG: pin the weld state so a gate can run the
		// SAME script both ways and assert the difference. Without a negative control "the seam
		// is visible" is a claim about a screenshot rather than about the build.
		const char *dev = getenv("ZONE_PAINTER_FORCE_WELD");
		if (dev && *dev)
			want = *dev != '0';
		if (want != g_WeldedLandscape)
		{
			g_WeldedLandscape = want;
			zpSyncLandscapeWeld();
		}
	}
	// Entering patch edit lands on Object level, : the artist picks a sub-object
	// level deliberately, and arriving already in vertex mode makes the first click surprising.
	if (mode == CPaintMouseListener::ModePatch)
		g_PaintCtx.Paint->SubObj = CPaintMouseListener::SubObject;
	// A sub-object selection belongs to the level it was made at, so leaving patch edit (or
	// re-entering it, which lands on Object) drops it rather than leaving a stale set to
	// surprise the next move.
	g_PatchVertSel.clear();
	g_PatchEdgeSel.clear();
	g_PatchFaceSel.clear();
	g_PatchTanSel.clear();
	// TODO (cursors): set the mode's pointer shape here - one setCursor() per mode, back to
	// "curs_default.tga" for Prop and for any mode whose art is still missing. See the
	// manifest above for the names and where the bitmaps come from.
	ZPSCRIPT::record(NLMISC::toString("painter.setMode(%d)", mode));
}


/**
 * HUD labels. Both tables are indexed by the enum they name and bounds-checked here rather
 * than at each call site - the previous pair of local `modeNames[4]` tables would have
 * quietly relabelled patch mode as TILE.
 */
const char *zpModeName(int mode)
{
	static const char *kNames[CPaintMouseListener::ModePatch + 1] =
		{ "TILE", "COLOR", "DISPLACE", "PROP", "PATCH" };
	if (mode < 0 || mode > CPaintMouseListener::ModePatch)
		return "?";
	return kNames[mode];
}

const char *zpSubObjName(int level)
{
	static const char *kNames[CPaintMouseListener::SubCount] =
		{ "OBJECT", "VERTEX", "EDGE", "PATCH", "TILE" };
	if (level < 0 || level >= CPaintMouseListener::SubCount)
		return "?";
	return kNames[level];
}

/** Sub-object level inside patch-edit mode (EP_* order). No-op outside ModePatch. */
void zpSelectSubObject(int level)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (g_PaintCtx.Paint->Mode != CPaintMouseListener::ModePatch) return;
	if (level < 0) level = 0;
	if (level >= CPaintMouseListener::SubCount) level = CPaintMouseListener::SubCount - 1;
	if (level != g_PaintCtx.Paint->SubObj)
	{
		// Same rule as the mode switch: a drag in flight targets the outgoing level's
		// selection, so it cannot survive the switch that clears it.
		zpPatchGizmoCancelDrag();
		zpWeldDragCancel();
		// A selection made at one level means nothing at another, and carrying it over would
		// leave a gizmo sitting on things the new level cannot address.
		g_PatchVertSel.clear();
		g_PatchEdgeSel.clear();
		g_PatchFaceSel.clear();
		g_PatchTanSel.clear();
	}
	g_PaintCtx.Paint->SubObj = level;
	ZPSCRIPT::record(NLMISC::toString("painter.setSubObject(%d)", level));
}


/**
 * Prop-mode selectable: any editable node, read-only context excluded.
 *
 * Zone properties are Max appdata on the NODE, and nodes showing one object share that
 * pointer - so editing them through a second node of the object writes the same storage and
 * is exactly as correct as editing them through the first. There is no display-copy case to
 * exclude.
 */
bool zpZoneIsPropSelectable(uint zoneId)
{
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (pz)
		return pz->Editable;
	// Headless before assembly hands over the zone list: the core still knows read-only.
	if (g_PaintCtx.Core)
		return !g_PaintCtx.Core->zoneFrozen(zoneId);
	return false;
}

void zpClearPropSelection()
{
	g_HavePropSelection = false;
	g_SelectedZoneId = 0;
	g_PropStatusMsg.clear();
}

/**
 * Zone outline - outer perimeter only, chained into closed loops.
 *
 * Edge set (authoritative adjacency, not a raw open-edge scan):
 *   - PatchMesh edge shared by two patches of THIS mesh → interior.
 *   - CPatchInfo::BindEdges with NPatchs!=0 and ZoneId==this zone → same-zone neighbor
 *     (shared edge, 1-1/1-2/1-4 binds via dividEdge/offsetEdge) → interior.
 *   - BindEdges NPatchs==0 or ZoneId!=this (cross-zone weld) → outer perimeter.
 * Earlier gated the BindEdges test on Rp vert Binded flags, so intra-zone bind seams that
 * are open in PatchMesh still drew as "boundary" (interior tangle / bowties).
 *
 * Connectivity: tessellate each outer edge along CBezierPatch in V[e]→V[(e+1)&3] order,
 * chain by mesh vertex endpoints into closed loop polylines. One polyline per loop.
 * Rendering: project to screen-space CDRU lines (no depth test - overdraw accepted;
 * priority is correct perimeter geometry). Thin hover / thick multi-pass selection.
 */

void zpEvalPatchEdgePoint(const NL3D::CBezierPatch &bp, uint e, float t,
                                 NLMISC::CVector &out)
{
	// Edge param: e0 s=0 t:0→1, e1 t=1 s:0→1, e2 s=1 t:1→0, e3 t=0 s:1→0
	// Matches V[e] → V[(e+1)&3] with V0=(0,0), V1=(0,1), V2=(1,1), V3=(1,0).
	float s = 0.f, tv = 0.f;
	switch (e & 3)
	{
	case 0: s = 0.f; tv = t; break;
	case 1: s = t; tv = 1.f; break;
	case 2: s = 1.f; tv = 1.f - t; break;
	default: s = 1.f - t; tv = 0.f; break;
	}
	out = bp.eval(s, tv);
}

bool zpIsZoneOuterBoundaryEdge(const SPaintZone &pz, size_t p, uint e)
{
	if (p >= pz.Patches.size())
		return false;
	const SPatchMesh &pm = pz.Ep.Pm;
	// 1) PatchMesh topology: two patches of this mesh share the edge → interior.
	if (p < pm.Patches.size())
	{
		const sint32 edgeIdx = pm.Patches[p].Edge[e];
		if (edgeIdx >= 0 && (size_t)edgeIdx < pm.Edges.size())
		{
			const SPmEdge &edge = pm.Edges[(size_t)edgeIdx];
			sint32 other = -1;
			if (!edge.Patches.empty())
			{
				if (edge.Patches[0] == (sint32)p)
				{
					if (edge.Patches.size() > 1)
						other = edge.Patches[1];
				}
				else
					other = edge.Patches[0];
			}
			if (other >= 0 && (size_t)other < pm.Patches.size())
				return false;
		}
	}
	// 2) BindEdges (session-welded): same-zone neighbor (any NPatchs) → interior.
	//    Cross-zone welds (ZoneId != this) stay outer for THIS zone's silhouette.
	//    NPatchs==0 is a true open / outer edge.
	const NL3D::CPatchInfo::CBindInfo &bi = pz.Patches[p].BindEdges[e];
	if (bi.NPatchs != 0 && bi.ZoneId == (uint16)pz.ZoneId)
		return false;
	return true;
}

/** Collect outer boundary edges, chain into closed loops, tessellate each loop. */
void zpCollectZoneBoundaryPolylines(const SPaintZone &pz, uint segsPerEdge,
                                           std::vector<NLMISC::CVector> &outPts,
                                           std::vector<uint> &outSegCounts,
                                           uint *outLoopCount,
                                           uint *outEdgeCount)
{
	outPts.clear();
	outSegCounts.clear();
	if (outLoopCount) *outLoopCount = 0;
	if (outEdgeCount) *outEdgeCount = 0;
	if (pz.Patches.empty() || pz.Ep.Pm.Patches.empty())
		return;
	const SPatchMesh &pm = pz.Ep.Pm;
	if (segsPerEdge < 2)
		segsPerEdge = 2;
	const uint nPts = segsPerEdge + 1;

	// --- gather outer edges ---
	std::vector<SZpBoundEdge> bounds;
	bounds.reserve(pm.Patches.size() * 2);
	for (size_t p = 0; p < pm.Patches.size() && p < pz.Patches.size(); ++p)
	{
		const SPmPatch &pp = pm.Patches[p];
		for (uint e = 0; e < 4; ++e)
		{
			if (!zpIsZoneOuterBoundaryEdge(pz, p, e))
				continue;
			SZpBoundEdge be;
			be.Patch = (uint)p;
			be.Edge = e;
			be.V0 = pp.V[e];
			be.V1 = pp.V[(e + 1) & 3];
			be.Reverse = false;
			// Prefer PatchMesh edge endpoint orientation when available
			const sint32 edgeIdx = pp.Edge[e];
			if (edgeIdx >= 0 && (size_t)edgeIdx < pm.Edges.size())
			{
				const SPmEdge &ed = pm.Edges[(size_t)edgeIdx];
				// Keep bezier V[e]→V[(e+1)] order; only flag if mesh edge is flipped vs that
				if (ed.V1 == be.V1 && ed.V2 == be.V0)
					be.Reverse = false; // still tessellate along patch edge dir
			}
			bounds.push_back(be);
		}
	}
	if (outEdgeCount) *outEdgeCount = (uint)bounds.size();
	if (bounds.empty())
		return;

	// --- adjacency: mesh vert → incident bound-edge indices ---
	// Fall back to quantized world pos when V indices are invalid / unmatched (rare binds).
	std::map<sint32, std::vector<uint> > byVert;
	for (uint i = 0; i < (uint)bounds.size(); ++i)
	{
		if (bounds[i].V0 >= 0)
			byVert[bounds[i].V0].push_back(i);
		if (bounds[i].V1 >= 0 && bounds[i].V1 != bounds[i].V0)
			byVert[bounds[i].V1].push_back(i);
	}

	// --- walk closed loops ---
	std::vector<bool> used(bounds.size(), false);
	uint loops = 0;
	for (uint start = 0; start < (uint)bounds.size(); ++start)
	{
		if (used[start])
			continue;
		// Build ordered edge sequence for this loop
		std::vector<uint> loopEdges;
		std::vector<bool> loopFwd; // true = walk V0→V1, false = V1→V0
		uint cur = start;
		sint32 enterVert = bounds[start].V0; // arrive at V0, leave toward V1
		// Prefer starting so we walk V0→V1
		bool fwd = true;
		uint guard = 0;
		const uint guardMax = (uint)bounds.size() + 2;
		while (!used[cur] && guard++ < guardMax)
		{
			used[cur] = true;
			loopEdges.push_back(cur);
			loopFwd.push_back(fwd);
			const SZpBoundEdge &be = bounds[cur];
			sint32 leaveVert = fwd ? be.V1 : be.V0;
			// Find next unused edge incident to leaveVert
			uint next = cur;
			bool nextFwd = true;
			bool found = false;
			std::map<sint32, std::vector<uint> >::const_iterator it = byVert.find(leaveVert);
			if (it != byVert.end())
			{
				const std::vector<uint> &cand = it->second;
				for (size_t c = 0; c < cand.size(); ++c)
				{
					const uint ei = cand[c];
					if (used[ei])
						continue;
					const SZpBoundEdge &nb = bounds[ei];
					if (nb.V0 == leaveVert)
					{
						next = ei;
						nextFwd = true;
						found = true;
						break;
					}
					if (nb.V1 == leaveVert)
					{
						next = ei;
						nextFwd = false;
						found = true;
						break;
					}
				}
			}
			if (!found)
				break; // open chain (should not happen for a closed perimeter)
			// Closed when we return to start edge's start vert after ≥1 edge
			if (leaveVert == enterVert && !loopEdges.empty())
			{
				// actually closed only if next would be start - handled by used[start]
				// If leaveVert is enterVert after first edge only, degenerate; continue.
			}
			cur = next;
			fwd = nextFwd;
			if (cur == start)
				break;
		}
		// If walk stopped without consuming a full cycle, still emit what we have
		if (loopEdges.empty())
			continue;
		++loops;

		// Tessellate loop as one continuous polyline (shared endpoints once)
		std::vector<NLMISC::CVector> loopPts;
		for (size_t li = 0; li < loopEdges.size(); ++li)
		{
			const SZpBoundEdge &be = bounds[loopEdges[li]];
			const bool goFwd = loopFwd[li];
			const NL3D::CBezierPatch &bp = pz.Patches[be.Patch].Patch;
			// For edges after the first, skip the duplicated start point
			const uint i0 = (li == 0) ? 0 : 1;
			for (uint i = i0; i < nPts; ++i)
			{
				const float tRaw = (float)i / (float)segsPerEdge;
				const float t = goFwd ? tRaw : (1.f - tRaw);
				NLMISC::CVector pt;
				zpEvalPatchEdgePoint(bp, be.Edge, t, pt);
				loopPts.push_back(pt);
			}
		}
		// Close the loop visually: append first point if not already coincident
		if (loopPts.size() >= 2)
		{
			const NLMISC::CVector &a = loopPts.front();
			const NLMISC::CVector &b = loopPts.back();
			if ((a - b).sqrnorm() > 1e-6f)
				loopPts.push_back(a);
		}
		if (loopPts.size() >= 2)
		{
			outSegCounts.push_back((uint)loopPts.size());
			outPts.insert(outPts.end(), loopPts.begin(), loopPts.end());
		}
	}
	// Orphan edges not reached by a loop walk (broken adjacency) - emit as single segs
	for (uint i = 0; i < (uint)bounds.size(); ++i)
	{
		if (used[i])
			continue;
		used[i] = true;
		const SZpBoundEdge &be = bounds[i];
		const NL3D::CBezierPatch &bp = pz.Patches[be.Patch].Patch;
		outSegCounts.push_back(nPts);
		for (uint k = 0; k < nPts; ++k)
		{
			const float t = (float)k / (float)segsPerEdge;
			NLMISC::CVector pt;
			zpEvalPatchEdgePoint(bp, be.Edge, t, pt);
			outPts.push_back(pt);
		}
		++loops; // count as degenerate 1-edge "loop"
	}
	if (outLoopCount) *outLoopCount = loops;
}

/**
 * Draw zone boundary as screen-space polylines.
 *
 * Project tessellated loop points through the camera; 2D CDRU::drawLine (Z always).
 * No depth test - occluded segments overdraw (acceptable; geometry correctness first).
 * Thin = 1px; thick = multi-pass with small screen-space offsets.
 */

void zpDrawZoneOutline(NL3D::IDriver *driver, NL3D::CCamera *camera,
                              const NL3D::CViewport &viewport,
                              const SPaintZone &pz, const NLMISC::CRGBA &col, bool thick)
{
	if (!driver || !camera)
		return;
	std::vector<NLMISC::CVector> pts;
	std::vector<uint> segs;
	uint nLoops = 0, nEdges = 0;
	zpCollectZoneBoundaryPolylines(pz, thick ? 16 : 10, pts, segs, &nLoops, &nEdges);
	if (pts.empty() || segs.empty())
	{
		static bool s_EmptyOnce = false;
		if (!s_EmptyOnce)
		{
			printf("prop-outline: no boundary segs for zone %u '%s' (patches=%u edges=%u)\n",
			       pz.ZoneId, pz.Name.c_str(), (uint)pz.Patches.size(),
			       (uint)pz.Ep.Pm.Edges.size());
			s_EmptyOnce = true;
		}
		return;
	}
	const NLMISC::CMatrix viewMat = camera->getMatrix().inverted();
	const NL3D::CFrustum &fr = camera->getFrustum();
	const float zLift = 0.4f;
	std::vector<NLMISC::CVector> proj;
	proj.resize(pts.size());
	std::vector<bool> ok(pts.size(), false);
	for (size_t i = 0; i < pts.size(); ++i)
	{
		NLMISC::CVector w = pts[i];
		w.z += zLift;
		const NLMISC::CVector eye = viewMat * w;
		if (eye.y <= fr.Near * 0.5f)
			continue;
		proj[i] = fr.project(eye);
		ok[i] = true;
	}
	const int passes = thick ? 5 : 1;
	const float ox[5] = { 0.f, 0.0015f, -0.0015f, 0.f, 0.f };
	const float oy[5] = { 0.f, 0.f, 0.f, 0.0015f, -0.0015f };
	// this is a screen-space overlay (no Z-test, drawn via the driver directly) so it
	// paints straight over the NLGUI panel/toolbar wherever its projected screen position lands
	// there, instead of being occluded by it. Cull segments whose endpoints fall under an active
	// GUI window (same pixel-hit-test the app already uses for wantsMouse()) rather than fight
	// over draw order against the GUI's own rendering.
	uint32 winW = 0, winH = 0;
	driver->getWindowSize(winW, winH);
	size_t base = 0;
	uint nDrawn = 0;
	for (size_t si = 0; si < segs.size(); ++si)
	{
		const uint n = segs[si];
		if (n < 2 || base + n > pts.size())
			break;
		for (uint i = 0; i + 1 < n; ++i)
		{
			if (!ok[base + i] || !ok[base + i + 1])
				continue;
			const NLMISC::CVector &a = proj[base + i];
			const NLMISC::CVector &b = proj[base + i + 1];
			if ((a.x < -0.2f && b.x < -0.2f) || (a.x > 1.2f && b.x > 1.2f)
			    || (a.y < -0.2f && b.y < -0.2f) || (a.y > 1.2f && b.y > 1.2f))
				continue;
			if (winW && winH)
			{
				NLGUI::CWidgetManager *wm = NLGUI::CWidgetManager::getInstance();
				const sint32 ax = (sint32)(a.x * winW), ay = (sint32)(a.y * winH);
				const sint32 bx = (sint32)(b.x * winW), by = (sint32)(b.y * winH);
				if (wm->getWindowUnder(ax, ay) || wm->getWindowUnder(bx, by))
					continue;
			}
			for (int pass = 0; pass < passes; ++pass)
			{
				NL3D::CDRU::drawLine(a.x + ox[pass], a.y + oy[pass],
				                     b.x + ox[pass], b.y + oy[pass],
				                     *driver, col, viewport);
				++nDrawn;
			}
		}
		base += n;
	}
	static bool s_CountOnce = false;
	if (!s_CountOnce)
	{
		printf("prop-outline: zone %u '%s' loops=%u boundary_edges=%u polylines=%u pts=%u drawn=%u thick=%d (screen-space, no Z-test)\n",
		       pz.ZoneId, pz.Name.c_str(), nLoops, nEdges, (uint)segs.size(),
		       (uint)pts.size(), nDrawn, (int)thick);
		s_CountOnce = true;
	}
	else
	{
		// Always log once per zone id change (selection screenshots)
		static uint s_LastLoggedZone = (uint)-1;
		if (pz.ZoneId != s_LastLoggedZone)
		{
			printf("prop-outline: zone %u '%s' loops=%u boundary_edges=%u polylines=%u pts=%u thick=%d\n",
			       pz.ZoneId, pz.Name.c_str(), nLoops, nEdges, (uint)segs.size(),
			       (uint)pts.size(), (int)thick);
			s_LastLoggedZone = pz.ZoneId;
		}
	}
}

/** Find SPaintZone by landscape zone id (g_PaintCtx.Zones). */
const SPaintZone *zpFindPaintZone(uint zoneId)
{
	if (!g_PaintCtx.Zones)
		return NULL;
	for (size_t i = 0; i < g_PaintCtx.Zones->size(); ++i)
		if ((*g_PaintCtx.Zones)[i].ZoneId == zoneId)
			return &(*g_PaintCtx.Zones)[i];
	return NULL;
}

SPaintZone *zpFindPaintZoneMut(uint zoneId)
{
	if (!g_PaintCtx.Zones)
		return NULL;
	for (size_t i = 0; i < g_PaintCtx.Zones->size(); ++i)
		if ((*g_PaintCtx.Zones)[i].ZoneId == zoneId)
			return &(*g_PaintCtx.Zones)[i];
	return NULL;
}

// ---------------------------------------------------------------------------------------------
// zone export properties (same Max-shape script AppData as the exporters)

/** BST_CHECKED / BST_UNCHECKED as used by nel_export_node_properties for LigoSymmetry. */
enum { ZP_BST_UNCHECKED = 0, ZP_BST_CHECKED = 1 };

bool zpEraseScriptAppData(CNodeImpl *node, uint32 subId)
{
	if (!node)
		return false;
	STORAGE::CAppData *ad = node->appData();
	if (!ad)
		return false;
	ad->erase(STORAGE::CAppData::ScriptClassId, STORAGE::CAppData::ScriptSuperClassId, subId);
	return true;
}

bool zpSetScriptAppDataStr(CNodeImpl *node, uint32 subId, const std::string &value)
{
	if (!node)
		return false;
	STORAGE::CAppData *ad = node->appData();
	if (!ad)
		return false;
	return ad->setScriptString(subId, value);
}


void zpReadZoneProps(CNodeImpl *node, SZoneProps &out)
{
	out = SZoneProps();
	if (!node)
		return;
	std::string s;
	if (APPDATA::getScriptAppData(node, NEL3D_APPDATA_ZONE_ROTATE, s))
	{
		out.HasRotate = true;
		NLMISC::fromString(s, out.Rotate);
		out.Rotate &= 3;
	}
	if (APPDATA::getScriptAppData(node, NEL3D_APPDATA_ZONE_SYMMETRY, s))
	{
		out.HasSymmetry = true;
		int v = 0;
		NLMISC::fromString(s, v);
		out.Symmetry = (v != ZP_BST_UNCHECKED);
	}
	// PASSABLE: presence-style - entry exists (any value, Max writes "1") = true
	if (APPDATA::getScriptAppData(node, NEL3D_APPDATA_LIGO_PASSABLE, s))
	{
		out.HasPassable = true;
		out.Passable = true;
	}
	if (APPDATA::getScriptAppData(node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, s))
	{
		out.HasUseBB = true;
		int v = 0;
		NLMISC::fromString(s, v);
		out.UseBoundingBox = (v != 0);
	}
}

/**
 * Write one export prop. Semantics match Max UIs:
 *   rotate   → NEL3D_APPDATA_ZONE_ROTATE decimal "0".."3" (always present after write)
 *   symmetry → NEL3D_APPDATA_ZONE_SYMMETRY "1"/"0" (BST_CHECKED/UNCHECKED)
 *   passable → presence: set "1" when true, DELETE entry when false (ligoscape rollout)
 *   usebbox  → "1" when true; DELETE when false (exporter getScriptAppDataInt default 0;
 *              absent and "0" both read false - delete is the least-surprising clean write)
 * When paint core is live, writes go through opProp so Ctrl+Z undoes them .
 */
bool zpWriteZoneProp(uint zoneId, const std::string &which, int value, std::string &err)
{
	SPaintZone *pz = zpFindPaintZoneMut(zoneId);
	if (!pz || !pz->Node)
	{
		err = "no zone/node";
		return false;
	}
	if (!zpZoneIsPropSelectable(zoneId))
	{
		err = "read-only";
		return false;
	}
	uint32 appId = 0;
	bool newHas = true;
	std::string newVal;
	if (which == "rotate")
	{
		appId = NEL3D_APPDATA_ZONE_ROTATE;
		newVal = NLMISC::toString("%d", value & 3);
	}
	else if (which == "symmetry")
	{
		appId = NEL3D_APPDATA_ZONE_SYMMETRY;
		newVal = NLMISC::toString("%d", value ? ZP_BST_CHECKED : ZP_BST_UNCHECKED);
	}
	else if (which == "passable")
	{
		appId = NEL3D_APPDATA_LIGO_PASSABLE;
		if (value)
			newVal = "1";
		else
			newHas = false;
	}
	else if (which == "usebbox")
	{
		appId = NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX;
		if (value)
			newVal = "1";
		else
			newHas = false;
	}
	else
	{
		err = "unknown prop " + which;
		return false;
	}
	if (g_PaintCtx.Core)
	{
		if (!g_PaintCtx.Core->opProp(zoneId, appId, newHas, newVal, err))
			return false;
	}
	else
	{
		// No core (rare): direct write without undo
		if (newHas)
		{
			if (!zpSetScriptAppDataStr(pz->Node, appId, newVal))
			{
				err = "setScriptString failed";
				return false;
			}
		}
		else
			zpEraseScriptAppData(pz->Node, appId);
	}
	// Live footprint re-derive when usebbox changes on the footprint-source zone - the
	// FIRST NON-FROZEN zone, exactly the one derivePrimaryFootprint picks (zones[0] can be
	// a frozen RO/embedded copy when scene order puts it first). Board sessions skip the
	// instant derive UNIFORMLY : per-file masks refresh at the next rebuild for
	// every open file alike, and an in-place derive would read translated geometry.
	if (which == "usebbox" && !g_BoardSession && g_PaintCtx.Zones && zpZoneIsFootprintSource(zoneId))
	{
		derivePrimaryFootprint(*g_PaintCtx.Zones, 0, g_PaintCtx.Zones->size(),
		                       g_SessionCellSize > 0.f ? g_SessionCellSize : 160.f,
		                       g_SessionSnap > 0.f ? g_SessionSnap : 1.f);
	}
	g_PropStatusMsg = which + "=" + NLMISC::toString("%d", value);
	return true;
}

/** True when zoneId is the zone derivePrimaryFootprint picks (first non-frozen). */
bool zpZoneIsFootprintSource(uint zoneId)
{
	if (!g_PaintCtx.Zones)
		return false;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (size_t i = 0; i < zones.size(); ++i)
	{
		if (!zones[i].Frozen)
			return zones[i].ZoneId == zoneId;
	}
	return false;
}

/** prop undo/redo re-derives footprint when USE_BOUNDINGBOX is restored. */
void zpOnPropChanged(uint zoneId, uint32 appDataId)
{
	if (appDataId != NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX)
		return;
	if (g_BoardSession)
		return; // board sessions refresh per-file masks uniformly at the next rebuild
	if (!g_PaintCtx.Zones || g_PaintCtx.Zones->empty())
		return;
	if (!zpZoneIsFootprintSource(zoneId))
		return;
	derivePrimaryFootprint(*g_PaintCtx.Zones, 0, g_PaintCtx.Zones->size(),
	                       g_SessionCellSize > 0.f ? g_SessionCellSize : 160.f,
	                       g_SessionSnap > 0.f ? g_SessionSnap : 1.f);
}

// Prop panel handlers (shared with future paint-script prop ops)
void zpPropRotateDelta(int d)
{
	if (!g_HavePropSelection) return;
	SZoneProps p;
	const SPaintZone *pz = zpFindPaintZone(g_SelectedZoneId);
	if (!pz) return;
	zpReadZoneProps(pz->Node, p);
	const int next = (p.Rotate + d) & 3;
	std::string err;
	if (!zpWriteZoneProp(g_SelectedZoneId, "rotate", next, err))
		g_PropStatusMsg = err;
	else
		ZPSCRIPT::record(NLMISC::toString("painter.setZoneProp(%u, \"rotate\", %d)", g_SelectedZoneId, next));
}

void zpPropToggleSymmetry()
{
	if (!g_HavePropSelection) return;
	const SPaintZone *pz = zpFindPaintZone(g_SelectedZoneId);
	if (!pz) return;
	SZoneProps p;
	zpReadZoneProps(pz->Node, p);
	std::string err;
	if (!zpWriteZoneProp(g_SelectedZoneId, "symmetry", p.Symmetry ? 0 : 1, err))
		g_PropStatusMsg = err;
	else
		ZPSCRIPT::record(NLMISC::toString("painter.setZoneProp(%u, \"symmetry\", %d)", g_SelectedZoneId, p.Symmetry ? 0 : 1));
}

void zpPropTogglePassable()
{
	if (!g_HavePropSelection) return;
	const SPaintZone *pz = zpFindPaintZone(g_SelectedZoneId);
	if (!pz) return;
	SZoneProps p;
	zpReadZoneProps(pz->Node, p);
	std::string err;
	if (!zpWriteZoneProp(g_SelectedZoneId, "passable", p.Passable ? 0 : 1, err))
		g_PropStatusMsg = err;
	else
		ZPSCRIPT::record(NLMISC::toString("painter.setZoneProp(%u, \"passable\", %d)", g_SelectedZoneId, p.Passable ? 0 : 1));
}

void zpPropToggleUseBBox()
{
	if (!g_HavePropSelection) return;
	const SPaintZone *pz = zpFindPaintZone(g_SelectedZoneId);
	if (!pz) return;
	SZoneProps p;
	zpReadZoneProps(pz->Node, p);
	std::string err;
	if (!zpWriteZoneProp(g_SelectedZoneId, "usebbox", p.UseBoundingBox ? 0 : 1, err))
		g_PropStatusMsg = err;
	else
		ZPSCRIPT::record(NLMISC::toString("painter.setZoneProp(%u, \"usebbox\", %d)", g_SelectedZoneId, p.UseBoundingBox ? 0 : 1));
}

/** Basename of the editable file owning a zone id (panel readout). */
std::string zpZoneFileBasename(uint zoneId)
{
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		const SEditableFileInfo &efi = g_EditableFiles[i];
		for (size_t z = 0; z < efi.ZoneIds.size(); ++z)
			if (efi.ZoneIds[z] == zoneId)
				return efi.Basename;
	}
	if (!g_StartupZone.Basename.empty())
		return g_StartupZone.Basename;
	return NLMISC::CFile::getFilenameWithoutExtension(g_InputPath);
}

/** Headless dump of the four export props per zone node (verification hook). */
int dumpZoneProps(const std::string &path)
{
	NL3D::registerSerial3d();
	PMAXLOAD::SLoadedMax lm;
	if (!PMAXLOAD::loadMaxFile(path, lm) || !lm.Scene)
	{
		fprintf(stderr, "ERROR: dump-zone-props cannot load %s\n", path.c_str());
		return 1;
	}
	std::vector<SZoneNode> nodes;
	collectZoneNodes(*lm.Scene, nodes);
	const std::string basen = NLMISC::CFile::getFilenameWithoutExtension(path);
	std::vector<bool> eligible;
	computeZoneEligibility(nodes, basen, eligible);
	printf("zone-props dump '%s': %u node(s)\n", basen.c_str(), (uint)nodes.size());
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const std::string name = ucstring(nodes[i].Node->userName()).toUtf8();
		SZoneProps p;
		zpReadZoneProps(nodes[i].Node, p);
		const bool elig = i < eligible.size() && eligible[i];
		printf("  zone '%s' frozen=%d eligible=%d  rotate=%d%s  symmetry=%d%s  passable=%d%s  usebbox=%d%s\n",
		       name.c_str(), (int)nodes[i].Frozen, (int)elig,
		       p.Rotate, p.HasRotate ? "" : "(default)",
		       (int)p.Symmetry, p.HasSymmetry ? "" : "(default)",
		       (int)p.Passable, p.HasPassable ? "" : "(absent)",
		       (int)p.UseBoundingBox, p.HasUseBB ? "" : "(default)");
	}
	return 0;
}


void zpSelectTileSetDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	uint count = g_PaintCtx.Core->tileSetCount();
	if (!count) return;
	int n = (int)count;
	int cur = g_PaintCtx.Paint->CurTileSet;
	cur = (cur + d) % n;
	if (cur < 0) cur += n;
	if (cur == g_PaintCtx.Paint->CurTileSet)
		return;
	g_PaintCtx.Paint->CurTileSet = cur;
	// Record the RESULTING absolute set (recorder convention: abs painter.* lines) -
	// the keyboard/panel delta path silently skipped recording while the palette's
	// abs path recorded, leaving replayed sessions with the wrong live tile set.
	ZPSCRIPT::record(NLMISC::toString("painter.setTileSet(%d)", cur));
	// Displace map files are per-tileset .
	zpRebuildTilesetPalette();
}

void zpSelectTileSetAbs(int idx)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	uint count = g_PaintCtx.Core->tileSetCount();
	if (!count || idx < 0 || idx >= (int)count) return;
	const int prev = g_PaintCtx.Paint->CurTileSet;
	g_PaintCtx.Paint->CurTileSet = idx;
	ZPSCRIPT::record(NLMISC::toString("painter.setTileSet(%d)", idx));
	// Displace map files are per-tileset; refresh the palette section when the set changes .
	if (prev != idx)
		zpRebuildTilesetPalette();
}

/** Absolute 128/256 tile mode (painterscript); records the resulting state. */
void zpSetTileSize256(bool on)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (g_PaintCtx.Paint->Mode256 == on) return;
	g_PaintCtx.Paint->Mode256 = on;
	ZPSCRIPT::record(on ? "painter.set256(true)" : "painter.set256(false)");
}

void zpToggleTileSize()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	zpSetTileSize256(!g_PaintCtx.Paint->Mode256);
}

/** Absolute color brush radius (meters, clamp 2..32); records the resulting state. */
void zpColorRadiusAbs(float m)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (m < 2.f) m = 2.f;
	if (m > 32.f) m = 32.f;
	if (g_PaintCtx.Paint->BrushRadius == m) return;
	g_PaintCtx.Paint->BrushRadius = m;
	// %.9g float-exact (matches the REC preamble and colorBrush lines) - %.3f quantized
	// the replayed radius after ×1.5/÷1.5 stepping.
	ZPSCRIPT::record(NLMISC::toString("painter.setRadius(%.9g)", m));
}

/** Color brush radius step (×1.5 / ÷1.5, clamp 2..32). Shared by SizeUp/Down in Color mode and panel. */
void zpColorRadiusDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (d > 0)
		zpColorRadiusAbs(g_PaintCtx.Paint->BrushRadius * 1.5f);
	else if (d < 0)
		zpColorRadiusAbs(g_PaintCtx.Paint->BrushRadius / 1.5f);
}

void zpBrushSizeDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	if (g_PaintCtx.Paint->Mode == CPaintMouseListener::ModeColor)
		zpColorRadiusDelta(d);
	else
	{
		int bs = (int)g_PaintCtx.Core->brushSize() + d;
		if (bs < 0) bs = 0;
		if (bs > 2) bs = 2;
		if ((uint)bs != g_PaintCtx.Core->brushSize())
		{
			g_PaintCtx.Core->setBrushSize((uint)bs);
			ZPSCRIPT::record(NLMISC::toString("painter.setBrushSize(%d)", bs));
		}
	}
}

void zpGroupDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	int g = (int)g_PaintCtx.Core->tileGroup() + d;
	g = ((g % 13) + 13) % 13;
	g_PaintCtx.Core->setTileGroup((uint)g);
	ZPSCRIPT::record(NLMISC::toString("painter.setTileGroup(%d)", g));
}

/** Absolute hardness 0..255 (painterscript); records the resulting state. */
void zpHardnessAbs(int v)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (v < 0) v = 0;
	if (v > 255) v = 255;
	if (g_PaintCtx.Paint->BrushHardness == (uint)v) return;
	g_PaintCtx.Paint->BrushHardness = (uint)v;
	ZPSCRIPT::record(NLMISC::toString("painter.setHardness(%d)", v));
}

/** Hardness ± (plugin ±0.2 on 0..1 → ±51 on 0..255). Keys Home/End + panel. */
void zpHardnessDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	zpHardnessAbs((int)g_PaintCtx.Paint->BrushHardness + d);
}

/** Absolute opacity 0..255 (painterscript); records the resulting state. */
void zpOpacityAbs(int v)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (v < 0) v = 0;
	if (v > 255) v = 255;
	if (g_PaintCtx.Paint->BrushOpacity == (uint)v) return;
	g_PaintCtx.Paint->BrushOpacity = (uint)v;
	ZPSCRIPT::record(NLMISC::toString("painter.setOpacity(%d)", v));
}

/** Opacity ± (same step scale as hardness). Keys Insert/Delete + panel. */
void zpOpacityDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	zpOpacityAbs((int)g_PaintCtx.Paint->BrushOpacity + d);
}

/** Cycle brush mask: none → file1 → … → none (S key / panel). */
void zpCycleBrushMask()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || g_MaskFiles.empty()) return;
	g_MaskCycle = (g_MaskCycle + 1) % ((int)g_MaskFiles.size() + 1);
	std::string err;
	if (g_MaskCycle == 0)
	{
		g_PaintCtx.Core->clearBrushMask();
		ZPSCRIPT::record("painter.setBrushMask(\"none\")");
	}
	else if (!g_PaintCtx.Core->loadBrushMask(g_MaskFiles[g_MaskCycle - 1], err))
		fprintf(stderr, "WARNING: %s\n", err.c_str());
	else
		ZPSCRIPT::record(NLMISC::toString("painter.setBrushMask(%s)",
			luaQuote(g_MaskFiles[g_MaskCycle - 1]).c_str()));
}

/** Toggle color-brush mask mode (Q key / panel). Records via the op-backed setter line. */
void zpToggleMaskMode()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	const bool on = !g_PaintCtx.Core->brushMaskMode();
	g_PaintCtx.Core->setBrushMaskMode(on);
	ZPSCRIPT::record(on ? "painter.setMaskMode(true)" : "painter.setMaskMode(false)");
}

/** Displace paint index ±1 mod 16 ([ ] keys / panel). */
void zpDisplaceIndexDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	int v = ((int)g_PaintCtx.Paint->DisplaceIndex + d) % 16;
	if (v < 0) v += 16;
	if (g_PaintCtx.Paint->DisplaceIndex == (uint)v) return;
	g_PaintCtx.Paint->DisplaceIndex = (uint)v;
	ZPSCRIPT::record(NLMISC::toString("painter.setDisplaceIndex(%d)", v));
}


void zpDisplaceIndexAbs(int idx)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (idx < 0) idx = 0;
	if (idx > 15) idx = 15;
	if (g_PaintCtx.Paint->DisplaceIndex == (uint)idx) return;
	g_PaintCtx.Paint->DisplaceIndex = (uint)idx;
	ZPSCRIPT::record(NLMISC::toString("painter.setDisplaceIndex(%d)", idx));
}

/** Brush color from the color picker ; same field CLI --color initializes. */
void zpSetBrushColor(int r, int g, int b)
{
	if (r < 0) r = 0; if (r > 255) r = 255;
	if (g < 0) g = 0; if (g > 255) g = 255;
	if (b < 0) b = 0; if (b > 255) b = 255;
	g_ViewerBrushColor = NLMISC::CRGBA((uint8)r, (uint8)g, (uint8)b, 255);
	if (g_PaintCtx.Paint)
		g_PaintCtx.Paint->BrushColor = g_ViewerBrushColor;
	ZPSCRIPT::record(NLMISC::toString("painter.setBrushColor(%d, %d, %d)", r, g, b));
}

void zpToggleLockBorders()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	const bool on = !g_PaintCtx.Core->lockBordersOn();
	g_PaintCtx.Core->setLockBorders(on);
	ZPSCRIPT::record(on ? "painter.setLockBorders(true)" : "painter.setLockBorders(false)");
}

/**
 * A replayed TOPOLOGY record (Kind 6) restored storage from inside the core, which cannot
 * rebuild the session that owns it. Rebuild here, once per undo/redo, with the same two
 * flags the op used: the pristine is stale relative to the restored storage (skip the
 * write-back that would clobber it) and the stacks must survive the re-init.
 *
 * Every undo/redo surface funnels here: the U/I keys via zpUndo/zpRedo, the painterscript
 * `undo`/`redo` op lines via the executor. Headless paint-script runs never set the flag
 * (topological ops need a live session), so the guard is cheap there.
 */
void zpHandleTopoRestorePending()
{
	if (!g_PaintCtx.Core || !g_PaintCtx.Core->topoRestorePending())
		return;
	g_PaintCtx.Core->clearTopoRestorePending();
	// Stale sub-object selections died with the restored topology.
	g_PatchVertSel.clear();
	g_PatchEdgeSel.clear();
	g_PatchFaceSel.clear();
	g_PatchTanSel.clear();
	zpPatchGizmoInvalidate();
	std::string err;
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds, /* skipWriteBack= */ true, /* keepUndo= */ true))
		g_PropStatusMsg = "undo: session rebuild failed: " + err;
	else
		g_PropStatusMsg = "topology restored";
}

void zpUndo()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	g_PaintCtx.Core->opUndo();
	zpHandleTopoRestorePending();
	ZPSCRIPT::record("painter.undo()");
}

void zpRedo()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	g_PaintCtx.Core->opRedo();
	zpHandleTopoRestorePending();
	ZPSCRIPT::record("painter.redo()");
}

void zpFill(int rot)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	CPaintMouseListener &pl = *g_PaintCtx.Paint;
	if (!pl.HaveHover || g_PaintCtx.Core->zoneFrozen(pl.HoverZone)) return;
	std::string err;
	uint patch = (uint)(pl.HoverTile / ZP_NUM_TILE_SEL);
	if (pl.Mode == CPaintMouseListener::ModeTile)
	{
		if (g_PaintCtx.Core->opFillTile(pl.HoverZone, patch, pl.CurTileSet, rot, pl.Mode256, err))
			ZPSCRIPT::record(NLMISC::toString("painter.fillTile(%u, %u, %d, %d, %s)",
				pl.HoverZone, patch, pl.CurTileSet, rot, pl.Mode256 ? "true" : "false"));
	}
	else if (pl.Mode == CPaintMouseListener::ModeColor)
	{
		if (g_PaintCtx.Core->opFillColor(pl.HoverZone, patch, pl.BrushColor, 256, err))
			ZPSCRIPT::record(NLMISC::toString("painter.fillColor(%u, %u, \"%02x%02x%02x\")",
				pl.HoverZone, patch, pl.BrushColor.R, pl.BrushColor.G, pl.BrushColor.B));
	}
	else
	{
		if (g_PaintCtx.Core->opFillDisplace(pl.HoverZone, patch, pl.DisplaceIndex, err))
			ZPSCRIPT::record(NLMISC::toString("painter.fillDisplace(%u, %u, %u)",
				pl.HoverZone, patch, pl.DisplaceIndex));
	}
}


// ---------------------------------------------------------------------------------------------
// painterscript host : the Lua binding's window into the op layer. All calls
// route through the SAME functions the keys/UI/--paint-script use (single op layer).

bool zpScriptExecOp(const std::string &line, std::string &err)
{
	if (!g_PaintCtx.Core) { err = "no active paint session"; return false; }
	std::string op;
	bool unknown = false;
	return zpExecScriptOp(*g_PaintCtx.Core, line, err, &op, &unknown);
}

void zpScriptZonesInfo(std::vector<ZPSCRIPT::SZoneInfo> &out)
{
	out.clear();
	if (!g_PaintCtx.Zones) return;
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	std::string primaryBase = NLMISC::CFile::getFilenameWithoutExtension(g_PaintCtx.InputPath);
	for (size_t i = 0; i < zones.size(); ++i)
	{
		ZPSCRIPT::SZoneInfo zi;
		zi.Id = zones[i].ZoneId;
		zi.Name = zones[i].Name;
		zi.Frozen = zones[i].Frozen;
		zi.Editable = !zones[i].Frozen;
		zi.Dirty = g_PaintCtx.Core ? g_PaintCtx.Core->isZoneDirty(zones[i].ZoneId) : false;
		// Source file: primary-file zones get the input basename; multi-session files are
		// tracked per id range in g_EditableFiles.
		zi.File = primaryBase;
		for (size_t f = 0; f < g_EditableFiles.size(); ++f)
		{
			const std::vector<uint> &ids = g_EditableFiles[f].ZoneIds;
			for (size_t k = 0; k < ids.size(); ++k)
				if (ids[k] == zones[i].ZoneId)
					zi.File = NLMISC::CFile::getFilenameWithoutExtension(g_EditableFiles[f].Path);
		}
		out.push_back(zi);
	}
}

bool zpScriptGetZoneProp(uint zoneId, const std::string &which, int &value, std::string &err)
{
	const SPaintZone *pz = zpFindPaintZone(zoneId);
	if (!pz || !pz->Node) { err = NLMISC::toString("no zone %u", zoneId); return false; }
	SZoneProps p;
	zpReadZoneProps(const_cast<CNodeImpl *>(pz->Node), p);
	if (which == "rotate") value = p.Rotate;
	else if (which == "symmetry") value = p.Symmetry ? 1 : 0;
	else if (which == "passable") value = p.Passable ? 1 : 0;
	else if (which == "usebbox") value = p.UseBoundingBox ? 1 : 0;
	else { err = "unknown property '" + which + "' (rotate|symmetry|passable|usebbox)"; return false; }
	return true;
}

// Viewer-only host capabilities (screenshot / pump / cancel) are installed by runViewer.
// g_ScriptScreenshotFn / g_ScriptPumpFn / g_ScriptCancel are defined in main.cpp.
bool zpScriptCancelRequested() { return g_ScriptCancel; }
void zpScriptResetCancel() { g_ScriptCancel = false; }


bool zpScriptOpenZone(const std::string &basename, std::string &err)
{
	if (!g_SessionOpsAvailable) { err = "openZone: board session only"; return false; }
	return sessionOpenZone(basename, err);
}
// (sessionOpenZone itself rejects ecosystem sessions - eco opens need a board cell.)


bool zpScriptEcoGate(const char *op, std::string &err)
{
	if (!g_SessionOpsAvailable) { err = std::string(op) + ": board session only"; return false; }
	if (g_StartupWorld.Kind != ZPWS::Ecosystem || g_StartupWorld.WorldName.empty())
	{ err = std::string(op) + ": ecosystem board only"; return false; }
	return true;
}
bool zpScriptOpenZoneAt(const std::string &basename, int cx, int cy, std::string &err)
{
	if (!zpScriptEcoGate("openZone(cx,cy)", err)) return false;
	return scratchOpenEditable(cx, cy, basename, err);
}
bool zpScriptPlaceInstance(int cx, int cy, const std::string &basename, std::string &err)
{
	if (!zpScriptEcoGate("placeInstance", err)) return false;
	return basename.empty() ? scratchPlace(cx, cy, err)
	                        : scratchPlaceInstanceOf(cx, cy, basename, err);
}
bool zpScriptRemoveInstance(int cx, int cy, std::string &err)
{
	if (!zpScriptEcoGate("removeInstance", err)) return false;
	return scratchRemove(cx, cy, err);
}
bool zpScriptRotateInstance(int cx, int cy, int delta, std::string &err)
{
	if (!zpScriptEcoGate("rotateInstance", err)) return false;
	return scratchRotate(cx, cy, delta, err);
}
bool zpScriptMirrorInstance(int cx, int cy, std::string &err)
{
	if (!zpScriptEcoGate("mirrorInstance", err)) return false;
	return scratchMirror(cx, cy, err);
}

bool zpScriptCloseZone(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err)
{
	if (!g_SessionOpsAvailable) { err = "closeZone: board session only"; return false; }
	return sessionCloseZone(basename, saveFirst, forceDiscard, err);
}

// Board-op completion: same gating split as the UI: scratch ops are eco-board,
// toggle/save are any board session.
bool zpScriptPlaceContext(int cx, int cy, const std::string &basename, std::string &err)
{
	if (!zpScriptEcoGate("placeContext", err)) return false;
	return scratchPlaceContext(cx, cy, basename, err);
}
bool zpScriptRemoveContext(int cx, int cy, std::string &err)
{
	if (!zpScriptEcoGate("removeContext", err)) return false;
	return scratchRemoveContext(cx, cy, err);
}
bool zpScriptRotateContext(int cx, int cy, int delta, std::string &err)
{
	if (!zpScriptEcoGate("rotateContext", err)) return false;
	return scratchRotateContext(cx, cy, delta, err);
}
bool zpScriptMirrorContext(int cx, int cy, std::string &err)
{
	if (!zpScriptEcoGate("mirrorContext", err)) return false;
	return scratchMirrorContext(cx, cy, err);
}
bool zpScriptMakeEditable(int cx, int cy, std::string &err)
{
	if (!zpScriptEcoGate("makeEditable", err)) return false;
	return scratchContextToEditable(cx, cy, err);
}
bool zpScriptDragCell(int fx, int fy, int tx, int ty, bool copy, std::string &err)
{
	if (!zpScriptEcoGate(copy ? "copyCell" : "moveCell", err)) return false;
	return scratchDragDrop(fx, fy, tx, ty, copy, err);
}
bool zpScriptToggleZone(const std::string &basename, bool saveFirst, bool forceDiscard,
                               std::string &err)
{
	if (!g_SessionOpsAvailable) { err = "toggleZone: board session only"; return false; }
	return sessionToggleEditable(basename, saveFirst, forceDiscard, err);
}
bool zpScriptSaveZone(const std::string &basename, std::string &err)
{
	if (!g_SessionOpsAvailable) { err = "saveZone: board session only"; return false; }
	return sessionSaveZone(basename, err);
}


void zpInstallScriptHost(ZPUI::SPaintUIBridge *bridgePtr)
{
	g_ScriptHost.execOp = zpScriptExecOp;
	g_ScriptHost.refreshBridge = zpScriptRefreshBridge;
	g_ScriptHost.zonesInfo = zpScriptZonesInfo;
	g_ScriptHost.getZoneProp = zpScriptGetZoneProp;
	g_ScriptHost.saveTo = zpSaveTo;
	g_ScriptHost.saveAll = zpSaveOverwrite;
	g_ScriptHost.screenshot = g_ScriptScreenshotFn;
	g_ScriptHost.pumpUI = g_ScriptPumpFn;
	g_ScriptHost.cancelRequested = zpScriptCancelRequested;
	g_ScriptHost.resetCancel = zpScriptResetCancel;
	g_ScriptHost.openZone = zpScriptOpenZone;
	g_ScriptHost.closeZone = zpScriptCloseZone;
	g_ScriptHost.openZoneAt = zpScriptOpenZoneAt;
	g_ScriptHost.placeInstance = zpScriptPlaceInstance;
	g_ScriptHost.removeInstance = zpScriptRemoveInstance;
	g_ScriptHost.rotateInstance = zpScriptRotateInstance;
	g_ScriptHost.mirrorInstance = zpScriptMirrorInstance;
	g_ScriptHost.placeContext = zpScriptPlaceContext;
	g_ScriptHost.removeContext = zpScriptRemoveContext;
	g_ScriptHost.rotateContext = zpScriptRotateContext;
	g_ScriptHost.mirrorContext = zpScriptMirrorContext;
	g_ScriptHost.makeEditable = zpScriptMakeEditable;
	g_ScriptHost.dragCell = zpScriptDragCell;
	g_ScriptHost.toggleZone = zpScriptToggleZone;
	g_ScriptHost.saveZone = zpScriptSaveZone;
	g_ScriptHost.bridge = bridgePtr;
	ZPSCRIPT::setHost(&g_ScriptHost);
}

// painterscript viewer pump : refresh gated at >100ms of processing since the
// last actual pump so tight script loops cost nothing; input locked except ESC=cancel.

void zpScriptPumpImpl()
{
	if (!g_PumpCtx.Driver || !g_PumpCtx.Scene)
		return;
	NLMISC::TTime now = NLMISC::CTime::getLocalTime();
	if (g_PumpCtx.LastPump != 0 && now - g_PumpCtx.LastPump < 100)
		return;
	g_PumpCtx.LastPump = now;
	g_ScriptUiLock = true;
	NLMISC::CMatrix navBefore;
	if (g_PumpCtx.Nav) navBefore = g_PumpCtx.Nav->getViewMatrix();
	g_PumpCtx.Driver->EventServer.pump();
	if (g_PumpCtx.Nav) g_PumpCtx.Nav->setMatrix(navBefore); // discard nav during scripts
	if (g_PumpCtx.Driver->AsyncListener.isKeyPushed(NLMISC::KeyESCAPE))
		g_ScriptCancel = true;
	// Refresh the bridge snapshot so panel sync (and script getters) see current state
	zpScriptRefreshBridge();
	g_PumpCtx.Driver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
	g_PumpCtx.Scene->render();
	if (g_PumpCtx.Ui) { g_PumpCtx.Ui->update(); g_PumpCtx.Ui->draw(); }
	g_PumpCtx.Driver->swapBuffers();
	g_ScriptUiLock = false;
}

bool zpScriptScreenshotImpl(const std::string &path, std::string &err)
{
	if (!g_PumpCtx.Driver || !g_PumpCtx.Scene) { err = "screenshot: viewer only"; return false; }
	g_PumpCtx.Driver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
	g_PumpCtx.Scene->render();
	if (g_PumpCtx.Ui) { g_PumpCtx.Ui->update(); g_PumpCtx.Ui->draw(); }
	// Read the just-drawn backbuffer BEFORE swap (glReadPixels default = GL_BACK; after
	// swap it holds the previous frame, or garbage on swap-exchange drivers) - same rule
	// as the --screenshot path's refine/redraw comment.
	NLMISC::CBitmap btm;
	static_cast<NL3D::CDriverUser *>(g_PumpCtx.Driver)->getDriver()->getBuffer(btm);
	g_PumpCtx.Driver->swapBuffers();
	NLMISC::COFile fs;
	if (!fs.open(path)) { err = "cannot open " + path; return false; }
	btm.writeTGA(fs, 24);
	printf("OK lua screenshot: -> %s\n", path.c_str());
	return true;
}


/** Season cache tag for tileset previews (preference code, or "auto"). */
std::string zpSeasonCacheKey()
{
	const std::string &pref = ZPCTX::seasonPreference();
	return pref.empty() ? std::string("auto") : pref;
}

/** Rebuild the Tiles + Displace palette grids; no-op without a bank. */
void zpRebuildTilesetPalette()
{
	int ts = 0;
	if (g_PaintCtx.Paint)
		ts = g_PaintCtx.Paint->CurTileSet;
	if (!g_PaintCtx.Bank)
	{
		ZPUI::rebuildTilesetPalette(NULL, std::string(), zpSeasonCacheKey(), ts);
		return;
	}
	ZPUI::rebuildTilesetPalette(g_PaintCtx.Bank, g_PaintCtx.BankPath, zpSeasonCacheKey(), ts);
}

void zpTogglePalette()
{
	ZPUI::toggleTilesetPalette();
}

void zpToggleBoard()
{
	ZPUI::toggleSessionBoard();
}


/** Apply current season preference + live-flush landscape tile textures (paint state untouched). */
void zpSeasonApplyFlush()
{
	printf("season: %s\n", ZPCTX::seasonPreferenceLabel().c_str());
	ZPSCRIPT::record(NLMISC::toString("painter.setSeason(%s)",
		luaQuote(ZPCTX::seasonPreference()).c_str()));
	if (!g_PaintCtx.Bank || !g_PaintCtx.Land || !g_PaintCtx.UDriver)
		return;
	NL3D::IDriver *driver = static_cast<NL3D::CDriverUser *>(g_PaintCtx.UDriver)->getDriver();
	// Live flush (preferred over full session reload): re-remap CPath season postfixes,
	// releaseTiles so CTextureFile re-looks up, optional preload flush.
	ZPCTX::reloadLandscapeSeasonTextures(*g_PaintCtx.Bank, g_PaintCtx.BankPath,
	                                     &g_PaintCtx.Land->Landscape, driver, g_PreloadTiles);
	// Palette previews follow the season : regenerate/invalidate cached thumbs.
	zpRebuildTilesetPalette();
}

/** Cycle season preference + live-flush (Y key / SeasonNext). */
void zpSeasonNext()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.AvailableSeasons || g_PaintCtx.AvailableSeasons->size() < 2)
		return;
	if (!ZPCTX::cycleSeasonPreference(*g_PaintCtx.AvailableSeasons))
		return;
	zpSeasonApplyFlush();
}

/** Select a specific season code (toolbar menu). */
void zpSeasonSelect(const std::string &code)
{
	if (!g_PaintCtx.Active)
		return;
	// Availability FIRST: mutating the preference before this check desynced the toolbar
	// label / Y-cycling / cache keys from the actual textures on the error path (only
	// scripts hit it - the UI menu offers valid seasons only).
	if (g_PaintCtx.AvailableSeasons && !g_PaintCtx.AvailableSeasons->empty() && !code.empty())
	{
		bool ok = false;
		for (size_t i = 0; i < g_PaintCtx.AvailableSeasons->size(); ++i)
			if ((*g_PaintCtx.AvailableSeasons)[i] == code) { ok = true; break; }
		if (!ok)
		{
			fprintf(stderr, "season select: '%s' not available for this bank\n", code.c_str());
			return;
		}
	}
	if (!ZPCTX::setSeasonPreference(code))
	{
		fprintf(stderr, "season select: unknown code '%s'\n", code.c_str());
		return;
	}
	zpSeasonApplyFlush();
}

/** Show season picker buttons for discovered seasons (modal s0..s3). */
void zpSeasonMenuFill(void * /*unused*/)
{
	static const char *kIds[4] = {
		"ui:zp:season_menu:content:s0",
		"ui:zp:season_menu:content:s1",
		"ui:zp:season_menu:content:s2",
		"ui:zp:season_menu:content:s3",
	};
	for (int i = 0; i < 4; ++i)
	{
		NLGUI::CInterfaceElement *el = NLGUI::CWidgetManager::getInstance()->getElementFromId(kIds[i]);
		if (!el) continue;
		el->setActive(false);
	}
	if (!g_PaintCtx.AvailableSeasons) return;
	const size_t n = g_PaintCtx.AvailableSeasons->size();
	for (size_t i = 0; i < n && i < 4; ++i)
	{
		const std::string &code = (*g_PaintCtx.AvailableSeasons)[i];
		std::string lab = code;
		if (code == "sp") lab = "SPRING";
		else if (code == "su") lab = "SUMMER";
		else if (code == "au") lab = "AUTUMN";
		else if (code == "wi") lab = "WINTER";
		else lab = NLMISC::toUpperAscii(code);
		NLGUI::CInterfaceElement *el = NLGUI::CWidgetManager::getInstance()->getElementFromId(kIds[i]);
		if (!el) continue;
		el->setActive(true);
		if (NLGUI::CCtrlTextButton *tb = dynamic_cast<NLGUI::CCtrlTextButton *>(el))
		{
			tb->setHardText(lab);
			// params_l already set in XML to sp/su/au/wi in order - remap if discovery order differs
		}
		// Rebind onclick params to actual code
		if (NLGUI::CCtrlBaseButton *btn = dynamic_cast<NLGUI::CCtrlBaseButton *>(el))
		{
			btn->setActionOnLeftClick("zp_season_select");
			btn->setParamsOnLeftClick(code);
		}
	}
}

/**
 * Prop-panel footprint readout for a NON-primary zone, memoized.
 *
 * zpFillBridgeState runs every viewer frame, and deriveZoneFootprintMask is a full
 * NLLIGO::CZoneTemplate build over the zone's open edges (plus a stderr/stdout line on
 * the AABB-fallback path). Recomputing it per frame while a zone is merely SELECTED cost
 * real time and spammed the console once per frame on any zone whose template mask does
 * not build. The result only depends on the node's authored geometry (fixed for a Node
 * within a working set; the pointer changes on rebuild) plus the two appdata flags the
 * derivation reads and the session cell/snap - so key on exactly those.
 */
static void zpPropFootprintCached(const SPaintZone &pz, bool useBB, bool symmetry,
                                  int &cellsW, int &cellsH, bool &fromTemplate, bool &filled)
{
	// s_HaveKey: the cached key is populated (suppresses the retry-every-frame the derive
	// was doing). s_Ok: that derive produced a usable result.
	static bool s_HaveKey = false;
	static const PIPELINE::MAX::BUILTIN::CNodeImpl *s_Node = NULL;
	static bool s_UseBB = false, s_Symmetry = false;
	static float s_CellSize = 0.f, s_Snap = 0.f;
	static bool s_Ok = false;
	static int s_W = 1, s_H = 1;
	static bool s_FromT = false, s_Filled = true;

	const float cellSize = g_SessionCellSize > 0.f ? g_SessionCellSize : 160.f;
	const float snap = g_SessionSnap > 0.f ? g_SessionSnap : 1.f;
	if (!s_HaveKey || s_Node != pz.Node || s_UseBB != useBB || s_Symmetry != symmetry
	    || s_CellSize != cellSize || s_Snap != snap)
	{
		s_HaveKey = true;
		s_Node = pz.Node;
		s_UseBB = useBB;
		s_Symmetry = symmetry;
		s_CellSize = cellSize;
		s_Snap = snap;
		std::vector<bool> mask;
		float ox = 0.f, oy = 0.f;
		std::string e;
		int w = 1, h = 1;
		bool ft = false;
		s_Ok = deriveZoneFootprintMask(pz, cellSize, snap, mask, w, h, ox, oy, ft, e);
		if (s_Ok)
		{
			s_W = w;
			s_H = h;
			s_FromT = ft;
			s_Filled = !maskHasHole(mask);
		}
	}
	if (!s_Ok)
		return; // leave the caller's primary-footprint defaults
	cellsW = s_W;
	cellsH = s_H;
	fromTemplate = s_FromT;
	filled = s_Filled;
}

// Fill the UI bridge state snapshot (labels / button push state).
void zpScriptRefreshBridge();
void zpFillBridgeState(ZPUI::SPaintUIBridge &bridge)
{
	bridge.HaveCore = g_PaintCtx.Active && g_PaintCtx.Core && g_PaintCtx.Paint;
	if (!bridge.HaveCore) return;
	CPaintMouseListener &pl = *g_PaintCtx.Paint;
	bridge.Mode = pl.Mode;
	bridge.SubObj = pl.SubObj;
	// Patch rollout snapshot (cheap counts; the tri-state walks the edge selection only)
	bridge.PatchSelVerts = zpPatchVertSelCount();
	bridge.PatchSelEdges = zpPatchEdgeSelCount();
	bridge.PatchSelFaces = zpPatchFaceSelCount();
	bridge.PatchSelTans = zpPatchTangentSelCount();
	bridge.PatchNoSmooth = zpEdgeNoSmoothTriState();
	// The zones the CURRENT level's selection lives in: one -> show its name in the
	// readout, several -> show the count (a multi-file selection is easy to make and
	// invisible without this).
	{
		std::set<uint> selZones;
		if (pl.SubObj == CPaintMouseListener::SubEdge)
		{
			for (std::set<SPatchEdgeId>::const_iterator it = g_PatchEdgeSel.begin();
			     it != g_PatchEdgeSel.end(); ++it)
				selZones.insert(it->Zone);
		}
		else if (pl.SubObj == CPaintMouseListener::SubPatch)
		{
			for (std::set<TPatchFaceId>::const_iterator it = g_PatchFaceSel.begin();
			     it != g_PatchFaceSel.end(); ++it)
				selZones.insert(it->first);
		}
		else
		{
			for (std::set<TPatchVertId>::const_iterator it = g_PatchVertSel.begin();
			     it != g_PatchVertSel.end(); ++it)
				selZones.insert(it->first);
			for (std::set<TPatchVertId>::const_iterator it = g_PatchTanSel.begin();
			     it != g_PatchTanSel.end(); ++it)
				selZones.insert(it->first);
		}
		bridge.PatchSelZones = (uint)selZones.size();
		bridge.PatchSelZoneName[0] = 0;
		if (selZones.size() == 1)
		{
			// The FILE basename, not the node name: ligo nodes are often generically
			// named ("metazone") while the brick identity the artist thinks in is the
			// file. Node name is the fallback for synthetic sessions.
			std::string label = zpZoneFileBasename(*selZones.begin());
			if (label.empty())
			{
				const SPaintZone *pz = zpFindPaintZone(*selZones.begin());
				if (pz)
					label = pz->Name;
			}
			strncpy(bridge.PatchSelZoneName, label.c_str(),
			        sizeof(bridge.PatchSelZoneName) - 1);
			bridge.PatchSelZoneName[sizeof(bridge.PatchSelZoneName) - 1] = 0;
		}
	}
	bridge.WeldTargetArmed = g_WeldTargetArmed;
	bridge.WeldThreshold = zpLastWeldThreshold();
	bridge.FilterVerts = g_PatchFilterVerts;
	bridge.FilterVecs = g_PatchFilterVecs;
	bridge.LockHandles = g_PatchLockHandles;
	bridge.ShowArrows = g_ShowArrows;
	// Surface Properties snapshot: tri-state smoothing bits over the face selection and
	// the steering patch's tile orders.
	bridge.SmGroupAll = 0;
	bridge.SmGroupAny = 0;
	bridge.TessU = 0;
	bridge.TessV = 0;
	if (!g_PatchFaceSel.empty())
	{
		bool first = true;
		uint32 allBits = 0, anyBits = 0;
		for (std::set<TPatchFaceId>::const_iterator it = g_PatchFaceSel.begin();
		     it != g_PatchFaceSel.end(); ++it)
		{
			uint32 mask = 0;
			if (!zpPatchSmGroupsQuery(it->first, it->second, mask))
				continue;
			if (first) { allBits = mask; first = false; }
			else allBits &= mask;
			anyBits |= mask;
		}
		bridge.SmGroupAll = first ? 0 : allBits;
		bridge.SmGroupAny = anyBits;
		int u = 0, v = 0;
		if (zpPatchTessQuery(g_PatchFaceSel.begin()->first, g_PatchFaceSel.begin()->second, u, v))
		{
			bridge.TessU = u;
			bridge.TessV = v;
		}
	}
	// Scene-menu compass availability (patch level with a face selection only).
	bridge.MoveDirMask = 0;
	if (pl.SubObj == CPaintMouseListener::SubPatch && !g_PatchFaceSel.empty())
		for (int d = 0; d < 8; ++d)
		{
			uint dst = 0;
			if (zpMoveDirTarget(d, dst))
				bridge.MoveDirMask |= (1u << d);
		}
	bridge.PivotMode = g_PivotMode;
	bridge.PivotLabel[0] = 0;
	strncpy(bridge.PivotLabel, zpPivotModeName(g_PivotMode), sizeof(bridge.PivotLabel) - 1);
	bridge.CurTileSet = pl.CurTileSet;
	bridge.TileSetCount = g_PaintCtx.Core->tileSetCount();
	std::string name = g_PaintCtx.Core->tileSetName(pl.CurTileSet);
	// smallbanks often store empty set names: derive from first 128 diffuse stem
	// (y-plages-128-a-01.png → plages) so the panel matches the Tiles palette .
	if ((name.empty() || name == "<none>") && g_PaintCtx.Bank
	    && pl.CurTileSet >= 0 && pl.CurTileSet < g_PaintCtx.Bank->getTileSetCount())
	{
		const NL3D::CTileSet *ts = g_PaintCtx.Bank->getTileSet(pl.CurTileSet);
		if (ts)
		{
			for (sint t = 0; t < ts->getNumTile128() && (name.empty() || name == "<none>"); ++t)
			{
				const NL3D::CTile *pt = g_PaintCtx.Bank->getTile(ts->getTile128(t));
				if (!pt) continue;
				std::string fn = pt->getRelativeFileName(NL3D::CTile::diffuse);
				if (fn.empty()) continue;
				std::string base = NLMISC::CFile::getFilenameWithoutExtension(fn);
				if (base.size() > 2 && base[1] == '-')
					base = base.substr(2);
				std::string::size_type p = base.find("-128");
				if (p == std::string::npos) p = base.find("-256");
				if (p != std::string::npos) base = base.substr(0, p);
				if (!base.empty()) name = base;
			}
		}
	}
	strncpy(bridge.TileSetName, name.c_str(), sizeof(bridge.TileSetName) - 1);
	bridge.TileSetName[sizeof(bridge.TileSetName) - 1] = 0;
	bridge.Mode256 = pl.Mode256;
	bridge.BrushSize = g_PaintCtx.Core->brushSize();
	bridge.TileGroup = g_PaintCtx.Core->tileGroup();
	bridge.LockBorders = g_PaintCtx.Core->lockBordersOn();
	bridge.UndoDepth = g_PaintCtx.Core->undoDepth();
	// Interactive modal flow: always enabled. Legacy / --save: only with a save path.
	bridge.CanSave = g_PaintCtx.InteractiveSave || !g_PaintCtx.SavePath.empty();
	bridge.InteractiveSave = g_PaintCtx.InteractiveSave;
	bridge.BoardSession = g_BoardSession;
	bridge.ThumbnailsDisabled = g_NoThumbnailWrites;
	bridge.InstanceCount = g_InstanceCount;
	{
		std::string base = NLMISC::CFile::getFilenameWithoutExtension(g_PaintCtx.InputPath);
		strncpy(bridge.EditableBasename, base.c_str(), sizeof(bridge.EditableBasename) - 1);
		bridge.EditableBasename[sizeof(bridge.EditableBasename) - 1] = 0;
		std::string dir = NLMISC::CFile::getPath(g_PaintCtx.InputPath);
		strncpy(bridge.InputDir, dir.c_str(), sizeof(bridge.InputDir) - 1);
		bridge.InputDir[sizeof(bridge.InputDir) - 1] = 0;
	}
	// Season 
	{
		std::string lab = ZPCTX::seasonPreferenceLabel();
		strncpy(bridge.SeasonLabel, lab.c_str(), sizeof(bridge.SeasonLabel) - 1);
		bridge.SeasonLabel[sizeof(bridge.SeasonLabel) - 1] = 0;
		bridge.SeasonCount = g_PaintCtx.AvailableSeasons
			? (uint)g_PaintCtx.AvailableSeasons->size() : 0;
	}
	// Multi-file dirty 
	bridge.EditableFileCount = g_EditableFiles.empty() ? 1u : (uint)g_EditableFiles.size();
	bridge.DirtyFileCount = countDirtyEditableFiles();
	// Color / displace 
	bridge.ColorRadius = pl.BrushRadius;
	bridge.ColorHardness = pl.BrushHardness;
	bridge.ColorOpacity = pl.BrushOpacity;
	bridge.ColorR = pl.BrushColor.R;
	bridge.ColorG = pl.BrushColor.G;
	bridge.ColorB = pl.BrushColor.B;
	bridge.DisplaceIndex = pl.DisplaceIndex;
	bridge.BrushMaskMode = g_PaintCtx.Core->brushMaskMode();
	{
		// Panel button: mask basename when a mask is loaded, else "none"
		// (HUD still shows "off" when mask mode is disabled - different concern.)
		std::string lab = "none";
		if (!g_PaintCtx.Core->brushMaskName().empty())
			lab = NLMISC::CFile::getFilename(g_PaintCtx.Core->brushMaskName());
		strncpy(bridge.BrushMaskLabel, lab.c_str(), sizeof(bridge.BrushMaskLabel) - 1);
		bridge.BrushMaskLabel[sizeof(bridge.BrushMaskLabel) - 1] = 0;
	}
	// Prop panel
	bridge.PropHaveSelection = g_HavePropSelection;
	bridge.PropZoneId = g_SelectedZoneId;
	bridge.PropZoneName[0] = 0;
	bridge.PropFileBasename[0] = 0;
	bridge.PropFootprint[0] = 0;
	bridge.PropStatus[0] = 0;
	bridge.PropFootprintFilled = true;
	bridge.PropEditable = false;
	bridge.PropDirty = false;
	bridge.PropRotate = 0;
	bridge.PropSymmetry = false;
	bridge.PropPassable = false;
	bridge.PropUseBBox = false;
	if (g_HavePropSelection)
	{
		const SPaintZone *pz = zpFindPaintZone(g_SelectedZoneId);
		if (pz)
		{
			strncpy(bridge.PropZoneName, pz->Name.c_str(), sizeof(bridge.PropZoneName) - 1);
			bridge.PropZoneName[sizeof(bridge.PropZoneName) - 1] = 0;
			const std::string basen = zpZoneFileBasename(g_SelectedZoneId);
			strncpy(bridge.PropFileBasename, basen.c_str(), sizeof(bridge.PropFileBasename) - 1);
			bridge.PropFileBasename[sizeof(bridge.PropFileBasename) - 1] = 0;
			bridge.PropEditable = zpZoneIsPropSelectable(g_SelectedZoneId);
			bridge.PropDirty = g_PaintCtx.Core->isZoneDirty(g_SelectedZoneId);
			SZoneProps props;
			zpReadZoneProps(pz->Node, props);
			bridge.PropRotate = props.Rotate;
			bridge.PropSymmetry = props.Symmetry;
			bridge.PropPassable = props.Passable;
			bridge.PropUseBBox = props.UseBoundingBox;
			// Footprint: primary uses g_Footprint*; others re-derive for display (memoized -
			// this runs once per frame while a selection is live, and the derive is a full
			// CZoneTemplate build that also logs on failure).
			int cw = g_FootprintCellsW, ch = g_FootprintCellsH;
			bool fromT = g_FootprintFromTemplate;
			bool filled = !maskHasHole(g_FootprintMask);
			if (pz->ZoneId != 0 || g_FootprintMask.empty())
				zpPropFootprintCached(*pz, props.UseBoundingBox, props.Symmetry,
				                      cw, ch, fromT, filled);
			snprintf(bridge.PropFootprint, sizeof(bridge.PropFootprint),
			         "%dx%d (source=%s)%s", cw, ch, fromT ? "template" : "aabb",
			         filled ? "" : " hole");
			bridge.PropFootprintFilled = filled;
		}
	}
	if (!g_PropStatusMsg.empty())
	{
		strncpy(bridge.PropStatus, g_PropStatusMsg.c_str(), sizeof(bridge.PropStatus) - 1);
		bridge.PropStatus[sizeof(bridge.PropStatus) - 1] = 0;
	}
}

/** Script-host hook: refresh the live bridge's frame-synced snapshot on demand - the
 *  snapshot goes stale for the whole run of a script, and painterscript getters
 *  (getMode/getTileSet) must see their own setters' effects. */
void zpScriptRefreshBridge()
{
	ZPSCRIPT::SScriptHost *h = ZPSCRIPT::getHost();
	if (h && h->bridge)
		zpFillBridgeState(*h->bridge);
}


