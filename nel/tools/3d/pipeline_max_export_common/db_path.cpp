/**
 * \file db_path.cpp
 * \brief See db_path.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
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
#include "db_path.h"

#include <nel/misc/algo.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include <vector>

namespace DBPATH {

namespace {

std::string g_defaultRoot;

struct SAlias
{
	std::string Prefix; // normalized: forward slashes, lowercased, no trailing slash
	std::string Root;
};
std::vector<SAlias> g_aliases;

std::string normalizeSlashes(const std::string &path)
{
	std::string s = path;
	for (uint i = 0; i < s.size(); ++i)
		if (s[i] == '\\') s[i] = '/';
	return s;
}

std::string rstripSlash(const std::string &path)
{
	std::string s = path;
	while (!s.empty() && s[s.size() - 1] == '/') s.resize(s.size() - 1);
	return s;
}

// Case-insensitive resolution under root: directory components lowercased (the checkout is
// lowercase on disk except possibly some filenames), filename lowercase first then verbatim.
bool resolvePathCI(const std::string &root, const std::string &relative, std::string &out)
{
	std::vector<std::string> parts;
	NLMISC::splitString(relative, "/", parts);
	while (!parts.empty() && parts[0].empty()) parts.erase(parts.begin());
	if (parts.empty()) return false;
	std::string dir = root;
	for (uint i = 0; i + 1 < parts.size(); ++i)
	{
		if (parts[i].empty()) continue;
		dir += "/" + NLMISC::toLowerAscii(parts[i]);
	}
	const std::string &file = parts[parts.size() - 1];
	std::string lower = dir + "/" + NLMISC::toLowerAscii(file);
	if (NLMISC::CFile::fileExists(lower) || NLMISC::CFile::isDirectory(lower))
	{
		out = lower;
		return true;
	}
	std::string verbatim = dir + "/" + file;
	if (NLMISC::CFile::fileExists(verbatim) || NLMISC::CFile::isDirectory(verbatim))
	{
		out = verbatim;
		return true;
	}
	return false;
}

} /* anonymous namespace */

void setDefaultRoot(const std::string &root)
{
	g_defaultRoot = root;
}

const std::string &defaultRoot()
{
	return g_defaultRoot;
}

void addAlias(const std::string &windowsPrefix, const std::string &replacementRoot)
{
	SAlias a;
	a.Prefix = NLMISC::toLowerAscii(rstripSlash(normalizeSlashes(windowsPrefix)));
	a.Root = replacementRoot;
	g_aliases.push_back(a);
}

bool resolve(const std::string &authoredPath, std::string &out)
{
	std::string norm = normalizeSlashes(authoredPath);
	std::string normLower = NLMISC::toLowerAscii(norm);

	// Longest matching registered alias wins.
	const SAlias *best = NULL;
	for (uint i = 0; i < g_aliases.size(); ++i)
	{
		const SAlias &a = g_aliases[i];
		if (a.Prefix.empty()) continue;
		if (normLower.compare(0, a.Prefix.size(), a.Prefix) != 0) continue;
		if (norm.size() > a.Prefix.size() && norm[a.Prefix.size()] != '/') continue;
		if (!best || a.Prefix.size() > best->Prefix.size()) best = &a;
	}
	if (best)
	{
		std::string rel = norm.substr(best->Prefix.size());
		while (!rel.empty() && rel[0] == '/') rel = rel.substr(1);
		return resolvePathCI(best->Root, rel, out);
	}

	// Fallback: strip the drive letter and a leading "graphics"/"database" path component,
	// resolve the remainder under the default root.
	std::string rel = norm;
	std::string::size_type colon = rel.find(':');
	if (colon != std::string::npos) rel = rel.substr(colon + 1);
	while (!rel.empty() && rel[0] == '/') rel = rel.substr(1);
	{
		std::string::size_type slash = rel.find('/');
		if (slash != std::string::npos)
		{
			std::string first = NLMISC::toLowerAscii(rel.substr(0, slash));
			if (first == "graphics" || first == "database")
				rel = rel.substr(slash + 1);
		}
	}
	if (g_defaultRoot.empty()) return false;
	return resolvePathCI(g_defaultRoot, rel, out);
}

} /* namespace DBPATH */

/* end of file */
