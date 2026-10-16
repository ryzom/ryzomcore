/**
 * \file material_build.h
 * \brief NeL material construction from .max materials — the headless counterpart of
 * CExportNel::buildMaterials / buildAMaterial / buildATexture (plugin_max/nel_mesh_lib/
 * export_material.cpp). NeL Material (scripted plugin, classid (0x64c75fec, 0x222b9eb9))
 * parameters are read from its ParamBlock2 blocks through the script-version parameter
 * tables (v14 = the whole shape corpus); standard materials through the delegate's blocks.
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_MATERIAL_BUILD_H
#define PIPELINE_MAX_EXPORT_SHAPE_MATERIAL_BUILD_H

#include <nel/misc/types_nl.h>
#include <nel/3d/material.h>

#include <string>
#include <vector>

#include "scene_lib.h"

namespace MATBUILD {

using namespace SCENELIB;

#define MAX_MAX_TEXTURE 8
#define UVGEN_MISSING (-1)
#define UVGEN_REFLEXION (-2)

// NeL material shader types (script iShaderType values)
#define SHADER_NORMAL 1
#define SHADER_BUMP 2
#define SHADER_USER_COLOR 3
#define SHADER_LIGHTMAP 4
#define SHADER_SPECULAR 5
#define SHADER_WATER 6
#define SHADER_PER_PIXEL_LIGHTING 7
#define SHADER_PER_PIXEL_LIGHTING_NO_SPEC 8

// CExportNel::CMaterialDesc counterpart
struct SMaterialDesc
{
	sint IndexInMaxMaterial;
	sint IndexInMaxMaterialAlternative;
	MAXMATH::Matrix3M UVMatrix;
	float CropU, CropV, CropW, CropH;

	SMaterialDesc()
	{
		IndexInMaxMaterial = UVGEN_MISSING;
		IndexInMaxMaterialAlternative = UVGEN_MISSING;
		UVMatrix = MAXMATH::Matrix3M::identity();
		CropU = 0.f;
		CropV = 0.f;
		CropW = 1.f;
		CropH = 1.f;
	}
};

// CExportNel::CMaxMaterialInfo counterpart
struct SMaterialInfo
{
	std::vector<SMaterialDesc> RemapChannel;
	std::string MaterialName;
	bool AlphaVertex;
	uint8 UVRouting[MAX_MAX_TEXTURE];
	bool ColorVertex;
	uint AlphaVertexChannel;
	bool TextureMatrixEnabled;

	SMaterialInfo()
	{
		AlphaVertex = false;
		ColorVertex = false;
		AlphaVertexChannel = 0;
		TextureMatrixEnabled = false;
		for (uint i = 0; i < MAX_MAX_TEXTURE; i++)
			UVRouting[i] = 0xff;
	}
};

// CExportNel::CMaxMeshBaseBuild counterpart
struct SMaxMeshBaseBuild
{
	uint FirstMaterial;
	uint NumMaterials;
	uint8 UVRouting[MAX_MAX_TEXTURE];
	bool NeedVertexColor;
	std::vector<SMaterialInfo> MaterialInfo;

	SMaxMeshBaseBuild()
	{
		FirstMaterial = 0;
		NumMaterials = 0;
		NeedVertexColor = false;
		for (uint i = 0; i < MAX_MAX_TEXTURE; i++)
			UVRouting[i] = 0xff;
	}
};

// Is this material scene object a NeL material with bWater set? (single material only)
bool hasWaterMaterial(INode &node);

// Does any (sub-)material carry a per-pixel-lighting shader needing a specific VP?
bool hasMaterialWithShaderForVP(INode &node, NL3D::CMaterial::TShader &shader);

// buildMaterials replication: appends this node's materials to the array.
void buildMaterials(std::vector<NL3D::CMaterial> &materials, SMaxMeshBaseBuild &maxBaseBuild, INode &node,
                    bool exportLighting);

// Material name (0x4001 chunk on the material scene object, UTF-16)
std::string materialName(CSceneClass *mtl);

} /* namespace MATBUILD */

#endif /* PIPELINE_MAX_EXPORT_SHAPE_MATERIAL_BUILD_H */

/* end of file */
