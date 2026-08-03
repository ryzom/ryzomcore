/**
 * \file max_load.h
 * \brief The one .max scene loader of the export toolchain: OLE compound file in,
 * DllDirectory/ClassDirectory3/Scene parsed against the full class registry. Previously this
 * ~25-line orchestration (and the registry list, with accidentally drifting subsets) was
 * hand-rolled in every tool main; the per-file process flows now RECEIVE a loaded scene from
 * their caller instead of parsing their own — the standalone tools load once here and call the
 * flow, and the glTF writer loads once and feeds all of its process flows the same instance.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_MAX_LOAD_H
#define PIPELINE_MAX_EXPORT_COMMON_MAX_LOAD_H

#include <nel/misc/types_nl.h>

#include <string>

namespace PIPELINE {
namespace MAX {
class CDllDirectory;
class CClassDirectory3;
class CScene;
class CSceneClassRegistry;
}
}

namespace PMAXLOAD {

struct SLoadedMax
{
	PIPELINE::MAX::CDllDirectory *Dll;
	PIPELINE::MAX::CClassDirectory3 *Cd;
	PIPELINE::MAX::CScene *Scene;
	SLoadedMax() : Dll(NULL), Cd(NULL), Scene(NULL) { }
};

// One-time registry construction — the FULL class set (builtin + update1 + epoly + biped +
// nelpatch). Chunks of unregistered classes parse as raw containers, so the superset is the
// safe default for every tool (verified byte-stable corpus-wide when the tools that carried
// partial lists moved here).
PIPELINE::MAX::CSceneClassRegistry *sceneRegistry();

// Load and parse a .max file's DllDirectory/ClassDirectory3/Scene streams (VersionUnknown).
// Returns false and leaves lm empty on failure. Caller owns the pointers (or use
// loadMaxFileCached).
bool loadMaxFile(const std::string &path, SLoadedMax &lm);

// Cached load, keyed by path (also caches failure). Used for XRef and interface files.
SLoadedMax *loadMaxFileCached(const std::string &path);

} /* namespace PMAXLOAD */

#endif /* PIPELINE_MAX_EXPORT_COMMON_MAX_LOAD_H */

/* end of file */
