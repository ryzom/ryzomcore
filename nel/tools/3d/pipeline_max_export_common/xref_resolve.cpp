/**
 * \file xref_resolve.cpp
 * \brief See xref_resolve.h.
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

#include "xref_resolve.h"

#include "db_path.h"

#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/storage_ole.h"
#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/derived_object.h"

#include <nel/misc/common.h>
#include <nel/misc/ucstring.h>

#include <cstdio>
#include <map>
#include <string>

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

namespace XREFRESOLVE {

// One loaded (cached) referenced .max: directories + parsed scene; failures cache as empty.
struct SLoadedMax
{
	CDllDirectory *Dll;
	CClassDirectory3 *Cd;
	CScene *Scene;
	SLoadedMax() : Dll(NULL), Cd(NULL), Scene(NULL) { }
};

// Only pointers to the loaded scenes; the SLoadedMax entries themselves live in a static map so a
// failure ("Scene = NULL") is cached too (repeated resolveXRefObject on the same broken path
// doesn't re-attempt the load).
static CSceneClassRegistry *g_registry = NULL;
static std::map<std::string, SLoadedMax> g_scenes;

void configure(CSceneClassRegistry *registry)
{
	g_registry = registry;
	// Whenever the registry changes (or is reset by a new tool invocation) the cached scenes are
	// stale (they hold parsed CSceneClass instances registered against the old registry). Clear.
	clearCache();
}

void clearCache()
{
	for (std::map<std::string, SLoadedMax>::iterator it = g_scenes.begin(); it != g_scenes.end(); ++it)
	{
		delete it->second.Scene;
		delete it->second.Cd;
		delete it->second.Dll;
	}
	g_scenes.clear();
}

static SLoadedMax *loadMaxFileCached(const std::string &path)
{
	std::map<std::string, SLoadedMax>::iterator it = g_scenes.find(path);
	if (it != g_scenes.end()) return it->second.Scene ? &it->second : NULL;
	SLoadedMax &lm = g_scenes[path]; // inserted empty; failure is cached too
	CStorageOleIn in;
	if (!in.open(path)) { fprintf(stderr, "WARNING: xref: not an OLE compound file: %s\n", path.c_str()); return NULL; }
	CDllDirectory *dll = new CDllDirectory();
	CClassDirectory3 *cd = new CClassDirectory3(dll);
	CScene *scene = new CScene(g_registry, dll, cd);
	bool ok = true;
	{ std::vector<uint8> b; if (in.readStream("DllDirectory", b)) { CStorageStream st(b); dll->serial(st); dll->parse(VersionUnknown); } else ok = false; }
	if (ok) { std::vector<uint8> b; if (in.readStream("ClassDirectory3", b)) { CStorageStream st(b); cd->serial(st); cd->parse(VersionUnknown); } else ok = false; }
	if (ok) { std::vector<uint8> b; if (in.readStream("Scene", b)) { CStorageStream st(b); scene->serial(st); scene->parse(VersionUnknown); } else ok = false; }
	if (!ok)
	{
		fprintf(stderr, "WARNING: xref: missing streams in %s\n", path.c_str());
		delete scene; delete cd; delete dll;
		return NULL;
	}
	lm.Dll = dll;
	lm.Cd = cd;
	lm.Scene = scene;
	return &lm;
}

static bool xrefChildString(CStorageContainer *cont, uint16 id, std::string &out)
{
	for (CStorageContainer::TStorageObjectConstIt it = cont->chunks().begin(); it != cont->chunks().end(); ++it)
	{
		if (it->first != id) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) return false;
		ucstring us;
		us.resize(raw->Value.size() / 2);
		if (!us.empty()) memcpy(&us[0], nlVectorData(raw->Value), us.size() * 2);
		out = us.toUtf8();
		return true;
	}
	return false;
}

CSceneClass *resolveXRefObject(CSceneClass *xrefObj, int depth)
{
	if (depth > 8)
	{
		fprintf(stderr, "WARNING: xref: recursion depth exceeded\n");
		return NULL;
	}
	if (!g_registry)
	{
		fprintf(stderr, "WARNING: xref: not configured (call XREFRESOLVE::configure at startup)\n");
		return NULL;
	}
	CStorageContainer *rec = NULL;
	const CStorageContainer::TStorageObjectContainer &orphans = xrefObj->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first != 0x0170) continue;
		rec = dynamic_cast<CStorageContainer *>(it->second);
		break;
	}
	if (!rec) { fprintf(stderr, "WARNING: xref: no 0x0170 record on XRefObject\n"); return NULL; }
	std::string file, objName;
	if (!xrefChildString(rec, 0x0100, file) || !xrefChildString(rec, 0x0110, objName))
	{
		fprintf(stderr, "WARNING: xref: incomplete 0x0170 record\n");
		return NULL;
	}
	std::string resolved;
	if (!DBPATH::resolve(file, resolved))
	{
		fprintf(stderr, "WARNING: xref: cannot resolve '%s' under db root '%s'\n",
		        file.c_str(), DBPATH::defaultRoot().c_str());
		return NULL;
	}
	SLoadedMax *lm = loadMaxFileCached(resolved);
	if (!lm) return NULL;
	CSceneClassContainer *ssc = lm->Scene->container();
	std::string wantLower = NLMISC::toLowerAscii(objName);
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		if (NLMISC::toLowerAscii(ucstring(node->userName()).toUtf8()) != wantLower) continue;
		return baseObjectOfObj(dynamic_cast<CSceneClass *>(node->getReference(1)), depth + 1);
	}
	fprintf(stderr, "WARNING: xref: node '%s' not found in %s\n", objName.c_str(), resolved.c_str());
	return NULL;
}

CSceneClass *baseObjectOfObj(CSceneClass *obj, int depth)
{
	int guard = 16;
	while (obj && guard-- > 0)
	{
		if (isXRefObject(obj))
		{
			CSceneClass *resolved = resolveXRefObject(obj, depth);
			if (!resolved) return obj; // unresolvable: keep the wrapper for the caller to classify
			obj = resolved;
			continue;
		}
		CDerivedObject *derived = dynamic_cast<CDerivedObject *>(obj);
		if (!derived) break;
		CSceneClass *base = derived->baseObject();
		if (!base) break;
		obj = base;
	}
	return obj;
}

CSceneClass *baseObjectOf(INode &node)
{
	return baseObjectOfObj(dynamic_cast<CSceneClass *>(node.getReference(1)), 0);
}

} /* namespace XREFRESOLVE */

/* end of file */
