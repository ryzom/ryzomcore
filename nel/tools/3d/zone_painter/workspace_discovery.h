/**
 * \file workspace_discovery.h
 * \brief Graphics workspace fingerprint + discovery for zone_painter startup (ui M2)
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
 */
bool selectAuto(const std::vector<SWorldEntry> &worlds,
                const std::string &autoPath,
                SWorldEntry &worldOut,
                SZoneEntry &zoneOut,
                std::string &err);

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

} // namespace ZPWS

#endif // ZONE_PAINTER_WORKSPACE_DISCOVERY_H

/* end of file */
