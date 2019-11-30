// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2015  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
//
// Author: Jan BOON (Kaetemi) <jan.boon@kaetemi.be>

#include <nel/misc/types_nl.h>
#include "assimp_shape.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#define NL_NODE_INTERNAL_TYPE aiNode
#define NL_SCENE_INTERNAL_TYPE aiScene
#include "scene_context.h"

#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/pipeline/tool_logger.h>

#include <nel/3d/mesh.h>
#include <nel/3d/texture_file.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

// http://assimp.sourceforge.net/lib_html/materials.html

inline CRGBA convColor(const aiColor3D &ac, uint8 a = 255)
{
	return CRGBA(ac.r * 255.99f, ac.g * 255.99f, ac.b * 255.99f);
}

inline CRGBA convColor(const aiColor4D &ac)
{
	return CRGBA(ac.r * 255.99f, ac.g * 255.99f, ac.b * 255.99f, ac.a * 255.99f);
}

void assimpMaterial(NL3D::CMaterial &mat, CMeshUtilsContext &context, const aiMaterial *am)
{
	aiString amname;
	if (am->Get(AI_MATKEY_NAME, amname) != aiReturn_SUCCESS)
		amname = "";

	mat.initLighted();
	mat.setShader(CMaterial::Normal);

	int i;
	float f;
	aiColor3D c3;
	aiColor4D c4;

	if (am->Get(AI_MATKEY_TWOSIDED, i) == aiReturn_SUCCESS)
		mat.setDoubleSided(i != 0);

	if (am->Get(AI_MATKEY_BLEND_FUNC, i) == aiReturn_SUCCESS) switch ((aiBlendMode)i)
	{
	case aiBlendMode_Default:
		mat.setSrcBlend(CMaterial::srcalpha);
		mat.setDstBlend(CMaterial::invsrcalpha);
		break;
	case aiBlendMode_Additive:
		mat.setSrcBlend(CMaterial::one);
		mat.setDstBlend(CMaterial::one);
		break;
	}

	// Colors follow GL convention
	// "While the ambient, diffuse, specular and emission
	// "material parameters all have alpha components, only the diffuse"
	// "alpha component is used in the lighting computation."
	if (am->Get(AI_MATKEY_COLOR_DIFFUSE, c3) == aiReturn_SUCCESS)
		mat.setDiffuse(convColor(c3));

	if (am->Get(AI_MATKEY_OPACITY, f) == aiReturn_SUCCESS)
		mat.setOpacity(f * 255.99f);

	if (am->Get(AI_MATKEY_COLOR_AMBIENT, c3) == aiReturn_SUCCESS)
		mat.setAmbient(convColor(c3));

	if (am->Get(AI_MATKEY_SHININESS, f) == aiReturn_SUCCESS)
		mat.setShininess(f); // (float)pow(2.0, f * 10.0) * 4.f;

	if (am->Get(AI_MATKEY_COLOR_SPECULAR, c3) == aiReturn_SUCCESS)
		mat.setSpecular(convColor(c3));

	if (am->Get(AI_MATKEY_SHININESS_STRENGTH, f) == aiReturn_SUCCESS)
		mat.setSpecular(CRGBAF(mat.getSpecular()) * f);
	else
		mat.setSpecular(NLMISC::CRGBA::Black);

	if (am->Get(AI_MATKEY_COLOR_EMISSIVE, c3) == aiReturn_SUCCESS)
		mat.setEmissive(convColor(c3));

	// Textures
	unsigned int texCount = am->GetTextureCount(aiTextureType_DIFFUSE);
	if (texCount > IDRV_MAT_MAXTEXTURES)
	{
		tlwarning(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
			"Material '%s' has more than %i textures (%i textures found)", amname.C_Str(), IDRV_MAT_MAXTEXTURES, texCount);
		texCount = IDRV_MAT_MAXTEXTURES;
	}
	
	for (unsigned int ti = 0; ti < texCount; ++ti)
	{
		aiString path;
		aiTextureMapping mapping;
		unsigned int uvindex;
		float blend; // Partially supported
		aiTextureOp op;
		aiTextureMapMode mapmode;
		if (am->GetTexture(aiTextureType_DIFFUSE, ti, &path, &mapping, &uvindex, &blend, &op, &mapmode) != aiReturn_SUCCESS)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Failed to get texture %i in material '%s'", ti, amname.C_Str());
			break;
		}

		std::string fileName = CFile::getFilename(CPath::standardizePath(path.C_Str(), false));
		std::string knownPath = CPath::lookup(fileName, false, false, false);
		if (knownPath.empty())
		{
			tlwarning(context.ToolLogger, context.Settings.SourceFilePath.c_str(), 
				"Texture '%s' referenced in material '%s' but not found in the database search paths", fileName.c_str(), amname.C_Str());
		}

		// NeL supports bitmap and cubemap, but we import only basic bitmap here. Cubemap can be inserted from the mesh editor tool
		// NeL also has fancy multi-bitmap thing to switch between summer and winter and so on. Same story
		CSmartPtr<CTextureFile> tex = new CTextureFile();
		tex->setFileName(fileName);
		tex->setWrapS(mapmode == aiTextureMapMode_Clamp ? ITexture::Clamp : ITexture::Repeat);
		tex->setWrapT(mapmode == aiTextureMapMode_Clamp ? ITexture::Clamp : ITexture::Repeat);
		mat.setTexture(ti, tex);

		// TODO uvindex for uv routing (probably necessary during shape import - if so also need to also ask the uv channel in the editor and store in meta)

		// TODO aiTextureMapping texcoordgen if useful to import

		mat.texEnvArg0Alpha(ti, CMaterial::Texture, CMaterial::SrcAlpha);
		mat.texEnvArg0RGB(ti, CMaterial::Texture, CMaterial::SrcColor);
		mat.texEnvArg1Alpha(ti, ti == 0 ? CMaterial::Diffuse : CMaterial::Previous, CMaterial::SrcAlpha);
		mat.texEnvArg1RGB(ti, ti == 0 ? CMaterial::Diffuse : CMaterial::Previous, CMaterial::SrcColor);
		switch (op)
		{
		case aiTextureOp_Multiply:
		default:
			mat.texEnvOpAlpha(ti, CMaterial::Modulate);
			mat.texEnvOpRGB(ti, CMaterial::Modulate);
			break;
		case aiTextureOp_Add:
			mat.texEnvOpAlpha(ti, CMaterial::Add);
			mat.texEnvOpRGB(ti, CMaterial::Add);
			break;
		case aiTextureOp_Subtract:
			mat.texEnvArg0Alpha(ti, CMaterial::Texture, CMaterial::InvSrcAlpha);
			mat.texEnvArg0RGB(ti, CMaterial::Texture, CMaterial::InvSrcColor);
			mat.texEnvOpAlpha(ti, CMaterial::Add);
			mat.texEnvOpRGB(ti, CMaterial::Add);
			break;
		case aiTextureOp_SignedAdd:
			mat.texEnvOpAlpha(ti, CMaterial::AddSigned);
			mat.texEnvOpRGB(ti, CMaterial::AddSigned);
			break;
		}
	}
}

CSmartPtr<CMaterial> assimpMaterial(CMeshUtilsContext &context, const aiMaterial *am)
{
	CSmartPtr<CMaterial> matp = new CMaterial();
	CMaterial &mat = *matp;
	assimpMaterial(mat, context, am);
	return matp;
}

void assimpMaterials(CMeshUtilsContext &context)
{
	set<CSString> materialNames;

	const aiScene *scene = context.InternalScene;
	for (unsigned int mi = 0; mi < scene->mNumMaterials; ++mi)
	{
		const aiMaterial *am = scene->mMaterials[mi];

		for (unsigned int pi = 0; pi < am->mNumProperties; ++pi) // DEBUG
		{ // DEBUG
			const aiMaterialProperty *amp = am->mProperties[pi];
			printf("%s\n", amp->mKey.C_Str());
		} // DEBUG

		aiString amname;
		if (am->Get(AI_MATKEY_NAME, amname) != aiReturn_SUCCESS)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Material has no name");
			continue;
		}

		if (materialNames.find(amname.C_Str()) != materialNames.end())
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Material name '%s' used more than once", amname.C_Str());
			continue;
		}
		
		if (context.SceneMeta.Materials.find(amname.C_Str())
			== context.SceneMeta.Materials.end())
		{
			materialNames.insert(amname.C_Str());
			context.SceneMeta.Materials[amname.C_Str()] = assimpMaterial(context, am);
		}
	}
}

/* end of file */
