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

#include "nel/3d/lod_character_shape.h"
#include "nel/3d/mrm_parameters.h"
#include "nel/3d/mesh_mrm.h"

using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

bool  CExportNel::buildLodCharacter (NL3D::CLodCharacterShapeBuild& lodBuild, INode& node, TimeValue time, const TInodePtrInt *nodeMap)
{
	// Is it a tri object ?
	bool res = false;

	// If skinning, disable skin modifier
	if (nodeMap)
		enableSkinModifier (node, false);
	// a lod character must have skinning
	else
		return false;

	// Get a pointer on the object's node
    ObjectState os = node.EvalWorldState(time);
    Object *obj = os.obj;

	// Check if there is an object
	if (obj)
	{
		Class_ID  clid = obj->ClassID();

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

				// Get the node matrix
				Matrix3 nodeMatrixMax;
				CMatrix nodeMatrix;
				getLocalMatrix (nodeMatrixMax, node, time);
				convertMatrix (nodeMatrix, nodeMatrixMax);


				// build the shape
				{					
					// Array of name for the material
					CMaxMeshBaseBuild maxBaseBuild;

					// Fill the build interface of CMesh
					CMeshBase::CMeshBaseBuild buildBaseMesh;
					buildBaseMeshInterface (buildBaseMesh, maxBaseBuild, node, time, nodeMatrix);

					CMesh::CMeshBuild buildMesh;
					buildMeshInterface (*tri, buildMesh, buildBaseMesh, maxBaseBuild, node, time, nodeMap);


					/* Build a mesh. (duplicate UV/normals)
						Use a CMeshMRM for an historic reason: simpler because don't use "matrix blocks".
					*/
					CMRMParameters		mrmParams;
					CMeshMRM			meshMRM;
					// To avoid heavy MRM compute, ask for only one lod (=> no poly reduction)
					mrmParams.Divisor= 1;
					mrmParams.NLods= 1;
					meshMRM.build (buildBaseMesh, buildMesh, std::vector<CMesh::CMeshBuild*>(), mrmParams);
					const CMeshMRMGeom	&meshMRMGeom= meshMRM.getMeshGeom();
					nlassert(meshMRMGeom.getNbLod()==1);

					// Build the lodBuild with the mesh
					const CVertexBuffer &VB= meshMRMGeom.getVertexBuffer();
					uint32	format= VB.getVertexFormat();
					uint	numVerts= VB.getNumVertices();

					// The mesh must have at least skinning.
					if( (format & CVertexBuffer::PositionFlag) && meshMRMGeom.isSkinned() )
					{
						uint		i;
						CVertexBufferRead vba;
						VB.lock (vba);

						// build vertices and skinWeights
						lodBuild.Vertices.resize(numVerts);
						for(i=0;i<numVerts;i++)
							lodBuild.Vertices[i]= *(const CVector*)vba.getVertexCoordPointer(i);
						// copy skinWeights
						lodBuild.SkinWeights= meshMRMGeom.getSkinWeights();
						nlassert(lodBuild.SkinWeights.size() == numVerts);

						// build UVs and normals
						lodBuild.UVs.clear();
						lodBuild.Normals.clear();
						lodBuild.UVs.resize(numVerts, CUV(0,0));
						lodBuild.Normals.resize(numVerts, CVector::K);
						if( format & CVertexBuffer::TexCoord0Flag )
						{
							for(i=0;i<numVerts;i++)
								lodBuild.UVs[i]= *(const CUV*)vba.getTexCoordPointer(i);
						}
						if( format & CVertexBuffer::NormalFlag )
						{
							for(i=0;i<numVerts;i++)
								lodBuild.Normals[i]= *(const CVector*)vba.getNormalCoordPointer(i);
						}

						// build triangles.
						std::vector<bool>	triangleSelection;
						lodBuild.TriangleIndices.resize(buildMesh.Faces.size() * 3);
						triangleSelection.resize(buildMesh.Faces.size(), false);
						uint	dstTriIdx= 0;
						for(i=0;i<meshMRMGeom.getNbRdrPass(0);i++)
						{
							const CIndexBuffer &pb= meshMRMGeom.getRdrPassPrimitiveBlock(0, i);
							CIndexBufferRead iba;
							pb.lock (iba);
							nlassert(dstTriIdx+pb.getNumIndexes() <= lodBuild.TriangleIndices.size());
							// copy the index block
							uint32	*dst= &lodBuild.TriangleIndices[dstTriIdx];
							if(pb.getFormat()==CIndexBuffer::Indices32)
								memcpy(dst, iba.getPtr(), pb.getNumIndexes()*sizeof(uint32));
							else
							{
								nlassert(pb.getFormat()==CIndexBuffer::Indices16);
								uint16	*src= (uint16*)iba.getPtr();
								for(uint n=pb.getNumIndexes();n>0;--n)
									*(dst++)=*(src++);
							}
							// if the material of this pass is the 0th material, flag tris for TextureInfo selection
							if(meshMRMGeom.getRdrPassMaterial(0,i)==0)
							{
								for(uint tri= dstTriIdx/3; tri<dstTriIdx/3+pb.getNumIndexes()/3; tri++)
									triangleSelection[tri]= true;
							}
							// next
							dstTriIdx+= pb.getNumIndexes();
						}
						nlassert(dstTriIdx == lodBuild.TriangleIndices.size());

						// build boneNames
						lodBuild.BonesNames= meshMRMGeom.getBonesName();

						// End: compile texturing information
						lodBuild.compile(triangleSelection);

						// Ok!!
						res= true;
					}
				}

				// Delete the triObject if we should...
				if (deleteIt)
					tri->DeleteThis();
			}
		}
	}

	// If skinning, renable skin modifier
	if (nodeMap)
		enableSkinModifier (node, true);

	return res;
}
