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

bool getScriptAppData(CSceneClass *sc, uint32 subId, std::string &out)
{
	// The script-entry key and the null-terminated string convention live in the typed
	// CAppData now (getScriptString/setScriptString — the library also carries the write half).
	CAnimatable *anim = dynamic_cast<CAnimatable *>(sc);
	if (!anim) return false;
	// existingAppData, not appData — a read must not create an empty AppData container as a
	// side effect (appData() is the authoring accessor).
	STORAGE::CAppData *ad = anim->existingAppData();
	if (!ad) return false;
	return ad->getScriptString(subId, out);
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

float getScriptAppDataFloat(CSceneClass *sc, uint32 subId, float def)
{
	std::string s;
	if (!getScriptAppData(sc, subId, s)) return def;
	float value = 0.0f;
	if (NLMISC::fromString(s, value)) return value;
	return def;
}

} /* namespace APPDATA */

/* end of file */
