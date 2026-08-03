/**
 * \file xref_resolve.h
 * \brief Resolve an XRefObject (ClassId partA 0x92aab38c, the corpus-observed XRef "geom" object)
 * to the referenced scene's base object — the EvalWorldState semantics the reference 3ds Max
 * exporter gets for free (Max resolves XRefs live) and every headless tool has to do explicitly.
 * The XRefObject's 0x0170 orphaned container holds the source file (0x0100 UTF-16, authored-era
 * `R:\graphics\...` Windows path) and the source node name (0x0110 UTF-16); DBPATH::resolve
 * (db_path.h) maps that to an on-disk path under the current DB checkout, the file is loaded and
 * cached, the named node found, and its own base object returned (recursively resolving nested
 * XRefs). Consolidated from pipeline_max_export_ig for the cmb tool (which previously warned and
 * skipped every XRef collision node, so ~6 of 1201 ligo brick files produced no output at all —
 * design-doc §10v "Open follow-ups"), so both tools share one XRef path.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_XREF_RESOLVE_H
#define PIPELINE_MAX_EXPORT_COMMON_XREF_RESOLVE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/class_id.h>

#include "../pipeline_max/scene_class.h"
#include "../pipeline_max/scene_class_registry.h"
#include "../pipeline_max/builtin/i_node.h"

namespace XREFRESOLVE {

/// XRefObject's Max ClassId part A. PartB varies per-scripted-plugin edit (as with `nel_ps`);
/// the corpus consistently matches on part A only.
const uint32 CLASSID_XREF_OBJECT_A = 0x92aab38c;

/// True iff \a obj is an XRefObject (matched on ClassId part A only, per corpus practice).
inline bool isXRefObject(PIPELINE::MAX::CSceneClass *obj)
{
	return obj && obj->classDesc()->classId().a() == CLASSID_XREF_OBJECT_A;
}

/// Set up XRef resolution for the current run: registry the loaded XRef sub-scenes should register
/// their scene classes into (typically the same registry the main scene uses), and the number of
/// derived-scene handles to keep cached. Call once at tool startup. May be called again with a
/// different registry pointer to reset the cache (it also clears the loaded-scene map).
void configure(PIPELINE::MAX::CSceneClassRegistry *registry);

/// Resolve \a xrefObj (must be an XRefObject; caller checked isXRefObject) to the referenced
/// scene's own base object. Returns NULL when the referenced file cannot be located, opened, or
/// the named source node cannot be found there. Warnings are emitted on stderr on any failure
/// (path resolution, missing streams, missing node) — same discipline as the ig tool's copy this
/// replaces. \a depth is the recursion guard for nested XRefs; callers pass 0.
PIPELINE::MAX::CSceneClass *resolveXRefObject(PIPELINE::MAX::CSceneClass *xrefObj, int depth);

/// Walk \a obj through OSM/WSM Derived wrappers (the typed CDerivedObject::baseObject at each
/// step) and through XRefObject resolutions (recursively), returning the concrete base object
/// at the end. Returns \a obj itself when it's already a base object; returns the XRef wrapper
/// when its source can't be resolved (caller can then classify the node as "unresolved XRef"
/// via isXRefObject on the return value).
PIPELINE::MAX::CSceneClass *baseObjectOfObj(PIPELINE::MAX::CSceneClass *obj, int depth = 0);

/// Convenience: the node's `getReference(1)` walked through baseObjectOfObj.
PIPELINE::MAX::CSceneClass *baseObjectOf(PIPELINE::MAX::BUILTIN::INode &node);

/// Drop the loaded-scene cache. Safe to call at any point; a subsequent resolveXRefObject reloads
/// on demand. Useful for tools that process multiple main files in sequence with different DB
/// roots.
void clearCache();

} /* namespace XREFRESOLVE */

#endif /* PIPELINE_MAX_EXPORT_COMMON_XREF_RESOLVE_H */

/* end of file */
