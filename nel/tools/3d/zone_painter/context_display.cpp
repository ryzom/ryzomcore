/**
 * \file context_display.cpp
 * \brief See context_display.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Grok 4.5
 */
// Port map:
//   paint.cpp includeMeshes branch -> addContextMeshes + setupDriverLights + decodeSceneAmbient
//   paint_light.cpp CPaintLight::build/setup -> setupPaintLights
//   CExportNel::buildShape (viewport meshes) -> shape exporter evaluation (evalNodeMesh +
//     buildBaseMeshInterface/buildMeshInterface -> NL3D::CMesh; special shapes warn and skip)
//   CExportNel::buildLight (driver CLight) -> LMSCENE::convertLightmapLight + driver rules
//     (mode map, color * multiplier, cutoff/exponent, attenuation)

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
#include "context_display.h"

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/smart_ptr.h>
#include <nel/3d/driver.h>
#include <nel/3d/landscape.h>
#include <nel/3d/landscape_model.h>
#include <nel/3d/light.h>
#include <nel/3d/lightmap_scene.h>
#include <nel/3d/mesh.h>
#include <nel/3d/point_light_model.h>
#include <nel/3d/scene.h>
#include <nel/3d/shape_bank.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/texture_multi_file.h>
#include <nel/3d/transform_shape.h>

#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include <nel/3d/tile_bank.h>

#include "../pipeline_max/scene.h"
#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/param_block_2.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"

#include "../pipeline_max_export_common/max_load.h"
#include "../pipeline_max_export_common/appdata_util.h"
#include "../pipeline_max_export_common/db_path.h"
#include "../pipeline_max_export_common/export_ids.h"

#include "../pipeline_max_export_shape/scene_lib.h"
#include "../pipeline_max_export_shape/mesh_eval.h"
#include "../pipeline_max_export_shape/material_build.h"
#include "../pipeline_max_export_shape/mesh_build.h"
#include "../pipeline_max_export_shape/lm_scene_build.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

namespace ZPCTX {

// GeomObject superclass (the plugin's isMesh gate started from geometry objects)
static const TSClassId ZP_SCLASS_GEOMOBJECT = 0x10;

static bool isDebugMarker(const std::string &name)
{
	return name.size() >= 9 && name.compare(0, 9, "[NELLIGO]") == 0;
}

// Max Matrix3 row basis -> NeL matrix (the original convertMatrix mapping)
static NLMISC::CMatrix toNelMatrix(const MAXMATH::Matrix3M &m)
{
	NLMISC::CMatrix out;
	out.identity();
	out.setRot(NLMISC::CVector(m.m[0][0], m.m[0][1], m.m[0][2]),
	           NLMISC::CVector(m.m[1][0], m.m[1][1], m.m[1][2]),
	           NLMISC::CVector(m.m[2][0], m.m[2][1], m.m[2][2]), true);
	out.setPos(NLMISC::CVector(m.m[3][0], m.m[3][1], m.m[3][2]));
	return out;
}

// ---------------------------------------------------------------------------------------------
// Out-of-the-box texture resolution (see the header). The authored paths ride the ParamBlock2
// storage: the PBBitmap value's trailing 0x0003 container carries { BitmapInfo blob, UTF-16
// file path, UTF-16 device } (the same location the material decode's file-name read uses),
// and scripted texmaps (multi-bitmap slots, water bumps, ...) keep authored paths in filename
// STRING params. Both forms observed corpus-wide as absolute "R:\graphics\..." paths.

static bool looksLikeAuthoredPath(const std::string &s)
{
	if (s.size() < 4 || s.size() > 260) return false;
	return s.find(":\\") != std::string::npos || s.find(":/") != std::string::npos;
}

// Database root from the input path when unset (the ig/cmb convention: parent of the first
// stuff/landscape/graphics/database component walking up).
void ensureDbRootFrom(const std::string &inputPath)
{
	if (!DBPATH::defaultRoot().empty()) return;
	std::string abs = inputPath;
	std::string::size_type slash = abs.find_last_of("/\\");
	while (slash != std::string::npos)
	{
		abs.resize(slash);
		slash = abs.find_last_of("/\\");
		if (slash != std::string::npos)
		{
			std::string tail = abs.substr(slash + 1);
			if (tail == "stuff" || tail == "landscape" || tail == "graphics" || tail == "database")
			{
				DBPATH::setDefaultRoot(abs.substr(0, slash));
				break;
			}
		}
	}
}

// dds answers .tga/.png lookups (the converted sets); registered once, before any search path
// (CPath requires remaps first).
static void ensureExtensionRemaps()
{
	static bool remapped = false;
	if (!remapped)
	{
		NLMISC::CPath::remapExtension("dds", "tga", true);
		NLMISC::CPath::remapExtension("dds", "png", true);
		remapped = true;
	}
}

uint registerContextTexturePaths(PMAXLOAD::SLoadedMax &lm, const std::string &inputPath,
                                 const std::string &bankPath,
                                 uint &resolvedOut, uint &missingOut)
{
	resolvedOut = 0;
	missingOut = 0;

	// The game-facing texture set: shapes reference .tga names while the converted textures
	// next to the ecosystem bank are .dds; register the extension remap BEFORE any search
	// path so the .dds files also answer .tga lookups, and add the bank's sibling map dir
	// (~/.../ecosystems/<eco>/map, the build_gamedata-converted set) when present.
	ensureExtensionRemaps();
	if (!bankPath.empty())
	{
		std::string mapDir = NLMISC::CFile::getPath(bankPath) + "../map";
		if (NLMISC::CFile::isDirectory(mapDir))
			NLMISC::CPath::addSearchPath(mapDir, false, false);
	}

	ensureDbRootFrom(inputPath);

	// Authored path collection over every ParamBlock2 in the scene
	std::set<std::string> authored;
	CSceneClassContainer *ssc = lm.Scene->container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CParamBlock2 *pb = dynamic_cast<CParamBlock2 *>(it->second);
		if (!pb) continue;
		// (a) PBBitmap trailing 0x0003 containers (orphaned): child index 1 = UTF-16 path
		const CStorageContainer::TStorageObjectContainer &orphans = pb->orphanedChunks();
		for (CStorageContainer::TStorageObjectConstIt ot = orphans.begin(); ot != orphans.end(); ++ot)
		{
			if (ot->first != 0x0003) continue;
			CStorageContainer *c = dynamic_cast<CStorageContainer *>(ot->second);
			if (!c) continue;
			uint idx = 0;
			for (CStorageContainer::TStorageObjectConstIt jt = c->chunks().begin(); jt != c->chunks().end(); ++jt, ++idx)
			{
				if (idx != 1) continue;
				CStorageRaw *raw = dynamic_cast<CStorageRaw *>(jt->second);
				if (!raw) break;
				ucstring us;
				us.resize(raw->Value.size() / 2);
				if (!us.empty()) memcpy(&us[0], nlVectorData(raw->Value), us.size() * 2);
				std::string path = us.toUtf8();
				while (!path.empty() && path[path.size() - 1] == '\0') path.resize(path.size() - 1);
				if (looksLikeAuthoredPath(path)) authored.insert(path);
				break;
			}
		}
		// (b) filename string params (scripted texmap slots etc.)
		const std::vector<CParamBlock2::SParam> &params = pb->params();
		for (size_t p = 0; p < params.size(); ++p)
			if (looksLikeAuthoredPath(params[p].S))
				authored.insert(params[p].S);
	}

	// Resolve through DBPATH; register each resolved file's directory once
	std::set<std::string> dirs;
	for (std::set<std::string>::const_iterator at = authored.begin(); at != authored.end(); ++at)
	{
		std::string disk;
		if (DBPATH::resolve(*at, disk))
		{
			++resolvedOut;
			dirs.insert(NLMISC::CFile::getPath(disk));
		}
		else
		{
			++missingOut;
			fprintf(stderr, "WARNING: context texture path unresolved: %s\n", at->c_str());
		}
	}
	for (std::set<std::string>::const_iterator dt = dirs.begin(); dt != dirs.end(); ++dt)
		NLMISC::CPath::addSearchPath(*dt, false, false);
	return (uint)dirs.size();
}

// Collect every texture file name of a material stage set
static void collectMaterialTexNames(const NL3D::CMaterial &mat, std::set<std::string> &names)
{
	for (uint s = 0; s < NL3D::IDRV_MAT_MAXTEXTURES; ++s)
	{
		NL3D::ITexture *tex = mat.getTexture((uint8)s);
		if (!tex) continue;
		if (NL3D::CTextureFile *tf = dynamic_cast<NL3D::CTextureFile *>(tex))
		{
			if (!tf->getFileName().empty()) names.insert(tf->getFileName());
		}
		else if (NL3D::CTextureMultiFile *tm = dynamic_cast<NL3D::CTextureMultiFile *>(tex))
		{
			for (uint i = 0; i < tm->getNumFileName(); ++i)
				if (!tm->getFileName(i).empty()) names.insert(tm->getFileName(i));
		}
	}
}

void resolveContextShapeTextures(const SContextStats &stats, uint &resolvedOut, uint &missingOut)
{
	resolvedOut = 0;
	missingOut = 0;
	std::set<std::string> names;
	for (size_t i = 0; i < stats.Shapes.size(); ++i)
	{
		NL3D::CMeshBase *mb = dynamic_cast<NL3D::CMeshBase *>(stats.Shapes[i]);
		if (!mb) continue;
		for (uint m = 0; m < mb->getNbMaterial(); ++m)
			collectMaterialTexNames(mb->getMaterial(m), names);
	}
	resolveNamesWithSeasons(names, "context texture", resolvedOut, missingOut);
}

// Season preference. Empty = auto (try sp first, historical default).
static std::string s_SeasonPref;

static const char *kSeasonCodes[4] = { "sp", "su", "au", "wi" };

static bool isSeasonCode(const std::string &c)
{
	for (int i = 0; i < 4; ++i)
		if (c == kSeasonCodes[i])
			return true;
	return false;
}

bool setSeasonPreference(const std::string &code)
{
	std::string c = NLMISC::toLowerAscii(code);
	if (c.empty())
	{
		s_SeasonPref.clear();
		return true;
	}
	if (!isSeasonCode(c))
		return false;
	s_SeasonPref = c;
	return true;
}

const std::string &seasonPreference()
{
	return s_SeasonPref;
}

std::string seasonPreferenceLabel()
{
	if (s_SeasonPref.empty())
		return "auto";
	if (s_SeasonPref == "sp") return "spring";
	if (s_SeasonPref == "su") return "summer";
	if (s_SeasonPref == "au") return "autumn";
	if (s_SeasonPref == "wi") return "winter";
	return s_SeasonPref;
}

/** Build try-order of season postfixes ("_sp", ...) with preferred first. */
static void seasonTryOrder(std::string out[4], int &nOut)
{
	nOut = 0;
	// Preferred first when set
	if (!s_SeasonPref.empty())
	{
		out[nOut++] = std::string("_") + s_SeasonPref;
	}
	for (int i = 0; i < 4; ++i)
	{
		std::string p = std::string("_") + kSeasonCodes[i];
		bool already = false;
		for (int j = 0; j < nOut; ++j)
			if (out[j] == p) { already = true; break; }
		if (!already)
			out[nOut++] = p;
	}
}

// Shared season-variant resolution. When a season preference is set and that postfix exists
// on the path, remaps even if an unpostfixed (or previously remapped other-season) name already
// resolves, so live toggles re-point CPath. Otherwise: as-is first, then first postfix that
// hits (_sp historically first). Extension remaps serve .tga/.png -> .dds.
void resolveNamesWithSeasons(const std::set<std::string> &names, const char *what,
                             uint &resolvedOut, uint &missingOut)
{
	std::string order[4];
	int nOrder = 0;
	seasonTryOrder(order, nOrder);

	for (std::set<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
	{
		std::string base = NLMISC::CFile::getFilenameWithoutExtension(*it);
		std::string ext = NLMISC::CFile::getExtension(*it);
		const std::string extPart = ext.empty() ? std::string() : ("." + ext);

		// Forced preference: always try preferred postfix first and remap when found.
		if (!s_SeasonPref.empty())
		{
			std::string preferred = base + "_" + s_SeasonPref + extPart;
			if (!NLMISC::CPath::lookup(preferred, false, false).empty())
			{
				NLMISC::CPath::remapFile(*it, preferred);
				++resolvedOut;
				continue;
			}
		}

		// As-is (unpostfixed or already correct on disk)
		if (!NLMISC::CPath::lookup(*it, false, false).empty())
		{
			++resolvedOut;
			continue;
		}

		// Seasonal fallback in try-order
		bool found = false;
		for (int s = 0; s < nOrder && !found; ++s)
		{
			std::string candidate = base + order[s] + extPart;
			if (!NLMISC::CPath::lookup(candidate, false, false).empty())
			{
				NLMISC::CPath::remapFile(*it, candidate);
				++resolvedOut;
				found = true;
			}
		}
		if (!found)
		{
			++missingOut;
			// NULL what = quiet (the bank references its ecosystem's whole tile set; entries a
			// given zone never loads are expected to be absent; the load path warns for real).
			if (what)
				fprintf(stderr, "WARNING: %s not found (any season): %s\n", what, it->c_str());
		}
	}
}

void discoverAvailableSeasons(const std::string &bankPath, std::vector<std::string> &seasonsOut)
{
	seasonsOut.clear();
	if (bankPath.empty())
		return;

	// eco name from bank file (lacustre.smallbank -> lacustre)
	std::string eco = NLMISC::CFile::getFilenameWithoutExtension(bankPath);
	// Converted tiles next to the smallbank: ../tiles or sibling *_tiles under export tree
	std::string tilesDir = NLMISC::CFile::getPath(bankPath) + "../tiles";
	// Also common: <bankdir> itself or bank path with _bank suffix replaced by _tiles
	std::string bankDir = NLMISC::CFile::getPath(bankPath);
	std::string bankBase = NLMISC::CFile::getFilenameWithoutExtension(bankPath);
	// core4_data style: lacustre_bank/lacustre.smallbank -> lacustre_tiles/
	std::string exportTiles;
	{
		std::string parent = bankDir;
		// strip trailing slash
		while (!parent.empty() && (parent[parent.size() - 1] == '/' || parent[parent.size() - 1] == '\\'))
			parent.erase(parent.size() - 1);
		// parent dir name
		std::string::size_type sl = parent.find_last_of("/\\");
		std::string leaf = (sl == std::string::npos) ? parent : parent.substr(sl + 1);
		std::string grand = (sl == std::string::npos) ? std::string() : parent.substr(0, sl);
		if (leaf.size() > 5 && leaf.compare(leaf.size() - 5, 5, "_bank") == 0)
		{
			std::string tilesLeaf = leaf.substr(0, leaf.size() - 5) + "_tiles";
			exportTiles = grand.empty() ? tilesLeaf : (grand + "/" + tilesLeaf);
		}
	}

	for (int s = 0; s < 4; ++s)
	{
		const char *code = kSeasonCodes[s];
		const std::string postfix = std::string("_") + code;
		bool found = false;

		// Source season directory under the graphics workspace
		if (!DBPATH::defaultRoot().empty())
		{
			std::string dir = DBPATH::defaultRoot() + "/landscape/_texture_tiles/" + eco + postfix;
			if (NLMISC::CFile::isDirectory(dir))
				found = true;
		}

		// Converted sibling tiles/ (scan for *_{season}.dds)
		if (!found)
		{
			const char *dirs[] = { tilesDir.c_str(), exportTiles.c_str(), bankDir.c_str(), NULL };
			for (int d = 0; dirs[d] && !found; ++d)
			{
				if (!dirs[d][0] || !NLMISC::CFile::isDirectory(dirs[d]))
					continue;
				std::vector<std::string> files;
				NLMISC::CPath::getPathContent(dirs[d], false, false, true, files);
				const std::string needle = postfix + ".dds";
				const std::string needlePng = postfix + ".png";
				const std::string needleTga = postfix + ".tga";
				for (size_t f = 0; f < files.size() && !found; ++f)
				{
					std::string bn = NLMISC::toLowerAscii(NLMISC::CFile::getFilename(files[f]));
					if (bn.size() >= needle.size()
					    && (bn.compare(bn.size() - needle.size(), needle.size(), needle) == 0
					        || bn.compare(bn.size() - needlePng.size(), needlePng.size(), needlePng) == 0
					        || bn.compare(bn.size() - needleTga.size(), needleTga.size(), needleTga) == 0))
						found = true;
				}
			}
		}

		if (found)
			seasonsOut.push_back(code);
	}
}

bool cycleSeasonPreference(const std::vector<std::string> &available)
{
	if (available.size() < 2)
		return false;
	// Find current index (or -1 if auto/not in list)
	int cur = -1;
	for (size_t i = 0; i < available.size(); ++i)
	{
		if (available[i] == s_SeasonPref)
		{
			cur = (int)i;
			break;
		}
	}
	// Advance: auto or unknown -> first; last -> first
	int next = (cur < 0) ? 0 : ((cur + 1) % (int)available.size());
	if (available[next] == s_SeasonPref)
		return false;
	s_SeasonPref = available[next];
	return true;
}

void reloadLandscapeSeasonTextures(NL3D::CTileBank &bank, const std::string &bankPath,
                                   NL3D::CLandscape *landscape, NL3D::IDriver *driver,
                                   bool preload)
{
	uint resolved = 0, missing = 0;
	resolveBankTextures(bank, bankPath, resolved, missing);
	printf("season reload (%s): bank textures %u resolved, %u missing\n",
	       seasonPreferenceLabel().c_str(), resolved, missing);
	if (!landscape)
		return;
	// Drop every loaded tile so the next flush/draw re-creates CTextureFile through CPath
	// remaps. TileTextureMap RefPtrs go NULL; findTileTexture recreates on demand.
	landscape->releaseTiles(0, 65536);
	if (preload && driver)
	{
		// Flush every tileset entry (same as CPaintCore::preloadTiles)
		for (sint ts = 0; ts < bank.getTileSetCount(); ++ts)
		{
			const NL3D::CTileSet *tileSet = bank.getTileSet(ts);
			if (!tileSet) continue;
			sint tl;
			for (tl = 0; tl < tileSet->getNumTile128(); ++tl)
				landscape->flushTiles(driver, (uint16)tileSet->getTile128(tl), 1);
			for (tl = 0; tl < tileSet->getNumTile256(); ++tl)
				landscape->flushTiles(driver, (uint16)tileSet->getTile256(tl), 1);
			for (tl = 0; tl < NL3D::CTileSet::count; ++tl)
				landscape->flushTiles(driver, (uint16)tileSet->getTransition(tl)->getTile(), 1);
		}
	}
}

void resolveBankTextures(NL3D::CTileBank &bank, const std::string &bankPath,
                         uint &resolvedOut, uint &missingOut)
{
	resolvedOut = 0;
	missingOut = 0;
	ensureExtensionRemaps();
	// The build-converted sets sit next to the smallbank: tiles/ (seasonal tile + alpha-noise
	// .dds) and diplace/ (displacement maps).
	if (!bankPath.empty())
	{
		std::string tilesDir = NLMISC::CFile::getPath(bankPath) + "../tiles";
		if (NLMISC::CFile::isDirectory(tilesDir))
			NLMISC::CPath::addSearchPath(tilesDir, false, false);
		std::string displaceDir = NLMISC::CFile::getPath(bankPath) + "../diplace";
		if (NLMISC::CFile::isDirectory(displaceDir))
			NLMISC::CPath::addSearchPath(displaceDir, false, false);
		// The converted sets are incomplete (e.g. the alphanoise c/d transition families exist
		// only as sources): fall back to the workspace source tree
		// <dbroot>/landscape/_texture_tiles/<eco>[_<season>], recursive (transitions/ subdirs),
		// eco from the bank file name. Missing dirs skip silently (foreign banks).
		if (!DBPATH::defaultRoot().empty())
		{
			static const char *seasonDirs[5] = { "", "_sp", "_su", "_au", "_wi" };
			std::string eco = NLMISC::CFile::getFilenameWithoutExtension(bankPath);
			std::string base = DBPATH::defaultRoot() + "/landscape/_texture_tiles/" + eco;
			for (int s = 0; s < 5; ++s)
			{
				std::string dir = base + seasonDirs[s];
				if (NLMISC::CFile::isDirectory(dir))
					NLMISC::CPath::addSearchPath(dir, true, false);
			}
		}
	}
	// Every texture name the bank references, as the BASENAME the loader will request
	// (makeAllPathRelative already ran; stored names may still carry authored subdirs).
	std::set<std::string> names;
	for (sint t = 0; t < bank.getTileCount(); ++t)
	{
		const NL3D::CTile *tile = bank.getTile(t);
		if (!tile) continue;
		for (int b = 0; b < NL3D::CTile::bitmapCount; ++b)
		{
			std::string name = tile->getRelativeFileName((NL3D::CTile::TBitmap)b);
			if (name.empty()) continue;
			for (size_t k = 0; k < name.size(); ++k)
				if (name[k] == '\\') name[k] = '/';
			names.insert(NLMISC::CFile::getFilename(name));
		}
	}
	for (uint d = 0; d < bank.getDisplacementMapCount(); ++d)
	{
		const char *name = bank.getDisplacementMap(d);
		if (name && *name) names.insert(NLMISC::CFile::getFilename(name));
	}
	resolveNamesWithSeasons(names, NULL, resolvedOut, missingOut);
}

void addContextMeshes(PMAXLOAD::SLoadedMax &lm, NL3D::CScene *scene, NL3D::CShapeBank *shapeBank,
                      NL3D::CLandscapeModel *land, SContextStats &stats)
{
	SCENELIB::SNodeTMCache tmCache;
	std::set<std::string> usedNames;
	CSceneClassContainer *ssc = lm.Scene->container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		std::string name = SCENELIB::nodeName(*node);
		if (isDebugMarker(name)) continue;
		// XRef-RESOLVING object walk (SCENELIB::baseObjectOf, the same one the mesh eval uses):
		// the buildings of village/town bricks are XRefObject nodes referencing construction
		// .max files, and the XRefObject wrapper's registered superclass is 0x60; a wrapper-
		// level superclass test drops every one of them silently. Resolve first, then gate.
		CSceneClass *obj = SCENELIB::baseObjectOf(*node, NULL, NULL);
		if (!obj) continue;
		if (obj->classDesc()->classId().a() == 0x92aab38c)
		{
			// Unresolvable XRef (missing referenced file); the resolver already warned.
			++stats.Skipped;
			continue;
		}
		// The plugin's viewport rule: zones stay in the landscape; every other buildable mesh
		// displays. GeomObject superclass gates out lights/cameras/helpers/splines silently.
		if (obj->classDesc()->superClassId() != ZP_SCLASS_GEOMOBJECT) continue;
		if (dynamic_cast<NELPATCH::CRklPatchObject *>(obj)) continue;

		// Property-respecting display filters: the flags that mark meta-geometry never meant
		// to render: collision meshes, accelerator cluster/portal volumes, PACS primitives and
		// light/camera targets. The node HIDDEN flag is deliberately IGNORED, like the
		// plugin's buildShape walk: village/town bricks are saved with the XRef'd buildings
		// hidden (terrain-work viewport state), and the painting scene wants them as context;
		// measured corpus-side, hidden catches ONLY those (all meta-geometry carries the
		// appdata/class marks). DONOTEXPORT likewise stays visible (export exclusion, not
		// viewport invisibility).
		{
			NLMISC::CClassId ocid = obj->classDesc()->classId();
			if (ocid == PMAX_EXPORT_IDS::CLASSID_PACS_BOX || ocid == PMAX_EXPORT_IDS::CLASSID_PACS_CYL
				|| ocid == SCENELIB::CLASSID_TARGET) { ++stats.Filtered; ++stats.FilteredClass; continue; }
			// Accel is a BITFIELD: accelerator type in bits 0-1, independent flags above
			// (FATHER_VISIBLE=4, VISIBLE_FROM_FATHER=8, CLUSTERIZED=32, ...). The exporter's
			// gate is (accel & 3) == 0 → ordinary renderable mesh (export_scene.cpp); a
			// string-equality filter ("0"/"32" only) silently dropped renderable meshes
			// carrying extra flags (36, 40, 4, 96...). Default 32 = clusterized mesh.
			int accel = APPDATA::getScriptAppDataInt(node, NEL3D_APPDATA_ACCEL, NEL3D_APPDATA_ACCEL_DEFAULT);
			if ((accel & 3) != 0) { ++stats.Filtered; ++stats.FilteredAccel; continue; }
			if (APPDATA::getScriptAppDataStr(node, NEL3D_APPDATA_COLLISION, "") == "1") { ++stats.Filtered; ++stats.FilteredCollision; continue; }
			if (APPDATA::getScriptAppDataStr(node, NEL3D_APPDATA_COLLISION_EXTERIOR, "") == "1") { ++stats.Filtered; ++stats.FilteredCollision; continue; }
		}

		// Plain-mesh build through the shared shape evaluation (the clod reuse route).
		MAXMATH::Matrix3M localTM = MESHBUILD::getLocalMatrix(*node, tmCache);
		MATBUILD::SMaxMeshBaseBuild maxBaseBuild;
		NL3D::CMeshBase::CMeshBaseBuild buildBaseMesh;
		MESHBUILD::buildBaseMeshInterface(buildBaseMesh, maxBaseBuild, *node, tmCache, localTM,
		                                  /*exportLighting=*/false);
		MESHEVAL::SEvalMesh evalMesh;
		std::vector<std::string> warnings;
		if (!MESHEVAL::evalNodeMesh(*node, evalMesh, &warnings))
		{
			++stats.Skipped;
			fprintf(stderr, "WARNING: context mesh '%s' skipped:", name.c_str());
			for (size_t w = 0; w < warnings.size(); ++w)
				fprintf(stderr, " %s", warnings[w].c_str());
			fprintf(stderr, "\n");
			continue;
		}
		NL3D::CMesh::CMeshBuild buildMesh;
		MESHBUILD::buildMeshInterface(evalMesh, buildMesh, buildBaseMesh, maxBaseBuild, *node, tmCache,
		                              /*skinned=*/false);
		NL3D::CMesh *mesh = new NL3D::CMesh();
		try
		{
			mesh->build(buildBaseMesh, buildMesh);
		}
		catch (const NLMISC::Exception &e)
		{
			delete mesh;
			++stats.Skipped;
			fprintf(stderr, "WARNING: context mesh '%s' build failed: %s\n", name.c_str(), e.what());
			continue;
		}

		// Unique bank name per instance
		std::string bankName = name;
		int suffix = 1;
		while (!usedNames.insert(bankName).second)
			bankName = name + NLMISC::toString("~%d", suffix++);
		shapeBank->add(bankName, mesh);
		NL3D::CTransformShape *inst = scene->createInstance(bankName);
		if (!inst)
		{
			++stats.Skipped;
			fprintf(stderr, "WARNING: context mesh '%s': createInstance failed\n", name.c_str());
			continue;
		}
		// Stand at the node's world TM at t=0 (correct for nested parents too; root-level
		// nodes match the baked shape defaults the plugin displayed).
		inst->setTransformMode(NL3D::ITransformable::DirectMatrix);
		inst->setMatrix(toNelMatrix(SCENELIB::getNodeTM(node, tmCache)));
		// The plugin's "Big hack to sort": clip-parent the instance under the landscape model
		land->clipAddChild(inst);
		stats.Shapes.push_back(mesh);
		++stats.Built;
	}
}

// ---------------------------------------------------------------------------------------------
// Scene ambient: render-environment reference 0 = the ambient color controller (Point3 0..1);
// its default value at t=0 through the typed keyframer, the storage counterpart of the
// original scene-ambient read.

bool decodeSceneAmbient(PMAXLOAD::SLoadedMax &lm, NLMISC::CRGBA &out)
{
	CSceneClassContainer *ssc = lm.Scene->container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CSceneImpl *impl = dynamic_cast<CSceneImpl *>(it->second);
		if (!impl) continue;
		CReferenceMaker *env = impl->getReference(4); // RenderEnvironment
		if (!env) return false;
		// The ambient color controller is the environment's first Point3 keyframer reference
		// (reference 0 in the reference layout observed; scan the first few defensively: the
		// value must be a 12-byte float triple in [0,1]).
		for (uint r = 0; r < 4; ++r)
		{
			CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(env->getReference(r));
			if (!kf) continue;
			uint size = 0;
			const uint8 *data = kf->defaultValue(size);
			if (!data || size < 12) continue;
			float rgb[3];
			memcpy(rgb, data, 12);
			bool plausible = true;
			for (int i = 0; i < 3; ++i)
				if (rgb[i] < 0.f || rgb[i] > 1.f) plausible = false;
			if (!plausible) continue;
			out = NLMISC::CRGBA((uint8)(rgb[0] * 255.f), (uint8)(rgb[1] * 255.f), (uint8)(rgb[2] * 255.f));
			return true;
		}
		return false;
	}
	return false;
}

// ---------------------------------------------------------------------------------------------
// Scene lights, decoded once through the lightmapper's storage decode. NB: the decode carries
// the lightmap-export appdata filter (default checked); a light unchecked for lightmap but
// checked for realtime would be missed (warned when detected, zero corpus hits expected;
// both flags default checked).

struct SDecodedLight
{
	NL3D::CLightmapLight L;
	bool Realtime;
};

static void decodeSceneLights(PMAXLOAD::SLoadedMax &lm, std::vector<SDecodedLight> &out)
{
	out.clear();
	SCENELIB::SNodeTMCache tmCache;
	// Node-handle map for the exclusion-list resolution (unused for display, required by the
	// decode signature)
	std::map<uint32, std::string> nodeByHandle;
	CSceneClassContainer *ssc = lm.Scene->container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		uint32 handle;
		if (LMSCENE::nodeHandle(node, handle))
			nodeByHandle[handle] = SCENELIB::nodeName(*node);
	}
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *node = dynamic_cast<CNodeImpl *>(it->second);
		if (!node) continue;
		// XRef-RESOLVING walk, same as the mesh path above: an XRefObject wrapper reports
		// superclass 0x60, so a wrapper-level 0x30 gate silently drops XRef'd lights.
		CSceneClass *obj = SCENELIB::baseObjectOf(*node, NULL, NULL);
		if (!obj) continue;
		if (obj->classDesc()->classId().a() == 0x92aab38c)
			continue; // unresolvable XRef (missing referenced file); resolver already warned
		if (obj->classDesc()->superClassId() != 0x30) continue; // lights only
		SDecodedLight dl;
		dl.Realtime = APPDATA::getScriptAppDataInt(node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, 1) == 1;
		if (!LMSCENE::convertLightmapLight(dl.L, *node, tmCache, nodeByHandle))
		{
			if (dl.Realtime)
				fprintf(stderr, "WARNING: light '%s' undecodable (or lightmap-unchecked); skipped for display\n",
				        SCENELIB::nodeName(*node).c_str());
			continue;
		}
		out.push_back(dl);
	}
}

// Color * multiplier, clamped (the original driver conversion applied GetIntensity)
static NLMISC::CRGBA multColor(NLMISC::CRGBA c, float mult)
{
	NLMISC::CRGBAF f(c);
	f *= mult;
	f.A = 1.f;
	NLMISC::CRGBA out;
	out.R = (uint8)std::min(255.f, std::max(0.f, f.R * 255.f));
	out.G = (uint8)std::min(255.f, std::max(0.f, f.G * 255.f));
	out.B = (uint8)std::min(255.f, std::max(0.f, f.B * 255.f));
	out.A = 255;
	return out;
}

uint setupDriverLights(PMAXLOAD::SLoadedMax &lm, NL3D::IDriver *driver)
{
	std::vector<SDecodedLight> lights;
	decodeSceneLights(lm, lights);
	uint n = 0;
	for (size_t i = 0; i < lights.size() && n < 8; ++i)
	{
		const NL3D::CLightmapLight &l = lights[i].L;
		NL3D::CLight nel;
		switch (l.Type)
		{
		case NL3D::CLightmapLight::LightPoint:
		case NL3D::CLightmapLight::LightAmbient:
			nel.setMode(NL3D::CLight::PointLight);
			break;
		case NL3D::CLightmapLight::LightSpot:
			nel.setMode(NL3D::CLight::SpotLight);
			break;
		case NL3D::CLightmapLight::LightDir:
			nel.setMode(NL3D::CLight::DirectionalLight);
			break;
		default:
			continue;
		}
		// Ambient-only lights feed the ambient term; others diffuse+specular (the original
		// affect-diffuse/specular storage was never located; both-on corpus-wide)
		NLMISC::CRGBA color = multColor(l.bAmbientOnly ? l.Ambient : l.Diffuse, l.rMult);
		if (l.bAmbientOnly)
		{
			nel.setAmbiant(color);
			nel.setDiffuse(NLMISC::CRGBA(0, 0, 0));
			nel.setSpecular(NLMISC::CRGBA(0, 0, 0));
		}
		else
		{
			nel.setAmbiant(NLMISC::CRGBA(0, 0, 0));
			nel.setDiffuse(color);
			nel.setSpecular(color);
		}
		nel.setPosition(l.Position);
		nel.setDirection(l.Direction);
		nel.setCutoff(l.rFallof);           // already the half-angle radians
		nel.setupSpotExponent(l.rHotspot);  // same conversion as the original
		if (l.rRadiusMax > 0.f)
			nel.setupAttenuation(l.rRadiusMin > 0.f ? l.rRadiusMin : 0.1f, l.rRadiusMax);
		else
			nel.setNoAttenuation();
		driver->setLight((uint8)n, nel);
		driver->enableLight((uint8)n, true);
		++n;
	}
	return n;
}

uint setupPaintLights(PMAXLOAD::SLoadedMax &lm, NL3D::CLandscape &landscape, NL3D::CScene &scene)
{
	// CPaintLight::setup preamble
	landscape.setDynamicLightingMaxAttEnd(1000);
	scene.enableLightingSystem(true);

	std::vector<SDecodedLight> lights;
	decodeSceneLights(lm, lights);
	uint n = 0;
	for (size_t i = 0; i < lights.size(); ++i)
	{
		const NL3D::CLightmapLight &l = lights[i].L;
		// CPaintLight::build filters: realtime-checked, directional skipped
		if (!lights[i].Realtime) continue;
		if (l.Type == NL3D::CLightmapLight::LightDir) continue;

		NL3D::CTransform *model = scene.createModel(NL3D::PointLightModelId);
		if (!model) return n;
		NL3D::CPointLightModel *plm = NLMISC::safe_cast<NL3D::CPointLightModel *>(model);
		plm->setTransformMode(NL3D::ITransformable::DirectMatrix);
		NLMISC::CMatrix mt = NLMISC::CMatrix::Identity;
		mt.setPos(l.Position);
		plm->setMatrix(mt);
		plm->PointLight.setupAttenuation(l.rRadiusMin, l.rRadiusMax);
		NLMISC::CRGBA ambient = l.Ambient;
		ambient.A = 255; // localAmbient contract (paint_light.cpp)
		plm->PointLight.setAmbient(ambient);
		plm->PointLight.setDiffuse(l.Diffuse);
		plm->PointLight.setSpecular(l.Specular);
		if (l.bAmbientOnly || l.Type == NL3D::CLightmapLight::LightAmbient)
			plm->PointLight.setType(NL3D::CPointLight::AmbientLight);
		else if (l.Type == NL3D::CLightmapLight::LightPoint)
			plm->PointLight.setType(NL3D::CPointLight::PointLight);
		else if (l.Type == NL3D::CLightmapLight::LightSpot)
		{
			plm->PointLight.setType(NL3D::CPointLight::SpotLight);
			plm->lookAt(l.Position, l.Position + l.Direction);
			plm->PointLight.setupSpotAngle(l.rHotspot, l.rFallof);
		}
		++n;
	}
	return n;
}

} /* namespace ZPCTX */

/* end of file */
