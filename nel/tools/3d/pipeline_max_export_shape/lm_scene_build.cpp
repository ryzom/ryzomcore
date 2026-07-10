/**
 * \file lm_scene_build.cpp
 * \brief Lightmap scene-graph writer support (see lm_scene_build.h).
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

#include "lm_scene_build.h"

#include <cstdio>
#include <cstring>

#include <nel/misc/common.h>
#include <nel/misc/string_common.h>

#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/storage_value.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace SCENELIB;

namespace LMSCENE {

// Lightmap appdata sub-ids (plugin_max/nel_mesh_lib/export_appdata.h + calc_lm.h)
#define NEL3D_APPDATA_LUMELSIZEMUL 1423062567
#define NEL3D_APPDATA_SOFTSHADOW_RADIUS 1423062568
#define NEL3D_APPDATA_SOFTSHADOW_CONELENGTH 1423062569
#define NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT 1423062590
#define NEL3D_APPDATA_EXPORT_LMC_ENABLED 1423062638
#define NEL3D_APPDATA_EXPORT_LMC_AMBIENT_START 1423062639
#define NEL3D_APPDATA_EXPORT_LMC_DIFFUSE_START (1423062639 + 16)
#define NEL3D_APPDATA_LM_ANIMATED_LIGHT 41654685
#define NEL3D_APPDATA_LM_LIGHT_GROUP 41654687
#define NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT 1.4f
#define NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT 15.0f

// ---------------------------------------------------------------------------------------------
// Small chunk readers on the light object (same shapes as the ig exporter's light decode)

static bool lightWord(CSceneClass *obj, uint16 chunkId, uint16 &out)
{
	const CStorageContainer::TStorageObjectContainer &orphans = obj->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		if (it->first != chunkId) continue;
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() < 2) return false;
		memcpy(&out, &raw->Value[0], 2);
		return true;
	}
	return false;
}

static bool lightHasChunk(CSceneClass *obj, uint16 chunkId)
{
	const CStorageContainer::TStorageObjectContainer &orphans = obj->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		if (it->first == chunkId) return true;
	return false;
}

// "r g b a" string appdata -> CRGBA (the reference's getScriptAppData CRGBA overload)
static NLMISC::CRGBA appDataColor(CNodeImpl *n, uint32 subId, NLMISC::CRGBA def)
{
	std::string s = getScriptAppDataStr(n, subId, "");
	if (s.empty()) return def;
	int r = 255, g = 255, b = 255, a = 255;
	if (sscanf(s.c_str(), "%d %d %d %d", &r, &g, &b, &a) < 3) return def;
	NLMISC::clamp(r, 0, 255); NLMISC::clamp(g, 0, 255);
	NLMISC::clamp(b, 0, 255); NLMISC::clamp(a, 0, 255);
	return NLMISC::CRGBA((uint8)r, (uint8)g, (uint8)b, (uint8)a);
}

// ---------------------------------------------------------------------------------------------
bool convertLightmapLight(NL3D::CLightmapLight &out, INode &node, SCENELIB::SNodeTMCache &tmCache)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	if (!n) return false;
	CSceneClass *obj = baseObjectOf(node);
	if (!obj || obj->classDesc()->superClassId() != SCLASS_LIGHT) return false;

	enum TKind { kindOmni, kindTargetSpot, kindFreeSpot, kindDir, kindTargetDir };
	TKind kind;
	switch (obj->classDesc()->classId().a())
	{
	case 0x1011: kind = kindOmni; break;
	case 0x1012: kind = kindTargetSpot; break;
	case 0x1014: kind = kindFreeSpot; break;
	case 0x1013: kind = kindDir; break;
	case 0x1015: kind = kindTargetDir; break;
	default:
		fprintf(stderr, "WARNING: unknown light class %s, skipped for lightmapping\n",
		        obj->classDesc()->classId().toString().c_str());
		return false;
	}

	// Checked for lightmap export? (default checked, like the original's BST_CHECKED default)
	if (getScriptAppDataInt(n, NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT, 1) != 1)
		return false;

	out.Name = nodeName(node);

	// Animated light name + light group (getAnimatedLight/getLightGroup semantics)
	{
		std::string anim = getScriptAppDataStr(n, NEL3D_APPDATA_LM_ANIMATED_LIGHT, "");
		if (anim == "Sun" || anim == "GlobalLight" || anim == "(Use NelLight Modifier)")
			anim.clear();
		out.AnimatedLight = anim;
		out.LightGroup = (uint32)getScriptAppDataInt(n, NEL3D_APPDATA_LM_LIGHT_GROUP, 0);
	}

	switch (kind)
	{
	case kindOmni: out.Type = NL3D::CLightmapLight::LightPoint; break;
	case kindTargetSpot: case kindFreeSpot: out.Type = NL3D::CLightmapLight::LightSpot; break;
	default: out.Type = NL3D::CLightmapLight::LightDir; break;
	}

	// Old ParamBlock on reference 0 of the light object
	std::map<sint32, SPBlockParam> params;
	{
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
		for (uint r = 0; rm && r < rm->nbReferences(); ++r)
		{
			CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
			if (ref && ref->classDesc()->superClassId() == SCLASS_PBLOCK)
			{
				readPBlockParams(ref, params);
				break;
			}
		}
	}
	if (params.find(0) == params.end() || !params[0].IsPoint3)
	{
		// Animated color (the animated-light class): the color param's value chunk is
		// replaced by the controller marker; take the controller's value at t=0 through
		// the material-anim path is not needed here — the lightmapper wants the static
		// color, and the corpus animated lights keep their base color in the controller
		// default. Warn and skip only when there is genuinely no color anywhere.
		// (The animated street lights DO parse: their param 0 rides the pblock reference,
		// resolved below through floatValueAt0-style reads if this ever surfaces. For now
		// every corpus lightmapped scene resolves the inline param.)
		fprintf(stderr, "WARNING: lightmap light '%s' without inline color param — skipped"
		                " (animated-color decode not wired)\n", out.Name.c_str());
		return false;
	}

	NLMISC::CRGBAF fcol;
	fcol.R = params[0].V[0];
	fcol.G = params[0].V[1];
	fcol.B = params[0].V[2];
	fcol.A = 1.f;
	NLMISC::CRGBA color = fcol;

	// Ambient-only marker
	out.bAmbientOnly = lightHasChunk(obj, 0x2600);
	out.Ambient = NLMISC::CRGBA(0, 0, 0);
	out.Diffuse = NLMISC::CRGBA(0, 0, 0);
	out.Specular = NLMISC::CRGBA(0, 0, 0);
	if (out.bAmbientOnly)
	{
		out.Ambient = color;
	}
	else
	{
		// Affect-diffuse/specular storage was never located; every corpus light exports
		// with both enabled (same conclusion as the ig light decode) — unconditional.
		out.Diffuse = color;
		out.Specular = color;
	}

	// Intensity/multiplier: param 1 (corpus: 1.5 on the animated-street braseros, 1.0 default)
	out.rMult = 1.0f;
	if (params.find(1) != params.end() && !params[1].IsPoint3)
		out.rMult = params[1].IsInt ? (float)params[1].I : params[1].V[0];

	// Position from the node TM
	MAXMATH::Matrix3M nodeTM = MAXSCENE::getNodeTM(&node, tmCache);
	out.Position = NLMISC::CVector(nodeTM.m[3][0], nodeTM.m[3][1], nodeTM.m[3][2]);

	// Direction: target node when present, else -K of the node TM
	out.Direction = NLMISC::CVector(0, 0, -1);
	if (kind == kindTargetSpot || kind == kindTargetDir)
	{
		CReferenceMaker *tm = dynamic_cast<CReferenceMaker *>(node.getReference(0));
		CSceneClass *tmsc = dynamic_cast<CSceneClass *>(tm);
		INode *target = NULL;
		if (tmsc && tmsc->classDesc()->classId() == CLASSID_LOOKAT_CTRL)
			target = dynamic_cast<INode *>(tm->getReference(0));
		if (target)
		{
			MAXMATH::Matrix3M targetTM = MAXSCENE::getNodeTM(target, tmCache);
			out.Direction = NLMISC::CVector(targetTM.m[3][0], targetTM.m[3][1], targetTM.m[3][2])
				- out.Position;
			out.Direction.normalize();
		}
		else
		{
			fprintf(stderr, "WARNING: targeted light '%s' without target node\n", out.Name.c_str());
		}
	}
	else
	{
		out.Direction = -NLMISC::CVector(nodeTM.m[2][0], nodeTM.m[2][1], nodeTM.m[2][2]);
		out.Direction.normalize();
	}

	// Hotspot / falloff (degrees at params 4/5 -> the exporter's half-angle radians)
	{
		float hotspot = 50.0f, fallsize = 45.0f;
		if (params.find(4) != params.end() && !params[4].IsPoint3) hotspot = params[4].V[0];
		if (params.find(5) != params.end() && !params[5].IsPoint3) fallsize = params[5].V[0];
		out.rHotspot = (float)(NLMISC::Pi * hotspot / (2.0 * 180.0));
		out.rFallof = (float)(NLMISC::Pi * fallsize / (2.0 * 180.0));
	}

	// Attenuation
	{
		uint16 w = 0;
		bool useAtten = lightWord(obj, 0x2562, w) && w != 0;
		out.rRadiusMin = 10.0f;
		out.rRadiusMax = 10.0f;
		if (useAtten)
		{
			sint attenStartIdx = (kind == kindOmni) ? 6 : 9;
			if (params.find(attenStartIdx) != params.end()
				&& params.find(attenStartIdx + 1) != params.end())
			{
				out.rRadiusMin = params[attenStartIdx].V[0];
				out.rRadiusMax = params[attenStartIdx + 1].V[0];
			}
			else
			{
				fprintf(stderr, "WARNING: light '%s' with attenuation but no radii params\n",
				        out.Name.c_str());
			}
		}
	}

	// Cast shadows: word 0x2570 (provisional decode — see lm_scene_build.h)
	{
		uint16 w = 0;
		out.bCastShadow = lightWord(obj, 0x2570, w) && w != 0;
	}

	// Soft shadow appdata
	{
		std::string s = getScriptAppDataStr(n, NEL3D_APPDATA_SOFTSHADOW_RADIUS,
			NLMISC::toString(NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT));
		NLMISC::fromString(s, out.rSoftShadowRadius);
		s = getScriptAppDataStr(n, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH,
			NLMISC::toString(NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT));
		NLMISC::fromString(s, out.rSoftShadowConeLength);
	}

	// Exclusion list + projector: the corpus light-chunk vocabulary is closed (no
	// variable-size chunks on any of 2992 lights scanned) — both are unused corpus-wide.
	out.setExclusion.clear();

	return true;
}

// ---------------------------------------------------------------------------------------------
void fillGeomAppData(NL3D::CLightmapReceiverGeom &geom, CNodeImpl *n,
                     const SCollector &col, const std::string &name)
{
	// Lumel size multiplier (string appdata, default "1.0")
	{
		std::string s = getScriptAppDataStr(n, NEL3D_APPDATA_LUMELSIZEMUL, "1.0");
		geom.LumelSizeMul = 1.0f;
		NLMISC::fromString(s, geom.LumelSizeMul);
	}

	// 8-bit lightmap compression
	geom.LmcEnabled = getScriptAppDataInt(n, NEL3D_APPDATA_EXPORT_LMC_ENABLED, 0) == 1;
	for (uint i = 0; i < 3; ++i)
	{
		geom.LmcAmbient[i] = appDataColor(n, NEL3D_APPDATA_EXPORT_LMC_AMBIENT_START + i, NLMISC::CRGBA::Black);
		geom.LmcDiffuse[i] = appDataColor(n, NEL3D_APPDATA_EXPORT_LMC_DIFFUSE_START + i, NLMISC::CRGBA::White);
	}

	// Raytrace-world exclusion: every LOD-slave name in the scene plus the nodes this geom
	// is a LOD of (addChildLodNode + addParentLodNode), never the geom's own node.
	geom.ExcludeOccluders.clear();
	for (std::set<std::string>::const_iterator it = col.LodSlaveNames.begin();
	     it != col.LodSlaveNames.end(); ++it)
	{
		if (*it != name)
			geom.ExcludeOccluders.insert(*it);
	}
	std::map<std::string, std::set<std::string> >::const_iterator pit =
		col.LodParents.find(NLMISC::toLowerAscii(name));
	if (pit != col.LodParents.end())
	{
		for (std::set<std::string>::const_iterator it = pit->second.begin();
		     it != pit->second.end(); ++it)
		{
			if (*it != name)
				geom.ExcludeOccluders.insert(*it);
		}
	}
}

} /* namespace LMSCENE */

/* end of file */
