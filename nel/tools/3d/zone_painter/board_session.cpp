/**
 * \file board_session.cpp
 * \brief Working-set rebuild + eco scratch board ops + neighbor context loading.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * `rebuildWorkingSet` (open/close/toggle mid-viewer: writeBack -> stash -> rebuild zones
 * -> weld -> core.init -> restore -> reattach landscape); the `scratch*` op family that
 * drives the eco board (place / rotate / mirror / drag-drop / open editable / context
 * bricks); neighbor .max discovery via the hint chain (appdata -> embedded -> grid);
 * per-file save-in-place helper. Session zone open/close/save/toggle sit in session_ops.cpp.
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

#include "zp_state.h"

// TU-local forward decls for the *Impl split. Each public op wraps SBoardOpScope + record
// around a *Impl body so nested ops record at depth 1 only.
static bool scratchPlaceContextImpl(int cx, int cy, const std::string &basename, std::string &err);
static bool scratchRemoveContextImpl(int cx, int cy, std::string &err);
static bool scratchRotateContextImpl(int cx, int cy, int delta, std::string &err);
static bool scratchMirrorContextImpl(int cx, int cy, std::string &err);
static bool scratchDragDropImpl(int fx, int fy, int tx, int ty, bool copy, std::string &err);
static bool scratchOpenEditableImpl(int cx, int cy, const std::string &basenameIn, std::string &err);
static bool scratchContextToEditableImpl(int cx, int cy, std::string &err);
static bool scratchPlaceInstanceOfImpl(int cx, int cy, const std::string &basenameIn, std::string &err);
static bool scratchRotateImpl(int cx, int cy, int delta, std::string &err);
static bool scratchMirrorImpl(int cx, int cy, std::string &err);
static bool scratchRemoveImpl(int cx, int cy, std::string &err);
static bool sessionOpenZoneImpl(const std::string &basename, std::string &err);
static bool sessionCloseZoneImpl(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err);
static bool sessionSaveZoneImpl(const std::string &basename, std::string &err);
static bool sessionToggleEditableImpl(const std::string &basename, bool saveFirst,
                                      bool forceDiscard, std::string &err);

/**
 * Clear and free all context/neighbor scenes.
 * Callers must not hold dangling zone Node pointers from those scenes.
 */
void clearContextFiles()
{
	for (size_t i = 0; i < g_ContextFiles.size(); ++i)
		freeLoadedMax(g_ContextFiles[i].Lm);
	g_ContextFiles.clear();
	g_NeighborScenes.clear();
}

void loadNeighborContextFiles(std::vector<SPaintZone> &zones, float cellSize,
                                     const std::vector<std::string> &skipBasenames)
{
	clearContextFiles();
	g_SessionHintCells.clear();
	if (!g_LoadNeighbors) return;
	if (g_StartupWorld.MaxDir.empty()) return;
	if (!g_BoardSession && g_StartupWorld.Kind != ZPWS::Continent) return;

	// Home footprint origin (for ecosystem translation): globals when derived
	float homeOriginX = 0.f, homeOriginY = 0.f;
	scratchBoardAnchor(homeOriginX, homeOriginY); // session-frozen board frame
	float stepX = 0.f, stepY = 0.f;
	int fw = g_FootprintCellsW > 0 ? g_FootprintCellsW : 1;
	int fh = g_FootprintCellsH > 0 ? g_FootprintCellsH : 1;
	if ((g_FootprintMask.empty() || g_FootprintCellsW < 1) && !zones.empty())
	{
		size_t primaryEnd = 0;
		for (size_t i = 0; i < zones.size(); ++i)
			if (zones[i].ZoneId < 1000) primaryEnd = i + 1;
		if (primaryEnd > 0)
			computeFootprintRect(zones, 0, primaryEnd, cellSize,
			                     homeOriginX, homeOriginY, stepX, stepY, fw, fh);
	}
	else
	{
		stepX = (float)fw * cellSize;
		stepY = (float)fh * cellSize;
	}
	if (fw < 1) fw = 1;
	if (fh < 1) fh = 1;

	std::set<std::string> skip;
	for (size_t i = 0; i < skipBasenames.size(); ++i)
		skip.insert(NLMISC::toLowerAscii(skipBasenames[i]));
	// Also skip basenames already queued
	std::set<std::string> loaded;

	// Collect from each editable file's scene
	struct SPending
	{
		ZPWS::SZoneEntry Zone;
		int Dx, Dy;
		bool Translate;
		// Hint-carried placement transform (context rot/mirror survives reopen)
		uint Rot;
		bool Mirror;
		// Floor-snapped origin of the named embedded copy in the primary (when present).
		// Placement prefers this over home+dx so freestanding reconstructs the island layout
		// that floor-only keys used (weld quality) while recorded dx stay raw-distinct.
		bool HaveEmbFloor;
		float EmbFloorX, EmbFloorY;
		SPending() : Dx(0), Dy(0), Translate(false), Rot(0), Mirror(false),
		             HaveEmbFloor(false), EmbFloorX(0.f), EmbFloorY(0.f) {}
	};
	std::vector<SPending> pending;

	// name → floor origin from primary editable scenes (embedded copies)
	struct SEmbFloor { float X, Y; };
	std::map<std::string, SEmbFloor> embFloorByName;
	SNodeTMCache embTmCache;
	for (size_t ei = 0; ei < g_EditableFiles.size(); ++ei)
	{
		if (!g_EditableFiles[ei].Editable) continue;
		PMAXLOAD::SLoadedMax *lm = g_EditableFiles[ei].Lm ? g_EditableFiles[ei].Lm : g_PrimaryLm;
		if (!lm || !lm->Scene) continue;
		std::vector<SZoneNode> nodes;
		collectZoneNodes(*lm->Scene, nodes);
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			std::string name = ucstring(nodes[i].Node->userName()).toUtf8();
			if (name.empty()) continue;
			float fx = 0.f, fy = 0.f;
			if (!zoneNodeAuthoredFootprintOrigin(nodes[i].Node, embTmCache, cellSize, fx, fy))
				continue;
			// Scene TMs are authored-space; non-primary eco files are translated to their
			// board cell, so add the file's applied translation to get a WORLD floor origin.
			SEmbFloor ef;
			ef.X = fx + g_EditableFiles[ei].PlacedDX;
			ef.Y = fy + g_EditableFiles[ei].PlacedDY;
			const std::string low = NLMISC::toLowerAscii(name);
			embFloorByName[low] = ef;
			size_t dash = low.find_last_of('-');
			if (dash != std::string::npos && dash + 1 < low.size())
				embFloorByName[low.substr(dash + 1)] = ef;
		}
	}

	for (size_t ei = 0; ei < g_EditableFiles.size(); ++ei)
	{
		if (!g_EditableFiles[ei].Editable) continue;
		PMAXLOAD::SLoadedMax *lm = g_EditableFiles[ei].Lm ? g_EditableFiles[ei].Lm : g_PrimaryLm;
		if (!lm || !lm->Scene) continue;

		std::vector<SNeighborHint> hints;
		std::vector<ZPWS::SZoneEntry> resolved;
		std::string source;
		collectNeighborHints(*lm->Scene, g_EditableFiles[ei].Basename, g_StartupWorld,
		                     cellSize, hints, resolved, source);
		// Pair hints with resolved (resolved may be shorter if some failed)
		// Re-resolve to keep (dx,dy) pairing
		for (size_t hi = 0; hi < hints.size(); ++hi)
		{
			ZPWS::SZoneEntry ze;
			if (!resolveHintToZone(g_StartupWorld, hints[hi].Basename, ze))
				continue;
			const std::string key = NLMISC::toLowerAscii(ze.Basename);
			// Remember every resolved hint at its BOARD cell (rebased from the
			// carrying file's placement) for the board's per-cell offers, deduped.
			{
				const int bx = g_EditableFiles[ei].CellX + hints[hi].Dx;
				const int by = g_EditableFiles[ei].CellY + hints[hi].Dy;
				bool dup = false;
				for (size_t d = 0; d < g_SessionHintCells.size() && !dup; ++d)
					dup = g_SessionHintCells[d].CellX == bx && g_SessionHintCells[d].CellY == by
					      && NLMISC::toLowerAscii(g_SessionHintCells[d].Basename) == key;
				if (!dup)
					g_SessionHintCells.push_back(SSessionHintCell(bx, by, ze.Basename));
			}
			if (skip.count(key) || loaded.count(key))
				continue;
			loaded.insert(key);
			SPending p;
			p.Zone = ze;
			// Hints are stored relative to the CARRYING file's board cell (per-file
			// stamping); placement and SContextFile::CellX live in board space, so rebase
			// exactly like the session-hint-cell offers above. Zero-delta for continents
			// (CellX/CellY stay 0 there); the eco primary rebases by its own cell like
			// every other file (movable home).
			p.Dx = g_EditableFiles[ei].CellX + hints[hi].Dx;
			p.Dy = g_EditableFiles[ei].CellY + hints[hi].Dy;
			p.Rot = hints[hi].Rot;
			p.Mirror = hints[hi].Mirror;
			// Continent files sit in absolute world space (no translate; identical to
			// applying the unified rule when hints == authored origin deltas). Ecosystem
			// always places via authored-origin-relative translation.
			p.Translate = (g_StartupWorld.Kind == ZPWS::Ecosystem);
			const std::string hintLow = NLMISC::toLowerAscii(hints[hi].Basename);
			std::map<std::string, SEmbFloor>::const_iterator it = embFloorByName.find(hintLow);
			if (it == embFloorByName.end())
			{
				size_t dash = hintLow.find_last_of('-');
				if (dash != std::string::npos)
					it = embFloorByName.find(hintLow.substr(dash + 1));
			}
			if (it == embFloorByName.end())
				it = embFloorByName.find(key);
			if (it != embFloorByName.end())
			{
				p.HaveEmbFloor = true;
				p.EmbFloorX = it->second.X;
				p.EmbFloorY = it->second.Y;
			}
			(void)source;
			(void)fw;
			(void)fh;
			pending.push_back(p);
		}
	}

	// place-context entries already in g_ContextFiles? (filled before call): none yet
	// CLI --place-context handled separately via g_PlaceContextSpecs

	// Primary authored extent for the continent placement sanity check below.
	NLMISC::CAABBox primaryBox;
	bool havePrimaryBox = false;
	{
		size_t primaryEnd = 0;
		for (size_t i = 0; i < zones.size(); ++i)
			if (zones[i].ZoneId < 1000) primaryEnd = i + 1;
		havePrimaryBox = authoredAABB(zones, 0, primaryEnd, primaryBox);
	}

	printf("neighbors: loading %u context file(s)\n", (uint)pending.size());
	for (size_t ni = 0; ni < pending.size(); ++ni)
	{
		const SPending &p = pending[ni];
		PMAXLOAD::SLoadedMax *nlm = new PMAXLOAD::SLoadedMax();
		if (!PMAXLOAD::loadMaxFile(p.Zone.MaxPath, *nlm))
		{
			fprintf(stderr, "WARNING: neighbor load failed: %s\n", p.Zone.MaxPath.c_str());
			delete nlm;
			continue;
		}
		uint base = nextZoneIdBase(zones);
		const uint minBase = (uint)((g_EditableFiles.size() + g_ContextFiles.size() + 1) * 1000);
		if (base < minBase) base = minBase;
		const size_t before = zones.size();
		if (!buildPaintZones(*nlm->Scene, zones, base, /*forceFrozen=*/true, p.Zone.Basename))
		{
			fprintf(stderr, "WARNING: neighbor has no paint zones: %s\n", p.Zone.MaxPath.c_str());
			freeLoadedMax(nlm);
			continue;
		}
		if (!p.Translate && before < zones.size() && havePrimaryBox)
		{
			// Continent context placement sanity (source units drift): snowballs 5_AC.max is
			// authored in inches (values 39.37x meters) with NO distinguishing unit metadata
			// anywhere in the file (its Config/SceneImpl chunks match the meters-authored
			// siblings), yet the shipped 2002 reference zone sits at authored/39.37. Absolute
			// placement of such a file lands kilometers off-grid (and blanked the session
			// camera before the editable-only framing fix). Detect by distance to the primary
			// and try the known unit ratios both ways; unmatched files are dropped as context
			// (the name hint chain keeps them listed).
			NLMISC::CAABBox nb;
			if (authoredAABB(zones, before, zones.size(), nb))
			{
				const float span = std::max(
					std::max(primaryBox.getHalfSize().x, primaryBox.getHalfSize().y) * 2.f, cellSize);
				const float tol = 3.f * span;
				NLMISC::CVector d = nb.getCenter() - primaryBox.getCenter();
				d.z = 0.f;
				if (d.norm() > tol)
				{
					// meters<->inches/cm/feet/mm, either direction
					static const float kUnitRatios[] = {
						0.0254f, 0.01f, 0.3048f, 0.001f,
						39.3700787f, 100.f, 3.2808399f, 1000.f };
					float matched = 0.f;
					for (uint ri = 0; ri < sizeof(kUnitRatios) / sizeof(kUnitRatios[0]); ++ri)
					{
						NLMISC::CVector ds = nb.getCenter() * kUnitRatios[ri] - primaryBox.getCenter();
						ds.z = 0.f;
						if (ds.norm() <= tol) { matched = kUnitRatios[ri]; break; }
					}
					if (matched != 0.f)
					{
						scaleZonesXYZ(zones, before, zones.size(), matched);
						fprintf(stderr, "WARNING: context '%s' authored off-grid (center %.0f,%.0f): "
						        "unit-normalized x%g\n",
						        p.Zone.Basename.c_str(), nb.getCenter().x, nb.getCenter().y, matched);
					}
					else
					{
						fprintf(stderr, "WARNING: context '%s' authored far off-grid "
						        "(center %.0f,%.0f vs primary %.0f,%.0f, no unit ratio fits): "
						        "SKIPPED as context\n",
						        p.Zone.Basename.c_str(), nb.getCenter().x, nb.getCenter().y,
						        primaryBox.getCenter().x, primaryBox.getCenter().y);
						zones.erase(zones.begin() + before, zones.end());
						freeLoadedMax(nlm);
						continue;
					}
				}
			}
		}
		if (p.Translate && before < zones.size())
		{
			// Placement: translation = wantFloor - freestandingFloor
			// Prefer the named embedded copy's floor origin when the primary still has it
			// (exact island layout that floor-only keys used for weld quality). Else
			// intended = homeFloor + raw-distinct cellOffset * cellSize (appdata reopen).
			// Hints carrying rot/mirror transform about the block pivot instead
			// (embedded-floor preference is a rot0 concept and does not apply).
			float cOx = 0.f, cOy = 0.f, cSx = 0.f, cSy = 0.f;
			int cw = 1, ch = 1;
			computeFootprintRect(zones, before, zones.size(), cellSize, cOx, cOy, cSx, cSy, cw, ch);
			if ((p.Rot & 3) != 0 || p.Mirror)
				placeContextRange(zones, before, zones.size(), cOx, cOy, cw, ch,
				                  homeOriginX, homeOriginY, cellSize, p.Dx, p.Dy, p.Rot, p.Mirror);
			else
			{
				float wantX = homeOriginX + (float)p.Dx * cellSize;
				float wantY = homeOriginY + (float)p.Dy * cellSize;
				if (p.HaveEmbFloor)
				{
					wantX = p.EmbFloorX;
					wantY = p.EmbFloorY;
				}
				translateZonesXY(zones, before, zones.size(), wantX - cOx, wantY - cOy);
			}
		}
		SContextFile cf;
		cf.Path = p.Zone.MaxPath;
		cf.Basename = p.Zone.Basename;
		cf.CellX = p.Dx;
		cf.CellY = p.Dy;
		cf.TranslateGeom = p.Translate;
		cf.Rot = p.Rot & 3;
		cf.Mirror = p.Mirror;
		if (p.Translate && before < zones.size())
		{
			// Board occupancy mask (hint-loaded contexts need collision occupancy).
			// Pick the FIRST of the range: under board authority only the eligible brick
			// zone(s) survive load; context rows are all force-frozen so a "first non-frozen"
			// walk lands on the last (embedded-neighbor) zone. First-of-range matches the
			// export brick / derivePrimaryFootprint product model.
			size_t cPick = before;
			float mOx = 0.f, mOy = 0.f;
			bool fromT = false;
			std::string derr;
			deriveZoneFootprintMask(zones[cPick], cellSize, g_SessionSnap > 0.f ? g_SessionSnap : 1.f,
			                        cf.Mask, cf.CellsW, cf.CellsH, mOx, mOy, fromT, derr);
		}
		cf.Lm = nlm;
		g_ContextFiles.push_back(cf);
		g_NeighborScenes.push_back(nlm);
		printf(" context '%s' @ cell (%d,%d)%s%s zoneIdBase=%u FROZEN%s\n",
		       cf.Basename.c_str(), cf.CellX, cf.CellY,
		       cf.Rot ? NLMISC::toString(" R%u", cf.Rot * 90).c_str() : "",
		       cf.Mirror ? " M" : "", base,
		       p.Translate ? " (translated)" : "");
	}
}


// ---------------------------------------------------------------------------------------------
// Session working-set rebuild - open/close/toggle mid-viewer.
//
// Sequence: writeBack retained paint → stash OriginalBytes → rebuild zones vector from kept
// SLoadedMax scenes (no reload of already-open files) → weld → core.init → restore
// OriginalBytes → reattach landscape (removeZone all + addZone). Undo is cleared by init().
// Closing frees the SLoadedMax ONLY after confirm flow resolves.

SEditableFileInfo *findEditableByBasename(const std::string &basename)
{
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (NLMISC::toLowerAscii(g_EditableFiles[i].Basename)
		    == NLMISC::toLowerAscii(basename))
			return &g_EditableFiles[i];
	}
	return NULL;
}

const ZPWS::SZoneEntry *findWorldZone(const std::string &basename)
{
	if (g_StartupWorld.MaxDir.empty())
		return NULL;
	static std::vector<ZPWS::SZoneEntry> s_listed;
	static std::string s_listedDir;
	// Cached list, but a miss re-lists once: the board re-lists on every open, so a
	// zone file created mid-session (e.g. "Save copy" into the world dir) must be
	// openable here too, not stuck behind a process-lifetime cache.
	for (int attempt = 0; attempt < 2; ++attempt)
	{
		if (attempt == 1 || s_listedDir != g_StartupWorld.MaxDir)
		{
			ZPWS::listZones(g_StartupWorld, s_listed);
			s_listedDir = g_StartupWorld.MaxDir;
		}
		for (size_t i = 0; i < s_listed.size(); ++i)
		{
			if (NLMISC::toLowerAscii(s_listed[i].Basename)
			    == NLMISC::toLowerAscii(basename))
				return &s_listed[i];
		}
	}
	return NULL;
}


/**
 * Rebuild the painting landscape from current g_EditableFiles (+ continent ring).
 * Retains open SLoadedMax scenes; reloads nothing already open.
 * Returns weld count; fills err on hard failure.
 */
/**
 * transform a loaded context range to board cell (dx,dy) with rot/mirror about the
 * source footprint-block pivot (instance math). Rot0/no-mirror reduces to a pure translate
 * whose result equals the historical translateZonesXY path. Zones get Rotate/Symmetry so
 * applyInstanceDisplayTiles and the paint graph see the transform.
 */
void placeContextRange(std::vector<SPaintZone> &zones, size_t rb, size_t re,
                              float cOx, float cOy, int cw, int ch,
                              float boardOriginX, float boardOriginY, float cellSize,
                              int dx, int dy, uint rot, bool mirror)
{
	const float stepX = (float)(cw > 0 ? cw : 1) * cellSize;
	const float stepY = (float)(ch > 0 ? ch : 1) * cellSize;
	const float pivotX = cOx + stepX * 0.5f;
	const float pivotY = cOy + stepY * 0.5f;
	float tdx = 0.f, tdy = 0.f;
	computePlaceTranslationFrom(cOx, cOy, stepX, stepY, pivotX, pivotY,
	                            boardOriginX, boardOriginY, cellSize,
	                            dx, dy, rot & 3, mirror, tdx, tdy);
	for (size_t i = rb; i < re && i < zones.size(); ++i)
	{
		SPaintZone &pz = zones[i];
		pz.Rotate = rot & 3;
		pz.Symmetry = mirror;
		pz.DisplayTM = pz.DisplayTM * instanceDisplayTM(pivotX, pivotY, tdx, tdy, rot & 3, mirror);
		for (size_t pp = 0; pp < pz.Patches.size(); ++pp)
		{
			NL3D::CPatchInfo &pi = pz.Patches[pp];
			for (uint v = 0; v < 4; ++v)
				transformInstanceXY(pi.Patch.Vertices[v].x, pi.Patch.Vertices[v].y,
				                    pivotX, pivotY, tdx, tdy, rot & 3, mirror);
			for (uint v = 0; v < 8; ++v)
				transformInstanceXY(pi.Patch.Tangents[v].x, pi.Patch.Tangents[v].y,
				                    pivotX, pivotY, tdx, tdy, rot & 3, mirror);
			for (uint v = 0; v < 4; ++v)
				transformInstanceXY(pi.Patch.Interiors[v].x, pi.Patch.Interiors[v].y,
				                    pivotX, pivotY, tdx, tdy, rot & 3, mirror);
		}
	}
}

/**
 * derive + translate one eco editable's zones range to its board cell
 * (authored-origin-relative, same rule as place-context); stores the footprint on the
 * file entry for board occupancy. Requires the session board anchor already captured.
 *
 * Footprint = exporter-identical mask of the file's single editable paint zone
 * (by design: one editable zone per normal .max; first non-frozen in range).
 * Not a "union all zones" pass - extra patches are context/embeds, not co-targets.
 * All-frozen (RO-demoted) files: pick the FIRST of the range - walking to LAST
 * would grab an embedded display copy and shift occupancy on toggle.
 */
void placeEcoEditableRange(std::vector<SPaintZone> &zones, SEditableFileInfo &efi,
                                  size_t rb, size_t re, float cellSize, float snap)
{
	if (rb >= re) return;
	size_t pick = rb;
	for (size_t i = rb; i < re; ++i)
	{
		if (!zones[i].Frozen)
		{
			pick = i;
			break;
		}
	}
	float ox = 0.f, oy = 0.f;
	int cw = 1, ch = 1;
	bool fromT = false;
	std::vector<bool> mask;
	std::string derr;
	deriveZoneFootprintMask(zones[pick], cellSize, snap, mask, cw, ch, ox, oy, fromT, derr);
	float ax = 0.f, ay = 0.f;
	scratchBoardAnchor(ax, ay); // session-frozen frame, not the current file[0]
	const float wantX = ax + (float)efi.CellX * cellSize;
	const float wantY = ay + (float)efi.CellY * cellSize;
	translateZonesXY(zones, rb, re, wantX - ox, wantY - oy);
	efi.PlacedDX = wantX - ox;
	efi.PlacedDY = wantY - oy;
	efi.CellsW = cw;
	efi.CellsH = ch;
	efi.Mask = mask;
}

bool rebuildWorkingSet(std::string &err, uint &outWelds, bool skipWriteBack, bool keepUndo)
{
	outWelds = 0;
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Zones || !g_PaintCtx.Land
	    || !g_SessionBank)
	{
		err = "session rebuild: viewer not live";
		return false;
	}
	// Live lock-borders survives re-init (the L-key/panel/script toggle only touches the
	// core; without this capture every board op reverted it to the CLI startup value).
	g_SessionLockBorders = g_PaintCtx.Core->lockBordersOn();
	// Preserve dirty OriginalBytes across re-init. A topological op flushes paint itself
	// and then MUTATES the carrier bytes, so its rebuild must not write back: the core's
	// pristine still holds the pre-op state and would clobber the transformed blob.
	if (!skipWriteBack)
	{
		std::string wbErr;
		if (!g_PaintCtx.Core->writeBack(wbErr))
		{
			err = "write-back before rebuild: " + wbErr;
			return false;
		}
	}
	std::map<const void *, std::vector<uint8> > originals;
	g_PaintCtx.Core->stashOriginalBytes(originals);

	// Prop selection is session-local to the zone id set - clear on working-set change.
	zpClearPropSelection();

	// Snapshot previous zone ids for landscape remove
	std::vector<uint> oldIds;
	for (size_t i = 0; i < g_PaintCtx.Zones->size(); ++i)
		oldIds.push_back((*g_PaintCtx.Zones)[i].ZoneId);

	// Neighbor/context scenes are cleared as loadNeighborContextFiles's first act
	// below, so no separate clear here (it would be a no-op).

	// Rebuild zones vector
	std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	zones.clear();
	std::vector<std::pair<size_t, size_t> > fileRanges; // zones-vector range per editable file
	for (size_t ei = 0; ei < g_EditableFiles.size(); ++ei)
	{
		SEditableFileInfo &efi = g_EditableFiles[ei];
		PMAXLOAD::SLoadedMax *sceneLm = efi.Lm ? efi.Lm : g_PrimaryLm;
		if (!sceneLm || !sceneLm->Scene)
		{
			err = "missing scene for " + efi.Basename;
			return false;
		}
		const uint base = (uint)(ei * 1000);
		const size_t before = zones.size();
		// When demoted to RO (Editable=false), forceFrozen=true for all zones in file
		const bool forceRo = !efi.Editable;
		bool ok = buildPaintZones(*sceneLm->Scene, zones, base, forceRo, efi.Basename);
		if (!ok)
		{
			fprintf(stderr, "WARNING: rebuild: no zones in %s\n", efi.Path.c_str());
		}
		efi.ZoneIds.clear();
		for (size_t zi = before; zi < zones.size(); ++zi)
			efi.ZoneIds.push_back(zones[zi].ZoneId);
		fileRanges.push_back(std::make_pair(before, zones.size()));
	}

	// Refresh primary footprint mask from rebuilt primary zones
	{
		size_t primaryOnly = 0;
		for (size_t i = 0; i < zones.size(); ++i)
			if (zones[i].ZoneId < 1000) ++primaryOnly;
		if (primaryOnly > 0)
			derivePrimaryFootprint(zones, 0, primaryOnly, g_SessionCellSize,
			                       g_SessionSnap > 0.f ? g_SessionSnap : 1.f);
	}

	// Every eco open file - the first-opened included - places identically:
	// per-file footprint derive in authored space, then session-anchor-relative
	// translation to its board cell (an unmoved cell-0,0 file is a bit-exact no-op
	// translate, since the anchor was captured from the first file's authored origin).
	// Save byte-guarantee unaffected: paint writes tile values only, geometry is a
	// display/weld copy rebuilt from the scene on every rebuild.
	if (g_StartupWorld.Kind == ZPWS::Ecosystem)
	{
		for (size_t ei = 0; ei < g_EditableFiles.size() && ei < fileRanges.size(); ++ei)
			placeEcoEditableRange(zones, g_EditableFiles[ei],
			                      fileRanges[ei].first, fileRanges[ei].second,
			                      g_SessionCellSize, g_SessionSnap > 0.f ? g_SessionSnap : 1.f);
	}

	// Ecosystem scratch instances: re-append display clones after primary rebuild.
	// prune places whose source file is no longer open (closed mid-session).
	if (!g_Places.empty() && g_StartupWorld.Kind == ZPWS::Ecosystem)
	{
		for (size_t i = g_Places.size(); i-- > 0; )
		{
			if (g_Places[i].SourceBasename.empty()) continue;
			if (!findEditableByBasename(g_Places[i].SourceBasename))
			{
				fprintf(stderr, "WARNING: instance source '%s' closed - removing its place at (%d,%d)\n",
				        g_Places[i].SourceBasename.c_str(), g_Places[i].CellX, g_Places[i].CellY);
				g_Places.erase(g_Places.begin() + (std::ptrdiff_t)i);
			}
		}
		g_InstanceCount = 1 + (uint)g_Places.size();
	}
	if (!g_Places.empty() && g_StartupWorld.Kind == ZPWS::Ecosystem && !g_EditableFiles.empty())
	{
		size_t primaryOnly = 0;
		for (size_t i = 0; i < zones.size(); ++i)
			if (zones[i].ZoneId < 1000) ++primaryOnly;
		if (primaryOnly > 0)
		{
			appendInstanceZones(zones, primaryOnly, g_Places, g_SessionCellSize);
			g_InstanceCount = 1 + (uint)g_Places.size();
		}
	}

	// Neighbor/context via hint chain (appdata → embedded → continent grid)
	{
		normalizePlaceContextSpecBasenames(); // suffix-resolved specs must match by resolved name
		std::vector<std::string> skip;
		for (size_t i = 0; i < g_EditableFiles.size(); ++i)
			skip.push_back(g_EditableFiles[i].Basename);
		// Board-placed context SPECS are authoritative: without this, a saved
		// hint of a spec'd brick reloads a second copy at the OLD cell after a drag/rotate.
		for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
			skip.push_back(g_PlaceContextSpecs[i].Basename);
		loadNeighborContextFiles(zones, g_SessionCellSize, skip);
	}
	// re-apply place-context specs (CLI + scratch UI) after hint load.
	// Failed specs are DROPPED - same rule as the initial assembly path. Keeping a
	// soft-failed spec produced phantom C: board cells (provisional 1×1 occupancy, no
	// RO geometry) that still collided and recorded as placeContext success.
	for (size_t pci = g_PlaceContextSpecs.size(); pci-- > 0; )
	{
		std::string perr;
		if (!loadOnePlaceContext(zones, g_SessionCellSize, g_PlaceContextSpecs[pci], perr))
		{
			fprintf(stderr, "WARNING: rebuild place-context: %s - dropping the placement\n",
			        perr.c_str());
			g_PlaceContextSpecs.erase(g_PlaceContextSpecs.begin() + (std::ptrdiff_t)pci);
		}
	}

	// One display-tile transform pass over the FULL assembly (instances + rotated context;
	// untransformed zones skip, and a single call avoids double-applying).
	if (g_SessionBank)
		applyInstanceDisplayTiles(zones, g_SessionBank);

	outWelds = weldPaintZones(zones);
	printf("session rebuild: %u zones, %u welds, %u editable files, %u neighbor files\n",
	       (uint)zones.size(), outWelds, (uint)g_EditableFiles.size(),
	       (uint)g_NeighborScenes.size());

	// Landscape: remove old, add new
	for (size_t i = 0; i < oldIds.size(); ++i)
		g_PaintCtx.Land->Landscape.removeZone((uint16)oldIds[i]);
	for (size_t i = 0; i < zones.size(); ++i)
	{
		NL3D::CZone zone;
		buildDisplayZone(zones[i], zone);
		if (!g_PaintCtx.Land->Landscape.addZone(zone))
			fprintf(stderr, "WARNING: rebuild addZone failed for %u '%s'\n",
			        zones[i].ZoneId, zones[i].Name.c_str());
	}
	g_PaintCtx.Land->Landscape.setRefineMode(true);

	// Paint core re-init (clears undo)
	std::vector<ZPPAINT::SPaintZoneInput> inputs;
	buildPaintInputs(zones, inputs);
	std::string initErr;
	if (!g_PaintCtx.Core->init(inputs, g_SessionBank, g_SessionCellSize, g_SessionSnap,
	                           g_SessionLockBorders, initErr, keepUndo))
	{
		err = "paint core re-init: " + initErr;
		return false;
	}
	g_PaintCtx.Core->restoreOriginalBytes(originals);
	g_PaintCtx.Core->attachLandscape(&g_PaintCtx.Land->Landscape);
	printf("session rebuild: undo %s; retained dirty flags restored (%u carriers stashed)\n",
	       keepUndo ? "kept (topology rebuild)" : "cleared", (uint)originals.size());
	return true;
}

// ---------------------------------------------------------------------------------------------
// Ecosystem scratch board callbacks (multi-cell occupancy)

int scratchFw() { return g_FootprintCellsW > 0 ? g_FootprintCellsW : 1; }
int scratchFh() { return g_FootprintCellsH > 0 ? g_FootprintCellsH : 1; }

const std::vector<bool> &scratchHomeMask()
{
	return g_FootprintMask;
}


void scratchBoardAnchor(float &ax, float &ay)
{
	if (g_SessionAnchorSet)
	{
		ax = g_SessionAnchorX;
		ay = g_SessionAnchorY;
		return;
	}
	ax = g_FootprintOriginX;
	ay = g_FootprintOriginY;
}

/**
 * an instance's source footprint (mask + dims): home when SourceBasename is empty,
 * else the matching open file's derived footprint. Falls back to home when the source
 * file is missing (pruned at the next rebuild).
 */
void instanceSourceFootprint(const SInstancePlace &pl, std::vector<bool> &mask,
                                    int &fw, int &fh)
{
	// sources resolve uniformly over EVERY open file (an empty SourceBasename is
	// the legacy spelling for the first-opened one); the derived-globals fallback only
	// covers the legacy direct-.max path with no file registry.
	const std::string srcName = (pl.SourceBasename.empty() && !g_EditableFiles.empty())
		? g_EditableFiles[0].Basename : pl.SourceBasename;
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (NLMISC::toLowerAscii(g_EditableFiles[i].Basename)
		    == NLMISC::toLowerAscii(srcName))
		{
			fw = g_EditableFiles[i].CellsW > 0 ? g_EditableFiles[i].CellsW : 1;
			fh = g_EditableFiles[i].CellsH > 0 ? g_EditableFiles[i].CellsH : 1;
			mask = g_EditableFiles[i].Mask;
			if (mask.empty()) mask.assign((size_t)fw * (size_t)fh, true);
			return;
		}
	}
	fw = scratchFw();
	fh = scratchFh();
	mask = scratchHomeMask();
	if (mask.empty()) mask.assign((size_t)fw * (size_t)fh, true);
}

/** True if place's rotFlip-transformed mask claims (cx,cy). */
bool scratchPlaceOccupies(const SInstancePlace &pl, int cx, int cy)
{
	std::vector<bool> sm;
	int sfw = 1, sfh = 1;
	instanceSourceFootprint(pl, sm, sfw, sfh);
	return maskCellOccupied(sm, sfw, sfh, pl.CellX, pl.CellY, pl.Rot, pl.Mirror, cx, cy);
}

/** Find place whose masked cells contain (cx,cy). */
bool scratchFindPlace(int cx, int cy, size_t &idx)
{
	for (size_t i = 0; i < g_Places.size(); ++i)
	{
		if (scratchPlaceOccupies(g_Places[i], cx, cy))
		{
			idx = i;
			return true;
		}
	}
	return false;
}

/** True when a place-context spec claims this basename (specs are authoritative). */
bool contextBasenameHasSpec(const std::string &basename)
{
	const std::string low = NLMISC::toLowerAscii(basename);
	for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
		if (NLMISC::toLowerAscii(g_PlaceContextSpecs[i].Basename) == low)
			return true;
	return false;
}

/**
 * True if the candidate mask collides with a hint-loaded RO context file (
 * these had no board occupancy at all). Spec-backed context files are excluded - the
 * caller's spec loop already covers them.
 */
bool hintContextConflicts(const std::vector<bool> &cmask, int cfw, int cfh,
                                 int ox, int oy, uint rot, bool mirror, std::string &err)
{
	for (size_t i = 0; i < g_ContextFiles.size(); ++i)
	{
		const SContextFile &cf = g_ContextFiles[i];
		if (!cf.TranslateGeom || contextBasenameHasSpec(cf.Basename)) continue;
		const int cw = cf.CellsW > 0 ? cf.CellsW : 1;
		const int ch = cf.CellsH > 0 ? cf.CellsH : 1;
		if (masksCollide(cmask, cfw, cfh, ox, oy, rot, mirror,
		                 cf.Mask, cw, ch, cf.CellX, cf.CellY, cf.Rot, cf.Mirror))
		{
			err = "mask overlaps read-only context '" + cf.Basename + "'";
			return true;
		}
	}
	return false;
}

/**
 * True if a candidate mask (any source's footprint) at (ox,oy) with rot/mirror
 * collides with home or any place except skipIdx (-1 = none).
 */
bool scratchMaskConflictsSrc(const std::vector<bool> &cmask, int cfw, int cfh,
                                    int ox, int oy, uint rot, bool mirror,
                                    int skipIdx, std::string &err)
{
	// no separate home check - the first-opened file collides through the uniform
	// open-files loop below like every other file.
	for (size_t i = 0; i < g_Places.size(); ++i)
	{
		if ((int)i == skipIdx) continue;
		std::vector<bool> om;
		int ofw = 1, ofh = 1;
		instanceSourceFootprint(g_Places[i], om, ofw, ofh);
		if (masksCollide(cmask, cfw, cfh, ox, oy, rot, mirror,
		                 om, ofw, ofh, g_Places[i].CellX, g_Places[i].CellY,
		                 g_Places[i].Rot, g_Places[i].Mirror))
		{
			err = NLMISC::toString("mask overlaps instance at (%d,%d)",
			                       g_Places[i].CellX, g_Places[i].CellY);
			return true;
		}
	}
	// Context bricks (multi-cell masks)
	for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
	{
		const SPlaceContextSpec &pc = g_PlaceContextSpecs[i];
		const int cw = pc.CellsW > 0 ? pc.CellsW : 1;
		const int ch = pc.CellsH > 0 ? pc.CellsH : 1;
		std::vector<bool> cm = pc.Mask;
		if (cm.empty()) cm.assign((size_t)cw * (size_t)ch, true);
		if (masksCollide(cmask, cfw, cfh, ox, oy, rot, mirror,
		                 cm, cw, ch, pc.Dx, pc.Dy, pc.Rot, pc.Mirror))
		{
			err = NLMISC::toString("mask overlaps context '%s' at (%d,%d)",
			                       pc.Basename.c_str(), pc.Dx, pc.Dy);
			return true;
		}
	}
	// Open files - ALL of them, first-opened included
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		const SEditableFileInfo &ef = g_EditableFiles[i];
		const int cw = ef.CellsW > 0 ? ef.CellsW : 1;
		const int ch = ef.CellsH > 0 ? ef.CellsH : 1;
		std::vector<bool> em = ef.Mask;
		if (em.empty()) em.assign((size_t)cw * (size_t)ch, true);
		if (masksCollide(cmask, cfw, cfh, ox, oy, rot, mirror,
		                 em, cw, ch, ef.CellX, ef.CellY, 0, false))
		{
			err = NLMISC::toString("mask overlaps open file '%s' at (%d,%d)",
			                       ef.Basename.c_str(), ef.CellX, ef.CellY);
			return true;
		}
	}
	if (hintContextConflicts(cmask, cfw, cfh, ox, oy, rot, mirror, err))
		return true;
	return false;
}

/** Candidate order for duplicate-instance placement: ring (max-norm), then Manhattan
 * within the ring so axis-aligned offsets come before the corner diagonals. */
struct SDupCandLess
{
	bool operator()(const std::pair<int, int> &a, const std::pair<int, int> &b) const
	{
		const int ra = std::max(std::abs(a.first), std::abs(a.second));
		const int rb = std::max(std::abs(b.first), std::abs(b.second));
		if (ra != rb) return ra < rb;
		return std::abs(a.first) + std::abs(a.second) < std::abs(b.first) + std::abs(b.second);
	}
};

/** register an instance of an OPEN file at (or ring-spiraled near) (cx,cy) -
 * duplicate selector opens resolve to shared-paint instances, never a second load of
 * the same file (two carrier sets would diverge and fight over one save path).
 * Pre-rebuild registration only: callers rebuild (or the initial assembly appends). */
bool placeDupInstanceNear(const std::string &srcName, int cx, int cy)
{
	SInstancePlace pl(cx, cy, 0, false);
	pl.SourceBasename = srcName;
	std::vector<bool> sm;
	int fw = 1, fh = 1;
	instanceSourceFootprint(pl, sm, fw, fh);
	std::string tmp;
	// Ring order, axis-aligned offsets first within each ring - a duplicate of a WxH
	// block lands side-by-side (e.g. (W,0)) instead of at the ring's corner diagonal.
	std::vector<std::pair<int, int> > cand;
	for (int r = 0; r <= 12; ++r)
	for (int dy = -r; dy <= r; ++dy)
	for (int dx = -r; dx <= r; ++dx)
		if (std::max(std::abs(dx), std::abs(dy)) == r)
			cand.push_back(std::make_pair(dx, dy));
	std::stable_sort(cand.begin(), cand.end(), SDupCandLess());
	for (size_t i = 0; i < cand.size(); ++i)
	{
		const int dx = cand[i].first, dy = cand[i].second;
		if (!scratchMaskConflictsSrc(sm, fw, fh, cx + dx, cy + dy, 0, false, -1, tmp))
		{
			pl.CellX = cx + dx;
			pl.CellY = cy + dy;
			g_Places.push_back(pl);
			g_InstanceCount = 1 + (uint)g_Places.size();
			printf("duplicate open '%s' -> shared-paint instance @ (%d,%d)\n",
			       srcName.c_str(), pl.CellX, pl.CellY);
			return true;
		}
	}
	fprintf(stderr, "WARNING: duplicate open '%s': no free cell within r=12 - dropped\n",
	        srcName.c_str());
	return false;
}

// (The old "home-shaped candidate" conflict wrappers - scratchMaskConflicts,
// scratchBlockConflicts, scratchBlocksOverlap - are gone; every caller passes the real
// per-source footprint through scratchMaskConflictsSrc.)

bool scratchRebuild(std::string &err)
{
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds))
		return false;
	printf("scratch rebuild: %u place(s), footprint %dx%d, %u welds\n",
	       (uint)g_Places.size(), scratchFw(), scratchFh(), welds);
	return true;
}

/** After a FAILED scratchRebuild, once the caller rolled its board mutation back:
 * rebuild again so the session stays on live zones. A failed rebuild leaves the core
 * referencing the CLEARED zones vector - without recovery the next frame's hover walk
 * is a use-after-free (same rule as the open/close/toggle ops). */
void scratchRecoveryRebuild(const char *op)
{
	std::string rerr;
	if (!scratchRebuild(rerr))
		fprintf(stderr, "ERROR: %s: recovery rebuild failed: %s\n", op, rerr.c_str());
}

/** Find place-context index whose masked cells cover (cx,cy) (multi-cell). */
bool scratchFindContext(int cx, int cy, size_t &idx)
{
	for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
	{
		const SPlaceContextSpec &pc = g_PlaceContextSpecs[i];
		const int cw = pc.CellsW > 0 ? pc.CellsW : 1;
		const int ch = pc.CellsH > 0 ? pc.CellsH : 1;
		if (maskCellOccupied(pc.Mask, cw, ch, pc.Dx, pc.Dy, pc.Rot, pc.Mirror, cx, cy))
		{
			idx = i;
			return true;
		}
	}
	return false;
}

/** find the non-primary open file whose masked cells cover (cx,cy) (eco board). */
bool scratchFindEditableAt(int cx, int cy, size_t &idx)
{
	// ALL open files, the first-opened included - it occupies board cells through
	// the same per-file mask fields as everyone else.
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		const SEditableFileInfo &ef = g_EditableFiles[i];
		const int cw = ef.CellsW > 0 ? ef.CellsW : 1;
		const int ch = ef.CellsH > 0 ? ef.CellsH : 1;
		if (maskCellOccupied(ef.Mask, cw, ch, ef.CellX, ef.CellY, 0, false, cx, cy))
		{
			idx = i;
			return true;
		}
	}
	return false;
}

/** Bridge info: open-file block covering (cx,cy) → origin + name + editable flag. */
bool scratchGetEditableAt(int cx, int cy, int &ox, int &oy, std::string &basename,
                                 bool &editable)
{
	size_t idx = 0;
	if (!scratchFindEditableAt(cx, cy, idx))
		return false;
	ox = g_EditableFiles[idx].CellX;
	oy = g_EditableFiles[idx].CellY;
	basename = g_EditableFiles[idx].Basename;
	editable = g_EditableFiles[idx].Editable;
	return true;
}

bool scratchGetCellState(const std::string &basename, ZPUI::ESessionCellState &out)
{
	// Multi-cell: basenames I:ox,oy@cx,cy / E:cx,cy / F:... / C:cx,cy:basename (
	// no H: tokens - the first-opened file is an F: open-file cell like any other)
	if (basename.size() >= 4 && basename[0] == 'I' && basename[1] == ':')
	{
		out = ZPUI::CellScratchInstance;
		return true;
	}
	if (basename.size() >= 4 && basename[0] == 'C' && basename[1] == ':')
	{
		out = ZPUI::CellScratchContext;
		return true;
	}
	if (basename.size() >= 4 && basename[0] == 'F' && basename[1] == ':')
	{
		// Open-file cell F:ox,oy:name / F:ox,oy@cx,cy:name - state from the file
		// entry. Prefer the trailing NAME (unique per file); the origin-cell lookup
		// fails when a template mask's min-corner cell is a hole.
		std::string rest = basename.substr(2);
		{
			std::string::size_type lastColon = rest.rfind(':');
			if (lastColon != std::string::npos && lastColon + 1 < rest.size())
			{
				if (const SEditableFileInfo *ef = findEditableByBasename(rest.substr(lastColon + 1)))
				{
					if (!ef->Editable)
						out = ZPUI::CellOpenReadOnly;
					else
						out = g_PaintCtx.Core && g_PaintCtx.Core->anyZoneDirty(ef->ZoneIds)
						          ? ZPUI::CellDirtyEditable
						          : ZPUI::CellOpenEditable;
					return true;
				}
			}
		}
		std::string::size_type comma = rest.find(',');
		int ox = 0, oy = 0;
		if (comma != std::string::npos && NLMISC::fromString(rest.substr(0, comma), ox))
		{
			std::string tail = rest.substr(comma + 1);
			std::string::size_type stop = tail.find_first_of("@:");
			if (NLMISC::fromString(stop == std::string::npos ? tail : tail.substr(0, stop), oy))
			{
				size_t idx = 0;
				if (scratchFindEditableAt(ox, oy, idx))
				{
					const SEditableFileInfo &ef = g_EditableFiles[idx];
					if (!ef.Editable)
						out = ZPUI::CellOpenReadOnly;
					else
						out = g_PaintCtx.Core && g_PaintCtx.Core->anyZoneDirty(ef.ZoneIds)
						          ? ZPUI::CellDirtyEditable
						          : ZPUI::CellOpenEditable;
					return true;
				}
			}
		}
		out = ZPUI::CellOpenEditable;
		return true;
	}
	if (basename.size() >= 4 && basename[0] == 'L' && basename[1] == ':')
	{
		out = ZPUI::CellScratchLocked;
		return true;
	}
	if (basename.size() >= 4 && basename[0] == 'E' && basename[1] == ':')
	{
		// Empty unless a place-context or open file occupies this cell
		char sk = 0;
		int cx = 0, cy = 0;
		// parse E:cx,cy
		std::string rest = basename.substr(2);
		std::string::size_type comma = rest.find(',');
		if (comma != std::string::npos
		    && NLMISC::fromString(rest.substr(0, comma), cx)
		    && NLMISC::fromString(rest.substr(comma + 1), cy))
		{
			size_t idx = 0;
			if (scratchFindContext(cx, cy, idx))
			{
				out = ZPUI::CellScratchContext;
				return true;
			}
			if (scratchFindEditableAt(cx, cy, idx))
			{
				out = g_EditableFiles[idx].Editable ? ZPUI::CellOpenEditable
				                                    : ZPUI::CellOpenReadOnly;
				return true;
			}
		}
		out = ZPUI::CellScratchEmpty;
		return true;
	}
	return false;
}


bool loadOnePlaceContext(std::vector<SPaintZone> &zones, float cellSize,
                                const SPlaceContextSpec &pc, std::string &err)
{
	if (g_StartupWorld.MaxDir.empty())
	{
		err = "no world";
		return false;
	}
	ZPWS::SZoneEntry ze;
	if (!resolveHintToZone(g_StartupWorld, pc.Basename, ze))
	{
		err = "unresolved brick: " + pc.Basename;
		return false;
	}
	for (size_t i = 0; i < g_ContextFiles.size(); ++i)
	{
		if (NLMISC::toLowerAscii(g_ContextFiles[i].Basename)
		    == NLMISC::toLowerAscii(ze.Basename)
		    && g_ContextFiles[i].CellX == pc.Dx && g_ContextFiles[i].CellY == pc.Dy)
			return true; // already loaded at this offset
	}
	if (findEditableByBasename(ze.Basename))
	{
		err = "basename is editable: " + ze.Basename;
		return false;
	}
	PMAXLOAD::SLoadedMax *nlm = new PMAXLOAD::SLoadedMax();
	if (!PMAXLOAD::loadMaxFile(ze.MaxPath, *nlm))
	{
		delete nlm;
		err = "load failed: " + ze.MaxPath;
		return false;
	}
	uint base = nextZoneIdBase(zones);
	const uint minBase = (uint)((g_EditableFiles.size() + g_ContextFiles.size() + 1) * 1000);
	if (base < minBase) base = minBase;
	const size_t before = zones.size();
	if (!buildPaintZones(*nlm->Scene, zones, base, /*forceFrozen=*/true, ze.Basename))
	{
		freeLoadedMax(nlm);
		err = "no zones in " + ze.Basename;
		return false;
	}
	// Home footprint origin (prefer derived globals when set)
	float homeOriginX = 0.f, homeOriginY = 0.f;
	scratchBoardAnchor(homeOriginX, homeOriginY); // session-frozen board frame
	if (g_FootprintCellsW < 1)
	{
		float stepX = 0.f, stepY = 0.f;
		int fw = 1, fh = 1;
		size_t primaryEnd = 0;
		for (size_t i = 0; i < zones.size(); ++i)
			if (zones[i].ZoneId < 1000) primaryEnd = i + 1;
		if (primaryEnd > 0)
			computeFootprintRect(zones, 0, primaryEnd, cellSize,
			                     homeOriginX, homeOriginY, stepX, stepY, fw, fh);
	}
	// Context brick footprint (mask + origin) for snap + multi-cell occupancy
	float cOx = 0.f, cOy = 0.f;
	int cw = 1, ch = 1;
	bool fromT = false;
	std::vector<bool> cmask;
	std::string derr;
	// Eligible brick zone for this context file (same product model as export /
	// derivePrimaryFootprint). Board authority already dropped ineligible embeds;
	// force-frozen load means "first non-frozen" would never break - fall through
	// keeps last-of-range as a legacy --embedded-context safety, not the preferred path.
	size_t cPick = before;
	for (size_t i = before; i < zones.size(); ++i)
	{
		if (!zones[i].Frozen) { cPick = i; break; }
		cPick = i;
	}
	if (cPick < zones.size())
		deriveZoneFootprintMask(zones[cPick], cellSize, g_SessionSnap > 0.f ? g_SessionSnap : 1.f,
		                        cmask, cw, ch, cOx, cOy, fromT, derr);
	else
	{
		float cSx = 0.f, cSy = 0.f;
		computeFootprintRect(zones, before, zones.size(), cellSize, cOx, cOy, cSx, cSy, cw, ch);
		cmask.assign((size_t)cw * (size_t)ch, true);
	}
	// Same rule as neighbor load: intended(cell) - authoredFootprintOrigin.
	// non-default rot/mirror transforms about the block pivot (instance math);
	// the rot0 path keeps the historical pure translate byte-for-byte.
	if ((pc.Rot & 3) == 0 && !pc.Mirror)
	{
		const float wantX = homeOriginX + (float)pc.Dx * cellSize;
		const float wantY = homeOriginY + (float)pc.Dy * cellSize;
		translateZonesXY(zones, before, zones.size(), wantX - cOx, wantY - cOy);
	}
	else
		placeContextRange(zones, before, zones.size(), cOx, cOy, cw, ch,
		                  homeOriginX, homeOriginY, cellSize, pc.Dx, pc.Dy, pc.Rot, pc.Mirror);

	// Stash multi-cell mask on the matching place-context spec (if any)
	for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
	{
		if (g_PlaceContextSpecs[i].Dx == pc.Dx && g_PlaceContextSpecs[i].Dy == pc.Dy
		    && NLMISC::toLowerAscii(g_PlaceContextSpecs[i].Basename)
		       == NLMISC::toLowerAscii(ze.Basename))
		{
			g_PlaceContextSpecs[i].CellsW = cw;
			g_PlaceContextSpecs[i].CellsH = ch;
			g_PlaceContextSpecs[i].Mask = cmask;
			break;
		}
	}

	SContextFile cf;
	cf.Path = ze.MaxPath;
	cf.Basename = ze.Basename;
	cf.CellX = pc.Dx;
	cf.CellY = pc.Dy;
	cf.TranslateGeom = true;
	cf.Rot = pc.Rot & 3;
	cf.Mirror = pc.Mirror;
	cf.CellsW = cw;
	cf.CellsH = ch;
	cf.Mask = cmask;
	cf.Lm = nlm;
	g_ContextFiles.push_back(cf);
	g_NeighborScenes.push_back(nlm);
	printf("place-context: '%s' @ (%d,%d) footprint %dx%d source=%s mask=%s%s%s FROZEN translated\n",
	       cf.Basename.c_str(), cf.CellX, cf.CellY, cw, ch,
	       fromT ? "template" : "aabb-square",
	       maskToTFString(cmask, cw, ch).c_str(),
	       cf.Rot ? NLMISC::toString(" R%u", cf.Rot * 90).c_str() : "",
	       cf.Mirror ? " M" : "");
	return true;
}


bool scratchPlaceContext(int cx, int cy, const std::string &basename, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchPlaceContextImpl(cx, cy, basename, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.placeContext(%d, %d, %s)", cx, cy, luaQuote(basename).c_str()));
	return ok;
}
bool scratchPlaceContextImpl(int cx, int cy, const std::string &basename, std::string &err)
{
	size_t idx = 0;
	if (scratchFindPlace(cx, cy, idx))
	{
		err = "cell has a self-instance";
		return false;
	}
	if (scratchFindContext(cx, cy, idx))
	{
		err = "cell already has context";
		return false;
	}
	if (scratchFindEditableAt(cx, cy, idx))
	{
		err = "cell has an open file";
		return false;
	}
	SPlaceContextSpec pc;
	pc.Dx = cx;
	pc.Dy = cy;
	pc.Basename = basename;
	pc.CellsW = 1;
	pc.CellsH = 1;
	pc.Mask.assign(1, true);
	// strip .max
	std::string::size_type dot = pc.Basename.rfind('.');
	if (dot != std::string::npos && NLMISC::toLowerAscii(pc.Basename.substr(dot)) == ".max")
		pc.Basename = pc.Basename.substr(0, dot);
	// Refuse an already-open editable of the same basename - loadOnePlaceContext would
	// fail mid-rebuild and (pre-fix) leave a phantom board cell that still "succeeded".
	if (findEditableByBasename(pc.Basename))
	{
		err = "basename is editable: " + pc.Basename;
		return false;
	}
	if (contextBasenameHasSpec(pc.Basename))
	{
		err = "context already placed: " + pc.Basename;
		return false;
	}
	g_PlaceContextSpecs.push_back(pc);
	if (!scratchRebuild(err))
	{
		g_PlaceContextSpecs.pop_back();
		// Recovery rebuild: the failed one left the core referencing the cleared zones
		// vector - rebuilding without the bad spec keeps the session live (UAF otherwise).
		std::string rerr;
		if (!scratchRebuild(rerr))
			fprintf(stderr, "ERROR: place-context recovery rebuild failed: %s\n", rerr.c_str());
		return false;
	}
	// Spec may have been dropped by rebuild if load failed for another reason.
	if (g_PlaceContextSpecs.empty()
	    || NLMISC::toLowerAscii(g_PlaceContextSpecs.back().Basename)
	       != NLMISC::toLowerAscii(pc.Basename))
	{
		err = "place-context load failed: " + pc.Basename;
		return false;
	}
	// After load, multi-cell mask is filled in loadOnePlaceContext - re-check collisions
	// against the derived mask (1×1 provisional may have missed multi-cell overlap).
	// a conflicting placement auto-shifts to the nearest fitting cell instead of
	// rolling back (the brick still loads; the user drags it into place afterwards).
	{
		const size_t li = g_PlaceContextSpecs.size() - 1;
		std::string cerr;
		if (contextCandidateConflicts(li, g_PlaceContextSpecs[li].Dx, g_PlaceContextSpecs[li].Dy,
		                              g_PlaceContextSpecs[li].Rot, g_PlaceContextSpecs[li].Mirror,
		                              cerr))
		{
			const int wantX = g_PlaceContextSpecs[li].Dx, wantY = g_PlaceContextSpecs[li].Dy;
			bool placed = false;
			for (int r = 1; r <= 12 && !placed; ++r)
			for (int dy = -r; dy <= r && !placed; ++dy)
			for (int dx = -r; dx <= r && !placed; ++dx)
			{
				if (std::max(std::abs(dx), std::abs(dy)) != r) continue;
				std::string tmp;
				if (!contextCandidateConflicts(li, wantX + dx, wantY + dy,
				                               g_PlaceContextSpecs[li].Rot,
				                               g_PlaceContextSpecs[li].Mirror, tmp))
				{
					g_PlaceContextSpecs[li].Dx = wantX + dx;
					g_PlaceContextSpecs[li].Dy = wantY + dy;
					placed = true;
				}
			}
			if (!placed)
			{
				g_PlaceContextSpecs.pop_back();
				if (!scratchRebuild(err))
					scratchRecoveryRebuild("place-context no-fit");
				err = cerr + " (no nearby fit)";
				return false;
			}
			fprintf(stderr, "place-context: '%s' does not fit at (%d,%d) - auto-shifted to (%d,%d)\n",
			        g_PlaceContextSpecs[li].Basename.c_str(), wantX, wantY,
			        g_PlaceContextSpecs[li].Dx, g_PlaceContextSpecs[li].Dy);
			if (!scratchRebuild(err))
			{
				// Second rebuild failed with the shifted cell - drop the spec and recover
				// (same UAF class as every other board op).
				g_PlaceContextSpecs.pop_back();
				scratchRecoveryRebuild("place-context auto-shift");
				return false;
			}
			return true;
		}
	}
	return true;
}


bool scratchRemoveContext(int cx, int cy, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchRemoveContextImpl(cx, cy, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.removeContext(%d, %d)", cx, cy));
	return ok;
}
bool scratchRemoveContextImpl(int cx, int cy, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindContext(cx, cy, idx))
	{
		err = "no context at cell";
		return false;
	}
	const SPlaceContextSpec removed = g_PlaceContextSpecs[idx];
	g_PlaceContextSpecs.erase(g_PlaceContextSpecs.begin() + (std::ptrdiff_t)idx);
	if (!scratchRebuild(err))
	{
		g_PlaceContextSpecs.insert(g_PlaceContextSpecs.begin() + (std::ptrdiff_t)idx, removed);
		scratchRecoveryRebuild("remove-context");
		return false;
	}
	return true;
}

/** transform mask of a context spec for collision checks. */
void contextSpecMask(const SPlaceContextSpec &pc, std::vector<bool> &m, int &cw, int &ch)
{
	cw = pc.CellsW > 0 ? pc.CellsW : 1;
	ch = pc.CellsH > 0 ? pc.CellsH : 1;
	m = pc.Mask;
	if (m.empty()) m.assign((size_t)cw * (size_t)ch, true);
}

/** does candidate context spec state (cell+transform) collide with the rest? */
bool contextCandidateConflicts(size_t idx, int nx, int ny, uint nrot, bool nmirror,
                                      std::string &err)
{
	std::vector<bool> cm;
	int cw = 1, ch = 1;
	contextSpecMask(g_PlaceContextSpecs[idx], cm, cw, ch);
	// the first-opened file collides through the uniform open-files loop below.
	for (size_t i = 0; i < g_Places.size(); ++i)
	{
		std::vector<bool> om;
		int ow = 1, oh = 1;
		instanceSourceFootprint(g_Places[i], om, ow, oh);
		if (masksCollide(om, ow, oh, g_Places[i].CellX, g_Places[i].CellY,
		                 g_Places[i].Rot, g_Places[i].Mirror,
		                 cm, cw, ch, nx, ny, nrot, nmirror))
		{
			err = "would overlap an instance";
			return true;
		}
	}
	for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
	{
		if (i == idx) continue;
		std::vector<bool> om;
		int ow = 1, oh = 1;
		contextSpecMask(g_PlaceContextSpecs[i], om, ow, oh);
		if (masksCollide(cm, cw, ch, nx, ny, nrot, nmirror,
		                 om, ow, oh, g_PlaceContextSpecs[i].Dx, g_PlaceContextSpecs[i].Dy,
		                 g_PlaceContextSpecs[i].Rot, g_PlaceContextSpecs[i].Mirror))
		{
			err = "would overlap context '" + g_PlaceContextSpecs[i].Basename + "'";
			return true;
		}
	}
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		const SEditableFileInfo &ef = g_EditableFiles[i];
		const int ow = ef.CellsW > 0 ? ef.CellsW : 1;
		const int oh = ef.CellsH > 0 ? ef.CellsH : 1;
		std::vector<bool> om = ef.Mask;
		if (om.empty()) om.assign((size_t)ow * (size_t)oh, true);
		if (masksCollide(cm, cw, ch, nx, ny, nrot, nmirror,
		                 om, ow, oh, ef.CellX, ef.CellY, 0, false))
		{
			err = "would overlap open file '" + ef.Basename + "'";
			return true;
		}
	}
	if (hintContextConflicts(cm, cw, ch, nx, ny, nrot, nmirror, err))
		return true;
	return false;
}

/** rotate a placed context brick about its footprint-block center (instance rule). */
bool scratchRotateContextImpl(int cx, int cy, int delta, std::string &err);
bool scratchRotateContext(int cx, int cy, int delta, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchRotateContextImpl(cx, cy, delta, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.rotateContext(%d, %d, %d)", cx, cy, delta));
	return ok;
}
bool scratchRotateContextImpl(int cx, int cy, int delta, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindContext(cx, cy, idx))
	{
		err = "no context at cell";
		return false;
	}
	SPlaceContextSpec &pc = g_PlaceContextSpecs[idx];
	// Rotate anchored at the block's min corner (origin stays put, size transposes):
	// the old floor(center − newSize/2) pivot drifted odd-parity blocks (2x1 etc.) by a
	// half-cell-floored step per rotation - CW then CCW landed one cell off, and four CWs
	// walked the block diagonally. Center-pivoting an odd-parity block on a whole-cell
	// grid is unrepresentable; the min-corner anchor is exact and reversible.
	const uint newRot = (uint)((int)pc.Rot + delta) & 3;
	if (contextCandidateConflicts(idx, pc.Dx, pc.Dy, newRot, pc.Mirror, err))
		return false;
	const uint oldRot = pc.Rot;
	pc.Rot = newRot;
	if (!scratchRebuild(err))
	{
		if (idx < g_PlaceContextSpecs.size())
			g_PlaceContextSpecs[idx].Rot = oldRot;
		scratchRecoveryRebuild("rotate-context");
		return false;
	}
	return true;
}

/** mirror a placed context brick (block AABB stays; mask occupancy rechecked). */
bool scratchMirrorContextImpl(int cx, int cy, std::string &err);
bool scratchMirrorContext(int cx, int cy, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchMirrorContextImpl(cx, cy, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.mirrorContext(%d, %d)", cx, cy));
	return ok;
}
bool scratchMirrorContextImpl(int cx, int cy, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindContext(cx, cy, idx))
	{
		err = "no context at cell";
		return false;
	}
	SPlaceContextSpec &pc = g_PlaceContextSpecs[idx];
	if (contextCandidateConflicts(idx, pc.Dx, pc.Dy, pc.Rot, !pc.Mirror, err))
		return false;
	pc.Mirror = !pc.Mirror;
	if (!scratchRebuild(err))
	{
		if (idx < g_PlaceContextSpecs.size())
			g_PlaceContextSpecs[idx].Mirror = !g_PlaceContextSpecs[idx].Mirror;
		scratchRecoveryRebuild("mirror-context");
		return false;
	}
	return true;
}

/** context transform for board labels. */
bool scratchGetContextTransform(int cx, int cy, uint &rot, bool &mirror)
{
	size_t idx = 0;
	if (!scratchFindContext(cx, cy, idx))
		return false;
	rot = g_PlaceContextSpecs[idx].Rot;
	mirror = g_PlaceContextSpecs[idx].Mirror;
	return true;
}


/**
 * board drag-drop - move (or copy, Ctrl/Shift held) whatever occupies the grab
 * cell so its block origin shifts by the drag delta. Move keeps identity (instance
 * transform, context file, open file); copy duplicates (home → home instance, instance
 * → same-source instance, context → second placement, open file → instance of it).
 */
bool scratchDragDropImpl(int fx, int fy, int tx, int ty, bool copy, std::string &err);
bool scratchDragDrop(int fx, int fy, int tx, int ty, bool copy, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchDragDropImpl(fx, fy, tx, ty, copy, err);
	if (ok)
		recordBoardOp(NLMISC::toString(copy ? "painter.copyCell(%d, %d, %d, %d)" : "painter.moveCell(%d, %d, %d, %d)", fx, fy, tx, ty));
	return ok;
}
bool scratchDragDropImpl(int fx, int fy, int tx, int ty, bool copy, std::string &err)
{
	const int dx = tx - fx, dy = ty - fy;
	if (dx == 0 && dy == 0) { err = "same cell"; return false; }
	size_t idx = 0;
	// no home branch - the first-opened file is found by scratchFindEditableAt like
	// every other open file, and moves/copies through the uniform open-file branch below.
	if (scratchFindPlace(fx, fy, idx))
	{
		SInstancePlace pl = g_Places[idx];
		const int nx = pl.CellX + dx, ny = pl.CellY + dy;
		std::vector<bool> sm;
		int sfw = 1, sfh = 1;
		instanceSourceFootprint(pl, sm, sfw, sfh);
		if (copy)
		{
			if (scratchMaskConflictsSrc(sm, sfw, sfh, nx, ny, pl.Rot, pl.Mirror, -1, err))
				return false;
			SInstancePlace np = pl;
			np.CellX = nx;
			np.CellY = ny;
			g_Places.push_back(np);
			g_InstanceCount = 1 + (uint)g_Places.size();
			if (!scratchRebuild(err))
			{
				g_Places.pop_back();
				g_InstanceCount = 1 + (uint)g_Places.size();
				scratchRecoveryRebuild("drag-copy instance");
				return false;
			}
			return true;
		}
		if (scratchMaskConflictsSrc(sm, sfw, sfh, nx, ny, pl.Rot, pl.Mirror, (int)idx, err))
			return false;
		g_Places[idx].CellX = nx;
		g_Places[idx].CellY = ny;
		if (!scratchRebuild(err))
		{
			if (idx < g_Places.size())
			{
				g_Places[idx].CellX = pl.CellX;
				g_Places[idx].CellY = pl.CellY;
			}
			scratchRecoveryRebuild("drag-move instance");
			return false;
		}
		return true;
	}
	if (scratchFindContext(fx, fy, idx))
	{
		const int nx = g_PlaceContextSpecs[idx].Dx + dx, ny = g_PlaceContextSpecs[idx].Dy + dy;
		if (copy)
		{
			SPlaceContextSpec np = g_PlaceContextSpecs[idx];
			np.Dx = nx;
			np.Dy = ny;
			g_PlaceContextSpecs.push_back(np);
			std::string cerr;
			if (contextCandidateConflicts(g_PlaceContextSpecs.size() - 1, nx, ny,
			                              np.Rot, np.Mirror, cerr))
			{
				g_PlaceContextSpecs.pop_back();
				err = cerr;
				return false;
			}
			if (!scratchRebuild(err))
			{
				g_PlaceContextSpecs.pop_back();
				scratchRecoveryRebuild("drag-copy context");
				return false;
			}
			return true;
		}
		std::string cerr;
		if (contextCandidateConflicts(idx, nx, ny, g_PlaceContextSpecs[idx].Rot,
		                              g_PlaceContextSpecs[idx].Mirror, cerr))
		{
			err = cerr;
			return false;
		}
		const int odx = g_PlaceContextSpecs[idx].Dx, ody = g_PlaceContextSpecs[idx].Dy;
		g_PlaceContextSpecs[idx].Dx = nx;
		g_PlaceContextSpecs[idx].Dy = ny;
		if (!scratchRebuild(err))
		{
			if (idx < g_PlaceContextSpecs.size())
			{
				g_PlaceContextSpecs[idx].Dx = odx;
				g_PlaceContextSpecs[idx].Dy = ody;
			}
			scratchRecoveryRebuild("drag-move context");
			return false;
		}
		return true;
	}
	if (scratchFindEditableAt(fx, fy, idx))
	{
		SEditableFileInfo &ef = g_EditableFiles[idx];
		const int nx = ef.CellX + dx, ny = ef.CellY + dy;
		if (copy)
			return scratchPlaceInstanceOf(nx, ny, ef.Basename, err);
		const int ox = ef.CellX, oy = ef.CellY;
		ef.CellX = nx;
		ef.CellY = ny;
		std::string cerr;
		if (scratchEditableConflicts(idx, cerr))
		{
			ef.CellX = ox;
			ef.CellY = oy;
			err = cerr;
			return false;
		}
		if (!scratchRebuild(err))
		{
			// The home-move branch had this recovery; the uniform branch must
			// too - a failed rebuild otherwise keeps the new cell with the session dead.
			if (idx < g_EditableFiles.size())
			{
				g_EditableFiles[idx].CellX = ox;
				g_EditableFiles[idx].CellY = oy;
			}
			scratchRecoveryRebuild("drag-move file");
			return false;
		}
		return true;
	}
	err = "nothing to drag at cell";
	return false;
}

bool scratchGetContext(int cx, int cy, std::string &basename)
{
	size_t idx = 0;
	if (!scratchFindContext(cx, cy, idx))
		return false;
	basename = g_PlaceContextSpecs[idx].Basename;
	return true;
}

/** does open file `idx`'s derived footprint collide with anything else on the board? */
bool scratchEditableConflicts(size_t idx, std::string &err)
{
	if (idx >= g_EditableFiles.size()) return false;
	const SEditableFileInfo &ef = g_EditableFiles[idx];
	const int cw = ef.CellsW > 0 ? ef.CellsW : 1;
	const int ch = ef.CellsH > 0 ? ef.CellsH : 1;
	std::vector<bool> em = ef.Mask;
	if (em.empty()) em.assign((size_t)cw * (size_t)ch, true);
	// the first-opened file collides through the uniform open-files loop below.
	for (size_t i = 0; i < g_Places.size(); ++i)
	{
		std::vector<bool> om;
		int ow = 1, oh = 1;
		instanceSourceFootprint(g_Places[i], om, ow, oh);
		if (masksCollide(om, ow, oh,
		                 g_Places[i].CellX, g_Places[i].CellY, g_Places[i].Rot, g_Places[i].Mirror,
		                 em, cw, ch, ef.CellX, ef.CellY, 0, false))
		{
			err = "footprint overlaps an instance";
			return true;
		}
	}
	for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
	{
		const SPlaceContextSpec &pc = g_PlaceContextSpecs[i];
		const int ow = pc.CellsW > 0 ? pc.CellsW : 1;
		const int oh = pc.CellsH > 0 ? pc.CellsH : 1;
		std::vector<bool> om = pc.Mask;
		if (om.empty()) om.assign((size_t)ow * (size_t)oh, true);
		if (masksCollide(em, cw, ch, ef.CellX, ef.CellY, 0, false,
		                 om, ow, oh, pc.Dx, pc.Dy, pc.Rot, pc.Mirror))
		{
			err = "footprint overlaps context '" + pc.Basename + "'";
			return true;
		}
	}
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (i == idx) continue;
		const SEditableFileInfo &of = g_EditableFiles[i];
		const int ow = of.CellsW > 0 ? of.CellsW : 1;
		const int oh = of.CellsH > 0 ? of.CellsH : 1;
		std::vector<bool> om = of.Mask;
		if (om.empty()) om.assign((size_t)ow * (size_t)oh, true);
		if (masksCollide(em, cw, ch, ef.CellX, ef.CellY, 0, false,
		                 om, ow, oh, of.CellX, of.CellY, 0, false))
		{
			err = "footprint overlaps open file '" + of.Basename + "'";
			return true;
		}
	}
	if (hintContextConflicts(em, cw, ch, ef.CellX, ef.CellY, 0, false, err))
		return true;
	return false;
}

/**
 * open a world brick as an EDITABLE file placed at board cell (cx,cy) - the eco
 * counterpart of the continent sessionOpenZone. The footprint derives at rebuild; a
 * post-rebuild collision check rolls the open back (auto-shift is not implemented yet).
 */
bool scratchOpenEditableImpl(int cx, int cy, const std::string &basenameIn, std::string &err);
bool scratchOpenEditable(int cx, int cy, const std::string &basenameIn, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchOpenEditableImpl(cx, cy, basenameIn, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.openZone(%s, %d, %d)", luaQuote(basenameIn).c_str(), cx, cy));
	return ok;
}
bool scratchOpenEditableImpl(int cx, int cy, const std::string &basenameIn, std::string &err)
{
	size_t idx = 0;
	if (scratchFindPlace(cx, cy, idx)) { err = "cell has an instance"; return false; }
	if (scratchFindContext(cx, cy, idx)) { err = "cell has context (remove it first)"; return false; }
	if (scratchFindEditableAt(cx, cy, idx)) { err = "cell has an open file"; return false; }
	std::string basename = basenameIn;
	std::string::size_type dot = basename.rfind('.');
	if (dot != std::string::npos && NLMISC::toLowerAscii(basename.substr(dot)) == ".max")
		basename = basename.substr(0, dot);
	// A RO context SPEC of this basename must be promoted via makeEditable (which removes
	// the spec first). Opening a second copy under the same basename would soft-fail the
	// place-context reload of the first and leave a phantom board cell.
	if (contextBasenameHasSpec(basename))
	{
		err = "basename is a placed context (use Make editable): " + basename;
		return false;
	}
	ZPWS::SZoneEntry ze;
	if (!resolveHintToZone(g_StartupWorld, basename, ze))
	{
		err = "unresolved brick: " + basename;
		return false;
	}
	if (SEditableFileInfo *dup = findEditableByBasename(ze.Basename))
	{
		// Basename is the file-identity key - a DIFFERENT file with the same name must
		// refuse, not silently become an instance of the other file .
		if (absFilePath(dup->Path) != absFilePath(ze.MaxPath))
		{
			err = "another file with basename '" + ze.Basename + "' is already open ("
			      + dup->Path + ")";
			return false;
		}
		// the file is one on disk - opening it again is an INSTANCE placement
		// (shared paint backing), never a second independent load. No new file slot
		// consumed, so the 10-file cap does not apply here.
		printf("'%s' already open - placing a shared-paint instance @ (%d,%d)\n",
		       ze.Basename.c_str(), cx, cy);
		return scratchPlaceInstanceOf(cx, cy, ze.Basename, err);
	}
	// Per-file zone-id base is index*1000 and instance clone ids start at
	// kInstanceZoneIdBase (10000): file index 10 would alias the instance id space.
	// Checked AFTER dup detection so a re-open of an already-loaded file (which
	// becomes a shared-paint instance) does not spuriously trip the cap.
	if (g_EditableFiles.size() >= 10)
	{
		err = "board editable limit reached (10 files; zone-id space)";
		return false;
	}
	PMAXLOAD::SLoadedMax *extra = new PMAXLOAD::SLoadedMax();
	if (!PMAXLOAD::loadMaxFile(ze.MaxPath, *extra))
	{
		delete extra;
		err = "cannot load " + ze.MaxPath;
		return false;
	}
	SEditableFileInfo efi;
	efi.Path = ze.MaxPath;
	efi.Basename = ze.Basename;
	efi.Lm = extra;
	efi.Editable = true;
	efi.CellX = cx;
	efi.CellY = cy;
	g_ExtraEditableScenes.push_back(extra);
	g_EditableFiles.push_back(efi);
	std::string rerr;
	bool ok = scratchRebuild(rerr);
	if (ok)
	{
		// Mask known only after rebuild - collision means the open does not fit here.
		// auto-shift to the nearest fitting cell (spiral) instead of refusing -
		// a large brick still loads, the user drags it into place afterwards.
		std::string cerr;
		if (scratchEditableConflicts(g_EditableFiles.size() - 1, cerr))
		{
			SEditableFileInfo &ne = g_EditableFiles.back();
			bool placed = false;
			for (int r = 1; r <= 12 && !placed; ++r)
			for (int dy = -r; dy <= r && !placed; ++dy)
			for (int dx = -r; dx <= r && !placed; ++dx)
			{
				if (std::max(std::abs(dx), std::abs(dy)) != r) continue;
				ne.CellX = cx + dx;
				ne.CellY = cy + dy;
				std::string tmp;
				if (!scratchEditableConflicts(g_EditableFiles.size() - 1, tmp))
					placed = true;
			}
			if (placed)
			{
				fprintf(stderr, "scratch open editable: '%s' does not fit at (%d,%d) - "
				        "auto-shifted to (%d,%d)\n",
				        ze.Basename.c_str(), cx, cy, ne.CellX, ne.CellY);
				ok = scratchRebuild(rerr);
			}
			else
			{
				ok = false;
				rerr = cerr + NLMISC::toString(" at (%d,%d) (no nearby fit)", cx, cy);
			}
		}
	}
	if (!ok)
	{
		g_EditableFiles.pop_back();
		g_ExtraEditableScenes.pop_back();
		std::string err2;
		uint w2 = 0;
		// Free only when the recovery rebuild succeeded - a failed recovery (early
		// writeBack guard) can leave zones pointing into the scene; leak over UAF.
		if (rebuildWorkingSet(err2, w2))
			freeLoadedMax(extra);
		err = rerr;
		return false;
	}
	printf("scratch open editable: '%s' @ (%d,%d) footprint %dx%d\n",
	       ze.Basename.c_str(), cx, cy,
	       g_EditableFiles.back().CellsW, g_EditableFiles.back().CellsH);
	return true;
}

/** convert a placed RO context brick to an editable file at the same cell. */
bool scratchContextToEditableImpl(int cx, int cy, std::string &err);
bool scratchContextToEditable(int cx, int cy, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchContextToEditableImpl(cx, cy, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.makeEditable(%d, %d)", cx, cy));
	return ok;
}
bool scratchContextToEditableImpl(int cx, int cy, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindContext(cx, cy, idx))
	{
		err = "no context at cell";
		return false;
	}
	const SPlaceContextSpec pc = g_PlaceContextSpecs[idx];
	g_PlaceContextSpecs.erase(g_PlaceContextSpecs.begin() + (std::ptrdiff_t)idx);
	if (!scratchOpenEditable(pc.Dx, pc.Dy, pc.Basename, err))
	{
		// Restore the context placement on failure
		g_PlaceContextSpecs.push_back(pc);
		std::string err2;
		scratchRebuild(err2);
		return false;
	}
	return true;
}

bool scratchPlace(int cx, int cy, std::string &err)
{
	// Legacy spelling for "place an instance of the first-opened file". Source name is
	// pinned at assembly (g_LegacyPlaceSourceName), so after that file closes this fails
	// loudly rather than silently retargeting whichever file is now index 0.
	if (!g_LegacyPlaceSourceName.empty())
		return scratchPlaceInstanceOf(cx, cy, g_LegacyPlaceSourceName, err);
	if (g_EditableFiles.empty())
	{
		err = "no open file to instance";
		return false;
	}
	return scratchPlaceInstanceOf(cx, cy, g_EditableFiles[0].Basename, err);
}

/**
 * place an instance of ANY open brick (home or an open editable file) at (cx,cy).
 * Display clones share the source file's carriers (pointer keying), so painting the
 * source repaints every instance live, exactly like home self-instances.
 */
bool scratchPlaceInstanceOfImpl(int cx, int cy, const std::string &basenameIn, std::string &err);
bool scratchPlaceInstanceOf(int cx, int cy, const std::string &basenameIn, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchPlaceInstanceOfImpl(cx, cy, basenameIn, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.placeInstance(%d, %d, %s)", cx, cy, luaQuote(basenameIn).c_str()));
	return ok;
}
bool scratchPlaceInstanceOfImpl(int cx, int cy, const std::string &basenameIn, std::string &err)
{
	size_t idx = 0;
	if (scratchFindPlace(cx, cy, idx)) { err = "cell already occupied"; return false; }
	if (scratchFindContext(cx, cy, idx)) { err = "cell has context"; return false; }
	if (scratchFindEditableAt(cx, cy, idx)) { err = "cell has an open file"; return false; }
	std::string basename = basenameIn;
	std::string::size_type dot = basename.rfind('.');
	if (dot != std::string::npos && NLMISC::toLowerAscii(basename.substr(dot)) == ".max")
		basename = basename.substr(0, dot);
	// every source resolves uniformly over the open files - the first-opened one
	// is not special, and SourceBasename is always stored explicitly.
	SInstancePlace pl(cx, cy, 0, false);
	bool found = false;
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (NLMISC::toLowerAscii(g_EditableFiles[i].Basename)
		    == NLMISC::toLowerAscii(basename))
		{
			pl.SourceBasename = g_EditableFiles[i].Basename;
			found = true;
			break;
		}
	}
	if (!found)
	{
		err = "instance source must be an OPEN brick: " + basename;
		return false;
	}
	std::vector<bool> sm;
	int fw = 1, fh = 1;
	instanceSourceFootprint(pl, sm, fw, fh);
	if (scratchMaskConflictsSrc(sm, fw, fh, cx, cy, 0, false, -1, err))
		return false;
	g_Places.push_back(pl);
	g_InstanceCount = 1 + (uint)g_Places.size();
	if (!scratchRebuild(err))
	{
		g_Places.pop_back();
		g_InstanceCount = 1 + (uint)g_Places.size();
		scratchRecoveryRebuild("place-instance");
		return false;
	}
	return true;
}

/** open-brick count (home + editables) for the instance-source picker gate. */
int scratchOpenFileCount()
{
	return (int)g_EditableFiles.size();
}

/** instance label parts for the board (source short name when not home). */
bool scratchGetInstanceSource(int cx, int cy, std::string &basename)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
	{
		// Callers pass the block ORIGIN, which can be a hole in a template mask
		// Fall back to matching the stored origin coords.
		bool found = false;
		for (size_t i = 0; i < g_Places.size() && !found; ++i)
		{
			if (g_Places[i].CellX == cx && g_Places[i].CellY == cy)
			{
				idx = i;
				found = true;
			}
		}
		if (!found)
			return false;
	}
	basename = g_Places[idx].SourceBasename;
	return true;
}

/** saved-neighbor hint naming board cell (cx,cy), if any (not currently loaded there). */
bool scratchGetHintAt(int cx, int cy, std::string &basename)
{
	for (size_t i = 0; i < g_SessionHintCells.size(); ++i)
	{
		if (g_SessionHintCells[i].CellX != cx || g_SessionHintCells[i].CellY != cy)
			continue;
		basename = g_SessionHintCells[i].Basename;
		return true;
	}
	return false;
}

/** every hinted basename this session (picker priority sort). */
void scratchHintNames(std::vector<std::string> &out)
{
	out.clear();
	for (size_t i = 0; i < g_SessionHintCells.size(); ++i)
	{
		bool dup = false;
		for (size_t d = 0; d < out.size() && !dup; ++d)
			dup = NLMISC::toLowerAscii(out[d]) == NLMISC::toLowerAscii(g_SessionHintCells[i].Basename);
		if (!dup)
			out.push_back(g_SessionHintCells[i].Basename);
	}
}

/**
 * Rotate an instance anchored at its block's min corner (origin stays put, block size
 * transposes); refuse if the new mask would collide with home or another instance.
 * (Was: pivot about the block center with floor(center − newSize/2) - odd-parity blocks
 * like 2x1 drifted a half-cell-floored step per rotation, so CW then CCW landed one cell
 * off and four CWs walked the block diagonally. Center-pivoting an odd-parity block on a
 * whole-cell grid is unrepresentable; the min-corner anchor is exact and reversible.)
 */
bool scratchRotateImpl(int cx, int cy, int delta, std::string &err);
bool scratchRotate(int cx, int cy, int delta, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchRotateImpl(cx, cy, delta, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.rotateInstance(%d, %d, %d)", cx, cy, delta));
	return ok;
}
bool scratchRotateImpl(int cx, int cy, int delta, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
	{
		err = "no instance at cell";
		return false;
	}
	SInstancePlace pl = g_Places[idx];
	std::vector<bool> sm;
	int fw = 1, fh = 1;
	instanceSourceFootprint(pl, sm, fw, fh);
	const uint newRot = (uint)((int)pl.Rot + delta) & 3;
	if (scratchMaskConflictsSrc(sm, fw, fh, pl.CellX, pl.CellY, newRot, pl.Mirror, (int)idx, err))
		return false;
	g_Places[idx].Rot = newRot;
	if (!scratchRebuild(err))
	{
		if (idx < g_Places.size())
			g_Places[idx].Rot = pl.Rot;
		scratchRecoveryRebuild("rotate instance");
		return false;
	}
	return true;
}

bool scratchMirrorImpl(int cx, int cy, std::string &err);
bool scratchMirror(int cx, int cy, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchMirrorImpl(cx, cy, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.mirrorInstance(%d, %d)", cx, cy));
	return ok;
}
bool scratchMirrorImpl(int cx, int cy, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
	{
		err = "no instance at cell";
		return false;
	}
	// Mirror alone keeps the axis-aligned cell AABB; only the geometry flips -
	// but an asymmetric MASK still changes occupancy, so recheck collisions .
	{
		SInstancePlace pl = g_Places[idx];
		std::vector<bool> sm;
		int fw = 1, fh = 1;
		instanceSourceFootprint(pl, sm, fw, fh);
		if (scratchMaskConflictsSrc(sm, fw, fh, pl.CellX, pl.CellY, pl.Rot, !pl.Mirror,
		                            (int)idx, err))
			return false;
	}
	g_Places[idx].Mirror = !g_Places[idx].Mirror;
	if (!scratchRebuild(err))
	{
		if (idx < g_Places.size())
			g_Places[idx].Mirror = !g_Places[idx].Mirror;
		scratchRecoveryRebuild("mirror instance");
		return false;
	}
	return true;
}

bool scratchRemoveImpl(int cx, int cy, std::string &err);
bool scratchRemove(int cx, int cy, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = scratchRemoveImpl(cx, cy, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.removeInstance(%d, %d)", cx, cy));
	return ok;
}
bool scratchRemoveImpl(int cx, int cy, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
	{
		err = "no instance at cell";
		return false;
	}
	const SInstancePlace removed = g_Places[idx];
	g_Places.erase(g_Places.begin() + idx);
	g_InstanceCount = 1 + (uint)g_Places.size();
	if (!scratchRebuild(err))
	{
		g_Places.insert(g_Places.begin() + (std::ptrdiff_t)idx, removed);
		g_InstanceCount = 1 + (uint)g_Places.size();
		scratchRecoveryRebuild("remove instance");
		return false;
	}
	return true;
}

bool scratchGetInstance(int cx, int cy, uint &rot, bool &mirror)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
		return false;
	rot = g_Places[idx].Rot;
	mirror = g_Places[idx].Mirror;
	return true;
}

/** Resolve any cell in a block to the place origin (for board labels / popups). */
bool scratchGetInstanceOrigin(int cx, int cy, int &ox, int &oy, uint &rot, bool &mirror)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
		return false;
	ox = g_Places[idx].CellX;
	oy = g_Places[idx].CellY;
	rot = g_Places[idx].Rot;
	mirror = g_Places[idx].Mirror;
	return true;
}
