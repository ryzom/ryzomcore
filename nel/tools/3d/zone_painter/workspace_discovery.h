/**
 * \file workspace_discovery.h
 * \brief Graphics workspace fingerprint + discovery for zone_painter startup
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 *
 * Include contract: NLMISC only (path/file/config). No patch_eval, SCENELIB, NLGUI, or
 * pipeline_max headers. Safe for editor_ui / startup_ui TUs.
 *
 * Two workspace kinds (fixed model):
 *  - Ecosystem (ligo): G/landscape/ligo/<eco>/max/*.max + bank at
 *    G/landscape/_texture_tiles/<eco>/<eco>.bank (selectable only when bank exists).
 *  - Continent (snowballs-style): G/max/zones/*.max + first G/tilebank/*.bank.
 *
 * Root detection priority is implemented by discoverWorkspaces().
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

#ifndef ZONE_PAINTER_WORKSPACE_DISCOVERY_H
#define ZONE_PAINTER_WORKSPACE_DISCOVERY_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

namespace ZPWS {

enum EWorkspaceKind
{
	Ecosystem = 0,
	Continent = 1
};

/** One selectable world (an ecosystem under a graphics root, or a continent root). */
struct SWorldEntry
{
	EWorkspaceKind Kind;
	std::string GraphicsRoot;       ///< Absolute path of G
	std::string WorldName;          ///< eco name or continent folder basename
	std::string BankPath;           ///< .bank path (empty if missing)
	std::string TextureSearchPath;  ///< recursive texture search root
	std::string MaxDir;             ///< directory of .max files
	std::string ThumbnailDir;       ///< ecosystems: zonebitmaps/; empty for continents
	bool BankOk;                    ///< false => list disabled in UI

	SWorldEntry()
		: Kind(Ecosystem), BankOk(false)
	{
	}
};

/** One .max zone/brick under a world. */
struct SZoneEntry
{
	std::string MaxPath;        ///< absolute .max path
	std::string Basename;       ///< filename without extension
	std::string ThumbnailPath;  ///< absolute png when present, else empty
	std::string Group;          ///< material / transition / zonematerial / other
};

/** Remembered last-used choice (startup.cfg). */
struct SStartupCfg
{
	std::string LastGraphicsFolder;
	std::string LastWorld;
	/** Ecosystem open layout: "1x1" (default) / "2x1" / "1x2" / "2x2" / "3x3". */
	std::string LastInstances;
	/** Zone browser display mode: large-thumbnail grid (true) vs detail-tile list. */
	bool ZoneBrowserLarge;

	SStartupCfg() : ZoneBrowserLarge(false) {}
};

// Path helpers
std::string normalizeDir(const std::string &path);
std::string dirBasename(const std::string &path);

// Fingerprint a single directory G (collects zero or more worlds into out, appends).
void fingerprintWorkspace(const std::string &graphicsRoot, std::vector<SWorldEntry> &out);

// Scan immediate subdirectories of root for workspaces (symlinks ok).
void scanChildrenForWorkspaces(const std::string &root, std::vector<SWorldEntry> &out);

/**
 * Root detection:
 *  1. seedFolder (optional positional): workspace, or .nel root / plain root of subdirs
 *  2. walk up from cwd: fingerprint ancestor; stop after scanning a .nel parent
 *  3. rememberedFolder if still valid (added if not already present)
 * Deduplicates by (GraphicsRoot, WorldName). Does NOT create any .nel directory.
 */
void discoverWorkspaces(const std::string &seedFolder,
                        const std::string &rememberedFolder,
                        std::vector<SWorldEntry> &out);

// Zone listing (sorted basename; ecosystems fill Group + ThumbnailPath)
void listZones(const SWorldEntry &world, std::vector<SZoneEntry> &out);

// Thumbnail basename rule: strip leading "zonematerial-" then look for <name>.png
std::string zoneThumbnailPath(const SWorldEntry &world, const std::string &maxBasename);

/**
 * Resolve --startup-auto "workspace-name/zone-basename".
 * workspace-name matches WorldName or the GraphicsRoot directory basename.
 * zone-basename is without .max. On failure fills err and returns false.
 *
 * Multi-select: "workspace-name/zoneA+zoneB+zoneC" selects multiple zones
 * (plus-separated). zoneOut is the first; zoneOuts receives all in order. Single-zone
 * paths still fill zoneOuts with one entry when the multi overload is used.
 *
 * preferRoot (optional): when several BankOk worlds share the same WorldName (typical:
 * CLI seed folder + LastGraphicsFolder both contribute lacustre/…), prefer the world
 * whose GraphicsRoot is preferRoot or sits under it. Without this, alphabetical
 * GraphicsRoot order can pick the remembered tree over the seed workspace that holds
 * the file the user just saved.
 */
bool selectAuto(const std::vector<SWorldEntry> &worlds,
                const std::string &autoPath,
                SWorldEntry &worldOut,
                SZoneEntry &zoneOut,
                std::string &err,
                const std::string &preferRoot = std::string());

bool selectAutoMulti(const std::vector<SWorldEntry> &worlds,
                     const std::string &autoPath,
                     SWorldEntry &worldOut,
                     std::vector<SZoneEntry> &zonesOut,
                     std::string &err,
                     const std::string &preferRoot = std::string());

/**
 * Union of 8-ring neighbors of every zone in `centers`, excluding any basename already
 * in centers. Continent-only; empty for ecosystems. Used by multi-open assembly.
 */
void listContinentNeighborUnion(const SWorldEntry &world,
                                const std::vector<SZoneEntry> &centers,
                                std::vector<SZoneEntry> &out);

// Find world by name (WorldName or GraphicsRoot basename); BankOk worlds only unless allowDisabled.
const SWorldEntry *findWorld(const std::vector<SWorldEntry> &worlds,
                             const std::string &name,
                             bool allowDisabled = false);

// startup.cfg in CPath::getApplicationDirectory("zone_painter")
std::string startupCfgPath();
bool loadStartupCfg(SStartupCfg &cfg); // corrupt/missing => false, cfg cleared
void saveStartupCfg(const SStartupCfg &cfg); // best-effort

// True when path looks like a .max file (by extension; may not exist yet)
bool isMaxPath(const std::string &path);

// ---------------------------------------------------------------------------------------------
// Continent zone grid names (ligo convention)
//
// Reference: nel/tools/3d/zone_lib/zone_utility.cpp getZoneNameByCoord / getZoneCoordByName,
// and nel/tools/3d/tga_cut/tga_cut.cpp getZoneNameFromXY:
//   basename = "<row>_<L1><L2>" where row is a decimal Y index and
//   col = (L1-'A')*26 + (L2-'A')  (two uppercase letters, A..Z).
// Example: 3_AR -> row=3, col=(0)*26+17 = 17.
//
// Prefixed forms: any prefix ending in '-' is stripped before the match, so
// "zonematerial-converted-193_ec" and "193_EC" both parse to row=193, col=EC.
// Letters are case-insensitive. Neighbor lookup indexes MaxDir by parsed coords so
// mixed bare/prefixed basenames share one board.

/** Parse a continent zone basename; returns false if not [prefix-]<row>_<AA>. */
bool parseContinentZoneName(const std::string &basename, int &row, int &col);

/** Build basename from grid coords (matches getZoneNameByCoord). */
std::string continentZoneName(int row, int col);

/**
 * List the 8-ring neighbor .max files of a continent zone that exist under world.MaxDir.
 * Does not include the center zone. Empty for ecosystems or unparseable names.
 */
void listContinentNeighbors(const SWorldEntry &world, const SZoneEntry &zone,
                            std::vector<SZoneEntry> &out);

/** Count of existing 8-ring neighbors (convenience for Screen B subtitles). */
uint countContinentNeighbors(const SWorldEntry &world, const SZoneEntry &zone);

} // namespace ZPWS

#endif // ZONE_PAINTER_WORKSPACE_DISCOVERY_H

/* end of file */
