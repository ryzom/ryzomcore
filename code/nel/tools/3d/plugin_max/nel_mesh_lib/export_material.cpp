// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdafx.h"
#include "export_nel.h"
#include "../tile_utility/tile_utility.h"
#include "nel/misc/path.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/texture_multi_file.h"
#include "nel/3d/texture_cube.h"
#include "nel/3d/tangent_space_build.h"
#include "nel/3d/meshvp_per_pixel_light.h"

#include <vector>
#include <string>

using namespace NLMISC;
using namespace NL3D;
using namespace std;

// Name of animatable values
#define BMTEX_CROP_APPLY "apply"
#define BMTEX_CROP_U_NAME "clipu"
#define BMTEX_CROP_V_NAME "clipv"
#define BMTEX_CROP_W_NAME "clipw"
#define BMTEX_CROP_H_NAME "cliph"

#define MAT_SLOTS 4

#define SHADER_NORMAL 1
#define SHADER_BUMP 2
#define SHADER_USER_COLOR 3
#define SHADER_LIGHTMAP 4
#define SHADER_SPECULAR 5
#define SHADER_WATER 6
#define SHADER_PER_PIXEL_LIGHTING 7
#define SHADER_PER_PIXEL_LIGHTING_NO_SPEC 8



#define NEL_BITMAP_TEXTURE_CLASS_ID_A 0x5a8003f9
#define NEL_BITMAP_TEXTURE_CLASS_ID_B 0x43e0955


/** Test wether the given max material is a water material. A water object should only have one material, and must have planar, convex geometry.
  * Morevover, the mapping should only have scale and offsets, no rotation
  */
bool CExportNel::hasWaterMaterial(INode& node, TimeValue time)
{
	// Get primary material pointer of the node
	Mtl* pNodeMat = node.GetMtl();
	// If NULL, no material at all at this node
	if (pNodeMat == NULL) return false;
	if (pNodeMat->NumSubMtls() != 0) return 0; // subMaterials not supported for water

	int bWater = 0; // false
	CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bWater", (ParamType2)TYPE_BOOL, &bWater, time);
	return bWater != 0;
}



///===================================================================================================
bool				CExportNel::hasMaterialWithShaderForVP(INode &node, TimeValue time, NL3D::CMaterial::TShader &shader)
{
	// Get primary material pointer of the node
	Mtl* pNodeMat = node.GetMtl();
	// If NULL, no material at all at this node
	if (pNodeMat == NULL) return false;
	return CExportNel::needVP(*pNodeMat, time, shader);
}

///===================================================================================================
bool                CExportNel::needVP(Mtl &mat, TimeValue time, NL3D::CMaterial::TShader &shader)
{
	if (mat.NumSubMtls() != 0) // look at sub materials
	{
		for (uint k = 0; k < (uint) mat.NumSubMtls(); ++k)
		{
			if (CExportNel::needVP(*(mat.GetSubMtl(k)), time, shader)) return true;
		}
		return false;
	}

	int shaderType = 0; // false
	CExportNel::getValueByNameUsingParamBlock2 (mat, "iShaderType", (ParamType2)TYPE_INT, &shaderType, time);
	switch (shaderType)
	{
		case 7:	// per pixel lighting (shader 7) need a specific VP					
			shader = CMaterial::PerPixelLighting;
			return true;
		break;
		case 8:	// per pixel lighting, no specular
			shader = CMaterial::PerPixelLightingNoSpec;
			return true;
		break;
		default:
			return false;
		break;
	}
}

///===================================================================================================
/// Build a per-pixel vertex program
static NL3D::CMeshVPPerPixelLight *BuildPerPixelLightingVP(NL3D::CMesh::CMeshBuild *mb, bool wantSpecular)
{
	// We need at least one texture set to be able to build the tangent space information.
	// In this version, we support only one texture coordinate for the input. If there are more the result is undefined.
	NL3D::CMesh::CMeshBuild otherMb;
	if (NL3D::BuildTangentSpace(otherMb, *mb)) // build succesful ?
	{
		*mb = otherMb;
		CMeshVPPerPixelLight *vp = new CMeshVPPerPixelLight;
		vp->SpecularLighting = wantSpecular;
		return vp;
	}
	else
	{
		return NULL;
	}

	return NULL;
}

///===================================================================================================
IMeshVertexProgram           *CExportNel::buildMeshMaterialShaderVP(NL3D::CMaterial::TShader shader, NL3D::CMesh::CMeshBuild *mb)
{
	nlassert(shader < CMaterial::shaderCount);
	switch (shader)
	{
		case CMaterial::PerPixelLighting:
			return BuildPerPixelLightingVP(mb, true /* with specular */);
		break;
		case CMaterial::PerPixelLightingNoSpec:
			return BuildPerPixelLightingVP(mb, false /* no spec */);
		break;
		default: return NULL; // no need for a vp
	}	
}

// Build an array of NeL material corresponding with max material at this node. Return the number of material exported.
// Fill an array to remap the 3ds vertexMap channels for each materials. maxBaseBuild.RemapChannel.size() must be == to materials.size(). 
// maxBaseBuild.RemapChannel[mat].size() is the final count of NeL vertexMap channels used for the material nb mat.
// For each NeL channel of a material, copy the 3ds channel maxBaseBuild.RemapChannel[nelChannel]._IndexInMaxMaterial using the transformation matrix
// maxBaseBuild.RemapChannel[nelChannel]._UVMatrix.
// maxBaseBuild.NeedVertexColor will be true if at least one material need vertex color. Forced to true if lightmaps are used.
// maxBaseBuild.AlphaVertex[mat] will be true if the material use per vertex alpha.
// maxBaseBuild.AlphaVertexChannel[mat] will be the channel to use to get the alpha if the material use per vertex alpha.
// This method append the node material to the vector passed.
void CExportNel::buildMaterials (std::vector<NL3D::CMaterial>& materials, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
								TimeValue time)
{
	// Material count
	maxBaseBuild.FirstMaterial=materials.size();
	int nMaterialCount=0;

	// Get primary material pointer of the node
	Mtl* pNodeMat=node.GetMtl();

	// If NULL, no material at all at this node
	if (pNodeMat!=NULL)
	{
		// Number of sub material at in this material
		nMaterialCount=pNodeMat->NumSubMtls();

		// If it is a multisub object, export all its sub materials
		if (nMaterialCount>0)
		{
			// Resize the destination array
			materials.resize (materials.size()+nMaterialCount);

			// Resize the vertMap remap table
			maxBaseBuild.MaterialInfo.resize (nMaterialCount);

			// Export all the sub materials
			for (int nSub=0; nSub<nMaterialCount; nSub++)
			{
				// Get a pointer on the sub material
				Mtl* pSub=pNodeMat->GetSubMtl(nSub);

				// Should not be NULL
				nlassert (pSub);

				// Export it
				buildAMaterial (materials[maxBaseBuild.FirstMaterial+nSub], maxBaseBuild.MaterialInfo[nSub], *pSub, time);

				// Need vertex color ?
				maxBaseBuild.NeedVertexColor |= maxBaseBuild.MaterialInfo[nSub].AlphaVertex | maxBaseBuild.MaterialInfo[nSub].ColorVertex;
			}
		}
		// Else export only this material, so, count is 1
		else
		{
			// Only one material
			nMaterialCount=1;

			// Resize the destination array
			materials.resize (materials.size()+1);

			// Resize the vertMap remap table
			maxBaseBuild.MaterialInfo.resize (1);

			// Export the main material
			buildAMaterial (materials[maxBaseBuild.FirstMaterial], maxBaseBuild.MaterialInfo[0], *pNodeMat, time);

			// Need vertex color ?
			maxBaseBuild.NeedVertexColor |= maxBaseBuild.MaterialInfo[0].AlphaVertex | maxBaseBuild.MaterialInfo[0].ColorVertex;
		}
	}

	// Normalize UVRouting
	uint i;
	for (i=0; i<maxBaseBuild.MaterialInfo.size (); i++)
	{
		uint j;
		for (j=0; j<MAX_MAX_TEXTURE; j++)
		{
			uint8 routing = maxBaseBuild.MaterialInfo[i].UVRouting[j];
			if (maxBaseBuild.UVRouting[j] == 0xff)
			{
				maxBaseBuild.UVRouting[j] = routing;
			}
			else
			{
				if (routing != 0xff )
				{
					if (routing != maxBaseBuild.UVRouting[j])
					{
						// Active the channel because someone need it
						maxBaseBuild.UVRouting[j] = j;
					}
				}
			}
		}
	}

	// If no material exported
	if (nMaterialCount==0)
	{
		// Insert at least a material
		materials.resize (materials.size()+1);
		nMaterialCount=1;

		// Resize the vertMap remap table
		maxBaseBuild.MaterialInfo.resize (1);

		// Init the first material
		materials[maxBaseBuild.FirstMaterial].initLighted();

		// Default mat
		materials[maxBaseBuild.FirstMaterial].setLighting(true, CRGBA::Black, CRGBA::White, CRGBA::White, CRGBA::Black);

		// Export the main material
		maxBaseBuild.MaterialInfo[0].MaterialName = "Default";
	}

	// Return the count of material
	maxBaseBuild.NumMaterials=nMaterialCount;
}

// Build a NeL material corresponding with a max material.
void CExportNel::buildAMaterial (NL3D::CMaterial& material, CMaxMaterialInfo& materialInfo, Mtl& mtl, TimeValue time)
{
	// It is a NeL material ?
	if (isClassIdCompatible (mtl, Class_ID(NEL_MTL_A,NEL_MTL_B)))
	{

		// Get the shader value now
		int iShaderType = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "iShaderType", (ParamType2)TYPE_INT, &iShaderType, time);

		// Carefule with options
		if ((iShaderType == SHADER_LIGHTMAP) && (!_Options.bExportLighting))
			iShaderType = SHADER_NORMAL;

		// Water or lightmap ??
		bool	isWaterOrLightmap= iShaderType==SHADER_WATER || iShaderType==SHADER_LIGHTMAP;
		bool	isLightmap= iShaderType==SHADER_LIGHTMAP;

		// *** Init lighted or unlighted ?

		// Get unlighted flag
		int bUnlighted = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bUnlighted", (ParamType2)TYPE_BOOL, &bUnlighted, 0);

		// force unlighted in case of lightmap.
		if(isLightmap)
			bUnlighted= 1;

		// Init the material
		if( bUnlighted )
			material.initUnlit ();
		else
			material.initLighted ();

		// *** Choose a shader
		switch (iShaderType)
		{
		case SHADER_NORMAL:
		case SHADER_BUMP:
		case SHADER_USER_COLOR:
		case SHADER_LIGHTMAP:
		case SHADER_SPECULAR:
		case SHADER_PER_PIXEL_LIGHTING:
		case SHADER_PER_PIXEL_LIGHTING_NO_SPEC:
			material.setShader ((CMaterial::TShader)(iShaderType-1));
			break;		
		case SHADER_WATER:
			material.setShader (CMaterial::Normal);
			break;
		default:
			nlassert(0);
			break;
		}

		// Stain glass window flag
		int bStainGlass = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bStainedGlassWindow", (ParamType2)TYPE_BOOL, &bStainGlass, time);		
		material.setStainedGlassWindow (bStainGlass!=0);

		// *** Alpha

		// Alpha blend ?
		int bAlphaBlend = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaBlend", (ParamType2)TYPE_BOOL, &bAlphaBlend, time);
		
		// Set the blend
		material.setBlend (bAlphaBlend!=0);

		// Blend function
		int blendFunc = 1;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "iBlendSrcFunc", (ParamType2)TYPE_INT, &blendFunc, time);
		material.setSrcBlend ((CMaterial::TBlend)(blendFunc-1));
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "iBlendDestFunc", (ParamType2)TYPE_INT, &blendFunc, time);
		material.setDstBlend ((CMaterial::TBlend)(blendFunc-1));
		// if water or lightMap, force std blend.
		if(isWaterOrLightmap)
			material.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);

		// Set the alpha test flag
		int bAlphaTest = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaTest", (ParamType2)TYPE_BOOL, &bAlphaTest, time);
		material.setAlphaTest (bAlphaTest != 0);

		// Vertex alpha
		int bAlphaVertex = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaVertex", (ParamType2)TYPE_BOOL, &bAlphaVertex, time);
		// if water or lightMap, disable alphaVertex
		if(isWaterOrLightmap)
			bAlphaVertex= 0;

		// Active vertex alpha if in alpha test or alpha blend mode
		materialInfo.AlphaVertex = (bAlphaBlend || bAlphaTest) && (bAlphaVertex != 0);

		// Get channel to use for alpha vertex
		materialInfo.AlphaVertexChannel = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "iAlphaVertexChannel", (ParamType2)TYPE_INT, &materialInfo.AlphaVertexChannel, time);

		// *** Zbuffer

		// ZBias
		float zBias = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "fZBias", (ParamType2)TYPE_FLOAT, &zBias, time);
		material.setZBias ( zBias );

		// Alpha blend ?
		if (bAlphaBlend)
		{
			// Force z write ?
			int bForceZWrite = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceZWrite", (ParamType2)TYPE_BOOL, &bForceZWrite, time);
			material.setZWrite( bForceZWrite != 0 );
		}
		else
		{
			// Force no z write ?
			int bForceNoZWrite = 0; // false
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceNoZWrite", (ParamType2)TYPE_BOOL, &bForceNoZWrite, time);
			material.setZWrite( bForceNoZWrite == 0 );
		}

		// *** Colors

		// * Vertex color ?

		int bColorVertex = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bColorVertex", (ParamType2)TYPE_BOOL, &bColorVertex, time);
		material.setLightedVertexColor (material.isLighted() && (bColorVertex != 0));

		// Ok, color vertex
		materialInfo.ColorVertex = bColorVertex != 0;


		// * Diffuse

		// Get from max
		Point3 maxDiffuse;
		CRGBA  nelDiffuse;
		CExportNel::getValueByNameUsingParamBlock2(mtl, "cDiffuse", (ParamType2)TYPE_RGBA, &maxDiffuse, time)
			|| CExportNel::getValueByNameUsingParamBlock2(mtl, "diffuse", (ParamType2)TYPE_RGBA, &maxDiffuse, time);

		// Convert to NeL color
		convertColor (nelDiffuse, maxDiffuse);

		// Set the opacity
		float fOp = 0.0f;
		CExportNel::getValueByNameUsingParamBlock2(mtl, "pOpacity", (ParamType2)TYPE_PCNT_FRAC, &fOp, time)
			|| CExportNel::getValueByNameUsingParamBlock2(mtl, "opacity", (ParamType2)TYPE_PCNT_FRAC, &fOp, time);

		// Add alpha to the value
		float fA=(fOp*255.f+0.5f);
		clamp (fA, 0.f, 255.f);
		nelDiffuse.A=(uint8)fA;

		// Set le NeL diffuse color material
		if (bUnlighted)
		{
			material.setColor (nelDiffuse);

			// For the lightmap
			material.setDiffuse (nelDiffuse);
		}
		else
		{
			material.setDiffuse (nelDiffuse);
			material.setOpacity (nelDiffuse.A);
		}
		
		// * Self illum color

		CRGBA nelEmissive;
		int bSelfIllumColorOn;
		Point3 maxSelfIllum;
		float fTemp;
		CExportNel::getValueByNameUsingParamBlock2(mtl, "bUseSelfIllumColor", (ParamType2)TYPE_BOOL, &bSelfIllumColorOn, time)
			|| CExportNel::getValueByNameUsingParamBlock2(mtl, "useSelfIllumColor", (ParamType2)TYPE_BOOL, &bSelfIllumColorOn, time);
		if( bSelfIllumColorOn )
		{
			CExportNel::getValueByNameUsingParamBlock2(mtl, "cSelfIllumColor", (ParamType2)TYPE_RGBA, &maxSelfIllum, time)
				|| CExportNel::getValueByNameUsingParamBlock2(mtl, "selfIllumColor", (ParamType2)TYPE_RGBA, &maxSelfIllum, time);
		}
		else
		{
			CExportNel::getValueByNameUsingParamBlock2(mtl, "pSelfIllumAmount", (ParamType2)TYPE_PCNT_FRAC, &fTemp, time)
				|| CExportNel::getValueByNameUsingParamBlock2(mtl, "selfIllumAmount", (ParamType2)TYPE_PCNT_FRAC, &fTemp, time);
			maxSelfIllum = maxDiffuse * fTemp;
		}
		convertColor( nelEmissive, maxSelfIllum );
		material.setEmissive (nelEmissive);

		// * Ambient

		CRGBA nelAmbient;
		Point3 maxAmbient;
		CExportNel::getValueByNameUsingParamBlock2(mtl, "cAmbient", (ParamType2)TYPE_RGBA, &maxAmbient, time)
			|| CExportNel::getValueByNameUsingParamBlock2(mtl, "ambient", (ParamType2)TYPE_RGBA, &maxAmbient, time);
		convertColor (nelAmbient, maxAmbient);
		material.setAmbient (nelAmbient);

		// * Specular

		CRGBA nelSpecular;
		Point3 maxSpecular;
		CExportNel::getValueByNameUsingParamBlock2(mtl, "cSpecular", (ParamType2)TYPE_RGBA, &maxSpecular, time)
			|| CExportNel::getValueByNameUsingParamBlock2(mtl, "specular", (ParamType2)TYPE_RGBA, &maxSpecular, time);
		convertColor (nelSpecular, maxSpecular);

		// Get specular level
		float shininess;
		CExportNel::getValueByNameUsingParamBlock2(mtl, "pSpecularLevel", (ParamType2)TYPE_PCNT_FRAC, &shininess, time)
			|| CExportNel::getValueByNameUsingParamBlock2(mtl, "specularLevel", (ParamType2)TYPE_PCNT_FRAC, &shininess, time);
		clamp(shininess, 0.f, 1.f);
		CRGBAF fColor = nelSpecular;
		fColor *= shininess;
		nelSpecular = fColor;
		material.setSpecular (nelSpecular);

		// * Get specular shininess
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "pGlossiness", (ParamType2)TYPE_PCNT_FRAC, &shininess, time)
			|| CExportNel::getValueByNameUsingParamBlock2 (mtl, "glossiness", (ParamType2)TYPE_PCNT_FRAC, &shininess, time);
		shininess=(float)pow(2.0, shininess * 10.0) * 4.f;
		material.setShininess (shininess);

		// * Usercolor

		if (iShaderType == SHADER_USER_COLOR)
		{
			CRGBA nelUserColor;
			Point3 maxUserColor;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "cUserColor", (ParamType2)TYPE_RGBA, &maxUserColor, time);
			convertColor (nelUserColor, maxUserColor);
			material.setUserColor (nelUserColor);
		}

		// * Double sided flag
		int bDoubleSided;
		CExportNel::getValueByNameUsingParamBlock2(mtl, "bTwoSided", (ParamType2)TYPE_BOOL, &bDoubleSided, time)
			|| CExportNel::getValueByNameUsingParamBlock2(mtl, "twoSided", (ParamType2)TYPE_BOOL, &bDoubleSided, time);
		material.setDoubleSided ( bDoubleSided!=0 );

		// *** Textures


		/// test wether texture matrix animation should be exported
		int bExportTexMatAnim = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bExportTextureMatrix", (ParamType2)TYPE_BOOL, &bExportTexMatAnim, 0);
		materialInfo.TextureMatrixEnabled = (bExportTexMatAnim != 0);

		// Reset info
		materialInfo.RemapChannel.clear ();

		// For each textures
		uint i;
		for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
		{
			// Lightmap, only one texture
			if ((iShaderType==SHADER_USER_COLOR) && (i>0)) break;
			if ((iShaderType==SHADER_LIGHTMAP) && (i>0)) break;
			if ((iShaderType==SHADER_SPECULAR) && (i>1)) break;

			// Get the enable flag name
			char enableSlotName[100];
			smprintf (enableSlotName, 100, "bEnableSlot_%d", i+1);

			// Get the enable flag 
			int bEnableSlot = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, enableSlotName, (ParamType2)TYPE_BOOL, &bEnableSlot, time);
			if (bEnableSlot)
			{
				// Get the texture arg name
				char textureName[100];
				smprintf (textureName, 100, "tTexture_%d", i+1);

				// Get the texture pointer
				Texmap *pTexmap = NULL;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, textureName, (ParamType2)TYPE_TEXMAP, &pTexmap, time);
				if (pTexmap)
				{
					// Pointer on the  diffuse texture
					static ITexture* pTexture=NULL;

					// Ok export the texture in NeL format
					CMaterialDesc materialDesc;
					pTexture=buildATexture (*pTexmap, materialDesc, time, (i==1) && (iShaderType==SHADER_SPECULAR) );

					// Get the gen texture coord flag
					char texGenName[100];
					smprintf (texGenName, 100, "bTexGen_%d", i+1);
					int bTexGen = 0;
					CExportNel::getValueByNameUsingParamBlock2 (mtl, texGenName, (ParamType2)TYPE_BOOL, &bTexGen, time);

					// If specular shader, set uv channel to reflexion
					if ( ((i==1) && (iShaderType==SHADER_SPECULAR)) || bTexGen )
					{
						materialDesc._IndexInMaxMaterial = UVGEN_REFLEXION;
					}

					materialInfo.RemapChannel.push_back (materialDesc);
		
					// Add flags if mapping coodinates are used..
					if (materialDesc._IndexInMaxMaterial>=0)
					{
						uint j;
						for (j=0; j<i; j++)
						{
							// Same UV channel ?
							if (materialInfo.RemapChannel[j]._IndexInMaxMaterial == materialDesc._IndexInMaxMaterial)
								break;
						}

						// Channel routing
						materialInfo.UVRouting[i] = (j == i) ? i : j;
					}

					// Add the texture if it exist
					material.setTexture (i, pTexture);

					// Envmap gen ?
					material.setTexCoordGen (i, materialDesc._IndexInMaxMaterial<0);

					/// texture matrix animation ?
					if (bExportTexMatAnim != 0)
					{										
						/// and activate flag
						material.enableUserTexMat(i);
						/// setup the uv matrix
						CMatrix uvMat;
						CExportNel::uvMatrix2NelUVMatrix(materialDesc.getUVMatrix(), uvMat);
						material.setUserTexMat(i, uvMat);
					}
				}
			}
		}

		// Export mapping channel 2 if lightmap asked.
		if ( iShaderType==SHADER_LIGHTMAP ) // lightmap enabled ?
		{
			// Get the lightmap UV channel
			int iLightMapChannel = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "iLightMapChannel", (ParamType2)TYPE_INT, &iLightMapChannel, time);

			// Resize it
			uint size = materialInfo.RemapChannel.size();
			materialInfo.RemapChannel.resize( size + 1 );

			// Copy information from channel 0
			if (materialInfo.RemapChannel.size () > 1)
				materialInfo.RemapChannel[size] = materialInfo.RemapChannel[0];

			// Source lightmap mapping channel
			materialInfo.RemapChannel[size]._IndexInMaxMaterial = iLightMapChannel;

			// Backup old mapping channel
			materialInfo.RemapChannel[size]._IndexInMaxMaterialAlternative = materialInfo.RemapChannel[0]._IndexInMaxMaterial;
		}

		// For each slot
		uint stage;
		for (stage=0; stage<std::min ((uint)MAT_SLOTS, (uint)IDRV_MAT_MAXTEXTURES); stage++)
		{			
			// Make a post fixe
			char postfixC[10];
			smprintf (postfixC, 10, "_%d", stage+1);
			string postfix = postfixC;

			// Shader normal ?
			if (iShaderType==SHADER_NORMAL)
			{
				// RGB, get the values
				int opRGB = 0;
				int opRGBBlend = 0;
				int opRGBArg0 = 0;
				int opRGBArg1 = 0;
				int opRGBArg2 = 0;
				int opRGBArg0Operand = 0;
				int opRGBArg1Operand = 0;
				int opRGBArg2Operand = 0;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbOperation"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGB, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbBlendSource"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBBlend, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg0"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg0, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg1"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg1, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg2"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg2, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg0Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg0Operand, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg1Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg1Operand, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg2Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg2Operand, time);				
				// Setup the value
				if (opRGB<5)
					material.texEnvOpRGB (stage, (CMaterial::TTexOperator)(opRGB-1));
				else
				{
					if (opRGB == 6)
					{
						material.texEnvOpRGB (stage, CMaterial::Mad);
					}
					else
					{					
						material.texEnvOpRGB (stage, (CMaterial::TTexOperator)(opRGBBlend+3));
					}
				}								
				material.texEnvArg0RGB (stage, (CMaterial::TTexSource)(opRGBArg0-1), (CMaterial::TTexOperand)(opRGBArg0Operand-1));				
				if (opRGBArg1 == 4)
				{				
					material.texEnvArg1RGB (stage, CMaterial::Texture, (CMaterial::TTexOperand)(opRGBArg1Operand-1));
				}
				else
				{
					material.texEnvArg1RGB (stage, (CMaterial::TTexSource)(opRGBArg1), (CMaterial::TTexOperand)(opRGBArg1Operand-1));				
				}
				if (opRGBArg2 == 4)
				{
					material.texEnvArg2RGB (stage, CMaterial::Texture, (CMaterial::TTexOperand)(opRGBArg2Operand-1));				
				}
				else
				{									
					material.texEnvArg2RGB (stage, (CMaterial::TTexSource)(opRGBArg2), (CMaterial::TTexOperand)(opRGBArg2Operand-1));
				}

				// Alpha, get the values
				int opAlpha = 0;
				int opAlphaBlend = 0;
				int opAlphaArg0 = 0;
				int opAlphaArg1 = 0;
				int opAlphaArg2 = 0;
				int opAlphaArg0Operand = 0;
				int opAlphaArg1Operand = 0;
				int opAlphaArg2Operand = 0;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaOperation"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlpha, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaBlendSource"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaBlend, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg0"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg0, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg1"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg1, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg2"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg2, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg0Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg0Operand, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg1Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg1Operand, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg2Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg2Operand, time);

				// Setup the value
				if (opAlpha<5)
					material.texEnvOpAlpha (stage, (CMaterial::TTexOperator)(opAlpha-1));
				else
				{
					if (opAlpha == 6)
					{
						material.texEnvOpAlpha(stage, CMaterial::Mad);
					}
					else
					{
						material.texEnvOpAlpha (stage, (CMaterial::TTexOperator)(opAlphaBlend+3));
					}
				}
				material.texEnvArg0Alpha (stage, (CMaterial::TTexSource)(opAlphaArg0-1), (CMaterial::TTexOperand)(opAlphaArg0Operand-1));
				if (opAlphaArg1 == 4)
				{
					material.texEnvArg1Alpha (stage, CMaterial::Texture, (CMaterial::TTexOperand)(opAlphaArg1Operand-1));					
				}
				else
				{				
					material.texEnvArg1Alpha (stage, (CMaterial::TTexSource)(opAlphaArg1), (CMaterial::TTexOperand)(opAlphaArg1Operand-1));
				}
				if (opAlphaArg2 == 4)
				{				
					material.texEnvArg2Alpha (stage, CMaterial::Texture, (CMaterial::TTexOperand)(opAlphaArg2Operand-1));
				}
				else
				{
					material.texEnvArg2Alpha (stage, (CMaterial::TTexSource)(opAlphaArg2), (CMaterial::TTexOperand)(opAlphaArg2Operand-1));					
				}
		
				// Constant color
				Point3 constantColor;
				CRGBA  nelConstantColor;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("cConstant"+postfix).c_str(), (ParamType2)TYPE_RGBA, &constantColor, time);

				// Convert to NeL color
				convertColor (nelConstantColor, constantColor);
		
				// Constant alpha
				int constantAlpha = 255;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iConstantAlpha"+postfix).c_str(), (ParamType2)TYPE_INT, &constantAlpha, time);
				nelConstantColor.A = (uint8)constantAlpha;

				// Setup the constant color
				material.texConstantColor(stage, nelConstantColor);

				// Texenv mode
				int texEnvMode = 0;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iTextureShader"+postfix).c_str(), (ParamType2)TYPE_INT, &texEnvMode, time);
				if (texEnvMode>1)
				{
					// Enable and setup it
					material.enableTexAddrMode ();
					material.setTexAddressingMode (stage, (CMaterial::TTexAddressingMode)(texEnvMode-2));
				}
			}
		}

		// Set material name		
		TSTR name=mtl.GetName();
		materialInfo.MaterialName = MaxTStrToUtf8(name);
	}
	else
	{
		// Not a nel material, try to get something near...

		// Init the material lighted
		material.initLighted ();

		// *** ***************
		// *** Export Textures
		// *** ***************

		/// TODO: Only one texture for the time. Add multitexture support, and shaders support.

		// Look for a diffuse texmap
		vector<bool> mapEnables;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "mapEnables", (ParamType2)TYPE_BOOL_TAB, &mapEnables, time, false);

		Texmap *pDifTexmap = NULL;
		Texmap *pOpaTexmap = NULL;
		Texmap *pSpeTexmap = NULL;

		if (mapEnables[ID_DI])
			pDifTexmap = mtl.GetSubTexmap (ID_DI);
		if (mapEnables[ID_OP])
			pOpaTexmap = mtl.GetSubTexmap (ID_OP);
		if (mapEnables[ID_SP])
			pSpeTexmap = mtl.GetSubTexmap (ID_SP);

		// Is there a lightmap handling wanted
		int bLightMap = 0; // false
		int bAlphaTest = 1; // true
		int bForceZWrite = 0; // false
		int bForceNoZWrite = 0; // false

		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceZWrite", (ParamType2)TYPE_BOOL, &bForceZWrite, time, false);
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceNoZWrite", (ParamType2)TYPE_BOOL, &bForceNoZWrite, time, false);

		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaTest", (ParamType2)TYPE_BOOL, &bAlphaTest, time, false);

		if( pSpeTexmap != NULL )
		{
			material.setShader (CMaterial::Specular);
		}
		else
		{
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bLightMap", (ParamType2)TYPE_BOOL, &bLightMap, time, false);
			if (bLightMap)
			{
				material.setShader (CMaterial::LightMap);
			}
			else
			{
				material.setShader (CMaterial::Normal);
			}
		}

		int bStainedGlassWindow = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bStainedGlassWindow", (ParamType2)TYPE_BOOL, &bStainedGlassWindow, time, false);
		material.setStainedGlassWindow( bStainedGlassWindow!=0 );

		material.setAlphaTest(false);

		// By default set blend to false
		material.setBlend (false);

		// Diffuse texmap is present ?
		if (pDifTexmap)
		{
			// Pointer on the  diffuse texture
			static ITexture* pTexture=NULL;

			// Is it a simple file ?
			if (isClassIdCompatible(*pDifTexmap, Class_ID (BMTEX_CLASS_ID, 0))
				||	isClassIdCompatible(*pDifTexmap, Class_ID (NEL_BITMAP_TEXTURE_CLASS_ID_A, NEL_BITMAP_TEXTURE_CLASS_ID_B))
				)
			{
				// List of channels used by this texture
				CMaterialDesc _3dsTexChannel;
				
				// Ok export the texture in NeL format
				pTexture=buildATexture (*pDifTexmap, _3dsTexChannel, time);

				// For this shader, only need a texture channel.
				materialInfo.RemapChannel.resize (1);

				// Need an explicit channel, not generated
				if ( _3dsTexChannel._IndexInMaxMaterial < 0 )
				{
					materialInfo.RemapChannel[0]._IndexInMaxMaterial=UVGEN_MISSING;
					materialInfo.RemapChannel[0]._UVMatrix.IdentityMatrix();
				}
				// Else copy it
				else 
					materialInfo.RemapChannel[0]=_3dsTexChannel;

				// Add the texture if it exist
				material.setTexture(0, pTexture);

				// Active blend if texture in opacity
				if( pOpaTexmap!=NULL )
				{
					if( bAlphaTest )
					{ // If Alpha Test enabled no blend required just check if we are forced to NOT write in the ZBuffer
						material.setAlphaTest(true);
						if( bForceNoZWrite )
							material.setZWrite( false );
						else
							material.setZWrite( true );
					}
					else
					{ // No Alpha Test so we have to blend and check if we are forced to write in the ZBuffer
						material.setBlend( true );
						if( bForceZWrite )
							material.setZWrite( true );
						else
							material.setZWrite( false );
					}
				}
				
	
				// Add flags if mapping coodinates are used..
				if (_3dsTexChannel._IndexInMaxMaterial>=0)
				{
					// Channel routing
					materialInfo.UVRouting[0] = 0;
				}

				// Export mapping channel 2 if lightmap asked.
				if ( bLightMap ) // lightmap enabled ?
				{
					materialInfo.RemapChannel.resize( 2 );
					// Copy information from channel 0
					materialInfo.RemapChannel[1] = materialInfo.RemapChannel[0];
					materialInfo.RemapChannel[1]._IndexInMaxMaterial = 2;
					materialInfo.RemapChannel[1]._IndexInMaxMaterialAlternative = materialInfo.RemapChannel[0]._IndexInMaxMaterial;

					// Add flags if mapping coodinates are used..
					materialInfo.UVRouting[1] |= 1;
				}
			}
		}

		if (pSpeTexmap)
		{
			// Pointer to the specular reflection texture
			CTextureCube* pTextureCube = NULL;

			if (isClassIdCompatible(*pSpeTexmap, Class_ID(ACUBIC_CLASS_ID, 0)))
			{
				pTextureCube = buildTextureCubeFromReflectRefract(*pSpeTexmap, time);
			}
			else if (isClassIdCompatible(*pSpeTexmap, Class_ID(COMPOSITE_CLASS_ID, 0)))
			{
				pTextureCube = buildTextureCubeFromComposite(*pSpeTexmap, time);
			}
			else if (isClassIdCompatible(*pSpeTexmap, Class_ID(BMTEX_CLASS_ID, 0)))
			{
				pTextureCube = buildTextureCubeFromTexture(*pSpeTexmap, time);
			}
			// Add the texture if it exist
			material.setTexture(1, pTextureCube);
		}

		// Blend mode
		int opacityType = 0; // 0-filter 1-substractive 2-additive
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "opacityType", (ParamType2)TYPE_INT, &opacityType, time, false);
		if( opacityType == 0 ) // Filter
			material.setBlendFunc (CMaterial::srcalpha, CMaterial::invsrcalpha);
		else
			material.setBlendFunc (CMaterial::srcalpha, CMaterial::one);

		// Z function by default. TODO.
		material.setZFunc (CMaterial::lessequal);

		// Z bias by default. TODO.
		material.setZBias (0.f);

		// Is the mtl a std material ?
		// The class Id can be the StdMat one, or StdMat2
		// It can be the superClassId if the mtl is derived from StdMat or StdMat2.
		if (
			isClassIdCompatible (mtl, Class_ID(DMTL_CLASS_ID, 0))	||
			isClassIdCompatible (mtl, Class_ID(DMTL2_CLASS_ID, 0))
			)
		{
			// Get a pointer on a stdmat
			//StdMat2* stdmat=(StdMat2*)&mtl;

			// *****************************************
			// *** Colors, self illumination and opacity
			// *****************************************

			// Get the diffuse color of the max material
			//Color color=stdmat->GetDiffuse (time);
			Point3 maxDiffuse;
			CRGBA  nelDiffuse;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "diffuse", (ParamType2)TYPE_RGBA, &maxDiffuse, time, false);

			// Convert to NeL color
			convertColor (nelDiffuse, maxDiffuse);
			// Get the opacity value from the material
			// float fOp=stdmat->GetOpacity (time);
			float fOp = 0.0f;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "opacity", (ParamType2)TYPE_PCNT_FRAC, &fOp, time, false);

			// Add alpha to the value
			float fA=(fOp*255.f+0.5f);
			clamp (fA, 0.f, 255.f);
			nelDiffuse.A=(uint8)fA;
			// Set le NeL diffuse color material
			material.setColor (nelDiffuse);

			// Set the blend mode on if opacity is not 1.f
			if( fOp < 0.99f )
			{
				if( bAlphaTest )
				{ // If Alpha Test enabled no blend required just check if we are forced to NOT write in the ZBuffer
					material.setAlphaTest(true);
					if( bForceNoZWrite )
						material.setZWrite( false );
					else
						material.setZWrite( true );
				}
				else
				{ // No Alpha Test so we have to blend and check if we are forced to write in the ZBuffer
					material.setBlend( true );
					if( bForceZWrite )
						material.setZWrite( true );
					else
						material.setZWrite( false );
				}
			}

			// Get colors of 3dsmax material
			CRGBA nelEmissive;
			CRGBA nelAmbient;
			CRGBA nelSpecular;
			//if (stdmat->GetSelfIllumColorOn())
			//	convertColor (emissiveColor, stdmat->GetSelfIllumColor (time));
			//else
			//	convertColor (emissiveColor, stdmat->GetDiffuse (time)*stdmat->GetSelfIllum (time));
			int bSelfIllumColorOn;
			Point3 maxSelfIllum;
			float fTemp;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "useSelfIllumColor", (ParamType2)TYPE_BOOL, &bSelfIllumColorOn, time, false);
			if( bSelfIllumColorOn )
			{
				CExportNel::getValueByNameUsingParamBlock2 (mtl, "selfIllumColor", (ParamType2)TYPE_RGBA, &maxSelfIllum, time, false);
			}
			else
			{
				CExportNel::getValueByNameUsingParamBlock2 (mtl, "selfIllumAmount", (ParamType2)TYPE_PCNT_FRAC, &fTemp, time, false);
				maxSelfIllum = maxDiffuse * fTemp;
			}
			convertColor( nelEmissive, maxSelfIllum );

			Point3 maxAmbient;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "ambient", (ParamType2)TYPE_RGBA, &maxAmbient, time, false);
			convertColor (nelAmbient, maxAmbient);

			Point3 maxSpecular;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "specular", (ParamType2)TYPE_RGBA, &maxSpecular, time, false);
			convertColor (nelSpecular, maxSpecular);

			// Specular level
			float shininess; //=stdmat->GetShinStr(time);
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "specularLevel", (ParamType2)TYPE_PCNT_FRAC, &shininess, time, false);
			CRGBAF fColor = nelSpecular;
			fColor *= shininess;
			nelSpecular = fColor;

			// Shininess
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "glossiness", (ParamType2)TYPE_PCNT_FRAC, &shininess, time, false);
			//shininess=stdmat->GetShader()->GetGlossiness(time);
			shininess=(float)pow(2.0, shininess * 10.0) * 4.f;

			// Light parameters
			material.setLighting (true, nelEmissive, nelAmbient, nelDiffuse, nelSpecular, shininess);

			// Double sided
			int bDoubleSided;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "twoSided", (ParamType2)TYPE_BOOL, &bDoubleSided, time, false);

			//material.setDoubleSided (stdmat->GetTwoSided()!=FALSE);
			material.setDoubleSided ( bDoubleSided!=0 );
		}

		if( ! bLightMap )
		{
			int bUnlighted = 0; // false
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bUnlighted", (ParamType2)TYPE_BOOL, &bUnlighted, 0, false);
			if( bUnlighted )	
			{
				material.setLighting( false );
			}
		}

		// Use alpha vertex ?
		int bAlphaVertex = 0; // false

		// Get the scripted value
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaVertex", (ParamType2)TYPE_BOOL, &bAlphaVertex, time, false);

		// Find ?
		if ( bAlphaVertex )
		{
			// Ok, alpha vertex
			materialInfo.AlphaVertex = true;

			if( bAlphaTest )
			{ 
				// If Alpha Test enabled no blend required just check if we are forced to NOT write in the ZBuffer
				material.setAlphaTest(true);
				if( bForceNoZWrite )
					material.setZWrite( false );
				else
					material.setZWrite( true );
			}
			else
			{ // No Alpha Test so we have to blend and check if we are forced to write in the ZBuffer
				material.setBlend( true );
				if( bForceZWrite )
					material.setZWrite( true );
				else
					material.setZWrite( false );
			}

			// Get the channel for alpha vertex
			materialInfo.AlphaVertexChannel = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "iAlphaVertexChannel", (ParamType2)TYPE_INT, &materialInfo.AlphaVertexChannel, time, false);
		}

		// Use color vertex ?
		int bColorVertex = 0; // false

		// Get the scripted value
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bColorVertex", (ParamType2)TYPE_BOOL, &bColorVertex, time, false);

		// Find ?
		if ( bColorVertex )
		{
			// Ok, color vertex
			materialInfo.ColorVertex = true;

			// Active vertex color in lighted mode
			material.setLightedVertexColor (material.isLighted());
		}

		// Set material name		
		TSTR name=mtl.GetName();
		materialInfo.MaterialName = MaxTStrToUtf8(name);
	}
}

// Get 3ds channels uv used by a texmap and make a good index channel
int CExportNel::getVertMapChannel (Texmap& texmap, Matrix3& channelMatrix, TimeValue time)
{
	// *** Get the channel matrix

	// Set to identity because deafult implementation of texmapGetUVTransform make nothing
	channelMatrix.IdentityMatrix();

	// Get UV channel matrix
	texmap.GetUVTransform(channelMatrix);

	// Return the map channel
	return texmap.GetMapChannel();
}
	
// get the absolute or relative path from a texture filename
static std::string 	ConvertTexFileName(const std::string &path, bool _AbsolutePath)
{
	if (_AbsolutePath)
	{
		return path;
	}
	else
	{
		return NLMISC::CFile::getFilename(path);
	}
}

// Build a NeL texture corresponding with a max Texmap.
// Fill an array with the 3ds vertexMap used by this texture. 
// Texture file uses only 1 channel.
ITexture* CExportNel::buildATexture (Texmap& texmap, CMaterialDesc &remap3dsTexChannel, TimeValue time, bool forceCubic)
{
	/// TODO: support other texmap than Bitmap
	// By default, not build
	ITexture* pTexture=NULL;

	// Is it a bitmap texture file ?
	if (isClassIdCompatible(texmap, Class_ID (BMTEX_CLASS_ID,0))
		|| isClassIdCompatible(texmap, Class_ID(NEL_BITMAP_TEXTURE_CLASS_ID_A, NEL_BITMAP_TEXTURE_CLASS_ID_B))
		)
	{
		// Cast the pointer
		BitmapTex* pBitmap=(BitmapTex*)&texmap;
		

		// Get the apply crop value
		int bApply;
		bool bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_APPLY, (ParamType2)TYPE_BOOL, &bApply, time);
		nlassert (bRes);

		// If a crop is applyed
		if (bApply)
		{
			// Get the crop value U
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_U_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel._CropU), time);
			nlassert (bRes);

			// Get the crop value V
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_V_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel._CropV), time);
			nlassert (bRes);

			// Get the crop value W
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_W_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel._CropW), time);
			nlassert (bRes);

			// Get the crop value H
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_H_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel._CropH), time);
			nlassert (bRes);
		}

		
		//  1) build a texture that can either be a file texture or a multifile texture

		ITexture  *srcTex;
		const uint numNelTextureSlots = 8;
		static const char *fileNamesTab[] = { "bitmap1FileName",
											 "bitmap2FileName",
											 "bitmap3FileName",
										     "bitmap4FileName",
											 "bitmap5FileName",
											 "bitmap6FileName",
											 "bitmap7FileName",
											 "bitmap8FileName"		
											};
		

		if (isClassIdCompatible(texmap, Class_ID(NEL_BITMAP_TEXTURE_CLASS_ID_A, NEL_BITMAP_TEXTURE_CLASS_ID_B))) // is it a set of textures ?
		{
			uint k;
			std::string fileName[numNelTextureSlots];
			for (k = 0; k < numNelTextureSlots; ++k) // copy each texture of the set
			{
				nlassert(k < sizeof(fileNamesTab) / sizeof(const char *));				

				/// get the file name from the parameter block
				bRes=getValueByNameUsingParamBlock2 (texmap, fileNamesTab[k], (ParamType2)TYPE_STRING, &fileName[k], time);
				nlassert (bRes);
			}

			uint numUsedSlots = 0;

			uint l;
			for (l = 0; l < numNelTextureSlots; ++l)
			{
				if (!fileName[l].empty()) numUsedSlots = l + 1;
			}

			// if only the first slot is used we create a CTextureFile...
			if (l == 1 && !fileName[0].empty())
			{
				srcTex = new CTextureFile;
				static_cast<CTextureFile *>(srcTex)->setFileName (ConvertTexFileName(fileName[0], _AbsolutePath));
			}
			else
			{
				srcTex = new CTextureMultiFile(numUsedSlots);
				for (k = 0; k < numUsedSlots; ++k) // copy each texture of the set
				{				
					if (!fileName[k].empty())
					{
						/// set the name of the texture after converting it
						std::string convertMultiTex = ConvertTexFileName(fileName[k], _AbsolutePath);
						static_cast<CTextureMultiFile *>(srcTex)->setFileName(k, convertMultiTex.c_str());
					}
				}
			}
		}
		else // standard texture
		{
			srcTex = new CTextureFile;
			std::string mapName = MCharStrToUtf8(pBitmap->GetMapName());
			static_cast<CTextureFile *>(srcTex)->setFileName (ConvertTexFileName(mapName, _AbsolutePath));
		}

		// 2) Use this texture 'as it', or duplicate it to create the faces of a cube map

		// Force cubic
		if (forceCubic)
		{
			// Cube side
			const static CTextureCube::TFace tfNewOrder[6] = {	CTextureCube::positive_z, CTextureCube::negative_z,
														CTextureCube::negative_x, CTextureCube::positive_x,
														CTextureCube::positive_y, CTextureCube::negative_y	};

			// Alloc a cube texture
			CTextureCube* pTextureCube = new CTextureCube;

			
			for (uint side=0; side<6; side++)
			{				
				// Set a copy of the source texture in the cube
				if (dynamic_cast<CTextureMultiFile *>(srcTex))
				{
					pTextureCube->setTexture (tfNewOrder[side], new CTextureMultiFile(*static_cast<CTextureMultiFile *>(srcTex)));
				}
				else
				{
					pTextureCube->setTexture (tfNewOrder[side], new CTextureFile(*static_cast<CTextureFile *>(srcTex)));
				}
			}

			// Ok, good texture
			pTexture=pTextureCube;
			delete srcTex;
			srcTex = NULL;
		}
		else
		{
			/// just use the source texture
			pTexture = srcTex;			
		}
	}
	else if (isClassIdCompatible(texmap, Class_ID(ACUBIC_CLASS_ID, 0)))
	{
		pTexture = buildTextureCubeFromReflectRefract(texmap, time);
	}
	else if (isClassIdCompatible(texmap, Class_ID(COMPOSITE_CLASS_ID, 0)))
	{
		pTexture = buildTextureCubeFromComposite(texmap, time);
	}

	// Get the UVs channel and the channel matrix
	Matrix3	channelMatrix;
	int nChannel=getVertMapChannel (texmap, channelMatrix, time);

	// Add the UVs channel
	remap3dsTexChannel._IndexInMaxMaterial=nChannel;
	remap3dsTexChannel._UVMatrix=channelMatrix;

	// check for tiling
	if (pTexture != NULL) // avoid possible crash
	{
		if (!(texmap.GetTextureTiling() & U_WRAP)) pTexture->setWrapS(ITexture::Clamp);
		if (!(texmap.GetTextureTiling() & V_WRAP)) pTexture->setWrapT(ITexture::Clamp);
	}

	// Return the texture pointer
	return pTexture;
}

/// Build a NeL texture cube from a Reflect/refract map containing 6 textures.
NL3D::CTextureCube *CExportNel::buildTextureCubeFromReflectRefract(Texmap &texmap, TimeValue time)
{
	if (isClassIdCompatible(texmap, Class_ID(ACUBIC_CLASS_ID, 0)))
	{
		// texture cube
		CTextureCube *pTextureCube = new CTextureCube();

		// set settings
		pTextureCube->setWrapS(ITexture::Clamp);
		pTextureCube->setWrapT(ITexture::Clamp);
		pTextureCube->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);

		// face order
		const static CTextureCube::TFace tfNewOrder[6] = 
			{ CTextureCube::positive_z, CTextureCube::negative_z, // up dn
			CTextureCube::negative_x, CTextureCube::positive_x, // lf rt
			// CTextureCube::positive_y, CTextureCube::negative_y };
			CTextureCube::negative_y, CTextureCube::positive_y }; // fr bk

		// vector of bitmap names
		vector<string> names;
		CExportNel::getValueByNameUsingParamBlock2(texmap, "bitmapName", (ParamType2)TYPE_STRING_TAB, &names, time);

		uint nNbSubMap = std::min<uint>((uint)names.size(), 6);
		for (uint i = 0; i < nNbSubMap; ++i)
		{			
			// Create a texture file
			CTextureFile *pT = new CTextureFile();
			
			// Set the file name
			pT->setFileName(ConvertTexFileName(names[i], _AbsolutePath));

			// Set the texture
			pTextureCube->setTexture(tfNewOrder[i], pT);
		}

		// return the texture cube
		return pTextureCube;
	}
	else return NULL;
}

/// Build a NeL texture cube from a Composite map containing 6 textures (note: no re-ordering is done, because it didn't reorder composite cube in previous version either).
NL3D::CTextureCube *CExportNel::buildTextureCubeFromComposite(Texmap &texmap, TimeValue time)
{
	if (isClassIdCompatible(texmap, Class_ID(COMPOSITE_CLASS_ID, 0)))
	{
		// texture cube
		CTextureCube *pTextureCube = new CTextureCube();

		// set settings
		pTextureCube->setWrapS(ITexture::Clamp);
		pTextureCube->setWrapT(ITexture::Clamp);
		pTextureCube->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
		
		// get number of maps
		uint nNbSubMap = std::min<uint>((uint)texmap.NumSubTexmaps(), 6);
		for (uint i = 0; i < nNbSubMap; ++i)
		{
			std::vector<CMaterialDesc> _3dsTexChannel;
			Texmap *pSubMap = texmap.GetSubTexmap(i);

			if (pSubMap != NULL && isClassIdCompatible(*pSubMap, Class_ID(BMTEX_CLASS_ID, 0)))
			{
				CMaterialDesc _3dsTexChannel;
				ITexture *pTexture = buildATexture(*pSubMap, _3dsTexChannel, time);
				pTextureCube->setTexture((CTextureCube::TFace)i, pTexture);
			}
		}

		// return the texture cube
		return pTextureCube;
	}
	else return NULL;
}

/// Build a NeL texture cube from a single texture.
NL3D::CTextureCube *CExportNel::buildTextureCubeFromTexture(Texmap &texmap, TimeValue time)
{
	if (isClassIdCompatible(texmap, Class_ID(BMTEX_CLASS_ID, 0)))
	{
		// texture cube
		CTextureCube *pTextureCube = new CTextureCube();

		// set settings
		pTextureCube->setWrapS(ITexture::Clamp);
		pTextureCube->setWrapT(ITexture::Clamp);
		pTextureCube->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);

		// List of channels used by this texture
		CMaterialDesc _3dsTexChannel;
		
		// Ok export the texture in NeL format
		ITexture *pTexture = buildATexture(texmap, _3dsTexChannel, time);
		pTextureCube->setTexture(CTextureCube::positive_x, pTexture);
		return pTextureCube;
	}
	else return NULL;
}

#pragma optimize("", on)
