/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
// Standalone zone painter (design doc §14-paint): load a .max directly through pipeline_max,
// assemble the painting landscape the way the in-Max painter's NeL thread did
// (plugin_max/nel_patch_paint paint.cpp myThread), paint tiles against a user-selected tile
// bank (P3b: the tile brush with automatic transitions, rotation, 128/256, undo — see
// paint_core.h), and save the .max back through the proven P1/P2 write path.
//
// Modes:
//   (default)      viewer: UDriver/UScene + CLandscapeModel painting scene (unwrapped to
//                  IDriver/CScene for the landscape assembly), tile bank from --bank,
//                  CEvent3dMouseListener edit3d orbiting the landscape bbox center. LEFT MOUSE
//                  paints the selected tile set, right mouse picks the set under the cursor,
//                  PgUp/PgDn (and 0-9) select the tile set, B toggles 128/256, Ctrl+Z / Ctrl+E
//                  undo/redo, ESC or window close exits (--save writes the result). HUD text
//                  via --font (any .ttf; defaults to a system font when present).
//   --screenshot   same scene setup, render one refined frame, dump the framebuffer to .tga
//                  and exit (the visual gate; combine with --paint-script for before/after).
//   --paint-script headless (or viewer pre-pass) scripted painting: one op per line,
//                  '#' comments — `tile <zone> <patch> <u> <v> <tileSet> [rot]`,
//                  `tile256 ...` (same args), `clear <zone> <patch> <u> <v>`, `clear256 ...`,
//                  `undo`, `redo`, `seed <n>`, `mask <file.tga|none>` (color-brush bitmap
//                  mask, P3e). Ops go through the SAME implementations as the mouse path
//                  (single op layer in paint_core).
//
// P3e additions: color-brush bitmap masks (plugin loadBrush port; shipped set in brushes/,
// CLI --brush-mask, viewer cycle key, script `mask`), the displace area brush (plugin
// PutDisplace recursion; brush sizes 0-2 = depths 0/4/8), keys/vars cfg loading (plugin
// LoadKeyCfg/LoadVarCfg port over --keys-cfg/--vars-cfg or the default cwd names — see the
// --help description for the accepted variable sets), and live hardness/opacity keys.
//   --save         write-back + whole-file save after ops: each (possibly tile-mutated)
//                  pristine carrier blob is encoded into its P2 write-target (topmost 0x4001
//                  snapshot, else base 0x08FD via setRPatch), the Scene stream is rebuilt and
//                  every other stream kept verbatim (OLE class id preserved).
//   --null-edit    the same write-back path with no ops at all: evaluate, resolve carriers,
//                  write back untouched pristine blobs, save to --out. With --verify-identical
//                  every stream must byte-compare against the input (the §14-paint null-edit
//                  property, now THROUGH the paint save path).
//   --dump-zones   headless proof of the eval->weld->build path: write every built CZone
//                  (serial version 4, the reference era) and report patch/bind/border counts.
//   --dump-rpo     dump every carrier's pristine tile records (the mechanical verification
//                  surface for the paint round-trip); runs after --paint-script when given.
//   --dump-bank-xref / --dump-carrier-blob   bank xref table / raw carrier blob bytes, for
//                  the transition-witness and surgical-diff checks.
//
// Scene assembly replicates the painter plugin: per RklPatch node evalNodePatch + object TM
// at t=0 -> buildPatchInfo in authored space (NO symmetry/rotate — the painting scene shows
// what the artist authored; zoneId = node collection index like the plugin's vectMesh index)
// -> optional ecosystem self-instances (ui M4a/M12/M14a: --place dx,dy[,rot][,m]; --instances
// NxM is a deprecated translation-only alias). Display clones with rot/mirror about the
// FOOTPRINT block center (origin snapped to cell grid + half step); place (dx,dy) is the
// min-corner cell of the transformed block. Same Node pointers so paint_core shares one
// carrier; ids from 10000. Per-zone Rotate/Symmetry feed transformDesc (plugin GetTile/
// SetTile). -> cross-zone open-edge weld (paint.cpp WELD_THRESOLD port, session-only) ->
// CZone::build -> CZoneCornerSmoother -> Landscape.addZone. Frozen nodes (0x0976) are
// boundary-reference display: landscape + weld + metaTile graph, never paint targets, never
// rewritten.
//
// ui M4c / M12b: self-instance weld creates self-adjacency in the metaTile graph. Transition
// paint adjacent to that seam propagates across it through ordinary PropagateBorder (visited
// is SPaintTile*). Interior ops through an R-rotated instance are byte-identical to the
// compensated primary op (tile/256/fill store rot (r+R)&3; color/displace identity on UV).
// Cross-seam (M4c methodology from the primary) on lacustre/material-fond?place=1,0,R:
// R0 16 writes / R1 11 / R2 11 / R3 16; checkseams 0 illegal on zones 0 and 10000; <1s.
// Color closure across rotated seams: 2 slot writes. paint+undo through instance == null-edit.

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

// MSVC 9.0 (VS2008) has no C99 snprintf; _snprintf is equivalent for our fixed-size formatting.
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
#include <nel/3d/tile_bank.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/viewport.h>
#include <nel/3d/zone.h>
#include <nel/3d/zone_corner_smoother.h>
#include <nel/3d/zone_symmetrisation.h>

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


// Patch-state eval + RPO->CPatchInfo conversion: the shared unit of the zone exporter and this
// painter. Header-only static implementation unit (zone x87 tier is TU-sensitive; see the
// header doc for the include contract this file follows).
#include "../pipeline_max_export_common/patch_eval.h"

// The tile painting core (P3b): metaTile graph, transition solver, pristine carrier state,
// live-landscape mirror, undo, write-back.
#include "paint_core.h"

// Include-meshes context display + scene lights (P3d; own TU — the SCENELIB evaluation
// headers must not share a TU with the patch_eval implementation unit above).
#include "context_display.h"

// In-engine NLGUI shell (own TU — must not pull patch_eval / SCENELIB into editor_ui.cpp).
#include "editor_ui.h"
#include <nel/gui/ctrl_base.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/ctrl_text_button.h>
#include <nel/gui/widget_manager.h>

// Workspace fingerprint / discovery / startup.cfg (own TU — NLMISC only).
#include "workspace_discovery.h"

// Startup screens A/B/C (own TU — NLGUI + discovery; no patch_eval / SCENELIB).
#include "startup_ui.h"

static bool g_verbose = false;
// Result of the viewer script pre-pass (propagated as the viewer exit code for scripted gates)
static int g_ViewerScriptRc = 0;
// Viewer paint defaults (CLI-configurable)
static NLMISC::CRGBA g_ViewerBrushColor(255, 255, 255, 255);
static float g_ViewerBrushRadius = 8.f;
static uint g_ViewerBrushHardness = 128;
static uint g_ViewerBrushOpacity = 255;
static uint g_ViewerDisplaceIndex = 0;
static bool g_PreloadTiles = false;
static bool g_IncludeMeshes = false;
static std::string g_InputPath;
static std::string g_BankPath;
// Extra texture search path derived by the startup flow (empty on the legacy CLI path)
static std::string g_StartupTexturePath;
// When true, bank directory is searched recursively (startup flow always wants this)
static bool g_ForceBankRecursive = false;
// Startup interactive (or startup-auto) without --save: panel Save opens the modal
static bool g_InteractiveSave = false;
// Continent/ecosystem neighbor loading (M3b/M16): default on for startup/auto, off for legacy
static bool g_LoadNeighbors = false;
// Board-driven session (startup interactive / --startup-auto). Not the legacy direct-.max path.
static bool g_BoardSession = false;
// Escape hatch: board sessions display embedded non-eligible/frozen copies (M16a default: skip).
static bool g_EmbeddedContext = false;
// World/zone selected via startup (for neighbor discovery); empty on legacy path
static ZPWS::SWorldEntry g_StartupWorld;
static ZPWS::SZoneEntry g_StartupZone;
// Multi-select editable set (M6b); empty means single g_StartupZone only
static std::vector<ZPWS::SZoneEntry> g_StartupEditableZones;
// RO context files loaded for the session (continent ring / eco hints / place-context).
// Owns SLoadedMax; cleared on rebuild. Cell offsets are relative to primary footprint origin.
struct SContextFile
{
	std::string Path;
	std::string Basename;
	int CellX, CellY;
	bool TranslateGeom; // ecosystem: shift geometry to cell; continent natural coords: false
	PMAXLOAD::SLoadedMax *Lm;
	SContextFile() : CellX(0), CellY(0), TranslateGeom(false), Lm(NULL) {}
};
static std::vector<SContextFile> g_ContextFiles;
// Legacy name used throughout; points at scenes inside g_ContextFiles
static std::vector<PMAXLOAD::SLoadedMax *> g_NeighborScenes;
// Extra editable .max scenes beyond the primary stack `lm` (M6b multi-open)
static std::vector<PMAXLOAD::SLoadedMax *> g_ExtraEditableScenes;
// Per-editable-file zone-id membership for dirty tracking / per-file save (M6b)
struct SEditableFileInfo
{
	std::string Path;
	std::string Basename;
	PMAXLOAD::SLoadedMax *Lm; // NULL = primary stack lm
	std::vector<uint> ZoneIds;
	/** M11a: when false the file is open as read-only context (no paint; like neighbor). */
	bool Editable;
	SEditableFileInfo() : Lm(NULL), Editable(true) {}
};
static std::vector<SEditableFileInfo> g_EditableFiles;
// M11a: primary stack scene pointer (set once after load; used by session rebuild)
static PMAXLOAD::SLoadedMax *g_PrimaryLm = NULL;
// M11b: when true, every non-forceFrozen zone is a paint target (today's open-everything).
// Default false = exporter-faithful eligibility.
static bool g_AllZones = false;
// Session rebuild live pointers (valid only while runViewer runs)
static float g_SessionCellSize = 160.f;
static float g_SessionSnap = 1.f;
static bool g_SessionLockBorders = false;
static NL3D::CTileBank *g_SessionBank = NULL;
// Ecosystem self-instances (ui M4a/M12/M14a): board-driven placements. Primary zones sit at
// the layout origin (rot 0, no mirror); each --place adds a display-level duplicate sharing
// the same Node pointers (paint_core carrier keying) with geometry transformed about the
// FOOTPRINT-block center and translated so the transformed block's min-corner lands at the
// place origin cell. --instances NxM is a deprecated alias that expands to translation-only
// placements for the non-origin cells of a grid of whole footprints.
//
// Place convention (M14a): CellX,CellY = origin (min-corner) of the TRANSFORMED footprint
// block in fine cells of --cellsize, relative to the primary footprint origin cell (0,0).
// Primary home occupies [0,fw)×[0,fh). A rot-0 instance at (fw,0) sits immediately east of
// home (adjacent blocks). Rot 1/3 transpose occupancy to fh×fw. Overlapping blocks refused.
// Instance zone ids use the sparse base kInstanceZoneIdBase (CBorderVertex ids are uint16).
static const uint kInstanceZoneIdBase = 10000;
struct SInstancePlace
{
	int CellX, CellY; // origin cell of transformed footprint block (fine cells; see above)
	uint Rot;         // 0..3 CCW (NEL3D_APPDATA_ZONE_ROTATE / CZoneRegion::Rot)
	bool Mirror;      // Flip (NEL3D_APPDATA_ZONE_SYMMETRY / CZoneRegion::Flip)
	SInstancePlace() : CellX(0), CellY(0), Rot(0), Mirror(false) { }
	SInstancePlace(int x, int y, uint r, bool m) : CellX(x), CellY(y), Rot(r), Mirror(m) { }
};
static std::vector<SInstancePlace> g_Places; // empty = primary only
// Primary footprint in fine cells (set by appendInstanceZones / computeFootprintRect)
static int g_FootprintCellsW = 1;
static int g_FootprintCellsH = 1;
// Legacy NxN reporting (deprecated --instances / Screen B layout)
static uint g_InstanceCols = 1;
static uint g_InstanceRows = 1;
static uint g_InstanceCount = 1; // 1 + g_Places.size()
// Shipped brush mask cycle (viewer SelectColorBrush key; 0 = none, i = g_MaskFiles[i-1])
static std::vector<std::string> g_MaskFiles;
static int g_MaskCycle = 0;

// ---------------------------------------------------------------------------------------------
// keys.cfg / vars.cfg (plugin paint_ui.cpp LoadKeyCfg/LoadVarCfg port). The plugin read BOTH
// variable sets from one keys.cfg next to the plugin dll (NLMISC::CConfigFile: the file itself
// defines the Key* constants, then `Action = KeyX;` assignments — the original keys.cfg parses
// verbatim). Here: --keys-cfg / --vars-cfg CLI paths, else zone_painter_keys.cfg /
// zone_painter_vars.cfg in the cwd, else the hardcoded defaults below (identical to the
// pre-cfg tool). One file may serve both loaders, exactly like the plugin.
//
// Key actions keep the plugin's cfg NAMES (a plugin-era keys.cfg rebinds them unchanged);
// values are NeL TKey codes (== the Windows VK codes the plugin used). Actions without a
// standalone-tool equivalent are accepted and ignored (documented in --help): Select, Pick,
// ToggleColor, BackgroundColor, ToggleArrows, Zouille, AutomaticLighting, GetState,
// ResetPatch. ZoomIn/ZoomOut are implemented but default UNBOUND (0) because the plugin's
// default keys 1/2 select tile sets in this tool.

enum TPainterKey
{
	ZPK_Select = 0,
	ZPK_Pick,
	ZPK_Fill0,
	ZPK_Fill1,
	ZPK_Fill2,
	ZPK_Fill3,
	ZPK_MModeTile,
	ZPK_MModeColor,
	ZPK_MModeDisplace,
	ZPK_ToggleColor,
	ZPK_SizeUp,
	ZPK_SizeDown,
	ZPK_ToggleTileSize,
	ZPK_GroupUp,
	ZPK_GroupDown,
	ZPK_BackgroundColor,
	ZPK_ToggleArrows,
	ZPK_HardnessUp,
	ZPK_HardnessDown,
	ZPK_OpacityUp,
	ZPK_OpacityDown,
	ZPK_Zouille,
	ZPK_AutomaticLighting,
	ZPK_SelectColorBrush,
	ZPK_ToggleColorBrushMode,
	ZPK_LockBorders,
	ZPK_ZoomIn,
	ZPK_ZoomOut,
	ZPK_GetState,
	ZPK_ResetPatch,
	ZPK_ToggleUI,
	ZPK_SeasonNext, // ui M6a: cycle landscape season textures
	ZPK_TogglePalette, // ui M8: show/hide tileset thumbnail palette
	ZPK_ToggleBoard, // ui M11a: session board hub over live viewer
	ZPK_KeyCounter
};

// The plugin's cfg variable names (paint_ui.cpp PainterKeysName, order preserved)
static const char *kPainterKeysName[ZPK_KeyCounter] =
{
	"Select",
	"Pick",
	"Fill0",
	"Fill1",
	"Fill2",
	"Fill3",
	"ModeTile",
	"ModeColor",
	"ModeDisplace",
	"ToggleColor",
	"SizeUp",
	"SizeDown",
	"ToggleTileSize",
	"GroupUp",
	"GroupDown",
	"BackgroundColor",
	"ToggleArrows",
	"HardnessUp",
	"HardnessDown",
	"OpacityUp",
	"OpacityDown",
	"Zouille",
	"AutomaticLighting",
	"SelectColorBrush",
	"ToggleColorBrushMode",
	"LockBorders",
	"ZoomIn",
	"ZoomOut",
	"GetState",
	"ResetPatch",
	"ToggleUI",
	"SeasonNext",
	"TogglePalette",
	"ToggleBoard",
};

// Tool defaults: the pre-cfg hardcoded viewer keys stay on their keys (T/C/D, +/-, B, G, F);
// new actions land on free keys (documented in --help). 0 = unbound.
static uint g_PainterKeys[ZPK_KeyCounter] =
{
	0,                    // Select (in-plugin paint-modifier action; no tool equivalent)
	0,                    // Pick (the tool picks on right mouse, hardcoded)
	NLMISC::KeyF,         // Fill0 (the pre-cfg F fill, rotation 0)
	NLMISC::KeyF6,        // Fill1 (plugin default)
	NLMISC::KeyF7,        // Fill2 (plugin default)
	NLMISC::KeyF8,        // Fill3 (plugin default)
	NLMISC::KeyT,         // ModeTile
	NLMISC::KeyC,         // ModeColor
	NLMISC::KeyD,         // ModeDisplace
	0,                    // ToggleColor (single brush color in this tool)
	NLMISC::KeyADD,       // SizeUp
	NLMISC::KeySUBTRACT,  // SizeDown
	NLMISC::KeyB,         // ToggleTileSize
	NLMISC::KeyG,         // GroupUp
	NLMISC::KeyV,         // GroupDown (plugin default)
	0,                    // BackgroundColor
	0,                    // ToggleArrows
	NLMISC::KeyHOME,      // HardnessUp (plugin PgUp/PgDn select tile sets here)
	NLMISC::KeyEND,       // HardnessDown
	NLMISC::KeyINSERT,    // OpacityUp
	NLMISC::KeyDELETE,    // OpacityDown
	0,                    // Zouille
	0,                    // AutomaticLighting
	NLMISC::KeyS,         // SelectColorBrush (cycles the shipped mask set; plugin default key)
	NLMISC::KeyQ,         // ToggleColorBrushMode (plugin default)
	NLMISC::KeyL,         // LockBorders (plugin default)
	0,                    // ZoomIn (bindable; plugin default Key1 selects a tile set here)
	0,                    // ZoomOut
	0,                    // GetState
	0,                    // ResetPatch
	NLMISC::KeyF10,       // ToggleUI (NLGUI panel visibility)
	NLMISC::KeyY,         // SeasonNext (ui M6a; free key — cycle season textures)
	NLMISC::KeyP,         // TogglePalette (ui M8; free key — tileset thumbnail palette)
	NLMISC::KeyO,         // ToggleBoard (ui M11a; free key — session board hub)
};

// paint_ui.cpp light/zoom variable defaults (LoadVarCfg overrides; identical to the previous
// hardcoded painting-scene constants)
static NLMISC::CVector g_LightDirection(1.f, 1.f, -1.f);
static NLMISC::CRGBA g_LightDiffuse(255, 255, 255);
static NLMISC::CRGBA g_LightAmbiant(0, 0, 0);
static float g_LightMultiply = 1.f;
static float g_ZoomSpeed = 300.f;

// LoadKeyCfg port: per-action lookup, absent/typed-wrong names silently keep the default (the
// plugin's per-var try/catch). `required` = the path came from the CLI (missing file is fatal);
// the default-named cwd file is optional. Parse errors are always fatal.
static bool loadKeysCfg(const std::string &path, bool required)
{
	if (!NLMISC::CFile::fileExists(path))
	{
		if (required) { fprintf(stderr, "ERROR: keys cfg not found: %s\n", path.c_str()); return false; }
		return true;
	}
	NLMISC::CConfigFile cf;
	try
	{
		cf.load(path);
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: keys cfg %s: %s\n", path.c_str(), e.what());
		return false;
	}
	uint loaded = 0;
	for (uint key = 0; key < ZPK_KeyCounter; ++key)
	{
		try
		{
			NLMISC::CConfigFile::CVar &value = cf.getVar(kPainterKeysName[key]);
			g_PainterKeys[key] = (uint)value.asInt();
			++loaded;
		}
		catch (const NLMISC::EConfigFile &)
		{
			// keep the default (plugin behavior)
		}
	}
	printf("keys cfg %s: %u binding(s) applied\n", path.c_str(), loaded);
	return true;
}

// LoadVarCfg port: the exact plugin variable set — LightDirection (3 floats), LightDiffuse /
// LightAmbiant (3 ints), LightMultiply (float), ZoomSpeed (float). Same per-var tolerance.
static bool loadVarsCfg(const std::string &path, bool required)
{
	if (!NLMISC::CFile::fileExists(path))
	{
		if (required) { fprintf(stderr, "ERROR: vars cfg not found: %s\n", path.c_str()); return false; }
		return true;
	}
	NLMISC::CConfigFile cf;
	try
	{
		cf.load(path);
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: vars cfg %s: %s\n", path.c_str(), e.what());
		return false;
	}
	uint loaded = 0;
	try
	{
		NLMISC::CConfigFile::CVar &lightDirection = cf.getVar("LightDirection");
		if (lightDirection.size() == 3)
		{
			g_LightDirection.x = lightDirection.asFloat(0);
			g_LightDirection.y = lightDirection.asFloat(1);
			g_LightDirection.z = lightDirection.asFloat(2);
			++loaded;
		}
	}
	catch (const NLMISC::EConfigFile &)
	{
	}
	try
	{
		NLMISC::CConfigFile::CVar &lightDiffuse = cf.getVar("LightDiffuse");
		if (lightDiffuse.size() == 3)
		{
			g_LightDiffuse.R = (uint8)lightDiffuse.asInt(0);
			g_LightDiffuse.G = (uint8)lightDiffuse.asInt(1);
			g_LightDiffuse.B = (uint8)lightDiffuse.asInt(2);
			++loaded;
		}
	}
	catch (const NLMISC::EConfigFile &)
	{
	}
	try
	{
		NLMISC::CConfigFile::CVar &lightAmbiant = cf.getVar("LightAmbiant");
		if (lightAmbiant.size() == 3)
		{
			g_LightAmbiant.R = (uint8)lightAmbiant.asInt(0);
			g_LightAmbiant.G = (uint8)lightAmbiant.asInt(1);
			g_LightAmbiant.B = (uint8)lightAmbiant.asInt(2);
			++loaded;
		}
	}
	catch (const NLMISC::EConfigFile &)
	{
	}
	try
	{
		NLMISC::CConfigFile::CVar &lightMultiply = cf.getVar("LightMultiply");
		g_LightMultiply = lightMultiply.asFloat();
		++loaded;
	}
	catch (const NLMISC::EConfigFile &)
	{
	}
	try
	{
		NLMISC::CConfigFile::CVar &zoomSpeed = cf.getVar("ZoomSpeed");
		g_ZoomSpeed = zoomSpeed.asFloat();
		++loaded;
	}
	catch (const NLMISC::EConfigFile &)
	{
	}
	printf("vars cfg %s: %u variable(s) applied (light %u,%u,%u dir %.3f,%.3f,%.3f mul %.2f zoom %.1f)\n",
	       path.c_str(), loaded, g_LightDiffuse.R, g_LightDiffuse.G, g_LightDiffuse.B,
	       g_LightDirection.x, g_LightDirection.y, g_LightDirection.z, g_LightMultiply, g_ZoomSpeed);
	return true;
}

// Bound-key test helpers (0 = unbound). g_ViewerAsync is the viewer's UDriver AsyncListener
// (set for the duration of runViewer; headless modes never touch it).
static NLMISC::CEventListenerAsync *g_ViewerAsync = NULL;

static bool zpKeyPushed(TPainterKey action)
{
	uint k = g_PainterKeys[action];
	return k != 0 && g_ViewerAsync && g_ViewerAsync->isKeyPushed((NLMISC::TKey)k);
}

static bool zpKeyDown(TPainterKey action)
{
	uint k = g_PainterKeys[action];
	return k != 0 && g_ViewerAsync && g_ViewerAsync->isKeyDown((NLMISC::TKey)k);
}

// ---------------------------------------------------------------------------------------------
// Zone writing (--dump-zones): current CZone::serial writes version 5; the references are
// version 4, and the two encodings differ only in the version byte (verified by reference
// roundtrip in the zone exporter). Serialize to memory and write the version byte as 4.

static bool writeZoneV4(NL3D::CZone &zone, const std::string &path)
{
	NLMISC::CMemStream mem;
	nlassert(!mem.isReading());
	zone.serial(mem);
	if (mem.length() < 1) return false;
	std::vector<uint8> buf(mem.buffer(), mem.buffer() + mem.length());
	if (buf[0] != 5)
	{
		fprintf(stderr, "WARNING: unexpected CZone serial version %u, version byte left untouched\n", buf[0]);
	}
	else
	{
		buf[0] = 4;
	}
	NLMISC::COFile f;
	if (!f.open(path)) return false;
	f.serialBuffer(nlVectorData(buf), (uint)buf.size());
	return true;
}

// ---------------------------------------------------------------------------------------------
// The painting scene zones: every RklPatch node, evaluated in authored space, zoneId = node
// collection index (the plugin used its vectMesh index the same way).

struct SPaintZone
{
	CNodeImpl *Node;
	bool Frozen;
	std::string Name;
	uint ZoneId;
	std::vector<NL3D::CPatchInfo> Patches;
	std::vector<NL3D::CBorderVertex> BorderVertices; // session-only, filled by the weld pass
	SEvalPatch Ep; // evaluated topology, kept for the paint core's metaTile graph (P3b)
	// Display transform (M12): primary = (0,false); instances carry placement rot/mirror
	uint Rotate;
	bool Symmetry;
	SPaintZone() : Node(NULL), Frozen(false), ZoneId(0), Rotate(0), Symmetry(false) { }
};

// Exporter-faithful zone eligibility (M11b).
//
// Mirrors pipeline_max_export_zone + ligo/zone maxscript selection:
//   - collectZoneNodes skips [NELLIGO] debug markers (shared patch_eval rule).
//   - zonematerial- / zonespecial-: ligo selectAllPatch = all non-frozen (0x0976);
//     export requires exactly one. If multiple non-frozen, prefer node name matching the
//     cell token (case-insensitive); otherwise first non-frozen is eligible, rest RO.
//   - zonetransition-: all non-frozen (transition scheme grid classification at export).
//   - otherwise (direct ExportRykolZone / continent .max): first RklPatch that is not
//     DONOTEXPORT and has a findID-parseable name (exportDirectZone loop); rest RO.
//
// File-frozen (0x0976) always remain frozen. --all-zones restores pre-M11b behavior:
// every non-forceFrozen zone is eligible (only 0x0976 / neighbor forceFrozen stay RO).
// Eligibility only affects paint targets; writeBack still covers every unfrozen carrier
// of editable files; null-edit round-trips the whole file.
//
// Returns a same-length bool vector: true = eligible paint target when not forceFrozen.
static void computeZoneEligibility(const std::vector<SZoneNode> &nodes,
                                   const std::string &fileBasename,
                                   std::vector<bool> &eligible)
{
	eligible.assign(nodes.size(), false);
	if (nodes.empty())
		return;
	if (g_AllZones)
	{
		for (size_t i = 0; i < nodes.size(); ++i)
			eligible[i] = !nodes[i].Frozen; // still respect 0x0976 under --all-zones? 
		// Today open-everything = non-0x0976 only. Match that.
		return;
	}

	// Tokenize basename on '-'
	std::vector<std::string> tokens;
	{
		std::string::size_type pos = 0;
		const std::string &s = fileBasename;
		while (pos <= s.size())
		{
			std::string::size_type dash = s.find('-', pos);
			if (dash == std::string::npos) { tokens.push_back(s.substr(pos)); break; }
			tokens.push_back(s.substr(pos, dash - pos));
			pos = dash + 1;
		}
	}
	const std::string t0 = tokens.empty() ? std::string() : NLMISC::toLowerAscii(tokens[0]);

	// Gather non-frozen indices
	std::vector<size_t> nonFrozen;
	for (size_t i = 0; i < nodes.size(); ++i)
		if (!nodes[i].Frozen)
			nonFrozen.push_back(i);

	if (t0 == "zonematerial" || t0 == "zonespecial")
	{
		// Ligo single-brick: one non-frozen. Prefer name match to cell token.
		std::string cell;
		if (t0 == "zonematerial" && tokens.size() >= 3)
			cell = tokens.back(); // e.g. 200_dz
		else if (t0 == "zonespecial" && tokens.size() >= 2)
			cell = tokens[1];
		int match = -1;
		if (!cell.empty())
		{
			const std::string cellL = NLMISC::toLowerAscii(cell);
			for (size_t k = 0; k < nonFrozen.size(); ++k)
			{
				std::string n = NLMISC::toLowerAscii(
					ucstring(nodes[nonFrozen[k]].Node->userName()).toUtf8());
				if (n == cellL || n.find(cellL) != std::string::npos)
				{
					match = (int)nonFrozen[k];
					break;
				}
			}
		}
		if (match >= 0)
			eligible[(size_t)match] = true;
		else if (!nonFrozen.empty())
			eligible[nonFrozen[0]] = true;
		for (size_t k = 0; k < nonFrozen.size(); ++k)
			if (!eligible[nonFrozen[k]])
				printf("eligibility: '%s' node '%s' non-export → read-only context\n",
				       fileBasename.c_str(),
				       ucstring(nodes[nonFrozen[k]].Node->userName()).toUtf8().c_str());
		return;
	}
	if (t0 == "zonetransition")
	{
		// All non-frozen (exporter classifies on transition grid)
		for (size_t k = 0; k < nonFrozen.size(); ++k)
			eligible[nonFrozen[k]] = true;
		return;
	}

	// Direct zone process (ExportRykolZone): first non-DONOTEXPORT with findID-able name.
	// findID lives in export_zone; replicate the name parse (NUM_AB...).
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].Frozen)
			continue;
		// DONOTEXPORT appdata
		{
			// local reader — appdata on node
			std::string s;
			// getScriptAppDataInt is in export tools; use APPDATA via patch_eval's scene only.
			// DONOTEXPORT rare on zone sources; skip if we cannot read (treat as exportable).
		}
		std::string name = ucstring(nodes[i].Node->userName()).toUtf8();
		// findID: parts[0]=num, parts[1]=two letters
		std::vector<std::string> parts;
		std::string::size_type pos = 0;
		while (pos <= name.size())
		{
			std::string::size_type us = name.find('_', pos);
			if (us == std::string::npos) { parts.push_back(name.substr(pos)); break; }
			parts.push_back(name.substr(pos, us - pos));
			pos = us + 1;
		}
		bool idOk = false;
		if (parts.size() >= 2 && parts[1].size() >= 2)
		{
			char l1 = parts[1][0], l2 = parts[1][1];
			if (l1 >= 'A' && l1 <= 'Z' && l2 >= 'A' && l2 <= 'Z')
			{
				int num = 0;
				if (sscanf(parts[0].c_str(), "%d", &num) == 1)
					idOk = true;
			}
			// also accept lowercase letters
			if (!idOk && parts[1].size() >= 2)
			{
				char l1 = (char)toupper(parts[1][0]), l2 = (char)toupper(parts[1][1]);
				if (l1 >= 'A' && l1 <= 'Z' && l2 >= 'A' && l2 <= 'Z')
				{
					int num = 0;
					if (sscanf(parts[0].c_str(), "%d", &num) == 1)
						idOk = true;
				}
			}
		}
		if (idOk)
		{
			eligible[i] = true;
			// Only the FIRST (export writes one .zone and exits)
			for (size_t j = i + 1; j < nodes.size(); ++j)
			{
				if (nodes[j].Frozen) continue;
				printf("eligibility: '%s' node '%s' not first exportable → read-only context\n",
				       fileBasename.c_str(),
				       ucstring(nodes[j].Node->userName()).toUtf8().c_str());
			}
			return;
		}
	}
	// Fallback: first non-frozen (ligo-like) when no findID match
	if (!nonFrozen.empty())
		eligible[nonFrozen[0]] = true;
}

/** True when board sessions should omit embedded non-eligible/frozen display copies (M16a). */
static bool boardSkipEmbedded()
{
	return g_BoardSession && !g_EmbeddedContext;
}

/**
 * Append paint zones from one Max scene.
 * zoneIdOffset: first zone id for this file (must not collide with existing zones).
 * forceFrozen: true for neighbor/context files (landscape + weld + metaTile, never paint,
 *   carriers never rewritten because AnyUnfrozen stays false).
 * fileBasename: used for exporter-faithful eligibility (M11b).
 *
 * M16a board authority: when boardSkipEmbedded(), non-eligible and 0x0976 nodes are NOT
 * displayed (logged and skipped). Neighbor files under the same rule only surface their
 * eligible zone(s) as RO. Legacy direct-.max and --embedded-context keep pre-M16 display
 * of embedded RO copies. Save still round-trips every carrier byte-faithfully (load-time
 * display filter only — nodes remain in the scene graph).
 */
static bool buildPaintZones(CScene &scene, std::vector<SPaintZone> &zones,
                            uint zoneIdOffset, bool forceFrozen,
                            const std::string &fileBasename = std::string())
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	std::vector<bool> eligible;
	// Always compute eligibility when we may skip or when not force-frozen.
	// Neighbor files under board authority need eligibility to pick the real brick zone.
	if (forceFrozen && !boardSkipEmbedded())
		eligible.assign(nodes.size(), false); // legacy neighbor: all RO display
	else
		computeZoneEligibility(nodes, fileBasename, eligible);

	const bool skipEmb = boardSkipEmbedded();
	SNodeTMCache tmCache;
	bool any = false;
	uint nEligible = 0, nRo = 0, nSkipped = 0;
	std::string skippedNames;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		CNodeImpl *node = nodes[i].Node;
		std::string name = ucstring(node->userName()).toUtf8();
		const bool fileFrozen = nodes[i].Frozen;
		const bool isEligible = (i < eligible.size() && eligible[i]);

		// Board authority: never display embedded 0x0976 or non-eligible nodes.
		// Neighbor load (forceFrozen) under board: only eligible zone(s) as RO.
		if (skipEmb)
		{
			if (fileFrozen || !isEligible)
			{
				if (nSkipped++) skippedNames += ", ";
				skippedNames += name;
				if (fileFrozen) skippedNames += "[0x0976]";
				else skippedNames += "[ineligible]";
				continue;
			}
		}

		SEvalPatch ep;
		std::string err;
		if (!evalNodePatch(node, ep, err))
		{
			// The plugin showed a message box and kept going; keep the surviving zones displayed.
			fprintf(stderr, "WARNING: node '%s': %s (zone skipped)\n", name.c_str(), err.c_str());
			continue;
		}
		Matrix3M objectTM = getObjectTM(node, tmCache);
		SPaintZone pz;
		pz.Node = node;
		// Neighbor files / non-eligible / 0x0976 → read-only context (when displayed).
		const bool ineligible = !forceFrozen && !isEligible;
		pz.Frozen = forceFrozen || fileFrozen || ineligible;
		pz.Name = name;
		pz.ZoneId = zoneIdOffset + (uint)i;
		if (!buildPatchInfo(ep, objectTM, (int)pz.ZoneId, pz.Patches, err))
		{
			fprintf(stderr, "WARNING: node '%s': %s (zone skipped)\n", name.c_str(), err.c_str());
			continue;
		}
		pz.Ep = ep;
		zones.push_back(pz);
		any = true;
		if (pz.Frozen) ++nRo; else ++nEligible;
		if (g_verbose)
			printf("zone %u '%s'%s: %u patches\n", pz.ZoneId, pz.Name.c_str(),
			       pz.Frozen ? " FROZEN" : "", (uint)pz.Patches.size());
	}
	if (skipEmb && nSkipped)
		printf("board authority '%s': skipped %u embedded/non-eligible zone(s): %s\n",
		       fileBasename.c_str(), nSkipped, skippedNames.c_str());
	if (!forceFrozen && any)
		printf("eligibility '%s': %u editable, %u read-only context (all-zones=%s board-skip=%s)\n",
		       fileBasename.c_str(), nEligible, nRo, g_AllZones ? "on" : "off",
		       skipEmb ? "on" : "off");
	return any;
}

/** Next free zone id base after the highest already-assigned id (leave a gap of 1). */
static uint nextZoneIdBase(const std::vector<SPaintZone> &zones)
{
	uint maxId = 0;
	bool any = false;
	for (size_t i = 0; i < zones.size(); ++i)
	{
		if (!any || zones[i].ZoneId > maxId)
		{
			maxId = zones[i].ZoneId;
			any = true;
		}
	}
	return any ? (maxId + 1) : 0;
}

// Forward: defined with footprint helpers below (used by M16 neighbor load).
static void computeFootprintRect(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                                 size_t primaryEnd, float cellSize,
                                 float &originX, float &originY,
                                 float &stepX, float &stepY,
                                 int &cellsW, int &cellsH);

// ---------------------------------------------------------------------------------------------
// Neighbor hints (M16a/b) — versioned appdata on the eligible node + embedded-copy fallback.
//
// Format: single string value  v1|dx,dy:basename|dx,dy:basename|...
//   dx,dy  = integer cell offsets relative to the eligible zone's footprint origin
//   basename = file basename without .max
// Unknown future versions: parser ignores the entry (tolerates, does not fail open).

struct SNeighborHint
{
	int Dx, Dy;
	std::string Basename;
	SNeighborHint() : Dx(0), Dy(0) {}
	SNeighborHint(int dx, int dy, const std::string &b) : Dx(dx), Dy(dy), Basename(b) {}
};

/** Parse appdata payload; returns false if absent/empty/unknown version. */
static bool parseNeighborHintsString(const std::string &raw, std::vector<SNeighborHint> &out)
{
	out.clear();
	if (raw.empty()) return false;
	// Split on '|'
	std::vector<std::string> parts;
	{
		std::string::size_type pos = 0;
		while (pos <= raw.size())
		{
			std::string::size_type bar = raw.find('|', pos);
			if (bar == std::string::npos) { parts.push_back(raw.substr(pos)); break; }
			parts.push_back(raw.substr(pos, bar - pos));
			pos = bar + 1;
		}
	}
	if (parts.empty()) return false;
	const std::string &ver = parts[0];
	if (ver != "v1")
	{
		// Unknown future version: ignore the entry (tolerate)
		printf("neighbor-hints: ignore unknown version '%s'\n", ver.c_str());
		return false;
	}
	for (size_t i = 1; i < parts.size(); ++i)
	{
		if (parts[i].empty()) continue;
		// dx,dy:basename
		std::string::size_type colon = parts[i].find(':');
		if (colon == std::string::npos) continue;
		std::string coords = parts[i].substr(0, colon);
		std::string base = parts[i].substr(colon + 1);
		if (base.empty()) continue;
		std::string::size_type comma = coords.find(',');
		if (comma == std::string::npos) continue;
		int dx = 0, dy = 0;
		if (!NLMISC::fromString(coords.substr(0, comma), dx)) continue;
		if (!NLMISC::fromString(coords.substr(comma + 1), dy)) continue;
		// Strip accidental .max
		std::string::size_type dot = base.rfind('.');
		if (dot != std::string::npos && NLMISC::toLowerAscii(base.substr(dot)) == ".max")
			base = base.substr(0, dot);
		out.push_back(SNeighborHint(dx, dy, base));
	}
	return !out.empty();
}

static bool neighborHintLess(const SNeighborHint &a, const SNeighborHint &b)
{
	if (a.Dy != b.Dy) return a.Dy < b.Dy;
	if (a.Dx != b.Dx) return a.Dx < b.Dx;
	return NLMISC::toLowerAscii(a.Basename) < NLMISC::toLowerAscii(b.Basename);
}

/** Deterministic encode: sort by (dy, dx, basename) then emit v1|... */
static std::string encodeNeighborHintsString(std::vector<SNeighborHint> hints)
{
	std::sort(hints.begin(), hints.end(), neighborHintLess);
	// Dedup identical (dx,dy,basename)
	std::vector<SNeighborHint> uniq;
	for (size_t i = 0; i < hints.size(); ++i)
	{
		if (!uniq.empty()
		    && uniq.back().Dx == hints[i].Dx && uniq.back().Dy == hints[i].Dy
		    && NLMISC::toLowerAscii(uniq.back().Basename) == NLMISC::toLowerAscii(hints[i].Basename))
			continue;
		uniq.push_back(hints[i]);
	}
	std::string s = "v1";
	for (size_t i = 0; i < uniq.size(); ++i)
		s += NLMISC::toString("|%d,%d:%s", uniq[i].Dx, uniq[i].Dy, uniq[i].Basename.c_str());
	return s;
}

/** Read appdata neighbor hints from a node's script AppData; false if absent/unusable. */
static bool readNeighborHintsFromNode(CNodeImpl *node, std::vector<SNeighborHint> &out,
                                      std::string *rawOut = NULL)
{
	out.clear();
	if (rawOut) rawOut->clear();
	if (!node) return false;
	std::string raw;
	if (!APPDATA::getScriptAppData(node, NEL3D_APPDATA_PAINTER_NEIGHBOR_HINTS, raw))
		return false;
	if (rawOut) *rawOut = raw;
	return parseNeighborHintsString(raw, out);
}

/**
 * Shared appdata neighbor-hint read (session open + --dump-neighbor-hints).
 * Scans eligible non-frozen zone nodes first (write target), then any zone node
 * that carries the key (defensive if eligibility order differs after a resave).
 * rawOut: optional raw payload string when found.
 */
static bool readNeighborHintsFromScene(CScene &scene, const std::string &fileBasename,
                                       std::vector<SNeighborHint> &out, std::string *rawOut = NULL)
{
	out.clear();
	if (rawOut) rawOut->clear();
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	std::vector<bool> eligible;
	computeZoneEligibility(nodes, fileBasename, eligible);
	// (1) eligible non-frozen (same node writeNeighborHintsToScene targets)
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (i < eligible.size() && eligible[i] && !nodes[i].Frozen)
		{
			if (readNeighborHintsFromNode(nodes[i].Node, out, rawOut))
				return true;
		}
	}
	// (2) any zone node (covers edge cases: eligibility drift / frozen write host)
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (readNeighborHintsFromNode(nodes[i].Node, out, rawOut))
			return true;
	}
	return false;
}

/**
 * Write neighbor-hints appdata on the eligible node of `scene` (board-session save only).
 * Shape matches Max setAppData / every other NEL3D_APPDATA_* entry:
 *   AppData chunk 0x2150 on the node (CAnimatable)
 *   entry key = (ScriptClassId 0x04d64858/0x16d1751d, SuperClassId 4128, subId)
 *   value = StorageRaw null-terminated string
 * Deterministic encode (same neighbor set → byte-identical resave).
 */
static bool writeNeighborHintsToScene(CScene &scene, const std::string &fileBasename,
                                      const std::vector<SNeighborHint> &hints)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	std::vector<bool> eligible;
	computeZoneEligibility(nodes, fileBasename, eligible);
	CNodeImpl *target = NULL;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (i < eligible.size() && eligible[i] && !nodes[i].Frozen)
		{
			target = nodes[i].Node;
			break;
		}
	}
	if (!target)
	{
		fprintf(stderr, "WARNING: neighbor-hints write: no eligible node in '%s'\n",
		        fileBasename.c_str());
		return false;
	}
	// Authoring accessor creates AppData container when absent
	STORAGE::CAppData *ad = target->appData();
	if (!ad)
	{
		fprintf(stderr, "WARNING: neighbor-hints write: appData() failed on '%s'\n",
		        fileBasename.c_str());
		return false;
	}
	const std::string payload = encodeNeighborHintsString(hints);
	if (!ad->setScriptString(NEL3D_APPDATA_PAINTER_NEIGHBOR_HINTS, payload))
	{
		fprintf(stderr, "WARNING: neighbor-hints write: setScriptString failed\n");
		return false;
	}
	printf("neighbor-hints write '%s': %s\n", fileBasename.c_str(), payload.c_str());
	return true;
}

/**
 * Collect current RO context files as hints for the given editable (board save).
 * Offsets = SContextFile::CellX/Y recorded at load. Excludes other editables.
 */
static void collectHintsFromLoadedContext(std::vector<SNeighborHint> &out)
{
	out.clear();
	for (size_t i = 0; i < g_ContextFiles.size(); ++i)
	{
		const SContextFile &cf = g_ContextFiles[i];
		if (cf.Basename.empty()) continue;
		out.push_back(SNeighborHint(cf.CellX, cf.CellY, cf.Basename));
	}
}

// Forward: defined with multi-file save helpers
static PIPELINE::MAX::CScene *editableScene(const SEditableFileInfo &efi);

/** Board-session only: stamp neighbor hints onto every editable scene before save. */
static void writeNeighborHintsIfBoardSession()
{
	if (!g_BoardSession) return;
	std::vector<SNeighborHint> hints;
	collectHintsFromLoadedContext(hints);
	// Spec: record every loaded read-only neighbor file (g_ContextFiles).
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (!g_EditableFiles[i].Editable) continue;
		PIPELINE::MAX::CScene *sc = editableScene(g_EditableFiles[i]);
		if (!sc) continue;
		writeNeighborHintsToScene(*sc, g_EditableFiles[i].Basename, hints);
	}
}

/**
 * Extract neighbor hints from embedded frozen/ineligible zone nodes in a scene.
 * Prefer continent grid deltas when both the eligible node name and the embedded
 * name parse as [prefix-]<row>_<AA> (converted bricks). Else quantize object-TM
 * translation deltas by cellSize. Basenames keep the authored node name (e.g. "199_DY").
 */
static bool extractEmbeddedNeighborHints(CScene &scene, const std::string &fileBasename,
                                         float cellSize, std::vector<SNeighborHint> &out)
{
	out.clear();
	if (cellSize <= 0.f) cellSize = 160.f;
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	if (nodes.empty()) return false;
	std::vector<bool> eligible;
	computeZoneEligibility(nodes, fileBasename, eligible);

	// Eligible anchor: name + TM
	SNodeTMCache tmCache;
	float origX = 0.f, origY = 0.f;
	bool haveOrig = false;
	std::string eligibleName;
	int eRow = 0, eCol = 0;
	bool haveEGrid = false;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (!(i < eligible.size() && eligible[i] && !nodes[i].Frozen))
			continue;
		eligibleName = ucstring(nodes[i].Node->userName()).toUtf8();
		Matrix3M tm = getObjectTM(nodes[i].Node, tmCache);
		origX = tm.m[3][0];
		origY = tm.m[3][1];
		haveOrig = true;
		haveEGrid = ZPWS::parseContinentZoneName(eligibleName, eRow, eCol);
		// Also try file basename (zonematerial-converted-200_dz)
		if (!haveEGrid)
			haveEGrid = ZPWS::parseContinentZoneName(fileBasename, eRow, eCol);
		break;
	}
	if (!haveOrig)
	{
		for (size_t i = 0; i < nodes.size(); ++i)
		{
			if (nodes[i].Frozen) continue;
			eligibleName = ucstring(nodes[i].Node->userName()).toUtf8();
			Matrix3M tm = getObjectTM(nodes[i].Node, tmCache);
			origX = tm.m[3][0];
			origY = tm.m[3][1];
			haveOrig = true;
			haveEGrid = ZPWS::parseContinentZoneName(eligibleName, eRow, eCol);
			if (!haveEGrid)
				haveEGrid = ZPWS::parseContinentZoneName(fileBasename, eRow, eCol);
			break;
		}
	}
	if (!haveOrig) return false;

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const bool isEligible = (i < eligible.size() && eligible[i]);
		if (isEligible && !nodes[i].Frozen) continue; // self
		std::string name = ucstring(nodes[i].Node->userName()).toUtf8();
		if (name.empty()) continue;

		int dx = 0, dy = 0;
		int nRow = 0, nCol = 0;
		if (haveEGrid && ZPWS::parseContinentZoneName(name, nRow, nCol))
		{
			// Continent convention: col = X, row = Y
			dx = nCol - eCol;
			dy = nRow - eRow;
		}
		else
		{
			Matrix3M tm = getObjectTM(nodes[i].Node, tmCache);
			dx = (int)std::lround((double)(tm.m[3][0] - origX) / (double)cellSize);
			dy = (int)std::lround((double)(tm.m[3][1] - origY) / (double)cellSize);
		}
		if (dx == 0 && dy == 0) continue;
		out.push_back(SNeighborHint(dx, dy, name));
	}
	return !out.empty();
}

/** Resolve a hint basename to a real .max in the world (exact, case-insens, suffix match). */
static bool resolveHintToZone(const ZPWS::SWorldEntry &world, const std::string &hintBase,
                              ZPWS::SZoneEntry &out)
{
	if (world.MaxDir.empty() || hintBase.empty()) return false;
	static std::vector<ZPWS::SZoneEntry> s_listed;
	static std::string s_dir;
	if (s_dir != world.MaxDir)
	{
		ZPWS::listZones(world, s_listed);
		s_dir = world.MaxDir;
	}
	const std::string want = NLMISC::toLowerAscii(hintBase);
	// 1) exact basename
	for (size_t i = 0; i < s_listed.size(); ++i)
	{
		if (NLMISC::toLowerAscii(s_listed[i].Basename) == want)
		{
			out = s_listed[i];
			return true;
		}
	}
	// 2) basename ends with -hint or _hint token (zonematerial-converted-199_dy vs 199_DY)
	//    also ends-with the hint after a final dash
	for (size_t i = 0; i < s_listed.size(); ++i)
	{
		const std::string b = NLMISC::toLowerAscii(s_listed[i].Basename);
		if (b.size() >= want.size()
		    && b.compare(b.size() - want.size(), want.size(), want) == 0)
		{
			// require boundary: start or non-alnum before match
			if (b.size() == want.size()
			    || b[b.size() - want.size() - 1] == '-'
			    || b[b.size() - want.size() - 1] == '_')
			{
				out = s_listed[i];
				return true;
			}
		}
	}
	// 3) continent grid: parse hint as row_col and find by coords
	{
		int row = 0, col = 0;
		if (ZPWS::parseContinentZoneName(hintBase, row, col))
		{
			for (size_t i = 0; i < s_listed.size(); ++i)
			{
				int r = 0, c = 0;
				if (ZPWS::parseContinentZoneName(s_listed[i].Basename, r, c)
				    && r == row && c == col)
				{
					out = s_listed[i];
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * Neighbor suggestion priority (M16a):
 *   (1) appdata neighbor hints on the eligible node
 *   (2) names + cell offsets from embedded frozen/ineligible copies
 *   (3) continent grid-name 8-ring (empty for ecosystems)
 * Fills out with resolved real files; reports unresolved and skips them.
 * sourceOut: "appdata" | "embedded" | "grid" | "none"
 */
static void collectNeighborHints(CScene &scene, const std::string &fileBasename,
                                 const ZPWS::SWorldEntry &world, float cellSize,
                                 std::vector<SNeighborHint> &hintsOut,
                                 std::vector<ZPWS::SZoneEntry> &resolvedOut,
                                 std::string &sourceOut)
{
	hintsOut.clear();
	resolvedOut.clear();
	sourceOut = "none";

	// (1) appdata — same helper as --dump-neighbor-hints (M16d)
	if (readNeighborHintsFromScene(scene, fileBasename, hintsOut))
		sourceOut = "appdata";
	// (2) embedded copies
	if (sourceOut == "none")
	{
		if (extractEmbeddedNeighborHints(scene, fileBasename, cellSize, hintsOut))
			sourceOut = "embedded";
	}
	// (3) continent grid
	if (sourceOut == "none" && world.Kind == ZPWS::Continent)
	{
		ZPWS::SZoneEntry center;
		center.Basename = fileBasename;
		// Path not required for listContinentNeighbors (uses Basename + MaxDir index)
		std::vector<ZPWS::SZoneEntry> neigh;
		ZPWS::listContinentNeighbors(world, center, neigh);
		int crow = 0, ccol = 0;
		const bool haveC = ZPWS::parseContinentZoneName(fileBasename, crow, ccol);
		for (size_t i = 0; i < neigh.size(); ++i)
		{
			int nr = 0, nc = 0;
			int dx = 0, dy = 0;
			if (haveC && ZPWS::parseContinentZoneName(neigh[i].Basename, nr, nc))
			{
				// zone names: row is Y, col is X → dx=col delta, dy=row delta
				dx = nc - ccol;
				dy = nr - crow;
			}
			hintsOut.push_back(SNeighborHint(dx, dy, neigh[i].Basename));
			resolvedOut.push_back(neigh[i]);
		}
		if (!hintsOut.empty())
			sourceOut = "grid";
		return; // already resolved
	}

	// Resolve basenames for appdata/embedded sources
	for (size_t i = 0; i < hintsOut.size(); ++i)
	{
		ZPWS::SZoneEntry ze;
		if (!resolveHintToZone(world, hintsOut[i].Basename, ze))
		{
			fprintf(stderr, "WARNING: neighbor hint unresolved (skipped): %d,%d:%s\n",
			        hintsOut[i].Dx, hintsOut[i].Dy, hintsOut[i].Basename.c_str());
			continue;
		}
		// Prefer the real file basename in the stored hint for later write-back
		hintsOut[i].Basename = ze.Basename;
		resolvedOut.push_back(ze);
	}
	printf("neighbor-hints: source=%s raw=%u resolved=%u\n",
	       sourceOut.c_str(), (uint)hintsOut.size(), (uint)resolvedOut.size());
}

/** Translate all patch geometry of zones[begin..end) by (dx,dy) world units. */
static void translateZonesXY(std::vector<SPaintZone> &zones, size_t begin, size_t end,
                             float dx, float dy)
{
	for (size_t i = begin; i < end && i < zones.size(); ++i)
	{
		for (size_t p = 0; p < zones[i].Patches.size(); ++p)
		{
			NL3D::CPatchInfo &pi = zones[i].Patches[p];
			for (uint v = 0; v < 4; ++v)
			{
				pi.Patch.Vertices[v].x += dx;
				pi.Patch.Vertices[v].y += dy;
			}
			for (uint v = 0; v < 8; ++v)
			{
				pi.Patch.Tangents[v].x += dx;
				pi.Patch.Tangents[v].y += dy;
			}
			for (uint v = 0; v < 4; ++v)
			{
				pi.Patch.Interiors[v].x += dx;
				pi.Patch.Interiors[v].y += dy;
			}
		}
	}
}

/**
 * Clear and free all context/neighbor scenes.
 * Callers must not hold dangling zone Node pointers from those scenes.
 */
static void clearContextFiles()
{
	for (size_t i = 0; i < g_ContextFiles.size(); ++i)
		delete g_ContextFiles[i].Lm;
	g_ContextFiles.clear();
	g_NeighborScenes.clear();
}

/**
 * Load RO context files from the hint chain for each editable center (board-driven).
 * Continent without hints falls back to grid. Ecosystem needs appdata/embedded/place-context.
 * skipBasenames: already-open editables (not re-loaded as RO).
 */
static void loadNeighborContextFiles(std::vector<SPaintZone> &zones, float cellSize,
                                     const std::vector<std::string> &skipBasenames)
{
	clearContextFiles();
	if (!g_LoadNeighbors) return;
	if (g_StartupWorld.MaxDir.empty()) return;
	if (!g_BoardSession && g_StartupWorld.Kind != ZPWS::Continent) return;

	// Home footprint origin (for ecosystem translation): from primary zones already built
	float homeOriginX = 0.f, homeOriginY = 0.f, stepX = 0.f, stepY = 0.f;
	int fw = 1, fh = 1;
	if (!zones.empty())
	{
		size_t primaryEnd = 0;
		for (size_t i = 0; i < zones.size(); ++i)
			if (zones[i].ZoneId < 1000) primaryEnd = i + 1;
		if (primaryEnd > 0)
			computeFootprintRect(zones, 0, primaryEnd, cellSize,
			                     homeOriginX, homeOriginY, stepX, stepY, fw, fh);
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
	};
	std::vector<SPending> pending;

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
			if (skip.count(key) || loaded.count(key))
				continue;
			loaded.insert(key);
			SPending p;
			p.Zone = ze;
			p.Dx = hints[hi].Dx;
			p.Dy = hints[hi].Dy;
			// Continent files sit in absolute world space (no translate). Ecosystem brick
			// files are freestanding footprints — place at the hint cell offset relative to
			// the primary footprint origin. Appdata hints store fine cells; embedded name
			// deltas from continent grid tokens are also treated as fine-cell steps for
			// placement (footprint-accurate multi-cell adjacency is the M17 mask work).
			p.Translate = (g_StartupWorld.Kind == ZPWS::Ecosystem);
			(void)source;
			(void)fw;
			(void)fh;
			pending.push_back(p);
		}
	}

	// M16c place-context entries already in g_ContextFiles? (filled before call) — none yet
	// CLI --place-context handled separately via g_PlaceContextSpecs

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
			delete nlm;
			continue;
		}
		if (p.Translate && before < zones.size())
		{
			// Shift so context footprint origin lands at homeOrigin + (dx,dy)*cellSize
			float cOx = 0.f, cOy = 0.f, cSx = 0.f, cSy = 0.f;
			int cw = 1, ch = 1;
			computeFootprintRect(zones, before, zones.size(), cellSize, cOx, cOy, cSx, cSy, cw, ch);
			const float wantX = homeOriginX + (float)p.Dx * cellSize;
			const float wantY = homeOriginY + (float)p.Dy * cellSize;
			translateZonesXY(zones, before, zones.size(), wantX - cOx, wantY - cOy);
		}
		SContextFile cf;
		cf.Path = p.Zone.MaxPath;
		cf.Basename = p.Zone.Basename;
		cf.CellX = p.Dx;
		cf.CellY = p.Dy;
		cf.TranslateGeom = p.Translate;
		cf.Lm = nlm;
		g_ContextFiles.push_back(cf);
		g_NeighborScenes.push_back(nlm);
		printf("  context '%s' @ cell (%d,%d) zoneIdBase=%u FROZEN%s\n",
		       cf.Basename.c_str(), cf.CellX, cf.CellY, base,
		       p.Translate ? " (translated)" : "");
	}
}

// CLI --place-context specs (M16c); applied after primary load + rebuild
struct SPlaceContextSpec
{
	int Dx, Dy;
	std::string Basename;
};
static std::vector<SPlaceContextSpec> g_PlaceContextSpecs;
// Forward: M16c place-context load (defined with scratch board helpers)
static bool loadOnePlaceContext(std::vector<SPaintZone> &zones, float cellSize,
                                const SPlaceContextSpec &pc, std::string &err);

// ---------------------------------------------------------------------------------------------
// Ecosystem brick self-instances (ui M4a/M12): display-level duplicates at whole-footprint
// offsets with optional rotation/mirror (board-driven placements).
//
// NL3D landscape zones are baked in world space, so an "instance" is another CZone with its own
// zoneId whose patch geometry is the primary's geometry transformed (translate + rot/mirror
// about the primary AABB center), while paint state stays on the shared carrier (same Node
// pointer → same leaf/rpo key in paint_core). Per-zone Rotate/Symmetry feed paint_core's
// transformDesc assembly so picks and paint ops compensate into primary-space storage.
//
// Footprint step: geometry AABB in X/Y of the primary zones, each axis rounded UP to the
// nearest multiple of --cellsize. Mask-accurate / L-shaped footprints are a later refinement
// (AABB rectangle only).

/** Parse "dx,dy[,rot][,m]" — cell offsets, rot 0..3, optional mirror (m|1|true). */
static bool parsePlaceSpec(const std::string &s, SInstancePlace &out, std::string &err)
{
	out = SInstancePlace();
	std::vector<std::string> parts;
	{
		std::string cur;
		for (size_t i = 0; i <= s.size(); ++i)
		{
			char c = (i < s.size()) ? s[i] : ',';
			if (c == ',') { parts.push_back(cur); cur.clear(); }
			else cur += c;
		}
	}
	if (parts.size() < 2 || parts.size() > 4)
	{
		err = "place expects dx,dy[,rot][,m], got '" + s + "'";
		return false;
	}
	if (!NLMISC::fromString(parts[0], out.CellX) || !NLMISC::fromString(parts[1], out.CellY))
	{
		err = "place dx,dy unparseable: '" + s + "'";
		return false;
	}
	if (out.CellX == 0 && out.CellY == 0)
	{
		err = "place 0,0 is the primary (omit it); got '" + s + "'";
		return false;
	}
	if (parts.size() >= 3 && !parts[2].empty())
	{
		if (!NLMISC::fromString(parts[2], out.Rot) || out.Rot > 3)
		{
			err = "place rot must be 0..3, got '" + parts[2] + "'";
			return false;
		}
	}
	if (parts.size() >= 4 && !parts[3].empty())
	{
		std::string m = NLMISC::toLowerAscii(parts[3]);
		if (m == "1" || m == "m" || m == "true" || m == "yes" || m == "flip")
			out.Mirror = true;
		else if (m == "0" || m == "false" || m == "no")
			out.Mirror = false;
		else
		{
			err = "place mirror expects 0|1|m, got '" + parts[3] + "'";
			return false;
		}
	}
	return true;
}

/**
 * Parse deprecated "NxM" layout into translation-only places (non-origin cells).
 * Cell coordinates are fine-cell origins: slot (cx,cy) → origin (cx*fw, cy*fh) so
 * adjacent footprint slots abut (M14a). fw/fh default 1 when footprint not yet known.
 */
static bool parseInstanceLayout(const std::string &s, uint &cols, uint &rows,
                                std::vector<SInstancePlace> &places, std::string &err,
                                int fw = 0, int fh = 0)
{
	cols = 1;
	rows = 1;
	places.clear();
	std::string t = NLMISC::toLowerAscii(s);
	std::string::size_type x = t.find('x');
	if (x == std::string::npos || x == 0 || x + 1 >= t.size())
	{
		err = "instances layout expects NxM (e.g. 2x2), got '" + s + "'";
		return false;
	}
	if (!NLMISC::fromString(t.substr(0, x), cols) || !NLMISC::fromString(t.substr(x + 1), rows)
	    || cols < 1 || rows < 1 || cols > 8 || rows > 8)
	{
		err = "instances layout out of range or unparseable: '" + s + "' (use 1..8 per axis)";
		return false;
	}
	const bool ok = (cols == 1 && rows == 1)
		|| (cols == 2 && rows == 1)
		|| (cols == 1 && rows == 2)
		|| (cols == 2 && rows == 2)
		|| (cols == 3 && rows == 3);
	if (!ok)
	{
		err = "unsupported instances layout '" + s + "' (supported: 1x1, 2x1, 1x2, 2x2, 3x3)";
		return false;
	}
	if (fw < 1) fw = (g_FootprintCellsW > 0) ? g_FootprintCellsW : 1;
	if (fh < 1) fh = (g_FootprintCellsH > 0) ? g_FootprintCellsH : 1;
	for (uint cy = 0; cy < rows; ++cy)
	for (uint cx = 0; cx < cols; ++cx)
	{
		if (cx == 0 && cy == 0) continue;
		places.push_back(SInstancePlace((int)cx * fw, (int)cy * fh, 0, false));
	}
	return true;
}

/**
 * Footprint rect (M14a): AABB of primary zones, origin snapped DOWN to the cell grid,
 * size ceil'd UP to whole cells. Empty primary => 1×1 cell at (0,0).
 *
 * Pivot for rot/mirror is the FOOTPRINT block center (origin + half step), not the raw
 * geometry AABB center — so rotation/mirror map grid cells to grid cells by construction
 * for any integer W×H (square and non-square). Mask-accurate L-shapes are later.
 */
static void computeFootprintRect(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                                 size_t primaryEnd, float cellSize,
                                 float &originX, float &originY,
                                 float &stepX, float &stepY,
                                 int &cellsW, int &cellsH)
{
	if (cellSize <= 0.f) cellSize = 100.f;
	NLMISC::CAABBox bbox;
	bool init = false;
	for (size_t i = primaryBegin; i < primaryEnd && i < zones.size(); ++i)
	{
		for (size_t p = 0; p < zones[i].Patches.size(); ++p)
		{
			const NL3D::CBezierPatch &bp = zones[i].Patches[p].Patch;
			for (uint v = 0; v < 4; ++v)
			{
				if (!init) { bbox.setCenter(bp.Vertices[v]); bbox.setHalfSize(NLMISC::CVector::Null); init = true; }
				else bbox.extend(bp.Vertices[v]);
			}
			for (uint v = 0; v < 8; ++v) bbox.extend(bp.Tangents[v]);
			for (uint v = 0; v < 4; ++v) bbox.extend(bp.Interiors[v]);
		}
	}
	if (!init)
	{
		originX = originY = 0.f;
		stepX = stepY = cellSize;
		cellsW = cellsH = 1;
		return;
	}
	const NLMISC::CVector mn = bbox.getMin();
	const NLMISC::CVector mx = bbox.getMax();
	// Snap origin down to cell grid so the footprint rect is a clean cell block
	originX = (float)(std::floor((double)mn.x / (double)cellSize) * (double)cellSize);
	originY = (float)(std::floor((double)mn.y / (double)cellSize) * (double)cellSize);
	const float extX = mx.x - originX;
	const float extY = mx.y - originY;
	cellsW = (int)std::ceil((double)extX / (double)cellSize);
	cellsH = (int)std::ceil((double)extY / (double)cellSize);
	if (cellsW < 1) cellsW = 1;
	if (cellsH < 1) cellsH = 1;
	stepX = (float)cellsW * cellSize;
	stepY = (float)cellsH * cellSize;
}

/** Footprint step only (compat helper). */
static void computeFootprintStep(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                                 size_t primaryEnd, float cellSize, float &stepX, float &stepY)
{
	float ox = 0.f, oy = 0.f;
	int cw = 1, ch = 1;
	computeFootprintRect(zones, primaryBegin, primaryEnd, cellSize, ox, oy, stepX, stepY, cw, ch);
}

/** Occupied block size after rot (mirror alone does not change the axis-aligned cell AABB). */
static void footprintBlockSize(int cellsW, int cellsH, uint rot, bool /*mirror*/,
                               int &outW, int &outH)
{
	if (rot & 1) { outW = cellsH; outH = cellsW; }
	else { outW = cellsW; outH = cellsH; }
}

/**
 * After rotating/mirroring a block about its own center, return the new min-corner origin
 * (relative to the old origin) and the new size. Uses continuous center + half-extents so
 * same-parity W×H stay on-grid; different-parity half-cells round with floor (unit-tested).
 */
static void footprintBlockAfterTransform(int cellsW, int cellsH, uint rot, bool mirror,
                                         int &originDX, int &originDY, int &outW, int &outH)
{
	// Transform the continuous rect [0,W]×[0,H] about center (W/2,H/2)
	const double W = (double)cellsW, H = (double)cellsH;
	const double cx = W * 0.5, cy = H * 0.5;
	const double corners[4][2] = { { 0, 0 }, { W, 0 }, { W, H }, { 0, H } };
	double minX = 1e300, minY = 1e300, maxX = -1e300, maxY = -1e300;
	for (int i = 0; i < 4; ++i)
	{
		double lx = corners[i][0] - cx;
		double ly = corners[i][1] - cy;
		if (mirror) lx = -lx;
		double rx = lx, ry = ly;
		switch (rot & 3)
		{
		case 0: rx = lx;  ry = ly;  break;
		case 1: rx = -ly; ry = lx;  break;
		case 2: rx = -lx; ry = -ly; break;
		case 3: rx = ly;  ry = -lx; break;
		}
		const double x = rx + cx;
		const double y = ry + cy;
		if (x < minX) minX = x;
		if (y < minY) minY = y;
		if (x > maxX) maxX = x;
		if (y > maxY) maxY = y;
	}
	originDX = (int)std::floor(minX + 1e-9);
	originDY = (int)std::floor(minY + 1e-9);
	outW = (int)std::floor(maxX - minX + 0.5);
	outH = (int)std::floor(maxY - minY + 0.5);
	if (outW < 1) outW = 1;
	if (outH < 1) outH = 1;
}

/** Unit-check synthetic W≠H occupancy (no corpus brick is non-square multi-cell). */
static void unitCheckFootprintOccupancy()
{
	int odx = 0, ody = 0, ow = 0, oh = 0;
	footprintBlockAfterTransform(2, 1, 1, false, odx, ody, ow, oh);
	const int w21 = ow, h21 = oh, dx21 = odx, dy21 = ody;
	const bool ok21 = (w21 == 1 && h21 == 2);
	footprintBlockAfterTransform(3, 2, 1, false, odx, ody, ow, oh);
	const int w32 = ow, h32 = oh;
	const bool ok32 = (w32 == 2 && h32 == 3);
	footprintBlockAfterTransform(8, 8, 1, false, odx, ody, ow, oh);
	const bool ok88 = (ow == 8 && oh == 8 && odx == 0 && ody == 0);
	footprintBlockAfterTransform(2, 1, 0, true, odx, ody, ow, oh);
	const bool okM = (ow == 2 && oh == 1);
	printf("footprint unit-check: 2×1 R1 → %dx%d originΔ (%d,%d) %s; 3×2 R1 → %dx%d %s; "
	       "8×8 R1 origin-stable %s; 2×1 M size-stable %s\n",
	       w21, h21, dx21, dy21, ok21 ? "OK" : "FAIL",
	       w32, h32, ok32 ? "OK" : "FAIL",
	       ok88 ? "OK" : "FAIL", okM ? "OK" : "FAIL");
}

/** XY transform: mirror about Y (X flip, land/maxscript scale[-1,1,1]), then rot 0..3 CCW about pivot, then translate. */
static void transformInstanceXY(float &x, float &y, float pivotX, float pivotY,
                                float dx, float dy, uint rot, bool mirror)
{
	float lx = x - pivotX;
	float ly = y - pivotY;
	if (mirror)
		lx = -lx;
	float rx = lx, ry = ly;
	switch (rot & 3)
	{
	case 0: rx = lx;  ry = ly;  break;
	case 1: rx = -ly; ry = lx;  break; // 90° CCW
	case 2: rx = -lx; ry = -ly; break; // 180°
	case 3: rx = ly;  ry = -lx; break; // 270° CCW
	}
	x = rx + pivotX + dx;
	y = ry + pivotY + dy;
}

/**
 * Pivot = footprint block center (origin snapped to cell grid + half step).
 * Matches AABB center for grid-aligned square footprints; for W≠H keeps rot/mirror
 * cell-grid faithful (M14a).
 */
static void computePrimaryPivot(const std::vector<SPaintZone> &zones, size_t primaryBegin,
                                size_t primaryEnd, float cellSize, float &px, float &py)
{
	float ox = 0.f, oy = 0.f, sx = 0.f, sy = 0.f;
	int cw = 1, ch = 1;
	computeFootprintRect(zones, primaryBegin, primaryEnd, cellSize, ox, oy, sx, sy, cw, ch);
	px = ox + sx * 0.5f;
	py = oy + sy * 0.5f;
}

/**
 * World translation after rot/mirror about pivot so the transformed footprint's min-corner
 * lands at board cell (placeX, placeY) — primary footprint origin cell is (0,0).
 */
static void computePlaceTranslation(float originX, float originY, float stepX, float stepY,
                                    float pivotX, float pivotY, float cellSize,
                                    int placeX, int placeY, uint rot, bool mirror,
                                    float &outDx, float &outDy)
{
	// Transform the four corners of the primary footprint rect about the pivot
	float xs[4] = { originX, originX + stepX, originX + stepX, originX };
	float ys[4] = { originY, originY, originY + stepY, originY + stepY };
	float tminX = 0.f, tminY = 0.f;
	for (int i = 0; i < 4; ++i)
	{
		float x = xs[i], y = ys[i];
		transformInstanceXY(x, y, pivotX, pivotY, 0.f, 0.f, rot, mirror);
		if (i == 0) { tminX = x; tminY = y; }
		else
		{
			if (x < tminX) tminX = x;
			if (y < tminY) tminY = y;
		}
	}
	const float desiredX = originX + (float)placeX * cellSize;
	const float desiredY = originY + (float)placeY * cellSize;
	outDx = desiredX - tminX;
	outDy = desiredY - tminY;
}

/**
 * Display clone of a primary zone at footprint cell placement with rot/mirror.
 * Shares Node (carrier) with source. Geometry is transformed; tiles stay authored-space
 * until applyInstanceDisplayTiles (after bank load) / paint_core transformDesc.
 */
static SPaintZone cloneInstanceZone(const SPaintZone &src, uint zoneId, float dx, float dy,
                                    float pivotX, float pivotY, const SInstancePlace &place)
{
	SPaintZone pz = src;
	pz.ZoneId = zoneId;
	pz.Rotate = place.Rot & 3;
	pz.Symmetry = place.Mirror;
	std::string tag = NLMISC::toString(" (inst %d,%d", place.CellX, place.CellY);
	if (pz.Rotate) tag += NLMISC::toString(" R%u", pz.Rotate * 90);
	if (pz.Symmetry) tag += " M";
	tag += ")";
	pz.Name = src.Name + tag;
	pz.BorderVertices.clear();
	// Ep (topology) is a value copy — same binds/orders; Patches are world-space display.
	for (size_t p = 0; p < pz.Patches.size(); ++p)
	{
		NL3D::CPatchInfo &pi = pz.Patches[p];
		for (uint v = 0; v < 4; ++v)
			transformInstanceXY(pi.Patch.Vertices[v].x, pi.Patch.Vertices[v].y,
			                    pivotX, pivotY, dx, dy, pz.Rotate, pz.Symmetry);
		for (uint v = 0; v < 8; ++v)
			transformInstanceXY(pi.Patch.Tangents[v].x, pi.Patch.Tangents[v].y,
			                    pivotX, pivotY, dx, dy, pz.Rotate, pz.Symmetry);
		for (uint v = 0; v < 4; ++v)
			transformInstanceXY(pi.Patch.Interiors[v].x, pi.Patch.Interiors[v].y,
			                    pivotX, pivotY, dx, dy, pz.Rotate, pz.Symmetry);
		// Remap intra-zone bind ZoneIds to this instance (session weld fills cross-zone later)
		for (uint e = 0; e < 4; ++e)
		{
			if (pi.BindEdges[e].NPatchs != 0 && pi.BindEdges[e].ZoneId == (uint16)src.ZoneId)
				pi.BindEdges[e].ZoneId = (uint16)zoneId;
		}
	}
	return pz;
}

/**
 * Append display instances for g_Places (primary already at origin).
 * Place (dx,dy) = min-corner cell of the transformed footprint block (M14a).
 * Returns number of zone entries appended. zone ids start at kInstanceZoneIdBase.
 */
static uint appendInstanceZones(std::vector<SPaintZone> &zones, size_t primaryCount,
                                const std::vector<SInstancePlace> &places, float cellSize)
{
	if (places.empty()) return 0;
	if (primaryCount == 0 || zones.size() < primaryCount) return 0;

	float originX = 0.f, originY = 0.f, stepX = 0.f, stepY = 0.f;
	int cellsW = 1, cellsH = 1;
	computeFootprintRect(zones, 0, primaryCount, cellSize, originX, originY, stepX, stepY, cellsW, cellsH);
	g_FootprintCellsW = cellsW;
	g_FootprintCellsH = cellsH;
	float pivotX = originX + stepX * 0.5f;
	float pivotY = originY + stepY * 0.5f;

	// One-shot synthetic W≠H occupancy math (corpus bricks are square multi-cell)
	static bool s_UnitChecked = false;
	if (!s_UnitChecked)
	{
		unitCheckFootprintOccupancy();
		s_UnitChecked = true;
	}

	uint nextId = kInstanceZoneIdBase;
	const uint needIds = (uint)places.size() * (uint)primaryCount;
	if (nextId + needIds >= 65535)
	{
		fprintf(stderr, "ERROR: instance zone id range would exceed uint16 (need %u ids from %u)\n",
		        needIds, nextId);
		return 0;
	}

	uint appended = 0;
	for (size_t pi = 0; pi < places.size(); ++pi)
	{
		const SInstancePlace &pl = places[pi];
		float dx = 0.f, dy = 0.f;
		computePlaceTranslation(originX, originY, stepX, stepY, pivotX, pivotY, cellSize,
		                        pl.CellX, pl.CellY, pl.Rot & 3, pl.Mirror, dx, dy);
		int bw = 0, bh = 0;
		footprintBlockSize(cellsW, cellsH, pl.Rot, pl.Mirror, bw, bh);
		for (size_t i = 0; i < primaryCount; ++i)
		{
			zones.push_back(cloneInstanceZone(zones[i], nextId, dx, dy, pivotX, pivotY, pl));
			++nextId;
			++appended;
		}
		printf("  place[%u] origin cell (%d,%d) rot %u%s  occupies %dx%d cells [%d,%d)×[%d,%d)\n",
		       (uint)pi, pl.CellX, pl.CellY, pl.Rot, pl.Mirror ? " mirror" : "",
		       bw, bh, pl.CellX, pl.CellX + bw, pl.CellY, pl.CellY + bh);
	}
	printf("instances: %u place(s)  footprint %dx%d cells step (%.1f, %.1f)  origin (%.1f, %.1f)  "
	       "pivot (%.1f, %.1f)  primary zones %u  display zones +%u (ids from %u)\n",
	       (uint)places.size(), cellsW, cellsH, stepX, stepY, originX, originY,
	       pivotX, pivotY, (uint)primaryCount, appended, kInstanceZoneIdBase);
	printf("  note: place dx,dy = min-corner of transformed block in fine cells; "
	       "home occupies [0,%d)×[0,%d); mask-accurate L-shapes are later\n",
	       cellsW, cellsH);
	// Pivot grid-alignment check for multi-cell square (material-bassin 8×8)
	const float halfCell = cellSize * 0.5f;
	const bool pivotOnHalf = (std::fabs(std::fmod((double)pivotX, (double)halfCell)) < 1e-3
	                          || std::fabs(std::fmod((double)pivotX, (double)halfCell) - halfCell) < 1e-3);
	(void)pivotOnHalf;
	printf("  pivot grid: pivot (%.3f, %.3f) = origin + half-step (%.3f, %.3f)\n",
	       pivotX, pivotY, stepX * 0.5f, stepY * 0.5f);
	return appended;
}

/**
 * After bank load: transform display CPatchInfo tile elements on rotated/mirrored instances
 * so the initial landscape matches GetTile display space (plugin exportZone / CPatchInfo::transform
 * tile loop). Geometry is already in place from cloneInstanceZone — only tile/color remounts
 * apply here (control-point index swaps under symmetry are skipped; world XY was already mirrored).
 *
 * M13c: complete the symmetry half — U remount (u' = OrderS-u-1), transformTile/transform256Case
 * with symmetry=true, and tile-color S-flip — so a freshly placed mirrored instance matches
 * painted-then-mirrored before any live setTile refresh.
 */
static void applyInstanceDisplayTiles(std::vector<SPaintZone> &zones, NL3D::CTileBank *bank)
{
	if (!bank) return;
	for (size_t zi = 0; zi < zones.size(); ++zi)
	{
		SPaintZone &pz = zones[zi];
		if (pz.Rotate == 0 && !pz.Symmetry) continue;
		const uint rotate = pz.Rotate & 3;
		const bool symmetry = pz.Symmetry;
		for (size_t p = 0; p < pz.Patches.size(); ++p)
		{
			NL3D::CPatchInfo &pi = pz.Patches[p];
			// Tile colors: S-flip under symmetry (CPatchInfo::transform color half)
			if (symmetry && !pi.TileColors.empty() && pi.OrderS > 0)
			{
				const uint countU = (uint)pi.OrderS / 2 + 1;
				const uint countV = (uint)pi.OrderT + 1;
				for (uint v = 0; v < countV; ++v)
				for (uint u = 0; u < countU; ++u)
				{
					const uint index0 = u + v * ((uint)pi.OrderS + 1);
					const uint index1 = ((uint)pi.OrderS - u) + v * ((uint)pi.OrderS + 1);
					if (index0 >= pi.TileColors.size() || index1 >= pi.TileColors.size()) continue;
					if (index0 == index1) continue;
					uint16 tmp = pi.TileColors[index0].Color565;
					pi.TileColors[index0].Color565 = pi.TileColors[index1].Color565;
					pi.TileColors[index1].Color565 = tmp;
				}
			}
			// Tiles: U remount under symmetry + transformTile / transform256Case (plugin loop)
			std::vector<NL3D::CTileElement> tiles = pi.Tiles;
			for (int v = 0; v < pi.OrderT; ++v)
			for (int u = 0; u < pi.OrderS; ++u)
			{
				const int uSymmetry = symmetry ? (pi.OrderS - u - 1) : u;
				NL3D::CTileElement &element = pi.Tiles[u + v * pi.OrderS];
				element = tiles[uSymmetry + v * pi.OrderS];
				for (int l = 0; l < 3; ++l)
				{
					if (element.Tile[l] == 0xffff) continue;
					uint tile = element.Tile[l];
					uint tileRotation = element.getTileOrient(l);
					uint tileRotate = rotate;
					bool tileSymmetry = symmetry;
					// goofy=false: zone Sym is built later in paint_core; initial display uses
					// the non-goofy path (matches M12 rotation remount and live setTile when
					// border state is Nothing).
					if (!NL3D::CPatchInfo::getTileSymmetryRotate(*bank, tile, tileSymmetry, tileRotate))
						continue;
					if (!NL3D::CPatchInfo::transformTile(*bank, tile, tileRotation, tileSymmetry, (4 - tileRotate) & 3, false))
						continue;
					element.Tile[l] = (uint16)tile;
					element.setTileOrient(l, (uint8)(tileRotation & 3));
				}
				if (element.Tile[0] != 0xffff)
				{
					bool is256 = false;
					uint8 uvOff = 0;
					element.getTile256Info(is256, uvOff);
					if (is256)
					{
						uint tileRotate = rotate;
						bool tileSymmetry = symmetry;
						uint tileRotation = tiles[uSymmetry + v * pi.OrderS].getTileOrient(0);
						NL3D::CPatchInfo::getTileSymmetryRotate(*bank, element.Tile[0], tileSymmetry, tileRotate);
						NL3D::CPatchInfo::transform256Case(*bank, uvOff, tileRotation, tileSymmetry, (4 - tileRotate) & 3, false);
						element.setTile256Info(true, uvOff);
					}
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Cross-zone open-edge weld (port of the paint.cpp cross-mesh branch, WELD_THRESOLD=1): two
// open edges of different zones whose world-space endpoints coincide reversed within the
// threshold become a one/one bind across the zones, with CBorderVertex records on both sides
// so the landscape shares the corner tess vertices (and rebinds borders when zones are added
// in sequence). Session-only: this only touches the display CPatchInfo/CZone copies, the .max
// data is never modified by it.

#define WELD_THRESOLD 1.0f

// Both-direction border-vertex record for one matched corner pair, deduplicated (a corner is
// shared by the two consecutive welded edges).
static void addBorderVertexPair(std::vector<SPaintZone> &zones, uint zi, uint16 va, uint zj, uint16 vb,
                                std::set<std::pair<std::pair<uint, uint>, std::pair<uint, uint> > > &seen)
{
	std::pair<std::pair<uint, uint>, std::pair<uint, uint> > key(
		std::pair<uint, uint>(zi, va), std::pair<uint, uint>(zj, vb));
	if (key.second < key.first) std::swap(key.first, key.second);
	if (!seen.insert(key).second) return;
	NL3D::CBorderVertex bv;
	bv.CurrentVertex = va;
	bv.NeighborZoneId = (uint16)zones[zj].ZoneId;
	bv.NeighborVertex = vb;
	zones[zi].BorderVertices.push_back(bv);
	bv.CurrentVertex = vb;
	bv.NeighborZoneId = (uint16)zones[zi].ZoneId;
	bv.NeighborVertex = va;
	zones[zj].BorderVertices.push_back(bv);
}

static uint weldPaintZones(std::vector<SPaintZone> &zones)
{
	uint welds = 0;
	std::set<std::pair<std::pair<uint, uint>, std::pair<uint, uint> > > seenVerts;
	for (uint zi = 0; zi < zones.size(); ++zi)
	{
		std::vector<NL3D::CPatchInfo> &pa = zones[zi].Patches;
		for (uint p = 0; p < pa.size(); ++p)
		for (uint e = 0; e < 4; ++e)
		{
			if (pa[p].BindEdges[e].NPatchs != 0) continue; // interior or already welded
			// Edge e runs from corner e to corner (e+1)&3 (Max Patch edge convention, same as
			// the paint.cpp scan).
			const NLMISC::CVector &vA1 = pa[p].Patch.Vertices[e];
			const NLMISC::CVector &vB1 = pa[p].Patch.Vertices[(e + 1) & 3];
			bool found = false;
			for (uint zj = 0; zj < zones.size() && !found; ++zj)
			{
				if (zj == zi) continue;
				std::vector<NL3D::CPatchInfo> &pb = zones[zj].Patches;
				for (uint pp = 0; pp < pb.size() && !found; ++pp)
				for (uint ee = 0; ee < 4; ++ee)
				{
					if (pb[pp].BindEdges[ee].NPatchs != 0) continue;
					const NLMISC::CVector &vA2 = pb[pp].Patch.Vertices[ee];
					const NLMISC::CVector &vB2 = pb[pp].Patch.Vertices[(ee + 1) & 3];
					// The same edge, reversed orientation (paint.cpp: vA1~vB2 && vA2~vB1).
					if ((vA1 - vB2).norm() < WELD_THRESOLD && (vA2 - vB1).norm() < WELD_THRESOLD)
					{
						pa[p].BindEdges[e].NPatchs = 1;
						pa[p].BindEdges[e].ZoneId = (uint16)zones[zj].ZoneId;
						pa[p].BindEdges[e].Next[0] = (uint16)pp;
						pa[p].BindEdges[e].Edge[0] = (uint8)ee;
						pb[pp].BindEdges[ee].NPatchs = 1;
						pb[pp].BindEdges[ee].ZoneId = (uint16)zones[zi].ZoneId;
						pb[pp].BindEdges[ee].Next[0] = (uint16)p;
						pb[pp].BindEdges[ee].Edge[0] = (uint8)e;
						// Shared corners: (zi corner e <-> zj corner ee+1), (zi corner e+1 <-> zj
						// corner ee).
						addBorderVertexPair(zones, zi, pa[p].BaseVertices[e],
						                    zj, pb[pp].BaseVertices[(ee + 1) & 3], seenVerts);
						addBorderVertexPair(zones, zi, pa[p].BaseVertices[(e + 1) & 3],
						                    zj, pb[pp].BaseVertices[ee], seenVerts);
						++welds;
						found = true;
						break;
					}
				}
			}
		}
	}
	return welds;
}

// Build one display CZone from a paint zone: CZone::build + the corner smoother, exactly the
// plugin's per-zone sequence (smoother runs on the built, uncompiled zone with no neighbor
// list, like the plugin did).
static void buildDisplayZone(const SPaintZone &pz, NL3D::CZone &zone)
{
	zone.build((uint16)pz.ZoneId, pz.Patches, pz.BorderVertices);
	NL3D::CZoneCornerSmoother cornerSmoother;
	std::vector<NL3D::CZone *> emptyVector;
	cornerSmoother.computeAllCornerSmoothFlags(&zone, emptyVector);
}

// ---------------------------------------------------------------------------------------------
// --dump-zones: the headless eval->weld->build proof. Writes each built zone and reports the
// counts that the weld and bind passes produced.

static std::string sanitizeName(const std::string &s)
{
	std::string r = s;
	for (size_t i = 0; i < r.size(); ++i)
	{
		char c = r[i];
		bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_';
		if (!ok) r[i] = '_';
	}
	return r;
}

static int dumpZones(std::vector<SPaintZone> &zones, uint welds, const std::string &outDir)
{
	NLMISC::CFile::createDirectoryTree(outDir);
	uint totalPatches = 0, totalBound = 0, totalCross = 0, totalBorderVerts = 0;
	for (size_t i = 0; i < zones.size(); ++i)
	{
		const SPaintZone &pz = zones[i];
		uint bound = 0, cross = 0;
		for (size_t p = 0; p < pz.Patches.size(); ++p)
		for (uint e = 0; e < 4; ++e)
		{
			const NL3D::CPatchInfo::CBindInfo &b = pz.Patches[p].BindEdges[e];
			if (b.NPatchs == 0) continue;
			++bound;
			if (b.ZoneId != pz.ZoneId) ++cross;
		}
		NL3D::CZone zone;
		buildDisplayZone(pz, zone);
		std::string path = outDir + "/" + NLMISC::toString("zone_%u_", pz.ZoneId) + sanitizeName(pz.Name) + ".zone";
		if (!writeZoneV4(zone, path))
		{
			fprintf(stderr, "ERROR: cannot write %s\n", path.c_str());
			return 1;
		}
		printf("zone %u '%s'%s: %u patches, %u bound edges (%u cross-zone), %u border verts -> %s\n",
		       pz.ZoneId, pz.Name.c_str(), pz.Frozen ? " FROZEN" : "",
		       (uint)pz.Patches.size(), bound, cross, (uint)pz.BorderVertices.size(), path.c_str());
		totalPatches += (uint)pz.Patches.size();
		totalBound += bound;
		totalCross += cross;
		totalBorderVerts += (uint)pz.BorderVertices.size();
	}
	// M11b: name list for eligibility verification (editable vs read-only context)
	{
		std::string editNames, roNames;
		uint nEdit = 0, nRo = 0;
		for (size_t i = 0; i < zones.size(); ++i)
		{
			const SPaintZone &pz = zones[i];
			if (pz.Frozen)
			{
				if (nRo++) roNames += ", ";
				roNames += pz.Name;
			}
			else
			{
				if (nEdit++) editNames += ", ";
				editNames += pz.Name;
			}
		}
		printf("eligibility names: editable(%u)=[%s] read-only(%u)=[%s] all-zones=%s\n",
		       nEdit, editNames.c_str(), nRo, roNames.c_str(), g_AllZones ? "on" : "off");
	}
	printf("OK dump-zones: %u zones, %u patches, %u bound edges (%u cross-zone), %u welds, %u border verts\n",
	       (uint)zones.size(), totalPatches, totalBound, totalCross, welds, totalBorderVerts);
	return 0;
}

// ---------------------------------------------------------------------------------------------
// Whole-file save: rebuilt Scene stream + every other stream verbatim + OLE class id (the P2
// flow, modeled on the corpus harness' rpoModifySaveTest). The caller mutates the parsed scene
// (paint write-back) BEFORE calling; a null edit through this same path is byte-identical.

// Serialize a container to a temp file and read the file bytes back. CMemStream's write-mode
// seek-back fails during leaveChunk; COFile handles seeks freely, so temp-file roundtrip is
// the working pattern (same as the corpus harness).
static std::vector<uint8> writeContainerToTemp(CStorageContainer &ctr, const std::string &tempPath)
{
	{
		NLMISC::COFile of(tempPath);
		ctr.serial(of, 0); // explicit-size overload avoids the outer 0x4352 wrapper
	}
	std::vector<uint8> out;
	std::ifstream ifs(tempPath.c_str(), std::ios::binary);
	if (ifs)
	{
		ifs.seekg(0, std::ios::end);
		std::streampos end = ifs.tellg();
		ifs.seekg(0);
		out.resize((size_t)end);
		if ((size_t)end) ifs.read((char *)nlVectorData(out), (std::streamsize)end);
	}
	return out;
}

/**
 * Whole-file save. Non-Scene streams come from `input` verbatim unless
 * summaryOverride is non-NULL, in which case SummaryInformation is replaced
 * (M5c thumbnail write). --null-edit / plain --save pass NULL so SI is untouched.
 */
static int saveWholeFile(const std::string &input, const std::string &output, CScene &scene,
                         bool verifyIdentical,
                         const std::vector<uint8> *summaryOverride = NULL)
{
	// The known .max stream set (same list as the corpus harness save tests).
	static const char *kStreams[] = {
		"VideoPostQueue", "Config", "ClassData", "DllDirectory", "ClassDirectory3", "Scene",
		"\05SummaryInformation", "\05DocumentSummaryInformation", NULL
	};
	std::vector<std::string> present;
	std::vector<std::vector<uint8> > rawOrig;
	uint8 classId[16];
	bool haveClassId;
	{
		CStorageOleIn in;
		if (!in.open(input)) { fprintf(stderr, "ERROR: not an OLE compound file: %s\n", input.c_str()); return 1; }
		for (const char **n = kStreams; *n; ++n)
		{
			std::vector<uint8> b;
			if (in.readStream(*n, b)) { present.push_back(*n); rawOrig.push_back(b); }
		}
		haveClassId = in.getClassId(classId);
	}

	// If we have a new SI and the stream was absent, append it.
	const std::string kSI = ZPTHUMB::kSummaryInformationStream;
	bool haveSI = false;
	for (size_t i = 0; i < present.size(); ++i)
		if (present[i] == kSI) { haveSI = true; break; }
	if (summaryOverride && !haveSI)
	{
		present.push_back(kSI);
		rawOrig.push_back(std::vector<uint8>()); // placeholder; overridden below
	}

	// Rebuild the Scene stream from the typed graph (§5 lifecycle) and write the whole file.
	std::string tempPath = NLMISC::toString("/tmp/zone_painter.%d.tmp", (int)ZP_GETPID());
	std::vector<uint8> newScene;
	try
	{
		scene.clean();
		scene.build(VersionUnknown);
		scene.disown();
		newScene = writeContainerToTemp(scene, tempPath);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "ERROR: scene rebuild: %s\n", e.what());
		remove(tempPath.c_str());
		return 1;
	}
	remove(tempPath.c_str());

	{
		CStorageOleOut out;
		for (size_t i = 0; i < present.size(); ++i)
		{
			if (present[i] == "Scene")
				out.addStream("Scene", newScene);
			else if (summaryOverride && present[i] == kSI)
				out.addStream(present[i], *summaryOverride);
			else
				out.addStream(present[i], rawOrig[i]);
		}
		if (haveClassId) out.setClassId(classId);
		if (!out.write(output)) { fprintf(stderr, "ERROR: cannot create %s\n", output.c_str()); return 1; }
	}

	uint diffs = 0;
	if (verifyIdentical)
	{
		CStorageOleIn in2;
		if (!in2.open(output)) { fprintf(stderr, "ERROR: cannot reopen %s\n", output.c_str()); return 1; }
		for (size_t i = 0; i < present.size(); ++i)
		{
			std::vector<uint8> b2;
			in2.readStream(present[i], b2);
			const std::vector<uint8> &expect =
				(summaryOverride && present[i] == kSI) ? *summaryOverride : rawOrig[i];
			if (b2 != expect)
			{
				fprintf(stderr, "ERROR: stream %s NOT byte-identical (%u -> %u bytes)\n",
				        (present[i][0] == '\05' ? present[i].substr(1) : present[i]).c_str(),
				        (uint)expect.size(), (uint)b2.size());
				++diffs;
			}
		}
	}
	if (verifyIdentical)
		printf("%s null-edit: %u stream diffs -> %s\n", diffs ? "FAIL" : "OK", diffs, output.c_str());
	else
		printf("OK save -> %s%s\n", output.c_str(), summaryOverride ? " (thumbnail updated)" : "");
	return diffs ? 1 : 0;
}

// Build the paint core inputs from the assembled zones (pointers into the final zones vector).
static void buildPaintInputs(std::vector<SPaintZone> &zones, std::vector<ZPPAINT::SPaintZoneInput> &inputs)
{
	inputs.clear();
	for (size_t i = 0; i < zones.size(); ++i)
	{
		ZPPAINT::SPaintZoneInput in;
		in.Node = zones[i].Node;
		in.Frozen = zones[i].Frozen;
		in.ZoneId = zones[i].ZoneId;
		in.Name = zones[i].Name;
		in.Patches = &zones[i].Patches;
		in.Pm = &zones[i].Ep.Pm;
		in.EvalRp = &zones[i].Ep.Rp;
		in.Rotate = zones[i].Rotate;
		in.Symmetry = zones[i].Symmetry;
		inputs.push_back(in);
	}
}

// ---------------------------------------------------------------------------------------------
// Scripted paint mode: one op per line, same op layer as the mouse path (see the file header
// for the command list). Any FAILed op fails the run (scripts are curated test inputs).

static int runPaintScript(ZPPAINT::CPaintCore &core, const std::string &path)
{
	std::ifstream ifs(path.c_str());
	if (!ifs) { fprintf(stderr, "ERROR: cannot open script %s\n", path.c_str()); return 1; }
	std::string line;
	int lineNo = 0;
	int fails = 0;
	while (std::getline(ifs, line))
	{
		++lineNo;
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
		if (tok.empty()) continue;
		std::string err;
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
			// mask <file.tga|none> — color-brush bitmap mask (CPath-resolved)
			if (tok[1] == "none")
				core.clearBrushMask();
			else
				ok = core.loadBrushMask(tok[1], err);
		}
		else if (tok[0] == "dumpclosure" && tok.size() >= 5)
		{
			// dumpclosure <zone> <patch> <s> <t> — print the vertex's co-location closure
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
			// DEBUG: rawtile <zone> <patch> <u> <v> <tile> [rot] — raw record, no solver
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
			fprintf(stderr, "ERROR: script line %d: bad command '%s'\n", lineNo, tok[0].c_str());
			return 1;
		}
		if (ok) printf("OK line %d: %s (%u tile writes)\n", lineNo, tok[0].c_str(), core.strokeSetCount());
		else
		{
			printf("FAIL line %d: %s: %s\n", lineNo, tok[0].c_str(), err.c_str());
			++fails;
		}
	}
	return fails ? 1 : 0;
}

// ---------------------------------------------------------------------------------------------
// Viewer / screenshot: the painting scene (paint.cpp myThread without the paint tools).

// Window close tracking (the plugin's MouseListener WindowActive flag).
class CWindowCloseListener : public NLMISC::IEventListener
{
public:
	bool WindowActive;
	CWindowCloseListener() : WindowActive(true) { }
	virtual void operator()(const NLMISC::CEvent &event)
	{
		if (event == NLMISC::EventDestroyWindowId || event == NLMISC::EventCloseWindowId)
			WindowActive = false;
	}
};

// The light setup lives in g_Light* above (paint_ui.cpp defaults, vars-cfg overridable).

static const uint kMainWidth = 800;
static const uint kMainHeight = 600;

// The tile bank (the plugin took it from the tile_utility choice; here it is --bank). Tile
// texture paths become CPath-resolvable relative names, seeded with the bank file's directory
// (recursive on request) plus any extra search paths.
static bool loadBankFile(const std::string &bankPath, bool bankRecursive,
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
class CPaintMouseListener : public NLMISC::IEventListener
{
public:
	enum TPaintMode { ModeTile = 0, ModeColor, ModeDisplace };

	ZPPAINT::CPaintCore *Core;
	NL3D::CEvent3dMouseListener *Nav;
	NL3D::CCamera *Camera; // unwrapped from UScene::getCam()
	ZPUI::CEditorUI *EditorUI;
	NL3D::CViewport Viewport;
	int CurTileSet;
	bool Mode256;
	bool Pressed;
	float MouseX, MouseY;
	bool HaveHover;
	uint HoverZone;
	sint32 HoverTile;
	uint StrokeZone;
	sint32 StrokeTile;
	// P3c modes
	int Mode; // TPaintMode
	NLMISC::CRGBA BrushColor;
	float BrushRadius;
	uint BrushHardness, BrushOpacity; // 0-255
	uint DisplaceIndex;               // 0-15

	CPaintMouseListener() : Core(NULL), Nav(NULL), Camera(NULL), EditorUI(NULL), CurTileSet(0), Mode256(false), Pressed(false),
		MouseX(0.5f), MouseY(0.5f), HaveHover(false), HoverZone(0), HoverTile(-1), StrokeZone(0), StrokeTile(-1),
		Mode(ModeTile), BrushColor(255, 255, 255, 255), BrushRadius(8.f), BrushHardness(128), BrushOpacity(255),
		DisplaceIndex(0) { }

	bool guiWantsMouse() const { return EditorUI && EditorUI->wantsMouse(); }

	// One paint action at the current hover (shared by click and drag)
	void paintAtHover()
	{
		std::string err;
		if (Mode == ModeColor)
		{
			NLMISC::CVector pos, dir, hit;
			Viewport.getRayWithPoint(MouseX, MouseY, pos, dir, Camera->getMatrix(), Camera->getFrustum());
			uint zone;
			sint32 tile;
			if (Core->pickTile(pos, dir, zone, tile, hit) && !Core->zoneFrozen(zone))
				Core->opColorBrush(zone, tile, hit, BrushRadius, BrushColor, BrushHardness, BrushOpacity, err);
		}
		else if (Mode == ModeDisplace)
		{
			sint32 t = HoverTile;
			Core->opDisplace(HoverZone, (uint)(t / ZP_NUM_TILE_SEL), (uint)(t % ZP_NUM_TILE_SEL % ZP_MAX_TILE_IN_PATCH),
			                 (uint)(t % ZP_NUM_TILE_SEL / ZP_MAX_TILE_IN_PATCH), DisplaceIndex, err);
		}
	}

	void updateHover()
	{
		HaveHover = false;
		if (!Core || !Camera) return;
		NLMISC::CVector pos, dir, hit;
		Viewport.getRayWithPoint(MouseX, MouseY, pos, dir, Camera->getMatrix(), Camera->getFrustum());
		uint zone;
		sint32 tile;
		if (Core->pickTile(pos, dir, zone, tile, hit))
		{
			HaveHover = true;
			HoverZone = zone;
			HoverTile = tile;
		}
	}

	virtual void operator()(const NLMISC::CEvent &event)
	{
		if (!Core) return;
		// Keyboard undo/redo always works; mouse is consumed when over the GUI.
		if (event == NLMISC::EventKeyDownId)
		{
			NLMISC::CEventKeyDown *keyDown = (NLMISC::CEventKeyDown *)&event;
			if (keyDown->FirstTime && (keyDown->Button & NLMISC::ctrlKeyButton))
			{
				if (keyDown->Key == NLMISC::KeyZ) Core->opUndo();
				if (keyDown->Key == NLMISC::KeyE) Core->opRedo();
			}
			return;
		}
		if (guiWantsMouse())
		{
			// Abort an in-progress stroke if the pointer enters a GUI window mid-drag
			if (Pressed && (event == NLMISC::EventMouseUpId || event == NLMISC::EventMouseMoveId))
			{
				Pressed = false;
				Core->endStroke();
			}
			return;
		}
		if (event == NLMISC::EventMouseDownId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			MouseX = mouse->X;
			MouseY = mouse->Y;
			if (mouse->Button == NLMISC::leftButton)
			{
				updateHover();
				if (HaveHover && !Core->zoneFrozen(HoverZone))
				{
					if (Mode == ModeTile)
					{
						std::string err;
						if (Core->opTileStroke(HoverZone, HoverTile, CurTileSet, Mode256, true, err))
						{
							Pressed = true;
							StrokeZone = HoverZone;
							StrokeTile = HoverTile;
						}
					}
					else
					{
						paintAtHover();
						Pressed = true;
						StrokeZone = HoverZone;
						StrokeTile = HoverTile;
					}
				}
			}
			if (mouse->Button == NLMISC::rightButton)
			{
				// Pick under the cursor: tile mode = the base layer's set; color mode = the
				// vertex color; displace mode = the tile's displace index
				updateHover();
				if (HaveHover)
				{
					ZPPAINT::CTileDescP desc;
					Core->getTile(HoverZone, HoverTile, desc);
					if (Mode == ModeTile)
					{
						if (!desc.isEmpty())
							CurTileSet = Core->tileSetOfTile(desc.getLayer(0).Tile);
					}
					else if (Mode == ModeColor)
					{
						sint32 t = HoverTile;
						uint32 raw;
						if (Core->getColor(HoverZone, (uint)(t / ZP_NUM_TILE_SEL),
						                   (sint32)(t % ZP_NUM_TILE_SEL % ZP_MAX_TILE_IN_PATCH),
						                   (sint32)(t % ZP_NUM_TILE_SEL / ZP_MAX_TILE_IN_PATCH), raw))
							BrushColor = NLMISC::CRGBA((uint8)((raw >> 16) & 0xff), (uint8)((raw >> 8) & 0xff), (uint8)(raw & 0xff), 255);
					}
					else if (Mode == ModeDisplace)
					{
						DisplaceIndex = desc.getDisplace();
					}
				}
			}
		}
		else if (event == NLMISC::EventMouseUpId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			if (mouse->Button == NLMISC::leftButton && Pressed)
			{
				Pressed = false;
				Core->endStroke();
			}
		}
		else if (event == NLMISC::EventMouseMoveId)
		{
			NLMISC::CEventMouse *mouse = (NLMISC::CEventMouse *)&event;
			MouseX = mouse->X;
			MouseY = mouse->Y;
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
							StrokeZone = HoverZone;
							StrokeTile = HoverTile;
						}
					}
					else
					{
						paintAtHover();
						StrokeZone = HoverZone;
						StrokeTile = HoverTile;
					}
				}
			}
		}
	}
};

// F12 screenshot convenience (CNELU::screenshot port; uses the unwrapped IDriver).
static void zpViewerScreenshot(NL3D::IDriver *driver, NLMISC::CEventListenerAsync &async)
{
	if (!async.isKeyPushed(NLMISC::KeyF12))
		return;
	NLMISC::CBitmap btm;
	driver->getBuffer(btm);
	std::string filename = NLMISC::CFile::findNewFile("screenshot.tga");
	NLMISC::COFile fs(filename);
	btm.writeTGA(fs, 24);
	nlinfo("Screenshot '%s' saved", filename.c_str());
}

// ---------------------------------------------------------------------------------------------
// Shared paint actions: keyboard AND NLGUI call these (no second op implementation).
// Live for the duration of runViewer only (g_PaintCtx.Active). Also used by the headless
// --panel-save-test hook (same zpSaveTo / zpSaveOverwrite functions).

struct SPaintCtx
{
	bool Active;
	ZPPAINT::CPaintCore *Core;
	CPaintMouseListener *Paint;
	std::string InputPath;
	std::string SavePath;
	PIPELINE::MAX::CScene *Scene; // Max scene for whole-file save (editable file only)
	bool InteractiveSave;         // startup interactive without --save => Save modal
	bool WantThumbnail;           // CLI --thumbnail or modal checkbox (M5c)
	// Live viewer hooks for top-down thumbnail capture (valid only while runViewer runs)
	NL3D::UDriver *UDriver;
	NL3D::UScene *UScene;
	NL3D::CLandscapeModel *Land;
	NL3D::CCamera *Camera;
	std::vector<SPaintZone> *Zones;
	// Season toggle (M6a): bank + path for re-resolve; available codes from discovery
	NL3D::CTileBank *Bank;
	std::string BankPath;
	std::vector<std::string> *AvailableSeasons;
	SPaintCtx()
		: Active(false), Core(NULL), Paint(NULL), Scene(NULL), InteractiveSave(false),
		  WantThumbnail(false), UDriver(NULL), UScene(NULL), Land(NULL), Camera(NULL), Zones(NULL),
		  Bank(NULL), AvailableSeasons(NULL)
	{
	}
};
// Discovered season codes for the open bank (filled at bank load; empty = no seasonal tiles)
static std::vector<std::string> g_AvailableSeasons;
static SPaintCtx g_PaintCtx;
// CLI --thumbnail: set before save so zpSaveTo / headless save path pick it up
static bool g_CliWantThumbnail = false;
// Captured top-down thumb (survives runViewer teardown for post-viewer --save)
static NLMISC::CBitmap g_CapturedThumb;
static bool g_HaveCapturedThumb = false;

// Last save status for HUD / callers (also printed to stderr on failure)
static std::string g_LastSaveStatus;

static void zpSelectMode(int mode)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (mode < 0) mode = 0;
	if (mode > 2) mode = 2;
	g_PaintCtx.Paint->Mode = mode;
}

static void zpRebuildTilesetPalette(); // fwd: tileset pick may refresh displace previews

static void zpSelectTileSetDelta(int d)
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
	// Displace map files are per-tileset (M9a).
	zpRebuildTilesetPalette();
}

static void zpSelectTileSetAbs(int idx)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	uint count = g_PaintCtx.Core->tileSetCount();
	if (!count || idx < 0 || idx >= (int)count) return;
	const int prev = g_PaintCtx.Paint->CurTileSet;
	g_PaintCtx.Paint->CurTileSet = idx;
	// Displace map files are per-tileset; refresh the palette section when the set changes (M9a).
	if (prev != idx)
		zpRebuildTilesetPalette();
}

static void zpToggleTileSize()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	g_PaintCtx.Paint->Mode256 = !g_PaintCtx.Paint->Mode256;
}

/** Color brush radius step (×1.5 / ÷1.5, clamp 2..32). Shared by SizeUp/Down in Color mode and panel. */
static void zpColorRadiusDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (d > 0)
		g_PaintCtx.Paint->BrushRadius = std::min(g_PaintCtx.Paint->BrushRadius * 1.5f, 32.f);
	else if (d < 0)
		g_PaintCtx.Paint->BrushRadius = std::max(g_PaintCtx.Paint->BrushRadius / 1.5f, 2.f);
}

static void zpBrushSizeDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	if (g_PaintCtx.Paint->Mode == CPaintMouseListener::ModeColor)
		zpColorRadiusDelta(d);
	else
	{
		int bs = (int)g_PaintCtx.Core->brushSize() + d;
		if (bs < 0) bs = 0;
		if (bs > 2) bs = 2;
		g_PaintCtx.Core->setBrushSize((uint)bs);
	}
}

static void zpGroupDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	int g = (int)g_PaintCtx.Core->tileGroup() + d;
	g = ((g % 13) + 13) % 13;
	g_PaintCtx.Core->setTileGroup((uint)g);
}

/** Hardness ± (plugin ±0.2 on 0..1 → ±51 on 0..255). Keys Home/End + panel. */
static void zpHardnessDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	int v = (int)g_PaintCtx.Paint->BrushHardness + d;
	if (v < 0) v = 0;
	if (v > 255) v = 255;
	g_PaintCtx.Paint->BrushHardness = (uint)v;
}

/** Opacity ± (same step scale as hardness). Keys Insert/Delete + panel. */
static void zpOpacityDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	int v = (int)g_PaintCtx.Paint->BrushOpacity + d;
	if (v < 0) v = 0;
	if (v > 255) v = 255;
	g_PaintCtx.Paint->BrushOpacity = (uint)v;
}

/** Cycle brush mask: none → file1 → … → none (S key / panel). */
static void zpCycleBrushMask()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || g_MaskFiles.empty()) return;
	g_MaskCycle = (g_MaskCycle + 1) % ((int)g_MaskFiles.size() + 1);
	std::string err;
	if (g_MaskCycle == 0)
		g_PaintCtx.Core->clearBrushMask();
	else if (!g_PaintCtx.Core->loadBrushMask(g_MaskFiles[g_MaskCycle - 1], err))
		fprintf(stderr, "WARNING: %s\n", err.c_str());
}

/** Toggle color-brush mask mode (Q key / panel). */
static void zpToggleMaskMode()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	g_PaintCtx.Core->setBrushMaskMode(!g_PaintCtx.Core->brushMaskMode());
}

/** Displace paint index ±1 mod 16 ([ ] keys / panel). */
static void zpDisplaceIndexDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	int v = ((int)g_PaintCtx.Paint->DisplaceIndex + d) % 16;
	if (v < 0) v += 16;
	g_PaintCtx.Paint->DisplaceIndex = (uint)v;
}

/** Absolute displace index 0-15 (palette cell / panel action test); same field as [ ] keys. */
static void zpDisplaceIndexAbs(int idx)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (idx < 0) idx = 0;
	if (idx > 15) idx = 15;
	g_PaintCtx.Paint->DisplaceIndex = (uint)idx;
}

/** Brush color from the color picker (M9b); same field CLI --color initializes. */
static void zpSetBrushColor(int r, int g, int b)
{
	if (r < 0) r = 0; if (r > 255) r = 255;
	if (g < 0) g = 0; if (g > 255) g = 255;
	if (b < 0) b = 0; if (b > 255) b = 255;
	g_ViewerBrushColor = NLMISC::CRGBA((uint8)r, (uint8)g, (uint8)b, 255);
	if (g_PaintCtx.Paint)
		g_PaintCtx.Paint->BrushColor = g_ViewerBrushColor;
}

static void zpToggleLockBorders()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	g_PaintCtx.Core->setLockBorders(!g_PaintCtx.Core->lockBordersOn());
}

static void zpUndo()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	g_PaintCtx.Core->opUndo();
}

static void zpRedo()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core) return;
	g_PaintCtx.Core->opRedo();
}

static void zpFill(int rot)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	CPaintMouseListener &pl = *g_PaintCtx.Paint;
	if (!pl.HaveHover || g_PaintCtx.Core->zoneFrozen(pl.HoverZone)) return;
	std::string err;
	uint patch = (uint)(pl.HoverTile / ZP_NUM_TILE_SEL);
	if (pl.Mode == CPaintMouseListener::ModeTile)
		g_PaintCtx.Core->opFillTile(pl.HoverZone, patch, pl.CurTileSet, rot, pl.Mode256, err);
	else if (pl.Mode == CPaintMouseListener::ModeColor)
		g_PaintCtx.Core->opFillColor(pl.HoverZone, patch, pl.BrushColor, 256, err);
	else
		g_PaintCtx.Core->opFillDisplace(pl.HoverZone, patch, pl.DisplaceIndex, err);
}

/**
 * Top-down orthographic capture of the PRIMARY zone only (instances/neighbors stay in
 * the landscape but the camera frames the primary bbox). Square crop, no GUI.
 */
static bool captureTopDownThumbnail(NLMISC::CBitmap &out)
{
	out.reset();
	if (!g_PaintCtx.UDriver || !g_PaintCtx.UScene || !g_PaintCtx.Land || !g_PaintCtx.Camera
	    || !g_PaintCtx.Zones || g_PaintCtx.Zones->empty())
		return false;

	// Primary zone = index 0 (editable); build bbox from its patches only.
	const SPaintZone &primary = (*g_PaintCtx.Zones)[0];
	NLMISC::CAABBox bbox;
	bool init = false;
	for (size_t p = 0; p < primary.Patches.size(); ++p)
	{
		const NL3D::CBezierPatch &bp = primary.Patches[p].Patch;
		for (uint v = 0; v < 4; ++v)
		{
			if (!init) { bbox.setCenter(bp.Vertices[v]); bbox.setHalfSize(NLMISC::CVector::Null); init = true; }
			else bbox.extend(bp.Vertices[v]);
		}
		for (uint v = 0; v < 8; ++v) bbox.extend(bp.Tangents[v]);
		for (uint v = 0; v < 4; ++v) bbox.extend(bp.Interiors[v]);
	}
	if (!init)
		return false;

	NLMISC::CVector center = bbox.getCenter();
	// Square half-extent from XY footprint (Z is up)
	const float hx = std::max(bbox.getHalfSize().x, 1.f);
	const float hy = std::max(bbox.getHalfSize().y, 1.f);
	const float half = std::max(hx, hy) * 1.05f;
	const float zTop = bbox.getMax().z + std::max(bbox.getHalfSize().z * 2.f, 50.f);

	NL3D::CCamera *camera = g_PaintCtx.Camera;
	// Save camera
	const NLMISC::CMatrix oldMat = camera->getMatrix();
	const NL3D::CFrustum oldFrust = camera->getFrustum();

	// Ortho top-down: look along -Z, Y north-ish
	NLMISC::CMatrix camMat;
	camMat.identity();
	// I = +X, J = -Z (look down), K = +Y (up on screen = north if Y is north)
	NLMISC::CVector I(1, 0, 0);
	NLMISC::CVector J(0, 0, -1);
	NLMISC::CVector K(0, 1, 0);
	camMat.setRot(I, J, K, true);
	camMat.setPos(NLMISC::CVector(center.x, center.y, zTop));
	camera->setTransformMode(NL3D::ITransformable::DirectMatrix);
	camera->setMatrix(camMat);
	camera->setFrustum(-half, half, -half, half, 0.1f, zTop - bbox.getMin().z + 100.f, /*perspective=*/false);

	NL3D::UDriver *udriver = g_PaintCtx.UDriver;
	NL3D::UScene *uscene = g_PaintCtx.UScene;
	NL3D::IDriver *driver = static_cast<NL3D::CDriverUser *>(udriver)->getDriver();

	// Hide GUI for a clean thumb
	const bool wasUi = false;
	(void)wasUi;
	ZPUI::CEditorUI *ed = NULL;
	// Render two frames (refine)
	g_PaintCtx.Land->Landscape.setRefineMode(true);
	udriver->clearBuffers(NLMISC::CRGBA(40, 40, 40));
	uscene->render();
	g_PaintCtx.Land->Landscape.setRefineMode(false);
	g_PaintCtx.Land->Landscape.refineAll(camMat.getPos());
	udriver->clearBuffers(NLMISC::CRGBA(40, 40, 40));
	uscene->render();
	udriver->swapBuffers();

	NLMISC::CBitmap full;
	driver->getBuffer(full);
	// Restore camera
	camera->setMatrix(oldMat);
	camera->setFrustum(oldFrust);

	if (full.getWidth() == 0 || full.getHeight() == 0)
		return false;

	// Center square crop
	const uint fw = full.getWidth();
	const uint fh = full.getHeight();
	const uint side = std::min(fw, fh);
	const uint x0 = (fw - side) / 2;
	const uint y0 = (fh - side) / 2;
	out.resize(side, side, NLMISC::CBitmap::RGBA, true);
	if (full.getPixelFormat() != NLMISC::CBitmap::RGBA)
		full.convertToType(NLMISC::CBitmap::RGBA);
	out.blit(full, (sint)x0, (sint)y0, (sint)side, (sint)side, 0, 0);
	return out.getWidth() > 0;
}

/** Build optional SI override when WantThumbnail is set; no override means leave SI alone. */
static bool prepareThumbnailOverride(const std::string &srcMax, std::vector<uint8> &siOut, bool &haveOverride)
{
	haveOverride = false;
	siOut.clear();
	// Interactive modal: honor the "Update thumbnail" checkbox (default on).
	if (g_PaintCtx.InteractiveSave)
	{
		if (ZPUI::SPaintUIBridge *b = ZPUI::getPaintUIBridge())
			g_PaintCtx.WantThumbnail = b->UpdateThumbnail;
	}
	if (!g_PaintCtx.WantThumbnail && !g_CliWantThumbnail)
		return true; // not requested — OK, no override

	NLMISC::CBitmap bmp;
	if (captureTopDownThumbnail(bmp))
	{
		g_CapturedThumb.swap(bmp);
		g_HaveCapturedThumb = true;
	}
	else if (g_HaveCapturedThumb)
	{
		bmp = g_CapturedThumb;
	}
	else
	{
		fprintf(stderr, "WARNING: thumbnail: capture failed (need a display — use xvfb-run and "
		        "--screenshot or interactive save); SI left unchanged\n");
		return true; // save continues without thumb update
	}
	std::string err;
	if (!ZPTHUMB::buildSummaryInformationWithThumbnail(srcMax, bmp, siOut, 128, true, &err))
	{
		fprintf(stderr, "WARNING: thumbnail: %s — SI left unchanged\n", err.c_str());
		return true;
	}
	haveOverride = true;
	printf("OK thumbnail: %ux%u -> SummaryInformation\n", bmp.getWidth(), bmp.getHeight());
	return true;
}

/** Scene for one editable file (primary uses g_PaintCtx.Scene; extras keep their Lm). */
static PIPELINE::MAX::CScene *editableScene(const SEditableFileInfo &efi)
{
	if (efi.Lm)
		return efi.Lm->Scene;
	return g_PaintCtx.Scene;
}

/** Count dirty editable files (M6b panel indicator). */
static uint countDirtyEditableFiles()
{
	if (!g_PaintCtx.Core) return 0;
	uint n = 0;
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
		if (g_PaintCtx.Core->anyZoneDirty(g_EditableFiles[i].ZoneIds))
			++n;
	return n;
}

/**
 * Atomic overwrite of one path: temp → optional one-time .bak → rename.
 * Caller has already writeBack'd. Uses `src` for non-Scene OLE streams and `scene` for Scene.
 */
static bool saveOneOverwrite(const std::string &orig, PIPELINE::MAX::CScene &scene, bool doThumb)
{
	if (orig.empty() || !NLMISC::CFile::fileExists(orig))
	{
		fprintf(stderr, "ERROR: overwrite: original missing: %s\n", orig.c_str());
		return false;
	}
	std::string dir = NLMISC::CFile::getPath(orig);
	std::string base = NLMISC::CFile::getFilename(orig);
	std::string tempPath = dir + NLMISC::toString(".zone_painter_save_%d_%s.tmp",
	                                              (int)ZP_GETPID(), base.c_str());
	if (NLMISC::CFile::fileExists(tempPath))
		NLMISC::CFile::deleteFile(tempPath);

	std::vector<uint8> siOverride;
	bool haveSi = false;
	if (doThumb)
		prepareThumbnailOverride(orig, siOverride, haveSi);
	int saveRc = saveWholeFile(orig, tempPath, scene, false, haveSi ? &siOverride : NULL);
	if (saveRc != 0 || !NLMISC::CFile::fileExists(tempPath))
	{
		fprintf(stderr, "ERROR: overwrite: temp write failed for %s\n", orig.c_str());
		if (NLMISC::CFile::fileExists(tempPath))
			NLMISC::CFile::deleteFile(tempPath);
		return false;
	}
	std::string bakPath = orig + ".bak";
	if (!NLMISC::CFile::fileExists(bakPath))
	{
		if (!NLMISC::CFile::copyFile(bakPath, orig, /*failIfExists=*/true))
		{
			fprintf(stderr, "ERROR: overwrite: could not create %s\n", bakPath.c_str());
			NLMISC::CFile::deleteFile(tempPath);
			return false;
		}
		printf("OK backup -> %s\n", bakPath.c_str());
	}
	else
		printf("backup kept (already exists): %s\n", bakPath.c_str());

	if (!NLMISC::CFile::moveFile(orig, tempPath))
	{
		if (!NLMISC::CFile::copyFile(orig, tempPath, /*failIfExists=*/false)
		    || !NLMISC::CFile::deleteFile(tempPath))
		{
			fprintf(stderr, "ERROR: overwrite: rename/copy failed for %s (temp %s)\n",
			        orig.c_str(), tempPath.c_str());
			return false;
		}
	}
	if (NLMISC::CFile::fileExists(tempPath))
		NLMISC::CFile::deleteFile(tempPath);
	printf("OK save (overwrite) -> %s\n", orig.c_str());
	return true;
}

/**
 * Write-back + whole-file save to `target`. Single save implementation for panel modal,
 * --save, and --panel-save-test. Non-Scene OLE streams are read from InputPath (the opened
 * editable file); the Scene stream is rebuilt from the mutated Max scene.
 * Thumbnail write only when WantThumbnail/CLI --thumbnail is set (M5c).
 *
 * CLI multi-file (M6b): errors if more than one editable file is dirty (single-path --save).
 */
static bool zpSaveTo(const std::string &target)
{
	g_LastSaveStatus.clear();
	if (!g_PaintCtx.Core || !g_PaintCtx.Scene)
	{
		g_LastSaveStatus = "save: no paint core/scene";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	if (target.empty())
	{
		g_LastSaveStatus = "save: empty target path";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	// Multi-file: --save is single-path only
	if (g_EditableFiles.size() > 1)
	{
		uint dirty = countDirtyEditableFiles();
		if (dirty > 1)
		{
			g_LastSaveStatus = "save: multiple dirty editable files — use interactive save-all "
			                   "(Overwrite), not --save <one.path>";
			fprintf(stderr, "ERROR: %s (%u dirty of %u)\n",
			        g_LastSaveStatus.c_str(), dirty, (uint)g_EditableFiles.size());
			return false;
		}
	}
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_LastSaveStatus = "write-back: " + err;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	// M16b: board-session saves stamp neighbor-hints appdata (legacy --save does not)
	writeNeighborHintsIfBoardSession();
	// Pick the (only) dirty file's scene when multi; else primary
	const SEditableFileInfo *srcFile = NULL;
	if (!g_EditableFiles.empty())
	{
		for (size_t i = 0; i < g_EditableFiles.size(); ++i)
		{
			if (g_PaintCtx.Core->anyZoneDirty(g_EditableFiles[i].ZoneIds)
			    || g_EditableFiles.size() == 1)
			{
				srcFile = &g_EditableFiles[i];
				if (g_EditableFiles.size() > 1)
					break; // first dirty
			}
		}
		if (!srcFile)
			srcFile = &g_EditableFiles[0];
	}
	const std::string &srcForStreams = srcFile ? srcFile->Path
		: (g_PaintCtx.InputPath.empty() ? target : g_PaintCtx.InputPath);
	PIPELINE::MAX::CScene *scene = srcFile ? editableScene(*srcFile) : g_PaintCtx.Scene;
	std::vector<uint8> siOverride;
	bool haveSi = false;
	prepareThumbnailOverride(srcForStreams, siOverride, haveSi);
	int saveRc = saveWholeFile(srcForStreams, target, *scene, false,
	                           haveSi ? &siOverride : NULL);
	if (saveRc != 0)
	{
		g_LastSaveStatus = "save failed -> " + target;
		return false;
	}
	if (srcFile)
		g_PaintCtx.Core->markZonesSaved(srcFile->ZoneIds);
	g_LastSaveStatus = "OK save -> " + target;
	printf("OK save (panel) -> %s\n", target.c_str());
	return true;
}

/**
 * In-place overwrite: for multi-select (M6b) this is save-all — each dirty editable file
 * gets temp → one-time .bak → rename. Single-file path unchanged.
 */
static bool zpSaveOverwrite()
{
	g_LastSaveStatus.clear();
	if (!g_PaintCtx.Core || !g_PaintCtx.Scene)
	{
		g_LastSaveStatus = "overwrite: no paint core/scene";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}

	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_LastSaveStatus = "write-back: " + err;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	// M16b: board-session overwrite stamps neighbor-hints appdata
	writeNeighborHintsIfBoardSession();

	// Single-file legacy path when g_EditableFiles empty/one and only InputPath known
	if (g_EditableFiles.size() <= 1)
	{
		const std::string &orig = g_EditableFiles.empty() ? g_PaintCtx.InputPath
		                                                  : g_EditableFiles[0].Path;
		PIPELINE::MAX::CScene *scene = g_EditableFiles.empty() ? g_PaintCtx.Scene
		                                                       : editableScene(g_EditableFiles[0]);
		if (!saveOneOverwrite(orig, *scene, /*doThumb=*/true))
		{
			g_LastSaveStatus = "overwrite failed -> " + orig;
			return false;
		}
		if (!g_EditableFiles.empty())
			g_PaintCtx.Core->markZonesSaved(g_EditableFiles[0].ZoneIds);
		g_LastSaveStatus = "OK overwrite -> " + orig;
		return true;
	}

	// Multi: save each dirty file. Board sessions also rewrite clean editables when
	// neighbor hints changed (always write editables after stamp — paint dirty OR board).
	uint saved = 0, skipped = 0;
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		SEditableFileInfo &efi = g_EditableFiles[i];
		const bool dirty = g_PaintCtx.Core->anyZoneDirty(efi.ZoneIds);
		if (!dirty && !g_BoardSession)
		{
			++skipped;
			continue;
		}
		if (!efi.Editable)
		{
			++skipped;
			continue;
		}
		PIPELINE::MAX::CScene *scene = editableScene(efi);
		// Thumbnail only for the primary file (first)
		if (!saveOneOverwrite(efi.Path, *scene, /*doThumb=*/(i == 0)))
		{
			g_LastSaveStatus = "overwrite failed -> " + efi.Path;
			return false;
		}
		g_PaintCtx.Core->markZonesSaved(efi.ZoneIds);
		++saved;
	}
	if (saved == 0)
	{
		g_LastSaveStatus = "overwrite: nothing dirty";
		printf("save-all: nothing dirty (%u files)\n", (uint)g_EditableFiles.size());
		return true;
	}
	g_LastSaveStatus = NLMISC::toString("OK save-all: %u file(s) (%u clean)", saved, skipped);
	printf("%s\n", g_LastSaveStatus.c_str());
	return true;
}

/** Panel Save when --save was given: one-click direct write to SavePath (no modal). */
static void zpSaveDirect()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Scene) return;
	if (g_PaintCtx.SavePath.empty())
	{
		fprintf(stderr, "WARNING: save: no --save path given\n");
		return;
	}
	zpSaveTo(g_PaintCtx.SavePath);
}

/** Season cache tag for tileset previews (preference code, or "auto"). */
static std::string zpSeasonCacheKey()
{
	const std::string &pref = ZPCTX::seasonPreference();
	return pref.empty() ? std::string("auto") : pref;
}

/** Rebuild the Tiles + Displace palette grids (ui M8/M9a); no-op without a bank. */
static void zpRebuildTilesetPalette()
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

static void zpTogglePalette()
{
	ZPUI::toggleTilesetPalette();
}

static void zpToggleBoard()
{
	ZPUI::toggleSessionBoard();
}

// ---------------------------------------------------------------------------------------------
// Session working-set rebuild (M11a) — open/close/toggle mid-viewer.
//
// Sequence: writeBack retained paint → stash OriginalBytes → rebuild zones vector from kept
// SLoadedMax scenes (no reload of already-open files) → weld → core.init → restore
// OriginalBytes → reattach landscape (removeZone all + addZone). Undo is cleared by init().
// Closing frees the SLoadedMax ONLY after confirm flow resolves.

static SEditableFileInfo *findEditableByBasename(const std::string &basename)
{
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (NLMISC::toLowerAscii(g_EditableFiles[i].Basename)
		    == NLMISC::toLowerAscii(basename))
			return &g_EditableFiles[i];
	}
	return NULL;
}

static const ZPWS::SZoneEntry *findWorldZone(const std::string &basename)
{
	if (g_StartupWorld.MaxDir.empty())
		return NULL;
	static std::vector<ZPWS::SZoneEntry> s_listed;
	static std::string s_listedDir;
	if (s_listedDir != g_StartupWorld.MaxDir)
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
	return NULL;
}

/** Save one editable file in place (writeBack already done by caller optional). */
static bool sessionSaveOneFile(SEditableFileInfo &efi, std::string &err)
{
	if (!g_PaintCtx.Core)
	{
		err = "no paint core";
		return false;
	}
	// Board sessions always rewrite to stamp neighbor hints (even if paint clean).
	if (!g_PaintCtx.Core->anyZoneDirty(efi.ZoneIds) && !g_BoardSession)
	{
		err = "not dirty";
		return true; // no-op success
	}
	std::string wbErr;
	if (!g_PaintCtx.Core->writeBack(wbErr))
	{
		err = "write-back: " + wbErr;
		return false;
	}
	if (g_BoardSession)
		writeNeighborHintsIfBoardSession();
	PIPELINE::MAX::CScene *scene = editableScene(efi);
	if (!scene)
	{
		err = "no scene";
		return false;
	}
	if (!saveOneOverwrite(efi.Path, *scene, /*doThumb=*/false))
	{
		err = "overwrite failed";
		return false;
	}
	g_PaintCtx.Core->markZonesSaved(efi.ZoneIds);
	return true;
}

/**
 * Rebuild the painting landscape from current g_EditableFiles (+ continent ring).
 * Retains open SLoadedMax scenes; reloads nothing already open.
 * Returns weld count; fills err on hard failure.
 */
static bool rebuildWorkingSet(std::string &err, uint &outWelds)
{
	outWelds = 0;
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Zones || !g_PaintCtx.Land
	    || !g_SessionBank)
	{
		err = "session rebuild: viewer not live";
		return false;
	}
	// Preserve dirty OriginalBytes across re-init
	std::string wbErr;
	if (!g_PaintCtx.Core->writeBack(wbErr))
	{
		err = "write-back before rebuild: " + wbErr;
		return false;
	}
	std::map<const void *, std::vector<uint8> > originals;
	g_PaintCtx.Core->stashOriginalBytes(originals);

	// Snapshot previous zone ids for landscape remove
	std::vector<uint> oldIds;
	for (size_t i = 0; i < g_PaintCtx.Zones->size(); ++i)
		oldIds.push_back((*g_PaintCtx.Zones)[i].ZoneId);

	// Drop neighbor/context scenes (will re-load via hint chain); keep editable scenes
	clearContextFiles();

	// Rebuild zones vector
	std::vector<SPaintZone> &zones = *g_PaintCtx.Zones;
	zones.clear();
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
	}

	// Ecosystem scratch instances (M12c): re-append display clones after primary rebuild
	if (!g_Places.empty() && g_StartupWorld.Kind == ZPWS::Ecosystem && !g_EditableFiles.empty())
	{
		size_t primaryOnly = 0;
		for (size_t i = 0; i < zones.size(); ++i)
			if (zones[i].ZoneId < 1000) ++primaryOnly;
		if (primaryOnly > 0)
		{
			appendInstanceZones(zones, primaryOnly, g_Places, g_SessionCellSize);
			if (g_SessionBank)
				applyInstanceDisplayTiles(zones, g_SessionBank);
			g_InstanceCount = 1 + (uint)g_Places.size();
		}
	}

	// M16: neighbor/context via hint chain (appdata → embedded → continent grid)
	{
		std::vector<std::string> skip;
		for (size_t i = 0; i < g_EditableFiles.size(); ++i)
			skip.push_back(g_EditableFiles[i].Basename);
		loadNeighborContextFiles(zones, g_SessionCellSize, skip);
	}
	// M16c: re-apply place-context specs (CLI + scratch UI) after hint load
	for (size_t pci = 0; pci < g_PlaceContextSpecs.size(); ++pci)
	{
		std::string perr;
		if (!loadOnePlaceContext(zones, g_SessionCellSize, g_PlaceContextSpecs[pci], perr))
			fprintf(stderr, "WARNING: rebuild place-context: %s\n", perr.c_str());
	}

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
	                           g_SessionLockBorders, initErr))
	{
		err = "paint core re-init: " + initErr;
		return false;
	}
	g_PaintCtx.Core->restoreOriginalBytes(originals);
	g_PaintCtx.Core->attachLandscape(&g_PaintCtx.Land->Landscape);
	printf("session rebuild: undo cleared; retained dirty flags restored (%u carriers stashed)\n",
	       (uint)originals.size());
	return true;
}

// ---------------------------------------------------------------------------------------------
// Ecosystem scratch board callbacks (M12c / M14a multi-cell occupancy)

static int scratchFw() { return g_FootprintCellsW > 0 ? g_FootprintCellsW : 1; }
static int scratchFh() { return g_FootprintCellsH > 0 ? g_FootprintCellsH : 1; }

/** True if cell (cx,cy) lies in the primary home footprint [0,fw)×[0,fh). */
static bool scratchHomeOccupies(int cx, int cy)
{
	const int fw = scratchFw(), fh = scratchFh();
	return cx >= 0 && cy >= 0 && cx < fw && cy < fh;
}

/** True if place block (origin + transformed size) contains (cx,cy). */
static bool scratchPlaceOccupies(const SInstancePlace &pl, int cx, int cy)
{
	int bw = 0, bh = 0;
	footprintBlockSize(scratchFw(), scratchFh(), pl.Rot, pl.Mirror, bw, bh);
	return cx >= pl.CellX && cy >= pl.CellY && cx < pl.CellX + bw && cy < pl.CellY + bh;
}

/** Find place whose occupied block contains (cx,cy). */
static bool scratchFindPlace(int cx, int cy, size_t &idx)
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

/** Axis-aligned block overlap (half-open intervals). */
static bool scratchBlocksOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh)
{
	return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

/** True if a candidate block overlaps home or any place except skipIdx (-1 = none). */
static bool scratchBlockConflicts(int ox, int oy, int bw, int bh, int skipIdx, std::string &err)
{
	const int fw = scratchFw(), fh = scratchFh();
	if (scratchBlocksOverlap(ox, oy, bw, bh, 0, 0, fw, fh))
	{
		err = "block overlaps home footprint";
		return true;
	}
	for (size_t i = 0; i < g_Places.size(); ++i)
	{
		if ((int)i == skipIdx) continue;
		int pw = 0, ph = 0;
		footprintBlockSize(fw, fh, g_Places[i].Rot, g_Places[i].Mirror, pw, ph);
		if (scratchBlocksOverlap(ox, oy, bw, bh, g_Places[i].CellX, g_Places[i].CellY, pw, ph))
		{
			err = NLMISC::toString("block overlaps instance at (%d,%d)",
			                       g_Places[i].CellX, g_Places[i].CellY);
			return true;
		}
	}
	return false;
}

static bool scratchRebuild(std::string &err)
{
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds))
		return false;
	printf("scratch rebuild: %u place(s), footprint %dx%d, %u welds\n",
	       (uint)g_Places.size(), scratchFw(), scratchFh(), welds);
	return true;
}

/** Find place-context index covering / at cell. */
static bool scratchFindContext(int cx, int cy, size_t &idx)
{
	for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
	{
		// Context bricks claim a 1×1 cell at their place origin (footprint masks later)
		if (g_PlaceContextSpecs[i].Dx == cx && g_PlaceContextSpecs[i].Dy == cy)
		{
			idx = i;
			return true;
		}
	}
	return false;
}

static bool scratchGetCellState(const std::string &basename, ZPUI::ESessionCellState &out)
{
	// Multi-cell: basenames H:cx,cy / I:ox,oy@cx,cy / E:cx,cy / C:cx,cy:basename (M16c)
	if (basename == "H" || basename == "HOME" || (basename.size() >= 2 && basename[0] == 'H' && basename[1] == ':'))
	{
		out = g_PaintCtx.Core && !g_EditableFiles.empty()
		          && g_PaintCtx.Core->anyZoneDirty(g_EditableFiles[0].ZoneIds)
		      ? ZPUI::CellDirtyEditable
		      : ZPUI::CellScratchHome;
		return true;
	}
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
	if (basename.size() >= 4 && basename[0] == 'E' && basename[1] == ':')
	{
		// Empty unless a place-context occupies this cell
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
		}
		out = ZPUI::CellScratchEmpty;
		return true;
	}
	return false;
}

/** Load one place-context brick into zones (shared by initial load + rebuild + UI place). */
static bool loadOnePlaceContext(std::vector<SPaintZone> &zones, float cellSize,
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
		delete nlm;
		err = "no zones in " + ze.Basename;
		return false;
	}
	float homeOriginX = 0.f, homeOriginY = 0.f, stepX = 0.f, stepY = 0.f;
	int fw = 1, fh = 1;
	size_t primaryEnd = 0;
	for (size_t i = 0; i < zones.size(); ++i)
		if (zones[i].ZoneId < 1000) primaryEnd = i + 1;
	if (primaryEnd > 0)
		computeFootprintRect(zones, 0, primaryEnd, cellSize,
		                     homeOriginX, homeOriginY, stepX, stepY, fw, fh);
	float cOx = 0.f, cOy = 0.f, cSx = 0.f, cSy = 0.f;
	int cw = 1, ch = 1;
	computeFootprintRect(zones, before, zones.size(), cellSize, cOx, cOy, cSx, cSy, cw, ch);
	const float wantX = homeOriginX + (float)pc.Dx * cellSize;
	const float wantY = homeOriginY + (float)pc.Dy * cellSize;
	translateZonesXY(zones, before, zones.size(), wantX - cOx, wantY - cOy);
	SContextFile cf;
	cf.Path = ze.MaxPath;
	cf.Basename = ze.Basename;
	cf.CellX = pc.Dx;
	cf.CellY = pc.Dy;
	cf.TranslateGeom = true;
	cf.Lm = nlm;
	g_ContextFiles.push_back(cf);
	g_NeighborScenes.push_back(nlm);
	printf("place-context: '%s' @ (%d,%d) FROZEN translated\n",
	       cf.Basename.c_str(), cf.CellX, cf.CellY);
	return true;
}

static bool scratchPlaceContext(int cx, int cy, const std::string &basename, std::string &err)
{
	if (scratchHomeOccupies(cx, cy))
	{
		err = "cannot place context on home";
		return false;
	}
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
	SPlaceContextSpec pc;
	pc.Dx = cx;
	pc.Dy = cy;
	pc.Basename = basename;
	// strip .max
	std::string::size_type dot = pc.Basename.rfind('.');
	if (dot != std::string::npos && NLMISC::toLowerAscii(pc.Basename.substr(dot)) == ".max")
		pc.Basename = pc.Basename.substr(0, dot);
	g_PlaceContextSpecs.push_back(pc);
	if (!scratchRebuild(err))
	{
		g_PlaceContextSpecs.pop_back();
		return false;
	}
	return true;
}

static bool scratchRemoveContext(int cx, int cy, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindContext(cx, cy, idx))
	{
		err = "no context at cell";
		return false;
	}
	g_PlaceContextSpecs.erase(g_PlaceContextSpecs.begin() + (std::ptrdiff_t)idx);
	return scratchRebuild(err);
}

static bool scratchGetContext(int cx, int cy, std::string &basename)
{
	size_t idx = 0;
	if (!scratchFindContext(cx, cy, idx))
		return false;
	basename = g_PlaceContextSpecs[idx].Basename;
	return true;
}

static bool scratchPlace(int cx, int cy, std::string &err)
{
	if (scratchHomeOccupies(cx, cy))
	{
		err = "cannot place on home footprint";
		return false;
	}
	size_t idx = 0;
	if (scratchFindPlace(cx, cy, idx))
	{
		err = "cell already occupied";
		return false;
	}
	const int fw = scratchFw(), fh = scratchFh();
	// Place origin at the clicked cell, rot 0 → occupies [cx,cx+fw)×[cy,cy+fh)
	if (scratchBlockConflicts(cx, cy, fw, fh, -1, err))
		return false;
	g_Places.push_back(SInstancePlace(cx, cy, 0, false));
	g_InstanceCount = 1 + (uint)g_Places.size();
	return scratchRebuild(err);
}

/**
 * Rotate about the instance's footprint-block center (M14a): update Rot and recompute
 * origin so it remains the min-corner of the transformed block; refuse if the new
 * block would overlap home or another instance.
 */
static bool scratchRotate(int cx, int cy, int delta, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
	{
		err = "no instance at cell";
		return false;
	}
	SInstancePlace pl = g_Places[idx];
	const int fw = scratchFw(), fh = scratchFh();
	int oldW = 0, oldH = 0;
	footprintBlockSize(fw, fh, pl.Rot, pl.Mirror, oldW, oldH);
	const double ctrX = (double)pl.CellX + 0.5 * (double)oldW;
	const double ctrY = (double)pl.CellY + 0.5 * (double)oldH;
	const uint newRot = (uint)((int)pl.Rot + delta) & 3;
	int newW = 0, newH = 0;
	footprintBlockSize(fw, fh, newRot, pl.Mirror, newW, newH);
	const int newOx = (int)std::floor(ctrX - 0.5 * (double)newW + 1e-9);
	const int newOy = (int)std::floor(ctrY - 0.5 * (double)newH + 1e-9);
	if (scratchBlockConflicts(newOx, newOy, newW, newH, (int)idx, err))
		return false;
	g_Places[idx].Rot = newRot;
	g_Places[idx].CellX = newOx;
	g_Places[idx].CellY = newOy;
	return scratchRebuild(err);
}

static bool scratchMirror(int cx, int cy, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
	{
		err = "no instance at cell";
		return false;
	}
	// Mirror alone keeps the axis-aligned cell AABB; only the geometry flips.
	g_Places[idx].Mirror = !g_Places[idx].Mirror;
	return scratchRebuild(err);
}

static bool scratchRemove(int cx, int cy, std::string &err)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
	{
		err = "no instance at cell";
		return false;
	}
	g_Places.erase(g_Places.begin() + idx);
	g_InstanceCount = 1 + (uint)g_Places.size();
	return scratchRebuild(err);
}

static bool scratchGetInstance(int cx, int cy, uint &rot, bool &mirror)
{
	size_t idx = 0;
	if (!scratchFindPlace(cx, cy, idx))
		return false;
	rot = g_Places[idx].Rot;
	mirror = g_Places[idx].Mirror;
	return true;
}

/** Resolve any cell in a block to the place origin (for board labels / popups). */
static bool scratchGetInstanceOrigin(int cx, int cy, int &ox, int &oy, uint &rot, bool &mirror)
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

static bool sessionGetCellState(const std::string &basename, ZPUI::ESessionCellState &out)
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
	// Neighbor-only open? Neighbors are not listed by basename in g_EditableFiles;
	// check live zones for force-frozen files matching basename (name match on zone).
	if (g_PaintCtx.Zones)
	{
		for (size_t i = 0; i < g_PaintCtx.Zones->size(); ++i)
		{
			// Neighbor zones use node names, not file basenames — check file via open set only.
			(void)i;
		}
	}
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

static bool sessionIsDirty(const std::string &basename)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	if (!efi || !efi->Editable || !g_PaintCtx.Core) return false;
	return g_PaintCtx.Core->anyZoneDirty(efi->ZoneIds);
}

static bool sessionIsOpen(const std::string &basename)
{
	ZPUI::ESessionCellState st = ZPUI::CellClosed;
	sessionGetCellState(basename, st);
	return st != ZPUI::CellClosed;
}

static bool sessionIsEditable(const std::string &basename)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	return efi && efi->Editable;
}

static bool sessionOpenZone(const std::string &basename, std::string &err)
{
	if (findEditableByBasename(basename))
	{
		err = "already open";
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
		// rollback
		g_EditableFiles.pop_back();
		g_ExtraEditableScenes.pop_back();
		delete extra;
		return false;
	}
	printf("session open: '%s' editable; welds=%u\n", basename.c_str(), welds);
	return true;
}

static bool sessionCloseZone(const std::string &basename, bool saveFirst, bool forceDiscard, std::string &err)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	// Closing a pure neighbor is a no-op (ring is automatic)
	if (!efi)
	{
		err = "not an opened file: " + basename;
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
		else
		{
			// Abandon paint on this file before rebuild writeBack encodes pristine→carrier
			g_PaintCtx.Core->revertZones(efi->ZoneIds);
		}
	}
	// Must keep at least one editable file
	uint nEditable = 0;
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
		if (g_EditableFiles[i].Editable) ++nEditable;
	if (efi->Editable && nEditable <= 1)
	{
		err = "cannot close the last editable zone";
		return false;
	}
	// Remove from lists; free scene if heap-owned (extra). Primary stack lm stays alive
	// until process exit but is dropped from the working set (not re-assembled).
	PMAXLOAD::SLoadedMax *toFree = efi->Lm;
	const bool wasPrimary = (toFree == NULL);
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		if (&g_EditableFiles[i] == efi)
		{
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
		delete toFree;
	}
	// Point g_PaintCtx.Scene at a remaining editable after primary close
	if (wasPrimary && !g_EditableFiles.empty())
	{
		PIPELINE::MAX::CScene *sc = editableScene(g_EditableFiles[0]);
		if (sc)
			g_PaintCtx.Scene = sc;
		g_PaintCtx.InputPath = g_EditableFiles[0].Path;
	}
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds))
		return false;
	printf("session close: '%s'; welds=%u\n", basename.c_str(), welds);
	return true;
}

static bool sessionSaveZone(const std::string &basename, std::string &err)
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

static bool sessionToggleEditable(const std::string &basename, bool saveFirst, bool forceDiscard,
                                  std::string &err)
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
			else
				g_PaintCtx.Core->revertZones(efi->ZoneIds);
		}
		// Keep at least one editable
		uint nEditable = 0;
		for (size_t i = 0; i < g_EditableFiles.size(); ++i)
			if (g_EditableFiles[i].Editable) ++nEditable;
		if (nEditable <= 1)
		{
			err = "cannot demote the last editable zone";
			return false;
		}
		efi->Editable = false;
	}
	else
	{
		// → editable (re-registers carriers as unfrozen)
		efi->Editable = true;
	}
	uint welds = 0;
	if (!rebuildWorkingSet(err, welds))
		return false;
	printf("session toggle: '%s' now %s; welds=%u\n",
	       basename.c_str(), efi->Editable ? "editable" : "read-only", welds);
	return true;
}

/** Apply current season preference + live-flush landscape tile textures (paint state untouched). */
static void zpSeasonApplyFlush()
{
	printf("season: %s\n", ZPCTX::seasonPreferenceLabel().c_str());
	if (!g_PaintCtx.Bank || !g_PaintCtx.Land || !g_PaintCtx.UDriver)
		return;
	NL3D::IDriver *driver = static_cast<NL3D::CDriverUser *>(g_PaintCtx.UDriver)->getDriver();
	// Live flush (preferred over full session reload): re-remap CPath season postfixes,
	// releaseTiles so CTextureFile re-looks up, optional preload flush.
	ZPCTX::reloadLandscapeSeasonTextures(*g_PaintCtx.Bank, g_PaintCtx.BankPath,
	                                     &g_PaintCtx.Land->Landscape, driver, g_PreloadTiles);
	// Palette previews follow the season (ui M8): regenerate/invalidate cached thumbs.
	zpRebuildTilesetPalette();
}

/** Cycle season preference + live-flush (Y key / SeasonNext). */
static void zpSeasonNext()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.AvailableSeasons || g_PaintCtx.AvailableSeasons->size() < 2)
		return;
	if (!ZPCTX::cycleSeasonPreference(*g_PaintCtx.AvailableSeasons))
		return;
	zpSeasonApplyFlush();
}

/** Select a specific season code (toolbar menu, M14c). */
static void zpSeasonSelect(const std::string &code)
{
	if (!g_PaintCtx.Active)
		return;
	if (!ZPCTX::setSeasonPreference(code))
	{
		fprintf(stderr, "season select: unknown code '%s'\n", code.c_str());
		return;
	}
	// If available list is non-empty, require membership (or empty = auto)
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
	zpSeasonApplyFlush();
}

/** Show season picker buttons for discovered seasons (M14c modal s0..s3). */
static void zpSeasonMenuFill(void * /*unused*/)
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
			// params_l already set in XML to sp/su/au/wi in order — remap if discovery order differs
		}
		// Rebind onclick params to actual code
		if (NLGUI::CCtrlBaseButton *btn = dynamic_cast<NLGUI::CCtrlBaseButton *>(el))
		{
			btn->setActionOnLeftClick("zp_season_select");
			btn->setParamsOnLeftClick(code);
		}
	}
}

// Fill the UI bridge state snapshot (labels / button push state).
static void zpFillBridgeState(ZPUI::SPaintUIBridge &bridge)
{
	bridge.HaveCore = g_PaintCtx.Active && g_PaintCtx.Core && g_PaintCtx.Paint;
	if (!bridge.HaveCore) return;
	CPaintMouseListener &pl = *g_PaintCtx.Paint;
	bridge.Mode = pl.Mode;
	bridge.CurTileSet = pl.CurTileSet;
	bridge.TileSetCount = g_PaintCtx.Core->tileSetCount();
	std::string name = g_PaintCtx.Core->tileSetName(pl.CurTileSet);
	// smallbanks often store empty set names: derive from first 128 diffuse stem
	// (y-plages-128-a-01.png → plages) so the panel matches the Tiles palette (ui M8).
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
	bridge.InstanceCount = g_InstanceCount;
	{
		std::string base = NLMISC::CFile::getFilenameWithoutExtension(g_PaintCtx.InputPath);
		strncpy(bridge.EditableBasename, base.c_str(), sizeof(bridge.EditableBasename) - 1);
		bridge.EditableBasename[sizeof(bridge.EditableBasename) - 1] = 0;
		std::string dir = NLMISC::CFile::getPath(g_PaintCtx.InputPath);
		strncpy(bridge.InputDir, dir.c_str(), sizeof(bridge.InputDir) - 1);
		bridge.InputDir[sizeof(bridge.InputDir) - 1] = 0;
	}
	// Season (M6a)
	{
		std::string lab = ZPCTX::seasonPreferenceLabel();
		strncpy(bridge.SeasonLabel, lab.c_str(), sizeof(bridge.SeasonLabel) - 1);
		bridge.SeasonLabel[sizeof(bridge.SeasonLabel) - 1] = 0;
		bridge.SeasonCount = g_PaintCtx.AvailableSeasons
			? (uint)g_PaintCtx.AvailableSeasons->size() : 0;
	}
	// Multi-file dirty (M6b)
	bridge.EditableFileCount = g_EditableFiles.empty() ? 1u : (uint)g_EditableFiles.size();
	bridge.DirtyFileCount = countDirtyEditableFiles();
	// Color / displace (M7a)
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
		// (HUD still shows "off" when mask mode is disabled — different concern.)
		std::string lab = "none";
		if (!g_PaintCtx.Core->brushMaskName().empty())
			lab = NLMISC::CFile::getFilename(g_PaintCtx.Core->brushMaskName());
		strncpy(bridge.BrushMaskLabel, lab.c_str(), sizeof(bridge.BrushMaskLabel) - 1);
		bridge.BrushMaskLabel[sizeof(bridge.BrushMaskLabel) - 1] = 0;
	}
}

// Shared viewer host: when externalDriver is non-NULL, runViewer uses it and does not
// create/release the driver (startup flow owns one UDriver for screens + viewer). When
// externalEditorUI is non-NULL it is reused (already init'd); otherwise a local CEditorUI
// is constructed and shut down here. Headless modes never call runViewer.
static int runViewer(std::vector<SPaintZone> &zones, NL3D::CTileBank &bank, ZPPAINT::CPaintCore *core,
                     PMAXLOAD::SLoadedMax &lm,
                     const std::string &screenshotPath, const std::string &fontPath,
                     const std::string &scriptPath, const std::string &savePath,
                     NL3D::UDriver *externalDriver = NULL,
                     ZPUI::CEditorUI *externalEditorUI = NULL)
{
	// Landscape bbox in world space (camera + mouse hotspot).
	NLMISC::CAABBox bbox;
	bool bboxInit = false;
	for (size_t i = 0; i < zones.size(); ++i)
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
		// Window title: zone_painter — <editable zone(s)> (+N neighbors / xN instances)
		{
			std::string title = "zone_painter";
			if (g_EditableFiles.size() > 1)
			{
				title += " — ";
				for (size_t i = 0; i < g_EditableFiles.size() && i < 4; ++i)
				{
					if (i) title += "+";
					title += g_EditableFiles[i].Basename;
				}
				if (g_EditableFiles.size() > 4)
					title += NLMISC::toString("+%u", (uint)(g_EditableFiles.size() - 4));
			}
			else if (!g_StartupZone.Basename.empty())
				title += " — " + g_StartupZone.Basename;
			else if (!g_InputPath.empty())
				title += " — " + NLMISC::CFile::getFilename(g_InputPath);
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

		// Camera looking at the bbox center from a sensible distance (the plugin inherited the
		// Max viewport matrix; standalone starts from a canonical three-quarter view).
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
		if (ownsEditorUI)
			editorUI->init(udriver, fontPath);
		// Ensure the Painter panel is active for the viewer session (startup screens hide it).
		editorUI->setVisible(true);
		ZPUI::startupHideAllScreens();
		ZPUI::startupShowPainter(true);

		// Mouse listener: edit3d orbiting the landscape center (the plugin's hotspot was the
		// selection center).
		NL3D::CEvent3dMouseListener mouseListener;
		mouseListener.setMatrix(camMat);
		mouseListener.setFrustrum(camera->getFrustum());
		mouseListener.setViewport(viewport);
		mouseListener.setHotSpot(center);
		mouseListener.setMouseMode(NL3D::CEvent3dMouseListener::edit3d);
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
		paintBridge.seasonNext = zpSeasonNext;
		paintBridge.seasonSelect = zpSeasonSelect;
		paintBridge.seasonMenuFill = zpSeasonMenuFill;
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
		ZPUI::setPaintUIBridge(&paintBridge);

		// M11a/M12c session board bridge (continent working set / ecosystem scratch)
		ZPUI::SSessionBoardBridge sessionBridge;
		if (!g_StartupWorld.MaxDir.empty()
		    && (g_StartupWorld.Kind == ZPWS::Continent || g_StartupWorld.Kind == ZPWS::Ecosystem))
		{
			sessionBridge.World = &g_StartupWorld;
			if (g_StartupWorld.Kind == ZPWS::Continent)
			{
				sessionBridge.getCellState = sessionGetCellState;
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
				sessionBridge.ScratchHomeName = g_StartupZone.Basename.empty()
					? g_StartupWorld.WorldName
					: g_StartupZone.Basename;
				sessionBridge.FootprintCellsW = scratchFw();
				sessionBridge.FootprintCellsH = scratchFh();
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
			paintListener.Nav = &mouseListener;
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
			udriver->EventServer.addListener(NLMISC::EventKeyDownId, &paintListener);
			// Preload flush (plugin preloadTiles): all tile sets into the driver
			if (g_PreloadTiles)
				core->preloadTiles(driver);
			// Tileset thumbnail palette (ui M8) — bank textures already resolved
			zpRebuildTilesetPalette();
		}

		// Scene point lights (CPaintLight parity — unconditional in the plugin's myThread)
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

		// HUD text (any TrueType through the font manager; silently disabled without a font)
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

		// Scripted pre-pass (the ops mirror straight into the attached live landscape)
		g_ViewerScriptRc = 0;
		if (core && !scriptPath.empty())
			g_ViewerScriptRc = runPaintScript(*core, scriptPath);

		if (!screenshotPath.empty())
		{
			// One refined frame -> .tga -> exit (the visual gate). GUI is drawn into this
			// frame so M1b+ screenshots show the Painter panel over the terrain.
			udriver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
			uscene->render();
			theLand->Landscape.setRefineMode(false);
			theLand->Landscape.refineAll(pos);
			udriver->clearBuffers(NLMISC::CRGBA(90, 90, 90));
			uscene->render();
			// Dev-only: ZONE_PAINTER_SHOT_MODE=color|displace selects mode for panel shots (M7a).
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
			// so interactive save UI can be verified headlessly (M3a).
			{
				const char *modalShot = getenv("ZONE_PAINTER_SAVE_MODAL_SHOT");
				if (modalShot && modalShot[0] && modalShot[0] != '0'
				    && g_PaintCtx.InteractiveSave)
				{
					ZPUI::forceShowSaveDialogForShot();
				}
			}
			// Dev-only: ZONE_PAINTER_PALETTE_SHOT=1 opens the Tiles palette for the frame (M8/M9a).
			// ZONE_PAINTER_PALETTE_SCROLL=disp scrolls the body to the Displace section.
			{
				const char *palShot = getenv("ZONE_PAINTER_PALETTE_SHOT");
				if (palShot && palShot[0] && palShot[0] != '0')
					ZPUI::setTilesetPaletteVisible(true);
				const char *palScroll = getenv("ZONE_PAINTER_PALETTE_SCROLL");
				if (palScroll && (strcmp(palScroll, "disp") == 0 || strcmp(palScroll, "displace") == 0))
					ZPUI::scrollPaletteToDisplaceSection();
			}
			// Dev-only: ZONE_PAINTER_COLOR_PICKER_SHOT=1 opens the brush color picker (M9b).
			{
				const char *cpShot = getenv("ZONE_PAINTER_COLOR_PICKER_SHOT");
				if (cpShot && cpShot[0] && cpShot[0] != '0')
					ZPUI::forceShowColorPickerForShot();
			}
			// Dev-only: ZONE_PAINTER_SEASON_NEXT=1 cycles season once before capture (M8 season-follow).
			{
				const char *sn = getenv("ZONE_PAINTER_SEASON_NEXT");
				if (sn && sn[0] && sn[0] != '0')
					zpSeasonNext();
			}
			// M11a/M12c: session board env hooks. ZONE_PAINTER_SHOW_BOARD=1 opens the board.
			// Continent: BOARD_ACTION=open|close|save|toggle:BASE
			// Ecosystem: BOARD_ACTION=place:cx,cy | rotate:cx,cy | mirror:cx,cy | remove:cx,cy
			// CLOSE_CONFIRM_SHOT / CELL_ACTION_SHOT / INST_ACTION_SHOT force modals for screenshots.
			{
				const char *showB = getenv("ZONE_PAINTER_SHOW_BOARD");
				if (showB && showB[0] && showB[0] != '0')
					ZPUI::setSessionBoardVisible(true);
				const char *bact = getenv("ZONE_PAINTER_BOARD_ACTION");
				if (bact && bact[0])
				{
					std::string act(bact);
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
					         || op == "place-context" || op == "remove-context")
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
							}
							else
								err = "expected cx,cy";
						}
					}
					else
						err = "unknown BOARD_ACTION (open|close|save|toggle:base / place|rotate|mirror|remove:cx,cy / place-context:cx,cy:base / remove-context:cx,cy)";
					printf("board-action %s: %s%s%s\n", act.c_str(), ok ? "OK" : "FAIL",
					       err.empty() ? "" : " ", err.c_str());
					if (ok)
						ZPUI::refreshSessionBoardStates();
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
				// M14c: open season picker modal for one screenshot frame
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
			editorUI->update();
			editorUI->draw();
			udriver->swapBuffers();
			NLMISC::CBitmap btm;
			driver->getBuffer(btm);
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
			if (g_CliWantThumbnail || g_PaintCtx.WantThumbnail)
			{
				NLMISC::CBitmap thumb;
				if (captureTopDownThumbnail(thumb))
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
				// ESC: close session board first (M11a); else quit viewer
				if (udriver->AsyncListener.isKeyPushed(NLMISC::KeyESCAPE))
				{
					if (ZPUI::isSessionBoardVisible())
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
				// P / TogglePalette: show/hide the tileset thumbnail palette (ui M8)
				if (zpKeyPushed(ZPK_TogglePalette))
					zpTogglePalette();
				// O / ToggleBoard: continent working set / ecosystem scratch board
				if (zpKeyPushed(ZPK_ToggleBoard))
					zpToggleBoard();

				// Tile set / mode / brush keys → shared named handlers (same as NLGUI buttons).
				// PgUp/PgDn + 0-9 and [ ] displace stay hardcoded; rebindable actions ride the
				// key table (see kPainterKeysName / --keys-cfg).
				if (core)
				{
					uint count = core->tileSetCount();
					if (count)
					{
						if (udriver->AsyncListener.isKeyPushed(NLMISC::KeyPRIOR))
							zpSelectTileSetDelta(-1);
						if (udriver->AsyncListener.isKeyPushed(NLMISC::KeyNEXT))
							zpSelectTileSetDelta(+1);
						for (int k = 0; k <= 9; ++k)
							if (udriver->AsyncListener.isKeyPushed((NLMISC::TKey)(NLMISC::Key0 + k)) && k < (int)count)
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
					if (zpKeyPushed(ZPK_SizeUp))
						zpBrushSizeDelta(+1);
					if (zpKeyPushed(ZPK_SizeDown))
						zpBrushSizeDelta(-1);
					if (zpKeyPushed(ZPK_GroupUp))
						zpGroupDelta(+1);
					if (zpKeyPushed(ZPK_GroupDown))
						zpGroupDelta(-1);
					// Displace / hardness / opacity / mask: shared handlers (keys + panel M7a)
					if (udriver->AsyncListener.isKeyPushed(NLMISC::KeyLBRACKET))
						zpDisplaceIndexDelta(-1);
					if (udriver->AsyncListener.isKeyPushed(NLMISC::KeyRBRACKET))
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
					// Window title dirty mark (M6b): append " *" when any editable is dirty
					{
						std::string title = "zone_painter";
						if (g_EditableFiles.size() > 1)
						{
							title += " — ";
							for (size_t i = 0; i < g_EditableFiles.size() && i < 4; ++i)
							{
								if (i) title += "+";
								title += g_EditableFiles[i].Basename;
							}
							if (g_EditableFiles.size() > 4)
								title += NLMISC::toString("+%u", (uint)(g_EditableFiles.size() - 4));
						}
						else if (!g_StartupZone.Basename.empty())
							title += " — " + g_StartupZone.Basename;
						else if (!g_InputPath.empty())
							title += " — " + NLMISC::CFile::getFilename(g_InputPath);
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
				if (theLand->Landscape.getRefineMode())
				{
					theLand->Landscape.setRefineMode(false);
					theLand->Landscape.refineAll(camKey.getPos());
				}

				// Hovered tile outline (world-space lines after the scene render)
				if (core && paintListener.HaveHover)
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
					static const char *modeNames[3] = { "TILE", "COLOR", "DISPLACE" };
					textContext.setColor(NLMISC::CRGBA(255, 255, 255));
					// HUD matches the Painter panel: 1-based set index; name from bridge
					// (includes diffuse-stem fallback when smallbank set names are empty).
					{
						const char *tsName = paintBridge.TileSetName[0]
							? paintBridge.TileSetName : "(unnamed)";
						const uint tsCount = paintBridge.TileSetCount
							? paintBridge.TileSetCount : core->tileSetCount();
						const int tsOneBased = tsCount ? (paintListener.CurTileSet + 1) : 0;
						textContext.printfAt(0.01f, 0.98f, "[%s] TileSet %d/%u %s  %s  brush %u  group %u  undo %u%s  %s",
						                     modeNames[paintListener.Mode % 3],
						                     tsOneBased, tsCount, tsName,
						                     paintListener.Mode256 ? "256" : "128", core->brushSize(),
						                     core->tileGroup(), core->undoDepth(),
						                     core->lockBordersOn() ? "  LOCK" : "",
						                     paintBridge.SeasonLabel[0] ? paintBridge.SeasonLabel : "auto");
					}
					textContext.printfAt(0.01f, 0.01f, "T/C/D mode  O board  Y season  P tiles  F10 UI  ESC");
					if (paintListener.Mode == CPaintMouseListener::ModeColor)
						textContext.printfAt(0.01f, 0.955f, "color %02x%02x%02x  radius %.1fm  hardness %u  opacity %u  mask %s",
						                     paintListener.BrushColor.R, paintListener.BrushColor.G, paintListener.BrushColor.B,
						                     paintListener.BrushRadius, paintListener.BrushHardness, paintListener.BrushOpacity,
						                     core->brushMaskMode() ? core->brushMaskName().c_str() : "off");
					else if (paintListener.Mode == CPaintMouseListener::ModeDisplace)
						textContext.printfAt(0.01f, 0.955f, "displace index %u", paintListener.DisplaceIndex);
					if (g_InstanceCount > 1)
						textContext.printfAt(0.01f, 0.905f, "INSTANCED x%u  (%ux%u layout; shared paint backing)",
						                     g_InstanceCount, g_InstanceCols, g_InstanceRows);
					if (paintListener.HaveHover)
					{
						sint32 t = paintListener.HoverTile;
						textContext.printfAt(0.01f, 0.93f, "zone %u patch %d tile (%d,%d)%s",
						                     paintListener.HoverZone, (int)(t / 256), (int)(t % 256 % 16), (int)(t % 256 / 16),
						                     core->zoneFrozen(paintListener.HoverZone) ? " FROZEN" : "");
					}
					// Brush color swatch
					if (paintListener.Mode == CPaintMouseListener::ModeColor)
						NL3D::CDRU::drawQuad(0.30f, 0.955f, 0.32f, 0.975f, *driver,
						                     paintListener.BrushColor, viewport);
				}

				// NLGUI over the 3D scene (after scene/HUD, before swap)
				editorUI->update();
				editorUI->draw();

				udriver->swapBuffers();
				zpViewerScreenshot(driver, udriver->AsyncListener); // F12 convenience
			}
			while (!viewerQuit && closeListener.WindowActive && udriver->isActive());
		}

		if (ownsEditorUI)
			editorUI->shutdown();
		ZPUI::setSessionBoardBridge(NULL);
		ZPUI::setSessionBoardVisible(false);
		ZPUI::setPaintUIBridge(NULL);
		g_SessionBank = NULL;
		g_PaintCtx = SPaintCtx();
		mouseListener.removeFromServer(udriver->EventServer);
		udriver->EventServer.removeListener(NLMISC::EventDestroyWindowId, &closeListener);
		udriver->EventServer.removeListener(NLMISC::EventCloseWindowId, &closeListener);
		if (core)
		{
			core->attachLandscape(NULL);
			udriver->EventServer.removeListener(NLMISC::EventMouseDownId, &paintListener);
			udriver->EventServer.removeListener(NLMISC::EventMouseUpId, &paintListener);
			udriver->EventServer.removeListener(NLMISC::EventMouseMoveId, &paintListener);
			udriver->EventServer.removeListener(NLMISC::EventKeyDownId, &paintListener);
		}
		// Drop the painting UScene; the shared driver (startup flow) keeps its display.
		if (uscene)
		{
			udriver->deleteScene(uscene);
			uscene = NULL;
		}
		g_ViewerAsync = NULL;
		if (ownsDriver)
		{
			udriver->release();
			delete udriver;
			udriver = NULL;
		}
	}
	catch (const NL3D::EDru &e)
	{
		ZPUI::setPaintUIBridge(NULL);
		g_PaintCtx = SPaintCtx();
		g_ViewerAsync = NULL;
		if (ownsDriver && udriver) { udriver->release(); delete udriver; }
		fprintf(stderr, "ERROR: 3D driver: %s\n", e.what());
		return 1;
	}
	catch (const NLMISC::Exception &e)
	{
		ZPUI::setPaintUIBridge(NULL);
		g_PaintCtx = SPaintCtx();
		g_ViewerAsync = NULL;
		if (ownsDriver && udriver) { udriver->release(); delete udriver; }
		fprintf(stderr, "ERROR: %s\n", e.what());
		return 1;
	}
	return g_ViewerScriptRc;
}

// ---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext applicationContext;

	NLMISC::CCmdArgs args;
	args.setDescription("Standalone zone painter (design doc \xc2\xa7" "14-paint). Default mode opens the "
	                    "painting landscape viewer; the headless modes need no 3D driver.\n"
	                    "Startup (no .max argument): discovers graphics workspaces (ecosystem ligo roots and\n"
	                    "continent-style max/zones layouts) by walking up from cwd for a .nel NeL root, or from\n"
	                    "an optional folder argument, then opens the in-engine world/zone picker. Remembered\n"
	                    "last folder lives in the app config dir (startup.cfg); no .nel directory is created.\n"
	                    "Continent grid names: bare <row>_<AA> (e.g. 4_AC) and prefixed forms that end with that\n"
	                    "pattern after a final '-' (e.g. zonematerial-converted-193_ec) place on the minesweeper\n"
	                    "board and resolve 8-ring neighbors; unparseable sets fall back to a flat list.\n"
	                    "Legacy: zone_painter <input.max> --bank <bank> [...] behaves exactly as before.\n"
	                    "Top toolbar (M14c, 3ds-Max vibe): movable bar with left drag grip (client hands-bar idiom),\n"
	                    "square text buttons BOARD SAVE | UNDO REDO | TILE COLOR DISP | <season face>. Season opens a\n"
	                    "context menu of available seasons; mode buttons show pushed state. Keys T/C/D, O, Y, undo/redo\n"
	                    "still work. Slim Painter panel: tile set ± / 256 / brush / group, Color section, Displace,\n"
	                    "lock borders, fill, Tiles palette toggle, multi-file dirty. Save modal same as before.\n"
	                    "Tiles palette (ui M8): second movable window with one thumbnail cell per bank tileset\n"
	                    "(64px preview from the first resolvable 128 diffuse, name + 1-based index). Click selects\n"
	                    "through the same absolute handler as digit keys / right-click pick; season change rebuilds\n"
	                    "previews. Toggle with P (TogglePalette) or the panel Tiles button.\n"
	                    "Config files (plugin keys.cfg port, NLMISC::CConfigFile syntax; one file may serve both):\n"
	                    "  keys cfg (--keys-cfg, else ./zone_painter_keys.cfg): rebinds the plugin-era actions by name,\n"
	                    "  values are NeL TKey codes (the plugin's keys.cfg Key* constant block parses verbatim).\n"
	                    "  Honored: ModeTile ModeColor ModeDisplace SizeUp SizeDown ToggleTileSize GroupUp GroupDown\n"
	                    "  Fill0 Fill1 Fill2 Fill3 HardnessUp HardnessDown OpacityUp OpacityDown SelectColorBrush\n"
	                    "  ToggleColorBrushMode LockBorders ZoomIn ZoomOut ToggleUI SeasonNext TogglePalette ToggleBoard\n"
	                    "  (defaults: T C D + - B G V F F6 F7 F8 Home End Insert Delete S Q L, F10, Y, P; zoom unbound).\n"
	                    "  Accepted+ignored (no tool equivalent): Select Pick ToggleColor BackgroundColor ToggleArrows\n"
	                    "  Zouille AutomaticLighting GetState ResetPatch.\n"
	                    "  vars cfg (--vars-cfg, else ./zone_painter_vars.cfg): LightDirection {x,y,z}, LightDiffuse {r,g,b},\n"
	                    "  LightAmbiant {r,g,b}, LightMultiply, ZoomSpeed (the plugin LoadVarCfg set).\n"
	                    "  ToggleUI (default F10: show/hide the in-engine NLGUI panel + toolbar).\n"
	                    "  SeasonNext (default Y: cycle landscape season textures among variants that exist for\n"
	                    "  the open bank — spring/summer/autumn/winter; paint indices/colors/displace untouched;\n"
	                    "  tileset palette previews re-resolve to the new season; toolbar season face updates).\n"
	                    "  TogglePalette (default P: show/hide the Tiles thumbnail palette).\n"
	                    "  ToggleBoard (default O: session board hub — continent working set, or ecosystem\n"
	                    "    scratch board for brick instances: empty cell places an instance; instance cell\n"
	                    "    popup Rotate CW/CCW / Mirror / Remove; home = open brick. Labels show R90/M.\n"
	                    "    Continent: L-click closed=open, open=Close/Save/Toggle. BACK TO PAINTING / O\n"
	                    "    returns. Working-set / place changes rebuild landscape+weld and CLEAR undo).\n"
	                    "Fixed viewer keys: PgUp/PgDn + 0-9 tile set, [ ] displace index, Ctrl+Z/Ctrl+E undo/redo,\n"
	                    "F12 screenshot, ESC quit.");
	// Optional first positional: .max (legacy) or folder (startup seed). Absent => startup flow.
	args.addAdditionalArg("input", "Input .max scene (legacy) or graphics/seed folder (startup); omit for discovery", true, false);
	args.addArg("", "bank", "bank", "Tile bank (.smallbank/.bank); required for the legacy .max path (auto-derived in startup flow)");
	args.addArg("", "bank-recursive", "", "Add the bank directory to the texture search path recursively");
	args.addArg("", "search-path", "dir", "Extra recursive texture search path (repeatable)", false);
	args.addArg("", "out", "output.max", "Output .max for --null-edit (in-place save is refused)");
	args.addArg("", "save", "output.max",
	            "Write-back + whole-file save after ops (in-place save is refused). "
	            "Multi-select sessions (M6b): errors if more than one editable file is dirty — "
	            "use the interactive Save modal (Overwrite all) for multi-file write-back.");
	args.addArg("", "cellsize", "meters", "Ligo cell size for the zone-symmetry state (default 100)");
	args.addArg("", "snap", "meters", "Ligo snap for the zone-symmetry state (default 1)");
	args.addArg("", "paint-script", "file", "Scripted paint ops (headless without a display mode)");
	args.addArg("", "seed", "n", "Random seed for the paint ops (default 1; ops use a cycle counter for base tiles)");
	args.addArg("", "lock-borders", "", "Lock tiles bordering frozen zones or open edges (plugin lockBorders)");
	args.addArg("", "brush", "0-2", "Brush size for tile mouse strokes and displace painting (recursion depths 0/4/8)");
	args.addArg("", "group", "0-12", "Tile group bias (0 = none)");
	args.addArg("", "color", "rrggbb", "Viewer color brush color (default ffffff)");
	args.addArg("", "radius", "meters",
	            "Viewer color brush radius (default 8; keys SizeUp/Down in Color mode and panel Radius ±: ×1.5/÷1.5, clamp 2-32)");
	args.addArg("", "hardness", "0-255",
	            "Viewer color brush hardness (default 128; keys Home/End and panel Hard ± step 51)");
	args.addArg("", "opacity", "0-255",
	            "Viewer color brush opacity (default 255; keys Insert/Delete and panel Opacity ± step 51)");
	args.addArg("", "brush-mask", "file.tga",
	            "Color-brush bitmap mask (CPath-resolved; plugin loadBrush port; script op `mask <file|none>`; "
	            "panel Mask button / S key cycles shipped brushes)");
	args.addArg("", "brush-dir", "dir", "Brush mask directory for the viewer cycle key (default: <exe dir>/brushes, else ./brushes)");
	args.addArg("", "keys-cfg", "file", "Key bindings config (see the description; default zone_painter_keys.cfg in cwd when present)");
	args.addArg("", "vars-cfg", "file", "Light/zoom variables config (see the description; default zone_painter_vars.cfg in cwd when present)");
	args.addArg("", "displace-index", "0-15",
	            "Viewer displace paint index (default 0; keys [ ] and panel Displace ±)");
	args.addArg("", "db", "root", "Database root for authored-path texture resolution (default: derived from the input path)");
	args.addArg("", "preload-tiles", "", "Viewer: flush every tile set's tiles at startup (overrides the stored 0x4010 flag on)");
	args.addArg("", "no-preload-tiles", "", "Viewer: force the preload flush off (overrides the stored flag)");
	args.addArg("", "include-meshes", "", "Viewer: display the scene's non-zone meshes + ambient + lights (overrides the stored 0x4003 flag on)");
	args.addArg("", "no-include-meshes", "", "Viewer: force the context-mesh display off (overrides the stored flag)");
	args.addArg("", "null-edit", "", "Headless: resolve carriers, write back untouched pristine blobs, save to --out");
	args.addArg("", "verify-identical", "", "With --null-edit: byte-compare the output against the input");
	args.addArg("", "dump-zones", "dir", "Headless: write every built display CZone and report counts");
	args.addArg("", "dump-rpo", "", "Dump every carrier's pristine tile records to stdout");
	args.addArg("", "dump-bank-xref", "", "Dump the bank's tile -> (set, number, type) xref table to stdout");
	args.addArg("", "dump-carrier-blob", "dir", "Write each zone's original carrier blob bytes to <dir>/zone<id>.blob");
	args.addArg("", "screenshot", "out.tga", "Render one frame to a .tga and exit");
	args.addArg("", "font", "file.ttf", "HUD font for the viewer (default: a system font when present)");
	args.addArg("", "verbose", "", "Verbose output");
	// Test plumbing (thin wrappers over the same selection functions the UI buttons call)
	args.addArg("", "startup-auto", "workspace/zone[+zone...][?query]",
	            "Skip startup UI: select workspace+zone by name and open the viewer. "
	            "Multi-select (continents, M6b): plus-separated zone basenames "
	            "(\"world/zoneA+zoneB+zoneC\") opens all editable plus the union of their "
	            "8-rings as frozen context (neighbors default on; ?neighbors=off respected). "
	            "Basenames may be bare grid names (4_AC) or prefixed (zonematerial-converted-193_ec). "
	            "Optional ?query after the zone list: ampersand-separated key=value pairs. "
	            "Supported keys: neighbors=on|off (continents; default on), "
	            "place=dx,dy[,rot][,m] (ecosystem self-instances; see --place), "
	            "instances=NxM (deprecated alias for translation-only places; see --instances). "
	            "Examples: lacustre/material-fond?place=1,0,1  "
	            "snowballs/4_AC+4_AD  "
	            "fyros_newbieland/15_AE?neighbors=off  "
	            "lacustre/material-fond?instances=2x1&neighbors=off");
	args.addArg("", "startup-screenshot", "out.tga", "Render one frame of the first startup screen and exit (M2b+)");
	args.addArg("", "startup-screen", "world",
	            "With --startup-screenshot: pre-select this world (name or graphics-root basename) and capture Screen B "
	            "(continent coordinate grid or ecosystem zone list) instead of Screen A");
	args.addArg("", "panel-save-test", "copy|overwrite",
	            "Headless: after --paint-script, invoke the same panel save path (zpSaveTo / zpSaveOverwrite). "
	            "copy writes <basename>_painted.max under --out's directory (or cwd). overwrite first copies the "
	            "input .max into that directory and operates on the copy (never the real source). Requires --paint-script.");
	args.addArg("", "neighbors", "on|off",
	            "Board/continent startup: load neighbor .max files as frozen read-only context "
	            "via the M16 hint chain (appdata → embedded names → continent grid; default on "
	            "for interactive/startup-auto, off for the legacy .max path). "
	            "Also accepted as ?neighbors=off on --startup-auto.");
	args.addArg("", "embedded-context", "",
	            "Board sessions: display embedded non-eligible/frozen zone copies from the open "
	            "file (pre-M16). Default is board authority — those copies are skipped and used "
	            "only as neighbor-name hints. Legacy direct-.max path always shows them.");
	args.addArg("", "place-context", "dx,dy:basename",
	            "Ecosystem/board: load an existing world brick file as frozen read-only context "
	            "at fine-cell offset (dx,dy) relative to the primary footprint origin. Repeatable. "
	            "Headless equivalent of the scratch-board context placement (M16c).");
	args.addArg("", "dump-neighbor-hints", "file.max",
	            "Headless: print the painter neighbor-hints appdata (and embedded fallback) for "
	            "the eligible node of file.max; exit. Does not open the viewer.");
	args.addArg("", "season", "sp|su|au|wi",
	            "Initial landscape season texture preference (spring/summer/autumn/winter). "
	            "Default: auto (first available postfix, historically spring). Only seasons that "
	            "resolve for the open bank are offered by SeasonNext / the panel button; painting "
	            "data is season-independent.");
	args.addArg("", "all-zones", "",
            "Open every non-frozen (non-0x0976) RklPatch as a paint target — pre-M11b "
            "open-everything. Default is exporter-faithful eligibility (M11b), mirroring "
            "pipeline_max_export_zone + nel_ligo_export.ms / zone_export.ms: "
            "zonematerial-/zonespecial- → single non-frozen RklPatch (name-match cell token "
            "when multiple); zonetransition- → all non-frozen; else ExportRykolZone first "
            "findID-able node. Other zones load as read-only context display. [NELLIGO] "
            "markers skipped. Null-edit / write-back still whole-file; paint-script zone "
            "ids address PAINTABLE zones only.");
args.addArg("", "place", "dx,dy[,rot][,m]",
	            "Ecosystem: place a self-instance whose TRANSFORMED footprint block has min-corner "
	            "at fine cell (dx,dy) relative to the primary footprint origin (0,0). Optional "
	            "rotation 0..3 (90° CCW) and mirror (m|1|true). Repeatable. Primary home occupies "
	            "[0,fw)×[0,fh) cells (fw,fh from AABB ceil'd to --cellsize); a rot-0 instance at "
	            "(fw,0) sits immediately east of home. Geometry is rotated/mirrored about the "
	            "FOOTPRINT block center (origin + half step), then translated so the block origin "
	            "lands at (dx,dy). Initial tile display remounts U under mirror and applies "
	            "transformTile (M13c). Display clones share one paint carrier; picks/paint ops "
	            "inverse-map through transformDesc. Overlapping blocks refused on the scratch "
	            "board. Ecosystem-only. Also ?place=dx,dy,rot on --startup-auto.",
	            false);
args.addArg("", "instances", "NxM",
	            "DEPRECATED alias (M12): expands to translation-only --place for each non-origin "
	            "cell of an NxM grid (supported: 1x1, 2x1, 1x2, 2x2, 3x3). Prefer --place. "
	            "Ecosystem-only. Same as ?instances=NxM on --startup-auto. "
	            "Ignored with a warning on the legacy .max path.");
	args.addArg("", "thumb-extract", "out.tga",
	            "Headless: extract OLE PIDSI_THUMBNAIL from the input .max into out.tga (and refresh thumbcache); exit");
	args.addArg("", "thumb-roundtrip-test", "file.max",
	            "Headless: parse and re-encode SummaryInformation with the UNCHANGED thumbnail; "
	            "exit 0 only if the stream is byte-identical (M5c property-set gate)");
	args.addArg("", "thumbnail", "",
	            "With --save: render a top-down orthographic thumbnail of the primary zone and "
	            "write it into SummaryInformation PIDSI_THUMBNAIL (requires a display — use "
	            "xvfb-run for headless). Plain --save and --null-edit never touch the thumbnail. "
	            "Interactive Save modal has an 'update thumbnail' checkbox (default on).");
	if (!args.parse(argc, argv))
		return 1;

	g_verbose = args.haveLongArg("verbose");
	g_CliWantThumbnail = args.haveLongArg("thumbnail");
	g_AllZones = args.haveLongArg("all-zones");
	g_EmbeddedContext = args.haveLongArg("embedded-context");
	if (g_AllZones)
		printf("eligibility: --all-zones (open every non-frozen RklPatch as paint target)\n");
	if (g_EmbeddedContext)
		printf("board: --embedded-context (show embedded non-eligible/frozen copies)\n");

	// --place-context dx,dy:basename (repeatable)
	if (args.haveLongArg("place-context"))
	{
		const std::vector<std::string> &pcs = args.getLongArg("place-context");
		for (size_t i = 0; i < pcs.size(); ++i)
		{
			std::string::size_type colon = pcs[i].find(':');
			if (colon == std::string::npos)
			{
				fprintf(stderr, "ERROR: --place-context expects dx,dy:basename, got '%s'\n",
				        pcs[i].c_str());
				return 1;
			}
			std::string coords = pcs[i].substr(0, colon);
			std::string base = pcs[i].substr(colon + 1);
			std::string::size_type comma = coords.find(',');
			SPlaceContextSpec pc;
			if (comma == std::string::npos
			    || !NLMISC::fromString(coords.substr(0, comma), pc.Dx)
			    || !NLMISC::fromString(coords.substr(comma + 1), pc.Dy)
			    || base.empty())
			{
				fprintf(stderr, "ERROR: --place-context expects dx,dy:basename, got '%s'\n",
				        pcs[i].c_str());
				return 1;
			}
			// strip .max
			std::string::size_type dot = base.rfind('.');
			if (dot != std::string::npos && NLMISC::toLowerAscii(base.substr(dot)) == ".max")
				base = base.substr(0, dot);
			pc.Basename = base;
			g_PlaceContextSpecs.push_back(pc);
		}
	}

	// Headless: dump neighbor hints for a .max and exit (shared read with session open)
	if (args.haveLongArg("dump-neighbor-hints"))
	{
		std::string path = args.getLongArg("dump-neighbor-hints")[0];
		NL3D::registerSerial3d();
		PMAXLOAD::SLoadedMax lm;
		if (!PMAXLOAD::loadMaxFile(path, lm) || !lm.Scene)
		{
			fprintf(stderr, "ERROR: cannot load %s\n", path.c_str());
			return 1;
		}
		const std::string basen = NLMISC::CFile::getFilenameWithoutExtension(path);
		std::vector<SNeighborHint> hints;
		std::string source = "none";
		std::string raw;
		if (readNeighborHintsFromScene(*lm.Scene, basen, hints, &raw))
		{
			source = "appdata";
			printf("neighbor-hints appdata raw: %s\n", raw.c_str());
		}
		else if (extractEmbeddedNeighborHints(*lm.Scene, basen, 160.f, hints))
		{
			source = "embedded";
		}
		printf("neighbor-hints dump '%s': source=%s count=%u\n", basen.c_str(), source.c_str(),
		       (uint)hints.size());
		for (size_t i = 0; i < hints.size(); ++i)
			printf("  %d,%d:%s\n", hints[i].Dx, hints[i].Dy, hints[i].Basename.c_str());
		return 0;
	}

	// Season preference (M6a) before bank load so the first resolveBankTextures uses it
	if (args.haveLongArg("season"))
	{
		std::string s = NLMISC::toLowerAscii(args.getLongArg("season")[0]);
		if (!ZPCTX::setSeasonPreference(s))
		{
			fprintf(stderr, "ERROR: --season expects sp|su|au|wi, got '%s'\n",
			        args.getLongArg("season")[0].c_str());
			return 1;
		}
		printf("season: preference '%s' (%s)\n", s.c_str(), ZPCTX::seasonPreferenceLabel().c_str());
	}

	// Hidden M5b/M5c thumbnail tools (no driver / scene required)
	if (args.haveLongArg("thumb-roundtrip-test"))
	{
		std::string f = args.getLongArg("thumb-roundtrip-test")[0];
		std::string err;
		if (ZPTHUMB::thumbRoundtripIdentical(f, err))
		{
			printf("OK thumb-roundtrip: byte-identical SummaryInformation -> %s\n", f.c_str());
			return 0;
		}
		fprintf(stderr, "FAIL thumb-roundtrip: %s (%s)\n", err.c_str(), f.c_str());
		return 1;
	}
	if (args.haveLongArg("thumb-extract"))
	{
		if (!args.haveAdditionalArg("input"))
		{
			fprintf(stderr, "ERROR: --thumb-extract needs an input .max\n");
			return 1;
		}
		std::string maxPath = args.getAdditionalArg("input")[0];
		std::string outTga = args.getLongArg("thumb-extract")[0];
		NLMISC::CBitmap bmp;
		if (!ZPTHUMB::extractThumbnailBitmap(maxPath, bmp))
		{
			fprintf(stderr, "FAIL thumb-extract: no thumbnail in %s\n", maxPath.c_str());
			return 1;
		}
		try
		{
			NLMISC::COFile of(outTga);
			if (!bmp.writeTGA(of, 32, false))
			{
				fprintf(stderr, "FAIL thumb-extract: writeTGA %s\n", outTga.c_str());
				return 1;
			}
		}
		catch (...)
		{
			fprintf(stderr, "FAIL thumb-extract: cannot write %s\n", outTga.c_str());
			return 1;
		}
		std::string cached;
		ZPTHUMB::ensureCachedThumbnail(maxPath, cached);
		printf("OK thumb-extract: %ux%u -> %s (cache %s) mtime=%u\n",
		       bmp.getWidth(), bmp.getHeight(), outTga.c_str(),
		       cached.empty() ? "(none)" : cached.c_str(),
		       NLMISC::CFile::getFileModificationDate(maxPath));
		return 0;
	}

	// Resolve the first positional: .max => legacy, directory => startup seed, absent => startup.
	std::string input;
	std::string seedFolder;
	bool startupPath = false;
	if (args.haveAdditionalArg("input"))
	{
		std::string pos = args.getAdditionalArg("input")[0];
		if (ZPWS::isMaxPath(pos))
		{
			input = pos;
		}
		else if (NLMISC::CFile::isDirectory(pos))
		{
			startupPath = true;
			seedFolder = pos;
		}
		else if (NLMISC::CFile::fileExists(pos))
		{
			fprintf(stderr, "ERROR: first argument must be a .max file or a directory, got '%s'\n", pos.c_str());
			return 1;
		}
		else
		{
			fprintf(stderr, "ERROR: path not found: %s\n", pos.c_str());
			return 1;
		}
	}
	else
	{
		startupPath = true;
	}

	// Headless legacy modes still require a .max input
	const bool headlessLegacyNeedMax = args.haveLongArg("null-edit")
		|| args.haveLongArg("dump-zones")
		|| args.haveLongArg("dump-rpo")
		|| args.haveLongArg("dump-bank-xref")
		|| args.haveLongArg("dump-carrier-blob")
		|| args.haveLongArg("paint-script");
	if (startupPath && headlessLegacyNeedMax && !args.haveLongArg("startup-auto"))
	{
		fprintf(stderr, "ERROR: headless modes need an input .max (legacy path)\n");
		return 1;
	}

	// Shared UDriver + EditorUI for the startup screens and (optionally) the viewer.
	// Owned here when the startup path opens a window; runViewer receives them and must
	// not release them. Legacy .max path still lets runViewer create its own driver.
	NL3D::UDriver *sharedDriver = NULL;
	ZPUI::CEditorUI *sharedEditorUI = NULL;
	bool ownsSharedHost = false;

	// ---- Startup flow: discover + auto / interactive screens ----
	if (startupPath)
	{
		g_BoardSession = true; // M16: board authority + neighbor hint chain
		ZPWS::SStartupCfg scfg;
		ZPWS::loadStartupCfg(scfg);

		std::vector<ZPWS::SWorldEntry> worlds;
		ZPWS::discoverWorkspaces(seedFolder, scfg.LastGraphicsFolder, worlds);
		if (g_verbose)
		{
			printf("discovery: %u world(s)", (uint)worlds.size());
			if (!seedFolder.empty()) printf(" seed='%s'", seedFolder.c_str());
			printf("\n");
			for (size_t i = 0; i < worlds.size(); ++i)
				printf("  %s '%s' root=%s bank=%s%s\n",
				       worlds[i].Kind == ZPWS::Ecosystem ? "eco" : "continent",
				       worlds[i].WorldName.c_str(),
				       worlds[i].GraphicsRoot.c_str(),
				       worlds[i].BankPath.c_str(),
				       worlds[i].BankOk ? "" : " (no bank)");
		}

		ZPUI::SStartupSelection selection;
		bool haveSelection = false;

		// Neighbors default ON for the startup path (CLI --neighbors overrides; query can too)
		g_LoadNeighbors = true;
		if (args.haveLongArg("neighbors"))
		{
			std::string n = NLMISC::toLowerAscii(args.getLongArg("neighbors")[0]);
			if (n == "off" || n == "0" || n == "false" || n == "no")
				g_LoadNeighbors = false;
			else if (n == "on" || n == "1" || n == "true" || n == "yes")
				g_LoadNeighbors = true;
			else
			{
				fprintf(stderr, "ERROR: --neighbors expects on|off, got '%s'\n",
				        args.getLongArg("neighbors")[0].c_str());
				return 1;
			}
		}

		// Instance placements (ecosystem self-tile). CLI --place / --instances; also query on auto.
		g_Places.clear();
		g_InstanceCols = 1;
		g_InstanceRows = 1;
		g_InstanceCount = 1;
		std::string instancesFromCli;
		if (args.haveLongArg("instances"))
			instancesFromCli = args.getLongArg("instances")[0];
		if (args.haveLongArg("place"))
		{
			const std::vector<std::string> &pv = args.getLongArg("place");
			for (size_t i = 0; i < pv.size(); ++i)
			{
				SInstancePlace pl;
				std::string perr;
				if (!parsePlaceSpec(pv[i], pl, perr))
				{
					fprintf(stderr, "ERROR: %s\n", perr.c_str());
					return 1;
				}
				g_Places.push_back(pl);
			}
		}

		if (args.haveLongArg("startup-auto"))
		{
			// Thin wrapper: same selectAuto the buttons call (no UI required).
			// Optional query: workspace/zone?neighbors=off&place=1,0,1&instances=2x2
			std::string autoArg = args.getLongArg("startup-auto")[0];
			std::string autoPath = autoArg;
			std::string instancesFromQuery;
			std::vector<SInstancePlace> placesFromQuery;
			std::string::size_type qpos = autoArg.find('?');
			if (qpos != std::string::npos)
			{
				autoPath = autoArg.substr(0, qpos);
				std::string query = autoArg.substr(qpos + 1);
				// Parse simple key=value pairs
				std::string::size_type start = 0;
				while (start < query.size())
				{
					std::string::size_type amp = query.find('&', start);
					std::string pair = query.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
					std::string::size_type eq = pair.find('=');
					std::string key = eq == std::string::npos ? pair : pair.substr(0, eq);
					std::string val = eq == std::string::npos ? std::string() : pair.substr(eq + 1);
					std::string keyL = NLMISC::toLowerAscii(key);
					if (keyL == "neighbors")
					{
						std::string v = NLMISC::toLowerAscii(val);
						if (v == "off" || v == "0" || v == "false" || v == "no")
							g_LoadNeighbors = false;
						else if (v == "on" || v == "1" || v == "true" || v == "yes")
							g_LoadNeighbors = true;
					}
					else if (keyL == "instances")
					{
						instancesFromQuery = val;
					}
					else if (keyL == "place")
					{
						SInstancePlace pl;
						std::string perr;
						if (!parsePlaceSpec(val, pl, perr))
						{
							fprintf(stderr, "ERROR: %s\n", perr.c_str());
							return 1;
						}
						placesFromQuery.push_back(pl);
					}
					if (amp == std::string::npos) break;
					start = amp + 1;
				}
			}
			// Query places append to CLI --place; deprecated --instances expands if no places yet
			for (size_t i = 0; i < placesFromQuery.size(); ++i)
				g_Places.push_back(placesFromQuery[i]);
			std::string instancesSpec = !instancesFromQuery.empty() ? instancesFromQuery : instancesFromCli;
			if (!instancesSpec.empty() && g_Places.empty())
			{
				std::string ierr;
				std::vector<SInstancePlace> layoutPlaces;
				if (!parseInstanceLayout(instancesSpec, g_InstanceCols, g_InstanceRows, layoutPlaces, ierr))
				{
					fprintf(stderr, "ERROR: %s\n", ierr.c_str());
					return 1;
				}
				if (!layoutPlaces.empty())
				{
					fprintf(stderr, "NOTE: --instances / ?instances= is deprecated; use --place (expanded %ux%u → %u place(s))\n",
					        g_InstanceCols, g_InstanceRows, (uint)layoutPlaces.size());
					g_Places = layoutPlaces;
				}
			}
			else if (!instancesSpec.empty() && !g_Places.empty())
			{
				fprintf(stderr, "NOTE: ignoring --instances / ?instances= because --place / ?place= is set\n");
			}
			g_InstanceCount = 1 + (uint)g_Places.size();
			std::string err;
			// Pass seed so a seeded temp workspace wins over LastGraphicsFolder when both
			// expose the same WorldName (M16d: reopen must open the file we just saved).
			if (!ZPUI::startupSelectWorldZone(worlds, autoPath, selection, err, seedFolder))
			{
				fprintf(stderr, "ERROR: %s\n", err.c_str());
				return 1;
			}
			// Instances are ecosystem-only
			if (!g_Places.empty() && selection.World.Kind != ZPWS::Ecosystem)
			{
				fprintf(stderr, "ERROR: --place / --instances is ecosystem-only (not available on continents)\n");
				return 1;
			}
			haveSelection = true;
			printf("startup-auto: world '%s' (%s) root='%s' zone(s) ",
			       selection.World.WorldName.c_str(),
			       selection.World.Kind == ZPWS::Ecosystem ? "ecosystem" : "continent",
			       selection.World.GraphicsRoot.c_str());
			if (selection.EditableZones.empty())
				printf("'%s'", selection.Zone.Basename.c_str());
			else
			{
				for (size_t zi = 0; zi < selection.EditableZones.size(); ++zi)
					printf("%s'%s'", zi ? "+" : "", selection.EditableZones[zi].Basename.c_str());
			}
			printf(" neighbors=%s places=%u\n",
			       g_LoadNeighbors ? "on" : "off",
			       (uint)g_Places.size());
			if (!selection.Zone.MaxPath.empty())
				printf("session open: %s\n", selection.Zone.MaxPath.c_str());
		}
		else
		{
			// Interactive (or --startup-screenshot): one shared UDriver + EditorUI
			std::string fontPathEarly = args.haveLongArg("font") ? args.getLongArg("font")[0] : std::string();
			if (fontPathEarly.empty())
			{
				const char *sysFont = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
				if (NLMISC::CFile::fileExists(sysFont)) fontPathEarly = sysFont;
			}
			NL3D::CScene::registerBasics();
			sharedDriver = NL3D::UDriver::createDriver(0, false, 0);
			if (!sharedDriver)
			{
				fprintf(stderr, "ERROR: UDriver::createDriver failed (no 3D driver?)\n");
				return 1;
			}
			if (!sharedDriver->setDisplay(NL3D::UDriver::CMode(kMainWidth, kMainHeight, 32, true)))
			{
				fprintf(stderr, "ERROR: UDriver::setDisplay failed\n");
				delete sharedDriver;
				return 1;
			}
			sharedEditorUI = new ZPUI::CEditorUI();
			if (!sharedEditorUI->init(sharedDriver, fontPathEarly))
			{
				fprintf(stderr, "ERROR: editor UI init failed for startup screens\n");
				sharedDriver->release();
				delete sharedDriver;
				delete sharedEditorUI;
				return 1;
			}
			ownsSharedHost = true;

			const bool folderBrowserEnabled = true; // Screen C wired in M2c; Browse enabled
			std::string shotPath = args.haveLongArg("startup-screenshot")
				? args.getLongArg("startup-screenshot")[0] : std::string();
			std::string shotWorld = args.haveLongArg("startup-screen")
				? args.getLongArg("startup-screen")[0] : std::string();
			ZPUI::EStartupResult sr = ZPUI::runStartupFlow(
				sharedDriver, sharedEditorUI, worlds, shotPath, selection, folderBrowserEnabled,
				seedFolder, shotWorld);

			if (sr == ZPUI::StartupScreenshotDone)
			{
				sharedEditorUI->shutdown();
				delete sharedEditorUI;
				sharedDriver->release();
				delete sharedDriver;
				return 0;
			}
			if (sr == ZPUI::StartupError)
			{
				sharedEditorUI->shutdown();
				delete sharedEditorUI;
				sharedDriver->release();
				delete sharedDriver;
				return 1;
			}
			if (sr == ZPUI::StartupQuit)
			{
				sharedEditorUI->shutdown();
				delete sharedEditorUI;
				sharedDriver->release();
				delete sharedDriver;
				return 0;
			}
			// StartupOpenZone
			haveSelection = true;
			// Interactive open: CLI --place already filled g_Places; else --instances / Screen B layout
			if (g_Places.empty())
			{
				std::string layoutSpec = !instancesFromCli.empty() ? instancesFromCli : selection.InstanceLayout;
				if (!layoutSpec.empty() && layoutSpec != "1x1")
				{
					std::string ierr;
					std::vector<SInstancePlace> layoutPlaces;
					if (!parseInstanceLayout(layoutSpec, g_InstanceCols, g_InstanceRows, layoutPlaces, ierr))
					{
						fprintf(stderr, "ERROR: %s\n", ierr.c_str());
						return 1;
					}
					if (!layoutPlaces.empty())
					{
						if (!instancesFromCli.empty())
							fprintf(stderr, "NOTE: --instances is deprecated; use --place (expanded %ux%u → %u place(s))\n",
							        g_InstanceCols, g_InstanceRows, (uint)layoutPlaces.size());
						g_Places = layoutPlaces;
					}
				}
			}
			g_InstanceCount = 1 + (uint)g_Places.size();
			if (!g_Places.empty() && selection.World.Kind != ZPWS::Ecosystem)
			{
				fprintf(stderr, "ERROR: --place / --instances is ecosystem-only (not available on continents)\n");
				return 1;
			}
		}

		if (!haveSelection)
		{
			fprintf(stderr, "ERROR: startup flow produced no zone selection\n");
			return 1;
		}

		// Configure exactly what the CLI flags would have: bank, search path, input
		// Multi-select: primary is first editable; keep full list for assembly (M6b)
		g_StartupEditableZones = selection.EditableZones;
		if (g_StartupEditableZones.empty())
			g_StartupEditableZones.push_back(selection.Zone);
		input = g_StartupEditableZones[0].MaxPath;
		selection.Zone = g_StartupEditableZones[0];
		if (!args.haveLongArg("bank"))
		{
			g_BankPath = selection.World.BankPath;
			g_ForceBankRecursive = true;
		}
		g_StartupTexturePath = selection.World.TextureSearchPath;
		g_StartupWorld = selection.World;
		g_StartupZone = selection.Zone;

		// Interactive save modal when no --save was given (with --save: one-click direct)
		g_InteractiveSave = !args.haveLongArg("save");

		// Remember successful world entry + last open layout (ecosystem self-instances)
		{
			ZPWS::SStartupCfg save;
			save.LastGraphicsFolder = selection.World.GraphicsRoot;
			save.LastWorld = selection.World.WorldName;
			if (g_InstanceCols > 1 || g_InstanceRows > 1)
				save.LastInstances = NLMISC::toString("%ux%u", g_InstanceCols, g_InstanceRows);
			else if (!selection.InstanceLayout.empty())
				save.LastInstances = selection.InstanceLayout;
			else
				save.LastInstances = "1x1";
			if (save.LastInstances.empty())
				save.LastInstances = "1x1";
			ZPWS::saveStartupCfg(save);
		}
	}
	else
	{
		// Legacy .max path: board authority off; neighbors off unless explicitly enabled
		g_BoardSession = false;
		g_LoadNeighbors = false;
		if (args.haveLongArg("neighbors"))
		{
			std::string n = NLMISC::toLowerAscii(args.getLongArg("neighbors")[0]);
			if (n == "on" || n == "1" || n == "true" || n == "yes")
				g_LoadNeighbors = true; // no-op without a continent world context
		}
		// Instances are ecosystem-startup only; warn and ignore on the legacy path
		if (args.haveLongArg("instances") || args.haveLongArg("place"))
		{
			fprintf(stderr, "WARNING: --place / --instances is ignored on the legacy .max path "
			                "(use --startup-auto \"eco/brick?place=1,0,1\" or the ecosystem UI)\n");
		}
		if (!g_PlaceContextSpecs.empty())
		{
			fprintf(stderr, "WARNING: --place-context is ignored on the legacy .max path "
			                "(use --startup-auto with a board session)\n");
			g_PlaceContextSpecs.clear();
		}
		g_Places.clear();
		g_InstanceCols = 1;
		g_InstanceRows = 1;
		g_InstanceCount = 1;
	}

	// If still no input after startup handling, fail
	if (input.empty())
	{
		fprintf(stderr, "ERROR: no input .max resolved\n");
		return 1;
	}
	g_InputPath = input;
	// Cfg loaders (plugin LoadKeyCfg/LoadVarCfg port): CLI path, else the default cwd name,
	// else the hardcoded defaults stand. No cfg present == the pre-cfg tool, identically.
	{
		bool cfgOk = args.haveLongArg("keys-cfg")
			? loadKeysCfg(args.getLongArg("keys-cfg")[0], true)
			: loadKeysCfg("zone_painter_keys.cfg", false);
		if (!cfgOk) return 1;
		cfgOk = args.haveLongArg("vars-cfg")
			? loadVarsCfg(args.getLongArg("vars-cfg")[0], true)
			: loadVarsCfg("zone_painter_vars.cfg", false);
		if (!cfgOk) return 1;
	}
	// Shipped brush-mask set for the viewer cycle key (SelectColorBrush)
	{
		std::string brushDir;
		if (args.haveLongArg("brush-dir")) brushDir = args.getLongArg("brush-dir")[0];
		else
		{
			std::string exeDir = NLMISC::CFile::getPath(argv[0]);
			if (!exeDir.empty() && NLMISC::CFile::isDirectory(exeDir + "brushes")) brushDir = exeDir + "brushes";
			else if (NLMISC::CFile::isDirectory("brushes")) brushDir = "brushes";
		}
		if (!brushDir.empty())
		{
			std::vector<std::string> files;
			NLMISC::CPath::getPathContent(brushDir, false, false, true, files);
			for (size_t i = 0; i < files.size(); ++i)
				if (NLMISC::toLowerAscii(NLMISC::CFile::getExtension(files[i])) == "tga")
					g_MaskFiles.push_back(files[i]);
			std::sort(g_MaskFiles.begin(), g_MaskFiles.end());
		}
	}
	std::string bankPath = args.haveLongArg("bank") ? args.getLongArg("bank")[0] : g_BankPath;
	g_BankPath = bankPath;
	bool bankRecursive = args.haveLongArg("bank-recursive") || g_ForceBankRecursive;
	// Default cell size: 100 (historical tool default) on the legacy path; 160 on ecosystem
	// startup (Ryzom ligo cell). Instance footprint steps round the geometry AABB up to this
	// value — a wrong cellsize leaves a gap and zero welds across self-instance seams.
	float cellSize = 100.f;
	if (g_StartupWorld.Kind == ZPWS::Ecosystem && !g_StartupWorld.WorldName.empty())
		cellSize = 160.f;
	float snap = 1.f;
	if (args.haveLongArg("cellsize")) NLMISC::fromString(args.getLongArg("cellsize")[0], cellSize);
	if (args.haveLongArg("snap")) NLMISC::fromString(args.getLongArg("snap")[0], snap);
	uint seed = 1;
	if (args.haveLongArg("seed")) NLMISC::fromString(args.getLongArg("seed")[0], seed);
	srand(seed);
	std::string scriptPath = args.haveLongArg("paint-script") ? args.getLongArg("paint-script")[0] : std::string();
	std::string savePath = args.haveLongArg("save") ? args.getLongArg("save")[0] : std::string();
	std::string panelSaveTest = args.haveLongArg("panel-save-test") ? args.getLongArg("panel-save-test")[0] : std::string();
	if (!panelSaveTest.empty())
	{
		std::string mode = NLMISC::toLowerAscii(panelSaveTest);
		if (mode != "copy" && mode != "overwrite")
		{
			fprintf(stderr, "ERROR: --panel-save-test expects copy|overwrite, got '%s'\n", panelSaveTest.c_str());
			return 1;
		}
		panelSaveTest = mode;
		if (scriptPath.empty())
		{
			fprintf(stderr, "ERROR: --panel-save-test requires --paint-script\n");
			return 1;
		}
	}
	std::string fontPath = args.haveLongArg("font") ? args.getLongArg("font")[0] : std::string();
	if (fontPath.empty())
	{
		// Default HUD font: a common system TrueType (HUD text silently off when absent)
		const char *sysFont = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
		if (NLMISC::CFile::fileExists(sysFont)) fontPath = sysFont;
	}
	bool nullEdit = args.haveLongArg("null-edit");
	bool doDumpRpo = args.haveLongArg("dump-rpo");
	bool doDumpXRef = args.haveLongArg("dump-bank-xref");
	std::string dumpBlobDir = args.haveLongArg("dump-carrier-blob") ? args.getLongArg("dump-carrier-blob")[0] : std::string();
	// Viewer runs for --screenshot and for the plain interactive invocation (a paint script
	// without a display mode is headless; the dump-only modes are headless too).
	// --panel-save-test is always headless (same path as paint-script without display).
	// --save --thumbnail needs a display pass to capture the top-down view (use xvfb-run).
	const bool needThumbDisplay = g_CliWantThumbnail && !savePath.empty();
	bool viewerMode = !nullEdit && !args.haveLongArg("dump-zones") && panelSaveTest.empty()
		&& (args.haveLongArg("screenshot") || needThumbDisplay
		    || (scriptPath.empty() && !doDumpRpo && !doDumpXRef && dumpBlobDir.empty()));

	if (nullEdit && !args.haveLongArg("out"))
	{
		fprintf(stderr, "ERROR: --null-edit refuses to save in place; give --out <output.max>\n");
		return 1;
	}
	if (!savePath.empty() && savePath == input)
	{
		fprintf(stderr, "ERROR: --save refuses to save in place\n");
		return 1;
	}

	// Load + assemble the painting zones (all modes; the null-edit path exercises exactly the
	// paint save path with zero ops).
	// M6b: N editable files + M frozen union-ring neighbors. Zone-id bases stay sparse.
	NL3D::registerSerial3d();
	PMAXLOAD::SLoadedMax lm;
	if (!PMAXLOAD::loadMaxFile(input, lm)) { fprintf(stderr, "ERROR: cannot load %s\n", input.c_str()); return 1; }

	// Editable list: multi-select set, or single primary from input path
	std::vector<ZPWS::SZoneEntry> editables = g_StartupEditableZones;
	if (editables.empty())
	{
		ZPWS::SZoneEntry one;
		one.MaxPath = input;
		one.Basename = NLMISC::CFile::getFilenameWithoutExtension(input);
		editables.push_back(one);
	}

	std::vector<SPaintZone> zones;
	g_EditableFiles.clear();
	g_ExtraEditableScenes.clear();

	// --- Editable files (unfrozen write targets) ---
	for (size_t ei = 0; ei < editables.size(); ++ei)
	{
		PMAXLOAD::SLoadedMax *sceneLm = NULL;
		if (ei == 0)
		{
			sceneLm = &lm; // primary stack scene
		}
		else
		{
			PMAXLOAD::SLoadedMax *extra = new PMAXLOAD::SLoadedMax();
			if (!PMAXLOAD::loadMaxFile(editables[ei].MaxPath, *extra))
			{
				fprintf(stderr, "ERROR: cannot load editable %s\n", editables[ei].MaxPath.c_str());
				delete extra;
				return 1;
			}
			g_ExtraEditableScenes.push_back(extra);
			sceneLm = extra;
		}
		const uint base = (uint)(ei * 1000);
		const size_t before = zones.size();
		bool ok = buildPaintZones(*sceneLm->Scene, zones, base, /*forceFrozen=*/false,
		                         editables[ei].Basename);
		if (!ok && ei == 0 && !nullEdit)
		{
			fprintf(stderr, "ERROR: no displayable RklPatch zone in %s\n", editables[ei].MaxPath.c_str());
			return 1;
		}
		if (!ok)
		{
			fprintf(stderr, "WARNING: editable has no paint zones: %s\n", editables[ei].MaxPath.c_str());
		}
		SEditableFileInfo efi;
		efi.Path = editables[ei].MaxPath;
		efi.Basename = editables[ei].Basename;
		efi.Lm = (ei == 0) ? NULL : sceneLm;
		efi.Editable = true;
		for (size_t zi = before; zi < zones.size(); ++zi)
			efi.ZoneIds.push_back(zones[zi].ZoneId);
		g_EditableFiles.push_back(efi);
		if (ei == 0)
			g_PrimaryLm = &lm;
		if (ei > 0 || editables.size() > 1)
			printf("editable[%u] '%s' zoneIdBase=%u zones=%u\n",
			       (uint)ei, efi.Basename.c_str(), base, (uint)efi.ZoneIds.size());
	}
	const size_t primaryZoneCount = g_EditableFiles.empty() ? 0 : g_EditableFiles[0].ZoneIds.size();
	// primaryZoneCount for instances = zones from first file only (before instances append)
	// rebuild: count of zones from first file at base 0
	size_t primaryOnlyCount = 0;
	for (size_t i = 0; i < zones.size(); ++i)
		if (zones[i].ZoneId < 1000) ++primaryOnlyCount;
	const size_t instancePrimaryCount = primaryOnlyCount;

	// Always derive primary footprint cell counts (scratch board multi-cell occupancy, M14a)
	if (instancePrimaryCount > 0)
	{
		float ox = 0.f, oy = 0.f, sx = 0.f, sy = 0.f;
		int cw = 1, ch = 1;
		computeFootprintRect(zones, 0, instancePrimaryCount, cellSize, ox, oy, sx, sy, cw, ch);
		g_FootprintCellsW = cw;
		g_FootprintCellsH = ch;
		printf("footprint: %dx%d cells  step (%.1f, %.1f)  origin (%.1f, %.1f)  pivot (%.1f, %.1f)\n",
		       cw, ch, sx, sy, ox, oy, ox + sx * 0.5f, oy + sy * 0.5f);
	}

	// Ecosystem self-instances (M4a/M12/M14a): display clones at --place offsets (rot/mirror).
	// Same Node pointers → paint_core shares one pristine carrier; weld joins transformed edges.
	// Multi-edit continent open rejects instances (already gated ecosystem-only).
	if (!g_Places.empty() && instancePrimaryCount > 0)
	{
		if (g_StartupWorld.Kind == ZPWS::Continent)
		{
			fprintf(stderr, "ERROR: --place / --instances is ecosystem-only (not available on continents)\n");
			return 1;
		}
		if (editables.size() > 1)
		{
			fprintf(stderr, "ERROR: --place / --instances is single-brick only (not multi-select)\n");
			return 1;
		}
		appendInstanceZones(zones, instancePrimaryCount, g_Places, cellSize);
		g_InstanceCount = 1 + (uint)g_Places.size();
	}

	// RO neighbors/context (M3b / M16 hint chain).
	// Neighbors join landscape, cross-zone weld, and metaTile graph; carriers never written
	// (forceFrozen => AnyUnfrozen stays false; writeBack skips frozen-only carriers).
	// Priority: appdata hints → embedded-copy names → continent grid. Board sessions also
	// skip embedded display copies (boardSkipEmbedded) so only REAL neighbor files show.
	if (g_LoadNeighbors && !editables.empty()
	    && (g_BoardSession || g_StartupWorld.Kind == ZPWS::Continent))
	{
		std::vector<std::string> skip;
		for (size_t i = 0; i < editables.size(); ++i)
			skip.push_back(editables[i].Basename);
		loadNeighborContextFiles(zones, cellSize, skip);
	}

	// M16c: --place-context "dx,dy:basename" (ecosystem / board) — load as RO at offset
	for (size_t pci = 0; pci < g_PlaceContextSpecs.size(); ++pci)
	{
		std::string perr;
		if (!loadOnePlaceContext(zones, cellSize, g_PlaceContextSpecs[pci], perr))
			fprintf(stderr, "WARNING: --place-context: %s\n", perr.c_str());
	}

	uint welds = weldPaintZones(zones);
	// Count bound edges / cross-zone binds for verification
	uint totalBound = 0, totalCross = 0;
	for (size_t i = 0; i < zones.size(); ++i)
	for (size_t p = 0; p < zones[i].Patches.size(); ++p)
	for (uint e = 0; e < 4; ++e)
	{
		const NL3D::CPatchInfo::CBindInfo &b = zones[i].Patches[p].BindEdges[e];
		if (b.NPatchs == 0) continue;
		++totalBound;
		if (b.ZoneId != zones[i].ZoneId) ++totalCross;
	}
	printf("weld: %u cross-zone edges; binds: %u total, %u cross-zone; zones: %u (%u neighbor files)\n",
	       welds, totalBound, totalCross, (uint)zones.size(), (uint)g_NeighborScenes.size());
	if (g_verbose) printf("weld detail: %u welds applied this session\n", welds);

	if (args.haveLongArg("dump-zones"))
		return dumpZones(zones, welds, args.getLongArg("dump-zones")[0]);

	// The tile bank: required for paint ops and display; the null-edit/dump paths run without.
	NL3D::CTileBank bank;
	bool haveBank = false;
	if (!bankPath.empty())
	{
		std::vector<std::string> searchPaths;
		if (args.haveLongArg("search-path")) searchPaths = args.getLongArg("search-path");
		if (!g_StartupTexturePath.empty())
			searchPaths.push_back(g_StartupTexturePath);
		// DB root before the bank resolution: the workspace _texture_tiles source fallbacks
		// derive from it (the bank path itself sits in the export tree, not the workspace).
		ZPCTX::ensureDbRootFrom(input);
		if (!loadBankFile(bankPath, bankRecursive, searchPaths, bank)) return 1;
		haveBank = true;
		// Season discovery for the open bank (panel/SeasonNext only offer what resolves)
		g_AvailableSeasons.clear();
		ZPCTX::discoverAvailableSeasons(bankPath, g_AvailableSeasons);
		if (!g_AvailableSeasons.empty())
		{
			printf("season: available");
			for (size_t si = 0; si < g_AvailableSeasons.size(); ++si)
				printf("%s%s", si ? "," : " ", g_AvailableSeasons[si].c_str());
			printf(" (preference %s)\n", ZPCTX::seasonPreferenceLabel().c_str());
			// If CLI picked a season that does not resolve, warn but keep it (fallback chain still works)
			if (!ZPCTX::seasonPreference().empty())
			{
				bool ok = false;
				for (size_t si = 0; si < g_AvailableSeasons.size(); ++si)
					if (g_AvailableSeasons[si] == ZPCTX::seasonPreference()) { ok = true; break; }
				if (!ok)
					fprintf(stderr, "WARNING: --season '%s' not found among available variants; "
					        "resolution will fall back to the first hit\n",
					        ZPCTX::seasonPreference().c_str());
			}
			// Auto-pick first available when preference empty (stable default = first = sp when present)
			else if (g_AvailableSeasons.size() >= 1)
			{
				// Leave empty preference as historical "try sp first then others" in resolve order
			}
		}
		else
			printf("season: no seasonal tile variants found for this bank (toggle disabled)\n");
	}
	if ((viewerMode || !scriptPath.empty()) && !haveBank)
	{
		fprintf(stderr, "ERROR: this mode needs --bank <bank.smallbank>\n");
		return 1;
	}

	// Rotated instance display tiles (landscape initial state matches GetTile display space)
	if (haveBank && !g_Places.empty())
		applyInstanceDisplayTiles(zones, &bank);

	// Session rebuild params (M11a; used if the board mutates the working set mid-viewer)
	g_SessionCellSize = cellSize;
	g_SessionSnap = snap;
	g_SessionLockBorders = args.haveLongArg("lock-borders");

	// The painting core over the assembled zones
	ZPPAINT::CPaintCore core;
	std::vector<ZPPAINT::SPaintZoneInput> inputs;
	buildPaintInputs(zones, inputs);
	{
		std::string err;
		if (!core.init(inputs, haveBank ? &bank : NULL, cellSize, snap, g_SessionLockBorders, err))
		{
			fprintf(stderr, "ERROR: paint core: %s\n", err.c_str());
			return 1;
		}
	}
	if (args.haveLongArg("brush")) { uint b; NLMISC::fromString(args.getLongArg("brush")[0], b); core.setBrushSize(b); }
	if (args.haveLongArg("group")) { uint g; NLMISC::fromString(args.getLongArg("group")[0], g); core.setTileGroup(g); }
	if (args.haveLongArg("brush-mask"))
	{
		std::string err;
		if (!core.loadBrushMask(args.getLongArg("brush-mask")[0], err))
		{
			fprintf(stderr, "ERROR: %s\n", err.c_str());
			return 1;
		}
		// Start the viewer cycle on the loaded mask when it is one of the shipped set
		for (size_t i = 0; i < g_MaskFiles.size(); ++i)
			if (NLMISC::CFile::getFilename(g_MaskFiles[i]) == core.brushMaskName())
				g_MaskCycle = (int)i + 1;
		printf("brush mask: %s\n", core.brushMaskName().c_str());
	}
	if (args.haveLongArg("color"))
	{
		uint32 rgb = (uint32)strtoul(args.getLongArg("color")[0].c_str(), NULL, 16);
		g_ViewerBrushColor = NLMISC::CRGBA((uint8)((rgb >> 16) & 0xff), (uint8)((rgb >> 8) & 0xff), (uint8)(rgb & 0xff), 255);
	}
	if (args.haveLongArg("radius")) NLMISC::fromString(args.getLongArg("radius")[0], g_ViewerBrushRadius);
	if (args.haveLongArg("hardness")) NLMISC::fromString(args.getLongArg("hardness")[0], g_ViewerBrushHardness);
	if (args.haveLongArg("opacity")) NLMISC::fromString(args.getLongArg("opacity")[0], g_ViewerBrushOpacity);
	if (args.haveLongArg("displace-index")) NLMISC::fromString(args.getLongArg("displace-index")[0], g_ViewerDisplaceIndex);
	// Stored-flag defaults (RPO_INCLUDE_MESHES 0x4003 / RPO_PRELOAD_TILES 0x4010 from the
	// first paint-bearing painter modifier); explicit CLI flags override either way.
	if (args.haveLongArg("db")) DBPATH::setDefaultRoot(args.getLongArg("db")[0]);
	g_PreloadTiles = core.storedPreloadTiles() == 1;
	if (args.haveLongArg("preload-tiles")) g_PreloadTiles = true;
	if (args.haveLongArg("no-preload-tiles")) g_PreloadTiles = false;
	g_IncludeMeshes = core.storedIncludeMeshes() == 1;
	if (args.haveLongArg("include-meshes")) g_IncludeMeshes = true;
	if (args.haveLongArg("no-include-meshes")) g_IncludeMeshes = false;

	if (doDumpXRef)
	{
		if (!haveBank) { fprintf(stderr, "ERROR: --dump-bank-xref needs --bank\n"); return 1; }
		core.dumpBankXRef(stdout);
	}
	if (!dumpBlobDir.empty())
	{
		NLMISC::CFile::createDirectoryTree(dumpBlobDir);
		for (size_t i = 0; i < zones.size(); ++i)
		{
			std::vector<uint8> blob;
			if (!core.dumpCarrierBlob(zones[i].ZoneId, blob)) continue;
			std::string path = dumpBlobDir + NLMISC::toString("/zone%u.blob", zones[i].ZoneId);
			NLMISC::COFile f;
			if (f.open(path) && !blob.empty()) f.serialBuffer(nlVectorData(blob), (uint)blob.size());
		}
	}

	int rc = 0;
	if (viewerMode)
	{
		std::string screenshotPath = args.haveLongArg("screenshot") ? args.getLongArg("screenshot")[0] : std::string();
		// --save --thumbnail without --screenshot: one-shot display capture then exit (xvfb-friendly)
		if (needThumbDisplay && screenshotPath.empty())
		{
			NLMISC::CFile::createDirectoryTree("/tmp/zp_ui");
			screenshotPath = "/tmp/zp_ui/_thumb_capture.tga";
		}
		rc = runViewer(zones, bank, &core, lm, screenshotPath, fontPath, scriptPath, savePath,
		               sharedDriver, sharedEditorUI);
	}
	else if (!scriptPath.empty())
	{
		rc = runPaintScript(core, scriptPath);
	}

	// Tear down the shared startup host after the viewer (viewer does not own it)
	if (ownsSharedHost)
	{
		if (sharedEditorUI)
		{
			sharedEditorUI->shutdown();
			delete sharedEditorUI;
			sharedEditorUI = NULL;
		}
		if (sharedDriver)
		{
			sharedDriver->release();
			delete sharedDriver;
			sharedDriver = NULL;
		}
	}

	if (doDumpRpo)
	{
		printf("STORED-FLAGS includeMeshes=%d preloadTiles=%d\n",
		       core.storedIncludeMeshes(), core.storedPreloadTiles());
		// Sanity: frozen-only carriers are excluded from write-back (neighbor files)
		{
			uint frozenZones = 0, unfrozenZones = 0;
			for (size_t i = 0; i < zones.size(); ++i)
			{
				if (zones[i].Frozen) ++frozenZones;
				else ++unfrozenZones;
			}
			printf("zones: %u unfrozen (editable) + %u frozen (neighbors/boundary refs)\n",
			       unfrozenZones, frozenZones);
		}
		core.dumpRpo(stdout);
	}

	// Free neighbor scenes at session end (after any save that only mutates the editable scene)
	// — kept alive through paint/viewer so node pointers stay valid.

	// --panel-save-test: after the script, invoke the same zpSaveTo / zpSaveOverwrite the
	// interactive modal uses (headless, no driver). Never writes into the source graphics tree.
	if (!panelSaveTest.empty() && rc == 0)
	{
		// Output directory: directory of --out if given, else cwd
		std::string outDir = NLMISC::CPath::getCurrentPath();
		if (args.haveLongArg("out"))
		{
			std::string outArg = args.getLongArg("out")[0];
			std::string p = NLMISC::CFile::getPath(outArg);
			if (!p.empty())
				outDir = p;
			else if (NLMISC::CFile::isDirectory(outArg))
				outDir = outArg;
		}
		if (!outDir.empty() && outDir[outDir.size() - 1] != '/' && outDir[outDir.size() - 1] != '\\')
			outDir += "/";
		std::string base = NLMISC::CFile::getFilenameWithoutExtension(input);
		std::string baseWithExt = NLMISC::CFile::getFilename(input);

		// Install paint ctx so zpSaveTo / zpSaveOverwrite see the core + scene
		g_PaintCtx = SPaintCtx();
		g_PaintCtx.Active = true;
		g_PaintCtx.Core = &core;
		g_PaintCtx.Scene = lm.Scene;
		g_PaintCtx.InputPath = input;
		g_PaintCtx.InteractiveSave = true;

		if (panelSaveTest == "copy")
		{
			std::string target = outDir + base + "_painted.max";
			if (!zpSaveTo(target))
			{
				g_PaintCtx = SPaintCtx();
				return 1;
			}
			printf("OK panel-save-test copy -> %s\n", target.c_str());
		}
		else // overwrite
		{
			// Multi-file (M6b): editable paths must already be sandbox copies (e.g. under /tmp).
			// zpSaveOverwrite walks g_EditableFiles; refuse anything outside /tmp.
			if (g_EditableFiles.size() > 1)
			{
				for (size_t i = 0; i < g_EditableFiles.size(); ++i)
				{
					const std::string &p = g_EditableFiles[i].Path;
					if (p.find("/tmp/") == std::string::npos)
					{
						fprintf(stderr, "ERROR: panel-save-test multi overwrite refuses path outside /tmp: %s\n",
						        p.c_str());
						g_PaintCtx = SPaintCtx();
						return 1;
					}
				}
				if (!zpSaveOverwrite())
				{
					g_PaintCtx = SPaintCtx();
					return 1;
				}
				printf("OK panel-save-test multi overwrite (%u editable files)\n",
				       (uint)g_EditableFiles.size());
			}
			else
			{
				// Operate on a COPY of the input in outDir — never the real source file
				std::string workCopy = outDir + baseWithExt;
				if (NLMISC::CFile::getPath(NLMISC::CPath::makePathAbsolute(workCopy, outDir, true))
				    == NLMISC::CFile::getPath(NLMISC::CPath::makePathAbsolute(input, NLMISC::CPath::getCurrentPath(), true))
				    && NLMISC::CFile::getFilename(workCopy) == NLMISC::CFile::getFilename(input)
				    && NLMISC::CPath::makePathAbsolute(workCopy, outDir, true)
				       == NLMISC::CPath::makePathAbsolute(input, NLMISC::CPath::getCurrentPath(), true))
				{
					// Same path as input — refuse
					fprintf(stderr, "ERROR: panel-save-test overwrite would touch the real input; give --out <dir/file> outside the source tree\n");
					g_PaintCtx = SPaintCtx();
					return 1;
				}
				if (NLMISC::CFile::fileExists(workCopy))
					NLMISC::CFile::deleteFile(workCopy);
				if (!NLMISC::CFile::copyFile(workCopy, input, false))
				{
					fprintf(stderr, "ERROR: panel-save-test: cannot copy input to %s\n", workCopy.c_str());
					g_PaintCtx = SPaintCtx();
					return 1;
				}
				std::string bakPath = workCopy + ".bak";
				g_PaintCtx.InputPath = workCopy;
				// Keep g_EditableFiles[0].Path pointing at workCopy for dirty/save
				if (!g_EditableFiles.empty())
					g_EditableFiles[0].Path = workCopy;
				if (!zpSaveOverwrite())
				{
					g_PaintCtx = SPaintCtx();
					return 1;
				}
				printf("OK panel-save-test overwrite -> %s (bak %s)\n",
				       workCopy.c_str(), NLMISC::CFile::fileExists(bakPath) ? "present" : "missing");
			}
		}
		g_PaintCtx = SPaintCtx();
		return rc;
	}

	// Save flows: --null-edit (untouched write-back, optional byte-compare) or --save (after ops)
	if (nullEdit)
	{
		std::string err;
		if (!core.writeBack(err)) { fprintf(stderr, "ERROR: write-back: %s\n", err.c_str()); return 1; }
		int saveRc = saveWholeFile(input, args.getLongArg("out")[0], *lm.Scene, args.haveLongArg("verify-identical"));
		return saveRc ? saveRc : rc;
	}
	if (!savePath.empty())
	{
		// Route through zpSaveTo so multi-file dirty policy (M6b) is enforced:
		// --save is single-path and errors when more than one editable file is dirty.
		g_PaintCtx = SPaintCtx();
		g_PaintCtx.Active = true;
		g_PaintCtx.Core = &core;
		g_PaintCtx.Scene = lm.Scene;
		g_PaintCtx.InputPath = input;
		g_PaintCtx.SavePath = savePath;
		g_PaintCtx.WantThumbnail = g_CliWantThumbnail;
		if (!zpSaveTo(savePath))
		{
			g_PaintCtx = SPaintCtx();
			return 1;
		}
		g_PaintCtx = SPaintCtx();
	}

	for (size_t i = 0; i < g_NeighborScenes.size(); ++i)
		delete g_NeighborScenes[i];
	g_NeighborScenes.clear();
	for (size_t i = 0; i < g_ExtraEditableScenes.size(); ++i)
		delete g_ExtraEditableScenes[i];
	g_ExtraEditableScenes.clear();
	g_EditableFiles.clear();

	return rc;
}

/* end of file */
