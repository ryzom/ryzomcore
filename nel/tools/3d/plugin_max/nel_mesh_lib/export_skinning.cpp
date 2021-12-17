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
#include "export_appdata.h"
#include "nel/3d/skeleton_shape.h"
#include "iskin.h"

using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

#define PHYSIQUE_CLASS_ID Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)

// ***************************************************************************

void CExportNel::buildSkeletonShape (CSkeletonShape& skeletonShape, INode& node, mapBoneBindPos* mapBindPos, TInodePtrInt& mapId, 
									 TimeValue time)
{
	// Build a bone vector
	std::vector<CBoneBase> bonesArray;

	// Counter
	sint32 idCount=0;

	// Set for node name
	std::set<std::string> nameSet;

	// Parse the tree
	buildSkeleton (bonesArray, node, mapBindPos, mapId, nameSet, time, idCount);

	// Then build the object
	skeletonShape.build (bonesArray);
}


// ***************************************************************************
bool CExportNel::isBipedNode(INode &node)
{
	bool	ret= false;
	Control	*c= node.GetTMController();
	if( c && (
		c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID ||
		c->ClassID() == BIPBODY_CONTROL_CLASS_ID ))
		ret= true;

	return ret;
}


// ***************************************************************************
bool CExportNel::getNELUnHeritFatherScale(INode &node)
{
	// Default.
	bool	ret= false;


	/* If my parent exist, and if my parent is a BipedNode, always suppose I must unherit scale.
		This is because Biped NodeTM always have a Scale 1,1,1. Hence Sons bones do not herit scale.
	*/
	INode	*parentNode= node.GetParentNode();
	if(parentNode && isBipedNode(*parentNode) )
	{
		ret= true;
	}
	// Else, test the std way.
	else
	{
		// Get the TM controler
		Control *c;
		c = node.GetTMController();

		// If a TM controler exist
		if (c)
		{
			// Get the inherit flags
			DWORD flags=c->GetInheritanceFlags();

			// Unherit scale if all scale inherit flags are cleared
			ret= (flags&(INHERIT_SCL_X|INHERIT_SCL_Y|INHERIT_SCL_Z))!=0;
		}
	}

	return ret;
}


// ***************************************************************************
INode *CExportNel::getNELScaleReferenceNode(INode &node)
{
	INode	*referenceNode= NULL;

	bool	exportScale= getScriptAppData (&node, NEL3D_APPDATA_EXPORT_BONE_SCALE, BST_UNCHECKED)==BST_CHECKED;
	// Get the reference node
	if(exportScale)
	{
		std::string	boneScaleNameExt= getScriptAppData (&node, NEL3D_APPDATA_EXPORT_BONE_SCALE_NAME_EXT, "");
		if(!boneScaleNameExt.empty())
		{
			std::string	boneScaleName= getName(node) + boneScaleNameExt;
			// Get the reference node
			referenceNode= _Ip->GetINodeByName(MaxTStrFromUtf8(boneScaleName));
		}
	}

	return referenceNode;
}


// ***************************************************************************
void CExportNel::getNELBoneLocalScale(INode &node, TimeValue time, NLMISC::CVector &nelScale)
{
	// To get the correct local Scale, we use an other bone as reference (if present)
	INode	*referenceNode= getNELScaleReferenceNode(node);

	// get the Max local pos.
	Matrix3 localTM (TRUE);
	getLocalMatrix (localTM, node, time);
	NLMISC::CVector		maxScale;
	NLMISC::CQuat		maxRot;
	NLMISC::CVector		maxPos;
	decompMatrix (maxScale, maxRot, maxPos, localTM);


	// Biped node case, take the scale from ratio with the reference node.
	if(isBipedNode(node))
	{
		// Replace scale with ratio from reference value, if possible
		if (referenceNode)
		{
			ScaleValue	scaleValue;
			// get my Offset scale.
			CVector		myScale;
			scaleValue = node.GetObjOffsetScale();
			myScale.x= scaleValue.s.x;
			myScale.y= scaleValue.s.y;
			myScale.z= scaleValue.s.z;

			// get its Offset scale.
			CVector		refScale;
			scaleValue = referenceNode->GetObjOffsetScale();
			refScale.x= scaleValue.s.x;
			refScale.y= scaleValue.s.y;
			refScale.z= scaleValue.s.z;

			// Get The ratio as the result
			nelScale.x= myScale.x / refScale.x;
			nelScale.y= myScale.y / refScale.y;
			nelScale.z= myScale.z / refScale.z;
		}
		else
		{
			// Not present, suppose no scale.
			nelScale.set(1,1,1);
		}

	}
	// get the scale from std way.
	else
	{
		// We are a normal node here, not biped.

		/* If this node do not inherit scale (ie must unherit), then we must not take the localScale computed 
			with getLocalMatrix. In this case, the local Scale is simply the NodeTM scale.
		*/
		if( getNELUnHeritFatherScale(node) )
		{
			Matrix3	nodeTM;
			nodeTM = node.GetNodeTM (time);
			decompMatrix (maxScale, maxRot, maxPos, nodeTM);
			// Get the scale from worldMatrix.
			nelScale= maxScale;
		}
		else
		{
			// Get the scale from localMatrix.
			nelScale= maxScale;
		}
	}

}


// ***************************************************************************
void CExportNel::getNELBoneLocalTM(INode &node, TimeValue time,
		NLMISC::CVector &nelScale, NLMISC::CQuat &nelQuat, NLMISC::CVector &nelPos)
{
	// get the Max local pos.
	Matrix3 localTM (TRUE);
	getLocalMatrix (localTM, node, time);
	NLMISC::CVector		maxScale;
	NLMISC::CQuat		maxRot;
	NLMISC::CVector		maxPos;
	decompMatrix (maxScale, maxRot, maxPos, localTM);

	// get bone Rot.
	//=============

	// Same Rot.
	nelQuat= maxRot;
	nelQuat.normalize();


	// get Bone Scale.
	//=============

	// Get the NEL local scale.
	getNELBoneLocalScale(node, time, nelScale);

	// get bone Pos.
	//=============

	// Default is same pos.
	nelPos= maxPos;

	/* The following is important To have correct NEL position
		Let me explain: 
			- Biped bones NodeTM have correct World Position, but NodeTM.Scale == 1,1,1 (always)
				We can get the bone local scale from OffsetScale, this why we need a Reference Node, to perform the 
				difference (because the Reference Node ofsset scale is not a 1,1,1 scale)
			- Nel bones Pos are the LOCAL pos, and they INHERIT parent scale (like in Maya).
				Nel UnheritScale is used to unherit the scale from father to the rotation/scale only part of the son.
		The problem is that if we export Position default pos from the current scaled "node", we will have the 
		"already scaled from father Local Pos" from Biped. Instead we want the "not scaled from father Local Pos"
		because NEL will do the "scaled from father" job in RealTime.
		
		So to get the correct local Position, we must remove the fahter Nel Local Scale.
	*/

	// If my father is a biped bone, then do special stuff.
	INode	*parentNode= node.GetParentNode();
	// NB: don't need to test If I must unherit scale, because sons of Biped nodes always unherit scale...
	if(parentNode && isBipedNode(*parentNode))
	{
		/* In this case, I must remove my father Nel scale, because the biped NodeTM has a scale 1,1,1.
			Hence the Local position, computed with getLocalMatrix(), we have is false.
		*/
		CVector		fatherScale;
		// NB: avoid calling getNELBoneLocalTM() for father else Recursive call untill to the root !!!
		// We need just the scale here.
		getNELBoneLocalScale(*parentNode, time, fatherScale);
		nelPos.x/= fatherScale.x;
		nelPos.y/= fatherScale.y;
		nelPos.z/= fatherScale.z;
	}
}


// ***************************************************************************

void CExportNel::buildSkeleton (std::vector<CBoneBase>& bonesArray, INode& node, mapBoneBindPos* mapBindPos, TInodePtrInt& mapId, 
								std::set<std::string> &nameSet, TimeValue time, sint32& idCount, sint32 father)
{
	// **** Save the current the id
	int id=idCount;

	// **** Create a bone
	CBoneBase bone;

	// ** Set Name and id

	// Bone name
	bone.Name=getName (node);

	// Inserted ?
	if (!nameSet.insert (bone.Name).second)
		bone.Name+="_Second";

	// Id of the father
	bone.FatherId=father;

	// Insert id with name
	mapId.insert (TInodePtrInt::value_type(&node, id));

	// ** Set inherit flags

	// Must remove Scale from my father??
	bone.UnheritScale= getNELUnHeritFatherScale(node);


	// **** Set default tracks

	// Get the Nel Local TM
	NLMISC::CVector		nelScale;
	NLMISC::CQuat		nelRot;
	NLMISC::CVector		nelPos;
	getNELBoneLocalTM(node, time, nelScale, nelRot, nelPos);

	// Root must be exported with Identity because path are setuped interactively in the root of the skeleton
	if(id==0)
	{
		// Keep only the scale, because this last is not animated.
		nelPos= CVector::Null;
		nelRot= CQuat::Identity;
	}

	// Set the default tracks
	bone.DefaultScale=nelScale;
	bone.DefaultRotQuat=nelRot;
	bone.DefaultRotEuler=CVector (0,0,0);
	bone.DefaultPos=nelPos;


	// **** Get node bindpos matrix
	Matrix3 worldTM;
	bool matrixComputed=false;
	if (mapBindPos)
	{
		// Look for an entry in the map
		mapBoneBindPos::iterator bindPos=mapBindPos->find (&node);

		// Found ?
		if (bindPos!=mapBindPos->end())
		{
			// Get bind pos
			worldTM=bindPos->second;

			// Computed
			matrixComputed=true;
		}
	}

	// Compute the matrix the normal way ?
	if (!matrixComputed)
	{
		// if we have a reference node, ie the one with the good original pos/rot/scale used for bindPos, must use it.
		INode	*referenceNode= getNELScaleReferenceNode(node);
		if (referenceNode)
		{
			worldTM= referenceNode->GetNodeTM (time);
		}
		// else use mine.
		else
		{
			worldTM= node.GetNodeTM (time);
		}
	}

	// Set the bind pos of the bone, it is invNodeTM;
	convertMatrix (bone.InvBindPos, worldTM);
	bone.InvBindPos.invert();

	// **** Get bone Lod disactivation
	bone.LodDisableDistance= getScriptAppData (&node, NEL3D_APPDATA_BONE_LOD_DISTANCE, 0.f);
	bone.LodDisableDistance= std::max(0.f, bone.LodDisableDistance);

	// **** Add the bone
	bonesArray.push_back (bone);

	// **** Call on child
	const int numChildren = node.NumberOfChildren();
	for (int children=0; children<numChildren; children++)
		buildSkeleton (bonesArray, *node.GetChildNode(children), mapBindPos, mapId, nameSet, time, ++idCount, id);
}

// ***************************************************************************

bool CExportNel::isSkin (INode& node)
{
	// Return success
	bool ok=false;

	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASSID);

	// Found it ?
	if (skin)
	{
		// Modifier enabled ?
		//if (skin->IsEnabled())
		{
			// Get a com_skin2 interface
			ISkin *comSkinInterface=(ISkin*)skin->GetInterface (I_SKIN);

			// Found com_skin2 ?
			if (comSkinInterface)
			{
				// Get local data
				ISkinContextData *localData=comSkinInterface->GetContextInterface(&node);

				// Found ?
				if (localData)
				{
					// Skinned
					ok=true;

					// Release the interface
					skin->ReleaseInterface (I_SKIN, comSkinInterface);
				}
			}
		}
	}

	// Try physic
	if (!ok)
	{
		// Get the skin modifier
		Modifier* skin=getModifier (&node, PHYSIQUE_CLASS_ID);

		// Found it ?
		if (skin)
		{
			// Modifier enabled ?
			//if (skin->IsEnabled())
			{
				// Get a com_skin2 interface
				IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

				// Found com_skin2 ?
				if (physiqueInterface)
				{
					// Skinned
					ok=true;

					// Release the interface
					skin->ReleaseInterface (I_PHYINTERFACE, physiqueInterface);
				}
			}
		}
	}
	return ok;
}

// ***************************************************************************

uint CExportNel::buildSkinning (CMesh::CMeshBuild& buildMesh, const TInodePtrInt& skeletonShape, INode& node)
{
	// Return success
	uint ok=NoError;

	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASSID);

	// Build a the name array
	buildMesh.BonesNames.resize (skeletonShape.size());
	TInodePtrInt::const_iterator ite=skeletonShape.begin();
	while (ite != skeletonShape.end())
	{
		// Check the id
		nlassert ((uint)ite->second<buildMesh.BonesNames.size());

		// Names
		buildMesh.BonesNames[ite->second] = MCharStrToUtf8(ite->first->GetName());

		// Next
		ite++;
	}

	// Found it ?
	if (skin)
	{
		// **********    COMSKIN EXPORT    **********

		// Get a com_skin2 interface
		ISkin *comSkinInterface=(ISkin*)skin->GetInterface (I_SKIN);

		// Should been controled with isSkin before.
		nlassert (comSkinInterface);

		// Found com_skin2 ?
		if (comSkinInterface)
		{
			// Get local data
			ISkinContextData *localData=comSkinInterface->GetContextInterface(&node);

			// Should been controled with isSkin before.
			nlassert (localData);

			// Found ?
			if (localData)
			{
				// Check same vertices count
				uint vertCount=localData->GetNumPoints();

				// Ctrl we have the same number of vertices in the mesh and in the modifier.
				if (buildMesh.Vertices.size()!=vertCount)
				{
					ok=InvalidSkeleton;
				}
				else
				{
					// If not the same count, return false (perhaps, the modifier is not in the good place in the stack..)
					if (buildMesh.Vertices.size()==vertCount)
					{
						// Rebuild the array
						buildMesh.SkinWeights.resize (vertCount);

						// For each vertex
						for (uint vert=0; vert<vertCount; vert++)
						{
							// Get bones count for this vertex
							uint boneCount=localData->GetNumAssignedBones (vert);

							// No bones, can't export
							if (boneCount==0)
							{
								// Error
								ok=VertexWithoutWeight;
								break;
							}

							// A map of float / string
							std::multimap<float, uint> weightMap;

							// For each bones
							for (uint bone=0; bone<boneCount; bone++)
							{
								// Get the bone weight
								float weight=localData->GetBoneWeight (vert, bone);

								// Get bone number
								uint boneId=localData->GetAssignedBone (vert, bone);

								// Insert in the map
								weightMap.insert (std::map<float, uint>::value_type (weight, boneId));
							}

							// Keep only the NL3D_MESH_SKINNING_MAX_MATRIX highest bones
							while (weightMap.size()>NL3D_MESH_SKINNING_MAX_MATRIX)
							{
								// Remove the lowest weights
								weightMap.erase (weightMap.begin());
							}

							// Sum the NL3D_MESH_SKINNING_MAX_MATRIX highest bones
							float sum=0.f;
							std::multimap<float, uint>::iterator ite=weightMap.begin();
							while (ite!=weightMap.end())
							{
								// Add to the sum
								sum+=ite->first;
								
								// Next value
								ite++;
							}

							// Erase bones
							for (uint i=0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
							{
								// Erase
								buildMesh.SkinWeights[vert].MatrixId[i]=0;
								buildMesh.SkinWeights[vert].Weights[i]=0;
							}

							// For each bones in the list, build the skin information
							uint id=0;
							ite=weightMap.end();
							while (ite!=weightMap.begin())
							{
								// Previous value
								ite--;
								
								// Get the bones ID
								sint32 matrixId=-1;
								TInodePtrInt::const_iterator itId=skeletonShape.find (comSkinInterface->GetBone(ite->second));
								if (itId!=skeletonShape.end())
									matrixId=itId->second;

								// Find the bone ?
								if (matrixId==-1)
								{
									// no, error, wrong skeleton
									ok=InvalidSkeleton;
									break;
								}

								// Set the weight
								buildMesh.SkinWeights[vert].MatrixId[id]=matrixId;
								buildMesh.SkinWeights[vert].Weights[id]=ite->first/sum;
								
								// Next Id
								id++;
							}
							// Breaked ?
							if (ite!=weightMap.begin())
							{
								// break again to exit
								break;
							}
						}
					}
				}
			}
		}

		// Release the interface
		skin->ReleaseInterface (I_SKIN, comSkinInterface);
	}
	else
	{
		// **********    PHYSIQUE EXPORT    **********

		// Physique mode
		Modifier* skin=getModifier (&node, PHYSIQUE_CLASS_ID);

		// Must exist
		nlassert (skin);
		
		// Get a com_skin2 interface
		IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

		// Should been controled with isSkin before.
		nlassert (physiqueInterface);

		// Found com_skin2 ?
		if (physiqueInterface)
		{
			// Get local data
			IPhyContextExport *localData=physiqueInterface->GetContextInterface(&node);

			// Should been controled with isSkin before.
			nlassert (localData);

			// Found ?
			if (localData)
			{
				// Use rigid export
				localData->ConvertToRigid (TRUE);

				// Allow blending
				localData->AllowBlending (TRUE);

				// Check same vertices count
				uint vertCount=localData->GetNumberVertices();

				// Ctrl we have the same number of vertices in the mesh and in the modifier.
				if (buildMesh.Vertices.size()!=vertCount)
				{
					ok=InvalidSkeleton;
				}
				else
				{
					// If not the same count, return false (perhaps, the modifier is not in the good place in the stack..)
					if (buildMesh.Vertices.size()==vertCount)
					{
						// Rebuild the array
						buildMesh.SkinWeights.resize (vertCount);

						// For each vertex
						for (uint vert=0; vert<vertCount; vert++)
						{
							// Get a vertex interface
							IPhyVertexExport *vertexInterface=localData->GetVertexInterface (vert);

							// Check if it is a rigid vertex or a blended vertex
							IPhyRigidVertex			*rigidInterface=NULL;
							IPhyBlendedRigidVertex	*blendedInterface=NULL;
							int type=vertexInterface->GetVertexType ();
							if (type==RIGID_TYPE)
							{
								// this is a rigid vertex
								rigidInterface=(IPhyRigidVertex*)vertexInterface;
							}
							else
							{
								// It must be a blendable vertex
								nlassert (type==RIGID_BLENDED_TYPE);
								blendedInterface=(IPhyBlendedRigidVertex*)vertexInterface;
							}

							// Get bones count for this vertex
							uint boneCount;
							if (blendedInterface)
							{
								// If blenvertex, only one bone
								boneCount=blendedInterface->GetNumberNodes();
							}
							else
							{
								// If rigid vertex, only one bone
								boneCount=1;
							}

							// No bones, can't export
							if (boneCount==0)
							{
								// Error
								ok=VertexWithoutWeight;
								break;
							}

							// A map of float / string
							std::multimap<float, INode*> weightMap;

							// For each bones
							for (uint bone=0; bone<boneCount; bone++)
							{
								if (blendedInterface)
								{
									// Get node
									INode *node=blendedInterface->GetNode(bone);
									if (node == NULL)
									{
										nlwarning("node == NULL; bone = %i / %i", bone, boneCount);
									}
									else
									{
										// Get the bone weight
										float weight=blendedInterface->GetWeight(bone);
										
										// Insert in the map
										weightMap.insert (std::map<float, INode*>::value_type (weight, node));
									}
								}
								else
								{
									// Get node
									INode *node=rigidInterface->GetNode();
									nlassert (node);

									// Insert in the map
									weightMap.insert (std::map<float, INode*>::value_type (1.f, node));
								}
							}

							// Keep only the NL3D_MESH_SKINNING_MAX_MATRIX highest bones
							while (weightMap.size()>NL3D_MESH_SKINNING_MAX_MATRIX)
							{
								// Remove the lowest weights
								weightMap.erase (weightMap.begin());
							}

							// Sum the NL3D_MESH_SKINNING_MAX_MATRIX highest bones
							float sum=0.f;
							std::multimap<float, INode*>::iterator ite=weightMap.begin();
							while (ite!=weightMap.end())
							{
								// Add to the sum
								sum+=ite->first;
								
								// Next value
								ite++;
							}

							// Erase bones
							for (uint i=0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
							{
								// Erase
								buildMesh.SkinWeights[vert].MatrixId[i]=0;
								buildMesh.SkinWeights[vert].Weights[i]=0;
							}

							// For each bones in the list, build the skin information
							uint id=0;
							ite=weightMap.end();
							while (ite!=weightMap.begin())
							{
								// Previous value
								ite--;

								// Get the bones ID
								sint32 matrixId=-1;
								TInodePtrInt::const_iterator itId=skeletonShape.find (ite->second);
								if (itId!=skeletonShape.end())
									matrixId=itId->second;

								// Find the bone ?
								if (matrixId==-1)
								{
									// no, error, wrong skeleton
									ok=InvalidSkeleton;
									break;
								}

								// Set the weight
								buildMesh.SkinWeights[vert].MatrixId[id]=matrixId;
								buildMesh.SkinWeights[vert].Weights[id]=ite->first/sum;
								
								// Next Id
								id++;
							}
							
							// Breaked ?
							if (ite!=weightMap.begin())
							{
								// break again to exit
								break;
							}

							// Release vertex interfaces
							localData->ReleaseVertexInterface (vertexInterface);
						}
					}
				}

				// Release locaData interface
				physiqueInterface->ReleaseContextInterface (localData);
			}

		}

		// Release the interface
		skin->ReleaseInterface (I_PHYINTERFACE, physiqueInterface);
	}

	return ok;
}

// ***************************************************************************

INode *getRoot (INode *pNode)
{
	INode* parent=pNode->GetParentNode();
	if (parent)
	{
		if (parent->IsRootNode())
			return pNode;
		else
			return getRoot (parent);
	}
	else 
		return NULL;
}

// ***************************************************************************

INode* CExportNel::getSkeletonRootBone (INode& node)
{
	// Return node
	INode* ret=NULL;

	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASSID);

	// Found it ?
	if (skin)
	{
		// Get a com_skin2 interface
		ISkin *comSkinInterface=(ISkin*)skin->GetInterface (I_SKIN);

		// Found com_skin2 ?
		if (comSkinInterface)
		{
			// Get local data
			ISkinContextData *localData=comSkinInterface->GetContextInterface(&node);

			// Found ?
			if (localData)
			{
				// Look for a bone...

				// For each vertices
				for (uint vtx=0; vtx<(uint)localData->GetNumPoints(); vtx++)
				{
					// For each bones
					uint bone;
					for (bone=0; bone<(uint)localData->GetNumAssignedBones (vtx); bone++)
					{
						// Get the bone pointer
						INode *newBone=comSkinInterface->GetBone(localData->GetAssignedBone(vtx, bone));

						// Get the root of the hierarchy
						ret=getRoot (newBone);
						break;
					}
					// Rebreak
					if (bone!=(uint)localData->GetNumAssignedBones (vtx))
						break;
				}
			}

			// Release the interface
			skin->ReleaseInterface (I_SKIN, comSkinInterface);
		}
	}
	else
	{
		// Get the skin modifier
		skin=getModifier (&node, PHYSIQUE_CLASS_ID);

		// Found it ?
		if (skin)
		{
			// Get a com_skin2 interface
			IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

			// Found com_skin2 ?
			if (physiqueInterface)
			{
				// Get local data
				IPhyContextExport *localData=physiqueInterface->GetContextInterface(&node);

				// Found ?
				if (localData)
				{
					// Use rigid export
					localData->ConvertToRigid (TRUE);

					// Allow blending
					localData->AllowBlending (TRUE);

					// Look for a bone...

					// For each vertices
					uint numVert=(uint)localData->GetNumberVertices();
					for (uint vtx=0; vtx<numVert; vtx++)
					{
						bool found=false;

						// Get a vertex interface
						IPhyVertexExport *vertexInterface=localData->GetVertexInterface (vtx);

						if (vertexInterface)
						{
							// Check if it is a rigid vertex or a blended vertex
							int type=vertexInterface->GetVertexType ();
							if (type==RIGID_TYPE)
							{
								// this is a rigid vertex
								IPhyRigidVertex			*rigidInterface=(IPhyRigidVertex*)vertexInterface;

								// Get the bone
								INode *newBone=rigidInterface->GetNode();

								// Get the root of the hierarchy
								ret=getRoot (newBone);
								found=true;
								break;
							}
							else
							{
								// It must be a blendable vertex
								nlassert (type==RIGID_BLENDED_TYPE);
								IPhyBlendedRigidVertex	*blendedInterface=(IPhyBlendedRigidVertex*)vertexInterface;

								// For each bones
								uint bone;
								uint count=(uint)blendedInterface->GetNumberNodes ();
								for (bone=0; bone<count; bone++)
								{
									// Get the bone pointer
									INode *newBone=blendedInterface->GetNode(bone);

									// Get the root of the hierarchy
									ret=getRoot (newBone);
									found=true;
									break;
								}
							}
						}
						else
						{
							nlwarning("Physique vertex interface NULL");
						}

						// Release vertex interfaces
						localData->ReleaseVertexInterface (vertexInterface);

						// Rebreak
						if (found)
							break;

						// Release vertex interfaces
						localData->ReleaseVertexInterface (vertexInterface);
					}

					// Release locaData interface
					physiqueInterface->ReleaseContextInterface (localData);
				}

				// Release the interface
				skin->ReleaseInterface (I_PHYINTERFACE, physiqueInterface);
			}
		}
	}

	// Return result;
	return ret;
}

// ***************************************************************************

void CExportNel::addSkeletonBindPos (INode& skinedNode, mapBoneBindPos& boneBindPos)
{
	// Return success
	uint ok=NoError;

	// Get the skin modifier
	Modifier* skin=getModifier (&skinedNode, SKIN_CLASSID);

	// Found it ?
	if (skin)
	{
		// Get a com_skin2 interface
		ISkin *comSkinInterface=(ISkin*)skin->GetInterface (I_SKIN);

		// Should been controled with isSkin before.
		nlassert (comSkinInterface);

		// Found com_skin2 ?
		if (comSkinInterface)
		{
			// Get local data
			ISkinContextData *localData=comSkinInterface->GetContextInterface(&skinedNode);

			// Should been controled with isSkin before.
			nlassert (localData);

			// Found ?
			if (localData)
			{
				// Check same vertices count
				uint vertCount=localData->GetNumPoints();

				// For each vertex
				for (uint vert=0; vert<vertCount; vert++)
				{
					// Get bones count for this vertex
					uint boneCount=localData->GetNumAssignedBones (vert);

					// For each bones
					for (uint bone=0; bone<boneCount; bone++)
					{
						// Get the bone id
						int boneId=localData->GetAssignedBone(vert, bone);

						// Get bone INode*
						INode *boneNode=comSkinInterface->GetBone(boneId);

						// Get the bind matrix of the bone
						Matrix3 bindPos;
						comSkinInterface->GetBoneInitTM(boneNode, bindPos);

						// Add an entry inthe map
						boneBindPos.insert (mapBoneBindPos::value_type (boneNode, bindPos));
					}
				}
			}

			// Release the interface
			skin->ReleaseInterface (I_SKIN, comSkinInterface);
		}
	}
	else
	{
		// Get the skin modifier
		Modifier* skin=getModifier (&skinedNode, PHYSIQUE_CLASS_ID);

		// Should been controled with isSkin before.
		nlassert (skin);

		// Found it ?
		if (skin)
		{
			// Get a com_skin2 interface
			IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

			// Should been controled with isSkin before.
			nlassert (physiqueInterface);

			// Found com_skin2 ?
			if (physiqueInterface)
			{
				// Get local data
				IPhyContextExport *localData=physiqueInterface->GetContextInterface(&skinedNode);

				// Should been controled with isSkin before.
				nlassert (localData);

				// Found ?
				if (localData)
				{
					// Use rigid export
					localData->ConvertToRigid (TRUE);

					// Allow blending
					localData->AllowBlending (TRUE);

					// Check same vertices count
					uint vertCount=localData->GetNumberVertices();

					// For each vertex
					for (uint vert=0; vert<vertCount; vert++)
					{
						if (vert==111)
							int toto=0;
						// Get a vertex interface
						IPhyVertexExport *vertexInterface=localData->GetVertexInterface (vert);

						// Check if it is a rigid vertex or a blended vertex
						int type=vertexInterface->GetVertexType ();
						if (type==RIGID_TYPE)
						{
							// this is a rigid vertex
							IPhyRigidVertex			*rigidInterface=(IPhyRigidVertex*)vertexInterface;

							// Get bone INode*
							INode *bone=rigidInterface->GetNode();

							// Get the bind matrix of the bone
							Matrix3 bindPos;
							int res=physiqueInterface->GetInitNodeTM (bone, bindPos);
							nlassert (res==MATRIX_RETURNED);

							// Add an entry inthe map
							if (boneBindPos.insert (mapBoneBindPos::value_type (bone, bindPos)).second)
							{
#ifdef NL_DEBUG
								// *** Debug info

								// Bone name
								std::string boneName=getName (*bone);

								// Local matrix
								Matrix3 nodeTM;
								nodeTM=bone->GetNodeTM (0);

								// Offset matrix
								Matrix3 offsetScaleTM (TRUE);
								Matrix3 offsetRotTM (TRUE);
								Matrix3 offsetPosTM (TRUE);
								ApplyScaling (offsetScaleTM, bone->GetObjOffsetScale ());
								offsetRotTM.SetRotate (bone->GetObjOffsetRot ());
								offsetPosTM.SetTrans (bone->GetObjOffsetPos ());
								Matrix3 offsetTM = offsetScaleTM * offsetRotTM * offsetPosTM;

								// Local + offset matrix
								Matrix3 nodeOffsetTM = offsetTM * nodeTM;

								// Init TM
								Matrix3 initTM;
								int res=physiqueInterface->GetInitNodeTM (bone, initTM);
								nlassert (res==MATRIX_RETURNED);

								// invert
								initTM.Invert();
								Matrix3 compNode=nodeTM*initTM;
								Matrix3 compOffsetNode=nodeOffsetTM*initTM;
								Matrix3 compOffsetNode2=nodeOffsetTM*initTM;
#endif // NL_DEBUG
							}
						}
						else
						{
							// It must be a blendable vertex
							nlassert (type==RIGID_BLENDED_TYPE);
							IPhyBlendedRigidVertex	*blendedInterface=(IPhyBlendedRigidVertex*)vertexInterface;

							// For each bones
							uint boneIndex;
							uint count=(uint)blendedInterface->GetNumberNodes ();
							for (boneIndex=0; boneIndex<count; boneIndex++)
							{
								// Get the bone pointer
								INode *bone = blendedInterface->GetNode(boneIndex);

								if (bone == NULL)
								{
									nlwarning("bone == NULL; boneIndex = %i / %i", boneIndex, count);
								}
								else
								{
									// Get the bind matrix of the bone
									Matrix3 bindPos;
									int res = physiqueInterface->GetInitNodeTM (bone, bindPos);

									if (res != MATRIX_RETURNED)
									{
										nlwarning("res != MATRIX_RETURNED; res = %d; boneIndex = %u / %u", res, boneIndex, count);
										nlwarning("bone = %p", bone);
										std::string boneName = getName (*bone);
										nlwarning("boneName = %s", boneName.c_str());
										nlassert(false);
									}

									// Add an entry inthe map
									if (boneBindPos.insert (mapBoneBindPos::value_type (bone, bindPos)).second)
									{
	#ifdef NL_DEBUG
										// *** Debug info

										// Bone name
										std::string boneName=getName (*bone);

										// Local matrix
										Matrix3 nodeTM;
										nodeTM=bone->GetNodeTM (0);

										// Offset matrix
										Matrix3 offsetScaleTM (TRUE);
										Matrix3 offsetRotTM (TRUE);
										Matrix3 offsetPosTM (TRUE);
										ApplyScaling (offsetScaleTM, bone->GetObjOffsetScale ());
										offsetRotTM.SetRotate (bone->GetObjOffsetRot ());
										offsetPosTM.SetTrans (bone->GetObjOffsetPos ());
										Matrix3 offsetTM = offsetScaleTM * offsetRotTM * offsetPosTM;

										// Local + offset matrix
										Matrix3 nodeOffsetTM = offsetTM * nodeTM;

										// Init TM
										Matrix3 initTM;
										int res=physiqueInterface->GetInitNodeTM (bone, initTM);
										nlassert (res==MATRIX_RETURNED);

										// invert
										initTM.Invert();
										Matrix3 compNode=nodeTM*initTM;
										Matrix3 compOffsetNode=nodeOffsetTM*initTM;
										Matrix3 compOffsetNode2=nodeOffsetTM*initTM;
#endif // NL_DEBUG
									}
								}
							}
						}
					
						// Release vertex interfaces
						localData->ReleaseVertexInterface (vertexInterface);
					}

					// Release locaData interface
					physiqueInterface->ReleaseContextInterface (localData);
				}

				// Release the interface
				skin->ReleaseInterface (I_SKIN, physiqueInterface);
			}
		}
	}
}

// ***************************************************************************


void CExportNel::enableSkinModifier (INode& node, bool enable)
{
	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASSID);

	// Found it ?
	if (skin)
	{
		// Enable ?
		if (enable)
			skin->EnableMod ();
		else
			skin->DisableMod ();
	}
	else
	{
		// Get the physique modifier
		Modifier* skin=getModifier (&node, PHYSIQUE_CLASS_ID);

		// Found it ?
		if (skin)
		{
			// Enable ?
			if (enable)
				skin->EnableMod ();
			else
				skin->DisableMod ();
		}
	}
}

// ***************************************************************************
#define TEMP_MAX_WEIGHT	8
struct CTempSkinVertex
{
	// World Position. TODO: world,local????
	NLMISC::CVector		Pos;
	// The number of weight. TODO: more
	uint				NumWeight;
	INode				*Bone[TEMP_MAX_WEIGHT];
	float				Weight[TEMP_MAX_WEIGHT];
	// If this vertex belongs to the input (=> not be modified)
	bool				Input;
	// If this vertex is an output modified related to mirroring (NB: only 
	bool				Mirrored;

	CTempSkinVertex()
	{
		NumWeight= 0;
		Input= false;
		Mirrored= false;
	}
};

struct CSortVertex 
{
	uint	Index;
	float	SqrDist;
	bool	operator<(const CSortVertex &o) const
	{
		return SqrDist < o.SqrDist;
	}
};

// get the bone Side -1,0,1
static sint	getBoneSide(INode *bone, std::string &mirrorName)
{
	sint	side= 0;
	sint	pos;
	mirrorName = MCharStrToUtf8(bone->GetName());

	if((pos= mirrorName.find(" R "))!=std::string::npos)
	{
		side= 1;
		mirrorName[pos+1]= 'L';
	}
	else if((pos= mirrorName.find(" L "))!=std::string::npos)
	{
		side= -1;
		mirrorName[pos+1]= 'R';
	}

	return side;
}

// From a bone, retrieve the bone mirror (by name: R / L). NB: if not found, return same (eg: important for mirror on spine).
static INode *getMirrorBone(const std::vector<INode*>	&skeletonNodes, INode *bone)
{
	// TODO: optimize doing a map bone / mirrored bone
	std::string	mirrorName;
	sint	bs= getBoneSide(bone, mirrorName);

	// if not a middle bone
	if(bs!=0)
	{
		// find
		for(uint i=0;i<skeletonNodes.size();i++)
		{
			if(mirrorName == MCharStrToUtf8(skeletonNodes[i]->GetName()))
				return skeletonNodes[i];
		}
	}

	// if fails, return him
	return bone;
}

bool CExportNel::mirrorPhysiqueSelection(INode &node, TimeValue tvTime, const std::vector<uint> &vertIn, 
		float threshold)
{
	bool	ok;
	uint	i;

	// no vertices selected?
	if(vertIn.empty())
		return true;

	// **** Get all the skeleton node 
	std::vector<INode*>		skeletonNodes;
	INode	*skelRoot= getSkeletonRootBone(node);
	if(!skelRoot)
		return false;
	getObjectNodes(skeletonNodes, tvTime, skelRoot);


	// **** Build the Vector (world) part
	std::vector<CTempSkinVertex>	tempVertex;
	uint	vertCount;

	// Get a pointer on the object's node.
    ObjectState os = node.EvalWorldState(tvTime);
    Object *obj = os.obj;

	// Check if there is an object
	ok= false;
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

				// Get the node matrix. TODO: Matrix headhache?
				/*Matrix3 nodeMatrixMax;
				CMatrix nodeMatrix;
				getLocalMatrix (nodeMatrixMax, node, tvTime);
				convertMatrix (nodeMatrix, nodeMatrixMax);*/

				// retrive Position geometry
				vertCount= tri->NumPoints();
				tempVertex.resize(vertCount);
				for(uint i=0;i<vertCount;i++)
				{
					Point3 v= tri->GetPoint(i);
					tempVertex[i].Pos.set(v.x, v.y, v.z);
				}

				// Delete the triObject if we should...
				if (deleteIt)
					tri->DeleteThis();
				tri = NULL;

				// ok!
				ok= true;
			}
		}
	}
	if(!ok)
		return false;

	// no vertices? abort
	if(vertCount==0)
		return true;


	// **** Mark all Input vertices
	for(i=0;i<vertIn.size();i++)
	{
		nlassert(vertIn[i]<vertCount);
		tempVertex[vertIn[i]].Input= true;
	}


	// **** Build the output vertices
	std::vector<uint>	vertOut;
	vertOut.reserve(tempVertex.size());

	// Build the in bbox
	CAABBox		bbox;
	bbox.setCenter(tempVertex[vertIn[0]].Pos);
	for(i=0;i<vertIn.size();i++)
	{
		bbox.extend(tempVertex[vertIn[i]].Pos);
	}
	bbox.setHalfSize(bbox.getHalfSize()+CVector(threshold, threshold, threshold));

	// mirror in X
	CVector		vMin= bbox.getMin();
	CVector		vMax= bbox.getMax();
	vMin.x= -vMin.x;
	vMax.x= -vMax.x;
	std::swap(vMin.x, vMax.x);
	bbox.setMinMax(vMin, vMax);

	// get all out vertices in the mirrored bbox.
	for(i=0;i<tempVertex.size();i++)
	{
		if(bbox.include(tempVertex[i].Pos))
		{
			vertOut.push_back(i);
		}
	}


	// **** Build the skin information
	// Get the skin modifier
	Modifier* skin=getModifier (&node, PHYSIQUE_CLASS_ID);

	// Found it ?
	ok= false;
	if (skin)
	{
		// Get a com_skin2 interface
		IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

		// Found com_skin2 ?
		if (physiqueInterface)
		{
			// Get local data
			IPhyContextExport *localData= physiqueInterface->GetContextInterface(&node);

			// Found ?
			if (localData)
			{
				// Use rigid export
				localData->ConvertToRigid (TRUE);

				// Allow blending
				localData->AllowBlending (TRUE);

				// Skinned
				ok=true;

				// TODO?
				nlassert(tempVertex.size()<=(uint)localData->GetNumberVertices());

				// For each vertex
				for (uint vert=0; vert<vertCount; vert++)
				{
					// Get a vertex interface
					IPhyVertexExport *vertexInterface= localData->GetVertexInterface (vert);

					// Check if it is a rigid vertex or a blended vertex
					IPhyRigidVertex			*rigidInterface=NULL;
					IPhyBlendedRigidVertex	*blendedInterface=NULL;
					int type=vertexInterface->GetVertexType ();
					if (type==RIGID_TYPE)
					{
						// this is a rigid vertex
						rigidInterface=(IPhyRigidVertex*)vertexInterface;
					}
					else
					{
						// It must be a blendable vertex
						nlassert (type==RIGID_BLENDED_TYPE);
						blendedInterface=(IPhyBlendedRigidVertex*)vertexInterface;
					}

					// Get bones count for this vertex
					uint boneCount;
					if (blendedInterface)
					{
						// If blenvertex, only one bone
						boneCount=blendedInterface->GetNumberNodes();
					}
					else
					{
						// If rigid vertex, only one bone
						boneCount=1;
					}
					if(boneCount>TEMP_MAX_WEIGHT)
						boneCount= TEMP_MAX_WEIGHT;

					// NB: if input 0, won't be mirrored
					tempVertex[vert].NumWeight= boneCount;
					for(uint bone=0;bone<boneCount;bone++)
					{
						if (blendedInterface)
						{
							tempVertex[vert].Bone[bone]= blendedInterface->GetNode(bone);
							nlassert(tempVertex[vert].Bone[bone]);
							tempVertex[vert].Weight[bone]= blendedInterface->GetWeight(bone);
						}
						else
						{
							tempVertex[vert].Bone[bone]= rigidInterface->GetNode();
							tempVertex[vert].Weight[bone]= 1;
						}
					}

					// Release vertex interfaces
					localData->ReleaseVertexInterface (vertexInterface);
				}

			}

			// release context interface
			physiqueInterface->ReleaseContextInterface(localData);
		}

		// Release the interface
		skin->ReleaseInterface (I_PHYINTERFACE, physiqueInterface);
	}
	if(!ok)
		return false;


	// **** Real Algo stuff:
	// For all vertices wanted to be mirrored
	std::vector<CSortVertex>	sortVert;
	sortVert.reserve(tempVertex.size());
	for(i=0;i<vertIn.size();i++)
	{
		CTempSkinVertex		&svIn= tempVertex[vertIn[i]];
		// if it still has no bones set, skip
		if(svIn.NumWeight==0)
			continue;

		// mirror vert to test
		CVector		vertTest= svIn.Pos;
		vertTest.x*= -1;

		// get the best vertex
		sortVert.clear();

		// Search for all output vertices if ones match
		for(uint j=0;j<vertOut.size();j++)
		{
			uint	dstIdx= vertOut[j];
			nlassert(dstIdx<tempVertex.size());
			CTempSkinVertex	&skinv= tempVertex[dstIdx];
			// take only if not an input, and if not already mirrored
			if(!skinv.Input && !skinv.Mirrored)
			{
				CSortVertex		sortv;
				sortv.Index= dstIdx;
				sortv.SqrDist= (skinv.Pos - vertTest).sqrnorm();
				// Finally, take it only if sufficiently near
				if(sortv.SqrDist <= threshold*threshold)
					sortVert.push_back(sortv);
			}
		}

		// if some found.
		if(!sortVert.empty())
		{
			// sort array.
			std::sort(sortVert.begin(), sortVert.end());

			// take the first, mirror setup
			uint	dstIdx= sortVert[0].Index;
			tempVertex[dstIdx].NumWeight= svIn.NumWeight;
			for(uint k=0;k<svIn.NumWeight;k++)
			{
				tempVertex[dstIdx].Weight[k]= svIn.Weight[k];
				tempVertex[dstIdx].Bone[k]= getMirrorBone( skeletonNodes, svIn.Bone[k] );
			}

			// mark as mirrored!
			tempVertex[dstIdx].Mirrored= true;
		}
	}


	// **** Write the result to the skin.
	ok= false;
	if (skin)
	{
		// Get a com_skin2 interface
		IPhysiqueImport *physiqueInterface=(IPhysiqueImport *)skin->GetInterface (I_PHYIMPORT);

		// Found com_skin2 ?
		if (physiqueInterface)
		{
			// Get local data
			IPhyContextImport *localData= physiqueInterface->GetContextInterface(&node);

			// TODO?
			nlassert(tempVertex.size()<=(uint)localData->GetNumberVertices());

			// Found ?
			if (localData)
			{
				// Skinned
				ok=true;
				
				for(uint i=0;i<tempVertex.size();i++)
				{
					CTempSkinVertex		&sv= tempVertex[i];

					// if its a mirrored output vertex
					if(sv.Mirrored)
					{
						IPhyBlendedRigidVertexImport	*blendedInterface= NULL;
						blendedInterface= (IPhyBlendedRigidVertexImport*)localData->SetVertexInterface(i, RIGID_BLENDED_TYPE);

						if(blendedInterface)
						{
							// set the vertex data
							for(uint bone=0;bone<sv.NumWeight;bone++)
							{
								blendedInterface->SetWeightedNode(sv.Bone[bone], sv.Weight[bone], bone==0);
							}

							// UI bonus: lock it
							blendedInterface->LockVertex(TRUE);

							// release
							localData->ReleaseVertexInterface(blendedInterface);
						}
					}
				}
			}

			// release
			physiqueInterface->ReleaseContextInterface(localData);
		}

		// release
		skin->ReleaseInterface(I_PHYIMPORT, physiqueInterface);
	}


	return ok;
}


