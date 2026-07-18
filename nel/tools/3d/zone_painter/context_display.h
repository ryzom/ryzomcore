/**
 * \file context_display.h
 * \brief Include-meshes context display + scene lights for the zone painter (P3d)
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The in-Max painter's "include meshes" option displayed the scene's non-zone meshes in the
 * painting viewport with the scene ambient and driver lights (paint.cpp myThread), and always
 * ran CPaintLight (point-light models feeding the scene lighting system). This TU ports both,
 * building shapes headlessly through the shape exporter's shared evaluation sources (the clod
 * reuse pattern: scene_lib/mesh_eval/material_build/mesh_build compiled in, no exporter source
 * modified) and decoding lights through the lightmapper's light decode (lm_scene_build).
 *
 * Kept in its own TU: the SCENELIB headers and the painter's patch_eval.h implementation unit
 * each define their own node-TM helpers; they must not share a translation unit.
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

#include <string>
#include <vector>

namespace NL3D {
class CScene;
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
	uint Skipped; // eligible mesh nodes whose build failed (warned)
	SContextStats() : Built(0), Skipped(0) { }
};

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
