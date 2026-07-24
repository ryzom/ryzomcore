/**
 * \file session_ops.cpp
 * \brief Session zone open/close/save/toggle (continent working-set).
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * `sessionOpenZone` / `sessionCloseZone` / `sessionSaveZone` / `sessionToggleEditable`
 * with their Impl split (recovery rollback + record-on-success). The eco scratch board
 * and the working-set rebuild machinery live in board_session.cpp.
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

#include "zp_state.h"

// TU-local fwd decls for the wrapper→impl split.
static bool sessionOpenZoneImpl(const std::string &basename, std::string &err);
static bool sessionCloseZoneImpl(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err);
static bool sessionSaveZoneImpl(const std::string &basename, std::string &err);
static bool sessionToggleEditableImpl(const std::string &basename, bool saveFirst,
                                      bool forceDiscard, std::string &err);


bool sessionGetCellState(const std::string &basename, ZPUI::ESessionCellState &out)
{
	out = ZPUI::CellClosed;
	SEditableFileInfo *efi = findEditableByBasename(basename);
	if (efi)
	{
		if (!efi->Editable)
			out = ZPUI::CellOpenReadOnly;
		else if (g_PaintCtx.Core && g_PaintCtx.Core->anyZoneDirty(efi->ZoneIds))
			out = ZPUI::CellDirtyEditable;
		else
			out = ZPUI::CellOpenEditable;
		return true;
	}
	// (Neighbor zones use node names, not file basenames - neighbors resolve through
	// the open set and the loaded-context list below, never by zone-name scan.)
	// Loaded RO context (hint chain / place-context / continent ring)
	for (size_t i = 0; i < g_ContextFiles.size(); ++i)
	{
		if (NLMISC::toLowerAscii(g_ContextFiles[i].Basename)
		    == NLMISC::toLowerAscii(basename))
		{
			out = ZPUI::CellOpenReadOnly;
			return true;
		}
	}
	return true; // closed
}

bool sessionIsDirty(const std::string &basename)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	if (!efi || !efi->Editable || !g_PaintCtx.Core) return false;
	return g_PaintCtx.Core->anyZoneDirty(efi->ZoneIds);
}

bool sessionIsOpen(const std::string &basename)
{
	ZPUI::ESessionCellState st = ZPUI::CellClosed;
	sessionGetCellState(basename, st);
	return st != ZPUI::CellClosed;
}

bool sessionIsEditable(const std::string &basename)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	return efi && efi->Editable;
}


bool sessionOpenZone(const std::string &basename, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = sessionOpenZoneImpl(basename, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.openZone(%s)", luaQuote(basename).c_str()));
	return ok;
}
bool sessionOpenZoneImpl(const std::string &basename, std::string &err)
{
	if (g_StartupWorld.Kind == ZPWS::Ecosystem)
	{
		// Eco opens carry a board cell  - this unplaced open would stack on home.
		err = "ecosystem: open via the board cell (scratchOpenEditable)";
		return false;
	}
	if (findEditableByBasename(basename))
	{
		err = "already open";
		return false;
	}
	// Same cap as the eco path (scratchOpenEditable): per-file zone-id base is
	// index*1000 and file index 10 would alias the instance id space - an 11th
	// continent file would get ids >= kInstanceZoneIdBase and Prop mode would
	// misclassify its zones as instance clones.
	if (g_EditableFiles.size() >= 10)
	{
		err = "board editable limit reached (10 files; zone-id space)";
		return false;
	}
	const ZPWS::SZoneEntry *ze = findWorldZone(basename);
	if (!ze)
	{
		err = "zone not found in world: " + basename;
		return false;
	}
	PMAXLOAD::SLoadedMax *extra = new PMAXLOAD::SLoadedMax();
	if (!PMAXLOAD::loadMaxFile(ze->MaxPath, *extra))
	{
		delete extra;
		err = "cannot load " + ze->MaxPath;
		return false;
	}
	SEditableFileInfo efi;
	efi.Path = ze->MaxPath;
	efi.Basename = ze->Basename;
	efi.Lm = extra;
	efi.Editable = true;
	g_ExtraEditableScenes.push_back(extra);
	g_EditableFiles.push_back(efi);
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds))
	{
		// Rollback + rebuild again so the session is not left half-torn; deep-free the
		// new scene only after the core no longer references it. If the recovery rebuild
		// itself fails (early writeBack guard), zones may still point into the scene -
		// leak it rather than free under live references.
		g_EditableFiles.pop_back();
		g_ExtraEditableScenes.pop_back();
		std::string err2;
		uint w2 = 0;
		if (rebuildWorkingSet(err2, w2))
			freeLoadedMax(extra);
		return false;
	}
	printf("session open: '%s' editable; welds=%u\n", basename.c_str(), welds);
	return true;
}


bool sessionCloseZone(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = sessionCloseZoneImpl(basename, saveFirst, forceDiscard, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.closeZone(%s, %s, %s)", luaQuote(basename).c_str(), saveFirst ? "true" : "false", forceDiscard ? "true" : "false"));
	return ok;
}
bool sessionCloseZoneImpl(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	// Closing a pure neighbor is a no-op (ring is automatic)
	if (!efi)
	{
		err = "not an opened file: " + basename;
		return false;
	}
	// Gate last-editable BEFORE any paint mutation: a refuse after forceDiscard would
	// leave the paint discarded while the file stayed open.
	uint nEditable = 0;
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
		if (g_EditableFiles[i].Editable) ++nEditable;
	if (efi->Editable && nEditable <= 1)
	{
		err = "cannot close the last editable zone";
		return false;
	}
	if (efi->Editable && g_PaintCtx.Core && g_PaintCtx.Core->anyZoneDirty(efi->ZoneIds))
	{
		if (saveFirst)
		{
			if (!sessionSaveOneFile(*efi, err))
				return false;
		}
		else if (!forceDiscard)
		{
			err = "dirty (need save or discard)";
			return false;
		}
		// forceDiscard: do NOT revertZones here. The file is about to leave the
		// working set and its scene is freed on success (paint dies with the free).
		// Reverting before a rebuild that can still fail permanently destroyed paint
		// while rolling the file back open - silent data loss on the failure path.
	}
	// Remove from lists. The parsed scene is deep-freed only AFTER the rebuild re-inits
	// the core - until then the old core/carriers still point into it. Every
	// file's scene is heap-owned and registered uniformly (the first-opened included),
	// so closing ANY file genuinely frees its memory.
	SEditableFileInfo closedCopy = *efi; // rollback copy (efi invalidated by erase)
	// The rebuild prunes instances sourced from the closed file early - snapshot for rollback
	std::vector<SInstancePlace> placesCopy = g_Places;
	PMAXLOAD::SLoadedMax *toFree = efi->Lm;
	const bool sceneWasActive = toFree && g_PaintCtx.Scene == toFree->Scene;
	size_t closedIdx = 0; // original slot - rollback must restore ORDER (zone-id bases)
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (&g_EditableFiles[i] == efi)
		{
			closedIdx = i;
			g_EditableFiles.erase(g_EditableFiles.begin() + (std::ptrdiff_t)i);
			break;
		}
	}
	if (toFree)
	{
		for (size_t i = 0; i < g_ExtraEditableScenes.size(); ++i)
		{
			if (g_ExtraEditableScenes[i] == toFree)
			{
				g_ExtraEditableScenes.erase(g_ExtraEditableScenes.begin() + (std::ptrdiff_t)i);
				break;
			}
		}
	}
	// Repoint the active save context when the closed file's scene was it
	if (sceneWasActive && !g_EditableFiles.empty())
	{
		PIPELINE::MAX::CScene *sc = editableScene(g_EditableFiles[0]);
		if (sc)
			g_PaintCtx.Scene = sc;
		g_PaintCtx.InputPath = g_EditableFiles[0].Path;
	}
	// an eco close falls back to read-only context. Register a
	// context SPEC at the file's board cell so the demoted brick stays board-managed
	// (drag/remove/occupancy) instead of resurrecting invisibly through the hint chain
	// (which specs skip). Continent closes keep the grid-ring behavior.
	bool addedCloseSpec = false;
	if (g_StartupWorld.Kind == ZPWS::Ecosystem
	    && !contextBasenameHasSpec(closedCopy.Basename))
	{
		SPlaceContextSpec pc;
		pc.Dx = closedCopy.CellX;
		pc.Dy = closedCopy.CellY;
		pc.Basename = closedCopy.Basename;
		pc.CellsW = closedCopy.CellsW;
		pc.CellsH = closedCopy.CellsH;
		pc.Mask = closedCopy.Mask;
		g_PlaceContextSpecs.push_back(pc);
		addedCloseSpec = true;
	}
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds))
	{
		// Best-effort rollback: restore the file AT ITS ORIGINAL SLOT (a failed close
		// must not permute file order - zone-id bases and the [0] save identity key on
		// it), undo the save-context repoint, and rebuild again so the session is not
		// left half-torn (zones/landscape/core inconsistent with g_EditableFiles).
		g_EditableFiles.insert(
		    g_EditableFiles.begin()
		        + (std::ptrdiff_t)(closedIdx <= g_EditableFiles.size() ? closedIdx
		                                                               : g_EditableFiles.size()),
		    closedCopy);
		if (toFree) g_ExtraEditableScenes.push_back(toFree);
		if (sceneWasActive && toFree)
		{
			g_PaintCtx.Scene = toFree->Scene;
			g_PaintCtx.InputPath = closedCopy.Path;
		}
		g_Places = placesCopy; // the failed rebuild already pruned this file's instances
		// rebuildWorkingSet erases failed place-context specs mid-flight, so a plain
		// pop_back() would drop whatever survived at the tail rather than our close-spec.
		// Erase by content (basename+cell match, first hit).
		if (addedCloseSpec)
		{
			const std::string lowClose = NLMISC::toLowerAscii(closedCopy.Basename);
			for (size_t si = 0; si < g_PlaceContextSpecs.size(); ++si)
			{
				const SPlaceContextSpec &s = g_PlaceContextSpecs[si];
				if (s.Dx == closedCopy.CellX && s.Dy == closedCopy.CellY
				    && NLMISC::toLowerAscii(s.Basename) == lowClose)
				{
					g_PlaceContextSpecs.erase(g_PlaceContextSpecs.begin()
					                          + (std::ptrdiff_t)si);
					break;
				}
			}
		}
		std::string err2;
		uint w2 = 0;
		rebuildWorkingSet(err2, w2);
		return false;
	}
	if (toFree == g_PrimaryLm)
		g_PrimaryLm = NULL; // fallback pointer must not dangle (legacy Lm-NULL files only)
	freeLoadedMax(toFree);
	printf("session close: '%s'; welds=%u\n", basename.c_str(), welds);
	return true;
}


bool sessionSaveZone(const std::string &basename, std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = sessionSaveZoneImpl(basename, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.saveZone(%s)", luaQuote(basename).c_str()));
	return ok;
}
bool sessionSaveZoneImpl(const std::string &basename, std::string &err)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	if (!efi)
	{
		err = "not open: " + basename;
		return false;
	}
	if (!efi->Editable)
	{
		err = "read-only: " + basename;
		return false;
	}
	return sessionSaveOneFile(*efi, err);
}


bool sessionToggleEditable(const std::string &basename, bool saveFirst, bool forceDiscard,
                                  std::string &err)
{
	SBoardOpScope boardOp;
	const bool ok = sessionToggleEditableImpl(basename, saveFirst, forceDiscard, err);
	if (ok)
		recordBoardOp(NLMISC::toString("painter.toggleZone(%s, %s, %s)",
		                               luaQuote(basename).c_str(),
		                               saveFirst ? "true" : "false",
		                               forceDiscard ? "true" : "false"));
	return ok;
}
bool sessionToggleEditableImpl(const std::string &basename, bool saveFirst,
                                      bool forceDiscard, std::string &err)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	if (!efi)
	{
		// Opening as RO then making editable: open first as editable
		if (!sessionOpenZone(basename, err))
			return false;
		efi = findEditableByBasename(basename);
		if (!efi) { err = "open failed"; return false; }
		// already editable after open
		return true;
	}
	if (efi->Editable)
	{
		// Keep at least one editable - gate BEFORE any paint mutation (same class as close).
		uint nEditable = 0;
		for (size_t i = 0; i < g_EditableFiles.size(); ++i)
			if (g_EditableFiles[i].Editable) ++nEditable;
		if (nEditable <= 1)
		{
			err = "cannot demote the last editable zone";
			return false;
		}
		// → read-only
		if (g_PaintCtx.Core && g_PaintCtx.Core->anyZoneDirty(efi->ZoneIds))
		{
			if (saveFirst)
			{
				if (!sessionSaveOneFile(*efi, err))
					return false;
			}
			else if (!forceDiscard)
			{
				err = "dirty (need save or discard)";
				return false;
			}
			// forceDiscard: do NOT revertZones - demotion rebuild force-freezes the file's
			// zones; a failed rebuild rolls Editable back and must keep the paint.
		}
		efi->Editable = false;
	}
	else
	{
		// → editable (re-registers carriers as unfrozen)
		efi->Editable = true;
	}
	const bool nowEditable = efi->Editable;
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds))
	{
		// A failed rebuild leaves the core referencing the CLEARED zones vector - roll the
		// toggle back and rebuild again so the session stays live (same recovery rule as
		// open/close; without it the next frame's hover walk is a use-after-free).
		efi = findEditableByBasename(basename);
		if (efi) efi->Editable = !nowEditable;
		std::string rerr;
		uint rwelds = 0;
		if (!rebuildWorkingSet(rerr, rwelds))
			fprintf(stderr, "ERROR: toggle recovery rebuild failed: %s\n", rerr.c_str());
		return false;
	}
	printf("session toggle: '%s' now %s; welds=%u\n",
	       basename.c_str(), nowEditable ? "editable" : "read-only", welds);
	return true;
}


