/**
 * \file context_display.h
 * \brief Include-meshes context display + scene lights for the zone painter
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * Ports the in-Max "include meshes" path (non-zone shapes in the paint viewport, scene ambient
 * and driver lights) plus CPaintLight point-light models. Shapes build through the shape
 * exporter's shared evaluation (scene_lib/mesh_eval/material_build/mesh_build); lights decode
 * through lm_scene_build.
 *
 * Own TU: SCENELIB and the painter's patch_eval.h each define node-TM helpers and must not
 * share a translation unit.
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

#ifndef ZONE_PAINTER_CONTEXT_DISPLAY_H
#define ZONE_PAINTER_CONTEXT_DISPLAY_H

#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>

#include <set>
#include <string>
#include <vector>

namespace NL3D {
class IShape;
class CScene;
class CTileBank;
class CShapeBank;
class CLandscape;
class CLandscapeModel;
class IDriver;
}

namespace PMAXLOAD {
struct SLoadedMax;
}

namespace ZPCTX {

struct SContextStats
{
	uint Built;
	uint Skipped;  // eligible mesh nodes whose build failed (warned)
	uint Filtered; // meta-geometry excluded by node properties (breakdown below)
	uint FilteredHidden, FilteredCollision, FilteredAccel, FilteredClass;
	std::vector<NL3D::IShape *> Shapes; // built shapes (bank-owned; texture resolution walks them)
	SContextStats() : Built(0), Skipped(0), Filtered(0),
		FilteredHidden(0), FilteredCollision(0), FilteredAccel(0), FilteredClass(0) { }
};

/** Out-of-the-box texture resolution for the context meshes: the built shapes carry BARE
 *	texture names (the export convention), but the materials' ParamBlock2 storage retains the
 *	AUTHORED absolute paths (the "R:\\graphics\\..." class) in the PBBitmap trailing 0x0003
 *	containers (UTF-16 path child) and in filename string params. Every authored path is
 *	resolved through DBPATH (the same mapping the xref/ig machinery uses; the database root is
 *	derived from the input path with the ig/cmb convention when unset) and each resolved
 *	file's DIRECTORY is registered on CPath (deduplicated, non-recursive). --search-path stays
 *	as an additional override. Returns the number of directories registered.
 */
uint registerContextTexturePaths(PMAXLOAD::SLoadedMax &lm, const std::string &inputPath,
                                 const std::string &bankPath,
                                 uint &resolvedOut, uint &missingOut);

/** Second-stage texture resolution over the BUILT shapes' material texture names: names the
 *	registered directories cannot serve (the game-facing seasonal vegetation set: shapes
 *	store `name.tga`, only `name_sp.dds` etc. exist, converted next to the ecosystem bank)
 *	are remapped per file to the first season variant found (spring first, the reference
 *	default). Fills resolved/missing counts over the unique texture names.
 */
void resolveContextShapeTextures(const SContextStats &stats, uint &resolvedOut, uint &missingOut);

/** Out-of-the-box texture resolution for the LANDSCAPE side: the bank references unpostfixed
 *	authored names (tile diffuse/additive/alpha bitmaps, displacement maps, e.g.
 *	alpha_noiseb_00.png) while the workspace carries only season-postfixed converted files
 *	(ecosystems/<eco>/tiles/<base>_<season>.dds next to the smallbank, or the
 *	landscape/_texture_tiles/<eco>_<season> sources). Registers the bank's sibling tiles/ and
 *	diplace/ dirs plus a per-name CPath::remapFile to a season-postfixed variant (preferred
 *	season first when set via setSeasonPreference, else discovery order starting at _sp),
 *	the same fallback the context-mesh names use. Re-calling after a preference change
 *	re-applies remaps (live season toggle). */
void resolveBankTextures(NL3D::CTileBank &bank, const std::string &bankPath,
                         uint &resolvedOut, uint &missingOut);

/// Shared season-variant name resolution (see resolveBankTextures). NULL what =
/// no per-name warnings (expected-absent sets). When a season preference is set and that
/// postfix exists, remaps even if an unpostfixed or other-season file already resolves.
void resolveNamesWithSeasons(const std::set<std::string> &names, const char *what,
                             uint &resolvedOut, uint &missingOut);

// ---------------------------------------------------------------------------------------------
// Season preference. Painting data is season-independent; only texture remaps change.

/** Season codes: "sp" spring, "su" summer, "au" autumn, "wi" winter. Empty = auto (first
 *	available postfix, historically _sp). Invalid codes return false without changing state. */
bool setSeasonPreference(const std::string &code);
/** Current preference code ("sp"/"su"/"au"/"wi") or empty when auto/unset. */
const std::string &seasonPreference();
/** Human label for the current preference (or "auto"). */
std::string seasonPreferenceLabel();

/**
 * Probe which season postfixes actually exist for a bank/workspace:
 *  - sibling converted tiles next to the bank (`../tiles/*_<season>.dds`)
 *  - source dirs `<dbroot>/landscape/_texture_tiles/<eco>_<season>/`
 * Only seasons with at least one resolvable file/dir are returned, in sp/su/au/wi order.
 * When none are found (e.g. snowballs unpostfixed tiles), the list is empty and the toggle
 * is a no-op.
 */
void discoverAvailableSeasons(const std::string &bankPath,
                              std::vector<std::string> &seasonsOut);

/** Cycle preference among discovered seasons (no-op when empty or size 1). Returns true when
 *	the preference changed. */
bool cycleSeasonPreference(const std::vector<std::string> &available);

/**
 * Live landscape reload after a season preference change: re-resolve bank remaps, release
 * every loaded tile so CTextureFile paths re-lookup through CPath, then optionally preload.
 * Paint state (tile indices / colors / displace) is untouched. Context-mesh materials are
 * NOT re-resolved (disproportionate; they keep the textures loaded at open).
 */
void reloadLandscapeSeasonTextures(NL3D::CTileBank &bank, const std::string &bankPath,
                                   NL3D::CLandscape *landscape, NL3D::IDriver *driver,
                                   bool preload);

/// Derive the DBPATH default root from a workspace file path when unset (the ig/cmb
/// convention). Called by the texture registration; call early when the bank resolution needs
/// the workspace source fallbacks before any context pass runs.
void ensureDbRootFrom(const std::string &inputPath);

/** Build every eligible non-zone mesh node of the loaded scene into a display shape and
 *	instance it into the viewer scene (ShapeBank->add + createInstance + clipAddChild under the
 *	landscape model, the plugin's "big hack to sort"). Eligibility is the plugin's viewport
 *	rule, broader than any export filter: every GeomObject node that is not an RklPatch zone
 *	and not a "[NELLIGO]" debug marker; nodes whose evaluation fails warn and are skipped.
 *	Instances stand at the node's world TM at t=0.
 */
void addContextMeshes(PMAXLOAD::SLoadedMax &lm, NL3D::CScene *scene, NL3D::CShapeBank *shapeBank,
                      NL3D::CLandscapeModel *land, SContextStats &stats);

/** Scene ambient color: the render-environment reference's ambient controller (environment
 *	reference 0) default value at t=0, the storage counterpart of the original exporter's
 *	scene-ambient read. Returns false (out untouched) when the environment or its ambient
 *	controller is not resolvable in this file.
 */
bool decodeSceneAmbient(PMAXLOAD::SLoadedMax &lm, NLMISC::CRGBA &out);

/** Driver lights (the includeMeshes branch's getLights -> setLight): every decodable scene
 *	light (no appdata filter in the original driver path, directional included), color scaled
 *	by the light multiplier like the original driver conversion. Returns the light count set.
 */
uint setupDriverLights(PMAXLOAD::SLoadedMax &lm, NL3D::IDriver *driver);

/** CPaintLight parity (unconditional in the plugin): enable the scene lighting system, set
 *	the landscape dynamic-light attenuation cap, and create a point-light model per scene
 *	light checked for realtime export (directional lights skipped, ambient-only respected).
 *	Returns the number of light models created.
 */
uint setupPaintLights(PMAXLOAD::SLoadedMax &lm, NL3D::CLandscape &landscape, NL3D::CScene &scene);

} /* namespace ZPCTX */

#endif /* ZONE_PAINTER_CONTEXT_DISPLAY_H */

/* end of file */
