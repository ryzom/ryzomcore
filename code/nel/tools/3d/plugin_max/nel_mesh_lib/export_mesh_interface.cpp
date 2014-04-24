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

#include <set>
#include <vector>


#include "nel/misc/line.h"
#include "nel/misc/polygon.h"
#include "nel/misc/path.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include "export_appdata.h"



using namespace NLMISC;
using namespace NL3D;
using namespace std;




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// a vertice of a mesh interface, private use
struct CMeshInterfaceVertex
{
	CVector Pos;
	CVector Normal;

	// test wether the given vertex can be welded to one of this interface vertices
	bool	canWeld(const NLMISC::CVector &vert, float threshold) const
	{
		NLMISC::CVector distV = vert - Pos;
		return distV.norm() < threshold;
	}

	// try snapping a vertex to this mesh Interface
	bool	snapVert(NLMISC::CVector &vert, NLMISC::CVector &normal, const NLMISC::CMatrix &toWorldMat, float threshold)
	{					
		if (canWeld(toWorldMat * vert, threshold))
		{
			// snap to pos and normal
			// The pos of the interface vertex is in world space, so we put it back in object space			
			NLMISC::CMatrix invMat = toWorldMat.inverted();
			vert   = invMat * Pos; 
			normal = invMat.mulVector(Normal);
			return true;
		}				
		return false;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// a mesh Interface, private use
struct CMeshInterface
{
	std::vector<CMeshInterfaceVertex> Verts;
	// try to snap a vertex to one of this Interface vertex
	bool	snapVert(NLMISC::CVector &vert, NLMISC::CVector &normal, const NLMISC::CMatrix &toWorldMat, float threshold)
	{
		for(uint k = 0; k < Verts.size(); ++k)
		{
			if (Verts[k].snapVert(vert, normal, toWorldMat, threshold)) return true;
		}		
		return false;
	}

	// test wether a vertex can be welded to that interface. snapTo get the index of the interface vertex snapped to
	bool	canWeld(const NLMISC::CVector &pos, float threshold, uint &snapTo) const
	{
		for(uint k = 0; k < Verts.size(); ++k)
		{
			if (Verts[k].canWeld(pos, threshold))
			{
				snapTo= k;
				return true;
			}
		}
		return false;
	}
	// test wether a vertex can be welded to that interface
	bool	canWeld(const NLMISC::CVector &pos, float threshold) const
	{
		uint	dummy;
		return canWeld(pos, threshold, dummy);
	}


	// build a bbox from this interface
	void	buildBBox(NLMISC::CAABBox &dest)
	{
		nlassert(!Verts.empty());
		CVector minV = Verts[0].Pos;
		CVector maxV = Verts[0].Pos;		
		for(uint k = 1; k < Verts.size(); ++k)
		{
			minV.minof(minV, Verts[k].Pos);
			maxV.maxof(maxV, Verts[k].Pos);			
		}
		dest.setMinMax(minV, maxV);
	}

	// build this Interface from a max mesh (usually a polygon converted to a mesh)
	bool buildFromMaxMesh(INode &node, TimeValue tvTime);
};


///////////////////////
bool CMeshInterface::buildFromMaxMesh(INode &node, TimeValue tvTime)
{
	// Get a pointer on the object's node
	ObjectState os = node.EvalWorldState(tvTime);
    Object *obj = os.obj;

	// Check if there is an object
	if (!obj) return false;
	
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
	{ 
		// Get a triobject from the node
		TriObject *tri = (TriObject*)obj->ConvertToType(tvTime, Class_ID(TRIOBJ_CLASS_ID, 0));
		
		if (!tri) return false;

		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		bool deleteIt=false;
		if (obj != tri) 
			deleteIt = true;
		Mesh &mesh = tri->GetMesh();

		// build a local to world matrix
		Matrix3 localToWorld = node.GetObjectTM(tvTime);
		CMatrix nelMatLocalToWorld;
		CExportNel::convertMatrix(nelMatLocalToWorld, localToWorld);

		// Build the vertices, setup in world
		CPolygon	poly;
		CVector		polyNormal;
		CExportNel::maxPolygonMeshToOrderedPoly(mesh, poly.Vertices, nelMatLocalToWorld, polyNormal);

		// copy to dst
		uint numVerts = poly.Vertices.size();
		Verts.resize(numVerts);
		uint k;
		for(k = 0; k < numVerts; ++k)
		{
			Verts[k].Pos= poly.Vertices[k];
		}

		// compute normals
		for(k = 0; k < numVerts; ++k)
		{
			CVector prevNorm = (Verts[k].Pos - Verts[(k + numVerts - 1) % numVerts].Pos) ^ polyNormal;
			CVector nextNorm = (Verts[(k + 1) % numVerts].Pos - Verts[k].Pos) ^ polyNormal;
			Verts[k].Normal = (prevNorm + nextNorm).normed();				
		}
		//
		if (deleteIt)
			tri->DeleteThis();
		tri = NULL;
		return true;						
	}
	return false;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// private func to apply a set of mesh Interface to a meshbuild
static void ApplyMeshInterfacesForMRM(std::vector<CMeshInterface> &interfaces, CMesh::CMeshBuild &mbuild, const NLMISC::CMatrix &toWorldMat, float threshold)
{	
	// get the mat from worldSpace to objectMat.
	CMatrix	toObjectMat= toWorldMat;
	toObjectMat.invert();
	// get the correct mat to apply to normals
	CMatrix	toObjectMatNormal= toObjectMat;
	toObjectMatNormal.setPos(CVector::Null);
	toObjectMatNormal.invert();
	toObjectMatNormal.transpose();


	// **** build Mesh Interfaces info in meshbuild
	mbuild.Interfaces.resize(interfaces.size());
	for(uint m = 0; m < interfaces.size(); ++m)
	{
		// Copy the polygon vertices/normals
		mbuild.Interfaces[m].Vertices.resize( interfaces[m].Verts.size() );
		for(uint k = 0; k < mbuild.Interfaces[m].Vertices.size(); ++k)
		{
			// back in object Space, because the CMeshInterface is in WorldSpace
			mbuild.Interfaces[m].Vertices[k].Pos= toObjectMat * interfaces[m].Verts[k].Pos;

			mbuild.Interfaces[m].Vertices[k].Normal= toObjectMatNormal * interfaces[m].Verts[k].Normal;
			mbuild.Interfaces[m].Vertices[k].Normal.normalize();
		}
	}

	// **** for every vertices, link to interfaces
	mbuild.InterfaceLinks.resize(mbuild.Vertices.size());
	for(uint k = 0; k < mbuild.Vertices.size(); ++k)
	{
		// reset
		mbuild.InterfaceLinks[k].InterfaceId= -1;

		// against each Interface
		for(uint m = 0; m < interfaces.size(); ++m)
		{
			uint	snapTo;
			if ( interfaces[m].canWeld(toWorldMat * mbuild.Vertices[k], threshold, snapTo) )
			{
				mbuild.InterfaceLinks[k].InterfaceId= m;
				mbuild.InterfaceLinks[k].InterfaceVertexId= snapTo;
				mbuild.InterfaceVertexFlag.set(k);

				// Force pack / unpack to be aligned with CMeshMRMSkinned vertices
				CMeshMRMSkinnedGeom::CPackedVertexBuffer::CPackedVertex vertex;
				vertex.setPos (mbuild.Vertices[k], NL3D_MESH_MRM_SKINNED_DEFAULT_POS_SCALE);
				vertex.getPos (mbuild.Vertices[k], NL3D_MESH_MRM_SKINNED_DEFAULT_POS_SCALE);

				break;
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// private func to apply a set of mesh Interface to a meshbuild
static void ApplyMeshInterfacesUsingInterfaceNormals(std::vector<CMeshInterface> &interfaces, CMesh::CMeshBuild &mbuild, const NLMISC::CMatrix &toWorldMat, float threshold)
{	
	// for every faces
	for(uint k = 0; k < mbuild.Faces.size(); ++k)
	{
		// test each corner
		for(uint l = 0; l < 3; ++l)
		{
			// against each Interface
			for(uint m = 0; m < interfaces.size(); ++m)
			{
				interfaces[m].snapVert(mbuild.Vertices[mbuild.Faces[k].Corner[l].Vertex],
									  mbuild.Faces[k].Corner[l].Normal,
									  toWorldMat,
									  threshold
									 );
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// a face of a node, used in a quad tree
struct CNodeFace
{
	CVector P[3];			// vertices;
	uint32  SmoothGroup;    // smoothgroup;
	void	buildBBox(NLMISC::CAABBox &dest)
	{
		CVector minV(P[0]);
		CVector maxV(P[0]);
		minV.minof(minV, P[1]);
		minV.minof(minV, P[2]);
		maxV.maxof(maxV, P[1]);
		maxV.maxof(maxV, P[2]);
		dest.setMinMax(minV, maxV);
	}
	CVector getNormal() const
	{
		return ((P[1] - P[0]) ^ (P[2] - P[1])).normed();
	}

	float getArea() const
	{
		return 0.5f * ((P[1] - P[0]) ^ (P[2] - P[0])).norm();
	}
};

typedef NL3D::CQuadGrid<CNodeFace> TNodeFaceQG;

/** Append faces from a node tree to the given quadgrid
  */ 
static void AddNodeToQuadGrid(const NLMISC::CAABBox &delimiter, TNodeFaceQG &destQuadGrid, INode &node, TimeValue time)
{	 
	CAABBox nodeBBox;
	if (CExportNel::buildMeshAABBox(node, nodeBBox, time))
	{
		if (delimiter.intersect(nodeBBox))
		{
			nldebug((std::string("Adding ") + node.GetName() + std::string(" to mesh interface quad grid")).c_str());
			// add this node tris
			ObjectState os = node.EvalWorldState(time);
			Object *obj = os.obj;	
			if (obj)
			{			
				if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
				{				
					 // Get a triobject from the node
					TriObject *tri = (TriObject*)obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));
					if (tri)
					{
						// Note that the TriObject should only be deleted
						// if the pointer to it is not equal to the object
						// pointer that called ConvertToType()
						bool deleteIt = false;
						if (obj != tri) 
							deleteIt = true;
						Mesh &mesh = tri->GetMesh();

						Matrix3   nodeMat = node.GetObjectTM(time);
						CNodeFace nodeFace;

						NLMISC::CAABBox faceBBox;

						uint numFaceAdded = 0;
						for(sint l = 0; l < mesh.getNumFaces(); ++l)
						{
							for(uint m = 0; m < 3; ++m)
							{
								Point3 pos = nodeMat * mesh.getVert(mesh.faces[l].v[m]);
								CExportNel::convertVector(nodeFace.P[m], pos);
							}
							// test if we must insert in quadgrid
							nodeFace.buildBBox(faceBBox);
							if (faceBBox.intersect(delimiter))
							{
								nodeFace.SmoothGroup = mesh.faces[l].smGroup;
								destQuadGrid.insert(faceBBox.getMin(), faceBBox.getMax(), nodeFace);
								++ numFaceAdded;
							}
							
						}	
						nldebug("%d faces where added", numFaceAdded);
						//
						if (deleteIt)
							tri->DeleteThis();
						tri = NULL;
					}
				}
			}
		}
	}

	// deals with sons
	for(sint k = 0; k < node.NumberOfChildren(); ++k)
	{
		::AddNodeToQuadGrid(delimiter, destQuadGrid, *node.GetChildNode(k), time);
	}
}


/** Build a quadgrid of all the faces in a node and its sons that are inside the given BBox    
  * The quad grid is arbitrarily oriented in the X-Z plane (has this will mainly be used with characters)
  */ 
static void BuildNodeFacesQuadGrid(const NLMISC::CAABBox &delimiter, TNodeFaceQG &destQuadGrid, INode &baseNode, TimeValue time)
{	 
	const uint numQuads = 16;
	NLMISC::CMatrix	qgBasis;	
	qgBasis.identity();
	qgBasis.setRot(NLMISC::CVector::I, NLMISC::CVector::K, - NLMISC::CVector::J, true);
	destQuadGrid.changeBase (qgBasis);
	NLMISC::CVector halfSize = delimiter.getHalfSize();
	float width = 2.f * NLMISC::maxof(halfSize.x, halfSize.y, halfSize.z);
	if (width == 0.f) width = 0.1f;
	destQuadGrid.create(numQuads, width / numQuads);
	::AddNodeToQuadGrid(delimiter, destQuadGrid, baseNode, time);
}


/// Build a normal from a list of node faces. It is assumes that all faces share at least one smoothing group
static void BuildNormalFromNodeFaces(const std::vector<const CNodeFace *> &faces, NLMISC::CVector &dest)
{
	nlassert(!faces.empty());
	dest = CVector::Null;
	for(uint k = 0; k < faces.size(); ++k)
	{
		dest += faces[k]->getArea() * faces[k]->getNormal();
	}	
	dest.normalize();
}
 


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ApplyMeshInterfacesUsingSceneNormals(INode &sceneBaseNode, std::vector<CMeshInterface> &interfaces, CMesh::CMeshBuild &mbuild, const NLMISC::CMatrix &toWorldMat, float threshold, TimeValue time)
{
	NLMISC::CMatrix toWorldMatInv = toWorldMat.inverted();
	std::vector<const CNodeFace *> candidateFaces;
	TNodeFaceQG sceneQG;
    // for each interface
	for(uint k = 0; k < interfaces.size(); ++k)
	{
		NLMISC::CAABBox iBBox;
		interfaces[k].buildBBox(iBBox);
		// extend bbox from threshold
		iBBox.extend(iBBox.getMax() + NLMISC::CVector(threshold, threshold, threshold));
		sceneQG.clear();
		::BuildNodeFacesQuadGrid(iBBox, sceneQG, sceneBaseNode, time);

		uint numWelds = 0;
		// test each corner of the meshbuild faces
		for(uint l = 0; l < mbuild.Faces.size(); ++l)
		{
			for(uint m = 0; m < 3; ++m)
			{
				candidateFaces.clear();
				const CVector &vert = toWorldMat * mbuild.Vertices[mbuild.Faces[l].Corner[m].Vertex];
				if (interfaces[k].canWeld(vert, threshold))
				{
					// find all candidate faces
					sceneQG.select(vert - NLMISC::CVector(threshold, threshold, threshold),
								   vert + NLMISC::CVector(threshold, threshold, threshold)
								  );
					TNodeFaceQG::CIterator faceIt = sceneQG.begin();
					while (faceIt != sceneQG.end())
					{
						uint32 sg = (*faceIt).SmoothGroup;
						// the face must have at least a smoothing group in common with this one
						if (((*faceIt).SmoothGroup & mbuild.Faces[l].SmoothGroup) != 0)
						{						
							// test each vertex to see if it can weld with the current corner
							for(uint n = 0; n < 3; ++n)
							{
								if (((*faceIt).P[n] - vert).norm() <= threshold)
								{
									candidateFaces.push_back(&(*faceIt));
									break;
								}
							}							
						}
						++faceIt;
					}
					if (!candidateFaces.empty()) 
					{
						::BuildNormalFromNodeFaces(candidateFaces, mbuild.Faces[l].Corner[m].Normal);
						 mbuild.Faces[l].Corner[m].Normal = toWorldMatInv.mulVector(mbuild.Faces[l].Corner[m].Normal);
						 ++ numWelds;
					}
				}
			}						
		}
		nldebug("%d vertices have been welded for interface %d", numWelds, k);
		
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** private func : Select vertices in a max mesh that match a CMeshInterface
  * It returns true if the operation was done properly
  */
static bool SelectVerticesInMeshFromInterfaces(const std::vector<CMeshInterface> &inters, float threshold, INode &node, TimeValue tvTime)
{	
	ObjectState os = node.EvalWorldState(tvTime);
    Object *obj = os.obj;
	// Check if there is an object
	if (!obj) return false;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
	{ 
		// Get a triobject from the node
		TriObject *tri = (TriObject*)obj->ConvertToType(tvTime, Class_ID(TRIOBJ_CLASS_ID, 0));			
		if (!tri) return false;
		if (obj != tri) 
		{
			// not a mesh object, so do nothing
			tri->DeleteThis();			
			tri = NULL;
			return false;
		}
		
		Mesh &mesh = tri->GetMesh();
		// unselect all vertices
		mesh.VertSel().ClearAll();

		// build the local to wordl matrix
		Matrix3 localToWorld = node.GetObjectTM(tvTime);
		CMatrix nelMatLocalToWorld;
		CExportNel::convertMatrix(nelMatLocalToWorld, localToWorld);

		mesh.selLevel = DISP_SELVERTS ;
				
		// test each vertices against the junctions
		for(uint k = 0; k < (uint) mesh.getNumVerts(); ++k)
		{
			CVector pos;
			Point3  maxPos = mesh.getVert(k);
			CExportNel::convertVector(pos, maxPos);			

			for(uint l = 0; l < inters.size(); ++l)
			{
				if (inters[l].canWeld(nelMatLocalToWorld * pos, threshold))
				{
					mesh.VertSel().Set(k);
					break;
				}
			}
		}
		mesh.SetDispFlag(DISP_SELVERTS);	
	}
	return true;				
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** private func to build a vector of meshs interface from a max file  
  * The max file is merged to this one 
  */
static bool BuildMeshInterfaces(const char *cMaxFileName, std::vector<CMeshInterface> &meshInterfaces, CExportNel &exportNel, TimeValue tvTime)
{
	std::string maxFileName(cMaxFileName);
	// make a set of current scene nodes	
	std::vector<INode *> nodes;
	exportNel.getObjectNodes(nodes, tvTime);	
	std::set<INode *> firstNodes(nodes.begin(), nodes.end());

	if  (CFile::getExtension(maxFileName).empty())
	{
		maxFileName += ".max";
	}

	/** don't know why, but a call to Interface::MergeFromFile freeze the application, so we
	  * use a call to maxscript instead
	  */
	std::string maxFileNameWithSlash;
	for(uint k = 0; k < maxFileName.length(); ++k)
	{
		if (maxFileName[k] == '\\')
		{
			maxFileNameWithSlash += "\\\\";
		}
		else
		{
			maxFileNameWithSlash += maxFileName[k];
		}
	}

	// Before merging, rename the materials if in non-echec export mode and backup there original names in a map
	map<string, string> renameMap;
	nlassert (exportNel.getInterface ());
	MtlBaseLib *lib = exportNel.getInterface ()->GetSceneMtls ();
	nlassert (lib);
	uint size = lib->Count ();
	uint i;
	for (i=0; i<size; i++)
	{
		// Rename the material
		string newName = "NelAutoMergeRenamedTmp" + toString (i);
		string originalName = (*lib)[i]->GetName ();
		renameMap.insert (map<string, string>::value_type (newName, originalName));
		(*lib)[i]->SetName (newName.c_str ());
	}

	// Merge the interface project
	bool mergeSuccess = true;
	std::string command("mergeMAXFile \"" + maxFileNameWithSlash + "\" #noRedraw #mergeDups");
	if (CExportNel::scriptEvaluate(command.c_str(), NULL, scriptNothing) == false)
	{
		nlwarning("Unable to merge %s", maxFileName.c_str());
		CExportNel::scriptEvaluate(("print \"Failed to load mesh interfaces " + maxFileNameWithSlash + "\"").c_str(), NULL, scriptNothing);
		mergeSuccess = false;
	}

	// Rename the new material name with a generics names
	nlassert (exportNel.getInterface ());
	lib = exportNel.getInterface ()->GetSceneMtls ();
	nlassert (lib);
	size = lib->Count ();

	// First, rename all the new materials in the scene
	for (i=0; i<size; i++)
	{
		// Find the name in the map ?
		string key = (*lib)[i]->GetName ();
		map<string, string>::iterator ite = renameMap.find (key);

		// Not found ? This is a merged material
		if (ite == renameMap.end ())
		{
			// Rename the material
			string newName = "NelAutoMergeRenamed" + toString (i);
			string originalName = (*lib)[i]->GetName ();
			renameMap.insert (map<string, string>::value_type (newName, originalName));
			(*lib)[i]->SetName (newName.c_str ());
		}
	}

	// Now, rename all the old materials in the scene with there original names
	for (i=0; i<size; i++)
	{
		// Find the name
		string key = (*lib)[i]->GetName ();
		map<string, string>::iterator ite = renameMap.find (key);
		if (ite != renameMap.end ())
		{
			// Rename the material with its original name
			(*lib)[i]->SetName (ite->second.c_str ());
		}
	}

	// Continue ?
	if (!mergeSuccess)
		return false;

	// make a set of current scene nodes + merged nodes
	nodes.clear();
	exportNel.getObjectNodes(nodes, tvTime);		
	std::set<INode *> allNodes(nodes.begin(), nodes.end());
	std::set<INode *> mergedNodes;


	// compute difference to get merged nodes
	std::set_difference(allNodes.begin(), allNodes.end(),
						firstNodes.begin(), firstNodes.end(),						
						std::inserter(mergedNodes, mergedNodes.begin())
					   );

	if (mergedNodes.size() == 0)
	{
		nlwarning("Couldn't find interface : %s", maxFileName.c_str());
	}
	
	// build meshs interfaces set from merged nodes
	std::set<INode *>::iterator it;
	for(it = mergedNodes.begin(); it != mergedNodes.end(); ++it)
	{
		CMeshInterface meshInterface;
		if (meshInterface.buildFromMaxMesh(**it, tvTime))
		{
			if (!meshInterface.Verts.empty())
			{			
				nldebug("adding interface %d from %s", meshInterfaces.size(), exportNel.getNelObjectName(**it).c_str());
				// well we could avoid a vector copy
				meshInterfaces.push_back(meshInterface);
			}
		}
		else
		{
			nlwarning("unable to build interface from %s", exportNel.getNelObjectName(**it).c_str());
		}
	}
	

	// remove loaded nodes from scene
	for(it = mergedNodes.begin(); it != mergedNodes.end(); ++it)
	{
		exportNel.getInterface()->DeleteNode(*it, FALSE);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CExportNel::applyInterfaceToMeshBuild(INode &node, CMesh::CMeshBuild &mbuild, const NLMISC::CMatrix &toWorldMat, TimeValue time)
{
	nldebug("===============================================");
	nldebug("Applying interface on : %s", node.GetName());
	std::string interfaceFile = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_FILE, "");
	if (interfaceFile.empty()) return;

	// get the threshold
	float threshold = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_THRESHOLD, -1.f);

	if (threshold < 0.f)
	{
		nlwarning("Invalid threshold used for interface merging, in node %s", CExportNel::getNelObjectName(node).c_str() );
		return;
	}

	std::vector<CMeshInterface> meshInterface;
	if (! ::BuildMeshInterfaces(interfaceFile.c_str(), meshInterface, *this, time))
	{
		return;
	}

	mbuild.InterfaceVertexFlag.resize(mbuild.Vertices.size());

	// store interface info in mesh for MRM, & mark vertices that are on an interface
	::ApplyMeshInterfacesForMRM(meshInterface, mbuild, toWorldMat, threshold);

	// process the mesh build to correct normal
	bool useSceneNodeNormals = (CExportNel::getScriptAppData(&node, NEL3D_APPDATA_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS, 0) != 0);
	if (!useSceneNodeNormals)
	{	
		::ApplyMeshInterfacesUsingInterfaceNormals(meshInterface, mbuild, toWorldMat, threshold);
	}
	else
	{
		::ApplyMeshInterfacesUsingSceneNormals(*CExportNel::getRootNode(), meshInterface, mbuild, toWorldMat, threshold, time);
	}	
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool	CExportNel::useInterfaceMesh(INode &node)
{
	std::string interfaceFile = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_FILE, "");
	return !interfaceFile.empty();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool	CExportNel::selectInterfaceVertices(INode &node, TimeValue time)
{
	std::string interfaceFile = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_FILE, "");
	if (interfaceFile.empty()) 
	{
		nlwarning("SelectInterfaceVertices : This node has no mesh interface : %s", this->getNelObjectName(node).c_str() );
		return false;
	}

	// get the threshold
	float threshold = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_THRESHOLD, -1.f);

	if (threshold < 0.f)
	{
		nlwarning("Invalid threshold used for interface merging, in node %s", CExportNel::getNelObjectName(node).c_str() );
		return false;
	}

	std::vector<CMeshInterface> meshInterfaces;
	if (!::BuildMeshInterfaces(interfaceFile.c_str(), meshInterfaces, *this, time))
	{
		return false;
	}

	return ::SelectVerticesInMeshFromInterfaces(meshInterfaces, threshold, node, time );			
}

























