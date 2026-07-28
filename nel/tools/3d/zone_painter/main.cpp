/**
 * \file main.cpp
 * \brief Standalone zone painter: CLI entry + argument parsing + top-level dispatch.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * Loads a .max through pipeline_max, assembles the painting landscape, paints tiles /
 * colors / displace against a chosen bank, and saves byte-faithfully. Default mode is
 * the interactive viewer; headless modes cover --screenshot / --paint-script / --lua-script
 * / --null-edit / --dump-zones / --dump-rpo and related probes. See --help.
 *
 * This TU owns CLI parsing + global state definitions + main() dispatch. Implementation
 * lives in the topical TUs (see zp_state.h for their declarations):
 * scene_paint - zone eligibility, build, weld, instance placement
 * hints_and_footprint - neighbor-hint appdata + footprint mask math
 * save_ops - whole-file write-back, thumbnails, atomic copy
 * script_and_ui - paint-script executor + brush/tile/prop UI actions + script host
 * board_session - working-set rebuild + eco scratch board ops
 * session_ops - continent per-file open/close/save/toggle
 * viewer - runViewer main loop + CPaintMouseListener bodies
 *
 * Scene-assembly rules (paint_core sees this shape): eligible RklPatches -> evalNodePatch +
 * object TM at t=0 -> buildPatchInfo in AUTHORED space. Optional ecosystem self-instances
 * (--place dx,dy[,rot][,m]) clone display zones about the footprint block center; place
 * (dx,dy) is the min-corner cell of the transformed block. Display clones share the source
 * carrier by Node pointer; ids from kInstanceZoneIdBase. Per-zone Rotate/Symmetry feed
 * transformDesc. Cross-zone open-edge weld (WELD_THRESOLD, session-only) -> CZone::build
 * -> CZoneCornerSmoother -> Landscape.addZone. Frozen (0x0976) nodes are display+weld only.
 *
 * Interior paint through an R-rotated instance is byte-identical to the compensated primary
 * op (tile/256/fill store rot (r+R)&3; color/displace identity on UV).
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


// Patch-state eval + RPO->CPatchInfo conversion: the shared unit of the zone exporter and this
// painter. Header-only static implementation unit (zone x87 tier is TU-sensitive; see the
// header doc for the include contract this file follows).
#include "../pipeline_max_export_common/patch_eval.h"

// The tile painting core: metaTile graph, transition solver, pristine carrier state,
// live-landscape mirror, undo, write-back.
#include "paint_core.h"

// Include-meshes context display + scene lights (own TU; the SCENELIB evaluation
// headers must not share a TU with the patch_eval implementation unit above).
#include "context_display.h"

// In-engine NLGUI shell (own TU; must not pull patch_eval / SCENELIB into editor_ui.cpp).
#include "editor_ui.h"
#include <nel/gui/ctrl_base.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/ctrl_text_button.h>
#include <nel/gui/widget_manager.h>

// Workspace fingerprint / discovery / startup.cfg (own TU; NLMISC only).
#include "workspace_discovery.h"

// Startup screens A/B/C (own TU; NLGUI + discovery; no patch_eval / SCENELIB).
#include "startup_ui.h"

// painterscript: MaxScript-like Lua over the op layer (own TU, NLGUI-lua only).
#include "script_api.h"

// Cross-TU state, types, and forward decls shared by every implementation TU.
#include "zp_state.h"
#include "viewer_listener.h" // CPaintMouseListener::SubCount, for the sub-object digit run

bool g_verbose = false;
// Result of the viewer script pre-pass (propagated as the viewer exit code for scripted gates)
int g_ViewerScriptRc = 0;
// Viewer paint defaults (CLI-configurable)
NLMISC::CRGBA g_ViewerBrushColor(255, 255, 255, 255);
float g_ViewerBrushRadius = 8.f;
uint g_ViewerBrushHardness = 128;
uint g_ViewerBrushOpacity = 255;
uint g_ViewerDisplaceIndex = 0;
bool g_PreloadTiles = false;
bool g_IncludeMeshes = false;
std::string g_InputPath;
std::string g_BankPath;
// Extra texture search path derived by the startup flow (empty on the legacy CLI path)
std::string g_StartupTexturePath;
// When true, bank directory is searched recursively (startup flow always wants this)
bool g_ForceBankRecursive = false;
// Startup interactive (or startup-auto) without --save: panel Save opens the modal
bool g_InteractiveSave = false;
// Continent/ecosystem neighbor loading: default on for startup/auto, off for legacy
bool g_LoadNeighbors = false;
// Board-driven session (startup interactive / --startup-auto). Not the legacy direct-.max path.
bool g_BoardSession = false;
// Escape hatch: board sessions display embedded non-eligible/frozen copies (default: skip).
bool g_EmbeddedContext = false;
// World/zone selected via startup (for neighbor discovery); empty on legacy path
ZPWS::SWorldEntry g_StartupWorld;
ZPWS::SZoneEntry g_StartupZone;
// Multi-select editable set; empty means single g_StartupZone only
std::vector<ZPWS::SZoneEntry> g_StartupEditableZones;
// RO context files loaded for the session (continent ring / eco hints / place-context).
// Owns SLoadedMax; cleared on rebuild. Cell offsets are relative to primary footprint origin.
std::vector<SContextFile> g_ContextFiles;
// Session hint cells (board space, primary-relative): every hint named by any open
// editable, whether currently loaded as context or not. Board menus offer these per cell.
std::vector<SSessionHintCell> g_SessionHintCells;
// Legacy name used throughout; points at scenes inside g_ContextFiles
std::vector<PMAXLOAD::SLoadedMax *> g_NeighborScenes;
// Every editable file's heap-owned scene, the first-opened included. Close frees through
// this registry; final teardown deletes what remains.
std::vector<PMAXLOAD::SLoadedMax *> g_ExtraEditableScenes;
// Per-editable-file zone-id membership for dirty tracking / per-file save
std::vector<SEditableFileInfo> g_EditableFiles;
// The first-opened file's scene (heap-owned like every other, registered in
// g_ExtraEditableScenes); NULLed when that file closes. Legacy fallback reads only.
PMAXLOAD::SLoadedMax *g_PrimaryLm = NULL;
// When true, every non-forceFrozen zone is a paint target (open-everything mode).
// Default false = exporter-faithful eligibility (one editable zone per normal .max).
bool g_AllZones = false;
// Session rebuild live pointers (valid only while runViewer runs)
float g_SessionCellSize = 160.f;
float g_SessionSnap = 1.f;
bool g_SessionLockBorders = false;
NL3D::CTileBank *g_SessionBank = NULL;
// Ecosystem self-instances: board-driven placements. Primary zones sit at the layout origin
// (rot 0, no mirror); each --place adds a display-level duplicate sharing the same Node
// pointers (paint_core carrier keying) with geometry transformed about the FOOTPRINT-block
// center and translated so the transformed block's min-corner lands at the place origin
// cell. --instances NxM is a translation-only alias that expands to placements for the
// non-origin cells of a grid of whole footprints.
//
// Place convention: CellX,CellY = origin (min-corner) of the TRANSFORMED footprint block
// in fine cells of --cellsize, relative to the primary footprint origin cell (0,0).
// Primary home occupies [0,fw)×[0,fh). A rot-0 instance at (fw,0) sits immediately east of
// home (adjacent blocks). Rot 1/3 transpose occupancy to fh×fw. Overlapping blocks refused.
// Instance zone ids use the sparse base kInstanceZoneIdBase (CBorderVertex ids are uint16).
std::vector<SInstancePlace> g_Places; // empty = primary only
// Primary footprint in fine cells (set by appendInstanceZones / computeFootprintRect / mask)
int g_FootprintCellsW = 1;
int g_FootprintCellsH = 1;
// Exporter-identical occupancy mask over [0,W)×[0,H). Empty → treat as filled rect.
std::vector<bool> g_FootprintMask;
float g_FootprintOriginX = 0.f; // world min-corner of cell (0,0) (AABB snap fallback)
float g_FootprintOriginY = 0.f;
bool g_FootprintFromTemplate = false; // true = CZoneTemplate::getMask; false = AABB square
// Legacy NxN reporting (--instances / Screen B layout)
uint g_InstanceCols = 1;
uint g_InstanceRows = 1;
uint g_InstanceCount = 1; // 1 + g_Places.size()
// Shipped brush mask cycle (viewer SelectColorBrush key; 0 = none, i = g_MaskFiles[i-1])
std::vector<std::string> g_MaskFiles;
int g_MaskCycle = 0;

// Prop mode selection: persists across mode switches, visible only in Prop mode,
// cleared on working-set rebuild (session open/close/place). Hover is transient.
bool g_HavePropSelection = false;
uint g_SelectedZoneId = 0;
std::set<TPatchVertId> g_PatchVertSel;
std::string g_PropStatusMsg; // click "read-only" / selection name (HUD + panel)

// Three parallel tables indexed by TPainterKey; the sizes are left implicit and checked
// below, so adding an action without extending all three is a compile error rather than a
// silently zero-filled (and therefore dead, or unnamed) binding.
typedef char zpCheckKeyNames[(sizeof(kPainterKeysName) / sizeof(kPainterKeysName[0])) == ZPK_KeyCounter ? 1 : -1];
typedef char zpCheckKeyDefaults[(sizeof(g_PainterKeys) / sizeof(g_PainterKeys[0])) == ZPK_KeyCounter ? 1 : -1];
typedef char zpCheckKeyModes[(sizeof(kPainterKeyModes) / sizeof(kPainterKeyModes[0])) == ZPK_KeyCounter ? 1 : -1];

// ---------------------------------------------------------------------------------------------
// keys.cfg / vars.cfg (plugin paint_ui.cpp LoadKeyCfg/LoadVarCfg port). The plugin read BOTH
// variable sets from one keys.cfg next to the plugin dll (NLMISC::CConfigFile: the file itself
// defines the Key* constants, then `Action = KeyX;` assignments; the original keys.cfg parses
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


// The plugin's cfg variable names (paint_ui.cpp PainterKeysName, order preserved)
const char *kPainterKeysName[] =
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
	"ModeProp",
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
	"ZoomExtentsSelected",
	"TileSetPrev",
	"TileSetNext",
	"TileSetDigits",
	"DisplacePrev",
	"DisplaceNext",
	"Undo",
	"Redo",
	"Redo2",
	"ViewUndo",
	"ViewRedo",
	"Screenshot",
	"ModePatch",
	"SubObjectDigits",
};

// Tool defaults: the pre-cfg hardcoded viewer keys stay on their keys (T/C/D, +/-, B, G, F);
// new actions land on free keys (documented in --help). 0 = unbound.
uint g_PainterKeys[] =
{
	0, // Select (in-plugin paint-modifier action; no tool equivalent)
	0, // Pick (the tool picks on right mouse, hardcoded)
	NLMISC::KeyF, // Fill0 (the pre-cfg F fill, rotation 0)
	NLMISC::KeyF6, // Fill1 (plugin default)
	NLMISC::KeyF7, // Fill2 (plugin default)
	NLMISC::KeyF8, // Fill3 (plugin default)
	NLMISC::KeyT, // ModeTile
	NLMISC::KeyC, // ModeColor
	NLMISC::KeyD, // ModeDisplace
	NLMISC::KeyR, // ModeProp (free key; T/C/D modes, O board, P palette, Y season)
	0, // ToggleColor (single brush color in this tool)
	NLMISC::KeyADD, // SizeUp
	NLMISC::KeySUBTRACT, // SizeDown
	NLMISC::KeyB, // ToggleTileSize
	NLMISC::KeyG, // GroupUp
	NLMISC::KeyV, // GroupDown (plugin default)
	0, // BackgroundColor
	0, // ToggleArrows
	NLMISC::KeyHOME, // HardnessUp (plugin PgUp/PgDn select tile sets here)
	NLMISC::KeyEND, // HardnessDown
	NLMISC::KeyINSERT, // OpacityUp
	NLMISC::KeyDELETE, // OpacityDown
	0, // Zouille
	0, // AutomaticLighting
	NLMISC::KeyS, // SelectColorBrush (cycles the shipped mask set; plugin default key)
	NLMISC::KeyQ, // ToggleColorBrushMode (plugin default)
	NLMISC::KeyL, // LockBorders (plugin default)
	0, // ZoomIn (bindable; plugin default Key1 selects a tile set here)
	0, // ZoomOut
	0, // GetState
	0, // ResetPatch
	NLMISC::KeyF10, // ToggleUI (NLGUI panel visibility)
	NLMISC::KeyY, // SeasonNext (free key; cycle season textures)
	NLMISC::KeyP, // TogglePalette (free key; tileset thumbnail palette)
	NLMISC::KeyO, // ToggleBoard (free key; session board hub)
	NLMISC::KeyZ, // ZoomExtentsSelected
	NLMISC::KeyPRIOR, // TileSetPrev (PgUp)
	NLMISC::KeyNEXT, // TileSetNext (PgDn)
	NLMISC::Key0, // TileSetDigits: base of the 0-9 run
	NLMISC::KeyLBRACKET, // DisplacePrev
	NLMISC::KeyRBRACKET, // DisplaceNext
	ZPK_BIND(NLMISC::KeyZ, ZPKM_CTRL), // Undo
	ZPK_BIND(NLMISC::KeyY, ZPKM_CTRL), // Redo
	ZPK_BIND(NLMISC::KeyE, ZPKM_CTRL), // Redo2 (what the tool shipped before)
	ZPK_BIND(NLMISC::KeyZ, ZPKM_SHIFT), // ViewUndo
	ZPK_BIND(NLMISC::KeyY, ZPKM_SHIFT), // ViewRedo
	NLMISC::KeyF12, // Screenshot
	NLMISC::KeyM, // ModePatch (free; T/C/D/R are the paint modes, M for the mesh)
	NLMISC::Key1, // SubObjectDigits: base of the 1-5 run (sub-object keys)
};

// Mode scope per action (see ZPKS_* in zp_state.h). Everything that only makes sense while
// painting is ZPKS_PAINT so the patch-edit modes can reuse those keys; everything the artist
// needs from any mode - navigation, undo, view, panels, the mode switches themselves - is
// ZPKS_ANY. Identical behaviour today: no mode outside 0-3 exists yet.
const uint8 kPainterKeyModes[] =
{
	ZPKS_PAINT, // Select
	ZPKS_PAINT, // Pick
	ZPKS_PAINT, // Fill0
	ZPKS_PAINT, // Fill1
	ZPKS_PAINT, // Fill2
	ZPKS_PAINT, // Fill3
	ZPKS_ANY, // ModeTile (mode switches must work FROM any mode)
	ZPKS_ANY, // ModeColor
	ZPKS_ANY, // ModeDisplace
	ZPKS_ANY, // ModeProp
	ZPKS_PAINT, // ToggleColor
	ZPKS_PAINT, // SizeUp
	ZPKS_PAINT, // SizeDown
	ZPKS_PAINT, // ToggleTileSize
	ZPKS_PAINT, // GroupUp
	ZPKS_PAINT, // GroupDown
	ZPKS_PAINT, // BackgroundColor
	ZPKS_PAINT, // ToggleArrows
	ZPKS_PAINT, // HardnessUp
	ZPKS_PAINT, // HardnessDown
	ZPKS_PAINT, // OpacityUp
	ZPKS_PAINT, // OpacityDown
	ZPKS_PAINT, // Zouille
	ZPKS_PAINT, // AutomaticLighting
	ZPKS_PAINT, // SelectColorBrush
	ZPKS_PAINT, // ToggleColorBrushMode
	ZPKS_ANY, // LockBorders (a paint constraint, but harmless and useful to pre-set)
	ZPKS_ANY, // ZoomIn
	ZPKS_ANY, // ZoomOut
	ZPKS_PAINT, // GetState
	ZPKS_PAINT, // ResetPatch
	ZPKS_ANY, // ToggleUI
	ZPKS_ANY, // SeasonNext
	ZPKS_ANY, // TogglePalette
	ZPKS_ANY, // ToggleBoard
	ZPKS_ANY, // ZoomExtentsSelected
	ZPKS_PAINT, // TileSetPrev <- the digit row / PgUp-PgDn belong to the patch-edit
	ZPKS_PAINT, // TileSetNext modes' sub-object levels once those exist
	ZPKS_PAINT, // TileSetDigits
	ZPKS_PAINT, // DisplacePrev
	ZPKS_PAINT, // DisplaceNext
	ZPKS_ANY, // Undo
	ZPKS_ANY, // Redo
	ZPKS_ANY, // Redo2
	ZPKS_ANY, // ViewUndo
	ZPKS_ANY, // ViewRedo
	ZPKS_ANY, // Screenshot
	ZPKS_ANY, // ModePatch (a mode switch, so it must work FROM any mode)
	ZPKS_PATCH, // SubObjectDigits <- shares the digit row with TileSetDigits above
};

// paint_ui.cpp light/zoom variable defaults (LoadVarCfg overrides; identical to the previous
// hardcoded painting-scene constants)
NLMISC::CVector g_LightDirection(1.f, 1.f, -1.f);
NLMISC::CRGBA g_LightDiffuse(255, 255, 255);
NLMISC::CRGBA g_LightAmbiant(0, 0, 0);
float g_LightMultiply = 1.f;
float g_ZoomSpeed = 300.f;

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

// LoadVarCfg port: the exact plugin variable set: LightDirection (3 floats), LightDiffuse /
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
NLMISC::CEventListenerAsync *g_ViewerAsync = NULL;
// painterscript: --lua-script path; file-static so runViewer's pre-pass sees
// it without widening the signature (mirrors scriptPath's flow).
std::string g_LuaScriptPath;
std::string g_StartupLuaPath; // --startup-lua (else ./zone_painter_startup.lua)
// painterscript pump: while a script pumps the UI, viewer input is locked
// (paint/nav ignored; ESC requests cancel). Checked by CPaintMouseListener.
bool g_ScriptUiLock = false;

// Session anchor / legacy source (see zp_state.h for docstrings; definitions here so main
// and every extracted TU see one canonical instance).
float g_SessionAnchorX = 0.f, g_SessionAnchorY = 0.f;
std::string g_LegacyPlaceSourceName;
bool g_SessionAnchorSet = false;
bool g_HintStampEnabled = true;

// Board-op recorder depth (outermost op records)
int g_BoardOpDepth = 0;

// Paint context + save state (viewer-lifetime storage)
std::vector<std::string> g_AvailableSeasons;
SPaintCtx g_PaintCtx;
bool g_CliWantThumbnail = false;
bool g_NoThumbnailWrites = false;
NLMISC::CBitmap g_CapturedThumb;
bool g_HaveCapturedThumb = false;
std::string g_LastSaveStatus;

// Place-context / open-editable specs (CLI + scratch board mutates)
std::vector<SPlaceContextSpec> g_PlaceContextSpecs;
std::vector<SOpenEditableSpec> g_OpenEditableSpecs;

// Script host state (installed by runViewer)
bool (*g_ScriptScreenshotFn)(const std::string &path, std::string &err) = NULL;
void (*g_ScriptPumpFn)() = NULL;
bool g_ScriptCancel = false;
bool g_SessionOpsAvailable = false;
ZPSCRIPT::SScriptHost g_ScriptHost;
SScriptPumpCtx g_PumpCtx;

// Viewer main-window constants (extern in zp_state.h)
const uint kMainWidth = 800;
const uint kMainHeight = 600;

// SBoardOpScope ctor/dtor (definition; declared in zp_state.h)
SBoardOpScope::SBoardOpScope() { ++g_BoardOpDepth; }
SBoardOpScope::~SBoardOpScope() { --g_BoardOpDepth; }

/** Modifier bits currently held, in ZPKM_* terms. */
static uint zpHeldMods()
{
	if (!g_ViewerAsync)
		return 0;
	uint m = 0;
	if (g_ViewerAsync->isKeyDown(NLMISC::KeyCONTROL)) m |= ZPKM_CTRL;
	if (g_ViewerAsync->isKeyDown(NLMISC::KeySHIFT)) m |= ZPKM_SHIFT;
	if (g_ViewerAsync->isKeyDown(NLMISC::KeyMENU)) m |= ZPKM_ALT;
	return m;
}

/** The action is live in the mode the painter is currently in (ZPKS_* scope mask). */
static bool zpModeAllows(TPainterKey action)
{
	const uint8 scope = kPainterKeyModes[action];
	if (scope == ZPKS_ANY)
		return true;
	const int mode = zpCurrentPaintMode();
	if (mode < 0 || mode > 7)
		return true; // headless / unknown: nothing is mode-restricted
	return (scope & (1 << mode)) != 0;
}

/** Modifiers must match EXACTLY: an action bound to Z must not fire on Shift+Z. */
static bool zpBindingMatches(TPainterKey action, uint &keyOut)
{
	const uint b = g_PainterKeys[action];
	if (b == 0 || !g_ViewerAsync)
		return false;
	if (zpHeldMods() != ZPK_MODS(b))
		return false;
	if (!zpModeAllows(action))
		return false;
	keyOut = ZPK_KEY(b);
	return true;
}

bool zpKeyPushed(TPainterKey action)
{
	uint k = 0;
	return zpBindingMatches(action, k) && g_ViewerAsync->isKeyPushed((NLMISC::TKey)k);
}

bool zpKeyDown(TPainterKey action)
{
	uint k = 0;
	return zpBindingMatches(action, k) && g_ViewerAsync->isKeyDown((NLMISC::TKey)k);
}

bool zpKeyDigitPushed(uint digit)
{
	uint base = 0;
	if (digit > 9 || !zpBindingMatches(ZPK_TileSetDigits, base))
		return false;
	return g_ViewerAsync->isKeyPushed((NLMISC::TKey)(base + digit));
}

bool zpKeySubObjPushed(uint level)
{
	uint base = 0;
	if (level >= CPaintMouseListener::SubCount || !zpBindingMatches(ZPK_SubObjDigits, base))
		return false;
	// The run is 1-based on the keyboard (1-5) and 0-based in EP_*, so level 0 is the
	// key the binding names and the rest follow it.
	return g_ViewerAsync->isKeyPushed((NLMISC::TKey)(base + level));
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
	                    "Top toolbar: movable bar with left drag grip (client hands-bar idiom),\n"
	                    "square text buttons BOARD SAVE | UNDO REDO | TILE COLOR DISP PROP | <season face>. Season opens a\n"
	                    "context menu of available seasons; mode buttons show pushed state. Keys T/C/D/R, O, Y, undo/redo\n"
	                    "still work. Slim Painter panel: tile set ± / 256 / brush / group, Color section, Displace,\n"
	                    "Prop mode (zone select + boundary outlines; property panel),\n"
	                    "lock borders, fill, Tiles palette toggle, multi-file dirty. Save modal same as before.\n"
	                    "Tiles palette : second movable window with one thumbnail cell per bank tileset\n"
	                    "(64px preview from the first resolvable 128 diffuse, name + 1-based index). Click selects\n"
	                    "through the same absolute handler as digit keys / right-click pick; season change rebuilds\n"
	                    "previews. Toggle with P (TogglePalette) or the panel Tiles button.\n"
	                    "Config files (plugin keys.cfg port, NLMISC::CConfigFile syntax; one file may serve both):\n"
	                    " keys cfg (--keys-cfg, else ./zone_painter_keys.cfg): rebinds the actions by name,\n"
	                    " values are NeL TKey codes (the plugin's keys.cfg Key* constant block parses verbatim).\n"
	                    " A binding may carry modifiers: value = key + 65536*mods, mods = 1 Ctrl | 2 Shift | 4 Alt\n"
	                    " (so Ctrl+Z is 90+65536). Modifier matching is EXACT - an action on Z does not fire on\n"
	                    " Shift+Z. Actions are also MODE-SCOPED: the paint bindings (tile set, brush, fill,\n"
	                    " hardness/opacity, displace) are live in the paint modes only, so the digit row and the\n"
	                    " transform keys stay free for the patch-edit sub-object levels; navigation, undo, view,\n"
	                    " panels and the mode switches are live everywhere.\n"
	                    " Honored: ModeTile ModeColor ModeDisplace ModeProp SizeUp SizeDown ToggleTileSize GroupUp GroupDown\n"
	                    " Fill0 Fill1 Fill2 Fill3 HardnessUp HardnessDown OpacityUp OpacityDown SelectColorBrush\n"
	                    " ToggleColorBrushMode LockBorders ZoomIn ZoomOut ToggleUI SeasonNext TogglePalette ToggleBoard\n"
	                    " ZoomExtentsSelected\n"
	                    " (defaults: T C D R + - B G V F F6 F7 F8 Home End Insert Delete S Q L, F10, Y, P, Z; zoom unbound).\n"
	                    " ModeProp (default R): property-edit mode - hover thin zone outline, click selects thick;\n"
	                    " only editable (unfrozen primary) zones; RO/instance click reports read-only .\n"
	                    " Accepted+ignored (no tool equivalent): Select Pick ToggleColor BackgroundColor ToggleArrows\n"
	                    " Zouille AutomaticLighting GetState ResetPatch.\n"
	                    " vars cfg (--vars-cfg, else ./zone_painter_vars.cfg): LightDirection {x,y,z}, LightDiffuse {r,g,b},\n"
	                    " LightAmbiant {r,g,b}, LightMultiply, ZoomSpeed (the plugin LoadVarCfg set).\n"
	                    " ToggleUI (default F10: show/hide the in-engine NLGUI panel + toolbar).\n"
	                    " SeasonNext (default Y: cycle landscape season textures among variants that exist for\n"
	                    " the open bank - spring/summer/autumn/winter; paint indices/colors/displace untouched;\n"
	                    " tileset palette previews re-resolve to the new season; toolbar season face updates).\n"
	                    " TogglePalette (default P: show/hide the Tiles thumbnail palette).\n"
	                    " ToggleBoard (default O: session board hub - continent working set, or ecosystem\n"
	                    " scratch board for brick instances: empty cell places an instance; instance cell\n"
	                    " popup Rotate CW/CCW / Mirror / Remove; home = open brick. Labels show R90/M.\n"
	                    " Continent: L-click closed=open, open=Close/Save/Toggle. BACK TO PAINTING / O\n"
	                    " returns. Working-set / place changes rebuild landscape+weld and CLEAR undo).\n"
	                    "Navigation (middle-button set; the LEFT button is never navigation, it\n"
	                    "belongs to paint/select): MMB pan, Alt+MMB orbit, Ctrl+Alt+MMB dolly, Ctrl+MMB fast\n"
	                    "pan, Shift+MMB axis-locked pan, wheel stepped zoom. Orbit/dolly/wheel all pivot on\n"
	                    "the VIEW TARGET, which pans with the camera and is reset by Z - not on a point\n"
	                    "frozen at session start.\n"
	                    " ZoomExtentsSelected (default Z): frames the last edit, else the hovered tile,\n"
	                    " else the Prop selection, else every editable zone. Shift+Z / Shift+Y step the\n"
	                    " view history.\n"
	                    " Also honored (were hardcoded, now rebindable + mode-scoped): TileSetPrev TileSetNext\n"
	                    " TileSetDigits (base of the 0-9 run) DisplacePrev DisplaceNext Undo Redo Redo2 ViewUndo\n"
	                    " ViewRedo Screenshot (defaults PgUp PgDn 0 [ ] Ctrl+Z Ctrl+Y Ctrl+E Shift+Z Shift+Y F12).\n"
	                    "Patch edit (ModePatch, default M): shows the patch control cage for every editable\n"
                    "  zone. SubObjectDigits (base of the 1-5 run, default 1) picks the sub-object level -\n"
                    "  1 Object, 2 Vertex, 3 Edge, 4 Patch, 5 Tile, the same order and meaning as the\n"
                    "  legacy plugin modifier. The digit row is shared with TileSetDigits: only one of the two is\n"
                    "  live, because each is scoped to the modes it belongs to. Entering the mode lands on\n"
                    "  Object level. At Vertex level: click selects a vertex,\n"
                    "  Ctrl adds, Alt removes, a click on nothing clears; the move gizmo appears on\n"
                    "  the selection and dragging an axis / plane / its centre moves the selected\n"
                    "  vertices. BOUND vertices (drawn black) are never moved - they are recomputed\n"
                    "  from the edge they bind into, so a written position could not survive a\n"
                    "  reload; they follow when that edge moves. Rotated or mirrored zones refuse\n"
                    "  the move for now. Moves are undoable as one step per drag.\n"
	                    "ESC is the only fixed key: closes the session board, else quits; also cancels a script.");
	// Optional first positional: .max (legacy) or folder (startup seed). Absent => startup flow.
	args.addAdditionalArg("input", "Input .max scene (legacy) or graphics/seed folder (startup); omit for discovery", true, false);
	args.addArg("", "bank", "bank", "Tile bank (.smallbank/.bank); required for the legacy .max path (auto-derived in startup flow)");
	args.addArg("", "bank-recursive", "", "Add the bank directory to the texture search path recursively");
	args.addArg("", "search-path", "dir", "Extra recursive texture search path (repeatable)", false);
	args.addArg("", "out", "output.max", "Output .max for --null-edit (in-place save is refused)");
	args.addArg("", "save", "output.max",
	            "Write-back + whole-file save after ops (in-place save is refused). "
	            "Multi-select sessions : errors if more than one editable file is dirty - "
	            "use the interactive Save modal (Overwrite all) for multi-file write-back.");
	args.addArg("", "cellsize", "meters", "Ligo cell size for the zone-symmetry state (default 100)");
	args.addArg("", "snap", "meters", "Ligo snap for the zone-symmetry state (default 1)");
	args.addArg("", "paint-script", "file", "Scripted paint ops (headless without a display mode)");
	args.addArg("", "lua-script", "file", "painterscript: Lua over the same op layer (painter.* API; "
	            "headless or viewer pre-pass; see the wiki painterscript stories)");
	args.addArg("", "startup-lua", "file", "painterscript run once at viewer-session start "
	            "(default: ./zone_painter_startup.lua when present; errors never abort the viewer)");
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
	args.addArg("", "check-ligozone", "file.ligozone",
	            "Test/CI only: after .max-derived footprint, compare mask/size to an existing "
	            ".ligozone export artifact. Authoring never requires .ligozone (build output).");
	args.addArg("", "dump-rpo", "", "Dump every carrier's pristine tile records to stdout");
	args.addArg("", "dump-bank-xref", "", "Dump the bank's tile -> (set, number, type) xref table to stdout");
	args.addArg("", "dump-carrier-blob", "dir", "Write each zone's original carrier blob bytes to <dir>/zone<id>.blob");
	args.addArg("", "screenshot", "out.tga", "Render one frame to a .tga and exit");
	args.addArg("", "font", "file.ttf", "Enable 3D HUD overlay text with this TrueType font (off unless given; NLGUI uses a system font either way)");
	args.addArg("", "verbose", "", "Verbose output");
	// Test plumbing (thin wrappers over the same selection functions the UI buttons call)
	args.addArg("", "startup-auto", "workspace/zone[+zone...][?query]",
	            "Skip startup UI: select workspace+zone by name and open the viewer. "
	            "Multi-select (continents): plus-separated zone basenames "
	            "(\"world/zoneA+zoneB+zoneC\") opens all editable plus the union of their "
	            "8-rings as frozen context (neighbors default on; ?neighbors=off respected). "
	            "Basenames may be bare grid names (4_AC) or prefixed (zonematerial-converted-193_ec). "
	            "Optional ?query after the zone list: ampersand-separated key=value pairs. "
	            "Supported keys: neighbors=on|off (continents; default on), "
	            "place=dx,dy[,rot][,m] (ecosystem self-instances; see --place), "
	            "instances=NxM (deprecated alias for translation-only places; see --instances). "
	            "Examples: lacustre/material-fond?place=1,0,1 "
	            "snowballs/4_AC+4_AD "
	            "fyros_newbieland/15_AE?neighbors=off "
	            "lacustre/material-fond?instances=2x1&neighbors=off");
	args.addArg("", "startup-screenshot", "out.tga", "Render one frame of the first startup screen and exit");
	args.addArg("", "startup-screen", "world",
	            "With --startup-screenshot: pre-select this world (name or graphics-root basename) and capture Screen B "
	            "(continent coordinate grid or ecosystem zone list) instead of Screen A");
	args.addArg("", "panel-save-test", "copy|overwrite",
	            "Headless: after --paint-script, invoke the same panel save path (zpSaveTo / zpSaveOverwrite). "
	            "copy writes <basename>_painted.max under --out's directory (or cwd). overwrite first copies the "
	            "input .max into that directory and operates on the copy (never the real source). Requires --paint-script.");
	args.addArg("", "neighbors", "on|off",
	            "Board/continent startup: load neighbor .max files as frozen read-only context "
	            "via the neighbor hint chain (appdata → embedded names → continent grid; default on "
	            "for interactive/startup-auto, off for the legacy .max path). "
	            "Also accepted as ?neighbors=off on --startup-auto.");
	args.addArg("", "embedded-context", "",
	            "Board sessions: display embedded non-eligible/frozen zone copies from the open "
	            "file (legacy display). Default is board authority - those copies are skipped and used "
	            "only as neighbor-name hints. Legacy direct-.max path always shows them.");
	args.addArg("", "place-context", "dx,dy:basename",
	            "Ecosystem/board: load an existing world brick file as frozen read-only context "
	            "at fine-cell offset (dx,dy) relative to the primary footprint origin. Repeatable. "
	            "Headless equivalent of the scratch-board context placement .",
	            false);
	args.addArg("", "open-editable", "cx,cy:basename",
	            "Ecosystem startup: open an additional world brick as EDITABLE with its footprint "
	            "origin at board cell (cx,cy) (repeatable; the board Open editable's CLI form)",
	            false);
	args.addArg("", "dump-neighbor-hints", "file.max",
	            "Headless: print the painter neighbor-hints appdata (and embedded fallback) for "
	            "the eligible node of file.max; exit. Does not open the viewer.");
	args.addArg("", "dump-zone-props", "file.max",
	            "Headless: print Ligo Rotate / Symmetry / Passable / Use Bounding Box appdata "
	            "for every zone node in file.max; exit (verification hook).");
	args.addArg("", "season", "sp|su|au|wi",
	            "Initial landscape season texture preference (spring/summer/autumn/winter). "
	            "Default: auto (first available postfix, historically spring). Only seasons that "
	            "resolve for the open bank are offered by SeasonNext / the panel button; painting "
	            "data is season-independent.");
	args.addArg("", "all-zones", "",
            "Escape hatch: open every non-frozen (non-0x0976) RklPatch as a paint target "
            "(legacy open-everything). Default is by design ONE editable zone per .max "
            "(exporter-faithful): zonematerial-/zonespecial- → single non-frozen "
            "RklPatch (name-match cell token when multiple); zonetransition- → all "
            "non-frozen (9-slot scheme exception); else ExportRykolZone first findID-able "
            "node. Other zones load as read-only context. [NELLIGO] markers skipped. "
            "Null-edit / write-back still whole-file; paint-script zone ids address "
            "PAINTABLE zones only.");
args.addArg("", "place", "dx,dy[,rot][,m]",
	            "Ecosystem: place a self-instance whose TRANSFORMED footprint block has min-corner "
	            "at fine cell (dx,dy) relative to the primary footprint origin (0,0). Optional "
	            "rotation 0..3 (90° CCW) and mirror (m|1|true). Repeatable. Primary home occupies "
	            "[0,fw)×[0,fh) cells (fw,fh from AABB ceil'd to --cellsize); a rot-0 instance at "
	            "(fw,0) sits immediately east of home. Geometry is rotated/mirrored about the "
	            "FOOTPRINT block center (origin + half step), then translated so the block origin "
	            "lands at (dx,dy). Initial tile display remounts U under mirror and applies "
	            "transformTile . Display clones share one paint carrier; picks/paint ops "
	            "inverse-map through transformDesc. Overlapping blocks refused on the scratch "
	            "board. Ecosystem-only. Also ?place=dx,dy,rot on --startup-auto.",
	            false);
args.addArg("", "instances", "NxM",
	            "DEPRECATED alias : expands to translation-only --place for each non-origin "
	            "cell of an NxM grid (supported: 1x1, 2x1, 1x2, 2x2, 3x3). Prefer --place. "
	            "Ecosystem-only. Same as ?instances=NxM on --startup-auto. "
	            "Ignored with a warning on the legacy .max path.");
	args.addArg("", "thumb-extract", "out.tga",
	            "Headless: extract OLE PIDSI_THUMBNAIL from the input .max into out.tga (and refresh thumbcache); exit");
	args.addArg("", "thumb-roundtrip-test", "file.max",
	            "Headless: parse and re-encode SummaryInformation with the UNCHANGED thumbnail; "
	            "exit 0 only if the stream is byte-identical (property-set gate)");
	args.addArg("", "thumbnail", "",
	            "With --save: render a top-down orthographic thumbnail of the primary zone and "
	            "write it into SummaryInformation PIDSI_THUMBNAIL (requires a display - use "
	            "xvfb-run for headless). Plain --save and --null-edit never touch the thumbnail. "
	            "Interactive Save modal has an 'update thumbnail' checkbox (default on).");
	args.addArg("", "no-thumbnail", "",
	            "Never write SummaryInformation thumbnails, on ANY save path (interactive "
	            "included; overrides --thumbnail and the modal checkbox). Byte gates use this "
	            "so session saves keep the SI stream provably untouched.");
	args.addArg("", "no-hint-stamp", "",
	            "Board sessions: do not stamp the neighbor-hint appdata on save. Saves become "
	            "byte-pure (== the null-edit output of the same file) at the cost of the "
	            "layout-memory reopen chain.");
	if (!args.parse(argc, argv))
		return 1;

	g_verbose = args.haveLongArg("verbose");
	g_CliWantThumbnail = args.haveLongArg("thumbnail");
	g_NoThumbnailWrites = args.haveLongArg("no-thumbnail");
	g_HintStampEnabled = !args.haveLongArg("no-hint-stamp");
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
			std::vector<std::string> cfv;
			{
				std::string cur;
				for (std::string::size_type ci = 0; ci <= coords.size(); ++ci)
				{
					char c = ci < coords.size() ? coords[ci] : ',';
					if (c == ',') { cfv.push_back(cur); cur.clear(); }
					else cur += c;
				}
			}
			SPlaceContextSpec pc;
			if (cfv.size() < 2 || cfv.size() > 4
			    || !NLMISC::fromString(cfv[0], pc.Dx)
			    || !NLMISC::fromString(cfv[1], pc.Dy)
			    || base.empty())
			{
				fprintf(stderr, "ERROR: --place-context expects dx,dy[,rot][,m]:basename, got '%s'\n",
				        pcs[i].c_str());
				return 1;
			}
			if (cfv.size() >= 3 && !cfv[2].empty())
			{
				uint r = 0;
				if (!NLMISC::fromString(cfv[2], r) || r > 3)
				{
					fprintf(stderr, "ERROR: --place-context rot must be 0..3, got '%s'\n", cfv[2].c_str());
					return 1;
				}
				pc.Rot = r;
			}
			if (cfv.size() >= 4 && !cfv[3].empty())
			{
				std::string m = NLMISC::toLowerAscii(cfv[3]);
				pc.Mirror = (m == "1" || m == "m" || m == "true");
			}
			// strip .max
			std::string::size_type dot = base.rfind('.');
			if (dot != std::string::npos && NLMISC::toLowerAscii(base.substr(dot)) == ".max")
				base = base.substr(0, dot);
			pc.Basename = base;
			g_PlaceContextSpecs.push_back(pc);
		}
	}
	if (args.haveLongArg("open-editable"))
	{
		const std::vector<std::string> &oes = args.getLongArg("open-editable");
		for (size_t i = 0; i < oes.size(); ++i)
		{
			std::string::size_type colon = oes[i].find(':');
			std::string coords = colon == std::string::npos ? std::string() : oes[i].substr(0, colon);
			std::string base = colon == std::string::npos ? std::string() : oes[i].substr(colon + 1);
			std::string::size_type comma = coords.find(',');
			SOpenEditableSpec oe;
			if (colon == std::string::npos || comma == std::string::npos
			    || !NLMISC::fromString(coords.substr(0, comma), oe.Cx)
			    || !NLMISC::fromString(coords.substr(comma + 1), oe.Cy)
			    || base.empty())
			{
				fprintf(stderr, "ERROR: --open-editable expects cx,cy:basename, got '%s'\n",
				        oes[i].c_str());
				return 1;
			}
			std::string::size_type dot = base.rfind('.');
			if (dot != std::string::npos && NLMISC::toLowerAscii(base.substr(dot)) == ".max")
				base = base.substr(0, dot);
			oe.Basename = base;
			g_OpenEditableSpecs.push_back(oe);
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
		{
			if (hints[i].Rot != 0 || hints[i].Mirror)
				printf(" %d,%d,%u,%d:%s\n", hints[i].Dx, hints[i].Dy,
				       hints[i].Rot & 3, hints[i].Mirror ? 1 : 0, hints[i].Basename.c_str());
			else
				printf(" %d,%d:%s\n", hints[i].Dx, hints[i].Dy, hints[i].Basename.c_str());
		}
		return 0;
	}

	// Headless: dump zone export props
	if (args.haveLongArg("dump-zone-props"))
	{
		return dumpZoneProps(args.getLongArg("dump-zone-props")[0]);
	}

	// Season preference before bank load so the first resolveBankTextures uses it
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

	// Hidden thumbnail tools (no driver / scene required)
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
			// Absolutize once at parse: every downstream identity compare (dup-open
			// refusal, copy-over-open-file guard, bound-dialog fileDir resolution)
			// assumes registered paths are absolute - discovery-built paths are.
			input = absFilePath(pos);
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
		|| args.haveLongArg("paint-script")
		|| args.haveLongArg("lua-script");
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
		g_BoardSession = true; // board authority + neighbor hint chain
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
				printf(" %s '%s' root=%s bank=%s%s\n",
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
		std::string instancesFromCli;
		if (args.haveLongArg("instances"))
			instancesFromCli = args.getLongArg("instances")[0];
		if (!parseCliPlaces(args))
			return 1;

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
			// expose the same WorldName (reopen must open the file we just saved).
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

			const bool folderBrowserEnabled = true; // Screen C wired; Browse enabled
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
		// Multi-select: primary is first editable; keep full list for assembly
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

		// Remember successful world entry + last open layout (ecosystem self-instances).
		// Load-merge-save: a fresh SStartupCfg would reset every field this site doesn't
		// own (ZoneBrowserLarge went back to the detail list on every world entry).
		{
			ZPWS::SStartupCfg save;
			ZPWS::loadStartupCfg(save);
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
		// Placement flags are decided AFTER the synthetic-session check below - a
		// direct interactive .max open becomes an eco board session and takes
		// --place/--place-context through the same startup assembly as a workspace
		// session; only the genuinely headless legacy flows drop them.
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
	// Default cell size: 100 (historical tool default) on the legacy path; 160 on world
	// startup - both ecosystem bricks and continent zones sit on the Ryzom ligo cell.
	// Instance footprint steps round the geometry AABB up to this value - a wrong cellsize
	// leaves a gap and zero welds across self-instance seams, and breaks the template-mask
	// snap (continents at the old 100 default always fell back to AABB square footprints).
	float cellSize = 100.f;
	if ((g_StartupWorld.Kind == ZPWS::Ecosystem || g_StartupWorld.Kind == ZPWS::Continent)
	    && !g_StartupWorld.WorldName.empty())
		cellSize = 160.f;
	float snap = 1.f;
	if (args.haveLongArg("cellsize")) NLMISC::fromString(args.getLongArg("cellsize")[0], cellSize);
	if (args.haveLongArg("snap")) NLMISC::fromString(args.getLongArg("snap")[0], snap);
	// Session params must be live BEFORE initial assembly: loadNeighborContextFiles /
	// loadOnePlaceContext derive context footprint masks through g_SessionSnap - assigning
	// only after assembly (the old placement) built the initial masks at snap=1 while every
	// post-rebuild derivation used the real --snap, flipping template/AABB mask choices
	// after the first board op.
	g_SessionCellSize = cellSize;
	g_SessionSnap = snap;
	uint seed = 1;
	if (args.haveLongArg("seed")) NLMISC::fromString(args.getLongArg("seed")[0], seed);
	srand(seed);
	std::string scriptPath = args.haveLongArg("paint-script") ? args.getLongArg("paint-script")[0] : std::string();
	std::string luaScriptPath = args.haveLongArg("lua-script") ? args.getLongArg("lua-script")[0] : std::string();
	g_LuaScriptPath = luaScriptPath;
	if (args.haveLongArg("startup-lua"))
		g_StartupLuaPath = args.getLongArg("startup-lua")[0];
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
	// HUD textContext is opt-in via --font only . Empty fontPath → no 3D HUD overlay.
	// NLGUI still gets a system TTF inside runViewer / the startup host when --font is absent.
	std::string fontPath = args.haveLongArg("font") ? args.getLongArg("font")[0] : std::string();
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
		    || (scriptPath.empty() && luaScriptPath.empty()
		        && !doDumpRpo && !doDumpXRef && dumpBlobDir.empty()));

	// Interactive direct-.max opens synthesize a single-file ecosystem world rooted at
	// the file's directory so every viewer session goes through ONE code path. Siblings
	// in that directory become the world's zone list (board opens, instances, contexts,
	// the picker); the hint chain loads real neighbor files when they exist
	// (--embedded-context still forces the stale embedded-copy display).
	// Synthetic sessions NEVER stamp hint appdata (saves must not mutate arbitrary
	// pipeline files' Scene streams) and keep cellsize 100 (--cellsize 160 needed for
	// real ligo footprints). Headless flows (null-edit, dumps, scripts without a display,
	// panel-save-test) keep the minimal chain: those are codec/paint-core gates.
	bool syntheticSession = false;
	if (!startupPath && viewerMode)
	{
		std::string dir = NLMISC::CFile::getPath(input);
		if (dir.empty()) dir = ".";
		dir = ZPWS::normalizeDir(NLMISC::CPath::makePathAbsolute(dir, NLMISC::CPath::getCurrentPath(), true));
		g_StartupWorld = ZPWS::SWorldEntry();
		g_StartupWorld.Kind = ZPWS::Ecosystem;
		g_StartupWorld.GraphicsRoot = dir;
		g_StartupWorld.WorldName = ZPWS::dirBasename(dir);
		// Ligo convention keeps zones under .../<eco>/max - name the world after the
		// meaningful parent, not the literal "max" folder.
		if (NLMISC::toLowerAscii(g_StartupWorld.WorldName) == "max")
		{
			// normalizeDir strips the trailing slash, so getPath(dir) is the parent
			std::string parent = ZPWS::dirBasename(NLMISC::CFile::getPath(dir));
			if (!parent.empty())
				g_StartupWorld.WorldName = parent;
		}
		// A file at filesystem root has no dir basename - an empty WorldName would
		// split the eco gates (board bridge activates on MaxDir, zpScriptEcoGate
		// refuses on the empty name). Any non-empty label keeps them consistent.
		if (g_StartupWorld.WorldName.empty())
			g_StartupWorld.WorldName = dir;
		g_StartupWorld.MaxDir = dir;
		g_StartupWorld.BankPath = bankPath;
		g_StartupWorld.BankOk = !bankPath.empty();
		g_StartupZone.MaxPath = input;
		g_StartupZone.Basename = NLMISC::CFile::getFilenameWithoutExtension(input);
		g_BoardSession = true;
		g_LoadNeighbors = true; // hint chain (appdata → embedded names → siblings)
		g_HintStampEnabled = false;
		// Session, not the legacy --save flow: the toolbar SAVE must be the same
		// one-click save-all it is in workspace sessions (it was frozen here - the
		// InteractiveSave latch only ran in the startupPath branch).
		g_InteractiveSave = !args.haveLongArg("save");
		// CLI placements ride the same session assembly as a workspace startup
		// (--place-context and --open-editable already parse unconditionally above).
		if (!parseCliPlaces(args))
			return 1;
		if (args.haveLongArg("instances") && g_Places.empty())
		{
			std::string ierr;
			std::vector<SInstancePlace> layoutPlaces;
			if (!parseInstanceLayout(args.getLongArg("instances")[0],
			                         g_InstanceCols, g_InstanceRows, layoutPlaces, ierr))
			{
				fprintf(stderr, "ERROR: %s\n", ierr.c_str());
				return 1;
			}
			if (!layoutPlaces.empty())
			{
				fprintf(stderr, "NOTE: --instances is deprecated; use --place "
				                "(expanded %ux%u -> %u place(s))\n",
				        g_InstanceCols, g_InstanceRows, (uint)layoutPlaces.size());
				g_Places = layoutPlaces;
			}
		}
		syntheticSession = true;
		printf("direct open: synthesized session world '%s' (%s)\n",
		       g_StartupWorld.WorldName.c_str(), dir.c_str());
	}
	(void)syntheticSession;

	// Deferred placement-flag policy (see the legacy-path branch above): only the
	// headless legacy flows ignore placements now.
	if (!startupPath && !syntheticSession)
	{
		if (args.haveLongArg("instances") || args.haveLongArg("place"))
			fprintf(stderr, "WARNING: --place / --instances is ignored on the headless "
			                "legacy path (open interactively or use --startup-auto)\n");
		if (!g_PlaceContextSpecs.empty())
		{
			fprintf(stderr, "WARNING: --place-context is ignored on the headless "
			                "legacy path (open interactively or use --startup-auto)\n");
			g_PlaceContextSpecs.clear();
		}
		g_Places.clear();
		g_InstanceCols = 1;
		g_InstanceRows = 1;
		g_InstanceCount = 1;
	}

	if (nullEdit && !args.haveLongArg("out"))
	{
		fprintf(stderr, "ERROR: --null-edit refuses to save in place; give --out <output.max>\n");
		return 1;
	}
	if (nullEdit && args.haveLongArg("out")
	    && absFilePath(args.getLongArg("out")[0]) == input)
	{
		fprintf(stderr, "ERROR: --null-edit refuses to save in place (--out equals input)\n");
		return 1;
	}
	if (!savePath.empty() && absFilePath(savePath) == input)
	{
		fprintf(stderr, "ERROR: --save refuses to save in place\n");
		return 1;
	}

	// Load + assemble the painting zones (all modes; the null-edit path exercises exactly the
	// paint save path with zero ops).
	// N editable files + M frozen union-ring neighbors. Zone-id bases stay sparse.
	NL3D::registerSerial3d();
	// The first-opened file's scene is HEAP-owned and registered alongside every other
	// editable so closing ANY of them can actually free its scene. The reference alias
	// keeps the legacy single-file flows reading naturally; error paths before session
	// teardown leak at exit.
	PMAXLOAD::SLoadedMax *primaryLmHeap = new PMAXLOAD::SLoadedMax();
	PMAXLOAD::SLoadedMax &lm = *primaryLmHeap;
	if (!PMAXLOAD::loadMaxFile(input, lm))
	{
		fprintf(stderr, "ERROR: cannot load %s\n", input.c_str());
		delete primaryLmHeap;
		return 1;
	}

	// Synthetic sessions keep the legacy cellsize-100 default for plugin-era files -
	// but neighbor-hint appdata is stamped ONLY by workspace board sessions, which run
	// the ligo pitch (160). Replaying "dx,dy" hints at 100 would translate dx*100:
	// a 160 m neighbor lands 60 m INTO the primary. If the file carries hints and the
	// user gave no explicit --cellsize, adopt the stamping pitch.
	if (syntheticSession && !args.haveLongArg("cellsize") && lm.Scene)
	{
		std::vector<SNeighborHint> pitchProbe;
		if (readNeighborHintsFromScene(*lm.Scene, g_StartupZone.Basename, pitchProbe)
		    && !pitchProbe.empty())
		{
			cellSize = 160.f;
			g_SessionCellSize = 160.f;
			printf("direct open: neighbor hints present -> cellsize 160 (ligo pitch; "
			       "--cellsize overrides)\n");
		}
	}

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
	// a zone selected TWICE is one file on disk - it must never load twice (two
	// independent carrier sets would diverge and then fight over the same save path).
	// Duplicates become shared-paint INSTANCES on ecosystems (placed after footprints
	// derive below) and dedupe with a warning on continents (grid-anchored, no places).
	std::vector<std::string> dupInstanceNames;
	std::vector<std::pair<size_t, size_t> > editableRanges; // per-registered-file zone range
	for (size_t ei = 0; ei < editables.size(); ++ei)
	{
		if (ei > 0)
		{
			if (SEditableFileInfo *dup = findEditableByBasename(editables[ei].Basename))
			{
				// Basenames are the file-identity key everywhere (instances, hints,
				// saves) - a SECOND file with the same name is unsupportable, and
				// silently instancing the OTHER file would be worse than refusing.
				if (absFilePath(dup->Path) != absFilePath(editables[ei].MaxPath))
				{
					fprintf(stderr, "ERROR: two different files share the basename '%s':\n"
					        " %s\n %s\n (basenames are the session's file-identity key)\n",
					        editables[ei].Basename.c_str(), dup->Path.c_str(),
					        editables[ei].MaxPath.c_str());
					return 1;
				}
				if (g_StartupWorld.Kind == ZPWS::Ecosystem)
				{
					dupInstanceNames.push_back(editables[ei].Basename);
					printf("editable '%s' selected again - will place a shared-paint instance\n",
					       editables[ei].Basename.c_str());
				}
				else
					fprintf(stderr, "WARNING: '%s' selected twice - duplicate ignored "
					        "(continent zones are grid-anchored)\n", editables[ei].Basename.c_str());
				continue;
			}
		}
		if (g_EditableFiles.size() >= 10)
		{
			// Same cap as every other open route: per-file zone-id base is index*1000,
			// file index 10 would alias the instance id space (kInstanceZoneIdBase).
			fprintf(stderr, "ERROR: startup selection exceeds the board editable limit "
			                "(10 files; zone-id space)\n");
			return 1;
		}
		PMAXLOAD::SLoadedMax *sceneLm = NULL;
		if (ei == 0)
		{
			// Uniform ownership : the first file registers exactly like the others
			sceneLm = primaryLmHeap;
			g_ExtraEditableScenes.push_back(primaryLmHeap);
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
		// Base by FILE-LIST index (skipped duplicates must not leave id-base holes -
		// the uniform machinery maps g_EditableFiles index*1000 to the zone-id base)
		const uint base = (uint)(g_EditableFiles.size() * 1000);
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
		efi.Lm = sceneLm; // every file owns its scene pointer, index 0 included
		efi.Editable = true;
		for (size_t zi = before; zi < zones.size(); ++zi)
			efi.ZoneIds.push_back(zones[zi].ZoneId);
		g_EditableFiles.push_back(efi);
		editableRanges.push_back(std::make_pair(before, zones.size()));
		if (ei == 0)
			g_PrimaryLm = primaryLmHeap;
		if (ei > 0 || editables.size() > 1)
			printf("editable[%u] '%s' zoneIdBase=%u zones=%u\n",
			       (uint)(g_EditableFiles.size() - 1), efi.Basename.c_str(), base,
			       (uint)efi.ZoneIds.size());
	}
	// Count of zones from the first file at base 0 (before instances append)
	size_t primaryOnlyCount = 0;
	for (size_t i = 0; i < zones.size(); ++i)
		if (zones[i].ZoneId < 1000) ++primaryOnlyCount;
	const size_t instancePrimaryCount = primaryOnlyCount;

	// Always derive primary footprint (exporter-identical mask + W×H)
	if (instancePrimaryCount > 0)
		derivePrimaryFootprint(zones, 0, instancePrimaryCount, cellSize, snap);
	if (args.haveLongArg("check-ligozone") && instancePrimaryCount > 0)
		compareFootprintToLigozone(args.getLongArg("check-ligozone")[0]);
	// freeze the board frame at the first-opened file's authored footprint origin
	// and give that file the same per-file board fields as every other open file - from
	// here on it has no special role in board math (movable, closable, instance source
	// by name like any file).
	if (g_StartupWorld.Kind == ZPWS::Ecosystem && !g_EditableFiles.empty()
	    && instancePrimaryCount > 0)
	{
		g_SessionAnchorX = g_FootprintOriginX;
		g_SessionAnchorY = g_FootprintOriginY;
		g_SessionAnchorSet = true;
		g_EditableFiles[0].CellsW = g_FootprintCellsW;
		g_EditableFiles[0].CellsH = g_FootprintCellsH;
		g_EditableFiles[0].Mask = g_FootprintMask;
		// Pin legacy empty-source CLI --place entries to the CURRENT first file by name -
		// an empty source resolving lazily would silently re-source to whichever file is
		// first after a close (sources are per-name, uniform, never positional).
		for (size_t i = 0; i < g_Places.size(); ++i)
			if (g_Places[i].SourceBasename.empty())
				g_Places[i].SourceBasename = g_EditableFiles[0].Basename;
		// The runtime no-source scratchPlace alias pins the same way - resolving
		// g_EditableFiles[0] at CALL time would silently re-source a replayed script's
		// places to whichever file is first after a close.
		g_LegacyPlaceSourceName = g_EditableFiles[0].Basename;
		// Non-first MULTI-SELECT files place like reopened files: board cell derived from
		// the AUTHORED footprint origin relative to the session anchor, then the same
		// per-file placement the rebuild uses (fills CellsW/H/Mask; translate is a
		// near-no-op for grid-authored bricks). Conflicts (converted bricks authored at
		// one spot) auto-shift in the post-context re-audit below.
		for (size_t ei = 1; ei < g_EditableFiles.size() && ei < editableRanges.size(); ++ei)
		{
			SEditableFileInfo &ne = g_EditableFiles[ei];
			const size_t rb = editableRanges[ei].first, re = editableRanges[ei].second;
			if (rb >= re)
				continue;
			size_t pick = rb;
			for (size_t zi = rb; zi < re; ++zi)
				if (!zones[zi].Frozen) { pick = zi; break; }
			float ox = 0.f, oy = 0.f;
			int cw = 1, ch = 1;
			bool fromT = false;
			std::vector<bool> mask;
			std::string derr;
			deriveZoneFootprintMask(zones[pick], cellSize, snap, mask, cw, ch, ox, oy,
			                        fromT, derr);
			ne.CellX = (int)std::lround((ox - g_SessionAnchorX) / cellSize);
			ne.CellY = (int)std::lround((oy - g_SessionAnchorY) / cellSize);
			placeEcoEditableRange(zones, ne, rb, re, cellSize, snap);
			printf("editable[%u] '%s' @ cell (%d,%d) footprint %dx%d (authored origin)\n",
			       (uint)ei, ne.Basename.c_str(), ne.CellX, ne.CellY, ne.CellsW, ne.CellsH);
		}
		// duplicate startup selections become shared-paint instances, ring-placed
		// from the source block's origin (footprint fields are live from here on) -
		// the source's actual cell, not board origin: with authored-origin
		// placement a non-origin source's duplicate must hug the source, not file 0.
		for (size_t i = 0; i < dupInstanceNames.size(); ++i)
		{
			int scx = 0, scy = 0;
			if (SEditableFileInfo *src = findEditableByBasename(dupInstanceNames[i]))
			{
				scx = src->CellX;
				scy = src->CellY;
			}
			placeDupInstanceNear(dupInstanceNames[i], scx, scy);
		}
	}

	// --open-editable "cx,cy:basename" - additional EDITABLE files placed on the eco
	// board (the startup/E2E form of the board's Open editable action).
	if (!g_OpenEditableSpecs.empty())
	{
		if (g_StartupWorld.Kind != ZPWS::Ecosystem)
		{
			fprintf(stderr, "ERROR: --open-editable is ecosystem-startup only "
			                "(continents use multi-select zoneA+zoneB)\n");
			return 1;
		}
		for (size_t oi = 0; oi < g_OpenEditableSpecs.size(); ++oi)
		{
			const SOpenEditableSpec &oe = g_OpenEditableSpecs[oi];
			ZPWS::SZoneEntry ze;
			if (!resolveHintToZone(g_StartupWorld, oe.Basename, ze))
			{
				fprintf(stderr, "ERROR: --open-editable: unresolved brick '%s'\n",
				        oe.Basename.c_str());
				return 1;
			}
			if (SEditableFileInfo *dup = findEditableByBasename(ze.Basename))
			{
				// Basename is the file-identity key - refuse a DIFFERENT file with the
				// same name ; a true duplicate open = shared-paint instance .
				if (absFilePath(dup->Path) != absFilePath(ze.MaxPath))
				{
					fprintf(stderr, "ERROR: --open-editable: another file with basename "
					        "'%s' is already open (%s)\n", ze.Basename.c_str(),
					        dup->Path.c_str());
					return 1;
				}
				// Dup opens become shared-paint instances - no new file slot consumed,
				// so the 10-file cap does not apply here.
				placeDupInstanceNear(ze.Basename, oe.Cx, oe.Cy);
				continue;
			}
			if (g_EditableFiles.size() >= 10)
			{
				fprintf(stderr, "ERROR: --open-editable: board editable limit reached "
				                "(10 files; zone-id space)\n");
				return 1;
			}
			PMAXLOAD::SLoadedMax *extra = new PMAXLOAD::SLoadedMax();
			if (!PMAXLOAD::loadMaxFile(ze.MaxPath, *extra))
			{
				delete extra;
				fprintf(stderr, "ERROR: --open-editable: cannot load %s\n", ze.MaxPath.c_str());
				return 1;
			}
			g_ExtraEditableScenes.push_back(extra);
			const uint base = (uint)(g_EditableFiles.size() * 1000);
			const size_t before = zones.size();
			if (!buildPaintZones(*extra->Scene, zones, base, /*forceFrozen=*/false, ze.Basename))
				fprintf(stderr, "WARNING: --open-editable: no paint zones in %s\n", ze.MaxPath.c_str());
			SEditableFileInfo efi;
			efi.Path = ze.MaxPath;
			efi.Basename = ze.Basename;
			efi.Lm = extra;
			efi.Editable = true;
			efi.CellX = oe.Cx;
			efi.CellY = oe.Cy;
			for (size_t zi = before; zi < zones.size(); ++zi)
				efi.ZoneIds.push_back(zones[zi].ZoneId);
			g_EditableFiles.push_back(efi);
			placeEcoEditableRange(zones, g_EditableFiles.back(), before, zones.size(),
			                      cellSize, snap);
			// auto-shift when the requested cell does not fit (mirrors the board op)
			{
				std::string cerr;
				if (scratchEditableConflicts(g_EditableFiles.size() - 1, cerr))
				{
					SEditableFileInfo &ne = g_EditableFiles.back();
					const int bx = ne.CellX, by = ne.CellY;
					bool placed = false;
					for (int r = 1; r <= 12 && !placed; ++r)
					for (int sy = -r; sy <= r && !placed; ++sy)
					for (int sx = -r; sx <= r && !placed; ++sx)
					{
						if (std::max(std::abs(sx), std::abs(sy)) != r) continue;
						ne.CellX = bx + sx;
						ne.CellY = by + sy;
						std::string tmp;
						if (!scratchEditableConflicts(g_EditableFiles.size() - 1, tmp))
							placed = true;
					}
					if (!placed)
					{
						fprintf(stderr, "ERROR: --open-editable: '%s' does not fit at (%d,%d): %s\n",
						        ze.Basename.c_str(), bx, by, cerr.c_str());
						return 1;
					}
					translateZonesXY(zones, before, zones.size(),
					                 (float)(ne.CellX - bx) * cellSize,
					                 (float)(ne.CellY - by) * cellSize);
					ne.PlacedDX += (float)(ne.CellX - bx) * cellSize;
					ne.PlacedDY += (float)(ne.CellY - by) * cellSize;
					fprintf(stderr, "open-editable: '%s' does not fit at (%d,%d) - "
					        "auto-shifted to (%d,%d)\n",
					        ze.Basename.c_str(), bx, by, ne.CellX, ne.CellY);
				}
			}
			printf("open-editable: '%s' @ (%d,%d) zoneIdBase=%u footprint %dx%d\n",
			       ze.Basename.c_str(), g_EditableFiles.back().CellX, g_EditableFiles.back().CellY,
			       base, g_EditableFiles.back().CellsW, g_EditableFiles.back().CellsH);
		}
	}

	// Ecosystem self-instances: display clones at --place offsets (rot/mirror).
	// Same Node pointers → paint_core shares one pristine carrier; weld joins transformed edges.
	// Multi-edit continent open rejects instances (already gated ecosystem-only).
	if (!g_Places.empty() && instancePrimaryCount > 0)
	{
		if (g_StartupWorld.Kind == ZPWS::Continent)
		{
			fprintf(stderr, "ERROR: --place / --instances is ecosystem-only (not available on continents)\n");
			return 1;
		}
		// (The old single-brick-only guard is gone - sources resolve by name over
		// every open file, exactly like the mid-session rebuild path.)
		appendInstanceZones(zones, instancePrimaryCount, g_Places, cellSize);
		g_InstanceCount = 1 + (uint)g_Places.size();
	}

	// RO neighbors/context (hint chain).
	// Neighbors join landscape, cross-zone weld, and metaTile graph; carriers never written
	// (forceFrozen => AnyUnfrozen stays false; writeBack skips frozen-only carriers).
	// Priority: appdata hints → embedded-copy names → continent grid. Board sessions also
	// skip embedded display copies (boardSkipEmbedded) so only REAL neighbor files show.
	if (g_LoadNeighbors && !editables.empty()
	    && (g_BoardSession || g_StartupWorld.Kind == ZPWS::Continent))
	{
		normalizePlaceContextSpecBasenames(); // suffix-resolved specs must match by resolved name
		std::vector<std::string> skip;
		for (size_t i = 0; i < g_EditableFiles.size(); ++i)
			skip.push_back(g_EditableFiles[i].Basename);
		// Specs are authoritative - same rule as the session rebuild
		for (size_t i = 0; i < g_PlaceContextSpecs.size(); ++i)
			skip.push_back(g_PlaceContextSpecs[i].Basename);
		loadNeighborContextFiles(zones, cellSize, skip);
	}

	// --place-context "dx,dy:basename" (ecosystem / board) - load as RO at offset.
	// Failed specs are DROPPED (they would retry on every rebuild and show phantom cells).
	normalizePlaceContextSpecBasenames(); // idempotent; covers the no-neighbor-load path
	for (size_t pci = g_PlaceContextSpecs.size(); pci-- > 0; )
	{
		std::string perr;
		if (!loadOnePlaceContext(zones, cellSize, g_PlaceContextSpecs[pci], perr))
		{
			fprintf(stderr, "WARNING: %s - dropping the placement\n", perr.c_str());
			g_PlaceContextSpecs.erase(g_PlaceContextSpecs.begin() + (std::ptrdiff_t)pci);
		}
	}

	// the --open-editable conflict pass above ran while --place-context specs
	// still had provisional 1x1 masks (derived only inside loadOnePlaceContext). Re-audit
	// with the real masks and auto-shift again if a multi-cell context claimed the cell.
	// Index 0 included - the first-opened file is an ordinary movable cell, so
	// a context spec claiming ITS cells must shift it like any other file. REVERSE
	// order: when two FILES overlap (bricks are authored at origin, so multi-select
	// twins collide), the later-opened one yields; the first-opened file moves only
	// when nothing else resolves its cell.
	if (g_StartupWorld.Kind == ZPWS::Ecosystem)
	{
		for (size_t ei = g_EditableFiles.size(); ei-- > 0; )
		{
			std::string cerr;
			if (!scratchEditableConflicts(ei, cerr))
				continue;
			SEditableFileInfo &ne = g_EditableFiles[ei];
			const int bx = ne.CellX, by = ne.CellY;
			bool placed = false;
			for (int r = 1; r <= 12 && !placed; ++r)
			for (int sy = -r; sy <= r && !placed; ++sy)
			for (int sx = -r; sx <= r && !placed; ++sx)
			{
				if (std::max(std::abs(sx), std::abs(sy)) != r) continue;
				ne.CellX = bx + sx;
				ne.CellY = by + sy;
				std::string tmp;
				if (!scratchEditableConflicts(ei, tmp))
					placed = true;
			}
			if (!placed)
			{
				fprintf(stderr, "ERROR: open-editable '%s' conflicts at (%d,%d) after "
				        "context load: %s (no nearby fit)\n",
				        ne.Basename.c_str(), bx, by, cerr.c_str());
				return 1;
			}
			const float sdx = (float)(ne.CellX - bx) * cellSize;
			const float sdy = (float)(ne.CellY - by) * cellSize;
			std::set<uint> idSet(ne.ZoneIds.begin(), ne.ZoneIds.end());
			for (size_t zi = 0; zi < zones.size(); ++zi)
				if (idSet.count(zones[zi].ZoneId))
					translateZonesXY(zones, zi, zi + 1, sdx, sdy);
			ne.PlacedDX += sdx;
			ne.PlacedDY += sdy;
			fprintf(stderr, "startup placement: '%s' overlapped at (%d,%d) - "
			        "auto-shifted to (%d,%d)\n",
			        ne.Basename.c_str(), bx, by, ne.CellX, ne.CellY);
		}
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

	// Rotated instance/context display tiles (landscape initial state matches GetTile
	// display space). The function skips untransformed zones, so one unconditional call
	// after full assembly covers instances AND rotated context bricks.
	if (haveBank)
		applyInstanceDisplayTiles(zones, &bank);

	// Session rebuild params (used if the board mutates the working set mid-viewer)
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
	if (args.haveLongArg("preload-tiles") && args.haveLongArg("no-preload-tiles"))
	{
		fprintf(stderr, "ERROR: --preload-tiles and --no-preload-tiles are mutually exclusive\n");
		return 1;
	}
	if (args.haveLongArg("include-meshes") && args.haveLongArg("no-include-meshes"))
	{
		fprintf(stderr, "ERROR: --include-meshes and --no-include-meshes are mutually exclusive\n");
		return 1;
	}
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

	// Expose zones + core to prop helpers for headless --paint-script prop ops .
	// runViewer overwrites Active/Paint; headless keeps Core+Zones only.
	g_PaintCtx.Core = &core;
	g_PaintCtx.Zones = &zones;
	core.setPropChangedCallback(zpOnPropChanged);

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
	else if (!scriptPath.empty() || !luaScriptPath.empty())
	{
		// Headless scripted painting: paint-script first (when given), then painterscript.
		// Both run through the SAME zpExecScriptOp op layer.
		rc = 0;
		if (!scriptPath.empty())
			rc = runPaintScript(core, scriptPath);
		if (rc == 0 && !luaScriptPath.empty())
		{
			zpInstallScriptHost(NULL);
			rc = ZPSCRIPT::runFile(luaScriptPath);
		}
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
	// - kept alive through paint/viewer so node pointers stay valid.

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

		// Install paint ctx so zpSaveTo / zpSaveOverwrite see the core + scene.
		// a viewer session may have CLOSED (and freed) the first-opened file -
		// prefer whichever file survives, like the post-viewer --save block does.
		if (g_EditableFiles.empty() && !g_PrimaryLm)
		{
			fprintf(stderr, "ERROR: panel-save-test: every editable file was closed mid-session\n");
			return 1;
		}
		g_PaintCtx = SPaintCtx();
		g_PaintCtx.Active = true;
		g_PaintCtx.Core = &core;
		g_PaintCtx.Scene = g_EditableFiles.empty() ? lm.Scene
		                                           : editableScene(g_EditableFiles[0]);
		g_PaintCtx.InputPath = g_EditableFiles.empty() ? input : g_EditableFiles[0].Path;
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
			// Multi-file : editable paths must already be sandbox copies (e.g. under /tmp).
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
				// Operate on a COPY of the save-context file in outDir - never the real
				// source (the context is file 0's CURRENT path; == input unless closed)
				const std::string copySrc = g_PaintCtx.InputPath;
				std::string workCopy = outDir + baseWithExt;
				if (NLMISC::CFile::getPath(NLMISC::CPath::makePathAbsolute(workCopy, outDir, true))
				    == NLMISC::CFile::getPath(NLMISC::CPath::makePathAbsolute(copySrc, NLMISC::CPath::getCurrentPath(), true))
				    && NLMISC::CFile::getFilename(workCopy) == NLMISC::CFile::getFilename(copySrc)
				    && NLMISC::CPath::makePathAbsolute(workCopy, outDir, true)
				       == NLMISC::CPath::makePathAbsolute(copySrc, NLMISC::CPath::getCurrentPath(), true))
				{
					// Same path as the source - refuse
					fprintf(stderr, "ERROR: panel-save-test overwrite would touch the real input; give --out <dir/file> outside the source tree\n");
					g_PaintCtx = SPaintCtx();
					return 1;
				}
				if (NLMISC::CFile::fileExists(workCopy))
					NLMISC::CFile::deleteFile(workCopy);
				if (!NLMISC::CFile::copyFile(workCopy, copySrc, false))
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
		// Route through zpSaveTo so multi-file dirty policy is enforced:
		// --save is single-path and errors when more than one editable file is dirty.
		if (g_EditableFiles.empty() && !g_PrimaryLm)
		{
			// a viewer session closed (and freed) every editable - lm is gone
			fprintf(stderr, "ERROR: --save: every editable file was closed mid-session\n");
			return 1;
		}
		g_PaintCtx = SPaintCtx();
		g_PaintCtx.Active = true;
		g_PaintCtx.Core = &core;
		// the first-opened file may have been CLOSED (and freed) mid-session -
		// use whichever file survives as the save context, like the close repoint does.
		g_PaintCtx.Scene = g_EditableFiles.empty() ? lm.Scene
		                                           : editableScene(g_EditableFiles[0]);
		g_PaintCtx.InputPath = g_EditableFiles.empty() ? input : g_EditableFiles[0].Path;
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
