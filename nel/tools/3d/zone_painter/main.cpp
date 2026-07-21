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
// -> cross-zone open-edge weld (the paint.cpp WELD_THRESOLD port, session-only, never
// persisted) -> CZone::build -> CZoneCornerSmoother -> Landscape.addZone. Frozen nodes
// (empty node chunk 0x0976) are boundary-reference display like the exporter's boundary
// bricks: they participate in the landscape, the weld and the metaTile graph but are never
// paint targets and their carrier blobs are never rewritten.

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

#include <algorithm>
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
// Continent neighbor loading (M3b): default on for startup/auto, off for legacy .max path
static bool g_LoadNeighbors = false;
// World/zone selected via startup (for neighbor discovery); empty on legacy path
static ZPWS::SWorldEntry g_StartupWorld;
static ZPWS::SZoneEntry g_StartupZone;
// Neighbor .max scenes kept alive for the session (texture resolution / node pointers)
static std::vector<PMAXLOAD::SLoadedMax *> g_NeighborScenes;
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
};

/**
 * Append paint zones from one Max scene.
 * zoneIdOffset: first zone id for this file (must not collide with existing zones).
 * forceFrozen: true for continent neighbor files (same semantics as 0x0976 boundary
 *   reference zones: landscape + weld + metaTile graph, never paint targets, carriers
 *   never rewritten because AnyUnfrozen stays false).
 */
static bool buildPaintZones(CScene &scene, std::vector<SPaintZone> &zones,
                            uint zoneIdOffset, bool forceFrozen)
{
	std::vector<SZoneNode> nodes;
	collectZoneNodes(scene, nodes);
	SNodeTMCache tmCache;
	bool any = false;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		CNodeImpl *node = nodes[i].Node;
		std::string name = ucstring(node->userName()).toUtf8();
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
		// Neighbor files have no 0x0976 marker; forceFrozen marks them read-only context.
		pz.Frozen = forceFrozen || nodes[i].Frozen;
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
		if (g_verbose)
			printf("zone %u '%s'%s: %u patches\n", pz.ZoneId, pz.Name.c_str(),
			       pz.Frozen ? " FROZEN" : "", (uint)pz.Patches.size());
	}
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

static int saveWholeFile(const std::string &input, const std::string &output, CScene &scene, bool verifyIdentical)
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
			if (present[i] == "Scene") out.addStream("Scene", newScene);
			else out.addStream(present[i], rawOrig[i]);
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
			if (b2 != rawOrig[i])
			{
				fprintf(stderr, "ERROR: stream %s NOT byte-identical (%u -> %u bytes)\n",
				        (present[i][0] == '\05' ? present[i].substr(1) : present[i]).c_str(),
				        (uint)rawOrig[i].size(), (uint)b2.size());
				++diffs;
			}
		}
	}
	if (verifyIdentical)
		printf("%s null-edit: %u stream diffs -> %s\n", diffs ? "FAIL" : "OK", diffs, output.c_str());
	else
		printf("OK save -> %s\n", output.c_str());
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
	SPaintCtx()
		: Active(false), Core(NULL), Paint(NULL), Scene(NULL), InteractiveSave(false)
	{
	}
};
static SPaintCtx g_PaintCtx;

// Last save status for HUD / callers (also printed to stderr on failure)
static std::string g_LastSaveStatus;

static void zpSelectMode(int mode)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	if (mode < 0) mode = 0;
	if (mode > 2) mode = 2;
	g_PaintCtx.Paint->Mode = mode;
}

static void zpSelectTileSetDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	uint count = g_PaintCtx.Core->tileSetCount();
	if (!count) return;
	int n = (int)count;
	int cur = g_PaintCtx.Paint->CurTileSet;
	cur = (cur + d) % n;
	if (cur < 0) cur += n;
	g_PaintCtx.Paint->CurTileSet = cur;
}

static void zpSelectTileSetAbs(int idx)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	uint count = g_PaintCtx.Core->tileSetCount();
	if (!count || idx < 0 || idx >= (int)count) return;
	g_PaintCtx.Paint->CurTileSet = idx;
}

static void zpToggleTileSize()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Paint) return;
	g_PaintCtx.Paint->Mode256 = !g_PaintCtx.Paint->Mode256;
}

static void zpBrushSizeDelta(int d)
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Paint) return;
	if (g_PaintCtx.Paint->Mode == CPaintMouseListener::ModeColor)
	{
		if (d > 0)
			g_PaintCtx.Paint->BrushRadius = std::min(g_PaintCtx.Paint->BrushRadius * 1.5f, 32.f);
		else if (d < 0)
			g_PaintCtx.Paint->BrushRadius = std::max(g_PaintCtx.Paint->BrushRadius / 1.5f, 2.f);
	}
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
 * Write-back + whole-file save to `target`. Single save implementation for panel modal,
 * --save, and --panel-save-test. Non-Scene OLE streams are read from InputPath (the opened
 * editable file); the Scene stream is rebuilt from the mutated Max scene.
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
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_LastSaveStatus = "write-back: " + err;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	// OLE non-Scene streams come from the original opened file (not the target).
	const std::string &srcForStreams = g_PaintCtx.InputPath.empty() ? target : g_PaintCtx.InputPath;
	int saveRc = saveWholeFile(srcForStreams, target, *g_PaintCtx.Scene, false);
	if (saveRc != 0)
	{
		g_LastSaveStatus = "save failed -> " + target;
		return false;
	}
	g_LastSaveStatus = "OK save -> " + target;
	printf("OK save (panel) -> %s\n", target.c_str());
	return true;
}

/**
 * In-place overwrite of the opened .max: write to a temp in the same directory, create a
 * one-time `<file>.max.bak` only if no .bak exists yet (never clobber an existing backup),
 * then atomically rename the temp over the original. Failure at any step leaves the original
 * untouched.
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
	const std::string &orig = g_PaintCtx.InputPath;
	if (orig.empty())
	{
		g_LastSaveStatus = "overwrite: no input path";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	if (!NLMISC::CFile::fileExists(orig))
	{
		g_LastSaveStatus = "overwrite: original missing: " + orig;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}

	std::string dir = NLMISC::CFile::getPath(orig);
	std::string base = NLMISC::CFile::getFilename(orig);
	std::string tempPath = dir + NLMISC::toString(".zone_painter_save_%d_%s.tmp",
	                                              (int)ZP_GETPID(), base.c_str());
	// Drop any leftover temp from a previous crash
	if (NLMISC::CFile::fileExists(tempPath))
		NLMISC::CFile::deleteFile(tempPath);

	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_LastSaveStatus = "write-back: " + err;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	// Write new content to temp (OLE streams from original; Scene from mutated graph)
	int saveRc = saveWholeFile(orig, tempPath, *g_PaintCtx.Scene, false);
	if (saveRc != 0 || !NLMISC::CFile::fileExists(tempPath))
	{
		g_LastSaveStatus = "overwrite: temp write failed";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		if (NLMISC::CFile::fileExists(tempPath))
			NLMISC::CFile::deleteFile(tempPath);
		return false;
	}

	// One-time backup: only if <file>.bak does not already exist
	std::string bakPath = orig + ".bak";
	if (!NLMISC::CFile::fileExists(bakPath))
	{
		if (!NLMISC::CFile::copyFile(bakPath, orig, /*failIfExists=*/true))
		{
			g_LastSaveStatus = "overwrite: could not create " + bakPath;
			fprintf(stderr, "ERROR: %s (original left untouched)\n", g_LastSaveStatus.c_str());
			NLMISC::CFile::deleteFile(tempPath);
			return false;
		}
		printf("OK backup -> %s\n", bakPath.c_str());
	}
	else
	{
		printf("backup kept (already exists): %s\n", bakPath.c_str());
	}

	// Atomic replace: rename temp over original
	if (!NLMISC::CFile::moveFile(orig, tempPath))
	{
		// Fallback: copy+delete if rename across devices fails
		if (!NLMISC::CFile::copyFile(orig, tempPath, /*failIfExists=*/false)
		    || !NLMISC::CFile::deleteFile(tempPath))
		{
			g_LastSaveStatus = "overwrite: rename/copy onto original failed";
			fprintf(stderr, "ERROR: %s (temp left at %s; original may be intact)\n",
			        g_LastSaveStatus.c_str(), tempPath.c_str());
			return false;
		}
	}
	// moveFile may leave temp gone; ensure cleanup if copy path left it
	if (NLMISC::CFile::fileExists(tempPath))
		NLMISC::CFile::deleteFile(tempPath);

	g_LastSaveStatus = "OK overwrite -> " + orig;
	printf("OK save (overwrite) -> %s\n", orig.c_str());
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
	{
		std::string base = NLMISC::CFile::getFilenameWithoutExtension(g_PaintCtx.InputPath);
		strncpy(bridge.EditableBasename, base.c_str(), sizeof(bridge.EditableBasename) - 1);
		bridge.EditableBasename[sizeof(bridge.EditableBasename) - 1] = 0;
		std::string dir = NLMISC::CFile::getPath(g_PaintCtx.InputPath);
		strncpy(bridge.InputDir, dir.c_str(), sizeof(bridge.InputDir) - 1);
		bridge.InputDir[sizeof(bridge.InputDir) - 1] = 0;
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
		// Window title: zone_painter — <editable zone> (+N neighbors when loaded)
		{
			std::string title = "zone_painter";
			if (!g_StartupZone.Basename.empty())
				title += " — " + g_StartupZone.Basename;
			else if (!g_InputPath.empty())
				title += " — " + NLMISC::CFile::getFilename(g_InputPath);
			if (!g_NeighborScenes.empty())
				title += NLMISC::toString(" (+%u neighbors)", (uint)g_NeighborScenes.size());
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
		paintBridge.selectMode = zpSelectMode;
		paintBridge.selectTileSetDelta = zpSelectTileSetDelta;
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
		ZPUI::setPaintUIBridge(&paintBridge);

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
		}
		else
		{
			// MAIN LOOP (paint.cpp: pump, camera from the mouse listener, render; first frame
			// switches refine mode off and computes the full tessellation).
			NLMISC::TTime lastFrameTime = NLMISC::CTime::getLocalTime();
			do
			{
				// Snapshot the orbit matrix so GUI mouse capture can discard nav deltas
				NLMISC::CMatrix navMatBefore = mouseListener.getViewMatrix();
				udriver->EventServer.pump();
				if (editorUI->wantsMouse())
					mouseListener.setMatrix(navMatBefore);

				// Frame dt (the plugin's zoom timing)
				NLMISC::TTime nowTime = NLMISC::CTime::getLocalTime();
				float dt = (float)(nowTime - lastFrameTime) / 1000.f;
				lastFrameTime = nowTime;

				// F10 / ToggleUI: show/hide the NLGUI shell (keys still work either way)
				if (zpKeyPushed(ZPK_ToggleUI))
					editorUI->toggleVisible();

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
					if (udriver->AsyncListener.isKeyPushed(NLMISC::KeyLBRACKET))
						paintListener.DisplaceIndex = (paintListener.DisplaceIndex + 15) % 16;
					if (udriver->AsyncListener.isKeyPushed(NLMISC::KeyRBRACKET))
						paintListener.DisplaceIndex = (paintListener.DisplaceIndex + 1) % 16;
					// Live hardness/opacity (plugin: +-0.2 on a 0..1 float; 51/255 == the same
					// steps on the tool's 0-255 scale)
					if (zpKeyPushed(ZPK_HardnessUp))
						paintListener.BrushHardness = std::min(paintListener.BrushHardness + 51u, 255u);
					if (zpKeyPushed(ZPK_HardnessDown))
						paintListener.BrushHardness = paintListener.BrushHardness >= 51u ? paintListener.BrushHardness - 51u : 0u;
					if (zpKeyPushed(ZPK_OpacityUp))
						paintListener.BrushOpacity = std::min(paintListener.BrushOpacity + 51u, 255u);
					if (zpKeyPushed(ZPK_OpacityDown))
						paintListener.BrushOpacity = paintListener.BrushOpacity >= 51u ? paintListener.BrushOpacity - 51u : 0u;
					// Brush mask cycle + mode toggle (plugin SelectColorBrush was a file dialog;
					// the standalone key cycles the shipped set: none -> mask1 -> ... -> none)
					if (zpKeyPushed(ZPK_SelectColorBrush) && !g_MaskFiles.empty())
					{
						g_MaskCycle = (g_MaskCycle + 1) % ((int)g_MaskFiles.size() + 1);
						std::string err;
						if (g_MaskCycle == 0)
							core->clearBrushMask();
						else if (!core->loadBrushMask(g_MaskFiles[g_MaskCycle - 1], err))
							fprintf(stderr, "WARNING: %s\n", err.c_str());
					}
					if (zpKeyPushed(ZPK_ToggleColorBrushMode))
						core->setBrushMaskMode(!core->brushMaskMode());
					if (zpKeyPushed(ZPK_LockBorders))
						zpToggleLockBorders();
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
					// HUD matches the Painter panel: 1-based set index, (unnamed) for empty names
					{
						std::string tsName = core->tileSetName(paintListener.CurTileSet);
						if (tsName.empty()) tsName = "(unnamed)";
						const uint tsCount = core->tileSetCount();
						const int tsOneBased = tsCount ? (paintListener.CurTileSet + 1) : 0;
						textContext.printfAt(0.01f, 0.98f, "[%s] TileSet %d/%u %s  %s  brush %u  group %u  undo %u%s",
						                     modeNames[paintListener.Mode % 3],
						                     tsOneBased, tsCount, tsName.c_str(),
						                     paintListener.Mode256 ? "256" : "128", core->brushSize(),
						                     core->tileGroup(), core->undoDepth(),
						                     core->lockBordersOn() ? "  LOCK" : "");
					}
					if (paintListener.Mode == CPaintMouseListener::ModeColor)
						textContext.printfAt(0.01f, 0.955f, "color %02x%02x%02x  radius %.1fm  hardness %u  opacity %u  mask %s",
						                     paintListener.BrushColor.R, paintListener.BrushColor.G, paintListener.BrushColor.B,
						                     paintListener.BrushRadius, paintListener.BrushHardness, paintListener.BrushOpacity,
						                     core->brushMaskMode() ? core->brushMaskName().c_str() : "off");
					else if (paintListener.Mode == CPaintMouseListener::ModeDisplace)
						textContext.printfAt(0.01f, 0.955f, "displace index %u", paintListener.DisplaceIndex);
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
			while (!udriver->AsyncListener.isKeyPushed(NLMISC::KeyESCAPE) && closeListener.WindowActive && udriver->isActive());
		}

		if (ownsEditorUI)
			editorUI->shutdown();
		ZPUI::setPaintUIBridge(NULL);
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
	                    "Legacy: zone_painter <input.max> --bank <bank> [...] behaves exactly as before.\n"
	                    "Config files (plugin keys.cfg port, NLMISC::CConfigFile syntax; one file may serve both):\n"
	                    "  keys cfg (--keys-cfg, else ./zone_painter_keys.cfg): rebinds the plugin-era actions by name,\n"
	                    "  values are NeL TKey codes (the plugin's keys.cfg Key* constant block parses verbatim).\n"
	                    "  Honored: ModeTile ModeColor ModeDisplace SizeUp SizeDown ToggleTileSize GroupUp GroupDown\n"
	                    "  Fill0 Fill1 Fill2 Fill3 HardnessUp HardnessDown OpacityUp OpacityDown SelectColorBrush\n"
	                    "  ToggleColorBrushMode LockBorders ZoomIn ZoomOut (defaults: T C D + - B G V F F6 F7 F8\n"
	                    "  Home End Insert Delete S Q L, zoom unbound). Accepted+ignored (no tool equivalent):\n"
	                    "  Select Pick ToggleColor BackgroundColor ToggleArrows Zouille AutomaticLighting GetState ResetPatch.\n"
	                    "  vars cfg (--vars-cfg, else ./zone_painter_vars.cfg): LightDirection {x,y,z}, LightDiffuse {r,g,b},\n"
	                    "  LightAmbiant {r,g,b}, LightMultiply, ZoomSpeed (the plugin LoadVarCfg set).\n"
	                    "  ToggleUI (default F10: show/hide the in-engine NLGUI panel).\n"
	                    "Fixed viewer keys: PgUp/PgDn + 0-9 tile set, [ ] displace index, Ctrl+Z/Ctrl+E undo/redo,\n"
	                    "F12 screenshot, ESC quit.");
	// Optional first positional: .max (legacy) or folder (startup seed). Absent => startup flow.
	args.addAdditionalArg("input", "Input .max scene (legacy) or graphics/seed folder (startup); omit for discovery", true, false);
	args.addArg("", "bank", "bank", "Tile bank (.smallbank/.bank); required for the legacy .max path (auto-derived in startup flow)");
	args.addArg("", "bank-recursive", "", "Add the bank directory to the texture search path recursively");
	args.addArg("", "search-path", "dir", "Extra recursive texture search path (repeatable)", false);
	args.addArg("", "out", "output.max", "Output .max for --null-edit (in-place save is refused)");
	args.addArg("", "save", "output.max", "Write-back + whole-file save after ops (in-place save is refused)");
	args.addArg("", "cellsize", "meters", "Ligo cell size for the zone-symmetry state (default 100)");
	args.addArg("", "snap", "meters", "Ligo snap for the zone-symmetry state (default 1)");
	args.addArg("", "paint-script", "file", "Scripted paint ops (headless without a display mode)");
	args.addArg("", "seed", "n", "Random seed for the paint ops (default 1; ops use a cycle counter for base tiles)");
	args.addArg("", "lock-borders", "", "Lock tiles bordering frozen zones or open edges (plugin lockBorders)");
	args.addArg("", "brush", "0-2", "Brush size for tile mouse strokes and displace painting (recursion depths 0/4/8)");
	args.addArg("", "group", "0-12", "Tile group bias (0 = none)");
	args.addArg("", "color", "rrggbb", "Viewer color brush color (default ffffff)");
	args.addArg("", "radius", "meters", "Viewer color brush radius (default 8; keys +/- range 2-32)");
	args.addArg("", "hardness", "0-255", "Viewer color brush hardness (default 128; live keys Home/End)");
	args.addArg("", "opacity", "0-255", "Viewer color brush opacity (default 255; live keys Insert/Delete)");
	args.addArg("", "brush-mask", "file.tga", "Color-brush bitmap mask (CPath-resolved; plugin loadBrush port; script op `mask <file|none>`)");
	args.addArg("", "brush-dir", "dir", "Brush mask directory for the viewer cycle key (default: <exe dir>/brushes, else ./brushes)");
	args.addArg("", "keys-cfg", "file", "Key bindings config (see the description; default zone_painter_keys.cfg in cwd when present)");
	args.addArg("", "vars-cfg", "file", "Light/zoom variables config (see the description; default zone_painter_vars.cfg in cwd when present)");
	args.addArg("", "displace-index", "0-15", "Viewer displace paint index (default 0)");
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
	args.addArg("", "startup-auto", "workspace/zone", "Skip startup UI: select workspace+zone by name and open the viewer");
	args.addArg("", "startup-screenshot", "out.tga", "Render one frame of the first startup screen and exit (M2b+)");
	args.addArg("", "startup-screen", "world",
	            "With --startup-screenshot: pre-select this world (name or graphics-root basename) and capture Screen B "
	            "(continent coordinate grid or ecosystem zone list) instead of Screen A");
	args.addArg("", "panel-save-test", "copy|overwrite",
	            "Headless: after --paint-script, invoke the same panel save path (zpSaveTo / zpSaveOverwrite). "
	            "copy writes <basename>_painted.max under --out's directory (or cwd). overwrite first copies the "
	            "input .max into that directory and operates on the copy (never the real source). Requires --paint-script.");
	args.addArg("", "neighbors", "on|off",
	            "Continent startup: load 8-ring neighbor .max files as frozen read-only context "
	            "(default on for interactive/startup-auto, off for the legacy .max path). "
	            "Also accepted as ?neighbors=off on --startup-auto.");
	if (!args.parse(argc, argv))
		return 1;

	g_verbose = args.haveLongArg("verbose");

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

		if (args.haveLongArg("startup-auto"))
		{
			// Thin wrapper: same selectAuto the buttons call (no UI required).
			// Optional query: workspace/zone?neighbors=off
			std::string autoArg = args.getLongArg("startup-auto")[0];
			std::string autoPath = autoArg;
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
					if (NLMISC::toLowerAscii(key) == "neighbors")
					{
						std::string v = NLMISC::toLowerAscii(val);
						if (v == "off" || v == "0" || v == "false" || v == "no")
							g_LoadNeighbors = false;
						else if (v == "on" || v == "1" || v == "true" || v == "yes")
							g_LoadNeighbors = true;
					}
					if (amp == std::string::npos) break;
					start = amp + 1;
				}
			}
			std::string err;
			if (!ZPUI::startupSelectWorldZone(worlds, autoPath, selection, err))
			{
				fprintf(stderr, "ERROR: %s\n", err.c_str());
				return 1;
			}
			haveSelection = true;
			printf("startup-auto: world '%s' (%s) zone '%s' neighbors=%s\n",
			       selection.World.WorldName.c_str(),
			       selection.World.Kind == ZPWS::Ecosystem ? "ecosystem" : "continent",
			       selection.Zone.Basename.c_str(),
			       g_LoadNeighbors ? "on" : "off");
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
		}

		if (!haveSelection)
		{
			fprintf(stderr, "ERROR: startup flow produced no zone selection\n");
			return 1;
		}

		// Configure exactly what the CLI flags would have: bank, search path, input
		input = selection.Zone.MaxPath;
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

		// Remember successful world entry
		{
			ZPWS::SStartupCfg save;
			save.LastGraphicsFolder = selection.World.GraphicsRoot;
			save.LastWorld = selection.World.WorldName;
			ZPWS::saveStartupCfg(save);
		}
	}
	else
	{
		// Legacy .max path: neighbors off unless explicitly enabled
		g_LoadNeighbors = false;
		if (args.haveLongArg("neighbors"))
		{
			std::string n = NLMISC::toLowerAscii(args.getLongArg("neighbors")[0]);
			if (n == "on" || n == "1" || n == "true" || n == "yes")
				g_LoadNeighbors = true; // no-op without a continent world context
		}
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
	float cellSize = 100.f;
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
	bool viewerMode = !nullEdit && !args.haveLongArg("dump-zones") && panelSaveTest.empty()
		&& (args.haveLongArg("screenshot") || (scriptPath.empty() && !doDumpRpo && !doDumpXRef && dumpBlobDir.empty()));

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
	NL3D::registerSerial3d();
	PMAXLOAD::SLoadedMax lm;
	if (!PMAXLOAD::loadMaxFile(input, lm)) { fprintf(stderr, "ERROR: cannot load %s\n", input.c_str()); return 1; }

	std::vector<SPaintZone> zones;
	// Editable file first (zone ids from 0); only its carriers are write targets.
	bool haveZones = buildPaintZones(*lm.Scene, zones, /*zoneIdOffset=*/0, /*forceFrozen=*/false);
	if (!haveZones && !nullEdit)
	{
		fprintf(stderr, "ERROR: no displayable RklPatch zone in %s\n", input.c_str());
		return 1;
	}

	// Continent neighbors as frozen read-only context (M3b). Ecosystems stay single-file.
	// Neighbors join landscape, cross-zone weld, and metaTile graph; carriers never written
	// (forceFrozen => AnyUnfrozen stays false; writeBack skips frozen-only carriers).
	if (g_LoadNeighbors && g_StartupWorld.Kind == ZPWS::Continent && !g_StartupZone.Basename.empty())
	{
		std::vector<ZPWS::SZoneEntry> neigh;
		ZPWS::listContinentNeighbors(g_StartupWorld, g_StartupZone, neigh);
		printf("neighbors: loading %u of 8-ring around '%s'\n",
		       (uint)neigh.size(), g_StartupZone.Basename.c_str());
		for (size_t ni = 0; ni < neigh.size(); ++ni)
		{
			PMAXLOAD::SLoadedMax *nlm = new PMAXLOAD::SLoadedMax();
			if (!PMAXLOAD::loadMaxFile(neigh[ni].MaxPath, *nlm))
			{
				fprintf(stderr, "WARNING: neighbor load failed: %s\n", neigh[ni].MaxPath.c_str());
				delete nlm;
				continue;
			}
			uint base = nextZoneIdBase(zones);
			// Use a sparse base so node-index gaps in a file never collide
			if (base < (uint)((ni + 1) * 1000))
				base = (uint)((ni + 1) * 1000);
			bool ok = buildPaintZones(*nlm->Scene, zones, base, /*forceFrozen=*/true);
			if (!ok)
			{
				fprintf(stderr, "WARNING: neighbor has no paint zones: %s\n", neigh[ni].MaxPath.c_str());
				delete nlm;
				continue;
			}
			g_NeighborScenes.push_back(nlm);
			printf("  neighbor '%s' zoneIdBase=%u FROZEN\n", neigh[ni].Basename.c_str(), base);
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
	}
	if ((viewerMode || !scriptPath.empty()) && !haveBank)
	{
		fprintf(stderr, "ERROR: this mode needs --bank <bank.smallbank>\n");
		return 1;
	}

	// The painting core over the assembled zones
	ZPPAINT::CPaintCore core;
	std::vector<ZPPAINT::SPaintZoneInput> inputs;
	buildPaintInputs(zones, inputs);
	{
		std::string err;
		if (!core.init(inputs, haveBank ? &bank : NULL, cellSize, snap, args.haveLongArg("lock-borders"), err))
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
			// Also remove a stale .bak so the first overwrite creates a fresh one
			std::string bakPath = workCopy + ".bak";
			// Keep existing bak if present only when re-testing twice; first run starts clean:
			// leave bak alone if user is testing "second overwrite keeps bak" — they pre-seed.
			g_PaintCtx.InputPath = workCopy;
			if (!zpSaveOverwrite())
			{
				g_PaintCtx = SPaintCtx();
				return 1;
			}
			printf("OK panel-save-test overwrite -> %s (bak %s)\n",
			       workCopy.c_str(), NLMISC::CFile::fileExists(bakPath) ? "present" : "missing");
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
		std::string err;
		if (!core.writeBack(err)) { fprintf(stderr, "ERROR: write-back: %s\n", err.c_str()); return 1; }
		// Write-back only mutates carriers with AnyUnfrozen; neighbor (forceFrozen) carriers
		// are never rewritten — only the editable file's Scene is saved.
		int saveRc = saveWholeFile(input, savePath, *lm.Scene, false);
		if (saveRc) return saveRc;
	}

	for (size_t i = 0; i < g_NeighborScenes.size(); ++i)
		delete g_NeighborScenes[i];
	g_NeighborScenes.clear();

	return rc;
}

/* end of file */
