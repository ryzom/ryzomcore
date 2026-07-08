/**
 * \file appdata_util.cpp
 * \brief See appdata_util.h.
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
#include "appdata_util.h"

#include <nel/misc/common.h>

#include "../pipeline_max/builtin/animatable.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/storage_object.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

namespace APPDATA {

namespace {

// MAXSCRIPT_UTILITY_CLASS_ID / superclass, per plugin_max/nel_mesh_lib/export_appdata.h — the
// key every NEL3D_APPDATA_* script entry is stored under (see pipeline_max_design.md §8).
const NLMISC::CClassId ScriptClassId(0x04d64858, 0x16d1751d);
const uint32 ScriptSuperClassId = 4128;

} /* anonymous namespace */

bool getScriptAppData(CSceneClass *sc, uint32 subId, std::string &out)
{
	CAnimatable *anim = dynamic_cast<CAnimatable *>(sc);
	if (!anim) return false;
	STORAGE::CAppData *ad = anim->appData();
	if (!ad) return false;
	STORAGE::CAppData::TMap::const_iterator it = ad->entries().find(
		STORAGE::CAppData::TKey(ScriptClassId, ScriptSuperClassId, subId));
	if (it == ad->entries().end()) return false;
	CStorageRaw *raw = it->second->value<CStorageRaw>();
	if (!raw) return false;
	// Script AppData strings are null-terminated; require the trailing NUL like every other
	// reader of this convention (skel/anim/swt/ig/shape).
	if (raw->Value.empty() || raw->Value[raw->Value.size() - 1] != '\0') return false;
	out = std::string(raw->Value.begin(), raw->Value.end() - 1);
	return true;
}

std::string getScriptAppDataStr(CSceneClass *sc, uint32 subId, const std::string &def)
{
	std::string s;
	if (!getScriptAppData(sc, subId, s)) return def;
	return s;
}

int getScriptAppDataInt(CSceneClass *sc, uint32 subId, int def)
{
	std::string s;
	if (!getScriptAppData(sc, subId, s)) return def;
	int value = 0;
	if (NLMISC::fromString(s, value)) return value;
	return def;
}

} /* namespace APPDATA */

/* end of file */
