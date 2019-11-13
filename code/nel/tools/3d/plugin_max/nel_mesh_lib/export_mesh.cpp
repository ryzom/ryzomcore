// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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
#include "export_appdata.h"
#include "export_radial_normal.h"

#include "../nel_export/std_afx.h"
#include "../nel_export/nel_export.h"
#include "../nel_export/nel_export_scene.h"


#include "nel/3d/texture_file.h"
#include "nel/3d/texture_blend.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include "nel/3d/mesh_multi_lod.h"
#include "nel/3d/coarse_mesh_manager.h"
#include "nel/3d/water_shape.h"
#include "nel/3d/meshvp_wind_tree.h"


#include <nel/misc/polygon.h>
#include <nel/misc/path.h>
#include <nel/misc/aabbox.h>




using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************


void buildNeLMatrix (CMatrix& tm, const CVector& scale, const CQuat& rot, const CVector& pos)
{
	CMatrix scaleTM, rotTM, posTM;
	scaleTM.identity();
	scaleTM.scale (scale);
	rotTM.identity();
	rotTM.setRot (rot);
	posTM.identity();
	posTM.setPos (pos);
	tm=posTM*rotTM*scaleTM;
}

// ***************************************************************************


CMesh::CMeshBuild*	CExportNel::createMeshBuild(INode& node, TimeValue tvTime, CMesh::CMeshBaseBuild*& baseBuild, const CMatrix &masterNodeMat, bool isMorphTarget)
{
	CMesh::CMeshBuild *pMeshBuild = new CMesh::CMeshBuild();
	baseBuild = new CMeshBase::CMeshBaseBuild();

	// Get a pointer on the object's node
    ObjectState os = node.EvalWorldState(tvTime);
    Object *obj = os.obj;

	// Check if there is an object
	if (obj)
	{		

		// Object can be converted in triObject ?
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject*)obj->ConvertToType(tvTime, Class_ID(TRIOBJ_CLASS_ID, 0));
			if (tri)
			{

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				bool deleteIt=false;
				if (obj != tri) 
					deleteIt = true;

				// Description of materials
				CMaxMeshBaseBuild maxBaseBuild;

				// Fill the build interface of CMesh

				// Reset the material array of the buildMesh because it will be rebuild by the exporter
				baseBuild->Materials.clear();

				// Get the node matrix
				Matrix3 nodeMatrixMax;
				CMatrix nodeMatrix;
				getLocalMatrix (nodeMatrixMax, node, tvTime);
				convertMatrix (nodeMatrix, nodeMatrixMax);

				buildBaseMeshInterface (*baseBuild, maxBaseBuild, node, tvTime, nodeMatrix);
				buildMeshInterface (*tri, *pMeshBuild, *baseBuild, maxBaseBuild, node, tvTime, NULL, CMatrix::Identity, masterNodeMat, isMorphTarget);
			

				// Delete the triObject if we should...
				if (deleteIt)
					tri->DeleteThis();
				tri = NULL;
			}
		}
	}

	// Return the shape pointer or NULL if an error occurred.
	return pMeshBuild;
}

// ***************************************************************************
static void	copyMultiLodMeshBaseLod0Infos(CMeshBase::CMeshBaseBuild &dst, const CMeshBase::CMeshBaseBuild &src)
{
	dst.DefaultScale = src.DefaultScale;
	dst.DefaultRotQuat = src.DefaultRotQuat;
	dst.DefaultPos = src.DefaultPos;
	dst.CollisionMeshGeneration = src.CollisionMeshGeneration;
}

// ***************************************************************************
// Export a mesh
NL3D::IShape *CExportNel::buildShape (INode& node, TimeValue time, const TInodePtrInt *nodeMap, bool buildLods)
{

	// Is this a multi lod object ?
	bool multiLodObject = false;

	// Here, we must check what kind of node we can build with this mesh.
	// For the time, just Triobj is supported.
	NL3D::IShape *retShape = NULL;

	// If skinning, disable skin modifier
	if (nodeMap)
		enableSkinModifier (node, false);

	// Get a pointer on the object's node
    ObjectState os = node.EvalWorldState(time);
    Object *obj = os.obj;

	// Check if there is an object
	if (obj)
	{
		Class_ID  clid = obj->ClassID();
		// is the object a particle system ? (we do this defore meshs, because for now there is a mesh in max scenes to say where a particle system is...)
		///////////////
		// FX EXPORT //
		///////////////

		if (clid.PartA() == NEL_PARTICLE_SYSTEM_CLASS_ID)
		{
			return buildParticleSystem(node, time);			
		}

		if (clid.PartA() == NEL_WAVE_MAKER_CLASS_ID_A)
		{
			return buildWaveMakerShape(node, time);
		}

		// is it a remanent segment
		sint useRemanence = getScriptAppData (&node, NEL3D_APPDATA_USE_REMANENCE, 0);
		if (useRemanence)
		{
			return buildRemanence(node, time);
		}

		// is the object a flare ?
		if (clid.PartA() == NEL_FLARE_CLASS_ID_A /*&& clid.PartB() == NEL_FLARE_CLASS_ID_B*/)
		{
			return buildFlare(node, time);
		}


		

		// Object can be converted in triObject ?
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject *) obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));

			if (tri)
			{
				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				bool deleteIt=false;
				if (obj != tri) 
					deleteIt = true;

				if (hasWaterMaterial(node, time)) // is this a water shape ?
				{
					retShape = buildWaterShape(node, time);
				}
				else			
				{
					// Mesh base ?
					CMeshBase *meshBase = NULL;

					// Get the node matrix
					Matrix3 nodeMatrixMax;
					CMatrix nodeMatrix;
					getLocalMatrix (nodeMatrixMax, node, time);
					convertMatrix (nodeMatrix, nodeMatrixMax);

					// Is a multi lod object ?
					uint lodCount=getScriptAppData (&node, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
					if (lodCount && buildLods)
					{
						// This is a multilod object
						multiLodObject = true;

						// Listy of material names
						std::vector<std::string> listMaterialName;

						// Make the root mesh
						CMeshMultiLod::CMeshMultiLodBuild multiLodBuild;
						multiLodBuild.LodMeshes.reserve (lodCount+1);

						// Resize to one
						bool isTransparent;
						bool isOpaque;
						multiLodBuild.LodMeshes.resize (1);
						multiLodBuild.LodMeshes[0].MeshGeom=buildMeshGeom (node, time, nodeMap, multiLodBuild.BaseMesh, 
							listMaterialName, isTransparent, isOpaque, nodeMatrix);
						multiLodBuild.LodMeshes[0].DistMax=getScriptAppData (&node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
						multiLodBuild.LodMeshes[0].BlendLength=getScriptAppData (&node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT);
						multiLodBuild.LodMeshes[0].Flags=0;
						if (getScriptAppData (&node, NEL3D_APPDATA_LOD_BLEND_IN, NEL3D_APPDATA_LOD_BLEND_IN_DEFAULT))
							multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendIn;
						if (getScriptAppData (&node, NEL3D_APPDATA_LOD_BLEND_OUT, NEL3D_APPDATA_LOD_BLEND_OUT_DEFAULT))
							multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendOut;
						if ((getScriptAppData (&node, NEL3D_APPDATA_LOD_COARSE_MESH, NEL3D_APPDATA_LOD_COARSE_MESH_DEFAULT)) && (!_View))
							multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::CoarseMesh;
						if (isTransparent)
							multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::IsTransparent;
						if (isOpaque)
							multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::IsOpaque;
						multiLodBuild.StaticLod=getScriptAppData (&node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, NEL3D_APPDATA_LOD_DYNAMIC_MESH_DEFAULT)==0;

						// Bacup scale, rot and pos, etc...
						CMeshBase::CMeshBaseBuild	bkupMeshBase;
						copyMultiLodMeshBaseLod0Infos(bkupMeshBase, multiLodBuild.BaseMesh);

						// Build a world to local matrix
						CMatrix worldToNodeMatrix;

						// Is first slot is skinned ?
						INode *rootSkel=getSkeletonRootBone (node);

						// For all the other lods
						for (uint lod=0; lod<lodCount; lod++)
						{
							// Get the name
							std::string nodeName=getScriptAppData (&node, NEL3D_APPDATA_LOD_NAME+lod, "");

							// Get the node
							INode *lodNode=_Ip->GetINodeByName(MaxTStrFromUtf8(nodeName).data());
							if (lodNode)
							{
								// Index of the lod in the build structure
								uint index=multiLodBuild.LodMeshes.size();

								// Resize the build structure
								multiLodBuild.LodMeshes.resize (index+1);

								// Get matrix node
								CMatrix nodeTM;
								convertMatrix (nodeTM, lodNode->GetNodeTM (time));

								// Get the parent matrix
								CMatrix parentMatrix;
								if (rootSkel)
								{
									// Yes..
									CMatrix tmp;
									convertMatrix (tmp, rootSkel->GetNodeTM (time));
									parentMatrix=nodeTM;
								}
								else
								{
									buildNeLMatrix (parentMatrix, bkupMeshBase.DefaultScale, bkupMeshBase.DefaultRotQuat, bkupMeshBase.DefaultPos);
								}

								// Fill the structure
								multiLodBuild.LodMeshes[index].MeshGeom=buildMeshGeom (*lodNode, time, nodeMap, multiLodBuild.BaseMesh, 
									listMaterialName, isTransparent, isOpaque, parentMatrix);
								multiLodBuild.LodMeshes[index].DistMax=getScriptAppData (lodNode, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
								multiLodBuild.LodMeshes[index].BlendLength=getScriptAppData (lodNode, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT);
								multiLodBuild.LodMeshes[index].Flags=0;
								if (getScriptAppData (lodNode, NEL3D_APPDATA_LOD_BLEND_IN, NEL3D_APPDATA_LOD_BLEND_IN_DEFAULT))
									multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendIn;
								if (getScriptAppData (lodNode, NEL3D_APPDATA_LOD_BLEND_OUT, NEL3D_APPDATA_LOD_BLEND_OUT_DEFAULT))
									multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendOut;
								if ((getScriptAppData (lodNode, NEL3D_APPDATA_LOD_COARSE_MESH, NEL3D_APPDATA_LOD_COARSE_MESH_DEFAULT)) && (!_View))
									multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::CoarseMesh;
								if (isTransparent)
									multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::IsTransparent;
								if (isOpaque)
									multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::IsOpaque;
							}
						}

						// Restaure default pos, scale and rot, etc...
						copyMultiLodMeshBaseLod0Infos(multiLodBuild.BaseMesh, bkupMeshBase);


						// Make a CMeshMultiLod mesh object
						CMeshMultiLod *multiMesh = new CMeshMultiLod; // FIXME: there is a delete bug with CMeshMultiLod exported from max!!!

						// Build it
						multiMesh->build(multiLodBuild);

						// Return this pointer
						meshBase = multiMesh;

						// ** force material to be animatable
						if (CExportNel::getScriptAppData (&node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0) != 0)
						{
							/// todo hulud: check if material are animated before
							for (uint i=0; i<listMaterialName.size(); i++)
							{
								meshBase->setAnimatedMaterial (i, listMaterialName[i]);
							}
						}
					}
					else
					{					
						// Array of name for the material
						CMaxMeshBaseBuild maxBaseBuild;

						// Fill the build interface of CMesh
						CMeshBase::CMeshBaseBuild buildBaseMesh;
						buildBaseMeshInterface (buildBaseMesh, maxBaseBuild, node, time, nodeMatrix);

						CMesh::CMeshBuild buildMesh;
						buildMeshInterface (*tri, buildMesh, buildBaseMesh, maxBaseBuild, node, time, nodeMap);

						if( hasLightMap( node, time ) && _Options.bExportLighting )
							calculateLM(&buildMesh, &buildBaseMesh, node, time, maxBaseBuild.FirstMaterial, _Options.OutputLightmapLog);

						// optimized materials remap
						std::vector<sint>	materialRemap;

						// MRM mesh ?
						if (getScriptAppData (&node, NEL3D_APPDATA_LOD_MRM, 0))
						{
							// Build a MRM parameters block
							CMRMParameters	parameters;
							buildMRMParameters (node, parameters);

							// Get the blend shapes that can be linked
							std::vector<CMesh::CMeshBuild*> bsList;
							getBSMeshBuild (bsList, node, time, nodeMap!=NULL);

							// CMeshMRM or CMeshMRMSkinned ?

							/*
							 * Here, export plugin choose between CMeshMRM and CMeshMRMSkinned
							 */
							if (CMeshMRMSkinned::isCompatible(buildMesh) && bsList.empty())
							{
								// Make a CMesh object
								CMeshMRMSkinned* meshMRMSkinned=new CMeshMRMSkinned;

								// Build the mesh with the build interface
								meshMRMSkinned->build (buildBaseMesh, buildMesh, parameters);

								// optimize number of material
								meshMRMSkinned->optimizeMaterialUsage(materialRemap);

								// Return this pointer
								meshBase=meshMRMSkinned;
							}
							else
							{
								// Make a CMesh object
								CMeshMRM* meshMRM=new CMeshMRM;

								// Build the mesh with the build interface
								meshMRM->build (buildBaseMesh, buildMesh, bsList, parameters);

								// optimize number of material
								meshMRM->optimizeMaterialUsage(materialRemap);

								// Return this pointer
								meshBase=meshMRM;
							}
						}
						else
						{
							// Make a CMesh object
							CMesh* mesh=new CMesh;

							// Build the mesh with the build interface
							mesh->build (buildBaseMesh, buildMesh);

							// Must be done after the build to update vertex links
							// Pass to buildMeshMorph if the original mesh is skinned or not
							buildMeshMorph (buildMesh, node, time, nodeMap!=NULL);
							mesh->setBlendShapes (buildMesh.BlendShapes);

							// optimize number of material
							mesh->optimizeMaterialUsage(materialRemap);

							// Return this pointer
							meshBase=mesh;
						}

						// Animate materials (must do it after optimizeMaterialUsage());
						if (CExportNel::getScriptAppData (&node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0) != 0)
						{					
							for (uint i=0; i<maxBaseBuild.NumMaterials; i++)
							{
								// get the material name of the original material (not remaped)
								std::string	matName= maxBaseBuild.MaterialInfo[i].MaterialName;
								// get the remaped material id.
								sint	dstMatId= materialRemap[i];

								// if this material still exist in the final data
								if(dstMatId>=0)
									// animate it
									meshBase->setAnimatedMaterial (dstMatId, matName);
							}
						}
					}

					// check wether this mesh is auto-animated. Force to false if in view mode
					if ( !_View && (CExportNel::getScriptAppData (&node, NEL3D_APPDATA_AUTOMATIC_ANIMATION, 0) != 0) )
					{
						// yes, it is
						meshBase->setAutoAnim(true);
					}

					// Return the mesh base
					retShape = meshBase;
				}

				// Delete the triObject if we should...
				if (deleteIt)
					tri->DeleteThis();
				tri = NULL;
			}
		}
	}

	// If skinning, renable skin modifier
	if (nodeMap)
		enableSkinModifier (node, true);

	// Set the dist max for this shape
	if (retShape && !multiLodObject && buildLods)
	{
		// Get the dist max for this node
		float distmax = getScriptAppData (&node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
		retShape->setDistMax (distmax);
	}

	// Return the shape pointer or NULL if an error occurred.
	return retShape;
}

// ***************************************************************************

// Get the normal of a face for a given corner in localSpace
Point3 CExportNel::getLocalNormal (int face, int corner, Mesh& mesh)
{
	// Vertex number
	int nVertex=mesh.faces[face].v[corner];

	// Pointer on a render vertex
	RVertex *pRVertex=mesh.getRVertPtr(nVertex);

	// Smoothing group of the face
	DWORD nSmoothGroup=mesh.faces[face].smGroup;

	// No group ?
	if (nSmoothGroup==0)
		// Return face normal
		return mesh.getFaceNormal (face);

	// Specified normal
	if(pRVertex->rFlags & SPECIFIED_NORMAL)
	{
		return pRVertex->rn.getNormal();
	}

	// Get the number of normals in this vertex
	int nNumNormal=pRVertex->rFlags & NORCT_MASK;

	// If only on normal, get it
	if (nNumNormal==1)
	{
		return pRVertex->rn.getNormal();
	}
	// Else look for the good one
	else if (nNumNormal>1)
	{
		// Enum other normals and find the one with same smoothing group flags
		for (int nNormal=0; nNormal<nNumNormal; nNormal++)
		{
			// Get the normal's pointer
			RNormal* pNormal=&pRVertex->ern[nNormal];
			if (pNormal->getSmGroup()&nSmoothGroup)
			{
				return pNormal->getNormal();
			}
		}
	}
	
	// Return face normal
	return mesh.getFaceNormal (face);
}

// ***************************************************************************

// Build a base mesh interface
void CExportNel::buildBaseMeshInterface (NL3D::CMeshBase::CMeshBaseBuild& buildMesh, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
										 TimeValue time, const CMatrix& nodeBasis)
{
	// *** ***************
	// *** Exports some flags
	// *** ***************

	// shadow
	buildMesh.bCastShadows = (node.CastShadows() != 0);
	buildMesh.bRcvShadows  = (node.RcvShadows() != 0);

	// Export RealTime lighting info.
	NL3D::CMaterial::TShader shader;
	bool needVp = hasMaterialWithShaderForVP(node, time, shader);
	if (!needVp)
	{
		buildMesh.UseLightingLocalAttenuation= CExportNel::getScriptAppData (&node, NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION, BST_UNCHECKED) == BST_CHECKED;
	}
	else
	{
		// For now, all v.p that depends on material shader fon't handle local attenuation
		buildMesh.UseLightingLocalAttenuation = false;
	}

	// Export Camera Third person related
	sint	appDataCameraCol= CExportNel::getScriptAppData (&node, NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION, 0);
	// 3 means ForceCameraCol and Hide. Hide is encoded in the IG (not in the mesh)
	if(appDataCameraCol>=3)
		buildMesh.CollisionMeshGeneration= NL3D::CMeshBase::ForceCameraCol;
	else
		buildMesh.CollisionMeshGeneration= (NL3D::CMeshBase::TCameraCollisionGenerate)appDataCameraCol;

	// *** ****************
	// *** Export materials
	// *** ****************

	// Build materials in NeL format and get the number of materials exported in NeL format
	buildMaterials (buildMesh.Materials, maxBaseBuild, node, time);

	// Special: Add VertexColor If WindTree
	// What Vertexprogram is used??
	int	vpId= CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VERTEXPROGRAM_ID, 0);
	// Setup vertexProgram
	switch(vpId)
	{
		case 0: 
			break;
		case 1: 
			// Force VertexColor
			maxBaseBuild.NeedVertexColor= true;
			break;
	};

	// Some check. should have one rempa vertMap channel table by material
	nlassert (maxBaseBuild.MaterialInfo.size()==maxBaseBuild.NumMaterials);

	// *** *****************************
	// *** Export default transformation
	// *** *****************************

	// Get the node matrix
	Matrix3 localTM;
	convertMatrix (localTM, nodeBasis);

	// Get the translation, rotation, scale of the node
	CVector pos, scale;
	CQuat rot;
	decompMatrix (scale, rot, pos, localTM);

	// Set the default values
	buildMesh.DefaultScale=scale;
	buildMesh.DefaultRotQuat=rot;
	buildMesh.DefaultRotEuler=CVector (0,0,0);
	buildMesh.DefaultPivot=CVector (0,0,0);
	buildMesh.DefaultPos=pos;

	Modifier *pMorphMod = getModifier (&node, MAX_MORPHER_CLASS_ID);
	if (pMorphMod != NULL)
	for (uint32 i = 0; i < 100; ++i)
	{
		INode *pNode = (INode*)pMorphMod->GetReference (101+i);
		if (pNode == NULL)
			continue;
		// get factor here !
		buildMesh.DefaultBSFactors.push_back(0.0f);
		std::string sTemp = MCharStrToUtf8(pNode->GetName());
		buildMesh.BSNames.push_back (sTemp);
	}

	// Ok, done.
}

// ***************************************************************************

// Build a mesh interface
void CExportNel::buildMeshInterface (TriObject &tri, CMesh::CMeshBuild& buildMesh, const NL3D::CMeshBase::CMeshBaseBuild& buildBaseMesh, 
									 const CMaxMeshBaseBuild& maxBaseBuild, INode& node, TimeValue time, const TInodePtrInt* nodeMap, 
									 const CMatrix& newBasis, const CMatrix& finalSpace, bool isMorphTarget)
{
	// Get a pointer on the 3dsmax mesh
	Mesh *pMesh=&tri.mesh;

	// Build normals.
	// * "buildRenderNormals()" smooth faces that are not in the same material but at least in the same smoothing group.
	// * "buildNormals()" smooth faces that are in the same material and at least in the same smoothing group.
	// * I have choose buildRenderNormals (Hulud).
	pMesh->buildRenderNormals();		// I prefer this way to compute normals (Hulud)
	//pMesh->buildNormals();

	// Put default vertex flags. xyz and normals...
	buildMesh.VertexFlags=CVertexBuffer::PositionFlag|CVertexBuffer::NormalFlag;

	// Is this mesh skined ?
	bool skined=(nodeMap!=NULL);

	// *** ***************************
	// *** Compute the export matrix
	// *** ***************************

	// *** If the mesh is skined, vertices will be exported in world space.
	// *** If the mesh is not skined, vertices will be exported in offset space.

	// Compute matrices: object -> export space and the inverted one.
	CMatrix ToExportSpace;
	CMatrix FromExportSpace;

	if (skined)
	{
		// *** Get an "object to world" matrix

		// Simply go into world space
		Matrix3 tm=node.GetObjectTM(time);
		convertMatrix (ToExportSpace, tm);
		FromExportSpace=ToExportSpace;
		FromExportSpace.invert ();
	}
	else
	{
		// *** Get an "object to local" matrix

		/*
		 * In max, vertices are not in a pre specified basis. We must transform them by an object matrix (ObjectTM)
		 * to get them in the world basis. This matrix can change regarding the pipline flow of max.
		 *
		 * We will call local matrix the 3dsmax node offset matrix.
		 * We will call object matrix the matrix an object needs to be multiplied by to transform it into world space.
		 * We will call transformation matrix the matrix given by the animation of the node.
		 * We will call parent matrix the node matrix of the parent node of this node.
		 * We will call node matrix the complete transformation of a node without the local matrix.
		 *
		 * Our model vertices will be exported in local basis.
		 *
		 * The entire transformation used to transform the points in 3dsmax is: ( parent * transformation * local )
		 *
		 * objectToLocal = ( transformation-1 * parent-1 * object )
		 *
		 * But, parent * transformation = node
		 *
		 * objectToLocal = ( node-1 * object )
		 */

		// Get the invert node matrix
		Matrix3 invNodeTM=node.GetNodeTM(time);
		invNodeTM.Invert();

		// Get the object matrix
		Matrix3 objectTM=node.GetObjectTM(time);

		// Compute the local to world matrix
		Matrix3 objectToLocal=objectTM*invNodeTM;

		// Invert matrix in NeL format
		convertMatrix (ToExportSpace, objectToLocal);
		ToExportSpace=newBasis*ToExportSpace*finalSpace;
		FromExportSpace=ToExportSpace;
		FromExportSpace.invert ();
	}

	// *** *********************
	// *** Look for RGB vertices
	// *** *********************

	// Does the mesh have RGB vertices ? 0 is the color vertex channel.
	if (pMesh->mapSupport (0) && maxBaseBuild.NeedVertexColor)
	{
		// Add flag for color vertices
		buildMesh.VertexFlags|=CVertexBuffer::PrimaryColorFlag;
	}
	
	
	// *** ***************************
	// *** Build remap uv channel data
	// *** ***************************

	// Get the uv flags
	uint mappingChannelUsed = 0;
	uint i;
	for (i=0; i<MAX_MAX_TEXTURE; i++)
	{
		// This UV channel is used ?
		if (maxBaseBuild.UVRouting[i] == i)
			mappingChannelUsed |= 1<<i;
	}
	buildMesh.VertexFlags|=mappingChannelUsed<<(CVertexBuffer::TexCoord0);

	// Set the Uv routing
	const uint count = std::min((uint)MAX_MAX_TEXTURE, (uint)CVertexBuffer::MaxStage);
	for (i=0; i<count; i++)
	{
		if (maxBaseBuild.UVRouting[i] == 0xff)
			buildMesh.UVRouting[i] = i;
		else
			buildMesh.UVRouting[i] = maxBaseBuild.UVRouting[i];
	}
	for (; i<CVertexBuffer::MaxStage; i++)
		buildMesh.UVRouting[i] = i;

	// *** ***************
	// *** Export vertices
	// *** ***************

	// Number of vertices
	int nNumVertices=pMesh->getNumVerts();

	// Resize the vertex table
	buildMesh.Vertices.resize (nNumVertices);

	// Export vertex
	for (int vertex=0; vertex<nNumVertices; vertex++)
	{
		// Transforme the vertex in local coordinate
		Point3 v=pMesh->getVert(vertex);
		CVector vv=ToExportSpace*CVector (v.x, v.y, v.z);

		// Build a NeL vertex
		buildMesh.Vertices[vertex]=vv;
	}

	
	// *** ************
	// *** Export faces
	// *** ************

	// Number of faces
	int nNumFaces=pMesh->getNumFaces();
	
	// Resize the vertex table
	buildMesh.Faces.resize (nNumFaces);

	// Get the smoothing groups flag that use a radial vertices
	CRadialVertices radialVertices;
	radialVertices.init (&node, pMesh, time, *_Ip, *this);

	// Export vertex
	for (int face=0; face<nNumFaces; face++)
	{
		// Max face pointer
		Face *pFace=&pMesh->faces[face];

		// Does it use a radial normal ?
		bool useRadialNormal = radialVertices.isUsingRadialNormals (face);
		
		// *** ******************
		// *** Export material ID
		// *** ******************

		// Material index used by this face
		sint nMaterialID=(sint)pFace->getMatID();

		// Trunc the ID cause max take MatID%MatCount as final matID.
		// So MatId can be > than the number of material at this mod.
		if (maxBaseBuild.NumMaterials>0)
			nMaterialID%=maxBaseBuild.NumMaterials;
		else
			nMaterialID=0;

		// Material offset 
		nMaterialID+=maxBaseBuild.FirstMaterial;

		// Get the material ID
		buildMesh.Faces[face].MaterialId=nMaterialID;

		// Get the smooth group
		buildMesh.Faces[face].SmoothGroup = pFace->smGroup;
		

		// Ref on the material
		const CMaterial &material = buildBaseMesh.Materials[nMaterialID];

		// Info about the material
		CRGBA diffuse = material.getDiffuse ();
		CRGBA color = material.getColor ();
		uint8 opacity = material.getOpacity ();
		bool isLighted = material.isLighted ();

		// Check the matId is valid
		nlassert (buildMesh.Faces[face].MaterialId>=0);
		nlassert (buildMesh.Faces[face].MaterialId<(sint)(maxBaseBuild.FirstMaterial+maxBaseBuild.NumMaterials));

		// Does this object use a vertex program that need specific vertex format ?
		bool vpColorVertex = false;
		uint vertexProgram = getScriptAppData (&node, NEL3D_APPDATA_VERTEXPROGRAM_ID, 0);
		switch (vertexProgram)
		{
		case 1:
			// Wind tree
			vpColorVertex = true;
			break;
		}

		// Export the 3 corners
		for (int corner=0; corner<3; corner++)
		{
			// *** *************
			// *** Export vertex
			// *** *************

			// Corner pointer
			CMesh::CCorner *pCorner=&buildMesh.Faces[face].Corner[corner];

			// Vertex index
			pCorner->Vertex=pFace->v[corner];

			// Does this face use radial normal ?
			Point3 vNormal;
			if (useRadialNormal)
			{
				// Get the local normal
				vNormal = radialVertices.getLocalNormal (pCorner->Vertex, face);
			}
			else
			{
				// Normal value in local
				vNormal = getLocalNormal (face, corner, *pMesh);
			}

			// *** *************
			// *** Export normal
			// *** *************

			// *** Transform normal in world coordinate

			// Make a plane with the normal
			CPlane plane (vNormal.x, vNormal.y, vNormal.z, 0.f);

			// Use transformation of the plane to transforme the normal in world coordinate. (plane * M-1)
			plane=plane*FromExportSpace;

			// Store the normal
			pCorner->Normal=plane.getNormal();

			// Renormalize for security
			pCorner->Normal.normalize();


			// *** **********
			// *** Export Uvs
			// *** **********

			// Num of channels used in this material
			int nNumChannelUsed=maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].RemapChannel.size();

			// For each mapping channels used by this material
			int uv;
			for (uv=0; uv<nNumChannelUsed; uv++)
			{
				// No explicit channel or unsupported mapping channel, fill with garbage
				pCorner->Uvws[uv].U=0.f;
				pCorner->Uvws[uv].V=0.f;
				pCorner->Uvws[uv].W=0.f;

				// Corresponding max channel
				int nMaxChan=maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].RemapChannel[uv]._IndexInMaxMaterial;

				// Not a generated mapping channel ?
				if (nMaxChan>=0)
				{
					if( ! pMesh->mapSupport(nMaxChan) )
					{
						nMaxChan = maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].RemapChannel[uv]._IndexInMaxMaterialAlternative;
					}										

					Matrix3 uvMatrix;
					if (maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].TextureMatrixEnabled)
					{
						uvMatrix.IdentityMatrix();
					}
					else
					{
						uvMatrix = maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].RemapChannel[uv]._UVMatrix;
					}					 
					
					// Crop values
					float fCropU=maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].RemapChannel[uv]._CropU;
					float fCropV=maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].RemapChannel[uv]._CropV;
					float fCropW=maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].RemapChannel[uv]._CropW;
					float fCropH=maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].RemapChannel[uv]._CropH;

					// Check kind of channel and if channel is supported
					if ((nMaxChan>=0)&&(nMaxChan<MAX_MESHMAPS)&&pMesh->mapSupport(nMaxChan))
					{
						// *** Explicit channel

						// Get a pointer on Mappingvertex for this channel
						TVFace *pMapFace=pMesh->mapFaces(nMaxChan);

						// Get the index of the mapping vertex
						DWORD nMapVert=pMapFace[face].getTVert (corner);

						// Check it's a valid channel (not a color channel ie !=0)
						nlassert (nMaxChan>0);
						nlassert (nMaxChan<MAX_MESHMAPS);
						
						// Pointer on the mapping vertex
						UVVert *pMapVert=&pMesh->mapVerts(nMaxChan)[nMapVert];

						// Transforme the UV vertex
						Point3 uvTransformed=(*pMapVert)*uvMatrix;

						// Crop it
						uvTransformed.x=uvTransformed.x*fCropW+fCropU;

						// Max UV coordinate origine is the BottomLeft corner. In NeL, it is in the TopLeft corner.
						// So, inverse the V coordinate
						uvTransformed.y=(1.f-uvTransformed.y)*fCropH+fCropV;

						// Store value
						pCorner->Uvws[uv].U=uvTransformed.x;
						pCorner->Uvws[uv].V=uvTransformed.y;
						pCorner->Uvws[uv].W=0;
					}
				}
			}

			// For other channels, fill with garbage..
			for (; uv<nNumChannelUsed; uv++)
			{
				pCorner->Uvws[uv].U=0.f;
				pCorner->Uvws[uv].V=0.f;
				pCorner->Uvws[uv].W=0.f;
			}


			// *** ************
			// *** Export Alpha
			// *** ************

			// Export alpha vertex ?
			pCorner->Color.A = 255;
			if ( (maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].AlphaVertex) &&
				(buildMesh.VertexFlags&CVertexBuffer::PrimaryColorFlag) )
			{
				// Must have vertex color in the vertex buffer
				nlassert (buildMesh.VertexFlags&CVertexBuffer::PrimaryColorFlag);

				// Get the channel
				uint channel = maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].AlphaVertexChannel;

				// Channel supported ?
				if (pMesh->mapSupport (channel))
				{
					// Get a pointer on Mappingvertex for this channel. Channel 0 is the color channel.
				 	TVFace *pMapVert=pMesh->mapFaces (channel);

					// Get the index of the mapping vertex
					DWORD nMapVert=pMapVert[face].getTVert (corner);

					// Pointer on the alpha vertex. channel is the color channel to use.
					UVVert *pColorVert=&pMesh->mapVerts(channel)[nMapVert];

					// Set the alpha
					float fR=(pColorVert->x*255.f+0.5f);
					float fG=(pColorVert->y*255.f+0.5f);
					float fB=(pColorVert->z*255.f+0.5f);
					float fA=(fR+fG+fB)/3;
					clamp (fA, 0.f, 255.f);
					pCorner->Color.A=(uint8)fA;
				}
			}

			// Modulate the alpha
			pCorner->Color.A = (uint8) ((pCorner->Color.A*opacity) / 255);

			// *** ************
			// *** Export Color
			// *** ************

			// Export colors ?
			pCorner->Color.R=255;
			pCorner->Color.G=255;
			pCorner->Color.B=255;
			if ( ( (maxBaseBuild.MaterialInfo[nMaterialID-maxBaseBuild.FirstMaterial].ColorVertex) &&
				(buildMesh.VertexFlags&CVertexBuffer::PrimaryColorFlag) ) || vpColorVertex )
			{
				// Get a pointer on Mappingvertex for this channel. Channel 0 is the color channel.
				TVFace *pMapVert=pMesh->mapFaces(0);

				// Get the index of the mapping vertex
				DWORD nMapVert=pMapVert[face].getTVert (corner);

				// Pointer on the Color vertex. Channel 0 is the color channel.
				UVVert *pColorVert=&pMesh->mapVerts(0)[nMapVert];

				// Store the color
				float fR=(pColorVert->x*255.f+0.5f);
				float fG=(pColorVert->y*255.f+0.5f);
				float fB=(pColorVert->z*255.f+0.5f);
				clamp (fR, 0.f, 255.f);
				clamp (fG, 0.f, 255.f);
				clamp (fB, 0.f, 255.f);
				pCorner->Color.R=(uint8)fR;
				pCorner->Color.G=(uint8)fG;
				pCorner->Color.B=(uint8)fB;
			}

			// Modulate the color
			if (!vpColorVertex)
			{
				uint8 alphaBackup = pCorner->Color.A;
				pCorner->Color.modulateFromColor(pCorner->Color, isLighted ? diffuse : color);
				pCorner->Color.A = alphaBackup;
			}
		}
	}

	// *** **************
	// *** Export Weights
	// *** **************

	if (skined)
	{
		// Add skinning information to the buildMesh struct
		uint error = buildSkinning (buildMesh, *nodeMap, node);

		// Error code ?
		if (error!=NoError)
		{
			std::string msg = toString("%s skin: %s", getName(node).c_str(), ErrorMessage[error]);
			MessageBox(NULL, MaxTStrFromUtf8(msg).data(), _T("NeL export"), MB_OK | MB_ICONEXCLAMATION);
		}
		else
		{
			// Active skinning
			buildMesh.VertexFlags |= CVertexBuffer::PaletteSkinFlag;
		}
	}

	// *** ***********************************************************
	// *** interface mesh for Correct normals and MRM at junction   **
	// *** ***********************************************************

	// clear for MRM info
	buildMesh.Interfaces.clear();
	buildMesh.InterfaceLinks.clear();

	// don't do it for morph target (unusefull and slow)
	if (!isMorphTarget)
	{
		// Apply normal correction if there is a mesh interface
		if (skined)
		{	
			applyInterfaceToMeshBuild(node, buildMesh, NLMISC::CMatrix::Identity, time);
		}
		else
		{
			// go from export space to local
			// and then go to world space
			Matrix3 toWorldMax = node.GetObjectTM(time);
			NLMISC::CMatrix toWorld;
			convertMatrix(toWorld, toWorldMax);		
			applyInterfaceToMeshBuild(node, buildMesh, toWorld * FromExportSpace, time);
		}
	}


	// *** ***************************
	// *** Export VertexProgram.
	// *** ***************************


	NL3D::CMaterial::TShader shader;
	// If there is one material that need a specific vp ?
	// If there is, this override any vertex program setupped there
	if (CExportNel::hasMaterialWithShaderForVP(node, time, shader))
	{
		NL3D::IMeshVertexProgram *vp = buildMeshMaterialShaderVP(shader, &buildMesh);
		// build the appropriate vp
		buildMesh.MeshVertexProgram = vp;
	}
	else  // standard case
	{
		// What Vertexprogram is used??
		int	vpId = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VERTEXPROGRAM_ID, 0);
		// Setup vertexProgram
		switch(vpId)
		{
			case 0: 
				buildMesh.MeshVertexProgram = NULL;
				break;
			case 1:
			{
				// smartPtr set it.
				buildMesh.MeshVertexProgram = new CMeshVPWindTree;
				CMeshVPWindTree &vpwt = *(CMeshVPWindTree*)(IMeshVertexProgram*)buildMesh.MeshVertexProgram;

				// Read the AppData
				CVPWindTreeAppData apd;
				getScriptAppDataVPWT(&node, apd);

				// transform it to the vpwt.
				nlassert(CVPWindTreeAppData::HrcDepth == CMeshVPWindTree::HrcDepth);
				vpwt.SpecularLighting= apd.SpecularLighting == BST_CHECKED;
				// read all levels.
				float nticks = CVPWindTreeAppData::NumTicks;
				for (uint i = 0; i < CVPWindTreeAppData::HrcDepth; i++)
				{
					float scale;
					// read frequency
					scale = apd.FreqScale;
					vpwt.Frequency[i] = float(apd.Frequency[i])/nticks * scale;
					vpwt.FrequencyWindFactor[i] = float(apd.FrequencyWindFactor[i])/nticks * scale;
					// read Distance
					scale= apd.DistScale;
					vpwt.PowerXY[i] = float(apd.DistXY[i])/nticks * scale;
					vpwt.PowerZ[i] = float(apd.DistZ[i])/nticks * scale;
					// read Bias. expand to -2,2
					vpwt.Bias[i] = float(apd.Bias[i])/nticks*4 -2;
				}

				break;
			}
			default:
				nlstop;
		}
	}	
	// Ok, done.
}

// ***************************************************************************

void CExportNel::getBSMeshBuild (std::vector<CMesh::CMeshBuild*> &bsList, INode &node, TimeValue time, bool skined)
{
	Modifier *pMorphMod = getModifier (&node, MAX_MORPHER_CLASS_ID);

	if (pMorphMod == NULL)
		return;

	uint32 i, j;

	CMatrix finalSpace = CMatrix::Identity;
		if (skined)
			convertMatrix(finalSpace, node.GetNodeTM(time));

	CMeshBase::CMeshBaseBuild *dummyMBB = NULL;		
	CUniquePtr<CMesh::CMeshBuild> baseMB(createMeshBuild (node, time, dummyMBB, finalSpace));
	delete dummyMBB;
	dummyMBB = NULL;
	if (baseMB.get() == NULL) return;

	j = 0;
	for (i = 0; i < 100; ++i)
	{
		INode *pNode = (INode*)pMorphMod->GetReference (101+i);
		if (pNode == NULL)
			continue;
		++j;
	}

	bsList.resize(j, NULL);

	j = 0;
	for (i = 0; i < 100; ++i)
	{
		INode *pNode = (INode*)pMorphMod->GetReference (101+i);
		if (pNode == NULL)
			continue;

		CBlendShape bs;
		CMeshBase::CMeshBaseBuild *pMBB = NULL;		
		// get the meshbuild of the morhp target
		bsList[j] = createMeshBuild (*pNode, time, pMBB, finalSpace, true);
		delete pMBB;
		pMBB = NULL;
		// copy src normals from src mesh for vertices that are on interfaces
		CMesh::CMeshBuild *mb = bsList[j];
		if (mb)
		{		
			if (baseMB->InterfaceVertexFlag.size() != 0)
			{		
				for(uint k = 0; k < mb->Faces.size(); ++k)
				{
					for(uint l = 0; l < 3; ++l)
					{
						uint vert = mb->Faces[k].Corner[l].Vertex;
						if (vert < baseMB->InterfaceVertexFlag.size() && baseMB->InterfaceVertexFlag[vert])
						{
							mb->Faces[k].Corner[l].Normal = baseMB->Faces[k].Corner[l].Normal;
						}
					}
				}
			}
		}
		//
		++j;
	}
}


// ***************************************************************************

void CExportNel::buildMRMParameters (Animatable& node, CMRMParameters& params)
{
	// Lods count
	params.NLods=getScriptAppData (&node, NEL3D_APPDATA_LOD_NB_LOD, NEL3D_APPDATA_LOD_NB_LOD_DEFAULT);
	
	// Divisor
	params.Divisor=getScriptAppData (&node, NEL3D_APPDATA_LOD_DIVISOR, NEL3D_APPDATA_LOD_DIVISOR_DEFAULT);

	// Get skin reduction parmaters
	switch (getScriptAppData (&node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT))
	{
	case 0:
		params.SkinReduction=CMRMParameters::SkinReductionMin;
		break;
	case 1:
		params.SkinReduction=CMRMParameters::SkinReductionMax;
		break;
	case 2:
		params.SkinReduction=CMRMParameters::SkinReductionBest;
		break;
	default:
		nlassert (0);	// no!
	}

	// Distance finest
	params.DistanceFinest=getScriptAppData (&node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT);

	// Distance middle
	params.DistanceMiddle=getScriptAppData (&node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT);

	// Distance coarsest
	params.DistanceCoarsest=getScriptAppData (&node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT);
}

// ***************************************************************************

IMeshGeom *CExportNel::buildMeshGeom (INode& node, TimeValue time, const TInodePtrInt *nodeMap, 
								CMeshBase::CMeshBaseBuild &buildBaseMesh, 
								std::vector<std::string>& listMaterialName,
								bool& isTransparent, bool& isOpaque, const CMatrix& parentMatrix)
{
	DWORD t = timeGetTime();

	// Here, we must check what kind of node we can build with this mesh.
	// For the time, just Triobj is supported.
	IMeshGeom *meshGeom=NULL;

	// Skinning at this lod ?
	if (!isSkin (node))
		nodeMap=NULL;

	// If skinning, disable skin modifier
	if (nodeMap)
		enableSkinModifier (node, false);

	// Get a pointer on the object's node
	ObjectState os = node.EvalWorldState(time);
    Object *obj = os.obj;

	// Check if there is an object
	if (obj)
	{
		// Object can be converted in triObject ?
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject *) obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));
			
			if (tri)
			{
				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				bool deleteIt = (obj != tri);

				// Coarse mesh ?
				bool coarseMesh=(getScriptAppData (&node, NEL3D_APPDATA_LOD_COARSE_MESH, 0)!=0) && (!_View);

				// No skeleton shape
				if (coarseMesh)
					nodeMap=NULL;

				// Array of name for the material
				CMaxMeshBaseBuild maxBaseBuild;

				// Append material to the base
				buildBaseMeshInterface (buildBaseMesh, maxBaseBuild, node, time, parentMatrix);

				// Get the node matrix
				Matrix3 nodeMatrixMax;
				CMatrix nodeMatrix;
				getLocalMatrix (nodeMatrixMax, node, time);
				convertMatrix (nodeMatrix, nodeMatrixMax);

				// Get the node to parent matrix
				CMatrix nodeToParentMatrix;
				nodeToParentMatrix = parentMatrix.inverted () * nodeMatrix;

				// Fill the build interface of CMesh
				CMesh::CMeshBuild buildMesh;
				buildMeshInterface (*tri, buildMesh, buildBaseMesh, maxBaseBuild, node, time, nodeMap, nodeToParentMatrix);

				// Append material names
				isTransparent=false;
				isOpaque=false;
				for (uint i=0; i<maxBaseBuild.MaterialInfo.size(); i++)
				{
					// Is opaque, transparent ?
					if( buildBaseMesh.Materials[i+maxBaseBuild.FirstMaterial].getBlend() )
						isTransparent=true;
					else
						isOpaque=true;

					// Push the name
					listMaterialName.push_back (maxBaseBuild.MaterialInfo[i].MaterialName);
				}

				if( hasLightMap( node, time ) && _Options.bExportLighting )
					calculateLM(&buildMesh, &buildBaseMesh, node, time, maxBaseBuild.FirstMaterial, _Options.OutputLightmapLog);

				// MRM mesh ?
				if (getScriptAppData (&node, NEL3D_APPDATA_LOD_MRM, 0) && (!coarseMesh) )
				{
					// Build a MRM parameters block
					CMRMParameters	parameters;
					buildMRMParameters (node, parameters);

					// Make a CMesh object
					CMeshMRMGeom* meshMRMGeom=new CMeshMRMGeom;

					// Get the blend shapes but in mesh build form
					std::vector<CMesh::CMeshBuild*> bsList;
					getBSMeshBuild (bsList, node, time, nodeMap!=NULL);

					// Build the mesh with the build interface
					meshMRMGeom->build (buildMesh, bsList, buildBaseMesh.Materials.size(), parameters);

					// Return this pointer
					meshGeom=meshMRMGeom;

	#ifdef NL_DONT_FIND_MAX_CRASH
					for (uint32 bsListIt = 0; bsListIt < bsList.size(); ++bsListIt)
					{
						delete bsList[bsListIt];
						bsList[bsListIt] = NULL;
					}
	#endif
				}
				else
				{
					// Make a CMesh object
					CMeshGeom* mGeom=new CMeshGeom;

					// Coarse mesh ?
					if (coarseMesh)
					{
						// Force vertex format
						buildMesh.VertexFlags=NL3D_COARSEMESH_VERTEX_FORMAT_EXPORT;
					}

					// Build the mesh with the build interface
					mGeom->build(buildMesh, buildBaseMesh.Materials.size());

					// Return this pointer
					meshGeom=mGeom;
				}

				// Delete the triObject if we should...
				if (deleteIt)
					tri->DeleteThis();
				tri = NULL;
			}
		}
	}

	// If skinning, renable skin modifier
	if (nodeMap)
		enableSkinModifier (node, true);

	if (InfoLog)
		InfoLog->display("buildMeshGeom : %d ms\n", timeGetTime()-t);
	if (InfoLog)
		InfoLog->display("End of %s \n", node.GetName());

	// Return the shape pointer or NULL if an error occurred.
	return meshGeom;
}

// ***************************************************************************
void CExportNel::buildMeshMorph (CMesh::CMeshBuild& buildMesh, INode &node, TimeValue time, bool skined)
{
	
	Modifier *pMorphMod = getModifier (&node, MAX_MORPHER_CLASS_ID);

	if (pMorphMod == NULL)
		return;

	uint32 i, j;

	uint32 nNbVertVB = 0;

	bool wantTangentSpace = buildMesh.MeshVertexProgram == NULL ? false
																: buildMesh.MeshVertexProgram->needTangentSpace();

	uint tangentSpaceTexCoord = 0;
	if (wantTangentSpace)
	{
		for (uint k = 0; k < CVertexBuffer::MaxStage; ++k)
		{
			if (buildMesh.VertexFlags & (CVertexBuffer::TexCoord0Flag << k)) tangentSpaceTexCoord = k;
		}
		nlassert(tangentSpaceTexCoord != 0);
		nlassert(buildMesh.NumCoords[tangentSpaceTexCoord] == 3);
	}

	for (i = 0; i < buildMesh.VertLink.size(); ++i)
		if (buildMesh.VertLink[i].VertVB > nNbVertVB)
			nNbVertVB = buildMesh.VertLink[i].VertVB;
	++nNbVertVB; // Because we have the highest index to transform to a number

	// Orignal number of polygons
	uint polyCount = buildMesh.Faces.size();
	uint vertexCount = buildMesh.Vertices.size();

	for (i = 0; i < 100; ++i)
	{
		INode *pNode = (INode*)pMorphMod->GetReference (101+i);
		if (pNode == NULL)
			continue;

		CBlendShape bs;
		CMeshBase::CMeshBaseBuild *pMBB;
		CMatrix finalSpace = CMatrix::Identity;
		if (skined)
			convertMatrix(finalSpace, node.GetNodeTM(time));
		// get the meshbuild of the morhp target
		CMesh::CMeshBuild *pMB = createMeshBuild (*pNode, time, pMBB, finalSpace, true);

		// Same number of faces and vertices ?
		if (vertexCount != pMB->Vertices.size())
		{
			char message[512];
			smprintf (message, 512, "The morph target \"%s\" in slot %d has not the same vertex count than the original object \"%s\".", pNode->GetName(), i+1, node.GetName());
			outputErrorMessage (message);
			continue;
		}

		// Same number of faces and vertices ?
		if (polyCount != pMB->Faces.size())
		{
			char message[512];
			smprintf (message, 512, "The morph target \"%s\" in slot %d has not the same polygon count than the original object \"%s\".", pNode->GetName(), i+1, node.GetName());
			outputErrorMessage (message);
			continue;
		}

		bs.Name = MCharStrToUtf8(pNode->GetName());

		bool bIsDeltaPos = false;
		bs.deltaPos.resize (nNbVertVB, CVector::Null);
		bool bIsDeltaNorm = false;
		bs.deltaNorm.resize (nNbVertVB, CVector::Null);
		bool bIsDeltaUV = false;
		bs.deltaUV.resize (nNbVertVB, CUV(0.0f,0.0f));
		bool bIsDeltaCol = false;
		bs.deltaCol.resize (nNbVertVB, CRGBAF(0.0f,0.0f,0.0f,0.0f));
		bool bIsDeltaTgSpace = false;
		if (wantTangentSpace)
		{
			bs.deltaTgSpace.resize (nNbVertVB, CVector::Null);
		}

		bs.VertRefs.resize (nNbVertVB, 0xffffffff);

		for (j = 0; j < buildMesh.VertLink.size(); ++j)
		{
			uint32 nFace = buildMesh.VertLink[j].nFace;
			uint32 nCorner = buildMesh.VertLink[j].nCorner;
			uint32 VertRef = buildMesh.Faces[nFace].Corner[nCorner].Vertex;
			uint32 VertTar = pMB->Faces[nFace].Corner[nCorner].Vertex;
			uint32 iVB = buildMesh.VertLink[j].VertVB;

			CVector delta = pMB->Vertices[VertTar] - buildMesh.Vertices[VertRef];
			if (delta.norm() > 0.001f)
			{
				bs.deltaPos[iVB] = delta;
				bs.VertRefs[iVB] = iVB;
				bIsDeltaPos = true;
			}

			// check for normal change only if the vertex is not on an interface
			if (buildMesh.InterfaceVertexFlag.size() == 0 || !buildMesh.InterfaceVertexFlag[VertRef])
			{	
				CVector NormRef = buildMesh.Faces[nFace].Corner[nCorner].Normal;
				CVector NormTar = pMB->Faces[nFace].Corner[nCorner].Normal;
				delta = NormTar - NormRef;
				if (delta.norm() > 0.001f)
				{
					bs.deltaNorm[iVB] = delta;
					bs.VertRefs[iVB] = iVB;
					bIsDeltaNorm = true;
				}
			}

			if (wantTangentSpace)
			{
				CUVW TgSpaceRef = buildMesh.Faces[nFace].Corner[nCorner].Uvws[tangentSpaceTexCoord];
				CUVW TgSpaceTar = pMB->Faces[nFace].Corner[nCorner].Uvws[tangentSpaceTexCoord];
				CUVW deltaTS = TgSpaceTar - TgSpaceRef;
				float normDeltaTS = ::sqrtf(deltaTS.U * deltaTS.U + deltaTS.V * deltaTS.V + deltaTS.W * deltaTS.W);
				if (normDeltaTS > 0.001f)
				{
					bs.deltaTgSpace[iVB].set(deltaTS.U, deltaTS.V, deltaTS.W);
					bs.VertRefs[iVB] = iVB;
					bIsDeltaTgSpace = true;
				}	
			}

			CUV UVRef = (CUV) buildMesh.Faces[nFace].Corner[nCorner].Uvws[0];
			CUV UVTar = (CUV) pMB->Faces[nFace].Corner[nCorner].Uvws[0];
			CUV deltaUV = UVTar - UVRef;
			if ((deltaUV.U*deltaUV.U + deltaUV.V*deltaUV.V) > 0.0001f)
			{
				bs.deltaUV[iVB] = deltaUV;
				bs.VertRefs[iVB] = iVB;
				bIsDeltaUV = true;
			}
			
			CRGBAF RGBARef = buildMesh.Faces[nFace].Corner[nCorner].Color;
			CRGBAF RGBATar = pMB->Faces[nFace].Corner[nCorner].Color;
			CRGBAF deltaRGBA = RGBATar - RGBARef;
			if ((deltaRGBA.R*deltaRGBA.R + deltaRGBA.G*deltaRGBA.G +
				deltaRGBA.B*deltaRGBA.B + deltaRGBA.A*deltaRGBA.A) > 0.0001f)
			{
				bs.deltaCol[iVB] = deltaRGBA;
				bs.VertRefs[iVB] = iVB;
				bIsDeltaCol = true;
			}
		}

		// Delete unused items
		sint32 nNbVertUsed = nNbVertVB;
		sint32 nDstPos = 0;
		for (j = 0; j < nNbVertVB; ++j)
		{
			if (bs.VertRefs[j] == 0xffffffff) // Is vertex UNused
			{
				--nNbVertUsed;
			}
			else // Vertex used
			{
				if (nDstPos != (sint32) j)
				{
					bs.VertRefs[nDstPos]	= bs.VertRefs[j];
					bs.deltaPos[nDstPos]	= bs.deltaPos[j];
					bs.deltaNorm[nDstPos]	= bs.deltaNorm[j];
					bs.deltaUV[nDstPos]		= bs.deltaUV[j];
					bs.deltaCol[nDstPos]	= bs.deltaCol[j];
					if (wantTangentSpace) 
						bs.deltaTgSpace[nDstPos] = bs.deltaTgSpace[j];
				}
				++nDstPos;
			}
		}

		if (bIsDeltaPos)
			bs.deltaPos.resize (nNbVertUsed);
		else
			bs.deltaPos.resize (0);

		if (bIsDeltaNorm)
			bs.deltaNorm.resize (nNbVertUsed);
		else
			bs.deltaNorm.resize (0);

		if (bIsDeltaUV)
			bs.deltaUV.resize (nNbVertUsed);
		else
			bs.deltaUV.resize (0);

		if (bIsDeltaCol)
			bs.deltaCol.resize (nNbVertUsed);
		else
			bs.deltaCol.resize (0);

		if (bIsDeltaTgSpace)
			bs.deltaTgSpace.resize (nNbVertUsed);
		else
			bs.deltaTgSpace.resize (0);

		bs.VertRefs.resize (nNbVertUsed);

		// Add the new blend shape
		buildMesh.BlendShapes.push_back (bs);
	}

}


// ***************************************************************************
NL3D::IShape				*CExportNel::buildWaveMakerShape(INode& node, TimeValue time)
{
	NL3D::CWaveMakerShape *wms = new CWaveMakerShape;
	float  radius = 3;
	float  intensity = 1;
	float  period = 1;
	uint32 poolID = 0;
	int	   impulsionMode; 
	CExportNel::getValueByNameUsingParamBlock2 (node, "period", (ParamType2)TYPE_FLOAT, &period, time);
	CExportNel::getValueByNameUsingParamBlock2 (node, "intensity", (ParamType2)TYPE_FLOAT, &intensity, time);
	CExportNel::getValueByNameUsingParamBlock2 (node, "radius", (ParamType2)TYPE_FLOAT, &radius, time);
	CExportNel::getValueByNameUsingParamBlock2 (node, "poolID", (ParamType2)TYPE_FLOAT, &poolID, time);
	CExportNel::getValueByNameUsingParamBlock2 (node, "impulsionMode", (ParamType2)TYPE_BOOL, &impulsionMode, time);

	wms->setPeriod(period);
	wms->setIntensity(intensity);
	wms->setRadius(3);
	wms->setWaterPoolID(poolID);
	wms->setImpulsionMode(impulsionMode != 0);

	// Get the node matrix
	Matrix3 localTM;
	getLocalMatrix (localTM, node, time);
	Point3 pos = localTM.GetRow(3);
	NLMISC::CVector nelPos;
	convertVector(nelPos, pos);	
	wms->getDefaultPos()->setDefaultValue(nelPos);

	return wms;
}


// ***************************************************************************
NL3D::IShape				*CExportNel::buildWaterShape(INode& node, TimeValue time)
{
	// must have a water material
	nlassert(hasWaterMaterial(node, time));


	// Get a pointer on the object's node
	ObjectState os = node.EvalWorldState(time);
    Object *obj = os.obj;

	if (!obj) return NULL;

	// Get a triobject from the node
	TriObject *tri = (TriObject *) obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));

	if (!tri) return NULL;

	// Note that the TriObject should only be deleted
	// if the pointer to it is not equal to the object
	// pointer that called ConvertToType()
	bool deleteIt=false;
	if (obj != tri) 
		deleteIt = true;


	// Get a pointer on the 3dsmax mesh
	Mesh *pMesh = &tri->mesh;

	// take all vertices, and build their convex hull to get correct ordering (though we only support convex shapes for now)
	CPolygon   dest;	

	// compute export matrix
	// Get the invert node matrix
	Matrix3 invNodeTM=node.GetNodeTM(time);
	invNodeTM.Invert();

	// Get the object matrix
	Matrix3 objectTM=node.GetObjectTM(time);

	// Compute the local to world matrix
	Matrix3 objectToLocal = objectTM  * invNodeTM;

	CMatrix ToExportSpace;
	// Invert matrix in NeL format
	convertMatrix (ToExportSpace, objectToLocal);


	// Number of vertices
	int numVerts = pMesh->getNumVerts();
	dest.Vertices.reserve(numVerts);	
	for (int vertex=0; vertex < numVerts; ++vertex)
	{
		// Transform the vertex in local coordinate
		Point3 v = pMesh->getVert(vertex);
		CVector vv = ToExportSpace * CVector(v.x, v.y, v.z);
		dest.Vertices.push_back(vv);
	}



	static const float proj[] = { 1, 0, 0, 0,
								  0, 1, 0, 0,
								  0, 0, 0, 0,
								  0, 0, 0, 0 };
	CMatrix projMat;
	projMat.set(proj);
	CPolygon2D projDest(dest, projMat); // project the poly
	CPolygon2D convexPoly;
	projDest.buildConvexHull(convexPoly);

	uint CHNumVerts = convexPoly.Vertices.size();

	CWaterShape *ws = new CWaterShape;

	// get water material

	// Get primary material pointer of the node
	Mtl* pNodeMat=node.GetMtl();
	if ((pNodeMat!=NULL) && isClassIdCompatible (*pNodeMat, Class_ID(NEL_MTL_A,NEL_MTL_B)))
	{
		nlassert(pNodeMat != NULL);
		nlassert(pNodeMat->NumSubMtls() == 0); // no submaterial allowed

		// we use displacement, bump, and reflection map

		// look for available maps

		// Look for enable map
		int mapEnable1;
		int mapEnable2;
		int mapEnable3;
		int mapEnable4;
		int mapEnable5;
		int mapEnable6;
		int mapEnable7;
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bEnableSlot_1", (ParamType2)TYPE_BOOL, &mapEnable1, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bEnableSlot_2", (ParamType2)TYPE_BOOL, &mapEnable2, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bEnableSlot_3", (ParamType2)TYPE_BOOL, &mapEnable3, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bEnableSlot_4", (ParamType2)TYPE_BOOL, &mapEnable4, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bEnableSlot_5", (ParamType2)TYPE_BOOL, &mapEnable5, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bEnableSlot_6", (ParamType2)TYPE_BOOL, &mapEnable6, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bEnableSlot_7", (ParamType2)TYPE_BOOL, &mapEnable7, time);

		if (!(mapEnable1!=0) || !(mapEnable5!=0) || !(mapEnable6!=0))
		{
			nlinfo("ERROR : BuildWaterShape : didn't found all required map when exporting water node %s , see material.txt for help", node.GetName());
			// need these maps to do the job
			return NULL;
		}

		/// Build a texture from each map
		Texmap *maxDisplaceMap			= NULL;
		Texmap *maxBumpMap				= NULL;
		Texmap *maxEnvMap				= NULL;
		Texmap *maxDiffuseMap			= NULL;
		Texmap *maxEnvMap2				= NULL;
		Texmap *maxEnvMapUnderWater		= NULL;
		Texmap *maxEnvMapUnderWater2	= NULL;
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "tTexture_1", (ParamType2)TYPE_TEXMAP, &maxEnvMap, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "tTexture_2", (ParamType2)TYPE_TEXMAP, &maxEnvMap2, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "tTexture_3", (ParamType2)TYPE_TEXMAP, &maxEnvMapUnderWater, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "tTexture_4", (ParamType2)TYPE_TEXMAP, &maxEnvMapUnderWater2, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "tTexture_5", (ParamType2)TYPE_TEXMAP, &maxBumpMap, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "tTexture_6", (ParamType2)TYPE_TEXMAP, &maxDisplaceMap, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "tTexture_7", (ParamType2)TYPE_TEXMAP, &maxDiffuseMap, time);

		if (!maxDisplaceMap || !isClassIdCompatible(*maxDisplaceMap, Class_ID (BMTEX_CLASS_ID,0))) 
		{
			nlinfo("ERROR : BuildWaterShape : displace map is not a valid bitmap (when exporting water node : %s)", node.GetName());
			return NULL;
		}
		if (!maxBumpMap || !isClassIdCompatible(*maxBumpMap, Class_ID (BMTEX_CLASS_ID,0)))
		{
			nlinfo("ERROR : BuildWaterShape : bump map is not a valid bitmap (when exporting water node : %s)", node.GetName());
			return NULL;
		}
		if (!maxEnvMap || !isClassIdCompatible(*maxEnvMap, Class_ID (BMTEX_CLASS_ID,0)))
		{
			nlinfo("ERROR : BuildWaterShape : env map is not a valid bitmap (when exporting water node : %s)", node.GetName());
			return NULL;
		}

		if (mapEnable7 && maxDiffuseMap)
		{
			if (!isClassIdCompatible(*maxDiffuseMap, Class_ID (BMTEX_CLASS_ID,0))) 
			{
				nlinfo("ERROR : BuildWaterShape : diffuse map is not a valid bitmap (when exporting water node : %s)", node.GetName());
				return NULL;
			}
		}

		if (mapEnable2 && maxEnvMap2)
		{
			if (!isClassIdCompatible(*maxEnvMap2, Class_ID (BMTEX_CLASS_ID,0))) 
			{
				nlinfo("ERROR : BuildWaterShape : refraction map is not a valid bitmap (when exporting water node : %s)", node.GetName());
				return NULL;
			}
		}
		

		if (mapEnable3 && maxEnvMapUnderWater)
		{
			if (!isClassIdCompatible(*maxEnvMapUnderWater, Class_ID (BMTEX_CLASS_ID,0))) 
			{
				nlinfo("ERROR : BuildWaterShape : spcular map is not a valid bitmap (when exporting water node : %s)", node.GetName());
				return NULL;
			}
		}

		if (mapEnable4 && maxEnvMapUnderWater2)
		{
			if (!isClassIdCompatible(*maxEnvMapUnderWater2, Class_ID (BMTEX_CLASS_ID,0))) 
			{
				nlinfo("ERROR : BuildWaterShape : opacity map is not a valid bitmap (when exporting water node : %s)", node.GetName());
				return NULL;
			}
		}

		// List of channels used by this texture (we don't use it though)
		CMaterialDesc _3dsTexChannel;
					
		NLMISC::CSmartPtr<ITexture> bumpMap		 = buildATexture (*maxBumpMap, _3dsTexChannel, time);
		NLMISC::CSmartPtr<ITexture> displaceMap  = buildATexture (*maxDisplaceMap, _3dsTexChannel, time);
		if (bumpMap->supportSharing() && displaceMap->supportSharing())
		{
			if (bumpMap->getShareName() == displaceMap->getShareName())
			{
				nlinfo("Water shape : bump map and displacement map should not be the same");
				return NULL;
			}
		}

		NLMISC::CSmartPtr<ITexture> colorMap = NULL;
		if (maxDiffuseMap)
		{
			colorMap = buildATexture (*maxDiffuseMap, _3dsTexChannel, time);
		}
		NLMISC::CSmartPtr<ITexture> envMap				= NULL;
		NLMISC::CSmartPtr<ITexture> envMapUnderWater	= NULL;




		if (maxEnvMap)
		{
			if (!maxEnvMap2)
			{
				envMap = buildATexture (*maxEnvMap, _3dsTexChannel, time);
			}
			else
			{
				NLMISC::CSmartPtr<ITexture> tex0, tex1;
				tex0	 = buildATexture (*maxEnvMap, _3dsTexChannel, time);
				tex1	 = buildATexture (*maxEnvMap2, _3dsTexChannel, time);
				envMap = new CTextureBlend;
				(static_cast<CTextureBlend *>((ITexture *) envMap))->setBlendTexture(0, tex0);
				(static_cast<CTextureBlend *>((ITexture *) envMap))->setBlendTexture(1, tex1);
			}
		}

		if (maxEnvMapUnderWater)
		{
			if (!maxEnvMapUnderWater2)
			{
				envMapUnderWater = buildATexture (*maxEnvMapUnderWater, _3dsTexChannel, time);
			}
			else
			{
				NLMISC::CSmartPtr<ITexture> tex0, tex1;
				tex0	 = buildATexture (*maxEnvMapUnderWater, _3dsTexChannel, time);
				tex1	 = buildATexture (*maxEnvMapUnderWater2, _3dsTexChannel, time);
				envMapUnderWater = new CTextureBlend;
				(static_cast<CTextureBlend *>((ITexture *) envMapUnderWater))->setBlendTexture(0, tex0);
				(static_cast<CTextureBlend *>((ITexture *) envMapUnderWater))->setBlendTexture(1, tex1);
			}
		}


		nlinfo("buildWaterShape : Texture have been built");

		ws->setEnvMap(0, (ITexture *) envMap);
		ws->setEnvMap(1, (ITexture *) envMapUnderWater);

		ws->setHeightMap(0, (ITexture *) displaceMap);
		ws->setHeightMap(1, (ITexture *) bumpMap);

		/// rertrieve bump maps scale and speed

		NLMISC::CVector2f bumpMapScale;
		NLMISC::CVector2f bumpMapSpeed;
		NLMISC::CVector2f displaceMapScale;
		NLMISC::CVector2f displaceMapSpeed;

		CExportNel::getValueByNameUsingParamBlock2(node, "fBumpUScale", (ParamType2)TYPE_FLOAT, &bumpMapScale.x, 0);
		CExportNel::getValueByNameUsingParamBlock2(node, "fBumpVScale", (ParamType2)TYPE_FLOAT, &bumpMapScale.y, 0);
		CExportNel::getValueByNameUsingParamBlock2(node, "fBumpUSpeed", (ParamType2)TYPE_FLOAT, &bumpMapSpeed.x, 0);
		CExportNel::getValueByNameUsingParamBlock2(node, "fBumpVSpeed", (ParamType2)TYPE_FLOAT, &bumpMapSpeed.y, 0);

		CExportNel::getValueByNameUsingParamBlock2(node, "fDisplaceMapUScale", (ParamType2)TYPE_FLOAT, &displaceMapScale.x, 0);
		CExportNel::getValueByNameUsingParamBlock2(node, "fDisplaceMapVScale", (ParamType2)TYPE_FLOAT, &displaceMapScale.y, 0);
		CExportNel::getValueByNameUsingParamBlock2(node, "fDisplaceMapUSpeed", (ParamType2)TYPE_FLOAT, &displaceMapSpeed.x, 0);
		CExportNel::getValueByNameUsingParamBlock2(node, "fDisplaceMapVSpeed", (ParamType2)TYPE_FLOAT, &displaceMapSpeed.y, 0);



		ws->setHeightMapScale(0, displaceMapScale);
		ws->setHeightMapScale(1, bumpMapScale);

		ws->setHeightMapSpeed(0, displaceMapSpeed);
		ws->setHeightMapSpeed(1, bumpMapSpeed);

		// scene envmap usage
		int waterUseSceneEnvMapAbove = 0;
		int waterUseSceneEnvMapUnder = 0;
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bWaterUseSceneEnvMapAbove", (ParamType2)TYPE_BOOL, &waterUseSceneEnvMapAbove, time);
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bWaterUseSceneEnvMapUnder", (ParamType2)TYPE_BOOL, &waterUseSceneEnvMapUnder, time);
		ws->setUseSceneWaterEnvMap(0, waterUseSceneEnvMapAbove != 0);
		ws->setUseSceneWaterEnvMap(1, waterUseSceneEnvMapUnder != 0);

		if (colorMap != NULL)
		{
			if (pMesh->getNumTVerts() == 0) // no mapping
			{
				nlinfo("ERROR : BuildWaterShape : diffuse map found, but no mapping has been applied (when exporting water node : %s)", node.GetName());
				return NULL;
			}
			ws->setColorMap((ITexture *) colorMap);
			uint i0, i1, i2;
			projDest.getBestTriplet(i0, i1, i2);
			//nlinfo("i0 = %d, i1 =  %d, i2 = %d", i0, i1, i2);			

			NLMISC::CVector v0, v1, v2;
			CExportNel::convertVector(v0, objectToLocal * pMesh->getVert(i0));
			CExportNel::convertVector(v1, objectToLocal * pMesh->getVert(i1));
			CExportNel::convertVector(v2, objectToLocal * pMesh->getVert(i2));

			// get texture matrix
			Matrix3 texMat;
			texMat.IdentityMatrix();			
			maxDiffuseMap->GetUVTransform(texMat);
			
			NLMISC::CMatrix A, B, C;
			A.setRot(NLMISC::CVector(v0.x, v0.y, 1), NLMISC::CVector(v1.x, v1.y, 1), NLMISC::CVector(v2.x, v2.y, 1));			
			Point3 uv[3] = { pMesh->getTVert(i0) * texMat, pMesh->getTVert(i1) * texMat, pMesh->getTVert(i2) * texMat };

			float cropU = 0.f, cropV = 0.f, cropW = 1.f, cropH = 1.f;
			// crop result	
			#define BMTEX_CROP_APPLY "apply"
			#define BMTEX_CROP_U_NAME "clipu"
			#define BMTEX_CROP_V_NAME "clipv"
			#define BMTEX_CROP_W_NAME "clipw"
			#define BMTEX_CROP_H_NAME "cliph"
			int bApply;
			bool bRes=getValueByNameUsingParamBlock2 (*maxDiffuseMap, BMTEX_CROP_APPLY, (ParamType2)TYPE_BOOL, &bApply, time);
			nlassert (bRes);
			if (bApply)
			{
				bool bRes;
				bRes=getValueByNameUsingParamBlock2 (*maxDiffuseMap, BMTEX_CROP_U_NAME, (ParamType2)TYPE_FLOAT, &cropU, time);
				nlassert (bRes);			
				bRes=getValueByNameUsingParamBlock2 (*maxDiffuseMap, BMTEX_CROP_V_NAME, (ParamType2)TYPE_FLOAT, &cropV, time);
				nlassert (bRes);			
				bRes=getValueByNameUsingParamBlock2 (*maxDiffuseMap, BMTEX_CROP_W_NAME, (ParamType2)TYPE_FLOAT, &cropW, time);
				nlassert (bRes);			
				bRes=getValueByNameUsingParamBlock2 (*maxDiffuseMap, BMTEX_CROP_H_NAME, (ParamType2)TYPE_FLOAT, &cropH, time);
				nlassert (bRes);
			}

			C.setRot(NLMISC::CVector(uv[0].x * cropW + cropU, (1.f - uv[0].y) * cropH + cropV, 1),
					 NLMISC::CVector(uv[1].x * cropW + cropU, (1.f - uv[1].y) * cropH + cropV, 1),
					 NLMISC::CVector(uv[2].x * cropW + cropU, (1.f - uv[2].y) * cropH + cropV, 1)
					);			
			B = C * A.inverted();			
			// TMP TMP TMP TMP
			CVector r0 = B * CVector(v0.x, v0.y, 1.f);
			CVector r1 = B * CVector(v1.x, v1.y, 1.f);
			CVector r2 = B * CVector(v2.x, v2.y, 1.f);			

			ws->setColorMapMat(NLMISC::CVector2f(B.getI().x, B.getI().y),
							   NLMISC::CVector2f(B.getJ().x, B.getJ().y),
							   NLMISC::CVector2f(B.getK().x, B.getK().y));



		}

		

		
		ws->setShape(convexPoly);

		/// water height factor
		float waterHeightFactor = 1.f;
		CExportNel::getValueByNameUsingParamBlock2(node, "fWaterHeightFactor", (ParamType2)TYPE_FLOAT, &waterHeightFactor, 0);
		ws->setWaveHeightFactor(waterHeightFactor);
		
		// pool id
		uint32 poolID;
		CExportNel::getValueByNameUsingParamBlock2(node, "iWaterPoolID", (ParamType2)TYPE_INT, &poolID, 0);
		ws->setWaterPoolID(poolID);


		// Export default transformation

		// Get the node matrix
		Matrix3 localTM;
		getLocalMatrix (localTM, node, time);

		// Get the translation, rotation, scale of the node
		CVector pos, scale;
		CQuat rot;
		decompMatrix (scale, rot, pos, localTM);

		// Set the default values
		ws->getDefaultPos()->setDefaultValue(pos);					
		ws->getDefaultScale()->setDefaultValue(scale);
		ws->getDefaultRotQuat()->setDefaultValue(rot);


		// See if splash enabled
		int splashEnabled = 1;
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bEnableWaterSplash", (ParamType2)TYPE_BOOL, &splashEnabled, time);
		ws->enableSplash(splashEnabled != 0);


		// Delete the triObject if we should...
		if (deleteIt)
			tri->DeleteThis();
		tri = NULL;
		nlinfo("WaterShape : build succesful");
		return ws;
	}
	else
	{
		nlinfo("ERROR : BuildWaterShape : must have a NeL material : %s)", node.GetName());
		return NULL;
	}
}

// ***************************************************************************
bool CExportNel::buildMeshAABBox(INode &node, NLMISC::CAABBox &dest, TimeValue time)
{
	ObjectState os = node.EvalWorldState(time);
    Object *obj = os.obj;		
	if (!obj) return false;
	if (!obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))  return false;
	 // Get a triobject from the node
	TriObject *tri = (TriObject*)obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));
	if (!tri) return false;
	// Note that the TriObject should only be deleted
	// if the pointer to it is not equal to the object
	// pointer that called ConvertToType()
	bool deleteIt = false;
	if (obj != tri) 
		deleteIt = true;
	Mesh &mesh = tri->GetMesh();
	//
	Matrix3 toWorld = node.GetObjectTM(time);
	mesh.buildBoundingBox();
	Box3 bbox  = mesh.getBoundingBox(&toWorld);
	Point3 maxMin = bbox.Min();
	Point3 maxMax = bbox.Max();
	CVector nelMin, nelMax;
	convertVector(nelMin, maxMin);
	convertVector(nelMax, maxMax);
	//
	dest.setMinMax(nelMin, nelMax);
	//
	if (deleteIt)
		tri->DeleteThis();

	tri = NULL;
	return true;
}


