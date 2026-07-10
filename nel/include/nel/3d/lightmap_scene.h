// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef NL_LIGHTMAP_SCENE_H
#define NL_LIGHTMAP_SCENE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"
#include "nel/misc/rgba.h"
#include "nel/misc/bitmap.h"
#include "nel/3d/mesh.h"

#include <string>
#include <set>
#include <vector>

namespace NL3D
{

/**
 * Standalone-lightmapper scene-graph intermediate.
 *
 * One CLightmapScene is written per source scene at the 1_export stage (by the headless
 * .max exporter, the in-Max NeL export plugin, or the assimp mesh exporter) and consumed
 * by the standalone lightmapper at the 2_build stage. The lightmapper never reads any
 * source scene format — this file carries everything the lightmap computation reads from
 * the scene: receiver meshes as PRE-BUILD CMeshBuild/CMeshBaseBuild (the lightmapper
 * generates the lightmap UV unwrap and the final shape build runs after it), occluder
 * meshes for the shadow raytrace (with the materials whose transparency the rays sample),
 * and lights at source-scene fidelity (type incl. directional and ambient, shadow flag,
 * light group, animated-light name, hotspot/falloff, attenuation, exclusion lists).
 *
 * Project build options (lumel size, oversampling, shadow on/off, build quality) are NOT
 * in this file — they are build-step configuration and stay with the build scripts.
 *
 * Serialization is NeL versioned-binary; every record is versioned independently so the
 * format can evolve without breaking old files (this is a long-lived pipeline contract).
 */

// ***************************************************************************
/// A light, at source-scene fidelity (port of the Max exporter's SLightBuild).
class CLightmapLight
{
public:
	enum EType { LightAmbient = 0, LightPoint, LightDir, LightSpot };

	std::string		Name;
	std::string		AnimatedLight;
	uint32			LightGroup;
	EType			Type;
	NLMISC::CVector	Position;				// Used by LightPoint and LightSpot
	NLMISC::CVector	Direction;				// Used by LightSpot and LightDir
	float			rRadiusMin, rRadiusMax;	// Used by LightPoint and LightSpot
	float			rHotspot, rFallof;		// Used by LightSpot (half-angle, radians)
	NLMISC::CRGBA	Ambient;
	NLMISC::CRGBA	Diffuse;
	NLMISC::CRGBA	Specular;
	bool			bCastShadow;
	bool			bAmbientOnly;
	float			rMult;

	NLMISC::CBitmap	ProjBitmap;				// For projector (empty when none)
	NLMISC::CMatrix	mProj;					// For projector (light world matrix)

	std::set<std::string> setExclusion;		// Node names excluded from this light

	float			rSoftShadowRadius;
	float			rSoftShadowConeLength;

	// Runtime accelerator state, not serialized (set by the lightmapper per receiver)
	float			rDirRadius;

	CLightmapLight();

	void serial(NLMISC::IStream &f);
};

// ***************************************************************************
/// An occluder entry: one shadow-casting mesh of the scene, in object space.
class CLightmapSceneMesh
{
public:
	std::string					NodeName;
	CMesh::CMeshBuild			MeshBuild;
	CMeshBase::CMeshBaseBuild	BaseBuild;

	void serial(NLMISC::IStream &f);
};

// ***************************************************************************
/** One receiver geometry — a single lightmap computation unit (the receiver node itself,
 *	or one multi-lod slave slot). Carries the per-node lightmap appdata the computation
 *	reads and the multi-lod slot configuration the final shape build needs.
 */
class CLightmapReceiverGeom
{
public:
	std::string					NodeName;
	CMesh::CMeshBuild			MeshBuild;

	// Multi-lod slot configuration (final shape build; ignored for single-mesh receivers)
	float						DistMax;
	float						BlendLength;
	uint8						SlotFlags;		// CMeshMultiLodBuild::CBuildSlot flags
	// Per-slot MRM geometry (a multi-lod slot node's own LOD_MRM appdata + MRM parameters)
	bool						GeomMrm;
	uint32						MrmNLods;
	uint32						MrmDivisor;
	uint32						MrmSkinReduction;
	float						MrmDistanceFinest;
	float						MrmDistanceMiddle;
	float						MrmDistanceCoarsest;

	// Per-node lightmap appdata
	float						LumelSizeMul;
	bool						LmcEnabled;		// 8-bit lightmap compression
	NLMISC::CRGBA				LmcAmbient[3];	// per light group (0..2)
	NLMISC::CRGBA				LmcDiffuse[3];

	// First material index of this geometry's material span in the receiver's material
	// array (materials before it belong to other content and are not touched by the
	// lightmap material setup).
	uint32						FirstMaterial;

	// Occluder node names excluded from this geometry's raytrace world (the source
	// scene's LOD relationships: every LOD-slave in the scene plus the nodes this
	// geometry is a LOD of — never the geometry's own node).
	std::set<std::string>		ExcludeOccluders;

	CLightmapReceiverGeom();

	void serial(NLMISC::IStream &f);
};

// ***************************************************************************
/** A receiver: one exported shape with lightmapped materials, as pre-build state plus
 *	the recipe to finish the shape build after the lightmap computation.
 */
class CLightmapReceiver
{
public:
	std::string						NodeName;
	CMeshBase::CMeshBaseBuild		BaseBuild;

	// Geoms[0] = the receiver node itself; further entries = multi-lod slave slots.
	std::vector<CLightmapReceiverGeom>	Geoms;

	// Shape-build recipe
	bool							MultiLod;		// build a CMeshMultiLod from the geoms
	bool							StaticLod;		// CMeshMultiLodBuild::StaticLod
	bool							WantMrm;		// build a CMeshMRM (single-mesh path)
	// MRM parameters (CMRMParameters fields; used when WantMrm)
	uint32							MrmNLods;
	uint32							MrmDivisor;
	uint32							MrmSkinReduction;
	float							MrmDistanceFinest;
	float							MrmDistanceMiddle;
	float							MrmDistanceCoarsest;

	// Pre-remap material names, indexed like BaseBuild.Materials (animated-material setup)
	std::vector<std::string>		MaterialNames;
	bool							AnimatedMaterials;
	bool							AutoAnim;
	float							DistMax;

	// Output routing: shape goes to the with-coarse-mesh directory
	bool							CoarseOutput;

	CLightmapReceiver();

	void serial(NLMISC::IStream &f);
};

// ***************************************************************************
/// The per-source-scene container.
class CLightmapScene
{
public:
	/// Source scene name (project name — prefixes the generated lightmap texture names)
	std::string						ProjectName;

	std::vector<CLightmapLight>		Lights;
	std::vector<CLightmapSceneMesh>	Occluders;
	std::vector<CLightmapReceiver>	Receivers;

	void serial(NLMISC::IStream &f);

	/// File helpers (throw NLMISC::Exception on failure)
	void save(const std::string &path);
	void load(const std::string &path);
};

} // NL3D

#endif // NL_LIGHTMAP_SCENE_H

/* End of lightmap_scene.h */
