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


#include "std3d.h"

#include "nel/3d/tangent_space_build.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/misc/triangle.h"



namespace NL3D
{


//========================================================================================================================
/** private func : duplicate a mesh build
  * It returns the index of the new texture coordinate or 0 if the conversion couldn't occur..
  */
static uint    DuplicateMBAndAddTexCoord(CMesh::CMeshBuild &outMeshBuild, const CMesh::CMeshBuild &inMeshBuild)
{
	outMeshBuild = inMeshBuild;
	uint   numTexCoord = 0;
	for (uint k = CVertexBuffer::FirstTexCoordValue; k <= CVertexBuffer::FirstTexCoordValue; ++k)
	{
		if (inMeshBuild.VertexFlags & (1 << k))	numTexCoord = 1 + k - CVertexBuffer::FirstTexCoordValue;
	}
	if (numTexCoord == CVertexBuffer::MaxStage) return 0;
	outMeshBuild.VertexFlags = inMeshBuild.VertexFlags | (1 << (numTexCoord + CVertexBuffer::FirstTexCoordValue));
	outMeshBuild.NumCoords[numTexCoord] = 3;
	return numTexCoord;
}

//========== Build one tangent space vector ==================
static void BuildTGSpaceVect(const NLMISC::CVector &normal, const NLMISC::CVector &sGrad, NLMISC::CUVW &result)
{
	// start normalizing the basis
	NLMISC::CVector res = (sGrad - ((sGrad * normal) * normal)).normed();
	result.U = res.x;
	result.V = res.y;
	result.W = res.z;
}

//========== build one tri from a vb and 3 index in it=================
static void BuildTriFromMB(const CMesh::CMeshBuild &mb, const uint index[3], NLMISC::CTriangle &tri)
{
	tri.V0 = mb.Vertices[index[0]];
	tri.V1 = mb.Vertices[index[1]];
	tri.V2 = mb.Vertices[index[2]];
}


//========================================================================================================================
bool	BuildTangentSpace(CMesh::CMeshBuild &outMeshBuild, const CMesh::CMeshBuild &inMeshBuild)
{
	static const NLMISC::CUVW NullUVW(0, 0, 0); // todo add this directly in the CUVW class (for next compile ...)

	/// for each face, we must use a previously computed tangent space vector if other vertex datas are similar
	nlassert(&outMeshBuild != &inMeshBuild);
	uint tgSpaceStage = DuplicateMBAndAddTexCoord(outMeshBuild, inMeshBuild); // format the resulting vb
	if (tgSpaceStage == 0) return false; // unable to create the vb

	uint l, m, n, k;	 // some loop counters

	// Tells for each vertex what faces belong to it
	std::vector<std::vector<uint> > VertToFace(inMeshBuild.Vertices.size());

	// fill VertToFace
	for (l = 0; l < inMeshBuild.Faces.size(); ++l)
	{
		for (m = 0; m < 3; ++m)
		{
			outMeshBuild.Faces[l].Corner[m].Uvws[tgSpaceStage] = NLMISC::CUVW(0, 0, 0);
			VertToFace[outMeshBuild.Faces[l].Corner[m].Vertex].push_back(l);
		}
	}

/* TODO: debug this version
	std::vector<NLMISC::CVector> SGradArray(outMeshBuild.Faces.size());	// SGradient for each face

	// compute sGradient for each face
	for (k = 0; k < outMeshBuild.Faces.size(); ++k)
	{
		CMesh::CFace &f = outMeshBuild.Faces[k];
		NLMISC::CTriangle tri;
		tri.V0 = outMeshBuild.Vertices[f.Corner[0].Vertex];
		tri.V1 = outMeshBuild.Vertices[f.Corner[1].Vertex];
		tri.V2 = outMeshBuild.Vertices[f.Corner[2].Vertex];
		tri.computeGradient(f.Corner[0].Uvws[0].U,
						    f.Corner[1].Uvws[0].U,
							f.Corner[2].Uvws[0].U, SGradArray[k]);
		SGradArray[k].normalize();
	}


	// for each triangle, add the S gradient contribution to any neighbour vertex for which the owning face has TexCoords that do not mirror with that face
	for (k = 0; k < outMeshBuild.Faces.size(); ++k)
	{
		CMesh::CFace &f = outMeshBuild.Faces[k];
		for (l = 0; l < 3; ++l)
		{
			const std::vector<uint> &neighbours = VertToFace[f.Corner[l].Vertex];
			for (m = 0; m < neighbours.size(); ++m)
			{
				// other face must share smoothgroups with this one
				if (f.SmoothGroup & outMeshBuild.Faces[neighbours[m]].SmoothGroup)
				{
					// test if the other face UVs are not mirroring the current ones..
					float dp = SGradArray[k] * SGradArray[neighbours[m]];
					if (dp > 0.f)
					{
						f.Corner[l].Uvws[tgSpaceStage] += NLMISC::CUVW(SGradArray[neighbours[m]].x, SGradArray[neighbours[m]].y, SGradArray[neighbours[m]].z);
					}
				}
			}
		}
	}

	// normalize each tangent space vector
	for (k = 0; k < outMeshBuild.Faces.size(); ++k)
	{
		CMesh::CFace &f = outMeshBuild.Faces[k];
		for (l = 0; l < 3; ++l)
		{
			CMesh::CCorner &c = f.Corner[l];
			CVector tgs(c.Uvws[tgSpaceStage].U, c.Uvws[tgSpaceStage].V, c.Uvws[tgSpaceStage].W);
			tgs = (tgs - (tgs * c.Normal) * c.Normal).normed();
			c.Uvws[tgSpaceStage].U = tgs.x;
			c.Uvws[tgSpaceStage].V = tgs.y;
			c.Uvws[tgSpaceStage].W = tgs.z;
		}
	}
*/

	// Old tangent space version (no support for mirrored textures ..)
	for (l = 0; l < inMeshBuild.Faces.size(); ++l)
	{

		CMesh::CFace &curF = outMeshBuild.Faces[l];


		// Build each tangent space vector if needed
		for (m = 0; m < 3; ++m)
		{
			uint vertIndex = outMeshBuild.Faces[l].Corner[m].Vertex;
			bool found = false;
			NLMISC::CUVW *tsv=0; // a previously computed tangent space vector
			// Test whether it hasn't been built before, by looking in each corner of each face that share that vertex
			for (n = 0; n < VertToFace[vertIndex].size() && !found; ++n)
			{
				CMesh::CFace &f = outMeshBuild.Faces[VertToFace[vertIndex][n]]; // ref to the current face
				for (k = 0; k < 3; ++k)
				{
					// can only share with corners that are equal to this one
					if (f.Corner[k].Vertex == curF.Corner[m].Vertex) // same position
					{
						if (f.Corner[k].Uvws[tgSpaceStage] != NullUVW) // must have built the tangent space vector previously.
						{
							if (f.Corner[k].Normal == curF.Corner[m].Normal
								&& f.Corner[k].Uvws[0] ==  curF.Corner[m].Uvws[0]
							   )
							{
								// no texture and normal discontinuity
								found = true;
								tsv = &f.Corner[k].Uvws[tgSpaceStage];
							}
						}
					}
				}
			}

			if (!found)
			{
				NLMISC::CVector grad(0, 0, 0);
				// walk all the triangles around this vertex to sum the gradients
				// Get the s coordinate gradient over that triangle
				for (n = 0; n < VertToFace[vertIndex].size(); ++n)
				{
					CMesh::CFace &f = outMeshBuild.Faces[VertToFace[vertIndex][n]]; // ref to the current face

					// before to sum this face gradient, make sure there's no normal or mapping discontinuity
					bool canShare = true;
					for (k = 0; k < 3; ++k)
					{
						// can only share with corners that are equal to this one
						if (f.Corner[k].Vertex == curF.Corner[m].Vertex) // same position
						{
							if (!(f.Corner[k].Normal == curF.Corner[m].Normal
								  && f.Corner[k].Uvws[0] ==  curF.Corner[m].Uvws[0]
							     )
							   )
							{
								canShare = false;
								break;
							}
						}
					}

					if (!canShare) continue;

					// Get indices of vertices of current tri
					const uint indices[] = { (uint)f.Corner[0].Vertex,
											 (uint)f.Corner[1].Vertex,
											 (uint)f.Corner[2].Vertex };
					NLMISC::CTriangle tri;
					// Build it
					BuildTriFromMB(outMeshBuild, indices, tri);
					// Get s coordinates for each corner
					float s[3];
					for (k = 0; k < 3; ++k)
					{
						s[k] = f.Corner[k].Uvws[0].U;
					}

					NLMISC::CVector sGrad;
					tri.computeGradient(s[0], s[1], s[2], sGrad);
					grad += ((tri.V1 - tri.V0) ^ (tri.V2 - tri.V0)).norm() * sGrad; // ponderate by twice the area
				}
				grad.normalize();

				// build new tangent space vector
				BuildTGSpaceVect(inMeshBuild.Faces[l].Corner[m].Normal, grad, curF.Corner[m].Uvws[tgSpaceStage]);
			}
			else
			{
				// use previously built vector
				curF.Corner[m].Uvws[tgSpaceStage] = *tsv;
			}
		}
	}
	return true;
}





}
