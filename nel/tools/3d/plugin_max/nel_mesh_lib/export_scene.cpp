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

#include "../nel_export/std_afx.h"
#include "../nel_export/nel_export.h"
#include "../nel_export/nel_export_scene.h"
#include "export_appdata.h"
#include "../nel_patch_lib/rpo.h"
#include "../../ig_lighter_lib/ig_lighter_lib.h"

#include "nel/misc/path.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/scene.h"
#include "nel/3d/shape_bank.h"
#include "nel/3d/zone_symmetrisation.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/particle_system.h"


#include <vector>

using namespace NLMISC;
using namespace NL3D;
using namespace std;

// ***************************************************************************

#include "calc_lm.h" // For convertToWorldCoordinate

extern CVector vGlobalPos;

// ***************************************************************************

CInstanceGroup*	CExportNel::buildInstanceGroup(const vector<INode*>& vectNode, vector<INode*>& resultInstanceNode, TimeValue tvTime)
{
	// Extract from the node the name, the transformations and the parent

	CInstanceGroup::TInstanceArray aIGArray;
	uint32 i, nNumIG;
	uint32 j,k,m;

	aIGArray.empty ();
	resultInstanceNode.empty ();
	aIGArray.resize (vectNode.size());
	resultInstanceNode.resize (vectNode.size());

	int nNbInstance = 0;
	for (i = 0; i < vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 0) // If not an accelerator
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh (*pNode, tvTime) || CExportNel::isDummy(*pNode, tvTime))
		{
			++nNbInstance;
		}
	}

	// Check integrity of the hierarchy and set the parents
	std::vector<INode*>::const_iterator it = vectNode.begin();
	nNumIG = 0;
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 0) // If not an accelerator
		if (!RPO::isZone( *pNode, tvTime ))
		if (CExportNel::isMesh( *pNode, tvTime ) || CExportNel::isDummy(*pNode, tvTime))
		{
			aIGArray[nNumIG].DontAddToScene = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_DONT_ADD_TO_SCENE, 0)?true:false;
			aIGArray[nNumIG].InstanceName = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_INSTANCE_NAME, "");
			resultInstanceNode[nNumIG] = pNode;
			if (aIGArray[nNumIG].InstanceName == "") // no instance name was set, takes the node name instead
			{
				aIGArray[nNumIG].InstanceName = MCharStrToUtf8(pNode->GetName());
			}

			// Visible? always true, but if special flag for camera collision
			sint	appDataCameraCol= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION, 0);
			aIGArray[nNumIG].Visible= appDataCameraCol!=3;


			INode *pParent = pNode->GetParentNode();

			// Set the DontCastShadow flag.
			aIGArray[nNumIG].DontCastShadow= pNode->CastShadows()==0;

			// Set the Special DontCastShadow flag.
			aIGArray[nNumIG].DontCastShadowForInterior= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_INTERIOR, BST_UNCHECKED)?true:false;
			aIGArray[nNumIG].DontCastShadowForExterior= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_EXTERIOR, BST_UNCHECKED)?true:false;

			// Is the pNode has the root node for parent ?
			if( pParent->IsRootNode() == 0 )
			{
				// Look if the parent is in the selection
				int nNumIG2 = 0;
				for (j = 0; j < vectNode.size(); ++j)
				{
					INode *pNode2 = vectNode[j];

					int nAccelType2 = CExportNel::getScriptAppData (pNode2, NEL3D_APPDATA_ACCEL, 32);
					if ((nAccelType2&3) == 0) // If not an accelerator
					if (!RPO::isZone( *pNode2, tvTime ))
					if (CExportNel::isMesh( *pNode2, tvTime ))
					{
						if (pNode2 == pParent)
							break;
						++nNumIG2;
					}
				}
				if (nNumIG2 == nNbInstance)
				{
					// The parent is not selected ! link to root
					aIGArray[nNumIG].nParent = -1;
				}
				else
				{
					aIGArray[nNumIG].nParent = nNumIG2;
				}
			}
			else
			{
				aIGArray[nNumIG].nParent = -1;
			}
			++nNumIG;
		}
	}
	aIGArray.resize( nNumIG );
	resultInstanceNode.resize( nNumIG );

	// Build the array of node
	vGlobalPos = CVector(0,0,0);
	nNumIG = 0;
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 0) // If not an accelerator
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh (*pNode, tvTime) || CExportNel::isDummy(*pNode, tvTime))
		{
			CVector vScaleTemp;
			CQuat qRotTemp;
			CVector vPosTemp;

			// Get Nel Name for the object.
			aIGArray[nNumIG].Name= CExportNel::getNelObjectName(*pNode);

			//Get the local transformation matrix
			Matrix3 nodeTM = pNode->GetNodeTM(0);
			INode *pParent = pNode->GetParentNode();
			Matrix3 parentTM = pParent->GetNodeTM(0);
			Matrix3 localTM	= nodeTM*Inverse(parentTM);

			// Extract transformations
			CExportNel::decompMatrix (vScaleTemp, qRotTemp, vPosTemp, localTM);
			aIGArray[nNumIG].Rot   = qRotTemp;
			aIGArray[nNumIG].Pos   = vPosTemp;
			aIGArray[nNumIG].Scale = vScaleTemp;
			vGlobalPos += vPosTemp;
			++nNumIG;
		}
	}
	// todo Make this work (precision):
	/*
	vGlobalPos = vGlobalPos / nNumIG;
	for (i = 0; i < nNumIG; ++i)
		aIGArray[i].Pos -= vGlobalPos;
	*/

	vGlobalPos = CVector(0,0,0); // Temporary !!!

	// Accelerator Portal/Cluster part
	//=================

	// Creation of all the clusters
	vector<CCluster> vClusters;
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, NEL3D_APPDATA_ACCEL_DEFAULT);
		bool bFatherVisible = nAccelType&NEL3D_APPDATA_ACCEL_FATHER_VISIBLE?true:false;
		bool bVisibleFromFather = nAccelType&NEL3D_APPDATA_ACCEL_VISIBLE_FROM_FATHER?true:false;
		bool bAudibleLikeVisible = (nAccelType&NEL3D_APPDATA_ACCEL_AUDIBLE_NOT_LIKE_VISIBLE)?false:true;
		bool bFatherAudible = bAudibleLikeVisible ? bFatherVisible : nAccelType&NEL3D_APPDATA_ACCEL_FATHER_AUDIBLE?true:false;
		bool bAudibleFromFather = bAudibleLikeVisible ? bVisibleFromFather : nAccelType&NEL3D_APPDATA_ACCEL_AUDIBLE_FROM_FATHER?true:false;

		if ((nAccelType&NEL3D_APPDATA_ACCEL_TYPE) == NEL3D_APPDATA_ACCEL_CLUSTER) // If cluster
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh(*pNode, tvTime))
		{
			CCluster clusterTemp;
			std::string temp;

			temp = CExportNel::getScriptAppData(pNode, NEL3D_APPDATA_SOUND_GROUP, "no sound");
			clusterTemp.setSoundGroup(temp != "no sound" ? temp : "");
			temp = CExportNel::getScriptAppData(pNode, NEL3D_APPDATA_ENV_FX, "no fx");
			clusterTemp.setEnvironmentFx(temp != "no fx" ? temp : "");

			CMesh::CMeshBuild *pMB;
			CMeshBase::CMeshBaseBuild *pMBB;
			pMB = createMeshBuild (*pNode, tvTime, pMBB);

			convertToWorldCoordinate( pMB, pMBB );

			for (j = 0; j < pMB->Faces.size(); ++j)
			{
				if (!clusterTemp.makeVolume (pMB->Vertices[pMB->Faces[j].Corner[0].Vertex],
											 pMB->Vertices[pMB->Faces[j].Corner[1].Vertex],
											 pMB->Vertices[pMB->Faces[j].Corner[2].Vertex]) )
				{
					// ERROR : The volume is not convex !!!
					nlwarning("ERROR: The cluster %s is not convex.", MCharStrToUtf8(vectNode[i]->GetName()).c_str());
				}
			}

			clusterTemp.FatherVisible = bFatherVisible;
			clusterTemp.VisibleFromFather = bVisibleFromFather;
			clusterTemp.FatherAudible = bFatherAudible;
			clusterTemp.AudibleFromFather = bAudibleFromFather;
			clusterTemp.Name = MCharStrToUtf8(pNode->GetName());

			vClusters.push_back (clusterTemp);
			delete pMB; pMB = NULL;
			delete pMBB; pMBB = NULL;
		}
	}

	// Creation of all the portals
	vector<CPortal> vPortals;
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 1) // If Portal
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh(*pNode, tvTime))
		{
			CPortal portalTemp;
			std::string temp;

			temp = CExportNel::getScriptAppData(pNode, NEL3D_APPDATA_OCC_MODEL, "no occlusion");
			portalTemp.setOcclusionModel(temp != "no occlusion" ? temp : "");
			temp = CExportNel::getScriptAppData(pNode, NEL3D_APPDATA_OPEN_OCC_MODEL, "no occlusion");
			portalTemp.setOpenOcclusionModel(temp != "no occlusion" ? temp : "");

			CMesh::CMeshBuild *pMB;
			CMeshBase::CMeshBaseBuild *pMBB;
			pMB = createMeshBuild (*pNode, tvTime, pMBB);

			convertToWorldCoordinate( pMB, pMBB );

			vector<sint32> poly;
			vector<bool> facechecked;
			facechecked.resize (pMB->Faces.size());
			for (j = 0; j < pMB->Faces.size(); ++j)
				facechecked[j] = false;

			poly.push_back(pMB->Faces[0].Corner[0].Vertex);
			poly.push_back(pMB->Faces[0].Corner[1].Vertex);
			poly.push_back(pMB->Faces[0].Corner[2].Vertex);
			facechecked[0] = true;
			for (j = 0; j < pMB->Faces.size(); ++j)
			if (!facechecked[j])
			{
				bool found = false;

				for(k = 0; k < 3; ++k)
				{
					for(m = 0; m < poly.size(); ++m)
					{
						if ((pMB->Faces[j].Corner[k].Vertex == poly[m]) &&
							(pMB->Faces[j].Corner[(k+1)%3].Vertex == poly[(m+1)%poly.size()]))
						{
							found = true;
							break;
						}
						if ((pMB->Faces[j].Corner[(k+1)%3].Vertex == poly[m]) &&
							(pMB->Faces[j].Corner[k].Vertex == poly[(m+1)%poly.size()]))
						{
							found = true;
							break;
						}
					}
					if (found)
						break;
				}
				if (found)
				{
					// insert an empty space in poly between m and m+1
					poly.resize (poly.size()+1);
					for (uint32 a = poly.size()-2; a > m; --a)
						poly[a+1] = poly[a];
					poly[m+1] = pMB->Faces[j].Corner[(k+2)%3].Vertex;
					facechecked[j] = true;
					j = 0;
				}
			}
			vector<CVector> polyv;
			polyv.resize (poly.size());
			for (j = 0; j < poly.size(); ++j)
				polyv[j] = pMB->Vertices[poly[j]];
			
			if (!portalTemp.setPoly (polyv))
			{
				// ERROR : Poly not convex, or set of vertices not plane
				nlwarning("ERROR: The portal %s is not convex.", MCharStrToUtf8(vectNode[i]->GetName()).c_str());
			}

			if (nAccelType&16) // is dynamic portal ?
			{
				string InstanceName = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_INSTANCE_NAME, "");
				if (!InstanceName.empty())
					portalTemp.setName (InstanceName);
				else
					portalTemp.setName (MCharStrToUtf8(pNode->GetName()));
			}

			// Check if portal has 2 cluster
			int nNbCluster = 0;
			for (j = 0; j < vClusters.size(); ++j)
			{
				bool bPortalInCluster = true;
				for (k = 0; k < polyv.size(); ++k)
					if (!vClusters[j].isIn (polyv[k]) )
					{
						bPortalInCluster = false;
						break;
					}
				if (bPortalInCluster)
					++nNbCluster;
			}
			if (nNbCluster != 2)
			{
				// ERROR
				nlwarning("ERROR: The portal %s has not 2 clusters but %d", MCharStrToUtf8(vectNode[i]->GetName()).c_str(), nNbCluster);
			}


			vPortals.push_back (portalTemp);
			delete pMB;
			pMB = NULL;
			delete pMBB;
			pMBB = NULL;
		}
	}

	// Link instance to clusters (an instance has a list of clusters)
	nNumIG = 0;
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 0) // If not an accelerator
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh (*pNode, tvTime) || CExportNel::isDummy(*pNode, tvTime))
		{
			if (nAccelType&32) // Is the flag clusterize set ?
			{
				// Test against all clusters

				// The list of vertices used to test against cluster
				std::vector<NLMISC::CVector> *testVertices;
				std::vector<NLMISC::CVector>       FXVertices;  // Used only if the obj is a fx. It contains the corners of the bbox.
				bool  buildMeshBBox = true;

				/** If it is a mesh, we build its bbox and transform in world
				  * If it is a FX, we read its bbox from its shape
				  * If we can't read it, we use the bbox of the fx helper in max
				  */
				ObjectState os = pNode->EvalWorldState(tvTime);
				Object *obj = os.obj;
				// Check if there is an object
				if (obj)
				{
					Class_ID  clid = obj->ClassID();
					// is the object a particle system ?					
					if (clid.PartA() == NEL_PARTICLE_SYSTEM_CLASS_ID)
					{
						// build the shape from the file name
						// std::string objName = CExportNel::getNelObjectName(*pNode); 						
						std::string psFilePath;
						// try to get the complete path	
						if (!CExportNel::getValueByNameUsingParamBlock2(*pNode, "ps_file_name", (ParamType2) TYPE_STRING, &psFilePath, 0))
						{
							// if not found, get from the APP_DATAS
							psFilePath = CExportNel::getNelObjectName(*pNode);
							if (!psFilePath.empty())
								psFilePath = CPath::lookup(psFilePath, false);
						}
						if (!psFilePath.empty())
						{											
							NL3D::CShapeStream ss;
							NLMISC::CIFile iF;
							if (iF.open(psFilePath.c_str()))
							{
								try
								{								
									iF.serial(ss);
									NL3D::CParticleSystemShape *pss = dynamic_cast<NL3D::CParticleSystemShape *>(ss.getShapePointer());
									if (!pss)
									{
										nlwarning("ERROR: Node %s shape is not a FX", CExportNel::getName(*pNode).c_str());
									}
									else
									{									
										NLMISC::CAABBox bbox;
										pss->getAABBox(bbox);
										// transform in world
										Matrix3 xForm = pNode->GetNodeTM(tvTime);
										NLMISC::CMatrix nelXForm;
										CExportNel::convertMatrix(nelXForm, xForm);									
										bbox = NLMISC::CAABBox::transformAABBox(nelXForm, bbox);
										// store vertices of the bbox in the list
										FXVertices.reserve(8);
										for(uint k = 0; k < 8; ++k)
										{
											FXVertices.push_back(CVector(((k & 1) ? 1 : -1) * bbox.getHalfSize().x + bbox.getCenter().x,
																		 ((k & 2) ? 1 : -1) * bbox.getHalfSize().y + bbox.getCenter().y,
																		 ((k & 4) ? 1 : -1) * bbox.getHalfSize().z + bbox.getCenter().z));
										}
										//
										testVertices = &FXVertices;
										buildMeshBBox = false;
									}
									delete ss.getShapePointer();
									ss.setShapePointer(NULL);
								}
								catch (const NLMISC::Exception &e)
								{
									nlwarning(e.what());									
								}
							}							
							if (buildMeshBBox)
							{
								nlwarning("ERROR: Can't get bbox of a particle system from its shape, using helper bbox instead");
							}
						}
					}
				}

				CMesh::CMeshBuild *pMB = NULL;
				CMeshBase::CMeshBaseBuild *pMBB = NULL;

				if (buildMeshBBox)
				{				
					pMB = createMeshBuild (*pNode, tvTime, pMBB);
					convertToWorldCoordinate( pMB, pMBB );
					testVertices = &pMB->Vertices;
				}

				for(k = 0; k < vClusters.size(); ++k)
				{
					bool bMeshInCluster = false;

					for(j = 0; j < testVertices->size(); ++j)
					{
						if (vClusters[k].isIn ((*testVertices)[j]))
						{
							bMeshInCluster = true;
							break;
						}
					}

					if (bMeshInCluster)
					{
						aIGArray[nNumIG].Clusters.push_back (k);
					}
				}
				
				// debug purpose : to remove
				if (!vClusters.empty())
				if (aIGArray[nNumIG].Clusters.empty())
				{
					nlwarning("ERROR: Object %s is not attached to any cluster\nbut his flag clusterize is set", MCharStrToUtf8(pNode->GetName()).c_str());
				}
				// debug purpose : to remove

				delete pMB; pMB = NULL;
				delete pMBB; pMBB = NULL;
			}
			
			++nNumIG;
		}
		// debug purpose : to remove
		/*
		if ((nAccelType&3) == 0) // If not an accelerator
		if (!(nAccelType&32))
		{
			char tam[256];
			sprintf(tam,"Object %s is not clusterized", pNode->GetName());
			MessageBox(NULL, tam, "Info", MB_OK);
		}
		*/
		// debug purpose : to remove

	}


	// PointLight part
	//=================
	bool	sunLightEnabled= false;
	sint	nNumPointLight = 0;
	vector<CPointLightNamed>	pointLights;
	pointLights.resize(vectNode.size());
	// For all nodes
	for (i = 0; i < (sint)vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];

		SLightBuild		sLightBuild;

		// If it is a Max Light.
		if ( sLightBuild.canConvertFromMaxLight(pNode, tvTime) )
		{
			// And if this light is checked to realtime export
			int		nRTExport= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, BST_CHECKED);
			if(nRTExport == BST_CHECKED)
			{
				// get Max Light info.
				sLightBuild.convertFromMaxLight(pNode, tvTime);

				// Skip if LightDir
				if(sLightBuild.Type != SLightBuild::LightDir)
				{
					// Fill PointLight Info.
					NL3D::CPointLightNamed	&plNamed= pointLights[nNumPointLight];

					// Position
					plNamed.setPosition(sLightBuild.Position);
					// Attenuation
					plNamed.setupAttenuation(sLightBuild.rRadiusMin, sLightBuild.rRadiusMax);
					// Colors
					// Ensure A=255 for localAmbient to work.
					NLMISC::CRGBA	ambient= sLightBuild.Ambient;
					ambient.A= 255;
					plNamed.setDefaultAmbient(ambient);
					plNamed.setAmbient(ambient);
					plNamed.setDefaultDiffuse(sLightBuild.Diffuse);
					plNamed.setDiffuse(sLightBuild.Diffuse);
					plNamed.setDefaultSpecular(sLightBuild.Specular);
					plNamed.setSpecular(sLightBuild.Specular);

					// GroupName.
					plNamed.AnimatedLight = sLightBuild.AnimatedLight;
					plNamed.LightGroup = sLightBuild.LightGroup;

					// Which light type??
					if(sLightBuild.bAmbientOnly || sLightBuild.Type== SLightBuild::LightAmbient)
					{
						plNamed.setType(CPointLight::AmbientLight);
						// Special ambient info
						int		nRTAmbAdd= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_REALTIME_AMBIENT_ADD_SUN, BST_UNCHECKED);
						plNamed.setAddAmbientWithSun(nRTAmbAdd==BST_CHECKED);
					}
					else if(sLightBuild.Type== SLightBuild::LightPoint)
					{
						plNamed.setType(CPointLight::PointLight);
					}
					else if(sLightBuild.Type== SLightBuild::LightSpot)
					{
						plNamed.setType(CPointLight::SpotLight);
						// Export Spot infos.
						plNamed.setupSpotDirection(sLightBuild.Direction);
						plNamed.setupSpotAngle(sLightBuild.rHotspot, sLightBuild.rFallof);
					}
					else
					{
						// What???
						nlerror("sLightBuild.Type == %i", (uint32)(sLightBuild.Type));
						nlstop;
					}


					// inc Size
					++nNumPointLight;
				}
			}

			// if this light is a directionnal and checked to export as Sun Light
			int		nExportSun= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);
			if(nExportSun== BST_CHECKED)
			{
				// get Max Light info.
				sLightBuild.convertFromMaxLight(pNode, tvTime);

				// Skip if not dirLight.
				if(sLightBuild.Type == SLightBuild::LightDir)
					sunLightEnabled= true;
			}
		}
	}
	// Good size
	pointLights.resize(nNumPointLight);


	// Build the ig
	//=================

	CInstanceGroup* pIG = new CInstanceGroup;

	// Link portals and clusters and create meta cluster if one
	pIG->build (vGlobalPos,  aIGArray, vClusters, vPortals, pointLights);

	// IG touched by sun ??
	pIG->enableRealTimeSunContribution(sunLightEnabled);

	return pIG;
}

// ***************************************************************************

class	CMaxInstanceLighter : public NL3D::CInstanceLighter
{
public:
	void	initMaxLighter(Interface& _Ip)
	{
	}
	void	closeMaxLighter()
	{
	}

	virtual void progress (const char *message, float progress)
	{
	}
};

// ***************************************************************************

void CExportNel::buildScene (NL3D::CScene &scene, NL3D::CShapeBank &shapeBank, IDriver &driver, TimeValue tvTime, 
							 NL3D::CLandscape *landscape, IProgress *progress, bool buildHidden, 
							 bool onlySelected, bool buildLods)
{
	// Register classes
	// done in dllentry registerSerial3d ();

	CScene::registerBasics ();

	// Get node count
	int nNumNode=_Ip->GetRootNode ()->NumberOfChildren ();
	int nNbMesh=0;

	// *******************
	// * Then, build Mesh shapes
	// *******************

	// Prepare ig export.
	std::vector<INode*>						igVectNode;
	std::map<std::string, NL3D::IShape *>	igShapeMap;

	// SunDirection.
	NLMISC::CVector							igSunDirection(0, 1, -1);

	// SunColor.
	NLMISC::CRGBA							igSunColor(255, 255, 255);
	SLightBuild								sgLightBuild;

	// Build Mesh Shapes.
	_Options.FeedBack = progress;
	nNbMesh = 0;

	// View all selected objects
	int nNode;
	for (nNode=0; nNode<nNumNode; nNode++)
	{
		// Get the node
		INode* pNode=_Ip->GetRootNode ()->GetChildNode (nNode);
		if ( (!pNode->IsHidden () || buildHidden) && (pNode->Selected () || !onlySelected) )
		{
			string sTmp = "Object Name: ";
			sTmp += MCharStrToUtf8(pNode->GetName());
			if (progress)
				progress->setLine (0, sTmp);
			sTmp.clear();
			for (uint32 i = 1; i < 10; ++i) 
			{
				if (progress)
					progress->setLine (i, sTmp);
			}
			sTmp = "Last Error";
			if (progress)
			{
				progress->setLine (10, sTmp);
				progress->update();
			}
			
			// It is a zone ?
			if (RPO::isZone (*pNode, tvTime) && landscape)
			{
				// Get a Object pointer
				ObjectState os=pNode->EvalWorldState(_Ip->GetTime()); 

				// Ok ?
				if (os.obj)
				{
					// Convert in 3ds NeL patch mesh
					RPO *tri = (RPO *) os.obj->ConvertToType(_Ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
					if (tri)
					{
						CZone zone;
						CZoneSymmetrisation	sym;
						static int zoneId = 0;
						if (tri->rpatch->exportZone (pNode, &tri->patch, zone, sym, zoneId++, 160, 1, false))
							landscape->addZone (zone);
					}
				}
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, tvTime))
			{
				// Build skined ?
				bool skined=false;
				++nNbMesh;
				
				// Skinning ?
				if (CExportNel::isSkin (*pNode))
				{
					// todo: skinning export
				}
				
				// Build skined ?
				if (!skined)
				{
					// Is it an accelerator ?
					if ((CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32)&3) == 0)
					{
						// Export the shape
						IShape *pShape = NULL;
						pShape=buildShape (*pNode, tvTime, NULL, buildLods);

						// Export successful ?
						if (pShape)
						{
							// get the nelObjectName, as used in building of the instanceGroup.
							std::string	nelObjectName= CExportNel::getNelObjectName(*pNode);

							// ugly way to verify the shape is really added to the sahepBank: use a refPtr :)
							NLMISC::CRefPtr<IShape>		prefShape= pShape;


							std::string nelObjectNameNoExt;
							// Add to the view, but don't create the instance (created in ig).						
							if (!(nelObjectName.find(".shape") != std::string::npos || nelObjectName.find(".ps") != std::string::npos))
							{
								nelObjectNameNoExt = nelObjectName;
								nelObjectName += ".shape";							
							}
							else
							{
								std::string::size_type pos = nelObjectName.find(".");
								nlassert(pos != std::string::npos);
								nelObjectNameNoExt = std::string(nelObjectName, 0, pos);
							}
							
							// Add to the view, but don't create the instance (created in ig).
							// Since IG use strlwr version of the name, must strlwr it here.
							std::string	nelObjectNameLwr = nelObjectName;
							strlwr(nelObjectNameLwr);
							shapeBank.add (nelObjectNameLwr.c_str(), CSmartPtr<IShape> (pShape));
							scene.createInstance (nelObjectNameLwr.c_str());

							// If the shape is not destroyed in addMesh() (with smarPtr), then add it to the shape map.
							if(prefShape)
							{
								igShapeMap.insert( std::make_pair(nelObjectNameNoExt, pShape) );
							}
							
							// Add to list of node for IgExport.
							igVectNode.push_back(pNode);
						}
					}
				}
			}
		}
	}

	// if ExportLighting, Export all lights in scene (not only selected ones).
	if(_Options.bExportLighting)
	{
		// List all nodes in scene.
		vector<INode*>	nodeList;
		getObjectNodes(nodeList, tvTime);
		
		// For all of them.
		for(uint i=0;i<nodeList.size();i++)
		{
			INode	*pNode= nodeList[i];

			if( sgLightBuild.canConvertFromMaxLight(pNode, tvTime) )
			{
				// Convert it.
				sgLightBuild.convertFromMaxLight(pNode, tvTime);

				// PointLight/SpotLight/AmbientLight ??
				if(sgLightBuild.Type != SLightBuild::LightDir)
				{
					// Add to list of node for IgExport.
					igVectNode.push_back(pNode);
				}
				// "SunLight" ??
				else if( sgLightBuild.Type == SLightBuild::LightDir )
				{
					// if this light is checked to export as Sun Light
					int		nExportSun= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);
					if(nExportSun== BST_CHECKED)
					{
						// Add to list of node for IgExport (enabling SunLight in the ig)
						igVectNode.push_back(pNode);

						// Replace sun Direciton.
						igSunDirection= sgLightBuild.Direction;
						// Replace sun Color.
						igSunColor= sgLightBuild.Diffuse;
					}
				}
			}
		}
	}

	_Options.FeedBack = NULL;


	// *******************
	// * Export instance Group.
	// *******************

	// Info for lighting: retrieverBank and globalRetriever.
	CIgLighterLib::CSurfaceLightingInfo		slInfo;
	slInfo.RetrieverBank= NULL;
	slInfo.GlobalRetriever= NULL;

	// Result instance group
	vector<INode*> resultInstanceNode;

	// Build the ig (with pointLights)
	NL3D::CInstanceGroup	*ig= buildInstanceGroup(igVectNode, resultInstanceNode, tvTime);
	if(ig)
	{
		// If ExportLighting
		if( _Options.bExportLighting )
		{
			// Light the ig.
			NL3D::CInstanceGroup	*igOut= new NL3D::CInstanceGroup;
			// Init the lighter.
			CMaxInstanceLighter		maxInstanceLighter;
			maxInstanceLighter.initMaxLighter(*_Ip);

			// Setup LightDesc Ig.
			CInstanceLighter::CLightDesc	lightDesc;
			// Copy map to get info on shapes.
			lightDesc.UserShapeMap= igShapeMap;
			// Setup Shadow and overSampling.
			lightDesc.Shadow= _Options.bShadow;
			lightDesc.OverSampling= NLMISC::raiseToNextPowerOf2(_Options.nOverSampling);
			clamp(lightDesc.OverSampling, 0U, 32U);
			if(lightDesc.OverSampling==1)
				lightDesc.OverSampling= 0;
			// Setup LightDirection.
			lightDesc.LightDirection= igSunDirection.normed();
			// For interiors ig, disable Sun contrib according to ig.
			lightDesc.DisableSunContribution= !ig->getRealTimeSunContribution();


			// If View SurfaceLighting enabled
			if(_Options.bTestSurfaceLighting)
			{
				// Setup a CSurfaceLightingInfo
				slInfo.CellSurfaceLightSize= _Options.SurfaceLightingCellSize;
				NLMISC::clamp(slInfo.CellSurfaceLightSize, 0.001f, 1000000.f);
				slInfo.CellRaytraceDeltaZ= _Options.SurfaceLightingDeltaZ;
				slInfo.ColIdentifierPrefix= "col_";
				slInfo.ColIdentifierSuffix= "_";
			}


			// Light Ig.
			CIgLighterLib::lightIg(maxInstanceLighter, *ig, *igOut, lightDesc, slInfo, "");

			// Close the lighter.
			maxInstanceLighter.closeMaxLighter();

			// Swap pointer and release unlighted one.
			swap(ig, igOut);
			delete igOut; igOut = NULL;
		}
	
		// Add all models to the scene		
		ig->addToScene(scene, &driver);

		// Unfreeze all objects from HRC.
		ig->unfreezeHRC();
	}

	// *******************
	// * Launch
	// *******************

	// ExportLighting?
	if ( _Options.bExportLighting )
	{
		// Take the ambient of the scene as the ambient of the sun.
		CRGBA	sunAmb= getAmbientColor (tvTime);

		// Disable Global ambient light
		driver.setAmbientColor (CRGBA::Black);
		scene.setAmbientGlobal(CRGBA::Black);

		// setup lighting and sun, if any light added. Else use std OpenGL front lighting
		scene.enableLightingSystem(true);

		// Setup sun.
		scene.setSunAmbient(sunAmb);
		scene.setSunDiffuse(igSunColor);
		scene.setSunSpecular(igSunColor);
		scene.setSunDirection(igSunDirection);
	}
	else
	{
		/*
		// Setup ambient light
		driver.setAmbientColor (getAmbientColor (tvTime));
		scene.setAmbientGlobal (getAmbientColor (tvTime));

		// Build light vector
		std::vector<CLight> vectLight;
		getLights (vectLight, tvTime);

		// Light in the scene ?
		if (!vectLight.empty())
		{
			// Use old Driver Light mgt.
			scene.enableLightingSystem(false);
			scene.setSunAmbient(CRGBA::Black);
			scene.setSunDiffuse(igSunColor);
			scene.setSunSpecular(igSunColor);
			scene.setSunDirection(igSunDirection);

			// Insert each lights
			for (uint light=0; light<vectLight.size(); light++)
			{
				driver.enableLight (light);
				driver.setLight (light, vectLight[light]);
			}
		}
		*/
	}

	_Options.FeedBack = NULL;
}