/**
 * \file context_display.cpp
 * \brief See context_display.h (design doc §14-paint, P3d).
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
// Port map:
//   paint.cpp myThread includeMeshes branch -> addContextMeshes + setupDriverLights +
//     decodeSceneAmbient (the branch applied scene ambient + driver lights with the meshes)
//   paint_light.cpp CPaintLight::build/setup -> setupPaintLights (unconditional in myThread)
//   CExportNel::buildShape (viewport meshes)  -> the shape exporter's shared evaluation:
//     evalNodeMesh + buildBaseMeshInterface/buildMeshInterface -> NL3D::CMesh (plain-mesh
//     route; special shape classes warn and skip — they are context display, not export)
//   CExportNel::buildLight (driver CLight)    -> LMSCENE::convertLightmapLight + the original
//     driver conversion rules (mode map, color * multiplier, cutoff/exponent, attenuation)

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
#include <nel/3d/transform_shape.h>

#include <cstdio>
#include <map>
#include <set>

#include "../pipeline_max/scene.h"
#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"

#include "../pipeline_max_export_common/max_load.h"
#include "../pipeline_max_export_common/appdata_util.h"
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

static CSceneClass *unwrapObject(CNodeImpl *node)
{
	CSceneClass *obj = dynamic_cast<CSceneClass *>(node->getReference(1));
	int guard = 8;
	while (obj && guard-- > 0)
	{
		CDerivedObject *derived = dynamic_cast<CDerivedObject *>(obj);
		if (!derived) break;
		obj = derived->baseObject();
	}
	return obj;
}

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
		CSceneClass *obj = unwrapObject(node);
		if (!obj) continue;
		// The plugin's viewport rule: zones stay in the landscape; every other buildable mesh
		// displays. GeomObject superclass gates out lights/cameras/helpers/splines silently.
		if (obj->classDesc()->superClassId() != ZP_SCLASS_GEOMOBJECT) continue;
		if (dynamic_cast<NELPATCH::CRklPatchObject *>(obj)) continue;

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
		// (reference 0 in the reference layout observed; scan the first few defensively — the
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
// checked for realtime would be missed — warned when detected, zero corpus hits expected
// (both flags default checked).

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
		CSceneClass *obj = unwrapObject(node);
		if (!obj || obj->classDesc()->superClassId() != 0x30) continue; // lights only
		SDecodedLight dl;
		dl.Realtime = APPDATA::getScriptAppDataInt(node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, 1) == 1;
		if (!LMSCENE::convertLightmapLight(dl.L, *node, tmCache, nodeByHandle))
		{
			if (dl.Realtime)
				fprintf(stderr, "WARNING: light '%s' undecodable (or lightmap-unchecked) — skipped for display\n",
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
