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

#include "nel/misc/path.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/mesh.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/3d/stripifier.h"
#include "nel/3d/register_3d.h"


using namespace NLMISC;
using namespace NL3D;
using namespace std;


// ***************************************************************************

const CIndexBuffer *getRdrPassPrimitiveBlock(const CMeshMRMGeom *mesh, uint lodId, uint renderPass)
{
	return &(mesh->getRdrPassPrimitiveBlock(lodId, renderPass));
}

// ***************************************************************************

const CIndexBuffer *getRdrPassPrimitiveBlock(const CMeshMRMSkinnedGeom *mesh, uint lodId, uint renderPass)
{
	static CIndexBuffer block;
	mesh->getRdrPassPrimitiveBlock(lodId, renderPass, block);
	return &block;
}

// ***************************************************************************

template<class T>
void		addShadowMesh(T *meshIn, float paramFaceRatio, sint paramMaxFace, const std::vector<CMesh::CSkinWeight> &skinWeights, const CVertexBuffer &vertexBuffer)
{
	CVertexBufferRead vba;
	vertexBuffer.lock (vba);
	uint	i, j;

	// **** Select the Lod.
	uint	numLods= meshIn->getNbLod();

	// get the max tris displayed
	float	numMeshFacesMin= (float)meshIn->getLevelDetail().MinFaceUsed;
	float	numMeshFacesMax= (float)meshIn->getLevelDetail().MaxFaceUsed;
	float	maxFaceWanted= numMeshFacesMax * paramFaceRatio;
	// minimize with maxFace param
	maxFaceWanted= min(maxFaceWanted, (float)paramMaxFace);
	// find the lod
	float	fLod= ( (maxFaceWanted-numMeshFacesMin) / (numMeshFacesMax-numMeshFacesMin) )*(numLods-1);
	// round the lod wanted.
	sint lodId= (sint)floor(0.5f + fLod);
	clamp(lodId, 0, (sint)numLods-1);


	// **** First, for the best lod indicate what vertex is used or not. Also index geomorphs to know what real vertex is used
	vector<sint>		vertexUsed;
	// -1 means "not used"
	vertexUsed.resize(skinWeights.size(), -1);
	// Parse all triangles.
	for(i=0;i<meshIn->getNbRdrPass(lodId);i++)
	{
		const CIndexBuffer *pb = getRdrPassPrimitiveBlock(meshIn, lodId, i);
		CIndexBufferRead iba;
		pb->lock (iba);
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			const uint32	*triPtr= (const uint32 *) iba.getPtr();
			for(j=0;j<pb->getNumIndexes();j++)
			{
				uint	idx= *triPtr;
				// Flag the vertex with its own index => used.
				vertexUsed[idx]= idx;
				triPtr++;
			}
		}
		else
		{
			const uint16	*triPtr= (const uint16 *) iba.getPtr();
			for(j=0;j<pb->getNumIndexes();j++)
			{
				uint	idx= *triPtr;
				// Flag the vertex with its own index => used.
				vertexUsed[idx]= idx;
				triPtr++;
			}
		}
	}
	// Special for Geomorphs: must take The End target vertex.
	const std::vector<CMRMWedgeGeom>	&geomorphs= meshIn->getGeomorphs(lodId);
	for(i=0;i<geomorphs.size();i++)
	{
		uint	trueIdx= geomorphs[i].End;
		// map to the Geomorph Target.
		vertexUsed[i]= trueIdx;
		// mark also the real vertex used as used.
		vertexUsed[trueIdx]= trueIdx;
	}


	// **** For all vertices used (not geomorphs), compute vertex Skins.
	vector<CShadowVertex>		shadowVertices;
	vector<sint>				vertexToVSkin;
	vertexToVSkin.resize(vertexUsed.size());
	shadowVertices.reserve(vertexUsed.size());
	// use a map to remove duplicates (because of UV/normal discontinuities before!!)
	map<CShadowVertex, uint>	shadowVertexMap;
	uint						numMerged= 0;
	// Skip Geomorphs.
	for(i=(uint)geomorphs.size();i<vertexUsed.size();i++)
	{
		// If this vertex is used.
		if(vertexUsed[i]!=-1)
		{
			// Build the vertex
			CShadowVertex	shadowVert;
			shadowVert.Vertex= *(CVector*)vba.getVertexCoordPointer(i);
			// Select the best Matrix.
			CMesh::CSkinWeight		sw= skinWeights[i];
			float	maxW= 0;
			uint	matId= 0;
			for(j=0;j<NL3D_MESH_SKINNING_MAX_MATRIX;j++)
			{
				// if no more matrix influenced, stop
				if(sw.Weights[j]==0)
					break;
				if(sw.Weights[j]>maxW)
				{
					matId= sw.MatrixId[j];
					maxW= sw.Weights[j];
				}
			}
			shadowVert.MatrixId= matId;

			// If dont find the shadowVertex in the map.
			map<CShadowVertex, uint>::iterator		it= shadowVertexMap.find(shadowVert);
			if(it==shadowVertexMap.end())
			{
				// Append
				uint	index= (uint)shadowVertices.size();
				vertexToVSkin[i]= index;
				shadowVertices.push_back(shadowVert);
				shadowVertexMap.insert(make_pair(shadowVert, index));
			}
			else
			{
				// Ok, map.
				vertexToVSkin[i]= it->second;
				numMerged++;
			}
		}
	}

	// Some Info on what have been merged.
	if(shadowVertices.size())
	{
		// TestYoyo.
		/*nlinfo("%d Vertices have been merged. => %d %%", numMerged, 
			100*numMerged / (numMerged+shadowVertices.size()));*/
	}


	// **** Get All Faces 
	// Final List Of Triangles that match the bone.
	vector<uint32>			shadowTriangles;
	shadowTriangles.reserve(1000);
	// Parse all input tri of the mesh.
	for(i=0;i<meshIn->getNbRdrPass(lodId);i++)
	{
		const CIndexBuffer *pb = getRdrPassPrimitiveBlock(meshIn, lodId, i);
		CIndexBufferRead iba;
		pb->lock (iba);
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			const uint32	*triPtr= (const uint32 *) iba.getPtr();
			for(j=0;j<pb->getNumIndexes();j++)
			{
				uint	idx= *triPtr;
				// Get the real Vertex (ie not the geomporhed one).
				idx= vertexUsed[idx];
				// Get the ShadowVertex associated
				idx= vertexToVSkin[idx];

				shadowTriangles.push_back(idx);
				triPtr++;
			}
		}
		else
		{
			const uint16	*triPtr= (const uint16 *) iba.getPtr();
			for(j=0;j<pb->getNumIndexes();j++)
			{
				uint	idx= *triPtr;
				// Get the real Vertex (ie not the geomporhed one).
				idx= vertexUsed[idx];
				// Get the ShadowVertex associated
				idx= vertexToVSkin[idx];

				shadowTriangles.push_back(idx);
				triPtr++;
			}
		}
	}

	// Re-Optim VertexCache Hard usage
	if(shadowTriangles.size())
	{
		CIndexBuffer		pb;
		// fill
		pb.setNumIndexes((uint32)shadowTriangles.size());
		{
			CIndexBufferReadWrite iba;
			pb.lock (iba);
			for(i=0;i<shadowTriangles.size()/3;i++)
			{
				iba.setTri(i*3, shadowTriangles[i*3 + 0],
							 shadowTriangles[i*3 + 1],
							 shadowTriangles[i*3 + 2]);
			}
		}
		// optimize
		CStripifier		stripifier;
		stripifier.optimizeTriangles(pb, pb);
		// get.
		{
			CIndexBufferReadWrite iba;
			pb.lock (iba);
			if (iba.getFormat() == CIndexBuffer::Indices32)
			{
				const uint32	*triPtr= (const uint32 *) iba.getPtr();
				for(i=0;i<shadowTriangles.size();i++)
				{
					shadowTriangles[i]= *triPtr;
					triPtr++;
				}
			}
			else
			{
				const uint16	*triPtr= (const uint16 *) iba.getPtr();
				for(i=0;i<shadowTriangles.size();i++)
				{
					shadowTriangles[i]= *triPtr;
					triPtr++;
				}
			}
		}
	}

	// set
	meshIn->setShadowMesh(shadowVertices, shadowTriangles);
}

// ***************************************************************************

void		addShadowMeshIntro(CMeshMRMGeom *meshIn, float paramFaceRatio, sint paramMaxFace)
{
	const std::vector<CMesh::CSkinWeight>	&skinWeights= meshIn->getSkinWeights();
	const CVertexBuffer						&vertexBuffer= meshIn->getVertexBuffer();
	addShadowMesh (meshIn, paramFaceRatio, paramMaxFace, skinWeights, vertexBuffer);
}

// ***************************************************************************

void		addShadowMeshIntro(CMeshMRMSkinnedGeom *meshIn, float paramFaceRatio, sint paramMaxFace)
{
	std::vector<CMesh::CSkinWeight>	skinWeights;
	meshIn->getSkinWeights(skinWeights);
	CVertexBuffer vertexBuffer;
	meshIn->getVertexBuffer(vertexBuffer);
	addShadowMesh (meshIn, paramFaceRatio, paramMaxFace, skinWeights, vertexBuffer);
}

// ***************************************************************************

int		main(int argc, char *argv[])
{
	// Filter addSearchPath
	NLMISC::createDebug();
	NLMISC::InfoLog->addNegativeFilter ("adding the path");

	NL3D::registerSerial3d();

	if (argc <3 )
	{
		string	execName= CFile::getFilename(argv[0]);
		printf("%s add a ShadowSkin to a Skinned MRM Mesh\n", execName.c_str());
		printf("   usage: %s shape_in shape_out [facePercentage] [maxFaces] \n", execName.c_str());
		printf("   NB: shape_in and shape_out can be same file\n");
		printf("   NB: facePercentage is a number between 0 and 100. It computes the maxFaces for the shadow according to the input Mesh.\n");
		printf("   NB: maxFaces can be given manually. The min of the 2 parameters is taken.\n");
		exit(-1);
	}

	// Parse args.
	string	shapeNameIn= argv[1];
	string	shapeNameOut= argv[2];
	sint	percentageFace= 100;
	sint	maxFace= INT_MAX;
	if(argc>=4)
		NLMISC::fromString(argv[3], percentageFace);
	if(argc>=5)
		NLMISC::fromString(argv[4], maxFace);

	// **** Load the Mesh In.
	IShape *pResult = NULL;
	CMeshMRM	*pMesh;
	CShapeStream	shapeStreamIn;
	CIFile		shapeFileIn;
	if(!shapeFileIn.open(shapeNameIn))
	{
		nlwarning("ERROR: cannot open input file: %s", shapeNameIn.c_str());
		exit(-1);
	}
	shapeStreamIn.serial(shapeFileIn);
	pMesh= dynamic_cast<CMeshMRM*>(shapeStreamIn.getShapePointer());
	shapeFileIn.close();
	if(!pMesh)
	{
		CMeshMRMSkinned *pMeshSkinned= dynamic_cast<CMeshMRMSkinned*>(shapeStreamIn.getShapePointer());
		if(!pMeshSkinned)
		{
			nlwarning("ERROR: Not a MRM Mesh nor a MRM Mesh skinned: %s", shapeNameIn.c_str());
			exit(-1);
		}

		CMeshMRMSkinnedGeom	*pMeshGeom= (CMeshMRMSkinnedGeom*)&pMeshSkinned->getMeshGeom();

		// **** Add Shadow mesh.
		float	faceRatio;
		clamp(percentageFace, 0, 100);
		faceRatio= (float)percentageFace/100;
		maxFace= max(0, maxFace);
		addShadowMeshIntro(pMeshGeom, faceRatio, maxFace);
		pResult = pMeshSkinned;
	}
	else
	{
		CMeshMRMGeom	*pMeshGeom= (CMeshMRMGeom*)&pMesh->getMeshGeom();
		if( !pMeshGeom->isSkinned() )
		{
			nlwarning("ERROR: Not a Skinned MRM Mesh: %s", shapeNameIn.c_str());
			exit(-1);
		}

		// **** Add Shadow mesh.
		float	faceRatio;
		clamp(percentageFace, 0, 100);
		faceRatio= (float)percentageFace/100;
		maxFace= max(0, maxFace);
		addShadowMeshIntro(pMeshGeom, faceRatio, maxFace);
		pResult = pMesh;
	}

	// **** Save It
	CShapeStream	shapeStreamOut;
	COFile		shapeFileOut;
	if(!shapeFileOut.open(shapeNameOut) )
	{
		nlwarning("ERROR: cannot open output file: %s", shapeNameOut.c_str());
		exit(-1);
	}

	nlassert (pResult);
	shapeStreamOut.setShapePointer(pResult);
	shapeStreamOut.serial(shapeFileOut);
	shapeFileOut.close();

	return 0;
}
