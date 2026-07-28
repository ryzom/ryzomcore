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
		else if (tok[0] == "undo") { ok = core.opUndo(); if (!ok) err = "undo stack empty"; }
		else if (tok[0] == "redo") { ok = core.opRedo(); if (!ok) err = "redo stack empty"; }
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
 * curs_default select / no-op curs_pick eyedropper pick
 * curs_rotate rotate, and orbit drag curs_scale scale
 * curs_pan panning curs_can_pan pan available
 * curs_resize_* container resizers (wired in NLGUI already, not a tool concern)
 *
 * NEED BITMAPS - paint modes (this switch):
 * curs_zp_tile Tile mode brush
 * curs_zp_color Colour mode brush
 * curs_zp_displace Displace mode brush
 * (Prop mode uses curs_default: it is a selection mode.)
 *
 * NEED BITMAPS - patch-edit modes (same switch, once the sub-object levels exist):
 * curs_zp_move 4-way move
 * curs_zp_region rubber-band region select
 * curs_zp_bind bind vertex to edge
 * curs_zp_weld weld
 * curs_zp_attach attach
 * curs_zp_subdiv subdivide / add patch
 * (Rotate and Scale reuse curs_rotate / curs_scale. Turn-edge can reuse curs_rotate.)
 *
 * NEED BITMAPS - navigation (see nel/src/3d/nav_mouse_listener.cpp):
 * curs_zp_zoom dolly / zoom drag
 * (Pan and orbit reuse curs_pan / curs_rotate.)
 */
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

void zpSelectMode(int mode)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (mode < 0) mode = 0;
	if (mode > CPaintMouseListener::ModePatch) mode = CPaintMouseListener::ModePatch;
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
// Entering patch edit lands on Object level: the artist picks a sub-object
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
 * - PatchMesh edge shared by two patches of THIS mesh → interior.
 * - CPatchInfo::BindEdges with NPatchs!=0 and ZoneId==this zone → same-zone neighbor
 * (shared edge, 1-1/1-2/1-4 binds via dividEdge/offsetEdge) → interior.
 * - BindEdges NPatchs==0 or ZoneId!=this (cross-zone weld) → outer perimeter.
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
	// Cross-zone welds (ZoneId != this) stay outer for THIS zone's silhouette.
	// NPatchs==0 is a true open / outer edge.
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
// Lift used by every patch overlay: the control points sit ON the landscape, and un-lifted
// geometry disappears into the tessellation it describes. Same value the zone outline uses.
static const float kPatchLift = 0.4f;
static const NLMISC::CVector kNoOffset(0.f, 0.f, 0.f);

// Gizmo drag state; see the drag section below.
static bool s_Dragging = false;
static int s_DragHandle = ZPGIZ_NONE;
static NLMISC::CVector s_DragPlaneN, s_DragPlaneP, s_DragStartHit, s_DragDelta;
static NLMISC::CVector s_DragAxis;
// The transform the current drag represents. Move keeps using s_DragDelta - a translation is
// the same everywhere and needs no anchor - while rotate and scale fill this in, because their
// preview offset differs per element and cannot be a single vector.
static SPatchXform s_DragXform;
static NLMISC::CVector s_DragStartVec; // rotate: the ring vector the drag started on

/**
 * The offset this drag gives a point at `p`. Null when nothing is being dragged.
 *
 * The reason the preview and the commit cannot disagree: both run the SAME transform, this one
 * per displayed point and zpApplyPatchXform per written element.
 */
static NLMISC::CVector zpDragOffsetAt(const NLMISC::CVector &p)
{
	if (!s_Dragging)
		return kNoOffset;
	if (s_DragXform.Kind == ZPXF_Move)
		return s_DragDelta;
	return zpTransformPoint(s_DragXform, p) - p;
}

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
 * The object this node shows. Several nodes may return the SAME pointer: that is what makes
 * them nodes of one object rather than independent zones, and it is the identity every
 * per-object rule keys on - selection aliasing, geometry fan-out, the shared paint carrier.
 */
static const void *zpZoneNode(uint zoneId)
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
static NLMISC::CVector zpXformDelta(const NLMISC::CVector &d, const MAXMATH::Matrix3M &m)
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
 * context that shows them, and the vertex stays selected while you drag its handle.
 */
static bool zpHandleMode() { return !g_PatchTanSel.empty(); }

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
static NLMISC::CVector zpVertOffset(const SPaintZone &pz, uint16 vi)
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

/**
 * Preview offset for a HANDLE.
 *
 * A handle whose corner is moving rides that corner - the same rule the write applies, so the
 * preview and the commit cannot disagree about which of the two moved it. Only a handle whose
 * corner is standing still takes its own drag.
 */
static NLMISC::CVector zpTanOffset(const SPaintZone &pz, uint16 vecIdx)
{
	const uint16 owner = zpTangentOwner(pz, vecIdx);
	if (!zpHandleMode() && owner != (uint16)0xffff
	    && g_PatchVertSel.count(TPatchVertId(pz.ZoneId, owner)))
		return zpVertOffset(pz, owner);
	if (!s_Dragging || !g_PaintCtx.Zones)
		return kNoOffset;
	if (g_PatchTanSel.count(TPatchVertId(pz.ZoneId, vecIdx)))
	{
		float wp[3];
		if (zpPatchTangentWorld(pz.ZoneId, vecIdx, wp))
			return zpDragOffsetAt(NLMISC::CVector(wp[0], wp[1], wp[2]));
		return s_DragDelta;
	}
	// Selected through another node of the same object: the drag is in that node's displayed
	// space, so it comes back to object space through it and out again through this one.
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
	{
		const SPaintZone &other = zones[z];
		if (other.Node != pz.Node || other.ZoneId == pz.ZoneId)
			continue;
		if (!g_PatchTanSel.count(TPatchVertId(other.ZoneId, vecIdx)))
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
void zpDrawPatchLattice(NL3D::IDriver *driver, NL3D::CCamera *camera,
                        const SPaintZone &pz, int subObj)
{
	if (!driver || !camera || pz.Patches.empty())
		return;

	static const NLMISC::CRGBA kCageColor(90, 190, 255, 255);
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

	// TWO passes over the cage: everything plain, then everything selected.
	//
	// Not a nicety. Each patch draws all four of its own edges, so an edge between two patches
	// is drawn twice, and with a single pass the neighbour drawn later paints over the
	// highlight - a selected patch came out with two red edges and two blue ones depending on
	// where it sat in the patch order.
	for (uint pass = 0; pass < 2; ++pass)
	{
		const bool selPass = pass != 0;
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
				if ((faceSel || edgeSel) != selPass)
					continue;
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
				const NLMISC::CRGBA &col = selPass ? kVertSelColor : kCageColor;
				NLMISC::CVector chain[4];
				bool ok[4];
				ok[0] = zpProjectLifted(viewMat, fr, bp.Vertices[e] + offA, kPatchLift, chain[0]);
				ok[1] = zpProjectLifted(viewMat, fr, bp.Tangents[e * 2] + offTA, kPatchLift, chain[1]);
				ok[2] = zpProjectLifted(viewMat, fr, bp.Tangents[e * 2 + 1] + offTB, kPatchLift, chain[2]);
				ok[3] = zpProjectLifted(viewMat, fr, bp.Vertices[(e + 1) & 3] + offB, kPatchLift, chain[3]);
				for (uint k = 0; k + 1 < 4; ++k)
					if (ok[k] && ok[k + 1])
						NL3D::CDRU::drawLine(chain[k].x, chain[k].y,
						                     chain[k + 1].x, chain[k + 1].y, *driver, col);
			}
		}
	}

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
	// Dev hook, alongside ZONE_PAINTER_GIZMO_HOVER: "handle:x,y,z" forces a live drag so the
	// preview can be seen and gated from a --screenshot run, which has no pointer to drag.
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
	const std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	for (uint z = 0; z < zones.size(); ++z)
		if (zones[z].Editable)
			zpDrawPatchLattice(driver, camera, zones[z], subObj);
}

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
static const int kGizmoPlaneAxes[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };

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
static float s_GizmoWorldLen = 1.f;
static bool s_GizmoFitDirty = true;
static uint32 s_GizmoFitSerial = 0xffffffffu;
static uint32 s_GizmoFitW = 0, s_GizmoFitH = 0;
static int s_GizmoHover = -1;

int zpPatchGizmoHover() { return s_GizmoHover; }
void zpPatchGizmoInvalidate() { s_GizmoFitDirty = true; }

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

void zpDrawPatchGizmo(NL3D::IDriver *driver, NL3D::CCamera *camera,
                      float mouseX, float mouseY, bool navigating, uint32 viewSerial)
{
	s_GizmoHover = ZPGIZ_NONE;
	if (!driver || !camera)
		return;
	NLMISC::CVector o;
	if (!zpTransformPivot(o))
		return;
	o += s_DragDelta; // rides the drag, so the handle stays under the pointer
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
 *  one of the three world axes. */
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
// Bound vertices are derived from a target edge; they are recomputed on load, so
// a written position could not survive the round trip.
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

	if (level == CPaintMouseListener::SubEdge)
	{
		uint zone = 0;
		uint16 a = 0, b = 0;
		if (!zpPickPatchEdge(camera, driver, mx, my, zone, a, b))
		{
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
			zpPatchVertClear();
			return;
		}
		zpPatchFaceSelect(zone, patch, op);
		return;
	}

	// Handles first: they are drawn on top of the cage and sit near their corner, so a
	// corner-first order would make them unreachable wherever the two overlap.
	uint zone = 0;
	uint16 vec = 0;
	if (zpPickPatchTangent(camera, driver, mx, my, zone, vec))
	{
		zpPatchTangentSelect(zone, vec, op);
		return;
	}
	uint16 vert = 0;
	if (!zpPickPatchVertex(camera, driver, mx, my, zone, vert))
	{
		zpPatchVertClear();
		return;
	}
	zpPatchVertSelect(zone, vert, op);
}

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
 * rotate → NEL3D_APPDATA_ZONE_ROTATE decimal "0".."3" (always present after write)
 * symmetry → NEL3D_APPDATA_ZONE_SYMMETRY "1"/"0" (BST_CHECKED/UNCHECKED)
 * passable → presence: set "1" when true, DELETE entry when false (ligoscape rollout)
 * usebbox → "1" when true; DELETE when false (exporter getScriptAppDataInt default 0;
 * absent and "0" both read false - delete is the least-surprising clean write)
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
		printf(" zone '%s' frozen=%d eligible=%d rotate=%d%s symmetry=%d%s passable=%d%s usebbox=%d%s\n",
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

void zpUndo()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	g_PaintCtx.Core->opUndo();
	ZPSCRIPT::record("painter.undo()");
}

void zpRedo()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	g_PaintCtx.Core->opRedo();
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
 * snapshot goes stale for the whole run of a script, and painterscript getters
 * (getMode/getTileSet) must see their own setters' effects. */
void zpScriptRefreshBridge()
{
	ZPSCRIPT::SScriptHost *h = ZPSCRIPT::getHost();
	if (h && h->bridge)
		zpFillBridgeState(*h->bridge);
}


