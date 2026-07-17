/**
 * \file map_extender_mod.cpp
 * \brief See map_extender_mod.h.
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
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

#include "map_extender_mod.h"

#include <cstring>

#include <nel/misc/common.h>

#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/storage/map_extender_cache.h"

using namespace PIPELINE::MAX;
using namespace NLMISC;
using PIPELINE::MAX::BUILTIN::CDerivedObject;
using PIPELINE::MAX::BUILTIN::STORAGE::CMapExtenderCache;

namespace MAPEXT {

// LITERAL value, deliberately NOT `= CMapExtenderCache::ModifierClassId`: a cross-TU chain of
// dynamically-initialized statics reads 0 under MSVC when this TU initializes first (the
// static-init-order fiasco that broke the RklPatch superclass on the VS2008 build — see the
// vs2008 build notes). With the copy-init form the x87 shape build silently lost every Map
// Extender apply (isMapExtenderModifier never matched); x64 ELF init order masked it.
const CClassId CLASSID_MAP_EXTENDER(0x2ec82081, 0x045a6271);

bool isMapExtenderModifier(CSceneClass *mod)
{
	if (!mod || !mod->classDesc()) return false;
	return mod->classDesc()->classId() == CLASSID_MAP_EXTENDER
	    && mod->classDesc()->superClassId() == SCLASS_OSMODIFIER;
}

// Locate the 0x2512 LocalModData under \a modApp. Prefer a direct 0x2512 child of the 0x2500
// app; some call sites hand the OSM wrapper (with 0x2500 still nested) — fall through like
// PHYSIQUESKIN. The payload object itself may be a raw leaf or a typed container; the typed
// CMapExtenderCache decode handles both forms.
static IStorageObject *find2512(CStorageContainer *modApp)
{
	if (!modApp) return NULL;
	IStorageObject *lmd = CDerivedObject::modAppLocalModData(modApp);
	if (lmd) return lmd;
	for (CStorageContainer::TStorageObjectConstIt it = modApp->chunks().begin();
	     it != modApp->chunks().end(); ++it)
	{
		if (it->first != 0x2500) continue;
		CStorageContainer *nested = dynamic_cast<CStorageContainer *>(it->second);
		if (!nested) continue;
		lmd = CDerivedObject::modAppLocalModData(nested);
		if (lmd) return lmd;
	}
	return NULL;
}

// Thin copy from the typed library decode (BUILTIN::STORAGE::CMapExtenderCache, design-doc
// §10j-huit) into the SMapChannel evaluation record. The cache format knowledge (leaf/container
// dual form, functional chunk set, size rules) lives on the typed class now, corpus-selftested;
// this wrapper keeps the historical error strings and the face-index range check.
bool readMapExtenderCache(CStorageContainer *modApp, SMapChannel &out, std::string *err)
{
	out = SMapChannel();
	IStorageObject *lmd = find2512(modApp);
	if (!lmd)
	{
		if (err) *err = modApp ? "Map Extender mod-app missing 0x2512 cache" : "null modApp";
		return false;
	}
	CMapExtenderCache cache;
	if (!cache.decode(lmd))
	{
		if (err) *err = cache.lastError();
		return false;
	}
	if (!cache.faceCornersValid())
	{
		if (err) *err = "Map Extender face index out of range";
		return false;
	}
	out.Channel = cache.channel();
	out.UVs.resize(cache.numVerts());
	if (cache.numVerts())
		memcpy(&out.UVs[0], nlVectorData(cache.uvwWords()), (size_t)cache.numVerts() * 12);
	out.FaceUVs.resize((size_t)cache.numFaces() * 3);
	if (cache.numFaces())
		memcpy(&out.FaceUVs[0], nlVectorData(cache.faceCorners()), (size_t)cache.numFaces() * 12);
	return true;
}

bool applyMapExtender(CSceneClass *mod, CStorageContainer *modApp, uint currentFaceCount,
                      int &outChannel, std::vector<CVector> &outUVs, std::vector<uint32> &outFaceUVs,
                      std::string *err)
{
	outChannel = 1;
	outUVs.clear();
	outFaceUVs.clear();
	if (mod && !isMapExtenderModifier(mod))
	{
		if (err) *err = "not a Map Extender modifier";
		return false;
	}
	SMapChannel ch;
	if (!readMapExtenderCache(modApp, ch, err))
		return false;
	if (ch.numFaces() != currentFaceCount)
	{
		if (err)
			*err = toString("Map Extender cache face count %u != mesh face count %u",
			                ch.numFaces(), currentFaceCount);
		return false;
	}
	outChannel = ch.Channel;
	outUVs.swap(ch.UVs);
	outFaceUVs.swap(ch.FaceUVs);
	return true;
}

} /* namespace MAPEXT */

/* end of file */
