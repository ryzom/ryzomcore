/**
 * \file appdata_util.h
 * \brief Generic reader for NeL export AppData script entries — shared by every exporter that
 * needs to read a `NEL3D_APPDATA_*` sub-id off a node or object (previously duplicated verbatim
 * in pipeline_max_export_ig/main.cpp and pipeline_max_export_shape/scene_lib.cpp; consolidated
 * here for the pacs_prim/cmb tools rather than adding a third copy).
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_APPDATA_UTIL_H
#define PIPELINE_MAX_EXPORT_COMMON_APPDATA_UTIL_H

#include <nel/misc/types_nl.h>

#include <string>

#include "../pipeline_max/scene_class.h"

namespace APPDATA {

/// Read a NeL export script AppData entry (keyed MAXSCRIPT_UTILITY_CLASS_ID / superclass 4128 /
/// subId, null-terminated string value) off any scene class that derives from CAnimatable.
/// Returns false when the entry is absent (or the object carries no AppData at all).
bool getScriptAppData(PIPELINE::MAX::CSceneClass *sc, uint32 subId, std::string &out);

/// Same, with a default value when the entry is absent.
std::string getScriptAppDataStr(PIPELINE::MAX::CSceneClass *sc, uint32 subId, const std::string &def);

/// Same, parsed as an int (NeL script AppData booleans/enums are stored as decimal-string text —
/// "1"/"0" for checkboxes). Returns def when absent or unparseable.
int getScriptAppDataInt(PIPELINE::MAX::CSceneClass *sc, uint32 subId, int def);

} /* namespace APPDATA */

#endif /* PIPELINE_MAX_EXPORT_COMMON_APPDATA_UTIL_H */

/* end of file */
