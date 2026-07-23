/**
 * \file workspace_discovery.cpp
 * \brief Graphics workspace fingerprint + discovery for zone_painter startup
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
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

#include "workspace_discovery.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cstdlib> // realpath (POSIX)
#include <map>
#include <set>

#include <nel/misc/config_file.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

using namespace NLMISC;

namespace ZPWS {

// ---------------------------------------------------------------------------------------------

static std::string rstripSlash(std::string p)
{
	while (p.size() > 1 && (p[p.size() - 1] == '/' || p[p.size() - 1] == '\\'))
		p.resize(p.size() - 1);
	return p;
}

std::string normalizeDir(const std::string &path)
{
	if (path.empty())
		return std::string();
	std::string abs = CPath::makePathAbsolute(path, CPath::getCurrentPath(), true);
	abs = CPath::standardizePath(abs, false); // no trailing slash
	return rstripSlash(abs);
}

/** Resolve symlinks so a graphics root and its real path do not list twice (nit). */
static std::string canonicalizeDir(const std::string &path)
{
	std::string n = normalizeDir(path);
	if (n.empty())
		return n;
#if !defined(NL_OS_WINDOWS)
	char buf[PATH_MAX];
	if (realpath(n.c_str(), buf) != NULL)
		return rstripSlash(std::string(buf));
#endif
	// Windows: normalizeDir already absolute+standardized; junction resolve is optional
	return n;
}

std::string dirBasename(const std::string &path)
{
	std::string p = rstripSlash(path);
	std::string::size_type slash = p.find_last_of("/\\");
	if (slash == std::string::npos)
		return p;
	return p.substr(slash + 1);
}

bool isMaxPath(const std::string &path)
{
	std::string ext = toLowerAscii(CFile::getExtension(path));
	return ext == "max";
}

static bool dirHasMaxFiles(const std::string &dir)
{
	if (!CFile::isDirectory(dir))
		return false;
	std::vector<std::string> files;
	CPath::getPathContent(dir, false, false, true, files);
	for (size_t i = 0; i < files.size(); ++i)
		if (isMaxPath(files[i]))
			return true;
	return false;
}

static std::string firstBankInDir(const std::string &dir)
{
	if (!CFile::isDirectory(dir))
		return std::string();
	std::vector<std::string> files;
	CPath::getPathContent(dir, false, false, true, files);
	std::sort(files.begin(), files.end());
	for (size_t i = 0; i < files.size(); ++i)
	{
		std::string ext = toLowerAscii(CFile::getExtension(files[i]));
		if (ext == "bank" || ext == "smallbank")
			return files[i];
	}
	return std::string();
}

static bool sameWorld(const SWorldEntry &a, const SWorldEntry &b)
{
	// Compare canonical GraphicsRoot so symlink + realpath of the same tree dedup
	return a.Kind == b.Kind
		&& a.WorldName == b.WorldName
		&& canonicalizeDir(a.GraphicsRoot) == canonicalizeDir(b.GraphicsRoot);
}

static void appendUnique(std::vector<SWorldEntry> &out, const SWorldEntry &w)
{
	for (size_t i = 0; i < out.size(); ++i)
		if (sameWorld(out[i], w))
			return;
	out.push_back(w);
}

static void appendUniqueAll(std::vector<SWorldEntry> &out, const std::vector<SWorldEntry> &add)
{
	for (size_t i = 0; i < add.size(); ++i)
		appendUnique(out, add[i]);
}

static bool worldEntryLess(const SWorldEntry &a, const SWorldEntry &b)
{
	if (a.Kind != b.Kind)
		return a.Kind < b.Kind;
	if (a.WorldName != b.WorldName)
		return a.WorldName < b.WorldName;
	return a.GraphicsRoot < b.GraphicsRoot;
}

static int zoneGroupOrder(const std::string &g)
{
	if (g == "material") return 0;
	if (g == "transition") return 1;
	if (g == "zonematerial") return 2;
	if (g == "zonespecial") return 3;
	return 4;
}

static bool zoneEntryLess(const SZoneEntry &a, const SZoneEntry &b)
{
	int ia = zoneGroupOrder(a.Group);
	int ib = zoneGroupOrder(b.Group);
	if (ia != ib)
		return ia < ib;
	return a.Basename < b.Basename;
}

// ---------------------------------------------------------------------------------------------

void fingerprintWorkspace(const std::string &graphicsRoot, std::vector<SWorldEntry> &out)
{
	// Store the realpath so discovery from a symlink and from its target share one entry
	std::string G = canonicalizeDir(graphicsRoot);
	if (G.empty() || !CFile::isDirectory(G))
		return;

	// Ecosystem: G/landscape/ligo/<eco>/max/*.max
	const std::string ligoRoot = G + "/landscape/ligo";
	if (CFile::isDirectory(ligoRoot))
	{
		std::vector<std::string> ecos;
		CPath::getPathContent(ligoRoot, false, true, false, ecos);
		std::sort(ecos.begin(), ecos.end());
		for (size_t i = 0; i < ecos.size(); ++i)
		{
			std::string ecoDir = rstripSlash(ecos[i]);
			std::string ecoName = dirBasename(ecoDir);
			if (ecoName.empty() || ecoName[0] == '.')
				continue;
			std::string maxDir = ecoDir + "/max";
			if (!dirHasMaxFiles(maxDir))
				continue;

			SWorldEntry w;
			w.Kind = Ecosystem;
			w.GraphicsRoot = G;
			w.WorldName = ecoName;
			w.MaxDir = normalizeDir(maxDir);
			w.ThumbnailDir = normalizeDir(ecoDir + "/zonebitmaps");
			if (!CFile::isDirectory(w.ThumbnailDir))
				w.ThumbnailDir.clear();
			w.TextureSearchPath = normalizeDir(G + "/landscape/_texture_tiles/" + ecoName);
			w.BankPath = G + "/landscape/_texture_tiles/" + ecoName + "/" + ecoName + ".bank";
			if (!CFile::fileExists(w.BankPath))
			{
				// also accept.smallbank beside the conventional name
				std::string small = G + "/landscape/_texture_tiles/" + ecoName + "/" + ecoName + ".smallbank";
				if (CFile::fileExists(small))
					w.BankPath = small;
				else
				{
					// fall back to first bank under the eco texture dir
					std::string any = firstBankInDir(w.TextureSearchPath);
					if (!any.empty())
						w.BankPath = any;
					else
						w.BankPath.clear();
				}
			}
			w.BankOk = !w.BankPath.empty() && CFile::fileExists(w.BankPath);
			out.push_back(w);
		}
	}

	// Continent: G/max/zones/*.max + G/tilebank/*.bank
	const std::string zonesDir = G + "/max/zones";
	const std::string tilebankDir = G + "/tilebank";
	if (dirHasMaxFiles(zonesDir))
	{
		SWorldEntry w;
		w.Kind = Continent;
		w.GraphicsRoot = G;
		w.WorldName = dirBasename(G);
		w.MaxDir = normalizeDir(zonesDir);
		w.ThumbnailDir.clear();
		w.TextureSearchPath = normalizeDir(tilebankDir);
		w.BankPath = firstBankInDir(tilebankDir);
		w.BankOk = !w.BankPath.empty();
		// Require a bank to list the continent as a workspace (task: bank exists)
		if (w.BankOk)
			out.push_back(w);
	}
}

void scanChildrenForWorkspaces(const std::string &root, std::vector<SWorldEntry> &out)
{
	std::string R = normalizeDir(root);
	if (R.empty() || !CFile::isDirectory(R))
		return;
	std::vector<std::string> children;
	CPath::getPathContent(R, false, true, false, children);
	std::sort(children.begin(), children.end());
	for (size_t i = 0; i < children.size(); ++i)
	{
		std::string child = rstripSlash(children[i]);
		std::string name = dirBasename(child);
		if (name.empty() || name[0] == '.')
			continue; // skip .nel and other dot dirs as workspace candidates
		fingerprintWorkspace(child, out);
	}
}

static void collectFromSeed(const std::string &seed, std::vector<SWorldEntry> &out)
{
	std::string S = normalizeDir(seed);
	if (S.empty() || !CFile::isDirectory(S))
		return;

	// If the seed itself fingerprints as a workspace, collect it.
	std::vector<SWorldEntry> self;
	fingerprintWorkspace(S, self);
	appendUniqueAll(out, self);

	// If it contains a .nel subdir, treat as NeL root and scan immediate subdirs.
	if (CFile::isDirectory(S + "/.nel"))
	{
		scanChildrenForWorkspaces(S, out);
		return;
	}

	// Otherwise also scan immediate subdirs (folder that groups several workspaces).
	// Only do this when the seed itself was not already a rich workspace root? Task says:
	// "if it fingerprints as a workspace, use it; if it contains a .nel subdir, treat as
	// NeL root and scan; otherwise scan its immediate subdirs."
	// So when it fingerprints AND has no.nel, we still "use it" (done above). Scanning
	// subdirs when it already fingerprinted can still find nested layouts; keep it for the
	// "otherwise" branch only when self was empty.
	if (self.empty())
		scanChildrenForWorkspaces(S, out);
}

void discoverWorkspaces(const std::string &seedFolder,
                        const std::string &rememberedFolder,
                        std::vector<SWorldEntry> &out)
{
	out.clear();

	if (!seedFolder.empty())
	{
		collectFromSeed(seedFolder, out);
	}
	else
	{
		// Walk up from cwd
		std::string cur = normalizeDir(CPath::getCurrentPath());
		std::string prev;
		while (!cur.empty() && cur != prev)
		{
			// (a) ancestor itself fingerprints
			std::vector<SWorldEntry> at;
			fingerprintWorkspace(cur, at);
			appendUniqueAll(out, at);

			// (b) ancestor contains .nel → scan immediate subdirs and stop walking
			if (CFile::isDirectory(cur + "/.nel"))
			{
				scanChildrenForWorkspaces(cur, out);
				break;
			}

			prev = cur;
			std::string parent = CFile::getPath(cur);
			if (parent.empty() || parent == cur)
				break;
			// getPath may leave trailing slash; normalize
			cur = normalizeDir(parent);
			if (cur == prev)
				break;
		}
	}

	// Remembered last-used folder
	if (!rememberedFolder.empty())
	{
		std::vector<SWorldEntry> mem;
		collectFromSeed(rememberedFolder, mem);
		appendUniqueAll(out, mem);
	}

	// Stable sort: ecosystems first (by name), then continents (by name)
	std::sort(out.begin(), out.end(), worldEntryLess);
}

// ---------------------------------------------------------------------------------------------

std::string zoneThumbnailPath(const SWorldEntry &world, const std::string &maxBasename)
{
	if (world.ThumbnailDir.empty() || maxBasename.empty())
		return std::string();
	std::string name = maxBasename;
	const char *prefix = "zonematerial-";
	const size_t plen = 13; // strlen("zonematerial-")
	if (name.size() > plen && name.compare(0, plen, prefix) == 0)
		name = name.substr(plen);
	std::string png = world.ThumbnailDir + "/" + name + ".png";
	if (CFile::fileExists(png))
		return png;
	return std::string();
}

static std::string zoneGroupOf(const std::string &basename)
{
	if (basename.compare(0, 9, "material-") == 0)
		return "material";
	// Stock ecosystems use BOTH spellings (jungle: transition-*; corpus-wide:
	// zonetransition-*); missing the long forms dumped every real transition/special
	// brick into "other" and misordered Screen B's sections.
	if (basename.compare(0, 11, "transition-") == 0
	    || basename.compare(0, 15, "zonetransition-") == 0)
		return "transition";
	if (basename.compare(0, 13, "zonematerial-") == 0)
		return "zonematerial";
	if (basename.compare(0, 12, "zonespecial-") == 0)
		return "zonespecial";
	return "other";
}

void listZones(const SWorldEntry &world, std::vector<SZoneEntry> &out)
{
	out.clear();
	if (world.MaxDir.empty() || !CFile::isDirectory(world.MaxDir))
		return;
	std::vector<std::string> files;
	CPath::getPathContent(world.MaxDir, false, false, true, files);
	std::sort(files.begin(), files.end());
	for (size_t i = 0; i < files.size(); ++i)
	{
		if (!isMaxPath(files[i]))
			continue;
		SZoneEntry z;
		z.MaxPath = files[i];
		z.Basename = CFile::getFilenameWithoutExtension(files[i]);
		z.Group = zoneGroupOf(z.Basename);
		z.ThumbnailPath = zoneThumbnailPath(world, z.Basename);
		out.push_back(z);
	}
	// Ecosystems: group order material, transition, zonematerial, other; within group by name
	if (world.Kind == Ecosystem)
		std::sort(out.begin(), out.end(), zoneEntryLess);
}

const SWorldEntry *findWorld(const std::vector<SWorldEntry> &worlds,
                             const std::string &name,
                             bool allowDisabled)
{
	if (name.empty())
		return NULL;
	// Prefer exact WorldName match
	for (size_t i = 0; i < worlds.size(); ++i)
	{
		if (worlds[i].WorldName == name && (allowDisabled || worlds[i].BankOk))
			return &worlds[i];
	}
	// Then GraphicsRoot basename
	for (size_t i = 0; i < worlds.size(); ++i)
	{
		if (dirBasename(worlds[i].GraphicsRoot) == name && (allowDisabled || worlds[i].BankOk))
			return &worlds[i];
	}
	return NULL;
}

/** True when path is preferRoot or a descendant (normalized absolute dirs, no trailing slash). */
static bool pathUnderRoot(const std::string &path, const std::string &preferRoot)
{
	if (preferRoot.empty() || path.empty())
		return false;
	const std::string p = normalizeDir(path);
	const std::string r = normalizeDir(preferRoot);
	if (p.empty() || r.empty())
		return false;
	if (p == r)
		return true;
	// directory boundary: r is a strict prefix of p followed by a path separator
	if (p.size() > r.size() && p.compare(0, r.size(), r) == 0
	    && (p[r.size()] == '/' || p[r.size()] == '\\'))
		return true;
	return false;
}

bool selectAuto(const std::vector<SWorldEntry> &worlds,
                const std::string &autoPath,
                SWorldEntry &worldOut,
                SZoneEntry &zoneOut,
                std::string &err,
                const std::string &preferRoot)
{
	std::vector<SZoneEntry> zones;
	if (!selectAutoMulti(worlds, autoPath, worldOut, zones, err, preferRoot))
		return false;
	if (zones.empty())
	{
		err = "startup-auto: no zones resolved";
		return false;
	}
	zoneOut = zones[0];
	return true;
}

bool selectAutoMulti(const std::vector<SWorldEntry> &worlds,
                     const std::string &autoPath,
                     SWorldEntry &worldOut,
                     std::vector<SZoneEntry> &zonesOut,
                     std::string &err,
                     const std::string &preferRoot)
{
	err.clear();
	zonesOut.clear();
	std::string ap = autoPath;
	// strip quotes leftover
	if (ap.size() >= 2 && ((ap[0] == '"' && ap[ap.size() - 1] == '"') || (ap[0] == '\'' && ap[ap.size() - 1] == '\'')))
		ap = ap.substr(1, ap.size() - 2);

	std::string::size_type slash = ap.find_last_of("/\\");
	if (slash == std::string::npos || slash == 0 || slash + 1 >= ap.size())
	{
		err = "startup-auto expects \"<workspace-name>/<zone-basename>[+zone...]\", got '" + autoPath + "'";
		return false;
	}
	std::string wsName = ap.substr(0, slash);
	std::string zonePart = ap.substr(slash + 1);

	// Split zoneA+zoneB+zoneC (multi-select ); single name works too
	std::vector<std::string> zoneNames;
	{
		std::string::size_type start = 0;
		while (start <= zonePart.size())
		{
			std::string::size_type plus = zonePart.find('+', start);
			std::string one = zonePart.substr(start, plus == std::string::npos ? std::string::npos : plus - start);
			// trim whitespace
			while (!one.empty() && (one[0] == ' ' || one[0] == '\t')) one.erase(0, 1);
			while (!one.empty() && (one[one.size() - 1] == ' ' || one[one.size() - 1] == '\t')) one.erase(one.size() - 1);
			if (!one.empty())
			{
				if (isMaxPath(one))
					one = CFile::getFilenameWithoutExtension(one);
				zoneNames.push_back(one);
			}
			if (plus == std::string::npos) break;
			start = plus + 1;
		}
	}
	if (zoneNames.empty())
	{
		err = "startup-auto: empty zone list in '" + autoPath + "'";
		return false;
	}

	// Collect candidate worlds matching the workspace token
	std::vector<const SWorldEntry *> candidates;
	for (size_t i = 0; i < worlds.size(); ++i)
	{
		if (!worlds[i].BankOk)
			continue;
		if (worlds[i].WorldName == wsName || dirBasename(worlds[i].GraphicsRoot) == wsName)
			candidates.push_back(&worlds[i]);
	}
	if (candidates.empty())
	{
		err = "startup-auto: no selectable workspace matching '" + wsName + "'";
		return false;
	}

	// Score every full match; do not return the first WorldName hit (seed vs remembered).
	// Priority (high → low): under preferRoot, exact WorldName, discovery order.
	const SWorldEntry *bestWorld = NULL;
	std::vector<SZoneEntry> bestZones;
	int bestScore = -1;
	for (size_t i = 0; i < candidates.size(); ++i)
	{
		std::vector<SZoneEntry> zones;
		listZones(*candidates[i], zones);
		std::vector<SZoneEntry> matched;
		bool all = true;
		for (size_t n = 0; n < zoneNames.size(); ++n)
		{
			bool hit = false;
			for (size_t z = 0; z < zones.size(); ++z)
			{
				if (zones[z].Basename == zoneNames[n])
				{
					matched.push_back(zones[z]);
					hit = true;
					break;
				}
			}
			if (!hit) { all = false; break; }
		}
		if (!all)
			continue;
		int score = 0;
		if (pathUnderRoot(candidates[i]->GraphicsRoot, preferRoot)
		    || pathUnderRoot(candidates[i]->MaxDir, preferRoot))
			score += 1000;
		if (candidates[i]->WorldName == wsName)
			score += 100;
		// Stable tie-break: earlier discovery index (seed is collected before remembered)
		score += (int)(candidates.size() - i);
		if (score > bestScore)
		{
			bestScore = score;
			bestWorld = candidates[i];
			bestZones = matched;
		}
	}
	if (bestWorld)
	{
		worldOut = *bestWorld;
		zonesOut = bestZones;
		return true;
	}
	err = "startup-auto: zone(s) not found under workspace '" + wsName + "' (wanted";
	for (size_t n = 0; n < zoneNames.size(); ++n)
		err += (n ? "+" : " ") + zoneNames[n];
	err += ")";
	return false;
}

void listContinentNeighborUnion(const SWorldEntry &world,
                                const std::vector<SZoneEntry> &centers,
                                std::vector<SZoneEntry> &out)
{
	out.clear();
	if (world.Kind != Continent || centers.empty())
		return;
	std::set<std::string> centerNames;
	for (size_t i = 0; i < centers.size(); ++i)
		centerNames.insert(centers[i].Basename);
	std::map<std::string, SZoneEntry> unionMap;
	for (size_t i = 0; i < centers.size(); ++i)
	{
		std::vector<SZoneEntry> neigh;
		listContinentNeighbors(world, centers[i], neigh);
		for (size_t n = 0; n < neigh.size(); ++n)
		{
			if (centerNames.count(neigh[n].Basename))
				continue;
			unionMap[neigh[n].Basename] = neigh[n];
		}
	}
	for (std::map<std::string, SZoneEntry>::const_iterator it = unionMap.begin(); it != unionMap.end(); ++it)
		out.push_back(it->second);
}

// ---------------------------------------------------------------------------------------------

std::string startupCfgPath()
{
	std::string dir = CPath::getApplicationDirectory("zone_painter", true);
	if (!dir.empty() && !CFile::isDirectory(dir))
		CFile::createDirectoryTree(dir);
	if (dir.empty())
		return std::string("startup.cfg");
	if (dir[dir.size() - 1] != '/' && dir[dir.size() - 1] != '\\')
		dir += "/";
	return dir + "startup.cfg";
}

bool loadStartupCfg(SStartupCfg &cfg)
{
	cfg = SStartupCfg();
	std::string path = startupCfgPath();
	if (!CFile::fileExists(path))
		return false;
	CConfigFile cf;
	try
	{
		cf.load(path);
	}
	catch (const Exception &)
	{
		return false;
	}
	catch (...)
	{
		return false;
	}
	try
	{
		cfg.LastGraphicsFolder = cf.getVar("LastGraphicsFolder").asString();
	}
	catch (const EConfigFile &)
	{
	}
	try
	{
		cfg.LastWorld = cf.getVar("LastWorld").asString();
	}
	catch (const EConfigFile &)
	{
	}
	try
	{
		cfg.LastInstances = cf.getVar("LastInstances").asString();
	}
	catch (const EConfigFile &)
	{
	}
	try
	{
		cfg.ZoneBrowserLarge = cf.getVar("ZoneBrowserLarge").asInt() != 0;
	}
	catch (const EConfigFile &)
	{
	}
	return !cfg.LastGraphicsFolder.empty() || !cfg.LastWorld.empty() || !cfg.LastInstances.empty();
}

void saveStartupCfg(const SStartupCfg &cfg)
{
	std::string path = startupCfgPath();
	std::string dir = CFile::getPath(path);
	if (!dir.empty() && !CFile::isDirectory(dir))
		CFile::createDirectoryTree(dir);
	FILE *f = fopen(path.c_str(), "w");
	if (!f)
		return;
	fprintf(f, "// zone_painter startup config (auto-written; safe to delete)\n");
	fprintf(f, "LastGraphicsFolder = \"%s\";\n", cfg.LastGraphicsFolder.c_str());
	fprintf(f, "LastWorld = \"%s\";\n", cfg.LastWorld.c_str());
	if (!cfg.LastInstances.empty())
		fprintf(f, "LastInstances = \"%s\";\n", cfg.LastInstances.c_str());
	fprintf(f, "ZoneBrowserLarge = %d;\n", cfg.ZoneBrowserLarge ? 1 : 0);
	fclose(f);
}

// ---------------------------------------------------------------------------------------------
// Continent zone grid (ligo getZoneNameByCoord / getZoneNameFromXY convention)

bool parseContinentZoneName(const std::string &basename, int &row, int &col)
{
	row = 0;
	col = 0;
	// Strip any prefix ending in '-' (zonematerial-converted-193_ec → 193_ec).
	// Bare names (4_AC) have no dash and are matched as-is.
	std::string work = basename;
	std::string::size_type dash = work.rfind('-');
	if (dash != std::string::npos)
		work = work.substr(dash + 1);

	std::string::size_type us = work.find('_');
	if (us == std::string::npos || us == 0 || us + 3 != work.size())
		return false;
	// row digits
	for (size_t i = 0; i < us; ++i)
		if (work[i] < '0' || work[i] > '9')
			return false;
	if (!NLMISC::fromString(work.substr(0, us), row))
		return false;
	// two letters (case-insensitive → upper)
	char a = work[us + 1];
	char b = work[us + 2];
	if (a >= 'a' && a <= 'z') a = char(a - 'a' + 'A');
	if (b >= 'a' && b <= 'z') b = char(b - 'a' + 'A');
	if (a < 'A' || a > 'Z' || b < 'A' || b > 'Z')
		return false;
	col = (int)(a - 'A') * 26 + (int)(b - 'A');
	return true;
}

std::string continentZoneName(int row, int col)
{
	// getZoneNameByCoord / getLettersFromNum: col -> two letters
	if (row < 0 || col < 0 || col >= 26 * 26)
		return std::string();
	char L1 = char('A' + (col / 26));
	char L2 = char('A' + (col % 26));
	return NLMISC::toString("%d_%c%c", row, L1, L2);
}

void listContinentNeighbors(const SWorldEntry &world, const SZoneEntry &zone,
                            std::vector<SZoneEntry> &out)
{
	out.clear();
	if (world.Kind != Continent || world.MaxDir.empty())
		return;
	int row = 0, col = 0;
	if (!parseContinentZoneName(zone.Basename, row, col))
		return;

	// Index MaxDir by parsed grid coord so prefixed (zonematerial-converted-193_ec)
	// and bare (4_AC) names resolve the same way. Prefer a neighbor that shares the
	// center's prefix when two files map to one cell (rare).
	std::string centerPrefix;
	{
		std::string::size_type dash = zone.Basename.rfind('-');
		if (dash != std::string::npos)
			centerPrefix = zone.Basename.substr(0, dash + 1);
	}
	std::map<std::pair<int, int>, SZoneEntry> byCoord;
	std::vector<std::string> files;
	CPath::getPathContent(world.MaxDir, false, false, true, files);
	for (size_t i = 0; i < files.size(); ++i)
	{
		if (!isMaxPath(files[i]))
			continue;
		SZoneEntry z;
		z.MaxPath = files[i];
		z.Basename = CFile::getFilenameWithoutExtension(files[i]);
		z.Group = "other";
		z.ThumbnailPath.clear();
		int r = 0, c = 0;
		if (!parseContinentZoneName(z.Basename, r, c))
			continue;
		const std::pair<int, int> key(r, c);
		std::map<std::pair<int, int>, SZoneEntry>::iterator it = byCoord.find(key);
		if (it == byCoord.end())
			byCoord[key] = z;
		else if (!centerPrefix.empty())
		{
			// Prefer same prefix as the center zone
			const std::string &exist = it->second.Basename;
			const bool existMatch = exist.size() >= centerPrefix.size()
				&& exist.compare(0, centerPrefix.size(), centerPrefix) == 0;
			const bool newMatch = z.Basename.size() >= centerPrefix.size()
				&& z.Basename.compare(0, centerPrefix.size(), centerPrefix) == 0;
			if (newMatch && !existMatch)
				it->second = z;
		}
	}

	// 8-ring: row±1 × col±1 excluding center
	for (int dr = -1; dr <= 1; ++dr)
	{
		for (int dc = -1; dc <= 1; ++dc)
		{
			if (dr == 0 && dc == 0)
				continue;
			int nr = row + dr;
			int nc = col + dc;
			if (nr < 0 || nc < 0)
				continue;
			std::map<std::pair<int, int>, SZoneEntry>::const_iterator it =
				byCoord.find(std::make_pair(nr, nc));
			if (it == byCoord.end())
				continue;
			// Skip if the file is the center itself (same path)
			if (it->second.Basename == zone.Basename)
				continue;
			out.push_back(it->second);
		}
	}
}

uint countContinentNeighbors(const SWorldEntry &world, const SZoneEntry &zone)
{
	std::vector<SZoneEntry> n;
	listContinentNeighbors(world, zone, n);
	return (uint)n.size();
}

} // namespace ZPWS

/* end of file */
