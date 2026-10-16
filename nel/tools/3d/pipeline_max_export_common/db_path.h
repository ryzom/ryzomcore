/**
 * \file db_path.h
 * \brief Resolves authored (Windows, drive-letter, case-insensitive) absolute paths — the
 * R:\graphics\... / R:\database\... convention the whole corpus was authored under (XRef object
 * source files, and potentially texture/`.ps`/rbank-style references) — to on-disk paths under
 * a local checkout. Shared by pipeline_max_export_ig (XRef resolution) and _shape (XRef +
 * texture resolution); previously one copy of this logic per tool (see pipeline_max_design.md).
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_DB_PATH_H
#define PIPELINE_MAX_EXPORT_COMMON_DB_PATH_H

#include <nel/misc/types_nl.h>

#include <string>

namespace DBPATH {

/// Sets/gets the fallback database root: an authored path whose first path component (after the
/// drive letter) is literally "graphics" or "database" (case-insensitive) has that component
/// stripped and the remainder resolved case-insensitively under this root. This is the
/// convention every corpus file observed so far actually uses.
void setDefaultRoot(const std::string &root);
const std::string &defaultRoot();

/// Registers an additional root alias: an authored path whose prefix case-insensitively matches
/// windowsPrefix (either slash direction, e.g. "R:\graphics\common\sfx" or "R:/graphics") has
/// that prefix replaced with replacementRoot before case-insensitive on-disk resolution.
/// Repeatable; the longest matching registered prefix wins, and any match wins over the
/// setDefaultRoot()/"graphics"/"database" fallback. Use this for content that was authored under
/// a different drive letter or root folder name than the "R:\graphics\..." convention assumes —
/// --path-alias on the command line of the tools that call this.
void addAlias(const std::string &windowsPrefix, const std::string &replacementRoot);

/// Resolve an authored (Windows, case-insensitive) absolute path to an on-disk path: checks
/// registered aliases (longest matching prefix) first, then falls back to stripping a leading
/// "graphics"/"database" path component and resolving case-insensitively under defaultRoot().
/// Returns false if nothing resolves (no alias matches and defaultRoot() is empty or the
/// stripped-relative path doesn't exist on disk).
bool resolve(const std::string &authoredPath, std::string &out);

} /* namespace DBPATH */

#endif /* PIPELINE_MAX_EXPORT_COMMON_DB_PATH_H */

/* end of file */
