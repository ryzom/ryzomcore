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

#include "../nel_export/std_afx.h"
#include "../nel_export/nel_export.h"
#include "../nel_export/nel_export_scene.h"

#include "nel/pacs/collision_mesh_build.h"
#include "nel/pacs/retriever_bank.h"
#include "nel/pacs/global_retriever.h"
#include "nel/pacs/build_indoor.h"
#include "nel/pacs/primitive_block.h"

#include "nel/3d/quad_grid.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;

// ***************************************************************************

typedef pair<uint, uint>	TFaceRootMeshInfo;
typedef pair<uint, bool>	TEdgeInfo;

CCollisionMeshBuild*	CExportNel::createCollisionMeshBuild(std::vector<INode *> &nodes, TimeValue tvTime)
{
	CCollisionMeshBuild *pCollisionMeshBuild = new CCollisionMeshBuild();

	uint	i, j, node;
	uint	totalVertices = 0,
			totalFaces = 0,
			totalSurfaces = 0;

	vector<uint>				rootMeshVertices;
	vector<TFaceRootMeshInfo>	facesRootMeshesInfo;
	vector<string>				rootMeshNames;

	// merge all ondes into one CCollisionMeshBuild
	for (node=0; node<nodes.size(); ++node)
	{
		// Get a pointer on the object's node
		ObjectState os = nodes[node]->EvalWorldState(tvTime);
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
					// get the mesh name
					uint	meshId = rootMeshNames.size();
					rootMeshNames.push_back(MCharStrToUtf8(nodes[node]->GetName()));
					bool	collision = getScriptAppData (nodes[node], NEL3D_APPDATA_COLLISION, 0) != 0;
					bool	exterior = getScriptAppData (nodes[node], NEL3D_APPDATA_COLLISION_EXTERIOR, 0) != 0;

					bool deleteIt=false;
					if (collision)
					{
						// Note that the TriObject should only be deleted
						// if the pointer to it is not equal to the object
						// pointer that called ConvertToType()
						if (obj != tri) 
							deleteIt = true;

						uint	i;
						Mesh	&mesh = tri->GetMesh();

						// Get the object matrix
						CMatrix ToWorldSpace;
						Matrix3 verticesToWorld = nodes[node]->GetObjectTM(tvTime);
						convertMatrix (ToWorldSpace, verticesToWorld);

						// Convert the vertices
						for (i=0; i<(uint)mesh.numVerts; ++i)
						{
							Point3 v=mesh.verts[i];
							CVector vv=ToWorldSpace*CVector (v.x, v.y, v.z);
							pCollisionMeshBuild->Vertices.push_back(vv);
							rootMeshVertices.push_back(node);
						}

						uint	maxMatId = 0;

						// Convert the faces
						for (i=0; i<(uint)mesh.numFaces; ++i)
						{
							facesRootMeshesInfo.push_back(make_pair(meshId, i));

							pCollisionMeshBuild->Faces.resize(pCollisionMeshBuild->Faces.size()+1);
							pCollisionMeshBuild->Faces.back().V[0] = mesh.faces[i].v[0]+totalVertices;
							pCollisionMeshBuild->Faces.back().V[1] = mesh.faces[i].v[1]+totalVertices;
							pCollisionMeshBuild->Faces.back().V[2] = mesh.faces[i].v[2]+totalVertices;

							pCollisionMeshBuild->Faces.back().Visibility[0] = ((mesh.faces[i].flags & EDGE_B) != 0);
							pCollisionMeshBuild->Faces.back().Visibility[1] = ((mesh.faces[i].flags & EDGE_C) != 0);
							pCollisionMeshBuild->Faces.back().Visibility[2] = ((mesh.faces[i].flags & EDGE_A) != 0);

							uint32	maxMaterialId = mesh.faces[i].getMatID();
							if (!exterior && maxMaterialId > maxMatId)
								maxMatId = maxMaterialId;
							sint32	sid = (exterior) ? -1 : totalSurfaces+maxMaterialId;

							pCollisionMeshBuild->Faces.back().Surface = sid;
							pCollisionMeshBuild->Faces.back().Material = maxMaterialId;
						}

						totalVertices = pCollisionMeshBuild->Vertices.size();
						totalFaces = pCollisionMeshBuild->Faces.size();
						totalSurfaces += maxMatId+1;
					}

					// Delete the triObject if we should...
					if (deleteIt)
						tri->DeleteThis();

				}
			}
		}
	}

	// Weld identical vertices.
	// using a grid to store indexes of vertices
	const sint		GridSize = 64;
	const float		GridWidth = 1.0f;
	const float		WeldThreshold = 0.005f;

	NL3D::CQuadGrid<uint>	grid;
	vector<uint>	remapIds;
	vector<CVector>	remapVertices;
	vector<CVector>	&vertices = pCollisionMeshBuild->Vertices;

	vector<CVector>			previousVertices = pCollisionMeshBuild->Vertices;
	vector<CCollisionFace>	previousFaces = pCollisionMeshBuild->Faces;

	grid.create(GridSize, GridWidth);
	remapIds.resize(totalVertices);

	for (i=0; i<totalVertices; ++i)
	{
		remapIds[i] = i;

		CAABBox	box;
		box.setCenter(pCollisionMeshBuild->Vertices[i]);
		box.setHalfSize(CVector(WeldThreshold, WeldThreshold, 0.0f));

		grid.insert(box.getMin(), box.getMax(), i);
	}


	for (i=0; i<totalVertices; ++i)
	{
		if (remapIds[i] != i)
			continue;

		CVector		weldTo = vertices[i];

		// select close vertices
		grid.select(vertices[i], vertices[i]);

		// for each selected vertex, remaps it to the current vertex
		NL3D::CQuadGrid<uint>::CIterator	it;
		for (it=grid.begin(); it!=grid.end(); ++it)
		{
			uint	weldedId = *it;
			CVector		welded = vertices[weldedId];

			if (weldedId <= i || rootMeshVertices[i] == rootMeshVertices[weldedId] ||
				remapIds[weldedId] != weldedId || (welded-weldTo).norm() > WeldThreshold)
				continue;

			remapIds[weldedId] = i;
		}
	}

	for (i=0; i<totalVertices; ++i)
	{
		if (remapIds[i] > i)
			nlerror("found a greater remap id");

		if (remapIds[i] == i)
		{
			uint	newId = remapVertices.size();
			remapVertices.push_back(vertices[i]);
			remapIds[i] = newId;
		}
		else
		{
			remapIds[i] = remapIds[remapIds[i]];
		}
	}

	for (i=0; i<totalFaces; ++i)
		for (j=0; j<3; ++j)
			pCollisionMeshBuild->Faces[i].V[j] = remapIds[pCollisionMeshBuild->Faces[i].V[j]];

	// check for errors
	vector<string>	warnings;
	for (i=0; i<totalFaces; ++i)
	{
		if (pCollisionMeshBuild->Faces[i].V[0] == pCollisionMeshBuild->Faces[i].V[1] ||
			pCollisionMeshBuild->Faces[i].V[1] == pCollisionMeshBuild->Faces[i].V[2] ||
			pCollisionMeshBuild->Faces[i].V[2] == pCollisionMeshBuild->Faces[i].V[0])
		{
			warnings.push_back(string("mesh:") + rootMeshNames[facesRootMeshesInfo[i].first] + string(" face:") + toString(facesRootMeshesInfo[i].second));
		}
	}

	// and clean up the mesh if some errors appear
	vector<CCollisionFace>::iterator	it;
	for (it=pCollisionMeshBuild->Faces.begin(); it!=pCollisionMeshBuild->Faces.end(); )
	{
		if ((*it).V[0] == (*it).V[1] ||
			(*it).V[1] == (*it).V[2] ||
			(*it).V[2] == (*it).V[0])
		{
			it = pCollisionMeshBuild->Faces.erase(it);
		}
		else
		{
			++it;
		}
	}

	pCollisionMeshBuild->Vertices = remapVertices;

	// check bbox size
	CAABBox	box;
	if (!pCollisionMeshBuild->Vertices.empty())
	{
		box.setCenter(pCollisionMeshBuild->Vertices[0]);
		for (i=1; i<pCollisionMeshBuild->Vertices.size(); ++i)
			box.extend(pCollisionMeshBuild->Vertices[i]);
	}

	CVector	hs = box.getHalfSize();
	if (hs.x > 255.0f || hs.y > 255.0f)
		outputErrorMessage ("The bounding box of the selection exceeds 512 meters large!");
		
	// report warnings
	if (!warnings.empty())
	{
		string	message = "Warning(s) occurred during collision export\n(defective links may result) error";
		for (i=0; i<warnings.size(); ++i)
			message += string("\n")+warnings[i];

		outputWarningMessage ((message+"\n\n(This message was copied in the clipboard)").c_str());

		if (OpenClipboard (NULL))
		{
			HGLOBAL mem = GlobalAlloc (GHND|GMEM_DDESHARE, message.size()+1);
			if (mem)
			{
				char *pmem = (char *)GlobalLock (mem);
				strcpy (pmem, message.c_str());
				GlobalUnlock (mem);
				EmptyClipboard ();
				SetClipboardData (CF_TEXT, mem);
			}
			CloseClipboard ();
		}
	}

	vector<string>	errors;
	pCollisionMeshBuild->link(false, errors);
	pCollisionMeshBuild->link(true, errors);
	// report warnings
	if (!errors.empty())
	{
		string	message = "Error(s) occurred during collision export\n(edge issues)";
		for (i=0; i<errors.size(); ++i)
			message += string("\nERROR: ")+errors[i];

		outputErrorMessage ((message+"\n\n(This message was copied in the clipboard)").c_str());

		if (OpenClipboard (NULL))
		{
			HGLOBAL mem = GlobalAlloc (GHND|GMEM_DDESHARE, message.size()+1);
			if (mem)
			{
				char *pmem = (char *)GlobalLock (mem);
				strcpy (pmem, message.c_str());
				GlobalUnlock (mem);
				EmptyClipboard ();
				SetClipboardData (CF_TEXT, mem);
			}
			CloseClipboard ();
		}

		delete pCollisionMeshBuild;
		pCollisionMeshBuild = NULL;
	}

	// Return the shape pointer or NULL if an error occurred.
	return pCollisionMeshBuild;
}


// ***************************************************************************
bool	CExportNel::createCollisionMeshBuildList(std::vector<INode *> &nodes, TimeValue time,
	std::vector<std::pair<std::string, NLPACS::CCollisionMeshBuild*> > &meshBuildList)
{
	nlassert(meshBuildList.size()==0);
	// Result to return
	bool bRet=false;

	// Eval the objects a time
	uint	i, j;

	for (i=0; i<nodes.size(); ++i)
	{
		ObjectState os = nodes[i]->EvalWorldState(time);
		if (!os.obj)
			return bRet;
	}

	std::vector<std::pair<std::string, std::vector<INode *> > >	igs;
	for (i=0; i<nodes.size(); ++i)
	{
		// Object is flagged as a collision?
		int	bCol= getScriptAppData(nodes[i], NEL3D_APPDATA_COLLISION, BST_UNCHECKED);
		if(bCol == BST_CHECKED)
		{
			// If yes, add it to list
			std::string	ig = CExportNel::getScriptAppData(nodes[i], NEL3D_APPDATA_IGNAME, "");
			if (ig == "")
				ig = "unknown_ig";

			for (j=0; j<igs.size() && ig!=igs[j].first; ++j)
				;
			if (j == igs.size())
			{
				igs.push_back (std::pair<std::string, std::vector<INode *> >());
				igs[j].first = ig;
			}

			igs[j].second.push_back(nodes[i]);
		}
	}


	for (i=0; i<igs.size(); ++i)
	{
		std::string				igname = igs[i].first;
		std::vector<INode *>	&ignodes = igs[i].second;
		// Object exist ?
		CCollisionMeshBuild	*pCmb = CExportNel::createCollisionMeshBuild(ignodes, time);

		// Conversion success ?
		if (pCmb)
		{
			meshBuildList.push_back(make_pair(igname, pCmb));
			
			// All is good
			bRet=true;
		}
	}

	return bRet;
}


// ***************************************************************************
void	CExportNel::computeCollisionRetrieverFromScene(TimeValue time, 
	CRetrieverBank *&retrieverBank, CGlobalRetriever *&globalRetriever,
	const char *igNamePrefix, const char *igNameSuffix, std::string &retIgName)
{
	// Default: empty retrieverBank/globalRetriever
	retrieverBank= NULL;
	globalRetriever= NULL;
	retIgName.clear();

	// get list of nodes from scene
	std::vector<INode*>	nodes;
	getObjectNodes (nodes, time);

	// build list of cmb.
	std::vector<std::pair<std::string, NLPACS::CCollisionMeshBuild*> >	meshBuildList;
	if( createCollisionMeshBuildList(nodes, time, meshBuildList) && meshBuildList.size()>0 )
	{
		// create a retriverBnak and a global retrevier.
		retrieverBank= new CRetrieverBank;
		globalRetriever= new CGlobalRetriever;
		// must init default grid.
		globalRetriever->init();

		// list of valid instance to create.
		vector<pair<uint32, CVector> >	retrieverInstances;

		// fill the retrieverBank
		uint	i;
		for(i=0; i<meshBuildList.size();i++)
		{
			std::string				igname = meshBuildList[i].first;
			CCollisionMeshBuild		*pCmb = meshBuildList[i].second;

			// compute a localRetriever 
			CLocalRetriever		lr;
			CVector				translation;
			string				error;
			if( NLPACS::computeRetriever(*pCmb, lr, translation, error ) )
			{
				// set his id to the igname.
				lr.setIdentifier(igname);

				// Force Loaded State since computed here!
				lr.forceLoaded(true);

				// Add to the retrieverBank
				uint32	lrId= retrieverBank->addRetriever(lr);

				// add this valid retrieverInstnace.
				retrieverInstances.push_back(make_pair(lrId, translation));
			}

			// free the CCollisionMeshBuild.
			delete pCmb;
			pCmb = NULL;

			// does igname match prefix/suffix???
			if(igname.find(igNamePrefix)==0)
			{
				uint	lenPrefix= strlen(igNamePrefix);
				sint	endPos;
				// if no suffix
				if(string(igNameSuffix).empty())
				{
					endPos= igname.size();
				}
				else
				{
					endPos= igname.find(igNameSuffix, lenPrefix);
				}
				// if found suffix, or empty suffix.
				if(endPos!=string::npos)
				{
					// Yes => setup the name between prefix/suffix
					retIgName= igname.substr(lenPrefix, endPos-lenPrefix);
				}
			}
		}

		// fill the globalRetriever with all instances created.
		globalRetriever->setRetrieverBank(retrieverBank);
		for(i=0; i<retrieverInstances.size();i++)
		{
			// must set -translation
			globalRetriever->makeInstance(retrieverInstances[i].first, 0, -retrieverInstances[i].second);
		}
		// compile the globalRetriever.
		globalRetriever->initQuadGrid();
		globalRetriever->makeAllLinks();
	}

}

// ***************************************************************************
float	CExportNel::getZRot (const NLMISC::CVector &i)
{
	// Assume that tm.getK() is near CVector::K

	// Get vectors
	CVector _i = i;

	// Normalize I
	_i.z = 0;
	_i.normalize ();

	// Get cos a
	float cosa = _i * CVector::I;
	float sina = (CVector::I ^ _i) * CVector::K;

	// Get a
	return (sina>0) ? (float) acos (cosa) : (float)(2*Pi - acos (cosa));
}

// ***************************************************************************
bool	CExportNel::buildPrimitiveBlock (TimeValue time, std::vector<INode*> objects, NLPACS::CPrimitiveBlock &primitiveBlock)
{
	// Reserve some memory
	primitiveBlock.Primitives.clear ();
	primitiveBlock.Primitives.resize (objects.size());

	// Ok ?
	bool ok = true;

	// For each object
	uint o;
	for (o=0; o<objects.size(); o++)
	{
		// Get a ref on the node
		INode *node = objects[o];

		// Select the node
		_Ip->SelectNode (node);

		// Get a pointer on the object's node
		//Object *obj = node->EvalWorldState(time).obj;
		Object *obj = node->GetObjectRef ();

		// Check if there is an object
		if (obj)
		{
			// Get the class id
			Class_ID  clid = obj->ClassID();

			// Is the object a PACS primitive ?
			if ( ( (clid.PartA() == NEL_PACS_BOX_CLASS_ID_A) && (clid.PartB() == NEL_PACS_BOX_CLASS_ID_B) ) ||
				 ( (clid.PartA() == NEL_PACS_CYL_CLASS_ID_A) && (clid.PartB() == NEL_PACS_CYL_CLASS_ID_B) ) )
			{
				// Retrieve common parameters
				int reaction;
				int enterTrigger;
				int exitTrigger;
				int overlap;
				uint collision;
				uint occlusion;
				int obstacle;
				uint userdata0;
				uint userdata1;
				uint userdata2;
				uint userdata3;
				float absorbtion;
				bool error = 
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "Reaction", (ParamType2)TYPE_INT, &reaction, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "Obstacle", (ParamType2)TYPE_BOOL, &obstacle, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "EnterTrigger", (ParamType2)TYPE_BOOL, &enterTrigger, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "ExitTrigger", (ParamType2)TYPE_BOOL, &exitTrigger, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "OverlapTrigger", (ParamType2)TYPE_BOOL, &overlap, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "CollisionMask", (ParamType2)TYPE_INT, &collision, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "OcclusionMask", (ParamType2)TYPE_INT, &occlusion, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "UserData0", (ParamType2)TYPE_INT, &userdata0, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "UserData1", (ParamType2)TYPE_INT, &userdata1, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "UserData2", (ParamType2)TYPE_INT, &userdata2, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "UserData3", (ParamType2)TYPE_INT, &userdata3, 0)) ||
					(!CExportNel::getValueByNameUsingParamBlock2(*node, "Absorbtion", (ParamType2)TYPE_FLOAT, &absorbtion, 0));

				// Get the node matrix
				CMatrix mt;
				convertMatrix (mt, node->GetNodeTM (time));

				// Retrieve specific parameters
				float height;
				float length[2];
				float orientation;
				if ( (clid.PartA() == NEL_PACS_BOX_CLASS_ID_A) && (clid.PartB() == NEL_PACS_BOX_CLASS_ID_B) )
				{
					// For boxes
					nlverify (scriptEvaluate ("$.box.height", &height, scriptFloat));
					nlverify (scriptEvaluate ("$.box.width", &length[0], scriptFloat));
					nlverify (scriptEvaluate ("$.box.length", &length[1], scriptFloat));
					
					// Get the orientation
					orientation = getZRot (mt.getI());
				}
				else
				{
					// For cylinders
					nlverify (scriptEvaluate ("$.cylinder.height", &height, scriptFloat));
					nlverify (scriptEvaluate ("$.cylinder.radius", &length[0], scriptFloat));
					length[1] = 0;
					orientation = 0;
				}

				// No error?
				if (!error)
				{
					// Fill the structure
					CPrimitiveDesc &desc = primitiveBlock.Primitives[o];
					desc.Length[0] = length[0];
					desc.Length[1] = length[1];
					desc.Height = height;
					desc.Attenuation = absorbtion;
					desc.Type = ( (clid.PartA() == NEL_PACS_BOX_CLASS_ID_A) && (clid.PartB() == NEL_PACS_BOX_CLASS_ID_B) ) ? 
						UMovePrimitive::_2DOrientedBox : UMovePrimitive::_2DOrientedCylinder;
					desc.Reaction = (UMovePrimitive::TReaction)((reaction-1) << 4);
					desc.Trigger = (UMovePrimitive::TTrigger)
						(((enterTrigger!=0)?UMovePrimitive::EnterTrigger:0) |
						((exitTrigger!=0)?UMovePrimitive::ExitTrigger:0) |
						((overlap!=0)?UMovePrimitive::OverlapTrigger:0));
					desc.Obstacle = obstacle != 0;
					desc.OcclusionMask = occlusion;
					desc.CollisionMask = collision;
					desc.Position = mt.getPos ();
					desc.Orientation = orientation;
					desc.UserData = ((uint64)userdata0) | (((uint64)userdata1)<<16) | (((uint64)userdata2)<<32) | (((uint64)userdata3)<<48);
				}
				else
				{
					nlwarning ("Some properties are missing in objects \"%s\"", node->GetName());
					ok = false;
				}
			}
			else
			{
				nlwarning ("\"%s\" is not a PACS primitive", node->GetName());
				ok = false;
			}
		}
		else
		{
			nlwarning ("Can't evaluate object \"%s\"", node->GetName());
			ok = false;
		}
	}

	// Failed
	if (ok)
	{
		_Ip->ForceCompleteRedraw ();
	}
	else
	{
		primitiveBlock.Primitives.clear ();
	}

	return ok;
}


