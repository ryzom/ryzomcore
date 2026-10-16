/**
 * \file max_load.cpp
 * \brief See max_load.h.
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

#include <nel/misc/types_nl.h>
#include "max_load.h"

#include <cstdio>
#include <map>

#include "../pipeline_max/storage_ole.h"
#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"
#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/nelpatch/nelpatch.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

namespace PMAXLOAD {

CSceneClassRegistry *sceneRegistry()
{
	static CSceneClassRegistry *registry = nullptr;
	if (!registry)
	{
		registry = new CSceneClassRegistry();
		CBuiltin::registerClasses(registry);
		UPDATE1::CUpdate1::registerClasses(registry);
		EPOLY::CEPoly::registerClasses(registry);
		BIPED::CBiped::registerClasses(registry);
		NELPATCH::CNelPatch::registerClasses(registry);
	}
	return registry;
}

bool loadMaxFile(const std::string &path, SLoadedMax &lm)
{
	CStorageOleIn in;
	if (!in.open(path))
	{
		fprintf(stderr, "WARNING: not an OLE compound file: %s\n", path.c_str());
		return false;
	}
	CDllDirectory *dll = new CDllDirectory();
	CClassDirectory3 *cd = new CClassDirectory3(dll);
	CScene *scene = new CScene(sceneRegistry(), dll, cd);
	bool ok = true;
	{ std::vector<uint8> b; if (in.readStream("DllDirectory", b)) { CStorageStream st(b); dll->serial(st); dll->parse(VersionUnknown); } else ok = false; }
	if (ok) { std::vector<uint8> b; if (in.readStream("ClassDirectory3", b)) { CStorageStream st(b); cd->serial(st); cd->parse(VersionUnknown); } else ok = false; }
	if (ok) { std::vector<uint8> b; if (in.readStream("Scene", b)) { CStorageStream st(b); scene->serial(st); scene->parse(VersionUnknown); } else ok = false; }
	if (!ok)
	{
		fprintf(stderr, "WARNING: missing streams in %s\n", path.c_str());
		delete scene;
		delete cd;
		delete dll;
		return false;
	}
	lm.Dll = dll;
	lm.Cd = cd;
	lm.Scene = scene;
	return true;
}

static std::map<std::string, SLoadedMax> g_loadedScenes;

SLoadedMax *loadMaxFileCached(const std::string &path)
{
	std::map<std::string, SLoadedMax>::iterator it = g_loadedScenes.find(path);
	if (it != g_loadedScenes.end()) return it->second.Scene ? &it->second : nullptr;
	SLoadedMax &lm = g_loadedScenes[path]; // inserted empty: failure is cached too
	if (!loadMaxFile(path, lm)) return nullptr;
	return &lm;
}

} /* namespace PMAXLOAD */

/* end of file */
