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

#include <nel/misc/file.h>
#include <nel/3d/mesh.h>
#include <nel/3d/mesh_mrm.h> 
#include <nel/3d/mesh_mrm_skinned.h> 
#include <nel/3d/scene.h> 
#include <nel/3d/register_3d.h>
#include <nel/misc/app_context.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/i_xml.h>

#include <fstream>
#include <iostream>

using namespace NLMISC;
using namespace NL3D;
using namespace std;

struct CVertex
{
	CVector vertex;
	CVector normal;
	CUV uv;
};

bool operator == (const CVertex &v1, const CVertex &v2)
{
	return (v1.vertex == v2.vertex) && (v1.normal == v2.normal) && (v1.uv == v2.uv);
}

bool operator < (const CVertex &v1, const CVertex &v2)
{
	return (v1.vertex < v2.vertex);
}

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

bool ProcessMeshMRMSkinned(const std::string &filename, IShape *shapeMesh);
bool ProcessMeshMRM(const std::string &filename, IShape *shapeMesh);
//bool ProcessMesh(const std::string &filename, IShape *shapeMesh);

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Syntax : shape2obj <NeL .shape file>" << endl;

		return 1;
	}

	if (!NLMISC::INelContext::isContextInitialised()) new NLMISC::CApplicationContext();

	registerSerial3d();
	CScene::registerBasics();

	IShape *shapeMesh = NULL;

	CIFile ifile;

	// Sream a shape
	CShapeStream streamShape;

	string filename = argv[1];

	if (!ifile.open(filename)) return 1;

	try
	{
		// Stream it
		streamShape.serial(ifile);

		// Add the shape
		shapeMesh = streamShape.getShapePointer();
	}
	catch (Exception& e)
	{
		cout << "Error : " << e.what() << endl;

		return 1;
	}

	if (ProcessMeshMRMSkinned(filename, shapeMesh)) return 0;
	if (ProcessMeshMRM(filename, shapeMesh)) return 0;
//	if (ProcessMesh(filename, shapeMesh)) return 0;

	return 0;
}

bool ProcessMeshMRMSkinned(const std::string &filename, IShape *shapeMesh)
{
	CMeshMRMSkinned *mesh = dynamic_cast<CMeshMRMSkinned*>(shapeMesh);

	if (!mesh) return false;

	COFile ofile;

	CMeshMRMSkinnedGeom* meshIn = (CMeshMRMSkinnedGeom*)&mesh->getMeshGeom();

	std::vector<CMesh::CSkinWeight>	skinWeights;
	meshIn->getSkinWeights(skinWeights);
	CVertexBuffer vertexBuffer;
	meshIn->getVertexBuffer(vertexBuffer);

	CVertexBufferRead vba;
	vertexBuffer.lock (vba);
	uint	i, j;

	// **** Select the Lod.
	uint	numLods= meshIn->getNbLod();

	// get the max tris displayed
	float	numMeshFacesMin= (float)meshIn->getLevelDetail().MinFaceUsed;
	float	numMeshFacesMax= (float)meshIn->getLevelDetail().MaxFaceUsed;
	// find the lod
	sint lodId = numLods-1;

	// **** First, for the best lod indicate what vertex is used or not. Also index geomorphs to know what real vertex is used
	vector<sint>		vertexUsed;
	// -1 means "not used"
	vertexUsed.resize(skinWeights.size(), -1);
	// Parse all triangles.
	for(i=0;i<meshIn->getNbRdrPass(lodId); ++i)
	{
		const CIndexBuffer *pb = getRdrPassPrimitiveBlock(meshIn, lodId, i);
		CIndexBufferRead iba;
		pb->lock (iba);
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			const uint32	*triPtr= (const uint32 *) iba.getPtr();
			for(j=0;j<pb->getNumIndexes(); ++j)
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
			for(j=0;j<pb->getNumIndexes(); ++j)
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
	for(i=0;i<geomorphs.size(); ++i)
	{
		uint	trueIdx= geomorphs[i].End;
		// map to the Geomorph Target.
		vertexUsed[i]= trueIdx;
		// mark also the real vertex used as used.
		vertexUsed[trueIdx]= trueIdx;
	}


	// **** For all vertices used (not geomorphs), compute vertex Skins.
	vector<CVertex>		shadowVertices;
	vector<sint>		vertexToVSkin;
	vertexToVSkin.resize(vertexUsed.size());
	shadowVertices.reserve(vertexUsed.size());
	// use a map to remove duplicates (because of UV/normal discontinuities before!!)
	map<CVertex, uint>	shadowVertexMap;
	uint						numMerged= 0;
	// Skip Geomorphs.
	for(i=geomorphs.size();i<vertexUsed.size(); ++i)
	{
		// If this vertex is used.
		if(vertexUsed[i]!=-1)
		{
			// Build the vertex
			CVertex shadowVert;
			CUV uv;
			shadowVert.vertex = *(CVector*)vba.getVertexCoordPointer(i);
			shadowVert.normal = *(CVector*)vba.getNormalCoordPointer(i);
			shadowVert.uv = *(CUV*)vba.getTexCoordPointer(i);
/*
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
//			shadowVert.MatrixId= matId;
*/
			// If dont find the shadowVertex in the map.
			map<CVertex, uint>::iterator		it= shadowVertexMap.find(shadowVert);
			if(it==shadowVertexMap.end())
			{
				// Append
				uint	index= shadowVertices.size();
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

	ofstream ofs(string(filename + ".obj").c_str());

	for(size_t y = 0; y < shadowVertices.size(); ++y)
	{
		CVector v = shadowVertices[y].vertex;
		CVector vn = shadowVertices[y].normal;
		CUV vt = shadowVertices[y].uv;

		ofs << "v " << v.x << " " << v.y << " " << v.z << endl;
		ofs << "vn " << vn.x << " " << vn.y << " " << vn.z << endl;
		ofs << "vt " << vt.U << " " << vt.V << endl;
	}

	// **** Get All Faces 
	// Final List Of Triangles that match the bone.
	vector<uint32>			shadowTriangles;
	shadowTriangles.reserve(1000);
	// Parse all input tri of the mesh.
	for(i=0; i<meshIn->getNbRdrPass(lodId); ++i)
	{
		ofs << "g pass" << i << endl;

		const CIndexBuffer *pb = getRdrPassPrimitiveBlock(meshIn, lodId, i);
		CIndexBufferRead iba;
		pb->lock (iba);
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			const uint32	*triPtr= (const uint32 *) iba.getPtr();

			for(j=0; j<pb->getNumIndexes(); ++j)
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
			for(j=0; j<pb->getNumIndexes(); ++j)
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

		for(size_t pass = 0; pass<shadowTriangles.size(); pass += 3)
		{
			ofs << "f " << shadowTriangles[pass]+1 << "/" << shadowTriangles[pass]+1 << "/" << shadowTriangles[pass]+1 << " ";
			ofs << shadowTriangles[pass+1]+1 << "/" << shadowTriangles[pass+1]+1 << "/" << shadowTriangles[pass+1]+1 << " ";
			ofs << shadowTriangles[pass+2]+1 << "/" << shadowTriangles[pass+2]+1 << "/" << shadowTriangles[pass+2]+1 << endl;
		}

		shadowTriangles.clear();
	}

	ofs.close();

	return true;
}

bool ProcessMeshMRM(const std::string &filename, IShape *shapeMesh)
{
	CMeshMRM *mesh = dynamic_cast<CMeshMRM*>(shapeMesh);

	if (!mesh) return false;

	COFile ofile;

	CMeshMRMGeom* meshIn = (CMeshMRMGeom*)&mesh->getMeshGeom();

	std::vector<CMesh::CSkinWeight>	skinWeights = meshIn->getSkinWeights();
	CVertexBuffer vertexBuffer = meshIn->getVertexBuffer();

	CVertexBufferRead vba;
	vertexBuffer.lock (vba);
	uint	i, j;

	// **** Select the Lod.
	uint	numLods= meshIn->getNbLod();

	// get the max tris displayed
	float	numMeshFacesMin= (float)meshIn->getLevelDetail().MinFaceUsed;
	float	numMeshFacesMax= (float)meshIn->getLevelDetail().MaxFaceUsed;
	// find the lod
	sint lodId = numLods-1;

	// **** First, for the best lod indicate what vertex is used or not. Also index geomorphs to know what real vertex is used
	vector<sint>		vertexUsed;
	// -1 means "not used"
	vertexUsed.resize(skinWeights.size(), -1);
	// Parse all triangles.
	for(i=0;i<meshIn->getNbRdrPass(lodId); ++i)
	{
		const CIndexBuffer *pb = getRdrPassPrimitiveBlock(meshIn, lodId, i);
		CIndexBufferRead iba;
		pb->lock (iba);
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			const uint32	*triPtr= (const uint32 *) iba.getPtr();
			for(j=0;j<pb->getNumIndexes(); ++j)
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
			for(j=0;j<pb->getNumIndexes(); ++j)
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
	for(i=0;i<geomorphs.size(); ++i)
	{
		uint	trueIdx= geomorphs[i].End;
		// map to the Geomorph Target.
		vertexUsed[i]= trueIdx;
		// mark also the real vertex used as used.
		vertexUsed[trueIdx]= trueIdx;
	}


	// **** For all vertices used (not geomorphs), compute vertex Skins.
	vector<CVertex>		shadowVertices;
	vector<sint>		vertexToVSkin;
	vertexToVSkin.resize(vertexUsed.size());
	shadowVertices.reserve(vertexUsed.size());
	// use a map to remove duplicates (because of UV/normal discontinuities before!!)
	map<CVertex, uint>	shadowVertexMap;
	uint						numMerged= 0;
	// Skip Geomorphs.
	for(i=geomorphs.size();i<vertexUsed.size(); ++i)
	{
		// If this vertex is used.
		if(vertexUsed[i]!=-1)
		{
			// Build the vertex
			CVertex shadowVert;
			CUV uv;
			shadowVert.vertex = *(CVector*)vba.getVertexCoordPointer(i);
			shadowVert.normal = *(CVector*)vba.getNormalCoordPointer(i);
			shadowVert.uv = *(CUV*)vba.getTexCoordPointer(i);
/*
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
//			shadowVert.MatrixId= matId;
*/
			// If dont find the shadowVertex in the map.
			map<CVertex, uint>::iterator		it= shadowVertexMap.find(shadowVert);
			if(it==shadowVertexMap.end())
			{
				// Append
				uint	index= shadowVertices.size();
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

	ofstream ofs(string(filename + ".obj").c_str());

	for(size_t y = 0; y < shadowVertices.size(); ++y)
	{
		CVector v = shadowVertices[y].vertex;
		CVector vn = shadowVertices[y].normal;
		CUV vt = shadowVertices[y].uv;

		ofs << "v " << v.x << " " << v.y << " " << v.z << endl;
		ofs << "vn " << vn.x << " " << vn.y << " " << vn.z << endl;
		ofs << "vt " << vt.U << " " << vt.V << endl;
	}

	// **** Get All Faces 
	// Final List Of Triangles that match the bone.
	vector<uint32>			shadowTriangles;
	shadowTriangles.reserve(1000);
	// Parse all input tri of the mesh.
	for(i=0; i<meshIn->getNbRdrPass(lodId); ++i)
	{
		ofs << "g pass" << i << endl;

		const CIndexBuffer *pb = getRdrPassPrimitiveBlock(meshIn, lodId, i);
		CIndexBufferRead iba;
		pb->lock (iba);
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			const uint32	*triPtr= (const uint32 *) iba.getPtr();

			for(j=0; j<pb->getNumIndexes(); ++j)
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
			for(j=0; j<pb->getNumIndexes(); ++j)
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

		for(size_t pass = 0; pass<shadowTriangles.size(); pass += 3)
		{
			ofs << "f " << shadowTriangles[pass]+1 << "/" << shadowTriangles[pass]+1 << "/" << shadowTriangles[pass]+1 << " ";
			ofs << shadowTriangles[pass+1]+1 << "/" << shadowTriangles[pass+1]+1 << "/" << shadowTriangles[pass+1]+1 << " ";
			ofs << shadowTriangles[pass+2]+1 << "/" << shadowTriangles[pass+2]+1 << "/" << shadowTriangles[pass+2]+1 << endl;
		}

		shadowTriangles.clear();
	}

	ofs.close();

	return true;
}

/*

TODO: implement this

bool ProcessMesh(const std::string &filename, IShape *shapeMesh)
{
	CMesh *mesh = dynamic_cast<CMesh*>(shapeMesh);

	if (!mesh) return false;

	COFile ofile;

	CMeshGeom* meshIn = (CMeshGeom*)&mesh->getMeshGeom();

	CVertexBuffer vertexBuffer = meshIn->getVertexBuffer();

//	CVertexBufferRead vba;
//	vertexBuffer.lock (vba);

	uint i = vertexBuffer.getNumVertices();

	std::vector<NLMISC::CVector> vertices;
	meshIn->retrieveVertices(vertices);

	std::vector<uint32> indices;
	meshIn->retrieveTriangles(indices);


	// **** For all vertices used (not geomorphs), compute vertex Skins.
	vector<CVertex>		shadowVertices;
	vector<sint>		vertexToVSkin;
	vertexToVSkin.resize(indices.size());
	shadowVertices.reserve(indices.size());
	// use a map to remove duplicates (because of UV/normal discontinuities before!!)
	map<CVertex, uint>	shadowVertexMap;
	uint						numMerged= 0;
	// Skip Geomorphs.
	for(i=0;i<indices.size(); ++i)
	{
		// Build the vertex
		CVertex shadowVert;
		CUV uv;
		shadowVert.vertex = *(CVector*)vba.getVertexCoordPointer(i);
		shadowVert.normal = *(CVector*)vba.getNormalCoordPointer(i);
		shadowVert.uv = *(CUV*)vba.getTexCoordPointer(i);

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

		// If dont find the shadowVertex in the map.
		map<CVertex, uint>::iterator		it= shadowVertexMap.find(shadowVert);
		if(it==shadowVertexMap.end())
		{
			// Append
			uint	index= shadowVertices.size();
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

	ofstream ofs(string(filename + ".obj").c_str());

	for(size_t y = 0; y < shadowVertices.size(); ++y)
	{
		CVector v = shadowVertices[y].vertex;
		CVector vn = shadowVertices[y].normal;
		CUV vt = shadowVertices[y].uv;

		ofs << "v " << v.x << " " << v.y << " " << v.z << endl;
		ofs << "vn " << vn.x << " " << vn.y << " " << vn.z << endl;
		ofs << "vt " << vt.U << " " << vt.V << endl;
	}

	// **** Get All Faces 
	// Final List Of Triangles that match the bone.
	vector<uint32>			shadowTriangles;
	shadowTriangles.reserve(1000);
	// Parse all input tri of the mesh.
	for(i=0; i<meshIn->getNbRdrPass(lodId); ++i)
	{
		ofs << "g pass" << i << endl;

		const CIndexBuffer *pb = getRdrPassPrimitiveBlock(meshIn, lodId, i);
		CIndexBufferRead iba;
		pb->lock (iba);
		if (iba.getFormat() == CIndexBuffer::Indices32)
		{
			const uint32	*triPtr= (const uint32 *) iba.getPtr();

			for(j=0; j<pb->getNumIndexes(); ++j)
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
			for(j=0; j<pb->getNumIndexes(); ++j)
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

		for(size_t pass = 0; pass<shadowTriangles.size(); pass += 3)
		{
			ofs << "f " << shadowTriangles[pass]+1 << "/" << shadowTriangles[pass]+1 << "/" << shadowTriangles[pass]+1 << " ";
			ofs << shadowTriangles[pass+1]+1 << "/" << shadowTriangles[pass+1]+1 << "/" << shadowTriangles[pass+1]+1 << " ";
			ofs << shadowTriangles[pass+2]+1 << "/" << shadowTriangles[pass+2]+1 << "/" << shadowTriangles[pass+2]+1 << endl;
		}

		shadowTriangles.clear();
	}

	ofs.close();

	return true;
}

*/